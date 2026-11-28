// SPDX-License-Identifier: GPL-3.0-or-later

#include "foldermetadata.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QSaveFile>

#include <functional>
#include <utility>

namespace
{
constexpr qsizetype MaxMetadataBytes = 4096;
const QStringList AllowedLabels{QStringLiteral("red"),
                                QStringLiteral("orange"),
                                QStringLiteral("yellow"),
                                QStringLiteral("green"),
                                QStringLiteral("blue"),
                                QStringLiteral("purple"),
                                QStringLiteral("pink"),
                                QStringLiteral("gray")};
const QStringList AllowedColumns{QStringLiteral("type"),
                                 QStringLiteral("size"),
                                 QStringLiteral("date"),
                                 QStringLiteral("mode"),
                                 QStringLiteral("owner"),
                                 QStringLiteral("group"),
                                 QStringLiteral("links"),
                                 QStringLiteral("dims"),
                                 QStringLiteral("rating")};
const QStringList AllowedViews{QStringLiteral("list"), QStringLiteral("grid"), QStringLiteral("compact"), QStringLiteral("cards")};
const QStringList AllowedGroups{QStringLiteral("none"),
                                QStringLiteral("name"),
                                QStringLiteral("type"),
                                QStringLiteral("size"),
                                QStringLiteral("date"),
                                QStringLiteral("ext")};

QString metadataPath(const QString &folder)
{
    return QDir(folder).filePath(QStringLiteral(".mfm.json"));
}

bool validName(const QString &name)
{
    return !name.isEmpty() && name != QLatin1String(".") && name != QLatin1String("..") && name != QLatin1String(".mfm.json") && !name.contains(u'/')
        && !name.contains(u'\\');
}

bool validColor(const QString &color)
{
    static const QRegularExpression expression(QStringLiteral("^#[0-9a-fA-F]{3,8}$"));
    return expression.match(color).hasMatch();
}

QJsonObject readObject(const QString &folder, QString *error)
{
    QFile file(metadataPath(folder));
    if (!file.exists()) {
        return {};
    }
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) {
            *error = file.errorString();
        }
        return {};
    }
    if (file.size() > MaxMetadataBytes) {
        if (error) {
            *error = QStringLiteral("Metadata file exceeds 4096 bytes");
        }
        return {};
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (error) {
            *error = parseError.errorString();
        }
        return {};
    }
    return document.object();
}

