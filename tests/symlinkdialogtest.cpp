// SPDX-License-Identifier: GPL-3.0-or-later

#include "symlinkdialog.h"
#include "settings.h"
#include "symlinks.h"

#include <KProtocolManager>
#include <KUrlRequester>

#include <QCheckBox>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QScopeGuard>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

class SymlinkDialogTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void init()
    {
        Settings::setSymlinkSuffix(QString());
    }

    void defaultsBesideTheTargetWithAnAbsolutePath()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString source = dir.filePath(QStringLiteral("source.txt"));
        QFile file(source);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
        const QUrl target = QUrl::fromLocalFile(source);

        SymlinkDialog dialog(target, QStringLiteral("source.txt"), QUrl());
        auto *name = dialog.findChild<QLineEdit *>(QStringLiteral("symlinkName"));
        auto *beside = dialog.findChild<QRadioButton *>(QStringLiteral("symlinkBesideTarget"));
        auto *other = dialog.findChild<QRadioButton *>(QStringLiteral("symlinkOtherPane"));
        auto *requester = dialog.findChild<KUrlRequester *>(QStringLiteral("symlinkFolder"));
        auto *relative = dialog.findChild<QCheckBox *>(QStringLiteral("symlinkRelative"));
        auto *preview = dialog.findChild<QLabel *>(QStringLiteral("symlinkPreview"));
        auto *create = dialog.findChild<QPushButton *>(QStringLiteral("symlinkCreate"));
        QVERIFY(name && beside && other && requester && relative && preview && create);
        QCOMPARE(name->text(), QStringLiteral("source.txt - link"));
        QVERIFY(beside->isChecked());
        QVERIFY(!other->isEnabled());
        QVERIFY(!requester->isEnabled());
        QVERIFY(!relative->isChecked());
        QVERIFY(!dialog.isRelative());
        QCOMPARE(dialog.folder().adjusted(QUrl::StripTrailingSlash), QUrl::fromLocalFile(dir.path()));
        QVERIFY(create->isEnabled());
        QVERIFY(preview->text().contains(source));

        relative->setChecked(true);
        QVERIFY(dialog.isRelative());
        QVERIFY(preview->text().endsWith(QStringLiteral(": source.txt")));

        // The target's own name already exists beside it.
        name->setText(QStringLiteral("source.txt"));
        QVERIFY(!create->isEnabled());
        QVERIFY(preview->text().contains(QStringLiteral("already exists")));
        name->setText(QStringLiteral("a/b"));
        QVERIFY(!create->isEnabled());
        name->setText(QStringLiteral("alias"));
        QVERIFY(create->isEnabled());

        Settings::setSymlinkSuffix(QStringLiteral(".lnk"));
        SymlinkDialog suffixed(target, QStringLiteral("source.txt"), QUrl());
        QCOMPARE(suffixed.name(), QStringLiteral("source.txt.lnk"));
    }

    void offersCompatibleFolders()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("a")));
        QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("b/c")));
        const QString source = dir.filePath(QStringLiteral("a/source"));
        QFile file(source);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
        const QUrl target = QUrl::fromLocalFile(source);
        const QUrl otherFolder = QUrl::fromLocalFile(dir.filePath(QStringLiteral("b")));

        SymlinkDialog dialog(target, QStringLiteral("source"), otherFolder);
        auto *other = dialog.findChild<QRadioButton *>(QStringLiteral("symlinkOtherPane"));
        auto *choose = dialog.findChild<QRadioButton *>(QStringLiteral("symlinkChooseFolder"));
        auto *requester = dialog.findChild<KUrlRequester *>(QStringLiteral("symlinkFolder"));
        auto *relative = dialog.findChild<QCheckBox *>(QStringLiteral("symlinkRelative"));
        auto *preview = dialog.findChild<QLabel *>(QStringLiteral("symlinkPreview"));
        auto *create = dialog.findChild<QPushButton *>(QStringLiteral("symlinkCreate"));
        QVERIFY(other->isEnabled());
        other->setChecked(true);
        QCOMPARE(dialog.folder(), otherFolder);
        relative->setChecked(true);
        QVERIFY(preview->text().endsWith(QStringLiteral(": ../a/source")));
        QVERIFY(create->isEnabled());

        choose->setChecked(true);
        QVERIFY(requester->isEnabled());
        requester->setUrl(QUrl::fromLocalFile(dir.filePath(QStringLiteral("b/c"))));
        QCOMPARE(dialog.folder(), QUrl::fromLocalFile(dir.filePath(QStringLiteral("b/c"))));
        QVERIFY(preview->text().endsWith(QStringLiteral(": ../../a/source")));
        QVERIFY(create->isEnabled());
        requester->setUrl(QUrl(QStringLiteral("sftp://host/srv/")));
        QVERIFY(!create->isEnabled());
        QCOMPARE(preview->text(), Symlinks::destinationError(target, QUrl(QStringLiteral("sftp://host/srv/"))));
        requester->setUrl(QUrl::fromLocalFile(dir.filePath(QStringLiteral("missing"))));
        QVERIFY(!create->isEnabled());
        requester->clear();
        QVERIFY(!create->isEnabled());

        // A remote pane can't hold a link to a local target.
        SymlinkDialog remotePane(target, QStringLiteral("source"), QUrl(QStringLiteral("sftp://host/srv/")));
        auto *remoteOther = remotePane.findChild<QRadioButton *>(QStringLiteral("symlinkOtherPane"));
        QVERIFY(!remoteOther->isEnabled());
        QVERIFY(!remoteOther->toolTip().isEmpty());
    }

    void remoteTargetsAcceptOnlyTheSameHost()
    {
        const QUrl target(QStringLiteral("sftp://host/home/user/file"));
        if (!KProtocolManager::supportsLinking(target)) {
            QSKIP("The sftp KIO worker isn't installed");
        }
        SymlinkDialog dialog(target, QStringLiteral("file"), QUrl(QStringLiteral("sftp://other/srv/")));
        QVERIFY(!dialog.findChild<QRadioButton *>(QStringLiteral("symlinkOtherPane"))->isEnabled());
        auto *create = dialog.findChild<QPushButton *>(QStringLiteral("symlinkCreate"));
        auto *preview = dialog.findChild<QLabel *>(QStringLiteral("symlinkPreview"));
        QVERIFY(create->isEnabled());
        QVERIFY(preview->text().endsWith(QStringLiteral(": /home/user/file")));

        SymlinkDialog samePane(target, QStringLiteral("file"), QUrl(QStringLiteral("sftp://host/srv/links/")));
        auto *other = samePane.findChild<QRadioButton *>(QStringLiteral("symlinkOtherPane"));
        QVERIFY(other->isEnabled());
        other->setChecked(true);
        samePane.findChild<QCheckBox *>(QStringLiteral("symlinkRelative"))->setChecked(true);
        QVERIFY(samePane.findChild<QLabel *>(QStringLiteral("symlinkPreview"))->text().endsWith(QStringLiteral(": ../../home/user/file")));
    }
};

QTEST_MAIN(SymlinkDialogTest)
#include "symlinkdialogtest.moc"
