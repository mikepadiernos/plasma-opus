// SPDX-License-Identifier: GPL-3.0-or-later

#include "folderview.h"
#include "effectiveappearance.h"
#include "embeddedmetadata.h"

#include "appearancestyle.h"
#include "carddelegate.h"
#include "density.h"
#include "detailsview.h"
#include "filecolorrules.h"
#include "filelistminimap.h"
#include "fileoperations.h"
#include "flatview.h"
#include "foldercolors.h"
#include "foldergates.h"
#include "foldermetadata.h"
#include "hiddenfolders.h"
#include "ignorelist.h"
#include "labelcolor.h"
#include "localizeddates.h"
#include "localizednumbers.h"
#include "mailhandoff.h"
#include "searchquery.h"
#include "settings.h"
#include "symbolicicon.h"
#include "tagsearch.h"
#include "viewsettings.h"

#include <KCategorizedSortFilterProxyModel>
#include <KCategorizedView>
#include <KCategoryDrawer>
#include <KConfigGroup>
#include <KDirLister>
#include <KDirModel>
#include <KDirSortFilterProxyModel>
#include <KFileItemDelegate>
#include <KFileItemListProperties>
#include <KFileMetaData/UserMetaData>
#include <KFilePreviewGenerator>
#include <KIO/DirectorySizeJob>
#include <KIO/DropJob>
#include <KIO/Global>
#include <KIO/Job>
#include <KIconUtils>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QActionGroup>
#include <QApplication>
#include <QDate>
#include <QDateTime>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHash>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QJsonArray>
#include <QJsonObject>
#include <QKeyEvent>
#include <QListView>
#include <QLocale>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QStackedWidget>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QTimer>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>

#include <algorithm>
#include <optional>
#include <utility>

namespace
{
const QList<QPair<int, QString>> OptionalColumns{
    {KDirModel::Type, QStringLiteral("type")},
    {KDirModel::Size, QStringLiteral("size")},
    {KDirModel::ModifiedTime, QStringLiteral("date")},
    {KDirModel::Permissions, QStringLiteral("mode")},
    {KDirModel::Owner, QStringLiteral("owner")},
    {KDirModel::Group, QStringLiteral("group")},
};

int columnForSortKey(const QString &key)
{
    if (key == QLatin1String("size")) {
        return KDirModel::Size;
    }
    if (key == QLatin1String("date")) {
        return KDirModel::ModifiedTime;
    }
    if (key == QLatin1String("type")) {
        return KDirModel::Type;
    }
    if (key == QLatin1String("mode")) {
        return KDirModel::Permissions;
    }
    if (key == QLatin1String("owner")) {
        return KDirModel::Owner;
    }
    if (key == QLatin1String("group")) {
        return KDirModel::Group;
    }
    if (key == QLatin1String("rating")) {
        return KDirModel::Group;
    }
    return KDirModel::Name;
}

QString sortKeyForColumn(int column, bool ratingColumn)
{
    if (column == KDirModel::Group && ratingColumn) {
        return QStringLiteral("rating");
    }
    for (const auto &[candidate, key] : OptionalColumns) {
        if (candidate == column) {
            return key;
        }
    }
    return QStringLiteral("name");
}

KFileItemDelegate::InformationList informationForKeys(const QStringList &keys)
{
    const QHash<QString, KFileItemDelegate::Information> options{
        {QStringLiteral("size"), KFileItemDelegate::Size},
        {QStringLiteral("permissions"), KFileItemDelegate::Permissions},
        {QStringLiteral("octalPermissions"), KFileItemDelegate::OctalPermissions},
        {QStringLiteral("owner"), KFileItemDelegate::Owner},
        {QStringLiteral("ownerAndGroup"), KFileItemDelegate::OwnerAndGroup},
        {QStringLiteral("creationTime"), KFileItemDelegate::CreationTime},
        {QStringLiteral("modificationTime"), KFileItemDelegate::ModificationTime},
        {QStringLiteral("accessTime"), KFileItemDelegate::AccessTime},
        {QStringLiteral("mimeType"), KFileItemDelegate::MimeType},
        {QStringLiteral("friendlyMimeType"), KFileItemDelegate::FriendlyMimeType},
        {QStringLiteral("linkDest"), KFileItemDelegate::LinkDest},
        {QStringLiteral("localPathOrUrl"), KFileItemDelegate::LocalPathOrUrl},
        {QStringLiteral("comment"), KFileItemDelegate::Comment},
    };
    KFileItemDelegate::InformationList information;
    for (const QString &key : keys) {
        if (options.contains(key)) {
            information.append(options.value(key));
        }
    }
    return information;
}
}

/// Sorts like KDirSortFilterProxyModel and hides ignored items, items a NameFilter rejects and folders hidden by their metadata.
class NameFilterProxyModel : public KDirSortFilterProxyModel
{
public:
    explicit NameFilterProxyModel(QObject *parent)
        : KDirSortFilterProxyModel(parent)
        , m_ignoreList(IgnoreList::fromConfig(KSharedConfig::openConfig()->group(QStringLiteral("Files"))))
    {
    }

    NameFilter nameFilter() const
    {
        return m_filter;
    }

    void setNameFilter(const NameFilter &filter)
    {
        beginFilterChange();
        m_filter = filter;
        endFilterChange(QSortFilterProxyModel::Direction::Rows);
    }

    /// Hides every row while the shown folder is a locked password-gated folder or inside one.
    void setGateBlocked(bool blocked)
    {
        beginFilterChange();
        m_gateBlocked = blocked;
        endFilterChange(QSortFilterProxyModel::Direction::Rows);
    }

    /// Filters rows again after password-gated folders are locked or unlocked.
    void refreshGates()
    {
        beginFilterChange();
        endFilterChange(QSortFilterProxyModel::Direction::Rows);
        const int rows = rowCount();
        if (rows > 0) {
            Q_EMIT dataChanged(index(0, 0), index(rows - 1, 0), {Qt::DecorationRole});
        }
    }

    /// Filters rows again after hidden files are shown or hidden, or folder metadata hides or shows folders.
    void refreshHidden()
    {
        beginFilterChange();
        endFilterChange(QSortFilterProxyModel::Direction::Rows);
    }

    /// Hides folders and shows relative paths in the Type column; an empty URL ends flat mode.
    void setFlatBase(const QUrl &base)
    {
        beginFilterChange();
        m_flatBase = base;
        endFilterChange(QSortFilterProxyModel::Direction::Rows);
        Q_EMIT headerDataChanged(Qt::Horizontal, KDirModel::Type, KDirModel::Type);
    }

    void setMetadata(const QJsonObject &metadata, const QString &directory)
    {
        beginFilterChange();
        m_metadata = metadata;
        m_metadataDirectory = directory.isEmpty() ? QString() : QFileInfo(directory).absoluteFilePath();
        m_metadataCache.clear();
        m_tagsCache.clear();
        endFilterChange(QSortFilterProxyModel::Direction::Rows);
        sort(sortColumn(), sortOrder());
    }

    void clearFolderSizes()
    {
        const QList<QUrl> urls = m_folderSizes.keys();
        m_folderSizes.clear();
        for (const QUrl &url : urls) {
            const QModelIndex source = static_cast<KDirModel *>(sourceModel())->indexForUrl(url);
            const QModelIndex index = mapFromSource(source).siblingAtColumn(KDirModel::Size);
            if (index.isValid()) {
                Q_EMIT dataChanged(index, index, {Qt::DisplayRole});
            }
        }
        if (sortColumn() == KDirModel::Size && !urls.isEmpty()) {
            invalidate();
        }
    }

    QList<QUrl> calculatedFolderUrls() const
    {
        return m_folderSizes.keys();
    }

    void setFolderSize(const QUrl &url, quint64 size)
    {
        m_folderSizes.insert(url, size);
        const QModelIndex source = static_cast<KDirModel *>(sourceModel())->indexForUrl(url);
        const QModelIndex index = mapFromSource(source).siblingAtColumn(KDirModel::Size);
        if (index.isValid()) {
            Q_EMIT dataChanged(index, index, {Qt::DisplayRole});
            if (sortColumn() == KDirModel::Size) {
                invalidate();
            }
        }
    }

