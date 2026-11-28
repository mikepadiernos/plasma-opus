// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QFile>
#include <QString>

#include <sys/stat.h>

/// Sets the process umask for a scope and restores the previous one, even
/// when a test fails early.
class ScopedUmask
{
public:
    explicit ScopedUmask(mode_t mask)
        : m_previous(::umask(mask))
    {
    }
    ~ScopedUmask()
    {
        ::umask(m_previous);
    }
    ScopedUmask(const ScopedUmask &) = delete;
    ScopedUmask &operator=(const ScopedUmask &) = delete;

private:
    mode_t m_previous;
};

/// The permission bits of @p path, without following a final symbolic link,
/// or -1 if it cannot be read.
inline int fileMode(const QString &path)
{
    struct stat status;
    return ::lstat(QFile::encodeName(path).constData(), &status) == 0 ? int(status.st_mode & 07777) : -1;
}
