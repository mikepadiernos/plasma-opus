// SPDX-License-Identifier: GPL-3.0-or-later

#include "foldermetadata.h"

#include <KLocalizedString>

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>

#include <functional>
#include <optional>
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

/// Reads the unvalidated object; std::nullopt when the file exists but can't be used.
std::optional<QJsonObject> readObject(const QString &folder, QString *error)
{
    QFile file(metadataPath(folder));
    if (!file.exists()) {
        return QJsonObject();
    }
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) {
            *error = file.errorString();
        }
        return std::nullopt;
    }
    if (file.size() > MaxMetadataBytes) {
        if (error) {
            *error = QStringLiteral("Metadata file exceeds 4096 bytes");
        }
        return std::nullopt;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (error) {
            *error = parseError.error != QJsonParseError::NoError ? parseError.errorString() : QStringLiteral("Metadata file is not a JSON object");
        }
        return std::nullopt;
    }
    return document.object();
}

bool isAbsent(const QJsonValue &value)
{
    return value.isUndefined() || value.isNull();
}

QString unsupportedValue(const QString &folder, const QString &key)
{
    return i18nc("@info %1 is a metadata file path, %2 a metadata key",
                 "%1 has a “%2” value that Plasma Opus does not support, and saving would discard it",
                 metadataPath(folder),
                 key);
}

QString unsupportedEntry(const QString &folder, const QString &key, const QString &name)
{
    return i18nc("@info %1 is a metadata file path, %2 a metadata key, %3 a file name",
                 "%1 has a “%2” value for “%3” that Plasma Opus does not support, and saving would discard it",
                 metadataPath(folder),
                 key,
                 name);
}

QString unsafeName(const QString &folder, const QString &key)
{
    return i18nc("@info %1 is a metadata file path, %2 a metadata key",
                 "%1 has an unsafe file name under “%2”, and saving would discard it",
                 metadataPath(folder),
                 key);
}

bool supportedLabel(const QJsonValue &value)
{
    return AllowedLabels.contains(value.toString());
}

bool supportedColor(const QJsonValue &value)
{
    return validColor(value.toString());
}

bool supportedRating(const QJsonValue &value)
{
    const int rating = value.toInt(0);
    return rating >= 1 && rating <= 5;
}

bool supportedNote(const QJsonValue &value)
{
    return value.isString() && value.toString().trimmed().size() <= 500;
}

/// Whether setFormat() can rewrite the known fields of @p value without discarding anything.
bool supportedFormat(const QJsonValue &value)
{
    FolderMetadata::Format parsed;
    if (!value.isObject() || !FolderMetadata::format(QJsonObject{{QStringLiteral("format"), value}}, &parsed)) {
        return false;
    }
    const QJsonObject object = value.toObject();
    const QJsonValue columns = object.value(QStringLiteral("columns"));
    const QJsonValue sortKey = object.value(QStringLiteral("sortKey"));
    if ((!isAbsent(columns) && !columns.isArray()) || (!isAbsent(sortKey) && !sortKey.isString())) {
        return false;
    }
    for (const QString &key : {QStringLiteral("sortAsc"), QStringLiteral("includeDescendants"), QStringLiteral("calculateFolderSizes")}) {
        const QJsonValue flag = object.value(key);
        if (!isAbsent(flag) && !flag.isBool()) {
            return false;
        }
    }
    return true;
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

/// Atomically writes @p metadata as it is, deleting .mfm.json when the object is empty.
bool writeObject(const QString &folder, const QJsonObject &metadata, QString &error)
{
    const QString path = metadataPath(folder);
    if (metadata.isEmpty()) {
        if (QFile::exists(path) && !QFile::remove(path)) {
            error = QStringLiteral("Could not remove %1").arg(path);
            return false;
        }
        return true;
    }
    const QByteArray bytes = QJsonDocument(metadata).toJson(QJsonDocument::Compact);
    if (bytes.size() > MaxMetadataBytes) {
        error = i18nc("@info %1 is a metadata file path", "%1 would exceed the 4,096-byte limit for folder metadata", path);
        return false;
    }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit()) {
        error = file.errorString();
        return false;
    }
    return true;
}

/**
 * Applies @p mutation to the metadata read just before writing. The mutation
 * changes only the value it targets, so keys and entries Plasma Opus doesn't
 * know are written back as they are; it returns false with @p error to refuse
 * an edit that would discard data, and the file is then left unchanged.
 */
bool mutate(const QString &folder, const std::function<bool(QJsonObject &, QString &)> &mutation, QString *error)
{
    QString localError;
    std::optional<QJsonObject> metadata = readObject(folder, &localError);
    const bool written = metadata && mutation(*metadata, localError) && writeObject(folder, *metadata, localError);
    if (error) {
        *error = written ? QString() : localError;
    }
    return written;
}

/// Sets or, when @p value is null, removes the @p name entry of the @p key map.
bool setMapValue(const QString &folder,
                 QJsonObject &metadata,
                 const QString &key,
                 const QString &name,
                 const QJsonValue &value,
                 bool (*supported)(const QJsonValue &),
                 QString &error)
{
    const QJsonValue existing = metadata.value(key);
    if (!isAbsent(existing) && !existing.isObject()) {
        error = unsupportedValue(folder, key);
        return false;
    }
    QJsonObject values = existing.toObject();
    for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
        if (!validName(it.key())) {
            error = unsafeName(folder, key);
            return false;
        }
    }
    const QJsonValue current = values.value(name);
    if (!isAbsent(current) && !supported(current)) {
        error = unsupportedEntry(folder, key, name);
        return false;
    }
    if (isAbsent(value)) {
        values.remove(name);
    } else {
        values.insert(name, value);
    }
    if (values.isEmpty()) {
        metadata.remove(key);
    } else {
        metadata.insert(key, values);
    }
    return true;
}
}

