// SPDX-License-Identifier: GPL-3.0-or-later

#include "foldertree.h"

#include "contentheight.h"
#include "foldercolors.h"
#include "foldergates.h"
#include "hiddenfolders.h"
#include "ignorelist.h"
#include "symbolicicon.h"
#include "treelines.h"

#include <KDirLister>
#include <KDirModel>
#include <KDirSortFilterProxyModel>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QDir>
#include <QDropEvent>
#include <QHeaderView>
#include <QMimeData>
#include <QMouseEvent>
#include <QTreeView>
#include <QVBoxLayout>

namespace
{
QUrl normalized(const QUrl &url)
{
    return url.adjusted(QUrl::StripTrailingSlash | QUrl::NormalizePathSegments);
}

bool isAtOrBelow(const QUrl &url, const QUrl &root)
{
    return normalized(url) == normalized(root) || root.isParentOf(url);
}

/// Sorted folders below one root, without ignored names or folders hidden by their metadata; the root item is titled Home or Root.
class RootProxyModel : public KDirSortFilterProxyModel
{
public:
    RootProxyModel(const QUrl &root, QObject *parent)
        : KDirSortFilterProxyModel(parent)
        , m_ignoreList(IgnoreList::fromConfig(KSharedConfig::openConfig()->group(QStringLiteral("Files"))))
    {
        if (normalized(root) == normalized(QUrl::fromLocalFile(QDir::homePath()))) {
            m_rootTitle = i18nc("@item:intable the user's home folder", "Home");
        } else if (normalized(root) == QUrl::fromLocalFile(QStringLiteral("/"))) {
            m_rootTitle = i18nc("@item:intable the filesystem root", "Root");
        }
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (role == Qt::DisplayRole && !index.parent().isValid() && !m_rootTitle.isEmpty()) {
            return m_rootTitle;
        }
        const QVariant value = KDirSortFilterProxyModel::data(index, role);
        if (role == Qt::DecorationRole && value.typeId() == QMetaType::QIcon) {
            const KFileItem item = index.data(KDirModel::FileItemRole).value<KFileItem>();
            if (FolderGates::instance()->isLockedGateItem(item)) {
                return SymbolicIcon::wrap(QIcon::fromTheme(QStringLiteral("folder-locked"), value.value<QIcon>()), QStringLiteral("folder-locked"), {});
            }
            return FolderColors::decorate(SymbolicIcon::wrap(value.value<QIcon>(), item.iconName(), item.overlays()), item);
        }
        if (role == Qt::ForegroundRole && index.column() == KDirModel::Name) {
            const KFileItem item = index.data(KDirModel::FileItemRole).value<KFileItem>();
            if (const QColor color = FolderColors::nameColor(item, QGuiApplication::palette().color(QPalette::Base)); color.isValid()) {
                return color;
            }
        }
        return value;
    }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
        const KFileItem item = index.data(KDirModel::FileItemRole).value<KFileItem>();
        if (!KDirSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent) || m_ignoreList.ignores(item.name())
            || FolderGates::instance()->hidesItem(item)) {
            return false;
        }
        // The root item stays, like a root whose name starts with a dot.
        const KDirLister *lister = static_cast<const KDirModel *>(sourceModel())->dirLister();
        return !sourceParent.isValid() || !HiddenFolders::instance()->hidesItem(item, lister->showHiddenFiles());
    }

public:
    /// Filters rows again after password-gated folders are locked or unlocked, hidden folders are shown or hidden, or folder metadata hides or shows folders.
    void refilter()
    {
        beginFilterChange();
        endFilterChange(QSortFilterProxyModel::Direction::Rows);
    }

private:
    QString m_rootTitle;
    IgnoreList m_ignoreList;
};

QUrl urlAt(const QModelIndex &index)
{
    return index.data(KDirModel::FileItemRole).value<KFileItem>().url();
}

