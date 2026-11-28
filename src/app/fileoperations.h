// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KFileItem>

#include <QList>
#include <QUrl>

class QDropEvent;
class QWidget;
class KJob;
class TransferQueue;

namespace KIO
{
class DropJob;
}

/**
 * Thin wrappers around KIO jobs. Every job gets the main window as its
 * UI delegate parent so confirmation and rename dialogs are modal to it,
 * and every undoable job is recorded with KIO::FileUndoManager.
 */
namespace FileOperations
{
TransferQueue *transferQueue();
KJob *copy(const QList<QUrl> &sources, const QUrl &destination, QWidget *window);
KJob *move(const QList<QUrl> &sources, const QUrl &destination, QWidget *window);
KJob *trash(const QList<QUrl> &urls, QWidget *window);
KJob *deletePermanently(const QList<QUrl> &urls, QWidget *window);
void rename(const KFileItemList &items, QWidget *window);
KJob *createSymlink(const QUrl &target, const QUrl &folder, const QString &name, bool relative, QString &error, QWidget *window);

void copyToClipboard(const QList<QUrl> &urls, bool cut);
/// Returns false when the clipboard holds nothing pasteable.
bool paste(const QUrl &destination, QWidget *window);
/// The text KIO suggests for the paste action given the current clipboard.
QString pasteActionText(const KFileItem &destination, bool *enabled);

/**
 * Opens an item in its application. A program is asked about, opened, run,
 * or run in a terminal as the ExecutableAction setting says.
 * @param item the file or folder to open
 * @param window the parent of any dialog
 */
void open(const KFileItem &item, QWidget *window);

/**
 * Opens local media files in the separate mpv application.
 * @param urls local media URLs to pass to mpv
 * @returns an error message, or an empty string on success
 */
QString openWithMpv(const QList<QUrl> &urls);

/**
 * Starts a KIO drop. Without a modifier key held, the DropAction setting
 * can copy or move without asking, as if Ctrl or Shift were held.
 * @param event the drop event
 * @param destination the folder dropped on
 * @param window the parent of the drop menu and dialogs
 * @returns the started job
 */
KIO::DropJob *drop(const QDropEvent *event, const QUrl &destination, QWidget *window);
void openTerminal(const QUrl &folder, QWidget *window);
void showProperties(const KFileItemList &items, QWidget *window);
}
