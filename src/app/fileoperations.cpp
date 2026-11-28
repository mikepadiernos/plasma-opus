// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileoperations.h"

#include "activityreporting.h"
#include "executables.h"
#include "foldergates.h"
#include "librarymovetracker.h"
#include "settings.h"
#include "symlinks.h"
#include "transferqueue.h"

#include <KIO/CopyJob>
#include <KIO/DeleteOrTrashJob>
#include <KIO/DropJob>
#include <KIO/FileUndoManager>
#include <KIO/JobUiDelegate>
#include <KIO/JobUiDelegateFactory>
#include <KIO/OpenUrlJob>
#include <KIO/Paste>
#include <KIO/PasteJob>
#include <KIO/RenameFileDialog>
#include <KIO/SimpleJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPropertiesDialog>
#include <KTerminalLauncherJob>

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QDropEvent>
#include <QFileInfo>
#include <QMimeData>
#include <QPointer>
#include <QProcess>
#include <QStandardPaths>

#include <sys/stat.h>

namespace
{
void attachWindow(KJob *job, QWidget *window)
{
    job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, window));
    KJobWidgets::setWindow(job, window);
}

/**
 * Whether a transfer is refused because a source is, is inside or contains a
 * locked password-gated folder, or the destination is inside one; reports
 * the refusal. Copies, the trash and folders outside Plasma Opus would
 * otherwise show the contents without the password.
 */
bool refusedByGates(const QList<QUrl> &sources, const QUrl &destination, QWidget *window)
{
    const FolderGates *gates = FolderGates::instance();
    if (!gates->hasLocked()) {
        return false;
    }
    const bool refused = (destination.isValid() && gates->isLocked(destination)) || std::any_of(sources.cbegin(), sources.cend(), [gates](const QUrl &url) {
                             return gates->touchesLocked(url);
                         });
    if (refused) {
        KMessageBox::error(window,
                           i18nc("@info",
                                 "Items in or containing a folder that is protected with a password cannot be copied, moved or deleted until the folder "
                                 "is unlocked."),
                           i18nc("@title:window", "Folder Is Locked"));
    }
    return refused;
}

/// Moves password gates along with folders that a move job moves.
void trackGateMoves(KIO::CopyJob *job)
{
    if (!job || job->operationMode() != KIO::CopyJob::Move || FolderGates::instance()->folders().isEmpty()) {
        return;
    }
    QObject::connect(job,
                     &KIO::CopyJob::copyingDone,
                     FolderGates::instance(),
                     [](KIO::Job *, const QUrl &from, const QUrl &to, const QDateTime &, bool directory) {
                         if (directory && from.isLocalFile() && to.isLocalFile()) {
                             FolderGates::instance()->relocate(from.toLocalFile(), to.toLocalFile());
                         }
                     });
}

QString fileIdentity(const QString &path)
{
    struct stat info{};
    if (::lstat(QFile::encodeName(path).constData(), &info) != 0) {
        return {};
    }
    return QString::number(info.st_dev) + u':' + QString::number(info.st_ino);
}

/// Moves password gates along with folders that a rename dialog renames, matched by device and inode.
void trackGateRenames(KIO::RenameFileDialog *dialog, const KFileItemList &items)
{
    const QStringList gates = FolderGates::instance()->folders();
    QHash<QString, QString> sources;
    for (const KFileItem &item : items) {
        const QString path = FolderGates::canonicalPath(FolderGates::localPath(item.url()));
        const bool hasGate = !path.isEmpty() && std::any_of(gates.cbegin(), gates.cend(), [&path](const QString &gate) {
            return gate == path || gate.startsWith(path + u'/');
        });
        if (hasGate) {
            sources.insert(fileIdentity(path), path);
        }
    }
    sources.remove(QString());
    if (sources.isEmpty()) {
        return;
    }
    QObject::connect(dialog, &KIO::RenameFileDialog::renamingFinished, FolderGates::instance(), [sources](const QList<QUrl> &urls) {
        for (const QUrl &url : urls) {
            const QString path = FolderGates::localPath(url);
            const QString from = path.isEmpty() ? QString() : sources.value(fileIdentity(path));
            if (!from.isEmpty()) {
                FolderGates::instance()->relocate(from, path);
            }
        }
    });
}
}