/// A folder tree with Plasma Opus tree lines.
class FolderTreeView : public QTreeView
{
public:
    using QTreeView::QTreeView;

protected:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override
    {
        if (!TreeLines::drawBranches(this, painter, rect, index)) {
            QTreeView::drawBranches(painter, rect, index);
        }
    }
};
}

FolderTree::FolderTree(const QList<QUrl> &roots, QWidget *parent)
    : QWidget(parent)
    , m_roots(roots)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    for (const QUrl &root : roots) {
        auto *model = new KDirModel(this);
        model->dirLister()->setDirOnlyMode(true);
        model->dirLister()->setAutoErrorHandlingEnabled(false);
        auto *proxy = new RootProxyModel(root, this);
        proxy->setSourceModel(model);
        proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
        connect(FolderGates::instance(), &FolderGates::changed, proxy, &RootProxyModel::refilter);
        HiddenFolders::instance()->track(model->dirLister());
        connect(HiddenFolders::instance(), &HiddenFolders::changed, proxy, &RootProxyModel::refilter);

        auto *tree = new FolderTreeView(this);
        tree->setModel(proxy);
        tree->setHeaderHidden(true);
        for (int column = 1; column < KDirModel::ColumnCount; ++column) {
            tree->setColumnHidden(column, true);
        }
        tree->setSortingEnabled(true);
        tree->sortByColumn(KDirModel::Name, Qt::AscendingOrder);
        tree->setUniformRowHeights(true);
        tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
        fitHeightToContents(tree);
        tree->viewport()->installEventFilter(this);
        layout->addWidget(tree);

        connect(tree, &QTreeView::clicked, this, [this](const QModelIndex &index) {
            Q_EMIT urlActivated(urlAt(index));
        });
        connect(model, &KDirModel::expand, this, [this, tree, proxy, model](const QModelIndex &index) {
            if (normalized(model->itemForIndex(index).url()) == normalized(m_pendingReveal)) {
                tryReveal();
            } else {
                tree->expand(proxy->mapFromSource(index));
            }
        });
        connect(model, &KDirModel::rowsInserted, this, &FolderTree::tryReveal);
        tree->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tree, &QTreeView::customContextMenuRequested, this, [this, tree](const QPoint &pos) {
            const QModelIndex index = tree->indexAt(pos);
            if (index.isValid()) {
                Q_EMIT contextMenuRequested(urlAt(index), tree->viewport()->mapToGlobal(pos));
            }
        });
        tree->viewport()->setAcceptDrops(true);
        model->openUrl(root, KDirModel::ShowRoot);
        m_trees.append(tree);
    }
    layout->addStretch();
}

QList<QUrl> FolderTree::roots() const
{
    return m_roots;
}

QList<QUrl> FolderTree::expandedUrls() const
{
    QList<QUrl> urls;
    for (const QTreeView *tree : m_trees) {
        QList<QModelIndex> pending{QModelIndex()};
        while (!pending.isEmpty()) {
            const QModelIndex parent = pending.takeFirst();
            for (int row = 0; row < tree->model()->rowCount(parent); ++row) {
                const QModelIndex child = tree->model()->index(row, 0, parent);
                if (tree->isExpanded(child)) {
                    urls.append(urlAt(child));
                    pending.append(child);
                }
            }
        }
    }
    return urls;
}

void FolderTree::setExpandedUrls(const QList<QUrl> &urls)
{
    for (int i = 0; i < m_trees.size(); ++i) {
        auto *proxy = static_cast<KDirSortFilterProxyModel *>(m_trees.at(i)->model());
        auto *model = static_cast<KDirModel *>(proxy->sourceModel());
        for (const QUrl &url : urls) {
            if (isAtOrBelow(url, m_roots.at(i))) {
                model->expandToUrl(url);
            }
        }
    }
}

void FolderTree::reload()
{
    const QList<QUrl> expanded = expandedUrls();
    for (int i = 0; i < m_trees.size(); ++i) {
        auto *proxy = static_cast<KDirSortFilterProxyModel *>(m_trees.at(i)->model());
        auto *model = static_cast<KDirModel *>(proxy->sourceModel());
        model->openUrl(m_roots.at(i), KDirModel::Reload | KDirModel::ShowRoot);
    }
    setExpandedUrls(expanded);
}

