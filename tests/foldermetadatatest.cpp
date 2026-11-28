// SPDX-License-Identifier: GPL-3.0-or-later

#include "foldermetadata.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QTemporaryDir>
#include <QTest>

#include <functional>

namespace
{
QString metadataFile(const QTemporaryDir &folder)
{
    return folder.filePath(QStringLiteral(".mfm.json"));
}

void writeRaw(const QTemporaryDir &folder, const QByteArray &bytes)
{
    QFile file(metadataFile(folder));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(file.write(bytes), bytes.size());
}

QByteArray readRaw(const QTemporaryDir &folder)
{
    QFile file(metadataFile(folder));
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

QJsonObject rawObject(const QTemporaryDir &folder)
{
    return QJsonDocument::fromJson(readRaw(folder)).object();
}

const QJsonObject Unknown{
    {QStringLiteral("futureKey"), QJsonObject{{QStringLiteral("nested"), QJsonArray{1, QStringLiteral("x"), true}}}},
    {QStringLiteral("tool.version"), 3},
    {QStringLiteral("icon"), QStringLiteral("relative/icon.svg")},
};
}

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

    void unknownTopLevelKeysSurviveEverySetter()
    {
        QTemporaryDir folder;
        QVERIFY(folder.isValid());
        writeRaw(folder, QJsonDocument(Unknown).toJson());
        FolderMetadata::Format format{QStringLiteral("grid"), {QStringLiteral("size")}, QStringLiteral("name"), true, QStringLiteral("none"), true, true, true};
        const QString file = QStringLiteral("a.txt");
        const QList<std::function<bool()>> edits{
            [&] {
                return FolderMetadata::setLabel(folder.path(), file, QStringLiteral("red"));
            },
            [&] {
                return FolderMetadata::setColor(folder.path(), file, QStringLiteral("#abc"));
            },
            [&] {
                return FolderMetadata::setRating(folder.path(), file, 4);
            },
            [&] {
                return FolderMetadata::setNote(folder.path(), file, QStringLiteral("note"));
            },
            [&] {
                return FolderMetadata::togglePin(folder.path(), file);
            },
            [&] {
                return FolderMetadata::setFolderColor(folder.path(), QStringLiteral("#123456"));
            },
            [&] {
                return FolderMetadata::setHidden(folder.path(), true);
            },
            [&] {
                return FolderMetadata::setFormat(folder.path(), format);
            },
            [&] {
                return FolderMetadata::setLabel(folder.path(), file, QString());
            },
            [&] {
                return FolderMetadata::setColor(folder.path(), file, QString());
            },
            [&] {
                return FolderMetadata::setRating(folder.path(), file, 0);
            },
            [&] {
                return FolderMetadata::setNote(folder.path(), file, QString());
            },
            [&] {
                return FolderMetadata::togglePin(folder.path(), file);
            },
            [&] {
                return FolderMetadata::setFolderColor(folder.path(), QString());
            },
            [&] {
                return FolderMetadata::setHidden(folder.path(), false);
            },
            [&] {
                return FolderMetadata::clearFormat(folder.path());
            },
        };
        for (qsizetype i = 0; i < edits.size(); ++i) {
            QVERIFY2(edits.at(i)(), qPrintable(QString::number(i)));
            const QJsonObject raw = rawObject(folder);
            for (auto it = Unknown.constBegin(); it != Unknown.constEnd(); ++it) {
                QCOMPARE(raw.value(it.key()), it.value());
            }
        }
        QCOMPARE(rawObject(folder), Unknown);
    }

