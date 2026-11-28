// SPDX-License-Identifier: GPL-3.0-or-later

#include "hiddenfolders.h"
#include "foldermetadata.h"

#include <KCoreDirLister>
#include <KFileItem>

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

#include <sys/stat.h>

class HiddenFoldersTest : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir m_dir;
    QString m_root;

    static KFileItem folderItem(const QString &path)
    {
        return KFileItem(QUrl::fromLocalFile(path), QStringLiteral("inode/directory"), S_IFDIR);
    }

    QString makeFolder(const QString &name, bool hidden)
    {
        const QString path = m_root + QLatin1Char('/') + name;
        if (!QDir().mkpath(path)) {
            return {};
        }
        if (hidden && !FolderMetadata::setHidden(path, true)) {
            return {};
        }
        return path;
    }

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
        QVERIFY(m_dir.isValid());
        m_root = QDir(m_dir.path()).canonicalPath();
    }

    void hidesFlaggedFoldersUnlessHiddenFilesAreShown()
    {
        HiddenFolders folders;
        const QString flagged = makeFolder(QStringLiteral("flagged"), true);
        const QString plain = makeFolder(QStringLiteral("plain"), false);
        QVERIFY(!flagged.isEmpty());
        QVERIFY(!plain.isEmpty());

        QVERIFY(folders.hidesItem(folderItem(flagged), false));
        QVERIFY(!folders.hidesItem(folderItem(flagged), true));
        QVERIFY(!folders.hidesItem(folderItem(plain), false));
        QVERIFY(!folders.hidesItem(KFileItem(), false));
    }

    void ignoresFilesAndRemoteFolders()
    {
        HiddenFolders folders;
        const QString flagged = makeFolder(QStringLiteral("flaggedFile"), true);
        QVERIFY(!flagged.isEmpty());
        // A file can't carry its own metadata, even when a folder of that name would.
        QVERIFY(!folders.hidesItem(KFileItem(QUrl::fromLocalFile(flagged), QStringLiteral("text/plain"), S_IFREG), false));
        QVERIFY(!folders.isCached(flagged));

        const KFileItem remote(QUrl(QStringLiteral("sftp://example.invalid/flagged")), QStringLiteral("inode/directory"), S_IFDIR);
        QVERIFY(!folders.hidesItem(remote, false));
        QVERIFY(!folders.isCached(QStringLiteral("/flagged")));
    }

    void cachesDecisionsUntilForgotten()
    {
        HiddenFolders folders;
        const QString folder = makeFolder(QStringLiteral("cached"), false);
        QVERIFY(!folder.isEmpty());
        QVERIFY(!folders.isCached(folder));
        QVERIFY(!folders.hidesItem(folderItem(folder), true));
        QVERIFY(!folders.isCached(folder));

        QVERIFY(!folders.hidesItem(folderItem(folder), false));
        QVERIFY(folders.isCached(folder));
        QVERIFY(FolderMetadata::setHidden(folder, true));
        // The cached decision stands until something reports the change.
        QVERIFY(!folders.hidesItem(folderItem(folder), false));

        folders.forget(folder + QLatin1Char('/'));
        QVERIFY(!folders.isCached(folder));
        QVERIFY(folders.hidesItem(folderItem(folder), false));
    }

    void forgetsChildrenOfARelistedFolder()
    {
        HiddenFolders folders;
        const QString parent = makeFolder(QStringLiteral("parent"), false);
        const QString child = makeFolder(QStringLiteral("parent/child"), false);
        const QString grandchild = makeFolder(QStringLiteral("parent/child/grandchild"), false);
        QVERIFY(!grandchild.isEmpty());
        for (const QString &path : {parent, child, grandchild}) {
            folders.hidesItem(folderItem(path), false);
            QVERIFY(folders.isCached(path));
        }

        folders.forgetChildren(parent);
        QVERIFY(folders.isCached(parent));
        QVERIFY(!folders.isCached(child));
        QVERIFY(folders.isCached(grandchild));
    }

    void refreshReportsChangedFlags()
    {
        HiddenFolders folders;
        const QString folder = makeFolder(QStringLiteral("refreshed"), false);
        const QString other = makeFolder(QStringLiteral("uncached"), false);
        QVERIFY(!other.isEmpty());
        QSignalSpy changed(&folders, &HiddenFolders::changed);
        QVERIFY(!folders.hidesItem(folderItem(folder), false));

        folders.refresh({folder, other});
        QCOMPARE(changed.count(), 0);
        QVERIFY(!folders.isCached(other));

        QVERIFY(FolderMetadata::setHidden(folder, true));
        folders.refresh({folder});
        QCOMPARE(changed.count(), 1);
        QVERIFY(folders.hidesItem(folderItem(folder), false));
        folders.refresh({folder});
        QCOMPARE(changed.count(), 1);
    }

    void setHiddenWritesMetadataAndReportsTheChange()
    {
        HiddenFolders folders;
        const QString folder = makeFolder(QStringLiteral("toggled"), false);
        QSignalSpy changed(&folders, &HiddenFolders::changed);
        QVERIFY(!folders.hidesItem(folderItem(folder), false));

        QVERIFY(folders.setHidden(folder, true));
        QCOMPARE(changed.count(), 1);
        QVERIFY(FolderMetadata::hidden(FolderMetadata::read(folder)));
        QVERIFY(folders.hidesItem(folderItem(folder), false));

        QVERIFY(folders.setHidden(folder, false));
        QCOMPARE(changed.count(), 2);
        QVERIFY(!folders.hidesItem(folderItem(folder), false));
        QVERIFY(!QFile::exists(folder + QStringLiteral("/.mfm.json")));
    }

    void setHiddenKeepsUnreadableMetadata()
    {
        HiddenFolders folders;
        const QString folder = makeFolder(QStringLiteral("unreadable"), false);
        QFile file(folder + QStringLiteral("/.mfm.json"));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("{invalid");
        file.close();
        QSignalSpy changed(&folders, &HiddenFolders::changed);

        QVERIFY(!folders.setHidden(folder, true));
        QString error;
        QVERIFY(!folders.setHidden(folder, true, &error));
        QVERIFY(!error.isEmpty());
        QCOMPARE(changed.count(), 0);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.readAll(), QByteArrayLiteral("{invalid"));
        QVERIFY(!folders.hidesItem(folderItem(folder), false));
    }

    void trackedListingForgetsItsSubfolders()
    {
        HiddenFolders folders;
        const QString parent = makeFolder(QStringLiteral("listed"), false);
        const QString child = makeFolder(QStringLiteral("listed/child"), false);
        QVERIFY(!folders.hidesItem(folderItem(child), false));
        QVERIFY(folders.isCached(child));

        KCoreDirLister lister;
        lister.setAutoErrorHandlingEnabled(false);
        folders.track(&lister);
        QSignalSpy completed(&lister, qOverload<>(&KCoreDirLister::completed));
        QVERIFY(lister.openUrl(QUrl::fromLocalFile(parent)));
        QVERIFY(completed.wait() || completed.count() > 0);
        QVERIFY(!folders.isCached(child));
    }
};

QTEST_GUILESS_MAIN(HiddenFoldersTest)

#include "hiddenfolderstest.moc"
