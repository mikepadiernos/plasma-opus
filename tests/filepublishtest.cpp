// SPDX-License-Identifier: GPL-3.0-or-later

#include "filepublish.h"
#include "umaskfixture.h"

#include <QFile>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

class FilePublishTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
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
};

QTEST_GUILESS_MAIN(FilePublishTest)
#include "filepublishtest.moc"
