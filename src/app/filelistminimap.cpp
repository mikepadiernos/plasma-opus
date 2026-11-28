// SPDX-License-Identifier: GPL-3.0-or-later

#include "filelistminimap.h"

#include <KDirModel>
#include <KFileItem>
#include <KLocalizedString>

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QListView>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QTreeView>
#include <QWheelEvent>

namespace
{
constexpr int DefaultMinimapWidth = 14;
constexpr int MinimumMinimapWidth = 8;
constexpr int MaximumMinimapWidth = 64;
constexpr int MinimumViewportHeight = 8;

Qt::Orientation scrollOrientation(const QAbstractItemView *view)
{
    const auto *list = qobject_cast<const QListView *>(view);
    if (!list) {
        return Qt::Vertical;
    }
    // Items flow along one axis and, when wrapping, scroll along the other.
    const bool topToBottom = list->flow() == QListView::TopToBottom;
    return topToBottom == list->isWrapping() ? Qt::Horizontal : Qt::Vertical;
}

QColor blend(const MinimapBands::Band &band, const QPalette &palette)
{
    struct Share {
        QColor color;
        int count;
    };
    QColor folder = palette.color(QPalette::Highlight);
    folder.setAlpha(200);
    QColor file = palette.color(QPalette::Text);
    file.setAlpha(110);
    QColor unknown = palette.color(QPalette::Mid);
    unknown.setAlpha(110);
    const Share shares[] = {{folder, band.folders}, {file, band.files}, {unknown, band.unknown}};
    const int total = band.total();
    float red = 0;
    float green = 0;
    float blue = 0;
    float alpha = 0;
    for (const Share &share : shares) {
        const float weight = float(share.count) / float(total);
        red += share.color.redF() * weight;
        green += share.color.greenF() * weight;
        blue += share.color.blueF() * weight;
        alpha += share.color.alphaF() * weight;
    }
    return QColor::fromRgbF(red, green, blue, alpha);
}
}

FileListMinimap::FileListMinimap(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("fileListMinimap"));
    setAccessibleName(i18nc("@label", "File list minimap"));
    setToolTip(i18nc("@info:tooltip", "Click or drag to scroll the file list"));
    setFocusPolicy(Qt::NoFocus);
    setMinimapWidth(DefaultMinimapWidth);
    setMinimumHeight(MinimumViewportHeight);
}

void FileListMinimap::setMinimapWidth(int width)
{
    setFixedWidth(qBound(MinimumMinimapWidth, width, MaximumMinimapWidth));
}

void FileListMinimap::setView(QAbstractItemView *view)
{
    if (m_view == view) {
        return;
    }
    for (const QMetaObject::Connection &connection : std::as_const(m_viewConnections)) {
        disconnect(connection);
    }
    m_viewConnections.clear();

    m_view = view;
    m_model = view ? view->model() : nullptr;
    m_orientation = scrollOrientation(view);
    m_scrollBar = view ? (m_orientation == Qt::Horizontal ? view->horizontalScrollBar() : view->verticalScrollBar()) : nullptr;
    const auto invalidate = [this] {
        invalidateItems();
    };
    if (m_model) {
        m_viewConnections << connect(m_model, &QAbstractItemModel::dataChanged, this, invalidate)
                          << connect(m_model, &QAbstractItemModel::layoutChanged, this, invalidate)
                          << connect(m_model, &QAbstractItemModel::modelReset, this, invalidate)
                          << connect(m_model, &QAbstractItemModel::rowsInserted, this, invalidate)
                          << connect(m_model, &QAbstractItemModel::rowsRemoved, this, invalidate)
                          << connect(m_model, &QAbstractItemModel::rowsMoved, this, invalidate);
    }
    if (auto *tree = qobject_cast<QTreeView *>(view)) {
        m_viewConnections << connect(tree, &QTreeView::expanded, this, invalidate) << connect(tree, &QTreeView::collapsed, this, invalidate);
    }
    if (m_scrollBar) {
        const auto repaint = [this] {
            update();
        };
        m_viewConnections << connect(m_scrollBar, &QScrollBar::valueChanged, this, repaint) << connect(m_scrollBar, &QScrollBar::rangeChanged, this, repaint);
    }
    invalidateItems();
}

int FileListMinimap::itemCount() const
{
    ensureItems();
    return int(m_items.size());
}

Qt::Orientation FileListMinimap::trackedOrientation() const
{
    return m_orientation;
}

