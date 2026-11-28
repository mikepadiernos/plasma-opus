// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QUrl>

namespace KIO
{
class SimpleJob;
}

namespace Symlinks
{
/**
 * Resolves every symbolic link in a local folder path.
 * @param url the folder to open
 * @returns the canonical location for a local path that exists, otherwise
 *          @p url unchanged
 */
QUrl resolve(const QUrl &url);

/// How the path written into a new link refers to its target.
enum class TargetForm {
    Absolute,
    Relative,
};

/// A validated link request, or the reason it cannot be created.
struct Plan {
    QUrl link; ///< where the link is created
    QString target; ///< the path written into the link
    QString error; ///< why the link cannot be created; empty when valid
};

/**
 * Whether links can be created at @p url: local paths, and remote locations
 * whose KIO worker supports linking and whose URL paths are paths on the
 * host. Virtual locations such as the trash or search results are refused.
 */
bool supportsLinking(const QUrl &url);

/// The folder that holds @p target, where a link is created by default.
QUrl parentFolder(const QUrl &target);

/**
 * Why a link to @p target cannot be created in @p folder, or an empty string.
 * Both must be local, or on the same host, port, user and protocol with
 * linking support.
 */
QString destinationError(const QUrl &target, const QUrl &folder);

/// Why @p name is not a valid link name, or an empty string.
QString nameError(const QString &name);

/// The suggested link name for @p targetName using the configured @p suffix.
QString defaultName(const QString &targetName, const QString &suffix);

/**
 * Validates a link request and computes the link URL and target path.
 * Relative local targets are computed from the canonical folders, so they
 * resolve correctly when either folder is reached through a link. Remote
 * paths can't be resolved, so relative remote targets are computed from
 * the paths as given. Local requests fail when an item already has the
 * link's name.
 */
Plan plan(const QUrl &target, const QUrl &folder, const QString &name, TargetForm form = TargetForm::Absolute);

/**
 * Creates a link to @p target named @p name in @p folder without overwriting
 * an existing item. Returns a KIO job, or sets @p error when the request is
 * invalid.
 */
KIO::SimpleJob *create(const QUrl &target, const QUrl &folder, const QString &name, TargetForm form, QString &error);
}