    void setSettings(const ViewSettings &settings, const FileColorRules &fileColors, const IgnoreList &ignoreList)
    {
        const bool calendarChanged = m_groupBy == QLatin1String("date");
        beginFilterChange();
        m_settings = settings;
        m_fileColors = fileColors;
        m_ignoreList = ignoreList;
        endFilterChange(QSortFilterProxyModel::Direction::Rows);
        if (calendarChanged || m_pinnedItemsFirst != Settings::pinnedItemsFirst()) {
            m_pinnedItemsFirst = Settings::pinnedItemsFirst();
            const int column = sortColumn();
            const Qt::SortOrder order = sortOrder();
            sort(-1);
            sort(column, order);
        }
        if (rowCount() > 0) {
            Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1), {Qt::DisplayRole, Qt::ForegroundRole});
        }
    }

    QJsonObject metadata() const
    {
        return m_metadata;
    }

    QString groupBy() const
    {
        return m_groupBy;
    }

    bool ratingColumn() const
    {
        return m_ratingColumn;
    }

    void setRatingColumn(bool enabled)
    {
        if (enabled == m_ratingColumn) {
            return;
        }
        m_ratingColumn = enabled;
        Q_EMIT headerDataChanged(Qt::Horizontal, KDirModel::Group, KDirModel::Group);
        if (rowCount() > 0) {
            Q_EMIT dataChanged(index(0, KDirModel::Group), index(rowCount() - 1, KDirModel::Group));
        }
        sort(sortColumn(), sortOrder());
    }

    void setGroupBy(const QString &group)
    {
        if (group == m_groupBy) {
            return;
        }
        m_groupBy = group;
        setCategorizedModel(m_groupBy != QLatin1String("none"));
        sort(sortColumn(), sortOrder());
    }

    QString groupLabel(const QModelIndex &index) const
    {
        if (m_groupBy == QLatin1String("none")) {
            return {};
        }
        const KFileItem item = itemAt(index);
        if (m_groupBy == QLatin1String("name")) {
            const QString first = item.name().left(1).toUpper();
            return first.isEmpty() || !first.at(0).isLetter() ? QStringLiteral("#") : first;
        }
        if (m_groupBy == QLatin1String("type")) {
            return item.isDir() ? i18nc("@item:inlistbox file group", "Folders") : item.mimeComment();
        }
        if (m_groupBy == QLatin1String("ext")) {
            if (item.isDir()) {
                return i18nc("@item:inlistbox file group", "Folders");
            }
            const QString suffix = QFileInfo(item.name()).suffix().toUpper();
            return suffix.isEmpty() ? i18nc("@item:inlistbox file group", "No Extension") : suffix;
        }
        if (m_groupBy == QLatin1String("size")) {
            if (item.isDir()) {
                return i18nc("@item:inlistbox file group", "Folders");
            }
            const qint64 size = item.size();
            if (size < 1024 * 1024) {
                return i18nc("@item:inlistbox file group", "Under 1 MiB");
            }
            if (size < 10LL * 1024 * 1024) {
                return i18nc("@item:inlistbox file group", "1-10 MiB");
            }
            if (size < 100LL * 1024 * 1024) {
                return i18nc("@item:inlistbox file group", "10-100 MiB");
            }
            if (size < 1024LL * 1024 * 1024) {
                return i18nc("@item:inlistbox file group", "100 MiB-1 GiB");
            }
            return i18nc("@item:inlistbox file group", "Over 1 GiB");
        }
        if (m_groupBy == QLatin1String("date")) {
            const QDate modified = item.time(KFileItem::ModificationTime).date();
            const auto group =
                LocalizedDates::group(modified, QDate::currentDate(), LocalizedDates::calendar(QLocale(), m_settings.dateFormat == QLatin1String("iso")));
            switch (group) {
            case LocalizedDates::Group::Today:
                return i18nc("@item:inlistbox file group", "Today");
            case LocalizedDates::Group::Yesterday:
                return i18nc("@item:inlistbox file group", "Yesterday");
            case LocalizedDates::Group::LastSevenDays:
                return i18nc("@item:inlistbox file group", "Last 7 Days");
            case LocalizedDates::Group::ThisMonth:
                return i18nc("@item:inlistbox file group", "This Month");
            case LocalizedDates::Group::ThisYear:
                return i18nc("@item:inlistbox file group", "This Year");
            case LocalizedDates::Group::Older:
                return i18nc("@item:inlistbox file group", "Older");
            }
        }
        return {};
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (role == Qt::DisplayRole && index.column() == KDirModel::Size) {
            const KFileItem item = itemAt(index);
            const auto found = m_folderSizes.constFind(item.url());
            if (item.isDir() && found != m_folderSizes.cend()) {
                return m_settings.sizeFormat == QLatin1String("bytes")
                    ? QString(LocalizedNumbers::shape(QLocale().toString(found.value())) + i18nc("@unit byte in file size", " B"))
                    : LocalizedNumbers::shape(KIO::convertSize(found.value()));
            }
        }
        if (role == Qt::DisplayRole && index.column() == KDirModel::Size && m_settings.sizeFormat == QLatin1String("bytes")) {
            const KFileItem item = itemAt(index);
            if (!item.isDir()) {
                return QString(LocalizedNumbers::shape(QLocale().toString(item.size())) + i18nc("@unit byte in file size", " B"));
            }
        }
        if (role == Qt::DisplayRole && index.column() == KDirModel::Size && Settings::numeralStyle() != Settings::EnumNumeralStyle::LocaleDigits) {
            return LocalizedNumbers::shape(KDirSortFilterProxyModel::data(index, role).toString());
        }
        if (role == Qt::DisplayRole && index.column() == KDirModel::ModifiedTime) {
            const QDateTime modified = itemAt(index).time(KFileItem::ModificationTime);
            if (modified.isValid()) {
                return LocalizedDates::listingDateTime(modified, m_settings.dateFormat == QLatin1String("iso"), m_settings.showSeconds);
            }
        }
        if (role == CardRoles::Rating || role == CardRoles::Label) {
            const KFileItem item = itemAt(index);
            const QJsonObject metadata = metadataForItem(item);
            if (role == CardRoles::Rating) {
                const int rating = metadata.value(QStringLiteral("ratings")).toObject().value(item.name()).toInt();
                return Settings::showFileRatings() && rating > 0 ? QVariant(rating) : QVariant();
            }
            const QString label = metadata.value(QStringLiteral("labels")).toObject().value(item.name()).toString();
            return Settings::showFileLabels() && !label.isEmpty() ? QVariant(label) : QVariant();
        }
        if (role == CardRoles::Tags) {
            const QString path = itemAt(index).localPath();
            if (path.isEmpty()) {
                return QStringList();
            }
            auto found = m_tagsCache.constFind(path);
            if (found == m_tagsCache.cend()) {
                found = m_tagsCache.insert(path, KFileMetaData::UserMetaData(path).tags());
            }
            return found.value();
        }
        if (role == KCategorizedSortFilterProxyModel::CategorySortRole) {
            return m_groupBy == QLatin1String("none") ? QString() : groupSortKey(index);
        }
        if (role == KCategorizedSortFilterProxyModel::CategoryDisplayRole) {
            return groupLabel(index);
        }
        if (m_ratingColumn && index.column() == KDirModel::Group && role == Qt::DisplayRole) {
            const KFileItem item = itemAt(index);
            const int rating = metadataForItem(item).value(QStringLiteral("ratings")).toObject().value(item.name()).toInt();
            return Settings::showFileRatings() && rating > 0 ? QVariant(rating) : QVariant();
        }
        if (isRelativePath(index.column()) && role == Qt::DisplayRole) {
            return FlatView::relativeFolder(m_flatBase, itemAt(index).url());
        }
        if (role == Qt::ForegroundRole || role == Qt::ToolTipRole) {
            const KFileItem item = itemAt(index);
            const QJsonObject metadata = metadataForItem(item);
            const QString name = item.name();
            const QString color = metadata.value(QStringLiteral("colors")).toObject().value(name).toString();
            const QString label = metadata.value(QStringLiteral("labels")).toObject().value(name).toString();
            if (role == Qt::ForegroundRole && index.column() == KDirModel::Name) {
                if (Settings::showFileColors() && !color.isEmpty()) {
                    return QColor(color);
                }
                const QColor extensionColor = m_fileColors.colorForFileName(name);
                if (extensionColor.isValid()) {
                    return extensionColor;
                }
                const QColor labelColor = LabelColors::rgb(LabelColors::fromKey(label));
                if (Settings::showFileLabels() && labelColor.isValid()) {
                    return labelColor;
                }
                if (const QColor folderColor = FolderColors::nameColor(item, QApplication::palette().color(QPalette::Base)); folderColor.isValid()) {
                    return folderColor;
                }
            }
            if (role == Qt::ToolTipRole) {
                QStringList details;
                const QString defaultTip = KDirSortFilterProxyModel::data(index, role).toString();
                if (!defaultTip.isEmpty()) {
                    details.append(defaultTip);
                }
                const QString note = metadata.value(QStringLiteral("notes")).toObject().value(name).toString();
                if (!note.isEmpty()) {
                    details.append(note);
                }
                if (Settings::showFilePins() && metadata.value(QStringLiteral("pins")).toArray().contains(name)) {
                    details.append(i18nc("@info:tooltip pinned file", "Pinned"));
                }
                if (!details.isEmpty()) {
                    return details.join(u'\n');
                }
            }
        }
        if (Settings::highlightFilterMatches() && role == Qt::BackgroundRole && index.column() == KDirModel::Name && m_filter.appliesTo(itemAt(index).isDir())
            && m_filter.text().trimmed().startsWith(u'#') == false && m_filter.text().trimmed().startsWith(u'*') == false) {
            QColor highlight = QApplication::palette().color(QPalette::Highlight);
            highlight.setAlpha(45);
            return highlight;
        }
        if (Settings::highlightFilterMatches() && role == Qt::FontRole && index.column() == KDirModel::Name && m_filter.appliesTo(itemAt(index).isDir())
            && !m_filter.text().trimmed().isEmpty() && !m_filter.text().contains(u'*') && !m_filter.text().contains(u'?') && !m_filter.text().contains(u'[')
            && itemAt(index).name().contains(m_filter.text().trimmed(), Qt::CaseInsensitive)) {
            QFont font;
            font.setBold(true);
            return font;
        }
        const QVariant value = KDirSortFilterProxyModel::data(index, role);
        if (role == Qt::DecorationRole && value.typeId() == QMetaType::QIcon) {
            const KFileItem item = itemAt(index);
            // A locked password-gated folder shows a lock rather than a preview of its contents.
            if (FolderGates::instance()->isLockedGateItem(item)) {
                return SymbolicIcon::wrap(QIcon::fromTheme(QStringLiteral("folder-locked"), value.value<QIcon>()), QStringLiteral("folder-locked"), {});
            }
            const QIcon icon = FolderColors::decorate(SymbolicIcon::wrap(value.value<QIcon>(), item.iconName(), item.overlays()), item);
            if (Settings::showFilePins() && metadataForItem(item).value(QStringLiteral("pins")).toArray().contains(item.name())) {
                return KIconUtils::addOverlays(icon, {QStringLiteral("emblem-favorite")});
            }
            return icon;
        }
        return value;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (orientation == Qt::Horizontal && section == KDirModel::Group && m_ratingColumn && role == Qt::DisplayRole) {
            return i18nc("@title:column", "Rating");
        }
        if (orientation == Qt::Horizontal && isRelativePath(section) && role == Qt::DisplayRole) {
            return i18nc("@title:column path below the flat folder", "Relative Path");
        }
        return KDirSortFilterProxyModel::headerData(section, orientation, role);
    }

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override
    {
        const KFileItem leftItem = itemAt(left);
        const KFileItem rightItem = itemAt(right);
        if (m_groupBy == QLatin1String("none") && sortColumn() == KDirModel::Size && (leftItem.isDir() || rightItem.isDir())
            && (leftItem.isDir() == rightItem.isDir() || !sortFoldersFirst())) {
            return subSortLessThan(left, right);
        }
        return KDirSortFilterProxyModel::lessThan(left, right);
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        if (!KDirSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent)) {
            return false;
        }
        if (m_gateBlocked) {
            return false;
        }
        const auto *model = static_cast<const KDirModel *>(sourceModel());
        const KFileItem item = model->itemForIndex(model->index(sourceRow, 0, sourceParent));
        if (m_flatBase.isValid() && item.isDir()) {
            return false;
        }
        if (FolderGates::instance()->hidesItem(item)) {
            return false;
        }
        if (HiddenFolders::instance()->hidesItem(item, model->dirLister()->showHiddenFiles())) {
            return false;
        }
        const QString name = item.name();
        const QJsonObject metadata = metadataForItem(item);
        const QString label = metadata.value(QStringLiteral("labels")).toObject().value(name).toString();
        const int rating = metadata.value(QStringLiteral("ratings")).toObject().value(name).toInt();
        return !m_ignoreList.ignores(name) && m_filter.matches(name, item.isDir(), label, rating);
    }

    bool subSortLessThan(const QModelIndex &left, const QModelIndex &right) const override
    {
        QStringList pins;
        const QJsonArray pinArray = metadataForItem(itemAt(left)).value(QStringLiteral("pins")).toArray();
        QStringList rightPins;
        const QJsonArray rightPinArray = metadataForItem(itemAt(right)).value(QStringLiteral("pins")).toArray();
        for (const QJsonValue &pin : pinArray) {
            pins.append(pin.toString());
        }
        for (const QJsonValue &pin : rightPinArray) {
            rightPins.append(pin.toString());
        }
        const bool leftPinned = pins.contains(itemAt(left).name());
        const bool rightPinned = rightPins.contains(itemAt(right).name());
        if (Settings::pinnedItemsFirst() && leftPinned != rightPinned) {
            return sortOrder() == Qt::DescendingOrder ? !leftPinned : leftPinned;
        }
        if (m_groupBy != QLatin1String("none")) {
            const int byGroup = QString::localeAwareCompare(groupSortKey(left), groupSortKey(right));
            if (byGroup != 0) {
                return sortOrder() == Qt::DescendingOrder ? byGroup > 0 : byGroup < 0;
            }
        }
        if (m_ratingColumn && sortColumn() == KDirModel::Group) {
            const int leftRating = metadataForItem(itemAt(left)).value(QStringLiteral("ratings")).toObject().value(itemAt(left).name()).toInt();
            const int rightRating = metadataForItem(itemAt(right)).value(QStringLiteral("ratings")).toObject().value(itemAt(right).name()).toInt();
            if (leftRating != rightRating) {
                return leftRating < rightRating;
            }
        }
        if (sortColumn() == KDirModel::Size) {
            const KFileItem leftItem = itemAt(left);
            const KFileItem rightItem = itemAt(right);
            if ((leftItem.isDir() || rightItem.isDir()) && (leftItem.isDir() == rightItem.isDir() || !sortFoldersFirst())) {
                const auto leftSize = m_folderSizes.constFind(leftItem.url());
                const auto rightSize = m_folderSizes.constFind(rightItem.url());
                const bool leftKnown = !leftItem.isDir() || leftSize != m_folderSizes.cend();
                const bool rightKnown = !rightItem.isDir() || rightSize != m_folderSizes.cend();
                if (leftKnown != rightKnown) {
                    return sortOrder() == Qt::AscendingOrder ? leftKnown : rightKnown;
                }
                if (leftKnown) {
                    const quint64 leftValue = leftItem.isDir() ? leftSize.value() : leftItem.size();
                    const quint64 rightValue = rightItem.isDir() ? rightSize.value() : rightItem.size();
                    if (leftValue != rightValue) {
                        return leftValue < rightValue;
                    }
                }
            }
        }
        if (isRelativePath(left.column())) {
            const int byPath = QString::localeAwareCompare(FlatView::relativeFolder(m_flatBase, itemAt(left).url()),
                                                           FlatView::relativeFolder(m_flatBase, itemAt(right).url()));
            if (byPath != 0) {
                return byPath < 0;
            }
        }
        return KDirSortFilterProxyModel::subSortLessThan(left, right);
    }

