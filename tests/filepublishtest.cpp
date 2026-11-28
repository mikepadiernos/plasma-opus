// SPDX-License-Identifier: GPL-3.0-or-later

#include "filepublish.h"
#include "umaskfixture.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>

#include <cerrno>
#include <memory>
#include <unistd.h>

using FilePublish::Method;
Q_DECLARE_METATYPE(FilePublish::Method)

namespace
{
int failWith(int error)
{
    errno = error;
    return -1;
}

QByteArray read(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

// A fully written private temporary file in dir, as callers prepare it before publishing.
std::unique_ptr<QTemporaryFile> prepared(const QTemporaryDir &dir, const QByteArray &content)
{
    auto file = std::make_unique<QTemporaryFile>(dir.filePath(QStringLiteral(".publish-XXXXXX")));
    if (!file->open() || file->write(content) != content.size() || !file->flush() || !FilePublish::applyDefaultMode(file->handle()).isEmpty()) {
        return {};
    }
    return file;
}

// Forces the publication path: renames unsupported for Link, and links unsupported too for Copy.
void force(Method method, int renameError = EINVAL, int linkError = EPERM)
{
    FilePublish::Testing::Syscalls hooks;
    if (method != Method::Rename) {
        hooks.rename = [renameError](const char *, const char *) {
            return failWith(renameError);
        };
    }
    if (method == Method::Copy) {
        hooks.link = [linkError](const char *, const char *) {
            return failWith(linkError);
        };
    }
    FilePublish::Testing::setSyscalls(hooks);
}
}

class FilePublishTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void init()
    {
        FilePublish::Testing::setSyscalls({});
    }

    void cleanup()
    {
        FilePublish::Testing::setSyscalls({});
    }

    void parsesStatusUmask_data()
    {
        QTest::addColumn<QByteArray>("status");
        QTest::addColumn<int>("expected");
        QTest::newRow("typical") << QByteArray("Name:\tcat\nUmask:\t0022\nState:\tR (running)\n") << 022;
        QTest::newRow("private") << QByteArray("Name:\tcat\nUmask:\t0077\n") << 077;
        QTest::newRow("group") << QByteArray("Umask:\t0002") << 02;
        QTest::newRow("zero") << QByteArray("Name:\tcat\nUmask:\t0000\n") << 0;
        QTest::newRow("spaces") << QByteArray("Umask:   027  \n") << 027;
        QTest::newRow("missing") << QByteArray("Name:\tcat\nState:\tR (running)\n") << -1;
        QTest::newRow("empty value") << QByteArray("Umask:\t\n") << -1;
        QTest::newRow("not octal") << QByteArray("Umask:\t0089\n") << -1;
        QTest::newRow("out of range") << QByteArray("Umask:\t01777\n") << -1;
        QTest::newRow("prefix only") << QByteArray("XUmask:\t0022\n") << -1;
    }

    void parsesStatusUmask()
    {
        QFETCH(QByteArray, status);
        QFETCH(int, expected);
        const std::optional<mode_t> mask = FilePublish::parseStatusUmask(status);
        QCOMPARE(mask ? int(*mask) : -1, expected);
    }

    void defaultModeFollowsTheUmask_data()
    {
        QTest::addColumn<int>("mask");
        QTest::addColumn<int>("expected");
        QTest::newRow("022") << 022 << 0644;
        QTest::newRow("077") << 077 << 0600;
        QTest::newRow("002") << 02 << 0664;
        QTest::newRow("027") << 027 << 0640;
        QTest::newRow("000") << 0 << 0666;
    }

    void defaultModeFollowsTheUmask()
    {
        QFETCH(int, mask);
        QFETCH(int, expected);
        const ScopedUmask scoped(mask);
        QCOMPARE(int(FilePublish::currentUmask()), mask);
        QCOMPARE(int(FilePublish::defaultFileMode()), expected);
        // Reading the umask must not change it.
        QCOMPARE(int(::umask(mask)), mask);
    }

    void appliesTheModeToAnOpenFile_data()
    {
        defaultModeFollowsTheUmask_data();
    }