QJsonObject sanitized(const QJsonObject &input)
{
    QJsonObject output;
    const QString folderColor = input.value(QStringLiteral("color")).toString();
    if (validColor(folderColor)) {
        output.insert(QStringLiteral("color"), folderColor);
    }
    const QString icon = input.value(QStringLiteral("icon")).toString();
    if (icon.startsWith(QLatin1String("g:")) || icon.startsWith(QLatin1String("p:")) || QDir::isAbsolutePath(icon)) {
        output.insert(QStringLiteral("icon"), icon);
    }
    if (input.value(QStringLiteral("hidden")).toBool()) {
        output.insert(QStringLiteral("hidden"), true);
    }

    QJsonArray pins;
    for (const QJsonValue &value : input.value(QStringLiteral("pins")).toArray()) {
        const QString name = value.toString();
        if (validName(name) && !pins.contains(name) && pins.size() < 200) {
            pins.append(name);
        }
    }
    if (!pins.isEmpty()) {
        output.insert(QStringLiteral("pins"), pins);
    }

    for (const QString &key : {QStringLiteral("labels"), QStringLiteral("colors")}) {
        QJsonObject values;
        const QJsonObject source = input.value(key).toObject();
        for (auto it = source.constBegin(); it != source.constEnd(); ++it) {
            if (!validName(it.key())) {
                continue;
            }
            const QString value = it.value().toString();
            if ((key == QLatin1String("labels") && AllowedLabels.contains(value)) || (key == QLatin1String("colors") && validColor(value))) {
                values.insert(it.key(), value);
            }
        }
        if (!values.isEmpty()) {
            output.insert(key, values);
        }
    }

    QJsonObject ratings;
    const QJsonObject sourceRatings = input.value(QStringLiteral("ratings")).toObject();
    for (auto it = sourceRatings.constBegin(); it != sourceRatings.constEnd(); ++it) {
        const int value = it.value().toInt();
        if (validName(it.key()) && value >= 1 && value <= 5) {
            ratings.insert(it.key(), value);
        }
    }
    if (!ratings.isEmpty()) {
        output.insert(QStringLiteral("ratings"), ratings);
    }

    QJsonObject notes;
    const QJsonObject sourceNotes = input.value(QStringLiteral("notes")).toObject();
    for (auto it = sourceNotes.constBegin(); it != sourceNotes.constEnd(); ++it) {
        const QString value = it.value().toString().trimmed();
        if (validName(it.key()) && !value.isEmpty() && value.size() <= 500) {
            notes.insert(it.key(), value);
        }
    }
    if (!notes.isEmpty()) {
        output.insert(QStringLiteral("notes"), notes);
    }

    FolderMetadata::Format viewFormat;
    if (FolderMetadata::format(input, &viewFormat)) {
        QJsonObject formatObject;
        formatObject.insert(QStringLiteral("view"), viewFormat.view);
        formatObject.insert(QStringLiteral("columns"), QJsonArray::fromStringList(viewFormat.columns));
        formatObject.insert(QStringLiteral("sortKey"), viewFormat.sortKey);
        formatObject.insert(QStringLiteral("sortAsc"), viewFormat.sortAscending);
        formatObject.insert(QStringLiteral("group"), viewFormat.group);
        formatObject.insert(QStringLiteral("includeDescendants"), viewFormat.includeDescendants);
        if (viewFormat.hasFolderSizePolicy) {
            formatObject.insert(QStringLiteral("calculateFolderSizes"), viewFormat.calculateFolderSizes);
        }
        output.insert(QStringLiteral("format"), formatObject);
    }
    return output;
}

bool mutate(const QString &folder, const std::function<void(QJsonObject &)> &mutation, QString *error)
{
    if (error) {
        error->clear();
    }
    QJsonObject metadata = readObject(folder, error);
    if (error && !error->isEmpty()) {
        return false;
    }
    mutation(metadata);
    if (error && !error->isEmpty()) {
        return false;
    }
    return FolderMetadata::write(folder, metadata, error);
}

void setMapValue(QJsonObject &metadata, const QString &mapName, const QString &name, const QString &value)
{
    QJsonObject values = metadata.value(mapName).toObject();
    if (value.isEmpty()) {
        values.remove(name);
    } else {
        values.insert(name, value);
    }
    if (values.isEmpty()) {
        metadata.remove(mapName);
    } else {
        metadata.insert(mapName, values);
    }
}
}

QJsonObject FolderMetadata::read(const QString &folder, QString *error)
{
    if (error) {
        error->clear();
    }
    return sanitized(readObject(folder, error));
}

bool FolderMetadata::write(const QString &folder, const QJsonObject &metadata, QString *error)
{
    if (error) {
        error->clear();
    }
    const QJsonObject clean = sanitized(metadata);
    const QString path = metadataPath(folder);
    if (clean.isEmpty()) {
        if (!QFile::exists(path)) {
            return true;
        }
        if (!QFile::remove(path)) {
            if (error) {
                *error = QStringLiteral("Could not remove %1").arg(path);
            }
            return false;
        }
        return true;
    }
    const QByteArray bytes = QJsonDocument(clean).toJson(QJsonDocument::Compact);
    if (bytes.size() > MaxMetadataBytes) {
        if (error) {
            *error = QStringLiteral("Metadata file exceeds 4096 bytes");
        }
        return false;
    }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error) {
            *error = file.errorString();
        }
        return false;
    }
    if (file.write(bytes) != bytes.size() || !file.commit()) {
        if (error) {
            *error = file.errorString();
        }
        return false;
    }
    return true;
}

