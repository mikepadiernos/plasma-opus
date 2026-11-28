// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QByteArrayView>
#include <QString>

#include <functional>
#include <optional>

#include <sys/types.h>

/// Shared rules for publishing new files that are written privately first.
namespace FilePublish
{
/// Reads the octal value of the "Umask:" line of a /proc/<pid>/status
/// document, or nothing when the line is missing or malformed.
std::optional<mode_t> parseStatusUmask(QByteArrayView status);

/// The process umask, read from /proc/self/status without changing it. If
/// that is unavailable, it is read by setting and restoring the umask under a
/// lock, using 077 for the moment it is set so that files other threads create
/// meanwhile can only end up more private, never more open.
mode_t currentUmask();

/// The mode open(2) gives new regular files: 0666 masked by the umask.
mode_t defaultFileMode();

/// Gives a fully written file, still open on @p fd, the default creation
/// mode. Call it after the last write and before the file is published, so
/// that the file is never more open while partly written and never published
/// owner-only. Filesystems whose permissions come from mount options, such as
/// FAT, refuse the change; the file then keeps the mode they assign and this
/// still succeeds. Returns an error message for any other failure.
QString applyDefaultMode(int fd);

/// How publishNoReplace() put a file in place, or the step that failed.
enum class Method {
    Rename,
    Link,
    Copy,
};

struct Publication {
    /// Empty when the file was published.
    QString error;
    Method method = Method::Rename;
};

/// Publishes @p temporary, a fully written regular file that is still open on
/// @p temporaryFd and already has its final mode, as @p destination in the same
/// directory. An existing entry at @p destination, including a dangling
/// symbolic link, is never replaced, even if it appears after the caller
/// checked for it:
///
/// 1. An atomic no-replace rename.
/// 2. If the filesystem doesn't support that, a hard link followed by
///    removing @p temporary, which is also atomic and refuses existing names.
/// 3. If hard links aren't supported either, as on FAT, exFAT and some FUSE
///    filesystems, an exclusively created, owner-only file that receives a copy
///    of the contents and then the default creation mode. This is not atomic:
///    while copying, a partly written file exists at @p destination. If the
///    copy fails, the file is removed again, but only while @p destination is
///    still the file this call created.
///
/// On success, @p temporary has been moved or removed, so the caller must not
/// remove it by name: another file may have taken the name since. On failure,
/// it is left for the caller to remove.
Publication publishNoReplace(int temporaryFd, const QString &temporary, const QString &destination);

/// Test hooks for simulating filesystems without no-replace renames or hard
/// links. Not thread-safe; for tests only.
namespace Testing
{
/// Each unset member uses the real system call. Hooks return -1 and set errno
/// to fail.
struct Syscalls {
    /// renameat2() with RENAME_NOREPLACE.
    std::function<int(const char *from, const char *to)> rename;
    /// linkat() without following symbolic links.
    std::function<int(const char *from, const char *to)> link;
    /// write() while copying.
    std::function<ssize_t(int fd, const void *data, size_t size)> write;
};

void setSyscalls(const Syscalls &syscalls);
}
}
