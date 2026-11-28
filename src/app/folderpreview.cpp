// SPDX-License-Identifier: GPL-3.0-or-later

#include "folderpreview.h"
#include "foldergates.h"
#include "hiddenfolders.h"

#include <KDirLister>
#include <KDirModel>
#include <KDirSortFilterProxyModel>
#include <KFileItem>
#include <KIO/Global>
#include <KIO/Job>
#include <KLocalizedString>

#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QScrollBar>
#include <QTimer>
#include <QVBoxLayout>

namespace
{
constexpr int RefreshDelay = 50;

/// Hides folders their metadata hides from their parent folder's listing.
class HiddenFolderProxyModel : public KDirSortFilterProxyModel
{
public:
    using KDirSortFilterProxyModel::KDirSortFilterProxyModel;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        if (!KDirSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent)) {
            return false;
        }
        const auto *model = static_cast<const KDirModel *>(sourceModel());
        const KFileItem item = model->itemForIndex(model->index(sourceRow, 0, sourceParent));
        return !HiddenFolders::instance()->hidesItem(item, model->dirLister()->showHiddenFiles());
    }
};
}

FolderPreview::FolderPreview(QWidget *parent)
    : QWidget(parent)
    , m_model(new KDirModel(this))
    , m_proxy(new HiddenFolderProxyModel(this))
    , m_list(new QListWidget(this))
    , m_note(new QLabel(this))
    , m_refresh(new QTimer(this))
    , m_timeout(new QTimer(this))
{
    setObjectName(QStringLiteral("previewFolder"));
    resetLister();
    connect(FolderGates::instance(), &FolderGates::changed, this, [this] {
        if (m_url.isValid() && FolderGates::instance()->isLocked(m_url)) {
            showLocked();
        }
    });
    connect(HiddenFolders::instance(), &HiddenFolders::changed, m_proxy, &QSortFilterProxyModel::invalidate);
    m_proxy->setSortFoldersFirst(true);
    m_proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->sort(KDirModel::Name, Qt::AscendingOrder);
    connect(m_proxy, &QAbstractItemModel::rowsInserted, this, &FolderPreview::scheduleRefresh);
    connect(m_proxy, &QAbstractItemModel::rowsRemoved, this, &FolderPreview::scheduleRefresh);
    connect(m_proxy, &QAbstractItemModel::layoutChanged, this, &FolderPreview::scheduleRefresh);
    connect(m_proxy, &QAbstractItemModel::modelReset, this, &FolderPreview::scheduleRefresh);
    connect(m_proxy, &QAbstractItemModel::dataChanged, this, &FolderPreview::scheduleRefresh);

    m_refresh->setSingleShot(true);
    m_refresh->setInterval(RefreshDelay);
    connect(m_refresh, &QTimer::timeout, this, &FolderPreview::refresh);
    m_timeout->setSingleShot(true);
    m_timeout->setInterval(ListingTimeout);
    connect(m_timeout, &QTimer::timeout, this, [this] {
        if (!m_listing) {
            return;
        }
        m_timedOut = true;
        m_lister->stop();
        setListing(false);
        refresh();
    });

    m_list->setObjectName(QStringLiteral("previewFolderContents"));
    m_list->setAccessibleName(i18nc("@info:accessibility", "Folder contents"));
    m_list->setFocusPolicy(Qt::NoFocus);
    m_list->setSelectionMode(QAbstractItemView::NoSelection);
    m_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_list->setDragDropMode(QAbstractItemView::NoDragDrop);
    m_list->setUniformItemSizes(true);
    m_list->setIconSize(QSize(16, 16));
    m_list->setTextElideMode(Qt::ElideMiddle);
    m_list->setFrameShape(QFrame::NoFrame);
    m_note->setObjectName(QStringLiteral("previewFolderNote"));
    m_note->setTextFormat(Qt::PlainText);
    m_note->setWordWrap(true);
    m_note->hide();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_list, 1);
    layout->addWidget(m_note);
}

FolderPreview::~FolderPreview()
{
    m_lister->disconnect(this);
}

