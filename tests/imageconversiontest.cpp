// SPDX-License-Identifier: GPL-3.0-or-later

#include "imageconversion.h"
#include "umaskfixture.h"

#include <QDir>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

#include <unistd.h>

class ImageConversionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void uniquePreviewAndRaceDoNotOverwrite()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QVERIFY(QDir().mkdir(dir.filePath(QStringLiteral("other"))));
        QImage image(40, 20, QImage::Format_RGB32);
        image.fill(Qt::red);
        const QString first = dir.filePath(QStringLiteral("photo.png"));
        const QString second = dir.filePath(QStringLiteral("other/photo.png"));
        QVERIFY(image.save(first));
        QVERIFY(image.save(second));
        QFile existing(dir.filePath(QStringLiteral("photo-1.png")));
        QVERIFY(existing.open(QIODevice::WriteOnly));
        QCOMPARE(existing.write("existing"), 8);
        existing.close();
        const QByteArray dangling = QFile::encodeName(dir.filePath(QStringLiteral("photo-2.png")));
        QCOMPARE(::symlink("missing", dangling.constData()), 0);
        const auto plan = ImageConversion::plan({QUrl::fromLocalFile(first), QUrl::fromLocalFile(second)}, dir.path(), "png");
        QVERIFY2(plan.error.isEmpty(), qPrintable(plan.error));
        QCOMPARE(plan.entries.size(), 2);
        QCOMPARE(plan.entries.at(0).destination, dir.filePath(QStringLiteral("photo-3.png")));
        QCOMPARE(plan.entries.at(1).destination, dir.filePath(QStringLiteral("photo-4.png")));

        QFile raced(plan.entries.at(0).destination);
        QVERIFY(raced.open(QIODevice::WriteOnly | QIODevice::NewOnly));
        QCOMPARE(raced.write("raced"), 5);
        raced.close();
        const auto failures = ImageConversion::convert(plan.entries, "png", QSize(10, 10), 90);
        QCOMPARE(failures.size(), 1);
        QCOMPARE(failures.first().entry.destination, plan.entries.first().destination);
        QVERIFY(raced.open(QIODevice::ReadOnly));
        QCOMPARE(raced.readAll(), QByteArray("raced"));
        QCOMPARE(QImageReader(plan.entries.at(1).destination).size(), QSize(10, 5));
        QVERIFY(existing.open(QIODevice::ReadOnly));
        QCOMPARE(existing.readAll(), QByteArray("existing"));
        QCOMPARE(QImageReader(first).size(), QSize(40, 20));
    }

    void publishedOutputsUseTheDefaultCreationMode_data()
    {
        QTest::addColumn<int>("mask");
        QTest::addColumn<int>("expected");
        QTest::newRow("022") << 022 << 0644;
        QTest::newRow("077") << 077 << 0600;
        QTest::newRow("002") << 02 << 0664;
    }

    void publishedOutputsUseTheDefaultCreationMode()
    {
        QFETCH(int, mask);
        QFETCH(int, expected);
        const ScopedUmask scoped(mask);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QImage image(16, 16, QImage::Format_RGB32);
        image.fill(Qt::blue);
        const QString source = dir.filePath(QStringLiteral("source.png"));
        QVERIFY(image.save(source));
        QVERIFY(QDir().mkdir(dir.filePath(QStringLiteral("out"))));
        const auto plan = ImageConversion::plan({QUrl::fromLocalFile(source)}, dir.filePath(QStringLiteral("out")), "png");
        QVERIFY2(plan.error.isEmpty(), qPrintable(plan.error));
        QVERIFY(ImageConversion::convert(plan.entries, "png", {}, 90).isEmpty());
        QCOMPARE(fileMode(plan.entries.first().destination), expected);
        QCOMPARE(QDir(dir.filePath(QStringLiteral("out"))).entryList(QDir::Files | QDir::Hidden), QStringList{QStringLiteral("source.png")});
    }

    void invalidInputAndOutputAreReported()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const auto remote = ImageConversion::plan({QUrl(QStringLiteral("https://example.com/image.png"))}, dir.path(), "png");
        QVERIFY(!remote.error.isEmpty());
        const auto badDirectory =
            ImageConversion::plan({QUrl::fromLocalFile(dir.filePath(QStringLiteral("missing.png")))}, dir.filePath(QStringLiteral("missing")), "png");
        QVERIFY(!badDirectory.error.isEmpty());
        const auto plan = ImageConversion::plan({QUrl::fromLocalFile(dir.filePath(QStringLiteral("missing.png")))}, dir.path(), "png");
        QVERIFY(plan.error.isEmpty());
        const auto failures = ImageConversion::convert(plan.entries, "png", {}, 90);
        QCOMPARE(failures.size(), 1);
        QVERIFY(!failures.first().error.isEmpty());
        QVERIFY(!QFileInfo::exists(plan.entries.first().destination));
    }

    void jpegQualityAndDimensions()
    {
        if (!QImageWriter::supportedImageFormats().contains("jpeg")) {
            QSKIP("No JPEG writer is installed");
        }
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QImage image(128, 64, QImage::Format_RGB32);
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                image.setPixelColor(x, y, QColor((x * 53 + y * 17) % 256, (x * 11 + y * 31) % 256, (x * 7 + y * 61) % 256));
            }
        }
        const QString source = dir.filePath(QStringLiteral("source.png"));
        QVERIFY(image.save(source));
        auto plan = ImageConversion::plan({QUrl::fromLocalFile(source)}, dir.path(), "jpeg");
        QVERIFY(plan.error.isEmpty());
        QVERIFY(ImageConversion::convert(plan.entries, "jpeg", QSize(64, 64), 10).isEmpty());
        const QString lowQuality = plan.entries.first().destination;
        plan = ImageConversion::plan({QUrl::fromLocalFile(source)}, dir.path(), "jpeg");
        QVERIFY(plan.error.isEmpty());
        QVERIFY(ImageConversion::convert(plan.entries, "jpeg", QSize(64, 64), 95).isEmpty());
        const QString highQuality = plan.entries.first().destination;
        QCOMPARE(QImageReader(highQuality).size(), QSize(64, 32));
        QVERIFY(QFileInfo(highQuality).size() > QFileInfo(lowQuality).size());
    }
};

QTEST_MAIN(ImageConversionTest)
#include "imageconversiontest.moc"
