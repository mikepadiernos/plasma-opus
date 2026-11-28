// SPDX-License-Identifier: GPL-3.0-or-later

#include "minimapbands.h"

namespace MinimapBands
{
namespace
{
void count(Band &band, ItemKind kind)
{
    switch (kind) {
    case ItemKind::Folder:
        ++band.folders;
        break;
    case ItemKind::File:
        ++band.files;
        break;
    case ItemKind::Unknown:
        ++band.unknown;
        break;
    }
}
}

QList<Band> bands(const QList<ItemKind> &items, int height)
{
    if (height <= 0) {
        return {};
    }
    QList<Band> result(height);
    const qint64 itemCount = items.size();
    if (itemCount == 0) {
        return result;
    }
    if (itemCount < height) {
        for (qint64 item = 0; item < itemCount; ++item) {
            const int first = int(item * height / itemCount);
            const int last = int((item + 1) * height / itemCount);
            for (int row = first; row < last; ++row) {
                count(result[row], items.at(item));
            }
        }
        return result;
    }
    for (qint64 item = 0; item < itemCount; ++item) {
        count(result[int(item * height / itemCount)], items.at(item));
    }
    return result;
}
}
