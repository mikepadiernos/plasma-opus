// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QList>

namespace MinimapBands
{
enum class ItemKind : quint8 {
    Unknown,
    File,
    Folder,
};

// The items that one pixel row of the file-list minimap stands for.
struct Band {
    int folders = 0;
    int files = 0;
    int unknown = 0;

    int total() const
    {
        return folders + files + unknown;
    }
};

// Maps every item, in display order, onto height pixel rows. When there are
// fewer items than rows, each item covers a run of consecutive rows; otherwise
// each row counts all the items that fall within it, so no item is skipped.
QList<Band> bands(const QList<ItemKind> &items, int height);
}