void FolderPreview::resetLister()
{
    if (m_lister) {
        m_lister->disconnect(this);
    }
    m_lister = new KDirLister(this);
    m_lister->setAutoErrorHandlingEnabled(false);
    m_lister->setShowHiddenFiles(m_showHiddenFiles);
    // The model deletes the lister it replaces, which stops its listing, and
    // drops its rows without announcing a reset, so detach the proxy first.
    m_proxy->setSourceModel(nullptr);
    m_model->setDirLister(m_lister);
    m_proxy->setSourceModel(m_model);
    HiddenFolders::instance()->track(m_lister);
    const auto finished = [this] {
        m_timeout->stop();
        setListing(false);
        refresh();
    };
    connect(m_lister, &KDirLister::completed, this, finished);
    connect(m_lister, &KDirLister::canceled, this, finished);
    connect(m_lister, &KDirLister::jobError, this, [this](KIO::Job *job) {
        m_error = job->errorString();
        scheduleRefresh();
    });
}

void FolderPreview::setFolder(const KFileItem &item)
{
    clear();
    if (item.isNull()) {
        return;
    }
    m_url = item.targetUrl();
    if (FolderGates::instance()->isLocked(item.url()) || FolderGates::instance()->isLocked(m_url)) {
        showLocked();
        return;
    }
    setListing(true);
    m_timeout->start();
    if (!m_lister->openUrl(m_url)) {
        m_timeout->stop();
        if (m_error.isEmpty()) {
            m_error = KIO::buildErrorString(KIO::ERR_CANNOT_ENTER_DIRECTORY, m_url.toDisplayString(QUrl::PreferLocalFile));
        }
        setListing(false);
    }
    refresh();
}

void FolderPreview::showLocked()
{
    const QUrl url = m_url;
    clear();
    m_url = url;
    m_error = i18nc("@info:status", "This folder is protected with a password.");
    refresh();
}

QUrl FolderPreview::url() const
{
    return m_url;
}

void FolderPreview::clear()
{
    m_timeout->stop();
    m_refresh->stop();
    m_url = {};
    m_error.clear();
    m_timedOut = false;
    resetLister();
    setListing(false);
    m_list->clear();
    m_note->clear();
    m_note->hide();
}

void FolderPreview::setShowHiddenFiles(bool show)
{
    if (show == m_showHiddenFiles) {
        return;
    }
    m_showHiddenFiles = show;
    m_lister->setShowHiddenFiles(show);
    m_lister->emitChanges();
    m_proxy->invalidate();
}

bool FolderPreview::showHiddenFiles() const
{
    return m_showHiddenFiles;
}

bool FolderPreview::isListing() const
{
    return m_listing;
}

int FolderPreview::entryCount() const
{
    return m_proxy->rowCount();
}

QListWidget *FolderPreview::list() const
{
    return m_list;
}

QLabel *FolderPreview::note() const
{
    return m_note;
}

void FolderPreview::scheduleRefresh()
{
    if (!m_url.isEmpty() && !m_refresh->isActive()) {
        m_refresh->start();
    }
}

void FolderPreview::refresh()
{
    m_refresh->stop();
    const int count = m_proxy->rowCount();
    const int shown = qMin(count, EntryLimit);
    const int scroll = m_list->verticalScrollBar()->value();
    m_list->setUpdatesEnabled(false);
    m_list->clear();
    for (int row = 0; row < shown; ++row) {
        const QModelIndex index = m_proxy->index(row, KDirModel::Name);
        new QListWidgetItem(index.data(Qt::DecorationRole).value<QIcon>(), index.data(Qt::DisplayRole).toString(), m_list);
    }
    m_list->verticalScrollBar()->setValue(scroll);
    m_list->setUpdatesEnabled(true);

    QStringList notes;
    if (count > shown) {
        notes.append(i18ncp("@info %1 is how many items of a folder the preview leaves out", "and %1 more…", "and %1 more…", count - shown));
    }
    if (!m_error.isEmpty()) {
        notes.append(i18nc("@info", "Could not list this folder: %1", m_error));
    } else if (m_timedOut) {
        notes.append(i18nc("@info", "Stopped listing because this folder is slow to respond."));
    } else if (m_listing) {
        notes.append(i18nc("@info", "Loading preview…"));
    } else if (count == 0 && !m_url.isEmpty()) {
        notes.append(i18nc("@info", "This folder is empty."));
    }
    m_note->setText(notes.join(QLatin1Char('\n')));
    m_note->setVisible(!notes.isEmpty());
}

void FolderPreview::setListing(bool listing)
{
    if (listing == m_listing) {
        return;
    }
    m_listing = listing;
    Q_EMIT listingChanged(listing);
}