    void appliesTheModeToAnOpenFile()
    {
        QFETCH(int, mask);
        QFETCH(int, expected);
        const ScopedUmask scoped(mask);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile file(dir.filePath(QStringLiteral("private")));
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::NewOnly, QFileDevice::ReadOwner | QFileDevice::WriteOwner));
        QCOMPARE(fileMode(file.fileName()), 0600);
        QCOMPARE(file.write("content"), 7);
        QVERIFY(file.flush());
        QCOMPARE(FilePublish::applyDefaultMode(file.handle()), QString());
        file.close();
        QCOMPARE(fileMode(file.fileName()), expected);
    }

    void reportsAnInvalidDescriptor()
    {
        QVERIFY(!FilePublish::applyDefaultMode(-1).isEmpty());
    }

    void publishes_data()
    {
        QTest::addColumn<Method>("method");
        QTest::addColumn<int>("renameError");
        QTest::addColumn<int>("linkError");
        QTest::addColumn<int>("mask");
        QTest::addColumn<int>("expected");
        QTest::newRow("rename") << Method::Rename << 0 << 0 << 022 << 0644;
        QTest::newRow("link after EINVAL") << Method::Link << EINVAL << 0 << 022 << 0644;
        QTest::newRow("link after ENOSYS") << Method::Link << ENOSYS << 0 << 077 << 0600;
        QTest::newRow("link after EOPNOTSUPP") << Method::Link << EOPNOTSUPP << 0 << 02 << 0664;
        QTest::newRow("copy after EPERM") << Method::Copy << EINVAL << EPERM << 022 << 0644;
        QTest::newRow("copy after EOPNOTSUPP") << Method::Copy << EINVAL << EOPNOTSUPP << 077 << 0600;
        QTest::newRow("copy after ENOSYS") << Method::Copy << ENOSYS << ENOSYS << 02 << 0664;
        QTest::newRow("copy after EXDEV") << Method::Copy << EOPNOTSUPP << EXDEV << 022 << 0644;
    }

    void publishes()
    {
        QFETCH(Method, method);
        QFETCH(int, renameError);
        QFETCH(int, linkError);
        QFETCH(int, mask);
        QFETCH(int, expected);
        const ScopedUmask scoped(mask);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QByteArray content = QByteArray(200 * 1024, 'x') + "end";
        const auto temporary = prepared(dir, content);
        QVERIFY(temporary);
        force(method, renameError, linkError);
        const QString destination = dir.filePath(QStringLiteral("published"));
        const auto published = FilePublish::publishNoReplace(temporary->handle(), temporary->fileName(), destination);
        QCOMPARE(published.error, QString());
        QCOMPARE(published.method, method);
        temporary->setAutoRemove(false);
        QCOMPARE(read(destination), content);
        QCOMPARE(fileMode(destination), expected);
        QVERIFY(!QFileInfo::exists(temporary->fileName()));
        QCOMPARE(QDir(dir.path()).entryList(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot), QStringList{QStringLiteral("published")});
    }

    void neverReplacesAnExistingEntry_data()
    {
        QTest::addColumn<Method>("method");
        QTest::addColumn<bool>("dangling");
        for (const Method method : {Method::Rename, Method::Link, Method::Copy}) {
            const char *name = method == Method::Rename ? "rename" : method == Method::Link ? "link" : "copy";
            QTest::addRow("%s, file", name) << method << false;
            QTest::addRow("%s, dangling link", name) << method << true;
        }
    }

    void neverReplacesAnExistingEntry()
    {
        QFETCH(Method, method);
        QFETCH(bool, dangling);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString destination = dir.filePath(QStringLiteral("published"));
        const QString target = dir.filePath(QStringLiteral("target"));
        if (dangling) {
            QVERIFY(QFile::link(target, destination));
        } else {
            QFile existing(destination);
            QVERIFY(existing.open(QIODevice::WriteOnly));
            QCOMPARE(existing.write("keep"), 4);
        }
        const auto temporary = prepared(dir, "new");
        QVERIFY(temporary);
        force(method);
        const auto published = FilePublish::publishNoReplace(temporary->handle(), temporary->fileName(), destination);
        QVERIFY(!published.error.isEmpty());
        QCOMPARE(published.method, method);
        QVERIFY(QFileInfo::exists(temporary->fileName()));
        if (dangling) {
            QCOMPARE(QFileInfo(destination).symLinkTarget(), target);
            QVERIFY(!QFileInfo::exists(target));
        } else {
            QCOMPARE(read(destination), QByteArray("keep"));
        }
    }

    void otherErrorsDoNotFallBack()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString destination = dir.filePath(QStringLiteral("published"));
        const auto temporary = prepared(dir, "new");
        QVERIFY(temporary);
        int links = 0;
        FilePublish::Testing::Syscalls hooks;
        hooks.rename = [](const char *, const char *) {
            return failWith(EACCES);
        };
        hooks.link = [&links](const char *, const char *) {
            ++links;
            return failWith(EACCES);
        };
        FilePublish::Testing::setSyscalls(hooks);
        auto published = FilePublish::publishNoReplace(temporary->handle(), temporary->fileName(), destination);
        QVERIFY(!published.error.isEmpty());
        QCOMPARE(published.method, Method::Rename);
        QCOMPARE(links, 0);

        hooks.rename = [](const char *, const char *) {
            return failWith(EINVAL);
        };
        FilePublish::Testing::setSyscalls(hooks);
        published = FilePublish::publishNoReplace(temporary->handle(), temporary->fileName(), destination);
        QVERIFY(!published.error.isEmpty());
        QCOMPARE(published.method, Method::Link);
        QCOMPARE(links, 1);
        QVERIFY(!QFileInfo::exists(destination));
        QVERIFY(QFileInfo::exists(temporary->fileName()));
    }

    void retriedLinkThatSucceededCounts()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString destination = dir.filePath(QStringLiteral("published"));
        const auto temporary = prepared(dir, "new");
        QVERIFY(temporary);
        FilePublish::Testing::Syscalls hooks;
        hooks.rename = [](const char *, const char *) {
            return failWith(EINVAL);
        };
        // As when a lost reply makes an NFS client retry a link the server already made.
        hooks.link = [](const char *from, const char *to) {
            return ::link(from, to) == 0 ? failWith(EEXIST) : -1;
        };
        FilePublish::Testing::setSyscalls(hooks);
        const auto published = FilePublish::publishNoReplace(temporary->handle(), temporary->fileName(), destination);
        QCOMPARE(published.error, QString());
        QCOMPARE(published.method, Method::Link);
        temporary->setAutoRemove(false);
        QCOMPARE(read(destination), QByteArray("new"));
        QVERIFY(!QFileInfo::exists(temporary->fileName()));
    }

    void copyStaysPrivateUntilComplete()
    {
        const ScopedUmask scoped(02);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString destination = dir.filePath(QStringLiteral("published"));
        const auto temporary = prepared(dir, QByteArray(100 * 1024, 'x'));
        QVERIFY(temporary);
        FilePublish::Testing::Syscalls hooks;
        hooks.rename = [](const char *, const char *) {
            return failWith(EINVAL);
        };
        hooks.link = [](const char *, const char *) {
            return failWith(EPERM);
        };
        QList<int> modes;
        hooks.write = [&modes, &destination](int fd, const void *data, size_t size) {
            modes.append(fileMode(destination));
            return ::write(fd, data, size);
        };
        FilePublish::Testing::setSyscalls(hooks);
        const auto published = FilePublish::publishNoReplace(temporary->handle(), temporary->fileName(), destination);
        QCOMPARE(published.error, QString());
        QCOMPARE(published.method, Method::Copy);
        temporary->setAutoRemove(false);
        QVERIFY(modes.size() >= 2);
        QCOMPARE(modes, QList<int>(modes.size(), 0600));
        QCOMPARE(fileMode(destination), 0664);
    }

    void failedCopyRemovesItsPartialFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString destination = dir.filePath(QStringLiteral("published"));
        const QByteArray content(100 * 1024, 'x');
        const auto temporary = prepared(dir, content);
        QVERIFY(temporary);
        FilePublish::Testing::Syscalls hooks;
        hooks.rename = [](const char *, const char *) {
            return failWith(EINVAL);
        };
        hooks.link = [](const char *, const char *) {
            return failWith(EPERM);
        };
        int writes = 0;
        hooks.write = [&writes](int fd, const void *data, size_t size) -> ssize_t {
            return ++writes == 1 ? ::write(fd, data, size) : failWith(ENOSPC);
        };
        FilePublish::Testing::setSyscalls(hooks);
        const auto published = FilePublish::publishNoReplace(temporary->handle(), temporary->fileName(), destination);
        QVERIFY(!published.error.isEmpty());
        QCOMPARE(published.method, Method::Copy);
        QCOMPARE(writes, 2);
        QVERIFY(!QFileInfo::exists(destination));
        QCOMPARE(read(temporary->fileName()), content);
    }

    void failedCopyKeepsAReplacement()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString destination = dir.filePath(QStringLiteral("published"));
        const QString moved = dir.filePath(QStringLiteral("moved"));
        const auto temporary = prepared(dir, "new");
        QVERIFY(temporary);
        FilePublish::Testing::Syscalls hooks;
        hooks.rename = [](const char *, const char *) {
            return failWith(EINVAL);
        };
        hooks.link = [](const char *, const char *) {
            return failWith(EPERM);
        };
        // Another program moves the partial file away and puts its own file at the destination.
        hooks.write = [&destination, &moved](int, const void *, size_t) -> ssize_t {
            QFile::rename(destination, moved);
            QFile other(destination);
            if (other.open(QIODevice::WriteOnly | QIODevice::NewOnly)) {
                other.write("other");
            }
            return failWith(EIO);
        };
        FilePublish::Testing::setSyscalls(hooks);
        const auto published = FilePublish::publishNoReplace(temporary->handle(), temporary->fileName(), destination);
        QVERIFY(!published.error.isEmpty());
        QCOMPARE(read(destination), QByteArray("other"));
        QVERIFY(QFileInfo::exists(moved));
        QVERIFY(QFileInfo::exists(temporary->fileName()));
    }
};

QTEST_GUILESS_MAIN(FilePublishTest)
#include "filepublishtest.moc"
