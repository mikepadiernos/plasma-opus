// SPDX-License-Identifier: GPL-3.0-or-later

#include "foldermetadata.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTest>

class FolderMetadataTest : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir m_dir;

private Q_SLOTS:
    void initTestCase()
    {
        QVERIFY(m_dir.isValid());
    }

    void metadataRoundTripsAndClears()
    {
        QVERIFY(FolderMetadata::setLabel(m_dir.path(), QStringLiteral("photo.jpg"), QStringLiteral("blue")));
        QVERIFY(FolderMetadata::setColor(m_dir.path(), QStringLiteral("photo.jpg"), QStringLiteral("#abc")));
        QVERIFY(FolderMetadata::setFolderColor(m_dir.path(), QStringLiteral("#123456")));
        QVERIFY(FolderMetadata::setRating(m_dir.path(), QStringLiteral("photo.jpg"), 5));
        QVERIFY(FolderMetadata::setNote(m_dir.path(), QStringLiteral("photo.jpg"), QStringLiteral("  favorite  ")));
        QVERIFY(FolderMetadata::togglePin(m_dir.path(), QStringLiteral("photo.jpg")));

        const QJsonObject metadata = FolderMetadata::read(m_dir.path());
        QCOMPARE(metadata.value(QStringLiteral("labels")).toObject().value(QStringLiteral("photo.jpg")).toString(), QStringLiteral("blue"));
        QCOMPARE(metadata.value(QStringLiteral("colors")).toObject().value(QStringLiteral("photo.jpg")).toString(), QStringLiteral("#abc"));
        QCOMPARE(metadata.value(QStringLiteral("color")).toString(), QStringLiteral("#123456"));
        QCOMPARE(metadata.value(QStringLiteral("ratings")).toObject().value(QStringLiteral("photo.jpg")).toInt(), 5);
        QCOMPARE(metadata.value(QStringLiteral("notes")).toObject().value(QStringLiteral("photo.jpg")).toString(), QStringLiteral("favorite"));
        QCOMPARE(metadata.value(QStringLiteral("pins")).toArray().at(0).toString(), QStringLiteral("photo.jpg"));

        QVERIFY(FolderMetadata::setRating(m_dir.path(), QStringLiteral("photo.jpg"), 0));
        QVERIFY(FolderMetadata::setNote(m_dir.path(), QStringLiteral("photo.jpg"), QString()));
        QVERIFY(FolderMetadata::togglePin(m_dir.path(), QStringLiteral("photo.jpg")));
        QVERIFY(FolderMetadata::setLabel(m_dir.path(), QStringLiteral("photo.jpg"), QString()));
        QVERIFY(FolderMetadata::setColor(m_dir.path(), QStringLiteral("photo.jpg"), QString()));
        QVERIFY(FolderMetadata::setFolderColor(m_dir.path(), QString()));
        QVERIFY(!QFile::exists(m_dir.filePath(QStringLiteral(".mfm.json"))));
    }

    void rejectsUnsafeNamesAndInvalidValues()
    {
        QString error;
        QVERIFY(!FolderMetadata::setLabel(m_dir.path(), QStringLiteral("../escape"), QStringLiteral("red"), &error));
        QVERIFY(!error.isEmpty());
        QVERIFY(!FolderMetadata::setLabel(m_dir.path(), QStringLiteral("file"), QStringLiteral("unknown")));
        QVERIFY(!FolderMetadata::setColor(m_dir.path(), QStringLiteral("file"), QStringLiteral("red")));
        QVERIFY(!FolderMetadata::setRating(m_dir.path(), QStringLiteral("file"), 6));
        QVERIFY(!FolderMetadata::setNote(m_dir.path(), QStringLiteral("file"), QString(501, u'x')));
    }

    void formatRoundTrips()
    {
        FolderMetadata::Format expected;
        expected.view = QStringLiteral("grid");
        expected.columns = {QStringLiteral("size"), QStringLiteral("rating")};
        expected.sortKey = QStringLiteral("size");
        expected.sortAscending = false;
        expected.group = QStringLiteral("type");
        QVERIFY(FolderMetadata::setFormat(m_dir.path(), expected));

        FolderMetadata::Format actual;
        QVERIFY(FolderMetadata::format(FolderMetadata::read(m_dir.path()), &actual));
        QCOMPARE(actual.view, expected.view);
        QCOMPARE(actual.columns, expected.columns);
        QCOMPARE(actual.sortKey, expected.sortKey);
        QCOMPARE(actual.sortAscending, expected.sortAscending);
        QCOMPARE(actual.group, expected.group);
    }

    void sanitizesLoadedMetadata()
    {
        const QJsonObject input{
            {QStringLiteral("labels"), QJsonObject{{QStringLiteral("good"), QStringLiteral("green")}, {QStringLiteral("../bad"), QStringLiteral("red")}}},
            {QStringLiteral("ratings"), QJsonObject{{QStringLiteral("good"), 9}, {QStringLiteral("bad"), 2}}},
            {QStringLiteral("colors"), QJsonObject{{QStringLiteral("good"), QStringLiteral("blue")}}},
            {QStringLiteral("hidden"), true},
            {QStringLiteral("icon"), QStringLiteral("/usr/share/icons/folder.svg")},
            {QStringLiteral("invalidIcon"), QStringLiteral("relative/icon.svg")},
        };
        QFile file(m_dir.filePath(QStringLiteral(".mfm.json")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QJsonDocument(input).toJson());
        file.close();

        const QJsonObject actual = FolderMetadata::read(m_dir.path());
        QCOMPARE(actual.value(QStringLiteral("labels")).toObject().size(), 1);
        QCOMPARE(actual.value(QStringLiteral("ratings")).toObject().size(), 1);
        QVERIFY(actual.value(QStringLiteral("colors")).toObject().isEmpty());
        QVERIFY(actual.value(QStringLiteral("hidden")).toBool());
        QCOMPARE(actual.value(QStringLiteral("icon")).toString(), QStringLiteral("/usr/share/icons/folder.svg"));
        QVERIFY(FolderMetadata::hidden(actual));
    }

    void hiddenFlagSetsAndClears()
    {
        QTemporaryDir folder;
        QVERIFY(folder.isValid());
        const QString metadataFile = folder.filePath(QStringLiteral(".mfm.json"));
        QVERIFY(!FolderMetadata::hidden(FolderMetadata::read(folder.path())));

        QVERIFY(FolderMetadata::setHidden(folder.path(), true));
        QVERIFY(FolderMetadata::hidden(FolderMetadata::read(folder.path())));
        QVERIFY(FolderMetadata::setFolderColor(folder.path(), QStringLiteral("#123456")));
        QVERIFY(FolderMetadata::setHidden(folder.path(), true));
        QJsonObject metadata = FolderMetadata::read(folder.path());
        QVERIFY(FolderMetadata::hidden(metadata));
        QCOMPARE(metadata.value(QStringLiteral("color")).toString(), QStringLiteral("#123456"));

        QVERIFY(FolderMetadata::setHidden(folder.path(), false));
        metadata = FolderMetadata::read(folder.path());
        QVERIFY(!FolderMetadata::hidden(metadata));
        QVERIFY(!metadata.contains(QStringLiteral("hidden")));
        QCOMPARE(metadata.value(QStringLiteral("color")).toString(), QStringLiteral("#123456"));

        QVERIFY(FolderMetadata::setHidden(folder.path(), true));
        QVERIFY(FolderMetadata::setFolderColor(folder.path(), QString()));
        QVERIFY(QFile::exists(metadataFile));
        QVERIFY(FolderMetadata::setHidden(folder.path(), false));
        QVERIFY(!QFile::exists(metadataFile));
    }

    void hiddenFlagNeedsValidMetadata()
    {
        QVERIFY(!FolderMetadata::hidden(QJsonObject{{QStringLiteral("hidden"), QStringLiteral("true")}}));
        QVERIFY(!FolderMetadata::hidden(QJsonObject{{QStringLiteral("hidden"), false}}));
        QVERIFY(FolderMetadata::hidden(QJsonObject{{QStringLiteral("hidden"), true}}));

        QTemporaryDir folder;
        QVERIFY(folder.isValid());
        QFile file(folder.filePath(QStringLiteral(".mfm.json")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QByteArray oversized = QJsonDocument(QJsonObject{{QStringLiteral("hidden"), true}}).toJson();
        oversized.append(QByteArray(4096, ' '));
        file.write(oversized);
        file.close();
        QString error;
        QVERIFY(!FolderMetadata::hidden(FolderMetadata::read(folder.path(), &error)));
        QVERIFY(!error.isEmpty());
        QVERIFY(!FolderMetadata::setHidden(folder.path(), false, &error));
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.readAll(), oversized);
        file.close();

        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
        file.write("{\"hidden\": true");
        file.close();
        QVERIFY(!FolderMetadata::hidden(FolderMetadata::read(folder.path())));
    }

    void nearestInheritedFormatTakesPrecedence()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("child/grandchild")));
        const QString child = root.filePath(QStringLiteral("child"));
        const QString grandchild = root.filePath(QStringLiteral("child/grandchild"));
        FolderMetadata::Format parent{QStringLiteral("grid"), {QStringLiteral("size")}, QStringLiteral("name"), true, QStringLiteral("none"), true, true, true};
        QVERIFY(FolderMetadata::setFormat(root.path(), parent));
        FolderMetadata::Format actual;
        QString source;
        QString error;
        QVERIFY(FolderMetadata::findFormat(grandchild, &actual, &source, &error));
        QCOMPARE(source, root.path());
        QCOMPARE(actual.view, QStringLiteral("grid"));
        QVERIFY(actual.calculateFolderSizes);

        FolderMetadata::Format local{QStringLiteral("compact"), {}, QStringLiteral("name"), true, QStringLiteral("none"), false, false};
        QVERIFY(FolderMetadata::setFormat(child, local));
        QVERIFY(FolderMetadata::findFormat(child, &actual, &source, &error));
        QCOMPARE(source, child);
        QCOMPARE(actual.view, QStringLiteral("compact"));
        QVERIFY(FolderMetadata::findFormat(grandchild, &actual, &source, &error));
        QCOMPARE(source, root.path());
        local.includeDescendants = true;
        QVERIFY(FolderMetadata::setFormat(child, local));
        QVERIFY(FolderMetadata::findFormat(grandchild, &actual, &source, &error));
        QCOMPARE(source, child);
        QCOMPARE(actual.view, QStringLiteral("compact"));

        local.view = QStringLiteral("cards");
        QVERIFY(FolderMetadata::setFormat(child, local));
        QVERIFY(FolderMetadata::findFormat(grandchild, &actual, &source, &error));
        QCOMPARE(actual.view, QStringLiteral("cards"));
        local.view = QStringLiteral("gallery");
        QVERIFY(!FolderMetadata::setFormat(child, local));
    }

    void clearingFormatPreservesMetadataAndRevealsInheritedRule()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QVERIFY(QDir(root.path()).mkdir(QStringLiteral("child")));
        const QString child = root.filePath(QStringLiteral("child"));
        FolderMetadata::Format parent{QStringLiteral("grid"), {}, QStringLiteral("name"), true, QStringLiteral("none"), true};
        FolderMetadata::Format local{QStringLiteral("list"), {}, QStringLiteral("name"), true, QStringLiteral("none"), false};
        QVERIFY(FolderMetadata::setFormat(root.path(), parent));
        QVERIFY(FolderMetadata::setFormat(child, local));
        QVERIFY(FolderMetadata::setLabel(child, QStringLiteral("file"), QStringLiteral("red")));
        QVERIFY(FolderMetadata::clearFormat(child));
        QCOMPARE(FolderMetadata::read(child).value(QStringLiteral("labels")).toObject().value(QStringLiteral("file")).toString(), QStringLiteral("red"));
        FolderMetadata::Format actual;
        QString source;
        QVERIFY(FolderMetadata::findFormat(child, &actual, &source));
        QCOMPARE(source, root.path());
        QCOMPARE(actual.view, QStringLiteral("grid"));
        QFile bad(root.filePath(QStringLiteral("child/.mfm.json")));
        QVERIFY(bad.open(QIODevice::WriteOnly | QIODevice::Truncate));
        QCOMPARE(bad.write("invalid"), 7);
        bad.close();
        QString error;
        QVERIFY(!FolderMetadata::clearFormat(child, &error));
        QVERIFY(!error.isEmpty());
        QVERIFY(bad.open(QIODevice::ReadOnly));
        QCOMPARE(bad.readAll(), QByteArrayLiteral("invalid"));
    }
};

QTEST_GUILESS_MAIN(FolderMetadataTest)

#include "foldermetadatatest.moc"
