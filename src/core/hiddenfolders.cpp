// SPDX-License-Identifier: GPL-3.0-or-later

#include "hiddenfolders.h"

#include "foldermetadata.h"

#include <KCoreDirLister>
#include <KFileItem>
#include <kio_version.h>

#include <QDir>
#include <QFileInfo>
#include <QUrl>

namespace
{
/// Enough for the folders of many large listings; the cache starts again beyond it.
constexpr qsizetype CacheLimit = 50000;

QString key(const QString &path)
{
    return path.isEmpty() ? QString() : QDir::cleanPath(path);
}

bool readFlag(const QString &folder)
{
    return FolderMetadata::hidden(FolderMetadata::read(folder));
}

bool onRemoteStorage(const KFileItem &item)
{
#if KIO_VERSION >= QT_VERSION_CHECK(6, 32, 0)
    return item.isOnRemoteStorage();
#else
    return item.isSlow();
#endif
}
}

HiddenFolders *HiddenFolders::instance()
{
    static HiddenFolders folders;
    return &folders;
}

HiddenFolders::HiddenFolders(QObject *parent)
    : QObject(parent)
{
}

bool HiddenFolders::hidesItem(const KFileItem &item, bool showHiddenFiles)
{
    if (showHiddenFiles || item.isNull() || !item.isDir() || !item.isLocalFile()) {
        return false;
    }
    const QString folder = key(item.localPath());
    if (folder.isEmpty()) {
        return false;
    }
    if (const auto found = m_cache.constFind(folder); found != m_cache.cend()) {
        return found.value() == Decision::Hidden;
    }
    if (m_cache.size() >= CacheLimit) {
        m_cache.clear();
    }
    const Decision decision = onRemoteStorage(item) ? Decision::Ignored : readFlag(folder) ? Decision::Hidden : Decision::Shown;
    m_cache.insert(folder, decision);
    return decision == Decision::Hidden;
}

bool HiddenFolders::setHidden(const QString &folder, bool hidden, QString *error)
{
    if (!FolderMetadata::setHidden(folder, hidden, error)) {
        return false;
    }
    forget(folder);
    Q_EMIT changed();
    return true;
}

void HiddenFolders::forget(const QString &folder)
{
    m_cache.remove(key(folder));
}

void HiddenFolders::forgetChildren(const QString &parent)
{
    const QString folder = key(parent);
    if (folder.isEmpty()) {
        return;
    }
    m_cache.removeIf([&folder](const QHash<QString, Decision>::iterator &it) {
        return QFileInfo(it.key()).path() == folder && it.key() != folder;
    });
}

void HiddenFolders::refresh(const QStringList &folders)
{
    bool anyChanged = false;
    for (const QString &path : folders) {
        const auto found = m_cache.find(key(path));
        if (found == m_cache.end() || found.value() == Decision::Ignored) {
            continue;
        }
        const Decision decision = readFlag(found.key()) ? Decision::Hidden : Decision::Shown;
        if (decision != found.value()) {
            found.value() = decision;
            anyChanged = true;
        }
    }
    if (anyChanged) {
        Q_EMIT changed();
    }
}

void HiddenFolders::track(KCoreDirLister *lister)
{
    connect(lister, &KCoreDirLister::started, this, [this](const QUrl &url) {
        if (url.isLocalFile()) {
            forgetChildren(url.toLocalFile());
        }
    });
    connect(lister, &KCoreDirLister::refreshItems, this, [this](const QList<QPair<KFileItem, KFileItem>> &items) {
        QStringList folders;
        for (const auto &[before, after] : items) {
            if (before.isLocalFile() && before.localPath() != after.localPath()) {
                forget(before.localPath());
            }
            if (after.isLocalFile() && after.isDir()) {
                folders.append(after.localPath());
            }
        }
        refresh(folders);
    });
    connect(lister, &KCoreDirLister::itemsDeleted, this, [this](const KFileItemList &items) {
        for (const KFileItem &item : items) {
            if (item.isLocalFile()) {
                forget(item.localPath());
            }
        }
    });
}

bool HiddenFolders::isCached(const QString &folder) const
{
    return m_cache.contains(key(folder));
}

void HiddenFolders::clear()
{
    m_cache.clear();
}
