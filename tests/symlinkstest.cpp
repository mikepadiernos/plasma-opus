// SPDX-License-Identifier: GPL-3.0-or-later

#include "symlinks.h"

#include <KIO/SimpleJob>
#include <KProtocolManager>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

namespace
{
bool put(const QString &path, const QByteArray &data)
{
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(data) == data.size();
}

QUrl local(const QTemporaryDir &dir, const QString &path)
{
    return QUrl::fromLocalFile(dir.filePath(path));
}
}

class SymlinksTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void resolvesLinkedFolder()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("real/sub")));
        QVERIFY(QFile::link(dir.filePath(QStringLiteral("real")), dir.filePath(QStringLiteral("link"))));

        const QUrl resolved = Symlinks::resolve(QUrl::fromLocalFile(dir.filePath(QStringLiteral("link/sub"))));
        QCOMPARE(resolved, QUrl::fromLocalFile(QDir(dir.filePath(QStringLiteral("real/sub"))).canonicalPath()));
    }

    void leavesOtherUrlsAlone()
    {
        const QUrl remote(QStringLiteral("sftp://host/link"));
        QCOMPARE(Symlinks::resolve(remote), remote);
        const QUrl missing = QUrl::fromLocalFile(QStringLiteral("/no/such/folder"));
        QCOMPARE(Symlinks::resolve(missing), missing);
    }

    void createsFileAndFolderLinks()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile file(dir.filePath(QStringLiteral("source")));
        QVERIFY(put(file.fileName(), "content"));
        QVERIFY(QDir(dir.path()).mkdir(QStringLiteral("folder")));
        const QUrl folder = QUrl::fromLocalFile(dir.path());
        QString error;
        KIO::SimpleJob *fileJob =
            Symlinks::create(QUrl::fromLocalFile(file.fileName()), folder, QStringLiteral("file link"), Symlinks::TargetForm::Absolute, error);
        QVERIFY2(fileJob, qPrintable(error));
        QVERIFY2(fileJob->exec(), qPrintable(fileJob->errorString()));
        QCOMPARE(QFileInfo(dir.filePath(QStringLiteral("file link"))).symLinkTarget(), file.fileName());
        KIO::SimpleJob *folderJob =
            Symlinks::create(local(dir, QStringLiteral("folder")), folder, QStringLiteral("folder link"), Symlinks::TargetForm::Absolute, error);
        QVERIFY2(folderJob, qPrintable(error));
        QVERIFY2(folderJob->exec(), qPrintable(folderJob->errorString()));
        QCOMPARE(QFileInfo(dir.filePath(QStringLiteral("folder link"))).symLinkTarget(), dir.filePath(QStringLiteral("folder")));
    }

    void defaultsBesideTheTargetWithTheConfiguredSuffix()
    {
        QCOMPARE(Symlinks::parentFolder(QUrl::fromLocalFile(QStringLiteral("/srv/data/file"))).adjusted(QUrl::StripTrailingSlash),
                 QUrl::fromLocalFile(QStringLiteral("/srv/data")));
        QCOMPARE(Symlinks::parentFolder(QUrl(QStringLiteral("sftp://host/srv/folder/"))).adjusted(QUrl::StripTrailingSlash),
                 QUrl(QStringLiteral("sftp://host/srv")));
        QCOMPARE(Symlinks::defaultName(QStringLiteral("file.txt"), QString()), QStringLiteral("file.txt - link"));
        QCOMPARE(Symlinks::defaultName(QStringLiteral("file.txt"), QStringLiteral(".lnk")), QStringLiteral("file.txt.lnk"));
    }

    void createsAbsoluteLinkInAnotherFolder()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("a")));
        QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("b/c")));
        const QString source = dir.filePath(QStringLiteral("a/source"));
        QVERIFY(put(source, "content"));
        QString error;
        KIO::SimpleJob *job =
            Symlinks::create(QUrl::fromLocalFile(source), local(dir, QStringLiteral("b/c")), QStringLiteral("link"), Symlinks::TargetForm::Absolute, error);
        QVERIFY2(job, qPrintable(error));
        QVERIFY2(job->exec(), qPrintable(job->errorString()));
        const QFileInfo link(dir.filePath(QStringLiteral("b/c/link")));
        QVERIFY(link.isSymLink());
        QCOMPARE(link.readSymLink(), source);
        QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("a/link"))));
    }

    void createsRelativeLinksThatResolveToTheTarget()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("a")));
        QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("b/c")));
        const QString source = dir.filePath(QStringLiteral("a/source"));
        QVERIFY(put(source, "content"));
        // A folder reached through a link resolves ".." from its real location.
        QVERIFY(QFile::link(dir.filePath(QStringLiteral("b/c")), dir.filePath(QStringLiteral("alias"))));
        const QUrl target = QUrl::fromLocalFile(source);
        const QString canonicalSource = QFileInfo(source).canonicalFilePath();

        struct Case {
            QString folder;
            QString expected;
        };
        const QList<Case> cases{
            {QStringLiteral("a"), QStringLiteral("source")},
            {QStringLiteral("b/c"), QStringLiteral("../../a/source")},
            {QStringLiteral("alias"), QStringLiteral("../../a/source")},
        };
        int index = 0;
        for (const Case &test : cases) {
            const QString name = QStringLiteral("link%1").arg(++index);
            const Symlinks::Plan plan = Symlinks::plan(target, local(dir, test.folder), name, Symlinks::TargetForm::Relative);
            QVERIFY2(plan.error.isEmpty(), qPrintable(plan.error));
            QCOMPARE(plan.target, test.expected);
            QString error;
            KIO::SimpleJob *job = Symlinks::create(target, local(dir, test.folder), name, Symlinks::TargetForm::Relative, error);
            QVERIFY2(job, qPrintable(error));
            QVERIFY2(job->exec(), qPrintable(job->errorString()));
            const QFileInfo link(dir.filePath(test.folder + QLatin1Char('/') + name));
            QCOMPARE(link.readSymLink(), test.expected);
            QCOMPARE(link.canonicalFilePath(), canonicalSource);
        }

        // A link inside the folder it points at.
        const QUrl folder = local(dir, QStringLiteral("a"));
        const Symlinks::Plan self = Symlinks::plan(folder, folder, QStringLiteral("self"), Symlinks::TargetForm::Relative);
        QVERIFY2(self.error.isEmpty(), qPrintable(self.error));
        QCOMPARE(self.target, QStringLiteral("."));
    }

    void plansRemoteLinksOnTheSameHost()
    {
        const QUrl target(QStringLiteral("sftp://user@Example.org/home/user/a%20file"));
        const QUrl folder(QStringLiteral("sftp://user@example.org/srv/links/"));
        if (!KProtocolManager::supportsLinking(folder)) {
            QSKIP("The sftp KIO worker isn't installed");
        }
        QVERIFY(Symlinks::supportsLinking(target));
        QCOMPARE(Symlinks::destinationError(target, folder), QString());
        const QString name = QStringLiteral("x#y%z");
        const Symlinks::Plan absolute = Symlinks::plan(target, folder, name);
        QVERIFY2(absolute.error.isEmpty(), qPrintable(absolute.error));
        QCOMPARE(absolute.target, QStringLiteral("/home/user/a file"));
        QCOMPARE(absolute.link.scheme(), QStringLiteral("sftp"));
        QCOMPARE(absolute.link.host(), QStringLiteral("example.org"));
        QCOMPARE(absolute.link.userName(), QStringLiteral("user"));
        QCOMPARE(absolute.link.path(), QStringLiteral("/srv/links/x#y%z"));
        QVERIFY(!absolute.link.hasFragment());
        const Symlinks::Plan relative = Symlinks::plan(target, folder, name, Symlinks::TargetForm::Relative);
        QCOMPARE(relative.target, QStringLiteral("../../home/user/a file"));
        QCOMPARE(Symlinks::plan(target, Symlinks::parentFolder(target), name, Symlinks::TargetForm::Relative).target, QStringLiteral("a file"));
    }

    void refusesUnsupportedLocationsAndCombinations()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QVERIFY(put(dir.filePath(QStringLiteral("source")), "content"));
        const QUrl target = local(dir, QStringLiteral("source"));
        const QUrl folder = QUrl::fromLocalFile(dir.path());
        const QUrl remote(QStringLiteral("sftp://host/home/user/file"));
        const QUrl remoteFolder(QStringLiteral("sftp://host/home/user/"));

        for (const QUrl &url : {QUrl(QStringLiteral("http://host/file")), QUrl(QStringLiteral("trash:/file")), QUrl(QStringLiteral("desktop:/file")), QUrl()}) {
            QVERIFY2(!Symlinks::supportsLinking(url), qPrintable(url.toString()));
        }
        const QList<std::pair<QUrl, QUrl>> refused{
            {target, remoteFolder},
            {remote, folder},
            {remote, QUrl(QStringLiteral("sftp://other/home/user/"))},
            {remote, QUrl(QStringLiteral("sftp://someone@host/home/user/"))},
            {remote, QUrl(QStringLiteral("sftp://host:2222/home/user/"))},
            {remote, QUrl(QStringLiteral("fish://host/home/user/"))},
            {QUrl(QStringLiteral("http://host/file")), QUrl(QStringLiteral("http://host/"))},
            {QUrl(QStringLiteral("trash:/file")), QUrl(QStringLiteral("trash:/"))},
            {target, QUrl()},
            {target, local(dir, QStringLiteral("missing"))},
            {target, target},
            {QUrl::fromLocalFile(QStringLiteral("relative")), folder},
            {local(dir, QStringLiteral("missing")), folder},
        };
        for (const auto &[from, to] : refused) {
            QString error;
            QVERIFY2(!Symlinks::create(from, to, QStringLiteral("link"), Symlinks::TargetForm::Absolute, error), qPrintable(to.toString()));
            QVERIFY(!error.isEmpty());
            QCOMPARE(Symlinks::destinationError(from, to), error);
        }
        QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("link"))));
    }

    void refusesInvalidRequestsAndExistingDestinations()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile file(dir.filePath(QStringLiteral("source")));
        QVERIFY(put(file.fileName(), "original"));
        const QUrl target = QUrl::fromLocalFile(file.fileName());
        const QUrl folder = QUrl::fromLocalFile(dir.path());
        QString error;
        for (const QString &name : {QString(), QStringLiteral("."), QStringLiteral(".."), QStringLiteral("other/link"), QString(QChar::Null)}) {
            QVERIFY(!Symlinks::create(target, folder, name, Symlinks::TargetForm::Absolute, error));
            QVERIFY(!error.isEmpty());
            QCOMPARE(Symlinks::nameError(name), error);
        }

        const QString dangling = dir.filePath(QStringLiteral("dangling"));
        QVERIFY(QFile::link(dir.filePath(QStringLiteral("missing")), dangling));
        for (const QString &name : {QStringLiteral("source"), QStringLiteral("dangling")}) {
            QVERIFY(!Symlinks::create(target, folder, name, Symlinks::TargetForm::Absolute, error));
            QVERIFY(error.contains(name));
        }

        // An item that appears after planning is still never replaced.
        KIO::SimpleJob *job = Symlinks::create(target, folder, QStringLiteral("late"), Symlinks::TargetForm::Absolute, error);
        QVERIFY2(job, qPrintable(error));
        QVERIFY(put(dir.filePath(QStringLiteral("late")), "late"));
        QVERIFY(!job->exec());
        QVERIFY(job->error());

        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.readAll(), QByteArray("original"));
        QVERIFY(QFileInfo(dangling).isSymLink());
        QFile late(dir.filePath(QStringLiteral("late")));
        QVERIFY(late.open(QIODevice::ReadOnly));
        QCOMPARE(late.readAll(), QByteArray("late"));
        QVERIFY(!QFileInfo(late.fileName()).isSymLink());
    }
};

QTEST_GUILESS_MAIN(SymlinksTest)

#include "symlinkstest.moc"
