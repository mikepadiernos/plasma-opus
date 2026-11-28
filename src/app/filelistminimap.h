// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "minimapbands.h"

#include <QList>
#include <QMetaObject>
#include <QWidget>

class QAbstractItemModel;
class QAbstractItemView;
class QModelIndex;
class QMouseEvent;
class QPaintEvent;
class QScrollBar;
class QWheelEvent;

class FileListMinimap : public QWidget
{
public:
    explicit FileListMinimap(QWidget *parent = nullptr);

    void setView(QAbstractItemView *view);
    void setMinimapWidth(int width);

    // The number of items shown, including the children of expanded folders.
    int itemCount() const;
    // The scroll direction the minimap follows: horizontal for views that
    // wrap items into columns, such as compact view.
    Qt::Orientation trackedOrientation() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void invalidateItems();
    void ensureItems() const;
    void collectItems(const QModelIndex &parent) const;
    void scrollToPosition(int y);

    QAbstractItemView *m_view = nullptr;
    QAbstractItemModel *m_model = nullptr;
    QScrollBar *m_scrollBar = nullptr;
    Qt::Orientation m_orientation = Qt::Vertical;
    QList<QMetaObject::Connection> m_viewConnections;
    mutable QList<MinimapBands::ItemKind> m_items;
    mutable bool m_itemsValid = false;
};