bool FolderMetadata::setLabel(const QString &folder, const QString &name, const QString &label, QString *error)
{
    if (!validName(name) || (!label.isEmpty() && !AllowedLabels.contains(label))) {
        if (error) {
            *error = QStringLiteral("Invalid file name or label");
        }
        return false;
    }
    return mutate(
        folder,
        [&name, &label](QJsonObject &metadata) {
            setMapValue(metadata, QStringLiteral("labels"), name, label);
        },
        error);
}

bool FolderMetadata::setColor(const QString &folder, const QString &name, const QString &color, QString *error)
{
    if (!validName(name) || (!color.isEmpty() && !validColor(color))) {
        if (error) {
            *error = QStringLiteral("Invalid file name or color");
        }
        return false;
    }
    return mutate(
        folder,
        [&name, &color](QJsonObject &metadata) {
            setMapValue(metadata, QStringLiteral("colors"), name, color);
        },
        error);
}

bool FolderMetadata::setRating(const QString &folder, const QString &name, int rating, QString *error)
{
    if (!validName(name) || rating < 0 || rating > 5) {
        if (error) {
            *error = QStringLiteral("Invalid file name or rating");
        }
        return false;
    }
    return mutate(
        folder,
        [&name, rating](QJsonObject &metadata) {
            QJsonObject ratings = metadata.value(QStringLiteral("ratings")).toObject();
            if (rating == 0) {
                ratings.remove(name);
            } else {
                ratings.insert(name, rating);
            }
            if (ratings.isEmpty()) {
                metadata.remove(QStringLiteral("ratings"));
            } else {
                metadata.insert(QStringLiteral("ratings"), ratings);
            }
        },
        error);
}

bool FolderMetadata::setNote(const QString &folder, const QString &name, const QString &note, QString *error)
{
    const QString trimmed = note.trimmed();
    if (!validName(name) || trimmed.size() > 500) {
        if (error) {
            *error = QStringLiteral("Invalid file name or note exceeds 500 characters");
        }
        return false;
    }
    return mutate(
        folder,
        [&name, &trimmed](QJsonObject &metadata) {
            setMapValue(metadata, QStringLiteral("notes"), name, trimmed);
        },
        error);
}

bool FolderMetadata::togglePin(const QString &folder, const QString &name, QString *error)
{
    if (!validName(name)) {
        if (error) {
            *error = QStringLiteral("Invalid file name");
        }
        return false;
    }
    return mutate(
        folder,
        [&name, error](QJsonObject &metadata) {
            QJsonArray pins = metadata.value(QStringLiteral("pins")).toArray();
            int index = -1;
            for (qsizetype i = 0; i < pins.size(); ++i) {
                if (pins.at(i).toString() == name) {
                    index = static_cast<int>(i);
                    break;
                }
            }
            if (index >= 0) {
                pins.removeAt(index);
            } else if (pins.size() < 200) {
                pins.append(name);
            } else if (error) {
                *error = QStringLiteral("A folder can have at most 200 pinned items");
                return;
            }
            if (pins.isEmpty()) {
                metadata.remove(QStringLiteral("pins"));
            } else {
                metadata.insert(QStringLiteral("pins"), pins);
            }
        },
        error);
}

bool FolderMetadata::setFolderColor(const QString &folder, const QString &color, QString *error)
{
    if (!color.isEmpty() && !validColor(color)) {
        if (error) {
            *error = QStringLiteral("Invalid folder color");
        }
        return false;
    }
    return mutate(
        folder,
        [&color](QJsonObject &metadata) {
            if (color.isEmpty()) {
                metadata.remove(QStringLiteral("color"));
            } else {
                metadata.insert(QStringLiteral("color"), color);
            }
        },
        error);
}

bool FolderMetadata::setHidden(const QString &folder, bool hiddenValue, QString *error)
{
    return mutate(
        folder,
        [hiddenValue](QJsonObject &metadata) {
            if (hiddenValue) {
                metadata.insert(QStringLiteral("hidden"), true);
            } else {
                metadata.remove(QStringLiteral("hidden"));
            }
        },
        error);
}