void FileListMinimap::invalidateItems()
{
    m_itemsValid = false;
    update();
}

void FileListMinimap::ensureItems() const
{
    if (m_itemsValid) {
        return;
    }
    m_items.clear();
    if (m_model) {
        collectItems(QModelIndex());
    }
    m_itemsValid = true;
}

void FileListMinimap::collectItems(const QModelIndex &parent) const
{
    const auto *tree = qobject_cast<const QTreeView *>(m_view);
    const auto *list = qobject_cast<const QListView *>(m_view);
    const int rows = m_model->rowCount(parent);
    for (int row = 0; row < rows; ++row) {
        if ((tree && tree->isRowHidden(row, parent)) || (list && list->isRowHidden(row))) {
            continue;
        }
        const QModelIndex index = m_model->index(row, 0, parent);
        const KFileItem item = index.data(KDirModel::FileItemRole).value<KFileItem>();
        m_items.append(item.isNull() ? MinimapBands::ItemKind::Unknown : (item.isDir() ? MinimapBands::ItemKind::Folder : MinimapBands::ItemKind::File));
        // Expanded folders in a tree view show their children inline.
        if (tree && tree->isExpanded(index)) {
            collectItems(index);
        }
    }
}

void FileListMinimap::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    const QColor background = palette().color(QPalette::AlternateBase);
    painter.fillRect(rect(), background);

    ensureItems();
    const QList<MinimapBands::Band> bands = MinimapBands::bands(m_items, height());
    int runStart = 0;
    QColor runColor;
    for (int y = 0; y <= bands.size(); ++y) {
        const QColor color = y < bands.size() && bands.at(y).total() > 0 ? blend(bands.at(y), palette()) : QColor();
        if (y < bands.size() && color == runColor) {
            continue;
        }
        if (runColor.isValid()) {
            painter.fillRect(QRect(2, runStart, width() - 4, y - runStart), runColor);
        }
        runStart = y;
        runColor = color;
    }

    if (!m_scrollBar) {
        return;
    }
    const int range = m_scrollBar->maximum();
    const int page = m_scrollBar->pageStep();
    const qint64 extent = qint64(range) + page;
    const int thumbHeight = extent > 0 ? qBound(MinimumViewportHeight, int(qint64(height()) * page / extent), height()) : height();
    const int travel = height() - thumbHeight;
    const int top = range > 0 ? int(qint64(travel) * m_scrollBar->value() / range) : 0;
    QColor viewport = palette().color(QPalette::Highlight);
    viewport.setAlpha(70);
    painter.fillRect(QRect(0, top, width(), thumbHeight), viewport);
    painter.setPen(palette().color(QPalette::Highlight));
    painter.drawRect(QRect(0, top, width() - 1, thumbHeight - 1));
}

void FileListMinimap::scrollToPosition(int y)
{
    if (!m_scrollBar || m_scrollBar->maximum() <= 0 || height() <= 0) {
        return;
    }
    const int page = m_scrollBar->pageStep();
    const qint64 extent = qint64(m_scrollBar->maximum()) + page;
    const int thumbHeight = extent > 0 ? qBound(MinimumViewportHeight, int(qint64(height()) * page / extent), height()) : height();
    const int travel = height() - thumbHeight;
    if (travel <= 0) {
        return;
    }
    const int top = qBound(0, y - thumbHeight / 2, travel);
    m_scrollBar->setValue(int(qint64(top) * m_scrollBar->maximum() / travel));
}

void FileListMinimap::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        scrollToPosition(event->position().toPoint().y());
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void FileListMinimap::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons().testFlag(Qt::LeftButton)) {
        scrollToPosition(event->position().toPoint().y());
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void FileListMinimap::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void FileListMinimap::wheelEvent(QWheelEvent *event)
{
    if (!m_scrollBar) {
        event->ignore();
        return;
    }
    // A vertical wheel turn also steps through horizontally scrolling columns.
    const QPoint pixels = event->pixelDelta();
    const QPoint angle = event->angleDelta();
    const int pixelDelta = pixels.y() != 0 ? pixels.y() : pixels.x();
    const int angleDelta = angle.y() != 0 ? angle.y() : angle.x();
    const int delta = pixelDelta != 0 ? pixelDelta : angleDelta * m_scrollBar->singleStep() / 120;
    m_scrollBar->setValue(m_scrollBar->value() - delta);
    event->accept();
}
