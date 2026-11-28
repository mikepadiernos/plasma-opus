// SPDX-License-Identifier: GPL-3.0-or-later

#include "filepublish.h"

#include <QByteArray>

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <mutex>
#include <sys/stat.h>
#include <unistd.h>

namespace
{
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