bool FolderMetadata::hidden(const QJsonObject &metadata)
{
    return metadata.value(QStringLiteral("hidden")).toBool();
}

bool FolderMetadata::setFormat(const QString &folder, const Format &formatValue, QString *error)
{
    if (!AllowedViews.contains(formatValue.view) || !AllowedGroups.contains(formatValue.group)) {
        if (error) {
            *error = QStringLiteral("Invalid folder view format");
        }
        return false;
    }
    for (const QString &column : formatValue.columns) {
        if (!AllowedColumns.contains(column)) {
            if (error) {
                *error = QStringLiteral("Invalid folder view column");
            }
            return false;
        }
    }
    QJsonObject formatObject;
    formatObject.insert(QStringLiteral("view"), formatValue.view);
    formatObject.insert(QStringLiteral("columns"), QJsonArray::fromStringList(formatValue.columns));
    formatObject.insert(QStringLiteral("sortKey"), formatValue.sortKey.left(19));
    formatObject.insert(QStringLiteral("sortAsc"), formatValue.sortAscending);
    formatObject.insert(QStringLiteral("group"), formatValue.group);
    formatObject.insert(QStringLiteral("includeDescendants"), formatValue.includeDescendants);
    if (formatValue.hasFolderSizePolicy) {
        formatObject.insert(QStringLiteral("calculateFolderSizes"), formatValue.calculateFolderSizes);
    }
    return mutate(
        folder,
        [&formatObject](QJsonObject &metadata) {
            metadata.insert(QStringLiteral("format"), formatObject);
        },
        error);
}

bool FolderMetadata::clearFormat(const QString &folder, QString *error)
{
    return mutate(
        folder,
        [](QJsonObject &metadata) {
            metadata.remove(QStringLiteral("format"));
        },
        error);
}

bool FolderMetadata::format(const QJsonObject &metadata, Format *result)
{
    if (!result) {
        return false;
    }
    const QJsonObject object = metadata.value(QStringLiteral("format")).toObject();
    const QString view = object.value(QStringLiteral("view")).toString();
    const QString group = object.value(QStringLiteral("group")).toString();
    const QString sortKey = object.value(QStringLiteral("sortKey")).toString();
    if (!AllowedViews.contains(view) || !AllowedGroups.contains(group) || sortKey.size() > 19) {
        return false;
    }
    QStringList columns;
    for (const QJsonValue &value : object.value(QStringLiteral("columns")).toArray()) {
        const QString column = value.toString();
        if (AllowedColumns.contains(column) && !columns.contains(column)) {
            columns.append(column);
        }
    }
    result->view = view;
    result->columns = columns;
    result->sortKey = sortKey;
    result->sortAscending = object.value(QStringLiteral("sortAsc")).toBool(true);
    result->group = group;
    result->includeDescendants = object.value(QStringLiteral("includeDescendants")).toBool(false);
    result->calculateFolderSizes = object.value(QStringLiteral("calculateFolderSizes")).toBool(false);
    result->hasFolderSizePolicy = object.value(QStringLiteral("calculateFolderSizes")).isBool();
    return true;
}

bool FolderMetadata::findFormat(const QString &folder, Format *result, QString *source, QString *error)
{
    if (error) {
        error->clear();
    }
    if (source) {
        source->clear();
    }
    if (!result || !QDir::isAbsolutePath(folder)) {
        if (error) {
            *error = QStringLiteral("Invalid folder path or format destination");
        }
        return false;
    }
    QDir directory(QDir::cleanPath(folder));
    bool exact = true;
    do {
        QString readError;
        const QJsonObject metadata = read(directory.absolutePath(), &readError);
        if (!readError.isEmpty()) {
            if (error) {
                *error = readError;
            }
            return false;
        }
        Format candidate;
        if (format(metadata, &candidate) && (exact || candidate.includeDescendants)) {
            *result = candidate;
            if (source) {
                *source = directory.absolutePath();
            }
            return true;
        }
        exact = false;
    } while (directory.cdUp());
    return false;
}