private:
    bool isRelativePath(int column) const
    {
        return m_flatBase.isValid() && column == KDirModel::Type;
    }

    /// The item at a proxy or source index of this model's KDirModel.
    KFileItem itemAt(const QModelIndex &index) const
    {
        const QModelIndex source = index.model() == this ? mapToSource(index) : index;
        return static_cast<const KDirModel *>(sourceModel())->itemForIndex(source);
    }

    QJsonObject metadataForItem(const KFileItem &item) const
    {
        const QString localPath = item.localPath();
        if (localPath.isEmpty()) {
            return m_metadata;
        }
        const QString directory = QFileInfo(localPath).absolutePath();
        if (directory == m_metadataDirectory) {
            return m_metadata;
        }
        const auto found = m_metadataCache.constFind(directory);
        if (found != m_metadataCache.cend()) {
            return found.value();
        }
        const QJsonObject metadata = FolderMetadata::read(directory);
        m_metadataCache.insert(directory, metadata);
        return metadata;
    }

    QString groupSortKey(const QModelIndex &index) const
    {
        const KFileItem item = itemAt(index);
        if ((m_groupBy == QLatin1String("type") || m_groupBy == QLatin1String("ext")) && item.isDir()) {
            return QStringLiteral("0");
        }
        if (m_groupBy == QLatin1String("size")) {
            if (item.isDir()) {
                return QStringLiteral("0");
            }
            const qint64 size = item.size();
            return size < 1024 * 1024         ? QStringLiteral("1")
                : size < 10LL * 1024 * 1024   ? QStringLiteral("2")
                : size < 100LL * 1024 * 1024  ? QStringLiteral("3")
                : size < 1024LL * 1024 * 1024 ? QStringLiteral("4")
                                              : QStringLiteral("5");
        }
        if (m_groupBy == QLatin1String("date")) {
            const QDate modified = item.time(KFileItem::ModificationTime).date();
            const auto group =
                LocalizedDates::group(modified, QDate::currentDate(), LocalizedDates::calendar(QLocale(), m_settings.dateFormat == QLatin1String("iso")));
            return QString::number(static_cast<int>(group));
        }
        return QStringLiteral("1") + groupLabel(index).toCaseFolded();
    }

    QUrl m_flatBase;
    bool m_gateBlocked = false;
    NameFilter m_filter;
    IgnoreList m_ignoreList;
    FileColorRules m_fileColors;
    ViewSettings m_settings;
    QJsonObject m_metadata;
    QString m_metadataDirectory;
    mutable QHash<QString, QJsonObject> m_metadataCache;
    mutable QHash<QString, QStringList> m_tagsCache;
    QHash<QUrl, quint64> m_folderSizes;
    QString m_groupBy = QStringLiteral("none");
    bool m_ratingColumn = false;
    bool m_pinnedItemsFirst = Settings::pinnedItemsFirst();
};

namespace
{
QRect selectionMarkerRect(const QRect &itemRect, Qt::LayoutDirection direction)
{
    const int size = 16;
    return QRect(direction == Qt::RightToLeft ? itemRect.right() - size + 1 : itemRect.left(), itemRect.bottom() - size + 1, size, size);
}

void paintSelectionMarker(QPainter *painter, const QStyleOptionViewItem &option)
{
    QStyleOptionButton check;
    check.rect = selectionMarkerRect(option.rect, option.direction);
    check.state = QStyle::State_Enabled | ((option.state & QStyle::State_Selected) ? QStyle::State_On : QStyle::State_Off);
    QApplication::style()->drawControl(QStyle::CE_CheckBox, &check, painter);
}
}

class GroupHeaderDelegate : public KFileItemDelegate
{
public:
    explicit GroupHeaderDelegate(NameFilterProxyModel *model, FolderView *folder, QObject *parent)
        : KFileItemDelegate(parent)
        , m_model(model)
        , m_folder(folder)
    {
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QSize size = KFileItemDelegate::sizeHint(option, index);
        size.rheight() = qMax(1, size.height() + 2 * (m_itemSpace - 2));
        if (firstInGroup(index)) {
            size.rheight() += HeaderHeight;
        }
        return size;
    }

    void setItemSpace(int space)
    {
        m_itemSpace = space;
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem prepared(option);
        AppearanceStyle::prepareItem(&prepared);
        if (!firstInGroup(index)) {
            KFileItemDelegate::paint(painter, prepared, index);
            if (index.column() == 0 && (m_folder->selectionMarkersVisible() || m_folder->selectionMode())) {
                paintSelectionMarker(painter, option);
            }
            return;
        }
        const QRect headerRect(option.rect.left(), option.rect.top(), option.rect.width(), HeaderHeight);
        if (index.column() == 0) {
            painter->fillRect(headerRect, option.palette.alternateBase());
            QFont font = option.font;
            font.setBold(true);
            painter->setFont(font);
            painter->setPen(option.palette.color(QPalette::Text));
            const QRect labelRect = QStyle::visualRect(option.direction, headerRect, headerRect.adjusted(8, 0, -8, 0));
            painter->drawText(labelRect, QStyle::visualAlignment(option.direction, Qt::AlignVCenter | Qt::AlignLeft), m_model->groupLabel(index));
        }
        QStyleOptionViewItem itemOption(prepared);
        itemOption.rect.setTop(itemOption.rect.top() + HeaderHeight);
        KFileItemDelegate::paint(painter, itemOption, index);
        if (index.column() == 0 && (m_folder->selectionMarkersVisible() || m_folder->selectionMode())) {
            paintSelectionMarker(painter, itemOption);
        }
    }

private:
    bool firstInGroup(const QModelIndex &index) const
    {
        if (m_model->groupBy() == QLatin1String("none")) {
            return false;
        }
        return index.row() == 0 || m_model->groupLabel(index.sibling(index.row(), 0)) != m_model->groupLabel(index.sibling(index.row() - 1, 0));
    }

    static constexpr int HeaderHeight = 20;
    NameFilterProxyModel *m_model;
    FolderView *m_folder;
    int m_itemSpace = 2;
};

class SelectionMarkerDelegate : public KFileItemDelegate
{
public:
    explicit SelectionMarkerDelegate(FolderView *folder, QObject *parent)
        : KFileItemDelegate(parent)
        , m_folder(folder)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem prepared(option);
        AppearanceStyle::prepareItem(&prepared);
        KFileItemDelegate::paint(painter, prepared, index);
        if (m_folder->selectionMarkersVisible() || m_folder->selectionMode()) {
            paintSelectionMarker(painter, option);
        }
    }

private:
    FolderView *m_folder;
};