void FolderTree::reveal(const QUrl &url)
{
    int innermost = -1;
    for (int i = 0; i < m_roots.size(); ++i) {
        if (isAtOrBelow(url, m_roots.at(i)) && (innermost < 0 || m_roots.at(innermost).isParentOf(m_roots.at(i)))) {
            innermost = i;
        }
    }
    if (innermost < 0) {
        return;
    }
    for (QTreeView *tree : std::as_const(m_trees)) {
        tree->selectionModel()->clear();
    }
    m_pendingReveal = url;
    m_pendingTree = innermost;
    auto *proxy = static_cast<KDirSortFilterProxyModel *>(m_trees.at(innermost)->model());
    static_cast<KDirModel *>(proxy->sourceModel())->expandToUrl(url);
    tryReveal();
}

void FolderTree::tryReveal()
{
    if (m_pendingTree < 0) {
        return;
    }
    QTreeView *tree = m_trees.at(m_pendingTree);
    auto *proxy = static_cast<KDirSortFilterProxyModel *>(tree->model());
    const QModelIndex source = static_cast<KDirModel *>(proxy->sourceModel())->indexForUrl(m_pendingReveal);
    if (!source.isValid()) {
        return;
    }
    const QModelIndex index = proxy->mapFromSource(source);
    m_pendingTree = -1;
    m_pendingReveal.clear();
    tree->setCurrentIndex(index);
    m_revealedTree = tree;
    Q_EMIT revealed();
}

QRect FolderTree::revealedArea() const
{
    if (!m_revealedTree || !m_revealedTree->currentIndex().isValid()) {
        return {};
    }
    const QRect row = m_revealedTree->visualRect(m_revealedTree->currentIndex());
    return QRect(m_revealedTree->viewport()->mapTo(this, row.topLeft()), row.size());
}

QTreeView *FolderTree::treeForViewport(const QObject *viewport) const
{
    for (QTreeView *tree : m_trees) {
        if (tree->viewport() == viewport) {
            return tree;
        }
    }
    return nullptr;
}

bool FolderTree::showHiddenFolders() const
{
    return m_showHiddenFolders;
}

void FolderTree::setShowHiddenFolders(bool show)
{
    if (show == m_showHiddenFolders) {
        return;
    }
    m_showHiddenFolders = show;
    for (const QTreeView *tree : std::as_const(m_trees)) {
        auto *proxy = static_cast<RootProxyModel *>(tree->model());
        KDirLister *lister = static_cast<KDirModel *>(proxy->sourceModel())->dirLister();
        lister->setShowHiddenFiles(show);
        lister->emitChanges();
        proxy->refilter();
    }
}

bool FolderTree::eventFilter(QObject *watched, QEvent *event)
{
    QTreeView *tree = treeForViewport(watched);
    if (!tree) {
        return QWidget::eventFilter(watched, event);
    }
    switch (event->type()) {
    case QEvent::MouseButtonRelease: {
        auto *mouse = static_cast<QMouseEvent *>(event);
        const QModelIndex index = tree->indexAt(mouse->position().toPoint());
        if (mouse->button() == Qt::MiddleButton && index.isValid()) {
            Q_EMIT tabRequested(urlAt(index));
            return true;
        }
        break;
    }
    case QEvent::DragEnter:
    case QEvent::DragMove:
    case QEvent::Drop: {
        auto *drop = static_cast<QDropEvent *>(event);
        const QModelIndex index = tree->indexAt(drop->position().toPoint());
        if (!drop->mimeData()->hasUrls() || !index.isValid()) {
            drop->ignore();
            return true;
        }
        drop->acceptProposedAction();
        if (event->type() == QEvent::Drop) {
            Q_EMIT urlsDropped(urlAt(index), drop);
        }
        return true;
    }
    default:
        break;
    }
    return QWidget::eventFilter(watched, event);
}
