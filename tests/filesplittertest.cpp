// SPDX-License-Identifier: GPL-3.0-or-later

#include "filesplitter.h"
#include "umaskfixture.h"

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

QByteArray read(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}
}

class FileSplitterTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void splitAndCombine()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString source = dir.filePath(QStringLiteral("source"));
        QVERIFY(put(source, "abcdefgh"));
        const auto split = FileSplitter::split(source, dir.filePath(QStringLiteral("part")), 3);
        QVERIFY2(split.error.isEmpty(), qPrintable(split.error));
        QCOMPARE(split.outputs.size(), 3);
        QCOMPARE(read(split.outputs.at(0)), QByteArray("abc"));
        QCOMPARE(read(split.outputs.at(1)), QByteArray("def"));
        QCOMPARE(read(split.outputs.at(2)), QByteArray("gh"));
        const auto combined = FileSplitter::combine(split.outputs, dir.filePath(QStringLiteral("combined")));
        QVERIFY2(combined.error.isEmpty(), qPrintable(combined.error));
        QCOMPARE(read(combined.outputs.first()), QByteArray("abcdefgh"));
    }

    void completeOutputsUseTheDefaultCreationMode_data()
    {
        QTest::addColumn<int>("mask");
        QTest::addColumn<int>("expected");
        QTest::newRow("022") << 022 << 0644;
        QTest::newRow("077") << 077 << 0600;
        QTest::newRow("002") << 02 << 0664;
    }

    void completeOutputsUseTheDefaultCreationMode()
    {
        QFETCH(int, mask);
        QFETCH(int, expected);
        const ScopedUmask scoped(mask);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString source = dir.filePath(QStringLiteral("source"));
        QVERIFY(put(source, "abcdefgh"));
        const auto split = FileSplitter::split(source, dir.filePath(QStringLiteral("part")), 3);
        QVERIFY2(split.error.isEmpty(), qPrintable(split.error));
        QCOMPARE(split.outputs.size(), 3);
        for (const QString &part : split.outputs) {
            QCOMPARE(fileMode(part), expected);
        }
        const auto combined = FileSplitter::combine(split.outputs, dir.filePath(QStringLiteral("combined")));
        QVERIFY2(combined.error.isEmpty(), qPrintable(combined.error));
        QCOMPARE(fileMode(combined.outputs.first()), expected);
    }

    void incompleteOutputsStayOwnerOnly()
    {
        const ScopedUmask scoped(02);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString source = dir.filePath(QStringLiteral("source"));
        QVERIFY(put(source, QByteArray(2 * 1024 * 1024, 'x')));
        int checks = 0;
        const auto split = FileSplitter::split(source, dir.filePath(QStringLiteral("part")), 2 * 1024 * 1024, [&checks] {
            return ++checks == 3;
        });
        QVERIFY(!split.error.isEmpty());
        QCOMPARE(split.outputs.size(), 1);
        QCOMPARE(fileMode(split.outputs.first()), 0600);
        checks = 0;
        const auto combined = FileSplitter::combine({source}, dir.filePath(QStringLiteral("combined")), [&checks] {
            return ++checks == 3;
        });
        QVERIFY(!combined.error.isEmpty());
        QCOMPARE(fileMode(combined.outputs.first()), 0600);
    }

    void emptyAndExactSize()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString source = dir.filePath(QStringLiteral("source"));
        QVERIFY(put(source, ""));
        auto result = FileSplitter::split(source, dir.filePath(QStringLiteral("empty")), 3);
        QVERIFY(result.error.isEmpty());
        QCOMPARE(result.outputs.size(), 1);
        QCOMPARE(QFileInfo(result.outputs.first()).size(), 0);
        QVERIFY(put(source, "abcdef"));
        result = FileSplitter::split(source, dir.filePath(QStringLiteral("exact")), 3);
        QVERIFY(result.error.isEmpty());
        QCOMPARE(result.outputs.size(), 2);
    }

    void refusesOverwriteAndReportsPartialOutputs()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString source = dir.filePath(QStringLiteral("source"));
        const QString prefix = dir.filePath(QStringLiteral("part"));
        QVERIFY(put(source, "abcdef"));
        QVERIFY(put(prefix + QStringLiteral(".002"), "keep"));
        const auto split = FileSplitter::split(source, prefix, 3);
        QVERIFY(!split.error.isEmpty());
        QCOMPARE(split.outputs, QStringList{prefix + QStringLiteral(".001")});
        QCOMPARE(read(prefix + QStringLiteral(".002")), QByteArray("keep"));
        const auto combined = FileSplitter::combine({source}, source);
        QVERIFY(!combined.error.isEmpty());
        QVERIFY(combined.outputs.isEmpty());
        QCOMPARE(read(source), QByteArray("abcdef"));
    }

    void rejectsInvalidInputsAndCancels()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString source = dir.filePath(QStringLiteral("source"));
        QVERIFY(put(source, "abcdef"));
        QVERIFY(!FileSplitter::split(source, dir.filePath(QStringLiteral("part")), 0).error.isEmpty());
        QVERIFY(!FileSplitter::combine({source, dir.filePath(QStringLiteral("missing"))}, dir.filePath(QStringLiteral("output"))).error.isEmpty());
        QVERIFY(!QFile::exists(dir.filePath(QStringLiteral("output"))));
        int checks = 0;
        const auto canceled = FileSplitter::split(source, dir.filePath(QStringLiteral("part")), 3, [&checks] {
            return ++checks == 3;
        });
        QVERIFY(!canceled.error.isEmpty());
        QCOMPARE(canceled.outputs.size(), 1);
        QCOMPARE(read(canceled.outputs.first()), QByteArray("abc"));
    }

    void preservesOrderAcrossBufferBoundaries()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString source = dir.filePath(QStringLiteral("source"));
        const QByteArray data = QByteArray(1024 * 1024, 'a') + QByteArray(17, 'b');
        QVERIFY(put(source, data));
        const auto split = FileSplitter::split(source, dir.filePath(QStringLiteral("part")), 1024 * 1024 + 1);
        QVERIFY(split.error.isEmpty());
        QCOMPARE(split.outputs.size(), 2);
        const auto combined = FileSplitter::combine(split.outputs, dir.filePath(QStringLiteral("combined")));
        QVERIFY(combined.error.isEmpty());
        QCOMPARE(read(combined.outputs.first()), data);
        const auto reversed = FileSplitter::combine({split.outputs.at(1), split.outputs.at(0)}, dir.filePath(QStringLiteral("reversed")));
        QVERIFY(reversed.error.isEmpty());
        QCOMPARE(read(reversed.outputs.first()), QByteArray(16, 'b') + QByteArray(1024 * 1024, 'a') + QByteArray(1, 'b'));
    }

    void combineCancellationReportsOutput()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString source = dir.filePath(QStringLiteral("source"));
        const QString output = dir.filePath(QStringLiteral("output"));
        QVERIFY(put(source, QByteArray(2 * 1024 * 1024, 'x')));
        int checks = 0;
        const auto result = FileSplitter::combine({source}, output, [&checks] {
            return ++checks == 3;
        });
        QVERIFY(!result.error.isEmpty());
        QCOMPARE(result.outputs, QStringList{output});
        QCOMPARE(QFileInfo(output).size(), 1024 * 1024);
        QCOMPARE(QFileInfo(source).size(), 2 * 1024 * 1024);
    }
};

QTEST_GUILESS_MAIN(FileSplitterTest)
#include "filesplittertest.moc"