FolderView::FolderView(QWidget *parent)
    : QWidget(parent)
    , m_lister(new KDirLister(this))
    , m_model(new KDirModel(this))
    , m_proxy(new NameFilterProxyModel(this))
    , m_sizeRefreshTimer(new QTimer(this))
    , m_stack(new QStackedWidget(this))
    , m_details(new DetailsView(this))
    , m_icons(new KCategorizedView(this))
    , m_compact(new QListView(this))
    , m_cards(new KCategorizedView(this))
{
    m_sizeRefreshTimer->setObjectName(QStringLiteral("folderSizeRefreshTimer"));
    connect(m_sizeRefreshTimer, &QTimer::timeout, this, [this] {
        if (m_flat || !isVisible()) {
            return;
        }
        KFileItemList items;
        for (const QUrl &url : m_proxy->calculatedFolderUrls()) {
            const QModelIndex sourceIndex = m_model->indexForUrl(url);
            const QModelIndex index = m_proxy->mapFromSource(sourceIndex);
            if (!index.isValid()) {
                continue;
            }
            bool shown = true;
            for (QModelIndex parent = index.parent(); parent.isValid(); parent = parent.parent()) {
                if (m_viewMode != ViewMode::Details || !m_details->isExpanded(parent)) {
                    shown = false;
                    break;
                }
            }
            if (!shown) {
                continue;
            }
            const KFileItem item = m_model->itemForIndex(sourceIndex);
            if (!item.isNull()) {
                items.append(item);
            }
        }
        queueFolderSizes(items);
    });
    updateFolderSizeRefreshTimer();
    m_lister->setAutoErrorHandlingEnabled(false);
    m_lister->setMainWindow(window());
    m_model->setDirLister(m_lister);
    m_model->setJobTransfersVisible(true);
    m_proxy->setSourceModel(m_model);
    connect(m_proxy, &QAbstractItemModel::layoutChanged, this, &FolderView::itemsOrderChanged);
    connect(m_proxy, &QAbstractItemModel::rowsInserted, this, &FolderView::itemsOrderChanged);
    connect(m_proxy, &QAbstractItemModel::rowsRemoved, this, &FolderView::itemsOrderChanged);
    m_proxy->setSortFoldersFirst(true);
    m_proxy->setSortCaseSensitivity(Qt::CaseInsensitive);

    m_details->setModel(m_proxy);
    m_details->setRootIsDecorated(true);
    m_details->setItemsExpandable(true);
    m_details->setExpandsOnDoubleClick(false);
    m_details->setUniformRowHeights(false);
    m_details->setSortingEnabled(true);
    m_details->sortByColumn(KDirModel::Name, Qt::AscendingOrder);
    m_details->setAllColumnsShowFocus(true);
    m_details->header()->setSectionsMovable(true);
    m_details->header()->setStretchLastSection(false);
    m_details->header()->setSectionResizeMode(KDirModel::Name, QHeaderView::Stretch);
    m_details->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    for (int column : {KDirModel::Permissions, KDirModel::Owner, KDirModel::Group}) {
        m_details->setColumnHidden(column, true);
    }
    setupView(m_details);

    m_icons->setModel(m_proxy);
    static_cast<KCategorizedView *>(m_icons)->setCategoryDrawer(new KCategoryDrawer(static_cast<KCategorizedView *>(m_icons)));
    m_icons->setSelectionModel(m_details->selectionModel());
    m_icons->setViewMode(QListView::IconMode);
    m_icons->setResizeMode(QListView::Adjust);
    m_icons->setFlow(QListView::LeftToRight);
    m_icons->setWrapping(true);
    m_icons->setMovement(QListView::Static);
    m_icons->setUniformItemSizes(true);
    m_icons->setWordWrap(true);
    m_icons->setIconSize(QSize(64, 64));
    m_icons->setGridSize(QSize(112, 112));
    m_icons->setSpacing(4);
    setupView(m_icons);

    m_compact->setModel(m_proxy);
    m_compact->setSelectionModel(m_details->selectionModel());
    m_compact->setViewMode(QListView::ListMode);
    m_compact->setFlow(QListView::TopToBottom);
    m_compact->setWrapping(true);
    m_compact->setResizeMode(QListView::Adjust);
    m_compact->setMovement(QListView::Static);
    m_compact->setIconSize(QSize(16, 16));
    setupView(m_compact);

    m_cards->setObjectName(QStringLiteral("cardView"));
    m_cards->setModel(m_proxy);
    static_cast<KCategorizedView *>(m_cards)->setCategoryDrawer(new KCategoryDrawer(static_cast<KCategorizedView *>(m_cards)));
    m_cards->setSelectionModel(m_details->selectionModel());
    m_cards->setViewMode(QListView::IconMode);
    m_cards->setResizeMode(QListView::Adjust);
    m_cards->setFlow(QListView::LeftToRight);
    m_cards->setWrapping(true);
    m_cards->setMovement(QListView::Static);
    m_cards->setUniformItemSizes(true);
    m_cards->setSpacing(0);
    m_cards->viewport()->setAttribute(Qt::WA_Hover);
    setupView(m_cards);

    const QList<QAbstractItemView *> views{m_details, m_icons, m_compact, m_cards};
    for (QAbstractItemView *view : views) {
        connect(view, &QAbstractItemView::entered, this, [this](const QModelIndex &index) {
            if (!m_informationHoverEnabled) {
                return;
            }
            const KFileItem item = m_model->itemForIndex(m_proxy->mapToSource(index));
            if (!item.isNull()) {
                Q_EMIT itemHovered(item);
            }
        });
        connect(view, &QAbstractItemView::viewportEntered, this, [this] {
            if (m_informationHoverEnabled) {
                Q_EMIT itemHoverCleared();
            }
        });
    }

    setPreviewsEnabled(true);

    m_stack->addWidget(m_details);
    m_stack->addWidget(m_icons);
    m_stack->addWidget(m_compact);
    m_stack->addWidget(m_cards);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    auto *viewLayout = new QHBoxLayout;
    viewLayout->setContentsMargins(0, 0, 0, 0);
    viewLayout->setSpacing(0);
    viewLayout->addWidget(m_stack, 1);
    m_minimap = new FileListMinimap(this);
    m_minimap->setView(m_details);
    viewLayout->addWidget(m_minimap);
    layout->addLayout(viewLayout, 1);
    m_minimap->setMinimapWidth(Settings::fileListMinimapWidth());
    m_minimap->setVisible(Settings::showFileListMinimap());
    m_selectionBar = new QWidget(this);
    m_selectionBar->setObjectName(QStringLiteral("selectionBar"));
    auto *barLayout = new QHBoxLayout(m_selectionBar);
    barLayout->setContentsMargins(0, 0, 0, 0);
    const auto addButton = [this, barLayout](const QString &name, const QString &text, const QString &icon, auto slot) {
        auto *button = new QToolButton(m_selectionBar);
        button->setObjectName(name);
        button->setText(text);
        button->setIcon(SymbolicIcon::fromTheme(icon));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        connect(button, &QToolButton::clicked, this, slot);
        barLayout->addWidget(button);
        return button;
    };
    auto *copy = addButton(QStringLiteral("selectionCopy"), i18nc("@action", "Copy"), QStringLiteral("edit-copy"), &FolderView::copySelectionRequested);
    auto *cut = addButton(QStringLiteral("selectionCut"), i18nc("@action", "Cut"), QStringLiteral("edit-cut"), &FolderView::cutSelectionRequested);
    auto *trash =
        addButton(QStringLiteral("selectionTrash"), i18nc("@action", "Move to Trash"), QStringLiteral("user-trash"), &FolderView::trashSelectionRequested);
    barLayout->addStretch();
    addButton(QStringLiteral("selectionDone"), i18nc("@action", "Done"), QStringLiteral("dialog-ok"), [this] {
        setSelectionMode(false);
    });
    const auto updateSelectionButtons = [this, copy, cut, trash] {
        const KFileItemList items = selectedItems();
        const KFileItemListProperties properties(items);
        copy->setEnabled(!items.isEmpty());
        cut->setEnabled(!items.isEmpty() && properties.supportsMoving());
        trash->setEnabled(!items.isEmpty() && properties.supportsDeleting() && properties.isLocal());
    };
    connect(this, &FolderView::selectedItemsChanged, this, updateSelectionButtons);
    updateSelectionButtons();
    m_selectionBar->hide();
    layout->addWidget(m_selectionBar);

    connect(FolderGates::instance(), &FolderGates::changed, this, &FolderView::updateGates);
    HiddenFolders::instance()->track(m_lister);
    connect(HiddenFolders::instance(), &HiddenFolders::changed, this, [this] {
        m_proxy->refreshHidden();
        Q_EMIT itemCountChanged();
    });
    connect(m_lister, &KDirLister::started, this, [this](const QUrl &url) {
        if (m_historyExpanding) {
            m_historyListingJobs.insert(url.adjusted(QUrl::StripTrailingSlash));
        }
        const QUrl listing = m_flat ? FlatView::listingUrl(m_folder) : m_folder;
        if (url.adjusted(QUrl::StripTrailingSlash) == listing.adjusted(QUrl::StripTrailingSlash)) {
            m_loading = true;
        }
        Q_EMIT loadingChanged(true);
    });
    connect(m_lister, &KDirLister::completed, this, [this] {
        m_loading = false;
        Q_EMIT loadingChanged(false);
        Q_EMIT itemCountChanged();
    });
    connect(m_lister, &KDirLister::canceled, this, [this] {
        m_loading = false;
        Q_EMIT loadingChanged(false);
    });
    connect(m_lister, &KDirLister::listingDirCompleted, this, [this](const QUrl &url) {
        if (m_historyExpanding) {
            m_historyListingJobs.remove(url.adjusted(QUrl::StripTrailingSlash));
            finishHistoryExpansionWhenIdle();
        }
        const QUrl listing = m_flat ? FlatView::listingUrl(m_folder) : m_folder;
        if (m_loading && url.adjusted(QUrl::StripTrailingSlash) == listing.adjusted(QUrl::StripTrailingSlash)) {
            m_loading = false;
            Q_EMIT listingReady();
        }
    });
    connect(m_lister, &KDirLister::listingDirCanceled, this, [this](const QUrl &url) {
        if (m_historyExpanding) {
            m_historyListingJobs.remove(url.adjusted(QUrl::StripTrailingSlash));
            finishHistoryExpansionWhenIdle();
        }
    });
    connect(m_lister, &KDirLister::redirection, this, [this](const QUrl &, const QUrl &newUrl) {
        if (!m_flat) {
            m_folder = newUrl;
        }
        Q_EMIT redirected(newUrl);
    });
    connect(m_lister, &KDirLister::jobError, this, [this](KIO::Job *job) {
        m_loading = false;
        Q_EMIT loadingChanged(false);
        Q_EMIT listingFailed(job->errorString());
    });
    connect(m_lister, &KDirLister::itemsAdded, this, &FolderView::itemCountChanged);
    connect(m_lister, &KDirLister::itemsAdded, this, [this](const QUrl &directory, const KFileItemList &items) {
        if (automaticFolderSizes() && !m_flat) {
            queueFolderSizes(items);
        }
        if (directory.adjusted(QUrl::StripTrailingSlash) != m_folder.adjusted(QUrl::StripTrailingSlash)) {
            scheduleFolderSizeRefresh();
        }
    });
    connect(m_lister, &KDirLister::itemsDeleted, this, &FolderView::itemCountChanged);
    connect(m_lister, &KDirLister::itemsDeleted, this, [this] {
        scheduleFolderSizeRefresh();
    });
    connect(m_lister, &KDirLister::refreshItems, this, [this] {
        scheduleFolderSizeRefresh();
    });
    connect(m_lister, &KDirLister::clear, this, &FolderView::itemCountChanged);

    connect(m_details->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FolderView::selectionChanged);
    connect(m_details->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FolderView::selectedItemsChanged);
    connect(m_details->selectionModel(), &QItemSelectionModel::currentChanged, this, &FolderView::selectionChanged);
    connect(m_details->selectionModel(), &QItemSelectionModel::currentChanged, this, [this] {
        if (!m_details->selectionModel()->hasSelection()) {
            Q_EMIT selectedItemsChanged();
        }
    });
    connect(m_model, &KDirModel::expand, this, [this](const QModelIndex &index) {
        if (m_historyExpanding && m_model->itemForIndex(index).isDir()) {
            m_details->expand(m_proxy->mapFromSource(index));
        }
    });
    connect(m_details, &QTreeView::expanded, this, &FolderView::itemsOrderChanged);
    connect(m_details, &QTreeView::collapsed, this, [this](const QModelIndex &collapsed) {
        QItemSelectionModel *selection = m_details->selectionModel();
        const QModelIndexList rows = selection->selectedRows();
        for (const QModelIndex &row : rows) {
            for (QModelIndex parent = row.parent(); parent.isValid(); parent = parent.parent()) {
                if (parent == collapsed) {
                    selection->select(row, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
                    break;
                }
            }
        }
        for (QModelIndex parent = selection->currentIndex().parent(); parent.isValid(); parent = parent.parent()) {
            if (parent == collapsed) {
                selection->setCurrentIndex(collapsed, QItemSelectionModel::NoUpdate);
                break;
            }
        }
        Q_EMIT itemsOrderChanged();
    });
    connect(m_details->header(), &QHeaderView::customContextMenuRequested, this, &FolderView::showColumnMenu);
    connect(m_details->header(), &QHeaderView::sortIndicatorChanged, this, [this] {
        saveFolderFormat();
        // Header clicks and folder formats sort without setSorting().
        Q_EMIT sortingChanged();
    });
    connect(m_details->header(), &QHeaderView::sectionMoved, this, [this] {
        saveFolderFormat();
    });
    applyDetailsPreferences();
    applyCompactPreferences();
    applyCardPreferences();
    m_defaultSelectionMarkers = Settings::defaultSelectionMarkers();
    m_defaultSelectionMode = Settings::defaultSelectionMode();
    setSelectionMarkersVisible(m_defaultSelectionMarkers);
    setSelectionMode(m_defaultSelectionMode);
}

FolderView::~FolderView()
{
    cancelFolderSizeJobs();
    // ~KCoreDirLister emits clear() while ~QWidget deletes children, after this type is gone.
    m_lister->disconnect(this);
}

void FolderView::setupView(QAbstractItemView *view)
{
    if (view == m_details) {
        view->setItemDelegate(new GroupHeaderDelegate(m_proxy, this, view));
    } else if (view == m_icons) {
        m_iconDelegate = new SelectionMarkerDelegate(this, view);
        view->setItemDelegate(m_iconDelegate);
    } else if (view == m_cards) {
        m_cardDelegate = new CardDelegate(view);
        m_cardDelegate->setSelectionMarkersVisible([this] {
            return m_selectionMarkers || m_selectionMode;
        });
        view->setItemDelegate(m_cardDelegate);
    } else {
        view->setItemDelegate(new SelectionMarkerDelegate(this, view));
    }
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setDragEnabled(true);
    view->setAcceptDrops(true);
    view->setDropIndicatorShown(true);
    view->setDragDropMode(QAbstractItemView::DragDrop);
    view->setDefaultDropAction(Qt::MoveAction);
    view->setTabKeyNavigation(false);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setFocusPolicy(Qt::StrongFocus);
    view->viewport()->installEventFilter(this);
    view->installEventFilter(this);

    connect(view, &QAbstractItemView::activated, this, &FolderView::onActivated);
    connect(view, &QAbstractItemView::customContextMenuRequested, this, &FolderView::onContextMenu);
}

QUrl FolderView::url() const
{
    return m_folder;
}

bool FolderView::isLoading() const
{
    return m_loading;
}

void FolderView::setUrl(const QUrl &url)
{
    if (!url.isValid() || url == m_folder) {
        return;
    }
    cancelFolderSizeJobs();
    m_folder = url;
    if (m_flat) {
        m_proxy->setFlatBase(url);
    }
    loadFolderFormat();
    openListing();
    updatePreviews();
    Q_EMIT urlChanged(url);
}

void FolderView::openListing()
{
    // A locked password-gated folder is not listed at all, so nothing inside
    // it is read, counted or previewed until it is unlocked.
    m_gateBlocked = FolderGates::instance()->isLocked(m_folder);
    m_proxy->setGateBlocked(m_gateBlocked);
    applyPreviewPlugins();
    if (m_gateBlocked) {
        m_lister->stop();
        Q_EMIT itemCountChanged();
        return;
    }
    m_lister->openUrl(m_flat ? FlatView::listingUrl(m_folder) : m_folder);
}

bool FolderView::isGateLocked() const
{
    return m_gateBlocked;
}

void FolderView::updateGates()
{
    m_proxy->refreshGates();
    if (FolderGates::instance()->isLocked(m_folder) != m_gateBlocked) {
        openListing();
    } else {
        applyPreviewPlugins();
    }
    Q_EMIT itemCountChanged();
}

void FolderView::reload()
{
    if (m_gateBlocked) {
        return;
    }
    cancelFolderSizeJobs();
    if (m_lister->url().isValid()) {
        m_lister->openUrl(m_lister->url(), KDirLister::Reload);
    }
}

FolderView::ViewMode FolderView::viewMode() const
{
    return m_viewMode;
}

bool FolderView::selectionMarkersVisible() const
{
    return m_selectionMarkers;
}

void FolderView::setSelectionMarkersVisible(bool visible)
{
    m_selectionMarkers = visible;
    currentView()->viewport()->update();
}

bool FolderView::selectionMode() const
{
    return m_selectionMode;
}

void FolderView::setSelectionMode(bool enabled)
{
    if (m_selectionMode == enabled) {
        return;
    }
    m_selectionMode = enabled;
    m_selectionBar->setVisible(enabled);
    currentView()->viewport()->update();
    Q_EMIT selectionModeChanged(enabled);
}

void FolderView::setInformationHoverEnabled(bool enabled)
{
    m_informationHoverEnabled = enabled;
    const QList<QAbstractItemView *> views{m_details, m_icons, m_compact, m_cards};
    for (QAbstractItemView *view : views) {
        view->setMouseTracking(enabled);
        view->viewport()->setMouseTracking(enabled);
    }
}

void FolderView::setViewMode(ViewMode mode)
{
    if (mode == m_viewMode) {
        return;
    }
    const bool hadFocus = currentView()->hasFocus();
    if (m_viewMode == ViewMode::Details && mode != ViewMode::Details) {
        QItemSelectionModel *selection = m_details->selectionModel();
        const QModelIndexList rows = selection->selectedRows();
        for (const QModelIndex &row : rows) {
            if (row.parent().isValid()) {
                selection->select(row, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
            }
        }
        QModelIndex current = selection->currentIndex();
        while (current.parent().isValid()) {
            current = current.parent();
        }
        if (current != selection->currentIndex()) {
            selection->setCurrentIndex(current, QItemSelectionModel::NoUpdate);
        }
    }
    m_viewMode = mode;
    m_stack->setCurrentWidget(currentView());
    updatePreviews();
    m_minimap->setView(currentView());
    if (hadFocus) {
        currentView()->setFocus();
    }
    saveFolderFormat();
    Q_EMIT viewModeChanged();
}

void FolderView::adjustIconSize(int steps)
{
    if (steps == 0) {
        return;
    }
    constexpr int MinimumSize = 16;
    constexpr int MaximumSize = 128;
    constexpr int StepSize = 8;
    if (m_viewMode == ViewMode::Details) {
        m_detailsIconSize = qBound(MinimumSize, m_detailsIconSize + steps * StepSize, MaximumSize);
    } else if (m_viewMode == ViewMode::Icons) {
        m_iconsIconSize = qBound(MinimumSize, m_iconsIconSize + steps * StepSize, MaximumSize);
    } else if (m_viewMode == ViewMode::Cards) {
        const int size = qBound(48, m_cardDelegate->previewSize() + steps * 16, 256);
        m_cardDelegate->setPreviewSize(size);
        m_cards->setIconSize(QSize(size, size));
        updateCardLayout();
        return;
    } else {
        const int size = qBound(MinimumSize, m_compact->iconSize().width() + steps * StepSize, MaximumSize);
        m_compact->setIconSize(QSize(size, size));
        return;
    }
    setIconSizes(m_detailsIconSize, m_iconsIconSize);
}

bool FolderView::showHiddenFiles() const
{
    return m_lister->showHiddenFiles();
}

void FolderView::setShowHiddenFiles(bool show)
{
    if (show == m_lister->showHiddenFiles()) {
        return;
    }
    m_lister->setShowHiddenFiles(show);
    m_lister->emitChanges();
    m_proxy->refreshHidden();
    Q_EMIT itemCountChanged();
    Q_EMIT hiddenFilesChanged(show);
}

bool FolderView::previewsEnabled() const
{
    return m_previewsEnabled;
}

void FolderView::setPreviewsEnabled(bool enabled)
{
    m_previewsEnabled = enabled;
    updatePreviews();
    Q_EMIT previewsEnabledChanged(enabled);
}

void FolderView::setRemotePreviewsEnabled(bool enabled)
{
    m_remotePreviewsEnabled = enabled;
    updatePreviews();
}

bool FolderView::previewsShown() const
{
    return m_previewsEnabled && (m_folder.isLocalFile() || m_remotePreviewsEnabled);
}

void FolderView::setPreviewPlugins(const QStringList &plugins)
{
    m_previewPlugins = plugins;
    applyPreviewPlugins();
}

QStringList FolderView::previewPlugins() const
{
    return m_previewPlugins.value_or(m_previews->enabledPlugins());
}

void FolderView::applyPreviewPlugins()
{
    if (!m_previews) {
        return;
    }
    // Folder thumbnails show the files inside a folder, so they are turned
    // off while a locked password-gated folder lies below the shown folder.
    const QStringList gates = FolderGates::instance()->lockedWithin(m_folder);
    const QString shown = FolderGates::canonicalPath(FolderGates::localPath(m_folder));
    const bool hideFolderThumbnails = std::any_of(gates.cbegin(), gates.cend(), [&shown](const QString &gate) {
        return gate != shown;
    });
    if (!m_previewPlugins && !hideFolderThumbnails) {
        return;
    }
    if (!m_previewPlugins) {
        m_previewPlugins = m_previews->enabledPlugins();
    }
    QStringList plugins = *m_previewPlugins;
    if (hideFolderThumbnails) {
        plugins.removeAll(QStringLiteral("directorythumbnail"));
    }
    if (plugins != m_previews->enabledPlugins()) {
        m_previews->setEnabledPlugins(plugins);
    }
}

void FolderView::setIconSizes(int details, int icons)
{
    m_detailsIconSize = details;
    m_iconsIconSize = icons;
    m_details->setIconSize(QSize(details, details));
    m_icons->setIconSize(QSize(icons, icons));
    const int height = qMax(icons + 48, icons + fontMetrics().height() * (Settings::iconNameLines() + Settings::iconInformation().size()) + 16);
    m_icons->setGridSize(QSize(icons + 48, height));
    m_iconDelegate->setMaximumSize(m_icons->gridSize());
}

void FolderView::setItemSpacing(int detailsSpace, int iconsSpace)
{
    auto *delegate = static_cast<GroupHeaderDelegate *>(m_details->itemDelegate());
    delegate->setItemSpace(detailsSpace);
    m_details->doItemsLayout();
    m_icons->setSpacing(2 * iconsSpace);
}

void FolderView::setDragFromAnywhere(bool enabled)
{
    m_details->setDragFromAnywhere(enabled);
}

void FolderView::setColumnVisible(int column, bool visible)
{
    if (column != KDirModel::Name) {
        m_details->setColumnHidden(column, !visible);
    }
}

bool FolderView::isColumnVisible(int column) const
{
    return !m_details->isColumnHidden(column);
}

void FolderView::setRatingColumn(bool enabled)
{
    m_proxy->setRatingColumn(enabled);
}

void FolderView::setSorting(int column, Qt::SortOrder order)
{
    m_details->sortByColumn(column, order);
    Q_EMIT sortingChanged();
}

int FolderView::sortColumn() const
{
    return m_details->header()->sortIndicatorSection();
}

Qt::SortOrder FolderView::sortOrder() const
{
    return m_details->header()->sortIndicatorOrder();
}

void FolderView::setSortFoldersFirst(bool foldersFirst)
{
    m_proxy->setSortFoldersFirst(foldersFirst);
}

bool FolderView::sortFoldersFirst() const
{
    return m_proxy->sortFoldersFirst();
}

void FolderView::setShowToolTips(bool show)
{
    auto *detailsDelegate = static_cast<GroupHeaderDelegate *>(m_details->itemDelegate());
    detailsDelegate->setShowToolTipWhenElided(show);
    m_iconDelegate->setShowToolTipWhenElided(show);
    static_cast<KFileItemDelegate *>(m_compact->itemDelegate())->setShowToolTipWhenElided(show);
}

bool FolderView::showToolTips() const
{
    return static_cast<GroupHeaderDelegate *>(m_details->itemDelegate())->showToolTipWhenElided();
}

void FolderView::updatePreviews()
{
    // The views share one model, so a generator writes its preview size into
    // the decoration role every view shows, and it lays out its view for each
    // batch of listed items, which in a large folder costs more than the
    // listing. Only the shown view keeps a generator.
    const bool shown = previewsShown();
    QAbstractItemView *view = currentView();
    if (m_previews && m_previews->parent() == view) {
        m_previews->setPreviewShown(shown);
        return;
    }
    if (m_previews) {
        m_previews->setPreviewShown(false);
        delete m_previews;
    }
    m_previews = new KFilePreviewGenerator(view);
    applyPreviewPlugins();
    if (shown) {
        m_previews->updateIcons();
    } else {
        m_previews->setPreviewShown(false);
    }
}

KFileItem FolderView::rootItem() const
{
    return m_flat || m_gateBlocked ? KFileItem(m_folder) : m_lister->rootItem();
}

KFileItemList FolderView::items() const
{
    return m_gateBlocked ? KFileItemList() : m_lister->items();
}

KFileItemList FolderView::itemsInViewOrder() const
{
    KFileItemList result;
    QModelIndex index = m_proxy->index(0, 0);
    while (index.isValid()) {
        const KFileItem item = m_model->itemForIndex(m_proxy->mapToSource(index));
        if (!item.isNull()) {
            result.append(item);
        }
        if (m_viewMode == ViewMode::Details) {
            index = m_details->indexBelow(index);
        } else {
            index = index.siblingAtRow(index.row() + 1);
        }
    }
    return result;
}

ListingExport::Table FolderView::listingForExport(const QList<int> &selectedColumns) const
{
    ListingExport::Table table;
    QList<int> columns;
    QHeaderView *header = m_details->header();
    for (int visual = 0; visual < header->count(); ++visual) {
        const int column = header->logicalIndex(visual);
        if (column >= 0 && (selectedColumns.isEmpty() ? !m_details->isColumnHidden(column) : selectedColumns.contains(column))) {
            columns.append(column);
            table.headers.append(m_proxy->headerData(column, Qt::Horizontal, Qt::DisplayRole).toString());
        }
    }
    QModelIndex index = m_proxy->index(0, 0);
    while (index.isValid()) {
        QStringList row;
        row.reserve(columns.size());
        for (int column : columns) {
            row.append(index.siblingAtColumn(column).data(Qt::DisplayRole).toString());
        }
        table.rows.append(row);
        index = m_viewMode == ViewMode::Details ? m_details->indexBelow(index) : index.siblingAtRow(index.row() + 1);
    }
    return table;
}

KFileItemList FolderView::selectedItems() const
{
    KFileItemList result;
    const QModelIndexList rows = currentView()->selectionModel()->selectedRows();
    result.reserve(rows.size());
    for (const QModelIndex &index : rows) {
        const KFileItem item = m_model->itemForIndex(m_proxy->mapToSource(index));
        if (!item.isNull()) {
            result.append(item);
        }
    }
    return result;
}

KFileItemList FolderView::selectedItemsOrRoot() const
{
    KFileItemList result = selectedItems();
    if (result.isEmpty()) {
        const KFileItem root = rootItem();
        if (!root.isNull()) {
            result.append(root);
        }
    }
    return result;
}

KFileItem FolderView::currentItem() const
{
    return m_model->itemForIndex(m_proxy->mapToSource(currentView()->currentIndex()));
}

int FolderView::itemCount() const
{
    return m_proxy->rowCount();
}

void FolderView::selectAll()
{
    currentView()->selectAll();
}

int FolderView::applySelectionPattern(const QString &pattern, bool select)
{
    const NameFilter filter(pattern, NameFilter::Scope::FilesAndFolders);
    QItemSelectionModel *selection = currentView()->selectionModel();
    const QItemSelectionModel::SelectionFlags operation = select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect;
    int matched = 0;
    for (int row = 0; row < m_proxy->rowCount(); ++row) {
        const QModelIndex index = m_proxy->index(row, 0);
        const KFileItem item = m_model->itemForIndex(m_proxy->mapToSource(index));
        if (filter.matches(item.name(), item.isDir())) {
            selection->select(index, operation | QItemSelectionModel::Rows);
            ++matched;
        }
    }
    return matched;
}

int FolderView::applySelectionPathFilter(const PathFilter &filter, bool select, QString *error)
{
    if (error) {
        error->clear();
    }
    const QString filterError = filter.error();
    if (!m_folder.isLocalFile() || !filterError.isEmpty()) {
        if (error) {
            *error = !filterError.isEmpty() ? filterError : i18nc("@info", "Path selection filters support local folders only.");
        }
        return -1;
    }
    QList<QModelIndex> matches;
    for (int row = 0; row < m_proxy->rowCount(); ++row) {
        const QModelIndex index = m_proxy->index(row, 0);
        const KFileItem item = m_model->itemForIndex(m_proxy->mapToSource(index));
        if (!item.url().isLocalFile()) {
            if (error) {
                *error = i18nc("@info", "The listing contains a non-local item; no selection was changed.");
            }
            return -1;
        }
        if (filter.matches(m_folder.toLocalFile(), item.url().toLocalFile())) {
            matches.append(index);
        }
    }
    const auto operation = select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect;
    for (const QModelIndex &index : std::as_const(matches)) {
        currentView()->selectionModel()->select(index, operation | QItemSelectionModel::Rows);
    }
    return matches.size();
}

int FolderView::applySelectionQuery(const SearchQuery::Query &query,
                                    const QStringList &tagLibraries,
                                    const TagSearch::Options &tagOptions,
                                    const QStringList &metadataKeys,
                                    bool select,
                                    QString *error)
{
    if (error) {
        error->clear();
    }
    QString problem;
    if (!query.isValid()) {
        problem = query.error();
    } else if (query.usesContent()) {
        problem = i18nc("@info", "Selection queries can't use content criteria. Use Search to find text within files.");
    } else if (!m_folder.isLocalFile()) {
        problem = i18nc("@info", "Selection queries support local folders only.");
    }
    if (!problem.isEmpty()) {
        if (error) {
            *error = problem;
        }
        return -1;
    }
    TagSearch::Context tags(tagLibraries, tagOptions);
    QList<QModelIndex> matches;
    for (int row = 0; row < m_proxy->rowCount(); ++row) {
        const QModelIndex index = m_proxy->index(row, 0);
        const KFileItem item = m_model->itemForIndex(m_proxy->mapToSource(index));
        if (!item.url().isLocalFile()) {
            if (error) {
                *error = i18nc("@info", "The listing contains a non-local item; no selection was changed.");
            }
            return -1;
        }
        const QString path = item.url().toLocalFile();
        if (query.matches(QFileInfo(path), {}, tags.matcher(path), EmbeddedMetadata::lazySearchTexts(path, metadataKeys))) {
            matches.append(index);
        }
    }
    if (!tags.errors().isEmpty()) {
        if (error) {
            *error = tags.errors().join(QLatin1Char('\n'));
        }
        return -1;
    }
    const auto operation = select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect;
    for (const QModelIndex &index : std::as_const(matches)) {
        currentView()->selectionModel()->select(index, operation | QItemSelectionModel::Rows);
    }
    return matches.size();
}

void FolderView::clearSelection()
{
    currentView()->selectionModel()->clearSelection();
}

void FolderView::invertSelection()
{
    QAbstractItemView *view = currentView();
    const QAbstractItemModel *model = view->model();
    const auto *tree = qobject_cast<const QTreeView *>(view);
    QItemSelection items;
    QList<QModelIndex> parents{QModelIndex()};
    while (!parents.isEmpty()) {
        const QModelIndex parent = parents.takeLast();
        const int rows = model->rowCount(parent);
        if (rows == 0) {
            continue;
        }
        items.select(model->index(0, 0, parent), model->index(rows - 1, 0, parent));
        // Only the items of expanded folders are shown, so only they change.
        for (int row = 0; tree && row < rows; ++row) {
            const QModelIndex index = model->index(row, 0, parent);
            if (tree->isExpanded(index)) {
                parents.append(index);
            }
        }
    }
    view->selectionModel()->select(items, QItemSelectionModel::Toggle | QItemSelectionModel::Rows);
}

void FolderView::selectUrls(const QList<QUrl> &urls)
{
    QItemSelectionModel *selection = currentView()->selectionModel();
    selection->clearSelection();
    bool first = true;
    for (const QUrl &url : urls) {
        const QModelIndex index = m_proxy->mapFromSource(m_model->indexForUrl(url));
        if (!index.isValid()) {
            continue;
        }
        selection->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        if (first) {
            selection->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
            currentView()->scrollTo(index);
            first = false;
        }
    }
}

bool FolderView::expandForHistorySelection(const QList<QUrl> &urls)
{
    if (m_viewMode != ViewMode::Details || m_flat || !Settings::inlineFolderExpansion()) {
        return false;
    }
    QList<QUrl> nested;
    const QUrl root = m_folder.adjusted(QUrl::StripTrailingSlash);
    for (const QUrl &url : urls) {
        const QUrl parent = url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash);
        if (root.isParentOf(parent) && parent != root) {
            nested.append(url);
        }
    }
    if (nested.isEmpty()) {
        return false;
    }
    m_historyExpanding = true;
    ++m_historyExpansionGeneration;
    m_historyListingJobs.clear();
    for (const QUrl &url : nested) {
        m_model->expandToUrl(url);
    }
    finishHistoryExpansionWhenIdle();
    return true;
}

void FolderView::cancelHistoryExpansion()
{
    ++m_historyExpansionGeneration;
    m_historyExpanding = false;
    m_historyListingJobs.clear();
}

void FolderView::finishHistoryExpansionWhenIdle()
{
    if (!m_historyExpanding || !m_historyListingJobs.isEmpty()) {
        return;
    }
    const quint64 generation = m_historyExpansionGeneration;
    QTimer::singleShot(0, this, [this, generation] {
        if (m_historyExpanding && generation == m_historyExpansionGeneration && m_historyListingJobs.isEmpty()) {
            m_historyExpanding = false;
            Q_EMIT historyExpansionFinished();
        }
    });
}

QAbstractItemView *FolderView::currentView() const
{
    if (m_viewMode == ViewMode::Details) {
        return m_details;
    }
    switch (m_viewMode) {
    case ViewMode::Icons:
        return m_icons;
    case ViewMode::Cards:
        return m_cards;
    default:
        return m_compact;
    }
}

NameFilter FolderView::nameFilter() const
{
    return m_proxy->nameFilter();
}

void FolderView::setNameFilter(const NameFilter &filter)
{
    if (filter == m_proxy->nameFilter()) {
        return;
    }
    m_proxy->setNameFilter(filter);
    Q_EMIT itemCountChanged();
}

bool FolderView::isFlat() const
{
    return m_flat;
}

void FolderView::setFlat(bool flat)
{
    if (flat == m_flat) {
        return;
    }
    cancelFolderSizeJobs();
    m_flat = flat;
    m_details->setRootIsDecorated(!flat && Settings::inlineFolderExpansion());
    m_details->setItemsExpandable(!flat && Settings::inlineFolderExpansion());
    if (flat) {
        m_details->collapseAll();
    }
    m_proxy->setFlatBase(flat ? m_folder : QUrl());
    if (m_folder.isValid()) {
        openListing();
    }
}

void FolderView::setFileListMinimapVisible(bool visible)
{
    m_minimap->setVisible(visible);
}

void FolderView::reloadMetadata()
{
    QString error;
    const QJsonObject metadata = m_folder.isLocalFile() ? FolderMetadata::read(m_folder.toLocalFile(), &error) : QJsonObject();
    ViewSettings settings = ViewSettings::fromConfig(KSharedConfig::openConfig()->group(QStringLiteral("View")));
    if (Settings::showRatingColumn() && !settings.visibleColumns.contains(QStringLiteral("rating"))) {
        settings.visibleColumns.append(QStringLiteral("rating"));
    } else if (!Settings::showRatingColumn()) {
        settings.visibleColumns.removeAll(QStringLiteral("rating"));
    }
    const KConfigGroup filesGroup = KSharedConfig::openConfig()->group(QStringLiteral("Files"));
    m_proxy->setSettings(settings, FileColorRules::fromConfig(filesGroup), IgnoreList::fromConfig(filesGroup));
    m_proxy->setMetadata(metadata, m_folder.isLocalFile() ? m_folder.toLocalFile() : QString());
    m_details->viewport()->update();
    m_icons->viewport()->update();
    m_compact->viewport()->update();
    m_cards->viewport()->update();
    if (!m_formatLocked && m_formatRuleSource.isEmpty()) {
        m_proxy->setGroupBy(settings.groupBy);
    }
    if (!error.isEmpty()) {
        Q_EMIT listingFailed(error);
    }
}

bool FolderView::hasFolderFormat() const
{
    return m_formatLocked || !m_formatRuleSource.isEmpty();
}

QString FolderView::formatRuleDescription() const
{
    if (m_formatLocked) {
        return i18nc("@info:status folder format", "Format locked in this pane");
    }
    if (!m_formatRuleSource.isEmpty()) {
        return i18nc("@info:status folder format", "Folder format: %1", m_formatRuleSource);
    }
    return i18nc("@info:status folder format", "Folder format: defaults");
}

QString FolderView::formatRuleSource() const
{
    return m_formatRuleSource;
}

bool FolderView::hasLocalFolderFormat() const
{
    FolderMetadata::Format format;
    return m_folder.isLocalFile() && FolderMetadata::format(m_proxy->metadata(), &format);
}

void FolderView::clearLocalFolderFormat()
{
    if (!hasLocalFolderFormat() || m_formatLocked || m_flat) {
        return;
    }
    QString error;
    if (!FolderMetadata::clearFormat(m_folder.toLocalFile(), &error)) {
        Q_EMIT listingFailed(error);
        return;
    }
    loadFolderFormat();
    refreshFolderSizePolicy();
}

bool FolderView::formatLocked() const
{
    return m_formatLocked;
}

void FolderView::setFormatLocked(bool locked)
{
    if (m_formatLocked == locked) {
        return;
    }
    if (locked) {
        m_lockedAutomaticFolderSizes = automaticFolderSizes();
    }
    m_formatLocked = locked;
    if (!locked) {
        loadFolderFormat();
        refreshFolderSizePolicy();
    }
}

bool FolderView::formatIncludesDescendants() const
{
    FolderMetadata::Format format;
    return m_folder.isLocalFile() && FolderMetadata::format(m_proxy->metadata(), &format) && format.includeDescendants;
}

void FolderView::setFormatIncludesDescendants(bool include)
{
    if (!m_folder.isLocalFile() || m_formatLocked) {
        return;
    }
    FolderMetadata::Format format = currentFormat();
    format.calculateFolderSizes = m_ruleCalculatesFolderSizes;
    format.hasFolderSizePolicy = m_ruleHasFolderSizePolicy;
    FolderMetadata::Format existing;
    if (FolderMetadata::format(m_proxy->metadata(), &existing)) {
        format.calculateFolderSizes = existing.calculateFolderSizes;
        format.hasFolderSizePolicy = existing.hasFolderSizePolicy;
    }
    format.includeDescendants = include;
    QString error;
    if (!FolderMetadata::setFormat(m_folder.toLocalFile(), format, &error)) {
        Q_EMIT listingFailed(error);
        return;
    }
    loadFolderFormat();
}

bool FolderView::automaticFolderSizes() const
{
    if (m_formatLocked) {
        return m_lockedAutomaticFolderSizes;
    }
    if (m_ruleHasFolderSizePolicy) {
        return m_ruleCalculatesFolderSizes;
    }
    return m_folder.isLocalFile() ? Settings::automaticLocalFolderSizes() : Settings::automaticRemoteFolderSizes();
}

void FolderView::setAutomaticFolderSizes(bool enabled)
{
    if (!m_folder.isLocalFile() || m_formatLocked) {
        return;
    }
    FolderMetadata::Format format = currentFormat();
    FolderMetadata::Format existing;
    if (FolderMetadata::format(m_proxy->metadata(), &existing)) {
        format.includeDescendants = existing.includeDescendants;
    }
    format.calculateFolderSizes = enabled;
    format.hasFolderSizePolicy = true;
    QString error;
    if (!FolderMetadata::setFormat(m_folder.toLocalFile(), format, &error)) {
        Q_EMIT listingFailed(error);
        return;
    }
    loadFolderFormat();
    refreshFolderSizePolicy();
}

void FolderView::clearFolderSizePolicy()
{
    if (!m_folder.isLocalFile() || m_formatLocked) {
        return;
    }
    FolderMetadata::Format format = currentFormat();
    FolderMetadata::Format existing;
    if (FolderMetadata::format(m_proxy->metadata(), &existing)) {
        format.includeDescendants = existing.includeDescendants;
    }
    QString error;
    if (!FolderMetadata::setFormat(m_folder.toLocalFile(), format, &error)) {
        Q_EMIT listingFailed(error);
        return;
    }
    loadFolderFormat();
    refreshFolderSizePolicy();
}

void FolderView::calculateFolderSizes()
{
    const KFileItemList selected = selectedItems();
    queueFolderSizes(selected.isEmpty() ? items() : selected);
}

void FolderView::refreshFolderSizePolicy()
{
    cancelFolderSizeJobs();
    if (automaticFolderSizes() && !m_flat) {
        queueFolderSizes(items());
    }
}

void FolderView::queueFolderSizes(const KFileItemList &items)
{
    for (const KFileItem &item : items) {
        if (item.isDir() && !item.isLink() && !m_sizePending.contains(item.url())) {
            m_sizePending.insert(item.url());
            m_sizeQueue.append(item);
        }
    }
    startFolderSizeJobs();
}

void FolderView::startFolderSizeJobs()
{
    while (m_sizeJobs.size() < 4 && !m_sizeQueue.isEmpty()) {
        const KFileItem item = m_sizeQueue.takeFirst();
        KIO::DirectorySizeJob *job = KIO::directorySize(KFileItemList{item});
        m_sizeJobs.insert(job);
        connect(job, &KJob::result, this, [this, job, url = item.url()] {
            m_sizeJobs.remove(job);
            m_sizePending.remove(url);
            if (job->error()) {
                Q_EMIT listingFailed(i18nc("@info:status folder size", "Could not calculate size of %1: %2", url.toDisplayString(), job->errorString()));
            } else {
                m_proxy->setFolderSize(url, job->totalSize());
            }
            startFolderSizeJobs();
        });
    }
}

void FolderView::cancelFolderSizeJobs()
{
    ++m_sizeJobGeneration;
    m_sizeRefreshScheduled = false;
    m_sizeQueue.clear();
    m_sizePending.clear();
    const auto jobs = m_sizeJobs;
    m_sizeJobs.clear();
    for (KIO::DirectorySizeJob *job : jobs) {
        job->disconnect(this);
        job->kill(KJob::Quietly);
    }
    m_proxy->clearFolderSizes();
}

void FolderView::scheduleFolderSizeRefresh()
{
    if (m_flat || m_sizeRefreshScheduled || (m_sizePending.isEmpty() && m_proxy->calculatedFolderUrls().isEmpty())) {
        return;
    }
    m_sizeRefreshScheduled = true;
    const quint64 generation = m_sizeJobGeneration;
    QTimer::singleShot(0, this, [this, generation] {
        if (generation != m_sizeJobGeneration) {
            return;
        }
        QSet<QUrl> urls(m_sizePending);
        const QList<QUrl> calculated = m_proxy->calculatedFolderUrls();
        for (const QUrl &url : calculated) {
            urls.insert(url);
        }
        cancelFolderSizeJobs();
        KFileItemList items;
        for (const QUrl &url : std::as_const(urls)) {
            const KFileItem item = m_model->itemForIndex(m_model->indexForUrl(url));
            if (!item.isNull()) {
                items.append(item);
            }
        }
        if (automaticFolderSizes()) {
            items.append(this->items());
        }
        queueFolderSizes(items);
    });
}

void FolderView::updateFolderSizeRefreshTimer()
{
    const int seconds = qBound(0, Settings::folderSizeRefreshInterval(), 86400);
    if (seconds == 0) {
        m_sizeRefreshTimer->stop();
    } else {
        m_sizeRefreshTimer->start(seconds * 1000);
    }
}

QString FolderView::groupBy() const
{
    return m_proxy->groupBy();
}

void FolderView::setGroupBy(const QString &group)
{
    m_proxy->setGroupBy(group);
    saveFolderFormat();
}

void FolderView::reloadSettings()
{
    loadFolderFormat();
}

void FolderView::reloadListingSettings()
{
    m_minimap->setMinimapWidth(Settings::fileListMinimapWidth());
    setFileListMinimapVisible(Settings::showFileListMinimap());
    updateFolderSizeRefreshTimer();
    applyDetailsPreferences();
    applyCompactPreferences();
    applyCardPreferences();
    if (m_defaultSelectionMarkers != Settings::defaultSelectionMarkers()) {
        m_defaultSelectionMarkers = Settings::defaultSelectionMarkers();
        setSelectionMarkersVisible(m_defaultSelectionMarkers);
    }
    if (m_defaultSelectionMode != Settings::defaultSelectionMode()) {
        m_defaultSelectionMode = Settings::defaultSelectionMode();
        setSelectionMode(m_defaultSelectionMode);
    }
    m_iconDelegate->setShowInformation(informationForKeys(Settings::iconInformation()));
    setIconSizes(Settings::detailsIconSize(), Settings::iconsIconSize());
    m_icons->doItemsLayout();
    reloadMetadata();
    m_details->viewport()->update();
    m_icons->viewport()->update();
    m_compact->viewport()->update();
    m_cards->viewport()->update();
}

void FolderView::applyCompactPreferences()
{
    const int size = Settings::compactIconSize();
    m_compact->setIconSize(QSize(size, size));
    m_compact->setSpacing(Settings::compactItemSpacing());
    static_cast<KFileItemDelegate *>(m_compact->itemDelegate())->setShowInformation(informationForKeys(Settings::compactInformation()));
    m_compact->doItemsLayout();
}

void FolderView::applyCardPreferences()
{
    const int preview = Settings::cardPreviewSize();
    m_cardDelegate->setPreviewSize(preview);
    m_cardDelegate->setItemSpace(Density::itemSpace(static_cast<Density::Items>(qBound(0, int(Settings::cardsDensity()), 3)), Settings::cardsCustomSpace()));
    m_cardDelegate->setInformation(Settings::cardInformation());
    // The preview generator renders at the view's icon size.
    m_cards->setIconSize(QSize(preview, preview));
    updateCardLayout();
}

void FolderView::setListFont(const QFont &font)
{
    setFont(font);
    // The icon grid and card cells are sized from the font.
    setIconSizes(m_detailsIconSize, m_iconsIconSize);
    updateCardLayout();
    m_details->doItemsLayout();
    m_compact->doItemsLayout();
}

void FolderView::updateCardLayout()
{
    const int columns = Settings::cardColumns();
    int width = 0;
    if (columns > 0) {
        // Always reserve the scroll bar so showing it cannot change the column width.
        const int available = m_cards->width() - 2 * m_cards->frameWidth() - m_cards->style()->pixelMetric(QStyle::PM_ScrollBarExtent, nullptr, m_cards);
        width = qMax(m_cardDelegate->minimumCellWidth(), available / columns);
    }
    m_cardDelegate->setCellWidth(width);
    const QSize cell = m_cardDelegate->cellSize(m_cards->font());
    if (m_cards->gridSize() != cell) {
        m_cards->setGridSize(cell);
    } else {
        m_cards->doItemsLayout();
    }
    m_cards->viewport()->update();
}

bool FolderView::renameInline()
{
    if (m_viewMode != ViewMode::Cards) {
        return false;
    }
    const QModelIndexList rows = m_cards->selectionModel()->selectedRows();
    if (rows.size() != 1 || !(rows.first().flags() & Qt::ItemIsEditable)) {
        return false;
    }
    const QModelIndex index = rows.first();
    m_cards->selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
    m_cards->scrollTo(index);
    m_cards->edit(index);
    return m_cards->indexWidget(index) != nullptr;
}

void FolderView::applyDetailsPreferences()
{
    const bool expandable = !m_flat && Settings::inlineFolderExpansion();
    if (!expandable) {
        const bool restoringHistory = m_historyExpanding;
        cancelHistoryExpansion();
        m_details->collapseAll();
        if (restoringHistory) {
            Q_EMIT historyExpansionFinished();
        }
    }
    m_details->setRootIsDecorated(expandable);
    m_details->setItemsExpandable(expandable);
    m_details->setAlternatingRowColors(EffectiveAppearance::stripeDetailsRows());
    QHeaderView *header = m_details->header();
    const int width = Settings::detailsColumnWidth();
    header->setSectionResizeMode(width == Settings::EnumDetailsColumnWidth::Automatic ? QHeaderView::ResizeToContents : QHeaderView::Interactive);
    if (width == Settings::EnumDetailsColumnWidth::StretchName) {
        header->setSectionResizeMode(KDirModel::Name, QHeaderView::Stretch);
    }
}

void FolderView::loadFolderFormat()
{
    const FolderMetadata::Format lockedFormat = currentFormat();
    m_applyingFolderFormat = true;
    reloadMetadata();
    const KConfigGroup defaults = KSharedConfig::openConfig()->group(QStringLiteral("Views"));
    const ViewSettings settings = ViewSettings::fromConfig(KSharedConfig::openConfig()->group(QStringLiteral("View")));
    FolderMetadata::Format format;
    QString ruleError;
    m_formatRuleSource.clear();
    const bool foundRule = m_folder.isLocalFile() && FolderMetadata::findFormat(m_folder.toLocalFile(), &format, &m_formatRuleSource, &ruleError);
    m_ruleCalculatesFolderSizes = foundRule && format.calculateFolderSizes;
    m_ruleHasFolderSizePolicy = foundRule && format.hasFolderSizePolicy;
    if (!ruleError.isEmpty()) {
        Q_EMIT listingFailed(ruleError);
    }
    if (m_formatLocked) {
        format = lockedFormat;
    }
    if (m_formatLocked || foundRule) {
        setViewMode(format.view == QLatin1String("grid")          ? ViewMode::Icons
                        : format.view == QLatin1String("compact") ? ViewMode::Compact
                        : format.view == QLatin1String("cards")   ? ViewMode::Cards
                                                                  : ViewMode::Details);
        m_proxy->setGroupBy(format.group);
        m_proxy->setRatingColumn(format.columns.contains(QStringLiteral("rating")));
        for (const auto &[column, key] : OptionalColumns) {
            const bool visible = column == KDirModel::Group
                ? format.columns.contains(QStringLiteral("group")) || format.columns.contains(QStringLiteral("rating"))
                : format.columns.contains(key);
            m_details->setColumnHidden(column, !visible);
        }
        QHeaderView *header = m_details->header();
        header->moveSection(header->visualIndex(KDirModel::Name), 0);
        int visual = 1;
        QSet<int> orderedColumns;
        for (const QString &key : format.columns) {
            const int column = key == QLatin1String("rating") ? KDirModel::Group : columnForSortKey(key);
            if (column != KDirModel::Name && !orderedColumns.contains(column) && header->visualIndex(column) >= 0) {
                header->moveSection(header->visualIndex(column), visual++);
                orderedColumns.insert(column);
            }
        }
        m_details->sortByColumn(columnForSortKey(format.sortKey), format.sortAscending ? Qt::AscendingOrder : Qt::DescendingOrder);
    } else {
        QHeaderView *header = m_details->header();
        for (int column = 0; column < header->count(); ++column) {
            header->moveSection(header->visualIndex(column), column);
        }
        m_proxy->setGroupBy(settings.groupBy);
        m_proxy->setRatingColumn(Settings::showRatingColumn());
        setColumnVisible(KDirModel::Size, Settings::showSizeColumn());
        setColumnVisible(KDirModel::ModifiedTime, Settings::showModifiedColumn());
        setColumnVisible(KDirModel::Permissions, Settings::showPermissionsColumn());
        setColumnVisible(KDirModel::Owner, Settings::showOwnerColumn());
        setColumnVisible(KDirModel::Group, Settings::showGroupColumn() || Settings::showRatingColumn());
        setColumnVisible(KDirModel::Type, Settings::showTypeColumn());
        setViewMode(static_cast<ViewMode>(qBound(0, int(EffectiveAppearance::defaultViewMode()), int(ViewMode::Cards))));
        m_details->sortByColumn(Settings::sortColumn(), Settings::sortDescending() ? Qt::DescendingOrder : Qt::AscendingOrder);
        setShowHiddenFiles(Settings::showHiddenFiles());
    }
    m_proxy->setSortFoldersFirst(Settings::foldersFirst());
    setIconSizes(Settings::detailsIconSize(), Settings::iconsIconSize());
    const auto itemSpace = [&defaults](const QString &densityKey, const QString &customKey) {
        const QString density = defaults.readEntry(densityKey, QStringLiteral("Normal"));
        return density == QLatin1String("VeryCompact") ? 0
            : density == QLatin1String("Compact")      ? 1
            : density == QLatin1String("Custom")       ? qBound(0, defaults.readEntry(customKey, 2), 10)
                                                       : 2;
    };
    setItemSpacing(itemSpace(QStringLiteral("DetailsDensity"), QStringLiteral("DetailsCustomSpace")),
                   itemSpace(QStringLiteral("IconsDensity"), QStringLiteral("IconsCustomSpace")));
    setDragFromAnywhere(defaults.readEntry("DragFromAnywhere", false));
    setShowToolTips(defaults.readEntry("ShowToolTips", true));

    m_iconDelegate->setShowInformation(informationForKeys(settings.iconInformation));
    setIconSizes(Settings::detailsIconSize(), Settings::iconsIconSize());
    m_icons->doItemsLayout();
    m_applyingFolderFormat = false;
}

void FolderView::saveFolderFormat()
{
    if (m_applyingFolderFormat || m_formatLocked || m_flat || !m_folder.isLocalFile()) {
        return;
    }
    FolderMetadata::Format format = currentFormat();
    format.calculateFolderSizes = m_ruleCalculatesFolderSizes;
    format.hasFolderSizePolicy = m_ruleHasFolderSizePolicy;
    FolderMetadata::Format existing;
    if (FolderMetadata::format(m_proxy->metadata(), &existing)) {
        format.includeDescendants = existing.includeDescendants;
        format.calculateFolderSizes = existing.calculateFolderSizes;
        format.hasFolderSizePolicy = existing.hasFolderSizePolicy;
    }
    QString error;
    if (!FolderMetadata::setFormat(m_folder.toLocalFile(), format, &error)) {
        Q_EMIT listingFailed(error);
        return;
    }
    m_formatRuleSource = m_folder.toLocalFile();
    reloadMetadata();
}

FolderMetadata::Format FolderView::currentFormat() const
{
    FolderMetadata::Format format;
    switch (m_viewMode) {
    case ViewMode::Icons:
        format.view = QStringLiteral("grid");
        break;
    case ViewMode::Compact:
        format.view = QStringLiteral("compact");
        break;
    case ViewMode::Cards:
        format.view = QStringLiteral("cards");
        break;
    case ViewMode::Details:
        format.view = QStringLiteral("list");
        break;
    }
    for (int visual = 0; visual < m_details->header()->count(); ++visual) {
        const int column = m_details->header()->logicalIndex(visual);
        if (column == KDirModel::Name || m_details->isColumnHidden(column)) {
            continue;
        }
        if (column == KDirModel::Group) {
            format.columns.append(m_proxy->ratingColumn() ? QStringLiteral("rating") : QStringLiteral("group"));
            continue;
        }
        for (const auto &[candidate, key] : OptionalColumns) {
            if (candidate == column) {
                format.columns.append(key);
                break;
            }
        }
    }
    format.sortKey = sortKeyForColumn(m_details->header()->sortIndicatorSection(), m_proxy->ratingColumn());
    format.sortAscending = m_details->header()->sortIndicatorOrder() == Qt::AscendingOrder;
    format.group = m_proxy->groupBy();
    return format;
}

void FolderView::showColumnMenu(const QPoint &pos)
{
    QMenu menu(this);
    for (const auto &[column, key] : OptionalColumns) {
        Q_UNUSED(key)
        if (column == KDirModel::Group) {
            continue;
        }
        QString text;
        switch (column) {
        case KDirModel::Type:
            text = i18nc("@option:check", "Type");
            break;
        case KDirModel::Size:
            text = i18nc("@option:check", "Size");
            break;
        case KDirModel::ModifiedTime:
            text = i18nc("@option:check", "Modified");
            break;
        case KDirModel::Permissions:
            text = i18nc("@option:check", "Permissions");
            break;
        case KDirModel::Owner:
            text = i18nc("@option:check", "Owner");
            break;
        case KDirModel::Group:
            text = i18nc("@option:check", "Group");
            break;
        default:
            continue;
        }
        QAction *action = menu.addAction(text);
        action->setCheckable(true);
        action->setChecked(!m_details->isColumnHidden(column));
        connect(action, &QAction::toggled, this, [this, column](bool shown) {
            m_details->setColumnHidden(column, !shown);
            saveFolderFormat();
        });
    }
    QMenu *groupColumn = menu.addMenu(i18nc("@menu", "Group Column"));
    const bool groupVisible = !m_details->isColumnHidden(KDirModel::Group);
    const QList<QPair<bool, QString>> groupColumnOptions{
        {false, i18nc("@option:check", "Group")},
        {true, i18nc("@option:check", "Rating")},
    };
    auto *groupColumnActions = new QActionGroup(groupColumn);
    groupColumnActions->setExclusive(true);
    for (const auto &[rating, label] : groupColumnOptions) {
        QAction *action = groupColumn->addAction(label);
        action->setCheckable(true);
        action->setChecked(groupVisible && m_proxy->ratingColumn() == rating);
        groupColumnActions->addAction(action);
        connect(action, &QAction::triggered, this, [this, rating] {
            m_proxy->setRatingColumn(rating);
            m_details->setColumnHidden(KDirModel::Group, false);
            saveFolderFormat();
        });
    }
    QAction *hideGroupColumn = groupColumn->addAction(i18nc("@option:check", "Hide Column"));
    hideGroupColumn->setCheckable(true);
    hideGroupColumn->setChecked(!groupVisible);
    groupColumnActions->addAction(hideGroupColumn);
    connect(hideGroupColumn, &QAction::triggered, this, [this] {
        m_details->setColumnHidden(KDirModel::Group, true);
        saveFolderFormat();
    });
    menu.addSeparator();
    QAction *fitColumn = menu.addAction(i18nc("@action:inmenu", "Fit Column to Contents"));
    const int column = m_details->header()->logicalIndexAt(pos);
    fitColumn->setEnabled(column >= 0);
    connect(fitColumn, &QAction::triggered, this, [this, column] {
        m_details->header()->setSectionResizeMode(column, QHeaderView::Interactive);
        m_details->resizeColumnToContents(column);
    });
    QAction *fitAll = menu.addAction(i18nc("@action:inmenu", "Fit All Columns to Contents"));
    connect(fitAll, &QAction::triggered, this, [this] {
        for (int section = 0; section < m_details->header()->count(); ++section) {
            if (!m_details->isColumnHidden(section)) {
                m_details->header()->setSectionResizeMode(section, QHeaderView::Interactive);
                m_details->resizeColumnToContents(section);
            }
        }
    });
    menu.addSeparator();
    QMenu *groups = menu.addMenu(i18nc("@menu", "Group By"));
    const QList<QPair<QString, QString>> options{
        {QStringLiteral("none"), i18nc("@option:check group files", "None")},
        {QStringLiteral("name"), i18nc("@option:check group files", "Name")},
        {QStringLiteral("type"), i18nc("@option:check group files", "Type")},
        {QStringLiteral("size"), i18nc("@option:check group files", "Size")},
        {QStringLiteral("date"), i18nc("@option:check group files", "Date")},
        {QStringLiteral("ext"), i18nc("@option:check group files", "Extension")},
    };
    auto *groupActions = new QActionGroup(groups);
    groupActions->setExclusive(true);
    for (const auto &[key, label] : options) {
        QAction *action = groups->addAction(label);
        action->setCheckable(true);
        action->setChecked(m_proxy->groupBy() == key);
        groupActions->addAction(action);
        connect(action, &QAction::triggered, this, [this, key] {
            m_proxy->setGroupBy(key);
            saveFolderFormat();
        });
    }
    menu.exec(m_details->header()->mapToGlobal(pos));
}

void FolderView::activateCurrentItem()
{
    const QModelIndex index = currentView()->currentIndex();
    if (index.isValid()) {
        onActivated(index);
    }
}

void FolderView::activateFirstItem()
{
    const NameFilter filter = m_proxy->nameFilter();
    for (int row = 0; row < m_proxy->rowCount(); ++row) {
        const QModelIndex index = m_proxy->index(row, 0);
        if (filter.appliesTo(m_model->itemForIndex(m_proxy->mapToSource(index)).isDir())) {
            onActivated(index);
            return;
        }
    }
    if (m_proxy->rowCount() > 0) {
        onActivated(m_proxy->index(0, 0));
    }
}

void FolderView::onActivated(const QModelIndex &index)
{
    const KFileItem item = m_model->itemForIndex(m_proxy->mapToSource(index));
    if (!item.isNull()) {
        Q_EMIT itemActivated(item);
    }
}

void FolderView::onContextMenu(const QPoint &pos)
{
    auto *view = qobject_cast<QAbstractItemView *>(sender());
    if (!view) {
        return;
    }
    if (!view->indexAt(pos).isValid()) {
        clearSelection();
    }
    Q_EMIT contextMenuRequested(view->viewport()->mapToGlobal(pos), selectedItems());
}

KFileItem FolderView::itemAt(QAbstractItemView *view, const QPoint &viewportPos) const
{
    return m_model->itemForIndex(m_proxy->mapToSource(view->indexAt(viewportPos)));
}

bool FolderView::handleDrop(QAbstractItemView *view, QEvent *event)
{
    auto *dropEvent = static_cast<QDropEvent *>(event);
    if (m_flat) {
        // Items in a flat list come from many folders, so a drop has no single destination.
        dropEvent->ignore();
        return true;
    }
    const QMimeData *mime = dropEvent->mimeData();
    const MailHandoff::DropKind kind = MailHandoff::classifyDrop(mime);
    if (kind == MailHandoff::DropKind::None) {
        return false;
    }

    const KFileItem target = itemAt(view, dropEvent->position().toPoint());
    const bool onFolder = !target.isNull() && target.isDir();

    if (event->type() != QEvent::Drop) {
        // KIO::DropJob decides between copy, move and link; accept everything.
        // A private mail reference is accepted too, so the drop can explain it.
        dropEvent->setDropAction(Qt::CopyAction);
        dropEvent->accept();
        return true;
    }

    if (kind == MailHandoff::DropKind::PrivateMailReference) {
        dropEvent->ignore();
        Q_EMIT dropRefused(i18nc("@info:status",
                                 "The mail client dragged a reference to a message or attachment on its mail server, not a file. Save the attachment "
                                 "from the mail client, then drop the saved file here."));
        return true;
    }

    QUrl destination = onFolder ? target.mostLocalUrl() : url();
    if (!destination.isValid()) {
        return false;
    }
    if (kind == MailHandoff::DropKind::Content) {
        // KIO saves the content as a new file after asking for its name.
        QMimeData content;
        MailHandoff::copyWithoutPrivateMailReferences(mime, &content);
        const QDropEvent stripped(dropEvent->position(), dropEvent->possibleActions(), &content, dropEvent->buttons(), dropEvent->modifiers());
        FileOperations::drop(&stripped, destination, window());
        dropEvent->acceptProposedAction();
        return true;
    }
    if (!onFolder) {
        // Dropping items onto their own folder is a no-op, not a copy.
        const QList<QUrl> urls = mime->urls();
        const bool allLocal = std::all_of(urls.cbegin(), urls.cend(), [&destination](const QUrl &source) {
            return source.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash) == destination.adjusted(QUrl::StripTrailingSlash);
        });
        if (allLocal) {
            dropEvent->ignore();
            return true;
        }
    }

    FileOperations::drop(dropEvent, destination, window());
    dropEvent->acceptProposedAction();
    return true;
}