    void unknownEntriesInKnownKeysSurvive()
    {
        QTemporaryDir folder;
        QVERIFY(folder.isValid());
        const QJsonObject labels{{QStringLiteral("teal.txt"), QStringLiteral("teal")}, {QStringLiteral("red.txt"), QStringLiteral("red")}};
        const QJsonObject colors{{QStringLiteral("named.txt"), QStringLiteral("red")}};
        const QJsonObject ratings{{QStringLiteral("high.txt"), 9}};
        const QJsonObject notes{{QStringLiteral("long.txt"), QString(600, u'x')}, {QStringLiteral("object.txt"), QJsonObject{{QStringLiteral("text"), 1}}}};
        const QJsonArray pins{QStringLiteral("kept.txt")};
        const QJsonObject format{{QStringLiteral("view"), QStringLiteral("list")},
                                 {QStringLiteral("columns"), QJsonArray{QStringLiteral("tags"), QStringLiteral("size"), 7}},
                                 {QStringLiteral("sortKey"), QStringLiteral("name")},
                                 {QStringLiteral("group"), QStringLiteral("none")},
                                 {QStringLiteral("density"), QStringLiteral("tight")},
                                 {QStringLiteral("calculateFolderSizes"), true}};
        writeRaw(folder,
                 QJsonDocument(QJsonObject{{QStringLiteral("labels"), labels},
                                           {QStringLiteral("colors"), colors},
                                           {QStringLiteral("ratings"), ratings},
                                           {QStringLiteral("notes"), notes},
                                           {QStringLiteral("pins"), pins},
                                           {QStringLiteral("format"), format}})
                     .toJson());

        QString error;
        QVERIFY2(FolderMetadata::setLabel(folder.path(), QStringLiteral("a"), QStringLiteral("blue"), &error), qPrintable(error));
        QVERIFY2(FolderMetadata::setColor(folder.path(), QStringLiteral("a"), QStringLiteral("#abc"), &error), qPrintable(error));
        QVERIFY2(FolderMetadata::setRating(folder.path(), QStringLiteral("a"), 3, &error), qPrintable(error));
        QVERIFY2(FolderMetadata::setNote(folder.path(), QStringLiteral("a"), QStringLiteral("hi"), &error), qPrintable(error));
        QVERIFY2(FolderMetadata::togglePin(folder.path(), QStringLiteral("a"), &error), qPrintable(error));
        FolderMetadata::Format newFormat{QStringLiteral("grid"), {QStringLiteral("date")}, QStringLiteral("size"), false, QStringLiteral("type"), false};
        QVERIFY2(FolderMetadata::setFormat(folder.path(), newFormat, &error), qPrintable(error));

        const QJsonObject raw = rawObject(folder);
        QJsonObject expectedLabels = labels;
        expectedLabels.insert(QStringLiteral("a"), QStringLiteral("blue"));
        QCOMPARE(raw.value(QStringLiteral("labels")).toObject(), expectedLabels);
        QCOMPARE(raw.value(QStringLiteral("colors")).toObject().value(QStringLiteral("named.txt")), colors.value(QStringLiteral("named.txt")));
        QCOMPARE(raw.value(QStringLiteral("ratings")).toObject().value(QStringLiteral("high.txt")), ratings.value(QStringLiteral("high.txt")));
        QJsonObject expectedNotes = notes;
        expectedNotes.insert(QStringLiteral("a"), QStringLiteral("hi"));
        QCOMPARE(raw.value(QStringLiteral("notes")).toObject(), expectedNotes);
        QCOMPARE(raw.value(QStringLiteral("pins")).toArray(), (QJsonArray{QStringLiteral("kept.txt"), QStringLiteral("a")}));

        const QJsonObject savedFormat = raw.value(QStringLiteral("format")).toObject();
        QCOMPARE(savedFormat.value(QStringLiteral("density")).toString(), QStringLiteral("tight"));
        QCOMPARE(savedFormat.value(QStringLiteral("columns")).toArray(), (QJsonArray{QStringLiteral("date"), QStringLiteral("tags"), 7}));
        QCOMPARE(savedFormat.value(QStringLiteral("view")).toString(), QStringLiteral("grid"));
        QVERIFY(!savedFormat.contains(QStringLiteral("calculateFolderSizes")));
        FolderMetadata::Format actual;
        QVERIFY(FolderMetadata::format(FolderMetadata::read(folder.path()), &actual));
        QCOMPARE(actual.columns, QStringList{QStringLiteral("date")});
    }

