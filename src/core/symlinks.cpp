// SPDX-License-Identifier: GPL-3.0-or-later

#include "symlinks.h"

#include <KIO/Global>
#include <KIO/SimpleJob>
#include <KLocalizedString>
#include <KProtocolInfo>
#include <KProtocolManager>

#include <QDir>
#include <QFileInfo>

namespace
{
bool sameServer(const QUrl &a, const QUrl &b)
{
    return a.scheme() == b.scheme() && a.host().compare(b.host(), Qt::CaseInsensitive) == 0 && a.port() == b.port() && a.userName() == b.userName();
}

QString relativePath(const QString &folder, const QString &target)
{
    const QString relative = QDir(folder).relativeFilePath(target);
    return relative.isEmpty() ? QStringLiteral(".") : relative;
}

// Resolves the folders leading to a local target but not the target itself,
// which may be a link that the new link should point at.
QString canonicalTarget(const QString &path)
{
    const QFileInfo info(path);
    const QString parent = QFileInfo(info.path()).canonicalFilePath();
    if (parent.isEmpty()) {
        return path;
    }
    return info.fileName().isEmpty() ? parent : QDir(parent).filePath(info.fileName());
}

QString childPath(const QString &folder, const QString &name)
{
    if (folder.endsWith(QLatin1Char('/'))) {
        return folder + name;
    }
    return folder + QLatin1Char('/') + name;
}
}

QUrl Symlinks::resolve(const QUrl &url)
{
    if (!url.isLocalFile()) {
        return url;
    }

    const QString canonical = QFileInfo(url.toLocalFile()).canonicalFilePath();
    return canonical.isEmpty() ? url : QUrl::fromLocalFile(canonical);
}

bool Symlinks::supportsLinking(const QUrl &url)
{
    if (url.isLocalFile()) {
        return true;
    }
    // Only network workers address paths on their host; virtual locations
    // such as desktop:, trash: or search results don't.
    return url.isValid() && !url.host().isEmpty() && KProtocolInfo::protocolClass(url.scheme()) == QLatin1String(":internet")
        && KProtocolManager::supportsLinking(url);
}

QUrl Symlinks::parentFolder(const QUrl &target)
{
    return KIO::upUrl(target);
}

QString Symlinks::destinationError(const QUrl &target, const QUrl &folder)
{
    const QString targetError = i18nc("@info", "The link target must be an existing file or folder.");
    const QString folderError = i18nc("@info", "The destination must be an existing folder.");
    if (!target.isValid() || target.isEmpty()) {
        return targetError;
    }
    if (!folder.isValid() || folder.isEmpty()) {
        return folderError;
    }
    if (target.isLocalFile() != folder.isLocalFile() || (!target.isLocalFile() && !sameServer(target, folder))) {
        return i18nc("@info", "The link and its target must both be local or on the same remote host.");
    }
    if (!supportsLinking(target)) {
        return i18nc("@info %1 is a URL scheme such as ftp", "Symbolic links can't be created in %1 locations.", target.scheme());
    }
    if (target.isLocalFile()) {
        const QString path = target.toLocalFile();
        if (!QDir::isAbsolutePath(path) || !QFileInfo::exists(path)) {
            return targetError;
        }
        const QString dir = folder.toLocalFile();
        return QDir::isAbsolutePath(dir) && QFileInfo(dir).isDir() ? QString() : folderError;
    }
    if (!target.path().startsWith(QLatin1Char('/'))) {
        return targetError;
    }
    return folder.path().startsWith(QLatin1Char('/')) ? QString() : folderError;
}

QString Symlinks::nameError(const QString &name)
{
    if (name.isEmpty() || name == QLatin1String(".") || name == QLatin1String("..") || name.contains(QLatin1Char('/')) || name.contains(QChar::Null)) {
        return i18nc("@info", "Enter a valid link name without a slash.");
    }
    return {};
}

QString Symlinks::defaultName(const QString &targetName, const QString &suffix)
{
    return suffix.isEmpty() ? i18nc("@item:intext default symbolic link name", "%1 - link", targetName) : targetName + suffix;
}

Symlinks::Plan Symlinks::plan(const QUrl &target, const QUrl &folder, const QString &name, TargetForm form)
{
    Plan result;
    result.error = destinationError(target, folder);
    if (result.error.isEmpty()) {
        result.error = nameError(name);
    }
    if (!result.error.isEmpty()) {
        return result;
    }

    if (target.isLocalFile()) {
        const QString targetPath = QDir::cleanPath(target.toLocalFile());
        const QString folderPath = QDir::cleanPath(folder.toLocalFile());
        const QString linkPath = childPath(folderPath, name);
        const QFileInfo existing(linkPath);
        if (existing.exists() || existing.isSymLink()) {
            result.error = i18nc("@info", "An item named “%1” already exists in the destination folder.", name);
            return result;
        }
        result.link = QUrl::fromLocalFile(linkPath);
        result.target = form == TargetForm::Relative ? relativePath(QFileInfo(folderPath).canonicalFilePath(), canonicalTarget(targetPath)) : targetPath;
        return result;
    }

    const QString targetPath = QDir::cleanPath(target.path());
    const QString folderPath = QDir::cleanPath(folder.path());
    result.link = folder.adjusted(QUrl::RemoveQuery | QUrl::RemoveFragment);
    result.link.setPath(childPath(folderPath, name), QUrl::DecodedMode);
    result.target = form == TargetForm::Relative ? relativePath(folderPath, targetPath) : targetPath;
    return result;
}

KIO::SimpleJob *Symlinks::create(const QUrl &target, const QUrl &folder, const QString &name, TargetForm form, QString &error)
{
    const Plan request = plan(target, folder, name, form);
    error = request.error;
    if (!error.isEmpty()) {
        return nullptr;
    }
    // Without KIO::Overwrite the worker refuses an existing name, including
    // one created after planning.
    return KIO::symlink(request.target, request.link, KIO::HideProgressInfo);
}