bool FolderView::eventFilter(QObject *watched, QEvent *event)
{
    auto *view = qobject_cast<QAbstractItemView *>(watched);
    if (!view) {
        view = qobject_cast<QAbstractItemView *>(watched->parent());
    }
    if (!view) {
        return QWidget::eventFilter(watched, event);
    }
    const auto markerHit = [this, view](const QModelIndex &index, const QPoint &pos) {
        const QRect rect = view->visualRect(index);
        if (view == m_cards) {
            return CardDelegate::markerRect(CardDelegate::cardRect(rect, m_cardDelegate->itemSpace()), view->layoutDirection()).contains(pos);
        }
        return selectionMarkerRect(rect, view->layoutDirection()).contains(pos);
    };

    switch (event->type()) {
    case QEvent::Resize:
        if (watched == m_cards && Settings::cardColumns() > 0) {
            updateCardLayout();
        }
        break;
    case QEvent::DragEnter:
    case QEvent::DragMove:
    case QEvent::Drop:
        if (watched == view->viewport() && handleDrop(view, event)) {
            return true;
        }
        break;
    case QEvent::FocusIn:
        Q_EMIT activated();
        break;
    case QEvent::MouseButtonPress:
        if (watched == view->viewport()) {
            Q_EMIT activated();
            auto *mouse = static_cast<QMouseEvent *>(event);
            const QModelIndex index = view->indexAt(mouse->position().toPoint());
            const QModelIndex nameIndex = index.siblingAtColumn(0);
            if (mouse->button() == Qt::LeftButton && mouse->modifiers() == Qt::NoModifier && index.isValid()
                && (m_selectionMode || (m_selectionMarkers && markerHit(nameIndex, mouse->position().toPoint())))) {
                view->selectionModel()->setCurrentIndex(nameIndex, QItemSelectionModel::NoUpdate);
                view->selectionModel()->select(nameIndex, QItemSelectionModel::Toggle | QItemSelectionModel::Rows);
                m_markerPress = true;
                return true;
            }
        }
        break;
    case QEvent::MouseButtonRelease:
        if (watched == view->viewport() && m_markerPress && static_cast<QMouseEvent *>(event)->button() == Qt::LeftButton) {
            m_markerPress = false;
            return true;
        }
        break;
    case QEvent::MouseButtonDblClick:
        if (watched == view->viewport() && (m_selectionMode || m_selectionMarkers)) {
            auto *mouse = static_cast<QMouseEvent *>(event);
            const QModelIndex index = view->indexAt(mouse->position().toPoint()).siblingAtColumn(0);
            if (index.isValid() && (m_selectionMode || markerHit(index, mouse->position().toPoint()))) {
                m_markerPress = true;
                return true;
            }
        }
        break;
    case QEvent::KeyPress: {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (watched != view) {
            break;
        }
        const QString text = keyEvent->text();
        if (Settings::filterTypeAhead() && (keyEvent->modifiers() & ~Qt::ShiftModifier) == Qt::NoModifier && !text.isEmpty() && text.at(0).isPrint()
            && !text.trimmed().isEmpty()) {
            Q_EMIT typeAheadRequested(text);
            return true;
        }
        if (keyEvent->modifiers() != Qt::NoModifier) {
            break;
        }
        const bool rightToLeft = view->layoutDirection() == Qt::RightToLeft;
        if (view == m_details && keyEvent->key() == (rightToLeft ? Qt::Key_Right : Qt::Key_Left)) {
            Q_EMIT goUpRequested();
            return true;
        }
        if (view == m_details && keyEvent->key() == (rightToLeft ? Qt::Key_Left : Qt::Key_Right)) {
            onActivated(view->currentIndex());
            return true;
        }
        if (keyEvent->key() == Qt::Key_Escape) {
            clearSelection();
            return true;
        }
        break;
    }
    default:
        break;
    }
    return QWidget::eventFilter(watched, event);
}