    void unsupportedExistingValuesRefuseWrites_data()
    {
        QTest::addColumn<QByteArray>("content");
        QTest::addColumn<int>("edit");

        enum Edit {
            Label,
            ClearLabel,
            Color,
            Rating,
            Note,
            Pin,
            FolderColor,
            Hidden,
            Format,
            ClearFormat
        };
        const QByteArray longNote = "{\"notes\":{\"a\":\"" + QByteArray(600, 'x') + "\"}}";
        QByteArray manyPins = "{\"pins\":[";
        for (int i = 0; i < 200; ++i) {
            manyPins += (i ? ",\"" : "\"") + QByteArray::number(i) + '"';
        }
        manyPins += "]}";
        QTest::newRow("labels not an object") << QByteArray("{\"labels\":\"red\"}") << int(Label);
        QTest::newRow("unsafe label name") << QByteArray("{\"labels\":{\"../bad\":\"red\"}}") << int(Label);
        QTest::newRow("unknown label replaced") << QByteArray("{\"labels\":{\"a\":\"teal\"}}") << int(Label);
        QTest::newRow("unknown label cleared") << QByteArray("{\"labels\":{\"a\":\"teal\"}}") << int(ClearLabel);
        QTest::newRow("named colour") << QByteArray("{\"colors\":{\"a\":\"red\"}}") << int(Color);
        QTest::newRow("rating out of range") << QByteArray("{\"ratings\":{\"a\":9}}") << int(Rating);
        QTest::newRow("rating as text") << QByteArray("{\"ratings\":{\"a\":\"3\"}}") << int(Rating);
        QTest::newRow("note too long") << longNote << int(Note);
        QTest::newRow("pins not an array") << QByteArray("{\"pins\":{}}") << int(Pin);
        QTest::newRow("pin not a name") << QByteArray("{\"pins\":[5]}") << int(Pin);
        QTest::newRow("unsafe pin") << QByteArray("{\"pins\":[\"../x\"]}") << int(Pin);
        QTest::newRow("too many pins") << manyPins << int(Pin);
        QTest::newRow("folder colour name") << QByteArray("{\"color\":\"red\"}") << int(FolderColor);
        QTest::newRow("hidden as text") << QByteArray("{\"hidden\":\"yes\"}") << int(Hidden);
        QTest::newRow("format not an object") << QByteArray("{\"format\":\"list\"}") << int(Format);
        QTest::newRow("unknown view") << QByteArray("{\"format\":{\"view\":\"gallery\",\"group\":\"none\"}}") << int(Format);
        QTest::newRow("columns not an array") << QByteArray("{\"format\":{\"view\":\"list\",\"group\":\"none\",\"columns\":\"size\"}}") << int(Format);
        QTest::newRow("sort order as text") << QByteArray("{\"format\":{\"view\":\"list\",\"group\":\"none\",\"sortAsc\":\"no\"}}") << int(Format);
        QTest::newRow("unknown view cleared") << QByteArray("{\"format\":{\"view\":\"gallery\",\"group\":\"none\"}}") << int(ClearFormat);
        QTest::newRow("not an object") << QByteArray("[1,2]") << int(Label);
        QTest::newRow("invalid JSON") << QByteArray("{\"labels\":") << int(Hidden);
    }

    void unsupportedExistingValuesRefuseWrites()
    {
        QFETCH(QByteArray, content);
        QFETCH(int, edit);
        QTemporaryDir folder;
        QVERIFY(folder.isValid());
        writeRaw(folder, content);
        const QString path = folder.path();
        const QString name = QStringLiteral("a");
        const FolderMetadata::Format format{QStringLiteral("list"), {}, QStringLiteral("name"), true, QStringLiteral("none")};
        const QList<std::function<bool(QString *)>> edits{
            [&](QString *error) {
                return FolderMetadata::setLabel(path, name, QStringLiteral("red"), error);
            },
            [&](QString *error) {
                return FolderMetadata::setLabel(path, name, QString(), error);
            },
            [&](QString *error) {
                return FolderMetadata::setColor(path, name, QStringLiteral("#fff"), error);
            },
            [&](QString *error) {
                return FolderMetadata::setRating(path, name, 0, error);
            },
            [&](QString *error) {
                return FolderMetadata::setNote(path, name, QStringLiteral("x"), error);
            },
            [&](QString *error) {
                return FolderMetadata::togglePin(path, name, error);
            },
            [&](QString *error) {
                return FolderMetadata::setFolderColor(path, QStringLiteral("#123"), error);
            },
            [&](QString *error) {
                return FolderMetadata::setHidden(path, true, error);
            },
            [&](QString *error) {
                return FolderMetadata::setFormat(path, format, error);
            },
            [&](QString *error) {
                return FolderMetadata::clearFormat(path, error);
            },
        };

        QVERIFY(!edits.at(edit)(nullptr));
        QCOMPARE(readRaw(folder), content);
        QString error;
        QVERIFY(!edits.at(edit)(&error));
        QVERIFY(!error.isEmpty());
        QCOMPARE(readRaw(folder), content);
    }