QJsonObject FolderMetadata::read(const QString &folder, QString *error)
{
    if (error) {
        error->clear();
    }
    return sanitized(readObject(folder, error).value_or(QJsonObject()));
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
        [&folder, &name, &label](QJsonObject &metadata, QString &reason) {
            const QJsonValue value = label.isEmpty() ? QJsonValue() : QJsonValue(label);
            return setMapValue(folder, metadata, QStringLiteral("labels"), name, value, supportedLabel, reason);
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
        [&folder, &name, &color](QJsonObject &metadata, QString &reason) {
            const QJsonValue value = color.isEmpty() ? QJsonValue() : QJsonValue(color);
            return setMapValue(folder, metadata, QStringLiteral("colors"), name, value, supportedColor, reason);
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
        [&folder, &name, rating](QJsonObject &metadata, QString &reason) {
            const QJsonValue value = rating == 0 ? QJsonValue() : QJsonValue(rating);
            return setMapValue(folder, metadata, QStringLiteral("ratings"), name, value, supportedRating, reason);
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
        [&folder, &name, &trimmed](QJsonObject &metadata, QString &reason) {
            const QJsonValue value = trimmed.isEmpty() ? QJsonValue() : QJsonValue(trimmed);
            return setMapValue(folder, metadata, QStringLiteral("notes"), name, value, supportedNote, reason);
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
        [&folder, &name](QJsonObject &metadata, QString &reason) {
            const QString key = QStringLiteral("pins");
            const QJsonValue existing = metadata.value(key);
            if (!isAbsent(existing) && !existing.isArray()) {
                reason = unsupportedValue(folder, key);
                return false;
            }
            QJsonArray pins = existing.toArray();
            QSet<QString> pinned;
            for (const QJsonValue &pin : std::as_const(pins)) {
                if (!pin.isString()) {
                    reason = unsupportedValue(folder, key);
                    return false;
                }
                if (!validName(pin.toString())) {
                    reason = unsafeName(folder, key);
                    return false;
                }
                pinned.insert(pin.toString());
            }
            if (pinned.contains(name)) {
                for (qsizetype i = pins.size() - 1; i >= 0; --i) {
                    if (pins.at(i).toString() == name) {
                        pins.removeAt(i);
                    }
                }
            } else if (pinned.size() < 200) {
                pins.append(name);
            } else {
                reason = QStringLiteral("A folder can have at most 200 pinned items");
                return false;
            }
            if (pins.isEmpty()) {
                metadata.remove(key);
            } else {
                metadata.insert(key, pins);
            }
            return true;
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
        [&folder, &color](QJsonObject &metadata, QString &reason) {
            const QString key = QStringLiteral("color");
            const QJsonValue existing = metadata.value(key);
            if (!isAbsent(existing) && !supportedColor(existing)) {
                reason = unsupportedValue(folder, key);
                return false;
            }
            if (color.isEmpty()) {
                metadata.remove(key);
            } else {
                metadata.insert(key, color);
            }
            return true;
        },
        error);
}

bool FolderMetadata::setHidden(const QString &folder, bool hiddenValue, QString *error)
{
    return mutate(
        folder,
        [&folder, hiddenValue](QJsonObject &metadata, QString &reason) {
            const QString key = QStringLiteral("hidden");
            const QJsonValue existing = metadata.value(key);
            if (!isAbsent(existing) && !existing.isBool()) {
                reason = unsupportedValue(folder, key);
                return false;
            }
            if (hiddenValue) {
                metadata.insert(key, true);
            } else {
                metadata.remove(key);
            }
            return true;
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
    return mutate(
        folder,
        [&folder, &formatValue](QJsonObject &metadata, QString &reason) {
            const QString key = QStringLiteral("format");
            const QJsonValue existing = metadata.value(key);
            if (!isAbsent(existing) && !supportedFormat(existing)) {
                reason = unsupportedValue(folder, key);
                return false;
            }
            // Fields and column ids Plasma Opus doesn't know are kept; unknown columns follow the known ones.
            QJsonObject formatObject = existing.toObject();
            QJsonArray columns = QJsonArray::fromStringList(formatValue.columns);
            for (const QJsonValue &column : formatObject.value(QStringLiteral("columns")).toArray()) {
                if (!AllowedColumns.contains(column.toString())) {
                    columns.append(column);
                }
            }
            formatObject.insert(QStringLiteral("view"), formatValue.view);
            formatObject.insert(QStringLiteral("columns"), columns);
            formatObject.insert(QStringLiteral("sortKey"), formatValue.sortKey.left(19));
            formatObject.insert(QStringLiteral("sortAsc"), formatValue.sortAscending);
            formatObject.insert(QStringLiteral("group"), formatValue.group);
            formatObject.insert(QStringLiteral("includeDescendants"), formatValue.includeDescendants);
            if (formatValue.hasFolderSizePolicy) {
                formatObject.insert(QStringLiteral("calculateFolderSizes"), formatValue.calculateFolderSizes);
            } else {
                formatObject.remove(QStringLiteral("calculateFolderSizes"));
            }
            metadata.insert(key, formatObject);
            return true;
        },
        error);
}

bool FolderMetadata::clearFormat(const QString &folder, QString *error)
{
    return mutate(
        folder,
        [&folder](QJsonObject &metadata, QString &reason) {
            const QString key = QStringLiteral("format");
            Format existing;
            if (!isAbsent(metadata.value(key)) && !format(metadata, &existing)) {
                reason = unsupportedValue(folder, key);
                return false;
            }
            metadata.remove(key);
            return true;
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
