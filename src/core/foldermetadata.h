// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QJsonObject>
#include <QStringList>

namespace FolderMetadata
{
struct Format {
    QString view;
    QStringList columns;
    QString sortKey;
    bool sortAscending = true;
    QString group;
    bool includeDescendants = false;
    bool calculateFolderSizes = false;
    bool hasFolderSizePolicy = false;
};

/// Reads validated metadata from a folder's .mfm.json. Values that fail
/// validation, and keys Plasma Opus doesn't know, are left out.
QJsonObject read(const QString &folder, QString *error = nullptr);

// The setters change only the value they target in the file read just before
// writing, and keep everything else, including keys, entries and format
// fields Plasma Opus doesn't know. A setter refuses the edit and leaves the
// file unchanged, reporting why in @p error, when the file can't be read, when
// the edit would replace or remove a value Plasma Opus doesn't support, when
// the key it changes lists an unsafe file name, or when the result would
// exceed 4,096 bytes. A file left with no keys is deleted.

bool setLabel(const QString &folder, const QString &name, const QString &label, QString *error = nullptr);
bool setColor(const QString &folder, const QString &name, const QString &color, QString *error = nullptr);
bool setRating(const QString &folder, const QString &name, int rating, QString *error = nullptr);
bool setNote(const QString &folder, const QString &name, const QString &note, QString *error = nullptr);
bool togglePin(const QString &folder, const QString &name, QString *error = nullptr);
bool setFolderColor(const QString &folder, const QString &color, QString *error = nullptr);
/// Sets or clears the upstream flag that hides @p folder from its parent folder's listing.
bool setHidden(const QString &folder, bool hidden, QString *error = nullptr);
bool setFormat(const QString &folder, const Format &format, QString *error = nullptr);
bool clearFormat(const QString &folder, QString *error = nullptr);

/// Whether @p metadata sets the flag that hides its folder from the parent folder's listing.
bool hidden(const QJsonObject &metadata);

/// Validates the per-folder view format before it is applied to a view.
bool format(const QJsonObject &metadata, Format *result);

/// Finds the nearest applicable local rule. An exact rule takes precedence
/// over inherited rules; an ancestor applies only when it includes descendants.
bool findFormat(const QString &folder, Format *result, QString *source = nullptr, QString *error = nullptr);
}
