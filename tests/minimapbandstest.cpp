// SPDX-License-Identifier: GPL-3.0-or-later

#include "minimapbands.h"

#include <QStandardPaths>
#include <QTest>

using MinimapBands::Band;
using MinimapBands::ItemKind;

class MinimapBandsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void emptyListOrHeightGivesEmptyBands()
    {
        QVERIFY(MinimapBands::bands({ItemKind::File}, 0).isEmpty());
        const QList<Band> bands = MinimapBands::bands({}, 4);
        QCOMPARE(bands.size(), 4);
        for (const Band &band : bands) {
            QCOMPARE(band.total(), 0);
        }
    }

    void fewItemsStretchOverConsecutiveRows()
    {
        const QList<Band> bands = MinimapBands::bands({ItemKind::Folder, ItemKind::File}, 6);
        QCOMPARE(bands.size(), 6);
        for (int row = 0; row < 3; ++row) {
            QCOMPARE(bands.at(row).folders, 1);
            QCOMPARE(bands.at(row).files, 0);
        }
        for (int row = 3; row < 6; ++row) {
            QCOMPARE(bands.at(row).folders, 0);
            QCOMPARE(bands.at(row).files, 1);
        }
    }

    void manyItemsAreAllCountedWithoutSampling()
    {
        // Well beyond the 512 rows the minimap used to sample.
        QList<ItemKind> items;
        for (int i = 0; i < 5000; ++i) {
            items.append(i == 4999 ? ItemKind::Folder : (i % 7 == 0 ? ItemKind::Unknown : ItemKind::File));
        }
        const QList<Band> bands = MinimapBands::bands(items, 300);
        QCOMPARE(bands.size(), 300);
        int folders = 0;
        int files = 0;
        int unknown = 0;
        for (const Band &band : bands) {
            QVERIFY(band.total() > 0);
            folders += band.folders;
            files += band.files;
            unknown += band.unknown;
        }
        QCOMPARE(folders + files + unknown, 5000);
        QCOMPARE(folders, 1);
        QCOMPARE(unknown, 715);
        QCOMPARE(bands.last().folders, 1);
    }

    void exactFitGivesOneItemPerRow()
    {
        const QList<Band> bands = MinimapBands::bands({ItemKind::File, ItemKind::Unknown, ItemKind::Folder}, 3);
        QCOMPARE(bands.at(0).files, 1);
        QCOMPARE(bands.at(1).unknown, 1);
        QCOMPARE(bands.at(2).folders, 1);
    }
};

QTEST_GUILESS_MAIN(MinimapBandsTest)

#include "minimapbandstest.moc"
