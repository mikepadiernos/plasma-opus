// SPDX-License-Identifier: GPL-3.0-or-later

#include "filepublish.h"

#include <KLocalizedString>

#include <QByteArray>
#include <QFile>

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <linux/fs.h>
#include <mutex>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

namespace
{
FilePublish::Testing::Syscalls &syscalls()
{
    static FilePublish::Testing::Syscalls hooks;
    return hooks;
}

int renameNoReplace(const char *from, const char *to)
{
    return syscalls().rename ? syscalls().rename(from, to) : ::renameat2(AT_FDCWD, from, AT_FDCWD, to, RENAME_NOREPLACE);
}

int linkNoFollow(const char *from, const char *to)
{
    return syscalls().link ? syscalls().link(from, to) : ::linkat(AT_FDCWD, from, AT_FDCWD, to, 0);
}

ssize_t writeSome(int fd, const void *data, size_t size)
{
    return syscalls().write ? syscalls().write(fd, data, size) : ::write(fd, data, size);
}

QString systemError(int error)
{
    return QString::fromLocal8Bit(std::strerror(error));
}

// Errors with which filesystems such as FUSE, CIFS and older ones refuse RENAME_NOREPLACE itself.
bool renameUnsupported(int error)
{
    return error == EINVAL || error == ENOSYS || error == EOPNOTSUPP;
}

// Errors with which filesystems such as FAT, exFAT and some FUSE ones refuse hard links.
bool linkUnsupported(int error)
{
    return error == EPERM || error == EOPNOTSUPP || error == ENOSYS || error == EXDEV;
}

bool sameFile(const struct stat &first, const struct stat &second)
{
    return first.st_dev == second.st_dev && first.st_ino == second.st_ino;
}

// Whether path is a name of the file open on fd, without following a symbolic link.
bool isFileAt(const char *path, int fd)
{
    struct stat opened;
    struct stat named;
    return ::fstat(fd, &opened) == 0 && ::lstat(path, &named) == 0 && sameFile(opened, named);
}

QString copyContents(int from, int to)
{
    char buffer[64 * 1024];
    off_t offset = 0;
    while (true) {
        const ssize_t count = ::pread(from, buffer, sizeof buffer, offset);
        if (count < 0 && errno == EINTR) {
            continue;
        }
        if (count < 0) {
            return systemError(errno);
        }
        if (count == 0) {
            return {};
        }
        ssize_t written = 0;
        while (written < count) {
            const ssize_t step = writeSome(to, buffer + written, size_t(count - written));
            if (step < 0 && errno == EINTR) {
                continue;
            }
            if (step <= 0) {
                return systemError(step < 0 ? errno : EIO);
            }
            written += step;
        }
        offset += count;
    }
}

// Removes path only while it is still the file that was created, so that a file another program put there
// meanwhile is kept. Returns whether nothing of that file remains under path.
bool removeCreated(const char *path, const struct stat &created)
{
    struct stat current;
    if (::lstat(path, &current) != 0) {
        return errno == ENOENT;
    }
    return sameFile(current, created) && ::unlink(path) == 0;
}

FilePublish::Publication publishCopy(int temporaryFd, const char *from, const char *to, const QString &destination)
{
    using FilePublish::Method;
    // O_EXCL refuses any existing entry, including a dangling symbolic link, so this still never replaces anything.
    // Unlike a rename or link, though, the copy is not atomic: until it finishes, a partly written file is visible
    // under the destination name. It stays owner-only until complete.
    const int output = ::open(to, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
    if (output < 0) {
        return {systemError(errno), Method::Copy};
    }
    struct stat created;
    if (::fstat(output, &created) != 0) {
        const int error = errno;
        ::close(output);
        return {i18nc("@info", "Could not write %1: %2. An incomplete file may remain there.", destination, systemError(error)), Method::Copy};
    }
    QString error = copyContents(temporaryFd, output);
    if (error.isEmpty()) {
        error = FilePublish::applyDefaultMode(output);
    }
    // Some network filesystems report write errors only when the file is closed.
    if (::close(output) != 0 && error.isEmpty()) {
        error = systemError(errno);
    }
    if (!error.isEmpty()) {
        return {removeCreated(to, created) ? i18nc("@info", "Could not write %1: %2", destination, error)
                                           : i18nc("@info", "Could not write %1: %2. An incomplete file may remain there.", destination, error),
                Method::Copy};
    }
    ::unlink(from);
    return {{}, Method::Copy};
}

std::optional<QByteArray> readProcStatus()
{
    const int fd = ::open("/proc/self/status", O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        return std::nullopt;
    }
    QByteArray status;
    char buffer[4096];
    while (true) {
        const ssize_t count = ::read(fd, buffer, sizeof buffer);
        if (count < 0 && errno == EINTR) {
            continue;
        }
        if (count <= 0) {
            ::close(fd);
            return count == 0 ? std::optional<QByteArray>(status) : std::nullopt;
        }
        status.append(buffer, count);
    }
}
}

std::optional<mode_t> FilePublish::parseStatusUmask(QByteArrayView status)
{
    while (!status.isEmpty()) {
        const qsizetype end = status.indexOf('\n');
        const QByteArrayView line = end < 0 ? status : status.first(end);
        status = end < 0 ? QByteArrayView() : status.sliced(end + 1);
        if (!line.startsWith("Umask:")) {
            continue;
        }
        bool ok = false;
        const uint value = line.sliced(6).trimmed().toUInt(&ok, 8);
        if (!ok || value > 0777) {
            return std::nullopt;
        }
        return static_cast<mode_t>(value);
    }
    return std::nullopt;
}

mode_t FilePublish::currentUmask()
{
    if (const std::optional<QByteArray> status = readProcStatus()) {
        if (const std::optional<mode_t> mask = parseStatusUmask(*status)) {
            return *mask;
        }
    }
    static std::mutex lock;
    const std::lock_guard guard(lock);
    const mode_t mask = ::umask(077);
    ::umask(mask);
    return mask;
}

mode_t FilePublish::defaultFileMode()
{
    return 0666 & ~currentUmask();
}

QString FilePublish::applyDefaultMode(int fd)
{
    if (::fchmod(fd, defaultFileMode()) == 0 || errno == EPERM || errno == EOPNOTSUPP) {
        return {};
    }
    return QString::fromLocal8Bit(std::strerror(errno));
}

FilePublish::Publication FilePublish::publishNoReplace(int temporaryFd, const QString &temporary, const QString &destination)
{
    const QByteArray from = QFile::encodeName(temporary);
    const QByteArray to = QFile::encodeName(destination);
    if (renameNoReplace(from.constData(), to.constData()) == 0) {
        return {{}, Method::Rename};
    }
    if (!renameUnsupported(errno)) {
        return {systemError(errno), Method::Rename};
    }
    if (linkNoFollow(from.constData(), to.constData()) == 0) {
        ::unlink(from.constData());
        return {{}, Method::Link};
    }
    const int linkError = errno;
    // A network filesystem can report EEXIST for a retried link that did succeed; the name is then this very file.
    if (linkError == EEXIST && isFileAt(to.constData(), temporaryFd)) {
        ::unlink(from.constData());
        return {{}, Method::Link};
    }
    if (!linkUnsupported(linkError)) {
        return {systemError(linkError), Method::Link};
    }
    return publishCopy(temporaryFd, from.constData(), to.constData(), destination);
}

void FilePublish::Testing::setSyscalls(const Syscalls &hooks)
{
    syscalls() = hooks;
}