QString FileOperations::openWithMpv(const QList<QUrl> &urls)
{
    QStringList arguments{QStringLiteral("--")};
    for (const QUrl &url : urls) {
        if (!url.isLocalFile()) {
            return i18nc("@info", "Opening media in mpv requires local files.");
        }
        arguments.append(url.toLocalFile());
    }
    if (urls.isEmpty()) {
        return i18nc("@info", "Select a media file to open in mpv.");
    }
    const QString executable = QStandardPaths::findExecutable(QStringLiteral("mpv"));
    if (executable.isEmpty()) {
        return i18nc("@info", "The mpv application is not installed or is not in PATH.");
    }
    if (!QProcess::startDetached(executable, arguments)) {
        return i18nc("@info", "Could not start the mpv application.");
    }
    return {};
}

TransferQueue *FileOperations::transferQueue()
{
    static auto *queue = new TransferQueue(qApp);
    queue->setConcurrency(Settings::transferQueueEnabled() ? Settings::transferQueueConcurrency() : 0);
    return queue;
}

KJob *FileOperations::copy(const QList<QUrl> &sources, const QUrl &destination, QWidget *window)
{
    if (sources.isEmpty() || !destination.isValid() || refusedByGates(sources, destination, window)) {
        return nullptr;
    }
    const QPointer<QWidget> safeWindow(window);
    const int conflict = Settings::transferConflictPolicy();
    return transferQueue()->enqueue(
        i18np("Copy one item to %2", "Copy %1 items to %2", sources.size(), destination.toDisplayString()),
        [sources, destination, safeWindow, conflict] {
            KIO::CopyJob *job =
                KIO::copy(sources, destination, conflict == Settings::EnumTransferConflictPolicy::Overwrite ? KIO::Overwrite : KIO::DefaultFlags);
            job->setAutoSkip(conflict == Settings::EnumTransferConflictPolicy::Skip);
            job->setAutoRename(conflict == Settings::EnumTransferConflictPolicy::Rename);
            attachWindow(job, safeWindow);
            KIO::FileUndoManager::self()->recordCopyJob(job);
            return job;
        });
}

KJob *FileOperations::move(const QList<QUrl> &sources, const QUrl &destination, QWidget *window)
{
    if (sources.isEmpty() || !destination.isValid() || refusedByGates(sources, destination, window)) {
        return nullptr;
    }
    const QPointer<QWidget> safeWindow(window);
    const int conflict = Settings::transferConflictPolicy();
    return transferQueue()->enqueue(
        i18np("Move one item to %2", "Move %1 items to %2", sources.size(), destination.toDisplayString()),
        [sources, destination, safeWindow, conflict] {
            KIO::CopyJob *job =
                KIO::move(sources, destination, conflict == Settings::EnumTransferConflictPolicy::Overwrite ? KIO::Overwrite : KIO::DefaultFlags);
            job->setAutoSkip(conflict == Settings::EnumTransferConflictPolicy::Skip);
            job->setAutoRename(conflict == Settings::EnumTransferConflictPolicy::Rename);
            attachWindow(job, safeWindow);
            KIO::FileUndoManager::self()->recordCopyJob(job);
            LibraryMoveTracker::instance()->trackCopyJob(job);
            trackGateMoves(job);
            return job;
        });
}

KJob *FileOperations::trash(const QList<QUrl> &urls, QWidget *window)
{
    if (urls.isEmpty() || refusedByGates(urls, QUrl(), window)) {
        return nullptr;
    }
    auto *job = new KIO::DeleteOrTrashJob(urls, KIO::AskUserActionInterface::Trash, KIO::AskUserActionInterface::DefaultConfirmation, window);
    job->start();
    return job;
}

KJob *FileOperations::deletePermanently(const QList<QUrl> &urls, QWidget *window)
{
    if (urls.isEmpty() || refusedByGates(urls, QUrl(), window)) {
        return nullptr;
    }
    auto *job = new KIO::DeleteOrTrashJob(urls, KIO::AskUserActionInterface::Delete, KIO::AskUserActionInterface::DefaultConfirmation, window);
    job->start();
    return job;
}

void FileOperations::rename(const KFileItemList &items, QWidget *window)
{
    if (items.isEmpty()) {
        return;
    }
    auto *dialog = new KIO::RenameFileDialog(items, window);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    LibraryMoveTracker::instance()->trackRenameDialog(dialog, items);
    trackGateRenames(dialog, items);
    dialog->show();
}

KJob *FileOperations::createSymlink(const QUrl &target, const QUrl &folder, const QString &name, bool relative, QString &error, QWidget *window)
{
    KIO::SimpleJob *job = Symlinks::create(target, folder, name, relative ? Symlinks::TargetForm::Relative : Symlinks::TargetForm::Absolute, error);
    if (job) {
        attachWindow(job, window);
    }
    return job;
}

