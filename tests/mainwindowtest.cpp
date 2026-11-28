// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "applicationpackagedialog.h"
#include "batchrenamedialog.h"
#include "checksumdialog.h"
#include "combinedpanel.h"
#include "commandpalettebar.h"
#include "contentchartdialog.h"
#include "customcommanddialog.h"
#include "customcommands.h"
#include "diskimagebackend.h"
#include "duplicatefinderdialog.h"
#include "editorpanel.h"
#include "effectiveappearance.h"
#include "favoritesmodel.h"
#include "fieldeditdialog.h"
#include "filediffdialog.h"
#include "filelistminimap.h"
#include "fileoperations.h"
#include "filesplitdialog.h"
#include "filetypeplaces.h"
#include "filterbar.h"
#include "foldercomparisondialog.h"
#include "foldersyncdialog.h"
#include "foldertree.h"
#include "folderview.h"
#include "gallerywindow.h"
#include "indexbackend.h"
#include "indexstatusdialog.h"
#include "informationpanel.h"
#include "interfacelanguage.h"
#include "keyboardhintoverlay.h"
#include "keyboardmapdialog.h"
#include "libraryregistry.h"
#include "localizeddates.h"
#include "metadataeditdialog.h"
#include "metadataimportdialog.h"
#include "navigationshortcuts.h"
#include "packagetools.h"
#include "pane.h"
#include "previewpanel.h"
#include "savedpathfilters.h"
#include "searchwidget.h"
#include "settings.h"
#include "settingsdialog.h"
#include "shortcutguard.h"
#include "shortcutstorage.h"
#include "symlinkdialog.h"
#include "tabmodel.h"
#include "tagfieldmanagerwidget.h"
#include "taggroupdialog.h"
#include "taggroupmanagerwidget.h"
#include "taglibrary.h"
#include "tagmanagerwidget.h"
#include "tagpanel.h"
#include "terminalpanel.h"
#include "transferqueue.h"
#include "transferqueuedialog.h"
#include "viewsettings.h"
#include "windowshortcuts.h"
#include "workspaceappearanceeditor.h"
#include "workspacedialog.h"
#include "yazimode.h"

#include <KActionCollection>
#include <KConfigGroup>
#include <KDirModel>
#include <KEditToolBar>
#include <KFileItemDelegate>
#include <KFilePlacesModel>
#include <KFilePlacesView>
#include <KLocalizedString>
#include <KPageWidgetItem>
#include <KProtocolInfo>
#include <KSelectAction>
#include <KSharedConfig>
#include <KTextEditor/Document>
#include <KToolBar>
#include <KXMLGUIFactory>

#include <QAbstractItemModel>
#include <QApplication>
#include <QBoxLayout>
#include <QCheckBox>
#include <QClipboard>
#include <QColor>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QCryptographicHash>
#include <QDialogButtonBox>
#include <QDir>
#include <QDockWidget>
#include <QDomDocument>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QGraphicsOpacityEffect>
#include <QGraphicsView>
#include <QHeaderView>
#include <QIcon>
#include <QImage>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QMediaPlayer>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QScopeGuard>
#include <QScrollBar>
#include <QShortcut>
#include <QSignalSpy>
#include <QSplitter>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStyle>
#include <QTabBar>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBlock>
#include <QTextBrowser>
#include <QTextLayout>
#include <QTimer>
#include <QToolButton>
#include <QTreeView>
#include <QTreeWidget>

#include <functional>

class WorkspaceTestJob : public KJob
{
public:
    void start() override
    {
    }

    void finish()
    {
        emitResult();
    }
};

// Answers the message box named objectName when it opens, polling briefly so
// earlier prompts can close first.
void answerMessageBox(const QString &objectName, const std::function<void(QMessageBox *)> &respond, int attempts = 200)
{
    QTimer::singleShot(10, [objectName, respond, attempts] {
        auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (box && box->isVisible() && box->objectName() == objectName) {
            respond(box);
        } else if (attempts > 0) {
            answerMessageBox(objectName, respond, attempts - 1);
        }
    });
}

// Accepts the folder dialog named objectName in folder when it opens.
void acceptFolderDialog(const QString &objectName, const QString &folder, int attempts = 200)
{
    QTimer::singleShot(10, [objectName, folder, attempts] {
        auto *dialog = qobject_cast<QFileDialog *>(QApplication::activeModalWidget());
        if (dialog && dialog->isVisible() && dialog->objectName() == objectName) {
            dialog->setDirectory(folder);
            QMetaObject::invokeMethod(dialog, "accept");
        } else if (attempts > 0) {
            acceptFolderDialog(objectName, folder, attempts - 1);
        }
    });
}

// Reports a fixed index configuration and indexer state.
class FakeIndexBackend : public IndexBackend
{
public:
    IndexStatus::Config fakeConfig;
    IndexStatus::Service fakeService;
    int settingsOpened = 0;

    IndexStatus::Config config() const override
    {
        return fakeConfig;
    }
    IndexStatus::Service service() const override
    {
        return fakeService;
    }
    bool isIndexed(const QString &) const override
    {
        return false;
    }
    bool hasExtractor(const QString &) const override
    {
        return false;
    }
    bool suspend(QString *) override
    {
        return false;
    }
    bool resume(QString *) override
    {
        return false;
    }
    bool checkUnindexedFiles(QString *) override
    {
        return false;
    }
    bool openSettings(QString *) override
    {
        ++settingsOpened;
        return true;
    }
};

// Emits a pane signal while MainWindow's children are being destroyed, after
// its members are gone, as KCoreDirListerCache can when the places model's
// lister stops a shared job.
class TeardownEmitter : public QObject
{
public:
    TeardownEmitter(FolderView *view, bool *emitted, QObject *parent)
        : QObject(parent)
        , m_view(view)
        , m_emitted(emitted)
    {
    }

    ~TeardownEmitter() override
    {
        if (m_view) {
            *m_emitted = true;
            Q_EMIT m_view->itemCountChanged();
            Q_EMIT m_view->selectionChanged();
        }
    }

private:
    QPointer<FolderView> m_view;
    bool *m_emitted;
};

// Restores the window's ui.rc file, or removes it when there was none, and the stored shortcuts once a test is done.
class ShortcutFilesGuard
{
public:
    explicit ShortcutFilesGuard(const QString &uiRcPath)
        : m_path(uiRcPath)
    {
        QFile file(m_path);
        m_existed = file.open(QIODevice::ReadOnly);
        if (m_existed) {
            m_contents = file.readAll();
        }
    }

    ~ShortcutFilesGuard()
    {
        if (m_existed) {
            QFile file(m_path);
            if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                file.write(m_contents);
            }
        } else {
            QFile::remove(m_path);
        }
        KSharedConfig::openConfig()->deleteGroup(QLatin1String(ShortcutStorage::Group));
        KSharedConfig::openConfig()->sync();
    }

    QString contents() const
    {
        QFile file(m_path);
        return file.open(QIODevice::ReadOnly) ? QString::fromUtf8(file.readAll()) : QString();
    }

private:
    QString m_path;
    QByteArray m_contents;
    bool m_existed = false;
};

class MainWindowTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
        QVERIFY(m_root.isValid());
        QVERIFY(QDir(m_root.path()).mkpath(QStringLiteral("a/sub")));
        QVERIFY(QDir(m_root.path()).mkpath(QStringLiteral("b")));
        QVERIFY(QDir(m_root.path()).mkpath(QStringLiteral("c")));
        QFile file(m_root.filePath(QStringLiteral("a/file.txt")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("hello");
        file.close();
        m_a = QUrl::fromLocalFile(m_root.filePath(QStringLiteral("a")));
        m_b = QUrl::fromLocalFile(m_root.filePath(QStringLiteral("b")));
        m_c = QUrl::fromLocalFile(m_root.filePath(QStringLiteral("c")));
    }

    void ignoresPaneSignalsDuringTeardown()
    {
        bool emitted = false;
        {
            auto *window = new MainWindow;
            window->openUrls({m_a, m_b});
            QObject *firstChild = window->children().constFirst();
            FolderView *view = window->activePane()->view();
            for (QObject *ancestor = view; ancestor; ancestor = ancestor->parent()) {
                QVERIFY(ancestor != firstChild);
            }
            new TeardownEmitter(view, &emitted, firstChild);
            delete window;
        }
        QVERIFY(emitted);
    }

    void tagPanelFollowsTheSelection()
    {
        QTemporaryDir folder;
        QVERIFY(folder.isValid());
        QFile file(folder.filePath(QStringLiteral("x.txt")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("x");
        file.close();
        QCOMPARE(LibraryRegistry::create(folder.path()), QString());
        const auto unregister = qScopeGuard([&folder] {
            LibraryRegistry::remove(folder.path());
        });

        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(folder.path())});
        window.show();
        auto *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 1);
        auto *dock = window.findChild<QDockWidget *>(QStringLiteral("tagDock"));
        auto *panel = window.findChild<TagPanel *>(QStringLiteral("tagPanel"));
        auto *assign = window.actionCollection()->action(QStringLiteral("assign_tags"));
        auto *toggle = window.actionCollection()->action(QStringLiteral("show_tag_panel"));
        QVERIFY(dock && panel && assign && toggle);
        QVERIFY(dock->isHidden());
        QVERIFY(assign->isEnabled());

        view->selectUrls({QUrl::fromLocalFile(folder.filePath(QStringLiteral("x.txt")))});
        assign->trigger();
        QVERIFY(dock->isVisible());
        QVERIFY(toggle->isChecked());
        QCOMPARE(panel->library(), folder.path());
        QVERIFY(panel->createAndAssign(QStringLiteral("Draft")));
        const TagLibrary::Library library = TagLibrary::Library::load(folder.path());
        QCOMPARE(library.entry(QStringLiteral("x.txt"))->tags, library.findTags(QStringLiteral("Draft")));

        // Manage Tags has a Groups tab, and its changes reach the tag panel.
        window.actionCollection()->action(QStringLiteral("manage_tags"))->trigger();
        auto *tagsDialog = window.findChild<QDialog *>(QStringLiteral("tagsDialog"));
        QVERIFY(tagsDialog && tagsDialog->findChild<TagManagerWidget *>());
        auto *groupManager = tagsDialog->findChild<TagGroupManagerWidget *>();
        QVERIFY(groupManager);
        TagGroupDialog *groupDialog = groupManager->createGroup();
        QVERIFY(groupDialog);
        groupDialog->findChild<QLineEdit *>(QStringLiteral("tagGroupName"))->setText(QStringLiteral("Status"));
        groupDialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
        auto *tagList = panel->findChild<QTreeWidget *>(QStringLiteral("tagPanelList"));
        QCOMPARE(tagList->topLevelItemCount(), 2);
        QCOMPARE(tagList->topLevelItem(0)->text(0), QStringLiteral("Status"));

        // Manage Tags has a Fields tab, and Edit Fields sets values on the selection.
        auto *fieldManager = tagsDialog->findChild<TagFieldManagerWidget *>();
        QVERIFY(fieldManager);
        TagFieldDialog *fieldDialog = fieldManager->createField();
        fieldDialog->findChild<QLineEdit *>(QStringLiteral("tagFieldName"))->setText(QStringLiteral("Owner"));
        const QString owner = fieldDialog->field().id;
        fieldDialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
        tagsDialog->close();
        auto *editFields = window.actionCollection()->action(QStringLiteral("edit_fields"));
        QVERIFY(editFields->isEnabled());
        editFields->trigger();
        auto *fieldEditor = window.findChild<FieldEditDialog *>();
        QVERIFY(fieldEditor && fieldEditor->editor(owner));
        QTest::keyClicks(fieldEditor->editor(owner), QStringLiteral("Ann"));
        QVERIFY(fieldEditor->apply());
        QCOMPARE(TagLibrary::Library::load(folder.path()).entry(QStringLiteral("x.txt"))->fields.value(owner), QStringLiteral("Ann"));

        // Import Embedded Metadata reads the selection in the background.
        auto *importMetadata = window.actionCollection()->action(QStringLiteral("import_metadata"));
        QVERIFY(importMetadata->isEnabled());
        importMetadata->trigger();
        auto *importDialog = window.findChild<MetadataImportDialog *>();
        QVERIFY(importDialog);
        QTRY_VERIFY(importDialog->isReady());
        QVERIFY(!importDialog->findChild<QPushButton *>(QStringLiteral("metadataImportPreview"))->isEnabled());
        importDialog->reject();

        panel->findChild<QAction *>(QStringLiteral("tagPanelPlaceBelow"))->trigger();
        QCOMPARE(window.dockWidgetArea(dock), Qt::BottomDockWidgetArea);
        QCOMPARE(Settings::tagPanelPlacement(), Settings::EnumTagPanelPlacement::Below);
        panel->findChild<QAction *>(QStringLiteral("tagPanelPlaceRight"))->trigger();
        QCOMPARE(window.dockWidgetArea(dock), Qt::RightDockWidgetArea);
        QCOMPARE(Settings::tagPanelPlacement(), Settings::EnumTagPanelPlacement::Right);
        toggle->trigger();
        QVERIFY(dock->isHidden());

        window.activePane()->setUrl(m_a);
        QTRY_VERIFY(!assign->isEnabled());
        QVERIFY(!editFields->isEnabled());
    }

    void opensMediaWithMpvWithoutShellInterpolation()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QFile executable(directory.filePath(QStringLiteral("mpv")));
        QVERIFY(executable.open(QIODevice::WriteOnly));
        QVERIFY(executable.write("#!/bin/sh\nprintf '%s\\n' \"$@\" > \"$MPV_TEST_OUTPUT\"\n") > 0);
        executable.close();
        QVERIFY(executable.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
        const QByteArray oldPath = qgetenv("PATH");
        const QByteArray oldOutput = qgetenv("MPV_TEST_OUTPUT");
        const auto restore = qScopeGuard([&] {
            qputenv("PATH", oldPath);
            qputenv("MPV_TEST_OUTPUT", oldOutput);
        });
        const QString outputPath = directory.filePath(QStringLiteral("arguments"));
        qputenv("PATH", directory.path().toLocal8Bit());
        qputenv("MPV_TEST_OUTPUT", outputPath.toLocal8Bit());
        const QString file = directory.filePath(QStringLiteral("video '$(touch unwanted)'.mp4"));
        QCOMPARE(FileOperations::openWithMpv({QUrl::fromLocalFile(file)}), QString());
        QTRY_VERIFY(QFile::exists(outputPath));
        QFile output(outputPath);
        QVERIFY(output.open(QIODevice::ReadOnly));
        QCOMPARE(output.readAll(), QByteArray("--\n") + QFile::encodeName(file) + '\n');
        QVERIFY(!QFile::exists(directory.filePath(QStringLiteral("unwanted"))));
        QVERIFY(!FileOperations::openWithMpv({QUrl(QStringLiteral("sftp://example.invalid/video.mp4"))}).isEmpty());
        qputenv("PATH", QByteArray());
        QVERIFY(!FileOperations::openWithMpv({QUrl::fromLocalFile(file)}).isEmpty());
    }

    void arkOpensAndExtractsArchivesKioCannotBrowse()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QDir root(directory.path());
        QVERIFY(root.mkpath(QStringLiteral("bin")) && root.mkpath(QStringLiteral("left")) && root.mkpath(QStringLiteral("right/music")));
        QFile executable(root.filePath(QStringLiteral("bin/ark")));
        QVERIFY(executable.open(QIODevice::WriteOnly));
        QVERIFY(executable.write("#!/bin/sh\nprintf '%s\\n' \"$@\" > \"$ARK_TEST_OUTPUT\"\n") > 0);
        executable.close();
        QVERIFY(executable.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
        const QString archive = root.filePath(QStringLiteral("left/music.rar"));
        QFile rar(archive);
        QVERIFY(rar.open(QIODevice::WriteOnly));
        QVERIFY(rar.write(QByteArray("Rar!\x1a\x07\x01\x00", 8) + QByteArray(64, '\0')) > 0);
        rar.close();

        const QByteArray oldPath = qgetenv("PATH");
        const QByteArray oldOutput = qgetenv("ARK_TEST_OUTPUT");
        const auto restore = qScopeGuard([&] {
            qputenv("PATH", oldPath);
            qputenv("ARK_TEST_OUTPUT", oldOutput);
        });
        const QString outputPath = root.filePath(QStringLiteral("arguments"));
        qputenv("PATH", root.filePath(QStringLiteral("bin")).toLocal8Bit());
        qputenv("ARK_TEST_OUTPUT", outputPath.toLocal8Bit());
        const auto readArguments = [&outputPath] {
            QFile output(outputPath);
            return output.open(QIODevice::ReadOnly) ? QString::fromLocal8Bit(output.readAll()).split(u'\n', Qt::SkipEmptyParts) : QStringList();
        };

        MainWindow window;
        const QJsonObject rarPlugin = QJsonDocument::fromJson(R"({"KPlugin": {"Id": "kerfuffle_clirar", "MimeTypes": ["application/vnd.rar"]},
            "X-KDE-Kerfuffle-ReadOnlyExecutables": ["unrar"], "X-KDE-Kerfuffle-ReadWrite": true, "X-KDE-Kerfuffle-ReadWriteExecutables": ["rar"]})")
                                          .object();
        window.setArkBackends(Ark::Backends::fromPlugins({rarPlugin}, [](const QString &program) {
            return program == QLatin1String("unrar");
        }));
        window.openUrls({QUrl::fromLocalFile(root.filePath(QStringLiteral("left"))), QUrl::fromLocalFile(root.filePath(QStringLiteral("right")))});
        window.show();
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(true);
        FolderView *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 1);
        QAction *open = window.actionCollection()->action(QStringLiteral("open_in_ark"));
        QVERIFY(open && !open->isEnabled());
        view->selectUrls({QUrl::fromLocalFile(archive)});
        QTRY_VERIFY(open->isEnabled());

        open->trigger();
        QTRY_COMPARE(readArguments(), (QStringList{QStringLiteral("--"), archive}));

        QFile::remove(outputPath);
        window.actionCollection()->action(QStringLiteral("extract_to_other_pane"))->trigger();
        QTRY_VERIFY(!readArguments().isEmpty());
        const QStringList arguments = readArguments();
        QCOMPARE(arguments.size(), 5);
        QCOMPARE(arguments.mid(0, 2), (QStringList{QStringLiteral("--batch"), QStringLiteral("--destination")}));
        QCOMPARE(arguments.mid(3), (QStringList{QStringLiteral("--"), archive}));
        const QString destination = arguments.at(2);
        QVERIFY2(destination != root.filePath(QStringLiteral("right/music")), qPrintable(destination));
        QVERIFY(destination.startsWith(root.filePath(QStringLiteral("right/music"))));
        QVERIFY(QFileInfo(destination).isDir());

        qputenv("PATH", QByteArray());
        QFile::remove(outputPath);
        bool reported = false;
        std::function<void(int)> closeReport = [&reported, &closeReport](int attempts) {
            QTimer::singleShot(10, [&reported, &closeReport, attempts] {
                auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                if (dialog && dialog->isVisible()) {
                    reported = dialog->windowTitle().contains(QStringLiteral("Ark Not Found"));
                    dialog->reject();
                } else if (attempts > 0) {
                    closeReport(attempts - 1);
                }
            });
        };
        closeReport(200);
        open->trigger();
        QVERIFY(reported);
        QVERIFY(!QFile::exists(outputPath));
    }

    void arkAddsSelectionOnlyToWritableArchives()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QDir root(directory.path());
        QVERIFY(root.mkpath(QStringLiteral("bin")) && root.mkpath(QStringLiteral("left/folder")));
        QFile executable(root.filePath(QStringLiteral("bin/ark")));
        QVERIFY(executable.open(QIODevice::WriteOnly));
        QVERIFY(executable.write("#!/bin/sh\nprintf '%s\\n' \"$@\" > \"$ARK_TEST_OUTPUT\"\n") > 0);
        executable.close();
        QVERIFY(executable.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
        const auto writeFile = [](const QString &path, const QByteArray &data) {
            QFile file(path);
            return file.open(QIODevice::WriteOnly) && file.write(data) == data.size();
        };
        const QString note = root.filePath(QStringLiteral("left/note.txt"));
        const QString zip = root.filePath(QStringLiteral("left/photos.zip"));
        const QString innerZip = root.filePath(QStringLiteral("left/folder/inner.zip"));
        const QString rar = root.filePath(QStringLiteral("left/music.rar"));
        const QByteArray emptyZip = QByteArray("PK\x05\x06", 4) + QByteArray(18, '\0');
        QVERIFY(writeFile(note, "hello"));
        QVERIFY(writeFile(zip, emptyZip));
        QVERIFY(writeFile(innerZip, emptyZip));
        QVERIFY(writeFile(rar, QByteArray("Rar!\x1a\x07\x01\x00", 8) + QByteArray(64, '\0')));

        const QByteArray oldPath = qgetenv("PATH");
        const QByteArray oldOutput = qgetenv("ARK_TEST_OUTPUT");
        const auto restore = qScopeGuard([&] {
            qputenv("PATH", oldPath);
            qputenv("ARK_TEST_OUTPUT", oldOutput);
        });
        const QString outputPath = root.filePath(QStringLiteral("arguments"));
        qputenv("PATH", root.filePath(QStringLiteral("bin")).toLocal8Bit());
        qputenv("ARK_TEST_OUTPUT", outputPath.toLocal8Bit());
        const auto readArguments = [&outputPath] {
            QFile output(outputPath);
            return output.open(QIODevice::ReadOnly) ? QString::fromLocal8Bit(output.readAll()).split(u'\n', Qt::SkipEmptyParts) : QStringList();
        };
        // Answers the confirmation once it is shown, by adding or cancelling.
        bool confirmed = false;
        std::function<void(bool, int)> answer = [&confirmed, &answer](bool add, int attempts) {
            QTimer::singleShot(10, [&confirmed, &answer, add, attempts] {
                auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                if (!dialog || !dialog->isVisible()) {
                    if (attempts > 0) {
                        answer(add, attempts - 1);
                    }
                    return;
                }
                confirmed = dialog->windowTitle().contains(QStringLiteral("Add to Archive"));
                const QList<QPushButton *> buttons = dialog->findChildren<QPushButton *>();
                for (QPushButton *button : buttons) {
                    if (add && button->text().remove(u'&') == QStringLiteral("Add")) {
                        button->click();
                        return;
                    }
                }
                dialog->reject();
            });
        };

        MainWindow window;
        const QJsonObject rarPlugin = QJsonDocument::fromJson(R"({"KPlugin": {"Id": "kerfuffle_clirar", "MimeTypes": ["application/vnd.rar"]},
            "X-KDE-Kerfuffle-ReadOnlyExecutables": ["unrar"], "X-KDE-Kerfuffle-ReadWrite": true, "X-KDE-Kerfuffle-ReadWriteExecutables": ["rar"]})")
                                          .object();
        const QJsonObject zipPlugin =
            QJsonDocument::fromJson(R"({"KPlugin": {"Id": "kerfuffle_libzip", "MimeTypes": ["application/zip"]}, "X-KDE-Kerfuffle-ReadWrite": true})").object();
        window.setArkBackends(Ark::Backends::fromPlugins({rarPlugin, zipPlugin}, [](const QString &program) {
            return program == QLatin1String("unrar");
        }));
        window.openUrls({QUrl::fromLocalFile(root.filePath(QStringLiteral("left")))});
        FolderView *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 4);
        QVERIFY(window.actionCollection()->action(QStringLiteral("add_to_archive")));
        view->selectUrls({QUrl::fromLocalFile(note)});
        QTRY_COMPARE(view->selectedItems().size(), 1);

        QVERIFY(!window.addSelectionToArchive(rar));
        QVERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("only extract")));
        QVERIFY(!window.addSelectionToArchive(root.filePath(QStringLiteral("left/missing.zip"))));

        answer(false, 200);
        QVERIFY(!window.addSelectionToArchive(zip));
        QVERIFY(confirmed);
        QVERIFY(!QFile::exists(outputPath));

        confirmed = false;
        answer(true, 200);
        QVERIFY(window.addSelectionToArchive(zip));
        QVERIFY(confirmed);
        QTRY_COMPARE(readArguments(), (QStringList{QStringLiteral("--changetofirstpath"), QStringLiteral("--add-to"), zip, QStringLiteral("--"), note}));

        QFile::remove(outputPath);
        view->selectUrls({QUrl::fromLocalFile(zip)});
        QTRY_COMPARE(view->selectedItems().size(), 1);
        QVERIFY(!window.addSelectionToArchive(zip));
        QVERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("itself")));
        view->selectUrls({QUrl::fromLocalFile(root.filePath(QStringLiteral("left/folder")))});
        QTRY_COMPARE(view->selectedItems().size(), 1);
        QVERIFY(!window.addSelectionToArchive(innerZip));
        QVERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("itself")));
        QVERIFY(!QFile::exists(outputPath));
    }

    void mountsDiskImagesAndReportsFailures()
    {
        struct FakeDiskImages : DiskImageBackend {
            bool installed = true;
            bool offersAction = true;
            // The states each plugin action moves through, in order.
            QList<Status> results;
            Status current;
            int runs = 0;
            bool pluginInstalled() const override
            {
                return installed;
            }
            bool runPluginAction(const QString &, QWidget *) override
            {
                if (!offersAction) {
                    return false;
                }
                ++runs;
                if (!results.isEmpty()) {
                    current = results.takeFirst();
                }
                return true;
            }
            Status status(const QString &) const override
            {
                return current;
            }
        };
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString image = directory.filePath(QStringLiteral("install.iso"));
        const QString note = directory.filePath(QStringLiteral("note.txt"));
        for (const QString &path : {image, note}) {
            QFile file(path);
            QVERIFY(file.open(QIODevice::WriteOnly) && file.write("x") == 1);
        }
        auto owned = std::make_unique<FakeDiskImages>();
        FakeDiskImages *fake = owned.get();
        MainWindow window;
        window.setDiskImageBackend(std::move(owned));
        window.openUrls({QUrl::fromLocalFile(directory.path())});
        FolderView *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 2);
        QAction *mount = window.actionCollection()->action(QStringLiteral("mount_disk_image"));
        QAction *unmount = window.actionCollection()->action(QStringLiteral("unmount_disk_image"));
        QVERIFY(mount && unmount);
        view->selectUrls({QUrl::fromLocalFile(note)});
        QTRY_COMPARE(view->selectedItems().size(), 1);
        QVERIFY(!mount->isEnabled());
        view->selectUrls({QUrl::fromLocalFile(image)});
        QTRY_VERIFY(mount->isEnabled() && unmount->isEnabled());
        const auto message = [&window] {
            return window.statusBar()->currentMessage();
        };

        fake->results = {{DiskImageBackend::State::Mounted, QStringLiteral("/run/media/user/INSTALL")}};
        mount->trigger();
        QCOMPARE(fake->runs, 1);
        QVERIFY2(message().contains(QStringLiteral("/run/media/user/INSTALL")), qPrintable(message()));
        mount->trigger();
        QCOMPARE(fake->runs, 1);
        QVERIFY(message().contains(QStringLiteral("already mounted")));

        fake->results = {{DiskImageBackend::State::Mounted, QStringLiteral("/run/media/user/INSTALL")}};
        unmount->trigger();
        QVERIFY(message().contains(QStringLiteral("still mounted")));
        fake->results = {{}};
        unmount->trigger();
        QVERIFY(message().startsWith(QStringLiteral("Unmounted")));
        unmount->trigger();
        QVERIFY(message().contains(QStringLiteral("not mounted")));

        fake->runs = 0;
        fake->results = {{}};
        mount->trigger();
        QVERIFY(message().contains(QStringLiteral("Authorization may have been denied")));
        fake->results = {{DiskImageBackend::State::Attached, {}}};
        mount->trigger();
        QVERIFY(message().contains(QStringLiteral("attached but not mounted")));
        // Mounting an attached image detaches it first, then mounts it again.
        fake->runs = 0;
        fake->results = {{}, {DiskImageBackend::State::Mounted, QStringLiteral("/mnt/image")}};
        mount->trigger();
        QCOMPARE(fake->runs, 2);
        QVERIFY(message().contains(QStringLiteral("/mnt/image")));

        fake->current = {};
        fake->offersAction = false;
        mount->trigger();
        QVERIFY(message().contains(QStringLiteral("offered no action")));

        fake->installed = false;
        bool reported = false;
        std::function<void(int)> closeReport = [&reported, &closeReport](int attempts) {
            QTimer::singleShot(10, [&reported, &closeReport, attempts] {
                auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                if (dialog && dialog->isVisible()) {
                    reported = dialog->windowTitle().contains(QStringLiteral("Mount ISO Plugin Not Found"));
                    dialog->reject();
                } else if (attempts > 0) {
                    closeReport(attempts - 1);
                }
            });
        };
        closeReport(200);
        mount->trigger();
        QVERIFY(reported);
    }

    void opensApplicationPackages()
    {
        struct NoTools : PackageTools {
            bool hasProgram(const QString &) const override
            {
                return false;
            }
            void run(const AppPackages::Command &, const QString &, QObject *, std::function<void(const Result &)>) override
            {
            }
            void launch(const AppPackages::Command &, QObject *, std::function<void(const QString &)>) override
            {
            }
        };
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString ref = directory.filePath(QStringLiteral("kate.flatpakref"));
        const QString note = directory.filePath(QStringLiteral("note.txt"));
        for (const auto &[path, contents] : {std::pair{ref, QByteArray("[Flatpak Ref]\nName=org.kde.kate\nTitle=Kate\n")}, std::pair{note, QByteArray("x")}}) {
            QFile file(path);
            QVERIFY(file.open(QIODevice::WriteOnly) && file.write(contents) == contents.size());
        }
        NoTools tools;
        MainWindow window;
        window.setPackageTools(&tools);
        window.openUrls({QUrl::fromLocalFile(directory.path())});
        FolderView *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 2);
        QAction *action = window.actionCollection()->action(QStringLiteral("application_package"));
        QVERIFY(action);
        view->selectUrls({QUrl::fromLocalFile(note)});
        QTRY_COMPARE(view->selectedItems().size(), 1);
        QVERIFY(!action->isEnabled());
        view->selectUrls({QUrl::fromLocalFile(ref)});
        QTRY_VERIFY(action->isEnabled());
        action->trigger();
        auto *dialog = window.findChild<ApplicationPackageDialog *>();
        QVERIFY(dialog);
        QCOMPARE(dialog->package().kind, AppPackages::Kind::FlatpakRef);
        QCOMPARE(dialog->package().name, QStringLiteral("Kate"));
        auto *install = dialog->findChild<QPushButton *>(QStringLiteral("packageInstall"));
        QVERIFY(install && !install->isEnabled());
        dialog->close();
    }

    void opensIndexStatus()
    {
        struct Unavailable : IndexBackend {
            IndexStatus::Config config() const override
            {
                return {};
            }
            IndexStatus::Service service() const override
            {
                return {};
            }
            bool isIndexed(const QString &) const override
            {
                return false;
            }
            bool hasExtractor(const QString &) const override
            {
                return false;
            }
            bool suspend(QString *) override
            {
                return false;
            }
            bool resume(QString *) override
            {
                return false;
            }
            bool checkUnindexedFiles(QString *) override
            {
                return false;
            }
            bool openSettings(QString *) override
            {
                return false;
            }
        };
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        Unavailable backend;
        MainWindow window;
        window.setIndexBackend(&backend);
        window.openUrls({QUrl::fromLocalFile(directory.path())});
        QAction *action = window.actionCollection()->action(QStringLiteral("index_status"));
        QVERIFY(action && action->isEnabled());
        action->trigger();
        auto *dialog = window.findChild<IndexStatusDialog *>(QStringLiteral("indexStatusDialog"));
        QVERIFY(dialog);
        QVERIFY(dialog->findChild<QLabel *>(QStringLiteral("indexWarnings"))->text().contains(QStringLiteral("without Baloo")));
        dialog->close();
    }

    void addsFileTypePlacesOnce()
    {
        // Every visible file-type place, so that a duplicate entry is counted.
        const auto placeRows = [](KFilePlacesModel *model) {
            QList<QPair<QUrl, QModelIndex>> rows;
            for (int row = 0; row < model->rowCount(); ++row) {
                const QModelIndex index = model->index(row, 0);
                if (FileTypePlaces::isPlaceUrl(model->url(index))) {
                    rows.append({model->url(index), index});
                }
            }
            return rows;
        };
        const auto rowFor = [](const QList<QPair<QUrl, QModelIndex>> &rows, FileTypePlaces::Category category) {
            for (const auto &[url, index] : rows) {
                if (url == FileTypePlaces::placeUrl(category)) {
                    return index;
                }
            }
            return QModelIndex();
        };
        KConfigGroup state = KSharedConfig::openStateConfig()->group(QStringLiteral("FileTypePlaces"));
        state.writeEntry("Added", false);
        Settings::setFileTypeCategoriesHidden({});
        {
            MainWindow window;
            auto *model = qobject_cast<KFilePlacesModel *>(window.findChild<KFilePlacesView *>()->model());
            QVERIFY(model);
            const auto rows = placeRows(model);
            QCOMPARE(rows.size(), FileTypePlaces::categories().size());
            for (const FileTypePlaces::Category category : FileTypePlaces::categories()) {
                const QModelIndex index = rowFor(rows, category);
                QVERIFY(index.isValid());
                QCOMPARE(model->text(index), FileTypePlaces::title(category));
                QCOMPARE(model->groupType(index), KFilePlacesModel::SearchForType);
            }
            model->removePlace(rowFor(rows, FileTypePlaces::Category::Ebooks));
            // Removing the entry hides its category in Preferences too.
            QTRY_COMPARE(Settings::fileTypeCategoriesHidden(), QStringList{QStringLiteral("ebooks")});
        }
        QVERIFY(state.readEntry("Added", false));
        {
            // A removed entry stays removed.
            MainWindow window;
            auto *model = qobject_cast<KFilePlacesModel *>(window.findChild<KFilePlacesView *>()->model());
            const auto rows = placeRows(model);
            QCOMPARE(rows.size(), FileTypePlaces::categories().size() - 1);
            QVERIFY(!rowFor(rows, FileTypePlaces::Category::Ebooks).isValid());
            QCOMPARE(Settings::fileTypeCategoriesHidden(), QStringList{QStringLiteral("ebooks")});

            // Showing the category again in Preferences restores only the missing entry.
            Settings::setFileTypeCategoriesHidden({});
            window.applySettings();
            QTRY_COMPARE(placeRows(model).size(), FileTypePlaces::categories().size());
        }
        state.writeEntry("Added", false);
        Settings::setFileTypeCategoriesHidden({QStringLiteral("large-files")});
        {
            // Adding again adds only the shown categories, and removes the hidden ones.
            MainWindow window;
            auto *model = qobject_cast<KFilePlacesModel *>(window.findChild<KFilePlacesView *>()->model());
            const auto rows = placeRows(model);
            QCOMPARE(rows.size(), FileTypePlaces::categories().size() - 1);
            QVERIFY(!rowFor(rows, FileTypePlaces::Category::LargeFiles).isValid());
            Settings::setFileTypeCategoriesHidden({});
            window.applySettings();
            QTRY_COMPARE(placeRows(model).size(), FileTypePlaces::categories().size());
        }
        Settings::self()->save();
    }

    void opensFileTypePlaceAsLiveSearchWhenIndexingIsOff()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QVERIFY(QDir(root.path()).mkdir(QStringLiteral("books")));
        for (const QString &name : {QStringLiteral("books/novel.epub"), QStringLiteral("books/report.pdf")}) {
            QFile file(root.filePath(name));
            QVERIFY(file.open(QIODevice::WriteOnly));
        }
        FakeIndexBackend backend;
        backend.fakeConfig.available = true;
        MainWindow window;
        window.setIndexBackend(&backend);
        const QUrl rootUrl = QUrl::fromLocalFile(root.path());
        window.openUrls({rootUrl});
        Pane *pane = window.activePane();

        bool offeredSettings = false;
        answerMessageBox(QStringLiteral("fileTypePlaceBox"), [&](QMessageBox *box) {
            offeredSettings = box->findChild<QPushButton *>(QStringLiteral("fileTypeSettings"));
            QVERIFY(!box->findChild<QPushButton *>(QStringLiteral("fileTypeIndexed")));
            box->findChild<QPushButton *>(QStringLiteral("fileTypeLive"))->click();
        });
        acceptFolderDialog(QStringLiteral("fileTypeFolderDialog"), root.filePath(QStringLiteral("books")));
        pane->setUrl(FileTypePlaces::placeUrl(FileTypePlaces::Category::Ebooks));
        QVERIFY(offeredSettings);
        QCOMPARE(pane->url(), rootUrl);

        auto *search = window.findChild<SearchWidget *>();
        QVERIFY(search);
        QCOMPARE(search->folder(), QUrl::fromLocalFile(root.filePath(QStringLiteral("books"))));
        QTRY_VERIFY(!search->isRunning());
        auto *results = search->findChild<QTreeWidget *>(QStringLiteral("searchResults"));
        QTRY_COMPARE(results->findItems(QStringLiteral("novel.epub"), Qt::MatchContains | Qt::MatchRecursive).size(), 1);
        QVERIFY(results->findItems(QStringLiteral("report.pdf"), Qt::MatchContains | Qt::MatchRecursive).isEmpty());

        const int tabCount = pane->tabs()->count();
        answerMessageBox(QStringLiteral("fileTypePlaceBox"), [](QMessageBox *box) {
            box->findChild<QPushButton *>(QStringLiteral("fileTypeSettings"))->click();
        });
        pane->openInNewTab(FileTypePlaces::placeUrl(FileTypePlaces::Category::Images));
        QCOMPARE(backend.settingsOpened, 1);
        QCOMPARE(pane->tabs()->count(), tabCount);
    }

    void opensFileTypePlaceFromIndex()
    {
        if (!KProtocolInfo::isKnownProtocol(QStringLiteral("baloosearch")) || FileTypePlaces::balooUrl(FileTypePlaces::Category::Images).isEmpty()) {
            QSKIP("Needs Baloo and its baloosearch KIO worker");
        }
        QTemporaryDir root;
        QVERIFY(root.isValid());
        FakeIndexBackend backend;
        backend.fakeConfig.available = true;
        backend.fakeConfig.enabled = true;
        backend.fakeService.running = true;
        backend.fakeService.state = static_cast<int>(IndexStatus::State::Idle);
        MainWindow window;
        window.setIndexBackend(&backend);
        window.openUrls({QUrl::fromLocalFile(root.path())});
        Pane *pane = window.activePane();
        const QUrl images = FileTypePlaces::balooUrl(FileTypePlaces::Category::Images);
        pane->setUrl(FileTypePlaces::placeUrl(FileTypePlaces::Category::Images));
        QCOMPARE(pane->url(), images);

        // A stopped indexer asks first, then shows the indexed results in a new tab.
        backend.fakeService.running = false;
        const int tabCount = pane->tabs()->count();
        bool warned = false;
        answerMessageBox(QStringLiteral("fileTypePlaceBox"), [&](QMessageBox *box) {
            warned = box->informativeText().contains(QStringLiteral("not running"));
            box->findChild<QPushButton *>(QStringLiteral("fileTypeIndexed"))->click();
        });
        pane->openInNewTab(FileTypePlaces::placeUrl(FileTypePlaces::Category::Audio));
        QVERIFY(warned);
        QCOMPARE(pane->tabs()->count(), tabCount + 1);
        QCOMPARE(pane->url(), FileTypePlaces::balooUrl(FileTypePlaces::Category::Audio));
    }

    void opensFilteredFolderComparison()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QVERIFY(QDir(root.path()).mkdir(QStringLiteral("left")));
        QVERIFY(QDir(root.path()).mkdir(QStringLiteral("right")));
        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(root.filePath(QStringLiteral("left"))), QUrl::fromLocalFile(root.filePath(QStringLiteral("right")))});
        auto *action = window.actionCollection()->action(QStringLiteral("compare_folders_filtered"));
        QVERIFY(action && action->isEnabled());
        action->trigger();
        auto *dialog = window.findChild<FolderSyncDialog *>(QStringLiteral("folderSyncDialog"));
        QVERIFY(dialog);
        dialog->close();
    }

    void opensTransferQueue()
    {
        MainWindow window;
        auto *action = window.actionCollection()->action(QStringLiteral("transfer_queue"));
        QVERIFY(action && action->isEnabled());
        action->trigger();
        auto *dialog = window.findChild<TransferQueueDialog *>(QStringLiteral("transferQueueDialog"));
        QVERIFY(dialog);
        dialog->close();
    }

    void opensWorkspaceSelector()
    {
        MainWindow window;
        auto *action = window.actionCollection()->action(QStringLiteral("manage_workspaces"));
        QVERIFY(action && action->isEnabled());
        action->trigger();
        auto *dialog = window.findChild<QDialog *>(QStringLiteral("workspaceDialog"));
        QVERIFY(dialog);
        dialog->close();
    }

    void showsWorkspaceChangesAndConfirmsRestore()
    {
        const auto config = KSharedConfig::openConfig();
        KConfigGroup group = config->group(QStringLiteral("Workspaces"));
        const bool hadCatalog = group.hasKey("Catalog");
        const QByteArray previousCatalog = group.readEntry("Catalog", QByteArray());
        group.deleteEntry("Catalog");
        QVERIFY(group.sync());
        const auto restore = qScopeGuard([config, hadCatalog, previousCatalog] {
            config->reparseConfiguration();
            KConfigGroup saved = config->group(QStringLiteral("Workspaces"));
            if (hadCatalog) {
                saved.writeEntry("Catalog", previousCatalog);
            } else {
                saved.deleteEntry("Catalog");
            }
            saved.sync();
        });

        MainWindow window;
        window.openUrls({m_a, m_b});
        window.show();
        QTRY_VERIFY(window.isVisible());
        const PaneSession savedTabs = window.activePane()->saveState();
        window.actionCollection()->action(QStringLiteral("manage_workspaces"))->trigger();
        auto *dialog = window.findChild<WorkspaceDialog *>(QStringLiteral("workspaceDialog"));
        QVERIFY(dialog);
        dialog->findChild<QLineEdit *>(QStringLiteral("workspaceName"))->setText(QStringLiteral("Work"));
        dialog->findChild<QLineEdit *>(QStringLiteral("workspaceShortLabel"))->setText(QStringLiteral("W"));
        dialog->findChild<QPushButton *>(QStringLiteral("workspaceCreate"))->click();
        QTimer::singleShot(0, [] {
            qobject_cast<QMessageBox *>(QApplication::activeModalWidget())->button(QMessageBox::Yes)->click();
        });
        dialog->findChild<QPushButton *>(QStringLiteral("workspaceActivate"))->click();
        dialog->close();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

        QAction *selector = window.actionCollection()->action(QStringLiteral("workspace_selector"));
        QAction *restoreLayout = window.actionCollection()->action(QStringLiteral("restore_workspace_layout"));
        QVERIFY(selector && restoreLayout);
        QTRY_VERIFY(restoreLayout->isEnabled());
        QTRY_VERIFY(selector->text().contains(QStringLiteral("Work")));
        window.activePane()->openInNewTab(m_c, true);
        QTRY_VERIFY(selector->text().endsWith(QStringLiteral(" *")));
        QTimer::singleShot(0, [] {
            qobject_cast<QMessageBox *>(QApplication::activeModalWidget())->button(QMessageBox::No)->click();
        });
        restoreLayout->trigger();
        QCOMPARE(window.activePane()->url(), m_c);
        QTRY_VERIFY(selector->text().endsWith(QStringLiteral(" *")));
        QTimer::singleShot(0, [] {
            qobject_cast<QMessageBox *>(QApplication::activeModalWidget())->button(QMessageBox::Yes)->click();
        });
        restoreLayout->trigger();
        QTRY_COMPARE(window.activePane()->saveState(), savedTabs);
        QTRY_VERIFY(!selector->text().endsWith(QStringLiteral(" *")));

        const auto resetIndicators = qScopeGuard([&window] {
            Settings::setWorkspaceSelectorPlacement(Settings::EnumWorkspaceSelectorPlacement::ViewMenu);
            Settings::setShowWorkspaceUnsavedIndicator(true);
            Settings::setConfirmWorkspaceLayoutRestore(true);
            window.applySettings();
        });
        auto *statusButton = window.findChild<QToolButton *>(QStringLiteral("workspaceStatusButton"));
        QVERIFY(statusButton);
        QVERIFY(selector->isVisible());
        QVERIFY(!statusButton->isVisible());
        Settings::setWorkspaceSelectorPlacement(Settings::EnumWorkspaceSelectorPlacement::StatusBar);
        Settings::setShowWorkspaceUnsavedIndicator(false);
        Settings::setConfirmWorkspaceLayoutRestore(false);
        window.applySettings();
        QVERIFY(!selector->isVisible());
        QVERIFY(statusButton->isVisible());
        QVERIFY(statusButton->menu());
        QVERIFY(window.actionCollection()->action(QStringLiteral("manage_workspaces"))->isVisible());
        window.activePane()->openInNewTab(m_c, true);
        QTRY_VERIFY(statusButton->text().contains(QStringLiteral("Work")));
        QTest::qWait(400);
        QVERIFY(!selector->text().endsWith(QStringLiteral(" *")));
        QVERIFY(!statusButton->text().endsWith(QStringLiteral(" *")));
        // Returning to the saved layout no longer asks.
        restoreLayout->trigger();
        QTRY_COMPARE(window.activePane()->saveState(), savedTabs);
        Settings::setWorkspaceSelectorPlacement(Settings::EnumWorkspaceSelectorPlacement::ViewMenuAndStatusBar);
        window.applySettings();
        QVERIFY(selector->isVisible() && statusButton->isVisible());
    }

    void opensWorkspaceById()
    {
        const auto config = KSharedConfig::openConfig();
        KConfigGroup group = config->group(QStringLiteral("Workspaces"));
        const bool hadCatalog = group.hasKey("Catalog");
        const QByteArray previousCatalog = group.readEntry("Catalog", QByteArray());
        group.deleteEntry("Catalog");
        QVERIFY(group.sync());
        const auto restore = qScopeGuard([config, hadCatalog, previousCatalog] {
            config->reparseConfiguration();
            KConfigGroup saved = config->group(QStringLiteral("Workspaces"));
            if (hadCatalog) {
                saved.writeEntry("Catalog", previousCatalog);
            } else {
                saved.deleteEntry("Catalog");
            }
            saved.sync();
        });

        MainWindow window;
        window.openUrls({m_a, m_b});
        window.show();
        QTRY_VERIFY(window.isVisible());
        window.actionCollection()->action(QStringLiteral("manage_workspaces"))->trigger();
        auto *dialog = window.findChild<WorkspaceDialog *>(QStringLiteral("workspaceDialog"));
        QVERIFY(dialog);
        dialog->findChild<QLineEdit *>(QStringLiteral("workspaceName"))->setText(QStringLiteral("Runner"));
        dialog->findChild<QLineEdit *>(QStringLiteral("workspaceShortLabel"))->setText(QStringLiteral("R"));
        dialog->findChild<QPushButton *>(QStringLiteral("workspaceCreate"))->click();
        dialog->close();
        delete dialog;
        config->reparseConfiguration();
        WorkspaceCatalog::Catalog catalog;
        QString error;
        QVERIFY2(WorkspaceCatalog::load(config->group(QStringLiteral("Workspaces")), catalog, error), qPrintable(error));
        QCOMPARE(catalog.workspaces.size(), 1);

        window.openWorkspace(QStringLiteral("missing"));
        QCOMPARE(window.statusBar()->currentMessage(), i18nc("@info:status", "The requested workspace no longer exists."));

        bool asked = false;
        QTimer::singleShot(0, &window, [&asked] {
            if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
                asked = true;
                box->button(QMessageBox::Yes)->click();
            }
        });
        window.openWorkspace(catalog.workspaces.first().id);
        QVERIFY(asked);
        if (auto *opened = window.findChild<WorkspaceDialog *>(QStringLiteral("workspaceDialog"))) {
            opened->close();
            delete opened;
        }
        QAction *selector = window.actionCollection()->action(QStringLiteral("workspace_selector"));
        QVERIFY(selector);
        QTRY_VERIFY(selector->text().contains(QStringLiteral("Runner")));
    }

    void appliesTheActiveWorkspaceVaultToEditors()
    {
        QTemporaryDir directory;
        const auto config = KSharedConfig::openConfig();
        const auto backup = KSharedConfig::openConfig(directory.filePath(QStringLiteral("backup")));
        KConfigGroup favorites = config->group(QStringLiteral("Favorites"));
        KConfigGroup workspaces = config->group(QStringLiteral("Workspaces"));
        KConfigGroup savedFavorites = backup->group(QStringLiteral("Favorites"));
        KConfigGroup savedWorkspaces = backup->group(QStringLiteral("Workspaces"));
        favorites.copyTo(&savedFavorites);
        workspaces.copyTo(&savedWorkspaces);
        favorites.deleteGroup();
        workspaces.deleteGroup();
        WorkspaceCatalog::Catalog catalog;
        QString error;
        QVERIFY(WorkspaceCatalog::create(catalog, QStringLiteral("Notes"), QStringLiteral("N"), error));
        const QString id = catalog.workspaces.first().id;
        QVERIFY(WorkspaceCatalog::assignLocation(catalog, WorkspaceCatalog::locationId(m_a), id, error));
        QVERIFY(WorkspaceCatalog::setVault(catalog, id, WorkspaceCatalog::locationId(m_a), error));
        QVERIFY(WorkspaceCatalog::activate(catalog, id, error));
        QVERIFY2(WorkspaceCatalog::save(workspaces, catalog, error), qPrintable(error));
        QVERIFY(config->sync());
        const auto restore = qScopeGuard([config, savedFavorites, savedWorkspaces]() mutable {
            config->reparseConfiguration();
            auto favorites = config->group(QStringLiteral("Favorites"));
            auto workspaces = config->group(QStringLiteral("Workspaces"));
            favorites.deleteGroup();
            workspaces.deleteGroup();
            savedFavorites.copyTo(&favorites);
            savedWorkspaces.copyTo(&workspaces);
            config->sync();
        });
        {
            MainWindow window;
            QVERIFY(window.favorites()->add(m_a));
            QTRY_COMPARE(EditorPanel::workspaceVault(), QDir::cleanPath(m_a.toLocalFile()));
            QVERIFY(window.favorites()->remove(m_a));
            QTRY_VERIFY(EditorPanel::workspaceVault().isEmpty());
            // The workspace still decides the vault and says why it can't be used.
            QVERIFY2(EditorPanel::workspaceVaultState().unavailable.contains(QStringLiteral("no longer in Favorites")),
                     qPrintable(EditorPanel::workspaceVaultState().unavailable));
            QVERIFY(window.favorites()->add(m_a));
            QTRY_VERIFY(!EditorPanel::workspaceVault().isEmpty());
            QVERIFY(EditorPanel::workspaceVaultState().unavailable.isEmpty());

            // Preferences › Applications › Editor turns the vault off, or gives it its own options.
            const auto saveVault = [&](bool enabled, const std::optional<VaultSettings::Options> &options) {
                config->reparseConfiguration();
                WorkspaceCatalog::Catalog edited;
                QString problem;
                QVERIFY(WorkspaceCatalog::load(workspaces, edited, problem));
                QVERIFY(WorkspaceCatalog::setVaultOptions(edited, id, enabled, options, problem));
                QVERIFY(WorkspaceCatalog::save(workspaces, edited, problem));
                window.applySettings();
            };
            saveVault(false, std::nullopt);
            QVERIFY(!EditorPanel::workspaceVaultState().isActive());
            VaultSettings::Options options;
            options.wikiLinks = false;
            saveVault(true, options);
            QCOMPARE(EditorPanel::workspaceVault(), QDir::cleanPath(m_a.toLocalFile()));
            QVERIFY(EditorPanel::workspaceVaultState().options == std::optional(options));
        }
        QVERIFY(EditorPanel::workspaceVault().isEmpty());
        QVERIFY(!EditorPanel::workspaceVaultState().isActive());
    }

    void switchesAndRestoresWorkspaceLocationVisibility()
    {
        QTemporaryDir directory;
        const auto config = KSharedConfig::openConfig();
        const auto backup = KSharedConfig::openConfig(directory.filePath(QStringLiteral("backup")));
        KConfigGroup favorites = config->group(QStringLiteral("Favorites"));
        KConfigGroup workspaces = config->group(QStringLiteral("Workspaces"));
        KConfigGroup savedFavorites = backup->group(QStringLiteral("Favorites"));
        KConfigGroup savedWorkspaces = backup->group(QStringLiteral("Workspaces"));
        favorites.copyTo(&savedFavorites);
        workspaces.copyTo(&savedWorkspaces);
        favorites.deleteGroup();
        workspaces.deleteGroup();
        QVERIFY(config->sync());
        const auto restore = qScopeGuard([config, savedFavorites, savedWorkspaces]() mutable {
            config->reparseConfiguration();
            auto favorites = config->group(QStringLiteral("Favorites"));
            auto workspaces = config->group(QStringLiteral("Workspaces"));
            favorites.deleteGroup();
            workspaces.deleteGroup();
            savedFavorites.copyTo(&favorites);
            savedWorkspaces.copyTo(&workspaces);
            config->sync();
        });
        MainWindow window;
        window.openUrls({m_a, m_b});
        QVERIFY(window.favorites()->add(m_a));
        QVERIFY(window.favorites()->add(m_b));
        auto *places = window.findChild<KFilePlacesModel *>();
        QVERIFY(places);
        places->addPlace(QStringLiteral("Workspace test"), m_c);
        const auto removePlace = qScopeGuard([&] {
            for (int row = places->rowCount() - 1; row >= 0; --row) {
                if (places->url(places->index(row, 0)) == m_c) {
                    places->removePlace(places->index(row, 0));
                }
            }
        });
        auto *placesDock = window.findChild<QDockWidget *>(QStringLiteral("placesDock"));
        QVERIFY(placesDock);
        auto *placesView = qobject_cast<KFilePlacesView *>(placesDock->widget());
        QVERIFY(placesView);
        placesView->setShowAll(true);
        auto *view = window.findChild<QListView *>(QStringLiteral("combinedFavoritesView"));
        QVERIFY(view);
        window.actionCollection()->action(QStringLiteral("manage_workspaces"))->trigger();
        auto *dialog = window.findChild<QDialog *>(QStringLiteral("workspaceDialog"));
        QVERIFY(dialog);
        auto *name = dialog->findChild<QLineEdit *>(QStringLiteral("workspaceName"));
        auto *label = dialog->findChild<QLineEdit *>(QStringLiteral("workspaceShortLabel"));
        auto *list = dialog->findChild<QListWidget *>(QStringLiteral("workspaceList"));
        auto *locations = dialog->findChild<QListWidget *>(QStringLiteral("workspaceLocationList"));
        auto *create = dialog->findChild<QPushButton *>(QStringLiteral("workspaceCreate"));
        auto *assign = dialog->findChild<QPushButton *>(QStringLiteral("workspaceAssignLocation"));
        auto *activate = dialog->findChild<QPushButton *>(QStringLiteral("workspaceActivate"));
        auto *status = dialog->findChild<QLabel *>(QStringLiteral("workspaceStatus"));
        QVERIFY(name && label && list && locations && create && assign && activate && status);
        name->setText(QStringLiteral("Work"));
        label->setText(QStringLiteral("W"));
        create->click();
        name->setText(QStringLiteral("Home"));
        label->setText(QStringLiteral("H"));
        create->click();
        QVERIFY2(list->count() == 2, qPrintable(status->text()));
        list->setCurrentRow(0);
        const QString locationId = WorkspaceCatalog::locationId(m_a);
        for (int row = 0; row < locations->count(); ++row) {
            if (locations->item(row)->data(Qt::UserRole).toString() == locationId) {
                locations->setCurrentRow(row);
                break;
            }
        }
        QVERIFY(locations->currentItem());
        assign->click();
        const QString placeId = WorkspaceCatalog::locationId(m_c);
        for (int row = 0; row < locations->count(); ++row) {
            if (locations->item(row)->data(Qt::UserRole).toString() == placeId) {
                locations->setCurrentRow(row);
                break;
            }
        }
        QVERIFY(locations->currentItem());
        QCOMPARE(locations->currentItem()->data(Qt::UserRole).toString(), placeId);
        assign->click();
        MainWindow observer;
        auto *observerView = observer.findChild<QListView *>(QStringLiteral("combinedFavoritesView"));
        QVERIFY(observerView);
        QVERIFY(!observerView->isRowHidden(0));
        list->setCurrentRow(1);
        QTimer::singleShot(0, [] {
            qobject_cast<QMessageBox *>(QApplication::activeModalWidget())->button(QMessageBox::Yes)->click();
        });
        activate->click();
        QTRY_VERIFY(view->isRowHidden(0));
        QTRY_VERIFY(observerView->isRowHidden(0));
        QVERIFY(!view->isRowHidden(1));
        const auto placeRow = [&] {
            for (int row = 0; row < places->rowCount(); ++row) {
                if (places->url(places->index(row, 0)) == m_c) {
                    return row;
                }
            }
            return -1;
        };
        QVERIFY(placeRow() >= 0);
        QTRY_VERIFY(placesView->isRowHidden(placeRow()));
        dialog->close();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        MainWindow reopened;
        auto *reopenedView = reopened.findChild<QListView *>(QStringLiteral("combinedFavoritesView"));
        QVERIFY(reopenedView);
        QTRY_VERIFY(reopenedView->isRowHidden(0));
        QVERIFY(!reopenedView->isRowHidden(1));
        window.actionCollection()->action(QStringLiteral("manage_workspaces"))->trigger();
        dialog = window.findChild<QDialog *>(QStringLiteral("workspaceDialog"));
        QVERIFY(dialog);
        list = dialog->findChild<QListWidget *>(QStringLiteral("workspaceList"));
        locations = dialog->findChild<QListWidget *>(QStringLiteral("workspaceLocationList"));
        assign = dialog->findChild<QPushButton *>(QStringLiteral("workspaceAssignLocation"));
        list->setCurrentRow(1);
        for (int row = 0; row < locations->count(); ++row) {
            if (locations->item(row)->data(Qt::UserRole).toString() == locationId) {
                locations->setCurrentRow(row);
                break;
            }
        }
        QVERIFY(locations->currentItem());
        QTimer::singleShot(0, [] {
            qobject_cast<QMessageBox *>(QApplication::activeModalWidget())->button(QMessageBox::Yes)->click();
        });
        assign->click();
        QTRY_VERIFY(!reopenedView->isRowHidden(0));
        dialog->close();
    }

    void appliesWorkspaceAppearanceWhenSwitching()
    {
        QTemporaryDir directory;
        const auto config = KSharedConfig::openConfig();
        const auto backup = KSharedConfig::openConfig(directory.filePath(QStringLiteral("backup")));
        KConfigGroup workspaces = config->group(QStringLiteral("Workspaces"));
        KConfigGroup savedWorkspaces = backup->group(QStringLiteral("Workspaces"));
        workspaces.copyTo(&savedWorkspaces);
        workspaces.deleteGroup();
        QVERIFY(config->sync());
        const auto restore = qScopeGuard([config, savedWorkspaces]() mutable {
            EffectiveAppearance::setLayer({});
            config->reparseConfiguration();
            auto workspaces = config->group(QStringLiteral("Workspaces"));
            workspaces.deleteGroup();
            savedWorkspaces.copyTo(&workspaces);
            config->sync();
        });
        MainWindow window;
        window.openUrls({m_a, m_b});
        window.actionCollection()->action(QStringLiteral("manage_workspaces"))->trigger();
        auto *dialog = window.findChild<QDialog *>(QStringLiteral("workspaceDialog"));
        QVERIFY(dialog);
        auto *name = dialog->findChild<QLineEdit *>(QStringLiteral("workspaceName"));
        auto *label = dialog->findChild<QLineEdit *>(QStringLiteral("workspaceShortLabel"));
        auto *list = dialog->findChild<QListWidget *>(QStringLiteral("workspaceList"));
        auto *create = dialog->findChild<QPushButton *>(QStringLiteral("workspaceCreate"));
        auto *activate = dialog->findChild<QPushButton *>(QStringLiteral("workspaceActivate"));
        auto *overrideViewMode = dialog->findChild<QCheckBox *>(QStringLiteral("workspaceOverrideViewMode"));
        auto *viewMode = dialog->findChild<QComboBox *>(QStringLiteral("workspaceViewMode"));
        auto *overrideDividers = dialog->findChild<QCheckBox *>(QStringLiteral("workspaceOverrideDividers"));
        auto *dividers = dialog->findChild<QComboBox *>(QStringLiteral("workspaceDividers"));
        auto *saveAppearance = dialog->findChild<QPushButton *>(QStringLiteral("workspaceSaveAppearance"));
        QVERIFY(name && label && list && create && activate && overrideViewMode && viewMode && overrideDividers && dividers && saveAppearance);
        name->setText(QStringLiteral("Work"));
        label->setText(QStringLiteral("W"));
        create->click();
        name->setText(QStringLiteral("Home"));
        label->setText(QStringLiteral("H"));
        create->click();
        QCOMPARE(list->count(), 2);
        list->setCurrentRow(0);
        auto *mode = dialog->findChild<QComboBox *>(QStringLiteral("workspaceOverrideMode"));
        QVERIFY(mode);
        mode->setCurrentIndex(int(WorkspaceAppearanceEditor::OverrideMode::Selected));
        overrideViewMode->setChecked(true);
        viewMode->setCurrentIndex(Settings::EnumDefaultViewMode::Cards);
        overrideDividers->setChecked(true);
        dividers->setCurrentIndex(Settings::EnumDividers::Hidden);
        saveAppearance->click();
        QCOMPARE(int(EffectiveAppearance::dividers()), int(Settings::dividers()));

        QTimer::singleShot(0, [] {
            qobject_cast<QMessageBox *>(QApplication::activeModalWidget())->button(QMessageBox::Yes)->click();
        });
        activate->click();
        QTRY_COMPARE(int(EffectiveAppearance::dividers()), int(Settings::EnumDividers::Hidden));
        QTRY_COMPARE(window.activePane()->view()->viewMode(), FolderView::ViewMode::Cards);
        QCOMPARE(window.inactivePane()->view()->viewMode(), FolderView::ViewMode::Cards);
        // The global settings keep their values.
        QCOMPARE(int(Settings::dividers()), int(Settings::EnumDividers::FollowStyle));
        QCOMPARE(int(Settings::defaultViewMode()), int(Settings::EnumDefaultViewMode::Details));

        list->setCurrentRow(1);
        QTimer::singleShot(0, [] {
            qobject_cast<QMessageBox *>(QApplication::activeModalWidget())->button(QMessageBox::Yes)->click();
        });
        activate->click();
        QTRY_VERIFY(EffectiveAppearance::layer().isEmpty());
        QCOMPARE(int(EffectiveAppearance::dividers()), int(Settings::EnumDividers::FollowStyle));

        // Previewing another workspace's appearance applies it without switching to it or changing the panes' view modes.
        auto *preview = dialog->findChild<QPushButton *>(QStringLiteral("workspacePreviewAppearance"));
        auto *revert = dialog->findChild<QPushButton *>(QStringLiteral("workspaceRevertAppearance"));
        QVERIFY(preview && revert && preview->isEnabled());
        const FolderView::ViewMode homeMode = window.activePane()->view()->viewMode();
        list->setCurrentRow(0);
        dividers->setCurrentIndex(Settings::EnumDividers::Thin);
        preview->click();
        QCOMPARE(int(EffectiveAppearance::dividers()), int(Settings::EnumDividers::Thin));
        QCOMPARE(window.activePane()->view()->viewMode(), homeMode);
        revert->click();
        QVERIFY(EffectiveAppearance::layer().isEmpty());
        preview->click();
        QVERIFY(!EffectiveAppearance::layer().isEmpty());
        dialog->close();
        QVERIFY(EffectiveAppearance::layer().isEmpty());
        // The preview was not saved.
        WorkspaceCatalog::Catalog catalog;
        QString error;
        config->reparseConfiguration();
        QVERIFY2(WorkspaceCatalog::load(config->group(QStringLiteral("Workspaces")), catalog, error), qPrintable(error));
        QCOMPARE(catalog.workspaces.first().appearance.overrides.value(QStringLiteral("Dividers")).toString(), QStringLiteral("Hidden"));
    }

    void retriesWorkspaceWithUnavailableComponents()
    {
        Settings::setConfirmWorkspaceSwitch(Settings::EnumConfirmWorkspaceSwitch::Always);
        const auto resetConfirmation = qScopeGuard([] {
            Settings::setConfirmWorkspaceSwitch(Settings::EnumConfirmWorkspaceSwitch::WhenUnsaved);
        });
        const auto config = KSharedConfig::openConfig();
        KConfigGroup group = config->group(QStringLiteral("Workspaces"));
        const bool hadCatalog = group.hasKey("Catalog");
        const QByteArray previousCatalog = group.readEntry("Catalog", QByteArray());
        group.deleteEntry("Catalog");
        QVERIFY(group.sync());
        const auto restore = qScopeGuard([config, hadCatalog, previousCatalog] {
            config->reparseConfiguration();
            KConfigGroup group = config->group(QStringLiteral("Workspaces"));
            if (hadCatalog) {
                group.writeEntry("Catalog", previousCatalog);
            } else {
                group.deleteEntry("Catalog");
            }
            group.sync();
        });

        MainWindow window;
        window.openUrls({m_a, m_b});
        const QUrl missingLocation = QUrl::fromLocalFile(m_root.filePath(QStringLiteral("temporarily-missing")));
        window.activePane()->openInNewTab(missingLocation, true);
        window.setStackedLayout(false);
        auto *dock = new QDockWidget(&window);
        dock->setObjectName(QStringLiteral("missingWorkspaceDock"));
        window.addDockWidget(Qt::LeftDockWidgetArea, dock);
        auto *anotherDock = new QDockWidget(&window);
        anotherDock->setObjectName(QStringLiteral("anotherMissingDock"));
        window.addDockWidget(Qt::RightDockWidgetArea, anotherDock);
        window.actionCollection()->action(QStringLiteral("manage_workspaces"))->trigger();
        auto *dialog = window.findChild<WorkspaceDialog *>(QStringLiteral("workspaceDialog"));
        QVERIFY(dialog);
        auto *name = dialog->findChild<QLineEdit *>(QStringLiteral("workspaceName"));
        auto *label = dialog->findChild<QLineEdit *>(QStringLiteral("workspaceShortLabel"));
        auto *list = dialog->findChild<QListWidget *>(QStringLiteral("workspaceList"));
        auto *create = dialog->findChild<QPushButton *>(QStringLiteral("workspaceCreate"));
        auto *activate = dialog->findChild<QPushButton *>(QStringLiteral("workspaceActivate"));
        auto *status = dialog->findChild<QLabel *>(QStringLiteral("workspaceStatus"));
        QVERIFY(name && label && list && create && activate && status);
        name->setText(QStringLiteral("Work"));
        label->setText(QStringLiteral("W"));
        create->click();
        QCOMPARE(list->count(), 1);
        dialog->close();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        delete dock;
        delete anotherDock;
        auto *unrelated = new QObject(&window);
        unrelated->setObjectName(QStringLiteral("anotherMissingDock"));

        KConfig external(config->name(), config->openFlags(), config->locationType());
        auto externalGroup = external.group(QStringLiteral("Workspaces"));
        WorkspaceCatalog::Catalog saved;
        QString error;
        QVERIFY(WorkspaceCatalog::load(externalGroup, saved, error));
        QVERIFY(saved.workspaces[0].layout->presentation->windowObjects.contains(QStringLiteral("missingWorkspaceDock")));
        QVERIFY(saved.workspaces[0].layout->presentation->windowObjects.contains(QStringLiteral("anotherMissingDock")));

        window.setStackedLayout(true);
        const PaneSession leftTabs = window.activePane()->saveState();
        const PaneSession rightTabs = window.inactivePane()->saveState();
        window.actionCollection()->action(QStringLiteral("manage_workspaces"))->trigger();
        dialog = window.findChild<WorkspaceDialog *>(QStringLiteral("workspaceDialog"));
        QVERIFY(dialog);
        list = dialog->findChild<QListWidget *>(QStringLiteral("workspaceList"));
        activate = dialog->findChild<QPushButton *>(QStringLiteral("workspaceActivate"));
        status = dialog->findChild<QLabel *>(QStringLiteral("workspaceStatus"));
        list->setCurrentRow(0);
        QTimer::singleShot(0, [] {
            qobject_cast<QMessageBox *>(QApplication::activeModalWidget())->button(QMessageBox::Yes)->click();
        });
        bool listedUnavailable = false;
        answerMessageBox(QStringLiteral("workspaceUnavailableItems"), [&](QMessageBox *box) {
            listedUnavailable =
                box->informativeText().contains(QStringLiteral("missingWorkspaceDock")) && box->informativeText().contains(missingLocation.toLocalFile());
            const auto buttons = box->buttons();
            for (QAbstractButton *button : buttons) {
                if (box->buttonRole(button) == QMessageBox::AcceptRole) {
                    button->click();
                }
            }
        });
        activate->click();
        QVERIFY(listedUnavailable);
        QVERIFY(status->text().contains(QStringLiteral("missingWorkspaceDock")));
        QVERIFY(status->text().contains(QStringLiteral("anotherMissingDock")));
        QVERIFY(status->text().contains(missingLocation.toLocalFile()));
        QVERIFY(!window.isStackedLayout());
        QCOMPARE(window.activePane()->saveState(), leftTabs);
        QCOMPARE(window.inactivePane()->saveState(), rightTabs);
        external.reparseConfiguration();
        WorkspaceCatalog::Catalog retained;
        QVERIFY(WorkspaceCatalog::load(externalGroup, retained, error));
        QCOMPARE(retained.workspaces[0].layout, saved.workspaces[0].layout);

        delete unrelated;
        dock = new QDockWidget(&window);
        dock->setObjectName(QStringLiteral("missingWorkspaceDock"));
        window.addDockWidget(Qt::RightDockWidgetArea, dock);
        anotherDock = new QDockWidget(&window);
        anotherDock->setObjectName(QStringLiteral("anotherMissingDock"));
        window.addDockWidget(Qt::LeftDockWidgetArea, anotherDock);
        QVERIFY(QDir().mkpath(missingLocation.toLocalFile()));
        QTimer::singleShot(0, [] {
            qobject_cast<QMessageBox *>(QApplication::activeModalWidget())->button(QMessageBox::Yes)->click();
        });
        activate->click();
        QVERIFY2(status->text().isEmpty(), qPrintable(status->text()));
        QCOMPARE(window.dockWidgetArea(dock), Qt::LeftDockWidgetArea);
        QCOMPARE(window.dockWidgetArea(anotherDock), Qt::RightDockWidgetArea);
        dialog->close();
    }

    void switchesWorkspaceLayoutWithoutLosingTabsOrEditor()
    {
        const auto config = KSharedConfig::openConfig();
        KConfigGroup group = config->group(QStringLiteral("Workspaces"));
        QTemporaryDir toolbarBackupDirectory;
        const auto toolbarBackup = KSharedConfig::openConfig(toolbarBackupDirectory.filePath(QStringLiteral("settings")));
        KConfigGroup paneToolbars = config->group(QStringLiteral("PaneToolbars"));
        KConfigGroup savedPaneToolbars = toolbarBackup->group(QStringLiteral("PaneToolbars"));
        paneToolbars.copyTo(&savedPaneToolbars);
        const auto restorePaneToolbars = qScopeGuard([config, paneToolbars, savedPaneToolbars]() mutable {
            paneToolbars.deleteGroup();
            savedPaneToolbars.copyTo(&paneToolbars);
            config->sync();
        });
        const bool hadCatalog = group.hasKey("Catalog");
        const QByteArray previousCatalog = group.readEntry("Catalog", QByteArray());
        group.deleteEntry("Catalog");
        QVERIFY(group.sync());
        const auto restore = qScopeGuard([config, hadCatalog, previousCatalog] {
            config->reparseConfiguration();
            KConfigGroup group = config->group(QStringLiteral("Workspaces"));
            if (hadCatalog) {
                group.writeEntry("Catalog", previousCatalog);
            } else {
                group.deleteEntry("Catalog");
            }
            group.sync();
        });
        const auto editorOpening = Settings::editorOpening();
        const auto restoreEditor = qScopeGuard([editorOpening] {
            Settings::setEditorOpening(editorOpening);
            Settings::self()->save();
        });
        const auto previewPlacement = Settings::previewPlacement();
        const auto restorePreview = qScopeGuard([previewPlacement] {
            Settings::setPreviewPlacement(previewPlacement);
            Settings::self()->save();
        });
        const auto state = KSharedConfig::openStateConfig();
        KConfigGroup panels = state->group(QStringLiteral("Panels"));
        const bool hadPanelLayout = panels.hasKey("Layout");
        const QString previousPanelLayout = panels.readEntry("Layout", QString());
        const auto restorePanels = qScopeGuard([panels, hadPanelLayout, previousPanelLayout]() mutable {
            if (hadPanelLayout) {
                panels.writeEntry("Layout", previousPanelLayout);
            } else {
                panels.deleteEntry("Layout");
            }
            panels.sync();
        });
        Settings::setEditorOpening(Settings::EnumEditorOpening::Panel);
        Settings::setPreviewPlacement(Settings::EnumPreviewPlacement::Right);
        MainWindow window;
        window.openUrls({m_a, m_b});
        window.show();
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(true);
        window.setStackedLayout(false);
        window.setPanelLayout(MainWindow::PanelLayout::Separate);
        window.actionCollection()->action(QStringLiteral("open_editor"))->trigger();
        auto *editor = window.findChild<EditorPanel *>(QStringLiteral("editorPanel"));
        QVERIFY(editor && editor->document());
        editor->document()->setText(QStringLiteral("unsaved workspace draft"));
        QVERIFY(editor->document()->isModified());
        auto *queue = FileOperations::transferQueue();
        const int previousConcurrency = queue->concurrency();
        queue->setConcurrency(0);
        QPointer<WorkspaceTestJob> running;
        auto *transfer = queue->enqueue(QStringLiteral("workspace test"), [&running] {
            running = new WorkspaceTestJob;
            return running.data();
        });
        const auto restoreQueue = qScopeGuard([queue, transfer, &running, previousConcurrency] {
            if (transfer->state() == QueuedTransfer::State::Waiting) {
                transfer->kill(KJob::EmitResult);
            } else if (running && transfer->state() == QueuedTransfer::State::Running) {
                running->finish();
            }
            queue->setConcurrency(previousConcurrency);
            queue->clearFinished();
        });
        QTRY_VERIFY(running);
        QCOMPARE(transfer->state(), QueuedTransfer::State::Running);
        auto *previewAction = window.actionCollection()->action(QStringLiteral("show_preview_panel"));
        QVERIFY(previewAction);
        previewAction->setChecked(true);
        window.actionCollection()->action(QStringLiteral("manage_workspaces"))->trigger();
        auto *dialog = window.findChild<QDialog *>(QStringLiteral("workspaceDialog"));
        QVERIFY(dialog);
        auto *name = dialog->findChild<QLineEdit *>(QStringLiteral("workspaceName"));
        auto *label = dialog->findChild<QLineEdit *>(QStringLiteral("workspaceShortLabel"));
        auto *list = dialog->findChild<QListWidget *>(QStringLiteral("workspaceList"));
        auto *create = dialog->findChild<QPushButton *>(QStringLiteral("workspaceCreate"));
        auto *activate = dialog->findChild<QPushButton *>(QStringLiteral("workspaceActivate"));
        auto *status = dialog->findChild<QLabel *>(QStringLiteral("workspaceStatus"));
        QVERIFY(name && label && list && create && activate && status);
        Pane *const savedActivePane = window.activePane();
        Pane *const savedInactivePane = window.inactivePane();
        const PaneSession savedActiveTabs = savedActivePane->saveState();
        const PaneSession savedInactiveTabs = savedInactivePane->saveState();
        const QStringList savedToolbarActions = savedActivePane->toolbarActions();
        const QStringList savedVisibleToolbarActions = savedActivePane->visibleToolbarActions();
        const QStringList savedHiddenToolbarActions = savedActivePane->hiddenToolbarActions();
        QVERIFY(!savedToolbarActions.isEmpty());
        QVERIFY(!savedVisibleToolbarActions.isEmpty());
        QStringList alteredVisibleToolbarActions = savedVisibleToolbarActions;
        alteredVisibleToolbarActions.removeFirst();
        const QUrl savedActiveLocation = savedActivePane->url();
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(false);
        name->setText(QStringLiteral("Work"));
        label->setText(QStringLiteral("W"));
        create->click();
        QCOMPARE(list->count(), 1);
        savedActivePane->setToolbarConfiguration(savedToolbarActions, alteredVisibleToolbarActions, savedHiddenToolbarActions);
        Q_EMIT editor->placementRequested(EditorPanel::Placement::Window);
        QVERIFY(editor->window() != &window);
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(true);
        auto *linked = window.actionCollection()->action(QStringLiteral("link_panes"));
        QVERIFY(linked);
        linked->setChecked(true);
        QVERIFY(linked->isChecked());
        QTimer::singleShot(0, [dialog] {
            qobject_cast<WorkspaceDialog *>(dialog)->refreshCatalog();
            qobject_cast<QMessageBox *>(QApplication::activeModalWidget())->button(QMessageBox::Yes)->click();
        });
        activate->click();
        QVERIFY(!status->text().isEmpty());
        QVERIFY(linked->isChecked());
        linked->setChecked(false);
        window.setPanelLayout(MainWindow::PanelLayout::Combined);
        previewAction->setChecked(false);
        window.actionCollection()->action(QStringLiteral("move_second_pane_to_tab"))->trigger();
        window.setStackedLayout(true);
        QTimer::singleShot(0, [] {
            qobject_cast<QMessageBox *>(QApplication::activeModalWidget())->button(QMessageBox::Yes)->click();
        });
        activate->click();
        QVERIFY2(status->text().isEmpty(), qPrintable(status->text()));
        QVERIFY(!window.isStackedLayout());
        QCOMPARE(window.panelLayout(), MainWindow::PanelLayout::Separate);
        QVERIFY(previewAction->isChecked());
        QVERIFY(window.actionCollection()->action(QStringLiteral("move_second_pane_to_tab"))->isEnabled());
        QCOMPARE(window.activePane(), savedActivePane);
        QCOMPARE(window.activePane()->url(), savedActiveLocation);
        QCOMPARE(window.activePane()->saveState(), savedActiveTabs);
        QCOMPARE(window.inactivePane()->saveState(), savedInactiveTabs);
        QCOMPARE(savedActivePane->toolbarActions(), savedToolbarActions);
        QCOMPARE(savedActivePane->visibleToolbarActions(), savedVisibleToolbarActions);
        QCOMPARE(editor->document()->text(), QStringLiteral("unsaved workspace draft"));
        QVERIFY(editor->document()->isModified());
        QCOMPARE(editor->window(), static_cast<QWidget *>(&window));
        QCOMPARE(transfer->state(), QueuedTransfer::State::Running);

        window.actionCollection()->action(QStringLiteral("move_second_pane_to_tab"))->trigger();
        name->setText(QStringLiteral("Study"));
        label->setText(QStringLiteral("S"));
        create->click();
        QCOMPARE(list->count(), 2);
        window.actionCollection()->action(QStringLiteral("restore_second_pane"))->trigger();
        QTimer::singleShot(0, [] {
            qobject_cast<QMessageBox *>(QApplication::activeModalWidget())->button(QMessageBox::Yes)->click();
        });
        activate->click();
        QVERIFY2(status->text().isEmpty(), qPrintable(status->text()));
        QVERIFY(!window.actionCollection()->action(QStringLiteral("move_second_pane_to_tab"))->isEnabled());
        QCOMPARE(editor->document()->text(), QStringLiteral("unsaved workspace draft"));
        list->setCurrentRow(0);
        QTimer::singleShot(0, [] {
            qobject_cast<QMessageBox *>(QApplication::activeModalWidget())->button(QMessageBox::Yes)->click();
        });
        activate->click();
        QVERIFY2(status->text().isEmpty(), qPrintable(status->text()));
        QVERIFY(window.actionCollection()->action(QStringLiteral("move_second_pane_to_tab"))->isEnabled());
        dialog->close();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

        {
            MainWindow reopened;
            reopened.openUrls({m_c});
            reopened.setStackedLayout(true);
            reopened.actionCollection()->action(QStringLiteral("manage_workspaces"))->trigger();
            auto *selector = reopened.findChild<QListWidget *>(QStringLiteral("workspaceList"));
            QVERIFY(selector && selector->count() == 2);
            QVERIFY(selector->item(0)->text().contains(QStringLiteral("current")));
            QCOMPARE(reopened.activePane()->url(), m_c);
            QVERIFY(reopened.isStackedLayout());
        }

        KConfig external(config->name(), config->openFlags(), config->locationType());
        KConfigGroup externalGroup = external.group(QStringLiteral("Workspaces"));
        WorkspaceCatalog::Catalog saved;
        QString error;
        QVERIFY(WorkspaceCatalog::load(externalGroup, saved, error));
        {
            // Layouts saved before the search panel existed have one workspace size fewer.
            auto legacy = saved;
            WorkspaceCatalog::Layout legacyLayout = *legacy.workspaces.first().layout;
            QCOMPARE(legacyLayout.workspaceSizes.size(), 3);
            legacyLayout.workspaceSizes.removeLast();
            QVERIFY(WorkspaceCatalog::setLayout(legacy, legacy.workspaces.first().id, legacyLayout, error));
            QVERIFY2(WorkspaceCatalog::saveIfUnchanged(externalGroup, saved, legacy, error), qPrintable(error));
            window.actionCollection()->action(QStringLiteral("manage_workspaces"))->trigger();
            dialog = window.findChild<QDialog *>(QStringLiteral("workspaceDialog"));
            QVERIFY(dialog);
            dialog->findChild<QListWidget *>(QStringLiteral("workspaceList"))->setCurrentRow(0);
            QTimer::singleShot(0, [] {
                qobject_cast<QMessageBox *>(QApplication::activeModalWidget())->button(QMessageBox::Yes)->click();
            });
            dialog->findChild<QPushButton *>(QStringLiteral("workspaceActivate"))->click();
            const QString legacyStatus = dialog->findChild<QLabel *>(QStringLiteral("workspaceStatus"))->text();
            QVERIFY2(legacyStatus.isEmpty(), qPrintable(legacyStatus));
            dialog->close();
            QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
            external.reparseConfiguration();
            QVERIFY(WorkspaceCatalog::load(externalGroup, saved, error));
        }
        {
            // A layout saved with the search panel beside the terminal restores while it is placed apart.
            auto shared = saved;
            WorkspaceCatalog::Layout sharedLayout = *shared.workspaces.first().layout;
            while (sharedLayout.workspaceSizes.size() < 3) {
                sharedLayout.workspaceSizes.append(1);
            }
            QVERIFY(sharedLayout.presentation);
            const bool terminalRight = sharedLayout.presentation->terminalPlacement == WorkspaceCatalog::TerminalPlacement::Right;
            QVERIFY(WorkspaceCatalog::setLayout(shared, shared.workspaces.first().id, sharedLayout, error));
            QVERIFY2(WorkspaceCatalog::saveIfUnchanged(externalGroup, saved, shared, error), qPrintable(error));
            Settings::setSearchPanelPlacement(terminalRight ? Settings::EnumSearchPanelPlacement::Below : Settings::EnumSearchPanelPlacement::Right);
            window.actionCollection()->action(QStringLiteral("manage_workspaces"))->trigger();
            dialog = window.findChild<QDialog *>(QStringLiteral("workspaceDialog"));
            QVERIFY(dialog);
            dialog->findChild<QListWidget *>(QStringLiteral("workspaceList"))->setCurrentRow(0);
            QTimer::singleShot(0, [] {
                qobject_cast<QMessageBox *>(QApplication::activeModalWidget())->button(QMessageBox::Yes)->click();
            });
            dialog->findChild<QPushButton *>(QStringLiteral("workspaceActivate"))->click();
            const QString separateStatus = dialog->findChild<QLabel *>(QStringLiteral("workspaceStatus"))->text();
            QVERIFY2(separateStatus.isEmpty(), qPrintable(separateStatus));
            auto *outer = window.findChild<QSplitter *>(QStringLiteral("searchSplitter"));
            QVERIFY(outer);
            QCOMPARE(outer->indexOf(window.findChild<QWidget *>(QStringLiteral("searchPanel"))), 1);
            Settings::setSearchPanelPlacement(Settings::EnumSearchPanelPlacement::WithTerminal);
            window.applySettings();
            QCOMPARE(outer->count(), 1);
            dialog->close();
            QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
            external.reparseConfiguration();
            QVERIFY(WorkspaceCatalog::load(externalGroup, saved, error));
        }
        auto broken = saved;
        WorkspaceCatalog::Layout invalid = *broken.workspaces.first().layout;
        invalid.windowState = QByteArray("not a Qt window state");
        QVERIFY(WorkspaceCatalog::setLayout(broken, broken.workspaces.first().id, invalid, error));
        QVERIFY2(WorkspaceCatalog::saveIfUnchanged(externalGroup, saved, broken, error), qPrintable(error));
        window.setStackedLayout(true);
        window.setPanelLayout(MainWindow::PanelLayout::Combined);
        window.actionCollection()->action(QStringLiteral("manage_workspaces"))->trigger();
        dialog = window.findChild<QDialog *>(QStringLiteral("workspaceDialog"));
        QVERIFY(dialog);
        list = dialog->findChild<QListWidget *>(QStringLiteral("workspaceList"));
        activate = dialog->findChild<QPushButton *>(QStringLiteral("workspaceActivate"));
        status = dialog->findChild<QLabel *>(QStringLiteral("workspaceStatus"));
        QVERIFY(list && activate && status);
        Pane *const activePaneBeforeFailure = window.activePane();
        const QUrl activeLocationBeforeFailure = activePaneBeforeFailure->url();
        const PaneSession activeTabsBeforeFailure = activePaneBeforeFailure->saveState();
        const PaneSession inactiveTabsBeforeFailure = window.inactivePane()->saveState();
        list->setCurrentRow(0);
        QTimer::singleShot(0, [] {
            qobject_cast<QMessageBox *>(QApplication::activeModalWidget())->button(QMessageBox::Yes)->click();
        });
        activate->click();
        QVERIFY(!status->text().isEmpty());
        QVERIFY(window.isStackedLayout());
        QCOMPARE(window.panelLayout(), MainWindow::PanelLayout::Combined);
        QCOMPARE(window.activePane(), activePaneBeforeFailure);
        QCOMPARE(window.activePane()->url(), activeLocationBeforeFailure);
        QCOMPARE(window.activePane()->saveState(), activeTabsBeforeFailure);
        QCOMPARE(window.inactivePane()->saveState(), inactiveTabsBeforeFailure);
        QCOMPARE(editor->document()->text(), QStringLiteral("unsaved workspace draft"));
        QVERIFY(editor->document()->isModified());
        QCOMPARE(transfer->state(), QueuedTransfer::State::Running);
        dialog->close();
    }

    void inactivePaneDimmingFollowsFocusAndPersists()
    {
        const bool previous = Settings::dimInactivePane();
        const auto restore = qScopeGuard([previous] {
            Settings::setDimInactivePane(previous);
            Settings::self()->save();
        });
        Settings::setDimInactivePane(false);
        MainWindow window;
        window.openUrls({m_a, m_b});
        window.show();
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(true);
        auto *action = window.actionCollection()->action(QStringLiteral("dim_inactive_pane"));
        QVERIFY(action && action->isCheckable());
        auto *active = window.activePane();
        auto *inactive = window.inactivePane();
        auto *activeEffect = qobject_cast<QGraphicsOpacityEffect *>(active->graphicsEffect());
        auto *inactiveEffect = qobject_cast<QGraphicsOpacityEffect *>(inactive->graphicsEffect());
        QVERIFY(activeEffect && inactiveEffect);
        QVERIFY(!activeEffect->isEnabled());
        QVERIFY(!inactiveEffect->isEnabled());
        QTRY_COMPARE(active->view()->itemCount(), 2);
        QTRY_COMPARE(inactive->view()->itemCount(), 0);
        const QRect inactiveRect(inactive->mapTo(&window, QPoint()), inactive->size());
        const QImage before = window.grab(inactiveRect).toImage();
        action->trigger();
        QVERIFY(Settings::dimInactivePane());
        QVERIFY(!activeEffect->isEnabled());
        QVERIFY(inactiveEffect->isEnabled());
        const QImage after = window.grab(inactiveRect).toImage();
        QVERIFY(before != after);
        QTest::mouseClick(inactive->view()->currentView()->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
        QCOMPARE(window.activePane(), inactive);
        QVERIFY(activeEffect->isEnabled());
        QVERIFY(!inactiveEffect->isEnabled());
        window.actionCollection()->action(QStringLiteral("switch_pane"))->trigger();
        QCOMPARE(window.activePane(), active);
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(false);
        QVERIFY(!window.activePane()->graphicsEffect()->isEnabled());
        Settings::self()->load();
        QVERIFY(Settings::dimInactivePane());
        MainWindow another;
        auto *otherAction = another.actionCollection()->action(QStringLiteral("dim_inactive_pane"));
        QVERIFY(otherAction->isChecked());
        QVERIFY(another.inactivePane()->graphicsEffect()->isEnabled());
        action->trigger();
        QVERIFY(!activeEffect->isEnabled());
        QVERIFY(!inactiveEffect->isEnabled());
        Settings::setDimInactivePane(true);
        window.applySettings();
        QVERIFY(action->isChecked());
        QVERIFY(inactiveEffect->isEnabled());
    }

    void swappingCurrentTabsPreservesTabsAndLinkedNavigation()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("a/sub")));
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("b/sub")));
        const QUrl a = QUrl::fromLocalFile(root.filePath(QStringLiteral("a")));
        const QUrl b = QUrl::fromLocalFile(root.filePath(QStringLiteral("b")));
        MainWindow window;
        window.openUrls({a, b});
        window.show();
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(true);
        auto *left = window.activePane();
        auto *right = window.inactivePane();
        auto *link = window.actionCollection()->action(QStringLiteral("link_panes"));
        link->setChecked(true);
        QVERIFY(link->isChecked());
        const QUrl leftSub = QUrl::fromLocalFile(root.filePath(QStringLiteral("a/sub")));
        const QUrl rightSub = QUrl::fromLocalFile(root.filePath(QStringLiteral("b/sub")));
        left->setUrl(leftSub);
        QCOMPARE(left->url(), leftSub);
        QCOMPARE(right->url(), rightSub);
        left->tabs()->setName(left->tabs()->currentIndex(), QStringLiteral("Left"));
        left->tabs()->setPinned(left->tabs()->currentIndex(), true);
        right->tabs()->setName(right->tabs()->currentIndex(), QStringLiteral("Right"));
        right->tabs()->setLocked(right->tabs()->currentIndex(), true);

        auto *swap = window.actionCollection()->action(QStringLiteral("swap_current_tabs"));
        QVERIFY(swap && swap->isEnabled());
        swap->trigger();
        QCOMPARE(left->url(), rightSub);
        QCOMPARE(right->url(), leftSub);
        QCOMPARE(left->tabs()->nameAt(left->tabs()->currentIndex()), QStringLiteral("Right"));
        QVERIFY(left->tabs()->isLocked(left->tabs()->currentIndex()));
        QCOMPARE(right->tabs()->nameAt(right->tabs()->currentIndex()), QStringLiteral("Left"));
        QVERIFY(right->tabs()->isPinned(right->tabs()->currentIndex()));
        QVERIFY(link->isChecked());
        left->tabs()->setLocked(left->tabs()->currentIndex(), false);
        left->setUrl(b);
        QCOMPARE(right->url(), a);
        QVERIFY(link->isChecked());
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(false);
        QVERIFY(!swap->isEnabled());
    }

    void swappingPaneSidesPreservesPaneWidgetsAndLayout()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("a/sub")));
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("b/sub")));
        const QUrl a = QUrl::fromLocalFile(root.filePath(QStringLiteral("a")));
        const QUrl b = QUrl::fromLocalFile(root.filePath(QStringLiteral("b")));
        MainWindow window;
        window.openUrls({a, b});
        window.show();
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(true);
        auto *first = window.activePane();
        auto *second = window.inactivePane();
        auto *splitter = window.findChild<QSplitter *>(QStringLiteral("paneSplitter"));
        QVERIFY(splitter);
        auto *link = window.actionCollection()->action(QStringLiteral("link_panes"));
        link->setChecked(true);
        QVERIFY(link->isChecked());
        window.setStackedLayout(true);
        QCOMPARE(splitter->widget(0), first);
        QCOMPARE(splitter->widget(1), second);
        auto *swap = window.actionCollection()->action(QStringLiteral("swap_pane_sides"));
        QVERIFY(swap && swap->isEnabled());
        swap->trigger();
        QCOMPARE(splitter->widget(0), second);
        QCOMPARE(splitter->widget(1), first);
        QCOMPARE(window.activePane(), first);
        QVERIFY(second->isTabBarAtBottom());
        QVERIFY(!first->isTabBarAtBottom());
        QVERIFY(link->isChecked());
        first->setUrl(QUrl::fromLocalFile(root.filePath(QStringLiteral("a/sub"))));
        QCOMPARE(second->url(), QUrl::fromLocalFile(root.filePath(QStringLiteral("b/sub"))));
        QVERIFY(link->isChecked());
    }

    void rightToLeftMirrorsPanesAndDetailsNavigation()
    {
        const auto previousDirection = QApplication::layoutDirection();
        const auto restoreDirection = qScopeGuard([&] {
            QApplication::setLayoutDirection(previousDirection);
        });
        QApplication::setLayoutDirection(InterfaceLanguage::layoutDirection({QStringLiteral("ar")}));
        MainWindow window;
        QCOMPARE(window.layoutDirection(), Qt::RightToLeft);
        window.resize(1000, 600);
        window.openUrls({m_a, m_b});
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(true);
        window.show();

        auto *splitter = window.findChild<QSplitter *>(QStringLiteral("paneSplitter"));
        QVERIFY(splitter);
        QTRY_VERIFY(splitter->widget(0)->geometry().left() > splitter->widget(1)->geometry().left());
        auto *view = window.activePane()->view();
        view->setViewMode(FolderView::ViewMode::Details);
        QSignalSpy up(view, &FolderView::goUpRequested);
        QTest::keyClick(view->currentView(), Qt::Key_Right);
        QCOMPARE(up.count(), 1);
        window.setLayoutDirection(Qt::LeftToRight);
        QTRY_VERIFY(splitter->widget(0)->geometry().left() < splitter->widget(1)->geometry().left());
        QTest::keyClick(view->currentView(), Qt::Key_Left);
        QCOMPARE(up.count(), 2);
    }

    void preferredPaneSwapUsesConfiguredOperation()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("left")));
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("right")));

        const auto previousMode = Settings::paneSwapMode();
        const auto restoreMode = qScopeGuard([&] {
            Settings::setPaneSwapMode(previousMode);
        });
        Settings::setPaneSwapMode(Settings::EnumPaneSwapMode::CurrentTabs);

        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(root.filePath(QStringLiteral("left"))), QUrl::fromLocalFile(root.filePath(QStringLiteral("right")))});
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(true);
        Pane *left = window.activePane();
        Pane *right = window.inactivePane();
        auto *splitter = window.findChild<QSplitter *>(QStringLiteral("paneSplitter"));
        QAction *swap = window.actionCollection()->action(QStringLiteral("swap_panes"));
        QVERIFY(left && right && splitter && swap && swap->isEnabled());
        const QUrl leftUrl = left->url();
        const QUrl rightUrl = right->url();
        swap->trigger();
        QCOMPARE(left->url(), rightUrl);
        QCOMPARE(right->url(), leftUrl);

        Settings::setPaneSwapMode(Settings::EnumPaneSwapMode::WholeSides);
        QWidget *firstSide = splitter->widget(0);
        QWidget *secondSide = splitter->widget(1);
        swap->trigger();
        QCOMPARE(splitter->widget(0), secondSide);
        QCOMPARE(splitter->widget(1), firstSide);
    }

    void secondPaneCanMoveBetweenSplitTabAndWindow()
    {
        const auto state = KSharedConfig::openStateConfig();
        KConfigGroup layout = state->group(QStringLiteral("WindowLayout"));
        const bool hadPlacement = layout.hasKey(QStringLiteral("SecondPanePlacement"));
        const QString previousPlacement = layout.readEntry("SecondPanePlacement", QString());
        layout.writeEntry("SecondPanePlacement", QStringLiteral("split"));
        state->sync();
        const auto restorePlacement = qScopeGuard([&] {
            if (hadPlacement) {
                layout.writeEntry("SecondPanePlacement", previousPlacement);
            } else {
                layout.deleteEntry("SecondPanePlacement");
            }
            state->sync();
        });

        MainWindow window;
        window.openUrls({m_a, m_b});
        window.show();
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(true);
        Pane *first = window.activePane();
        Pane *second = window.inactivePane();
        auto *splitter = window.findChild<QSplitter *>(QStringLiteral("paneSplitter"));
        auto *paneTabs = window.findChild<QTabWidget *>(QStringLiteral("paneLayoutTabs"));
        QVERIFY(splitter && paneTabs);
        QVERIFY(splitter->indexOf(second) >= 0);

        window.actionCollection()->action(QStringLiteral("move_second_pane_to_tab"))->trigger();
        QCOMPARE(paneTabs->currentWidget(), second);
        QVERIFY(splitter->indexOf(second) < 0);
        QCOMPARE(window.activePane(), second);
        paneTabs->setCurrentIndex(0);
        QCOMPARE(window.activePane(), first);
        paneTabs->setCurrentWidget(second);
        QCOMPARE(window.activePane(), second);
        window.actionCollection()->action(QStringLiteral("restore_second_pane"))->trigger();
        QVERIFY(splitter->indexOf(second) >= 0);
        QCOMPARE(window.activePane(), second);

        window.actionCollection()->action(QStringLiteral("move_second_pane_to_window"))->trigger();
        auto *detachedWindow = window.findChild<QMainWindow *>(QStringLiteral("secondPaneWindow"));
        QVERIFY(detachedWindow);
        QCOMPARE(detachedWindow->centralWidget(), second);
        detachedWindow->close();
        QTRY_VERIFY(splitter->indexOf(second) >= 0);
        QCOMPARE(window.activePane(), second);
        QCOMPARE(window.inactivePane(), first);
    }

    void preferredSecondPaneMoveUsesConfiguredDestinationAndStyle()
    {
        const auto state = KSharedConfig::openStateConfig();
        KConfigGroup layout = state->group(QStringLiteral("WindowLayout"));
        const bool hadPlacement = layout.hasKey(QStringLiteral("SecondPanePlacement"));
        const QString previousPlacement = layout.readEntry("SecondPanePlacement", QString());
        layout.writeEntry("SecondPanePlacement", QStringLiteral("split"));
        state->sync();
        const auto restorePlacement = qScopeGuard([&] {
            if (hadPlacement) {
                layout.writeEntry("SecondPanePlacement", previousPlacement);
            } else {
                layout.deleteEntry("SecondPanePlacement");
            }
            state->sync();
        });

        const auto previousDestination = Settings::paneMoveDestination();
        const auto previousStyle = Settings::paneMoveDisplayStyle();
        const auto previousDefaultMode = Settings::defaultViewMode();
        const auto restoreSettings = qScopeGuard([&] {
            Settings::setPaneMoveDestination(previousDestination);
            Settings::setPaneMoveDisplayStyle(previousStyle);
            Settings::setDefaultViewMode(previousDefaultMode);
        });
        Settings::setPaneMoveDestination(Settings::EnumPaneMoveDestination::Tab);
        Settings::setPaneMoveDisplayStyle(Settings::EnumPaneMoveDisplayStyle::Keep);
        Settings::setDefaultViewMode(Settings::EnumDefaultViewMode::Details);

        MainWindow window;
        window.openUrls({m_a, m_b});
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        Pane *second = window.inactivePane();
        QVERIFY(second);
        second->view()->setViewMode(FolderView::ViewMode::Icons);

        QAction *movePane = window.actionCollection()->action(QStringLiteral("move_second_pane"));
        auto *paneTabs = window.findChild<QTabWidget *>(QStringLiteral("paneLayoutTabs"));
        QVERIFY(movePane && paneTabs);
        movePane->trigger();
        QCOMPARE(paneTabs->currentWidget(), second);
        QCOMPARE(second->view()->viewMode(), FolderView::ViewMode::Icons);

        window.actionCollection()->action(QStringLiteral("restore_second_pane"))->trigger();
        Settings::setPaneMoveDestination(Settings::EnumPaneMoveDestination::Window);
        Settings::setPaneMoveDisplayStyle(Settings::EnumPaneMoveDisplayStyle::ResetToDefault);
        second->view()->setViewMode(FolderView::ViewMode::Icons);
        movePane->trigger();

        auto *detachedWindow = window.findChild<QMainWindow *>(QStringLiteral("secondPaneWindow"));
        QVERIFY(detachedWindow);
        QCOMPARE(detachedWindow->centralWidget(), second);
        QCOMPARE(second->view()->viewMode(), FolderView::ViewMode::Details);
        detachedWindow->close();
        auto *splitter = window.findChild<QSplitter *>(QStringLiteral("paneSplitter"));
        QVERIFY(splitter);
        QTRY_VERIFY(splitter->indexOf(second) >= 0);
    }

    void tabBarPlacementAndCloseButtonsFollowPaneAndLayout()
    {
        const int horizontalFirst = Settings::horizontalFirstTabPosition();
        const int horizontalSecond = Settings::horizontalSecondTabPosition();
        const int stackedFirst = Settings::stackedFirstTabPosition();
        const int stackedSecond = Settings::stackedSecondTabPosition();
        const bool together = Settings::stackedTabBarsTogether();
        const bool closeButtons = Settings::showTabCloseButtons();
        const auto restore = qScopeGuard([=] {
            Settings::setHorizontalFirstTabPosition(horizontalFirst);
            Settings::setHorizontalSecondTabPosition(horizontalSecond);
            Settings::setStackedFirstTabPosition(stackedFirst);
            Settings::setStackedSecondTabPosition(stackedSecond);
            Settings::setStackedTabBarsTogether(together);
            Settings::setShowTabCloseButtons(closeButtons);
            Settings::self()->save();
        });
        Settings::setHorizontalFirstTabPosition(0);
        Settings::setHorizontalSecondTabPosition(0);
        Settings::setStackedFirstTabPosition(0);
        Settings::setStackedSecondTabPosition(0);
        Settings::setStackedTabBarsTogether(true);
        Settings::setShowTabCloseButtons(true);

        MainWindow window;
        window.openUrls({m_a, m_b});
        window.show();
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(true);
        window.setStackedLayout(false);
        auto *splitter = window.findChild<QSplitter *>(QStringLiteral("paneSplitter"));
        QVERIFY(splitter);
        auto *first = qobject_cast<Pane *>(splitter->widget(0));
        auto *second = qobject_cast<Pane *>(splitter->widget(1));
        QVERIFY(first && second);
        first->setTabBarForcedVisible(true);
        second->setTabBarForcedVisible(true);
        if (window.activePane() != first) {
            window.actionCollection()->action(QStringLiteral("switch_pane"))->trigger();
        }
        auto *position = qobject_cast<KSelectAction *>(window.actionCollection()->action(QStringLiteral("tab_bar_position")));
        auto *stackedTogether = window.actionCollection()->action(QStringLiteral("stacked_tab_bars_together"));
        auto *showClose = window.actionCollection()->action(QStringLiteral("show_tab_close_buttons"));
        QVERIFY(position && stackedTogether && showClose);
        QCOMPARE(position->actions().size(), 4);
        position->actions().at(2)->trigger();
        QCOMPARE(first->tabBarPosition(), Pane::TabBarPosition::Left);
        QCOMPARE(second->tabBarPosition(), Pane::TabBarPosition::Top);
        auto *firstBar = first->findChild<QTabBar *>();
        QVERIFY(firstBar);
        QTRY_VERIFY(firstBar->parentWidget()->geometry().right() < first->view()->geometry().left());
        window.actionCollection()->action(QStringLiteral("switch_pane"))->trigger();
        position->actions().at(3)->trigger();
        QCOMPARE(second->tabBarPosition(), Pane::TabBarPosition::Right);
        auto *secondBar = second->findChild<QTabBar *>();
        QVERIFY(secondBar);
        QTRY_VERIFY(secondBar->parentWidget()->geometry().left() > second->view()->geometry().right());
        window.setStackedLayout(true);
        QCOMPARE(first->tabBarPosition(), Pane::TabBarPosition::Bottom);
        QCOMPARE(second->tabBarPosition(), Pane::TabBarPosition::Top);
        position->actions().at(1)->trigger();
        QVERIFY(!stackedTogether->isChecked());
        QCOMPARE(first->tabBarPosition(), Pane::TabBarPosition::Top);
        QCOMPARE(second->tabBarPosition(), Pane::TabBarPosition::Bottom);
        window.setStackedLayout(false);
        QCOMPARE(first->tabBarPosition(), Pane::TabBarPosition::Left);
        QCOMPARE(second->tabBarPosition(), Pane::TabBarPosition::Right);
        window.setStackedLayout(true);
        QCOMPARE(first->tabBarPosition(), Pane::TabBarPosition::Top);
        QCOMPARE(second->tabBarPosition(), Pane::TabBarPosition::Bottom);
        stackedTogether->setChecked(true);
        QCOMPARE(first->tabBarPosition(), Pane::TabBarPosition::Bottom);
        QCOMPARE(second->tabBarPosition(), Pane::TabBarPosition::Top);

        const int initialTabs = second->tabs()->count();
        second->openInNewTab(m_c, true);
        auto *tabBar = second->findChild<QTabBar *>();
        QVERIFY(tabBar);
        QTRY_COMPARE(tabBar->count(), initialTabs + 1);
        const int lockedIndex = second->tabs()->currentIndex();
        second->tabs()->setPinned(0, true);
        second->tabs()->setLocked(lockedIndex, true);
        showClose->setChecked(false);
        QVERIFY(!tabBar->tabsClosable());
        QVERIFY(tabBar->isVisible());
        Q_EMIT tabBar->tabCloseRequested(0);
        Q_EMIT tabBar->tabCloseRequested(lockedIndex);
        QCOMPARE(second->tabs()->count(), initialTabs + 1);
        showClose->setChecked(true);
        const auto closeSide = static_cast<QTabBar::ButtonPosition>(tabBar->style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, nullptr, tabBar));
        QVERIFY(tabBar->tabButton(0, closeSide) && !tabBar->tabButton(0, closeSide)->isVisible());
        QVERIFY(tabBar->tabButton(lockedIndex, closeSide) && !tabBar->tabButton(lockedIndex, closeSide)->isVisible());
        showClose->setChecked(false);
        second->tabs()->setLocked(lockedIndex, false);
        auto *closeTab = window.actionCollection()->action(QStringLiteral("close_tab"));
        QTRY_VERIFY(closeTab->isEnabled());
        closeTab->trigger();
        QCOMPARE(second->tabs()->count(), initialTabs);

        MainWindow restored;
        restored.openUrls({m_a, m_b});
        restored.show();
        restored.setStackedLayout(false);
        auto *restoredSplitter = restored.findChild<QSplitter *>(QStringLiteral("paneSplitter"));
        QVERIFY(restoredSplitter);
        QCOMPARE(qobject_cast<Pane *>(restoredSplitter->widget(0))->tabBarPosition(), Pane::TabBarPosition::Left);
        QCOMPARE(qobject_cast<Pane *>(restoredSplitter->widget(1))->tabBarPosition(), Pane::TabBarPosition::Right);
    }

    void savedCommandsCanBeAddedToToolbarAndRequirePreview()
    {
        QTemporaryDir backupDir;
        QVERIFY(backupDir.isValid());
        const auto config = KSharedConfig::openConfig();
        KConfigGroup group = config->group(QStringLiteral("CustomCommands"));
        const auto backup = KSharedConfig::openConfig(backupDir.filePath(QStringLiteral("settings")));
        KConfigGroup saved = backup->group(QStringLiteral("CustomCommands"));
        group.copyTo(&saved);
        const auto cleanup = qScopeGuard([&] {
            group.deleteGroup();
            saved.copyTo(&group);
            config->sync();
        });
        QString error;
        const CustomCommands::Command command{QStringLiteral("Toolbar command"), QStringLiteral("/bin/echo"), {QStringLiteral("{dir}")}, {}};
        QVERIFY2(CustomCommands::save(group, {command}, &error), qPrintable(error));
        MainWindow window;
        window.openUrls({m_a, m_b});
        window.show();
        const auto actions = window.actionCollection()->actions();
        const auto it = std::find_if(actions.cbegin(), actions.cend(), [&command](const QAction *action) {
            return action->iconText() == command.name && action->objectName().startsWith(QLatin1String("custom_command_preset_"));
        });
        QVERIFY(it != actions.cend());
        QPointer<QAction> action = *it;
        QVERIFY(action->isEnabled());
        // Configure Keyboard Shortcuts and the toolbar editor's available actions list the text.
        QCOMPARE(action->text(), QStringLiteral("Saved Command: Toolbar command"));
        KEditToolBar editor(window.guiFactory(), &window);
        editor.show();
        const auto listed = [&editor, &action] {
            const auto views = editor.findChildren<QAbstractItemView *>();
            return std::any_of(views.cbegin(), views.cend(), [&action](QAbstractItemView *view) {
                return view->model()
                    && !view->model()->match(view->model()->index(0, 0), Qt::DisplayRole, action->text(), 1, Qt::MatchExactly | Qt::MatchRecursive).isEmpty();
            });
        };
        QTRY_VERIFY(listed());
        editor.close();
        window.toolBar()->addAction(action);
        auto *toolbarButton = qobject_cast<QToolButton *>(window.toolBar()->widgetForAction(action));
        QVERIFY(toolbarButton);
        QTest::mouseClick(toolbarButton, Qt::LeftButton);
        auto *dialog = window.findChild<CustomCommandDialog *>(QStringLiteral("customCommandDialog"));
        QVERIFY(dialog);
        QCOMPARE(dialog->findChild<QLineEdit *>(QStringLiteral("commandProgram"))->text(), command.program);
        QCOMPARE(dialog->findChild<QPlainTextEdit *>(QStringLiteral("commandArguments"))->toPlainText(), QStringLiteral("{dir}"));
        QVERIFY(!dialog->findChild<QPushButton *>(QStringLiteral("commandRun"))->isEnabled());
        dialog->findChild<QPushButton *>(QStringLiteral("commandPreview"))->click();
        QVERIFY(dialog->findChild<QPushButton *>(QStringLiteral("commandRun"))->isEnabled());
        QTimer::singleShot(0, [] {
            auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            QVERIFY(box);
            box->button(QMessageBox::Yes)->click();
        });
        dialog->findChild<QPushButton *>(QStringLiteral("commandDeletePreset"))->click();
        QVERIFY(action.isNull());
        QTimer::singleShot(0, [] {
            auto *input = qobject_cast<QInputDialog *>(QApplication::activeModalWidget());
            QVERIFY(input);
            input->setTextValue(QStringLiteral("New toolbar command"));
            input->accept();
        });
        dialog->findChild<QPushButton *>(QStringLiteral("commandSavePreset"))->click();
        const auto refreshed = window.actionCollection()->actions();
        QVERIFY(std::any_of(refreshed.cbegin(), refreshed.cend(), [](const QAction *candidate) {
            return candidate->iconText() == QLatin1String("New toolbar command") && candidate->objectName().startsWith(QLatin1String("custom_command_preset_"));
        }));
        dialog->close();
    }

    void savedCommandsAndAlternatesTakeStoredShortcuts()
    {
        QTemporaryDir backupDir;
        QVERIFY(backupDir.isValid());
        const auto config = KSharedConfig::openConfig();
        KConfigGroup group = config->group(QStringLiteral("CustomCommands"));
        const auto backup = KSharedConfig::openConfig(backupDir.filePath(QStringLiteral("settings")));
        KConfigGroup saved = backup->group(QStringLiteral("CustomCommands"));
        group.copyTo(&saved);
        const auto cleanup = qScopeGuard([&] {
            group.deleteGroup();
            saved.copyTo(&group);
            config->sync();
        });
        const auto presetId = [](const QString &name) {
            return QStringLiteral("custom_command_preset_") + QString::fromLatin1(QCryptographicHash::hash(name.toUtf8(), QCryptographicHash::Sha256).toHex());
        };
        QString error;
        const CustomCommands::Command command{QStringLiteral("Shortcut command"), QStringLiteral("/bin/echo"), {QStringLiteral("{dir}")}, {}};
        const CustomCommands::Command later{QStringLiteral("Later command"), QStringLiteral("/bin/echo"), {}, {}};
        QVERIFY2(CustomCommands::save(group, {command}, &error), qPrintable(error));
        QString xmlPath;
        {
            MainWindow probe;
            xmlPath = probe.localXMLFile();
        }
        const ShortcutFilesGuard guard(xmlPath);
        KConfigGroup stored = config->group(QLatin1String(ShortcutStorage::Group));
        // A primary shortcut and two alternates, more than KDE's editor shows.
        const QList<QKeySequence> keys{QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_F10),
                                       QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_F11),
                                       QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_F12)};
        stored.writeEntry(presetId(command.name), QStringLiteral("Ctrl+Alt+F10; Ctrl+Alt+F11; Ctrl+Alt+F12"));
        stored.writeEntry(presetId(later.name), QStringLiteral("Ctrl+Alt+F9"));
        stored.writeEntry("open_gallery", QStringLiteral("Space; Ctrl+Alt+F8"));
        config->sync();

        MainWindow window;
        window.openUrls({m_a, m_b});
        window.show();
        window.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&window));
        // A window action keeps its default shortcut and gets the stored alternate.
        QCOMPARE(window.actionCollection()->action(QStringLiteral("open_gallery"))->shortcuts(),
                 (QList<QKeySequence>{QKeySequence(Qt::Key_Space), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_F8)}));
        QPointer<QAction> action = window.actionCollection()->action(presetId(command.name));
        QVERIFY(action);
        QCOMPARE(action->text(), QStringLiteral("Saved Command: Shortcut command"));
        QCOMPARE(action->shortcuts(), keys);
        // The command is on no menu or toolbar, yet each of its shortcuts opens its preview.
        QVERIFY(!window.toolBar()->actions().contains(action));
        QSignalSpy triggered(action.data(), &QAction::triggered);
        for (const QKeySequence &key : keys) {
            QTest::keySequence(&window, key);
            QCOMPARE(triggered.count(), int(keys.indexOf(key)) + 1);
            auto *dialog = window.findChild<CustomCommandDialog *>(QStringLiteral("customCommandDialog"));
            QVERIFY(dialog);
            QCOMPARE(dialog->findChild<QLineEdit *>(QStringLiteral("commandProgram"))->text(), command.program);
            QVERIFY(!dialog->findChild<QPushButton *>(QStringLiteral("commandRun"))->isEnabled());
            QPointer<CustomCommandDialog> closed = dialog;
            dialog->close();
            QTRY_VERIFY(closed.isNull());
            window.activateWindow();
            QVERIFY(QTest::qWaitForWindowActive(&window));
        }

        // Saving it in KDE's editor keeps the alternates it doesn't show.
        Q_EMIT window.guiFactory()->shortcutsSaved();
        QCOMPARE(stored.readEntry(presetId(command.name), QString()), QStringLiteral("Ctrl+Alt+F10; Ctrl+Alt+F11; Ctrl+Alt+F12"));

        // As the commands dialog announces commands saved or deleted there.
        const auto announce = [&window] {
            window.actionCollection()->action(QStringLiteral("custom_command"))->trigger();
            QPointer<CustomCommandDialog> dialog = window.findChild<CustomCommandDialog *>(QStringLiteral("customCommandDialog"));
            QVERIFY(dialog);
            Q_EMIT dialog->presetsChanged();
            dialog->close();
            QTRY_VERIFY(dialog.isNull());
        };

        // A command saved while the window is open gets its stored shortcut.
        QVERIFY2(CustomCommands::save(group, {command, later}, &error), qPrintable(error));
        announce();
        QAction *laterAction = window.actionCollection()->action(presetId(later.name));
        QVERIFY(laterAction);
        QCOMPARE(laterAction->shortcuts(), QList<QKeySequence>{QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_F9)});

        // Deleting a command forgets its shortcuts.
        QVERIFY2(CustomCommands::save(group, {later}, &error), qPrintable(error));
        announce();
        QVERIFY(action.isNull());
        QVERIFY(!stored.hasKey(presetId(command.name)));
        QVERIFY(stored.hasKey(presetId(later.name)));
    }

    void kdeToolbarEditorAppliesSeparatorAndSpacer()
    {
        QTemporaryDir backupDir;
        QVERIFY(backupDir.isValid());
        const auto config = KSharedConfig::openConfig();
        KConfigGroup customization = config->group(QStringLiteral("ToolbarCustomization"));
        KConfigGroup paneCustomization = config->group(QStringLiteral("PaneToolbars"));
        const auto backup = KSharedConfig::openConfig(backupDir.filePath(QStringLiteral("settings")));
        KConfigGroup saved = backup->group(QStringLiteral("ToolbarCustomization"));
        KConfigGroup savedPanes = backup->group(QStringLiteral("PaneToolbars"));
        customization.copyTo(&saved);
        paneCustomization.copyTo(&savedPanes);
        customization.deleteGroup();
        paneCustomization.deleteGroup();
        QString xmlPath;
        QByteArray xmlBackup;
        bool xmlExisted = false;
        const auto cleanup = qScopeGuard([&] {
            customization.deleteGroup();
            saved.copyTo(&customization);
            paneCustomization.deleteGroup();
            savedPanes.copyTo(&paneCustomization);
            config->sync();
            if (!xmlPath.isEmpty()) {
                if (xmlExisted) {
                    QFile file(xmlPath);
                    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
                    QCOMPARE(file.write(xmlBackup), qint64(xmlBackup.size()));
                } else {
                    QVERIFY(!QFile::exists(xmlPath) || QFile::remove(xmlPath));
                }
            }
        });
        QStringList expected;
        {
            MainWindow window;
            window.resize(1200, 700);
            window.show();
            xmlPath = window.localXMLFile();
            QVERIFY(!xmlPath.isEmpty());
            QFile xml(xmlPath);
            xmlExisted = xml.exists();
            if (xmlExisted) {
                QVERIFY(xml.open(QIODevice::ReadOnly));
                xmlBackup = xml.readAll();
                xml.close();
            }
            QAction *home = window.actionCollection()->action(QStringLiteral("go_home"));
            QVERIFY(home);
            window.toolBar()->addAction(home);
            Pane *pane = window.activePane();
            QAction *paneZoom = pane->toolbarAction(QStringLiteral("paneZoomIn"));
            QAction *paneSort = pane->toolbarAction(QStringLiteral("paneSortingButton"));
            QVERIFY(paneZoom && paneSort);
            const QString paneId = paneZoom->property("toolbarTransferId").toString();
            QVERIFY(!paneId.isEmpty());
            pane->removeToolbarAction(paneZoom);
            paneZoom->setVisible(true);
            window.toolBar()->addAction(paneZoom);
            pane->removeToolbarAction(paneSort);
            paneSort->setVisible(true);
            window.toolBar()->addAction(paneSort);
            QTRY_VERIFY(customization.readEntry("Actions", QStringList()).contains(QStringLiteral("action:") + paneId));
            QMenu *goMenu = window.findChild<QMenu *>(QStringLiteral("go"));
            QAction *forward = window.actionCollection()->action(QStringLiteral("go_forward"));
            QVERIFY(goMenu && forward && goMenu->actions().contains(forward));
            goMenu->removeAction(forward);
            customization.group(QStringLiteral("Menus"))
                .group(QStringLiteral("go"))
                .writeEntry(QStringLiteral("HiddenActions"), QStringList{QStringLiteral("go_forward")});
            QAction *configure = window.actionCollection()->action(QStringLiteral("options_configure_toolbars"));
            QVERIFY(configure && configure->isEnabled());
            configure->trigger();
            QTRY_VERIFY(window.findChild<KEditToolBar *>());
            auto *editor = window.findChild<KEditToolBar *>();
            QVERIFY(editor);
            QVERIFY(!window.actionCollection()->action(QStringLiteral("customize_main_toolbar"))->isEnabled());
            const auto lists = editor->findChildren<QListWidget *>();
            QCOMPARE(lists.size(), 2);
            QListWidget *available = lists.at(0);
            QListWidget *current = lists.at(1);
            QVERIFY(!current->findItems(home->text().remove(QLatin1Char('&')), Qt::MatchExactly).isEmpty());
            QVERIFY(!current->findItems(paneZoom->text().remove(QLatin1Char('&')), Qt::MatchExactly).isEmpty());
            QVERIFY(!current->findItems(paneSort->text().remove(QLatin1Char('&')), Qt::MatchExactly).isEmpty());
            const auto renameItems = current->findItems(QStringLiteral("Rename"), Qt::MatchExactly);
            QCOMPARE(renameItems.size(), 1);
            current->setCurrentItem(renameItems.first());
            available->setCurrentRow(0);
            QToolButton *insert = nullptr;
            for (auto *button : editor->findChildren<QToolButton *>()) {
                if (button->icon().name() == QLatin1String("go-next")) {
                    insert = button;
                }
            }
            QVERIFY(insert && insert->isEnabled());
            insert->click();
            available->setCurrentRow(1);
            QVERIFY(insert->isEnabled());
            insert->click();
            current->setCurrentItem(current->findItems(paneSort->text().remove(QLatin1Char('&')), Qt::MatchExactly).first());
            QPushButton *changeText = nullptr;
            for (auto *button : editor->findChildren<QPushButton *>()) {
                if (button->icon().name() == QLatin1String("edit-rename")) {
                    changeText = button;
                }
            }
            QVERIFY(changeText && changeText->isEnabled());
            QTimer::singleShot(0, [] {
                auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                QVERIFY(dialog);
                auto *field = dialog->findChild<QLineEdit *>();
                auto *box = dialog->findChild<QDialogButtonBox *>();
                QVERIFY(field && box);
                field->setText(QStringLiteral("Custom sort"));
                box->button(QDialogButtonBox::Ok)->click();
            });
            changeText->click();
            auto *buttons = editor->findChild<QDialogButtonBox *>();
            QVERIFY(buttons);
            buttons->button(QDialogButtonBox::Apply)->click();
            auto *toolbar = window.toolBar();
            const auto actions = toolbar->actions();
            const int rename = actions.indexOf(window.actionCollection()->action(QStringLiteral("rename")));
            QVERIFY(rename >= 0);
            QVERIFY(rename + 2 < actions.size());
            QVERIFY(actions.at(rename + 1)->isSeparator());
            const int newTab = actions.indexOf(window.actionCollection()->action(QStringLiteral("new_tab")));
            QVERIFY(newTab >= 0 && newTab + 2 < actions.size());
            QVERIFY(actions.at(newTab + 2)->property("toolbarSpacer").toBool());
            QTRY_VERIFY(toolbar->actionGeometry(actions.at(newTab + 2)).width() > 16);
            QVERIFY(actions.contains(home));
            QVERIFY(actions.contains(paneZoom));
            QVERIFY(actions.contains(paneSort));
            QVERIFY(toolbar->widgetForAction(paneSort));
            QCOMPARE(qobject_cast<QToolButton *>(toolbar->widgetForAction(paneSort))->text(), QStringLiteral("Custom sort"));
            goMenu = window.findChild<QMenu *>(QStringLiteral("go"));
            QVERIFY(goMenu && !goMenu->actions().contains(forward));
            expected = customization.readEntry("Actions", QStringList());
            QVERIFY(expected.contains(QStringLiteral("action:") + paneId));
            QCOMPARE(expected.at(expected.indexOf(QStringLiteral("action:rename")) + 1), QStringLiteral("separator"));
            QCOMPARE(expected.at(expected.indexOf(QStringLiteral("action:new_tab")) + 2), QStringLiteral("spacer"));
            available->setCurrentRow(0);
            QVERIFY(insert->isEnabled());
            insert->click();
            buttons->button(QDialogButtonBox::Apply)->click();
            window.toolBar()->removeAction(home);
            QTRY_VERIFY(!customization.readEntry("Actions", QStringList()).contains(QStringLiteral("action:go_home")));
            window.toolBar()->addAction(home);
            QTRY_VERIFY(customization.readEntry("Actions", QStringList()).contains(QStringLiteral("action:go_home")));
            expected = customization.readEntry("Actions", QStringList());
            QCOMPARE(expected.count(QStringLiteral("separator")), 4);
            QVERIFY(window.toolBar()->actions().contains(paneZoom));
            editor->reject();
        }
        {
            MainWindow restored;
            const QStringList actual = customization.readEntry("Actions", QStringList());
            QCOMPARE(actual, expected);
            const auto actions = restored.toolBar()->actions();
            QVERIFY(actions.contains(restored.actionCollection()->action(QStringLiteral("go_home"))));
            QVERIFY(actions.contains(restored.activePane()->toolbarAction(QStringLiteral("paneZoomIn"))));
            QVERIFY(actions.contains(restored.activePane()->toolbarAction(QStringLiteral("paneSortingButton"))));
            auto *sortButton =
                qobject_cast<QToolButton *>(restored.toolBar()->widgetForAction(restored.activePane()->toolbarAction(QStringLiteral("paneSortingButton"))));
            QVERIFY(sortButton);
            QCOMPARE(sortButton->text(), QStringLiteral("Custom sort"));
            restored.configureToolbars();
            auto *editor = restored.findChild<KEditToolBar *>();
            QVERIFY(editor);
            const auto lists = editor->findChildren<QListWidget *>();
            QCOMPARE(lists.size(), 2);
            QVERIFY(!lists.at(1)->findItems(QStringLiteral("Home"), Qt::MatchExactly).isEmpty());
            QAction *paneZoom = restored.activePane()->toolbarAction(QStringLiteral("paneZoomIn"));
            QVERIFY(!lists.at(1)->findItems(paneZoom->text().remove(QLatin1Char('&')), Qt::MatchExactly).isEmpty());
            editor->reject();
            QTRY_VERIFY(!restored.findChild<KEditToolBar *>());
            QVERIFY(restored.actionCollection()->action(QStringLiteral("customize_main_toolbar"))->isEnabled());
            QCOMPARE(customization.readEntry("Actions", QStringList()), expected);
            restored.configureToolbars();
            editor = restored.findChild<KEditToolBar *>();
            QVERIFY(editor);
            const auto reopenedLists = editor->findChildren<QListWidget *>();
            QCOMPARE(reopenedLists.size(), 2);
            reopenedLists.at(0)->setCurrentRow(0);
            QToolButton *insert = nullptr;
            for (auto *button : editor->findChildren<QToolButton *>()) {
                if (button->icon().name() == QLatin1String("go-next")) {
                    insert = button;
                }
            }
            QVERIFY(insert && insert->isEnabled());
            insert->click();
            editor->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Apply)->click();
            editor->reject();
            QAction *reset = restored.actionCollection()->action(QStringLiteral("reset_toolbar_customization"));
            QVERIFY(reset);
            QTimer::singleShot(0, [] {
                auto *dialog = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
                QVERIFY(dialog);
                dialog->button(QMessageBox::Yes)->click();
            });
            reset->trigger();
            Pane *pane = restored.activePane();
            QAction *sort = pane->toolbarAction(QStringLiteral("paneSortingButton"));
            QVERIFY(sort && pane->toolBar()->actions().contains(sort));
            QVERIFY(pane->toolBar()->widgetForAction(sort));
        }
    }

    void shortcutsKdeSavedInTheUiRcMoveIntoTheConfiguration()
    {
        QString xmlPath;
        {
            MainWindow probe;
            xmlPath = probe.localXMLFile();
            // The window moves the shortcuts before it has loaded the file, so it finds the file itself.
            QCOMPARE(WindowShortcuts::uiRcPath(probe.componentName(), QStringLiteral("opus-file-managerui.rc")), xmlPath);
        }
        const ShortcutFilesGuard guard(xmlPath);
        KConfigGroup stored = KSharedConfig::openConfig()->group(QLatin1String(ShortcutStorage::Group));
        const QKeySequence f9(Qt::Key_F9);
        const QKeySequence ctrlKG(QKeyCombination(Qt::CTRL | Qt::Key_K), QKeyCombination(Qt::Key_G));
        {
            MainWindow old;
            // What KDE's Configure Keyboard Shortcuts saves, as an earlier Plasma Opus left it.
            old.actionCollection()->action(QStringLiteral("open_gallery"))->setShortcuts({f9, ctrlKG});
            old.actionCollection()->writeSettings();
        }
        QVERIFY2(guard.contents().contains(QStringLiteral("shortcut=")), qPrintable(guard.contents()));
        QVERIFY(!stored.hasKey("open_gallery"));

        MainWindow window;
        window.resize(1200, 700);
        window.show();
        QAction *gallery = window.actionCollection()->action(QStringLiteral("open_gallery"));
        QCOMPARE(gallery->shortcuts(), (QList<QKeySequence>{f9, ctrlKG}));
        QCOMPARE(stored.readEntry("open_gallery", QString()), QStringLiteral("F9; Ctrl+K, G"));
        QCOMPARE(stored.readEntry(ShortcutStorage::VersionKey, 0), ShortcutStorage::Version);
        // The file keeps the rest of the window's layout.
        QVERIFY2(!guard.contents().contains(QStringLiteral("shortcut=")), qPrintable(guard.contents()));
        QVERIFY2(guard.contents().contains(QStringLiteral("mainToolBar")), qPrintable(guard.contents()));
        // KDE's default stays the default.
        QCOMPARE(KActionCollection::defaultShortcuts(gallery), QList<QKeySequence>{QKeySequence(Qt::Key_Space)});

        // Configure Toolbars saves the layout the window loaded and rebuilds the menus, which gives every action
        // its defaults; neither brings the old shortcuts back or loses the stored ones.
        window.configureToolbars();
        auto *editor = window.findChild<KEditToolBar *>();
        QVERIFY(editor);
        const auto lists = editor->findChildren<QListWidget *>();
        QCOMPARE(lists.size(), 2);
        lists.at(0)->setCurrentRow(0);
        QToolButton *insert = nullptr;
        for (auto *button : editor->findChildren<QToolButton *>()) {
            if (button->icon().name() == QLatin1String("go-next")) {
                insert = button;
            }
        }
        QVERIFY(insert && insert->isEnabled());
        insert->click();
        editor->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Apply)->click();
        editor->reject();
        QVERIFY2(!guard.contents().contains(QStringLiteral("shortcut=")), qPrintable(guard.contents()));
        QCOMPARE(gallery->shortcuts(), (QList<QKeySequence>{f9, ctrlKG}));
        QCOMPARE(KActionCollection::defaultShortcuts(gallery), QList<QKeySequence>{QKeySequence(Qt::Key_Space)});

        // Later windows read the stored shortcuts, with nothing left in the file.
        MainWindow later;
        QCOMPARE(later.actionCollection()->action(QStringLiteral("open_gallery"))->shortcuts(), (QList<QKeySequence>{f9, ctrlKG}));
    }

    void shortcutsSavedInKdeEditorAreStoredForEveryWindow()
    {
        MainWindow window;
        const ShortcutFilesGuard guard(window.localXMLFile());
        MainWindow other;
        KConfigGroup stored = KSharedConfig::openConfig()->group(QLatin1String(ShortcutStorage::Group));
        QAction *gallery = window.actionCollection()->action(QStringLiteral("open_gallery"));
        QAction *search = window.actionCollection()->action(QStringLiteral("search_folder"));
        QAction *otherGallery = other.actionCollection()->action(QStringLiteral("open_gallery"));
        QVERIFY(gallery && search && otherGallery);
        const QList<QKeySequence> galleryDefaults = gallery->shortcuts();
        const QList<QKeySequence> searchDefaults = search->shortcuts();
        const QKeySequence f9(Qt::Key_F9);

        // KDE's editor saves into the ui.rc file, then announces it.
        gallery->setShortcuts({f9});
        search->setShortcuts({});
        window.actionCollection()->writeSettings();
        Q_EMIT window.guiFactory()->shortcutsSaved();
        QCOMPARE(stored.readEntry("open_gallery", QString()), QStringLiteral("F9"));
        QCOMPARE(stored.readEntry("search_folder", QString()), QStringLiteral("none"));
        QVERIFY2(!guard.contents().contains(QStringLiteral("shortcut=")), qPrintable(guard.contents()));
        // Other windows get them right away.
        QCOMPARE(otherGallery->shortcuts(), QList<QKeySequence>{f9});
        QVERIFY(other.actionCollection()->action(QStringLiteral("search_folder"))->shortcuts().isEmpty());
        // Only actions that differ from their defaults have entries.
        QVERIFY(!stored.hasKey("go_up"));
        QVERIFY(!stored.hasKey("switch_pane"));

        // KDE's editor leaves nothing in the file for an action given its defaults again, but the entry goes too.
        gallery->setShortcuts(galleryDefaults);
        search->setShortcuts(searchDefaults);
        window.actionCollection()->writeSettings();
        Q_EMIT window.guiFactory()->shortcutsSaved();
        QVERIFY(!stored.hasKey("open_gallery"));
        QVERIFY(!stored.hasKey("search_folder"));
        QCOMPARE(otherGallery->shortcuts(), galleryDefaults);
        QCOMPARE(other.actionCollection()->action(QStringLiteral("search_folder"))->shortcuts(), searchDefaults);
    }

    void storedShortcutsSurviveRebuildingTheMenus()
    {
        MainWindow window;
        const ShortcutFilesGuard guard(window.localXMLFile());
        QAction *gallery = window.actionCollection()->action(QStringLiteral("open_gallery"));
        const QKeySequence f9(Qt::Key_F9);
        gallery->setShortcuts({f9});
        Q_EMIT window.guiFactory()->shortcutsSaved();
        // As Configure Toolbars rebuilds them.
        window.guiFactory()->removeClient(&window);
        window.setXMLGUIBuildDocument(QDomDocument());
        window.guiFactory()->addClient(&window);
        QCOMPARE(gallery->shortcuts(), QList<QKeySequence>{f9});
        QCOMPARE(KActionCollection::defaultShortcuts(gallery), QList<QKeySequence>{QKeySequence(Qt::Key_Space)});
        // Actions without stored shortcuts get the defaults the window applies, such as Tab switching panes.
        QVERIFY(window.actionCollection()->action(QStringLiteral("switch_pane"))->shortcuts().contains(QKeySequence(Qt::Key_Tab)));
    }

    void keyboardMapListsSearchesAndExportsShortcuts()
    {
        MainWindow window;
        const ShortcutFilesGuard guard(window.localXMLFile());
        QAction *mapAction = window.actionCollection()->action(QStringLiteral("keyboard_map"));
        QAction *gallery = window.actionCollection()->action(QStringLiteral("open_gallery"));
        QVERIFY(mapAction && gallery);
        // Help › Keyboard Map… takes no key from another action.
        QVERIFY(mapAction->shortcuts().isEmpty());
        auto *help = qobject_cast<QMenu *>(window.guiFactory()->container(QStringLiteral("help"), &window));
        QVERIFY(help);
        QVERIFY(help->actions().contains(mapAction));

        mapAction->trigger();
        auto *dialog = window.findChild<KeyboardMapDialog *>();
        QVERIFY(dialog);
        QVERIFY(dialog->isVisible());
        mapAction->trigger();
        QCOMPARE(window.findChildren<KeyboardMapDialog *>().size(), 1);
        auto *search = dialog->findChild<QLineEdit *>(QStringLiteral("keyboardMapSearch"));
        auto *withShortcuts = dialog->findChild<QCheckBox *>(QStringLiteral("keyboardMapWithShortcuts"));
        auto *list = dialog->findChild<QTreeWidget *>(QStringLiteral("keyboardMapList"));
        QVERIFY(search && withShortcuts && list);
        const auto names = [dialog] {
            QStringList found;
            for (const KeyboardMap::Entry &entry : dialog->shownEntries()) {
                found.append(entry.name);
            }
            return found;
        };
        const auto entryNamed = [dialog](const QString &name) {
            for (const KeyboardMap::Entry &entry : dialog->shownEntries()) {
                if (entry.name == name) {
                    return entry;
                }
            }
            return KeyboardMap::Entry{};
        };

        // Window actions, the gallery's keys and the Yazi mode preset are listed, each with where it applies.
        QCOMPARE(list->topLevelItemCount(), dialog->shownEntries().size());
        QCOMPARE(list->headerItem()->text(2), QStringLiteral("Applies in"));
        QCOMPARE(entryNamed(QStringLiteral("search_folder")).shortcuts, QList<QKeySequence>{QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_F)});
        QCOMPARE(entryNamed(QStringLiteral("search_folder")).context, QStringLiteral("Window"));
        bool gallerySeen = false;
        bool yaziSeen = false;
        for (const KeyboardMap::Entry &entry : dialog->shownEntries()) {
            gallerySeen = gallerySeen || entry.context == QStringLiteral("Image gallery");
            yaziSeen = yaziSeen || entry.context == QStringLiteral("Yazi mode columns");
        }
        QVERIFY(gallerySeen && yaziSeen);
        // Pressing Shift twice is listed with the search it opens.
        QCOMPARE(entryNamed(QStringLiteral("double_shift_search")).keysText, QStringLiteral("Shift twice"));
        QCOMPARE(entryNamed(QStringLiteral("double_shift_search")).context, QStringLiteral("Window"));
        QVERIFY(!names().contains(QStringLiteral("keyboard_map")));
        // A search that only finds actions without shortcuts says they are hidden.
        auto *count = dialog->findChild<QLabel *>(QStringLiteral("keyboardMapCount"));
        QVERIFY(count);
        search->setText(QStringLiteral("keyboard_map"));
        QVERIFY(names().isEmpty());
        QVERIFY(count->text().contains(QStringLiteral("1 more without a shortcut is hidden")));
        search->clear();
        withShortcuts->setChecked(false);
        QVERIFY(names().contains(QStringLiteral("keyboard_map")));
        QVERIFY(!entryNamed(QStringLiteral("keyboard_map")).description.isEmpty());
        withShortcuts->setChecked(true);

        // Typing searches names; pressing keys finds what they run.
        search->setText(QStringLiteral("search current"));
        QVERIFY(names().contains(QStringLiteral("search_folder")));
        QVERIFY(!names().contains(QStringLiteral("open_gallery")));
        search->clear();
        dialog->setSearchKeys(QKeySequence(Qt::Key_Space));
        QVERIFY(names().contains(QStringLiteral("open_gallery")));
        QVERIFY(!names().contains(QStringLiteral("search_folder")));

        // Saved shortcuts show at once.
        gallery->setShortcuts({QKeySequence(Qt::Key_F9), QKeySequence(Qt::CTRL | Qt::Key_K, QKeyCombination(Qt::Key_G))});
        Q_EMIT window.guiFactory()->shortcutsSaved();
        QVERIFY(!names().contains(QStringLiteral("open_gallery")));
        dialog->setSearchKeys(QKeySequence(Qt::CTRL | Qt::Key_K));
        QCOMPARE(names(), QStringList{QStringLiteral("open_gallery")});

        // The listed actions export as a printable reference.
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("keys.html"));
        QVERIFY(dialog->exportTo(path).isEmpty());
        QFile file(path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        const QString html = QString::fromUtf8(file.readAll());
        QVERIFY(html.contains(QStringLiteral("Open Gallery")));
        QVERIFY(!html.contains(QStringLiteral("Search Current Folder")));
        dialog->setSearchKeys({});
        QVERIFY(dialog->exportTo(dir.filePath(QStringLiteral("keys.pdf"))).isEmpty());

        gallery->setShortcuts({QKeySequence(Qt::Key_Space)});
        Q_EMIT window.guiFactory()->shortcutsSaved();
    }

    void keyboardHintsLabelAndActivateTargets()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("alpha/inner")));
        for (const QString &name : {QStringLiteral("beta.txt"), QStringLiteral("gamma.txt")}) {
            QFile file(dir.filePath(name));
            QVERIFY(file.open(QIODevice::WriteOnly));
        }
        const QUrl folder = QUrl::fromLocalFile(dir.path());
        const auto restore = qScopeGuard([] {
            Settings::self()->setDefaults();
            Settings::self()->save();
        });

        MainWindow window;
        const ShortcutFilesGuard guard(window.localXMLFile());
        window.openUrls({folder, m_b});
        window.show();
        window.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&window));
        Pane *left = window.activePane();
        Pane *right = window.inactivePane();
        QTRY_COMPARE(left->view()->itemCount(), 3);
        QAbstractItemView *view = left->view()->currentView();
        view->setFocus();
        QTRY_COMPARE(QApplication::focusWidget(), view);
        if (!window.actionCollection()->action(QStringLiteral("show_places"))->isChecked()) {
            window.actionCollection()->action(QStringLiteral("show_places"))->trigger();
        }
        const auto visiblePlaces = [&window]() -> KFilePlacesView * {
            const QList<KFilePlacesView *> views = window.findChildren<KFilePlacesView *>();
            for (KFilePlacesView *places : views) {
                if (places->isVisible()) {
                    return places;
                }
            }
            return nullptr;
        };
        QTRY_VERIFY(visiblePlaces());
        KeyboardHintOverlay *overlay = window.keyboardHintOverlay();
        QVERIFY(overlay);

        // Go › Keyboard Hints has a key that no other action, text field or Yazi mode preset uses.
        QAction *hints = window.actionCollection()->action(QStringLiteral("keyboard_hints"));
        QVERIFY(hints);
        QCOMPARE(KLocalizedString::removeAcceleratorMarker(hints->text()), QStringLiteral("Keyboard Hints"));
        auto *go = qobject_cast<QMenu *>(window.guiFactory()->container(QStringLiteral("go"), &window));
        QVERIFY(go && go->actions().contains(hints));
        const QKeySequence key(Qt::CTRL | Qt::SHIFT | Qt::Key_J);
        QCOMPARE(KActionCollection::defaultShortcuts(hints), QList<QKeySequence>{key});
        QVERIFY(!KeyBindings::isTypingKey(key[0]));
        const WindowShortcuts::Bindings bindings = WindowShortcuts::bindings(&window);
        for (const KeyBindings::Binding &binding : bindings.bindings) {
            const bool usesKey = std::any_of(binding.shortcuts.cbegin(), binding.shortcuts.cend(), [&key](const QKeySequence &shortcut) {
                return shortcut.matches(key) != QKeySequence::NoMatch || key.matches(shortcut) != QKeySequence::NoMatch;
            });
            QVERIFY2(!usesKey || binding.name == QLatin1String("keyboard_hints"), qPrintable(binding.name));
        }
        for (YaziMode::Preset preset : {YaziMode::Preset::Standard, YaziMode::Preset::Vim}) {
            QVERIFY(YaziMode::shadowedShortcuts(YaziMode::presetKeymap(preset), QList<YaziMode::ActionShortcuts>{{hints->objectName(), hints->text(), {key}}})
                        .isEmpty());
        }

        const auto typeKeys = [](const QString &keys, Qt::KeyboardModifiers lastModifiers = Qt::NoModifier) {
            for (int i = 0; i < keys.size(); ++i) {
                QTest::keyClick(QApplication::focusWidget(), keys.at(i).toLatin1(), i + 1 == keys.size() ? lastModifiers : Qt::NoModifier);
            }
        };
        const auto labelOf = [overlay](KeyboardHints::Kind kind, const QString &name) {
            const QList<KeyboardHintOverlay::Target> targets = overlay->targets();
            for (int i = 0; i < targets.size(); ++i) {
                if (targets.at(i).kind == kind && targets.at(i).name == name) {
                    return overlay->labels().at(i);
                }
            }
            return QString();
        };
        const auto start = [overlay] {
            QTest::keyClick(QApplication::focusWidget(), Qt::Key_J, Qt::ControlModifier | Qt::ShiftModifier);
            QVERIFY(overlay->isActive());
            QCOMPARE(QApplication::focusWidget(), overlay);
        };
        const auto nameAt = [](const QAbstractItemView *itemView) {
            return itemView->currentIndex().data(Qt::DisplayRole).toString();
        };

        // The key labels the items, location segments and places; items get the shortest labels.
        start();
        const QList<KeyboardHintOverlay::Target> targets = overlay->targets();
        const QStringList labels = overlay->labels();
        QCOMPARE(labels.size(), targets.size());
        QList<KeyboardHints::Kind> kinds;
        for (const KeyboardHintOverlay::Target &target : targets) {
            kinds.append(target.kind);
        }
        QCOMPARE(kinds.count(KeyboardHints::Kind::Item), 3);
        QVERIFY(kinds.contains(KeyboardHints::Kind::Location));
        QVERIFY(kinds.contains(KeyboardHints::Kind::Place));
        // Tabs other than the current one, when the tab bar shows.
        QCOMPARE(kinds.count(KeyboardHints::Kind::Tab), left->tabBar()->isVisible() ? left->tabBar()->count() - 1 : 0);
        QCOMPARE(labels, KeyboardHints::assign(kinds, KeyboardHints::alphabet(KeyboardHints::Layout::Qwerty)));
        const QString beta = labelOf(KeyboardHints::Kind::Item, QStringLiteral("beta.txt"));
        QVERIFY(!beta.isEmpty());
        const int betaIndex = labels.indexOf(beta);
        QLabel *betaLabel = overlay->labelWidgets().at(betaIndex);
        QVERIFY(betaLabel->isVisible());
        QCOMPARE(betaLabel->accessibleName(), beta + QStringLiteral(": beta.txt"));
        QCOMPARE(betaLabel->accessibleDescription(), QStringLiteral("File or folder"));
        QVERIFY(window.rect().contains(betaLabel->geometry()));

        // Typing the first key of a longer label narrows the labels to those it begins; Backspace widens them again.
        const QString longest = labels.constLast();
        QVERIFY(longest.size() >= 2);
        typeKeys(longest.left(1));
        QVERIFY(overlay->isActive());
        QCOMPARE(overlay->typed(), longest.left(1));
        for (int i = 0; i < labels.size(); ++i) {
            QCOMPARE(overlay->labelWidgets().at(i)->isVisible(), labels.at(i).startsWith(longest.left(1)));
        }
        QTest::keyClick(overlay, Qt::Key_Backspace);
        QVERIFY(overlay->isActive());
        QCOMPARE(overlay->typed(), QString());
        QVERIFY(betaLabel->isVisible());

        // Typing a label selects its item and gives the focus back.
        typeKeys(beta);
        QVERIFY(!overlay->isActive());
        QCOMPARE(QApplication::focusWidget(), view);
        QCOMPARE(nameAt(view), QStringLiteral("beta.txt"));
        QCOMPARE(view->selectionModel()->selectedRows().size(), 1);
        QCOMPARE(left->url(), folder);

        // Escape, Backspace with nothing typed and a key that continues no label cancel without moving anything.
        start();
        QTest::keyClick(overlay, Qt::Key_Escape);
        QVERIFY(!overlay->isActive());
        QCOMPARE(QApplication::focusWidget(), view);
        start();
        QTest::keyClick(overlay, Qt::Key_Backspace);
        QVERIFY(!overlay->isActive());
        start();
        typeKeys(QStringLiteral("z"));
        QVERIFY(!overlay->isActive());
        QCOMPARE(nameAt(view), QStringLiteral("beta.txt"));
        // A window shortcut doesn't run while labels show; its key cancels them.
        const int tabs = left->tabBar()->count();
        start();
        QTest::keyClick(overlay, Qt::Key_T, Qt::ControlModifier);
        QVERIFY(!overlay->isActive());
        QCOMPARE(left->tabBar()->count(), tabs);
        // Pressing the key again hides them too.
        start();
        QTest::keyClick(overlay, Qt::Key_J, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(!overlay->isActive());

        // Shift on the last key does the other thing: it opens a folder.
        start();
        typeKeys(labelOf(KeyboardHints::Kind::Item, QStringLiteral("alpha")), Qt::ShiftModifier);
        QVERIFY(!overlay->isActive());
        QTRY_COMPARE(left->url(), QUrl::fromLocalFile(dir.filePath(QStringLiteral("alpha"))));

        // A visible location segment opens its folder, in either breadcrumb presentation.
        for (Pane::LocationPresentation presentation : {Pane::LocationPresentation::FullBreadcrumbs, Pane::LocationPresentation::AbbreviatedBreadcrumbs}) {
            left->setLocationPresentation(presentation);
            left->setUrl(QUrl::fromLocalFile(dir.filePath(QStringLiteral("alpha/inner"))));
            QTRY_COMPARE(left->url(), QUrl::fromLocalFile(dir.filePath(QStringLiteral("alpha/inner"))));
            // New segments show once the event loop runs.
            QTRY_VERIFY(!KeyboardHintOverlay::locationTargets(left).isEmpty());
            view = left->view()->currentView();
            view->setFocus();
            QTRY_COMPARE(QApplication::focusWidget(), view);
            start();
            const QString segment = labelOf(KeyboardHints::Kind::Location, QStringLiteral("alpha"));
            QVERIFY(!segment.isEmpty());
            const int segmentIndex = overlay->labels().indexOf(segment);
            QCOMPARE(overlay->labelWidgets().at(segmentIndex)->accessibleDescription(), QStringLiteral("Location segment"));
            typeKeys(segment);
            QVERIFY(!overlay->isActive());
            QTRY_COMPARE(left->url(), QUrl::fromLocalFile(dir.filePath(QStringLiteral("alpha"))));
        }
        left->setUrl(folder);
        QTRY_COMPARE(left->url(), folder);
        QTRY_COMPARE(left->view()->itemCount(), 3);
        view = left->view()->currentView();
        view->setFocus();
        QTRY_COMPARE(QApplication::focusWidget(), view);

        // The other tabs are labelled; typing one's label switches to it.
        const int tabsBefore = left->tabBar()->count();
        left->openInNewTab(m_b);
        QTRY_COMPARE(left->tabBar()->count(), tabsBefore + 1);
        QTRY_VERIFY(left->tabBar()->isVisible());
        QCOMPARE(left->url(), folder);
        start();
        QCOMPARE(std::count_if(overlay->targets().cbegin(),
                               overlay->targets().cend(),
                               [](const KeyboardHintOverlay::Target &target) {
                                   return target.kind == KeyboardHints::Kind::Tab;
                               }),
                 tabsBefore);
        const int newTab = left->tabBar()->count() - 1;
        const QString tabLabel = labelOf(KeyboardHints::Kind::Tab, KLocalizedString::removeAcceleratorMarker(left->tabBar()->tabText(newTab)));
        QVERIFY(!tabLabel.isEmpty());
        typeKeys(tabLabel);
        QVERIFY(!overlay->isActive());
        QCOMPARE(left->tabBar()->currentIndex(), newTab);
        QTRY_COMPARE(left->url(), m_b);
        left->closeCurrentTab();
        QTRY_COMPARE(left->url(), folder);
        QTRY_COMPARE(left->view()->itemCount(), 3);
        view = left->view()->currentView();
        view->setFocus();
        QTRY_COMPARE(QApplication::focusWidget(), view);

        // A place opens in the active pane.
        KFilePlacesView *places = visiblePlaces();
        QString placeName;
        QUrl placeUrl;
        for (int row = 0; row < places->model()->rowCount() && placeName.isEmpty(); ++row) {
            const QModelIndex index = places->model()->index(row, 0);
            const QUrl url = index.data(KFilePlacesModel::UrlRole).toUrl();
            if (!places->isRowHidden(row) && url == QUrl::fromLocalFile(QDir::homePath())) {
                placeName = index.data(Qt::DisplayRole).toString();
                placeUrl = url;
            }
        }
        QVERIFY(!placeName.isEmpty());
        start();
        const QString place = labelOf(KeyboardHints::Kind::Place, placeName);
        QVERIFY(!place.isEmpty());
        typeKeys(place);
        QVERIFY(!overlay->isActive());
        QTRY_COMPARE(left->url().adjusted(QUrl::StripTrailingSlash), placeUrl.adjusted(QUrl::StripTrailingSlash));
        QCOMPARE(window.activePane(), left);
        left->setUrl(folder);
        QTRY_COMPARE(left->view()->itemCount(), 3);
        view = left->view()->currentView();
        view->setFocus();
        QTRY_COMPARE(QApplication::focusWidget(), view);

        // Resizing the window, moving the focus or switching panes hides the labels.
        start();
        window.resize(window.size() + QSize(20, 20));
        QTRY_VERIFY(!overlay->isActive());
        view->setFocus();
        QTRY_COMPARE(QApplication::focusWidget(), view);
        start();
        right->view()->currentView()->setFocus();
        QVERIFY(!overlay->isActive());
        view->setFocus();
        QTRY_COMPARE(window.activePane(), left);
        start();
        window.actionCollection()->action(QStringLiteral("switch_pane"))->trigger();
        QCOMPARE(window.activePane(), right);
        QVERIFY(!overlay->isActive());
        window.actionCollection()->action(QStringLiteral("switch_pane"))->trigger();
        QCOMPARE(window.activePane(), left);
        view->setFocus();
        QTRY_COMPARE(QApplication::focusWidget(), view);

        // Scrolling the places hides them.
        start();
        places->verticalScrollBar()->setRange(0, 100);
        places->verticalScrollBar()->setValue(50);
        QVERIFY(!overlay->isActive());
        view->setFocus();
        QTRY_COMPARE(QApplication::focusWidget(), view);

        // Never while typing: the filter bar keeps the key's focus and gets no labels.
        left->showFilterBar();
        QTRY_VERIFY(qobject_cast<QLineEdit *>(QApplication::focusWidget()));
        hints->trigger();
        QVERIFY(!overlay->isActive());
        QVERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("typing")));
        left->clearFilter();
        view->setFocus();
        QTRY_COMPARE(QApplication::focusWidget(), view);

        // Labels in Open mode open items, and Shift selects; Vim-style keys don't take the label keys.
        Settings::setKeyboardHintItemAction(Settings::EnumKeyboardHintItemAction::Open);
        Settings::setVimNavigation(true);
        Settings::self()->save();
        window.applySettings();
        QTRY_VERIFY(left->vimNavigationActive());
        view->setFocus();
        QTRY_COMPARE(QApplication::focusWidget(), view);
        start();
        typeKeys(labelOf(KeyboardHints::Kind::Item, QStringLiteral("gamma.txt")), Qt::ShiftModifier);
        QVERIFY(!overlay->isActive());
        QCOMPARE(nameAt(view), QStringLiteral("gamma.txt"));
        QCOMPARE(left->url(), folder);
        start();
        typeKeys(labelOf(KeyboardHints::Kind::Item, QStringLiteral("alpha")));
        QVERIFY(!overlay->isActive());
        QTRY_COMPARE(left->url(), QUrl::fromLocalFile(dir.filePath(QStringLiteral("alpha"))));
    }

    void commandPaletteRunsListedActionsInTheActivePane()
    {
        QTemporaryDir backupDir;
        QVERIFY(backupDir.isValid());
        const auto config = KSharedConfig::openConfig();
        KConfigGroup commands = config->group(QStringLiteral("CustomCommands"));
        const auto backup = KSharedConfig::openConfig(backupDir.filePath(QStringLiteral("settings")));
        KConfigGroup saved = backup->group(QStringLiteral("CustomCommands"));
        commands.copyTo(&saved);
        KConfigGroup state = KSharedConfig::openStateConfig()->group(QString::fromLatin1(CommandPalette::HistoryGroup));
        const auto cleanup = qScopeGuard([&] {
            commands.deleteGroup();
            saved.copyTo(&commands);
            config->sync();
            CommandPalette::clearHistory(state);
            Settings::setCommandPaletteSavedCommands(true);
        });
        CommandPalette::clearHistory(state);
        QString error;
        const CustomCommands::Command command{QStringLiteral("Palette command"), QStringLiteral("/bin/echo"), {QStringLiteral("{dir}")}, {}};
        QVERIFY2(CustomCommands::save(commands, {command}, &error), qPrintable(error));

        MainWindow window;
        const ShortcutFilesGuard guard(window.localXMLFile());
        window.openUrls({m_b, m_a});
        window.show();
        window.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&window));
        Pane *left = window.activePane();
        Pane *right = window.inactivePane();
        QTRY_COMPARE(right->view()->itemCount(), 2);
        const auto action = [&window](const QString &name) {
            return window.actionCollection()->action(name);
        };
        const auto paletteActions = [](CommandPaletteBar *palette) {
            QList<QAction *> found;
            auto *list = palette->findChild<QTreeView *>(QStringLiteral("commandPaletteActions"));
            for (int row = 0; list && row < list->model()->rowCount(); ++row) {
                found.append(list->model()->index(row, 0).data(Qt::UserRole).value<QAction *>());
            }
            return found;
        };
        const auto choose = [](CommandPaletteBar *palette, QAction *chosen) {
            auto *list = palette->findChild<QTreeView *>(QStringLiteral("commandPaletteActions"));
            auto *search = palette->findChild<QLineEdit *>(QStringLiteral("commandPaletteSearch"));
            QVERIFY(list && search);
            const QModelIndexList rows = list->model()->match(list->model()->index(0, 0), Qt::UserRole, QVariant::fromValue(chosen), 1, Qt::MatchExactly);
            QCOMPARE(rows.size(), 1);
            list->setCurrentIndex(rows.constFirst());
            QTest::keyClick(search, Qt::Key_Return);
        };
        const auto deletePalette = [](QPointer<CommandPaletteBar> palette) {
            QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
            QTRY_VERIFY(palette.isNull());
        };

        // Help › Command Palette… has Dolphin's and Kate's key, which no other action, text field or Yazi mode preset uses.
        QAction *paletteAction = action(QString::fromLatin1(CommandPalette::ActionName));
        QVERIFY(paletteAction);
        QCOMPARE(KLocalizedString::removeAcceleratorMarker(paletteAction->text()), QStringLiteral("Command Palette…"));
        auto *help = qobject_cast<QMenu *>(window.guiFactory()->container(QStringLiteral("help"), &window));
        QVERIFY(help && help->actions().contains(paletteAction));
        const QKeySequence key(Qt::CTRL | Qt::ALT | Qt::Key_I);
        QCOMPARE(paletteAction->shortcuts(), QList<QKeySequence>{key});
        QCOMPARE(KActionCollection::defaultShortcuts(paletteAction), QList<QKeySequence>{key});
        QVERIFY(KActionCollection::isShortcutsConfigurable(paletteAction));
        QVERIFY(!KeyBindings::isTypingKey(key[0]));
        const WindowShortcuts::Bindings bindings = WindowShortcuts::bindings(&window);
        for (int index = 0; index < bindings.bindings.size(); ++index) {
            const KeyBindings::Binding &binding = bindings.bindings.at(index);
            const bool usesKey = std::any_of(binding.shortcuts.cbegin(), binding.shortcuts.cend(), [&key](const QKeySequence &shortcut) {
                return shortcut.matches(key) != QKeySequence::NoMatch || key.matches(shortcut) != QKeySequence::NoMatch;
            });
            QVERIFY2(!usesKey || binding.name == QLatin1String(CommandPalette::ActionName), qPrintable(binding.name));
        }
        for (YaziMode::Preset preset : {YaziMode::Preset::Standard, YaziMode::Preset::Vim}) {
            QVERIFY(YaziMode::shadowedShortcuts(YaziMode::presetKeymap(preset),
                                                QList<YaziMode::ActionShortcuts>{{paletteAction->objectName(), paletteAction->text(), {key}}})
                        .isEmpty());
        }
        bool mapped = false;
        for (const KeyboardMap::Entry &entry : KeyboardMapDialog::entries(&window)) {
            mapped = mapped || (entry.name == paletteAction->objectName() && entry.shortcuts.contains(key));
        }
        QVERIFY(mapped);

        // Enabled menu, toolbar and saved-command actions are listed by menu; disabled ones and the palette itself aren't.
        QAction *savedCommand = nullptr;
        for (QAction *candidate : window.actionCollection()->actions()) {
            if (CommandPalette::isSavedCommand(candidate->objectName()) && candidate->iconText() == command.name) {
                savedCommand = candidate;
            }
        }
        QVERIFY(savedCommand && savedCommand->isEnabled());
        QAction *copy = action(QStringLiteral("edit_copy"));
        QVERIFY(copy && !copy->isEnabled());
        const QList<KCommandBar::ActionGroup> groups = CommandPaletteBar::actionGroups(&window);
        QList<QAction *> listed;
        QString newFolderGroup;
        for (const KCommandBar::ActionGroup &group : groups) {
            listed += group.actions;
            if (group.actions.contains(action(QStringLiteral("new_folder")))) {
                newFolderGroup = group.name;
            }
        }
        QCOMPARE(newFolderGroup, QStringLiteral("File"));
        QVERIFY(listed.contains(action(QStringLiteral("edit_select_all"))));
        QVERIFY(listed.contains(savedCommand));
        QVERIFY(!listed.contains(copy));
        QVERIFY(!listed.contains(paletteAction));
        QCOMPARE(QSet<QAction *>(listed.cbegin(), listed.cend()).size(), listed.size());
        CommandPalette::Options withoutSaved;
        withoutSaved.savedCommands = false;
        for (const KCommandBar::ActionGroup &group : CommandPaletteBar::actionGroups(&window, withoutSaved)) {
            QVERIFY(!group.actions.contains(savedCommand));
        }

        // The palette runs an action in the pane that had the focus, which keeps it.
        action(QStringLiteral("focus_second_pane"))->trigger();
        QCOMPARE(window.activePane(), right);
        QTRY_COMPARE(QApplication::focusWidget(), right->view()->currentView());
        QTest::keyClick(right->view()->currentView(), Qt::Key_I, Qt::ControlModifier | Qt::AltModifier);
        QPointer<CommandPaletteBar> palette = window.findChild<CommandPaletteBar *>();
        QVERIFY(palette);
        QVERIFY(palette->isVisible());
        QCOMPARE(palette->accessibleName(), QStringLiteral("Command Palette"));
        QTRY_COMPARE(QApplication::focusWidget(), palette->findChild<QLineEdit *>(QStringLiteral("commandPaletteSearch")));
        QVERIFY(!paletteActions(palette).contains(copy));
        QVERIFY(paletteActions(palette).contains(savedCommand));
        // Opening it again keeps the one palette.
        paletteAction->trigger();
        QCOMPARE(window.findChildren<CommandPaletteBar *>().size(), 1);
        choose(palette, action(QStringLiteral("edit_select_all")));
        if (QTest::currentTestFailed()) {
            return;
        }
        QVERIFY(!palette || palette->isHidden());
        QCOMPARE(window.activePane(), right);
        QTRY_COMPARE(QApplication::focusWidget(), right->view()->currentView());
        QCOMPARE(right->view()->selectedItems().size(), 2);
        QVERIFY(left->view()->selectedItems().isEmpty());
        // The shortcuts are left as they were.
        QCOMPARE(paletteAction->shortcuts(), QList<QKeySequence>{key});
        QCOMPARE(action(QStringLiteral("edit_select_all"))->shortcuts(), KActionCollection::defaultShortcuts(action(QStringLiteral("edit_select_all"))));

        // KDE's command bar remembers the action once closed, and lists it first next time.
        deletePalette(palette);
        QCOMPARE(CommandPalette::history(state).value(0), action(QStringLiteral("edit_select_all"))->text());
        paletteAction->trigger();
        palette = window.findChild<CommandPaletteBar *>();
        QVERIFY(palette);
        QCOMPARE(paletteActions(palette).value(0), action(QStringLiteral("edit_select_all")));
        // Escape closes it and gives the focus back.
        QTest::keyClick(palette->findChild<QLineEdit *>(QStringLiteral("commandPaletteSearch")), Qt::Key_Escape);
        QVERIFY(!palette || palette->isHidden());
        QTRY_COMPARE(QApplication::focusWidget(), right->view()->currentView());
        QCOMPARE(window.activePane(), right);
        deletePalette(palette);

        // A saved command opens its preview rather than running.
        paletteAction->trigger();
        palette = window.findChild<CommandPaletteBar *>();
        QVERIFY(palette);
        choose(palette, savedCommand);
        if (QTest::currentTestFailed()) {
            return;
        }
        auto *dialog = window.findChild<CustomCommandDialog *>(QStringLiteral("customCommandDialog"));
        QVERIFY(dialog);
        QCOMPARE(dialog->findChild<QLineEdit *>(QStringLiteral("commandProgram"))->text(), command.program);
        QVERIFY(!dialog->findChild<QPushButton *>(QStringLiteral("commandRun"))->isEnabled());
        dialog->close();
        deletePalette(palette);
        QTRY_VERIFY(!window.findChild<CustomCommandDialog *>(QStringLiteral("customCommandDialog")));

        // The setting leaves saved commands out.
        Settings::setCommandPaletteSavedCommands(false);
        paletteAction->trigger();
        palette = window.findChild<CommandPaletteBar *>();
        QVERIFY(palette);
        QVERIFY(!paletteActions(palette).contains(savedCommand));
        QVERIFY(paletteActions(palette).contains(action(QStringLiteral("new_folder"))));
        QTest::keyClick(palette->findChild<QLineEdit *>(QStringLiteral("commandPaletteSearch")), Qt::Key_Escape);
        deletePalette(palette);
    }

    void defaultToolbarSpacingSurvivesCustomization()
    {
        QTemporaryDir backupDir;
        QVERIFY(backupDir.isValid());
        const auto config = KSharedConfig::openConfig();
        KConfigGroup customization = config->group(QStringLiteral("ToolbarCustomization"));
        const auto backup = KSharedConfig::openConfig(backupDir.filePath(QStringLiteral("settings")));
        KConfigGroup saved = backup->group(QStringLiteral("ToolbarCustomization"));
        customization.copyTo(&saved);
        const auto cleanup = qScopeGuard([&] {
            customization.deleteGroup();
            saved.copyTo(&customization);
            config->sync();
        });
        customization.deleteGroup();
        {
            MainWindow window;
            window.resize(1200, 700);
            window.show();
            QAction *home = window.actionCollection()->action(QStringLiteral("go_home"));
            QVERIFY(home);
            window.toolBar()->addAction(home);
            QTRY_VERIFY(customization.readEntry("Actions", QStringList()).contains(QStringLiteral("action:go_home")));
            const QStringList savedItems = customization.readEntry("Actions", QStringList());
            QVERIFY(savedItems.contains(QStringLiteral("spacer")));
            QCOMPARE(savedItems.count(QStringLiteral("separator")), 2);
            QVERIFY(savedItems.indexOf(QStringLiteral("spacer")) < savedItems.indexOf(QStringLiteral("action:search_folder")));
        }
        {
            MainWindow restored;
            restored.resize(1200, 700);
            restored.show();
            QToolBar *toolbar = restored.toolBar();
            QAction *spacer = nullptr;
            for (QAction *action : toolbar->actions()) {
                if (action->property("toolbarSpacer").toBool()) {
                    spacer = action;
                }
            }
            QVERIFY(spacer);
            QTRY_VERIFY(toolbar->actionGeometry(spacer).width() > 16);
            const auto actions = toolbar->actions();
            QCOMPARE(std::count_if(actions.cbegin(),
                                   actions.cend(),
                                   [](QAction *action) {
                                       return action->isSeparator();
                                   }),
                     2);
        }
    }

    void defaultPaneToolbarLayout()
    {
        QTemporaryDir backupDir;
        QVERIFY(backupDir.isValid());
        const auto config = KSharedConfig::openConfig();
        KConfigGroup panes = config->group(QStringLiteral("PaneToolbars"));
        const auto backup = KSharedConfig::openConfig(backupDir.filePath(QStringLiteral("settings")));
        KConfigGroup saved = backup->group(QStringLiteral("PaneToolbars"));
        panes.copyTo(&saved);
        const auto cleanup = qScopeGuard([&] {
            panes.deleteGroup();
            saved.copyTo(&panes);
            config->sync();
        });
        panes.deleteGroup();

        MainWindow window;
        for (Pane *pane : {window.activePane(), window.inactivePane()}) {
            const auto actions = pane->toolBar()->actions();
            QCOMPARE(actions.first()->objectName(), QStringLiteral("paneViewModeButton"));
            QCOMPARE(actions.at(1)->objectName(), QStringLiteral("paneBack"));
            QVERIFY(!pane->toolbarAction(QStringLiteral("paneFilter"))->isVisible());
            QVERIFY(!pane->toolbarAction(QStringLiteral("paneSearch"))->isVisible());
        }
    }

    void navigationActionsMoveBetweenPanesTabsAndPanels()
    {
        MainWindow window;
        const ShortcutFilesGuard guard(window.localXMLFile());
        window.openUrls({m_a, m_b});
        window.show();
        window.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&window));
        Pane *left = window.activePane();
        Pane *right = window.inactivePane();
        QTRY_COMPARE(left->view()->itemCount(), 2);
        const auto action = [&window](const QString &name) {
            return window.actionCollection()->action(name);
        };

        // Each new action is in a menu, and has no shortcut until the user gives it one.
        const QList<std::pair<QString, QStringList>> menus{
            {QStringLiteral("edit"), {QStringLiteral("deselect_all"), QStringLiteral("invert_selection"), QStringLiteral("clear_filter")}},
            {QStringLiteral("view"), {QStringLiteral("focus_preview_panel"), QStringLiteral("focus_terminal")}},
            {QStringLiteral("go"), {QStringLiteral("focus_first_pane"), QStringLiteral("focus_second_pane"), QStringLiteral("activate_last_tab")}},
        };
        for (const auto &[menuName, names] : menus) {
            auto *menu = qobject_cast<QMenu *>(window.guiFactory()->container(menuName, &window));
            QVERIFY(menu);
            for (const QString &name : names) {
                QAction *found = action(name);
                QVERIFY2(found && menu->actions().contains(found), qPrintable(name));
                QVERIFY2(found->shortcuts().isEmpty() && KActionCollection::defaultShortcuts(found).isEmpty(), qPrintable(name));
                QVERIFY(KActionCollection::isShortcutsConfigurable(found));
            }
        }
        QCOMPARE(action(QStringLiteral("next_tab"))->shortcuts(), QList<QKeySequence>{QKeySequence(Qt::CTRL | Qt::Key_Tab)});

        // Focus First Pane and Focus Second Pane make a pane active and focus its view.
        action(QStringLiteral("focus_second_pane"))->trigger();
        QCOMPARE(window.activePane(), right);
        QTRY_COMPARE(QApplication::focusWidget(), right->view()->currentView());
        action(QStringLiteral("focus_first_pane"))->trigger();
        QCOMPARE(window.activePane(), left);
        QTRY_COMPARE(QApplication::focusWidget(), left->view()->currentView());

        // Invert Selection and Deselect All change the active pane's selection.
        FolderView *view = left->view();
        const QUrl file = QUrl::fromLocalFile(m_root.filePath(QStringLiteral("a/file.txt")));
        view->selectUrls({file});
        QTRY_COMPARE(view->selectedItems().size(), 1);
        action(QStringLiteral("invert_selection"))->trigger();
        QCOMPARE(view->selectedItems().size(), 1);
        QCOMPARE(view->selectedItems().constFirst().name(), QStringLiteral("sub"));
        action(QStringLiteral("invert_selection"))->trigger();
        QCOMPARE(view->selectedItems().size(), 1);
        QCOMPARE(view->selectedItems().constFirst().url(), file);
        action(QStringLiteral("edit_select_all"))->trigger();
        QCOMPARE(view->selectedItems().size(), 2);
        action(QStringLiteral("deselect_all"))->trigger();
        QVERIFY(view->selectedItems().isEmpty());

        // Clear Filter shows every item again and closes the filter bar.
        auto *filterBar = left->findChild<FilterBar *>();
        QVERIFY(filterBar);
        action(QStringLiteral("show_filter_bar"))->trigger();
        filterBar->setFilter(NameFilter(QStringLiteral("file"), NameFilter::Scope::FilesAndFolders));
        QTRY_COMPARE(view->itemCount(), 1);
        action(QStringLiteral("clear_filter"))->trigger();
        QVERIFY(filterBar->isHidden());
        QVERIFY(filterBar->filter().isEmpty());
        QTRY_COMPARE(view->itemCount(), 2);
        QTRY_COMPARE(QApplication::focusWidget(), view->currentView());

        // Focus Preview Panel moves the focus into the shown preview, and back to the pane.
        auto *panel = window.findChild<PreviewPanel *>(QStringLiteral("previewPanel"));
        QVERIFY(panel);
        view->selectUrls({file});
        action(QStringLiteral("focus_preview_panel"))->trigger();
        QVERIFY(action(QStringLiteral("show_preview_panel"))->isChecked());
        auto *text = panel->findChild<QPlainTextEdit *>(QStringLiteral("previewText"));
        QVERIFY(text);
        QTRY_COMPARE(text->toPlainText(), QStringLiteral("hello"));
        action(QStringLiteral("focus_preview_panel"))->trigger();
        if (!panel->isAncestorOf(QApplication::focusWidget())) {
            // The first time, the text may not have been shown yet.
            action(QStringLiteral("focus_preview_panel"))->trigger();
        }
        QTRY_COMPARE(QApplication::focusWidget(), text);
        action(QStringLiteral("focus_preview_panel"))->trigger();
        QTRY_COMPARE(QApplication::focusWidget(), window.activePane()->view()->currentView());
        action(QStringLiteral("show_preview_panel"))->trigger();

        // Focus Embedded Terminal shows the terminal, and gives the focus to its session and back.
        auto *terminal = window.findChild<TerminalPanel *>();
        QVERIFY(terminal);
        action(QStringLiteral("focus_terminal"))->trigger();
        QVERIFY(action(QStringLiteral("show_embedded_terminal"))->isChecked());
        QVERIFY(terminal->isVisible());
        if (terminal->hasSessions()) {
            QTRY_VERIFY(terminal->hasKeyboardFocus());
            action(QStringLiteral("focus_terminal"))->trigger();
            QTRY_COMPARE(QApplication::focusWidget(), window.activePane()->view()->currentView());
        }
        action(QStringLiteral("show_embedded_terminal"))->setChecked(false);

        // With the extra keys on, none conflicts, none is a key text fields or lists keep and no Yazi mode preset takes one.
        Settings::setExtraNavigationShortcuts(true);
        Settings::self()->save();
        window.applySettings();
        QList<YaziMode::ActionShortcuts> extras;
        for (const NavigationShortcuts::Action &navigation : NavigationShortcuts::actions()) {
            QAction *found = action(navigation.name);
            QVERIFY2(found, qPrintable(navigation.name));
            if (!navigation.extra.isEmpty()) {
                QVERIFY2(found->shortcuts().contains(navigation.extra), qPrintable(navigation.name));
                QVERIFY(KActionCollection::defaultShortcuts(found).contains(navigation.extra));
                extras.append({navigation.name, found->text(), {navigation.extra}});
            }
        }
        QCOMPARE(extras.size(), 8);
        QCOMPARE(action(QStringLiteral("next_tab"))->shortcuts(),
                 (QList<QKeySequence>{QKeySequence(Qt::CTRL | Qt::Key_Tab), QKeySequence(Qt::CTRL | Qt::Key_PageDown)}));
        const WindowShortcuts::Bindings bindings = WindowShortcuts::bindings(&window);
        const QList<KeyBindings::Conflict> conflicts = KeyBindings::conflicts(bindings.bindings);
        QVERIFY2(conflicts.isEmpty(), qPrintable(conflicts.isEmpty() ? QString() : KeyBindings::conflictText(conflicts.constFirst(), bindings.bindings)));
        // Global shortcuts are KGlobalAccel's, which earlier tests may have changed.
        for (int index : KeyBindings::customized(bindings.bindings)) {
            QVERIFY2(bindings.bindings.at(index).global, qPrintable(bindings.bindings.at(index).name));
        }
        for (YaziMode::Preset preset : {YaziMode::Preset::Standard, YaziMode::Preset::Vim}) {
            QVERIFY(YaziMode::shadowedShortcuts(YaziMode::presetKeymap(preset), extras).isEmpty());
        }
        // Nothing is stored for actions with their defaults.
        Q_EMIT window.guiFactory()->shortcutsSaved();
        const KConfigGroup stored = KSharedConfig::openConfig()->group(QLatin1String(ShortcutStorage::Group));
        QVERIFY(!stored.hasKey("next_tab"));
        QVERIFY(!stored.hasKey("focus_terminal"));
        // The keys run their actions.
        QTest::keyClick(left->view()->currentView(), Qt::Key_2, Qt::AltModifier);
        QTRY_COMPARE(window.activePane(), right);
        QTest::keyClick(right->view()->currentView(), Qt::Key_1, Qt::AltModifier);
        QTRY_COMPARE(window.activePane(), left);
        // The keyboard map lists them.
        bool mapped = false;
        for (const KeyboardMap::Entry &entry : KeyboardMapDialog::entries(&window)) {
            mapped = mapped || (entry.name == QStringLiteral("focus_terminal") && entry.shortcuts.contains(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F4)));
        }
        QVERIFY(mapped);

        // Activate Last Tab.
        action(QStringLiteral("new_tab"))->trigger();
        action(QStringLiteral("new_tab"))->trigger();
        TabModel *tabs = window.activePane()->tabs();
        QVERIFY(tabs->count() >= 3);
        tabs->setCurrentIndex(0);
        action(QStringLiteral("activate_last_tab"))->trigger();
        QCOMPARE(tabs->currentIndex(), tabs->count() - 1);

        // Turning the setting off takes the extra keys away again.
        Settings::setExtraNavigationShortcuts(false);
        Settings::self()->save();
        window.applySettings();
        QVERIFY(action(QStringLiteral("focus_terminal"))->shortcuts().isEmpty());
        QVERIFY(KActionCollection::defaultShortcuts(action(QStringLiteral("focus_terminal"))).isEmpty());
        QCOMPARE(action(QStringLiteral("next_tab"))->shortcuts(), QList<QKeySequence>{QKeySequence(Qt::CTRL | Qt::Key_Tab)});

        // Rebuilding the menus, as Configure Toolbars does, keeps them once they are on again.
        Settings::setExtraNavigationShortcuts(true);
        Settings::self()->save();
        window.applySettings();
        window.guiFactory()->removeClient(&window);
        window.setXMLGUIBuildDocument(QDomDocument());
        window.guiFactory()->addClient(&window);
        QVERIFY(action(QStringLiteral("next_tab"))->shortcuts().contains(QKeySequence(Qt::CTRL | Qt::Key_PageDown)));
        QVERIFY(action(QStringLiteral("focus_terminal"))->shortcuts().contains(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F4)));
        Settings::setExtraNavigationShortcuts(false);
        Settings::self()->save();
    }

    void yaziModeIsDiscoverableWithoutShortcuts()
    {
        QTemporaryDir backupDir;
        QVERIFY(backupDir.isValid());
        const auto config = KSharedConfig::openConfig();
        KConfigGroup customization = config->group(QStringLiteral("ToolbarCustomization"));
        KConfigGroup paneCustomization = config->group(QStringLiteral("PaneToolbars"));
        const auto backup = KSharedConfig::openConfig(backupDir.filePath(QStringLiteral("settings")));
        KConfigGroup savedGroup = backup->group(QStringLiteral("ToolbarCustomization"));
        KConfigGroup savedPaneGroup = backup->group(QStringLiteral("PaneToolbars"));
        customization.copyTo(&savedGroup);
        paneCustomization.copyTo(&savedPaneGroup);
        const auto cleanup = qScopeGuard([&] {
            customization.deleteGroup();
            savedGroup.copyTo(&customization);
            paneCustomization.deleteGroup();
            savedPaneGroup.copyTo(&paneCustomization);
            config->sync();
        });
        customization.deleteGroup();
        paneCustomization.deleteGroup();

        MainWindow window;
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        // Yazi mode changes no existing bindings: none of its actions has a shortcut by default.
        QStringList yaziActions;
        for (QAction *action : window.actionCollection()->actions()) {
            const QString name = action->objectName();
            if (!name.startsWith(QLatin1String("toggle_yazi")) && !name.startsWith(QLatin1String("yazi_"))) {
                continue;
            }
            yaziActions.append(name);
            QVERIFY2(window.actionCollection()->defaultShortcuts(action).isEmpty(), qPrintable(name));
            QVERIFY2(action->shortcuts().isEmpty(), qPrintable(name));
        }
        for (const char *name : {"toggle_yazi_mode",
                                 "toggle_yazi_preview",
                                 "toggle_yazi_preview_information",
                                 "toggle_yazi_parent_column",
                                 "toggle_yazi_vim_keys",
                                 "yazi_focus_parent_column",
                                 "yazi_focus_current_column",
                                 "yazi_focus_preview",
                                 "yazi_toggle_selection",
                                 "yazi_visual_selection"}) {
            QVERIFY2(yaziActions.contains(QLatin1String(name)), name);
        }

        // The toggle is offered by the toolbar customizer, but is on no toolbar by default.
        QAction *mode = window.actionCollection()->action(QStringLiteral("toggle_yazi_mode"));
        QVERIFY(mode);
        QVERIFY(!window.toolBar()->actions().contains(mode));
        for (Pane *pane : {window.activePane(), window.inactivePane()}) {
            auto *paneAction = pane->toolBar()->findChild<QAction *>(QStringLiteral("paneYaziMode"));
            QVERIFY(paneAction);
            QVERIFY(pane->toolBar()->actions().contains(paneAction));
            QVERIFY(!paneAction->isVisible());
        }
        QAction *customize = window.actionCollection()->action(QStringLiteral("customize_main_toolbar"));
        QVERIFY(customize);
        customize->trigger();
        auto *actionList = window.findChild<QListWidget *>(QStringLiteral("toolbarActionList"));
        QVERIFY(actionList);
        bool listed = false;
        for (int row = 0; row < actionList->count(); ++row) {
            listed = listed || actionList->item(row)->data(Qt::UserRole + 1).toString() == QLatin1String("toggle_yazi_mode");
        }
        QVERIFY(listed);
        customize->trigger();

        // The status bar has a toggle for the active pane.
        auto *status = window.statusBar()->findChild<QToolButton *>(QStringLiteral("yaziModeStatusButton"));
        QVERIFY(status);
        QCOMPARE(status->defaultAction(), mode);
        QCOMPARE(status->accessibleName(), QStringLiteral("Yazi mode"));
        QVERIFY(!status->accessibleDescription().isEmpty());
        QVERIFY(status->isVisible());
        QVERIFY(!status->isChecked());
        status->click();
        QVERIFY(window.activePane()->yaziMode());
        QVERIFY(!window.inactivePane()->yaziMode());
        QVERIFY(status->isChecked());
        status->click();
        QVERIFY(!window.activePane()->yaziMode());
        QVERIFY(!status->isChecked());
    }

    void mainToolbarCanBeEditedByDragAndDrop()
    {
        QTemporaryDir backupDir;
        QVERIFY(backupDir.isValid());
        const auto config = KSharedConfig::openConfig();
        KConfigGroup customization = config->group(QStringLiteral("ToolbarCustomization"));
        KConfigGroup paneCustomization = config->group(QStringLiteral("PaneToolbars"));
        const auto backup = KSharedConfig::openConfig(backupDir.filePath(QStringLiteral("settings")));
        KConfigGroup savedGroup = backup->group(QStringLiteral("ToolbarCustomization"));
        KConfigGroup savedPaneGroup = backup->group(QStringLiteral("PaneToolbars"));
        customization.copyTo(&savedGroup);
        paneCustomization.copyTo(&savedPaneGroup);
        const auto cleanup = qScopeGuard([&] {
            customization.deleteGroup();
            savedGroup.copyTo(&customization);
            paneCustomization.deleteGroup();
            savedPaneGroup.copyTo(&paneCustomization);
            config->sync();
        });
        customization.deleteGroup();
        paneCustomization.deleteGroup();

        MainWindow window;
        window.show();
        auto *toolbar = window.toolBar();
        auto *customize = window.actionCollection()->action(QStringLiteral("customize_main_toolbar"));
        auto *home = window.actionCollection()->action(QStringLiteral("go_home"));
        QVERIFY(toolbar && customize && home);
        QVERIFY(!toolbar->actions().contains(home));
        customize->trigger();
        auto *dock = window.findChild<QDockWidget *>(QStringLiteral("toolbarActionsDock"));
        auto *actionList = window.findChild<QListWidget *>(QStringLiteral("toolbarActionList"));
        QVERIFY(dock && actionList);
        QTRY_VERIFY(dock->isVisible());

        const auto drop = [](QWidget *target, const QString &source, const QString &kind, const QString &name, const QPointF &position) {
            QMimeData mimeData;
            mimeData.setData(QStringLiteral("application/x-opus-toolbar-item"), (source + QLatin1Char('\n') + kind + QLatin1Char('\n') + name).toUtf8());
            QDragEnterEvent enter(position.toPoint(), Qt::MoveAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
            QApplication::sendEvent(target, &enter);
            if (!enter.isAccepted()) {
                return false;
            }
            QDragMoveEvent move(position.toPoint(), Qt::MoveAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
            QApplication::sendEvent(target, &move);
            if (!move.isAccepted()) {
                return false;
            }
            QDropEvent event(position, Qt::MoveAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
            QApplication::sendEvent(target, &event);
            return event.isAccepted();
        };
        QVERIFY(drop(toolbar, QStringLiteral("palette"), QStringLiteral("action"), QStringLiteral("open_terminal"), QPointF(0, 0)));
        QVERIFY(drop(toolbar, QStringLiteral("palette"), QStringLiteral("action"), QStringLiteral("go_home"), QPointF(0, 0)));
        QCOMPARE(toolbar->actions().first(), home);
        QVERIFY(drop(toolbar, QStringLiteral("palette"), QStringLiteral("separator"), {}, QPointF(toolbar->width(), toolbar->height() / 2.0)));
        QVERIFY(toolbar->actions().last()->isSeparator());
        QVERIFY(drop(toolbar, QStringLiteral("palette"), QStringLiteral("spacer"), {}, QPointF(toolbar->width(), toolbar->height() / 2.0)));
        QVERIFY(toolbar->actions().last()->property("toolbarSpacer").toBool());
        QVERIFY(
            drop(toolbar, QStringLiteral("toolbar"), QStringLiteral("action"), QStringLiteral("go_home"), QPointF(toolbar->width(), toolbar->height() / 2.0)));
        QCOMPARE(toolbar->actions().last(), home);
        QVERIFY(drop(actionList->viewport(), QStringLiteral("toolbar"), QStringLiteral("action"), QStringLiteral("go_home"), QPointF(0, 0)));
        QVERIFY(!toolbar->actions().contains(home));

        const QStringList savedItems = customization.readEntry("Actions", QStringList());
        QVERIFY(savedItems.contains(QStringLiteral("separator")));
        QVERIFY(savedItems.contains(QStringLiteral("spacer")));
        QVERIFY(!savedItems.contains(QStringLiteral("action:go_home")));

        Pane *pane = window.activePane();
        QToolBar *paneToolbar = pane->toolBar();
        QAction *goBack = window.actionCollection()->action(QStringLiteral("go_back"));
        QVERIFY(paneToolbar && goBack);
        QVERIFY(drop(toolbar, QStringLiteral("palette"), QStringLiteral("action"), QStringLiteral("go_back"), QPointF(0, 0)));
        QVERIFY(toolbar->actions().contains(goBack));
        QVERIFY(drop(paneToolbar, QStringLiteral("toolbar"), QStringLiteral("action"), QStringLiteral("go_back"), QPointF(0, 0)));
        QVERIFY(!toolbar->actions().contains(goBack));
        QAction *paneBack = pane->toolbarAction(QStringLiteral("paneBack"));
        QVERIFY(paneBack && paneToolbar->actions().contains(paneBack));
        const QString paneBackId = QStringLiteral("pane:%1:paneBack").arg(pane->toolbarConfigId());
        QVERIFY(drop(toolbar, QStringLiteral("pane"), QStringLiteral("action"), paneBackId, QPointF(toolbar->width(), toolbar->height() / 2.0)));
        QVERIFY(toolbar->actions().contains(goBack));
        QVERIFY(!paneToolbar->actions().contains(paneBack));
        QAction *paneZoomIn = pane->toolbarAction(QStringLiteral("paneZoomIn"));
        QVERIFY(paneZoomIn);
        const QString paneZoomInId = QStringLiteral("pane:%1:paneZoomIn").arg(pane->toolbarConfigId());
        QVERIFY(drop(toolbar, QStringLiteral("pane"), QStringLiteral("action"), paneZoomInId, QPointF(toolbar->width(), toolbar->height() / 2.0)));
        QVERIFY(toolbar->actions().contains(paneZoomIn));
        QVERIFY(!paneToolbar->actions().contains(paneZoomIn));

        auto *goMenu = window.findChild<QMenu *>(QStringLiteral("go"));
        QAction *goForward = window.actionCollection()->action(QStringLiteral("go_forward"));
        QVERIFY(goMenu && goForward && goMenu->actions().contains(goForward));
        const QString menuGoForwardId = QStringLiteral("menu:go:go_forward");
        QVERIFY(drop(toolbar, QStringLiteral("menu"), QStringLiteral("action"), menuGoForwardId, QPointF(0, 0)));
        QVERIFY(toolbar->actions().contains(goForward));
        QVERIFY(!goMenu->actions().contains(goForward));
        QVERIFY(drop(goMenu, QStringLiteral("toolbar"), QStringLiteral("action"), QStringLiteral("go_forward"), QPointF(0, 0)));
        QVERIFY(!toolbar->actions().contains(goForward));
        QVERIFY(goMenu->actions().contains(goForward));
        QVERIFY(drop(paneToolbar, QStringLiteral("menu"), QStringLiteral("action"), menuGoForwardId, QPointF(0, 0)));
        QAction *paneForward = pane->toolbarAction(QStringLiteral("paneForward"));
        QVERIFY(paneForward && paneToolbar->actions().contains(paneForward));
        QVERIFY(!goMenu->actions().contains(goForward));
        const QString paneForwardId = QStringLiteral("pane:%1:paneForward").arg(pane->toolbarConfigId());
        QVERIFY(drop(goMenu, QStringLiteral("pane"), QStringLiteral("action"), paneForwardId, QPointF(0, 0)));
        QVERIFY(goMenu->actions().contains(goForward));
        QVERIFY(!paneToolbar->actions().contains(paneForward));
        QVERIFY(drop(goMenu, QStringLiteral("menu"), QStringLiteral("action"), menuGoForwardId, QPointF(0, goMenu->height() + 10)));
        QCOMPARE(goMenu->actions().last(), goForward);
        const KConfigGroup goMenuCustomization = customization.group(QStringLiteral("Menus")).group(QStringLiteral("go"));
        const QStringList savedGoMenuOrder = goMenuCustomization.readEntry("Order", QStringList());
        QCOMPARE(savedGoMenuOrder.last(), QStringLiteral("go_forward"));

        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(true);
        Pane *otherPane = window.inactivePane();
        QVERIFY(otherPane && otherPane != pane);
        QAction *otherPaneBack = otherPane->toolbarAction(QStringLiteral("paneBack"));
        QVERIFY(otherPaneBack && otherPane->toolBar()->actions().contains(otherPaneBack));
        const QString otherPaneBackId = QStringLiteral("pane:%1:paneBack").arg(otherPane->toolbarConfigId());
        QVERIFY(drop(paneToolbar, QStringLiteral("pane"), QStringLiteral("action"), otherPaneBackId, QPointF(0, 0)));
        QVERIFY(!otherPane->toolBar()->actions().contains(otherPaneBack));
        QVERIFY(paneToolbar->actions().contains(paneBack));

        const QString customIconName = home->icon().name();
        const QString terminalIconName = window.actionCollection()->action(QStringLiteral("open_terminal"))->icon().name();
        QVERIFY(!customIconName.isEmpty() && QIcon::hasThemeIcon(customIconName));
        KConfigGroup terminalAppearance = customization.group(QStringLiteral("Entries")).group(QStringLiteral("open_terminal"));
        terminalAppearance.writeEntry("ButtonStyle", static_cast<int>(Qt::ToolButtonTextOnly));
        terminalAppearance.writeEntry("IconName", customIconName);
        customization.sync();
        MainWindow restored;
        auto *restoredToolbar = restored.toolBar();
        Pane *restoredPane = restored.activePane();
        auto *terminal = restored.actionCollection()->action(QStringLiteral("open_terminal"));
        QVERIFY(restoredToolbar && terminal && restoredToolbar->actions().contains(terminal));
        QAction *restoredPaneZoomIn = restoredPane->toolbarAction(QStringLiteral("paneZoomIn"));
        QVERIFY(restoredPaneZoomIn && restoredToolbar->actions().contains(restoredPaneZoomIn));
        QVERIFY(!restoredPane->toolBar()->actions().contains(restoredPaneZoomIn));
        auto *restoredGoMenu = restored.findChild<QMenu *>(QStringLiteral("go"));
        QAction *restoredGoForward = restored.actionCollection()->action(QStringLiteral("go_forward"));
        QVERIFY(restoredGoMenu && restoredGoForward && restoredGoMenu->actions().contains(restoredGoForward));
        QStringList restoredGoMenuOrder;
        for (QAction *action : restoredGoMenu->actions()) {
            if (!action->objectName().isEmpty()) {
                restoredGoMenuOrder.append(action->objectName());
            }
        }
        QCOMPARE(restoredGoMenuOrder, savedGoMenuOrder);
        auto *terminalButton = qobject_cast<QToolButton *>(restoredToolbar->widgetForAction(terminal));
        QVERIFY(terminalButton);
        QCOMPARE(terminalButton->toolButtonStyle(), Qt::ToolButtonTextOnly);
        QCOMPARE(terminalButton->icon().name(), customIconName);
        QCOMPARE(terminal->icon().name(), terminalIconName);
    }

    void paneToolbarEntriesCanCustomizeAppearance()
    {
        MainWindow window;
        Pane *pane = window.activePane();
        QVERIFY(pane);
        QAction *action = pane->toolbarAction(QStringLiteral("paneBack"));
        auto *button = qobject_cast<QToolButton *>(pane->toolBar()->widgetForAction(action));
        QAction *customize = window.actionCollection()->action(QStringLiteral("customize_main_toolbar"));
        QVERIFY(action && button && customize);

        KConfigGroup appearance =
            KSharedConfig::openConfig()->group(QStringLiteral("ToolbarCustomization")).group(QStringLiteral("Entries")).group(action->objectName());
        const bool hadStyle = appearance.hasKey(QStringLiteral("ButtonStyle"));
        const int savedStyle = appearance.readEntry(QStringLiteral("ButtonStyle"), -1);
        const auto cleanup = qScopeGuard([&] {
            if (hadStyle) {
                appearance.writeEntry(QStringLiteral("ButtonStyle"), savedStyle);
            } else {
                appearance.deleteEntry(QStringLiteral("ButtonStyle"));
            }
            appearance.sync();
            if (customize->isChecked()) {
                customize->trigger();
            }
            window.applySettings();
        });

        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        customize->trigger();
        QVERIFY(customize->isChecked());
        QTimer fallbackClose;
        fallbackClose.setSingleShot(true);
        connect(&fallbackClose, &QTimer::timeout, &window, [] {
            if (auto *popup = qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
                popup->close();
            }
        });
        fallbackClose.start(1000);
        QTimer::singleShot(0, [&] {
            auto *popup = qobject_cast<QMenu *>(QApplication::activePopupWidget());
            QVERIFY(popup);
            auto *styles = popup->findChild<QMenu *>(QStringLiteral("toolbarButtonStyleMenu"));
            QVERIFY(styles);
            QCOMPARE(styles->actions().size(), 4);
            styles->actions().at(1)->trigger();
            popup->close();
        });
        const QPoint localPosition(1, 1);
        QContextMenuEvent contextMenu(QContextMenuEvent::Mouse, localPosition, button->mapToGlobal(localPosition));
        QApplication::sendEvent(button, &contextMenu);
        fallbackClose.stop();

        QCOMPARE(appearance.readEntry(QStringLiteral("ButtonStyle"), -1), static_cast<int>(Qt::ToolButtonTextOnly));
        QCOMPARE(button->toolButtonStyle(), Qt::ToolButtonTextOnly);
    }

    void toolbarContextMenusOpenToolbarSettings()
    {
        MainWindow window;
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        SettingsDialog *dialog = nullptr;

        Pane *first = window.activePane();
        Pane *second = window.inactivePane();
        QVERIFY(first && second);
        const QList<QPair<QToolBar *, QAction *>> toolbars{
            {window.toolBar(), window.actionCollection()->action(QStringLiteral("customize_main_toolbar"))},
            {first->toolBar(), first->toolbarAction(QStringLiteral("paneCustomizeToolbar"))},
            {second->toolBar(), second->toolbarAction(QStringLiteral("paneCustomizeToolbar"))},
        };
        for (const auto &[toolbar, customize] : toolbars) {
            QVERIFY(toolbar && customize);
            bool checked = false;
            QTimer fallbackClose;
            fallbackClose.setSingleShot(true);
            connect(&fallbackClose, &QTimer::timeout, &window, [] {
                if (auto *popup = qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
                    popup->close();
                }
            });
            fallbackClose.start(1000);
            QTimer::singleShot(0, [&] {
                auto *popup = qobject_cast<QMenu *>(QApplication::activePopupWidget());
                QVERIFY(popup);
                QCOMPARE(popup->objectName(), QStringLiteral("toolbarContextMenu"));
                QVERIFY(popup->actions().contains(customize));
                QCOMPARE(popup->findChild<QAction *>(QStringLiteral("nativeToolbarOptions")) != nullptr, toolbar == window.toolBar());
                QAction *settings = popup->findChild<QAction *>(QStringLiteral("toolbarSettings"));
                QVERIFY(settings);
                checked = true;
                popup->close();
                settings->trigger();
            });
            const QPoint position(1, 1);
            QContextMenuEvent contextMenu(QContextMenuEvent::Mouse, position, toolbar->mapToGlobal(position));
            QApplication::sendEvent(toolbar, &contextMenu);
            QCoreApplication::processEvents();
            fallbackClose.stop();
            QVERIFY(checked);
            auto *currentDialog = qobject_cast<SettingsDialog *>(KConfigDialog::exists(QLatin1String(SettingsDialog::Name)));
            QVERIFY(currentDialog);
            if (dialog) {
                QCOMPARE(currentDialog, dialog);
            }
            dialog = currentDialog;
            auto *tabs = dialog->currentPage()->widget()->findChild<QTabWidget *>();
            QVERIFY(tabs);
            QVERIFY(tabs->currentWidget()->findChild<QCheckBox *>(QStringLiteral("kcfg_LockToolbars")));
        }

        const QList<QWidget *> buttons{
            window.toolBar()->widgetForAction(window.actionCollection()->action(QStringLiteral("new_tab"))),
            first->toolBar()->widgetForAction(first->toolbarAction(QStringLiteral("paneViewModeButton"))),
        };
        for (QWidget *button : buttons) {
            QVERIFY(button);
            bool buttonMenuShown = false;
            QTimer::singleShot(0, [&] {
                if (auto *popup = qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
                    buttonMenuShown = popup->objectName() == QLatin1String("toolbarContextMenu");
                    popup->close();
                }
            });
            const QPoint buttonPosition(1, 1);
            QContextMenuEvent buttonMenu(QContextMenuEvent::Mouse, buttonPosition, button->mapToGlobal(buttonPosition));
            QApplication::sendEvent(button, &buttonMenu);
            QCoreApplication::processEvents();
            QVERIFY(buttonMenuShown);
        }

        bool nativeMenuShown = false;
        QTimer fallbackClose;
        fallbackClose.setSingleShot(true);
        connect(&fallbackClose, &QTimer::timeout, &window, [] {
            if (auto *popup = qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
                popup->close();
            }
        });
        fallbackClose.start(1000);
        QTimer::singleShot(0, [&] {
            auto *popup = qobject_cast<QMenu *>(QApplication::activePopupWidget());
            QVERIFY(popup);
            QAction *native = popup->findChild<QAction *>(QStringLiteral("nativeToolbarOptions"));
            QVERIFY(native);
            QTimer::singleShot(0, [&] {
                if (auto *nativePopup = qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
                    nativeMenuShown = nativePopup->objectName() != QLatin1String("toolbarContextMenu") && !nativePopup->actions().isEmpty();
                    nativePopup->close();
                }
            });
            QTest::mouseClick(popup, Qt::LeftButton, Qt::NoModifier, popup->actionGeometry(native).center());
        });
        const QPoint mainPosition(1, 1);
        QContextMenuEvent mainMenu(QContextMenuEvent::Mouse, mainPosition, window.toolBar()->mapToGlobal(mainPosition));
        QApplication::sendEvent(window.toolBar(), &mainMenu);
        QCoreApplication::processEvents();
        fallbackClose.stop();
        QVERIFY(nativeMenuShown);

        const bool wasLocked = Settings::lockToolbars();
        const auto restoreLock = qScopeGuard([&] {
            Settings::setLockToolbars(wasLocked);
            window.applySettings();
        });
        Settings::setLockToolbars(true);
        window.applySettings();
        bool lockedMenuChecked = false;
        QTimer::singleShot(0, [&] {
            if (auto *popup = qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
                QAction *settings = popup->findChild<QAction *>(QStringLiteral("toolbarSettings"));
                lockedMenuChecked = settings && settings->isEnabled() && !first->toolbarAction(QStringLiteral("paneCustomizeToolbar"))->isEnabled();
                popup->close();
            }
        });
        QContextMenuEvent lockedMenu(QContextMenuEvent::Mouse, mainPosition, first->toolBar()->mapToGlobal(mainPosition));
        QApplication::sendEvent(first->toolBar(), &lockedMenu);
        QCoreApplication::processEvents();
        QVERIFY(lockedMenuChecked);
        dialog->close();
    }

    void toolbarCustomizationsCanResetToDefaults()
    {
        QTemporaryDir backupDir;
        QVERIFY(backupDir.isValid());
        const auto config = KSharedConfig::openConfig();
        KConfigGroup customization = config->group(QStringLiteral("ToolbarCustomization"));
        KConfigGroup paneCustomization = config->group(QStringLiteral("PaneToolbars"));
        const auto backup = KSharedConfig::openConfig(backupDir.filePath(QStringLiteral("settings")));
        KConfigGroup savedCustomization = backup->group(QStringLiteral("ToolbarCustomization"));
        KConfigGroup savedPaneCustomization = backup->group(QStringLiteral("PaneToolbars"));
        customization.copyTo(&savedCustomization);
        paneCustomization.copyTo(&savedPaneCustomization);
        const auto cleanup = qScopeGuard([&] {
            customization.deleteGroup();
            savedCustomization.copyTo(&customization);
            paneCustomization.deleteGroup();
            savedPaneCustomization.copyTo(&paneCustomization);
            config->sync();
        });
        customization.deleteGroup();
        paneCustomization.deleteGroup();

        MainWindow window;
        QToolBar *toolbar = window.toolBar();
        Pane *pane = window.activePane();
        auto *goMenu = window.findChild<QMenu *>(QStringLiteral("go"));
        QVERIFY(toolbar && pane && goMenu);
        const QList<QAction *> defaultToolbarActions = toolbar->actions();
        const QList<QAction *> defaultPaneActions = pane->toolBar()->actions();
        const QList<QAction *> defaultMenuActions = goMenu->actions();
        QVERIFY(defaultToolbarActions.size() > 1 && defaultPaneActions.size() > 1 && defaultMenuActions.size() > 1);

        toolbar->removeAction(defaultToolbarActions.first());
        toolbar->addAction(defaultToolbarActions.first());
        pane->removeToolbarAction(defaultPaneActions.first());
        goMenu->removeAction(defaultMenuActions.first());
        goMenu->addAction(defaultMenuActions.first());
        customization.writeEntry(QStringLiteral("Actions"), QStringList{QStringLiteral("action:go_home")});
        customization.group(QStringLiteral("Entries"))
            .group(QStringLiteral("open_terminal"))
            .writeEntry(QStringLiteral("ButtonStyle"), static_cast<int>(Qt::ToolButtonTextOnly));
        customization.group(QStringLiteral("Menus"))
            .group(QStringLiteral("go"))
            .writeEntry(QStringLiteral("HiddenActions"), QStringList{defaultMenuActions.first()->objectName()});

        QAction *reset = window.actionCollection()->action(QStringLiteral("reset_toolbar_customization"));
        QVERIFY(reset);
        QTimer::singleShot(0, [] {
            auto *dialog = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            QVERIFY(dialog);
            QVERIFY(dialog->button(QMessageBox::Yes));
            dialog->button(QMessageBox::Yes)->click();
        });
        reset->trigger();

        QCOMPARE(toolbar->actions(), defaultToolbarActions);
        QCOMPARE(pane->toolBar()->actions(), defaultPaneActions);
        QCOMPARE(goMenu->actions(), defaultMenuActions);
        QCoreApplication::processEvents();
        QVERIFY(customization.keyList().isEmpty());
        QVERIFY(customization.groupList().isEmpty());
        QVERIFY(paneCustomization.groupList().isEmpty());
    }

    void paneToolbarsControlTheirOwnPane()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("left")));
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("right/sub")));
        QFile result(root.filePath(QStringLiteral("right/sub/result.txt")));
        QVERIFY(result.open(QIODevice::WriteOnly));
        result.close();

        MainWindow window;
        const QUrl leftUrl = QUrl::fromLocalFile(root.filePath(QStringLiteral("left")));
        const QUrl rightUrl = QUrl::fromLocalFile(root.filePath(QStringLiteral("right")));
        window.openUrls({leftUrl, rightUrl});
        Pane *left = window.activePane();
        Pane *right = window.inactivePane();
        QVERIFY(left && right);
        QCOMPARE(left->url(), leftUrl);
        QCOMPARE(right->url(), rightUrl);
        auto *leftToolbar = left->findChild<QToolBar *>(QStringLiteral("paneToolbar"));
        auto *rightToolbar = right->findChild<QToolBar *>(QStringLiteral("paneToolbar"));
        QVERIFY(leftToolbar && rightToolbar && leftToolbar != rightToolbar);

        QAction *hiddenFiles = left->findChild<QAction *>(QStringLiteral("paneHiddenFiles"));
        QAction *previews = left->findChild<QAction *>(QStringLiteral("panePreviews"));
        QAction *zoomIn = left->findChild<QAction *>(QStringLiteral("paneZoomIn"));
        QVERIFY(hiddenFiles && previews && zoomIn);
        const bool rightHidden = right->view()->showHiddenFiles();
        const bool rightPreviews = right->view()->previewsEnabled();
        hiddenFiles->trigger();
        QCOMPARE(left->view()->showHiddenFiles(), hiddenFiles->isChecked());
        QCOMPARE(right->view()->showHiddenFiles(), rightHidden);
        previews->trigger();
        QCOMPARE(left->view()->previewsEnabled(), previews->isChecked());
        QCOMPARE(right->view()->previewsEnabled(), rightPreviews);

        const QSize initialIconSize = left->view()->currentView()->iconSize();
        zoomIn->trigger();
        QVERIFY(left->view()->currentView()->iconSize().width() > initialIconSize.width());
        const int rightSortColumn = right->view()->sortColumn();
        auto *sortButton = left->findChild<QToolButton *>(QStringLiteral("paneSortingButton"));
        QVERIFY(sortButton && sortButton->menu());
        QAction *sortBySize = nullptr;
        for (QAction *action : sortButton->menu()->actions()) {
            if (action->text() == QStringLiteral("Size")) {
                sortBySize = action;
                break;
            }
        }
        QVERIFY(sortBySize);
        sortBySize->trigger();
        QCOMPARE(left->view()->sortColumn(), KDirModel::Size);
        QCOMPARE(right->view()->sortColumn(), rightSortColumn);

        auto *viewButton = left->findChild<QToolButton *>(QStringLiteral("paneViewModeButton"));
        QVERIFY(viewButton && viewButton->menu());
        QAction *compactView = nullptr;
        for (QAction *action : viewButton->menu()->actions()) {
            if (action->text() == QStringLiteral("Compact")) {
                compactView = action;
                break;
            }
        }
        QVERIFY(compactView);
        compactView->trigger();
        QCOMPARE(left->view()->viewMode(), FolderView::ViewMode::Compact);
        QVERIFY(right->view()->viewMode() != FolderView::ViewMode::Compact);
        const auto viewActions = viewButton->menu()->actions();
        QVERIFY(std::any_of(viewActions.cbegin(), viewActions.cend(), [](QAction *action) {
            return action->text() == QStringLiteral("Cards");
        }));

        auto *navigationLayout = qobject_cast<QBoxLayout *>(left->navigationBar()->layout());
        QVERIFY(navigationLayout);
        window.show();
        window.resize(500, 400);
        QCoreApplication::processEvents();
        QTRY_COMPARE(navigationLayout->direction(), QBoxLayout::TopToBottom);

        QAction *search = right->findChild<QAction *>(QStringLiteral("paneSearch"));
        QVERIFY(search);
        search->trigger();
        auto *dialog = window.findChild<SearchWidget *>();
        QVERIFY(dialog);
        const QUrl resultUrl = QUrl::fromLocalFile(root.filePath(QStringLiteral("right/sub/result.txt")));
        dialog->revealRequested(resultUrl);
        QCOMPARE(right->url(), QUrl::fromLocalFile(root.filePath(QStringLiteral("right/sub"))));
        QCOMPARE(left->url(), leftUrl);
    }

    void paneToolbarConfigurationRestoresVisibilityAndOrder()
    {
        QTemporaryDir backupDir;
        QVERIFY(backupDir.isValid());
        const auto config = KSharedConfig::openConfig();
        KConfigGroup toolbars = config->group(QStringLiteral("PaneToolbars"));
        const auto backup = KSharedConfig::openConfig(backupDir.filePath(QStringLiteral("settings")));
        KConfigGroup saved = backup->group(QStringLiteral("PaneToolbars"));
        toolbars.copyTo(&saved);
        const auto cleanup = qScopeGuard([&] {
            toolbars.deleteGroup();
            saved.copyTo(&toolbars);
            config->sync();
        });
        toolbars.deleteGroup();

        MainWindow window;
        Pane *pane = window.activePane();
        auto *toolbar = pane->findChild<QToolBar *>(QStringLiteral("paneToolbar"));
        auto *customize = pane->findChild<QAction *>(QStringLiteral("paneCustomizeToolbar"));
        QVERIFY(toolbar && customize);
        QListWidget *entries = nullptr;
        bool dialogOpened = false;
        QTimer::singleShot(0, &window, [&] {
            auto *dialog = window.findChild<QDialog *>(QStringLiteral("paneToolbarConfigurationDialog"));
            if (!dialog) {
                return;
            }
            dialogOpened = true;
            entries = dialog->findChild<QListWidget *>(QStringLiteral("paneToolbarEntries"));
            if (!entries) {
                dialog->reject();
                return;
            }
            int backRow = -1;
            int hiddenRow = -1;
            for (int row = 0; row < entries->count(); ++row) {
                const QString name = entries->item(row)->data(Qt::UserRole).toString();
                backRow = name == QLatin1String("paneBack") ? row : backRow;
                hiddenRow = name == QLatin1String("paneHiddenFiles") ? row : hiddenRow;
            }
            if (backRow < 0 || hiddenRow < 0) {
                dialog->reject();
                return;
            }
            entries->item(hiddenRow)->setCheckState(Qt::Unchecked);
            entries->addItem(entries->takeItem(backRow));
            dialog->accept();
        });
        customize->trigger();
        QVERIFY(dialogOpened);
        QVERIFY(entries);
        QVERIFY(!pane->findChild<QAction *>(QStringLiteral("paneHiddenFiles"))->isVisible());
        QCOMPARE(toolbar->actions().first()->objectName(), QStringLiteral("paneViewModeButton"));

        MainWindow restored;
        Pane *restoredPane = restored.activePane();
        auto *restoredToolbar = restoredPane->findChild<QToolBar *>(QStringLiteral("paneToolbar"));
        QVERIFY(restoredToolbar);
        QVERIFY(!restoredPane->findChild<QAction *>(QStringLiteral("paneHiddenFiles"))->isVisible());
        QCOMPARE(restoredToolbar->actions().first()->objectName(), QStringLiteral("paneViewModeButton"));
    }

    void splitterSizesAreSavedPerLayout()
    {
        QTemporaryDir backupDir;
        QVERIFY(backupDir.isValid());
        const auto state = KSharedConfig::openStateConfig();
        KConfigGroup sizes = state->group(QStringLiteral("SplitterSizes"));
        const auto backup = KSharedConfig::openConfig(backupDir.filePath(QStringLiteral("state")));
        KConfigGroup savedSizes = backup->group(QStringLiteral("SplitterSizes"));
        sizes.copyTo(&savedSizes);
        const auto cleanup = qScopeGuard([&] {
            sizes.deleteGroup();
            savedSizes.copyTo(&sizes);
            state->sync();
        });
        sizes.deleteGroup();

        MainWindow window;
        window.resize(1600, 1000);
        window.openUrls({m_a, m_b});
        window.show();
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(true);
        window.setStackedLayout(false);
        auto *panes = window.findChild<QSplitter *>(QStringLiteral("paneSplitter"));
        auto *workspace = window.findChild<QSplitter *>(QStringLiteral("workspaceSplitter"));
        QVERIFY(panes && workspace);
        workspace->widget(1)->show();
        QCoreApplication::processEvents();
        panes->setSizes({1000, 500, 0});
        workspace->setSizes({700, 300});
        const QList<int> sidePanes = panes->sizes();
        const QList<int> sideWorkspace = workspace->sizes();
        auto *save = window.actionCollection()->action(QStringLiteral("save_splitter_sizes"));
        auto *reset = window.actionCollection()->action(QStringLiteral("reset_splitter_sizes"));
        QVERIFY(save && reset);
        save->trigger();

        window.setStackedLayout(true);
        panes->setSizes({300, 700, 0});
        workspace->setSizes({1000, 500});
        const QList<int> stackedPanes = panes->sizes();
        save->trigger();
        window.setStackedLayout(false);
        QTRY_VERIFY(qAbs(panes->sizes().at(0) - sidePanes.at(0)) < 5);
        QTRY_VERIFY(qAbs(workspace->sizes().at(1) - sideWorkspace.at(1)) < 5);
        window.setStackedLayout(true);
        QTRY_VERIFY(qAbs(panes->sizes().at(0) - stackedPanes.at(0)) < 5);
        reset->trigger();
        QVERIFY(!sizes.group(QStringLiteral("Stacked")).hasKey("FirstPane"));
        QVERIFY(sizes.group(QStringLiteral("SideBySide")).exists());
        window.setStackedLayout(false);
        QTRY_VERIFY(qAbs(panes->sizes().at(0) - sidePanes.at(0)) < 5);
        QVERIFY(panes->sizes().at(0) > 0 && panes->sizes().at(1) > 0);

        MainWindow restored;
        restored.resize(1600, 1000);
        restored.openUrls({m_a, m_b});
        restored.show();
        restored.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(true);
        restored.setStackedLayout(false);
        auto *restoredPanes = restored.findChild<QSplitter *>(QStringLiteral("paneSplitter"));
        QVERIFY(restoredPanes);
        QTRY_VERIFY(restoredPanes->sizes().at(0) > restoredPanes->sizes().at(1));

        const auto editorOpening = Settings::editorOpening();
        const auto restoreEditorSetting = qScopeGuard([editorOpening] {
            Settings::setEditorOpening(editorOpening);
        });
        Settings::setEditorOpening(Settings::EnumEditorOpening::Panel);
        window.activePane()->view()->clearSelection();
        window.actionCollection()->action(QStringLiteral("open_editor"))->trigger();
        auto *editor = window.findChild<EditorPanel *>(QStringLiteral("editorPanel"));
        QVERIFY(editor && editor->isVisible());
        auto *previewAction = window.actionCollection()->action(QStringLiteral("show_preview_panel"));
        previewAction->setChecked(true);
        QCoreApplication::processEvents();
        QCOMPARE(panes->count(), 4);
        panes->setSizes({600, 450, 300, 250});
        save->trigger();
        KConfigGroup side = sizes.group(QStringLiteral("SideBySide"));
        const int previewSize = side.readEntry("Preview", 0);
        const int editorSize = side.readEntry("Editor", 0);
        QVERIFY(previewSize > 0 && editorSize > 0);
        previewAction->setChecked(false);
        save->trigger();
        QCOMPARE(side.readEntry("Preview", 0), previewSize);
        previewAction->setChecked(true);
        QVERIFY(panes->sizes().at(2) > 0 && panes->sizes().at(3) > 0);

        auto *placeRight = editor->findChild<QAction *>(QStringLiteral("editorPlaceRight"));
        auto *placePanel = editor->findChild<QAction *>(QStringLiteral("editorPlacePanel"));
        QVERIFY(placeRight && placePanel);
        placeRight->trigger();
        auto *dock = window.findChild<QDockWidget *>(QStringLiteral("editorDock"));
        QVERIFY(dock && dock->isVisible());
        window.resizeDocks({dock}, {400}, Qt::Horizontal);
        QCoreApplication::processEvents();
        save->trigger();
        const int dockSize = side.readEntry("EditorDockRight", 0);
        QVERIFY(dockSize > 0);
        placePanel->trigger();
        placeRight->trigger();
        QCoreApplication::processEvents();
        QVERIFY(qAbs(dock->width() * 10000 / window.width() - dockSize) < 400);
        previewAction->setChecked(false);
        dock->hide();
        placePanel->trigger();
        side.writeEntry("EditorDockRight", 10000);
        placeRight->trigger();
        QCoreApplication::processEvents();
        QVERIFY(dock->width() * 10000 / window.width() <= 6500);

        side.writeEntry("FirstPane", 1);
        side.writeEntry("SecondPane", 9999);
        restored.setStackedLayout(true);
        restored.setStackedLayout(false);
        QTRY_VERIFY(restoredPanes->sizes().at(0) > 0);
        const QList<int> safeSizes = restoredPanes->sizes();
        const int paneTotal = safeSizes.at(0) + safeSizes.at(1);
        QVERIFY(safeSizes.at(0) * 100 / paneTotal >= 15);
        QVERIFY(safeSizes.at(1) * 100 / paneTotal >= 15);

        KConfigGroup sideConfig = sizes.group(QStringLiteral("SideBySide"));
        sideConfig.writeEntry("FirstPane", -100);
        sideConfig.writeEntry("SecondPane", 10000000);
        sideConfig.writeEntry("Preview", 10000000);
        window.resize(600, 400);
        window.setStackedLayout(true);
        window.setStackedLayout(false);
        QCoreApplication::processEvents();
        QVERIFY(panes->sizes().at(0) > 0 && panes->sizes().at(1) > 0);
    }

    void fileListMinimapFollowsFilteredViewAndScrollPosition()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        for (int i = 0; i < 80; ++i) {
            QFile file(
                root.filePath(QStringLiteral("%1-%2.txt").arg(i < 40 ? QStringLiteral("keep") : QStringLiteral("hide")).arg(i, 3, 10, QLatin1Char('0'))));
            QVERIFY(file.open(QIODevice::WriteOnly));
            file.close();
        }

        const bool previousVisible = Settings::showFileListMinimap();
        const int previousWidth = Settings::fileListMinimapWidth();
        const auto restoreSetting = qScopeGuard([previousVisible, previousWidth] {
            Settings::setShowFileListMinimap(previousVisible);
            Settings::setFileListMinimapWidth(previousWidth);
            Settings::self()->save();
        });
        Settings::setShowFileListMinimap(true);
        Settings::setFileListMinimapWidth(22);
        Settings::self()->save();

        FolderView view;
        view.resize(460, 180);
        view.setUrl(QUrl::fromLocalFile(root.path()));
        view.show();
        QTRY_COMPARE(view.itemCount(), 80);

        auto *minimap = view.findChild<QWidget *>(QStringLiteral("fileListMinimap"));
        QVERIFY(minimap);
        QVERIFY(minimap->isVisible());
        QCOMPARE(minimap->width(), 22);
        Settings::setFileListMinimapWidth(28);
        view.reloadListingSettings();
        QCOMPARE(minimap->width(), 28);
        Settings::setFileListMinimapWidth(100);
        view.reloadListingSettings();
        QCOMPARE(minimap->width(), 64);
        Settings::setFileListMinimapWidth(28);
        view.reloadListingSettings();
        auto *scrollBar = view.currentView()->verticalScrollBar();
        QTRY_VERIFY(scrollBar->maximum() > 0);
        QTest::mouseClick(minimap, Qt::LeftButton, Qt::NoModifier, QPoint(minimap->width() / 2, minimap->height() - 1));
        QCOMPARE(scrollBar->value(), scrollBar->maximum());

        scrollBar->setValue(0);
        QCoreApplication::processEvents();
        const QImage unfilteredMap = minimap->grab().toImage();
        view.setNameFilter(NameFilter(QStringLiteral("keep-")));
        QTRY_COMPARE(view.itemCount(), 40);
        QCoreApplication::processEvents();
        QVERIFY(unfilteredMap != minimap->grab().toImage());

        view.setNameFilter(NameFilter());
        QTRY_COMPARE(view.itemCount(), 80);
        view.setViewMode(FolderView::ViewMode::Icons);
        auto *iconScrollBar = view.currentView()->verticalScrollBar();
        QTRY_VERIFY(iconScrollBar->maximum() > 0);
        QTest::mouseClick(minimap, Qt::LeftButton, Qt::NoModifier, QPoint(minimap->width() / 2, minimap->height() - 1));
        QCOMPARE(iconScrollBar->value(), iconScrollBar->maximum());

        Settings::setShowFileListMinimap(false);
        view.reloadListingSettings();
        QVERIFY(minimap->isHidden());
    }

    void fileListMinimapCoversExpandedFoldersAndCompactColumns()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QVERIFY(QDir(root.path()).mkdir(QStringLiteral("sub")));
        for (int i = 0; i < 30; ++i) {
            QVERIFY(QDir(root.filePath(QStringLiteral("sub"))).mkdir(QStringLiteral("child-%1").arg(i, 3, 10, QLatin1Char('0'))));
        }
        for (int i = 0; i < 150; ++i) {
            QFile file(root.filePath(QStringLiteral("file-%1.txt").arg(i, 3, 10, QLatin1Char('0'))));
            QVERIFY(file.open(QIODevice::WriteOnly));
        }

        const bool previousVisible = Settings::showFileListMinimap();
        const auto restoreSetting = qScopeGuard([previousVisible] {
            Settings::setShowFileListMinimap(previousVisible);
            Settings::self()->save();
        });
        Settings::setShowFileListMinimap(true);
        Settings::self()->save();

        FolderView view;
        view.resize(460, 180);
        view.setUrl(QUrl::fromLocalFile(root.path()));
        view.show();
        QTRY_COMPARE(view.itemCount(), 151);
        auto *minimap = dynamic_cast<FileListMinimap *>(view.findChild<QWidget *>(QStringLiteral("fileListMinimap")));
        QVERIFY(minimap);
        QCOMPARE(minimap->trackedOrientation(), Qt::Vertical);
        QCOMPARE(minimap->itemCount(), 151);

        auto *details = qobject_cast<QTreeView *>(view.currentView());
        QVERIFY(details);
        QModelIndex sub;
        for (int row = 0; row < details->model()->rowCount(); ++row) {
            const QModelIndex index = details->model()->index(row, 0);
            if (index.data(Qt::DisplayRole).toString() == QLatin1String("sub")) {
                sub = index;
            }
        }
        QVERIFY(sub.isValid());
        details->expand(sub);
        QTRY_COMPARE(minimap->itemCount(), 181);
        const QImage expandedMap = minimap->grab().toImage();
        details->collapse(sub);
        QCOMPARE(minimap->itemCount(), 151);
        QVERIFY(expandedMap != minimap->grab().toImage());

        view.setViewMode(FolderView::ViewMode::Compact);
        QCOMPARE(minimap->trackedOrientation(), Qt::Horizontal);
        auto *columns = view.currentView()->horizontalScrollBar();
        QTRY_VERIFY(columns->maximum() > 0);
        QTest::mouseClick(minimap, Qt::LeftButton, Qt::NoModifier, QPoint(minimap->width() / 2, minimap->height() - 1));
        QCOMPARE(columns->value(), columns->maximum());
        QTest::mouseClick(minimap, Qt::LeftButton, Qt::NoModifier, QPoint(minimap->width() / 2, 0));
        QCOMPARE(columns->value(), 0);

        view.setViewMode(FolderView::ViewMode::Icons);
        QCOMPARE(minimap->trackedOrientation(), Qt::Vertical);
    }

    void informationPanelFollowsSelectionAndOptionalHover()
    {
        QTemporaryDir folder;
        QVERIFY(folder.isValid());
        const QUrl firstUrl = QUrl::fromLocalFile(folder.filePath(QStringLiteral("first.txt")));
        const QUrl secondUrl = QUrl::fromLocalFile(folder.filePath(QStringLiteral("second.txt")));
        for (const QUrl &url : {firstUrl, secondUrl}) {
            QFile file(url.toLocalFile());
            QVERIFY(file.open(QIODevice::WriteOnly));
            file.write("content");
        }

        const QStringList previousFields = Settings::informationFields();
        const bool previousHover = Settings::informationHover();
        const auto restore = qScopeGuard([previousFields, previousHover] {
            Settings::setInformationFields(previousFields);
            Settings::setInformationHover(previousHover);
            Settings::self()->save();
        });
        Settings::setInformationFields({QStringLiteral("localPathOrUrl"), QStringLiteral("comment"), QStringLiteral("size")});
        Settings::setInformationHover(false);

        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(folder.path())});
        window.show();
        auto *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 2);
        auto *action = window.actionCollection()->action(QStringLiteral("show_information_panel"));
        QVERIFY(action);
        auto *dock = window.findChild<QDockWidget *>(QStringLiteral("informationDock"));
        QVERIFY(dock);
        dock->hide();
        action->setChecked(false);
        action->trigger();
        QVERIFY(dock && dock->isVisible());
        view->selectUrls({firstUrl});
        auto *path = dock->findChild<QLabel *>(QStringLiteral("informationValue_localPathOrUrl"));
        auto *comment = dock->findChild<QLabel *>(QStringLiteral("informationValue_comment"));
        auto *panel = dock->findChild<InformationPanel *>(QStringLiteral("informationPanelContents"));
        QVERIFY(path && comment && panel);
        QTRY_COMPARE(path->text(), firstUrl.toDisplayString(QUrl::PreferLocalFile));
        QCOMPARE(comment->text(), QStringLiteral("Unavailable"));
        auto *form = qobject_cast<QFormLayout *>(panel->layout());
        QVERIFY(form);
        int pathRow = -1;
        int commentRow = -1;
        QFormLayout::ItemRole pathRole = QFormLayout::SpanningRole;
        QFormLayout::ItemRole commentRole = QFormLayout::SpanningRole;
        form->getWidgetPosition(path, &pathRow, &pathRole);
        form->getWidgetPosition(comment, &commentRow, &commentRole);
        QCOMPARE(pathRole, QFormLayout::FieldRole);
        QCOMPARE(commentRole, QFormLayout::FieldRole);
        QVERIFY(pathRow >= 0 && pathRow < commentRow);

        Settings::setInformationHover(true);
        window.applySettings();
        QModelIndex secondIndex;
        for (int row = 0; row < view->currentView()->model()->rowCount(); ++row) {
            const QModelIndex candidate = view->currentView()->model()->index(row, 0);
            if (candidate.data(Qt::DisplayRole).toString() == QLatin1String("second.txt")) {
                secondIndex = candidate;
                break;
            }
        }
        QVERIFY(secondIndex.isValid());
        QVERIFY(view->currentView()->hasMouseTracking());
        QVERIFY(view->currentView()->viewport()->hasMouseTracking());
        Q_EMIT view->currentView()->entered(secondIndex);
        QTRY_COMPARE(path->text(), secondUrl.toDisplayString(QUrl::PreferLocalFile));
        view->itemHoverCleared();
        QTRY_COMPARE(path->text(), firstUrl.toDisplayString(QUrl::PreferLocalFile));

        Settings::setInformationHover(true);
        window.applySettings();
        Q_EMIT view->currentView()->entered(secondIndex);
        QTRY_COMPARE(path->text(), secondUrl.toDisplayString(QUrl::PreferLocalFile));
        Settings::setInformationHover(false);
        window.applySettings();
        QCOMPARE(path->text(), firstUrl.toDisplayString(QUrl::PreferLocalFile));
    }

    void imageBackgroundSettingsApplyToOpenViews()
    {
        const auto previous = Settings::imageBackground();
        const auto restore = qScopeGuard([previous] {
            Settings::setImageBackground(previous);
        });
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("transparent.png"));
        QImage image(20, 20, QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        QVERIFY(image.save(path));
        MainWindow window;
        window.show();
        window.actionCollection()->action(QStringLiteral("show_preview_panel"))->setChecked(true);
        auto *preview = window.findChild<PreviewPanel *>(QStringLiteral("previewPanel"));
        QVERIFY(preview);
        preview->setItems({KFileItem(QUrl::fromLocalFile(path))});
        auto *label = preview->findChild<QLabel *>(QStringLiteral("previewImage"));
        QTRY_VERIFY(label && label->isVisible() && !label->pixmap().isNull());
        QPointer<GalleryWindow> gallery = new GalleryWindow({QUrl::fromLocalFile(path)}, {}, 0, &window);
        gallery->show();
        auto *view = gallery->findChild<QGraphicsView *>(QStringLiteral("galleryView"));
        QVERIFY(view);
        Settings::setImageBackground(Settings::EnumImageBackground::Dark);
        window.applySettings();
        QCOMPARE(view->backgroundBrush().color(), QColor(35, 35, 35));
        QCOMPARE(label->grab().toImage().pixelColor(label->width() / 2, label->height() / 2), QColor(35, 35, 35));
        gallery->close();
        QTRY_VERIFY(gallery.isNull());
    }

    void textFieldsAndListsKeepTheirKeys()
    {
        const bool tabSwitched = Settings::tabSwitchesPanes();
        const auto restore = qScopeGuard([tabSwitched] {
            Settings::setTabSwitchesPanes(tabSwitched);
        });
        Settings::setTabSwitchesPanes(true);
        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(m_root.filePath(QStringLiteral("a"))), QUrl::fromLocalFile(m_root.filePath(QStringLiteral("b")))});
        window.applySettings();
        window.show();
        window.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&window));
        auto *probe = new QAction(&window);
        probe->setShortcuts({QKeySequence(Qt::Key_Down), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_J)});
        window.addAction(probe);
        QSignalSpy probed(probe, &QAction::triggered);
        QSignalSpy switched(window.actionCollection()->action(QStringLiteral("switch_pane")), &QAction::triggered);

        // A file view keeps the arrow keys, but other shortcuts and Tab still work there.
        Pane *pane = window.activePane();
        QAbstractItemView *items = pane->view()->currentView();
        items->setFocus(Qt::OtherFocusReason);
        QTRY_VERIFY(items->hasFocus());
        QCOMPARE(ShortcutGuard::focusContext(items), std::optional(KeyBindings::Context::FileView));
        QTest::keyClick(items, Qt::Key_Down);
        QCOMPARE(probed.count(), 0);
        QTest::keyClick(items, Qt::Key_J, Qt::ControlModifier | Qt::AltModifier);
        QCOMPARE(probed.count(), 1);

        // A text field keeps its typing keys and Tab, but other shortcuts still work there.
        pane->editUrl();
        QWidget *field = QApplication::focusWidget();
        QVERIFY(field);
        QCOMPARE(ShortcutGuard::focusContext(field), std::optional(KeyBindings::Context::TextInput));
        QTest::keyClick(field, Qt::Key_Down);
        QCOMPARE(probed.count(), 1);
        QTest::keyClick(field, Qt::Key_J, Qt::ControlModifier | Qt::AltModifier);
        QCOMPARE(probed.count(), 2);
        QTest::keyClick(field, Qt::Key_Tab);
        QCOMPARE(switched.count(), 0);

        // So does the folder tree, while widgets that take no text or keys are left alone.
        auto *tree = window.findChild<FolderTree *>();
        QVERIFY(tree);
        auto *treeView = tree->findChild<QTreeView *>();
        QVERIFY(treeView);
        QCOMPARE(ShortcutGuard::focusContext(treeView), std::optional(KeyBindings::Context::FolderTree));
        QCOMPARE(ShortcutGuard::focusContext(window.findChild<QTabBar *>()), std::nullopt);
        QLineEdit readOnly;
        readOnly.setReadOnly(true);
        QCOMPARE(ShortcutGuard::focusContext(&readOnly), std::nullopt);
        QComboBox choices;
        QCOMPARE(ShortcutGuard::focusContext(&choices), std::nullopt);
    }

    void galleryKeysFollowTheirTable()
    {
        GalleryWindow gallery({}, {}, 0);
        const QList<KeyBindings::Binding> bindings = GalleryWindow::keyBindings();
        QCOMPARE(gallery.findChildren<QShortcut *>().size(), bindings.size());
        for (const KeyBindings::Binding &binding : bindings) {
            auto *shortcut = gallery.findChild<QShortcut *>(binding.name);
            QVERIFY2(shortcut, qPrintable(binding.name));
            QCOMPARE(shortcut->keys(), binding.shortcuts);
            QCOMPARE(binding.defaults, binding.shortcuts);
            QCOMPARE(binding.context, KeyBindings::Context::Viewer);
            QVERIFY(!binding.text.isEmpty());
        }
    }

    void opensCustomCommandForLocalFolder()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(root.path())});
        QAction *action = window.actionCollection()->action(QStringLiteral("custom_command"));
        QVERIFY(action && action->isEnabled());
        action->trigger();
        auto *dialog = window.findChild<CustomCommandDialog *>(QStringLiteral("customCommandDialog"));
        QVERIFY(dialog);
        dialog->close();
    }

    void opensMetadataEditorForOneLocalFile()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QFile file(root.filePath(QStringLiteral("file.txt")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("data");
        file.close();
        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(root.path())});
        window.show();
        FolderView *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 1);
        auto *action = window.actionCollection()->action(QStringLiteral("edit_metadata"));
        QVERIFY(action && !action->isEnabled());
        view->selectUrls({QUrl::fromLocalFile(file.fileName())});
        QTRY_VERIFY(action->isEnabled());
        action->trigger();
        auto *dialog = window.findChild<MetadataEditDialog *>(QStringLiteral("metadataEditDialog"));
        QVERIFY(dialog);
    }

    void metadataPlacementPreservesPendingChanges()
    {
        if (QStandardPaths::findExecutable(QStringLiteral("exiftool")).isEmpty()) {
            QSKIP("ExifTool is not installed");
        }
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const QString path = root.filePath(QStringLiteral("image.jpg"));
        QImage image(4, 4, QImage::Format_RGB32);
        image.fill(Qt::red);
        QVERIFY(image.save(path));
        Settings::setMetadataPlacement(Settings::EnumMetadataPlacement::Below);
        Settings::setEmbeddedPanelTitlesVisible(false);
        MainWindow window;
        window.resize(1400, 900);
        window.openUrls({QUrl::fromLocalFile(root.path()), m_b});
        window.show();
        FolderView *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 1);
        view->selectUrls({QUrl::fromLocalFile(path)});
        auto *action = window.actionCollection()->action(QStringLiteral("edit_metadata"));
        QTRY_VERIFY(action->isEnabled());
        action->trigger();
        auto *dialog = window.findChild<MetadataEditDialog *>(QStringLiteral("metadataEditDialog"));
        QVERIFY(dialog);
        auto *dock = qobject_cast<QDockWidget *>(dialog->parentWidget());
        QVERIFY(dock && dock->isVisible());
        QVERIFY(dock->titleBarWidget());
        QCOMPARE(window.dockWidgetArea(dock), Qt::BottomDockWidgetArea);
        QVERIFY(dialog->isBusy());
        QVERIFY(!dock->close());
        QVERIFY(dock->isVisible());
        QVERIFY(!window.close());
        QVERIFY(window.isVisible());
        auto *placeRight = dialog->findChild<QAction *>(QStringLiteral("metadataPlaceRight"));
        auto *placeBelow = dialog->findChild<QAction *>(QStringLiteral("metadataPlaceBelow"));
        QVERIFY(placeRight && placeBelow);
        placeRight->trigger();
        QCOMPARE(window.dockWidgetArea(dock), Qt::RightDockWidgetArea);
        placeBelow->trigger();
        QCOMPARE(window.dockWidgetArea(dock), Qt::BottomDockWidgetArea);
        auto *preview = dialog->findChild<QPushButton *>(QStringLiteral("metadataPreview"));
        auto *apply = dialog->findChild<QPushButton *>(QStringLiteral("metadataApply"));
        QTRY_VERIFY(!dialog->isBusy());
        QVERIFY2(preview->isEnabled(), qPrintable(dialog->findChild<QLabel *>(QStringLiteral("metadataStatus"))->text()));
        window.resizeDocks({dock}, {250}, Qt::Vertical);
        QCoreApplication::processEvents();
        const int belowHeight = dock->height();
        auto *artist = dialog->findChild<QLineEdit *>(QStringLiteral("metadataField_Artist"));
        QVERIFY(artist && apply);
        artist->setText(QStringLiteral("Pending artist"));
        preview->click();
        QVERIFY(apply->isEnabled());
        Settings::setEmbeddedPanelTitlesVisible(true);
        window.applySettings();
        QVERIFY(!dock->titleBarWidget());
        Settings::setEmbeddedPanelTitlesVisible(false);
        window.applySettings();
        QVERIFY(dock->titleBarWidget());
        QCOMPARE(artist->text(), QStringLiteral("Pending artist"));
        QVERIFY(apply->isEnabled());
        view->clearSelection();
        QCOMPARE(dialog->findChild<QTreeWidget *>(QStringLiteral("metadataChanges"))->topLevelItemCount(), 1);
        for (const auto &name : {QStringLiteral("metadataPlaceRight"), QStringLiteral("metadataPlaceWindow"), QStringLiteral("metadataPlaceBelow")}) {
            auto *place = dialog->findChild<QAction *>(name);
            QVERIFY(place);
            place->trigger();
            QCOMPARE(artist->text(), QStringLiteral("Pending artist"));
            QVERIFY(apply->isEnabled());
            QCOMPARE(dialog->findChild<QTreeWidget *>(QStringLiteral("metadataChanges"))->topLevelItemCount(), 1);
        }
        QCoreApplication::processEvents();
        QVERIFY(qAbs(dock->height() - belowHeight) < 70);
        QCOMPARE(Settings::metadataPlacement(), int(Settings::EnumMetadataPlacement::Below));
        QTimer::singleShot(0, [] {
            auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            QVERIFY(box);
            box->button(QMessageBox::No)->click();
        });
        apply->click();
        QVERIFY(apply->isEnabled());
        QTimer::singleShot(0, [] {
            auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            QVERIFY(box);
            box->button(QMessageBox::Yes)->click();
        });
        apply->click();
        QTRY_VERIFY_WITH_TIMEOUT(dialog->findChild<QLabel *>(QStringLiteral("metadataStatus"))->text().contains(QStringLiteral("updated and verified")), 15000);
        QCOMPARE(MetadataEditor::read(path).values.value(QStringLiteral("Artist")), QStringLiteral("Pending artist"));
        QPointer<MetadataEditDialog> alive(dialog);
        QVERIFY(dock->close());
        QTRY_VERIFY(alive.isNull());
    }

    void opensBatchRenameForMultipleLocalItems()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const QString firstPath = root.filePath(QStringLiteral("a.txt"));
        const QString secondPath = root.filePath(QStringLiteral("b.txt"));
        for (const QString &path : {firstPath, secondPath}) {
            QFile file(path);
            QVERIFY(file.open(QIODevice::WriteOnly));
        }
        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(root.path())});
        FolderView *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 2);
        view->selectAll();
        QTRY_COMPARE(view->selectedItems().size(), 2);
        QAction *rename = window.actionCollection()->action(QStringLiteral("rename"));
        QVERIFY(rename && rename->isEnabled());
        rename->trigger();
        auto *dialog = window.findChild<BatchRenameDialog *>(QStringLiteral("batchRenameDialog"));
        QVERIFY(dialog);
        dialog->close();
    }

    void selectsVisibleItemsByPathFilter()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        for (const QString &name : {QStringLiteral("keep.txt"), QStringLiteral("other.txt")}) {
            QFile file(root.filePath(name));
            QVERIFY(file.open(QIODevice::WriteOnly));
        }
        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(root.path())});
        FolderView *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 2);
        QTRY_VERIFY(!view->isLoading());
        auto *action = window.actionCollection()->action(QStringLiteral("select_by_path_filter"));
        QVERIFY(action);
        QTimer::singleShot(0, [&] {
            auto *dialog = window.findChild<QDialog *>(QStringLiteral("selectionPathDialog"));
            QVERIFY(dialog);
            dialog->findChild<QLineEdit *>(QStringLiteral("selectionPathFilter"))->setText(QStringLiteral("keep.txt"));
            auto *excluded = dialog->findChild<QPlainTextEdit *>(QStringLiteral("selectionExcludedFolders"));
            auto *buttons = dialog->findChild<QDialogButtonBox *>();
            excluded->setPlainText(QStringLiteral("relative"));
            buttons->button(QDialogButtonBox::Ok)->click();
            QVERIFY(!dialog->findChild<QLabel *>(QStringLiteral("selectionPathError"))->text().isEmpty());
            QVERIFY(view->selectedItems().isEmpty());
            excluded->clear();
            buttons->button(QDialogButtonBox::Ok)->click();
        });
        action->trigger();
        QCOMPARE(view->selectedItems().size(), 1);
        QCOMPARE(view->selectedItems().first().name(), QStringLiteral("keep.txt"));
    }

    void selectsItemsByQuery()
    {
        QTemporaryDir folder;
        QVERIFY(folder.isValid());
        for (const QString &name : {QStringLiteral("a.txt"), QStringLiteral("b.txt")}) {
            QFile file(folder.filePath(name));
            QVERIFY(file.open(QIODevice::WriteOnly));
        }
        QCOMPARE(LibraryRegistry::create(folder.path()), QString());
        const auto unregister = qScopeGuard([&folder] {
            LibraryRegistry::remove(folder.path());
        });
        QString error;
        const bool updated = TagLibrary::update(
            folder.path(),
            [](TagLibrary::Library &library, QString *error) {
                return library.setTag({QStringLiteral("t-draft"), QStringLiteral("Draft"), {}, {}, {}, {}}, error)
                    && library.assignTags({{QStringLiteral("a.txt"), false, 0, 0, 0, 0}}, {QStringLiteral("t-draft")}, {}, nullptr, error);
            },
            &error);
        QVERIFY2(updated, qPrintable(error));

        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(folder.path())});
        FolderView *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 2);
        QTRY_VERIFY(!view->isLoading());
        auto *select = window.actionCollection()->action(QStringLiteral("select_by_query"));
        auto *deselect = window.actionCollection()->action(QStringLiteral("deselect_by_query"));
        QVERIFY(select && deselect);
        QTimer::singleShot(0, [&] {
            auto *dialog = window.findChild<QDialog *>(QStringLiteral("selectionQueryDialog"));
            QVERIFY(dialog);
            auto *edit = dialog->findChild<QLineEdit *>(QStringLiteral("selectionQuery"));
            auto *status = dialog->findChild<QLabel *>(QStringLiteral("selectionQueryStatus"));
            auto *ok = dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok);
            edit->setText(QStringLiteral("tag:draft AND"));
            QVERIFY(!status->text().isEmpty());
            ok->click();
            QVERIFY(dialog->isVisible());
            edit->setText(QStringLiteral("content:x"));
            ok->click();
            QVERIFY(dialog->isVisible());
            QVERIFY(view->selectedItems().isEmpty());
            edit->setText(QStringLiteral("tag:draft"));
            QVERIFY(status->text().contains(QStringLiteral("tag:\"draft\"")));
            ok->click();
        });
        select->trigger();
        QCOMPARE(view->selectedItems().size(), 1);
        QCOMPARE(view->selectedItems().first().name(), QStringLiteral("a.txt"));

        // The last query is offered again.
        QTimer::singleShot(0, [&] {
            auto *dialog = window.findChild<QDialog *>(QStringLiteral("selectionQueryDialog"));
            QVERIFY(dialog);
            QCOMPARE(dialog->findChild<QLineEdit *>(QStringLiteral("selectionQuery"))->text(), QStringLiteral("tag:draft"));
            dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
        });
        deselect->trigger();
        QVERIFY(view->selectedItems().isEmpty());
    }

    void selectsUsingSharedPathFilter()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        for (const QString &name : {QStringLiteral("keep.txt"), QStringLiteral("other.txt")}) {
            QFile file(root.filePath(name));
            QVERIFY(file.open(QIODevice::WriteOnly));
        }
        auto config = KSharedConfig::openConfig();
        const auto group = config->group(QStringLiteral("SavedPathFilters"));
        QString error;
        QVERIFY(SavedPathFilters::save(group, {{QStringLiteral("Keep"), {QStringLiteral("keep.txt"), {}, {}, 0}}}, &error));
        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(root.path())});
        FolderView *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 2);
        QTRY_VERIFY(!view->isLoading());
        QTimer::singleShot(0, [&] {
            auto *dialog = window.findChild<QDialog *>(QStringLiteral("selectionPathDialog"));
            QVERIFY(dialog);
            QCOMPARE(dialog->findChild<QComboBox *>(QStringLiteral("selectionFilterPresets"))->currentText(), QStringLiteral("Keep"));
            dialog->findChild<QPushButton *>(QStringLiteral("selectionLoadFilterPreset"))->click();
            QCOMPARE(dialog->findChild<QLineEdit *>(QStringLiteral("selectionPathFilter"))->text(), QStringLiteral("keep.txt"));
            dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
        });
        window.actionCollection()->action(QStringLiteral("select_by_path_filter"))->trigger();
        QCOMPARE(view->selectedItems().size(), 1);
        QCOMPARE(view->selectedItems().first().name(), QStringLiteral("keep.txt"));
        config->deleteGroup(QStringLiteral("SavedPathFilters"));
        QVERIFY(config->sync());
    }

    void startupFoldersAndPaneDefaultsApply()
    {
        KConfigGroup startup = KSharedConfig::openConfig()->group(QStringLiteral("Startup"));
        startup.writeEntry("StartupMode", QStringLiteral("FixedFolders"));
        startup.writeEntry("LeftStartUrl", m_a.toString());
        startup.writeEntry("RightStartUrl", m_b.toString());
        KSharedConfig::openConfig()->group(QStringLiteral("Panes")).writeEntry("DualPaneAtStartup", QStringLiteral("Never"));
        KSharedConfig::openConfig()->group(QStringLiteral("Panes")).writeEntry("NewTabsAfterCurrent", true);
        KSharedConfig::openConfig()->group(QStringLiteral("Navigation")).writeEntry("FollowSymlinks", true);
        KSharedConfig::openConfig()->group(QStringLiteral("Views")).writeEntry("DefaultViewMode", QStringLiteral("Icons"));
        KSharedConfig::openConfig()->group(QStringLiteral("Views")).writeEntry("ShowHiddenFiles", true);
        KSharedConfig::openConfig()->sync();
        Settings::self()->load();

        MainWindow window;
        window.openUrls({});
        QCOMPARE(window.activePane()->url(), m_a);
        QCOMPARE(window.inactivePane()->url(), m_b);
        QVERIFY(!window.actionCollection()->action(QStringLiteral("dual_layout"))->isChecked());
        QCOMPARE(window.activePane()->view()->viewMode(), FolderView::ViewMode::Icons);
        QVERIFY(window.activePane()->view()->showHiddenFiles());

        Pane *pane = window.activePane();
        pane->openInNewTab(m_b, true);
        pane->tabs()->setCurrentIndex(0);
        pane->openInNewTab(m_c, true);
        QCOMPARE(pane->tabs()->urls(), (QList<QUrl>{m_a, m_c, m_b}));

        const QString link = m_root.filePath(QStringLiteral("link-to-b"));
        QVERIFY(QFile::link(m_b.toLocalFile(), link));
        pane->setUrl(QUrl::fromLocalFile(link));
        QCOMPARE(pane->url(), m_b);
    }

    void replacesWorkspacePaneSnapshot()
    {
        MainWindow window;
        window.openUrls({m_a, m_b});
        Pane *const pane = window.activePane();
        PaneSession snapshot;
        snapshot.tabs = {m_c, m_b, m_a};
        snapshot.currentTab = 1;
        snapshot.pinnedCount = 1;
        snapshot.lockedTabs = {2};
        snapshot.names = {QStringLiteral("Pinned"), {}, QStringLiteral("Locked")};
        snapshot.filterText = QStringLiteral("*.txt");
        snapshot.filterScope = 2;
        snapshot.locationPresentation = int(Pane::LocationPresentation::AbbreviatedBreadcrumbs);
        snapshot.viewMode = int(FolderView::ViewMode::Compact);
        pane->replaceState(snapshot);
        QCOMPARE(pane->saveState(), snapshot);
        QCOMPARE(pane->view()->viewMode(), FolderView::ViewMode::Compact);
        pane->setUrl(QUrl::fromLocalFile(m_root.path()));
        pane->setUrl(m_b);
        QVERIFY(pane->tabs()->canGoBack());
        PaneSession filtered = snapshot;
        filtered.filterText = QStringLiteral("*.md");
        pane->replaceState(filtered);
        QCOMPARE(pane->saveState(), filtered);
        QVERIFY2(pane->tabs()->canGoBack(), "replacing unchanged tabs keeps their history");
        const auto breadcrumbs = pane->findChildren<QToolButton *>(QStringLiteral("paneBreadcrumbSegment"));
        QVERIFY(std::any_of(breadcrumbs.cbegin(), breadcrumbs.cend(), [](const QToolButton *button) {
            return button->text() == QStringLiteral("m");
        }));
        pane->setUrl(QUrl::fromLocalFile(QStringLiteral("/tmp/__path/current")));
        const auto abbreviated = pane->findChildren<QToolButton *>(QStringLiteral("paneBreadcrumbSegment"));
        auto ancestor = std::find_if(abbreviated.cbegin(), abbreviated.cend(), [](const QToolButton *button) {
            return button->text() == QStringLiteral("__p");
        });
        QVERIFY(ancestor != abbreviated.cend());
        QCOMPARE((*ancestor)->toolTip(), QStringLiteral("__path"));
        (*ancestor)->click();
        QTRY_COMPARE(pane->url(), QUrl::fromLocalFile(QStringLiteral("/tmp/__path")));
    }

    void appliesConfiguredPaneLocationBars()
    {
        const auto reset = qScopeGuard([] {
            Settings::setLeftLocationPresentation(Settings::EnumLeftLocationPresentation::FullBreadcrumbs);
            Settings::setRightLocationPresentation(Settings::EnumRightLocationPresentation::FullBreadcrumbs);
        });
        Settings::setLeftLocationPresentation(Settings::EnumLeftLocationPresentation::EditablePath);
        Settings::setRightLocationPresentation(Settings::EnumRightLocationPresentation::AbbreviatedBreadcrumbs);
        MainWindow window;
        window.openUrls({m_a, m_b});
        Pane *const left = window.activePane();
        Pane *const right = window.inactivePane();
        QCOMPARE(left->locationPresentation(), Pane::LocationPresentation::EditablePath);
        QCOMPARE(right->locationPresentation(), Pane::LocationPresentation::AbbreviatedBreadcrumbs);

        // A choice made with the Path button survives unrelated settings changes.
        left->setLocationPresentation(Pane::LocationPresentation::FullBreadcrumbs);
        window.applySettings();
        QCOMPARE(left->locationPresentation(), Pane::LocationPresentation::FullBreadcrumbs);
        Settings::setRightLocationPresentation(Settings::EnumRightLocationPresentation::EditablePath);
        window.applySettings();
        QCOMPARE(left->locationPresentation(), Pane::LocationPresentation::FullBreadcrumbs);
        QCOMPARE(right->locationPresentation(), Pane::LocationPresentation::EditablePath);

        // A snapshot without a saved presentation uses the pane's configured default.
        PaneSession snapshot = right->saveState();
        snapshot.locationPresentation = -1;
        right->setLocationPresentation(Pane::LocationPresentation::FullBreadcrumbs);
        right->replaceState(snapshot);
        QCOMPARE(right->locationPresentation(), Pane::LocationPresentation::EditablePath);
    }

    void init()
    {
        // Start from a clean session so the first window does not inherit
        // tabs or layout from an earlier test.
        QFile::remove(m_root.filePath(QStringLiteral("a/.mfm.json")));
        QFile::remove(m_root.filePath(QStringLiteral("b/.mfm.json")));
        KSharedConfig::openStateConfig()->deleteGroup(QStringLiteral("Session"));
        KSharedConfig::openStateConfig()->group(QStringLiteral("Session")).writeEntry("Stacked", false);
        KSharedConfig::openStateConfig()->sync();
        KSharedConfig::openConfig()->deleteGroup(QStringLiteral("PopularFolders"));
        KSharedConfig::openConfig()->deleteGroup(QStringLiteral("SelectionFilters"));
        KSharedConfig::openConfig()->deleteGroup(QStringLiteral("View"));
        KSharedConfig::openConfig()->deleteGroup(QStringLiteral("PanelSettings"));
        KSharedConfig::openConfig()->deleteGroup(QStringLiteral("Archives"));
        KSharedConfig::openConfig()->deleteGroup(QStringLiteral("Tabs"));
        for (const QString &group : {QStringLiteral("Startup"),
                                     QStringLiteral("Panes"),
                                     QStringLiteral("Navigation"),
                                     QStringLiteral("Previews"),
                                     QStringLiteral("PreviewSettings"),
                                     QStringLiteral("Confirmations"),
                                     QStringLiteral("StatusBar"),
                                     QStringLiteral("Views"),
                                     QStringLiteral("Shortcuts"),
                                     QStringLiteral("Keyboard"),
                                     QStringLiteral("ContextMenu")}) {
            KSharedConfig::openConfig()->deleteGroup(group);
        }
        KSharedConfig::openConfig()->group(QStringLiteral("Confirmations")).writeEntry("ConfirmClosingMultipleTabs", false);
        KSharedConfig::openStateConfig()->deleteGroup(QStringLiteral("WindowLayout"));
        KConfigGroup files = KSharedConfig::openConfig()->group(QStringLiteral("Files"));
        files.deleteEntry("IgnoreList");
        files.deleteEntry("ExtensionColors");
        KSharedConfig::openConfig()->sync();
        Settings::self()->setDefaults();
        Settings::setConfirmClosingMultipleTabs(false);
        Settings::setLinkPanesAtStartup(false);
        Settings::setPreviewPlacement(Settings::EnumPreviewPlacement::Pane);
        Settings::setShowPreviews(true);
        Settings::setAlwaysShowTabBar(false);
        Settings::setNewTabsAfterCurrent(false);
        Settings::setInformationHover(false);
        Settings::setShowPlaces(true);
        Settings::setPanelTitlesVisible(true);
        Settings::setTerminalFollowActiveFolder(false);
        Settings::setFileActivationDestination(Settings::EnumFileActivationDestination::Application);
        Settings::self()->save();
    }

    void opensCommandLinePathsInBothPanes()
    {
        MainWindow window;
        window.openUrls({m_a, m_b});
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        QCOMPARE(window.activePane()->url(), m_a);
        QCOMPARE(window.inactivePane()->url(), m_b);
        QTRY_COMPARE(window.activePane()->view()->itemCount(), 2);
        QTRY_COMPARE(window.inactivePane()->view()->itemCount(), 0);
    }

    void previewFollowsSelectionAndRestoresLayout()
    {
        MainWindow window;
        window.openUrls({m_a, m_b});
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        Pane *left = window.activePane();
        Pane *right = window.inactivePane();
        QTRY_COMPARE(left->view()->itemCount(), 2);
        auto *toggle = window.actionCollection()->action(QStringLiteral("show_preview_panel"));
        auto *panel = window.findChild<PreviewPanel *>(QStringLiteral("previewPanel"));
        QVERIFY(toggle && panel);
        QVERIFY(toggle->shortcuts().contains(QKeySequence(Qt::Key_F3)));
        toggle->trigger();
        QVERIFY(panel->isVisible());
        QVERIFY(left->isVisible());
        QVERIFY(!right->isVisible());
        left->view()->selectUrls({QUrl::fromLocalFile(m_root.filePath(QStringLiteral("a/file.txt")))});
        auto *text = panel->findChild<QPlainTextEdit *>(QStringLiteral("previewText"));
        QVERIFY(text);
        QTRY_COMPARE(text->toPlainText(), QStringLiteral("hello"));
        left->view()->selectAll();
        QTRY_VERIFY(panel->findChild<QLabel *>(QStringLiteral("previewMessage"))->isVisible());
        window.actionCollection()->action(QStringLiteral("switch_pane"))->trigger();
        QCOMPARE(window.activePane(), right);
        QVERIFY(right->isVisible());
        QVERIFY(!left->isVisible());
        toggle->trigger();
        QVERIFY(!panel->isVisible());
        QVERIFY(left->isVisible());
        QVERIFY(right->isVisible());
    }

    void galleryUsesSortedItemsAndActivationSource()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QImage image(20, 10, QImage::Format_RGB32);
        image.fill(Qt::red);
        const QUrl alpha = QUrl::fromLocalFile(dir.filePath(QStringLiteral("alpha.png")));
        const QUrl beta = QUrl::fromLocalFile(dir.filePath(QStringLiteral("beta.png")));
        const QUrl gamma = QUrl::fromLocalFile(dir.filePath(QStringLiteral("gamma.png")));
        QVERIFY(image.save(alpha.toLocalFile()));
        QVERIFY(image.save(beta.toLocalFile()));
        QVERIFY(image.save(gamma.toLocalFile()));
        MainWindow window;
        window.openUrls({m_a, QUrl::fromLocalFile(dir.path())});
        window.show();
        Pane *other = window.inactivePane();
        FolderView *view = other->view();
        QTRY_COMPARE(view->itemCount(), 3);
        view->setSorting(KDirModel::Name, Qt::DescendingOrder);
        view->selectUrls({beta});
        Settings::setFileActivationDestination(Settings::EnumFileActivationDestination::Gallery);
        Q_EMIT view->itemActivated(view->selectedItems().first());
        QCOMPARE(window.activePane(), other);
        auto *gallery = window.findChild<GalleryWindow *>(QStringLiteral("galleryWindow"));
        QVERIFY(gallery);
        auto *filmstrip = gallery->findChild<QListWidget *>(QStringLiteral("galleryFilmstrip"));
        QVERIFY(filmstrip);
        QCOMPARE(filmstrip->count(), 3);
        QCOMPARE(filmstrip->item(0)->text(), QStringLiteral("gamma.png"));
        QCOMPARE(filmstrip->item(2)->text(), QStringLiteral("alpha.png"));
        QCOMPARE(filmstrip->currentRow(), 1);
        filmstrip->setCurrentRow(2);
        QCOMPARE(view->selectedItems().first().url(), alpha);
        gallery->close();
        QTRY_VERIFY(!window.findChild<GalleryWindow *>());
        view->selectUrls({alpha, gamma});
        window.actionCollection()->action(QStringLiteral("open_gallery"))->trigger();
        gallery = window.findChild<GalleryWindow *>(QStringLiteral("galleryWindow"));
        QVERIFY(gallery);
        filmstrip = gallery->findChild<QListWidget *>(QStringLiteral("galleryFilmstrip"));
        QCOMPARE(filmstrip->count(), 2);
        QCOMPARE(filmstrip->item(0)->text(), QStringLiteral("gamma.png"));
        QCOMPARE(filmstrip->item(1)->text(), QStringLiteral("alpha.png"));
        gallery->close();
    }

    void previewActivationPreservesFolderNavigation()
    {
        MainWindow window;
        window.openUrls({m_a, m_b});
        window.show();
        FolderView *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 2);
        const QUrl text = QUrl::fromLocalFile(m_root.filePath(QStringLiteral("a/file.txt")));
        const QUrl folder = QUrl::fromLocalFile(m_root.filePath(QStringLiteral("a/sub")));
        Settings::setFileActivationDestination(Settings::EnumFileActivationDestination::Preview);
        view->selectUrls({text});
        Q_EMIT view->itemActivated(view->selectedItems().first());
        auto *panel = window.findChild<PreviewPanel *>(QStringLiteral("previewPanel"));
        QVERIFY(panel->isVisible());
        auto *content = panel->findChild<QPlainTextEdit *>(QStringLiteral("previewText"));
        QTRY_COMPARE(content->toPlainText(), QStringLiteral("hello"));
        view->selectUrls({folder});
        Q_EMIT view->itemActivated(view->selectedItems().first());
        QTRY_COMPARE(window.activePane()->url(), folder);
    }

    void previewStopsPlaybackOnSelectionChange()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile audio(dir.filePath(QStringLiteral("audio.wav")));
        QVERIFY(audio.open(QIODevice::WriteOnly));
        const QByteArray header = QByteArray::fromHex("52494646641f000057415645666d74201000000001000100401f0000401f00000100080064617461401f0000");
        QCOMPARE(audio.write(header), header.size());
        QCOMPARE(audio.write(QByteArray(8000, char(0x80))), 8000);
        audio.close();
        QFile text(dir.filePath(QStringLiteral("next.txt")));
        QVERIFY(text.open(QIODevice::WriteOnly));
        QCOMPARE(text.write("next"), 4);
        text.close();
        PreviewPanel panel;
        panel.show();
        panel.setItems({KFileItem(QUrl::fromLocalFile(audio.fileName()))});
        QPointer<QMediaPlayer> player = panel.findChild<QMediaPlayer *>(QStringLiteral("mediaPlayer"));
        QVERIFY(player);
        QCOMPARE(player->playbackState(), QMediaPlayer::StoppedState);
        panel.setItems({KFileItem(QUrl::fromLocalFile(text.fileName()))});
        QVERIFY(player.isNull());
        auto *preview = panel.findChild<QPlainTextEdit *>(QStringLiteral("previewText"));
        QTRY_COMPARE(preview->toPlainText(), QStringLiteral("next"));
    }

    void previewMarkupDoesNotLoadExternalResources()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString imagePath = dir.filePath(QStringLiteral("private.png"));
        QImage image(4, 4, QImage::Format_RGB32);
        image.fill(Qt::red);
        QVERIFY(image.save(imagePath));
        QFile html(dir.filePath(QStringLiteral("page.html")));
        QVERIFY(html.open(QIODevice::WriteOnly));
        const QByteArray markup = QByteArray("<h1>Hello</h1><img src=\"") + QUrl::fromLocalFile(imagePath).toEncoded() + QByteArray("\">");
        QCOMPARE(html.write(markup), markup.size());
        html.close();
        PreviewPanel panel;
        panel.show();
        panel.setItems({KFileItem(QUrl::fromLocalFile(html.fileName()))});
        auto *rendered = panel.findChild<QTextBrowser *>(QStringLiteral("previewRenderedText"));
        auto *source = panel.findChild<QPlainTextEdit *>(QStringLiteral("previewText"));
        auto *mode = panel.findChild<QToolButton *>(QStringLiteral("previewRenderMode"));
        QVERIFY(rendered && source && mode);
        QTRY_VERIFY(rendered->isVisible());
        QVERIFY(rendered->toPlainText().contains(QStringLiteral("Hello")));
        const QVariant resource = rendered->document()->resource(QTextDocument::ImageResource, QUrl::fromLocalFile(imagePath));
        QCOMPARE(resource.metaType(), QMetaType::fromType<QImage>());
        QVERIFY(resource.value<QImage>().isNull());
        mode->click();
        QCOMPARE(source->toPlainText().toUtf8(), markup);
    }

    void previewPresentationSettings()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile html(dir.filePath(QStringLiteral("page.html")));
        QVERIFY(html.open(QIODevice::WriteOnly));
        QCOMPARE(html.write("<h1>Hello</h1>"), 14);
        html.close();
        QFile markdown(dir.filePath(QStringLiteral("page.md")));
        QVERIFY(markdown.open(QIODevice::WriteOnly));
        QCOMPARE(markdown.write("# Hello"), 7);
        markdown.close();
        QFile code(dir.filePath(QStringLiteral("code.cpp")));
        QVERIFY(code.open(QIODevice::WriteOnly));
        QCOMPARE(code.write("int main() { return 42; }\n"), 26);
        code.close();
        Settings::setPreviewFormatted(false);
        Settings::setPreviewLineNumbers(false);
        Settings::setPreviewSyntaxHighlighting(false);
        PreviewPanel panel;
        panel.show();
        auto *source = panel.findChild<QPlainTextEdit *>(QStringLiteral("previewText"));
        auto *rendered = panel.findChild<QTextBrowser *>(QStringLiteral("previewRenderedText"));
        auto *mode = panel.findChild<QToolButton *>(QStringLiteral("previewRenderMode"));
        auto *lineNumbers = panel.findChild<QWidget *>(QStringLiteral("previewLineNumbers"));
        QVERIFY(source && rendered && mode && lineNumbers);
        panel.setItems({KFileItem(QUrl::fromLocalFile(html.fileName()))});
        QTRY_VERIFY(source->isVisible());
        QVERIFY(!mode->isChecked());
        QVERIFY(lineNumbers->isHidden());
        mode->click();
        QVERIFY(rendered->isVisible());
        panel.setItems({KFileItem(QUrl::fromLocalFile(markdown.fileName()))});
        QTRY_VERIFY(source->isVisible());
        QVERIFY(!mode->isChecked());
        Settings::setPreviewFormatted(true);
        panel.applyPresentationSettings();
        QVERIFY(rendered->isVisible());
        QVERIFY(mode->isChecked());

        panel.setItems({KFileItem(QUrl::fromLocalFile(code.fileName()))});
        QTRY_VERIFY(source->isVisible());
        QTRY_COMPARE(source->toPlainText(), QStringLiteral("int main() { return 42; }\n"));
        QVERIFY(source->document()->firstBlock().layout()->formats().isEmpty());
        Settings::setPreviewLineNumbers(true);
        Settings::setPreviewSyntaxHighlighting(true);
        panel.applyPresentationSettings();
        QVERIFY(lineNumbers->isVisible());
        panel.setLayoutDirection(Qt::RightToLeft);
        QTRY_VERIFY(lineNumbers->geometry().left() >= source->viewport()->geometry().right());
        panel.setLayoutDirection(Qt::LeftToRight);
        QTRY_VERIFY(lineNumbers->geometry().right() <= source->viewport()->geometry().left());
        QTRY_VERIFY(!source->document()->firstBlock().layout()->formats().isEmpty());
        Settings::setPreviewSyntaxHighlighting(false);
        panel.applyPresentationSettings();
        QTRY_VERIFY(source->document()->firstBlock().layout()->formats().isEmpty());
    }

    void editorPlacementsKeepBothPanesAndDocument()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile file(dir.filePath(QStringLiteral("sample.txt")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write("before"), 6);
        file.close();
        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(dir.path())});
        window.show();
        auto *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 1);
        view->selectUrls({QUrl::fromLocalFile(file.fileName())});
        window.actionCollection()->action(QStringLiteral("show_preview_panel"))->setChecked(true);
        auto *open = window.actionCollection()->action(QStringLiteral("open_editor"));
        QVERIFY(open);
        open->trigger();
        auto *editor = window.findChild<EditorPanel *>(QStringLiteral("editorPanel"));
        QVERIFY(editor);
        QTRY_COMPARE(editor->document()->text(), QStringLiteral("before"));
        const auto panes = window.findChildren<Pane *>();
        QCOMPARE(panes.size(), 2);
        QVERIFY(panes.at(0)->isVisible() && panes.at(1)->isVisible());
        QVERIFY(editor->document()->setText(QStringLiteral("after")));
        auto *markdownButton = editor->findChild<QPushButton *>(QStringLiteral("editorMarkdownPreview"));
        auto *markdownPreview = editor->findChild<QTextBrowser *>(QStringLiteral("editorRenderedMarkdown"));
        QVERIFY(markdownButton && markdownPreview);
        markdownButton->setChecked(true);
        QVERIFY(editor->document()->setText(QStringLiteral("# Live heading")));
        QCOMPARE(markdownPreview->toPlainText().trimmed(), QStringLiteral("Live heading"));
        QVERIFY(editor->document()->setText(QStringLiteral("after")));
        const QStringList placements{QStringLiteral("editorPlaceBelow"),
                                     QStringLiteral("editorPlaceRight"),
                                     QStringLiteral("editorPlaceWindow"),
                                     QStringLiteral("editorPlaceMaximized"),
                                     QStringLiteral("editorPlacePanel")};
        for (const QString &name : placements) {
            auto *action = editor->findChild<QAction *>(name);
            QVERIFY(action);
            action->trigger();
            QCOMPARE(editor->document()->text(), QStringLiteral("after"));
            if (name != QLatin1String("editorPlaceWindow") && name != QLatin1String("editorPlaceMaximized")) {
                QVERIFY(panes.at(0)->isVisible() && panes.at(1)->isVisible());
            }
        }
        QVERIFY(editor->document()->documentSave());
        QVERIFY(file.open(QIODevice::ReadOnly));
        const QByteArray saved = file.readAll();
        QVERIFY(saved == QByteArray("after") || saved == QByteArray("after\n"));
        editor->findChild<QAction *>(QStringLiteral("editorPlaceRight"))->trigger();
        QCOMPARE(Settings::editorOpening(), Settings::EnumEditorOpening::Embedded);
        QCOMPARE(Settings::editorEmbeddedPlacement(), Settings::EnumEditorEmbeddedPlacement::Right);
        auto *dock = window.findChild<QDockWidget *>(QStringLiteral("editorDock"));
        QVERIFY(dock && dock->isVisible());
        dock->hide();
        open->trigger();
        QVERIFY(dock->isVisible());
        QVERIFY(editor->document()->text() == QStringLiteral("after") || editor->document()->text() == QStringLiteral("after\n"));
    }

    void previewPlacementKeepsPanesAndTerminalIndependent()
    {
        Settings::setTerminalPlacement(Settings::EnumTerminalPlacement::Below);
        MainWindow window;
        window.resize(1600, 1000);
        window.openUrls({m_a, m_b});
        window.show();
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(true);
        auto *previewAction = window.actionCollection()->action(QStringLiteral("show_preview_panel"));
        auto *terminalAction = window.actionCollection()->action(QStringLiteral("show_embedded_terminal"));
        auto *preview = window.findChild<PreviewPanel *>(QStringLiteral("previewPanel"));
        auto *dock = window.findChild<QDockWidget *>(QStringLiteral("previewDock"));
        auto *workspace = window.findChild<QSplitter *>(QStringLiteral("workspaceSplitter"));
        QVERIFY(previewAction && terminalAction && preview && dock && workspace);
        terminalAction->setChecked(true);
        const Qt::Orientation terminalOrientation = workspace->orientation();
        auto *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 2);
        view->selectUrls({QUrl::fromLocalFile(m_root.filePath(QStringLiteral("a/file.txt")))});
        previewAction->setChecked(true);
        auto *text = preview->findChild<QPlainTextEdit *>(QStringLiteral("previewText"));
        QVERIFY(text);
        QTRY_COMPARE(text->toPlainText(), QStringLiteral("hello"));
        for (bool stacked : {false, true}) {
            window.setStackedLayout(stacked);
            for (const auto &name : {QStringLiteral("previewPlaceBelow"), QStringLiteral("previewPlaceRight")}) {
                auto *place = preview->findChild<QAction *>(name);
                QVERIFY(place);
                place->trigger();
                QVERIFY(dock->isVisible());
                QCOMPARE(window.dockWidgetArea(dock), name == QLatin1String("previewPlaceBelow") ? Qt::BottomDockWidgetArea : Qt::RightDockWidgetArea);
                QVERIFY(window.activePane()->isVisible() && window.inactivePane()->isVisible());
                QVERIFY(terminalAction->isChecked());
                QCOMPARE(workspace->orientation(), terminalOrientation);
                QCOMPARE(text->toPlainText(), QStringLiteral("hello"));
            }
        }
        window.resizeDocks({dock}, {400}, Qt::Horizontal);
        QCoreApplication::processEvents();
        const int dockWidth = dock->width();
        previewAction->setChecked(false);
        QVERIFY(!dock->isVisible());
        previewAction->setChecked(true);
        QVERIFY(dock->isVisible());
        QCoreApplication::processEvents();
        QVERIFY(qAbs(dock->width() - dockWidth) < 70);
        QVERIFY(Settings::previewPlacement() == Settings::EnumPreviewPlacement::Right);
        MainWindow restored;
        restored.openUrls({m_a, m_b});
        restored.show();
        restored.actionCollection()->action(QStringLiteral("show_preview_panel"))->setChecked(true);
        auto *restoredDock = restored.findChild<QDockWidget *>(QStringLiteral("previewDock"));
        QVERIFY(restoredDock && restoredDock->isVisible());
        QCOMPARE(restored.dockWidgetArea(restoredDock), Qt::RightDockWidgetArea);
        QVERIFY(dock->close());
        QVERIFY(!previewAction->isChecked());
    }

    void embeddedTitlePreferenceKeepsPanelControlsAndDocumentStatus()
    {
        Settings::setPanelTitlesVisible(false);
        Settings::setEditorOpening(Settings::EnumEditorOpening::Embedded);
        MainWindow window;
        window.resize(1600, 1000);
        window.openUrls({m_a, m_b});
        window.show();
        auto *previewAction = window.actionCollection()->action(QStringLiteral("show_preview_panel"));
        auto *terminalAction = window.actionCollection()->action(QStringLiteral("show_embedded_terminal"));
        auto *openEditor = window.actionCollection()->action(QStringLiteral("open_editor"));
        QVERIFY(previewAction && terminalAction && openEditor);
        terminalAction->setChecked(true);
        auto *terminal = window.findChild<QWidget *>(QStringLiteral("terminalPanel"));
        auto *terminalTitle = terminal->findChild<QLabel *>(QStringLiteral("terminalPanelTitle"));
        QVERIFY(terminalTitle && !terminalTitle->isHidden());
        QVERIFY(window.findChild<QDockWidget *>(QStringLiteral("combinedDock"))->titleBarWidget());
        auto *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 2);
        view->selectUrls({QUrl::fromLocalFile(m_root.filePath(QStringLiteral("a/file.txt")))});
        previewAction->setChecked(true);
        openEditor->trigger();
        auto *editor = window.findChild<EditorPanel *>(QStringLiteral("editorPanel"));
        auto *preview = window.findChild<PreviewPanel *>(QStringLiteral("previewPanel"));
        QVERIFY(editor && preview);
        QTRY_COMPARE(editor->document()->text(), QStringLiteral("hello"));
        QVERIFY(editor->document()->setText(QStringLiteral("unsaved")));
        auto *editorTitle = editor->findChild<QLabel *>(QStringLiteral("editorTitle"));
        auto *documentStatus = editor->findChild<QWidget *>(QStringLiteral("editorDocumentStatus"));
        auto *previewTitle = preview->findChild<QLabel *>(QStringLiteral("previewTitle"));
        auto *previewHeader = preview->findChild<QHBoxLayout *>(QStringLiteral("previewHeaderLayout"));
        auto *previewFooter = preview->findChild<QHBoxLayout *>(QStringLiteral("previewFooterLayout"));
        auto *editorDock = window.findChild<QDockWidget *>(QStringLiteral("editorDock"));
        auto *previewDock = window.findChild<QDockWidget *>(QStringLiteral("previewDock"));
        QVERIFY(editorTitle && documentStatus && previewTitle && previewHeader && previewFooter && editorDock && previewDock);
        QTRY_VERIFY(editorTitle->text().contains(QLatin1Char('*')));
        QVERIFY(previewHeader->indexOf(previewTitle) >= 0);
        Settings::setEmbeddedPanelTitlesVisible(false);
        Settings::self()->save();
        window.applySettings();
        QVERIFY(terminalTitle->isHidden());
        QVERIFY(!terminal->findChild<QToolButton *>(QStringLiteral("terminalPanelMenuButton"))->isHidden());
        QVERIFY(!terminal->findChild<QToolButton *>(QStringLiteral("terminalPanelClose"))->isHidden());
        QVERIFY(editorDock->titleBarWidget() && previewDock->titleBarWidget());
        QVERIFY(!documentStatus->isHidden());
        QCOMPARE(editorTitle->parentWidget(), documentStatus);
        QVERIFY(editorTitle->text().contains(QLatin1Char('*')));
        QCOMPARE(editor->document()->text(), QStringLiteral("unsaved"));
        QVERIFY(!editor->findChild<QToolButton *>(QStringLiteral("editorPlacement"))->isHidden());
        QVERIFY(!editor->findChild<QPushButton *>(QStringLiteral("editorHide"))->isHidden());
        QVERIFY(previewHeader->indexOf(previewTitle) < 0 && previewFooter->indexOf(previewTitle) >= 0);
        QVERIFY(previewTitle->text().contains(QStringLiteral("file.txt")));
        QVERIFY(!preview->findChild<QToolButton *>(QStringLiteral("previewPlacement"))->isHidden());
        QVERIFY(!preview->findChild<QToolButton *>(QStringLiteral("previewClose"))->isHidden());
        view->clearSelection();
        QTRY_VERIFY(previewTitle->isHidden());
        view->selectUrls({QUrl::fromLocalFile(m_root.filePath(QStringLiteral("a/file.txt")))});
        QTRY_VERIFY(!previewTitle->isHidden());
        QVERIFY(previewTitle->text().contains(QStringLiteral("file.txt")));
        auto *information = window.findChild<QDockWidget *>(QStringLiteral("informationDock"));
        auto *informationAction = window.actionCollection()->action(QStringLiteral("show_information_panel"));
        QVERIFY(information && informationAction && information->titleBarWidget());
        information->hide();
        informationAction->trigger();
        auto *informationOptions = information->findChild<QToolButton *>(QStringLiteral("informationPanelOptions"));
        QVERIFY(informationOptions);
        QTRY_VERIFY(information->isVisible());
        QTRY_VERIFY(informationOptions->isVisible());
        auto *informationLeft = information->findChild<QAction *>(QStringLiteral("informationPlaceLeft"));
        auto *informationRight = information->findChild<QAction *>(QStringLiteral("informationPlaceRight"));
        auto *informationHide = information->findChild<QAction *>(QStringLiteral("informationHide"));
        QVERIFY(informationLeft && informationRight && informationHide);
        informationLeft->trigger();
        QCOMPARE(window.dockWidgetArea(information), Qt::LeftDockWidgetArea);
        informationRight->trigger();
        QCOMPARE(window.dockWidgetArea(information), Qt::RightDockWidgetArea);
        informationHide->trigger();
        QVERIFY(information->isHidden() && !informationAction->isChecked());
        informationAction->trigger();
        QVERIFY(information->isVisible());
        for (bool stacked : {false, true}) {
            window.setStackedLayout(stacked);
            for (const auto &name : {QStringLiteral("editorPlacePanel"), QStringLiteral("editorPlaceBelow"), QStringLiteral("editorPlaceRight")}) {
                auto *place = editor->findChild<QAction *>(name);
                QVERIFY(place);
                place->trigger();
                QVERIFY(!documentStatus->isHidden());
                QCOMPARE(editor->document()->text(), QStringLiteral("unsaved"));
                QVERIFY(editorTitle->text().contains(QLatin1Char('*')));
            }
            for (const auto &name : {QStringLiteral("previewPlaceBelow"), QStringLiteral("previewPlaceRight"), QStringLiteral("previewPlacePane")}) {
                auto *place = preview->findChild<QAction *>(name);
                QVERIFY(place);
                place->trigger();
                QVERIFY(previewFooter->indexOf(previewTitle) >= 0);
                QVERIFY(previewTitle->text().contains(QStringLiteral("file.txt")));
            }
        }
        preview->findChild<QToolButton *>(QStringLiteral("previewClose"))->click();
        QVERIFY(!previewAction->isChecked());
        previewAction->setChecked(true);
        QTRY_VERIFY(previewTitle->text().contains(QStringLiteral("file.txt")));
        QVERIFY(previewFooter->indexOf(previewTitle) >= 0);
        auto *placeWindow = editor->findChild<QAction *>(QStringLiteral("editorPlaceWindow"));
        auto *placeBelow = editor->findChild<QAction *>(QStringLiteral("editorPlaceBelow"));
        QVERIFY(placeWindow && placeBelow);
        placeWindow->trigger();
        QVERIFY(documentStatus->isHidden());
        QVERIFY(!editorTitle->isHidden());
        QCOMPARE(editor->document()->text(), QStringLiteral("unsaved"));
        placeBelow->trigger();
        QVERIFY(!documentStatus->isHidden());
        editor->findChild<QPushButton *>(QStringLiteral("editorHide"))->click();
        QVERIFY(editorDock->isHidden());
        openEditor->trigger();
        QVERIFY(editorDock->isVisible());
        QCOMPARE(editor->document()->text(), QStringLiteral("unsaved"));
        QVERIFY(editorTitle->text().contains(QLatin1Char('*')));
        Settings::self()->load();
        MainWindow restored;
        restored.openUrls({m_a, m_b});
        restored.show();
        QVERIFY(restored.findChild<QLabel *>(QStringLiteral("terminalPanelTitle"))->isHidden());
        QVERIFY(restored.findChild<QDockWidget *>(QStringLiteral("previewDock"))->titleBarWidget());
        restored.actionCollection()->action(QStringLiteral("open_editor"))->trigger();
        auto *restoredStatus = restored.findChild<QWidget *>(QStringLiteral("editorDocumentStatus"));
        QVERIFY(restoredStatus && !restoredStatus->isHidden());
        Settings::setEmbeddedPanelTitlesVisible(true);
        window.applySettings();
        QVERIFY(!terminalTitle->isHidden());
        QVERIFY(!editorDock->titleBarWidget() && !previewDock->titleBarWidget());
        QVERIFY(!information->titleBarWidget() && informationOptions->isHidden());
        QVERIFY(documentStatus->isHidden());
        QVERIFY(previewHeader->indexOf(previewTitle) >= 0);
        QCOMPARE(editor->document()->text(), QStringLiteral("unsaved"));
    }

    void editorUsesConfiguredOpeningPlacement()
    {
        Settings::setEditorOpening(Settings::EnumEditorOpening::Embedded);
        Settings::setEditorEmbeddedPlacement(Settings::EnumEditorEmbeddedPlacement::Right);
        MainWindow window;
        window.openUrls({m_a, m_b});
        window.show();
        window.actionCollection()->action(QStringLiteral("open_editor"))->trigger();
        auto *dock = window.findChild<QDockWidget *>(QStringLiteral("editorDock"));
        QVERIFY(dock);
        QVERIFY(dock->isVisible());
        QCOMPARE(window.dockWidgetArea(dock), Qt::RightDockWidgetArea);
        Settings::setEditorOpening(Settings::EnumEditorOpening::Window);
        window.actionCollection()->action(QStringLiteral("open_editor"))->trigger();
        QVERIFY(dock->isVisible());
        dock->hide();
        window.actionCollection()->action(QStringLiteral("open_editor"))->trigger();
        auto *editorWindow = window.findChild<QDialog *>(QStringLiteral("editorWindow"));
        QVERIFY(editorWindow && editorWindow->isVisible());
        QVERIFY(!dock->isVisible());
    }

    void fileComparison()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString first = dir.filePath(QStringLiteral("-first.txt"));
        const QString second = dir.filePath(QStringLiteral("second.txt"));
        QFile a(first);
        QFile b(second);
        QVERIFY(a.open(QIODevice::WriteOnly));
        QCOMPARE(a.write("same\nold\n"), 9);
        a.close();
        QVERIFY(b.open(QIODevice::WriteOnly));
        QCOMPARE(b.write("same\nnew\n"), 9);
        b.close();
        QVERIFY(!QStandardPaths::findExecutable(QStringLiteral("diff")).isEmpty());

        FileDiffDialog different(first, second);
        auto *status = different.findChild<QLabel *>(QStringLiteral("fileDiffStatus"));
        auto *output = different.findChild<QPlainTextEdit *>(QStringLiteral("fileDiffOutput"));
        QVERIFY(status && output);
        QTRY_COMPARE(status->text(), QStringLiteral("The files differ."));
        QVERIFY(output->toPlainText().contains(QStringLiteral("-old")));
        QVERIFY(output->toPlainText().contains(QStringLiteral("+new")));

        FileDiffDialog identical(first, first);
        QTRY_COMPARE(identical.findChild<QLabel *>(QStringLiteral("fileDiffStatus"))->text(), QStringLiteral("The files are identical."));
        FileDiffDialog missing(first, dir.filePath(QStringLiteral("missing")));
        QTRY_VERIFY(missing.findChild<QLabel *>(QStringLiteral("fileDiffStatus"))->text().startsWith(QStringLiteral("The comparison failed:")));

        const QByteArray path = qgetenv("PATH");
        qputenv("PATH", dir.path().toLocal8Bit());
        FileDiffDialog missingTool(first, second);
        qputenv("PATH", path);
        QTRY_VERIFY(missingTool.findChild<QLabel *>(QStringLiteral("fileDiffStatus"))->text().startsWith(QStringLiteral("Could not start diff:")));

        QVERIFY(b.open(QIODevice::WriteOnly | QIODevice::Truncate));
        QCOMPARE(b.write(QByteArray(5 * 1024 * 1024, 'x')), qint64(5 * 1024 * 1024));
        b.close();
        FileDiffDialog large(first, second);
        QTRY_COMPARE(large.findChild<QLabel *>(QStringLiteral("fileDiffStatus"))->text(),
                     QStringLiteral("The comparison output exceeded 4 MiB and was stopped."));
        QVERIFY(large.findChild<QPlainTextEdit *>(QStringLiteral("fileDiffOutput"))->toPlainText().isEmpty());
    }

    void fileDiffWrapDoesNotChangeComparison()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile first(dir.filePath(QStringLiteral("a")));
        QFile second(dir.filePath(QStringLiteral("b")));
        QVERIFY(first.open(QIODevice::WriteOnly));
        QVERIFY(first.write(QByteArray(500, 'a')) == 500);
        first.close();
        QVERIFY(second.open(QIODevice::WriteOnly));
        QVERIFY(second.write(QByteArray(500, 'b')) == 500);
        second.close();
        const bool previous = Settings::fileDiffWrapLines();
        const auto restore = qScopeGuard([previous] {
            Settings::setFileDiffWrapLines(previous);
        });
        Settings::setFileDiffWrapLines(true);
        FileDiffDialog dialog(first.fileName(), second.fileName());
        auto *status = dialog.findChild<QLabel *>(QStringLiteral("fileDiffStatus"));
        auto *output = dialog.findChild<QPlainTextEdit *>(QStringLiteral("fileDiffOutput"));
        auto *wrap = dialog.findChild<QCheckBox *>(QStringLiteral("fileDiffWrapLines"));
        QVERIFY(status && output && wrap);
        QTRY_COMPARE(status->text(), QStringLiteral("The files differ."));
        QVERIFY(wrap->isChecked());
        QCOMPARE(output->lineWrapMode(), QPlainTextEdit::WidgetWidth);
        const QString comparison = output->toPlainText();
        wrap->setChecked(false);
        QCOMPARE(output->lineWrapMode(), QPlainTextEdit::NoWrap);
        QCOMPARE(output->toPlainText(), comparison);
        QCOMPARE(status->text(), QStringLiteral("The files differ."));
    }

    void compareFilesActionUsesBothPanes()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QVERIFY(QDir(dir.path()).mkdir(QStringLiteral("left")));
        QVERIFY(QDir(dir.path()).mkdir(QStringLiteral("right")));
        const QString first = dir.filePath(QStringLiteral("left/first.txt"));
        const QString second = dir.filePath(QStringLiteral("right/second.txt"));
        QFile a(first);
        QFile b(second);
        QVERIFY(a.open(QIODevice::WriteOnly));
        a.write("first\n");
        a.close();
        QVERIFY(b.open(QIODevice::WriteOnly));
        b.write("second\n");
        b.close();
        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(dir.filePath(QStringLiteral("left"))), QUrl::fromLocalFile(dir.filePath(QStringLiteral("right")))});
        window.show();
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(true);
        auto *left = window.activePane()->view();
        auto *right = window.inactivePane()->view();
        QTRY_COMPARE(left->itemCount(), 1);
        QTRY_COMPARE(right->itemCount(), 1);
        auto *action = window.actionCollection()->action(QStringLiteral("compare_files"));
        QVERIFY(action && !action->isEnabled());
        left->selectUrls({QUrl::fromLocalFile(first)});
        QVERIFY(!action->isEnabled());
        right->selectUrls({QUrl::fromLocalFile(second)});
        QTRY_VERIFY(action->isEnabled());
        action->trigger();
        auto *dialog = window.findChild<FileDiffDialog *>(QStringLiteral("fileDiffDialog"));
        QVERIFY(dialog);
        QTRY_COMPARE(dialog->findChild<QLabel *>(QStringLiteral("fileDiffStatus"))->text(), QStringLiteral("The files differ."));
        window.actionCollection()->action(QStringLiteral("show_preview_panel"))->setChecked(true);
        QVERIFY(!action->isEnabled());
        window.actionCollection()->action(QStringLiteral("show_preview_panel"))->setChecked(false);
        QVERIFY(action->isEnabled());
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(false);
        QVERIFY(!action->isEnabled());
        QPointer<FileDiffDialog> closedDialog = dialog;
        dialog->close();
        QTRY_VERIFY(closedDialog.isNull());
        QFile localSecond(dir.filePath(QStringLiteral("left/third.txt")));
        QVERIFY(localSecond.open(QIODevice::WriteOnly));
        localSecond.write("third\n");
        localSecond.close();
        QTRY_COMPARE(left->itemCount(), 2);
        left->selectUrls({QUrl::fromLocalFile(first), QUrl::fromLocalFile(localSecond.fileName())});
        QTRY_VERIFY(action->isEnabled());
        action->trigger();
        auto *samePaneDialog = window.findChild<FileDiffDialog *>(QStringLiteral("fileDiffDialog"));
        QVERIFY(samePaneDialog);
        QTRY_COMPARE(samePaneDialog->findChild<QLabel *>(QStringLiteral("fileDiffStatus"))->text(), QStringLiteral("The files differ."));
    }

    void createsSymbolicLinkFromSelection()
    {
        const QString previousSuffix = Settings::symlinkSuffix();
        const auto restore = qScopeGuard([previousSuffix] {
            Settings::setSymlinkSuffix(previousSuffix);
        });
        Settings::setSymlinkSuffix(QString());
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QVERIFY(QDir(dir.path()).mkdir(QStringLiteral("a")));
        QVERIFY(QDir(dir.path()).mkdir(QStringLiteral("b")));
        QFile source(dir.filePath(QStringLiteral("a/source.txt")));
        QVERIFY(source.open(QIODevice::WriteOnly));
        source.write("source");
        source.close();
        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(dir.filePath(QStringLiteral("a"))), QUrl::fromLocalFile(dir.filePath(QStringLiteral("b")))});
        window.show();
        FolderView *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 1);
        QCOMPARE(window.inactivePane()->url(), QUrl::fromLocalFile(dir.filePath(QStringLiteral("b"))));
        QAction *action = window.actionCollection()->action(QStringLiteral("create_symlink"));
        QVERIFY(action && !action->isEnabled());
        view->selectUrls({QUrl::fromLocalFile(source.fileName())});
        QTRY_VERIFY(action->isEnabled());
        QTimer::singleShot(0, [] {
            auto *dialog = qobject_cast<SymlinkDialog *>(QApplication::activeModalWidget());
            QVERIFY(dialog);
            QCOMPARE(dialog->name(), QStringLiteral("source.txt - link"));
            QVERIFY(dialog->findChild<QRadioButton *>(QStringLiteral("symlinkBesideTarget"))->isChecked());
            dialog->reject();
        });
        action->trigger();
        Settings::setSymlinkSuffix(QStringLiteral(".shortcut"));
        QTimer::singleShot(0, [] {
            auto *dialog = qobject_cast<SymlinkDialog *>(QApplication::activeModalWidget());
            QVERIFY(dialog);
            auto *name = dialog->findChild<QLineEdit *>(QStringLiteral("symlinkName"));
            QCOMPARE(name->text(), QStringLiteral("source.txt.shortcut"));
            name->setText(QStringLiteral("alias"));
            dialog->accept();
        });
        action->trigger();
        const QString link = dir.filePath(QStringLiteral("a/alias"));
        QTRY_VERIFY(QFileInfo(link).isSymLink());
        QCOMPARE(QFileInfo(link).symLinkTarget(), source.fileName());

        view->selectUrls({QUrl::fromLocalFile(source.fileName())});
        QTRY_VERIFY(action->isEnabled());
        QTimer::singleShot(0, [] {
            auto *dialog = qobject_cast<SymlinkDialog *>(QApplication::activeModalWidget());
            QVERIFY(dialog);
            auto *other = dialog->findChild<QRadioButton *>(QStringLiteral("symlinkOtherPane"));
            QVERIFY(other->isEnabled());
            other->setChecked(true);
            dialog->findChild<QCheckBox *>(QStringLiteral("symlinkRelative"))->setChecked(true);
            dialog->findChild<QLineEdit *>(QStringLiteral("symlinkName"))->setText(QStringLiteral("relative"));
            dialog->accept();
        });
        action->trigger();
        const QString relative = dir.filePath(QStringLiteral("b/relative"));
        QTRY_VERIFY(QFileInfo(relative).isSymLink());
        QCOMPARE(QFileInfo(relative).readSymLink(), QStringLiteral("../a/source.txt"));
        QCOMPARE(QFileInfo(relative).canonicalFilePath(), QFileInfo(source.fileName()).canonicalFilePath());
    }

    void exportsFilteredListingInColumnOrder()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        for (const QString &name : {QStringLiteral("z.txt"), QStringLiteral("a.txt"), QStringLiteral("hidden.bin")}) {
            QFile file(dir.filePath(name));
            QVERIFY(file.open(QIODevice::WriteOnly));
            file.write("data");
        }
        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(dir.path())});
        window.show();
        FolderView *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 3);
        QTRY_VERIFY(!view->isLoading());
        view->setViewMode(FolderView::ViewMode::Details);
        view->setNameFilter(NameFilter(QStringLiteral("*.txt")));
        view->setSorting(KDirModel::Name, Qt::AscendingOrder);
        auto *details = qobject_cast<QTreeView *>(view->currentView());
        QVERIFY(details);
        view->setColumnVisible(KDirModel::Size, true);
        details->header()->moveSection(details->header()->visualIndex(KDirModel::Size), 0);
        const ListingExport::Table table = view->listingForExport();
        QCOMPARE(table.rows.size(), 2);
        QCOMPARE(table.headers.first(), details->model()->headerData(KDirModel::Size, Qt::Horizontal).toString());
        const int nameColumn = table.headers.indexOf(details->model()->headerData(KDirModel::Name, Qt::Horizontal).toString());
        QVERIFY(nameColumn >= 0);
        QCOMPARE(table.rows.at(0).at(nameColumn), QStringLiteral("a.txt"));
        QCOMPARE(table.rows.at(1).at(nameColumn), QStringLiteral("z.txt"));
        const QString output = dir.filePath(QStringLiteral("listing.csv"));
        QTimer::singleShot(0, [&] {
            auto *dialog = qobject_cast<QFileDialog *>(QApplication::activeModalWidget());
            QVERIFY(dialog);
            dialog->selectFile(dir.filePath(QStringLiteral("listing")));
            QMetaObject::invokeMethod(dialog, "accept");
        });
        window.actionCollection()->action(QStringLiteral("export_listing_csv"))->trigger();
        QFile file(output);
        QVERIFY(file.open(QIODevice::ReadOnly));
        const QByteArray csv = file.readAll();
        QVERIFY(csv.contains("a.txt"));
        QVERIFY(csv.contains("z.txt"));
        QVERIFY(!csv.contains("hidden.bin"));
        QTimer::singleShot(0, [&] {
            auto *dialog = qobject_cast<QFileDialog *>(QApplication::activeModalWidget());
            QVERIFY(dialog);
            dialog->selectFile(dir.filePath(QStringLiteral("listing")));
            QMetaObject::invokeMethod(dialog, "accept");
        });
        window.actionCollection()->action(QStringLiteral("export_listing_pdf"))->trigger();
        QFile pdf(dir.filePath(QStringLiteral("listing.pdf")));
        QVERIFY(pdf.open(QIODevice::ReadOnly));
        QCOMPARE(pdf.read(5), QByteArray("%PDF-"));
    }

    void exportUsesCustomColumnsAndDefaultFormat()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile source(dir.filePath(QStringLiteral("item.txt")));
        QVERIFY(source.open(QIODevice::WriteOnly));
        source.write("data");
        source.close();
        const auto format = Settings::listingExportFormat();
        const bool custom = Settings::listingExportCustomColumns();
        const bool type = Settings::listingExportType();
        const bool size = Settings::listingExportSize();
        const bool modified = Settings::listingExportModified();
        const bool permissions = Settings::listingExportPermissions();
        const bool owner = Settings::listingExportOwner();
        const bool group = Settings::listingExportGroup();
        const auto restore = qScopeGuard([=] {
            Settings::setListingExportFormat(format);
            Settings::setListingExportCustomColumns(custom);
            Settings::setListingExportType(type);
            Settings::setListingExportSize(size);
            Settings::setListingExportModified(modified);
            Settings::setListingExportPermissions(permissions);
            Settings::setListingExportOwner(owner);
            Settings::setListingExportGroup(group);
        });
        Settings::setListingExportFormat(Settings::EnumListingExportFormat::Pdf);
        Settings::setListingExportCustomColumns(true);
        Settings::setListingExportType(false);
        Settings::setListingExportSize(false);
        Settings::setListingExportModified(false);
        Settings::setListingExportPermissions(false);
        Settings::setListingExportOwner(true);
        Settings::setListingExportGroup(false);
        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(dir.path())});
        window.show();
        FolderView *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 1);
        QTRY_VERIFY(!view->isLoading());
        view->setViewMode(FolderView::ViewMode::Details);
        view->setColumnVisible(KDirModel::Owner, false);
        const ListingExport::Table table = view->listingForExport({KDirModel::Name, KDirModel::Owner});
        QCOMPARE(table.headers.size(), 2);
        QCOMPARE(table.rows.size(), 1);
        QVERIFY(!table.rows.first().at(1).isEmpty());
        QTimer::singleShot(0, [&] {
            auto *dialog = qobject_cast<QFileDialog *>(QApplication::activeModalWidget());
            QVERIFY(dialog);
            dialog->selectFile(dir.filePath(QStringLiteral("custom")));
            QMetaObject::invokeMethod(dialog, "accept");
        });
        window.actionCollection()->action(QStringLiteral("export_listing_csv"))->trigger();
        QFile csv(dir.filePath(QStringLiteral("custom.csv")));
        QVERIFY(csv.open(QIODevice::ReadOnly));
        const QList<QByteArray> lines = csv.readAll().split('\n');
        QCOMPARE(lines.at(0).count(','), 1);
        QVERIFY(lines.at(1).contains("item.txt"));
        QTimer::singleShot(0, [&] {
            auto *dialog = qobject_cast<QFileDialog *>(QApplication::activeModalWidget());
            QVERIFY(dialog);
            dialog->selectFile(dir.filePath(QStringLiteral("default")));
            QMetaObject::invokeMethod(dialog, "accept");
        });
        window.actionCollection()->action(QStringLiteral("export_listing"))->trigger();
        QFile pdf(dir.filePath(QStringLiteral("default.pdf")));
        QVERIFY(pdf.open(QIODevice::ReadOnly));
        QCOMPARE(pdf.read(5), QByteArray("%PDF-"));
    }

    void contentChartUsesSelectedFolderOrCurrentFolder()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QVERIFY(QDir(dir.path()).mkdir(QStringLiteral("sub")));
        QFile file(dir.filePath(QStringLiteral("file")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("data");
        file.close();
        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(dir.path())});
        window.show();
        FolderView *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 2);
        auto *action = window.actionCollection()->action(QStringLiteral("content_chart"));
        QVERIFY(action && action->isEnabled());
        view->selectUrls({QUrl::fromLocalFile(dir.filePath(QStringLiteral("sub")))});
        action->trigger();
        QPointer<ContentChartDialog> chart = window.findChild<ContentChartDialog *>(QStringLiteral("contentChartDialog"));
        QVERIFY(chart);
        QCOMPARE(chart->findChild<QLabel *>(QStringLiteral("contentChartFolder"))->text(),
                 QUrl::fromLocalFile(dir.filePath(QStringLiteral("sub"))).toDisplayString());
        chart->close();
        QTRY_VERIFY(chart.isNull());
        view->selectUrls({QUrl::fromLocalFile(file.fileName())});
        action->trigger();
        chart = window.findChild<ContentChartDialog *>(QStringLiteral("contentChartDialog"));
        QVERIFY(chart);
        QCOMPARE(chart->findChild<QLabel *>(QStringLiteral("contentChartFolder"))->text(), QUrl::fromLocalFile(dir.path()).toDisplayString());
    }

    void createsAndVerifiesChecksumFiles()
    {
        const QString previousName = Settings::checksumFileName();
        const auto restore = qScopeGuard([previousName] {
            Settings::setChecksumFileName(previousName);
        });
        Settings::setChecksumFileName(QStringLiteral("custom.sha256"));
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile source(dir.filePath(QStringLiteral("source.txt")));
        QVERIFY(source.open(QIODevice::WriteOnly));
        source.write("source");
        source.close();
        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(dir.path())});
        window.show();
        FolderView *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 1);
        auto *create = window.actionCollection()->action(QStringLiteral("create_checksum"));
        auto *verify = window.actionCollection()->action(QStringLiteral("verify_checksum"));
        QVERIFY(create && verify && !create->isEnabled() && !verify->isEnabled());
        view->selectUrls({QUrl::fromLocalFile(source.fileName())});
        QTRY_VERIFY(create->isEnabled());
        const QString manifest = dir.filePath(QStringLiteral("checksums.sha256"));
        QTimer::singleShot(0, [&] {
            auto *dialog = qobject_cast<QFileDialog *>(QApplication::activeModalWidget());
            QVERIFY(dialog);
            QCOMPARE(dialog->selectedFiles(), QStringList{dir.filePath(QStringLiteral("custom.sha256"))});
            dialog->selectFile(manifest);
            QMetaObject::invokeMethod(dialog, "accept");
        });
        create->trigger();
        QPointer<ChecksumDialog> dialog = window.findChild<ChecksumDialog *>(QStringLiteral("checksumDialog"));
        QVERIFY(dialog);
        QTRY_COMPARE(dialog->findChild<QLabel *>(QStringLiteral("checksumStatus"))->text(), QStringLiteral("Checksum file created."));
        QVERIFY(QFileInfo::exists(manifest));
        dialog->close();
        QTRY_VERIFY(dialog.isNull());
        QTRY_COMPARE(view->itemCount(), 2);
        view->selectUrls({QUrl::fromLocalFile(manifest)});
        QTRY_VERIFY(verify->isEnabled());
        verify->trigger();
        dialog = window.findChild<ChecksumDialog *>(QStringLiteral("checksumDialog"));
        QVERIFY(dialog);
        QTRY_COMPARE(dialog->findChild<QLabel *>(QStringLiteral("checksumStatus"))->text(), QStringLiteral("All files verified."));
        auto *results = dialog->findChild<QTreeWidget *>(QStringLiteral("checksumResults"));
        QVERIFY(results);
        QCOMPARE(results->topLevelItemCount(), 1);
        QCOMPARE(results->topLevelItem(0)->text(0), QStringLiteral("source.txt"));
        dialog->close();
        QTRY_VERIFY(dialog.isNull());
        QVERIFY(source.open(QIODevice::WriteOnly | QIODevice::Truncate));
        source.write("changed");
        source.close();
        verify->trigger();
        dialog = window.findChild<ChecksumDialog *>(QStringLiteral("checksumDialog"));
        QVERIFY(dialog);
        QTRY_COMPARE(dialog->findChild<QLabel *>(QStringLiteral("checksumStatus"))->text(), QStringLiteral("Verification finished: 1 file failed."));
    }

    void opensDuplicateFinderForLocalFolder()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(dir.path())});
        window.show();
        auto *action = window.actionCollection()->action(QStringLiteral("find_duplicates"));
        QVERIFY(action && action->isEnabled());
        action->trigger();
        auto *dialog = window.findChild<DuplicateFinderDialog *>(QStringLiteral("duplicateFinderDialog"));
        QVERIFY(dialog);
        QTRY_VERIFY(!dialog->findChild<QPushButton *>(QStringLiteral("duplicateFinderCancel"))->isEnabled());
        QVERIFY(dialog->findChild<QLabel *>(QStringLiteral("duplicateFinderStatus"))->text().startsWith(QStringLiteral("Found 0 groups")));
    }

    void linksPaneNavigationWithinStartingFolders()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("left/sub/child")));
        QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("right/sub/child")));
        QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("left/only-left")));
        const QUrl leftRoot = QUrl::fromLocalFile(dir.filePath(QStringLiteral("left")));
        const QUrl rightRoot = QUrl::fromLocalFile(dir.filePath(QStringLiteral("right")));
        MainWindow window;
        window.openUrls({leftRoot, rightRoot});
        window.show();
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(true);
        auto *link = window.actionCollection()->action(QStringLiteral("link_panes"));
        QVERIFY(link);
        link->setChecked(true);
        QVERIFY(link->isChecked());
        Pane *left = window.activePane();
        Pane *right = window.inactivePane();
        left->setUrl(QUrl::fromLocalFile(dir.filePath(QStringLiteral("left/sub/child"))));
        QCOMPARE(right->url(), QUrl::fromLocalFile(dir.filePath(QStringLiteral("right/sub/child"))));
        right->setUrl(QUrl::fromLocalFile(dir.filePath(QStringLiteral("right/sub"))));
        QCOMPARE(left->url(), QUrl::fromLocalFile(dir.filePath(QStringLiteral("left/sub"))));
        left->setUrl(QUrl::fromLocalFile(dir.filePath(QStringLiteral("left/only-left"))));
        QVERIFY(!link->isChecked());
        QCOMPARE(right->url(), QUrl::fromLocalFile(dir.filePath(QStringLiteral("right/sub"))));
        QVERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("unlinked")));
        left->setUrl(leftRoot);
        right->setUrl(rightRoot);
        link->setChecked(true);
        QVERIFY(link->isChecked());
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(false);
        QVERIFY(!link->isChecked());
        link->setChecked(true);
        QVERIFY(!link->isChecked());
        QVERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("Show both panes")));
    }

    void linksInitialPaneFoldersOnlyWhenConfigured()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("left/sub")));
        QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("right/sub")));
        const auto oldLink = Settings::linkPanesAtStartup();
        const auto oldDual = Settings::dualPaneAtStartup();
        const auto oldMode = Settings::startupMode();
        const auto oldHome = Settings::homeUrl();
        const auto restore = qScopeGuard([oldLink, oldDual, oldMode, oldHome] {
            Settings::setLinkPanesAtStartup(oldLink);
            Settings::setDualPaneAtStartup(oldDual);
            Settings::setStartupMode(oldMode);
            Settings::setHomeUrl(oldHome);
        });
        Settings::setStartupMode(Settings::EnumStartupMode::HomeFolder);
        Settings::setHomeUrl(QUrl::fromLocalFile(dir.path()));
        Settings::setDualPaneAtStartup(Settings::EnumDualPaneAtStartup::Always);
        Settings::setLinkPanesAtStartup(false);
        const QUrl left = QUrl::fromLocalFile(dir.filePath(QStringLiteral("left")));
        const QUrl right = QUrl::fromLocalFile(dir.filePath(QStringLiteral("right")));
        {
            MainWindow window;
            window.openUrls({left, right});
            QVERIFY(!window.actionCollection()->action(QStringLiteral("link_panes"))->isChecked());
        }
        Settings::setLinkPanesAtStartup(true);
        {
            MainWindow window;
            window.openUrls({left, right});
            auto *link = window.actionCollection()->action(QStringLiteral("link_panes"));
            QVERIFY(link->isChecked());
            window.activePane()->setUrl(QUrl::fromLocalFile(dir.filePath(QStringLiteral("left/sub"))));
            QCOMPARE(window.inactivePane()->url(), QUrl::fromLocalFile(dir.filePath(QStringLiteral("right/sub"))));
            link->setChecked(false);
            window.openUrls({});
            QVERIFY(!link->isChecked());
        }
        {
            MainWindow window;
            window.openUrls({left, QUrl::fromLocalFile(dir.filePath(QStringLiteral("missing")))});
            QVERIFY(!window.actionCollection()->action(QStringLiteral("link_panes"))->isChecked());
            QVERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("existing local folders")));
        }
        Settings::setDualPaneAtStartup(Settings::EnumDualPaneAtStartup::Never);
        {
            MainWindow window;
            window.openUrls({left, right});
            QVERIFY(!window.actionCollection()->action(QStringLiteral("link_panes"))->isChecked());
            QVERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("Show both panes")));
        }
    }

    void opensSplitAndCombineForLocalFiles()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile source(dir.filePath(QStringLiteral("source")));
        QVERIFY(source.open(QIODevice::WriteOnly));
        QCOMPARE(source.write("data"), qint64(4));
        source.close();
        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(dir.path())});
        window.show();
        FolderView *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 1);
        auto *split = window.actionCollection()->action(QStringLiteral("split_file"));
        auto *combine = window.actionCollection()->action(QStringLiteral("combine_files"));
        QVERIFY(split && combine);
        QVERIFY(!split->isEnabled());
        QVERIFY(!combine->isEnabled());
        view->selectUrls({QUrl::fromLocalFile(source.fileName())});
        QTRY_VERIFY(split->isEnabled());
        QVERIFY(combine->isEnabled());
        split->trigger();
        auto *dialog = window.findChild<FileSplitDialog *>(QStringLiteral("fileSplitDialog"));
        QVERIFY(dialog);
        dialog->close();
        QTRY_VERIFY(!window.findChild<FileSplitDialog *>(QStringLiteral("fileSplitDialog")));
        combine->trigger();
        QVERIFY(window.findChild<FileSplitDialog *>(QStringLiteral("fileSplitDialog")));
    }

    void paneComparisonFiltersShownResultsWithoutSelectingAutomatically()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QVERIFY(QDir(root.path()).mkdir(QStringLiteral("left")));
        QVERIFY(QDir(root.path()).mkdir(QStringLiteral("right")));
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("left/sub")));
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("right/sub")));
        for (const auto &[name, data] : QList<QPair<QString, QByteArray>>{{QStringLiteral("left/left-only"), "l"},
                                                                          {QStringLiteral("right/right-only"), "r"},
                                                                          {QStringLiteral("left/shared"), "s"},
                                                                          {QStringLiteral("right/shared"), "larger"}}) {
            QFile file(root.filePath(name));
            QVERIFY(file.open(QIODevice::WriteOnly));
            QCOMPARE(file.write(data), qint64(data.size()));
        }
        const auto oldLeft = Settings::paneComparisonShowLeftOnly();
        const auto oldRight = Settings::paneComparisonShowRightOnly();
        const auto oldDifferent = Settings::paneComparisonShowDifferent();
        const auto oldMatching = Settings::paneComparisonShowMatching();
        const auto restore = qScopeGuard([oldLeft, oldRight, oldDifferent, oldMatching] {
            Settings::setPaneComparisonShowLeftOnly(oldLeft);
            Settings::setPaneComparisonShowRightOnly(oldRight);
            Settings::setPaneComparisonShowDifferent(oldDifferent);
            Settings::setPaneComparisonShowMatching(oldMatching);
        });
        Settings::setPaneComparisonShowLeftOnly(true);
        Settings::setPaneComparisonShowRightOnly(false);
        Settings::setPaneComparisonShowDifferent(false);
        Settings::setPaneComparisonShowMatching(false);
        FolderComparisonDialog dialog(QUrl::fromLocalFile(root.filePath(QStringLiteral("left"))), QUrl::fromLocalFile(root.filePath(QStringLiteral("right"))));
        auto *cancel = dialog.findChild<QPushButton *>(QStringLiteral("folderComparisonCancel"));
        QTRY_VERIFY(!cancel->isEnabled());
        auto *tree = dialog.findChild<QTreeWidget *>(QStringLiteral("folderComparisonResults"));
        auto row = [tree](const QString &name) {
            return tree->findItems(name, Qt::MatchExactly).value(0);
        };
        QVERIFY(row(QStringLiteral("left-only")) && !row(QStringLiteral("left-only"))->isHidden());
        QVERIFY(row(QStringLiteral("right-only")) && row(QStringLiteral("right-only"))->isHidden());
        QVERIFY(row(QStringLiteral("shared")) && row(QStringLiteral("shared"))->isHidden());
        QVERIFY(row(QStringLiteral("sub")) && !row(QStringLiteral("sub"))->isHidden());
        QSignalSpy selected(&dialog, &FolderComparisonDialog::selectDifferences);
        QCOMPARE(selected.size(), 0);
        dialog.findChild<QPushButton *>(QStringLiteral("folderComparisonSelect"))->click();
        QCOMPARE(selected.size(), 1);
        QCOMPARE(selected.first().at(0).value<QList<QUrl>>(), (QList<QUrl>{QUrl::fromLocalFile(root.filePath(QStringLiteral("left/left-only")))}));
        QVERIFY(selected.first().at(1).value<QList<QUrl>>().isEmpty());
        dialog.findChild<QCheckBox *>(QStringLiteral("folderComparisonShowLeftOnly"))->setChecked(false);
        QVERIFY(!dialog.findChild<QPushButton *>(QStringLiteral("folderComparisonSelect"))->isEnabled());
        dialog.findChild<QCheckBox *>(QStringLiteral("folderComparisonShowRightOnly"))->setChecked(true);
        QVERIFY(dialog.findChild<QPushButton *>(QStringLiteral("folderComparisonSelect"))->isEnabled());
    }

    void comparesPaneFoldersAndSelectsDifferences()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QVERIFY(QDir(dir.path()).mkdir(QStringLiteral("left")));
        QVERIFY(QDir(dir.path()).mkdir(QStringLiteral("right")));
        for (const auto &[name, data] : QList<QPair<QString, QByteArray>>{{QStringLiteral("left/left-only"), "l"},
                                                                          {QStringLiteral("left/.hidden"), "h"},
                                                                          {QStringLiteral("right/right-only"), "r"},
                                                                          {QStringLiteral("left/shared"), "small"},
                                                                          {QStringLiteral("right/shared"), "larger!"}}) {
            QFile file(dir.filePath(name));
            QVERIFY(file.open(QIODevice::WriteOnly));
            QCOMPARE(file.write(data), qint64(data.size()));
        }
        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(dir.filePath(QStringLiteral("left"))), QUrl::fromLocalFile(dir.filePath(QStringLiteral("right")))});
        window.show();
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(true);
        Pane *left = window.activePane();
        Pane *right = window.inactivePane();
        QTRY_COMPARE(left->view()->itemCount(), 2);
        QTRY_COMPARE(right->view()->itemCount(), 2);
        QAction *action = window.actionCollection()->action(QStringLiteral("compare_pane_folders"));
        QVERIFY(action && action->isEnabled());
        action->trigger();
        auto *dialog = window.findChild<FolderComparisonDialog *>(QStringLiteral("folderComparisonDialog"));
        QVERIFY(dialog);
        auto *select = dialog->findChild<QPushButton *>(QStringLiteral("folderComparisonSelect"));
        auto *results = dialog->findChild<QTreeWidget *>(QStringLiteral("folderComparisonResults"));
        QVERIFY(select && results);
        QTRY_VERIFY(select->isEnabled());
        QCOMPARE(results->topLevelItemCount(), 4);
        select->click();
        QTRY_COMPARE(left->view()->selectedItems().size(), 2);
        QTRY_COMPARE(right->view()->selectedItems().size(), 2);
        QVERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("Some differences could not be selected")));
        left->view()->setShowHiddenFiles(true);
        QTRY_COMPARE(left->view()->itemCount(), 3);
        select->click();
        QTRY_COMPARE(left->view()->selectedItems().size(), 3);
        right->setUrl(QUrl::fromLocalFile(dir.path()));
        select->click();
        QVERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("changed or are hidden")));
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(false);
        QVERIFY(!action->isEnabled());
    }

    void folderComparisonHandlesErrorsAndCancellation()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        FolderComparisonDialog failed(QUrl::fromLocalFile(dir.path()), QUrl::fromLocalFile(dir.filePath(QStringLiteral("missing"))));
        auto *status = failed.findChild<QLabel *>(QStringLiteral("folderComparisonStatus"));
        auto *cancel = failed.findChild<QPushButton *>(QStringLiteral("folderComparisonCancel"));
        auto *select = failed.findChild<QPushButton *>(QStringLiteral("folderComparisonSelect"));
        QVERIFY(status && cancel && select);
        QTRY_VERIFY(!cancel->isEnabled());
        QVERIFY(status->text().startsWith(QStringLiteral("Could not list")));
        QVERIFY(!select->isEnabled());
        FolderComparisonDialog canceled(QUrl::fromLocalFile(dir.path()), QUrl::fromLocalFile(dir.path()));
        canceled.findChild<QPushButton *>(QStringLiteral("folderComparisonCancel"))->click();
        QCOMPARE(canceled.findChild<QLabel *>(QStringLiteral("folderComparisonStatus"))->text(), QStringLiteral("Comparison canceled."));
        QVERIFY(!canceled.findChild<QPushButton *>(QStringLiteral("folderComparisonSelect"))->isEnabled());
    }

    void navigationUpdatesHistory()
    {
        MainWindow window;
        window.openUrls({m_a, m_b});
        Pane *pane = window.activePane();
        QTRY_COMPARE(pane->view()->itemCount(), 2);

        pane->setUrl(QUrl::fromLocalFile(m_root.filePath(QStringLiteral("a/sub"))));
        QVERIFY(pane->tabs()->canGoBack());
        pane->goUp();
        QCOMPARE(pane->url(), m_a);
        pane->goBack();
        QCOMPARE(pane->url().toLocalFile(), m_root.filePath(QStringLiteral("a/sub")));
    }

    void displayStyleIsIndependentPerPane()
    {
        MainWindow window;
        window.openUrls({m_a, m_b});
        Pane *active = window.activePane();
        Pane *other = window.inactivePane();
        QCOMPARE(active->view()->viewMode(), FolderView::ViewMode::Details);
        QCOMPARE(other->view()->viewMode(), FolderView::ViewMode::Details);

        window.actionCollection()->action(QStringLiteral("toggle_view_mode"))->trigger();
        QCOMPARE(active->view()->viewMode(), FolderView::ViewMode::Icons);
        QCOMPARE(other->view()->viewMode(), FolderView::ViewMode::Details);
        window.actionCollection()->action(QStringLiteral("compact_view"))->trigger();
        QCOMPARE(active->view()->viewMode(), FolderView::ViewMode::Compact);
        QCOMPARE(other->view()->viewMode(), FolderView::ViewMode::Details);
        window.actionCollection()->action(QStringLiteral("card_view"))->trigger();
        QCOMPARE(active->view()->viewMode(), FolderView::ViewMode::Cards);
        QCOMPARE(other->view()->viewMode(), FolderView::ViewMode::Details);
        window.actionCollection()->action(QStringLiteral("toggle_view_mode"))->trigger();
        QCOMPARE(active->view()->viewMode(), FolderView::ViewMode::Details);
    }

    void selectionModeActionBarCopiesFromItsPane()
    {
        MainWindow window;
        window.openUrls({m_a, m_b});
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        auto *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 2);
        auto *action = window.actionCollection()->action(QStringLiteral("selection_mode"));
        QVERIFY(action);
        action->trigger();
        QVERIFY(view->selectionMode());
        auto *copy = view->findChild<QToolButton *>(QStringLiteral("selectionCopy"));
        QVERIFY(copy);
        QVERIFY(!copy->isEnabled());
        const QUrl file = QUrl::fromLocalFile(m_root.filePath(QStringLiteral("a/file.txt")));
        view->selectUrls({file});
        QVERIFY(copy->isEnabled());
        copy->click();
        QCOMPARE(QApplication::clipboard()->mimeData()->urls(), QList<QUrl>{file});
        view->findChild<QToolButton *>(QStringLiteral("selectionDone"))->click();
        QVERIFY(!action->isChecked());
        QVERIFY(!view->selectionMode());
    }

    void tabAppearanceSettingsApplyToBothPanes()
    {
        KConfigGroup tabs = KSharedConfig::openConfig()->group(QStringLiteral("Tabs"));
        tabs.writeEntry("ShowIcons", false);
        tabs.writeEntry("Expand", true);
        tabs.sync();
        Settings::self()->load();

        MainWindow window;
        window.openUrls({m_a});
        for (Pane *pane : {window.activePane(), window.inactivePane()}) {
            QTabBar *tabBar = pane->findChild<QTabBar *>();
            QVERIFY(tabBar);
            QVERIFY(tabBar->expanding());
            QVERIFY(tabBar->tabIcon(0).isNull());
        }
    }

    void panelSettingsApplyOnStartup()
    {
        KConfigGroup panels = KSharedConfig::openConfig()->group(QStringLiteral("PanelSettings"));
        panels.writeEntry("Visible", false);
        panels.writeEntry("Position", QStringLiteral("right"));
        panels.writeEntry("TitlesVisible", false);
        panels.writeEntry("FavoritesVisible", false);
        panels.writeEntry("PlacesVisible", false);
        panels.writeEntry("FoldersVisible", false);
        panels.sync();
        Settings::self()->load();

        MainWindow window;
        auto *combined = window.findChild<QDockWidget *>(QStringLiteral("combinedDock"));
        auto *places = window.findChild<QDockWidget *>(QStringLiteral("placesDock"));
        QVERIFY(combined);
        QVERIFY(places);
        QVERIFY(combined->isHidden());
        QCOMPARE(window.dockWidgetArea(places), Qt::RightDockWidgetArea);
        QVERIFY(places->titleBarWidget());

        auto *panel = combined->findChild<CombinedPanel *>();
        QVERIFY(panel);
        auto *favorites = panel->findChild<QListView *>();
        auto *panelPlaces = panel->findChild<KFilePlacesView *>();
        auto *tree = panel->findChild<FolderTree *>();
        QVERIFY(favorites);
        QVERIFY(panelPlaces);
        QVERIFY(tree);
        QVERIFY(favorites->isHidden());
        QVERIFY(panelPlaces->isHidden());
        QVERIFY(tree->isHidden());
    }

    void jumpBackTogglesBetweenFolders()
    {
        MainWindow window;
        window.openUrls({m_a});
        Pane *pane = window.activePane();
        pane->setUrl(m_b);
        QAction *jumpBack = window.actionCollection()->action(QStringLiteral("jump_back"));
        QVERIFY(jumpBack && jumpBack->isEnabled());
        jumpBack->trigger();
        QCOMPARE(pane->url(), m_a);
        jumpBack->trigger();
        QCOMPARE(pane->url(), m_b);
    }

    void popularFoldersMenuListsVisitedLocations()
    {
        MainWindow window;
        window.openUrls({m_a});
        window.activePane()->setUrl(m_b);

        QAction *popularAction = window.actionCollection()->action(QStringLiteral("popular_folders"));
        QVERIFY(popularAction && popularAction->menu());
        QMenu *menu = popularAction->menu();
        QMetaObject::invokeMethod(menu, "aboutToShow");
        QStringList names;
        for (QAction *action : menu->actions()) {
            names.append(action->text());
        }
        QVERIFY(names.contains(QStringLiteral("a")));
        QVERIFY(names.contains(QStringLiteral("b")));
    }

    void savedSelectionFilterSelectsMatchingFiles()
    {
        KSharedConfig::openConfig()->group(QStringLiteral("SelectionFilters")).writeEntry("Patterns", QStringList{QStringLiteral("*.txt")});
        MainWindow window;
        window.openUrls({m_a});
        QTRY_COMPARE(window.activePane()->view()->itemCount(), 2);

        QAction *selectAction = window.actionCollection()->action(QStringLiteral("select_menu"));
        QVERIFY(selectAction && selectAction->menu());
        QMenu *saved = nullptr;
        for (QAction *action : selectAction->menu()->actions()) {
            if (action->menu() && action->text() == QLatin1String("Saved Selection Filters")) {
                saved = action->menu();
                break;
            }
        }
        QVERIFY(saved);
        QMetaObject::invokeMethod(saved, "aboutToShow");
        QCOMPARE(saved->actions().size(), 1);
        saved->actions().first()->trigger();
        QTRY_COMPARE(window.activePane()->view()->selectedItems().size(), 1);
        QCOMPARE(window.activePane()->view()->selectedItems().first().name(), QStringLiteral("file.txt"));
    }

    void occupiedSpaceActionUsesKioDirectorySize()
    {
        QFile nested(m_root.filePath(QStringLiteral("a/sub/nested.txt")));
        QVERIFY(nested.open(QIODevice::WriteOnly));
        QCOMPARE(nested.write("hi"), 2);
        nested.close();

        MainWindow window;
        window.openUrls({m_a});
        FolderView *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 2);
        view->selectUrls({QUrl::fromLocalFile(m_root.filePath(QStringLiteral("a/file.txt")))});

        window.actionCollection()->action(QStringLiteral("calculate_occupied_space"))->trigger();
        QTRY_VERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("occupied in")));
        QVERIFY2(window.statusBar()->currentMessage().contains(QStringLiteral("5 B")), qPrintable(window.statusBar()->currentMessage()));

        view->clearSelection();
        window.actionCollection()->action(QStringLiteral("calculate_occupied_space"))->trigger();
        QTRY_VERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("folder")));
    }

    void filterPersistsWithThePaneSession()
    {
        {
            MainWindow window;
            window.openUrls({m_a});
            auto *filter = window.activePane()->findChild<FilterBar *>();
            QVERIFY(filter);
            filter->setFilter(NameFilter(QStringLiteral("*.txt"), NameFilter::Scope::Files));
            window.close();
        }

        MainWindow restored;
        restored.openUrls({});
        QCOMPARE(restored.activePane()->view()->nameFilter(), NameFilter(QStringLiteral("*.txt"), NameFilter::Scope::Files));
        QTRY_COMPARE(restored.activePane()->view()->itemCount(), 2);
    }

    void viewSettingsApplyToCurrentListing()
    {
        ViewSettings settings;
        settings.visibleColumns = {QStringLiteral("size")};
        settings.groupBy = QStringLiteral("ext");
        settings.sizeFormat = QStringLiteral("bytes");
        settings.dateFormat = QStringLiteral("iso");
        settings.iconNameLines = 4;
        settings.iconInformation = {QStringLiteral("size")};
        KConfigGroup viewConfig = KSharedConfig::openConfig()->group(QStringLiteral("View"));
        settings.writeConfig(viewConfig);
        KConfigGroup files = KSharedConfig::openConfig()->group(QStringLiteral("Files"));
        files.writeEntry("IgnoreList", QStringList{QStringLiteral("sub")});
        files.writeEntry("ExtensionColors", QStringList{QStringLiteral("txt=#123456")});
        KSharedConfig::openConfig()->sync();

        MainWindow window;
        window.openUrls({m_a});
        FolderView *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 1);
        QCOMPARE(view->groupBy(), QStringLiteral("ext"));

        auto *details = view->findChild<QTreeView *>();
        QVERIFY(details);
        QVERIFY(details->isColumnHidden(KDirModel::Type));
        QVERIFY(!details->isColumnHidden(KDirModel::Size));
        QVERIFY(details->isColumnHidden(KDirModel::ModifiedTime));
        const QModelIndex nameIndex = details->model()->index(0, KDirModel::Name);
        QCOMPARE(details->model()->data(nameIndex, Qt::ForegroundRole).value<QColor>(), QColor(QStringLiteral("#123456")));
        QCOMPARE(details->model()->data(details->model()->index(0, KDirModel::Size)).toString(), QStringLiteral("5 B"));
        QVERIFY(QRegularExpression(QStringLiteral(R"(^\d{4}-\d{2}-\d{2} \d{2}:\d{2}$)"))
                    .match(details->model()->data(details->model()->index(0, KDirModel::ModifiedTime)).toString())
                    .hasMatch());

        view->setViewMode(FolderView::ViewMode::Icons);
        auto *delegate = qobject_cast<KFileItemDelegate *>(view->currentView()->itemDelegate());
        QVERIFY(delegate);
        QCOMPARE(delegate->showInformation(), KFileItemDelegate::InformationList{KFileItemDelegate::Size});
        auto *icons = qobject_cast<QListView *>(view->currentView());
        QVERIFY(icons);
        QVERIFY(icons->gridSize().height() > 112);
    }

    void persianListingDateAndIsoOverride()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QFile file(directory.filePath(QStringLiteral("dated.txt")));
        QVERIFY(file.open(QIODevice::ReadWrite));
        QVERIFY(file.setFileTime(QDateTime(QDate(2024, 3, 20), QTime(13, 45, 37)), QFileDevice::FileModificationTime));
        file.close();

        const QLocale previousLocale;
        auto config = KSharedConfig::openConfig()->group(QStringLiteral("View"));
        const ViewSettings previousSettings = ViewSettings::fromConfig(config);
        const auto restore = qScopeGuard([&] {
            previousSettings.writeConfig(config);
            QLocale::setDefault(previousLocale);
        });
        QLocale::setDefault(QLocale(QStringLiteral("fa_IR")));
        ViewSettings settings = previousSettings;
        settings.dateFormat = QStringLiteral("locale");
        settings.showSeconds = false;
        settings.writeConfig(config);

        FolderView view;
        view.setUrl(QUrl::fromLocalFile(directory.path()));
        QTRY_COMPARE(view.itemCount(), 1);
        auto *details = qobject_cast<QTreeView *>(view.currentView());
        QVERIFY(details);
        const QModelIndex date = details->model()->index(0, KDirModel::ModifiedTime);
        QTRY_COMPARE(date.data().toString(), LocalizedDates::dateTime(QFileInfo(file.fileName()).lastModified()));
        QVERIFY(date.data().toString().contains(QString::fromUtf8("۱۴۰۳")));

        settings.dateFormat = QStringLiteral("iso");
        settings.writeConfig(config);
        view.reloadMetadata();
        QCOMPARE(date.data().toString(), QStringLiteral("2024-03-20 13:45"));
    }

    void preferencesActionAppliesSettingsToOpenPanes()
    {
        MainWindow window;
        window.openUrls({m_a, m_b});
        FolderView *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 2);

        QAction *preferences = window.actionCollection()->action(QStringLiteral("options_configure"));
        QVERIFY(preferences);
        preferences->trigger();
        auto *dialog = window.findChild<SettingsDialog *>();
        QVERIFY(dialog);
        auto *groupBy = dialog->findChild<QComboBox *>(QStringLiteral("kcfg_GroupBy"));
        QVERIFY(groupBy);
        groupBy->setCurrentIndex(Settings::EnumGroupBy::ext);
        auto *typeColumn = dialog->findChild<QCheckBox *>(QStringLiteral("kcfg_ShowTypeColumn"));
        QVERIFY(typeColumn);
        typeColumn->setChecked(false);
        QPushButton *ok = dialog->button(QDialogButtonBox::Ok);
        QVERIFY(ok);
        ok->click();

        QCOMPARE(view->groupBy(), QStringLiteral("ext"));
        auto *details = view->findChild<QTreeView *>();
        QVERIFY(details);
        QVERIFY(details->isColumnHidden(KDirModel::Type));
        QCOMPARE(window.inactivePane()->view()->groupBy(), QStringLiteral("ext"));
    }

    void copyToOtherPaneUsesKio()
    {
        MainWindow window;
        window.openUrls({m_a, m_b});
        FolderView *view = window.activePane()->view();
        QTRY_COMPARE(view->itemCount(), 2);

        const QUrl source = QUrl::fromLocalFile(m_root.filePath(QStringLiteral("a/file.txt")));
        FileOperations::copy({source}, window.inactivePane()->url(), &window);
        QTRY_VERIFY(QFile::exists(m_root.filePath(QStringLiteral("b/file.txt"))));
        QTRY_COMPARE(window.inactivePane()->view()->itemCount(), 1);
    }

    void stackedLayoutMeetsTabBarsInTheMiddle()
    {
        MainWindow window;
        window.openUrls({m_a, m_b});
        window.inactivePane()->openInNewTab(m_a);
        window.resize(800, 600);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        window.setStackedLayout(true);
        QVERIFY(window.isStackedLayout());
        Pane *top = window.activePane();
        Pane *bottom = window.inactivePane();
        auto *splitter = window.findChild<QSplitter *>(QStringLiteral("paneSplitter"));
        QVERIFY(splitter);
        QCOMPARE(splitter->orientation(), Qt::Vertical);
        QCOMPARE(splitter->widget(0), top);
        QCOMPARE(splitter->widget(1), bottom);

        QTRY_VERIFY(tabBarOf(top)->isVisible() && tabBarOf(bottom)->isVisible());
        QCOMPARE(tabBarOf(top)->shape(), QTabBar::RoundedSouth);
        QCOMPARE(tabBarOf(bottom)->shape(), QTabBar::RoundedNorth);
        // Top pane: view above tabs. Bottom pane: tabs above view.
        QTRY_VERIFY(globalTop(tabBarOf(top)) > globalTop(top->view()));
        QTRY_VERIFY(globalTop(tabBarOf(bottom)) < globalTop(bottom->view()));
        // Nothing but the splitter handle separates the two tab bars.
        const int gap = globalTop(tabBarOf(bottom)) - (globalTop(tabBarOf(top)) + tabBarOf(top)->height());
        QVERIFY2(gap >= 0 && gap <= splitter->handleWidth(), qPrintable(QString::number(gap)));

        window.setStackedLayout(false);
        QCOMPARE(splitter->orientation(), Qt::Horizontal);
        QCOMPARE(tabBarOf(top)->shape(), QTabBar::RoundedNorth);
        QTRY_VERIFY(globalTop(tabBarOf(top)) < globalTop(top->view()));
    }

    void stackedLayoutSurvivesRestart()
    {
        {
            MainWindow window;
            window.openUrls({m_a, m_b});
            window.setStackedLayout(true);
            window.close();
        }

        MainWindow restored;
        restored.openUrls({});
        QVERIFY(restored.isStackedLayout());
    }

    void sessionSurvivesRestart()
    {
        {
            MainWindow window;
            window.openUrls({m_a, m_b});
            window.activePane()->openInNewTab(QUrl::fromLocalFile(m_root.filePath(QStringLiteral("a/sub"))), true);
            QCOMPARE(window.activePane()->tabs()->count(), 3);
            window.close();
        }

        MainWindow restored;
        restored.openUrls({});
        // The first window restored an empty session (home), then added a and a/sub.
        const QList<QUrl> left = restored.activePane()->tabs()->urls();
        QVERIFY(left.contains(m_a));
        QVERIFY(left.contains(QUrl::fromLocalFile(m_root.filePath(QStringLiteral("a/sub")))));
        QCOMPARE(restored.activePane()->url().toLocalFile(), m_root.filePath(QStringLiteral("a/sub")));
        QVERIFY(restored.inactivePane()->tabs()->urls().contains(m_b));
    }

private:
    static QTabBar *tabBarOf(Pane *pane)
    {
        return pane->findChild<QTabBar *>();
    }

    static int globalTop(QWidget *widget)
    {
        return widget->mapToGlobal(QPoint(0, 0)).y();
    }

    QTemporaryDir m_root;
    QUrl m_a;
    QUrl m_b;
    QUrl m_c;
};

QTEST_MAIN(MainWindowTest)

#include "mainwindowtest.moc"
