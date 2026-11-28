// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QByteArrayView>
#include <QString>

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
}