void FileOperations::copyToClipboard(const QList<QUrl> &urls, bool cut)
{
    if (urls.isEmpty()) {
        return;
    }
    auto *mimeData = new QMimeData;
    mimeData->setUrls(urls);
    KIO::setClipboardDataCut(mimeData, cut);
    QApplication::clipboard()->setMimeData(mimeData);
}

bool FileOperations::paste(const QUrl &destination, QWidget *window)
{
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    if (!mimeData || !destination.isValid()) {
        return false;
    }
    bool enabled = false;
    KIO::pasteActionText(mimeData, &enabled, KFileItem(destination));
    if (!enabled) {
        return false;
    }
    if (refusedByGates(mimeData->urls(), destination, window)) {
        // The refusal is reported, so callers do not report it again.
        return true;
    }
    KIO::PasteJob *job = KIO::paste(mimeData, destination);
    attachWindow(job, window);
    QObject::connect(job, &KIO::PasteJob::copyJobStarted, LibraryMoveTracker::instance(), &LibraryMoveTracker::trackCopyJob);
    QObject::connect(job, &KIO::PasteJob::copyJobStarted, FolderGates::instance(), &trackGateMoves);
    return true;
}

QString FileOperations::pasteActionText(const KFileItem &destination, bool *enabled)
{
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    if (!mimeData) {
        if (enabled) {
            *enabled = false;
        }
        return QString();
    }
    return KIO::pasteActionText(mimeData, enabled, destination);
}

void FileOperations::open(const KFileItem &item, QWidget *window)
{
    if (item.isNull()) {
        return;
    }
    ActivityReporting::reportUsed(item.mostLocalUrl());
    const auto action = Settings::executableAction();
    const QString path = item.localPath();
    const bool program = !path.isEmpty() && !item.isDir() && Executables::isProgram(item.mimetype(), QFileInfo(path).isExecutable());
    if (program && (action == Settings::EnumExecutableAction::RunInTerminal || action == Settings::EnumExecutableAction::RunInTerminalKeepOpen)) {
        auto *terminal = new KTerminalLauncherJob(Executables::terminalCommand(path, action == Settings::EnumExecutableAction::RunInTerminalKeepOpen));
        terminal->setWorkingDirectory(QFileInfo(path).absolutePath());
        terminal->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, window));
        terminal->start();
        return;
    }
    auto *job = new KIO::OpenUrlJob(item.targetUrl(), item.mimetype());
    job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, window));
    job->setRunExecutables(action == Settings::EnumExecutableAction::Run);
    job->setShowOpenOrExecuteDialog(action == Settings::EnumExecutableAction::Ask);
    job->start();
}

KIO::DropJob *FileOperations::drop(const QDropEvent *event, const QUrl &destination, QWidget *window)
{
    if (refusedByGates(event->mimeData() ? event->mimeData()->urls() : QList<QUrl>(), destination, window)) {
        return nullptr;
    }
    const auto setting = Settings::dropAction();
    const bool modifierHeld = event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier);
    KIO::DropJob *job = nullptr;
    if (modifierHeld || setting == Settings::EnumDropAction::Ask) {
        job = KIO::drop(event, destination);
    } else {
        const bool copy = setting == Settings::EnumDropAction::Copy;
        QDropEvent decided(event->position(), event->possibleActions(), event->mimeData(), event->buttons(), copy ? Qt::ControlModifier : Qt::ShiftModifier);
        decided.setDropAction(copy ? Qt::CopyAction : Qt::MoveAction);
        job = KIO::drop(&decided, destination);
    }
    KJobWidgets::setWindow(job, window);
    QObject::connect(job, &KIO::DropJob::copyJobStarted, LibraryMoveTracker::instance(), &LibraryMoveTracker::trackCopyJob);
    QObject::connect(job, &KIO::DropJob::copyJobStarted, FolderGates::instance(), &trackGateMoves);
    return job;
}

void FileOperations::openTerminal(const QUrl &folder, QWidget *window)
{
    if (FolderGates::instance()->isLocked(folder)) {
        KMessageBox::error(window, i18nc("@info", "A terminal cannot be opened in a folder that is protected with a password until it is unlocked."));
        return;
    }
    auto *job = new KTerminalLauncherJob(QString());
    job->setWorkingDirectory(folder.isLocalFile() ? folder.toLocalFile() : QString());
    job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, window));
    job->start();
}

void FileOperations::showProperties(const KFileItemList &items, QWidget *window)
{
    if (items.isEmpty()) {
        return;
    }
    KPropertiesDialog::showDialog(items, window, false);
}
