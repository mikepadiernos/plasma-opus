// SPDX-License-Identifier: GPL-3.0-or-later

#include "foldermetadata.h"
#include "folderpreview.h"
#include "foldertree.h"
#include "folderview.h"
#include "hiddenfolders.h"
#include "yazicolumns.h"

#include <KDirModel>
#include <KFileItem>

#include <QDir>
#include <QFile>
#include <QListView>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <QTreeView>

#include <sys/stat.h>

class HiddenFoldersUiTest : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir m_dir;
    QString m_root;
    QString m_visible;
    QString m_flagged;

    QUrl url(const QString &relative = QString()) const
    {
        return QUrl::fromLocalFile(relative.isEmpty() ? m_root : m_root + QLatin1Char('/') + relative);
    }

    static QStringList names(const QAbstractItemModel *model, const QModelIndex &parent = {})
    {
        QStringList result;
        for (int row = 0; row < model->rowCount(parent); ++row) {
            result.append(model->index(row, 0, parent).data().toString());
        }
        result.sort();
        return result;
    }

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
        QVERIFY(m_dir.isValid());
        m_root = QDir(m_dir.path()).canonicalPath();
        m_visible = m_root + QStringLiteral("/visible");
        m_flagged = m_root + QStringLiteral("/flagged");
        QVERIFY(QDir().mkpath(m_visible));
        QVERIFY(QDir().mkpath(m_flagged));
        QVERIFY(QDir().mkpath(m_root + QStringLiteral("/.dotted")));
        QFile file(m_root + QStringLiteral("/file.txt"));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("text\n");
    }

    void init()
    {
        QVERIFY(FolderMetadata::setHidden(m_flagged, true));
        QVERIFY(FolderMetadata::setHidden(m_visible, false));
        HiddenFolders::instance()->clear();
    }

    void folderViewHidesFlaggedFoldersLikeDotFiles()
    {
        FolderView view;
        view.setUrl(url());
        QTRY_COMPARE(view.itemCount(), 2);
        QCOMPARE(names(view.currentView()->model()), QStringList({QStringLiteral("file.txt"), QStringLiteral("visible")}));

        view.setShowHiddenFiles(true);
        QTRY_COMPARE(view.itemCount(), 4);
        view.setShowHiddenFiles(false);
        QTRY_COMPARE(view.itemCount(), 2);
    }

    void folderViewFollowsFlagChanges()
    {
        FolderView view;
        view.setUrl(url());
        QTRY_COMPARE(view.itemCount(), 2);

        QVERIFY(HiddenFolders::instance()->setHidden(m_visible, true));
        QTRY_COMPARE(view.itemCount(), 1);
        QVERIFY(HiddenFolders::instance()->setHidden(m_flagged, false));
        QTRY_COMPARE(view.itemCount(), 2);
        QVERIFY(HiddenFolders::instance()->setHidden(m_visible, false));
        QTRY_COMPARE(view.itemCount(), 3);
    }

    void folderViewIgnoresTheFlagOfTheListedFolder()
    {
        FolderView view;
        view.setUrl(url(QStringLiteral("flagged")));
        QTest::qWait(200);
        QVERIFY(view.url().matches(url(QStringLiteral("flagged")), QUrl::StripTrailingSlash));
        QCOMPARE(view.itemCount(), 0);
        QVERIFY(QDir().mkpath(m_flagged + QStringLiteral("/inner")));
        view.reload();
        QTRY_COMPARE(view.itemCount(), 1);
        QVERIFY(QDir(m_flagged + QStringLiteral("/inner")).removeRecursively());
    }

    void folderTreeHidesFlaggedFolders()
    {
        FolderTree tree({url()});
        tree.show();
        QVERIFY(QTest::qWaitForWindowExposed(&tree));
        auto *view = tree.findChild<QTreeView *>();
        QTRY_COMPARE(view->model()->rowCount(), 1);
        view->expand(view->model()->index(0, 0));
        QTRY_COMPARE(names(view->model(), view->model()->index(0, 0)), QStringList({QStringLiteral("visible")}));

        tree.setShowHiddenFolders(true);
        QTRY_COMPARE(names(view->model(), view->model()->index(0, 0)),
                     QStringList({QStringLiteral(".dotted"), QStringLiteral("flagged"), QStringLiteral("visible")}));
        tree.setShowHiddenFolders(false);
        QTRY_COMPARE(names(view->model(), view->model()->index(0, 0)), QStringList({QStringLiteral("visible")}));

        QVERIFY(HiddenFolders::instance()->setHidden(m_visible, true));
        QTRY_COMPARE(view->model()->rowCount(view->model()->index(0, 0)), 0);
    }

    void folderTreeKeepsAFlaggedRoot()
    {
        FolderTree tree({url(QStringLiteral("flagged"))});
        tree.show();
        QVERIFY(QTest::qWaitForWindowExposed(&tree));
        auto *view = tree.findChild<QTreeView *>();
        QTRY_COMPARE(view->model()->rowCount(), 1);
        QTest::qWait(200);
        QCOMPARE(view->model()->rowCount(), 1);
    }

    void yaziParentColumnHidesFlaggedFolders()
    {
        YaziColumns columns;
        auto *view = new FolderView;
        view->setUrl(url(QStringLiteral("visible")));
        columns.setCurrentView(view);
        columns.setUrl(url(QStringLiteral("visible")));
        columns.resize(800, 400);
        columns.show();
        QVERIFY(QTest::qWaitForWindowExposed(&columns));
        QListView *parent = columns.parentColumn();
        QTRY_COMPARE(names(parent->model()), QStringList({QStringLiteral("file.txt"), QStringLiteral("visible")}));

        columns.setShowHiddenFiles(true);
        QTRY_COMPARE(parent->model()->rowCount(), 4);
        columns.setShowHiddenFiles(false);
        QTRY_COMPARE(parent->model()->rowCount(), 2);
        QVERIFY(HiddenFolders::instance()->setHidden(m_flagged, false));
        QTRY_COMPARE(parent->model()->rowCount(), 3);
    }

    void folderPreviewHidesFlaggedFolders()
    {
        FolderPreview preview;
        preview.setFolder(KFileItem(url(), QStringLiteral("inode/directory"), S_IFDIR));
        QTRY_VERIFY(!preview.isListing());
        QTRY_COMPARE(preview.entryCount(), 2);

        preview.setShowHiddenFiles(true);
        QTRY_COMPARE(preview.entryCount(), 4);
        preview.setShowHiddenFiles(false);
        QTRY_COMPARE(preview.entryCount(), 2);
        QVERIFY(HiddenFolders::instance()->setHidden(m_flagged, false));
        QTRY_COMPARE(preview.entryCount(), 3);
    }
};

QTEST_MAIN(HiddenFoldersUiTest)

#include "hiddenfoldersuitest.moc"