    void sizeLimitRefusesRatherThanTruncates()
    {
        QTemporaryDir folder;
        QVERIFY(folder.isValid());
        // {"padding":"…"} is 14 bytes plus the padding.
        const QByteArray content = QJsonDocument(QJsonObject{{QStringLiteral("padding"), QString(4030, u'p')}}).toJson(QJsonDocument::Compact);
        QCOMPARE(content.size(), 4044);
        writeRaw(folder, content);

        QString error;
        QVERIFY(!FolderMetadata::setNote(folder.path(), QStringLiteral("a"), QString(100, u'n'), &error));
        QVERIFY(error.contains(QStringLiteral("4,096")));
        QCOMPARE(readRaw(folder), content);

        QVERIFY2(FolderMetadata::setLabel(folder.path(), QStringLiteral("a"), QStringLiteral("red"), &error), qPrintable(error));
        const QJsonObject raw = rawObject(folder);
        QCOMPARE(raw.value(QStringLiteral("padding")).toString(), QString(4030, u'p'));
        QCOMPARE(raw.value(QStringLiteral("labels")).toObject().value(QStringLiteral("a")).toString(), QStringLiteral("red"));
        QVERIFY(readRaw(folder).size() <= 4096);
    }

    void readingAppliesOnlyValidatedValues()
    {
        QTemporaryDir folder;
        QVERIFY(folder.isValid());
        QJsonObject content = Unknown;
        content.insert(QStringLiteral("labels"),
                       QJsonObject{{QStringLiteral("../bad"), QStringLiteral("red")}, {QStringLiteral("teal.txt"), QStringLiteral("teal")}});
        content.insert(QStringLiteral("ratings"), QJsonObject{{QStringLiteral("high.txt"), 9}});
        content.insert(QStringLiteral("format"),
                       QJsonObject{{QStringLiteral("view"), QStringLiteral("list")},
                                   {QStringLiteral("group"), QStringLiteral("none")},
                                   {QStringLiteral("columns"), QJsonArray{QStringLiteral("tags")}},
                                   {QStringLiteral("density"), QStringLiteral("tight")}});
        writeRaw(folder, QJsonDocument(content).toJson());

        QString error;
        QVERIFY2(FolderMetadata::setRating(folder.path(), QStringLiteral("good.txt"), 2, &error), qPrintable(error));
        QVERIFY2(FolderMetadata::setHidden(folder.path(), true, &error), qPrintable(error));
        QCOMPARE(rawObject(folder).value(QStringLiteral("icon")).toString(), QStringLiteral("relative/icon.svg"));

        const QJsonObject metadata = FolderMetadata::read(folder.path(), &error);
        QVERIFY(error.isEmpty());
        const QStringList keys = metadata.keys();
        QCOMPARE(QSet<QString>(keys.cbegin(), keys.cend()), (QSet<QString>{QStringLiteral("ratings"), QStringLiteral("hidden"), QStringLiteral("format")}));
        QCOMPARE(metadata.value(QStringLiteral("ratings")).toObject(), (QJsonObject{{QStringLiteral("good.txt"), 2}}));
        const QJsonObject format = metadata.value(QStringLiteral("format")).toObject();
        QVERIFY(!format.contains(QStringLiteral("density")));
        QVERIFY(format.value(QStringLiteral("columns")).toArray().isEmpty());
    }
};

QTEST_GUILESS_MAIN(FolderMetadataTest)

#include "foldermetadatatest.moc"
