// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>

class KCoreDirLister;
class KFileItem;

/**
 * Folders hidden from their parent folder's listing by the "hidden" flag in
 * their own .mfm.json, as in Max File Manager. Views treat such a folder like
 * a folder whose name starts with a dot: they list it only while they show
 * hidden files.
 *
 * Only local folders on local storage are checked, through the size-bounded
 * FolderMetadata reader; remote locations and network mounts ignore the
 * flag, so listings never wait for them. Decisions are cached per folder
 * until a tracked listing lists the parent folder again, refreshes or
 * deletes the folder, or Plasma Opus changes the flag with setHidden().
 */
class HiddenFolders : public QObject
{
    Q_OBJECT

public:
    /// The cache shared by every view.
    static HiddenFolders *instance();

    explicit HiddenFolders(QObject *parent = nullptr);

    /**
     * Whether a view hides @p item: a folder on local storage whose own
     * metadata sets the flag, while the view doesn't show hidden files.
     */
    bool hidesItem(const KFileItem &item, bool showHiddenFiles);
    /// Sets or clears the flag through FolderMetadata, forgets the cached decision and emits changed().
    bool setHidden(const QString &folder, bool hidden, QString *error = nullptr);

    /// Forgets the cached decision for @p folder.
    void forget(const QString &folder);
    /// Forgets the cached decisions for the folders directly inside @p parent.
    void forgetChildren(const QString &parent);
    /// Reads the flag of the cached @p folders again and emits changed() when a decision changes.
    void refresh(const QStringList &folders);
    /**
     * Keeps the cache current with @p lister's listings: listing a local
     * folder forgets the decisions for its subfolders, refreshed folders are
     * read again and deleted folders are forgotten.
     */
    void track(KCoreDirLister *lister);

    /// Whether a decision for @p folder is cached; for tests.
    bool isCached(const QString &folder) const;
    /// Forgets every cached decision.
    void clear();

Q_SIGNALS:
    /// A folder's flag changed, so views should filter their rows again.
    void changed();

private:
    enum class Decision {
        Shown,
        Hidden,
        /// On remote storage, where the flag isn't read.
        Ignored,
    };

    QHash<QString, Decision> m_cache;
};
