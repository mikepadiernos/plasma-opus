// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "metadataeditdialog.h"

#include "activitycontroller.h"
#include "activityreporting.h"
#include "adminmode.h"
#include "appearancedefaults.h"
#include "appearancestyle.h"
#include "appfonts.h"
#include "appidentity.h"
#include "applicationpackagedialog.h"
#include "apppackages.h"
#include "archives.h"
#include "archivewriter.h"
#include "batchrenamedialog.h"
#include "checksumdialog.h"
#include "cloudactivity.h"
#include "cloudactivitydialog.h"
#include "cloudsyncdialog.h"
#include "cloudsyncrunner.h"
#include "colorschemes.h"
#include "combinedpanel.h"
#include "commandpalettebar.h"
#include "compressdialog.h"
#include "contentchartdialog.h"
#include "customcommanddialog.h"
#include "customstylesheet.h"
#include "density.h"
#include "desktoptray.h"
#include "diskimagebackend.h"
#include "diskimages.h"
#include "doubleshift.h"
#include "dropdownwindow.h"
#include "duplicatefinderdialog.h"
#include "editorpanel.h"
#include "effectiveappearance.h"
#include "executables.h"
#include "favoritesmodel.h"
#include "filediffdialog.h"
#include "fileencryptiondialog.h"
#include "filemanagerrequest.h"
#include "fileoperations.h"
#include "filesplitdialog.h"
#include "filetypeplaces.h"
#include "foldercolors.h"
#include "foldercomparisondialog.h"
#include "foldergatedialog.h"
#include "foldergates.h"
#include "foldermetadata.h"
#include "foldersyncdialog.h"
#include "foldertree.h"
#include "folderview.h"
#include "gallerywindow.h"
#include "hiddenfolders.h"
#include "indexbackend.h"
#include "indexstatusdialog.h"
#include "informationpanel.h"
#include "kdeconnectclient.h"
#include "kdeconnectshare.h"
#include "kdeconnectsharedialog.h"
#include "keyboardhintoverlay.h"
#include "keyboardmapdialog.h"
#include "kwinintegration.h"

#include "labelcolor.h"
#include "lansharedialog.h"
#include "libraryautorefresh.h"
#include "libraryregistry.h"
#include "linkednavigation.h"
#include "localizeddates.h"
#include "mountbackend.h"
#include "mountmanagerdialog.h"
#include "nextcloudaccountsdialog.h"
#include "nextcloudassetsdialog.h"
#include "nextclouddesktop.h"
#include "nextcloudmenu.h"
#include "nextcloudphotosdialog.h"
#include "nextcloudsharesdialog.h"
#include "nextcloudtagsdialog.h"
#include "onlineaccountsmenu.h"
#include "packagetools.h"
#include "pane.h"
#include "pathfiltereditor.h"
#include "plasmavaultmenu.h"
#include "previewpanel.h"
#include "remotemountdialog.h"
#include "remotemounts.h"
#include "remoteprofilesdialog.h"
#include "remoteprofileseditor.h"
#include "searchdefaults.h"
#include "searchpanel.h"
#include "searchquery.h"
#include "searchtoolbaraction.h"
#include "sendbyemaildialog.h"
#include "session.h"
#include "settings.h"
#include "settingsdialog.h"
#include "shortcutguard.h"
#include "shortcuthintpopup.h"
#include "shortcutreference.h"
#include "startup.h"
#include "symbolicicon.h"
#include "tabcolors.h"
#include "tabmodel.h"
#include "tagassignment.h"
#include "taggingcontroller.h"
#include "taggingsettings.h"
#include "taggroupmanagerwidget.h"
#include "tagmanagerwidget.h"
#include "terminallauncher.h"
#include "terminalpanel.h"
#include "toolbarcustomizer.h"
#include "transferqueue.h"
#include "transferqueuedialog.h"

#include "themeprofiles.h"
#include "viewsettings.h"
#include "windoweffects.h"
#include "windowsequencehints.h"
#include "windowshortcuts.h"
#include "workspaceappearance.h"
#include "workspacedialog.h"
#include "workspacelocations.h"
#include "yazicolumns.h"
#include "yazikeys.h"

#include <KActionCollection>
#include <KActionMenu>
#include <KConfig>
#include <KConfigGroup>
#include <KDirModel>
#include <KEditToolBar>
#include <KFileCopyToMenu>
#include <KFileItemActions>
#include <KFileItemListProperties>
#include <KFilePlacesModel>
#include <KFilePlacesView>
#include <KFileUtils>
#include <KFormat>
#include <KGlobalAccel>
#include <KGuiItem>
#include <KHamburgerMenu>
#include <KIO/ApplicationLauncherJob>
#include <KIO/AuthInfo>
#include <KIO/CommandLauncherJob>
#include <KIO/CopyJob>
#include <KIO/DirectorySizeJob>
#include <KIO/DropJob>
#include <KIO/FileSystemFreeSpaceJob>
#include <KIO/FileUndoManager>
#include <KIO/Global>
#include <KIO/JobUiDelegateFactory>
#include <KIO/ListJob>
#include <KIO/MkdirJob>
#include <KIO/PreviewJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <KMessageWidget>
#include <KNewFileMenu>
#include <KPasswdServerClient>
#include <KPropertiesDialog>
#include <KProtocolInfo>
#include <KSelectAction>
#include <KService>
#include <KSharedConfig>
#include <KStandardAction>
#include <KTerminalLauncherJob>
#include <KTextEditor/Document>
#include <KToggleAction>
#include <KToolBar>
#include <KWindowEffects>
#include <KWindowSystem>
#include <KX11Extras>
#include <KXMLGUIFactory>
#include <Solid/Device>

#include <QAbstractItemView>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QCloseEvent>
#include <QComboBox>
#include <QCryptographicHash>
#include <QCursor>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDockWidget>
#include <QDomDocument>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QFormLayout>
#include <QFutureWatcher>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeDatabase>
#include <QPainter>
#include <QPointer>
#include <QPushButton>
#include <QScopeGuard>
#include <QScopedValueRollback>
#include <QScrollArea>
#include <QSet>
#include <QSignalBlocker>
#include <QSizeGrip>
#include <QSplitter>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStyleHints>
#include <QStyleOption>
#include <QTabWidget>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QtConcurrentRun>
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
#include <QAccessibilityHints>
#endif

#include <algorithm>

// The XMLGUI rc file is compiled into the static library; make sure the
// linker keeps it even though nothing else references the resource.
static void initResources()
{
    Q_INIT_RESOURCE(opusfm);
}

namespace
{
constexpr auto SessionGroup = "Session";
constexpr auto PanelsGroup = "Panels";
constexpr auto FlatGroup = "Flat";
constexpr auto PopularFoldersGroup = "PopularFolders";
constexpr auto SelectionFiltersGroup = "SelectionFilters";
constexpr auto WindowLayoutGroup = "WindowLayout";
constexpr auto SplitterSizesGroup = "SplitterSizes";

/// Whether folder comparison and synchronization can use @p url: a local
/// folder, or a folder on an SSH host through the rsync backend.
bool synchronizableFolder(const QUrl &url)
{
    const QString scheme = url.scheme().toLower();
    return url.isLocalFile() || scheme == QLatin1String("sftp") || scheme == QLatin1String("fish");
}

static_assert(int(WorkspaceCatalog::PreviewPlacement::Pane) == int(PreviewPanel::Placement::Pane));
static_assert(int(WorkspaceCatalog::PreviewPlacement::Below) == int(PreviewPanel::Placement::Below));
static_assert(int(WorkspaceCatalog::PreviewPlacement::Right) == int(PreviewPanel::Placement::Right));
static_assert(int(WorkspaceCatalog::EditorPlacement::Panel) == int(EditorPanel::Placement::Panel));
static_assert(int(WorkspaceCatalog::EditorPlacement::Below) == int(EditorPanel::Placement::Below));
static_assert(int(WorkspaceCatalog::EditorPlacement::Right) == int(EditorPanel::Placement::Right));
static_assert(int(WorkspaceCatalog::EditorPlacement::Maximized) == int(EditorPanel::Placement::Maximized));
static_assert(int(WorkspaceCatalog::EditorPlacement::Window) == int(EditorPanel::Placement::Window));

class PaddedStatusBar : public QStatusBar
{
public:
    explicit PaddedStatusBar(QWidget *parent)
        : QStatusBar(parent)
    {
        connect(this, &QStatusBar::messageChanged, this, qOverload<>(&QWidget::update));
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        if (currentMessage().isEmpty()) {
            QStatusBar::paintEvent(event);
            return;
        }

        QPainter painter(this);
        QStyleOption panel;
        panel.initFrom(this);
        style()->drawPrimitive(QStyle::PE_PanelStatusBar, &panel, &painter, this);
        QRect messageRect = rect().adjusted(8, 0, -8, 0);
        const auto children = findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly);
        for (QWidget *child : children) {
            if (!child->isVisible()) {
                continue;
            }
            if (layoutDirection() == Qt::RightToLeft) {
                messageRect.setLeft(qMax(messageRect.left(), child->geometry().right() + 3));
            } else {
                messageRect.setRight(qMin(messageRect.right(), child->geometry().left() - 3));
            }
            if (!qobject_cast<QSizeGrip *>(child)) {
                QStyleOption frame;
                frame.rect = child->geometry().adjusted(-2, -1, 2, 1);
                frame.palette = palette();
                frame.state = QStyle::State_None;
                style()->drawPrimitive(QStyle::PE_FrameStatusBarItem, &frame, &painter, child);
            }
        }
        if (messageRect.isValid()) {
            painter.setClipRect(messageRect);
            painter.setPen(palette().color(QPalette::WindowText));
            painter.drawText(messageRect, QStyle::visualAlignment(layoutDirection(), Qt::AlignLeft | Qt::AlignVCenter) | Qt::TextSingleLine, currentMessage());
        }
    }
};

void setDockTitleVisible(QDockWidget *dock, bool visible)
{
    if (!dock) {
        return;
    }
    QWidget *titleBar = dock->titleBarWidget();
    if (visible && titleBar) {
        dock->setTitleBarWidget(nullptr);
        delete titleBar;
    } else if (!visible && !titleBar) {
        titleBar = new QWidget(dock);
        titleBar->setFixedHeight(0);
        dock->setTitleBarWidget(titleBar);
    }
}

class MetadataDock : public QDockWidget
{
public:
    MetadataDock(MetadataEditDialog *dialog, QWidget *parent)
        : QDockWidget(i18nc("@title:window", "Edit Metadata"), parent)
        , m_dialog(dialog)
    {
    }

protected:
    void closeEvent(QCloseEvent *event) override
    {
        if (m_dialog) {
            m_dialog->reject();
            if (m_dialog && m_dialog->isBusy()) {
                event->ignore();
                return;
            }
        }
        event->accept();
    }

private:
    QPointer<MetadataEditDialog> m_dialog;
};

QString flatKey(const QUrl &url)
{
    return url.adjusted(QUrl::StripTrailingSlash).toString();
}

KFileItemList comparisonFiles(Pane *active, Pane *other, bool dual)
{
    KFileItemList items = active->view()->selectedItems();
    if (items.size() == 1 && dual && !other->isHidden()) {
        items += other->view()->selectedItems();
    }
    if (items.size() != 2 || !std::all_of(items.cbegin(), items.cend(), [](const KFileItem &item) {
            return item.url().isLocalFile() && item.isFile();
        })) {
        return {};
    }
    return items;
}

/// Maps a details or icons density setting, whose choices are the same.
template<typename Choices>
Density::Items itemDensity(typename Choices::type density)
{
    switch (density) {
    case Choices::Compact:
        return Density::Items::Compact;
    case Choices::VeryCompact:
        return Density::Items::VeryCompact;
    case Choices::Custom:
        return Density::Items::Custom;
    default:
        return Density::Items::Normal;
    }
}

Density::Bars barDensity(Settings::EnumBarDensity::type density)
{
    switch (density) {
    case Settings::EnumBarDensity::Compact:
        return Density::Bars::Compact;
    case Settings::EnumBarDensity::VeryCompact:
        return Density::Bars::VeryCompact;
    case Settings::EnumBarDensity::Custom:
        return Density::Bars::Custom;
    default:
        return Density::Bars::Normal;
    }
}

/**
 * Sets the margins and spacing of a bar's layout, remembering the style's
 * own on first use so -1 restores them.
 */
/**
 * Sets the margin and spacing of a bar's layout.
 * @param margin the margin, or -1 for the bar's own margins scaled by @p scale percent
 */
void setBarMargin(QWidget *bar, int margin, int scale)
{
    QLayout *layout = bar->layout();
    if (!layout) {
        return;
    }
    constexpr auto MarginsKey = "opusfmStyleMargins";
    constexpr auto SpacingKey = "opusfmStyleSpacing";
    if (!bar->property(MarginsKey).isValid()) {
        bar->setProperty(MarginsKey, QVariant::fromValue(layout->contentsMargins()));
        bar->setProperty(SpacingKey, layout->spacing());
    }
    if (margin < 0) {
        const auto scaled = [scale](int value) {
            return value > 0 ? qRound(value * scale / 100.0) : value;
        };
        const QMargins margins = bar->property(MarginsKey).value<QMargins>();
        layout->setContentsMargins(scaled(margins.left()), scaled(margins.top()), scaled(margins.right()), scaled(margins.bottom()));
        layout->setSpacing(scaled(bar->property(SpacingKey).toInt()));
    } else {
        layout->setContentsMargins(margin, margin, margin, margin);
        layout->setSpacing(margin);
    }
}

/// Places model whose icons use SymbolicIcon at small sizes.
class SymbolicPlacesModel : public KFilePlacesModel
{
public:
    using KFilePlacesModel::KFilePlacesModel;

    QVariant data(const QModelIndex &index, int role) const override
    {
        const QVariant value = KFilePlacesModel::data(index, role);
        if (role != Qt::DecorationRole || value.typeId() != QMetaType::QIcon) {
            return value;
        }
        const QString iconName = KFilePlacesModel::data(index, IconNameRole).toString();
        if (isDevice(index)) {
            return SymbolicIcon::wrap(value.value<QIcon>(), iconName, deviceForIndex(index).emblems());
        }
        const QIcon icon = SymbolicIcon::wrap(value.value<QIcon>(), iconName, {});
        const FolderColors::Style style = FolderColors::configured();
        if (style != FolderColors::Style::Icon && style != FolderColors::Style::Badge) {
            return icon;
        }
        return FolderColors::decorate(icon, iconName, {}, FolderColors::color(url(index)), style);
    }
};

QString stdName(KStandardAction::StandardAction id)
{
    return KStandardAction::name(id);
}

QString labelText(LabelColor color)
{
    switch (color) {
    case LabelColor::Red:
        return i18nc("@item:inmenu label colour", "Red");
    case LabelColor::Orange:
        return i18nc("@item:inmenu label colour", "Orange");
    case LabelColor::Yellow:
        return i18nc("@item:inmenu label colour", "Yellow");
    case LabelColor::Green:
        return i18nc("@item:inmenu label colour", "Green");
    case LabelColor::Blue:
        return i18nc("@item:inmenu label colour", "Blue");
    case LabelColor::Purple:
        return i18nc("@item:inmenu label colour", "Purple");
    case LabelColor::Pink:
        return i18nc("@item:inmenu label colour", "Pink");
    case LabelColor::Gray:
        return i18nc("@item:inmenu label colour", "Gray");
    case LabelColor::None:
        return i18nc("@item:inmenu label colour", "None");
    }
    return {};
}

/**
 * The canonical local folder that password protection actions apply to: the
 * single selected folder, the locked password-gated folder that hides the
 * shown folder, or the shown folder when nothing is selected.
 */
QString passwordFolder(Pane *pane)
{
    const KFileItemList selected = pane->view()->selectedItems();
    QUrl url;
    if (selected.size() == 1 && selected.first().isDir()) {
        url = selected.first().url();
    } else if (selected.isEmpty()) {
        if (const QString gate = FolderGates::instance()->lockedGateFor(pane->url()); !gate.isEmpty()) {
            return gate;
        }
        url = pane->url();
    }
    const QString path = FolderGates::localPath(url);
    return path.isEmpty() ? QString() : FolderGates::canonicalPath(path);
}

QList<QUrl> urlsOf(const KFileItemList &items)
{
    QList<QUrl> urls;
    urls.reserve(items.size());
    for (const KFileItem &item : items) {
        urls.append(item.url());
    }
    return urls;
}

QString previewMimeType(const KFileItem &item, bool determine = false)
{
    if (determine && item.url().isLocalFile()) {
        return item.mimetype();
    }
    return item.isMimeTypeKnown() ? item.currentMimeType().name() : QMimeDatabase().mimeTypeForFile(item.name(), QMimeDatabase::MatchExtension).name();
}
}

MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent)
    , m_placesModel(new SymbolicPlacesModel(this))
{
    initResources();
    if (!KWindowSystem::isPlatformX11() || KX11Extras::compositingActive()) {
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_NoSystemBackground, false);
    }
    KConfigGroup legacyView = KSharedConfig::openConfig()->group(QStringLiteral("View"));
    KConfigGroup views = KSharedConfig::openConfig()->group(QStringLiteral("Views"));
    KConfigGroup searchDefaults = KSharedConfig::openConfig()->group(QStringLiteral("Search"));
    const bool searchDefaultsMigrated = SearchDefaults::migrate(searchDefaults);
    if (searchDefaultsMigrated) {
        searchDefaults.sync();
    }
    const bool cloudSyncMigrated = CloudSync::migrateStartAtLogin(KSharedConfig::openConfig()->group(QStringLiteral("CloudSync")));
    if (ViewSettings::migrateColumns(legacyView, views) || searchDefaultsMigrated || cloudSyncMigrated) {
        Settings::self()->load();
    }
    setComponentName(AppIdentity::componentName(), AppIdentity::displayName());
    m_flatLocations = KSharedConfig::openStateConfig()->group(QLatin1String(FlatGroup)).readEntry("Locations", QStringList());
    m_favorites = new FavoritesModel(KSharedConfig::openConfig()->group(QStringLiteral("Favorites")), this);
    m_workspaceLocations = new WorkspaceLocations(m_favorites, m_placesModel, this);
    const auto config = KSharedConfig::openConfig();
    m_workspaceConfig = std::make_unique<KConfig>(config->name(), config->openFlags(), config->locationType());
    QString workspaceError;
    if (WorkspaceCatalog::load(m_workspaceConfig->group(QStringLiteral("Workspaces")), m_workspaceCatalog, workspaceError)) {
        m_currentWorkspaceId = m_workspaceCatalog.activeId;
    }
    // Layer the active workspace's appearance before the panes are created, so they start in its view mode.
    updateWorkspaceAppearanceLayer();
    m_tabColors = new TabColors(KSharedConfig::openConfig()->group(QStringLiteral("TabColors")), this);
    const KConfigGroup popular = KSharedConfig::openConfig()->group(QLatin1String(PopularFoldersGroup));
    const QStringList popularUrls = popular.readEntry("Urls", QStringList());
    const QList<int> popularVisits = popular.readEntry("Visits", QList<int>());
    for (qsizetype i = 0; i < popularUrls.size(); ++i) {
        const QUrl url(popularUrls.at(i), QUrl::StrictMode);
        if (url.isValid() && !url.isEmpty()) {
            m_popularFolderVisits.insert(url.adjusted(QUrl::StripTrailingSlash).toString(QUrl::FullyEncoded), qMax(0, popularVisits.value(i)));
        }
    }
    setupPanes();
    setupPlaces();
    setupPanels();
    setupStatusBar();
    setupActions();
    new ShortcutGuard(this);
    // Installed after the guard, so it sees the keys the guard keeps from shortcuts.
    m_windowSequenceHints = new WindowSequenceHints(this, [this] {
        return activePane();
    });
    m_shortcutReference = new ShortcutReference(this, [this] {
        return activePane();
    });
    m_keyboardHints = new KeyboardHintOverlay(this);
    m_desktopTray = new DesktopTray(this, actionCollection());
    connect(m_desktopTray, &DesktopTray::openRequested, this, [this](const QUrl &url) {
        openUrls({url});
    });
    connect(m_desktopTray, &DesktopTray::transfersRequested, this, &MainWindow::showTransferQueue);
    connect(m_desktopTray, &DesktopTray::cloudSyncRequested, this, &MainWindow::showCloudSync);
    m_cloudSync = new CloudSync::Scheduler(KSharedConfig::openConfig(), this);
    connect(m_cloudSync, &CloudSync::Scheduler::attentionNeeded, this, [this](const QString &id, const QString &message) {
        const auto pair = m_cloudSync->pair(id);
        const QString text = pair ? i18nc("@info:status %1 is a folder, %2 a status", "Cloud Sync of %1: %2", pair->localPath, message) : message;
        showStatusMessage(text);
        m_desktopTray->setAttention(text);
    });
    if (QCoreApplication *application = QCoreApplication::instance()) {
        // The main window is never deleted, so the running pair stops here,
        // keeping the changes it has made.
        connect(application, &QCoreApplication::aboutToQuit, m_cloudSync, &CloudSync::Scheduler::stop);
    }
    m_cloudSync->start();
    m_cloudActivity = new CloudActivity::Monitor(this);
    m_cloudActivity->setTransferQueue(FileOperations::transferQueue());
    m_cloudActivity->setScheduler(m_cloudSync);
    applyCloudLimits(CloudActivity::loadLimits(KSharedConfig::openConfig()->group(CloudActivity::limitsGroupName())));
    connect(m_desktopTray, &DesktopTray::cloudActivityRequested, this, &MainWindow::showCloudActivity);
    m_desktopTray->setQuitCheck([this] {
        return waitForCloudUploads(i18nc("@info", "Plasma Opus is still uploading files. Quitting now stops the uploads."), {}, true);
    });
    // Files written before an earlier session ended upload only when their remote is mounted again.
    if (const QStringList pending = CloudActivity::remotesWithPendingUploads(CloudActivity::rcloneCacheDir()); !pending.isEmpty()) {
        const QString text = i18ncp("@info:status %2 is a list of remotes",
                                    "Files written to %2 have not been uploaded yet. Mount the remote to upload them.",
                                    "Files written to %2 have not been uploaded yet. Mount the remotes to upload them.",
                                    pending.size(),
                                    QLocale().createSeparatedList(pending));
        showStatusMessage(text);
        m_cloudActivity->addEvent(i18nc("@info source of an event", "Cloud Storage"), text);
    }
    m_kwin = new KWinIntegration(std::make_unique<SystemKWinBackend>(), KSharedConfig::openConfig(), KSharedConfig::openStateConfig(), this);
    m_diskImages = std::make_unique<SystemDiskImageBackend>();
    m_packageTools = new SystemPackageTools(this);
    m_kwin->registerOnBus();
    m_kwin->setWorkspaceProvider([this] {
        return m_currentWorkspaceId;
    });
    const auto openOtherWorkspace = [this](const QString &id) {
        if (id != m_currentWorkspaceId) {
            openWorkspace(id);
        }
    };
    connect(m_kwin, &KWinIntegration::desktopWorkspaceRequested, this, openOtherWorkspace);
    connect(m_kwin, &KWinIntegration::edgeActivated, this, &MainWindow::toggleDropDown);
    m_dropDown = new DropDownWindow(this, m_kwin);
    connect(m_dropDown, &DropDownWindow::notice, this, [this](const QString &text) {
        statusBar()->showMessage(text, 5000);
    });
    connect(m_dropDown, &DropDownWindow::workspaceRequested, this, openOtherWorkspace);
    m_kwin->requestSync();
    m_activityController = new ActivityController(
        this,
        [this] {
            return m_workspaceCatalog;
        },
        [this] {
            return m_currentWorkspaceId;
        });
    connect(m_activityController, &ActivityController::applyRequested, this, &MainWindow::applyActivityPlan);
    connect(m_activityController, &ActivityController::statusMessage, this, [this](const QString &text) {
        statusBar()->showMessage(text, 5000);
    });
    {
        // The Activity offer sits above the panes, outside their layout.
        auto *central = new QWidget(this);
        central->setObjectName(QStringLiteral("centralArea"));
        auto *centralLayout = new QVBoxLayout(central);
        centralLayout->setContentsMargins(QMargins());
        centralLayout->setSpacing(0);
        QWidget *panes = takeCentralWidget();
        centralLayout->addWidget(m_activityController->messageWidget());
        centralLayout->addWidget(panes, 1);
        setCentralWidget(central);
    }
    m_activityController->setSource(new PlasmaActivitySource);
    applyWorkspaceSelectorPlacement();
    m_combinedPanel->setWorkspaceLocations(m_workspaceLocations);
    connect(m_workspaceLocations, &WorkspaceLocations::changed, this, [this] {
        QTimer::singleShot(0, this, &MainWindow::refreshWorkspaceLocations);
    });
    connect(m_placesView, &KFilePlacesView::allPlacesShownChanged, this, [this] {
        QTimer::singleShot(0, this, &MainWindow::refreshWorkspaceLocations);
    });
    const auto scheduleWorkspaceRefresh = [this] {
        QTimer::singleShot(0, this, &MainWindow::refreshWorkspaceLocations);
    };
    connect(m_placesModel, &QAbstractItemModel::rowsInserted, this, scheduleWorkspaceRefresh);
    connect(m_placesModel, &QAbstractItemModel::rowsRemoved, this, scheduleWorkspaceRefresh);
    connect(m_placesModel, &QAbstractItemModel::rowsMoved, this, scheduleWorkspaceRefresh);
    connect(m_placesModel, &QAbstractItemModel::modelReset, this, scheduleWorkspaceRefresh);
    connect(m_placesModel, &QAbstractItemModel::dataChanged, this, scheduleWorkspaceRefresh);
    reloadWorkspaceCatalog();
    watchWorkspaceCatalog();

    setStandardToolBarMenuEnabled(true);
    // Before the window loads its ui.rc file, so KXmlGui doesn't keep shortcuts that are now stored elsewhere.
    WindowShortcuts::migrate(WindowShortcuts::uiRcPath(componentName(), QStringLiteral("opus-file-managerui.rc")));
    setupGUI(QSize(1100, 700), Default, QStringLiteral("opus-file-managerui.rc"));
    WindowShortcuts::apply(this);
    // Rebuilding the menus and toolbars gives every action its default shortcuts again.
    connect(guiFactory(), &KXMLGUIFactory::makingChanges, this, [this](bool changing) {
        if (changing) {
            WindowShortcuts::prepareRebuild(this);
        } else {
            WindowShortcuts::apply(this);
        }
    });
    // Made before anything else follows shortcutsSaved, so it all sees the stored shortcuts.
    connect(guiFactory(), &KXMLGUIFactory::shortcutsSaved, this, [this] {
        static bool sharing = false;
        if (sharing) {
            // Another window stored its shortcuts, which this one already has.
            return;
        }
        WindowShortcuts::save(this);
        // Keys the user freed or took may change which extra navigation keys apply.
        WindowShortcuts::applyNavigationKeys(this);
        const QScopedValueRollback<bool> rollback(sharing, true);
        const QList<KMainWindow *> windows = KMainWindow::memberList();
        for (KMainWindow *member : windows) {
            auto *window = qobject_cast<MainWindow *>(member);
            if (window && window != this && window->guiFactory()) {
                WindowShortcuts::apply(window);
                Q_EMIT window->guiFactory()->shortcutsSaved();
            }
        }
    });
    m_toolbarCustomizer = new ToolbarCustomizer(toolBar(), actionCollection(), this, {m_left, m_right});
    connect(m_toolbarCustomizer, &ToolbarCustomizer::toolbarSettingsRequested, this, &MainWindow::showToolbarSettings);
    connect(guiFactory(), &KXMLGUIFactory::makingChanges, m_toolbarCustomizer, [this](bool changing) {
        if (changing) {
            m_toolbarCustomizer->prepareGuiRebuild();
        }
    });
    connect(actionCollection()->action(QStringLiteral("customize_main_toolbar")), &QAction::toggled, m_toolbarCustomizer, &ToolbarCustomizer::setEditing);
    connect(m_toolbarCustomizer,
            &ToolbarCustomizer::editingChanged,
            actionCollection()->action(QStringLiteral("customize_main_toolbar")),
            &QAction::setChecked);
    m_workspaceIndicatorTimer = new QTimer(this);
    m_workspaceIndicatorTimer->setSingleShot(true);
    m_workspaceIndicatorTimer->setInterval(250);
    connect(m_workspaceIndicatorTimer, &QTimer::timeout, this, &MainWindow::refreshWorkspaceSelector);
    const auto scheduleWorkspaceIndicator = [this] {
        scheduleWorkspaceSelectorRefresh();
    };
    for (Pane *pane : {m_left, m_right}) {
        connect(pane, &Pane::workspaceLayoutChanged, this, scheduleWorkspaceIndicator);
        connect(pane->tabs(), &TabModel::tabInserted, this, scheduleWorkspaceIndicator);
        connect(pane->tabs(), &TabModel::tabRemoved, this, scheduleWorkspaceIndicator);
        connect(pane->tabs(), &TabModel::tabMoved, this, scheduleWorkspaceIndicator);
        connect(pane->tabs(), &TabModel::currentIndexChanged, this, scheduleWorkspaceIndicator);
        connect(pane->tabs(), &TabModel::tabStateChanged, this, scheduleWorkspaceIndicator);
    }
    connect(m_splitter, &QSplitter::splitterMoved, this, scheduleWorkspaceIndicator);
    connect(m_workspaceSplitter, &QSplitter::splitterMoved, this, scheduleWorkspaceIndicator);
    connect(m_searchSplitter, &QSplitter::splitterMoved, this, scheduleWorkspaceIndicator);
    for (const QString &name : {QStringLiteral("dual_layout"),
                                QStringLiteral("stacked_layout"),
                                QStringLiteral("show_preview_panel"),
                                QStringLiteral("show_editor_panel"),
                                QStringLiteral("show_terminal_panel")}) {
        if (QAction *action = actionCollection()->action(name)) {
            connect(action, &QAction::toggled, this, scheduleWorkspaceIndicator);
        }
    }
    for (QDockWidget *dock : findChildren<QDockWidget *>(QString(), Qt::FindDirectChildrenOnly)) {
        connect(dock, &QDockWidget::visibilityChanged, this, scheduleWorkspaceIndicator);
        connect(dock, &QDockWidget::dockLocationChanged, this, scheduleWorkspaceIndicator);
        connect(dock, &QDockWidget::topLevelChanged, this, scheduleWorkspaceIndicator);
    }
    connect(m_toolbarCustomizer, &ToolbarCustomizer::editingChanged, this, scheduleWorkspaceIndicator);
    refreshWorkspaceSelector();
    const KConfigGroup panels = KSharedConfig::openStateConfig()->group(QLatin1String(PanelsGroup));
    setPanelLayout(panels.readEntry("Layout", QString()) == QLatin1String("Separate") ? PanelLayout::Separate : PanelLayout::Combined);
    applyPanelSettings();

    connect(KIO::FileUndoManager::self(), &KIO::FileUndoManager::undoAvailable, this, [this](bool available) {
        m_undoAction->setEnabled(available);
    });
    connect(KIO::FileUndoManager::self(), &KIO::FileUndoManager::undoTextChanged, this, [this](const QString &text) {
        m_undoAction->setText(text);
    });
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &MainWindow::updateActions);

    applyViewDefaults();
    setActivePane(m_left);
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    connect(QGuiApplication::styleHints()->accessibility(),
            &QAccessibilityHints::contrastPreferenceChanged,
            this,
            &MainWindow::applyContrastDependentAppearance);
#endif
    // An app-local scheme does not follow Plasma's palette, so Plasma's accent
    // is layered over it again when it changes.
    m_plasmaColorsWatcher = KConfigWatcher::create(KSharedConfig::openConfig(QStringLiteral("kdeglobals")));
    connect(m_plasmaColorsWatcher.data(), &KConfigWatcher::configChanged, this, [this](const KConfigGroup &group) {
        if ((group.name() == QLatin1String("General") || group.name().startsWith(QLatin1String("Colors:")))
            && EffectiveAppearance::accentColor() == QLatin1String(ColorSchemes::PlasmaAccent)
            && !ColorSchemes::path(EffectiveAppearance::colorScheme()).isEmpty()) {
            applyColorScheme();
        }
    });
    applySettings();
    updateActions();
    QTimer::singleShot(0, this, &MainWindow::mountRemoteProfilesAtStartup);
    if (Settings::restoreSearchPanel() && KSharedConfig::openStateConfig()->group(QStringLiteral("SearchPanel")).readEntry("Visible", false)) {
        // Deferred so the panel searches the folder opened after construction.
        QTimer::singleShot(0, this, [this] {
            const QScopedValueRollback<bool> restoring(m_restoringSearchPanel, true);
            m_searchPanelAction->setChecked(true);
        });
    }
}

namespace
{
/// Whether any option that high contrast can suspend differs from Plasma's appearance.
bool suspendableEffectsConfigured()
{
    return EffectiveAppearance::backgroundBlur() || EffectiveAppearance::windowOpacity() < 100 || EffectiveAppearance::paneOpacity() < 100
        || EffectiveAppearance::panelOpacity() < 100 || EffectiveAppearance::paneTint() != Settings::EnumPaneTint::None
        || EffectiveAppearance::cornerRadius() != Settings::EnumCornerRadius::FollowStyle
        || EffectiveAppearance::tabColorStrength() != Settings::EnumTabColorStrength::Text
        || (EffectiveAppearance::useCustomStyleSheet() && !EffectiveAppearance::customStyleSheet().trimmed().isEmpty());
}
}

void MainWindow::applyColorScheme()
{
    const QString scheme = EffectiveAppearance::colorScheme();
    const QStringList customColors = ColorSchemes::withAccent(scheme, EffectiveAppearance::accentColor(), EffectiveAppearance::customColors());
    const bool highContrast = ColorSchemes::highContrastPreferred();
    const bool customScheme = !scheme.isEmpty() || !customColors.isEmpty();
    const bool effectsSuspended = AppearanceDefaults::effectsSuspended() && suspendableEffectsConfigured();
    if (!ColorSchemes::apply(scheme, customColors, highContrast)) {
        statusBar()->showMessage(
            i18nc("@info:status", "Colour scheme %1 is not installed, so Plasma Opus follows the Plasma colour scheme until it is available.", scheme));
    } else if (highContrast && customScheme && effectsSuspended) {
        statusBar()->showMessage(i18nc("@info:status",
                                       "High contrast is requested, so Plasma Opus follows the Plasma colour scheme and suspends blur, "
                                       "translucency, tints, custom corners and the custom stylesheet."));
    } else if (highContrast && customScheme) {
        statusBar()->showMessage(
            i18nc("@info:status", "High contrast is requested, so Plasma Opus follows the Plasma colour scheme instead of its own colours."));
    } else if (effectsSuspended) {
        statusBar()->showMessage(
            i18nc("@info:status", "High contrast is requested, so Plasma Opus suspends blur, translucency, tints, custom corners and the custom stylesheet."));
    }
}

void MainWindow::applyContrastDependentAppearance()
{
    applyColorScheme();
    applyWindowEffects();
    applyBackgroundOpacity();
    applyAppearanceStyle();
    applyPaneBackgrounds();
    applyCustomStyleSheet();
}

void MainWindow::applyPaneBackgrounds()
{
    const bool suspended = AppearanceDefaults::effectsSuspended();
    const auto tint = suspended ? Pane::BackgroundTint::None : static_cast<Pane::BackgroundTint>(EffectiveAppearance::paneTint());
    for (Pane *pane : {m_left, m_right}) {
        pane->setBackgroundTint(tint, EffectiveAppearance::paneTintColor(), EffectiveAppearance::paneTintStrength());
        pane->setBackgroundOpacity(suspended ? 100 : EffectiveAppearance::paneOpacity());
    }
}

void MainWindow::resetAppearance()
{
    if (AppearanceDefaults::reset(Settings::self()).isEmpty()) {
        statusBar()->showMessage(i18nc("@info:status", "The appearance options already follow Plasma."));
        return;
    }
    Settings::self()->save();
    // An open settings dialog would otherwise offer to apply the old appearance again.
    if (auto *dialog = qobject_cast<SettingsDialog *>(KConfigDialog::exists(QLatin1String(SettingsDialog::Name)))) {
        dialog->resetAppearanceToDefaults();
    }
    applySettings();
    statusBar()->showMessage(i18nc("@info:status", "The appearance options were reset to follow Plasma."));
}

void MainWindow::applyWindowEffects()
{
    if (windowHandle()) {
        // Blur the whole window, so other translucent areas supplied by the
        // active style are included.
        const bool blur = EffectiveAppearance::backgroundBlur() && !AppearanceDefaults::effectsSuspended() && WindowEffects::isBlurAvailable(this);
        KWindowEffects::enableBlurBehind(windowHandle(), m_terminalBlur || blur);
    }
}

void MainWindow::showEvent(QShowEvent *event)
{
    KXmlGuiWindow::showEvent(event);
    applyWindowEffects();
    // The refresher queues libraries only once per process, after the first window appears.
    QTimer::singleShot(1000, LibraryAutoRefresh::instance(), &LibraryAutoRefresh::refreshAtStartup);
}

void MainWindow::applyBackgroundOpacity()
{
    // An opaque window cannot show what is behind it, so translucent colours
    // would only darken it. Only the Window role is resolved, so the window
    // keeps following the application palette for every other role.
    const ColorSchemes::Overrides overrides = WindowEffects::isTranslucent(this)
        ? ColorSchemes::translucent(QApplication::palette(this),
                                    {QPalette::Window},
                                    AppearanceDefaults::effectsSuspended() ? 100 : EffectiveAppearance::windowOpacity())
        : ColorSchemes::Overrides();
    setPalette(ColorSchemes::withOverrides(QPalette(), overrides));
    for (QDockWidget *dock : findChildren<QDockWidget *>(Qt::FindDirectChildrenOnly)) {
        applyPanelOpacity(dock);
    }
}

void MainWindow::applyPanelOpacity(QDockWidget *dock)
{
    if (!dock->property("opusPanelOpacity").toBool()) {
        dock->setProperty("opusPanelOpacity", true);
        connect(dock, &QDockWidget::topLevelChanged, this, [this, dock] {
            applyPanelOpacity(dock);
        });
    }
    // A floating panel is a separate window without a translucent background.
    const ColorSchemes::Overrides overrides = WindowEffects::isTranslucent(this) && !dock->isFloating()
        ? ColorSchemes::translucent(QApplication::palette(dock),
                                    {QPalette::Window, QPalette::Base, QPalette::AlternateBase},
                                    AppearanceDefaults::effectsSuspended() ? 100 : EffectiveAppearance::panelOpacity())
        : ColorSchemes::Overrides();
    dock->setPalette(ColorSchemes::withOverrides(QPalette(), overrides));
}

void MainWindow::changeEvent(QEvent *event)
{
    KXmlGuiWindow::changeEvent(event);
    // Recompute translucent colours from a new colour scheme. Setting an
    // unchanged palette sends no further event.
    if (event->type() == QEvent::PaletteChange) {
        applyBackgroundOpacity();
    } else if (event->type() == QEvent::StyleChange) {
        // A new Plasma widget style replaces the application style, including
        // the app-local style proxy; wrap the new style again once the style
        // change has reached every widget.
        QMetaObject::invokeMethod(this, &MainWindow::applyAppearanceStyle, Qt::QueuedConnection);
    }
}

void MainWindow::applyAppearanceStyle()
{
    AppearanceStyle::apply(AppearanceStyle::configured());
    setDockOptions(dockOptions().setFlag(QMainWindow::AnimatedDocks, AppearanceStyle::panelAnimationsEnabled(this)));
    // Tree lines are drawn by the views themselves.
    for (QTreeView *tree : findChildren<QTreeView *>()) {
        tree->viewport()->update();
    }
}

void MainWindow::applyCustomStyleSheet()
{
    QString error;
    const QString sheet = CustomStyleSheet::configured(&error);
    if (styleSheet() != sheet) {
        setStyleSheet(sheet);
    }
    if (!error.isEmpty()) {
        statusBar()->showMessage(i18nc("@info:status %1 is the reason", "The custom stylesheet was not applied. %1", error));
    }
}

void MainWindow::childEvent(QChildEvent *event)
{
    KXmlGuiWindow::childEvent(event);
    if (event->type() == QEvent::ChildPolished) {
        if (auto *dock = qobject_cast<QDockWidget *>(event->child())) {
            applyPanelOpacity(dock);
        }
    }
}

MainWindow::~MainWindow()
{
    delete findChild<WorkspaceDialog *>(QStringLiteral("workspaceDialog"));
    EditorPanel::setWorkspaceVault({});
    if (m_toolbarCustomizer) {
        m_toolbarCustomizer->setEditing(false);
    }
    // Children are destroyed after this window's members, and destroying one,
    // such as the places model's lister, can still make panes emit signals.
    // Stop those signals from reaching handlers that use destroyed members.
    const auto disconnectTree = [this](QObject *root) {
        if (!root) {
            return;
        }
        disconnect(root, nullptr, this, nullptr);
        const QList<QObject *> descendants = root->findChildren<QObject *>();
        for (QObject *descendant : descendants) {
            disconnect(descendant, nullptr, this, nullptr);
        }
    };
    const QList<QObject *> children = findChildren<QObject *>();
    for (QObject *child : children) {
        disconnect(child, nullptr, this, nullptr);
    }
    disconnectTree(m_left);
    disconnectTree(m_right);
}

void MainWindow::showWorkspaces()
{
    if (auto *existing = findChild<WorkspaceDialog *>(QStringLiteral("workspaceDialog"))) {
        existing->raise();
        existing->activateWindow();
        return;
    }
    reloadWorkspaceCatalog();
    auto *dialog = new WorkspaceDialog(
        m_workspaceConfig->group(QStringLiteral("Workspaces")),
        [this](WorkspaceCatalog::Layout &layout, QString &error) {
            return captureWorkspaceLayout(layout, error);
        },
        [this](const WorkspaceCatalog::Layout &layout, QString &error) {
            return applyWorkspaceLayout(layout, error);
        },
        m_currentWorkspaceId,
        this,
        m_workspaceLocations);
    connect(dialog, &WorkspaceDialog::catalogChanged, this, &MainWindow::reloadWorkspaceCatalog);
    dialog->setAvailabilityCheck([this](const WorkspaceCatalog::Layout &layout) {
        QStringList unavailable;
        QString error;
        workspaceLayoutAvailable(layout, unavailable, error);
        return unavailable;
    });
    connect(this, &MainWindow::workspaceCatalogChanged, dialog, &WorkspaceDialog::refreshCatalog);
    dialog->setAppearancePreview([this](const WorkspaceCatalog::Appearance *appearance) {
        previewWorkspaceAppearance(appearance);
    });
    connect(dialog, &WorkspaceDialog::currentWorkspaceChanged, this, [this](const QString &id) {
        if (!m_applyingActivity) {
            m_activityBehaviors.reset();
            m_activityBehaviorsWorkspaceId.clear();
        }
        m_currentWorkspaceId = id;
        refreshWorkspaceLocations();
        mountRemoteProfilesForWorkspace(id);
        const auto workspace = std::find_if(m_workspaceCatalog.workspaces.cbegin(), m_workspaceCatalog.workspaces.cend(), [&id](const auto &entry) {
            return entry.id == id;
        });
        if (workspace != m_workspaceCatalog.workspaces.cend() && workspaceBehaviors(*workspace).applies(workspaceBehaviors(*workspace).layout)) {
            m_kwin->restoreWorkspace(id);
        }
    });
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setModal(true);
    dialog->show();
}

void MainWindow::selectWorkspace(int index, bool confirmed, const std::optional<WorkspaceCatalog::BehaviorSelection> &behaviors)
{
    if (index < 0 || index >= m_workspaceCatalog.workspaces.size()) {
        return;
    }
    const QString id = m_workspaceCatalog.workspaces.at(index).id;
    showWorkspaces();
    auto *dialog = findChild<WorkspaceDialog *>(QStringLiteral("workspaceDialog"));
    if (!dialog) {
        return;
    }
    dialog->selectWorkspace(id);
    dialog->activateSelectedWorkspace(confirmed, behaviors);
    refreshWorkspaceSelector();
}

WorkspaceCatalog::BehaviorSelection MainWindow::workspaceBehaviors(const WorkspaceCatalog::Workspace &workspace) const
{
    return m_activityBehaviors && m_activityBehaviorsWorkspaceId == workspace.id ? *m_activityBehaviors : workspace.behaviorSelection;
}

ActivityController *MainWindow::activityController() const
{
    return m_activityController;
}

void MainWindow::showActivityAssociations()
{
    showSettings();
    if (auto *dialog = qobject_cast<SettingsDialog *>(KConfigDialog::exists(QLatin1String(SettingsDialog::Name)))) {
        dialog->showActivitiesPage();
    }
}

void MainWindow::applyActivityPlan(const ActivityAssociations::Plan &plan)
{
    if (!plan.workspaceId.isEmpty()) {
        reloadWorkspaceCatalog();
        const auto workspace = std::find_if(m_workspaceCatalog.workspaces.cbegin(), m_workspaceCatalog.workspaces.cend(), [&plan](const auto &entry) {
            return entry.id == plan.workspaceId;
        });
        if (workspace == m_workspaceCatalog.workspaces.cend()) {
            statusBar()->showMessage(i18nc("@info:status", "The associated workspace no longer exists."), 5000);
        } else {
            const auto previousBehaviors = m_activityBehaviors;
            const QString previousWorkspaceId = m_activityBehaviorsWorkspaceId;
            m_activityBehaviors = plan.behaviors;
            m_activityBehaviorsWorkspaceId = plan.workspaceId;
            m_applyingActivity = true;
            selectWorkspace(int(std::distance(m_workspaceCatalog.workspaces.cbegin(), workspace)), false, plan.behaviors);
            m_applyingActivity = false;
            if (m_currentWorkspaceId != plan.workspaceId) {
                // The switch was cancelled or failed; keep what applied before.
                m_activityBehaviors = previousBehaviors;
                m_activityBehaviorsWorkspaceId = previousWorkspaceId;
            }
            applyWorkspaceAppearance();
        }
    }
    if (plan.primaryFolder.isValid()) {
        m_left->openInNewTab(plan.primaryFolder, true);
    }
    if (plan.secondaryFolder.isValid()) {
        m_right->openInNewTab(plan.secondaryFolder, true);
    }
}

void MainWindow::openWorkspace(const QString &id)
{
    reloadWorkspaceCatalog();
    const auto workspace = std::find_if(m_workspaceCatalog.workspaces.cbegin(), m_workspaceCatalog.workspaces.cend(), [&id](const auto &entry) {
        return entry.id == id;
    });
    if (workspace == m_workspaceCatalog.workspaces.cend()) {
        statusBar()->showMessage(i18nc("@info:status", "The requested workspace no longer exists."), 5000);
        return;
    }
    selectWorkspace(int(std::distance(m_workspaceCatalog.workspaces.cbegin(), workspace)));
}

void MainWindow::restoreActiveWorkspaceLayout()
{
    const auto it = std::find_if(m_workspaceCatalog.workspaces.cbegin(), m_workspaceCatalog.workspaces.cend(), [this](const auto &workspace) {
        return workspace.id == m_currentWorkspaceId;
    });
    if (it == m_workspaceCatalog.workspaces.cend()) {
        return;
    }
    if (Settings::confirmWorkspaceLayoutRestore()
        && QMessageBox::question(this,
                                 i18nc("@title:window", "Restore Saved Workspace Layout"),
                                 i18nc("@info",
                                       "Return to the saved layout of %1? Changes to the layout, panes and tabs since it was saved will be "
                                       "replaced. Running file transfers continue.",
                                       it->name),
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::No)
            != QMessageBox::Yes) {
        return;
    }
    selectWorkspace(int(std::distance(m_workspaceCatalog.workspaces.cbegin(), it)), true);
}

void MainWindow::applyWorkspaceSelectorPlacement()
{
    const auto placement = Settings::workspaceSelectorPlacement();
    m_workspaceSelectorAction->setVisible(placement != Settings::EnumWorkspaceSelectorPlacement::StatusBar);
    m_workspaceStatusButton->setVisible(placement != Settings::EnumWorkspaceSelectorPlacement::ViewMenu);
}

void MainWindow::applyLocationPresentationDefaults()
{
    m_left->setDefaultLocationPresentation(static_cast<Pane::LocationPresentation>(Settings::leftLocationPresentation()));
    m_right->setDefaultLocationPresentation(static_cast<Pane::LocationPresentation>(Settings::rightLocationPresentation()));
}

void MainWindow::refreshWorkspaceSelector()
{
    if (!m_workspaceSelectorAction) {
        return;
    }
    QStringList items;
    int currentIndex = -1;
    const WorkspaceCatalog::Workspace *current = nullptr;
    for (int i = 0; i < m_workspaceCatalog.workspaces.size(); ++i) {
        const auto &workspace = m_workspaceCatalog.workspaces.at(i);
        items.append(i18nc("@item:inmenu workspace name and short label", "%1 (%2)", workspace.name, workspace.shortLabel));
        if (workspace.id == m_currentWorkspaceId) {
            currentIndex = i;
            current = &workspace;
        }
    }
    m_workspaceSelectorAction->setItems(items);
    m_workspaceSelectorAction->setEnabled(!items.isEmpty());
    bool modified = current && !current->layout;
    if (current && current->layout && Settings::showWorkspaceUnsavedIndicator()) {
        WorkspaceCatalog::Layout layout;
        QString error;
        modified = !captureWorkspaceLayout(layout, error) || layout != *current->layout;
    }
    modified = modified && Settings::showWorkspaceUnsavedIndicator();
    if (current) {
        m_workspaceSelectorAction->setText(i18nc("@action active workspace", "Workspace: %1%2", current->name, modified ? QStringLiteral(" *") : QString()));
        m_workspaceSelectorAction->setCurrentItem(currentIndex);
    } else {
        m_workspaceSelectorAction->setText(i18nc("@action no active workspace", "Workspace: None"));
    }
    if (m_workspaceStatusButton) {
        m_workspaceStatusButton->setText(m_workspaceSelectorAction->text());
        m_workspaceStatusButton->setToolTip(modified ? i18nc("@info:tooltip", "The current layout, panes or tabs differ from the workspace's saved layout.")
                                                     : QString());
    }
    if (QAction *restore = actionCollection()->action(QStringLiteral("restore_workspace_layout"))) {
        restore->setEnabled(current && current->layout.has_value());
    }
}

void MainWindow::scheduleWorkspaceSelectorRefresh()
{
    if (m_workspaceIndicatorTimer) {
        m_workspaceIndicatorTimer->start();
    }
}

void MainWindow::reloadWorkspaceCatalog()
{
    if (m_workspaceConfig->isDirty()) {
        statusBar()->showMessage(i18nc("@info", "Workspace changes cannot be refreshed while configuration changes are pending."));
        return;
    }
    m_workspaceConfig->reparseConfiguration();
    WorkspaceCatalog::Catalog catalog;
    QString error;
    if (!WorkspaceCatalog::load(m_workspaceConfig->group(QStringLiteral("Workspaces")), catalog, error)) {
        statusBar()->showMessage(error);
        return;
    }
    m_workspaceCatalog = catalog;
    m_currentWorkspaceId = catalog.activeId;
    refreshWorkspaceLocations();
    Q_EMIT workspaceCatalogChanged();
}

void MainWindow::updateWorkspaceAppearanceLayer()
{
    const auto workspace = std::find_if(m_workspaceCatalog.workspaces.cbegin(), m_workspaceCatalog.workspaces.cend(), [this](const auto &entry) {
        return entry.id == m_currentWorkspaceId;
    });
    const WorkspaceCatalog::Workspace *current = workspace != m_workspaceCatalog.workspaces.cend() ? &*workspace : nullptr;
    m_appearanceWorkspaceId = m_currentWorkspaceId;
    m_workspaceAppearance = current ? current->appearance : WorkspaceCatalog::Appearance{};
    std::optional<WorkspaceCatalog::Workspace> effective;
    if (current) {
        effective = *current;
        effective->behaviorSelection = workspaceBehaviors(*current);
    }
    m_workspaceAppearanceApplies = effective && effective->behaviorSelection.applies(effective->behaviorSelection.appearance);
    const QList<ThemeProfiles::Profile> profiles =
        ThemeProfiles::load(Settings::self(), KSharedConfig::openConfig()->group(QLatin1String(ThemeProfiles::Group)));
    const WorkspaceAppearance::Resolved resolved = WorkspaceAppearance::resolve(Settings::self(), effective ? &*effective : nullptr, profiles);
    EffectiveAppearance::setLayer(resolved.values);
    if (resolved.problems != m_workspaceAppearanceProblems) {
        m_workspaceAppearanceProblems = resolved.problems;
        if (current && !resolved.problems.isEmpty()) {
            statusBar()->showMessage(i18nc("@info:status %1 is a workspace name, %2 a list of themes, fonts or options",
                                           "Some appearance attached to %1 is unavailable and was skipped: %2. It applies again once available.",
                                           current->name,
                                           resolved.problems.join(i18nc("@item list separator", ", "))));
        }
    }
}

void MainWindow::applyWorkspaceAppearance()
{
    const auto workspace = std::find_if(m_workspaceCatalog.workspaces.cbegin(), m_workspaceCatalog.workspaces.cend(), [this](const auto &entry) {
        return entry.id == m_currentWorkspaceId;
    });
    const bool found = workspace != m_workspaceCatalog.workspaces.cend();
    const bool switched = m_appearanceWorkspaceId != m_currentWorkspaceId;
    if (!switched && (found ? workspace->appearance : WorkspaceCatalog::Appearance{}) == m_workspaceAppearance
        && (found && workspaceBehaviors(*workspace).applies(workspaceBehaviors(*workspace).appearance)) == m_workspaceAppearanceApplies) {
        return;
    }
    updateWorkspaceAppearanceLayer();
    applyAppearanceLayer();
    const QVariantMap &layer = EffectiveAppearance::layer();
    // The workspace's view mode, such as Card view, replaces the restored one when switching to it.
    if (switched && layer.contains(QStringLiteral("DefaultViewMode"))) {
        const auto mode = static_cast<FolderView::ViewMode>(qBound(0, int(EffectiveAppearance::defaultViewMode()), int(FolderView::ViewMode::Cards)));
        for (Pane *pane : {m_left, m_right}) {
            pane->view()->setViewMode(mode);
        }
    }
}

void MainWindow::applyAppearanceLayer()
{
    const QVariantMap layer = EffectiveAppearance::layer();
    if (layer == m_appliedAppearanceLayer) {
        return;
    }
    m_appliedAppearanceLayer = layer;
    applyAppearance();
    applyPaneBackgrounds();
    for (Pane *pane : {m_left, m_right}) {
        pane->reloadTabSettings();
        pane->view()->reloadListingSettings();
    }
    applyBarMargins();
}

void MainWindow::previewWorkspaceAppearance(const WorkspaceCatalog::Appearance *appearance)
{
    if (appearance) {
        const QList<ThemeProfiles::Profile> profiles =
            ThemeProfiles::load(Settings::self(), KSharedConfig::openConfig()->group(QLatin1String(ThemeProfiles::Group)));
        EffectiveAppearance::setLayer(WorkspaceAppearance::resolve(Settings::self(), *appearance, profiles).values);
    } else {
        updateWorkspaceAppearanceLayer();
    }
    applyAppearanceLayer();
}

void MainWindow::refreshWorkspaceLocations()
{
    applyWorkspaceAppearance();
    m_combinedPanel->setWorkspaceState(m_workspaceCatalog, m_currentWorkspaceId);
    const auto dialogs = findChildren<SearchWidget *>();
    for (SearchWidget *dialog : dialogs) {
        dialog->refreshWorkspaceScope();
    }
    for (int row = 0; row < m_placesModel->rowCount(); ++row) {
        const QModelIndex index = m_placesModel->index(row, 0);
        if (!m_workspaceLocations->isSavedPlace(index)) {
            continue;
        }
        const bool nativeVisible = m_placesView->allPlacesShown() || !m_placesModel->isHidden(index);
        const bool workspaceVisible = m_workspaceLocations->isVisible(m_placesModel->url(index), m_workspaceCatalog, m_currentWorkspaceId);
        m_placesView->setRowHidden(row, !nativeVisible || !workspaceVisible);
    }
    EditorPanel::setWorkspaceVaultState(workspaceVault());
    scheduleWorkspaceSelectorRefresh();
}

EditorPanel::WorkspaceVault MainWindow::workspaceVault() const
{
    const auto workspace = std::find_if(m_workspaceCatalog.workspaces.cbegin(), m_workspaceCatalog.workspaces.cend(), [this](const auto &entry) {
        return entry.id == m_currentWorkspaceId;
    });
    if (workspace == m_workspaceCatalog.workspaces.cend() || workspace->vaultLocationId.isEmpty() || !workspace->vaultEnabled) {
        return {};
    }
    EditorPanel::WorkspaceVault vault;
    vault.root = m_workspaceLocations->vaultFolder(workspace->vaultLocationId, &vault.unavailable);
    vault.options = workspace->vaultOptions;
    return vault;
}

void MainWindow::watchWorkspaceCatalog()
{
    m_workspaceConfigPath = m_workspaceConfig->name();
    if (QDir::isRelativePath(m_workspaceConfigPath)) {
        m_workspaceConfigPath = QDir(QStandardPaths::writableLocation(m_workspaceConfig->locationType())).filePath(m_workspaceConfigPath);
    }
    const QString directory = QFileInfo(m_workspaceConfigPath).absolutePath();
    m_workspaceWatcher = new QFileSystemWatcher(this);
    if (!QDir().mkpath(directory) || !m_workspaceWatcher->addPath(directory)) {
        statusBar()->showMessage(i18nc("@info", "The workspace configuration directory cannot be monitored for external changes."));
        return;
    }
    if (QFileInfo::exists(m_workspaceConfigPath)) {
        m_workspaceWatcher->addPath(m_workspaceConfigPath);
    }
    m_workspaceConfigModified = QFileInfo(m_workspaceConfigPath).lastModified().toMSecsSinceEpoch();
    const auto changed = [this] {
        QTimer::singleShot(0, this, [this] {
            if (QFileInfo::exists(m_workspaceConfigPath) && !m_workspaceWatcher->files().contains(m_workspaceConfigPath)) {
                if (!m_workspaceWatcher->addPath(m_workspaceConfigPath)) {
                    statusBar()->showMessage(i18nc("@info", "The workspace configuration cannot be monitored for external changes."));
                }
            }
            const QFileInfo configFile(m_workspaceConfigPath);
            if (!configFile.exists() || configFile.lastModified().toMSecsSinceEpoch() == m_workspaceConfigModified) {
                return;
            }
            m_workspaceConfigModified = configFile.lastModified().toMSecsSinceEpoch();
            QString error;
            if (!m_favorites->reload(error)) {
                statusBar()->showMessage(error);
            }
            reloadWorkspaceCatalog();
        });
    };
    connect(m_workspaceWatcher, &QFileSystemWatcher::fileChanged, this, changed);
    connect(m_workspaceWatcher, &QFileSystemWatcher::directoryChanged, this, changed);
}

bool MainWindow::captureWorkspaceLayout(WorkspaceCatalog::Layout &layout, QString &error) const
{
    if (actionCollection()->action(QStringLiteral("customize_main_toolbar"))->isChecked() || findChild<KEditToolBar *>()) {
        error = i18nc("@info", "Finish customizing the toolbar before saving or switching workspace layouts.");
        return false;
    }
    if (m_terminalPanelAction->isChecked() && !m_terminalPanel->hasSessions()) {
        error = i18nc("@info", "The terminal has no available session. Hide it or start a session before saving or switching workspace layouts.");
        return false;
    }
    for (const auto &dock : m_metadataDocks) {
        if (dock) {
            error = i18nc("@info", "Close the metadata editors before saving or switching workspace layouts.");
            return false;
        }
    }
    WorkspaceCatalog::Presentation presentation;
    presentation.panePlacement = m_rightWindow ? WorkspaceCatalog::PanePlacement::Window
        : m_secondPaneInTab                    ? WorkspaceCatalog::PanePlacement::Tab
                                               : WorkspaceCatalog::PanePlacement::Split;
    presentation.panelLayout = m_panelLayout == PanelLayout::Combined ? WorkspaceCatalog::PanelLayout::Combined : WorkspaceCatalog::PanelLayout::Separate;
    presentation.previewPlacement = static_cast<WorkspaceCatalog::PreviewPlacement>(m_previewPlacement);
    presentation.editorPlacement = static_cast<WorkspaceCatalog::EditorPlacement>(m_editorPlacement);
    presentation.terminalPlacement =
        m_workspaceSplitter->orientation() == Qt::Horizontal ? WorkspaceCatalog::TerminalPlacement::Right : WorkspaceCatalog::TerminalPlacement::Below;
    if (m_rightWindow) {
        presentation.detachedPaneGeometry = m_rightWindow->geometry();
    }
    presentation.toolbarActions = m_toolbarCustomizer->toolbarConfiguration();
    presentation.paneToolbars.insert(QStringLiteral("Left"), {m_left->toolbarActions(), m_left->visibleToolbarActions(), m_left->hiddenToolbarActions()});
    presentation.paneToolbars.insert(QStringLiteral("Right"), {m_right->toolbarActions(), m_right->visibleToolbarActions(), m_right->hiddenToolbarActions()});
    presentation.previewVisible = m_previewPanelAction->isChecked();
    presentation.terminalVisible = m_terminalPanelAction->isChecked();
    presentation.editorPresent = m_editor;
    if (m_editor) {
        presentation.editorVisible = m_editorPlacement == EditorPanel::Placement::Panel                                ? !m_editor->isHidden()
            : m_editorPlacement == EditorPanel::Placement::Below || m_editorPlacement == EditorPanel::Placement::Right ? !m_editorDock->isHidden()
                                                                                                                       : !m_editorWindow->isHidden();
    }
    const auto recordNames = [&presentation](const auto &widgets) {
        for (const auto *widget : widgets) {
            const QString name = widget->objectName();
            if (name.isEmpty() || presentation.windowObjects.contains(name)) {
                return false;
            }
            presentation.windowObjects.append(name);
        }
        return true;
    };
    if (!recordNames(findChildren<QDockWidget *>(QString(), Qt::FindDirectChildrenOnly))
        || !recordNames(findChildren<QToolBar *>(QString(), Qt::FindDirectChildrenOnly))) {
        error = i18nc("@info", "The window contains unnamed or duplicate layout components and cannot be saved.");
        return false;
    }
    WindowSession session;
    session.left = m_left->saveState();
    session.right = m_right->saveState();
    session.activePane = activePane() == m_right ? 1 : 0;
    session.dual = m_dualLayoutAction->isChecked();
    session.stacked = isStackedLayout();
    layout = {saveState(), m_splitter->sizes(), m_workspaceSplitter->sizes(), session.dual, session.stacked, presentation, session};
    error.clear();
    return true;
}

bool MainWindow::applyWorkspaceLayout(const WorkspaceCatalog::Layout &layout, QString &error)
{
    WorkspaceCatalog::Layout previous;
    if (!captureWorkspaceLayout(previous, error)) {
        return false;
    }
    bool changed = false;
    if (restoreWorkspaceLayout(layout, error, changed)) {
        return true;
    }
    if (!changed) {
        return false;
    }
    QString rollbackError;
    if (!restoreWorkspaceLayout(previous, rollbackError, changed) || !rollbackError.isEmpty()) {
        error = i18nc("@info", "%1 The previous layout could not be restored: %2", error, rollbackError);
    }
    return false;
}

bool MainWindow::workspaceLayoutAvailable(const WorkspaceCatalog::Layout &layout, QStringList &unavailable, QString &error) const
{
    if (!layout.presentation || !layout.session || layout.windowState.isEmpty()) {
        error = i18nc("@info", "This workspace has an older or incomplete layout. Save a new layout before switching to it.");
        return false;
    }
    const auto &p = *layout.presentation;
    for (const PaneSession *pane : {&layout.session->left, &layout.session->right}) {
        for (const QUrl &url : pane->tabs) {
            if (url.isLocalFile() && !QFileInfo::exists(url.toLocalFile())) {
                unavailable.append(url.toDisplayString(QUrl::PreferLocalFile));
            } else if (!url.isLocalFile() && !KProtocolInfo::isKnownProtocol(url)) {
                unavailable.append(
                    i18nc("@item unavailable location provider and location", "%1 (no %2 provider)", url.toDisplayString(QUrl::RemovePassword), url.scheme()));
            }
        }
    }
    const bool editorEmbedded =
        p.editorVisible && p.editorPlacement != WorkspaceCatalog::EditorPlacement::Maximized && p.editorPlacement != WorkspaceCatalog::EditorPlacement::Window;
    if (m_linkedPanesAction->isChecked()
        && (!layout.dual || p.panePlacement != WorkspaceCatalog::PanePlacement::Split
            || (p.previewVisible && p.previewPlacement == WorkspaceCatalog::PreviewPlacement::Pane && !editorEmbedded && !p.terminalVisible))) {
        error = i18nc("@info", "Unlink pane navigation before applying this workspace layout.");
        return false;
    }
    if (p.terminalVisible && !m_terminalPanel->hasSessions()) {
        error = i18nc("@info", "This workspace requires a terminal session. Open the terminal and try again.");
        return false;
    }
    for (const QString &name : p.windowObjects) {
        if (name == QLatin1String("editorDock") && p.editorPresent) {
            continue;
        }
        if (!findChild<QDockWidget *>(name, Qt::FindDirectChildrenOnly) && !findChild<QToolBar *>(name, Qt::FindDirectChildrenOnly)) {
            unavailable.append(name);
        }
    }
    return true;
}

void MainWindow::restoreWorkspacePanePlacement(WorkspaceCatalog::PanePlacement placement)
{
    if (placement == WorkspaceCatalog::PanePlacement::Tab) {
        moveSecondPaneToTab();
    } else if (placement == WorkspaceCatalog::PanePlacement::Window) {
        popOutSecondPane();
    } else {
        moveSecondPaneToDual();
    }
}

void MainWindow::restoreWorkspaceEditor(const WorkspaceCatalog::Presentation &presentation)
{
    if (presentation.editorPresent && !m_editor) {
        setupEditor();
    }
    if (!m_editor) {
        return;
    }
    setEditorPlacement(presentation.editorPresent ? static_cast<EditorPanel::Placement>(presentation.editorPlacement) : EditorPanel::Placement::Below);
    if (!presentation.editorVisible) {
        m_editor->hide();
        m_editorDock->hide();
        m_editorWindow->hide();
    }
}

bool MainWindow::restoreWorkspaceLayout(const WorkspaceCatalog::Layout &layout, QString &error, bool &changed)
{
    changed = false;
    QStringList unavailable;
    if (!workspaceLayoutAvailable(layout, unavailable, error)) {
        return false;
    }
    const auto &p = *layout.presentation;
    Pane *const active = activePane();
    const auto rightViewMode = m_right->view()->viewMode();
    const QScopedValueRollback<bool> movingPreview(m_movingPreview, true);
    const QScopedValueRollback<bool> movingEditor(m_movingEditor, true);
    const QSignalBlocker terminalSignals(m_terminalPanelAction);
    const QSignalBlocker linkedSignals(m_linkedPanesAction);
    const bool linked = m_linkedPanesAction->isChecked();
    const auto restoreLinked = qScopeGuard([this, linked] {
        m_linkedPanesAction->setChecked(linked);
    });
    changed = true;
    m_linkedPanesAction->setChecked(false);
    m_terminalPanelAction->setChecked(false);
    restoreWorkspacePanePlacement(p.panePlacement);
    m_right->view()->setViewMode(rightViewMode);
    setActivePane(active);
    setStackedLayout(layout.stacked);
    m_workspaceSplitter->setOrientation(p.terminalPlacement == WorkspaceCatalog::TerminalPlacement::Right ? Qt::Horizontal : Qt::Vertical);
    updateSearchPlacement();
    setPanelLayout(p.panelLayout == WorkspaceCatalog::PanelLayout::Combined ? PanelLayout::Combined : PanelLayout::Separate);
    setPreviewPlacement(static_cast<PreviewPanel::Placement>(p.previewPlacement));
    restoreWorkspaceEditor(p);
    if (!restoreState(layout.windowState)) {
        error = i18nc("@info", "The saved workspace window layout could not be restored.");
        return false;
    }
    if (p.panePlacement == WorkspaceCatalog::PanePlacement::Window && m_rightWindow && !p.detachedPaneGeometry.isEmpty()) {
        m_rightWindow->setGeometry(p.detachedPaneGeometry);
    }
    if (p.toolbarActions) {
        m_toolbarCustomizer->setToolbarConfiguration(*p.toolbarActions);
    }
    if (const auto it = p.paneToolbars.constFind(QStringLiteral("Left")); it != p.paneToolbars.cend()) {
        m_left->setToolbarConfiguration(it->actions, it->visible, it->hidden);
    }
    if (const auto it = p.paneToolbars.constFind(QStringLiteral("Right")); it != p.paneToolbars.cend()) {
        m_right->setToolbarConfiguration(it->actions, it->visible, it->hidden);
    }
    m_previewPanelAction->setChecked(p.previewVisible);
    m_terminalPanelAction->setChecked(p.terminalVisible);
    m_terminalPanel->setVisible(p.terminalVisible);
    m_dualLayoutAction->setChecked(layout.dual);
    toggleLayout(layout.dual);
    QList<int> workspaceSizes = layout.workspaceSizes;
    if (workspaceSizes.size() == m_workspaceSplitter->count() - 1) {
        // Layouts saved before the search panel existed, or while it was placed apart from the terminal, lack its size; keep the current one.
        workspaceSizes.append(m_workspaceSplitter->sizes().constLast());
    } else if (workspaceSizes.size() == m_workspaceSplitter->count() + 1) {
        // The search panel was saved beside the terminal but is now placed apart from it.
        workspaceSizes.removeLast();
    }
    if (m_splitter->count() != layout.paneSizes.size() || m_workspaceSplitter->count() != workspaceSizes.size()) {
        error = i18nc("@info", "The saved workspace splitter arrangement is unavailable.");
        return false;
    }
    m_splitter->setSizes(layout.paneSizes);
    m_workspaceSplitter->setSizes(workspaceSizes);
    m_left->replaceState(layout.session->left);
    m_right->replaceState(layout.session->right);
    setActivePane(layout.session->activePane == 1 ? m_right : m_left);
    error = unavailable.isEmpty()
        ? QString()
        : i18nc("@info",
                "The following workspace components or locations could not be restored: %1. Make them available and switch again to retry.",
                unavailable.join(QStringLiteral(", ")));
    return true;
}

void MainWindow::configureToolbars()
{
    if (auto *editor = findChild<KEditToolBar *>()) {
        editor->show();
        editor->raise();
        return;
    }
    m_toolbarCustomizer->setEditing(false);
    QDomDocument document = domDocument().cloneNode(true).toDocument();
    if (!m_toolbarCustomizer->prepareKdeToolbarEditor(document)) {
        QMessageBox::warning(this,
                             i18nc("@title:window", "Cannot Configure Toolbar"),
                             i18nc("@info", "The current toolbar contains an item that cannot be represented in the toolbar editor."));
        return;
    }
    setDOMDocument(document);
    KXmlGuiWindow::configureToolbars();
    if (auto *editor = findChild<KEditToolBar *>()) {
        QPointer<QAction> customize = actionCollection()->action(QStringLiteral("customize_main_toolbar"));
        customize->setEnabled(false);
        connect(editor, &QObject::destroyed, m_toolbarCustomizer, [customizer = m_toolbarCustomizer, customize] {
            customizer->finishKdeToolbarEditor();
            if (customize) {
                customize->setEnabled(!Settings::lockToolbars());
            }
        });
    }
}

void MainWindow::saveNewToolbarConfig()
{
    KXmlGuiWindow::saveNewToolbarConfig();
    m_toolbarCustomizer->toolbarRebuilt(toolBar());
}

void MainWindow::setupPanes()
{
    m_workspaceSplitter = new QSplitter(Qt::Vertical, this);
    m_workspaceSplitter->setObjectName(QStringLiteral("workspaceSplitter"));
    m_workspaceSplitter->setChildrenCollapsible(false);
    m_splitter = new QSplitter(Qt::Horizontal, m_workspaceSplitter);
    m_splitter->setObjectName(QStringLiteral("paneSplitter"));
    m_splitter->setChildrenCollapsible(false);
    m_left = new Pane(m_placesModel, m_tabColors, m_splitter, QStringLiteral("Left"));
    m_right = new Pane(m_placesModel, m_tabColors, m_splitter, QStringLiteral("Right"));
    applyLocationPresentationDefaults();
    m_previewPanel = new PreviewPanel(m_splitter);
    m_previewPanel->setServerPreviewHandler([this](const KFileItem &item, const QSize &size) {
        return requestNextcloudPreview(item, size);
    });
    m_splitter->addWidget(m_left);
    m_splitter->addWidget(m_right);
    m_splitter->addWidget(m_previewPanel);
    m_previewPanel->hide();
    m_previewDock = new QDockWidget(i18nc("@title:window", "Preview"), this);
    m_previewDock->setObjectName(QStringLiteral("previewDock"));
    m_previewDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
    m_previewDock->setFeatures(QDockWidget::DockWidgetClosable);
    addDockWidget(Qt::BottomDockWidgetArea, m_previewDock);
    m_previewDock->hide();
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 1);
    m_terminalPanel = new TerminalPanel(m_workspaceSplitter);
    connect(m_terminalPanel, &TerminalPanel::blurRegionChanged, this, [this](const QRegion &region) {
        m_terminalBlur = !region.isEmpty();
        applyWindowEffects();
    });
    m_terminalPanel->hide();
    m_searchPanel = new SearchPanel(
        [this] {
            SearchWidget::WorkspaceScope scope;
            const auto workspace = std::find_if(m_workspaceCatalog.workspaces.cbegin(), m_workspaceCatalog.workspaces.cend(), [this](const auto &entry) {
                return entry.id == m_currentWorkspaceId;
            });
            scope.active =
                workspace != m_workspaceCatalog.workspaces.cend() && workspaceBehaviors(*workspace).applies(workspaceBehaviors(*workspace).searchScope);
            if (scope.active) {
                for (const auto &location : m_workspaceLocations->locations()) {
                    if (m_workspaceCatalog.locationAssignments.value(location.id) == m_currentWorkspaceId) {
                        scope.locations.append(location.url);
                    }
                }
            }
            return scope;
        },
        m_workspaceSplitter);
    m_searchPanel->hide();
    m_indexBackend = new SystemIndexBackend(this);
    m_searchPanel->searchWidget()->setIndexBackend(m_indexBackend);
    connect(m_searchPanel->searchWidget(), &SearchWidget::revealRequested, this, [this](const QUrl &url) {
        activePane()->revealUrl(url);
    });
    connect(m_searchPanel->searchWidget(), &SearchWidget::openRequested, this, [this](const QUrl &url) {
        if (QFileInfo(url.toLocalFile()).isDir()) {
            activePane()->setUrl(url);
        } else {
            FileOperations::open(KFileItem(url), this);
        }
    });
    m_workspaceSplitter->addWidget(m_splitter);
    m_workspaceSplitter->addWidget(m_terminalPanel);
    m_workspaceSplitter->addWidget(m_searchPanel);
    m_workspaceSplitter->setStretchFactor(0, 3);
    m_workspaceSplitter->setStretchFactor(1, 1);
    m_workspaceSplitter->setStretchFactor(2, 1);
    m_searchSplitter = new QSplitter(Qt::Vertical, this);
    m_searchSplitter->setObjectName(QStringLiteral("searchSplitter"));
    m_searchSplitter->setChildrenCollapsible(false);
    m_searchSplitter->addWidget(m_workspaceSplitter);
    m_searchSplitter->setStretchFactor(0, 3);
    m_paneTabs = new QTabWidget(this);
    m_paneTabs->setObjectName(QStringLiteral("paneLayoutTabs"));
    m_paneTabs->addTab(m_searchSplitter, QString());
    m_paneTabs->tabBar()->hide();
    setCentralWidget(m_paneTabs);
    connect(m_paneTabs, &QTabWidget::currentChanged, this, [this](int index) {
        if (m_movingSecondPane || index < 0) {
            return;
        }
        setActivePane(m_paneTabs->widget(index) == m_right ? m_right : m_left);
    });

    connectPane(m_left);
    connectPane(m_right);
}

void MainWindow::connectPane(Pane *pane)
{
    connect(pane, &Pane::activated, this, [this, pane] {
        setActivePane(pane);
    });
    connect(pane, &Pane::searchRequested, this, [this, pane] {
        showSearchForPane(pane);
    });
    connect(pane, &Pane::fileTypePlaceRequested, this, [this, pane](const QUrl &url, bool newTab) {
        if (pane != activePane()) {
            setActivePane(pane);
        }
        openFileTypePlace(pane, url, newTab);
    });
    connect(pane, &Pane::yaziModeChanged, this, [this, pane] {
        if (pane == activePane()) {
            updateActions();
        }
    });
    connect(pane, &Pane::yaziPreviewChanged, this, [this, pane] {
        if (pane == activePane()) {
            updateActions();
        }
    });
    connect(pane, &Pane::yaziPreviewContentChanged, this, [this, pane] {
        if (pane == activePane()) {
            updateActions();
        }
    });
    connect(pane, &Pane::yaziParentColumnChanged, this, [this, pane] {
        if (pane == activePane()) {
            updateActions();
        }
    });
    pane->yaziColumns()->setTransferQueue(FileOperations::transferQueue());
    connect(pane->yaziColumns(), &YaziColumns::transferQueueRequested, this, &MainWindow::showTransferQueue);
    connect(pane, &Pane::yaziFileCommandRequested, this, [this, pane](YaziMode::Command command) {
        runYaziFileCommand(pane, command);
    });
    connect(pane, &Pane::yaziSearchRequested, this, [this, pane](bool contents) {
        showYaziSearch(pane, contents);
    });
    connect(pane->yaziColumns(), &YaziColumns::searchCancelRequested, this, [this, pane] {
        SearchWidget *search = m_searchPanel->searchWidget();
        if (m_yaziSearchPane != pane || !search->isRunning()) {
            pane->yaziColumns()->clearSearchStatus();
            return;
        }
        search->cancel();
        reportYaziSearch(search->status(), true);
    });
    connect(pane, &Pane::urlChanged, this, [this, pane](const QUrl &url) {
        syncLinkedPane(pane, url);
        recordFolderVisit(pane, url);
        pane->view()->setFlat(m_flatLocations.contains(flatKey(url)));
        const bool clearedHoveredInformation = m_hoveredInformationPane == pane;
        if (clearedHoveredInformation) {
            m_hoveredInformationPane = nullptr;
            m_hoveredInformationItem = {};
        }
        if (pane == m_active) {
            if (m_terminalPanelAction && m_terminalPanelAction->isChecked() && Settings::terminalFollowActiveFolder()) {
                m_terminalPanel->follow(url);
            }
            if (m_previewPanelAction && m_previewPanelAction->isChecked()) {
                m_previewPanel->clear();
            }
            updateActions();
            updateStatusBar();
            updateFreeSpace();
            updateWindowTitle();
            updateInformationPanel();
            syncFolderTrees();
            LibraryAutoRefresh::instance()->folderOpened(url);
        } else if (m_active) {
            if (Settings::statusBarLayout() == Settings::EnumStatusBarLayout::PerPane) {
                updateStatusBar();
                updateFreeSpace(pane);
            }
            updateActions();
        }
        if (clearedHoveredInformation && pane != m_active) {
            updateInformationPanel();
        }
    });
    connect(pane, &Pane::tabCountChanged, this, &MainWindow::updateTabBars);
    connect(pane, &Pane::statusMessage, this, &MainWindow::showStatusMessage);
    connect(pane->view(), &FolderView::listingReady, this, [this, pane] {
        fetchNextcloudPermissions(pane);
    });
    connect(pane->view(), &FolderView::itemActivated, this, [this, pane](const KFileItem &item) {
        activatePaneItem(pane, item);
    });
    connect(pane->view(), &FolderView::contextMenuRequested, this, [this, pane](const QPoint &pos, const KFileItemList &items) {
        setActivePane(pane);
        showContextMenu(pane, pos, items);
    });
    connect(pane->view(), &FolderView::selectedItemsChanged, this, [this, pane] {
        updateStatusBar();
        if (pane == m_active) {
            updatePreviewPanel();
            updateInformationPanel();
        }
        if (m_active) {
            updateActions();
        }
    });
    connect(pane->view(), &FolderView::selectionModeChanged, this, [this, pane] {
        if (pane == m_active) {
            updateActions();
        }
    });
    connect(pane->view(), &FolderView::copySelectionRequested, this, [this, pane] {
        setActivePane(pane);
        copy();
    });
    connect(pane->view(), &FolderView::cutSelectionRequested, this, [this, pane] {
        setActivePane(pane);
        cut();
    });
    connect(pane->view(), &FolderView::trashSelectionRequested, this, [this, pane] {
        setActivePane(pane);
        trash();
    });
    connect(pane->view(), &FolderView::itemCountChanged, this, [this, pane] {
        updateStatusBar();
        if (pane == m_active) {
            updatePreviewPanel();
            updateInformationPanel();
        }
    });
    connect(pane->view(), &FolderView::itemHovered, this, [this, pane](const KFileItem &item) {
        if (Settings::informationHover()) {
            m_hoveredInformationPane = pane;
            m_hoveredInformationItem = item;
            updateInformationPanel();
        }
    });
    connect(pane->view(), &FolderView::itemHoverCleared, this, [this, pane] {
        if (m_hoveredInformationPane == pane) {
            m_hoveredInformationPane = nullptr;
            m_hoveredInformationItem = {};
            updateInformationPanel();
        }
    });
    connect(pane->view(), &FolderView::itemsOrderChanged, this, [this, pane] {
        if (pane == m_active) {
            updatePreviewPanel();
        }
    });
    connect(pane->view(), &FolderView::loadingChanged, this, [this, pane](bool loading) {
        if (pane == m_active && !loading) {
            updateFreeSpace();
        }
    });
}

void MainWindow::setupPlaces()
{
    addFileTypePlaces();
    m_placesView = new KFilePlacesView(this);
    m_placesView->setModel(m_placesModel);
    m_placesView->setDropOnPlaceEnabled(true);
    m_placesView->setAutoResizeItemsEnabled(false);

    m_placesDock = new QDockWidget(i18nc("@title:window", "Places"), this);
    m_placesDock->setObjectName(QStringLiteral("placesDock"));
    m_placesDock->setWidget(m_placesView);
    m_placesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_placesDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
    addDockWidget(Qt::LeftDockWidgetArea, m_placesDock);

    connect(m_placesView, &KFilePlacesView::urlChanged, this, [this](const QUrl &url) {
        activePane()->setUrl(url);
        if (Settings::autoFocusPaneAfterPanelNavigation()) {
            activePane()->view()->currentView()->setFocus();
        }
    });
    connect(m_placesView, &KFilePlacesView::tabRequested, this, [this](const QUrl &url) {
        activePane()->openInNewTab(url);
    });
    connect(m_placesView, &KFilePlacesView::urlsDropped, this, [this](const QUrl &dest, QDropEvent *event, QWidget *) {
        if (FileTypePlaces::isPlaceUrl(dest)) {
            event->ignore();
            showStatusMessage(i18nc("@info:status", "Files can't be dropped on a file-type search."));
            return;
        }
        FileOperations::drop(event, dest, this);
    });
}

void MainWindow::setupPanels()
{
    const auto addPanelDock = [this](const QString &objectName, const QString &title, QWidget *widget) {
        auto *dock = new QDockWidget(title, this);
        dock->setObjectName(objectName);
        dock->setWidget(widget);
        dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
        addDockWidget(Qt::LeftDockWidgetArea, dock);
        return dock;
    };
    const auto navigate = [this](const QUrl &url) {
        activePane()->setUrl(url);
        if (Settings::autoFocusPaneAfterPanelNavigation()) {
            activePane()->view()->currentView()->setFocus();
        }
    };
    const auto openInTab = [this](const QUrl &url) {
        activePane()->openInNewTab(url);
    };
    const KConfigGroup panels = KSharedConfig::openStateConfig()->group(QLatin1String(PanelsGroup));

    m_folderTree = new FolderTree({QUrl::fromLocalFile(QDir::homePath()), QUrl::fromLocalFile(QStringLiteral("/"))});
    m_folderTree->setExpandedUrls(panels.readEntry("FoldersExpanded", QList<QUrl>()));
    connect(m_folderTree, &FolderTree::urlActivated, this, navigate);
    connect(m_folderTree, &FolderTree::tabRequested, this, openInTab);
    connect(m_folderTree, &FolderTree::contextMenuRequested, this, &MainWindow::showFolderTreeMenu);
    connect(m_folderTree, &FolderTree::urlsDropped, this, [this](const QUrl &destination, QDropEvent *event) {
        FileOperations::drop(event, destination, this);
    });
    auto *treeScroll = new QScrollArea;
    treeScroll->setWidgetResizable(true);
    treeScroll->setWidget(m_folderTree);
    m_foldersDock = addPanelDock(QStringLiteral("foldersDock"), i18nc("@title:window", "Folders"), treeScroll);
    splitDockWidget(m_placesDock, m_foldersDock, Qt::Vertical);

    m_combinedPanel = new CombinedPanel(m_placesModel, m_favorites);
    m_combinedPanel->folderTree()->setExpandedUrls(panels.readEntry("CombinedExpanded", QList<QUrl>()));
    connect(m_combinedPanel, &CombinedPanel::urlActivated, this, navigate);
    connect(m_combinedPanel, &CombinedPanel::tabRequested, this, openInTab);
    connect(m_combinedPanel, &CombinedPanel::folderContextMenuRequested, this, &MainWindow::showFolderTreeMenu);
    m_combinedPanel->setCollapsedSections(panels.readEntry(
        "CollapsedSections",
        QStringList{QStringLiteral("places:PlacesType"), QStringLiteral("places:DevicesType"), QStringLiteral("places:RemovableDevicesType")}));
    connect(m_combinedPanel, &CombinedPanel::urlsDropped, this, [this](const QUrl &destination, QDropEvent *event) {
        if (FileTypePlaces::isPlaceUrl(destination)) {
            event->ignore();
            showStatusMessage(i18nc("@info:status", "Files can't be dropped on a file-type search."));
            return;
        }
        FileOperations::drop(event, destination, this);
    });
    m_combinedDock = addPanelDock(QStringLiteral("combinedDock"), i18nc("@title:window", "Explorer"), m_combinedPanel);

    m_informationPanel = new InformationPanel;
    m_informationPanel->setFields(Settings::informationFields());
    m_informationDock = addPanelDock(QStringLiteral("informationDock"), i18nc("@title:window", "Information"), m_informationPanel);
    addDockWidget(Qt::RightDockWidgetArea, m_informationDock);
    connect(m_informationPanel, &InformationPanel::placementRequested, this, [this](Qt::DockWidgetArea area) {
        addDockWidget(area, m_informationDock);
    });
    connect(m_informationPanel, &InformationPanel::closeRequested, m_informationDock, &QDockWidget::hide);
    connect(m_informationDock, &QDockWidget::visibilityChanged, this, [this](bool visible) {
        // Server metadata is only read while the panel is shown.
        if (visible) {
            updateInformationPanel();
        }
    });
    m_informationDock->hide();

    m_tagPanel = new TagPanel;
    m_tagDock = new QDockWidget(i18nc("@title:window", "Tags"), this);
    m_tagDock->setObjectName(QStringLiteral("tagDock"));
    m_tagDock->setWidget(m_tagPanel);
    m_tagDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
    m_tagDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
    TaggingSettings::migrateTagPanelPlacement();
    const bool tagsBelow = Settings::tagPanelPlacement() == Settings::EnumTagPanelPlacement::Below;
    addDockWidget(tagsBelow ? Qt::BottomDockWidgetArea : Qt::RightDockWidgetArea, m_tagDock);
    m_tagPanel->setPlacement(tagsBelow ? TagPanel::Placement::Below : TagPanel::Placement::Right);
    m_tagDock->hide();
    connect(m_tagPanel, &TagPanel::placementRequested, this, &MainWindow::setTagPanelPlacement);
    connect(m_tagPanel, &TagPanel::closeRequested, m_tagDock, &QDockWidget::hide);
    connect(m_tagDock, &QDockWidget::dockLocationChanged, this, [this](Qt::DockWidgetArea area) {
        if (area == Qt::RightDockWidgetArea || area == Qt::BottomDockWidgetArea) {
            const bool below = area == Qt::BottomDockWidgetArea;
            m_tagPanel->setPlacement(below ? TagPanel::Placement::Below : TagPanel::Placement::Right);
            saveTagPanelPlacement(below);
        }
    });
    connect(m_tagDock, &QDockWidget::visibilityChanged, this, [this](bool visible) {
        if (visible) {
            updateTagPanel();
            m_tagPanel->reload();
        } else if (m_tagDock->isHidden() && m_tagDock->property("opusPanelShown").toBool()) {
            savePanelDockSize(m_tagDock, QStringLiteral("TagDock"));
        }
    });
    connect(m_tagPanel, &TagPanel::libraryChanged, this, [this](const QString &root) {
        TagAssignment::invalidate(root);
        updateInformationPanel();
    });
}

void MainWindow::setPanelLayout(PanelLayout layout)
{
    m_panelLayout = layout;
    const bool combined = layout == PanelLayout::Combined;
    applyPanelSettings();
    m_panelLayoutAction->setCurrentItem(combined ? 0 : 1);

    KConfigGroup panels = KSharedConfig::openStateConfig()->group(QLatin1String(PanelsGroup));
    panels.writeEntry("Layout", combined ? QStringLiteral("Combined") : QStringLiteral("Separate"));
    panels.sync();
}

MainWindow::PanelLayout MainWindow::panelLayout() const
{
    return m_panelLayout;
}

FavoritesModel *MainWindow::favorites() const
{
    return m_favorites;
}

TabColors *MainWindow::tabColors() const
{
    return m_tabColors;
}

void MainWindow::compress()
{
    const KFileItemList selected = activePane()->view()->selectedItems();
    QStringList sources;
    for (const KFileItem &item : selected) {
        if (!item.localPath().isEmpty()) {
            sources.append(item.localPath());
        }
    }
    if (sources.isEmpty()) {
        return;
    }
    const QString defaultName = selected.size() == 1 ? (selected.first().isDir() ? selected.first().name() : Archives::baseName(selected.first().name()))
                                                     : activePane()->url().adjusted(QUrl::StripTrailingSlash).fileName();
    CompressDialog dialog(defaultName,
                          inactivePane()->url().isLocalFile(),
                          this,
                          static_cast<ArchiveWriter::Format>(Settings::defaultArchiveFormat()),
                          Settings::archiveOtherPaneByDefault(),
                          Ark::isInstalled());
    dialog.setVolumeDefaults(Settings::archiveSplitByDefault(), Settings::archiveVolumeSizeMiB());
    dialog.setPasswordOffered(Settings::archivePromptForPassword());
    const int result = dialog.exec();
    if (result == CompressDialog::UseArk) {
        launchArk(Ark::compressArguments(sources));
        return;
    }
    if (result != QDialog::Accepted) {
        return;
    }

    const QUrl folder = dialog.toOtherPane() ? inactivePane()->url() : activePane()->url();
    if (!folder.isLocalFile()) {
        showStatusMessage(i18nc("@info:status", "Archives can only be created in local folders."));
        return;
    }
    const QDir directory(folder.toLocalFile());
    const qint64 volumeSize = dialog.volumeSize();
    // A split archive must not collide with existing volumes either.
    const auto taken = [&directory, volumeSize](const QString &name) {
        return QFileInfo::exists(directory.filePath(name)) || (volumeSize > 0 && QFileInfo::exists(directory.filePath(ArchiveWriter::firstVolumeName(name))));
    };
    QString fileName = dialog.archiveName() + u'.' + ArchiveWriter::suffix(dialog.format());
    while (taken(fileName)) {
        fileName = KFileUtils::suggestName(folder, fileName);
    }
    const QString archivePath = directory.filePath(fileName);

    auto *watcher = new QFutureWatcher<ArchiveWriter::Volumes>(this);
    connect(watcher, &QFutureWatcher<ArchiveWriter::Volumes>::finished, this, [this, watcher, fileName] {
        const ArchiveWriter::Volumes written = watcher->result();
        if (!written.error.isEmpty()) {
            showStatusMessage(i18nc("@info:status", "Could not create %1: %2", fileName, written.error));
        } else if (written.files.size() > 1) {
            showStatusMessage(i18ncp("@info:status",
                                     "Created %2 in %1 volume, starting with %3",
                                     "Created %2 in %1 volumes, starting with %3",
                                     written.files.size(),
                                     fileName,
                                     QFileInfo(written.files.first()).fileName()));
        } else {
            showStatusMessage(i18nc("@info:status", "Created %1", QFileInfo(written.files.value(0)).fileName()));
        }
        watcher->deleteLater();
    });
    const QString password = dialog.password();
    const ArchiveWriter::Format format = dialog.format();
    watcher->setFuture(QtConcurrent::run([archivePath, format, sources, password, volumeSize]() -> ArchiveWriter::Volumes {
        const QString error =
            password.isEmpty() ? ArchiveWriter::write(archivePath, format, sources) : ArchiveWriter::writeEncryptedZip(archivePath, format, sources, password);
        if (!error.isEmpty()) {
            return {{}, error};
        }
        if (volumeSize > 0 && QFileInfo(archivePath).size() > volumeSize) {
            return ArchiveWriter::splitIntoVolumes(archivePath, volumeSize);
        }
        return {{archivePath}, {}};
    }));
}

void MainWindow::setSelectionLabel(const QString &label)
{
    const KFileItemList items = activePane()->view()->selectedItems();
    for (const KFileItem &item : items) {
        const QFileInfo info(item.localPath());
        QString error;
        if (!FolderMetadata::setLabel(info.absolutePath(), info.fileName(), label, &error)) {
            showStatusMessage(i18nc("@info:status", "Could not set the label: %1", error));
            return;
        }
    }
    activePane()->view()->reloadMetadata();
}

void MainWindow::setSelectionColor(const QString &color)
{
    const KFileItemList items = activePane()->view()->selectedItems();
    for (const KFileItem &item : items) {
        const QFileInfo info(item.localPath());
        QString error;
        if (!FolderMetadata::setColor(info.absolutePath(), info.fileName(), color, &error)) {
            showStatusMessage(i18nc("@info:status", "Could not set the file colour: %1", error));
            return;
        }
    }
    activePane()->view()->reloadMetadata();
}

void MainWindow::setSelectionRating(int rating)
{
    const KFileItemList items = activePane()->view()->selectedItems();
    for (const KFileItem &item : items) {
        const QFileInfo info(item.localPath());
        QString error;
        if (!FolderMetadata::setRating(info.absolutePath(), info.fileName(), rating, &error)) {
            showStatusMessage(i18nc("@info:status", "Could not set the rating: %1", error));
            return;
        }
    }
    activePane()->view()->reloadMetadata();
}

void MainWindow::setSelectionNote()
{
    const KFileItemList items = activePane()->view()->selectedItems();
    if (items.isEmpty()) {
        return;
    }
    QString currentNote;
    if (items.size() == 1) {
        const QFileInfo info(items.first().localPath());
        QString error;
        const QJsonObject metadata = FolderMetadata::read(info.absolutePath(), &error);
        if (!error.isEmpty()) {
            showStatusMessage(i18nc("@info:status", "Could not read the file note: %1", error));
            return;
        }
        currentNote = metadata.value(QStringLiteral("notes")).toObject().value(info.fileName()).toString();
    }
    bool accepted = false;
    const QString note = QInputDialog::getMultiLineText(this,
                                                        i18nc("@title:window", "Edit Note"),
                                                        i18nc("@label:textbox", "Note (up to 500 characters):"),
                                                        currentNote,
                                                        &accepted);
    if (!accepted) {
        return;
    }
    if (note.size() > 500) {
        showStatusMessage(i18nc("@info:status", "Notes can be at most 500 characters."));
        return;
    }
    for (const KFileItem &item : items) {
        const QFileInfo info(item.localPath());
        QString error;
        if (!FolderMetadata::setNote(info.absolutePath(), info.fileName(), note, &error)) {
            showStatusMessage(i18nc("@info:status", "Could not set the note: %1", error));
            return;
        }
    }
    activePane()->view()->reloadMetadata();
}

void MainWindow::toggleSelectionPin()
{
    const KFileItemList items = activePane()->view()->selectedItems();
    for (const KFileItem &item : items) {
        const QFileInfo info(item.localPath());
        QString error;
        if (!FolderMetadata::togglePin(info.absolutePath(), info.fileName(), &error)) {
            showStatusMessage(i18nc("@info:status", "Could not change the pin: %1", error));
            return;
        }
    }
    activePane()->view()->reloadMetadata();
}

void MainWindow::editFolderLook(const QUrl &folder)
{
    if (!folder.isLocalFile()) {
        return;
    }
    QDialog dialog(this);
    dialog.setWindowTitle(i18nc("@title:window", "Folder Look"));
    auto *icon = new QComboBox(&dialog);
    const QList<QPair<QString, QString>> icons{
        {i18nc("@item:inlistbox folder icon", "Default"), QString()},
        {i18nc("@item:inlistbox folder icon", "Folder"), QStringLiteral("folder")},
        {i18nc("@item:inlistbox folder icon", "Home"), QStringLiteral("folder-home")},
        {i18nc("@item:inlistbox folder icon", "Documents"), QStringLiteral("folder-documents")},
        {i18nc("@item:inlistbox folder icon", "Downloads"), QStringLiteral("folder-download")},
        {i18nc("@item:inlistbox folder icon", "Pictures"), QStringLiteral("folder-pictures")},
        {i18nc("@item:inlistbox folder icon", "Music"), QStringLiteral("folder-music")},
        {i18nc("@item:inlistbox folder icon", "Videos"), QStringLiteral("folder-videos")},
    };
    for (const auto &[label, name] : icons) {
        icon->addItem(label, name);
    }
    const QString directoryFile = QDir(folder.toLocalFile()).filePath(QStringLiteral(".directory"));
    KConfig directoryConfig(directoryFile, KConfig::SimpleConfig);
    const QString existingIcon = directoryConfig.group(QStringLiteral("Desktop Entry")).readEntry("Icon", QString());
    const int iconIndex = icon->findData(existingIcon);
    if (iconIndex >= 0) {
        icon->setCurrentIndex(iconIndex);
    } else if (!existingIcon.isEmpty()) {
        icon->addItem(existingIcon, existingIcon);
        icon->setCurrentIndex(icon->count() - 1);
    }

    auto *color = new QComboBox(&dialog);
    color->addItem(i18nc("@item:inlistbox folder colour", "Default"), QString());
    for (LabelColor value : LabelColors::all()) {
        color->addItem(labelText(value), LabelColors::rgb(value).name());
    }
    QString metadataError;
    const QJsonObject metadata = FolderMetadata::read(folder.toLocalFile(), &metadataError);
    if (!metadataError.isEmpty()) {
        showStatusMessage(i18nc("@info:status", "Could not read the folder appearance: %1", metadataError));
        return;
    }
    const QString existingColor = metadata.value(QStringLiteral("color")).toString();
    const int colorIndex = color->findData(existingColor);
    if (colorIndex >= 0) {
        color->setCurrentIndex(colorIndex);
    }
    const bool wasHidden = FolderMetadata::hidden(metadata);
    auto *hidden = new QCheckBox(i18nc("@option:check", "Hide in parent folder"), &dialog);
    hidden->setObjectName(QStringLiteral("folderLookHidden"));
    hidden->setToolTip(i18nc("@info:tooltip", "List this folder in its parent folder only while hidden files are shown"));
    hidden->setChecked(wasHidden);

    auto *form = new QFormLayout(&dialog);
    form->addRow(i18nc("@label:listbox", "Icon:"), icon);
    form->addRow(i18nc("@label:listbox", "Colour:"), color);
    form->addRow(QString(), hidden);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    if (!directoryConfig.isConfigWritable(false)) {
        showStatusMessage(i18nc("@info:status", "The folder's .directory file is not writable."));
        return;
    }

    QString error;
    if (!FolderMetadata::setFolderColor(folder.toLocalFile(), color->currentData().toString(), &error)) {
        showStatusMessage(i18nc("@info:status", "Could not set the folder colour: %1", error));
        return;
    }
    refreshFolderColors();
    if (hidden->isChecked() != wasHidden && !HiddenFolders::instance()->setHidden(folder.toLocalFile(), hidden->isChecked(), &error)) {
        showStatusMessage(i18nc("@info:status", "Could not change whether the folder is hidden: %1", error));
        return;
    }
    KConfigGroup desktopEntry = directoryConfig.group(QStringLiteral("Desktop Entry"));
    const QString iconName = icon->currentData().toString();
    if (iconName.isEmpty()) {
        desktopEntry.deleteEntry("Icon");
    } else {
        desktopEntry.writeEntry("Icon", iconName);
    }
    if (!directoryConfig.sync()) {
        showStatusMessage(i18nc("@info:status", "Could not save the folder icon."));
        return;
    }
    const QUrl parent = KIO::upUrl(folder);
    for (Pane *pane : {m_left, m_right}) {
        if (pane->url() == parent || pane->url() == folder) {
            pane->view()->reload();
        }
    }
    m_folderTree->reload();
    m_combinedPanel->folderTree()->reload();
    m_folderTree->reveal(folder);
    m_combinedPanel->folderTree()->reveal(folder);
    if (hidden->isChecked() && !wasHidden && !activePane()->view()->showHiddenFiles()) {
        showStatusMessage(i18nc("@info:status", "The folder is listed in its parent folder only while hidden files are shown."));
    }
}

void MainWindow::extractToOtherPane()
{
    const QUrl destination = inactivePane()->url();
    const KFileItemList selected = activePane()->view()->selectedItems();
    for (const KFileItem &item : selected) {
        const QUrl archive = Archives::browseUrl(item.mostLocalUrl(), item.mimetype());
        const bool arkOnly = !archive.isValid() && isArkOnlyArchive(item);
        if (!archive.isValid() && !arkOnly) {
            continue;
        }
        if (arkOnly && !destination.isLocalFile()) {
            showStatusMessage(i18nc("@info:status archive extraction", "Ark can only extract %1 into a local folder.", item.name()));
            continue;
        }
        QString name = Archives::baseName(item.name());
        if (destination.isLocalFile() && QFileInfo::exists(QDir(destination.toLocalFile()).filePath(name))) {
            name = KFileUtils::suggestName(destination, name);
        }
        QUrl target = destination;
        target.setPath(QDir(destination.path()).filePath(name));
        if (!arkOnly) {
            extractArchive(archive, target);
            continue;
        }
        if (!Ark::isInstalled()) {
            launchArk({});
            return;
        }
        if (!QDir().mkdir(target.toLocalFile())) {
            showStatusMessage(i18nc("@info:status archive extraction", "Could not create the folder %1.", target.toLocalFile()));
            continue;
        }
        launchArk(Ark::extractArguments(item.localPath(), target.toLocalFile()));
    }
}

void MainWindow::extractProtectedToOtherPane()
{
    const QUrl destination = inactivePane()->url();
    if (!destination.isLocalFile()) {
        showStatusMessage(i18nc("@info:status archive extraction", "Password-protected archives require a local destination."));
        return;
    }
    for (const KFileItem &item : activePane()->view()->selectedItems()) {
        if (item.localPath().isEmpty() || Archives::browseUrl(item.mostLocalUrl(), item.mimetype()).isEmpty()) {
            continue;
        }
        bool accepted = false;
        const QString password = QInputDialog::getText(this,
                                                       i18nc("@title:window", "Archive Password"),
                                                       i18nc("@label:textbox", "Password for %1:", item.name()),
                                                       QLineEdit::Password,
                                                       QString(),
                                                       &accepted);
        if (!accepted) {
            continue;
        }
        if (password.isEmpty()) {
            showStatusMessage(i18nc("@info:status archive extraction", "An archive password is required."));
            continue;
        }
        QString name = Archives::baseName(item.name());
        if (QFileInfo::exists(QDir(destination.toLocalFile()).filePath(name))) {
            name = KFileUtils::suggestName(destination, name);
        }
        const QString target = QDir(destination.toLocalFile()).filePath(name);
        const QString archivePath = item.localPath();
        auto *watcher = new QFutureWatcher<QString>(this);
        connect(watcher, &QFutureWatcher<QString>::finished, this, [this, watcher, target] {
            const QString error = watcher->result();
            showStatusMessage(error.isEmpty()
                                  ? i18nc("@info:status archive extraction", "Extracted to %1", target)
                                  : i18nc("@info:status archive extraction", "Extraction failed: %1. A partial folder may remain at %2.", error, target));
            watcher->deleteLater();
        });
        watcher->setFuture(QtConcurrent::run([archivePath, target, password] {
            return Archives::extractProtected(archivePath, target, password);
        }));
    }
}

void MainWindow::extractArchive(const QUrl &archive, const QUrl &target)
{
    auto *list = KIO::listDir(archive, KIO::HideProgressInfo);
    auto entries = std::make_shared<QList<QUrl>>();
    connect(list, &KIO::ListJob::entries, this, [archive, entries](KIO::Job *, const KIO::UDSEntryList &listed) {
        for (const KIO::UDSEntry &entry : listed) {
            const QString name = entry.stringValue(KIO::UDSEntry::UDS_NAME);
            if (name != QLatin1String(".") && name != QLatin1String("..")) {
                QUrl child = archive;
                child.setPath(archive.path() + name);
                entries->append(child);
            }
        }
    });
    connect(list, &KJob::result, this, [this, target, entries](KJob *job) {
        if (job->error()) {
            showStatusMessage(job->errorString());
            return;
        }
        KIO::MkdirJob *mkdir = KIO::mkdir(target);
        KJobWidgets::setWindow(mkdir, this);
        connect(mkdir, &KJob::result, this, [this, target, entries](KJob *made) {
            if (made->error()) {
                showStatusMessage(made->errorString());
                return;
            }
            KIO::CopyJob *copy = KIO::copy(*entries, target);
            KIO::FileUndoManager::self()->recordCopyJob(copy);
            KJobWidgets::setWindow(copy, this);
        });
    });
}

void MainWindow::openInArk()
{
    const KFileItemList selected = activePane()->view()->selectedItems();
    if (selected.size() != 1 || selected.first().localPath().isEmpty()) {
        showStatusMessage(i18nc("@info:status", "Select one local archive to open in Ark."));
        return;
    }
    const KFileItem &item = selected.first();
    if (Ark::isInstalled() && !arkBackends().isEmpty() && !arkBackends().canRead(item.mimetype())) {
        showStatusMessage(i18nc("@info:status", "None of Ark's installed backends can open %1.", item.name()));
        return;
    }
    launchArk(Ark::openArguments(item.localPath()));
    const Ark::FormatSupport support = arkBackends().support(item.mimetype());
    if (Ark::isInstalled() && support.read && !support.write) {
        showStatusMessage(i18nc("@info:status",
                                "Ark can only extract %1 with the installed tools; install the format's full tools to add, rename or remove entries.",
                                item.name()));
    }
}

void MainWindow::addToArchive()
{
    const KFileItemList selected = activePane()->view()->selectedItems();
    if (selected.isEmpty() || std::any_of(selected.cbegin(), selected.cend(), [](const KFileItem &item) {
            return item.localPath().isEmpty();
        })) {
        showStatusMessage(i18nc("@info:status", "Select local files or folders to add to an archive."));
        return;
    }
    if (!Ark::isInstalled()) {
        launchArk({});
        return;
    }
    const QStringList writable = arkBackends().writableMimeTypes();
    if (writable.isEmpty()) {
        showStatusMessage(i18nc("@info:status", "None of Ark's installed backends can add files to archives."));
        return;
    }
    QFileDialog dialog(this, i18nc("@title:window", "Add to Archive"));
    if (activePane()->url().isLocalFile()) {
        dialog.setDirectory(activePane()->url().toLocalFile());
    }
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setMimeTypeFilters(writable);
    if (dialog.exec() != QDialog::Accepted || dialog.selectedFiles().isEmpty()) {
        return;
    }
    addSelectionToArchive(dialog.selectedFiles().constFirst());
}

bool MainWindow::addSelectionToArchive(const QString &archivePath)
{
    const KFileItemList selected = activePane()->view()->selectedItems();
    QStringList sources;
    for (const KFileItem &item : selected) {
        if (!item.localPath().isEmpty()) {
            sources.append(item.localPath());
        }
    }
    if (sources.isEmpty() || sources.size() != selected.size()) {
        showStatusMessage(i18nc("@info:status", "Select local files or folders to add to an archive."));
        return false;
    }
    const QFileInfo archive(archivePath);
    if (!archive.isFile()) {
        showStatusMessage(i18nc("@info:status", "%1 is not an archive file.", archivePath));
        return false;
    }
    const QString canonicalArchive = archive.canonicalFilePath();
    for (const QString &source : sources) {
        const QString canonicalSource = QFileInfo(source).canonicalFilePath();
        if (canonicalSource == canonicalArchive || canonicalArchive.startsWith(canonicalSource + u'/')) {
            showStatusMessage(i18nc("@info:status", "%1 cannot be added to itself.", archive.fileName()));
            return false;
        }
    }
    if (!Ark::isInstalled()) {
        launchArk({});
        return false;
    }
    const QMimeType mimeType = QMimeDatabase().mimeTypeForFile(archivePath);
    const Ark::FormatSupport support = arkBackends().support(mimeType.name());
    if (!support.write) {
        showStatusMessage(support.read ? i18nc("@info:status", "Ark can only extract %1 archives with the installed tools.", mimeType.comment())
                                       : i18nc("@info:status", "None of Ark's installed backends can add files to %1.", archive.fileName()));
        return false;
    }
    const auto answer =
        KMessageBox::warningContinueCancel(this,
                                           i18ncp("@info",
                                                  "Add %1 item to %2? Ark changes the archive in place, and an entry with the same name is replaced.",
                                                  "Add %1 items to %2? Ark changes the archive in place, and entries with the same names are replaced.",
                                                  sources.size(),
                                                  archive.fileName()),
                                           i18nc("@title:window", "Add to Archive"),
                                           KGuiItem(i18nc("@action:button", "Add"), QStringLiteral("archive-insert")));
    if (answer != KMessageBox::Continue) {
        return false;
    }
    launchArk(Ark::addToArguments(archivePath, sources));
    return true;
}

const Ark::Backends &MainWindow::arkBackends()
{
    if (!m_arkBackends) {
        m_arkBackends = Ark::Backends::installed();
    }
    return *m_arkBackends;
}

void MainWindow::setArkBackends(const Ark::Backends &backends)
{
    m_arkBackends = backends;
}

void MainWindow::setDiskImageBackend(std::unique_ptr<DiskImageBackend> backend)
{
    m_diskImages = std::move(backend);
}

void MainWindow::setPackageTools(PackageTools *tools)
{
    m_packageTools = tools;
}

KFileItem MainWindow::selectedPackage() const
{
    const KFileItemList selected = activePane()->view()->selectedItems();
    if (selected.size() != 1 || selected.first().localPath().isEmpty() || !AppPackages::isPackage(selected.first().localPath(), selected.first().mimetype())) {
        return {};
    }
    return selected.first();
}

void MainWindow::showApplicationPackage()
{
    const KFileItem item = selectedPackage();
    if (item.isNull()) {
        showStatusMessage(i18nc("@info:status", "Select one local AppImage, Flatpak or snap package."));
        return;
    }
    auto *dialog = new ApplicationPackageDialog(item.localPath(), item.mimetype(), m_packageTools, AppPackages::Locations::system(), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::setIndexBackend(IndexBackend *backend)
{
    m_indexBackend = backend;
    m_searchPanel->searchWidget()->setIndexBackend(backend);
}

void MainWindow::showIndexStatus()
{
    const QUrl url = activePane()->url();
    auto *dialog = new IndexStatusDialog(url.isLocalFile() ? url.toLocalFile() : QString(), m_indexBackend, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

namespace
{
/// The file-type places listed in @p model, with their rows.
QList<std::pair<FileTypePlaces::Category, QModelIndex>> fileTypePlaceRows(const KFilePlacesModel *model)
{
    // The model lists only the entries this application can see, so another
    // application's OnlyInApp entries for the same URLs don't count.
    QList<std::pair<FileTypePlaces::Category, QModelIndex>> rows;
    for (int row = 0; row < model->rowCount(); ++row) {
        const QModelIndex index = model->index(row, 0);
        FileTypePlaces::Category category = FileTypePlaces::Category::Documents;
        if (FileTypePlaces::fromPlaceUrl(model->url(index), &category)) {
            rows.append({category, index});
        }
    }
    return rows;
}

QList<FileTypePlaces::Category> fileTypePlacesIn(const KFilePlacesModel *model)
{
    QList<FileTypePlaces::Category> present;
    for (const auto &[category, index] : fileTypePlaceRows(model)) {
        present.append(category);
    }
    return present;
}
}

void MainWindow::addFileTypePlaces()
{
    KConfigGroup state = KSharedConfig::openStateConfig()->group(QStringLiteral("FileTypePlaces"));
    if (state.readEntry("Added", false)) {
        rememberFileTypePlaces();
    } else {
        syncFileTypePlaces();
        state.writeEntry("Added", true);
        state.sync();
    }
    // Removing an entry from Places hides its category, as clearing it in Preferences does.
    connect(m_placesModel, &QAbstractItemModel::rowsInserted, this, &MainWindow::rememberFileTypePlaces);
    connect(m_placesModel, &QAbstractItemModel::rowsRemoved, this, &MainWindow::rememberFileTypePlaces);
    connect(m_placesModel, &QAbstractItemModel::modelReset, this, &MainWindow::rememberFileTypePlaces);
}

void MainWindow::syncFileTypePlaces()
{
    m_syncingFileTypePlaces = true;
    const FileTypePlaces::PlaceChanges changes = FileTypePlaces::placeChanges(Settings::fileTypeCategoriesHidden(), fileTypePlacesIn(m_placesModel));
    for (const FileTypePlaces::Category category : changes.remove) {
        // Each removal moves the later rows, so look the entry up again.
        for (const auto &[present, index] : fileTypePlaceRows(m_placesModel)) {
            if (present == category) {
                m_placesModel->removePlace(index);
                break;
            }
        }
    }
    // OnlyInApp keeps the entries out of other applications' Places, which can't open them.
    const QString appName = QCoreApplication::applicationName();
    for (const FileTypePlaces::Category category : changes.add) {
        m_placesModel->addPlace(FileTypePlaces::title(category), FileTypePlaces::placeUrl(category), FileTypePlaces::iconName(category), appName);
    }
    m_syncingFileTypePlaces = false;
}

void MainWindow::rememberFileTypePlaces()
{
    if (m_syncingFileTypePlaces) {
        return;
    }
    const QStringList hidden = FileTypePlaces::absentIds(fileTypePlacesIn(m_placesModel));
    if (hidden != Settings::fileTypeCategoriesHidden()) {
        Settings::setFileTypeCategoriesHidden(hidden);
        Settings::self()->save();
    }
}

void MainWindow::openFileTypePlace(Pane *pane, const QUrl &url, bool newTab)
{
    FileTypePlaces::Category category = FileTypePlaces::Category::Documents;
    if (!FileTypePlaces::fromPlaceUrl(url, &category)) {
        return;
    }
    const QString title = FileTypePlaces::title(category);
    const auto chooseFolder = [this, url, title] {
        QFileDialog dialog(this, i18nc("@title:window %1 is a file-type category, such as Ebooks", "Search a Folder for %1", title));
        dialog.setObjectName(QStringLiteral("fileTypeFolderDialog"));
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setOption(QFileDialog::ShowDirsOnly);
        const QUrl current = activePane()->url();
        dialog.setDirectory(current.isLocalFile() ? current.toLocalFile() : QDir::homePath());
        if (dialog.exec() != QDialog::Accepted || dialog.selectedFiles().isEmpty()) {
            return;
        }
        searchFileTypeLive(url, QUrl::fromLocalFile(dialog.selectedFiles().constFirst()));
    };
    if (!FileTypePlaces::usesIndex(category)) {
        showStatusMessage(i18nc("@info:status %1 is a file-type category, %2 a size such as 100 MiB",
                                "File sizes aren't indexed, so %1 searches a chosen folder live for files over %2.",
                                title,
                                KFormat().formatByteSize(double(FileTypePlaces::largeFileSize()), 0)));
        chooseFolder();
        return;
    }

    const IndexStatus::Config config = m_indexBackend->config();
    const QUrl balooUrl = FileTypePlaces::balooUrl(category);
    QString reason;
    if (!config.available || balooUrl.isEmpty()) {
        reason = i18nc("@info %1 is a file-type category", "“%1” is an indexed search, but this build can't use KDE's file index.", title);
    } else if (!config.enabled) {
        reason = i18nc("@info %1 is a file-type category", "“%1” is an indexed search, but file indexing is turned off in KDE File Search settings.", title);
    } else if (!KProtocolInfo::isKnownProtocol(QStringLiteral("baloosearch"))) {
        reason =
            i18nc("@info %1 is a file-type category", "“%1” is an indexed search, but KDE's indexed search location, baloosearch:/, isn't installed.", title);
    }
    const QStringList warnings = reason.isEmpty() ? IndexStatus::warnings(config, m_indexBackend->service()) : QStringList();
    const auto showIndexed = [pane, balooUrl, newTab] {
        if (newTab) {
            pane->openInNewTab(balooUrl, true);
        } else {
            pane->setUrl(balooUrl);
        }
    };
    if (reason.isEmpty() && warnings.isEmpty()) {
        showIndexed();
        return;
    }

    QMessageBox box(this);
    box.setObjectName(QStringLiteral("fileTypePlaceBox"));
    box.setWindowTitle(title);
    box.setIcon(QMessageBox::Information);
    QPushButton *indexed = nullptr;
    QPushButton *settings = nullptr;
    if (reason.isEmpty()) {
        box.setText(i18nc("@info %1 is a file-type category", "The indexed results for “%1” may be incomplete.", title));
        box.setInformativeText(warnings.join(QLatin1Char('\n')) + QStringLiteral("\n\n")
                               + i18nc("@info", "Show the indexed results anyway, or search a chosen folder live instead."));
        indexed = box.addButton(i18nc("@action:button", "Show Indexed Results"), QMessageBox::AcceptRole);
        indexed->setObjectName(QStringLiteral("fileTypeIndexed"));
    } else {
        box.setText(reason);
        box.setInformativeText(i18nc("@info", "You can search a chosen folder live instead. A live search reads the folder's files, so it can take longer."));
        if (config.available && !config.enabled) {
            settings = box.addButton(i18nc("@action:button", "Open KDE File Search Settings"), QMessageBox::ActionRole);
            settings->setObjectName(QStringLiteral("fileTypeSettings"));
        }
    }
    QPushButton *live = box.addButton(i18nc("@action:button", "Search a Folder Live…"), QMessageBox::ActionRole);
    live->setObjectName(QStringLiteral("fileTypeLive"));
    box.addButton(QMessageBox::Cancel);
    box.setDefaultButton(indexed ? indexed : live);
    box.exec();
    if (box.clickedButton() == indexed && indexed) {
        showIndexed();
    } else if (box.clickedButton() == live) {
        chooseFolder();
    } else if (box.clickedButton() == settings && settings) {
        QString error;
        if (!m_indexBackend->openSettings(&error)) {
            showStatusMessage(error.isEmpty() ? i18nc("@info:status", "Could not open KDE File Search settings.") : error);
        }
    }
}

bool MainWindow::searchFileTypeLive(const QUrl &url, const QUrl &folder)
{
    FileTypePlaces::Category category = FileTypePlaces::Category::Documents;
    if (!FileTypePlaces::fromPlaceUrl(url, &category)) {
        return false;
    }
    SearchWidget *search = m_searchPanel->searchWidget();
    if (search->isRunning()) {
        showStatusMessage(i18nc("@info:status", "A search is running. Cancel it to search another folder."));
        return false;
    }
    search->setFolder(folder);
    search->prepareFolderSearch(LiveSearch::Field::Query);
    m_searchPanelAction->setChecked(true);
    return search->search(FileTypePlaces::liveQuery(category));
}

KFileItem MainWindow::selectedDiskImage() const
{
    const KFileItemList selected = activePane()->view()->selectedItems();
    if (selected.size() != 1 || selected.first().isDir() || selected.first().localPath().isEmpty() || !DiskImages::isDiskImage(selected.first().mimetype())) {
        return {};
    }
    return selected.first();
}

void MainWindow::setDiskImageMounted(bool mount)
{
    const KFileItem image = selectedDiskImage();
    if (image.isNull()) {
        showStatusMessage(i18nc("@info:status", "Select one local ISO or disk image."));
        return;
    }
    if (!m_diskImages->pluginInstalled()) {
        KMessageBox::error(this,
                           i18nc("@info",
                                 "Mounting disk images needs the Mount ISO plugin from Dolphin's plugins. Install the dolphin-plugins package "
                                 "from Discover or your distribution."),
                           i18nc("@title:window", "Mount ISO Plugin Not Found"));
        return;
    }
    const QString file = image.localPath();
    const DiskImageBackend::State before = m_diskImages->status(file).state;
    if (mount && before == DiskImageBackend::State::Mounted) {
        showStatusMessage(i18nc("@info:status", "%1 is already mounted at %2.", image.name(), m_diskImages->status(file).mountPoint));
        return;
    }
    if (!mount && before == DiskImageBackend::State::Detached) {
        showStatusMessage(i18nc("@info:status", "%1 is not mounted.", image.name()));
        return;
    }
    // Unmount detaches an image left attached by a refused mount, so a mount
    // that only got as far as the loop device is retried from scratch.
    if (mount && before == DiskImageBackend::State::Attached && !m_diskImages->runPluginAction(file, this)) {
        showStatusMessage(i18nc("@info:status", "The Mount ISO plugin offered no action for %1.", image.name()));
        return;
    }
    if (!m_diskImages->runPluginAction(file, this)) {
        showStatusMessage(
            i18nc("@info:status", "The Mount ISO plugin offered no action for %1. The session bus may not support passing files to UDisks2.", image.name()));
        return;
    }
    const DiskImageBackend::Status after = m_diskImages->status(file);
    if (mount) {
        switch (after.state) {
        case DiskImageBackend::State::Mounted:
            showStatusMessage(i18nc("@info:status", "Mounted %1 at %2.", image.name(), after.mountPoint));
            break;
        case DiskImageBackend::State::Attached:
            showStatusMessage(i18nc("@info:status",
                                    "%1 was attached but not mounted. Authorization may have been denied, or it has no file system Plasma can mount.",
                                    image.name()));
            break;
        case DiskImageBackend::State::Detached:
            showStatusMessage(
                i18nc("@info:status", "%1 was not mounted. Authorization may have been denied or cancelled, or UDisks2 is unavailable.", image.name()));
            break;
        }
    } else if (after.state == DiskImageBackend::State::Detached) {
        showStatusMessage(i18nc("@info:status", "Unmounted %1.", image.name()));
    } else if (after.state == DiskImageBackend::State::Attached) {
        showStatusMessage(i18nc("@info:status", "Unmounted %1, but it is still attached as a loop device. Try Unmount again.", image.name()));
    } else {
        showStatusMessage(i18nc("@info:status", "%1 is still mounted. A program may be using it, or authorization was denied.", image.name()));
    }
}

bool MainWindow::isArkOnlyArchive(const KFileItem &item)
{
    return !item.isDir() && !item.localPath().isEmpty() && !Archives::browseUrl(item.mostLocalUrl(), item.mimetype()).isValid()
        && arkBackends().canRead(item.mimetype());
}

void MainWindow::launchArk(const QStringList &arguments)
{
    if (!Ark::isInstalled()) {
        KMessageBox::error(this,
                           i18nc("@info", "Ark, KDE's archiving tool, is not installed or is not in PATH. Install it from Discover or your distribution."),
                           i18nc("@title:window", "Ark Not Found"));
        return;
    }
    auto *job = new KIO::CommandLauncherJob(QStringLiteral("ark"), arguments, this);
    job->setDesktopName(QStringLiteral("org.kde.ark"));
    connect(job, &KJob::result, this, [this](KJob *finished) {
        if (finished->error()) {
            showStatusMessage(i18nc("@info:status", "Could not start Ark: %1", finished->errorString()));
        }
    });
    job->start();
}

void MainWindow::addToFavorites()
{
    const KFileItemList selected = activePane()->view()->selectedItems();
    bool addedSelection = false;
    for (const KFileItem &item : selected) {
        if (item.isDir()) {
            m_favorites->add(item.targetUrl());
            addedSelection = true;
        }
    }
    if (!addedSelection) {
        m_favorites->add(activePane()->url());
    }
}

void MainWindow::setupStatusBar()
{
    setStatusBar(new PaddedStatusBar(this));
    auto *indicators = new QWidget(this);
    m_statusBarLayout = new QHBoxLayout(indicators);
    m_statusBarLayout->setContentsMargins(8, 0, 8, 0);
    m_statusBarLayout->setSpacing(8);
    m_itemsLabel = new QLabel(indicators);
    m_itemsLabel->setObjectName(QStringLiteral("itemsLabel"));
    m_messageLabel = new QLabel(indicators);
    m_freeSpaceLabel = new QLabel(indicators);
    m_freeSpaceLabel->setObjectName(QStringLiteral("freeSpaceLabel"));
    m_statusBarLayout->addWidget(m_itemsLabel);
    m_statusBarLayout->addWidget(m_messageLabel, 1);
    m_statusBarLayout->addWidget(m_freeSpaceLabel);
    statusBar()->addWidget(indicators, 1);
    for (Pane *pane : {m_left, m_right}) {
        auto *bar = new QWidget(pane);
        bar->setObjectName(QStringLiteral("paneStatusBar"));
        auto *layout = new QHBoxLayout(bar);
        layout->setContentsMargins(8, 0, 8, 0);
        layout->setSpacing(8);
        auto *items = new QLabel(bar);
        items->setObjectName(pane == m_left ? QStringLiteral("leftItemsLabel") : QStringLiteral("rightItemsLabel"));
        auto *freeSpace = new QLabel(bar);
        freeSpace->setObjectName(pane == m_left ? QStringLiteral("leftFreeSpaceLabel") : QStringLiteral("rightFreeSpaceLabel"));
        layout->addWidget(items);
        layout->addStretch();
        layout->addWidget(freeSpace);
        pane->setStatusBarWidget(bar);
        m_paneItemsLabels.insert(pane, items);
        m_paneFreeSpaceLabels.insert(pane, freeSpace);
    }
    updateStatusBarIndicatorOrder();
}

void MainWindow::updateStatusBarIndicatorOrder()
{
    const bool freeSpaceFirst = Settings::statusBarIndicatorOrder() == Settings::EnumStatusBarIndicatorOrder::FreeSpaceFirst;
    m_statusBarLayout->removeWidget(m_itemsLabel);
    m_statusBarLayout->removeWidget(m_freeSpaceLabel);
    m_statusBarLayout->insertWidget(0, freeSpaceFirst ? m_freeSpaceLabel : m_itemsLabel);
    m_statusBarLayout->addWidget(freeSpaceFirst ? m_itemsLabel : m_freeSpaceLabel);

    for (Pane *pane : {m_left, m_right}) {
        auto *layout = qobject_cast<QBoxLayout *>(pane->statusBarWidget()->layout());
        QLabel *items = m_paneItemsLabels.value(pane);
        QLabel *freeSpace = m_paneFreeSpaceLabels.value(pane);
        layout->removeWidget(items);
        layout->removeWidget(freeSpace);
        layout->insertWidget(0, freeSpaceFirst ? freeSpace : items);
        layout->addWidget(freeSpaceFirst ? items : freeSpace);
    }
}

void MainWindow::setupActions()
{
    KActionCollection *actions = actionCollection();

    m_workspaceSelectorAction = new KSelectAction(i18nc("@action", "Workspace: None"), this);
    actions->addAction(QStringLiteral("workspace_selector"), m_workspaceSelectorAction);
    connect(m_workspaceSelectorAction, &KSelectAction::indexTriggered, this, [this](int index) {
        selectWorkspace(index);
    });
    m_workspaceStatusButton = new QToolButton(this);
    m_workspaceStatusButton->setObjectName(QStringLiteral("workspaceStatusButton"));
    m_workspaceStatusButton->setAutoRaise(true);
    m_workspaceStatusButton->setPopupMode(QToolButton::InstantPopup);
    m_workspaceStatusButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_workspaceStatusButton->setMenu(m_workspaceSelectorAction->menu());
    m_workspaceStatusButton->setText(m_workspaceSelectorAction->text());
    statusBar()->addPermanentWidget(m_workspaceStatusButton);
    QAction *restoreWorkspace = actions->addAction(QStringLiteral("restore_workspace_layout"), this, &MainWindow::restoreActiveWorkspaceLayout);
    restoreWorkspace->setText(i18nc("@action", "Restore Saved Workspace Layout…"));
    QAction *workspaces = actions->addAction(QStringLiteral("manage_workspaces"), this, &MainWindow::showWorkspaces);
    workspaces->setText(i18nc("@action", "Workspaces…"));
    workspaces->setIcon(SymbolicIcon::fromTheme(QStringLiteral("view-grid")));
    QAction *activityAssociations = actions->addAction(QStringLiteral("activity_associations"), this, &MainWindow::showActivityAssociations);
    activityAssociations->setText(i18nc("@action", "Activity Associations…"));
    activityAssociations->setIcon(SymbolicIcon::fromTheme(QStringLiteral("activities")));
    QAction *applyActivity = actions->addAction(QStringLiteral("apply_activity_association"), this, [this] {
        m_activityController->applyCurrent();
    });
    applyActivity->setText(i18nc("@action", "Apply Activity Association"));
    QAction *kwinIntegration = actions->addAction(QStringLiteral("kwin_integration"), this, &MainWindow::showKWinIntegration);
    kwinIntegration->setText(i18nc("@action", "Configure KWin Integration…"));
    kwinIntegration->setToolTip(i18nc("@info:tooltip", "Open Preferences › KDE Plasma › KWin to manage the KWin script and the drop-down window"));
    kwinIntegration->setIcon(SymbolicIcon::fromTheme(QStringLiteral("kwin")));
    QAction *toggleDropDown = actions->addAction(QStringLiteral("toggle_drop_down"), this, &MainWindow::toggleDropDown);
    toggleDropDown->setText(i18nc("@action", "Show or Hide Drop-Down Window"));
    toggleDropDown->setIcon(SymbolicIcon::fromTheme(QStringLiteral("go-down")));
    // Unassigned by default; the user chooses one under Preferences › KDE Plasma › KWin.
    KGlobalAccel::setGlobalShortcut(toggleDropDown, QList<QKeySequence>{});

    // File
    m_newFileMenu = new KNewFileMenu(this);
    m_newFileMenu->setParentWidget(this);
    m_newFileMenu->setPopupMode(QToolButton::InstantPopup);
    actions->addAction(QStringLiteral("new_menu"), m_newFileMenu);
    actions->setDefaultShortcut(m_newFileMenu, Qt::CTRL | Qt::Key_N);
    connect(m_newFileMenu, &QAction::triggered, this, &MainWindow::showNewMenu);
    connect(m_newFileMenu->menu(), &QMenu::aboutToShow, this, [this] {
        m_newFileMenu->setWorkingDirectory(activePane()->url());
        m_newFileMenu->checkUpToDate();
    });

    QAction *newFolder = actions->addAction(QStringLiteral("new_folder"), this, &MainWindow::newFolder);
    newFolder->setText(i18nc("@action", "New Folder…"));
    newFolder->setIcon(QIcon::fromTheme(QStringLiteral("folder-new")));
    actions->setDefaultShortcut(newFolder, Qt::Key_F7);
    m_newFileMenu->setNewFolderShortcutAction(newFolder);

    QAction *copyTo = actions->addAction(QStringLiteral("copy_to_other_pane"), this, &MainWindow::copyToOtherPane);
    copyTo->setText(i18nc("@action", "Copy to Other Pane"));
    copyTo->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy")));
    actions->setDefaultShortcut(copyTo, Qt::Key_F5);

    QAction *moveTo = actions->addAction(QStringLiteral("move_to_other_pane"), this, &MainWindow::moveToOtherPane);
    moveTo->setText(i18nc("@action", "Move to Other Pane"));
    moveTo->setIcon(QIcon::fromTheme(QStringLiteral("edit-move")));
    actions->setDefaultShortcut(moveTo, Qt::Key_F6);

    QAction *renameAction = actions->addAction(QStringLiteral("rename"), this, &MainWindow::rename);
    renameAction->setText(i18nc("@action", "Rename…"));
    renameAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-rename")));
    actions->setDefaultShortcut(renameAction, Qt::Key_F2);
    QAction *batchRenameAction = actions->addAction(QStringLiteral("batch_rename"), this, &MainWindow::batchRename);
    batchRenameAction->setText(i18nc("@action", "Batch Rename…"));
    batchRenameAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-rename")));
    actions->setDefaultShortcut(batchRenameAction, Qt::SHIFT | Qt::Key_F2);
    QAction *editMetadata = actions->addAction(QStringLiteral("edit_metadata"), this, &MainWindow::editMetadata);
    editMetadata->setText(i18nc("@action", "Edit Metadata…"));
    editMetadata->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));

    QAction *symlink = actions->addAction(QStringLiteral("create_symlink"), this, &MainWindow::createSymlink);
    symlink->setText(i18nc("@action", "Create Symbolic Link…"));
    symlink->setIcon(QIcon::fromTheme(QStringLiteral("emblem-symbolic-link")));

    QAction *trashAction = actions->addAction(QStringLiteral("move_to_trash"), this, &MainWindow::trash);
    trashAction->setText(i18nc("@action", "Move to Trash"));
    trashAction->setIcon(QIcon::fromTheme(QStringLiteral("user-trash")));
    actions->setDefaultShortcuts(trashAction, {Qt::Key_F8, Qt::Key_Delete});

    QAction *deleteAction = actions->addAction(QStringLiteral("delete"), this, &MainWindow::deletePermanently);
    deleteAction->setText(i18nc("@action", "Delete Permanently"));
    deleteAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    actions->setDefaultShortcut(deleteAction, Qt::SHIFT | Qt::Key_Delete);

    QAction *properties = actions->addAction(QStringLiteral("properties"), this, &MainWindow::showProperties);
    properties->setText(i18nc("@action", "Properties"));
    properties->setIcon(QIcon::fromTheme(QStringLiteral("document-properties")));
    actions->setDefaultShortcut(properties, Qt::ALT | Qt::Key_Return);

    QAction *occupiedSpace = actions->addAction(QStringLiteral("calculate_occupied_space"), this, &MainWindow::calculateOccupiedSpace);
    occupiedSpace->setText(i18nc("@action", "Calculate Occupied Space"));
    occupiedSpace->setIcon(QIcon::fromTheme(QStringLiteral("dialog-information")));
    QAction *contentChart = actions->addAction(QStringLiteral("content_chart"), this, &MainWindow::showContentChart);
    contentChart->setText(i18nc("@action", "Folder Content Chart"));
    contentChart->setIcon(QIcon::fromTheme(QStringLiteral("view-statistics")));
    QAction *search = actions->addAction(QStringLiteral("search_folder"), this, &MainWindow::showSearch);
    search->setText(i18nc("@action", "Search Current Folder…"));
    search->setIcon(QIcon::fromTheme(QStringLiteral("edit-find")));
    actions->setDefaultShortcut(search, Qt::CTRL | Qt::ALT | Qt::Key_F);
    QAction *indexStatus = actions->addAction(QStringLiteral("index_status"), this, &MainWindow::showIndexStatus);
    indexStatus->setText(i18nc("@action", "Indexing Status…"));
    indexStatus->setToolTip(i18nc("@info:tooltip", "Show the file indexer's state, indexed folders and coverage of the current folder"));
    indexStatus->setIcon(QIcon::fromTheme(QStringLiteral("baloo")));
    QAction *duplicateFinder = actions->addAction(QStringLiteral("find_duplicates"), this, &MainWindow::showDuplicateFinder);
    duplicateFinder->setText(i18nc("@action", "Find Duplicate Files…"));
    QAction *compareFolders = actions->addAction(QStringLiteral("compare_pane_folders"), this, &MainWindow::comparePaneFolders);
    compareFolders->setText(i18nc("@action", "Compare Pane Folders"));
    QAction *filteredComparison = actions->addAction(QStringLiteral("compare_folders_filtered"), this, &MainWindow::compareFoldersFiltered);
    filteredComparison->setText(i18nc("@action", "Compare and Synchronize Folders…"));
    QAction *cloudSync = actions->addAction(QStringLiteral("cloud_sync"), this, &MainWindow::showCloudSync);
    cloudSync->setText(i18nc("@action", "Cloud Sync…"));
    cloudSync->setIcon(QIcon::fromTheme(QStringLiteral("folder-cloud")));
    cloudSync->setToolTip(i18nc("@info:tooltip", "Keep local folders synchronized with cloud and network folders"));
    QAction *cloudActivity = actions->addAction(QStringLiteral("cloud_activity"), this, &MainWindow::showCloudActivity);
    cloudActivity->setText(i18nc("@action", "Cloud Activity…"));
    cloudActivity->setIcon(QIcon::fromTheme(QStringLiteral("network-transmit-receive")));
    cloudActivity->setToolTip(i18nc("@info:tooltip", "Show cloud connections, transfers, speeds and recent errors"));
    QAction *createChecksum = actions->addAction(QStringLiteral("create_checksum"), this, &MainWindow::createChecksumFile);
    createChecksum->setText(i18nc("@action", "Create SHA-256 Checksum File…"));
    QAction *verifyChecksum = actions->addAction(QStringLiteral("verify_checksum"), this, &MainWindow::verifyChecksumFile);
    verifyChecksum->setText(i18nc("@action", "Verify SHA-256 Checksum File"));
    QAction *splitFile = actions->addAction(QStringLiteral("split_file"), this, &MainWindow::splitFile);
    splitFile->setText(i18nc("@action", "Split File…"));
    QAction *combineFiles = actions->addAction(QStringLiteral("combine_files"), this, &MainWindow::combineFiles);
    combineFiles->setText(i18nc("@action", "Combine Files…"));

    auto *encryption = new KActionMenu(SymbolicIcon::fromTheme(QStringLiteral("document-encrypt")), i18nc("@action", "Encryption"), this);
    encryption->setToolTip(i18nc("@info:tooltip", "Encrypt, decrypt, sign or verify the selected files"));
    encryption->setPopupMode(QToolButton::InstantPopup);
    actions->addAction(QStringLiteral("encryption"), encryption);
    QMenu *encryptionMenu = encryption->menu();
    encryptionMenu->setToolTipsVisible(true);
    const auto addEncryptionAction = [this, actions, encryptionMenu](const QString &name, const QString &text, const QString &icon, auto slot) {
        QAction *action = actions->addAction(name, this, slot);
        action->setText(text);
        action->setIcon(SymbolicIcon::fromTheme(icon));
        encryptionMenu->addAction(action);
        return action;
    };
    const auto runOperation = [this](FileEncryption::Operation operation) {
        return [this, operation] {
            encryptSelection(operation);
        };
    };
    addEncryptionAction(QStringLiteral("encrypt_files"),
                        i18nc("@action", "Encrypt…"),
                        QStringLiteral("document-encrypt"),
                        runOperation(FileEncryption::Operation::Encrypt));
    addEncryptionAction(QStringLiteral("decrypt_files"),
                        i18nc("@action", "Decrypt"),
                        QStringLiteral("document-decrypt"),
                        runOperation(FileEncryption::Operation::Decrypt));
    addEncryptionAction(QStringLiteral("sign_files"),
                        i18nc("@action", "Sign…"),
                        QStringLiteral("document-sign"),
                        runOperation(FileEncryption::Operation::Sign));
    addEncryptionAction(QStringLiteral("verify_signatures"),
                        i18nc("@action", "Verify Signature"),
                        QStringLiteral("document-edit-verify"),
                        runOperation(FileEncryption::Operation::Verify));
    encryptionMenu->addSeparator();
    addEncryptionAction(QStringLiteral("aes_encrypt_files"),
                        i18nc("@action", "Encrypt with AES Crypt…"),
                        QStringLiteral("document-encrypt"),
                        runOperation(FileEncryption::Operation::AesEncrypt));
    addEncryptionAction(QStringLiteral("aes_decrypt_files"),
                        i18nc("@action", "Decrypt with AES Crypt…"),
                        QStringLiteral("document-decrypt"),
                        runOperation(FileEncryption::Operation::AesDecrypt));
    encryptionMenu->addSeparator();
    addEncryptionAction(QStringLiteral("kleopatra_encrypt_sign"),
                        i18nc("@action", "Sign and Encrypt with Kleopatra…"),
                        QStringLiteral("document-edit-sign-encrypt"),
                        [this] {
                            openSelectionInKleopatra(false);
                        })
        ->setToolTip(i18nc("@info:tooltip", "Open Kleopatra's own sign and encrypt dialog for the selected files"));
    addEncryptionAction(QStringLiteral("kleopatra_decrypt_verify"),
                        i18nc("@action", "Decrypt and Verify with Kleopatra…"),
                        QStringLiteral("document-edit-decrypt-verify"),
                        [this] {
                            openSelectionInKleopatra(true);
                        })
        ->setToolTip(i18nc("@info:tooltip", "Open Kleopatra's own decrypt and verify dialog for the selected files"));
    connect(encryptionMenu, &QMenu::aboutToShow, this, &MainWindow::updateEncryptionActions);
    // The menu disables actions that do not suit the selection; shortcuts check the selection themselves.
    connect(encryptionMenu, &QMenu::aboutToHide, this, [encryptionMenu] {
        for (QAction *action : encryptionMenu->actions()) {
            action->setEnabled(true);
        }
    });

    QAction *terminal = actions->addAction(QStringLiteral("open_terminal"), this, &MainWindow::openTerminal);
    terminal->setText(i18nc("@action", "Open Terminal Here"));
    terminal->setIcon(QIcon::fromTheme(QStringLiteral("utilities-terminal")));
    actions->setDefaultShortcut(terminal, Qt::SHIFT | Qt::Key_F4);
    auto *terminalWith = new KActionMenu(QIcon::fromTheme(QStringLiteral("utilities-terminal")), i18nc("@action", "Open Terminal With"), this);
    terminalWith->setToolTip(i18nc("@info:tooltip", "Open the active folder in a chosen terminal application"));
    terminalWith->setPopupMode(QToolButton::InstantPopup);
    actions->addAction(QStringLiteral("open_terminal_with"), terminalWith);
    QMenu *terminalWithMenu = terminalWith->menu();
    terminalWithMenu->setToolTipsVisible(true);
    for (const TerminalProviders::Provider provider : TerminalProviders::separateProviders()) {
        QAction *entry = terminalWithMenu->addAction(QIcon::fromTheme(TerminalProviders::iconName(provider)), TerminalProviders::name(provider));
        entry->setObjectName(QStringLiteral("openTerminalWith_") + TerminalProviders::id(provider));
        entry->setData(static_cast<int>(provider));
        connect(entry, &QAction::triggered, this, [this, provider] {
            TerminalLauncher::open(provider, activePane()->url(), this, [window = QPointer<MainWindow>(this)](const QString &message) {
                if (window) {
                    window->showStatusMessage(message);
                }
            });
        });
    }
    connect(terminalWithMenu, &QMenu::aboutToShow, this, [terminalWithMenu] {
        const QList<QAction *> entries = terminalWithMenu->actions();
        for (QAction *entry : entries) {
            const auto provider = static_cast<TerminalProviders::Provider>(entry->data().toInt());
            const QString reason = TerminalProviders::unavailableReason(provider);
            entry->setEnabled(reason.isEmpty());
            entry->setToolTip(reason.isEmpty()
                                  ? i18nc("@info:tooltip %1 is a terminal application", "Open the active folder in %1", TerminalProviders::name(provider))
                                  : reason);
        }
    });
    QAction *customCommand = actions->addAction(QStringLiteral("custom_command"), this, &MainWindow::showCustomCommand);
    customCommand->setText(i18nc("@action", "Scripts and Custom Commands…"));
    customCommand->setIcon(QIcon::fromTheme(QStringLiteral("system-run")));
    refreshCustomCommandActions();
    QAction *transferQueue = actions->addAction(QStringLiteral("transfer_queue"), this, &MainWindow::showTransferQueue);
    transferQueue->setText(i18nc("@action", "Transfer Queue…"));
    transferQueue->setIcon(QIcon::fromTheme(QStringLiteral("view-list-details")));
    QAction *editor = actions->addAction(QStringLiteral("open_editor"), this, &MainWindow::openEditor);
    editor->setText(i18nc("@action", "Open Editor"));
    editor->setIcon(SymbolicIcon::fromTheme(QStringLiteral("view-pim-notes")));
    QAction *editFile = actions->addAction(QStringLiteral("edit_file"), this, &MainWindow::editFile);
    editFile->setText(i18nc("@action", "Edit"));
    editFile->setIcon(SymbolicIcon::fromTheme(QStringLiteral("document-edit")));
    actions->setDefaultShortcut(editFile, Qt::Key_F4);
    auto *editWith = new KActionMenu(SymbolicIcon::fromTheme(QStringLiteral("document-edit")), i18nc("@action", "Edit With"), this);
    editWith->setToolTip(i18nc("@info:tooltip", "Open the selected files in a chosen text editor"));
    editWith->setPopupMode(QToolButton::InstantPopup);
    actions->addAction(QStringLiteral("edit_with"), editWith);
    QMenu *editWithMenu = editWith->menu();
    editWithMenu->setToolTipsVisible(true);
    for (const EditorProviders::Editor editorProvider : EditorProviders::editors()) {
        QAction *entry = editWithMenu->addAction(QIcon::fromTheme(EditorProviders::iconName(editorProvider)), EditorProviders::name(editorProvider));
        entry->setObjectName(QStringLiteral("editWith_") + EditorProviders::id(editorProvider));
        entry->setData(static_cast<int>(editorProvider));
        connect(entry, &QAction::triggered, this, [this, editorProvider] {
            editWithEditor(editorProvider);
        });
    }
    connect(editWithMenu, &QMenu::aboutToShow, this, [editWithMenu] {
        const QList<QAction *> entries = editWithMenu->actions();
        for (QAction *entry : entries) {
            const auto editorProvider = static_cast<EditorProviders::Editor>(entry->data().toInt());
            const QString reason = EditorProviders::unavailableReason(editorProvider);
            entry->setEnabled(reason.isEmpty());
            if (!reason.isEmpty()) {
                entry->setToolTip(reason);
            } else if (EditorProviders::runsInTerminal(editorProvider)) {
                entry->setToolTip(EditorProviders::usesSeparateTerminal() ? i18nc("@info:tooltip %1 is a text editor",
                                                                                  "Open the selected files with %1 in a separate terminal",
                                                                                  EditorProviders::name(editorProvider))
                                                                          : i18nc("@info:tooltip %1 is a text editor",
                                                                                  "Open the selected files with %1 in the embedded terminal",
                                                                                  EditorProviders::name(editorProvider)));
            } else {
                entry->setToolTip(i18nc("@info:tooltip %1 is a text editor", "Open the selected files in %1", EditorProviders::name(editorProvider)));
            }
        }
    });
    auto *openInOffice = new KActionMenu(SymbolicIcon::fromTheme(QStringLiteral("x-office-document")), i18nc("@action", "Open in Office Application"), this);
    openInOffice->setToolTip(i18nc("@info:tooltip", "Open the selected documents, spreadsheets or presentations in a chosen office application"));
    openInOffice->setPopupMode(QToolButton::InstantPopup);
    actions->addAction(QStringLiteral("open_in_office"), openInOffice);
    QMenu *openInOfficeMenu = openInOffice->menu();
    openInOfficeMenu->setToolTipsVisible(true);
    for (const OfficeApps::App app : OfficeApps::apps()) {
        QAction *entry = openInOfficeMenu->addAction(QIcon::fromTheme(OfficeApps::iconName(app)), OfficeApps::name(app));
        entry->setObjectName(QStringLiteral("openInOffice_") + OfficeApps::id(app));
        entry->setData(static_cast<int>(app));
        connect(entry, &QAction::triggered, this, [this, app] {
            openInOfficeApp(app);
        });
    }
    connect(openInOfficeMenu, &QMenu::aboutToShow, this, [this, openInOfficeMenu] {
        QList<OfficeApps::Kind> kinds;
        const KFileItemList items = activePane()->view()->selectedItems();
        for (const KFileItem &item : items) {
            const OfficeApps::Kind kind = OfficeApps::kind(item.mimetype());
            if (!kinds.contains(kind)) {
                kinds.append(kind);
            }
        }
        const QList<QAction *> entries = openInOfficeMenu->actions();
        for (QAction *entry : entries) {
            const auto app = static_cast<OfficeApps::App>(entry->data().toInt());
            const QString reason = OfficeApps::unavailableReason(app, kinds);
            entry->setEnabled(reason.isEmpty());
            entry->setToolTip(reason.isEmpty() ? i18nc("@info:tooltip %1 is an office application", "Open the selected documents in %1", OfficeApps::name(app))
                                               : reason);
        }
    });
    QAction *sendToDevice = actions->addAction(QStringLiteral("kdeconnect_send"), this, &MainWindow::sendToDevice);
    sendToDevice->setText(i18nc("@action", "Send to Device…"));
    sendToDevice->setToolTip(i18nc("@info:tooltip", "Send the selected files to a paired device with KDE Connect"));
    sendToDevice->setIcon(SymbolicIcon::fromTheme(QStringLiteral("smartphone")));
    QAction *sendByEmail = actions->addAction(QStringLiteral("send_by_email"), this, &MainWindow::sendByEmail);
    sendByEmail->setText(i18nc("@action", "Send by Email…"));
    sendByEmail->setToolTip(i18nc("@info:tooltip", "Open a new message in your mail client with the selected files attached"));
    sendByEmail->setIcon(SymbolicIcon::fromTheme(QStringLiteral("mail-send")));
    QAction *shareOnNetwork = actions->addAction(QStringLiteral("share_on_network"), this, &MainWindow::shareOnNetwork);
    shareOnNetwork->setText(i18nc("@action", "Share on Local Network…"));
    shareOnNetwork->setToolTip(
        i18nc("@info:tooltip", "Share the selected items, or the current folder, with browsers on the local network for a limited time"));
    shareOnNetwork->setIcon(SymbolicIcon::fromTheme(QStringLiteral("network-server")));
    auto *actAsAdmin = new KToggleAction(SymbolicIcon::fromTheme(QStringLiteral("system-switch-user")), i18nc("@action", "Act as Administrator"), this);
    actAsAdmin->setToolTip(i18nc("@info:tooltip", "Open the current local folder with administrator rights through KIO's admin worker"));
    actions->addAction(QStringLiteral("act_as_admin"), actAsAdmin);
    connect(actAsAdmin, &KToggleAction::triggered, this, &MainWindow::setActingAsAdministrator);
    QAction *protectFolder = actions->addAction(QStringLiteral("protect_folder"), this, [this] {
        FolderGateDialog::protect(this, passwordFolder(activePane()));
    });
    protectFolder->setText(i18nc("@action", "Protect with Password…"));
    protectFolder->setToolTip(i18nc("@info:tooltip", "Require a password before Plasma Opus shows the selected folder's contents; this is not encryption"));
    protectFolder->setIcon(SymbolicIcon::fromTheme(QStringLiteral("folder-locked")));
    QAction *unlockFolder = actions->addAction(QStringLiteral("unlock_folder"), this, [this] {
        FolderGateDialog::unlock(this, passwordFolder(activePane()));
    });
    unlockFolder->setText(i18nc("@action", "Unlock…"));
    unlockFolder->setToolTip(i18nc("@info:tooltip", "Enter the password of the selected protected folder to show its contents"));
    unlockFolder->setIcon(SymbolicIcon::fromTheme(QStringLiteral("object-unlocked")));
    QAction *changeFolderPassword = actions->addAction(QStringLiteral("change_folder_password"), this, [this] {
        FolderGateDialog::changePassword(this, passwordFolder(activePane()));
    });
    changeFolderPassword->setText(i18nc("@action", "Change Password…"));
    changeFolderPassword->setToolTip(i18nc("@info:tooltip", "Change the password of the selected protected folder"));
    changeFolderPassword->setIcon(SymbolicIcon::fromTheme(QStringLiteral("lock-edit")));
    QAction *removeFolderPassword = actions->addAction(QStringLiteral("remove_folder_password"), this, [this] {
        FolderGateDialog::removeProtection(this, passwordFolder(activePane()));
    });
    removeFolderPassword->setText(i18nc("@action", "Remove Password Protection…"));
    removeFolderPassword->setToolTip(i18nc("@info:tooltip", "Stop requiring a password for the selected protected folder"));
    removeFolderPassword->setIcon(SymbolicIcon::fromTheme(QStringLiteral("edit-delete-remove")));
    QAction *lockFolders = actions->addAction(QStringLiteral("lock_protected_folders"), FolderGates::instance(), &FolderGates::lockAll);
    lockFolders->setText(i18nc("@action", "Lock Protected Folders"));
    lockFolders->setToolTip(i18nc("@info:tooltip", "Lock every unlocked password-protected folder again"));
    lockFolders->setIcon(SymbolicIcon::fromTheme(QStringLiteral("object-locked")));
    connect(FolderGates::instance(), &FolderGates::changed, this, &MainWindow::updateActions);
    QAction *mountManager = actions->addAction(QStringLiteral("mount_manager"), this, &MainWindow::showMountManager);
    mountManager->setText(i18nc("@action", "Mount Manager…"));
    mountManager->setToolTip(i18nc("@info:tooltip", "List file systems and mount, unmount or eject them"));
    mountManager->setIcon(SymbolicIcon::fromTheme(QStringLiteral("drive-harddisk")));
    QAction *compare = actions->addAction(QStringLiteral("compare_files"), this, &MainWindow::compareFiles);
    compare->setText(i18nc("@action", "Compare Files"));
    compare->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));
    QAction *exportDefault = actions->add<QAction>(QStringLiteral("export_listing"));
    exportDefault->setText(i18nc("@action", "Export Listing…"));
    connect(exportDefault, &QAction::triggered, this, [this] {
        exportListing(Settings::listingExportFormat() == Settings::EnumListingExportFormat::Pdf);
    });
    QAction *exportCsv = actions->add<QAction>(QStringLiteral("export_listing_csv"));
    exportCsv->setText(i18nc("@action", "Export Listing to CSV…"));
    connect(exportCsv, &QAction::triggered, this, [this] {
        exportListing(false);
    });
    QAction *exportPdf = actions->add<QAction>(QStringLiteral("export_listing_pdf"));
    exportPdf->setText(i18nc("@action", "Export Listing to PDF…"));
    connect(exportPdf, &QAction::triggered, this, [this] {
        exportListing(true);
    });

    KStandardAction::quit(this, &MainWindow::close, actions);

    // Edit
    m_undoAction = KStandardAction::undo(KIO::FileUndoManager::self(), &KIO::FileUndoManager::undo, actions);
    m_undoAction->setEnabled(KIO::FileUndoManager::self()->isUndoAvailable());
    // The platform alternates (Shift+Delete for cut, Backspace for back…)
    // collide with the file-manager bindings below, so pin single keys.
    QAction *cutAction = KStandardAction::cut(this, &MainWindow::cut, actions);
    actions->setDefaultShortcut(cutAction, Qt::CTRL | Qt::Key_X);
    QAction *copyAction = KStandardAction::copy(this, &MainWindow::copy, actions);
    actions->setDefaultShortcut(copyAction, Qt::CTRL | Qt::Key_C);
    m_pasteAction = KStandardAction::paste(this, &MainWindow::paste, actions);
    actions->setDefaultShortcut(m_pasteAction, Qt::CTRL | Qt::Key_V);
    actions->setDefaultShortcut(m_undoAction, Qt::CTRL | Qt::Key_Z);
    KStandardAction::selectAll(
        this,
        [this] {
            activePane()->view()->selectAll();
        },
        actions);
    // Navigation actions have no default shortcuts; Preferences › Keyboard › Navigation can give them keys no other action uses.
    QAction *deselectAll = actions->addAction(QStringLiteral("deselect_all"), this, [this] {
        activePane()->view()->clearSelection();
    });
    deselectAll->setText(i18nc("@action", "Deselect All"));
    deselectAll->setIcon(SymbolicIcon::fromTheme(QStringLiteral("edit-select-none")));
    QAction *invertSelection = actions->addAction(QStringLiteral("invert_selection"), this, [this] {
        activePane()->view()->invertSelection();
    });
    invertSelection->setText(i18nc("@action", "Invert Selection"));
    invertSelection->setToolTip(i18nc("@info:tooltip", "Select the items that aren't selected and deselect the others"));
    invertSelection->setIcon(SymbolicIcon::fromTheme(QStringLiteral("edit-select-invert")));

    auto *selectMenu = new QMenu(i18nc("@menu", "Select"), this);
    QAction *selectPattern = selectMenu->addAction(i18nc("@action:inmenu", "Select by Pattern…"));
    connect(selectPattern, &QAction::triggered, this, [this] {
        selectByPattern(true);
    });
    QAction *deselectPattern = selectMenu->addAction(i18nc("@action:inmenu", "Deselect by Pattern…"));
    connect(deselectPattern, &QAction::triggered, this, [this] {
        selectByPattern(false);
    });
    QAction *selectPaths = actions->addAction(QStringLiteral("select_by_path_filter"));
    selectPaths->setText(i18nc("@action:inmenu", "Select by Path Filter…"));
    selectMenu->addAction(selectPaths);
    connect(selectPaths, &QAction::triggered, this, [this] {
        selectByPathFilter(true);
    });
    QAction *deselectPaths = actions->addAction(QStringLiteral("deselect_by_path_filter"));
    deselectPaths->setText(i18nc("@action:inmenu", "Deselect by Path Filter…"));
    selectMenu->addAction(deselectPaths);
    connect(deselectPaths, &QAction::triggered, this, [this] {
        selectByPathFilter(false);
    });
    QAction *selectQuery = actions->addAction(QStringLiteral("select_by_query"));
    selectQuery->setText(i18nc("@action:inmenu", "Select by Query…"));
    selectQuery->setToolTip(i18nc("@info:tooltip", "Select items that match a search query, such as tag:holiday type:image/*"));
    selectMenu->addAction(selectQuery);
    connect(selectQuery, &QAction::triggered, this, [this] {
        selectByQuery(true);
    });
    QAction *deselectQuery = actions->addAction(QStringLiteral("deselect_by_query"));
    deselectQuery->setText(i18nc("@action:inmenu", "Deselect by Query…"));
    selectMenu->addAction(deselectQuery);
    connect(deselectQuery, &QAction::triggered, this, [this] {
        selectByQuery(false);
    });
    selectMenu->addSeparator();
    QAction *saveSelection = selectMenu->addAction(i18nc("@action:inmenu", "Save Selection Filter…"));
    connect(saveSelection, &QAction::triggered, this, &MainWindow::saveSelectionFilter);
    m_selectionFiltersMenu = selectMenu->addMenu(i18nc("@menu", "Saved Selection Filters"));
    connect(m_selectionFiltersMenu, &QMenu::aboutToShow, this, &MainWindow::populateSelectionFilters);
    actions->addAction(QStringLiteral("select_menu"), selectMenu->menuAction());

    QAction *copyPath = actions->addAction(QStringLiteral("copy_path"), this, [this] {
        const KFileItemList items = activePane()->view()->selectedItemsOrRoot();
        QStringList paths;
        for (const KFileItem &item : items) {
            paths.append(item.url().toDisplayString(QUrl::PreferLocalFile));
        }
        QApplication::clipboard()->setText(paths.join(QLatin1Char('\n')));
    });
    copyPath->setText(i18nc("@action", "Copy Path"));
    copyPath->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy-path")));

    // View
    m_dualLayoutAction = new KToggleAction(QIcon::fromTheme(QStringLiteral("view-split-left-right")), i18nc("@action", "Dual Pane"), this);
    m_dualLayoutAction->setChecked(true);
    actions->addAction(QStringLiteral("dual_layout"), m_dualLayoutAction);
    actions->setDefaultShortcut(m_dualLayoutAction, Qt::CTRL | Qt::SHIFT | Qt::Key_L);
    connect(m_dualLayoutAction, &KToggleAction::toggled, this, &MainWindow::toggleLayout);

    m_stackedLayoutAction = new KToggleAction(QIcon::fromTheme(QStringLiteral("view-split-top-bottom")), i18nc("@action", "Stacked Panes"), this);
    actions->addAction(QStringLiteral("stacked_layout"), m_stackedLayoutAction);
    connect(m_stackedLayoutAction, &KToggleAction::toggled, this, &MainWindow::setStackedLayout);

    auto *swapPanes = actions->addAction(QStringLiteral("swap_panes"), this, &MainWindow::swapPanesAccordingToPreference);
    swapPanes->setText(i18nc("@action", "Swap Panes"));
    auto *swapTabs = actions->addAction(QStringLiteral("swap_current_tabs"), this, &MainWindow::swapCurrentTabs);
    swapTabs->setText(i18nc("@action", "Swap Current Tabs"));
    auto *swapSides = actions->addAction(QStringLiteral("swap_pane_sides"), this, &MainWindow::swapPaneSides);
    swapSides->setText(i18nc("@action", "Swap Pane Sides"));
    auto *movePane = actions->addAction(QStringLiteral("move_second_pane"), this, &MainWindow::moveSecondPaneToPreferredDestination);
    movePane->setText(i18nc("@action", "Move Second Pane"));
    auto *movePaneToTab = actions->addAction(QStringLiteral("move_second_pane_to_tab"), this, &MainWindow::moveSecondPaneToTab);
    movePaneToTab->setText(i18nc("@action", "Move Second Pane to Tab"));
    auto *movePaneToWindow = actions->addAction(QStringLiteral("move_second_pane_to_window"), this, &MainWindow::popOutSecondPane);
    movePaneToWindow->setText(i18nc("@action", "Pop Out Second Pane"));
    auto *restorePane = actions->addAction(QStringLiteral("restore_second_pane"), this, &MainWindow::moveSecondPaneToDual);
    restorePane->setText(i18nc("@action", "Return Second Pane to Dual Layout"));
    restorePane->setEnabled(false);
    auto *saveSizes = actions->addAction(QStringLiteral("save_splitter_sizes"), this, &MainWindow::saveSplitterSizes);
    saveSizes->setText(i18nc("@action", "Save Pane and Panel Sizes"));
    auto *resetSizes = actions->addAction(QStringLiteral("reset_splitter_sizes"), this, &MainWindow::resetSplitterSizes);
    resetSizes->setText(i18nc("@action", "Reset Pane and Panel Sizes"));
    // Keyboard resizing works the same whether the divider is visible or hidden.
    auto *dividerBack = actions->addAction(QStringLiteral("move_pane_divider_back"), this, [this] {
        movePaneDivider(-1);
    });
    dividerBack->setText(i18nc("@action", "Move Pane Divider Left or Up"));
    actions->setDefaultShortcuts(dividerBack, {Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_Left, Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_Up});
    auto *dividerForward = actions->addAction(QStringLiteral("move_pane_divider_forward"), this, [this] {
        movePaneDivider(1);
    });
    dividerForward->setText(i18nc("@action", "Move Pane Divider Right or Down"));
    actions->setDefaultShortcuts(dividerForward, {Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_Right, Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_Down});
    auto *customizeCombinedPanel = actions->addAction(QStringLiteral("customize_combined_panel"), m_combinedPanel, &CombinedPanel::configureSections);
    customizeCombinedPanel->setText(i18nc("@action", "Customize Combined Panel…"));
    auto *tabPosition = new KSelectAction(i18nc("@action", "Active Pane Tab Bar Position"), this);
    tabPosition->setItems({i18nc("@item:inmenu tab bar position", "Above"),
                           i18nc("@item:inmenu tab bar position", "Below"),
                           i18nc("@item:inmenu tab bar position", "Left"),
                           i18nc("@item:inmenu tab bar position", "Right")});
    actions->addAction(QStringLiteral("tab_bar_position"), tabPosition);
    auto *tabsTogether = new KToggleAction(i18nc("@action", "Keep Stacked Tab Bars Together"), this);
    tabsTogether->setChecked(Settings::stackedTabBarsTogether());
    actions->addAction(QStringLiteral("stacked_tab_bars_together"), tabsTogether);
    connect(tabsTogether, &KToggleAction::toggled, this, [this](bool together) {
        Settings::setStackedTabBarsTogether(together);
        Settings::self()->save();
        updateTabBarPlacement();
    });
    connect(tabPosition, &KSelectAction::indexTriggered, this, [this, tabsTogether](int position) {
        if (isStackedLayout()) {
            if (tabsTogether->isChecked()) {
                tabsTogether->setChecked(false);
            }
            if (activePane() == m_left) {
                Settings::setStackedFirstTabPosition(position);
            } else {
                Settings::setStackedSecondTabPosition(position);
            }
        } else if (activePane() == m_left) {
            Settings::setHorizontalFirstTabPosition(position);
        } else {
            Settings::setHorizontalSecondTabPosition(position);
        }
        Settings::self()->save();
        updateTabBarPlacement();
    });
    auto *showTabClose = new KToggleAction(i18nc("@action", "Show Tab Close Buttons"), this);
    showTabClose->setChecked(Settings::showTabCloseButtons());
    actions->addAction(QStringLiteral("show_tab_close_buttons"), showTabClose);
    connect(showTabClose, &KToggleAction::toggled, this, [this](bool shown) {
        Settings::setShowTabCloseButtons(shown);
        Settings::self()->save();
        m_left->reloadTabSettings();
        m_right->reloadTabSettings();
    });

    auto *dimInactive = new KToggleAction(i18nc("@action", "Dim Inactive Pane"), this);
    dimInactive->setChecked(Settings::dimInactivePane());
    actions->addAction(QStringLiteral("dim_inactive_pane"), dimInactive);
    connect(dimInactive, &QAction::triggered, this, [this](bool enabled) {
        Settings::setDimInactivePane(enabled);
        Settings::self()->save();
        m_left->setInactiveDimming(enabled, Settings::inactivePaneOpacity());
        m_right->setInactiveDimming(enabled, Settings::inactivePaneOpacity());
    });

    m_terminalPanelAction = new KToggleAction(i18nc("@action", "Show Embedded Terminal"), this);
    actions->addAction(QStringLiteral("show_embedded_terminal"), m_terminalPanelAction);
    connect(m_terminalPanel, &TerminalPanel::closeRequested, this, [this] {
        m_terminalPanelAction->setChecked(false);
    });
    connect(m_terminalPanelAction, &KToggleAction::toggled, this, [this](bool shown) {
        if (shown) {
            m_terminalPanel->start(activePane()->url());
        }
        updateTerminalPanelVisibility();
    });
    QAction *focusTerminalAction = actions->addAction(QStringLiteral("focus_terminal"), this, &MainWindow::focusTerminal);
    focusTerminalAction->setText(i18nc("@action", "Focus Embedded Terminal"));
    focusTerminalAction->setToolTip(
        i18nc("@info:tooltip", "Show the embedded terminal and type in it; if the terminal has the keyboard focus, go back to the active pane"));
    focusTerminalAction->setIcon(SymbolicIcon::fromTheme(QStringLiteral("utilities-terminal")));
    QAction *terminalHistory = actions->addAction(QStringLiteral("search_terminal_history"), this, [this] {
        const int sessions = m_terminalPanel->sessionCount();
        if (!m_terminalPanelAction->isChecked()) {
            m_terminalPanelAction->setChecked(true);
        }
        // Showing the panel may have opened a session for the search already.
        m_terminalPanel->searchHistory(activePane()->url(), m_terminalPanel->sessionCount() > sessions);
    });
    terminalHistory->setText(i18nc("@action", "Search Command History with Atuin"));
    terminalHistory->setToolTip(i18nc("@info:tooltip", "Open Atuin's command history search in the embedded terminal without running the chosen command"));
    terminalHistory->setIcon(QIcon::fromTheme(QStringLiteral("view-history")));

    m_searchPanelAction = new KToggleAction(i18nc("@action", "Show Search Panel"), this);
    m_searchPanelAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-find")));
    actions->addAction(QStringLiteral("show_search_panel"), m_searchPanelAction);
    connect(m_searchPanel, &SearchPanel::closeRequested, this, [this] {
        m_searchPanelAction->setChecked(false);
    });
    connect(m_searchPanelAction, &KToggleAction::toggled, this, [this](bool shown) {
        SearchWidget *search = m_searchPanel->searchWidget();
        if (shown && search->folder().isEmpty()) {
            search->setFolder(activePane()->url());
        }
        const bool hadFocus = m_searchPanel->isAncestorOf(QApplication::focusWidget());
        m_searchPanel->setVisible(shown);
        toggleLayout(m_dualLayoutAction->isChecked());
        restoreSplitterSizes(false, true, true);
        const auto state = KSharedConfig::openStateConfig();
        state->group(QStringLiteral("SearchPanel")).writeEntry("Visible", shown);
        state->sync();
        if (shown && !m_restoringSearchPanel) {
            search->focusQuery();
        } else if (!shown && hadFocus) {
            activePane()->view()->currentView()->setFocus();
        }
    });

    auto *toolbarSearch = new SearchToolbarAction(this);
    actions->addAction(QStringLiteral("search_toolbar"), toolbarSearch);
    connect(toolbarSearch, &SearchToolbarAction::searchRequested, this, &MainWindow::searchFromToolbar);
    toolbarSearch->setSavedSearchProvider([this] {
        return m_searchPanel->searchWidget()->savedSearchNames();
    });
    toolbarSearch->setHistoryProvider([this] {
        return m_searchPanel->searchWidget()->history();
    });
    connect(toolbarSearch, &SearchToolbarAction::savedSearchRequested, this, [this](const QString &name) {
        if (!m_searchPanel->searchWidget()->isRunning() && m_yaziSearchPane) {
            m_yaziSearchPane->yaziColumns()->clearSearchStatus();
            m_yaziSearchPane = nullptr;
        }
        if (Settings::toolbarSearchOpensPanel()) {
            m_searchPanelAction->setChecked(true);
        }
        presentToolbarSearch(m_searchPanel->searchWidget()->runSavedSearch(name));
    });
    connect(toolbarSearch, &SearchToolbarAction::resultsRequested, this, [this] {
        SearchWidget *search = m_searchPanel->searchWidget();
        if (!m_searchPanelAction->isChecked()) {
            // Results found while the panel stayed hidden are one key away.
            if (!search->hasResults()) {
                return;
            }
            m_searchPanelAction->setChecked(true);
        }
        if (!search->focusResults()) {
            search->focusQuery();
        }
    });
    connect(toolbarSearch, &SearchToolbarAction::escapePressed, this, [this] {
        SearchWidget *search = m_searchPanel->searchWidget();
        if (search->isRunning()) {
            search->cancel();
        } else {
            activePane()->view()->currentView()->setFocus(Qt::OtherFocusReason);
        }
    });
    connect(m_searchPanel->searchWidget(), &SearchWidget::dismissRequested, this, [this] {
        activePane()->view()->currentView()->setFocus(Qt::OtherFocusReason);
    });
    connect(m_searchPanel->searchWidget(), &SearchWidget::finished, this, [this](const QString &status) {
        reportYaziSearch(status, false);
        if (std::exchange(m_reportToolbarSearch, false) && !m_searchPanelAction->isChecked()) {
            showStatusMessage(
                i18nc("@info:status %1 is the search outcome", "%1 Press Down in the search field or show the search panel to see the results.", status));
        }
    });
    connect(m_searchPanel->searchWidget(), &SearchWidget::started, this, [this] {
        reportYaziSearch(QString(), true);
    });
    connect(m_searchPanel->searchWidget(), &SearchWidget::progressChanged, this, [this](const QString &status) {
        reportYaziSearch(status, true);
    });
    connect(m_searchPanel->searchWidget(), &SearchWidget::startRejected, this, [this](const QString &message) {
        reportYaziSearch(message, false);
    });

    m_linkedPanesAction = new KToggleAction(i18nc("@action", "Link Pane Navigation"), this);
    actions->addAction(QStringLiteral("link_panes"), m_linkedPanesAction);
    connect(m_linkedPanesAction, &KToggleAction::toggled, this, [this](bool linked) {
        if (!linked) {
            return;
        }
        if (!isDualLayout() || m_left->isHidden() || m_right->isHidden()) {
            m_linkedPanesAction->setChecked(false);
            showStatusMessage(i18nc("@info:status", "Show both panes before linking their navigation."));
            return;
        }
        const auto result = LinkedNavigation::target(m_left->url(), m_right->url(), m_left->url());
        if (!result.error.isEmpty()) {
            m_linkedPanesAction->setChecked(false);
            showStatusMessage(result.error);
            return;
        }
        m_linkedLeftRoot = m_left->url();
        m_linkedRightRoot = m_right->url();
        showStatusMessage(i18nc("@info:status", "Pane navigation linked from the current folders."));
    });

    QAction *showPlaces = m_placesDock->toggleViewAction();
    showPlaces->setText(i18nc("@action", "Show Places Panel"));
    showPlaces->setIcon(QIcon::fromTheme(QStringLiteral("compass")));
    actions->addAction(QStringLiteral("show_places"), showPlaces);
    actions->setDefaultShortcut(showPlaces, Qt::Key_F9);
    connect(showPlaces, &QAction::triggered, this, &MainWindow::rememberPlacesShown);
    m_placesDock->installEventFilter(this);

    m_viewModeAction = actions->addAction(QStringLiteral("toggle_view_mode"), this, &MainWindow::toggleViewMode);
    m_viewModeAction->setText(i18nc("@action", "Icons View"));
    m_viewModeAction->setIcon(SymbolicIcon::fromTheme(QStringLiteral("view-list-icons")));
    actions->setDefaultShortcut(m_viewModeAction, Qt::CTRL | Qt::SHIFT | Qt::Key_V);
    QAction *compactView = actions->addAction(QStringLiteral("compact_view"), this, [this] {
        activePane()->view()->setViewMode(FolderView::ViewMode::Compact);
        updateActions();
    });
    compactView->setText(i18nc("@action", "Compact View"));
    compactView->setIcon(QIcon::fromTheme(QStringLiteral("view-list-tree")));
    QAction *cardView = actions->addAction(QStringLiteral("card_view"), this, [this] {
        activePane()->view()->setViewMode(FolderView::ViewMode::Cards);
        updateActions();
    });
    cardView->setText(i18nc("@action", "Card View"));
    cardView->setIcon(QIcon::fromTheme(QStringLiteral("view-grid"), QIcon::fromTheme(QStringLiteral("view-list-icons"))));

    m_yaziModeAction =
        new KToggleAction(SymbolicIcon::wrap(QIcon::fromTheme(QStringLiteral("view-file-columns"), QIcon::fromTheme(QStringLiteral("view-split-left-right")))),
                          i18nc("@action", "Yazi Mode"),
                          this);
    m_yaziModeAction->setToolTip(i18nc("@info:tooltip", "Show the active pane's folder in columns with its parent folder"));
    actions->addAction(QStringLiteral("toggle_yazi_mode"), m_yaziModeAction);
    connect(m_yaziModeAction, &KToggleAction::triggered, this, [this](bool enabled) {
        activePane()->setYaziMode(enabled);
    });
    m_yaziPreviewAction = new KToggleAction(SymbolicIcon::fromTheme(QStringLiteral("document-preview")), i18nc("@action", "Yazi Preview"), this);
    m_yaziPreviewAction->setToolTip(i18nc("@info:tooltip", "Show a preview of the focused item in the active pane's Yazi mode"));
    actions->addAction(QStringLiteral("toggle_yazi_preview"), m_yaziPreviewAction);
    connect(m_yaziPreviewAction, &KToggleAction::triggered, this, [this](bool visible) {
        activePane()->setYaziPreview(visible);
    });
    m_yaziPreviewInformationAction =
        new KToggleAction(SymbolicIcon::fromTheme(QStringLiteral("documentinfo")), i18nc("@action", "File Information in Yazi Preview"), this);
    m_yaziPreviewInformationAction->setToolTip(
        i18nc("@info:tooltip", "Show the focused item's file information instead of its preview in the active pane's Yazi mode"));
    actions->addAction(QStringLiteral("toggle_yazi_preview_information"), m_yaziPreviewInformationAction);
    connect(m_yaziPreviewInformationAction, &KToggleAction::triggered, this, [this](bool information) {
        activePane()->setYaziPreviewContent(information ? YaziMode::PreviewContent::Information : YaziMode::PreviewContent::Preview);
    });
    m_yaziParentColumnAction = new KToggleAction(SymbolicIcon::fromTheme(QStringLiteral("go-parent-folder")), i18nc("@action", "Yazi Parent Column"), this);
    m_yaziParentColumnAction->setToolTip(i18nc("@info:tooltip", "Show the parent folder before the current folder in the active pane's Yazi mode"));
    actions->addAction(QStringLiteral("toggle_yazi_parent_column"), m_yaziParentColumnAction);
    connect(m_yaziParentColumnAction, &KToggleAction::triggered, this, [this](bool visible) {
        activePane()->setYaziParentColumn(visible);
    });
    m_yaziModeStatusButton = new QToolButton(this);
    m_yaziModeStatusButton->setObjectName(QStringLiteral("yaziModeStatusButton"));
    m_yaziModeStatusButton->setAutoRaise(true);
    m_yaziModeStatusButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_yaziModeStatusButton->setDefaultAction(m_yaziModeAction);
    m_yaziModeStatusButton->setAccessibleName(i18nc("@action:button accessible name", "Yazi mode"));
    m_yaziModeStatusButton->setAccessibleDescription(i18nc("@info accessible description", "Turns Yazi mode on or off in the active pane"));
    statusBar()->addPermanentWidget(m_yaziModeStatusButton);
    m_yaziVimKeysAction = new KToggleAction(i18nc("@action", "Vim-Style Keys in Yazi Mode"), this);
    m_yaziVimKeysAction->setToolTip(
        i18nc("@info:tooltip", "Navigate Yazi mode's columns with Vim-style keys such as h, j, k, l and gg as well as the standard keys"));
    m_yaziVimKeysAction->setChecked(Settings::yaziKeyPreset() == Settings::EnumYaziKeyPreset::Vim);
    actions->addAction(QStringLiteral("toggle_yazi_vim_keys"), m_yaziVimKeysAction);
    connect(m_yaziVimKeysAction, &KToggleAction::triggered, this, [this](bool vim) {
        Settings::setYaziKeyPreset(vim ? Settings::EnumYaziKeyPreset::Vim : Settings::EnumYaziKeyPreset::Standard);
        Settings::self()->save();
        for (Pane *pane : {m_left, m_right}) {
            pane->reloadYaziKeySettings();
        }
    });
    m_vimNavigationAction = new KToggleAction(i18nc("@action", "Vim-Style Navigation"), this);
    m_vimNavigationAction->setToolTip(
        i18nc("@info:tooltip", "Move through the file views of panes that aren't in Yazi mode with Vim-style keys such as j, k, gg and /"));
    m_vimNavigationAction->setChecked(Settings::vimNavigation());
    actions->addAction(QStringLiteral("toggle_vim_navigation"), m_vimNavigationAction);
    connect(m_vimNavigationAction, &KToggleAction::triggered, this, [this](bool enabled) {
        Settings::setVimNavigation(enabled);
        Settings::self()->save();
        for (Pane *pane : {m_left, m_right}) {
            pane->reloadVimNavigationSetting();
        }
        showStatusMessage(enabled ? i18nc("@info:status", "Vim-style navigation on: j and k move, h opens the parent folder, l opens, / filters")
                                  : i18nc("@info:status", "Vim-style navigation off"));
    });
    const auto addYaziColumnAction = [this, actions](const QString &name, const QString &text, const QString &toolTip, YaziMode::Command command) {
        QAction *action = actions->addAction(name, this, [this, command] {
            activePane()->runYaziCommand(command);
        });
        action->setText(text);
        action->setToolTip(toolTip);
        m_yaziColumnActions.append(action);
    };
    addYaziColumnAction(QStringLiteral("yazi_focus_parent_column"),
                        i18nc("@action", "Focus Parent Column"),
                        i18nc("@info:tooltip", "Move the keyboard focus to the parent folder's column in the active pane's Yazi mode"),
                        YaziMode::Command::FocusParent);
    addYaziColumnAction(QStringLiteral("yazi_focus_current_column"),
                        i18nc("@action", "Focus Current Column"),
                        i18nc("@info:tooltip", "Move the keyboard focus to the current folder's column in the active pane's Yazi mode"),
                        YaziMode::Command::FocusCurrent);
    addYaziColumnAction(QStringLiteral("yazi_focus_preview"),
                        i18nc("@action", "Focus Preview Column"),
                        i18nc("@info:tooltip", "Move the keyboard focus to the preview column in the active pane's Yazi mode"),
                        YaziMode::Command::FocusPreview);
    addYaziColumnAction(QStringLiteral("yazi_toggle_selection"),
                        i18nc("@action", "Toggle Selection of Focused Item"),
                        i18nc("@info:tooltip",
                              "Select or deselect the focused item in the active pane's Yazi mode; the selection stays as the focus moves until it is cleared"),
                        YaziMode::Command::ToggleSelection);
    addYaziColumnAction(
        QStringLiteral("yazi_visual_selection"),
        i18nc("@action", "Range Selection"),
        i18nc("@info:tooltip", "Start or end selecting every item between the focused item and where the range started, in the active pane's Yazi mode"),
        YaziMode::Command::VisualSelection);

    QAction *formatSource = actions->addAction(QStringLiteral("folder_format_source"));
    formatSource->setEnabled(false);
    QAction *openFormatSource = actions->addAction(QStringLiteral("open_folder_format_source"), this, [this] {
        const QString source = activePane()->view()->formatRuleSource();
        if (!source.isEmpty()) {
            activePane()->setUrl(QUrl::fromLocalFile(source));
        }
    });
    openFormatSource->setText(i18nc("@action", "Go to Folder Format Source"));
    QAction *clearFormat = actions->addAction(QStringLiteral("clear_folder_format"), this, [this] {
        if (KMessageBox::warningContinueCancel(
                this,
                i18nc("@info", "Remove this folder's saved format? Other supported metadata is kept, but unsupported fields may be discarded."),
                i18nc("@title:window", "Remove Folder Format"))
            == KMessageBox::Continue) {
            activePane()->view()->clearLocalFolderFormat();
            updateActions();
        }
    });
    clearFormat->setText(i18nc("@action", "Remove This Folder's Format…"));
    QAction *formatLock = actions->addAction(QStringLiteral("lock_folder_format"));
    formatLock->setText(i18nc("@action", "Lock Current Pane Format"));
    formatLock->setCheckable(true);
    connect(formatLock, &QAction::triggered, this, [this](bool locked) {
        activePane()->view()->setFormatLocked(locked);
        updateActions();
    });
    QAction *formatDescendants = actions->addAction(QStringLiteral("inherit_folder_format"));
    formatDescendants->setText(i18nc("@action", "Apply Folder Format to Descendants"));
    formatDescendants->setCheckable(true);
    connect(formatDescendants, &QAction::triggered, this, [this](bool include) {
        activePane()->view()->setFormatIncludesDescendants(include);
        updateActions();
    });
    QAction *folderSizes = actions->addAction(QStringLiteral("calculate_folder_sizes"), this, [this] {
        activePane()->view()->calculateFolderSizes();
    });
    folderSizes->setText(i18nc("@action", "Calculate Folder Sizes"));
    QAction *automaticSizes = actions->addAction(QStringLiteral("automatic_folder_sizes"));
    automaticSizes->setText(i18nc("@action", "Automatically Calculate Folder Sizes for This Format"));
    automaticSizes->setCheckable(true);
    connect(automaticSizes, &QAction::triggered, this, [this](bool enabled) {
        activePane()->view()->setAutomaticFolderSizes(enabled);
        updateActions();
    });
    QAction *defaultSizePolicy = actions->addAction(QStringLiteral("default_folder_size_policy"), this, [this] {
        activePane()->view()->clearFolderSizePolicy();
        updateActions();
    });
    defaultSizePolicy->setText(i18nc("@action", "Use Location Default for This Format's Folder Sizes"));
    const auto addSizeDefault = [this, actions](const QString &name, const QString &text, bool local) {
        QAction *action = actions->addAction(name);
        action->setText(text);
        action->setCheckable(true);
        connect(action, &QAction::triggered, this, [this, local](bool enabled) {
            if (local) {
                Settings::setAutomaticLocalFolderSizes(enabled);
            } else {
                Settings::setAutomaticRemoteFolderSizes(enabled);
            }
            Settings::self()->save();
            m_automaticLocalSizeDefault = Settings::automaticLocalFolderSizes();
            m_automaticRemoteSizeDefault = Settings::automaticRemoteFolderSizes();
            m_left->view()->refreshFolderSizePolicy();
            m_right->view()->refreshFolderSizePolicy();
            updateActions();
        });
    };
    addSizeDefault(QStringLiteral("automatic_local_folder_sizes"), i18nc("@action", "Automatically Calculate Local Folder Sizes by Default"), true);
    addSizeDefault(QStringLiteral("automatic_remote_folder_sizes"), i18nc("@action", "Automatically Calculate Remote Folder Sizes by Default"), false);

    m_selectionModeAction = actions->addAction(QStringLiteral("selection_mode"));
    m_selectionModeAction->setText(i18nc("@action", "Selection Mode"));
    m_selectionModeAction->setCheckable(true);
    connect(m_selectionModeAction, &QAction::triggered, this, [this](bool enabled) {
        activePane()->view()->setSelectionMode(enabled);
    });
    m_selectionMarkersAction = actions->addAction(QStringLiteral("selection_markers"));
    m_selectionMarkersAction->setText(i18nc("@action", "Show Selection Markers"));
    m_selectionMarkersAction->setCheckable(true);
    connect(m_selectionMarkersAction, &QAction::triggered, this, [this](bool enabled) {
        activePane()->view()->setSelectionMarkersVisible(enabled);
    });

    m_hiddenFilesAction = new KToggleAction(QIcon::fromTheme(QStringLiteral("view-hidden")), i18nc("@action", "Show Hidden Files"), this);
    actions->addAction(QStringLiteral("show_hidden_files"), m_hiddenFilesAction);
    actions->setDefaultShortcut(m_hiddenFilesAction, Qt::CTRL | Qt::Key_H);
    connect(m_hiddenFilesAction, &KToggleAction::toggled, this, &MainWindow::setShowHiddenFiles);

    m_previewsAction = new KToggleAction(QIcon::fromTheme(QStringLiteral("view-preview")), i18nc("@action", "Previews"), this);
    m_previewsAction->setChecked(true);
    actions->addAction(QStringLiteral("show_previews"), m_previewsAction);
    connect(m_previewsAction, &KToggleAction::toggled, this, &MainWindow::setPreviewsEnabled);

    m_previewPanelAction = new KToggleAction(SymbolicIcon::fromTheme(QStringLiteral("document-preview")), i18nc("@action", "Preview Panel"), this);
    actions->addAction(QStringLiteral("show_preview_panel"), m_previewPanelAction);
    actions->setDefaultShortcut(m_previewPanelAction, Qt::Key_F3);
    connect(m_previewPanelAction, &KToggleAction::toggled, this, [this](bool shown) {
        if (!shown && !m_previewPanel->queryClose()) {
            // Keep the panel and its unsaved changes; re-checking restores a
            // dock that its close button already hid.
            m_previewPanelAction->setChecked(true);
            return;
        }
        toggleLayout(m_dualLayoutAction->isChecked());
        if (shown) {
            restorePanelDockSize(m_previewDock, QStringLiteral("PreviewDock"));
            updatePreviewPanel();
        } else {
            m_previewPanel->clear();
        }
    });
    connect(m_previewPanel, &PreviewPanel::closeRequested, m_previewPanelAction, &QAction::trigger);
    QAction *focusPreview = actions->addAction(QStringLiteral("focus_preview_panel"), this, &MainWindow::focusPreviewPanel);
    focusPreview->setText(i18nc("@action", "Focus Preview Panel"));
    focusPreview->setToolTip(
        i18nc("@info:tooltip", "Show the preview panel and move the keyboard focus into it; if the panel has the focus, go back to the active pane"));
    focusPreview->setIcon(SymbolicIcon::fromTheme(QStringLiteral("document-preview")));
    connect(m_previewPanel, &PreviewPanel::placementRequested, this, [this](PreviewPanel::Placement placement) {
        Settings::setPreviewPlacement(static_cast<Settings::EnumPreviewPlacement::type>(placement));
        Settings::self()->save();
        setPreviewPlacement(placement);
    });
    connect(m_previewDock, &QDockWidget::visibilityChanged, this, [this] {
        if (!m_movingPreview && m_previewDock->isHidden() && m_previewPlacement != PreviewPanel::Placement::Pane) {
            savePanelDockSize(m_previewDock, QStringLiteral("PreviewDock"));
            m_previewPanelAction->setChecked(false);
        }
    });
    connect(m_previewPanel, &PreviewPanel::galleryRequested, this, &MainWindow::openGallery);
    connect(m_previewPanel, &PreviewPanel::imageRequested, this, [this](const QUrl &url) {
        activePane()->view()->selectUrls({url});
    });
    connect(m_previewPanel, &PreviewPanel::openRequested, this, [this] {
        const KFileItemList items = activePane()->view()->selectedItems();
        if (items.size() == 1) {
            FileOperations::open(items.first(), this);
        }
    });
    connect(m_previewPanel, &PreviewPanel::editorRequested, this, [this](const QUrl &url, const QString &text, bool modified) {
        if (m_editor && m_editor->document()->url() == url && m_editor->document()->isModified()) {
            KMessageBox::error(this, i18nc("@info", "The editor panel already has unsaved changes to this file. Save or close them first."));
            return;
        }
        // Free the editor panel first, so the user can still keep editing in
        // the preview, then close the preview's copy before the editor panel
        // opens the file, so their swap files don't collide.
        if (m_editor && m_editor->document()->url() != url && !m_editor->closeDocument()) {
            return;
        }
        m_previewPanel->endEditing();
        if (!showEditor(url)) {
            showEditor({});
            if (modified) {
                m_editor->document()->setText(text);
                KMessageBox::error(this,
                                   i18nc("@info", "Could not open %1 in the editor panel. Your changes are in an untitled document.", url.toDisplayString()));
            }
            return;
        }
        if (modified) {
            m_editor->document()->setText(text);
        }
    });
    QAction *informationAction = actions->addAction(QStringLiteral("show_information_panel"));
    informationAction->setText(i18nc("@action", "Information Panel"));
    informationAction->setCheckable(true);
    connect(informationAction, &QAction::triggered, this, [this] {
        m_informationDock->setVisible(!m_informationDock->isVisible());
    });
    connect(m_informationDock, &QDockWidget::visibilityChanged, informationAction, &QAction::setChecked);
    QAction *customizeToolbar = actions->addAction(QStringLiteral("customize_main_toolbar"));
    customizeToolbar->setText(i18nc("@action", "Customize Main Toolbar"));
    customizeToolbar->setCheckable(true);
    QAction *resetToolbarCustomization = actions->addAction(QStringLiteral("reset_toolbar_customization"), this, [this] {
        if (QMessageBox::question(this,
                                  i18nc("@title:dialog", "Restore Default Toolbars"),
                                  i18nc("@label:dialog", "Restore the main toolbar, pane toolbars and hamburger menu to their default contents and order?"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No)
            == QMessageBox::Yes) {
            m_toolbarCustomizer->resetToDefaults();
        }
    });
    resetToolbarCustomization->setText(i18nc("@action", "Restore Default Toolbars"));
    QAction *gallery = actions->addAction(QStringLiteral("open_gallery"), this, &MainWindow::openGallery);
    gallery->setText(i18nc("@action", "Open Gallery"));
    gallery->setIcon(SymbolicIcon::fromTheme(QStringLiteral("view-media-preview")));
    actions->setDefaultShortcut(gallery, Qt::Key_Space);

    QAction *reload = KStandardAction::redisplay(
        this,
        [this] {
            activePane()->reload();
        },
        actions);
    // F5 is copy-to-other-pane here, so reload must not keep the F5 default.
    actions->setDefaultShortcut(reload, Qt::CTRL | Qt::Key_R);

    // Go
    QAction *back = KStandardAction::back(
        this,
        [this] {
            activePane()->goBack();
        },
        actions);
    actions->setDefaultShortcut(back, Qt::ALT | Qt::Key_Left);
    QAction *forward = KStandardAction::forward(
        this,
        [this] {
            activePane()->goForward();
        },
        actions);
    actions->setDefaultShortcut(forward, Qt::ALT | Qt::Key_Right);
    QAction *up = KStandardAction::up(
        this,
        [this] {
            activePane()->goUp();
        },
        actions);
    actions->setDefaultShortcuts(up, {Qt::Key_Backspace, Qt::ALT | Qt::Key_Up});
    KStandardAction::home(
        this,
        [this] {
            activePane()->setUrl(Settings::homeUrl());
        },
        actions);

    m_jumpBackAction = actions->addAction(QStringLiteral("jump_back"), this, &MainWindow::jumpBack);
    m_jumpBackAction->setText(i18nc("@action", "Jump Back"));
    m_jumpBackAction->setIcon(QIcon::fromTheme(QStringLiteral("go-jump")));
    m_jumpBackAction->setEnabled(false);

    m_popularFoldersMenu = new QMenu(i18nc("@menu", "Popular Folders"), this);
    actions->addAction(QStringLiteral("popular_folders"), m_popularFoldersMenu->menuAction());
    connect(m_popularFoldersMenu, &QMenu::aboutToShow, this, &MainWindow::populatePopularFolders);

    auto *mountPoints = new KActionMenu(SymbolicIcon::fromTheme(QStringLiteral("drive-harddisk")), i18nc("@action", "Mount Points"), this);
    mountPoints->setToolTip(i18nc("@info:tooltip", "Open, mount or manage file systems"));
    mountPoints->setPopupMode(QToolButton::InstantPopup);
    m_mountPointsMenu = mountPoints->menu();
    actions->addAction(QStringLiteral("mount_points"), mountPoints);
    connect(m_mountPointsMenu, &QMenu::aboutToShow, this, &MainWindow::populateMountPoints);

    auto *remoteConnections = new KActionMenu(SymbolicIcon::fromTheme(QStringLiteral("folder-network")), i18nc("@action", "Remote Connections"), this);
    remoteConnections->setToolTip(i18nc("@info:tooltip", "Open a saved remote connection or the local network"));
    remoteConnections->setPopupMode(QToolButton::InstantPopup);
    m_remoteConnectionsMenu = remoteConnections->menu();
    actions->addAction(QStringLiteral("remote_connections"), remoteConnections);
    connect(m_remoteConnectionsMenu, &QMenu::aboutToShow, this, &MainWindow::populateRemoteConnections);
    QAction *manageRemote = actions->addAction(QStringLiteral("manage_remote_connections"), this, &MainWindow::showRemoteConnections);
    manageRemote->setText(i18nc("@action", "Manage Remote Connections…"));
    manageRemote->setToolTip(i18nc("@info:tooltip", "Add, edit or remove saved SSH, WebDAV, SMB and FTP connections"));
    manageRemote->setIcon(SymbolicIcon::fromTheme(QStringLiteral("network-server")));
    auto *cloudStorage = new KActionMenu(SymbolicIcon::fromTheme(QStringLiteral("folder-cloud")), i18nc("@action", "Cloud Storage"), this);
    cloudStorage->setToolTip(i18nc("@info:tooltip", "Open a cloud remote configured in rclone"));
    cloudStorage->setPopupMode(QToolButton::InstantPopup);
    m_cloudStorageMenu = cloudStorage->menu();
    actions->addAction(QStringLiteral("cloud_storage"), cloudStorage);
    connect(m_cloudStorageMenu, &QMenu::aboutToShow, this, [this] {
        populateCloudStorage();
        rcloneMounts()->listRemotes();
    });

    auto *onlineAccounts = new KActionMenu(SymbolicIcon::fromTheme(QStringLiteral("internet-services")), i18nc("@action", "Online Accounts"), this);
    onlineAccounts->setToolTip(i18nc("@info:tooltip", "Open the files of an account configured in KDE Online Accounts"));
    onlineAccounts->setPopupMode(QToolButton::InstantPopup);
    m_onlineAccountsMenu = onlineAccounts->menu();
    actions->addAction(QStringLiteral("online_accounts"), onlineAccounts);
    connect(m_onlineAccountsMenu, &QMenu::aboutToShow, this, &MainWindow::populateOnlineAccounts);
    QAction *manageAccounts = actions->addAction(QStringLiteral("manage_online_accounts"), this, [this] {
        const QStringList command = OnlineAccounts::managerCommand();
        if (command.isEmpty()) {
            showStatusMessage(i18nc("@info:status", "System Settings was not found. Install it to manage online accounts."));
            return;
        }
        auto *job = new KIO::CommandLauncherJob(command.first(), command.mid(1), this);
        job->start();
    });
    manageAccounts->setText(i18nc("@action", "Manage Online Accounts…"));
    manageAccounts->setToolTip(i18nc("@info:tooltip", "Add, remove or sign in to online accounts in System Settings"));
    manageAccounts->setIcon(SymbolicIcon::fromTheme(QStringLiteral("configure")));

    auto *nextcloud = new KActionMenu(SymbolicIcon::fromTheme(QStringLiteral("folder-cloud")), i18nc("@action", "Nextcloud"), this);
    nextcloud->setToolTip(i18nc("@info:tooltip", "Open the files of a Nextcloud account or add one"));
    nextcloud->setPopupMode(QToolButton::InstantPopup);
    m_nextcloudMenu = nextcloud->menu();
    actions->addAction(QStringLiteral("nextcloud"), nextcloud);
    connect(m_nextcloudMenu, &QMenu::aboutToShow, this, &MainWindow::populateNextcloud);
    QAction *manageNextcloud = actions->addAction(QStringLiteral("manage_nextcloud_accounts"), this, &MainWindow::showNextcloudAccounts);
    manageNextcloud->setText(i18nc("@action", "Manage Nextcloud Accounts…"));
    manageNextcloud->setToolTip(i18nc("@info:tooltip", "Sign in to, check or remove Nextcloud accounts"));
    manageNextcloud->setIcon(SymbolicIcon::fromTheme(QStringLiteral("configure")));

    auto *plasmaVaults = new KActionMenu(SymbolicIcon::fromTheme(QStringLiteral("folder-encrypted")), i18nc("@action", "Plasma Vaults"), this);
    plasmaVaults->setToolTip(i18nc("@info:tooltip", "Open, close or create an encrypted vault with Plasma Vault"));
    plasmaVaults->setPopupMode(QToolButton::InstantPopup);
    actions->addAction(QStringLiteral("plasma_vaults"), plasmaVaults);
    setVaultClient(new DBusVaultClient(this));

    QAction *network = actions->addAction(QStringLiteral("network_locations"), this, [this] {
        const QUrl url = RemoteProfiles::networkUrl();
        if (url.isEmpty()) {
            showStatusMessage(i18nc("@info:status", "No KIO worker lists the local network. Install kio-extras."));
            return;
        }
        activePane()->setUrl(url);
    });
    network->setText(i18nc("@action", "Network"));
    network->setToolTip(i18nc("@info:tooltip", "Browse shares and services discovered on the local network"));
    network->setIcon(SymbolicIcon::fromTheme(QStringLiteral("network-workgroup")));

    QAction *editUrl = actions->addAction(QStringLiteral("edit_location"), this, [this] {
        activePane()->editUrl();
    });
    editUrl->setText(i18nc("@action", "Edit Location"));
    editUrl->setIcon(QIcon::fromTheme(QStringLiteral("edit-rename")));
    actions->setDefaultShortcut(editUrl, Qt::CTRL | Qt::Key_L);

    QAction *switchPane = actions->addAction(QStringLiteral("switch_pane"), this, [this] {
        setActivePane(inactivePane());
    });
    switchPane->setText(i18nc("@action", "Switch Pane"));
    actions->setDefaultShortcut(switchPane, Qt::Key_Tab);
    QAction *focusFirstPane = actions->addAction(QStringLiteral("focus_first_pane"), this, [this] {
        focusPane(m_left);
    });
    focusFirstPane->setText(i18nc("@action", "Focus First Pane"));
    focusFirstPane->setToolTip(i18nc("@info:tooltip", "Make the first pane, on the left or at the top, active and move the keyboard focus to it"));
    QAction *focusSecondPane = actions->addAction(QStringLiteral("focus_second_pane"), this, [this] {
        focusPane(m_right);
    });
    focusSecondPane->setText(i18nc("@action", "Focus Second Pane"));
    focusSecondPane->setToolTip(i18nc("@info:tooltip", "Make the second pane, on the right or at the bottom, active and move the keyboard focus to it"));
    QAction *keyboardHints = actions->addAction(QStringLiteral("keyboard_hints"), this, &MainWindow::showKeyboardHints);
    keyboardHints->setText(i18nc("@action", "Keyboard Hints"));
    keyboardHints->setIcon(SymbolicIcon::fromTheme(QStringLiteral("input-keyboard")));
    keyboardHints->setToolTip(
        i18nc("@info:tooltip", "Label the visible files and folders, location segments, tabs and places with keys; type a label to go there"));
    // No other action, text field or Yazi mode preset uses it.
    actions->setDefaultShortcut(keyboardHints, Qt::CTRL | Qt::SHIFT | Qt::Key_J);

    // Tabs
    QAction *newTab = actions->addAction(QStringLiteral("new_tab"), this, [this] {
        activePane()->newTab();
    });
    newTab->setText(i18nc("@action", "New Tab"));
    newTab->setIcon(QIcon::fromTheme(QStringLiteral("tab-new")));
    actions->setDefaultShortcut(newTab, Qt::CTRL | Qt::Key_T);

    QAction *closeTab = actions->addAction(QStringLiteral("close_tab"), this, [this] {
        activePane()->closeCurrentTab();
    });
    closeTab->setText(i18nc("@action", "Close Tab"));
    closeTab->setIcon(QIcon::fromTheme(QStringLiteral("tab-close")));
    actions->setDefaultShortcut(closeTab, Qt::CTRL | Qt::Key_W);

    QAction *reopenTab = actions->addAction(QStringLiteral("reopen_closed_tab"), this, [this] {
        activePane()->tabs()->reopenClosedTab();
    });
    reopenTab->setText(i18nc("@action", "Reopen Closed Tab"));
    reopenTab->setIcon(QIcon::fromTheme(QStringLiteral("edit-undo")));
    actions->setDefaultShortcut(reopenTab, Qt::CTRL | Qt::SHIFT | Qt::Key_T);

    QAction *nextTab = actions->addAction(QStringLiteral("next_tab"), this, [this] {
        activePane()->tabs()->activateNext();
    });
    nextTab->setText(i18nc("@action", "Next Tab"));
    actions->setDefaultShortcut(nextTab, Qt::CTRL | Qt::Key_Tab);

    QAction *previousTab = actions->addAction(QStringLiteral("previous_tab"), this, [this] {
        activePane()->tabs()->activatePrevious();
    });
    previousTab->setText(i18nc("@action", "Previous Tab"));
    actions->setDefaultShortcut(previousTab, Qt::CTRL | Qt::SHIFT | Qt::Key_Tab);

    QAction *lastTab = actions->addAction(QStringLiteral("activate_last_tab"), this, [this] {
        TabModel *tabs = activePane()->tabs();
        tabs->setCurrentIndex(tabs->count() - 1);
    });
    lastTab->setText(i18nc("@action", "Activate Last Tab"));
    lastTab->setIcon(SymbolicIcon::fromTheme(QStringLiteral("go-last-view")));

    for (int i = 1; i <= 9; ++i) {
        QAction *tabN = actions->addAction(QStringLiteral("activate_tab_%1").arg(i), this, [this, i] {
            activePane()->tabs()->setCurrentIndex(i - 1);
        });
        tabN->setText(i18nc("@action %1 is a tab number", "Activate Tab %1", i));
        actions->setDefaultShortcut(tabN, QKeySequence(Qt::CTRL | static_cast<Qt::Key>(Qt::Key_0 + i)));
    }

    QAction *compressAction = actions->addAction(QStringLiteral("compress"), this, &MainWindow::compress);
    compressAction->setText(i18nc("@action", "Compress…"));
    compressAction->setIcon(QIcon::fromTheme(QStringLiteral("archive-insert")));

    QAction *addToArchiveAction = actions->addAction(QStringLiteral("add_to_archive"), this, &MainWindow::addToArchive);
    addToArchiveAction->setText(i18nc("@action", "Add to Archive…"));
    addToArchiveAction->setToolTip(i18nc("@info:tooltip", "Add the selection to an existing archive with Ark"));
    addToArchiveAction->setIcon(QIcon::fromTheme(QStringLiteral("archive-insert")));

    QAction *extract = actions->addAction(QStringLiteral("extract_to_other_pane"), this, &MainWindow::extractToOtherPane);
    extract->setText(i18nc("@action", "Extract to Other Pane"));
    extract->setIcon(QIcon::fromTheme(QStringLiteral("archive-extract")));

    QAction *extractProtected = actions->addAction(QStringLiteral("extract_protected_archive"), this, &MainWindow::extractProtectedToOtherPane);
    extractProtected->setText(i18nc("@action", "Extract Password-Protected Archive to Other Pane…"));
    extractProtected->setIcon(QIcon::fromTheme(QStringLiteral("archive-extract")));

    QAction *testArchive = actions->addAction(QStringLiteral("test_archive_integrity"), this, &MainWindow::testArchiveIntegrity);
    testArchive->setText(i18nc("@action", "Test Archive Integrity…"));
    testArchive->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")));

    QAction *openInArk = actions->addAction(QStringLiteral("open_in_ark"), this, &MainWindow::openInArk);
    openInArk->setText(i18nc("@action", "Open in Ark"));
    openInArk->setToolTip(i18nc("@info:tooltip", "Open the archive in Ark to add, rename or remove its entries"));
    openInArk->setIcon(QIcon::fromTheme(QStringLiteral("ark")));

    QAction *mountImage = actions->addAction(QStringLiteral("mount_disk_image"), this, [this] {
        setDiskImageMounted(true);
    });
    mountImage->setText(i18nc("@action", "Mount Disk Image"));
    mountImage->setToolTip(i18nc("@info:tooltip", "Mount the selected ISO or disk image with Dolphin's Mount ISO plugin"));
    mountImage->setIcon(QIcon::fromTheme(QStringLiteral("media-mount")));

    QAction *unmountImage = actions->addAction(QStringLiteral("unmount_disk_image"), this, [this] {
        setDiskImageMounted(false);
    });
    unmountImage->setText(i18nc("@action", "Unmount Disk Image"));
    unmountImage->setIcon(QIcon::fromTheme(QStringLiteral("media-eject")));

    QAction *applicationPackage = actions->addAction(QStringLiteral("application_package"), this, &MainWindow::showApplicationPackage);
    applicationPackage->setText(i18nc("@action", "Application Package…"));
    applicationPackage->setToolTip(i18nc("@info:tooltip", "Show the selected AppImage, Flatpak or snap package and run, install, update or remove it"));
    applicationPackage->setIcon(QIcon::fromTheme(QStringLiteral("package-x-generic")));

    m_flatAction = new KToggleAction(QIcon::fromTheme(QStringLiteral("view-list-text")), i18nc("@action", "Show Flat (No Folders)"), this);
    actions->addAction(QStringLiteral("show_flat"), m_flatAction);
    actions->setDefaultShortcut(m_flatAction, Qt::CTRL | Qt::SHIFT | Qt::Key_F);
    connect(m_flatAction, &KToggleAction::triggered, this, &MainWindow::setFlat);

    QAction *filter = actions->addAction(QStringLiteral("show_filter_bar"), this, [this] {
        activePane()->showFilterBar();
    });
    filter->setText(i18nc("@action", "Filter…"));
    filter->setIcon(QIcon::fromTheme(QStringLiteral("view-filter")));
    actions->setDefaultShortcut(filter, Qt::CTRL | Qt::Key_F);
    QAction *clearFilter = actions->addAction(QStringLiteral("clear_filter"), this, [this] {
        if (!activePane()->clearFilter()) {
            showStatusMessage(i18nc("@info:status", "This pane has no filter to clear."));
        }
    });
    clearFilter->setText(i18nc("@action", "Clear Filter"));
    clearFilter->setToolTip(i18nc("@info:tooltip", "Show every item of the active pane again and close its filter bar"));
    clearFilter->setIcon(SymbolicIcon::fromTheme(QStringLiteral("edit-clear")));

    QAction *addFavorite = actions->addAction(QStringLiteral("add_to_favorites"), this, &MainWindow::addToFavorites);
    addFavorite->setText(i18nc("@action", "Add to Favorites"));
    addFavorite->setIcon(QIcon::fromTheme(QStringLiteral("bookmark-new")));
    actions->setDefaultShortcut(addFavorite, Qt::CTRL | Qt::Key_D);

    m_panelLayoutAction = new KSelectAction(QIcon::fromTheme(QStringLiteral("sidebar-show")), i18nc("@action", "Panels"), this);
    m_panelLayoutAction->setItems({i18nc("@item:inmenu panel layout", "Combined Panel"), i18nc("@item:inmenu panel layout", "Separate Places and Folders")});
    actions->addAction(QStringLiteral("panel_layout"), m_panelLayoutAction);
    connect(m_panelLayoutAction, &KSelectAction::indexTriggered, this, [this](int index) {
        setPanelLayout(index == 0 ? PanelLayout::Combined : PanelLayout::Separate);
    });

    m_tagging = new TaggingController(this,
                                      actions,
                                      {[this] {
                                           return activePane()->url();
                                       },
                                       [this] {
                                           return activePane()->view()->selectedItems();
                                       },
                                       [this](const QString &message) {
                                           showStatusMessage(message);
                                       }});
    connect(m_tagging, &TaggingController::libraryChanged, this, &MainWindow::reloadLibraryViews);
    connect(LibraryAutoRefresh::instance(), &LibraryAutoRefresh::refreshed, this, [this](const QString &root, const QString &message, bool changed) {
        if (changed) {
            reloadLibraryViews(root);
        }
        // Several windows share one refresher; only the one in use reports it.
        if (!message.isEmpty() && (isActiveWindow() || !QApplication::activeWindow())) {
            showStatusMessage(message);
        }
    });
    connect(m_tagPanel, &TagPanel::manageRequested, m_tagging, [this](const QString &root) {
        if (!root.isEmpty()) {
            m_tagging->manageTags(root);
        }
    });
    connect(m_tagging, &TaggingController::editEmbeddedRequested, this, &MainWindow::openMetadataEditor);
    QAction *tagPanelAction = actions->addAction(QStringLiteral("show_tag_panel"));
    tagPanelAction->setText(i18nc("@action", "Tag Panel"));
    tagPanelAction->setIcon(QIcon::fromTheme(QStringLiteral("tag")));
    tagPanelAction->setCheckable(true);
    connect(tagPanelAction, &QAction::triggered, this, [this](bool checked) {
        m_tagDock->setVisible(checked);
        if (checked) {
            m_tagDock->setProperty("opusPanelShown", true);
            restorePanelDockSize(m_tagDock, QStringLiteral("TagDock"));
        }
    });
    connect(m_tagDock, &QDockWidget::visibilityChanged, tagPanelAction, [this, tagPanelAction] {
        tagPanelAction->setChecked(!m_tagDock->isHidden());
    });
    QAction *assignTags = actions->addAction(QStringLiteral("assign_tags"), this, [this] {
        if (m_tagDock->isHidden()) {
            m_tagDock->show();
            m_tagDock->setProperty("opusPanelShown", true);
            restorePanelDockSize(m_tagDock, QStringLiteral("TagDock"));
        }
        m_tagDock->raise();
        updateTagPanel();
        m_tagPanel->focusSearch();
    });
    assignTags->setText(i18nc("@action", "Assign Tags…"));
    assignTags->setIcon(QIcon::fromTheme(QStringLiteral("tag")));
    assignTags->setToolTip(i18nc("@info:tooltip", "Show the tag panel to assign or remove tags on the selected items"));

    // Settings
    KStandardAction::preferences(this, &MainWindow::showSettings, actions);
    QAction *keyboardMap = actions->addAction(QStringLiteral("keyboard_map"), this, &MainWindow::showKeyboardMap);
    keyboardMap->setText(i18nc("@action", "Keyboard Map…"));
    keyboardMap->setIcon(QIcon::fromTheme(QStringLiteral("help-keybord-shortcuts")));
    keyboardMap->setToolTip(
        i18nc("@info:tooltip", "List every action with its shortcuts, search them by name or by pressing keys, and export a printable reference"));
    QAction *shortcutHints = actions->addAction(QStringLiteral("show_shortcut_hints"), this, [this] {
        m_shortcutReference->toggle();
    });
    shortcutHints->setText(i18nc("@action", "Shortcut Hints"));
    shortcutHints->setIcon(QIcon::fromTheme(QStringLiteral("help-keybord-shortcuts")));
    shortcutHints->setToolTip(i18nc("@info:tooltip", "Show or hide the keys that work where the keyboard focus is, over the active pane"));
    // KDE's window creates the command bar action, under Help; it opens the
    // palette, which lists only the actions it can run where the user is.
    setCommandBarEnabled(false);
    if (QAction *palette = actions->action(QString::fromLatin1(CommandPalette::ActionName))) {
        palette->setText(i18nc("@action", "Command Palette…"));
        palette->setIcon(SymbolicIcon::fromTheme(QStringLiteral("search")));
        palette->setToolTip(i18nc("@info:tooltip", "Search the actions of the menus and toolbars and the saved commands by name, and run one"));
        // Dolphin's and Kate's key, which no other action uses.
        KActionCollection::setDefaultShortcut(palette, QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_I));
        connect(palette, &QAction::triggered, this, &MainWindow::showCommandPalette);
    }
    QAction *showMenuBar = KStandardAction::showMenubar(menuBar(), &QMenuBar::setVisible, actions);
    KHamburgerMenu *hamburger = KStandardAction::hamburgerMenu(nullptr, nullptr, actions);
    hamburger->setMenuBar(menuBar());
    hamburger->setShowMenuBarAction(showMenuBar);

    m_copyToMenu = new KFileCopyToMenu(this);
    m_fileItemActions = new KFileItemActions(this);
    m_fileItemActions->setParentWidget(this);
}

Pane *MainWindow::activePane() const
{
    return m_active ? m_active : m_left;
}

Pane *MainWindow::inactivePane() const
{
    return activePane() == m_left ? m_right : m_left;
}

WindowSequenceHints *MainWindow::windowSequenceHints() const
{
    return m_windowSequenceHints;
}

ShortcutReference *MainWindow::shortcutReference() const
{
    return m_shortcutReference;
}

KeyboardHintOverlay *MainWindow::keyboardHintOverlay() const
{
    return m_keyboardHints;
}

void MainWindow::setActivePane(Pane *pane)
{
    if (m_keyboardHints && pane != m_active) {
        m_keyboardHints->cancel();
    }
    if (m_paneTabs && m_secondPaneInTab) {
        m_paneTabs->setCurrentWidget(pane == m_right ? static_cast<QWidget *>(m_right) : m_searchSplitter);
    }
    if (!pane || pane == m_active) {
        return;
    }
    if (m_active) {
        m_active->setActive(false);
    }
    m_active = pane;
    m_active->setActive(true);
    if (m_terminalPanelAction && m_terminalPanelAction->isChecked() && Settings::terminalFollowActiveFolder()) {
        m_terminalPanel->follow(pane->url());
    }
    if (m_dualLayoutAction) {
        toggleLayout(m_dualLayoutAction->isChecked());
    }
    m_hiddenFilesAction->setChecked(pane->view()->showHiddenFiles());
    syncFolderTrees();
    updateActions();
    updateStatusBar();
    updateFreeSpace();
    updateWindowTitle();
    updatePreviewPanel();
    updateInformationPanel();
}

void MainWindow::focusPane(Pane *pane)
{
    if (!pane) {
        return;
    }
    setActivePane(pane);
    if (QWidget *paneWindow = pane->window(); paneWindow != window()) {
        paneWindow->raise();
        paneWindow->activateWindow();
    }
    pane->view()->currentView()->setFocus(Qt::ShortcutFocusReason);
}

void MainWindow::focusPreviewPanel()
{
    const QWidget *focus = QApplication::focusWidget();
    if (m_previewPanelAction->isChecked() && focus && m_previewPanel->isAncestorOf(focus)) {
        focusPane(activePane());
        return;
    }
    if (!m_previewPanelAction->isChecked()) {
        m_previewPanelAction->setChecked(true);
    }
    if (QWidget *panelWindow = m_previewPanel->window(); panelWindow != window()) {
        panelWindow->raise();
        panelWindow->activateWindow();
    }
    if (!m_previewPanel->focusContent()) {
        showStatusMessage(i18nc("@info:status", "The preview panel has nothing to move the keyboard focus to."));
    }
}

void MainWindow::focusTerminal()
{
    if (m_terminalPanelAction->isChecked() && m_terminalPanel->hasKeyboardFocus()) {
        focusPane(activePane());
        return;
    }
    if (!m_terminalPanelAction->isChecked()) {
        m_terminalPanelAction->setChecked(true);
    } else if (!m_terminalPanel->hasSessions()) {
        m_terminalPanel->start(activePane()->url());
    }
    if (QWidget *panelWindow = m_terminalPanel->window(); panelWindow != window()) {
        panelWindow->raise();
        panelWindow->activateWindow();
    }
    // Without a session, the panel already tells why it couldn't open one.
    m_terminalPanel->focusSession();
}

void MainWindow::openUrls(const QList<QUrl> &urls)
{
    const bool startup = !m_startupLinkAttempted;
    m_startupLinkAttempted = true;
    if (urls.isEmpty()) {
        if (!m_sessionRestored) {
            restoreSession();
        }
        if (startup && Settings::linkPanesAtStartup()) {
            m_linkedPanesAction->setChecked(true);
        }
        return;
    }
    if (!m_sessionRestored) {
        restoreSession();
    }
    // Like `mc a b`: the first path goes to the active pane, the second to
    // the other pane, anything further becomes extra tabs in the active one.
    Pane *pane = activePane();
    Pane *other = inactivePane();
    for (int i = 0; i < urls.size(); ++i) {
        if (i == 1) {
            other->openInNewTab(urls.at(i), true);
        } else {
            pane->openInNewTab(urls.at(i), true);
        }
    }
    if (startup && Settings::linkPanesAtStartup()) {
        m_linkedPanesAction->setChecked(true);
    }
}

void MainWindow::showFolders(const QList<QUrl> &folders)
{
    openUrls({});
    for (const QUrl &folder : folders) {
        activePane()->openInNewTab(folder, true);
    }
}

void MainWindow::showItems(const QList<QUrl> &items)
{
    openUrls({});
    Pane *pane = activePane();
    const QList<FileManagerRequest::Reveal> reveals = FileManagerRequest::groupByFolder(items);
    for (const FileManagerRequest::Reveal &reveal : reveals) {
        pane->openInNewTab(reveal.folder, true);
        if (!reveal.items.isEmpty()) {
            pane->revealUrls(reveal.folder, reveal.items);
        }
    }
}

void MainWindow::showItemProperties(const QList<QUrl> &items)
{
    QList<QUrl> present;
    QStringList missing;
    for (const QUrl &item : items) {
        if (item.isLocalFile() && !QFileInfo::exists(item.toLocalFile())) {
            missing.append(item.toDisplayString(QUrl::PreferLocalFile));
        } else {
            present.append(item);
        }
    }
    if (!missing.isEmpty()) {
        statusBar()->showMessage(i18ncp("@info:status",
                                        "Could not find %1 requested item: %2",
                                        "Could not find %1 requested items: %2",
                                        missing.size(),
                                        missing.join(QStringLiteral(", "))));
    }
    if (!present.isEmpty()) {
        KPropertiesDialog::showDialog(present, this, false);
    }
}

void MainWindow::restoreSession()
{
    m_sessionRestored = true;
    const KConfigGroup group = KSharedConfig::openStateConfig()->group(QLatin1String(SessionGroup));
    const WindowSession session = Startup::resolve(Session::load(group, int(EffectiveAppearance::defaultViewMode())), Startup::fromSettings());

    m_left->restoreState(session.left);
    m_right->restoreState(session.right);
    setActivePane(session.activePane == 1 ? m_right : m_left);
    m_dualLayoutAction->setChecked(session.dual);
    setStackedLayout(session.stacked);
    restoreSecondPanePlacement();
}

void MainWindow::saveSession()
{
    WindowSession session;
    session.left = m_left->saveState();
    session.right = m_right->saveState();
    session.activePane = activePane() == m_right ? 1 : 0;
    session.dual = isDualLayout();
    session.stacked = isStackedLayout();

    KSharedConfig::Ptr state = KSharedConfig::openStateConfig();
    KConfigGroup group = state->group(QLatin1String(SessionGroup));
    Session::save(session, group);
    KConfigGroup panels = state->group(QLatin1String(PanelsGroup));
    panels.writeEntry("FoldersExpanded", m_folderTree->expandedUrls());
    panels.writeEntry("CombinedExpanded", m_combinedPanel->folderTree()->expandedUrls());
    panels.writeEntry("CollapsedSections", m_combinedPanel->collapsedSections());
    KConfigGroup layout = state->group(QLatin1String(WindowLayoutGroup));
    layout.writeEntry("DualPane", m_dualLayoutAction->isChecked());
    layout.writeEntry("SecondPanePlacement", m_rightWindow ? QStringLiteral("window") : (m_secondPaneInTab ? QStringLiteral("tab") : QStringLiteral("split")));
    state->sync();
}

bool MainWindow::queryClose()
{
    const auto transfers = FileOperations::transferQueue()->entries();
    const bool transfersPending = std::any_of(transfers.cbegin(), transfers.cend(), [](const QueuedTransfer *transfer) {
        return transfer->state() == QueuedTransfer::State::Waiting || transfer->state() == QueuedTransfer::State::Running
            || transfer->state() == QueuedTransfer::State::Paused;
    });
    if (transfersPending
        && KMessageBox::warningContinueCancel(this,
                                              i18nc("@info",
                                                    "Transfers are still queued or running. Closing the application may cancel active transfers "
                                                    "and discards waiting transfers. Close this window anyway?"),
                                              i18nc("@title:window", "Transfers in Progress"),
                                              KGuiItem(i18nc("@action:button", "Close Window"), QStringLiteral("window-close")))
            != KMessageBox::Continue) {
        return false;
    }
    const bool severalTabs = m_left->tabs()->count() > 1 || m_right->tabs()->count() > 1;
    if (Settings::confirmClosingMultipleTabs() && severalTabs
        && KMessageBox::warningContinueCancel(this,
                                              i18nc("@info", "This window has several tabs open. Close them all?"),
                                              i18nc("@title:window", "Close Tabs"),
                                              KGuiItem(i18nc("@action:button", "Close Window"), QStringLiteral("window-close")))
            != KMessageBox::Continue) {
        return false;
    }
    if (m_editor && !m_editor->queryClose()) {
        return false;
    }
    if (!m_previewPanel->queryClose()) {
        return false;
    }
    for (MetadataEditDialog *dialog : findChildren<MetadataEditDialog *>()) {
        if (dialog->isBusy()) {
            dialog->reject();
            return false;
        }
    }
    if (!waitForCloudUploads(i18nc("@info", "Plasma Opus is still uploading files. Closing now stops the uploads."), {}, true)) {
        return false;
    }
    if (m_editorDock && !m_editorDock->isHidden()) {
        savePanelDockSize(m_editorDock, QStringLiteral("EditorDock"));
    }
    if (m_previewDock && !m_previewDock->isHidden()) {
        savePanelDockSize(m_previewDock, QStringLiteral("PreviewDock"));
    }
    if (m_tagDock && !m_tagDock->isHidden()) {
        savePanelDockSize(m_tagDock, QStringLiteral("TagDock"));
    }
    for (const auto &dock : std::as_const(m_metadataDocks)) {
        if (dock && !dock->isHidden()) {
            savePanelDockSize(dock, QStringLiteral("MetadataDock"));
        }
    }
    const auto galleries = findChildren<GalleryWindow *>(QString(), Qt::FindDirectChildrenOnly);
    for (GalleryWindow *gallery : galleries) {
        if (!gallery->close()) {
            return false;
        }
    }
    saveSession();
    return KXmlGuiWindow::queryClose();
}

void MainWindow::readProperties(const KConfigGroup &group)
{
    WindowSession session = Session::load(group, int(EffectiveAppearance::defaultViewMode())).sanitized(QUrl::fromLocalFile(QDir::homePath()));
    const StartupOptions options = Startup::fromSettings();
    session.left.yaziMode = YaziMode::startupMode(options.leftYazi, session.left.yaziMode);
    session.right.yaziMode = YaziMode::startupMode(options.rightYazi, session.right.yaziMode);
    m_sessionRestored = true;
    m_left->restoreState(session.left);
    m_right->restoreState(session.right);
    setActivePane(session.activePane == 1 ? m_right : m_left);
    m_dualLayoutAction->setChecked(session.dual);
    setStackedLayout(session.stacked);
    restoreSecondPanePlacement();
}

void MainWindow::restoreSecondPanePlacement()
{
    const QString placement =
        KSharedConfig::openStateConfig()->group(QLatin1String(WindowLayoutGroup)).readEntry("SecondPanePlacement", QStringLiteral("split"));
    if (placement == QLatin1String("tab")) {
        moveSecondPaneToTab();
    } else if (placement == QLatin1String("window")) {
        popOutSecondPane();
    }
}

void MainWindow::saveProperties(KConfigGroup &group)
{
    WindowSession session;
    session.left = m_left->saveState();
    session.right = m_right->saveState();
    session.activePane = activePane() == m_right ? 1 : 0;
    session.dual = isDualLayout();
    session.stacked = isStackedLayout();
    Session::save(session, group);
}

void MainWindow::updateActions()
{
    KActionCollection *actions = actionCollection();
    FolderView *view = activePane()->view();
    const KFileItemList selected = view->selectedItems();
    const bool hasSelection = !selected.isEmpty();
    const KFileItemListProperties props(selected);
    const KFileItem root = view->rootItem();

    const bool bothVisible = isDualLayout() && !m_left->isHidden() && !m_right->isHidden();
    auto *tabPosition = qobject_cast<KSelectAction *>(actions->action(QStringLiteral("tab_bar_position")));
    tabPosition->setCurrentItem(static_cast<int>(activePane()->tabBarPosition()));
    actions->action(QStringLiteral("stacked_tab_bars_together"))->setEnabled(isStackedLayout());
    const bool currentTabsCanSwap = bothVisible && m_left->tabs()->currentIndex() >= 0 && m_right->tabs()->currentIndex() >= 0;
    actions->action(QStringLiteral("swap_current_tabs"))->setEnabled(currentTabsCanSwap);
    actions->action(QStringLiteral("swap_pane_sides"))->setEnabled(bothVisible);
    const bool swapWholeSides = Settings::paneSwapMode() == Settings::EnumPaneSwapMode::WholeSides;
    actions->action(QStringLiteral("swap_panes"))->setEnabled(swapWholeSides ? bothVisible : currentTabsCanSwap);
    actions->action(QStringLiteral("copy_to_other_pane"))->setEnabled(hasSelection);
    actions->action(QStringLiteral("move_to_other_pane"))->setEnabled(hasSelection && props.supportsMoving());
    actions->action(QStringLiteral("rename"))->setEnabled(hasSelection && props.supportsMoving());
    actions->action(QStringLiteral("batch_rename"))
        ->setEnabled(hasSelection && props.supportsMoving() && std::all_of(selected.cbegin(), selected.cend(), [](const KFileItem &item) {
                         return item.url().isLocalFile();
                     }));
    actions->action(QStringLiteral("create_symlink"))->setEnabled(selected.size() == 1 && selected.first().url().isLocalFile());
    actions->action(QStringLiteral("move_to_trash"))->setEnabled(hasSelection && props.supportsDeleting() && props.isLocal());
    actions->action(QStringLiteral("delete"))->setEnabled(hasSelection && props.supportsDeleting());
    actions->action(stdName(KStandardAction::Cut))->setEnabled(hasSelection && props.supportsMoving());
    actions->action(stdName(KStandardAction::Copy))->setEnabled(hasSelection);
    actions->action(QStringLiteral("open_terminal"))->setEnabled(activePane()->url().isLocalFile());
    QAction *actAsAdmin = actions->action(QStringLiteral("act_as_admin"));
    actAsAdmin->setEnabled(AdminMode::canToggle(activePane()->url()));
    actAsAdmin->setChecked(AdminMode::isAdminUrl(activePane()->url()));
    actions->action(QStringLiteral("open_terminal_with"))->setEnabled(activePane()->url().isLocalFile());
    actions->action(QStringLiteral("edit_with"))->setEnabled(hasSelection && std::none_of(selected.cbegin(), selected.cend(), [](const KFileItem &item) {
                                                                 return item.isDir();
                                                             }));
    actions->action(QStringLiteral("open_in_office"))->setEnabled(hasSelection && std::all_of(selected.cbegin(), selected.cend(), [](const KFileItem &item) {
                                                                      return !item.isDir() && OfficeApps::isOfficeDocument(item.mimetype());
                                                                  }));
    QAction *editFile = actions->action(QStringLiteral("edit_file"));
    const bool editable = selected.size() == 1 && !selected.first().isDir() && EditTools::category(selected.first().mimetype()) != EditTools::Category::None;
    editFile->setEnabled(editable);
    const std::optional<EditorProviders::Editor> preferredEditor = EditorProviders::preferredEditor();
    if (editable && preferredEditor && EditTools::category(selected.first().mimetype()) == EditTools::Category::Text) {
        const QString reason = EditorProviders::unavailableReason(*preferredEditor);
        editFile->setToolTip(
            reason.isEmpty() ? i18nc("@info:tooltip %1 is a text editor", "Edit the selected file with %1", EditorProviders::name(*preferredEditor)) : reason);
    } else if (editable) {
        QString error;
        const std::optional<EditTools::Choice> choice = EditTools::choose(selected.first().mostLocalUrl(), selected.first().mimetype(), &error);
        editFile->setToolTip(
            choice ? i18nc("@info:tooltip %1 is an application or the built-in editor", "Edit the selected file with %1", EditTools::name(choice->tool))
                   : error);
    } else {
        editFile->setToolTip(i18nc("@info:tooltip", "Edit the selected text file, office document, image or audio file with a suitable tool"));
    }
    const bool commandEnabled = activePane()->url().isLocalFile() && std::all_of(selected.cbegin(), selected.cend(), [](const KFileItem &item) {
                                    return item.url().isLocalFile();
                                });
    actions->action(QStringLiteral("custom_command"))->setEnabled(commandEnabled);
    for (QAction *action : std::as_const(m_customCommandActions)) {
        action->setEnabled(commandEnabled);
    }
    actions->action(QStringLiteral("content_chart"))->setEnabled(activePane()->url().isValid() && !activePane()->url().isEmpty());
    actions->action(QStringLiteral("find_duplicates"))->setEnabled(activePane()->url().isLocalFile());
    actions->action(QStringLiteral("compare_pane_folders"))
        ->setEnabled(isDualLayout() && !inactivePane()->isHidden() && activePane()->url().isValid() && !activePane()->url().isEmpty()
                     && inactivePane()->url().isValid() && !inactivePane()->url().isEmpty());
    actions->action(QStringLiteral("compare_folders_filtered"))
        ->setEnabled(isDualLayout() && !inactivePane()->isHidden() && synchronizableFolder(activePane()->url()) && synchronizableFolder(inactivePane()->url()));
    const bool localFiles = hasSelection && std::all_of(selected.cbegin(), selected.cend(), [](const KFileItem &item) {
                                return item.url().isLocalFile() && item.isFile() && !item.isLink();
                            });
    actions->action(QStringLiteral("create_checksum"))->setEnabled(localFiles);
    actions->action(QStringLiteral("kdeconnect_send"))->setEnabled(hasSelection && Settings::kdeConnectSharing());
    actions->action(QStringLiteral("send_by_email"))
        ->setEnabled(Settings::sendByEmail() && std::any_of(selected.cbegin(), selected.cend(), [](const KFileItem &item) {
                         return !item.isDir();
                     }));
    actions->action(QStringLiteral("share_on_network"))
        ->setEnabled(hasSelection ? std::all_of(selected.cbegin(),
                                                selected.cend(),
                                                [](const KFileItem &item) {
                                                    return !item.localPath().isEmpty();
                                                })
                                  : activePane()->url().isLocalFile());
    actions->action(QStringLiteral("verify_checksum"))->setEnabled(localFiles && selected.size() == 1);
    actions->action(QStringLiteral("edit_metadata"))->setEnabled(localFiles && selected.size() == 1);
    actions->action(QStringLiteral("split_file"))->setEnabled(localFiles && selected.size() == 1);
    actions->action(QStringLiteral("combine_files"))->setEnabled(localFiles);
    actions->action(QStringLiteral("encryption"))->setEnabled(hasSelection && std::all_of(selected.cbegin(), selected.cend(), [](const KFileItem &item) {
                                                                  return item.url().isLocalFile() && !item.isDir();
                                                              }));
    actions->action(QStringLiteral("compare_files"))->setEnabled(!comparisonFiles(activePane(), inactivePane(), isDualLayout()).isEmpty());
    const bool hasMedia = std::any_of(selected.cbegin(), selected.cend(), [](const KFileItem &item) {
        if (item.isDir() || !item.url().isLocalFile()) {
            return false;
        }
        const QString mime = item.mimetype();
        return mime.startsWith(QLatin1String("image/")) || mime.startsWith(QLatin1String("video/"));
    });
    actions->action(QStringLiteral("open_gallery"))->setEnabled(hasMedia);
    actions->action(QStringLiteral("test_archive_integrity"))->setEnabled(std::any_of(selected.cbegin(), selected.cend(), [](const KFileItem &item) {
        return !item.isDir() && Archives::browseUrl(item.mostLocalUrl(), item.mimetype()).isValid();
    }));
    actions->action(QStringLiteral("extract_protected_archive"))->setEnabled(std::any_of(selected.cbegin(), selected.cend(), [](const KFileItem &item) {
        return !item.localPath().isEmpty() && !item.isDir() && Archives::browseUrl(item.mostLocalUrl(), item.mimetype()).isValid();
    }));
    actions->action(QStringLiteral("open_in_ark"))
        ->setEnabled(selected.size() == 1 && !selected.first().isDir() && !selected.first().localPath().isEmpty()
                     && (Archives::browseUrl(selected.first().mostLocalUrl(), selected.first().mimetype()).isValid()
                         || arkBackends().canRead(selected.first().mimetype())));
    const bool diskImage = !selectedDiskImage().isNull();
    actions->action(QStringLiteral("mount_disk_image"))->setEnabled(diskImage);
    actions->action(QStringLiteral("unmount_disk_image"))->setEnabled(diskImage);
    actions->action(QStringLiteral("application_package"))->setEnabled(!selectedPackage().isNull());

    bool pasteEnabled = false;
    const QString pasteText = FileOperations::pasteActionText(root, &pasteEnabled);
    const bool flat = view->isFlat();
    m_pasteAction->setEnabled(pasteEnabled && !root.isNull() && root.isWritable() && !flat);
    m_flatAction->setChecked(flat);
    const QString flatReason = flat ? i18nc("@info:tooltip", "Not available in a flat list, whose items come from many folders") : QString();
    for (const QString &name : {QStringLiteral("new_menu"), QStringLiteral("new_folder")}) {
        QAction *create = actions->action(name);
        create->setEnabled(!flat);
        create->setToolTip(flat ? flatReason : create->text());
    }
    if (!pasteText.isEmpty()) {
        m_pasteAction->setText(pasteText);
    }
    applyNextcloudPermissions(selected);

    actions->action(stdName(KStandardAction::Back))->setEnabled(activePane()->tabs()->canGoBack());
    actions->action(stdName(KStandardAction::Forward))->setEnabled(activePane()->tabs()->canGoForward());
    const TabModel *tabs = activePane()->tabs();
    actions->action(QStringLiteral("close_tab"))
        ->setEnabled(tabs->count() > 1 && !tabs->isPinned(tabs->currentIndex()) && !tabs->isLocked(tabs->currentIndex()));

    m_selectionModeAction->setChecked(view->selectionMode());
    m_yaziModeAction->setChecked(activePane()->yaziMode());
    m_yaziPreviewAction->setChecked(activePane()->yaziPreview());
    m_yaziPreviewAction->setEnabled(activePane()->yaziMode());
    m_yaziPreviewInformationAction->setChecked(activePane()->yaziPreviewContent() == YaziMode::PreviewContent::Information);
    m_yaziPreviewInformationAction->setEnabled(activePane()->yaziMode());
    m_yaziParentColumnAction->setChecked(activePane()->yaziParentColumn());
    m_yaziParentColumnAction->setEnabled(activePane()->yaziMode());
    for (QAction *action : std::as_const(m_yaziColumnActions)) {
        action->setEnabled(activePane()->yaziMode());
    }
    m_selectionMarkersAction->setChecked(view->selectionMarkersVisible());
    actions->action(QStringLiteral("folder_format_source"))->setText(view->formatRuleDescription());
    actions->action(QStringLiteral("open_folder_format_source"))
        ->setEnabled(!flat && !view->formatLocked() && !view->formatRuleSource().isEmpty()
                     && QUrl::fromLocalFile(view->formatRuleSource()).adjusted(QUrl::StripTrailingSlash) != view->url().adjusted(QUrl::StripTrailingSlash));
    actions->action(QStringLiteral("clear_folder_format"))->setEnabled(!flat && !view->formatLocked() && view->hasLocalFolderFormat());
    actions->action(QStringLiteral("lock_folder_format"))->setChecked(view->formatLocked());
    QAction *inheritFormat = actions->action(QStringLiteral("inherit_folder_format"));
    inheritFormat->setChecked(view->formatIncludesDescendants());
    inheritFormat->setEnabled(view->url().isLocalFile() && !view->formatLocked() && !flat);
    QAction *automaticSizes = actions->action(QStringLiteral("automatic_folder_sizes"));
    automaticSizes->setChecked(view->automaticFolderSizes());
    automaticSizes->setEnabled(view->url().isLocalFile() && !view->formatLocked() && !flat);
    actions->action(QStringLiteral("default_folder_size_policy"))->setEnabled(view->url().isLocalFile() && !view->formatLocked() && !flat);
    actions->action(QStringLiteral("calculate_folder_sizes"))->setEnabled(!flat && !view->rootItem().isNull());
    actions->action(QStringLiteral("automatic_local_folder_sizes"))->setChecked(Settings::automaticLocalFolderSizes());
    actions->action(QStringLiteral("automatic_remote_folder_sizes"))->setChecked(Settings::automaticRemoteFolderSizes());
    const bool details = view->viewMode() == FolderView::ViewMode::Details;
    m_viewModeAction->setText(details ? i18nc("@action", "Icons View") : i18nc("@action", "Details View"));
    m_viewModeAction->setIcon(SymbolicIcon::fromTheme(details ? QStringLiteral("view-list-icons") : QStringLiteral("view-list-details")));
    actions->action(QStringLiteral("calculate_occupied_space"))->setEnabled(!view->rootItem().isNull());
    m_jumpBackAction->setEnabled(m_jumpBackPoints.value(activePane()).isValid());
    if (m_tagging) {
        m_tagging->updateActions();
        actions->action(QStringLiteral("assign_tags"))->setEnabled(!m_tagging->currentLibrary().isEmpty());
    }
    actions->action(QStringLiteral("move_second_pane_to_tab"))->setEnabled(!m_secondPaneInTab);
    actions->action(QStringLiteral("move_second_pane_to_window"))->setEnabled(!m_rightWindow);
    const bool moveToTab = Settings::paneMoveDestination() == Settings::EnumPaneMoveDestination::Tab;
    actions->action(QStringLiteral("move_second_pane"))->setEnabled(moveToTab ? !m_secondPaneInTab : !m_rightWindow);
    actions->action(QStringLiteral("restore_second_pane"))->setEnabled(m_secondPaneInTab || m_rightWindow);
    updateFolderGateActions();
}

void MainWindow::updateFolderGateActions()
{
    KActionCollection *actions = actionCollection();
    const FolderGates *gates = FolderGates::instance();
    const QString folder = passwordFolder(activePane());
    const bool gate = gates->isGate(folder);
    const bool locked = gate && gates->isGateLocked(folder);
    actions->action(QStringLiteral("protect_folder"))
        ->setEnabled(!folder.isEmpty() && !gate && QFileInfo(folder).isDir() && !gates->isLocked(QUrl::fromLocalFile(folder)));
    actions->action(QStringLiteral("unlock_folder"))->setEnabled(locked);
    actions->action(QStringLiteral("change_folder_password"))->setEnabled(gate);
    actions->action(QStringLiteral("remove_folder_password"))->setEnabled(gate);
    actions->action(QStringLiteral("lock_protected_folders"))->setEnabled(gates->hasUnlocked());

    // Nothing that reads, copies, moves or deletes the contents of a locked
    // password-gated folder is offered for it, or inside it.
    FolderView *view = activePane()->view();
    const KFileItemList selected = view->selectedItems();
    const bool folderLocked = view->isGateLocked();
    const bool selectionLocked = std::any_of(selected.cbegin(), selected.cend(), [gates](const KFileItem &item) {
        return gates->touchesLocked(item.url());
    });
    if (!folderLocked && !selectionLocked) {
        return;
    }
    static const QStringList contentActions{
        QStringLiteral("copy_to_other_pane"),
        QStringLiteral("move_to_other_pane"),
        QStringLiteral("compress"),
        QStringLiteral("add_to_archive"),
        QStringLiteral("batch_rename"),
        QStringLiteral("edit_metadata"),
        QStringLiteral("create_symlink"),
        QStringLiteral("move_to_trash"),
        QStringLiteral("delete"),
        QStringLiteral("edit_with"),
        QStringLiteral("open_in_office"),
        QStringLiteral("edit_file"),
        QStringLiteral("custom_command"),
        QStringLiteral("content_chart"),
        QStringLiteral("find_duplicates"),
        QStringLiteral("compare_pane_folders"),
        QStringLiteral("compare_folders_filtered"),
        QStringLiteral("create_checksum"),
        QStringLiteral("verify_checksum"),
        QStringLiteral("kdeconnect_send"),
        QStringLiteral("send_by_email"),
        QStringLiteral("share_on_network"),
        QStringLiteral("split_file"),
        QStringLiteral("combine_files"),
        QStringLiteral("encryption"),
        QStringLiteral("compare_files"),
        QStringLiteral("open_gallery"),
        QStringLiteral("calculate_folder_sizes"),
        QStringLiteral("calculate_occupied_space"),
        QStringLiteral("assign_tags"),
        QStringLiteral("export_listing"),
        QStringLiteral("export_listing_csv"),
        QStringLiteral("export_listing_pdf"),
    };
    for (const QString &name : contentActions) {
        if (QAction *action = actions->action(name)) {
            action->setEnabled(false);
        }
    }
    for (QAction *action : std::as_const(m_customCommandActions)) {
        action->setEnabled(false);
    }
    actions->action(stdName(KStandardAction::Cut))->setEnabled(false);
    actions->action(stdName(KStandardAction::Copy))->setEnabled(false);
    m_pasteAction->setEnabled(false);
    if (folderLocked) {
        for (const QString &name : {QStringLiteral("new_menu"),
                                    QStringLiteral("new_folder"),
                                    QStringLiteral("open_terminal"),
                                    QStringLiteral("open_terminal_with"),
                                    QStringLiteral("act_as_admin")}) {
            actions->action(name)->setEnabled(false);
        }
    }
}

void MainWindow::updateStatusBar()
{
    const bool perPane = Settings::statusBarLayout() == Settings::EnumStatusBarLayout::PerPane;
    const auto itemText = [](Pane *pane) {
        const KFileItemList selected = pane->view()->selectedItems();
        const int total = pane->view()->itemCount();
        if (selected.isEmpty()) {
            return i18ncp("@info:status", "%1 item", "%1 items", total);
        }
        KIO::filesize_t size = 0;
        int folders = 0;
        for (const KFileItem &item : selected) {
            if (item.isDir()) {
                ++folders;
            } else {
                size += item.size();
            }
        }
        QString text = i18ncp("@info:status", "%1 of %2 selected", "%1 of %2 selected", selected.size(), total);
        if (selected.size() > folders && Settings::showSelectionSize()) {
            text += QStringLiteral(" (%1)").arg(KIO::convertSize(size));
        }
        return text;
    };
    for (Pane *pane : {m_left, m_right}) {
        const bool showCounts = Settings::showItemCounts();
        const bool showFreeSpace = Settings::showFreeSpace();
        m_paneItemsLabels.value(pane)->setText(itemText(pane));
        m_paneItemsLabels.value(pane)->setVisible(perPane && showCounts);
        m_paneFreeSpaceLabels.value(pane)->setVisible(perPane && showFreeSpace);
        pane->statusBarWidget()->setVisible(perPane && (showCounts || showFreeSpace));
    }
    m_itemsLabel->setText(itemText(activePane()));
    m_itemsLabel->setVisible(!perPane && Settings::showItemCounts());
    m_freeSpaceLabel->setVisible(!perPane && Settings::showFreeSpace());

    const KFileItemList selected = activePane()->view()->selectedItems();
    if (selected.size() == 1) {
        const KFileItem &item = selected.first();
        QStringList parts{item.text(), item.mimeComment()};
        if (!item.isDir()) {
            parts.append(KIO::convertSize(item.size()));
        }
        parts.append(LocalizedDates::fileTime(item));
        parts.append(item.permissionsString());
        if (item.isLink()) {
            parts.append(i18nc("@info:status symlink target", "→ %1", item.linkDest()));
        }
        m_messageLabel->setText(parts.join(QStringLiteral("  ·  ")));
    } else {
        m_messageLabel->clear();
    }
}

void MainWindow::updateFreeSpace(Pane *pane)
{
    const bool perPane = Settings::statusBarLayout() == Settings::EnumStatusBarLayout::PerPane;
    const QList<Pane *> panes = pane ? QList<Pane *>{pane} : (perPane ? QList<Pane *>{m_left, m_right} : QList<Pane *>{activePane()});
    for (Pane *target : panes) {
        QLabel *label = m_paneFreeSpaceLabels.value(target);
        const QUrl url = target->url();
        label->clear();
        if (target == activePane()) {
            m_freeSpaceLabel->clear();
        }
        if (!url.isValid() || !Settings::showFreeSpace()) {
            continue;
        }
        KIO::FileSystemFreeSpaceJob *job = KIO::fileSystemFreeSpace(url);
        connect(job, &KJob::result, this, [this, job, target, url] {
            if (url != target->url()) {
                return;
            }
            if (job->error()) {
                m_paneFreeSpaceLabels.value(target)->clear();
                if (target == activePane()) {
                    m_freeSpaceLabel->clear();
                }
                return;
            }
            const QString text = i18nc("@info:status free disk space", "%1 free of %2", KIO::convertSize(job->availableSize()), KIO::convertSize(job->size()));
            m_paneFreeSpaceLabels.value(target)->setText(text);
            if (target == activePane()) {
                m_freeSpaceLabel->setText(text);
            }
        });
    }
}

void MainWindow::updateWindowTitle()
{
    setCaption(activePane()->url().toDisplayString(QUrl::PreferLocalFile));
}

void MainWindow::updateTabBars()
{
    const bool force = Settings::alwaysShowTabBar() || m_left->tabs()->count() > 1 || m_right->tabs()->count() > 1;
    m_left->setTabBarForcedVisible(force);
    m_right->setTabBarForcedVisible(force);
}

void MainWindow::showStatusMessage(const QString &message)
{
    statusBar()->showMessage(message, 5000);
}

void MainWindow::recordFolderVisit(Pane *pane, const QUrl &url)
{
    if (!url.isValid() || url.isEmpty()) {
        return;
    }
    const QUrl normalized = url.adjusted(QUrl::StripTrailingSlash);
    ActivityReporting::reportUsed(normalized);
    const QString key = normalized.toString(QUrl::FullyEncoded);
    const QUrl previous = m_lastFolderByPane.value(pane);
    if (!m_jumpingBack && previous.isValid() && previous != normalized) {
        m_jumpBackPoints.insert(pane, previous);
    }
    m_lastFolderByPane.insert(pane, normalized);
    m_popularFolderVisits.insert(key, m_popularFolderVisits.value(key) + 1);
    savePopularFolderVisits();
    if (m_jumpBackAction) {
        m_jumpBackAction->setEnabled(m_jumpBackPoints.value(pane).isValid());
    }
}

void MainWindow::savePopularFolderVisits()
{
    QList<QPair<QString, int>> entries;
    entries.reserve(m_popularFolderVisits.size());
    for (auto it = m_popularFolderVisits.cbegin(); it != m_popularFolderVisits.cend(); ++it) {
        entries.append({it.key(), it.value()});
    }
    std::sort(entries.begin(), entries.end(), [](const auto &left, const auto &right) {
        if (left.second != right.second) {
            return left.second > right.second;
        }
        return left.first < right.first;
    });
    if (entries.size() > 100) {
        entries.resize(100);
        m_popularFolderVisits.clear();
        for (const auto &[url, visits] : entries) {
            m_popularFolderVisits.insert(url, visits);
        }
    }
    QStringList urls;
    QList<int> visits;
    for (const auto &[url, count] : entries) {
        urls.append(url);
        visits.append(count);
    }
    KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String(PopularFoldersGroup));
    group.writeEntry("Urls", urls);
    group.writeEntry("Visits", visits);
    group.sync();
}

void MainWindow::populatePopularFolders()
{
    m_popularFoldersMenu->clear();
    QList<QPair<QString, int>> entries;
    entries.reserve(m_popularFolderVisits.size());
    for (auto it = m_popularFolderVisits.cbegin(); it != m_popularFolderVisits.cend(); ++it) {
        entries.append({it.key(), it.value()});
    }
    std::sort(entries.begin(), entries.end(), [](const auto &left, const auto &right) {
        if (left.second != right.second) {
            return left.second > right.second;
        }
        return left.first < right.first;
    });
    if (entries.isEmpty()) {
        m_popularFoldersMenu->addAction(i18nc("@info:menu", "No visited folders"))->setEnabled(false);
        return;
    }
    for (qsizetype i = 0; i < qMin<qsizetype>(entries.size(), 12); ++i) {
        const QUrl url(entries.at(i).first, QUrl::StrictMode);
        const QString label = url.fileName().isEmpty() ? url.toDisplayString(QUrl::PreferLocalFile) : url.fileName();
        QAction *action = m_popularFoldersMenu->addAction(label);
        action->setToolTip(url.toDisplayString(QUrl::PreferLocalFile));
        connect(action, &QAction::triggered, this, [this, url] {
            activePane()->setUrl(url);
        });
    }
}

void MainWindow::populateSelectionFilters()
{
    m_selectionFiltersMenu->clear();
    const QStringList patterns = KSharedConfig::openConfig()->group(QLatin1String(SelectionFiltersGroup)).readEntry("Patterns", QStringList());
    if (patterns.isEmpty()) {
        m_selectionFiltersMenu->addAction(i18nc("@info:menu", "No saved filters"))->setEnabled(false);
        return;
    }
    for (const QString &pattern : patterns) {
        QAction *action = m_selectionFiltersMenu->addAction(pattern);
        connect(action, &QAction::triggered, this, [this, pattern] {
            applySelectionFilter(pattern);
        });
    }
}

void MainWindow::applySelectionFilter(const QString &pattern)
{
    const int matched = activePane()->view()->applySelectionPattern(pattern, true);
    showStatusMessage(i18ncp("@info:status selected items by pattern", "Selected %1 item", "Selected %1 items", matched));
}

void MainWindow::selectByPattern(bool select)
{
    bool accepted = false;
    const QString pattern = QInputDialog::getText(this,
                                                  select ? i18nc("@title:window", "Select by Pattern") : i18nc("@title:window", "Deselect by Pattern"),
                                                  i18nc("@label:textbox", "File name pattern:"),
                                                  QLineEdit::Normal,
                                                  activePane()->view()->nameFilter().text(),
                                                  &accepted)
                                .trimmed();
    if (!accepted || pattern.isEmpty()) {
        return;
    }
    const int matched = activePane()->view()->applySelectionPattern(pattern, select);
    showStatusMessage(i18ncp(select ? "@info:status selected items by pattern" : "@info:status deselected items by pattern",
                             select ? "Selected %1 item" : "Deselected %1 item",
                             select ? "Selected %1 items" : "Deselected %1 items",
                             matched));
}

void MainWindow::selectByPathFilter(bool select)
{
    const QPointer<FolderView> view = activePane()->view();
    if (!view->url().isLocalFile()) {
        showStatusMessage(i18nc("@info", "Path selection filters support local folders only."));
        return;
    }
    const QUrl folder = view->url();
    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("selectionPathDialog"));
    dialog.setWindowTitle(select ? i18nc("@title:window", "Select by Path Filter") : i18nc("@title:window", "Deselect by Path Filter"));
    auto *layout = new QVBoxLayout(&dialog);
    auto *description = new QLabel(i18nc("@info", "Applies to the current top-level listing, or all shown items in flat view."), &dialog);
    description->setWordWrap(true);
    layout->addWidget(description);
    auto *editor = new PathFilterEditor(QStringLiteral("selection"), &dialog);
    layout->addWidget(editor);
    auto *error = new QLabel(&dialog);
    error->setObjectName(QStringLiteral("selectionPathError"));
    error->setTextFormat(Qt::PlainText);
    error->setWordWrap(true);
    layout->addWidget(error);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        if (!view || view->url() != folder || view->isLoading()) {
            error->setText(i18nc("@info", "The folder changed or is still loading. Close this dialog and try again."));
            return;
        }
        QString message;
        const int matched = view->applySelectionPathFilter(editor->filter(), select, &message);
        if (matched < 0) {
            error->setText(message);
            return;
        }
        showStatusMessage(i18ncp(select ? "@info:status selected items by path" : "@info:status deselected items by path",
                                 select ? "Selected %1 item" : "Deselected %1 item",
                                 select ? "Selected %1 items" : "Deselected %1 items",
                                 matched));
        dialog.accept();
    });
    dialog.exec();
}

void MainWindow::selectByQuery(bool select)
{
    const QPointer<FolderView> view = activePane()->view();
    if (!view->url().isLocalFile()) {
        showStatusMessage(i18nc("@info", "Selection queries support local folders only."));
        return;
    }
    const QUrl folder = view->url();
    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("selectionQueryDialog"));
    dialog.setWindowTitle(select ? i18nc("@title:window", "Select by Query") : i18nc("@title:window", "Deselect by Query"));
    auto *layout = new QVBoxLayout(&dialog);
    auto *description = new QLabel(i18nc("@info",
                                         "Combine name:, path:, type:, size:, modified:, tag: and tagged: criteria with AND, OR, NOT and parentheses. "
                                         "Applies to the current top-level listing, or all shown items in flat view."),
                                   &dialog);
    description->setWordWrap(true);
    layout->addWidget(description);
    auto *edit = new QLineEdit(m_selectionQuery, &dialog);
    edit->setObjectName(QStringLiteral("selectionQuery"));
    edit->setAccessibleName(i18nc("@label:textbox", "Query"));
    edit->setPlaceholderText(i18nc("@info:placeholder", "For example: tag:holiday NOT tagged:no"));
    edit->setClearButtonEnabled(true);
    layout->addWidget(edit);
    auto *status = new QLabel(&dialog);
    status->setObjectName(QStringLiteral("selectionQueryStatus"));
    status->setTextFormat(Qt::PlainText);
    status->setWordWrap(true);
    layout->addWidget(status);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttons);
    const auto describe = [edit, status] {
        if (edit->text().trimmed().isEmpty()) {
            status->clear();
            return;
        }
        const auto query = SearchQuery::Query::parse(edit->text());
        status->setText(query.isValid() ? i18nc("@info", "Query: %1", query.toString()) : query.error());
    };
    connect(edit, &QLineEdit::textChanged, &dialog, describe);
    describe();
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        if (!view || view->url() != folder || view->isLoading()) {
            status->setText(i18nc("@info", "The folder changed or is still loading. Close this dialog and try again."));
            return;
        }
        const auto query = SearchQuery::Query::parse(edit->text());
        QString message;
        const int matched = view->applySelectionQuery(query,
                                                      LibraryRegistry::roots(),
                                                      TaggingSettings::tagSearchOptions(),
                                                      TaggingSettings::embeddedMetadataKeys(),
                                                      select,
                                                      &message);
        if (matched < 0) {
            status->setText(message);
            return;
        }
        m_selectionQuery = edit->text().trimmed();
        showStatusMessage(i18ncp(select ? "@info:status selected items by query" : "@info:status deselected items by query",
                                 select ? "Selected %1 item" : "Deselected %1 item",
                                 select ? "Selected %1 items" : "Deselected %1 items",
                                 matched));
        dialog.accept();
    });
    dialog.exec();
}

void MainWindow::saveSelectionFilter()
{
    bool accepted = false;
    const QString pattern = QInputDialog::getText(this,
                                                  i18nc("@title:window", "Save Selection Filter"),
                                                  i18nc("@label:textbox", "File name pattern:"),
                                                  QLineEdit::Normal,
                                                  activePane()->view()->nameFilter().text(),
                                                  &accepted)
                                .trimmed();
    if (!accepted || pattern.isEmpty()) {
        return;
    }
    KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String(SelectionFiltersGroup));
    QStringList patterns = group.readEntry("Patterns", QStringList());
    if (!patterns.contains(pattern)) {
        patterns.append(pattern);
        group.writeEntry("Patterns", patterns);
        group.sync();
    }
    populateSelectionFilters();
    showStatusMessage(i18nc("@info:status", "Saved selection filter %1", pattern));
}

void MainWindow::calculateOccupiedSpace()
{
    const KFileItemList items = activePane()->view()->selectedItemsOrRoot();
    if (items.isEmpty()) {
        return;
    }
    showStatusMessage(i18nc("@info:status", "Calculating occupied space…"));
    KIO::DirectorySizeJob *job = KIO::directorySize(items);
    KJobWidgets::setWindow(job, this);
    connect(job, &KJob::result, this, [this, job] {
        if (job->error()) {
            showStatusMessage(job->errorString());
            return;
        }
        showStatusMessage(i18nc("@info:status occupied size",
                                "%1 occupied in %2 and %3",
                                KIO::convertSize(job->totalSize()),
                                i18ncp("@info:status", "%1 file", "%1 files", job->totalFiles()),
                                i18ncp("@info:status", "%1 folder", "%1 folders", job->totalSubdirs())));
    });
}

void MainWindow::testArchiveIntegrity()
{
    const KFileItemList items = activePane()->view()->selectedItems();
    for (const KFileItem &item : items) {
        if (Archives::browseUrl(item.mostLocalUrl(), item.mimetype()).isEmpty()) {
            continue;
        }
        if (item.localPath().isEmpty()) {
            showStatusMessage(i18nc("@info:status archive integrity", "Integrity testing requires a local archive."));
            return;
        }
        testArchiveFile(item.localPath());
    }
}

void MainWindow::testArchiveFile(const QString &fileName, const QString &password)
{
    using TestReport = QPair<Archives::TestResult, QString>;
    auto *watcher = new QFutureWatcher<TestReport>(this);
    connect(watcher, &QFutureWatcher<TestReport>::finished, this, [this, watcher, fileName, password] {
        const TestReport report = watcher->result();
        watcher->deleteLater();
        if (report.first == Archives::TestResult::PasswordRequired) {
            bool accepted = false;
            const QString entered = QInputDialog::getText(this,
                                                          i18nc("@title:window", "Archive Password"),
                                                          i18nc("@label:textbox", "Password for %1:", QFileInfo(fileName).fileName()),
                                                          QLineEdit::Password,
                                                          QString(),
                                                          &accepted);
            if (!accepted) {
                showStatusMessage(i18nc("@info:status archive integrity", "Archive test canceled."));
                return;
            }
            testArchiveFile(fileName, entered);
            return;
        }
        if (report.first == Archives::TestResult::Passed) {
            showStatusMessage(i18nc("@info:status archive integrity", "Archive is intact: %1", QFileInfo(fileName).fileName()));
        } else {
            showStatusMessage(i18nc("@info:status archive integrity", "Archive test failed for %1: %2", QFileInfo(fileName).fileName(), report.second));
        }
    });
    watcher->setFuture(QtConcurrent::run([fileName, password] {
        QString error;
        const Archives::TestResult result = Archives::testIntegrity(fileName, password, &error);
        return TestReport{result, error};
    }));
}

void MainWindow::jumpBack()
{
    Pane *pane = activePane();
    const QUrl target = m_jumpBackPoints.value(pane);
    if (!target.isValid()) {
        return;
    }
    const QUrl current = pane->url();
    m_jumpBackPoints.insert(pane, current);
    m_jumpingBack = true;
    pane->setUrl(target);
    m_jumpingBack = false;
    m_jumpBackAction->setEnabled(m_jumpBackPoints.value(pane).isValid());
}

void MainWindow::activateItem(const KFileItem &item)
{
    const QUrl archive = item.isDir() ? QUrl() : Archives::browseUrl(item.mostLocalUrl(), item.mimetype());
    if (item.isDir()) {
        activePane()->setUrl(item.targetUrl());
    } else if (archive.isValid()) {
        activePane()->setUrl(archive);
    } else {
        FileOperations::open(item, this);
    }
}

void MainWindow::activatePaneItem(Pane *pane, const KFileItem &item)
{
    setActivePane(pane);
    const QUrl target = item.targetUrl();
    const bool executable = target.isLocalFile() && Executables::isProgram(item.mimetype(), QFileInfo(target.toLocalFile()).isExecutable());
    const bool archive = !item.isDir() && Archives::browseUrl(item.mostLocalUrl(), item.mimetype()).isValid();
    if (item.isDir() || executable || archive) {
        activateItem(item);
        return;
    }
    const auto destination = Settings::fileActivationDestination();
    const QString mime = item.mimetype();
    if (destination == Settings::EnumFileActivationDestination::Gallery && item.url().isLocalFile()
        && (mime.startsWith(QLatin1String("image/")) || mime.startsWith(QLatin1String("video/")))) {
        pane->view()->selectUrls({item.url()});
        openGallery();
    } else if (destination == Settings::EnumFileActivationDestination::Preview) {
        pane->view()->selectUrls({item.url()});
        if (!m_previewPanelAction->isChecked()) {
            m_previewPanelAction->setChecked(true);
        } else {
            updatePreviewPanel();
        }
    } else {
        activateItem(item);
    }
}

void MainWindow::setupEditor()
{
    m_editor = new EditorPanel(m_splitter);
    m_splitter->addWidget(m_editor);
    m_editor->hide();
    m_editorDock = new QDockWidget(i18nc("@title:window", "Editor"), this);
    m_editorDock->setObjectName(QStringLiteral("editorDock"));
    m_editorDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
    setDockTitleVisible(m_editorDock, Settings::embeddedPanelTitlesVisible());
    addDockWidget(Qt::BottomDockWidgetArea, m_editorDock);
    m_editorDock->hide();
    m_editorWindow = new QDialog(this, Qt::Window);
    m_editorWindow->setObjectName(QStringLiteral("editorWindow"));
    m_editorWindow->setWindowTitle(i18nc("@title:window", "Editor"));
    m_editorWindow->resize(900, 650);
    new QVBoxLayout(m_editorWindow);
    connect(m_editor, &EditorPanel::placementRequested, this, [this](EditorPanel::Placement placement) {
        switch (placement) {
        case EditorPanel::Placement::Panel:
            Settings::setEditorOpening(Settings::EnumEditorOpening::Panel);
            break;
        case EditorPanel::Placement::Below:
        case EditorPanel::Placement::Right:
            Settings::setEditorOpening(Settings::EnumEditorOpening::Embedded);
            Settings::setEditorEmbeddedPlacement(placement == EditorPanel::Placement::Below ? Settings::EnumEditorEmbeddedPlacement::Below
                                                                                            : Settings::EnumEditorEmbeddedPlacement::Right);
            break;
        case EditorPanel::Placement::Maximized:
            Settings::setEditorOpening(Settings::EnumEditorOpening::Maximized);
            break;
        case EditorPanel::Placement::Window:
            Settings::setEditorOpening(Settings::EnumEditorOpening::Window);
            break;
        }
        Settings::self()->save();
        setEditorPlacement(placement);
    });
    connect(m_editor, &EditorPanel::hideRequested, this, [this] {
        switch (m_editorPlacement) {
        case EditorPanel::Placement::Panel:
            m_editor->hide();
            break;
        case EditorPanel::Placement::Below:
        case EditorPanel::Placement::Right:
            savePanelDockSize(m_editorDock, QStringLiteral("EditorDock"));
            m_editorDock->hide();
            break;
        case EditorPanel::Placement::Maximized:
        case EditorPanel::Placement::Window:
            m_editorWindow->hide();
            break;
        }
        toggleLayout(m_dualLayoutAction->isChecked());
    });
    connect(m_editorDock, &QDockWidget::visibilityChanged, this, [this] {
        if (m_movingEditor) {
            return;
        }
        if (m_editorDock->isHidden() && m_editorDock->widget() == m_editor
            && (m_editorPlacement == EditorPanel::Placement::Below || m_editorPlacement == EditorPanel::Placement::Right)) {
            savePanelDockSize(m_editorDock, QStringLiteral("EditorDock"));
        }
        toggleLayout(m_dualLayoutAction->isChecked());
    });
}

void MainWindow::setEditorPlacement(EditorPanel::Placement placement)
{
    const QScopedValueRollback<bool> moving(m_movingEditor, true);
    if (!m_editorDock->isHidden()) {
        savePanelDockSize(m_editorDock, QStringLiteral("EditorDock"));
    }
    m_editorPlacement = placement;
    m_editorWindow->hide();
    m_editorDock->hide();
    m_editor->setParent(nullptr);
    switch (placement) {
    case EditorPanel::Placement::Panel:
        m_splitter->addWidget(m_editor);
        m_editor->show();
        break;
    case EditorPanel::Placement::Below:
    case EditorPanel::Placement::Right:
        addDockWidget(placement == EditorPanel::Placement::Below ? Qt::BottomDockWidgetArea : Qt::RightDockWidgetArea, m_editorDock);
        m_editorDock->setWidget(m_editor);
        m_editorDock->show();
        m_editor->show();
        break;
    case EditorPanel::Placement::Maximized:
    case EditorPanel::Placement::Window:
        m_editorWindow->layout()->addWidget(m_editor);
        m_editor->show();
        if (placement == EditorPanel::Placement::Maximized) {
            m_editorWindow->showMaximized();
        } else {
            m_editorWindow->showNormal();
        }
        break;
    }
    const bool embedded =
        placement == EditorPanel::Placement::Panel || placement == EditorPanel::Placement::Below || placement == EditorPanel::Placement::Right;
    m_editor->setPanelTitleVisible(!embedded || Settings::embeddedPanelTitlesVisible());
    toggleLayout(m_dualLayoutAction->isChecked());
    restoreSplitterSizes(true, false);
    restoreEditorDockSize();
}

void MainWindow::openEditor()
{
    const KFileItemList items = activePane()->view()->selectedItems();
    showEditor(items.size() == 1 && !items.first().isDir() ? items.first().url() : QUrl());
}

bool MainWindow::showEditor(const QUrl &url)
{
    if (!m_editor) {
        setupEditor();
    }
    const bool alreadyOpen = (m_editorPlacement == EditorPanel::Placement::Panel && !m_editor->isHidden())
        || ((m_editorPlacement == EditorPanel::Placement::Below || m_editorPlacement == EditorPanel::Placement::Right) && !m_editorDock->isHidden())
        || ((m_editorPlacement == EditorPanel::Placement::Maximized || m_editorPlacement == EditorPanel::Placement::Window) && !m_editorWindow->isHidden());
    if (!url.isEmpty() && !m_editor->openUrl(url)) {
        return false;
    }
    if (alreadyOpen) {
        if (m_editorPlacement == EditorPanel::Placement::Maximized || m_editorPlacement == EditorPanel::Placement::Window) {
            if (m_editorWindow->isMinimized()) {
                if (m_editorPlacement == EditorPanel::Placement::Maximized) {
                    m_editorWindow->showMaximized();
                } else {
                    m_editorWindow->showNormal();
                }
            }
            m_editorWindow->raise();
            m_editorWindow->activateWindow();
        }
        return true;
    }
    switch (Settings::editorOpening()) {
    case Settings::EnumEditorOpening::Embedded:
        setEditorPlacement(Settings::editorEmbeddedPlacement() == Settings::EnumEditorEmbeddedPlacement::Right ? EditorPanel::Placement::Right
                                                                                                               : EditorPanel::Placement::Below);
        break;
    case Settings::EnumEditorOpening::Maximized:
        setEditorPlacement(EditorPanel::Placement::Maximized);
        break;
    case Settings::EnumEditorOpening::Window:
        setEditorPlacement(EditorPanel::Placement::Window);
        break;
    default:
        setEditorPlacement(EditorPanel::Placement::Panel);
        break;
    }
    return true;
}

void MainWindow::editFile()
{
    const KFileItemList items = activePane()->view()->selectedItems();
    if (items.size() != 1 || items.first().isDir()) {
        showStatusMessage(i18nc("@info:status", "Select one file to edit."));
        return;
    }
    const KFileItem &item = items.first();
    const QUrl url = item.mostLocalUrl();
    const QString mimeType = item.mimetype();
    // The editor chosen under Applications › Editor opens text; an unavailable one is reported rather than replaced.
    if (const std::optional<EditorProviders::Editor> editor = EditorProviders::preferredEditor();
        editor && EditTools::category(mimeType) == EditTools::Category::Text) {
        editWithEditor(*editor);
        return;
    }
    QString error;
    const std::optional<EditTools::Choice> choice = EditTools::choose(url, mimeType, &error);
    if (!choice) {
        showStatusMessage(error);
        return;
    }
    if (choice->tool == EditTools::Tool::BuiltInEditor) {
        openEditor();
        return;
    }
    const QString name = EditTools::name(choice->tool);
    auto *job = new KIO::CommandLauncherJob(choice->executable, EditTools::arguments(url), this);
    job->setDesktopName(EditTools::desktopName(choice->tool, mimeType));
    connect(job, &KJob::result, this, [this, name](KJob *finished) {
        if (finished->error()) {
            showStatusMessage(i18nc("@info:status %1 is an application, %2 the error", "Could not start %1: %2", name, finished->errorString()));
        }
    });
    job->start();
}

void MainWindow::editWithEditor(EditorProviders::Editor editor)
{
    const QString executable = EditorProviders::findExecutable(editor);
    if (executable.isEmpty()) {
        showStatusMessage(EditorProviders::unavailableReason(editor));
        return;
    }
    const KFileItemList items = activePane()->view()->selectedItems();
    QList<QUrl> urls;
    urls.reserve(items.size());
    for (const KFileItem &item : items) {
        urls.append(item.mostLocalUrl());
    }
    QString error;
    const QStringList arguments = EditorProviders::arguments(editor, urls, &error);
    if (!error.isEmpty()) {
        showStatusMessage(error);
        return;
    }
    const QString name = EditorProviders::name(editor);
    if (EditorProviders::runsInTerminal(editor) && EditorProviders::usesSeparateTerminal()) {
        // KDE's configured terminal application, as for Open Terminal Here.
        auto *job = new KTerminalLauncherJob(EditorProviders::terminalCommand(executable, arguments), this);
        job->setWorkingDirectory(EditorProviders::workingDirectory(urls));
        connect(job, &KJob::result, this, [this, name](KJob *finished) {
            if (finished->error()) {
                showStatusMessage(i18nc("@info:status %1 is a text editor, %2 the error", "Could not start %1: %2", name, finished->errorString()));
            }
        });
        job->start();
        return;
    }
    if (EditorProviders::runsInTerminal(editor)) {
        // Showing the panel through its action would also start a shell session.
        if (!m_terminalPanelAction->isChecked()) {
            const QSignalBlocker blocker(m_terminalPanelAction);
            m_terminalPanelAction->setChecked(true);
            updateTerminalPanelVisibility();
        }
        m_terminalPanel->openProgram(executable, arguments, EditorProviders::workingDirectory(urls), name);
        return;
    }
    auto *job = new KIO::CommandLauncherJob(executable, arguments, this);
    job->setDesktopName(EditorProviders::desktopName(editor));
    connect(job, &KJob::result, this, [this, name](KJob *finished) {
        if (finished->error()) {
            showStatusMessage(i18nc("@info:status %1 is a text editor, %2 the error", "Could not start %1: %2", name, finished->errorString()));
        }
    });
    job->start();
}

void MainWindow::openInOfficeApp(OfficeApps::App app)
{
    const KFileItemList items = activePane()->view()->selectedItems();
    QList<QUrl> urls;
    QStringList mimeTypes;
    urls.reserve(items.size());
    mimeTypes.reserve(items.size());
    for (const KFileItem &item : items) {
        urls.append(item.mostLocalUrl());
        mimeTypes.append(item.mimetype());
    }
    QString error;
    const QList<OfficeApps::Launch> launches = OfficeApps::launches(app, urls, mimeTypes, &error);
    if (!error.isEmpty()) {
        showStatusMessage(error);
        return;
    }
    const QString name = OfficeApps::name(app);
    for (const OfficeApps::Launch &launch : launches) {
        auto *job = new KIO::CommandLauncherJob(launch.executable, launch.arguments, this);
        job->setDesktopName(OfficeApps::desktopName(app, launch.kind));
        connect(job, &KJob::result, this, [this, name](KJob *finished) {
            if (finished->error()) {
                showStatusMessage(i18nc("@info:status %1 is an office application, %2 the error", "Could not start %1: %2", name, finished->errorString()));
            }
        });
        job->start();
    }
}

void MainWindow::updateTerminalPanelVisibility()
{
    m_terminalPanel->setVisible(m_terminalPanelAction->isChecked());
    toggleLayout(m_dualLayoutAction->isChecked());
    restoreSplitterSizes(false, true, true);
}

void MainWindow::openGallery()
{
    FolderView *view = activePane()->view();
    const KFileItemList selected = view->selectedItems();
    QSet<QUrl> selectedUrls;
    for (const KFileItem &item : selected) {
        selectedUrls.insert(item.url());
    }
    QList<QUrl> urls;
    QSet<QUrl> videos;
    for (const KFileItem &item : view->itemsInViewOrder()) {
        if (item.isDir() || !item.url().isLocalFile() || (selected.size() > 1 && !selectedUrls.contains(item.url()))) {
            continue;
        }
        const QString mime = item.mimetype();
        const bool video = mime.startsWith(QLatin1String("video/"));
        if (video || mime.startsWith(QLatin1String("image/"))) {
            urls.append(item.url());
            if (video) {
                videos.insert(item.url());
            }
        }
    }
    if (urls.isEmpty()) {
        showStatusMessage(i18nc("@info:status", "No local images or videos are available for the gallery."));
        return;
    }
    int index = urls.indexOf(view->currentItem().url());
    if (index < 0 && !selected.isEmpty()) {
        index = urls.indexOf(selected.first().url());
    }
    auto *gallery = new GalleryWindow(urls, videos, std::max(index, 0), this);
    const QPointer<FolderView> source(view);
    const QUrl sourceFolder = view->url();
    connect(gallery, &GalleryWindow::currentUrlChanged, gallery, [source, sourceFolder](const QUrl &url) {
        if (source && source->url() == sourceFolder) {
            source->selectUrls({url});
        }
    });
    connect(gallery, &QDialog::finished, this, [this, source] {
        if (source && activePane()->view() == source) {
            source->currentView()->setFocus();
        }
    });
    gallery->show();
}

void MainWindow::showContextMenu(Pane *pane, const QPoint &globalPos, const KFileItemList &items)
{
    QMenu menu(this);
    menu.setObjectName(QStringLiteral("fileContextMenu"));
    KActionCollection *actions = actionCollection();
    const KConfigGroup context = KSharedConfig::openConfig()->group(QStringLiteral("ContextMenu"));
    const QStringList hidden = Settings::hiddenContextActions();
    const auto visible = [&hidden](const char *id) {
        return !hidden.contains(QLatin1String(id));
    };

    if (items.isEmpty()) {
        if (visible("new_menu")) {
            m_newFileMenu->setWorkingDirectory(pane->url());
            m_newFileMenu->checkUpToDate();
            menu.addAction(m_newFileMenu);
        }
        menu.addSeparator();
        if (visible("paste")) {
            menu.addAction(m_pasteAction);
        }
        if (visible("new_tab") && context.readEntry("ShowOpenInNewTab", true)) {
            menu.addAction(actions->action(QStringLiteral("new_tab")));
        }
        menu.addSeparator();
        if (visible("refresh")) {
            menu.addAction(actions->action(stdName(KStandardAction::Redisplay)));
        }
        if (visible("terminal") && Settings::showOpenTerminal()) {
            menu.addAction(actions->action(QStringLiteral("open_terminal")));
            menu.addAction(actions->action(QStringLiteral("open_terminal_with")));
        }
        menu.addAction(actions->action(QStringLiteral("calculate_occupied_space")));
        menu.addAction(actions->action(QStringLiteral("content_chart")));
        QAction *folderLook = nullptr;
        if (pane->url().isLocalFile()) {
            folderLook = menu.addAction(i18nc("@action:inmenu", "Folder Look…"));
        }
        if (visible("copy_path") && Settings::showCopyPath()) {
            menu.addAction(actions->action(QStringLiteral("copy_path")));
        }
        menu.addSeparator();
        if (visible("properties")) {
            menu.addAction(actions->action(QStringLiteral("properties")));
        }
        if (menu.exec(globalPos) == folderLook && folderLook) {
            editFolderLook(pane->url());
        }
        return;
    }

    const bool singleFolder = items.size() == 1 && items.first().isDir();
    const KFileItemListProperties props(items);

    // Locked password-gated folders offer only actions that keep their contents hidden.
    const FolderGates *gates = FolderGates::instance();
    if (std::any_of(items.cbegin(), items.cend(), [gates](const KFileItem &item) {
            return gates->touchesLocked(item.url());
        })) {
        menu.setObjectName(QStringLiteral("lockedFolderContextMenu"));
        if (singleFolder && gates->isLockedGateItem(items.first())) {
            menu.addAction(actions->action(QStringLiteral("unlock_folder")));
            menu.addAction(actions->action(QStringLiteral("change_folder_password")));
            menu.addAction(actions->action(QStringLiteral("remove_folder_password")));
            menu.addSeparator();
        }
        if (visible("rename")) {
            menu.addAction(actions->action(QStringLiteral("rename")));
        }
        if (visible("properties")) {
            menu.addAction(actions->action(QStringLiteral("properties")));
        }
        menu.exec(globalPos);
        return;
    }

    QAction *openAction = nullptr;
    if (visible("open")) {
        openAction = menu.addAction(QIcon::fromTheme(QStringLiteral("document-open")), i18nc("@action:inmenu", "Open"));
    }
    QAction *openInTab = nullptr;
    QAction *openInOtherPane = nullptr;
    if (singleFolder) {
        if (visible("open_in_tab") && Settings::showOpenInNewTab()) {
            openInTab = menu.addAction(QIcon::fromTheme(QStringLiteral("tab-new")), i18nc("@action:inmenu", "Open in New Tab"));
        }
        if (m_dualLayoutAction->isChecked() && visible("open_in_other_pane") && Settings::showOpenInOtherPane()) {
            openInOtherPane = menu.addAction(QIcon::fromTheme(QStringLiteral("view-split-left-right")), i18nc("@action:inmenu", "Open in Other Pane"));
        }
    }
    m_fileItemActions->setItemListProperties(props);
    if (visible("open_with")) {
        m_fileItemActions->insertOpenWithActionsTo(nullptr, &menu, QStringList());
        const bool mediaSelection = std::all_of(items.cbegin(), items.cend(), [](const KFileItem &item) {
            const QString mime = item.mimetype();
            return !item.isDir() && (mime.startsWith(QLatin1String("audio/")) || mime.startsWith(QLatin1String("video/")));
        });
        if (mediaSelection) {
            auto *mpv = menu.addAction(i18nc("@action:inmenu", "Open with mpv"));
            mpv->setObjectName(QStringLiteral("openWithMpv"));
            connect(mpv, &QAction::triggered, this, [this, urls = urlsOf(items)] {
                const QString error = FileOperations::openWithMpv(urls);
                if (!error.isEmpty()) {
                    KMessageBox::error(this, error, i18nc("@title:window", "Could Not Open with mpv"));
                }
            });
        }
        const bool filesOnly = std::none_of(items.cbegin(), items.cend(), [](const KFileItem &item) {
            return item.isDir();
        });
        if (filesOnly) {
            if (items.size() == 1 && EditTools::category(items.first().mimetype()) != EditTools::Category::None) {
                menu.addAction(actions->action(QStringLiteral("edit_file")));
            }
            menu.addAction(actions->action(QStringLiteral("edit_with")));
        }
        const bool officeOnly = std::all_of(items.cbegin(), items.cend(), [](const KFileItem &item) {
            return !item.isDir() && OfficeApps::isOfficeDocument(item.mimetype());
        });
        if (officeOnly) {
            menu.addAction(actions->action(QStringLiteral("open_in_office")));
        }
    }

    menu.addSeparator();
    if (visible("cut")) {
        menu.addAction(actions->action(stdName(KStandardAction::Cut)));
    }
    if (visible("copy")) {
        menu.addAction(actions->action(stdName(KStandardAction::Copy)));
    }
    if (singleFolder && visible("paste")) {
        menu.addAction(m_pasteAction);
    }
    if (visible("copy_to")) {
        if (Settings::copyToSubmenu()) {
            addTransferMenu(&menu, items, false);
        } else if (Settings::showCopyToOtherPane()) {
            menu.addAction(actions->action(QStringLiteral("copy_to_other_pane")));
        }
    }
    if (visible("move_to")) {
        if (Settings::moveToSubmenu()) {
            addTransferMenu(&menu, items, true);
        } else if (Settings::showMoveToOtherPane()) {
            menu.addAction(actions->action(QStringLiteral("move_to_other_pane")));
        }
    }
    if (Settings::showCopyMoveToMenus()) {
        QList<QUrl> urls;
        urls.reserve(items.size());
        for (const KFileItem &item : items) {
            urls.append(item.url());
        }
        m_copyToMenu->setUrls(urls);
        m_copyToMenu->setReadOnly(!props.supportsMoving());
        m_copyToMenu->addActionsTo(&menu);
    }
    menu.addSeparator();
    if (visible("rename")) {
        menu.addAction(actions->action(QStringLiteral("rename")));
    }
    menu.addAction(actions->action(QStringLiteral("batch_rename")));
    menu.addAction(actions->action(QStringLiteral("create_symlink")));
    if (visible("trash")) {
        menu.addAction(actions->action(QStringLiteral("move_to_trash")));
    }
    if (visible("delete") && Settings::showDelete()) {
        menu.addAction(actions->action(QStringLiteral("delete")));
    }
    menu.addSeparator();
    if (singleFolder && items.first().isLocalFile() && visible("terminal") && Settings::showOpenTerminal()) {
        menu.addAction(actions->action(QStringLiteral("open_terminal")));
        menu.addAction(actions->action(QStringLiteral("open_terminal_with")));
    }
    if (singleFolder && !FolderGates::localPath(items.first().url()).isEmpty()) {
        if (gates->isGate(FolderGates::canonicalPath(FolderGates::localPath(items.first().url())))) {
            menu.addAction(actions->action(QStringLiteral("change_folder_password")));
            menu.addAction(actions->action(QStringLiteral("remove_folder_password")));
            menu.addAction(actions->action(QStringLiteral("lock_protected_folders")));
        } else {
            menu.addAction(actions->action(QStringLiteral("protect_folder")));
        }
    }
    if (actions->action(QStringLiteral("encryption"))->isEnabled()) {
        menu.addAction(actions->action(QStringLiteral("encryption")));
    }
    if (actions->action(QStringLiteral("send_by_email"))->isEnabled()) {
        menu.addAction(actions->action(QStringLiteral("send_by_email")));
    }
    if (singleFolder) {
        menu.addAction(actions->action(QStringLiteral("content_chart")));
        menu.addAction(actions->action(QStringLiteral("add_to_favorites")));
        if (items.first().isLocalFile()) {
            QAction *look = menu.addAction(i18nc("@action:inmenu", "Folder Look…"));
            connect(look, &QAction::triggered, this, [this, url = items.first().targetUrl()] {
                editFolderLook(url);
            });
        }
    }

    const bool allLocal = std::all_of(items.cbegin(), items.cend(), [](const KFileItem &item) {
        return !item.localPath().isEmpty();
    });
    if (allLocal && actions->action(QStringLiteral("assign_tags"))->isEnabled()) {
        menu.addAction(actions->action(QStringLiteral("assign_tags")));
        menu.addAction(actions->action(QStringLiteral("edit_fields")));
        menu.addAction(actions->action(QStringLiteral("import_metadata")));
    }
    if (!allLocal) {
        addNextcloudItemMenu(&menu, items);
    }
    if (allLocal) {
        QMenu *metadata = menu.addMenu(i18nc("@menu", "Metadata"));
        QMenu *labels = metadata->addMenu(i18nc("@menu", "Label"));
        QAction *clearLabel = labels->addAction(i18nc("@action:inmenu", "No Label"));
        connect(clearLabel, &QAction::triggered, this, [this] {
            setSelectionLabel(QString());
        });
        for (LabelColor color : LabelColors::all()) {
            const QString key = LabelColors::key(color);
            QAction *label = labels->addAction(labelText(color));
            connect(label, &QAction::triggered, this, [this, key] {
                setSelectionLabel(key);
            });
        }

        QMenu *colors = metadata->addMenu(i18nc("@menu", "File Colour"));
        QAction *clearColor = colors->addAction(i18nc("@action:inmenu", "No Colour"));
        connect(clearColor, &QAction::triggered, this, [this] {
            setSelectionColor(QString());
        });
        for (LabelColor color : LabelColors::all()) {
            const QString value = LabelColors::rgb(color).name();
            QAction *entry = colors->addAction(labelText(color));
            connect(entry, &QAction::triggered, this, [this, value] {
                setSelectionColor(value);
            });
        }

        QMenu *ratings = metadata->addMenu(i18nc("@menu", "Rating"));
        QAction *clearRating = ratings->addAction(i18nc("@action:inmenu", "No Rating"));
        connect(clearRating, &QAction::triggered, this, [this] {
            setSelectionRating(0);
        });
        for (int rating = 1; rating <= 5; ++rating) {
            QAction *entry = ratings->addAction(i18ncp("@action:inmenu", "%1 Star", "%1 Stars", rating));
            connect(entry, &QAction::triggered, this, [this, rating] {
                setSelectionRating(rating);
            });
        }
        QAction *note = metadata->addAction(i18nc("@action:inmenu", "Edit Note…"));
        connect(note, &QAction::triggered, this, &MainWindow::setSelectionNote);
        QAction *pin = metadata->addAction(i18nc("@action:inmenu", "Toggle Pin"));
        connect(pin, &QAction::triggered, this, &MainWindow::toggleSelectionPin);
    }
    const bool anyArchive = std::any_of(items.cbegin(), items.cend(), [this](const KFileItem &item) {
        return !item.isDir() && (Archives::browseUrl(item.mostLocalUrl(), item.mimetype()).isValid() || isArkOnlyArchive(item));
    });
    if (anyArchive) {
        menu.addAction(actions->action(QStringLiteral("extract_to_other_pane")));
        menu.addAction(actions->action(QStringLiteral("extract_protected_archive")));
        menu.addAction(actions->action(QStringLiteral("test_archive_integrity")));
        if (items.size() == 1) {
            menu.addAction(actions->action(QStringLiteral("open_in_ark")));
        }
    }
    if (items.size() == 1 && !selectedDiskImage().isNull()) {
        const bool attached = m_diskImages->pluginInstalled() && m_diskImages->status(items.first().localPath()).state != DiskImageBackend::State::Detached;
        menu.addAction(actions->action(attached ? QStringLiteral("unmount_disk_image") : QStringLiteral("mount_disk_image")));
    }
    if (items.size() == 1 && !selectedPackage().isNull()) {
        menu.addAction(actions->action(QStringLiteral("application_package")));
    }
    if (allLocal) {
        menu.addAction(actions->action(QStringLiteral("compress")));
        if (Ark::isInstalled()) {
            menu.addAction(actions->action(QStringLiteral("add_to_archive")));
        }
    }
    menu.addAction(actions->action(QStringLiteral("calculate_occupied_space")));
    if (visible("copy_path") && Settings::showCopyPath()) {
        menu.addAction(actions->action(QStringLiteral("copy_path")));
    }

    // Mount Disk Image replaces the Mount ISO plugin's own entries, which report no errors.
    m_fileItemActions->addActionsTo(&menu, KFileItemActions::MenuActionSource::All, {}, Settings::hiddenServiceActions() + QStringList{DiskImages::pluginId()});

    menu.addSeparator();
    if (visible("properties")) {
        menu.addAction(actions->action(QStringLiteral("properties")));
    }

    QAction *chosen = menu.exec(globalPos);
    if (chosen && chosen == openAction) {
        if (singleFolder) {
            activateItem(items.first());
        } else {
            for (const KFileItem &item : items) {
                FileOperations::open(item, this);
            }
        }
    } else if (chosen && chosen == openInTab) {
        pane->openInNewTab(items.first().targetUrl(), true);
    } else if (chosen && chosen == openInOtherPane) {
        inactivePane()->setUrl(items.first().targetUrl());
    }
}

void MainWindow::addTransferMenu(QMenu *menu, const KFileItemList &items, bool move)
{
    auto *submenu = menu->addMenu(move ? i18nc("@title:menu", "Move To") : i18nc("@title:menu", "Copy To"));
    submenu->setObjectName(move ? QStringLiteral("moveToMenu") : QStringLiteral("copyToMenu"));
    submenu->setEnabled(!move || KFileItemListProperties(items).supportsMoving());
    const QList<QUrl> sources = urlsOf(items);
    const auto transfer = [this, sources, move](const QUrl &destination) {
        if (move) {
            FileOperations::move(sources, destination, this);
        } else {
            FileOperations::copy(sources, destination, this);
        }
    };
    if ((!move && Settings::showCopyToOtherPane()) || (move && Settings::showMoveToOtherPane())) {
        connect(submenu->addAction(i18nc("@action:inmenu", "Other Pane")), &QAction::triggered, this, [this, transfer] {
            transfer(inactivePane()->url());
        });
    }
    connect(submenu->addAction(i18nc("@action:inmenu", "Home Folder")), &QAction::triggered, this, [transfer] {
        transfer(Settings::homeUrl());
    });
    connect(submenu->addAction(i18nc("@action:inmenu", "Choose Folder…")), &QAction::triggered, this, [this, transfer] {
        const QUrl destination = QFileDialog::getExistingDirectoryUrl(this, i18nc("@title:window", "Choose Destination Folder"), activePane()->url());
        if (destination.isValid() && !destination.isEmpty()) {
            transfer(destination);
        }
    });
}

void MainWindow::compareFiles()
{
    const KFileItemList items = comparisonFiles(activePane(), inactivePane(), isDualLayout());
    if (items.isEmpty()) {
        KMessageBox::error(this, i18nc("@info", "Select two local files to compare."));
        return;
    }
    auto *dialog = new FileDiffDialog(items.at(0).url().toLocalFile(), items.at(1).url().toLocalFile(), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::createSymlink()
{
    const KFileItemList items = activePane()->view()->selectedItems();
    if (items.size() != 1 || !items.first().url().isLocalFile()) {
        KMessageBox::error(this, i18nc("@info", "Select one local file or folder."));
        return;
    }
    const KFileItem item = items.first();
    bool accepted = false;
    const QString suffix = Settings::symlinkSuffix();
    const QString name =
        QInputDialog::getText(this,
                              i18nc("@title:window", "Create Symbolic Link"),
                              i18nc("@label:textbox", "Link name:"),
                              QLineEdit::Normal,
                              suffix.isEmpty() ? i18nc("@item:intext default symbolic link name", "%1 - link", item.name()) : item.name() + suffix,
                              &accepted);
    if (!accepted) {
        return;
    }
    QString error;
    if (!FileOperations::createSymlink(item.url(), name, error, this)) {
        KMessageBox::error(this, error);
    }
}

void MainWindow::exportListing(bool pdf)
{
    FolderView *view = activePane()->view();
    if (view->isLoading()) {
        KMessageBox::information(this, i18nc("@info", "Wait for the folder listing to finish before exporting."));
        return;
    }
    QList<int> columns;
    if (Settings::listingExportCustomColumns()) {
        columns.append(KDirModel::Name);
        if (Settings::listingExportType()) {
            columns.append(KDirModel::Type);
        }
        if (Settings::listingExportSize()) {
            columns.append(KDirModel::Size);
        }
        if (Settings::listingExportModified()) {
            columns.append(KDirModel::ModifiedTime);
        }
        if (Settings::listingExportPermissions()) {
            columns.append(KDirModel::Permissions);
        }
        if (Settings::listingExportOwner()) {
            columns.append(KDirModel::Owner);
        }
        if (Settings::listingExportGroup()) {
            columns.append(KDirModel::Group);
        }
    }
    const ListingExport::Table table = view->listingForExport(columns);
    QFileDialog dialog(this, i18nc("@title:window", "Export Listing"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDefaultSuffix(pdf ? QStringLiteral("pdf") : QStringLiteral("csv"));
    dialog.setNameFilter(pdf ? i18nc("@item:inlistbox", "PDF files (*.pdf)") : i18nc("@item:inlistbox", "CSV files (*.csv)"));
    if (dialog.exec() != QDialog::Accepted || dialog.selectedFiles().isEmpty()) {
        return;
    }
    const QString destination = dialog.selectedFiles().first();
    const QString error = pdf ? ListingExport::toPdf(table, destination) : ListingExport::toCsv(table, destination);
    if (!error.isEmpty()) {
        KMessageBox::error(this, i18nc("@info", "Could not export the listing: %1", error));
        return;
    }
    showStatusMessage(i18nc("@info:status", "Listing exported to %1", destination));
}

void MainWindow::showContentChart()
{
    const KFileItemList items = activePane()->view()->selectedItems();
    const QUrl folder = items.size() == 1 && items.first().isDir() ? items.first().url() : activePane()->url();
    if (!folder.isValid() || folder.isEmpty()) {
        KMessageBox::error(this, i18nc("@info", "No folder is available to scan."));
        return;
    }
    auto *dialog = new ContentChartDialog(folder, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::showSearch()
{
    showSearchForPane(activePane());
}

void MainWindow::showSearchForPane(Pane *pane)
{
    if (pane != activePane()) {
        setActivePane(pane);
    }
    SearchWidget *search = m_searchPanel->searchWidget();
    if (!search->setFolder(pane->url())) {
        showStatusMessage(i18nc("@info:status", "A search is running. Cancel it to search another folder."));
    } else if (m_yaziSearchPane) {
        // A search started from here isn't one the Yazi keys asked for.
        m_yaziSearchPane->yaziColumns()->clearSearchStatus();
        m_yaziSearchPane = nullptr;
    }
    m_searchPanelAction->setChecked(true);
    search->focusQuery();
}

void MainWindow::showYaziSearch(Pane *pane, bool contents)
{
    SearchWidget *search = m_searchPanel->searchWidget();
    const bool running = search->isRunning();
    showSearchForPane(pane);
    if (running) {
        if (m_yaziSearchPane != pane) {
            pane->yaziColumns()->setSearchStatus(i18nc("@info:status", "A search is running. Cancel it to search another folder."), false);
        }
        return;
    }
    m_yaziSearchPane = pane;
    search->prepareFolderSearch(contents ? LiveSearch::Field::Contents : LiveSearch::Field::FileName);
    if (const QString limitation = search->locationLimitation(); !limitation.isEmpty()) {
        pane->yaziColumns()->setSearchStatus(limitation, false);
    }
}

void MainWindow::reportYaziSearch(const QString &status, bool running)
{
    if (m_yaziSearchPane) {
        m_yaziSearchPane->yaziColumns()->setSearchStatus(status, running);
    }
}

void MainWindow::searchFromToolbar(const QString &text)
{
    if (text.trimmed().isEmpty()) {
        showSearch();
        return;
    }
    QWidget *field = QApplication::focusWidget();
    SearchWidget *search = m_searchPanel->searchWidget();
    if (!search->isRunning() && m_yaziSearchPane) {
        m_yaziSearchPane->yaziColumns()->clearSearchStatus();
        m_yaziSearchPane = nullptr;
    }
    // A running search keeps its folder; search() then reports that it is busy.
    search->setFolder(activePane()->url());
    if (Settings::toolbarSearchOpensPanel()) {
        m_searchPanelAction->setChecked(true);
    }
    presentToolbarSearch(search->search(text));
    if (field) {
        field->setFocus(Qt::OtherFocusReason);
    }
}

void MainWindow::presentToolbarSearch(bool started)
{
    m_reportToolbarSearch = started && !m_searchPanelAction->isChecked();
    if (!started && !m_searchPanelAction->isChecked()) {
        showStatusMessage(m_searchPanel->searchWidget()->status());
    }
}

void MainWindow::showDuplicateFinder()
{
    const QUrl folder = activePane()->url();
    if (!folder.isLocalFile()) {
        KMessageBox::error(this, i18nc("@info", "Duplicate finding requires a local folder."));
        return;
    }
    auto *dialog = new DuplicateFinderDialog(folder, this);
    connect(dialog, &DuplicateFinderDialog::trashRequested, this, [this](const QList<QUrl> &urls) {
        FileOperations::trash(urls, this);
    });
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::createChecksumFile()
{
    const KFileItemList items = activePane()->view()->selectedItems();
    if (items.isEmpty() || !std::all_of(items.cbegin(), items.cend(), [](const KFileItem &item) {
            return item.url().isLocalFile() && item.isFile() && !item.isLink();
        })) {
        KMessageBox::error(this, i18nc("@info", "Select local files that are not symbolic links."));
        return;
    }
    QStringList paths;
    for (const KFileItem &item : items) {
        paths.append(item.url().toLocalFile());
    }
    QFileDialog chooser(this, i18nc("@title:window", "Create Checksum File"), QFileInfo(paths.first()).absolutePath());
    chooser.setAcceptMode(QFileDialog::AcceptSave);
    chooser.setFileMode(QFileDialog::AnyFile);
    chooser.setDefaultSuffix(QStringLiteral("sha256"));
    chooser.setNameFilter(i18nc("@item:inlistbox", "SHA-256 checksum files (*.sha256)"));
    chooser.selectFile(Settings::checksumFileName());
    if (chooser.exec() != QDialog::Accepted || chooser.selectedFiles().isEmpty()) {
        return;
    }
    auto *dialog = new ChecksumDialog(ChecksumDialog::Mode::Create, paths, chooser.selectedFiles().first(), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

PlasmaVaultMenu *MainWindow::plasmaVaultMenu() const
{
    return m_plasmaVaultMenu;
}

void MainWindow::setVaultClient(VaultClient *client)
{
    delete m_plasmaVaultMenu;
    m_plasmaVaultMenu = nullptr;
    if (!client) {
        return;
    }
    client->setParent(this);
    auto *menu = qobject_cast<KActionMenu *>(actionCollection()->action(QStringLiteral("plasma_vaults")));
    Q_ASSERT(menu);
    m_plasmaVaultMenu = new PlasmaVaultMenu(menu->menu(), client, this);
    // The menu doesn't own the client, so a replaced client goes with it.
    connect(m_plasmaVaultMenu, &QObject::destroyed, client, &QObject::deleteLater);
    m_plasmaVaultMenu->setConfirmClose([this](const PlasmaVaults::Vault &vault, const QStringList &users) {
        return KMessageBox::warningContinueCancelList(this,
                                                      xi18nc("@info",
                                                             "These programs are using files in the vault <resource>%1</resource>. Close them first, "
                                                             "or Plasma Vault may refuse to close it.",
                                                             vault.displayName()),
                                                      users,
                                                      i18nc("@title:window", "Close Vault"),
                                                      KGuiItem(i18nc("@action:button", "Close Vault"), QStringLiteral("object-locked")))
            == KMessageBox::Continue;
    });
    connect(m_plasmaVaultMenu, &PlasmaVaultMenu::showRequested, this, [this](const QUrl &url) {
        activePane()->setUrl(url);
    });
    connect(m_plasmaVaultMenu, &PlasmaVaultMenu::statusMessage, this, [this](const QString &message) {
        showStatusMessage(message);
    });
}

KdeConnectClient *MainWindow::kdeConnectClient()
{
    if (!m_kdeConnect) {
        m_kdeConnect = new DBusKdeConnectClient(this);
    }
    return m_kdeConnect;
}

void MainWindow::setKdeConnectClient(KdeConnectClient *client)
{
    if (m_kdeConnect != client) {
        delete m_kdeConnect;
        m_kdeConnect = client;
        if (client) {
            client->setParent(this);
        }
    }
}

void MainWindow::sendByEmail()
{
    const KFileItemList items = activePane()->view()->selectedItems();
    if (items.isEmpty() || !Settings::sendByEmail()) {
        return;
    }
    auto *dialog = new SendByEmailDialog(items, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::sendToDevice()
{
    const KFileItemList items = activePane()->view()->selectedItems();
    if (items.isEmpty() || !Settings::kdeConnectSharing()) {
        return;
    }
    auto *dialog = new KdeConnectShareDialog(kdeConnectClient(), urlsOf(items), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::setActingAsAdministrator(bool enabled)
{
    Pane *pane = activePane();
    const QUrl url = pane->url();
    // The action follows the pane's URL, so it stays unchecked until the
    // user accepts the risks and the pane opens the admin: folder.
    updateActions();
    if (enabled == AdminMode::isAdminUrl(url)) {
        return;
    }
    if (!enabled) {
        pane->setUrl(AdminMode::toUser(url));
        return;
    }
    const QString title = i18nc("@title:window", "Act as Administrator");
    if (!AdminMode::canToggle(url)) {
        KMessageBox::error(this, i18nc("@info", "Only local folders can be opened as an administrator."), title);
        return;
    }
    if (!AdminMode::isAvailable()) {
        KMessageBox::error(this,
                           i18nc("@info",
                                 "Acting as an administrator needs KIO's admin worker, which the kio-admin package provides. Install kio-admin "
                                 "with your distribution's package manager, then try again."),
                           title);
        return;
    }
    const int answer = KMessageBox::warningContinueCancel(
        this,
        i18nc("@info",
              "While acting as an administrator, you can change, replace or delete any file or folder on this system, including the items "
              "it needs to start and other users' data. A single mistake can make the system unusable, often without a warning or error.\n\n"
              "KDE's authorization prompt asks for an administrator password before the folder opens, and deleted items skip the trash. "
              "The pane stops acting as an administrator when you turn this off or open a location outside the admin view, and it never "
              "reopens with administrator rights from a saved session or workspace."),
        i18nc("@title:window", "Risks of Acting as an Administrator"),
        KGuiItem(i18nc("@action:button", "Act as Administrator"), QStringLiteral("system-switch-user")),
        KStandardGuiItem::cancel(),
        QStringLiteral("ActAsAdministratorWarning"),
        KMessageBox::Notify | KMessageBox::Dangerous);
    if (answer != KMessageBox::Continue || activePane() != pane || pane->url() != url) {
        return;
    }
    pane->setUrl(AdminMode::toAdmin(url));
}

void MainWindow::shareOnNetwork()
{
    QStringList paths;
    const KFileItemList items = activePane()->view()->selectedItems();
    for (const KFileItem &item : items) {
        if (!item.localPath().isEmpty()) {
            paths.append(item.localPath());
        }
    }
    if (items.isEmpty() && activePane()->url().isLocalFile()) {
        paths.append(activePane()->url().toLocalFile());
    }
    if (paths.isEmpty()) {
        KMessageBox::error(this,
                           i18nc("@info", "Only local files and folders can be shared on the network."),
                           i18nc("@title:window", "Share on Local Network"));
        return;
    }
    auto *dialog = new LanShareDialog(paths, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

MountBackend *MainWindow::mountBackend()
{
    if (!m_mountBackend) {
        setMountBackend(new SystemMountBackend(this));
    }
    return m_mountBackend;
}

void MainWindow::setMountBackend(MountBackend *backend)
{
    if (m_mountBackend == backend) {
        return;
    }
    delete m_mountBackend;
    m_mountBackend = backend;
    if (!backend) {
        return;
    }
    backend->setParent(this);
    connect(backend, &MountBackend::finished, this, [this](const QString &id, MountOperation operation, const QString &error, const QString &mountPoint) {
        if (id != m_openAfterMount || operation != MountOperation::Mount) {
            return;
        }
        m_openAfterMount.clear();
        if (!error.isEmpty()) {
            KMessageBox::error(this, error, i18nc("@title:window", "Mount Points"));
        } else if (!mountPoint.isEmpty()) {
            activePane()->setUrl(QUrl::fromLocalFile(mountPoint));
        }
    });
}

void MainWindow::showMountManager()
{
    if (auto *existing = findChild<MountManagerDialog *>()) {
        existing->raise();
        existing->activateWindow();
        return;
    }
    auto *dialog = new MountManagerDialog(mountBackend(), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &MountManagerDialog::openRequested, this, [this](const QUrl &url) {
        activePane()->setUrl(url);
    });
    dialog->show();
}

void MainWindow::populateMountPoints()
{
    m_mountPointsMenu->clear();
    MountBackend *backend = mountBackend();
    backend->refresh();
    const QList<MountDevice> devices = backend->devices();
    const bool removableOnly = Settings::mountRemovableOnly();
    bool listed = false;
    for (const MountDevice &device : devices) {
        if (removableOnly && !device.removable) {
            continue;
        }
        listed = true;
        QString name = device.name;
        name.replace(QLatin1Char('&'), QStringLiteral("&&"));
        QString mountPoint = device.mountPoint;
        mountPoint.replace(QLatin1Char('&'), QStringLiteral("&&"));
        const QString text = mountPoint.isEmpty() || name == QFileInfo(device.mountPoint).fileName()
            ? (mountPoint.isEmpty() ? name : mountPoint)
            : i18nc("@action:inmenu %1 is a file system name, %2 its mount point", "%1 (%2)", name, mountPoint);
        QAction *action = m_mountPointsMenu->addAction(QIcon::fromTheme(device.iconName), text);
        action->setCheckable(true);
        action->setChecked(device.mounted);
        action->setData(device.id);
        const bool running = backend->isRunning(device.id);
        action->setEnabled(!running && (device.mounted || device.canMount));
        action->setToolTip(device.mounted ? i18nc("@info:tooltip", "Mounted; open it in the active pane")
                                          : i18nc("@info:tooltip", "Not mounted; mount it and open it in the active pane"));
        const QString id = device.id;
        const bool mounted = device.mounted;
        const QString path = device.mountPoint;
        connect(action, &QAction::triggered, this, [this, id, mounted, path] {
            if (mounted) {
                activePane()->setUrl(QUrl::fromLocalFile(path));
                return;
            }
            m_openAfterMount = id;
            mountBackend()->start(id, MountOperation::Mount);
        });
    }
    if (devices.isEmpty()) {
        m_mountPointsMenu->addAction(i18nc("@action:inmenu", "No file systems found"))->setEnabled(false);
    } else if (!listed) {
        QAction *none = m_mountPointsMenu->addAction(i18nc("@action:inmenu", "No removable devices"));
        none->setEnabled(false);
        none->setToolTip(i18nc("@info:tooltip", "Choose whether other file systems are listed under Preferences › KDE Plasma › Mount Manager"));
    }
    m_mountPointsMenu->setToolTipsVisible(true);
    m_mountPointsMenu->addSeparator();
    m_mountPointsMenu->addAction(actionCollection()->action(QStringLiteral("mount_manager")));
}

void MainWindow::populateRemoteConnections()
{
    m_remoteConnectionsMenu->clear();
    m_remoteConnectionsMenu->setToolTipsVisible(true);
    QString error;
    const QList<RemoteProfiles::Profile> profiles = RemoteProfiles::load(KSharedConfig::openConfig()->group(QStringLiteral("RemoteProfiles")), &error);
    const bool otherPane = isDualLayout() && !inactivePane()->isHidden();
    for (const RemoteProfiles::Profile &profile : profiles) {
        QString name = profile.name;
        name.replace(QLatin1Char('&'), QStringLiteral("&&"));
        QMenu *submenu = m_remoteConnectionsMenu->addMenu(QIcon::fromTheme(QStringLiteral("folder-network")), name);
        submenu->setToolTipsVisible(true);
        submenu->menuAction()->setToolTip(RemoteProfiles::url(profile).toDisplayString());
        submenu->addAction(SymbolicIcon::fromTheme(QStringLiteral("document-open-folder")), i18nc("@action:inmenu", "Open"), this, [this, profile] {
            openRemoteProfile(profile, false);
        });
        QAction *other = submenu->addAction(SymbolicIcon::fromTheme(QStringLiteral("view-split-left-right")),
                                            i18nc("@action:inmenu", "Open in Other Pane"),
                                            this,
                                            [this, profile] {
                                                openRemoteProfile(profile, true);
                                            });
        other->setEnabled(otherPane);
        if (RemoteProfiles::supportsSsh(profile.protocol)) {
            submenu->addAction(SymbolicIcon::fromTheme(QStringLiteral("utilities-terminal")),
                               i18nc("@action:inmenu", "Open SSH Terminal"),
                               this,
                               [this, profile] {
                                   openSshTerminal(profile);
                               });
        }
        submenu->addSeparator();
        RemoteMountManager *mounts = remoteMounts();
        if (mounts->isMounted(profile.id)) {
            submenu->addAction(SymbolicIcon::fromTheme(QStringLiteral("folder-open")), i18nc("@action:inmenu", "Open Mounted Folder"), this, [this, profile] {
                mountRemoteProfile(profile, true);
            });
            const QString id = profile.id;
            submenu->addAction(SymbolicIcon::fromTheme(QStringLiteral("media-eject")), i18nc("@action:inmenu", "Unmount"), this, [this, id] {
                unmountRemoteProfile(id);
            });
        } else {
            QAction *mount = submenu->addAction(SymbolicIcon::fromTheme(QStringLiteral("drive-harddisk")),
                                                i18nc("@action:inmenu", "Mount as Folder"),
                                                this,
                                                [this, profile] {
                                                    mountRemoteProfile(profile, true);
                                                });
            mount->setEnabled(!mounts->isStarting(profile.id));
            mount->setToolTip(mounts->isStarting(profile.id)
                                  ? i18nc("@info:tooltip", "Mounting…")
                                  : i18nc("@info:tooltip", "Mount it as a local folder that applications without KIO support can use, and open it"));
        }
        submenu->addAction(SymbolicIcon::fromTheme(QStringLiteral("configure")), i18nc("@action:inmenu", "Mount Options…"), this, [this, profile] {
            showRemoteMountOptions(profile);
        });
    }
    if (profiles.isEmpty()) {
        m_remoteConnectionsMenu->addAction(i18nc("@action:inmenu", "No saved connections"))->setEnabled(false);
    }
    if (!error.isEmpty()) {
        QAction *problem = m_remoteConnectionsMenu->addAction(SymbolicIcon::fromTheme(QStringLiteral("dialog-warning")), error);
        problem->setEnabled(false);
    }
    m_remoteConnectionsMenu->addSeparator();
    if (Settings::remoteShowNetwork()) {
        QAction *network = actionCollection()->action(QStringLiteral("network_locations"));
        network->setEnabled(!RemoteProfiles::networkUrl().isEmpty());
        m_remoteConnectionsMenu->addAction(network);
    }
    m_remoteConnectionsMenu->addAction(actionCollection()->action(QStringLiteral("manage_remote_connections")));
}

void MainWindow::populateOnlineAccounts()
{
    QString error;
    // The accounts are read only after opting in.
    const QList<OnlineAccounts::Account> accounts = Settings::onlineAccountsEnabled() ? OnlineAccounts::readAccounts(&error) : QList<OnlineAccounts::Account>();
    OnlineAccountsMenu::populate(
        m_onlineAccountsMenu,
        OnlineAccounts::shownAccounts(accounts, Settings::onlineAccountsHidden()),
        error,
        [](const QString &protocol) {
            return KProtocolInfo::isKnownProtocol(protocol);
        },
        isDualLayout() && !inactivePane()->isHidden(),
        actionCollection()->action(QStringLiteral("manage_online_accounts")),
        this,
        [this](const QUrl &url, bool otherPane) {
            if (otherPane && isDualLayout() && !inactivePane()->isHidden()) {
                inactivePane()->setUrl(url);
                return;
            }
            activePane()->setUrl(url);
        });
}

QList<NextcloudAccounts::Account> MainWindow::nextcloudAccounts(QString *error) const
{
    QString loadError;
    const QList<NextcloudAccounts::Account> saved = NextcloudAccounts::load(KSharedConfig::openConfig()->group(NextcloudAccounts::groupName()), &loadError);
    QString onlineError;
    // Online Accounts are read only after opting in.
    const QList<NextcloudAccounts::Account> online = Settings::onlineAccountsEnabled()
        ? NextcloudAccounts::fromOnlineAccounts(OnlineAccounts::shownAccounts(OnlineAccounts::readAccounts(&onlineError), Settings::onlineAccountsHidden()))
        : QList<NextcloudAccounts::Account>();
    if (error) {
        *error = !loadError.isEmpty() ? loadError : onlineError;
    }
    return NextcloudAccounts::merged(saved, online);
}

void MainWindow::populateNextcloud()
{
    QString error;
    m_nextcloudAccounts = nextcloudAccounts(&error);
    m_nextcloudAccountsLoaded = true;
    NextcloudMenu::AccountHandlers more;
    if (Settings::nextcloudPhotosShowAlbums() || Settings::nextcloudPhotosShowSharedAlbums() || Settings::nextcloudPhotosShowPlaces()) {
        more.photos = [this](const NextcloudAccounts::Account &account) {
            showNextcloudPhotos(account);
        };
    }
    if (Settings::nextcloudShares()) {
        more.shares = [this](const NextcloudAccounts::Account &account) {
            showNextcloudShares(account);
        };
    }
    NextcloudMenu::populate(
        m_nextcloudMenu,
        m_nextcloudAccounts,
        error,
        RemoteProfiles::unavailableReason(RemoteProfiles::Protocol::WebDavs),
        isDualLayout() && !inactivePane()->isHidden(),
        actionCollection()->action(QStringLiteral("manage_nextcloud_accounts")),
        this,
        [this](const NextcloudAccounts::Account &account, bool otherPane) {
            openNextcloudAccount(account, otherPane);
        },
        [this](const NextcloudAccounts::Account &account) {
            showNextcloudAssets(account);
        },
        more);
    populateNextcloudDesktop(true);
}

void MainWindow::setNextcloudDesktopLocations(const QString &configuration, const QString &socket)
{
    m_nextcloudDesktopConfiguration = configuration;
    m_nextcloudDesktopSocket = socket;
    delete m_nextcloudDesktopQuery;
    m_nextcloudDesktopQuery = nullptr;
    m_nextcloudDesktopStatuses.clear();
    m_nextcloudDesktopChecked = false;
    m_nextcloudDesktopRunning = false;
}

void MainWindow::populateNextcloudDesktop(bool refresh)
{
    NextcloudMenu::DesktopClient client;
    client.configuration = NextcloudDesktop::readConfiguration(m_nextcloudDesktopConfiguration.isEmpty() ? NextcloudDesktop::defaultConfigurationPath()
                                                                                                         : m_nextcloudDesktopConfiguration);
    client.installed = NextcloudMenu::desktopClientService();
    // Without the client, or when it is turned off in Preferences, the submenu stays out of the way.
    if ((!client.configuration.exists && !client.installed) || !Settings::nextcloudDesktopClientMenu()) {
        if (m_nextcloudDesktopMenu) {
            m_nextcloudMenu->removeAction(m_nextcloudDesktopMenu->menuAction());
        }
        return;
    }
    if (!m_nextcloudDesktopMenu) {
        m_nextcloudDesktopMenu = new QMenu(i18nc("@title:menu", "Desktop Client"), m_nextcloudMenu);
        m_nextcloudDesktopMenu->setObjectName(QStringLiteral("nextcloudDesktopClient"));
        m_nextcloudDesktopMenu->setIcon(SymbolicIcon::fromTheme(QStringLiteral("folder-sync")));
        m_nextcloudDesktopMenu->menuAction()->setToolTip(
            i18nc("@info:tooltip", "Open the folders the Nextcloud desktop client synchronizes and see their status"));
    }
    client.statusEnabled = Settings::nextcloudDesktopStatus();
    if (refresh && client.statusEnabled) {
        if (!m_nextcloudDesktopQuery) {
            m_nextcloudDesktopQuery =
                new NextcloudDesktop::StatusQuery(m_nextcloudDesktopSocket.isEmpty() ? NextcloudDesktop::defaultSocketPath() : m_nextcloudDesktopSocket, this);
            connect(m_nextcloudDesktopQuery, &NextcloudDesktop::StatusQuery::finished, this, [this](bool running, const QHash<QString, QString> &statuses) {
                m_nextcloudDesktopChecked = true;
                m_nextcloudDesktopRunning = running;
                m_nextcloudDesktopStatuses = statuses;
                populateNextcloudDesktop(false);
            });
        }
        if (!m_nextcloudDesktopQuery->isRunning()) {
            QStringList paths;
            for (const NextcloudDesktop::Folder &folder : std::as_const(client.configuration.folders)) {
                paths.append(folder.localPath);
            }
            m_nextcloudDesktopQuery->start(paths);
        }
    }
    client.statusChecked = m_nextcloudDesktopChecked;
    client.running = m_nextcloudDesktopRunning;
    client.statuses = m_nextcloudDesktopStatuses;
    if (!m_nextcloudAccountsLoaded) {
        m_nextcloudAccounts = nextcloudAccounts();
        m_nextcloudAccountsLoaded = true;
    }
    NextcloudMenu::populateDesktopClient(m_nextcloudDesktopMenu,
                                         client,
                                         m_nextcloudAccounts,
                                         isDualLayout() && !inactivePane()->isHidden(),
                                         this,
                                         NextcloudMenu::DesktopHandlers{[this](const QString &path, bool otherPane) {
                                                                            const QUrl url = QUrl::fromLocalFile(path);
                                                                            if (otherPane && isDualLayout() && !inactivePane()->isHidden()) {
                                                                                inactivePane()->setUrl(url);
                                                                                return;
                                                                            }
                                                                            activePane()->setUrl(url);
                                                                        },
                                                                        [this](const QUrl &url) {
                                                                            const QString unavailable =
                                                                                RemoteProfiles::unavailableReason(RemoteProfiles::Protocol::WebDavs);
                                                                            if (!unavailable.isEmpty()) {
                                                                                showStatusMessage(unavailable);
                                                                                return;
                                                                            }
                                                                            activePane()->setUrl(url);
                                                                        },
                                                                        [this] {
                                                                            openNextcloudDesktopClient();
                                                                        }});
    QAction *submenu = m_nextcloudDesktopMenu->menuAction();
    if (!m_nextcloudMenu->actions().contains(submenu)) {
        QAction *manage = actionCollection()->action(QStringLiteral("manage_nextcloud_accounts"));
        m_nextcloudMenu->insertAction(m_nextcloudMenu->actions().contains(manage) ? manage : nullptr, submenu);
    }
}

void MainWindow::openNextcloudDesktopClient()
{
    const KService::Ptr service = NextcloudMenu::desktopClientService();
    if (!service) {
        showStatusMessage(i18nc("@info:status", "The Nextcloud desktop client isn't installed."));
        return;
    }
    // The client runs once per user: starting it again shows the running client.
    auto *job = new KIO::ApplicationLauncherJob(service, this);
    job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
    job->start();
}

void MainWindow::showNextcloudAccounts()
{
    if (!m_nextcloudStore) {
        m_nextcloudStore = std::make_unique<Nextcloud::PasswordServerStore>(winId());
    }
    auto *dialog = new NextcloudAccountsDialog(KSharedConfig::openConfig()->group(NextcloudAccounts::groupName()), m_nextcloudStore.get(), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    if (Settings::onlineAccountsEnabled()) {
        QString error;
        const QList<OnlineAccounts::Account> accounts = OnlineAccounts::readAccounts(&error);
        dialog->setOnlineAccounts(NextcloudAccounts::fromOnlineAccounts(OnlineAccounts::shownAccounts(accounts, Settings::onlineAccountsHidden())), error);
    }
    dialog->setOtherPaneAvailable(isDualLayout() && !inactivePane()->isHidden());
    connect(dialog, &NextcloudAccountsDialog::openRequested, this, &MainWindow::openNextcloudAccount);
    connect(dialog, &NextcloudAccountsDialog::accountsChanged, this, [this] {
        m_nextcloudAccounts = nextcloudAccounts();
        m_nextcloudAccountsLoaded = true;
    });
    dialog->show();
}

void MainWindow::openNextcloudAccount(const NextcloudAccounts::Account &account, bool otherPane)
{
    const QString unavailable = RemoteProfiles::unavailableReason(RemoteProfiles::Protocol::WebDavs);
    if (!unavailable.isEmpty()) {
        showStatusMessage(unavailable);
        return;
    }
    const QUrl url = NextcloudAccounts::filesUrl(account);
    if (!m_nextcloudAccounts.contains(account)) {
        m_nextcloudAccounts = nextcloudAccounts();
        m_nextcloudAccountsLoaded = true;
    }
    if (otherPane && isDualLayout() && !inactivePane()->isHidden()) {
        inactivePane()->setUrl(url);
        return;
    }
    activePane()->setUrl(url);
}

namespace
{
/// Identifies a folder in MainWindow::m_nextcloudPermissions.
QString nextcloudFolderKey(const QUrl &url)
{
    return url.adjusted(QUrl::RemoveUserInfo | QUrl::StripTrailingSlash).toString();
}

/// Whether @p url may be a folder of a Nextcloud account, checked before looking for its account.
bool mayBeNextcloudFolder(const QUrl &url)
{
    return (url.scheme() == QLatin1String("webdavs") || url.scheme() == QLatin1String("webdav"))
        && url.path().contains(QLatin1String("/remote.php/dav/files/"));
}
}

std::optional<Nextcloud::FolderPermissions> MainWindow::nextcloudPermissions(const QUrl &url) const
{
    const auto permissions = m_nextcloudPermissions.constFind(nextcloudFolderKey(url));
    if (permissions == m_nextcloudPermissions.cend()) {
        return std::nullopt;
    }
    return *permissions;
}

void MainWindow::fetchNextcloudPermissions(Pane *pane)
{
    const QUrl folder = pane->url();
    if (!Settings::nextcloudCheckPermissions() || !mayBeNextcloudFolder(folder) || pane->view()->isFlat()) {
        return;
    }
    const std::optional<NextcloudAccounts::Account> account = nextcloudAccountFor(folder);
    const QString key = nextcloudFolderKey(folder);
    if (!account || m_nextcloudPermissionsPending.contains(key)) {
        return;
    }
    m_nextcloudPermissionsPending.insert(key);
    nextcloudClient()->fetchPermissions(*account, folder);
}

std::optional<NextcloudAccounts::Account> MainWindow::nextcloudAccountFor(const QUrl &url)
{
    if (!mayBeNextcloudFolder(url)) {
        return std::nullopt;
    }
    std::optional<NextcloudAccounts::Account> account = NextcloudAccounts::accountForUrl(m_nextcloudAccounts, url);
    if (!account && !m_nextcloudAccountsLoaded) {
        m_nextcloudAccounts = nextcloudAccounts();
        m_nextcloudAccountsLoaded = true;
        account = NextcloudAccounts::accountForUrl(m_nextcloudAccounts, url);
    }
    return account;
}

NextcloudClient *MainWindow::nextcloudClient()
{
    if (!m_nextcloudClient) {
        if (!m_nextcloudStore) {
            m_nextcloudStore = std::make_unique<Nextcloud::PasswordServerStore>(winId());
        }
        m_nextcloudClient = new NextcloudClient(this);
        m_nextcloudClient->setCredentialStore(m_nextcloudStore.get());
        connect(m_nextcloudClient, &NextcloudClient::permissionsFetched, this, &MainWindow::nextcloudPermissionsFetched);
        connect(m_nextcloudClient, &NextcloudClient::metadataFetched, this, &MainWindow::nextcloudMetadataFetched);
        connect(m_nextcloudClient, &NextcloudClient::previewFetched, this, [this](const QUrl &url, const QImage &image) {
            // Without a server preview, the panel downloads the file within its usual limits.
            if (m_previewPanel) {
                m_previewPanel->showServerPreview(url, image);
            }
        });
        connect(m_nextcloudClient, &NextcloudClient::favoriteSet, this, [this](const QUrl &url, bool favorite, const QString &error) {
            if (!m_nextcloudFavoritesPending.remove(nextcloudFolderKey(url))) {
                return;
            }
            if (error.isEmpty()) {
                ++m_nextcloudFavoritesChanged;
                forgetNextcloudMetadata({url});
            } else {
                m_nextcloudFavoriteErrors.append(
                    i18nc("@info %1 is a file name, %2 why it failed", "%1: %2", url.adjusted(QUrl::StripTrailingSlash).fileName(), error));
            }
            if (!m_nextcloudFavoritesPending.isEmpty()) {
                return;
            }
            if (m_nextcloudFavoriteErrors.isEmpty()) {
                showStatusMessage(favorite ? i18ncp("@info:status",
                                                    "Added %1 item to your Nextcloud favourites",
                                                    "Added %1 items to your Nextcloud favourites",
                                                    m_nextcloudFavoritesChanged)
                                           : i18ncp("@info:status",
                                                    "Removed %1 item from your Nextcloud favourites",
                                                    "Removed %1 items from your Nextcloud favourites",
                                                    m_nextcloudFavoritesChanged));
            } else {
                showStatusMessage(i18nc("@info:status %1 is a list of problems",
                                        "Couldn't change Nextcloud favourites: %1",
                                        m_nextcloudFavoriteErrors.join(QStringLiteral("; "))));
            }
            m_nextcloudFavoriteErrors.clear();
            m_nextcloudFavoritesChanged = 0;
        });
    }
    return m_nextcloudClient;
}

std::optional<Nextcloud::FileMetadata> MainWindow::cachedNextcloudMetadata(const KFileItem &item) const
{
    // Favourites and tags change without changing the file, so metadata is read again after a while.
    const qint64 maximumAge = Settings::nextcloudMetadataMaxAge();
    const auto cached = m_nextcloudMetadata.constFind(nextcloudFolderKey(item.url()));
    if (cached == m_nextcloudMetadata.cend() || cached->itemModified != item.time(KFileItem::ModificationTime)
        || cached->read.secsTo(QDateTime::currentDateTimeUtc()) > maximumAge) {
        return std::nullopt;
    }
    return cached->metadata;
}

void MainWindow::fetchNextcloudMetadata(const KFileItem &item)
{
    const std::optional<NextcloudAccounts::Account> account = nextcloudAccountFor(item.url());
    const QString key = nextcloudFolderKey(item.url());
    if (!account || m_nextcloudMetadataPending.contains(key)) {
        return;
    }
    m_nextcloudMetadataPending.insert(key, item.time(KFileItem::ModificationTime));
    nextcloudClient()->fetchMetadata(*account, item.url());
}

void MainWindow::nextcloudMetadataFetched(const QUrl &url, const Nextcloud::FileMetadata &metadata, const QString &error)
{
    const QString key = nextcloudFolderKey(url);
    const QDateTime modified = m_nextcloudMetadataPending.take(key);
    if (error.isEmpty()) {
        // Metadata is cheap to read again, so a bounded cache is enough.
        constexpr qsizetype cacheLimit = 256;
        if (m_nextcloudMetadata.size() >= cacheLimit) {
            m_nextcloudMetadata.clear();
        }
        m_nextcloudMetadata.insert(key, {modified, QDateTime::currentDateTimeUtc(), metadata});
    }
    if (const auto wanted = m_nextcloudPreviewsWanted.constFind(key); wanted != m_nextcloudPreviewsWanted.cend()) {
        if (error.isEmpty()) {
            fetchNextcloudPreview(wanted->first, metadata);
        } else {
            const QUrl previewUrl = wanted->first;
            m_nextcloudPreviewsWanted.remove(key);
            m_previewPanel->showServerPreview(previewUrl, {});
        }
    }
    if (m_informationPanel && m_informationDock->isVisible() && Settings::nextcloudServerMetadata()) {
        m_informationPanel->setServerMetadata(url, error.isEmpty() ? std::optional(metadata) : std::nullopt, error);
    }
}

bool MainWindow::requestNextcloudPreview(const KFileItem &item, const QSize &size)
{
    if (!Settings::nextcloudServerPreviews() || item.isDir() || !nextcloudAccountFor(item.url())) {
        return false;
    }
    const QString key = nextcloudFolderKey(item.url());
    // Only the latest item matters to the panel.
    m_nextcloudPreviewsWanted.clear();
    m_nextcloudPreviewsWanted.insert(key, {item.url(), Nextcloud::previewRequestSize(size)});
    if (const std::optional<Nextcloud::FileMetadata> cached = cachedNextcloudMetadata(item)) {
        fetchNextcloudPreview(item.url(), *cached);
    } else {
        fetchNextcloudMetadata(item);
    }
    return true;
}

void MainWindow::fetchNextcloudPreview(const QUrl &url, const Nextcloud::FileMetadata &metadata)
{
    const QString key = nextcloudFolderKey(url);
    const auto wanted = m_nextcloudPreviewsWanted.constFind(key);
    if (wanted == m_nextcloudPreviewsWanted.cend()) {
        return;
    }
    const auto [previewUrl, size] = *wanted;
    m_nextcloudPreviewsWanted.remove(key);
    const std::optional<NextcloudAccounts::Account> account = nextcloudAccountFor(previewUrl);
    // Asking for a preview the server says it can't render would only fail.
    if (!account || metadata.fileId.isEmpty() || metadata.hasPreview == false) {
        m_previewPanel->showServerPreview(previewUrl, {});
        return;
    }
    nextcloudClient()->fetchPreview(*account, metadata.fileId, size, previewUrl);
}

void MainWindow::updateNextcloudInformation(const KFileItem &item)
{
    if (item.isNull() || !m_informationDock->isVisible() || !nextcloudAccountFor(item.url())) {
        return;
    }
    if (!Settings::nextcloudServerMetadata()) {
        // Hides metadata shown before the setting changed.
        m_informationPanel->setServerMetadata(item.url(), std::nullopt);
        return;
    }
    if (const std::optional<Nextcloud::FileMetadata> cached = cachedNextcloudMetadata(item)) {
        m_informationPanel->setServerMetadata(item.url(), cached);
        return;
    }
    m_informationPanel->setServerMetadata(item.url(), std::nullopt, i18nc("@info:status", "Loading server metadata…"));
    fetchNextcloudMetadata(item);
}

void MainWindow::forgetNextcloudMetadata(const QList<QUrl> &urls)
{
    for (const QUrl &url : urls) {
        m_nextcloudMetadata.remove(nextcloudFolderKey(url));
    }
    updateInformationPanel();
}

void MainWindow::showNextcloudAssets(const NextcloudAccounts::Account &account)
{
    const QString unavailable = RemoteProfiles::unavailableReason(RemoteProfiles::Protocol::WebDavs);
    if (!unavailable.isEmpty()) {
        showStatusMessage(unavailable);
        return;
    }
    if (!m_nextcloudStore) {
        m_nextcloudStore = std::make_unique<Nextcloud::PasswordServerStore>(winId());
    }
    if (!m_nextcloudAccounts.contains(account)) {
        m_nextcloudAccounts = nextcloudAccounts();
        m_nextcloudAccountsLoaded = true;
    }
    auto *dialog = new NextcloudAssetsDialog(account, m_nextcloudStore.get(), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setResultLimit(Settings::nextcloudResultLimit());
    connect(dialog, &NextcloudAssetsDialog::showRequested, this, [this](const QUrl &url, bool folder) {
        if (folder) {
            activePane()->setUrl(url);
        } else {
            activePane()->revealUrls(url.adjusted(QUrl::RemoveFilename), {url});
        }
    });
    connect(dialog, &NextcloudAssetsDialog::itemsChanged, this, &MainWindow::forgetNextcloudMetadata);
    dialog->show();
}

namespace
{
/// The Photos choices of Preferences › Remote and Cloud › Nextcloud.
NextcloudPhotos::Options nextcloudPhotosOptions()
{
    NextcloudPhotos::Options options;
    options.albums = Settings::nextcloudPhotosShowAlbums();
    options.sharedAlbums = Settings::nextcloudPhotosShowSharedAlbums();
    options.places = Settings::nextcloudPhotosShowPlaces();
    switch (Settings::nextcloudPhotosGrouping()) {
    case Settings::EnumNextcloudPhotosGrouping::NoGrouping:
        options.grouping = Nextcloud::PhotoGrouping::None;
        break;
    case Settings::EnumNextcloudPhotosGrouping::ByMonth:
        options.grouping = Nextcloud::PhotoGrouping::Month;
        break;
    case Settings::EnumNextcloudPhotosGrouping::ByPlace:
        options.grouping = Nextcloud::PhotoGrouping::Place;
        break;
    default:
        // The grouping last chosen in the window.
        break;
    }
    options.confirmChanges = Settings::nextcloudPhotosConfirmChanges();
    return options;
}
}

void MainWindow::showNextcloudPhotos(const NextcloudAccounts::Account &account)
{
    const QString unavailable = RemoteProfiles::unavailableReason(RemoteProfiles::Protocol::WebDavs);
    if (!unavailable.isEmpty()) {
        showStatusMessage(unavailable);
        return;
    }
    if (!m_nextcloudStore) {
        m_nextcloudStore = std::make_unique<Nextcloud::PasswordServerStore>(winId());
    }
    if (!m_nextcloudAccounts.contains(account)) {
        m_nextcloudAccounts = nextcloudAccounts();
        m_nextcloudAccountsLoaded = true;
    }
    auto *dialog = new NextcloudPhotosDialog(account, m_nextcloudStore.get(), nextcloudPhotosOptions(), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &NextcloudPhotosDialog::showRequested, this, [this](const QUrl &url) {
        activePane()->revealUrls(url.adjusted(QUrl::RemoveFilename), {url});
    });
    dialog->show();
}

void MainWindow::showNextcloudShares(const NextcloudAccounts::Account &account, const std::optional<QUrl> &item, bool itemIsFolder)
{
    const QString unavailable = RemoteProfiles::unavailableReason(RemoteProfiles::Protocol::WebDavs);
    if (!unavailable.isEmpty()) {
        showStatusMessage(unavailable);
        return;
    }
    if (!m_nextcloudStore) {
        m_nextcloudStore = std::make_unique<Nextcloud::PasswordServerStore>(winId());
    }
    auto *dialog = new NextcloudSharesDialog(account, m_nextcloudStore.get(), item, itemIsFolder, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    // Links are offered for Send by Email only when the user hasn't turned it off.
    dialog->setSendByEmailEnabled(Settings::sendByEmail());
    dialog->setDefaultLinkExpiryDays(Settings::nextcloudLinkExpiryDays());
    connect(dialog, &NextcloudSharesDialog::showRequested, this, [this](const QUrl &url, bool folder) {
        if (folder) {
            activePane()->setUrl(url);
        } else {
            activePane()->revealUrls(url.adjusted(QUrl::RemoveFilename), {url});
        }
    });
    dialog->show();
}

void MainWindow::addToNextcloudAlbum(const QList<QUrl> &urls)
{
    const std::optional<NextcloudAccounts::Account> account = urls.isEmpty() ? std::nullopt : nextcloudAccountFor(urls.first());
    if (!account) {
        return;
    }
    if (!m_nextcloudStore) {
        m_nextcloudStore = std::make_unique<Nextcloud::PasswordServerStore>(winId());
    }
    auto *dialog = new NextcloudAddToAlbumDialog(*account, urls, m_nextcloudStore.get(), nextcloudPhotosOptions(), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &NextcloudAddToAlbumDialog::finished, this, &MainWindow::reportAddedToAlbum);
    dialog->show();
}

void MainWindow::reportAddedToAlbum(const QString &album, int added, const QStringList &errors)
{
    if (errors.isEmpty()) {
        showStatusMessage(i18ncp("@info:status %2 is an album name", "Added %1 file to the album “%2”.", "Added %1 files to the album “%2”.", added, album));
        return;
    }
    showStatusMessage(i18nc("@info:status %1 is an album name, %2 a list of problems",
                            "Some files couldn't be added to the album “%1”: %2",
                            album,
                            errors.join(QLatin1Char(' '))));
}

void MainWindow::showNextcloudTags(const QList<QUrl> &urls)
{
    const std::optional<NextcloudAccounts::Account> account = urls.isEmpty() ? std::nullopt : nextcloudAccountFor(urls.first());
    if (!account) {
        return;
    }
    if (!m_nextcloudStore) {
        m_nextcloudStore = std::make_unique<Nextcloud::PasswordServerStore>(winId());
    }
    QList<NextcloudTagsDialog::Item> items;
    for (const QUrl &url : urls) {
        // File IDs never change, so even outdated metadata saves reading them again.
        const auto cached = m_nextcloudMetadata.constFind(nextcloudFolderKey(url));
        items.append({url, cached != m_nextcloudMetadata.cend() ? cached->metadata.fileId : QString()});
    }
    auto *dialog = new NextcloudTagsDialog(*account, items, m_nextcloudStore.get(), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &NextcloudTagsDialog::tagsChanged, this, &MainWindow::forgetNextcloudMetadata);
    dialog->show();
}

void MainWindow::setNextcloudFavorite(const QList<QUrl> &urls, bool favorite)
{
    const std::optional<NextcloudAccounts::Account> account = urls.isEmpty() ? std::nullopt : nextcloudAccountFor(urls.first());
    if (!account) {
        return;
    }
    if (m_nextcloudFavoritesPending.isEmpty()) {
        m_nextcloudFavoriteErrors.clear();
        m_nextcloudFavoritesChanged = 0;
    }
    for (const QUrl &url : urls) {
        const QString key = nextcloudFolderKey(url);
        if (m_nextcloudFavoritesPending.contains(key)) {
            continue;
        }
        m_nextcloudFavoritesPending.insert(key);
        nextcloudClient()->setFavorite(*account, url, favorite);
    }
}

void MainWindow::addNextcloudItemMenu(QMenu *menu, const KFileItemList &items)
{
    if (items.isEmpty() || !mayBeNextcloudFolder(items.first().url())) {
        return;
    }
    const std::optional<NextcloudAccounts::Account> account = nextcloudAccountFor(items.first().url());
    if (!account) {
        return;
    }
    QList<QUrl> urls;
    std::optional<bool> favorite;
    bool known = true;
    for (const KFileItem &item : items) {
        if (NextcloudAccounts::accountForUrl({*account}, item.url()) != account) {
            return;
        }
        urls.append(item.url());
        const std::optional<Nextcloud::FileMetadata> cached = cachedNextcloudMetadata(item);
        if (!cached || !cached->favorite || (favorite && *favorite != *cached->favorite)) {
            known = false;
        } else {
            favorite = cached->favorite;
        }
    }
    NextcloudMenu::ItemHandlers handlers;
    if (Settings::nextcloudFavoritesAndTags()) {
        handlers.setFavorite = [this, urls](bool add) {
            setNextcloudFavorite(urls, add);
        };
        handlers.editTags = [this, urls] {
            showNextcloudTags(urls);
        };
    }
    handlers.organize = [this, account = *account] {
        showNextcloudAssets(account);
    };
    if ((Settings::nextcloudPhotosShowAlbums() || Settings::nextcloudPhotosShowSharedAlbums())
        && std::none_of(items.cbegin(), items.cend(), [](const KFileItem &item) {
               return item.isDir();
           })) {
        handlers.addToAlbum = [this, urls] {
            addToNextcloudAlbum(urls);
        };
    }
    if (Settings::nextcloudShares() && items.size() == 1 && !Nextcloud::sharePath(*account, items.first().url()).isEmpty()) {
        handlers.shares = [this, account = *account, url = items.first().url(), folder = items.first().isDir()] {
            showNextcloudShares(account, url, folder);
        };
    }
    NextcloudMenu::addItemMenu(menu, known ? favorite : std::nullopt, this, handlers);
}

void MainWindow::nextcloudPermissionsFetched(const QUrl &folder, const Nextcloud::FolderPermissions &permissions, const QString &error)
{
    const QString key = nextcloudFolderKey(folder);
    m_nextcloudPermissionsPending.remove(key);
    // Keep only the folders still shown, so that permissions never outlive their listing.
    QSet<QString> shown{nextcloudFolderKey(activePane()->url())};
    if (isDualLayout() && !inactivePane()->isHidden()) {
        shown.insert(nextcloudFolderKey(inactivePane()->url()));
    }
    m_nextcloudPermissions.removeIf([&shown](const auto &entry) {
        return !shown.contains(entry.key());
    });
    // Without permissions, nothing is disabled and the server reports what it refuses.
    if (!error.isEmpty() || !shown.contains(key) || !Settings::nextcloudCheckPermissions()) {
        m_nextcloudPermissions.remove(key);
        updateActions();
        return;
    }
    m_nextcloudPermissions.insert(key, permissions);
    updateActions();
    if (key == nextcloudFolderKey(activePane()->url()) && !Nextcloud::deniedReason(Nextcloud::Operation::Create, permissions, {}).isEmpty()) {
        showStatusMessage(i18nc("@info:status", "The Nextcloud server doesn't allow adding items to this folder."));
    }
}

void MainWindow::applyNextcloudPermissions(const KFileItemList &selected)
{
    const std::optional<Nextcloud::FolderPermissions> active = nextcloudPermissions(activePane()->url());
    std::optional<Nextcloud::FolderPermissions> other;
    if (isDualLayout() && !inactivePane()->isHidden()) {
        other = nextcloudPermissions(inactivePane()->url());
    }
    // Only the folder's own items have permissions; a flat list also shows deeper ones.
    QStringList names;
    const QUrl folder = activePane()->url().adjusted(QUrl::StripTrailingSlash);
    for (const KFileItem &item : selected) {
        if (item.url().adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).matches(folder, QUrl::RemoveUserInfo)) {
            names.append(item.name());
        }
    }
    auto denied = [&names](const std::optional<Nextcloud::FolderPermissions> &permissions, Nextcloud::Operation operation) {
        return permissions ? Nextcloud::deniedReason(operation, *permissions, names) : QString();
    };
    const char *defaultToolTip = "opusDefaultToolTip";
    const char *deniedToolTip = "opusNextcloudToolTip";
    auto restrict = [&](QAction *action, const QString &reason) {
        if (!action->property(defaultToolTip).isValid()) {
            action->setProperty(defaultToolTip, action->toolTip());
        }
        if (reason.isEmpty()) {
            // Restore the tooltip only if nothing else changed it since.
            const QString shown = action->property(deniedToolTip).toString();
            if (!shown.isEmpty() && action->toolTip() == shown) {
                action->setToolTip(action->property(defaultToolTip).toString());
            }
            action->setProperty(deniedToolTip, QString());
            return;
        }
        action->setEnabled(false);
        action->setToolTip(reason);
        action->setProperty(deniedToolTip, reason);
    };
    KActionCollection *actions = actionCollection();
    const QString move = denied(active, Nextcloud::Operation::Move);
    const QString intoOther = denied(other, Nextcloud::Operation::Create);
    restrict(actions->action(QStringLiteral("rename")), denied(active, Nextcloud::Operation::Rename));
    restrict(actions->action(QStringLiteral("delete")), denied(active, Nextcloud::Operation::Delete));
    restrict(actions->action(stdName(KStandardAction::Cut)), move);
    restrict(actions->action(QStringLiteral("move_to_other_pane")), !move.isEmpty() ? move : intoOther);
    restrict(actions->action(QStringLiteral("copy_to_other_pane")), intoOther);
    restrict(m_pasteAction, denied(active, Nextcloud::Operation::Create));
    restrict(actions->action(QStringLiteral("new_menu")), denied(active, Nextcloud::Operation::Create));
    restrict(actions->action(QStringLiteral("new_folder")), denied(active, Nextcloud::Operation::CreateFolder));
}

void MainWindow::applyNextcloudSettings()
{
    if (!Settings::nextcloudCheckPermissions()) {
        // Nothing stays disabled once the permissions are no longer checked.
        m_nextcloudPermissions.clear();
        m_nextcloudPermissionsPending.clear();
        updateActions();
        return;
    }
    for (Pane *pane : {m_left, m_right}) {
        if (!pane->isHidden() && !m_nextcloudPermissions.contains(nextcloudFolderKey(pane->url()))) {
            fetchNextcloudPermissions(pane);
        }
    }
}

void MainWindow::showRemoteConnections()
{
    auto *dialog = new RemoteProfilesDialog(KSharedConfig::openConfig()->group(QStringLiteral("RemoteProfiles")), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setNewProfileDefaults(RemoteProfilesEditor::defaultsFromSettings());
    dialog->setOtherPaneAvailable(isDualLayout() && !inactivePane()->isHidden());
    connect(dialog, &RemoteProfilesDialog::openRequested, this, [this, dialog](const QUrl &, bool otherPane) {
        openRemoteProfile(dialog->formProfile(), otherPane);
    });
    connect(dialog, &RemoteProfilesDialog::sshRequested, this, &MainWindow::openSshTerminal);
    connect(dialog, &QDialog::finished, this, [] {
        // Forget the mount options of removed connections.
        QStringList ids;
        const QList<RemoteProfiles::Profile> profiles = RemoteProfiles::load(KSharedConfig::openConfig()->group(QStringLiteral("RemoteProfiles")));
        for (const RemoteProfiles::Profile &profile : profiles) {
            ids.append(profile.id);
        }
        RemoteMounts::pruneOptions(KSharedConfig::openConfig()->group(QStringLiteral("RemoteMounts")), ids);
    });
    dialog->show();
}

void MainWindow::openRemoteProfile(const RemoteProfiles::Profile &profile, bool otherPane)
{
    const QString unavailable = RemoteProfiles::unavailableReason(profile.protocol);
    if (!unavailable.isEmpty()) {
        showStatusMessage(unavailable);
        return;
    }
    const QUrl url = RemoteProfiles::url(profile);
    if (otherPane && isDualLayout() && !inactivePane()->isHidden()) {
        inactivePane()->setUrl(url);
        return;
    }
    activePane()->setUrl(url);
}

void MainWindow::openSshTerminal(const RemoteProfiles::Profile &profile)
{
    if (!RemoteProfiles::supportsSsh(profile.protocol) || !RemoteProfiles::validationError(profile).isEmpty()) {
        return;
    }
    const QString ssh = QStandardPaths::findExecutable(QStringLiteral("ssh"));
    if (ssh.isEmpty()) {
        showStatusMessage(i18nc("@info:status", "The ssh command was not found. Install OpenSSH to open SSH terminals."));
        return;
    }
    // Showing the panel through its action would also start a shell session.
    if (!m_terminalPanelAction->isChecked()) {
        const QSignalBlocker blocker(m_terminalPanelAction);
        m_terminalPanelAction->setChecked(true);
        updateTerminalPanelVisibility();
    }
    m_terminalPanel->openProgram(ssh, RemoteProfiles::sshArguments(profile), QDir::homePath(), profile.name.trimmed());
}

RcloneMounts *MainWindow::rcloneMounts()
{
    if (m_rcloneMounts) {
        return m_rcloneMounts;
    }
    m_rcloneMounts = new RcloneMounts(this);
    m_rcloneMounts->setMountOptions(
        CloudActivity::rcloneMountOptions(CloudActivity::loadLimits(KSharedConfig::openConfig()->group(CloudActivity::limitsGroupName())),
                                          CloudActivity::rcloneCacheDir()));
    if (m_cloudActivityDialog) {
        m_cloudActivityDialog->setRcloneMounts(m_rcloneMounts);
    }
    connect(m_rcloneMounts, &RcloneMounts::remotesListed, this, [this](const QList<Rclone::Remote> &remotes, const QString &error) {
        m_rcloneRemotes = remotes;
        m_rcloneError = error;
        m_rcloneListed = true;
        if (m_cloudStorageMenu->isVisible()) {
            populateCloudStorage();
        }
    });
    connect(m_rcloneMounts, &RcloneMounts::mounted, this, [this](const QString &remote, const QString &path) {
        if (remote != m_openAfterRcloneMount) {
            return;
        }
        m_openAfterRcloneMount.clear();
        activePane()->setUrl(QUrl::fromLocalFile(path));
    });
    // Served remotes require a password that KDE's password server keeps, in
    // KDE Wallet when it's enabled, so KIO signs in without asking and without
    // the password appearing in the location or on rclone's command line.
    m_rcloneMounts->setPasswordProvider([this](const QString &, const QUrl &url, const std::function<void(const QString &)> &done) {
        KIO::AuthInfo info;
        info.url = url;
        info.username = Rclone::serveUser();
        info.realmValue = Rclone::serveRealm();
        KPasswdServerClient client;
        if (client.checkAuthInfo(&info, qlonglong(winId()), 0) && info.username == Rclone::serveUser() && !info.password.isEmpty()) {
            done(info.password);
            return;
        }
        info.url = url;
        info.username = Rclone::serveUser();
        info.realmValue = Rclone::serveRealm();
        info.password = Rclone::randomPassword();
        info.keepPassword = true;
        client.addAuthInfo(info, qlonglong(winId()));
        done(info.password);
    });
    connect(m_rcloneMounts, &RcloneMounts::served, this, [this](const QString &remote, const QUrl &url) {
        if (remote != m_openAfterRcloneMount) {
            return;
        }
        m_openAfterRcloneMount.clear();
        activePane()->setUrl(url);
    });
    connect(m_rcloneMounts, &RcloneMounts::serveStopped, this, [this](const QString &remote) {
        showStatusMessage(i18nc("@info:status %1 is an rclone remote", "Stopped serving %1", remote));
    });
    connect(m_rcloneMounts, &RcloneMounts::failed, this, [this](const QString &remote, const QString &error) {
        m_cloudActivity->addEvent(remote, error);
        if (remote != m_openAfterRcloneMount) {
            showStatusMessage(i18nc("@info:status %1 is an rclone remote, %2 the error", "Could not open %1: %2", remote, error));
            return;
        }
        m_openAfterRcloneMount.clear();
        KMessageBox::error(this,
                           i18nc("@info %1 is an rclone remote, %2 the error", "Could not open %1:\n%2", remote, error),
                           i18nc("@title:window", "Cloud Storage"));
    });
    connect(m_rcloneMounts, &RcloneMounts::unmounted, this, [this](const QString &remote) {
        showStatusMessage(i18nc("@info:status %1 is an rclone remote", "Unmounted %1", remote));
    });
    return m_rcloneMounts;
}

void MainWindow::populateCloudStorage()
{
    m_cloudStorageMenu->clear();
    m_cloudStorageMenu->setToolTipsVisible(true);
    RcloneMounts *mounts = rcloneMounts();
    if (!m_rcloneListed) {
        m_cloudStorageMenu->addAction(i18nc("@action:inmenu", "Loading…"))->setEnabled(false);
    } else if (!m_rcloneError.isEmpty()) {
        QAction *problem = m_cloudStorageMenu->addAction(SymbolicIcon::fromTheme(QStringLiteral("dialog-warning")), m_rcloneError);
        problem->setEnabled(false);
    } else if (m_rcloneRemotes.isEmpty()) {
        QAction *empty = m_cloudStorageMenu->addAction(i18nc("@action:inmenu", "No rclone remotes configured"));
        empty->setToolTip(i18nc("@info:tooltip", "Run rclone config in a terminal to add a remote"));
        empty->setEnabled(false);
    }
    const QStringList hidden = Settings::rcloneHiddenRemotes();
    const bool serve = Settings::rcloneOpenMode() == Settings::EnumRcloneOpenMode::WebDav;
    int shown = 0;
    for (const Rclone::Remote &remote : std::as_const(m_rcloneRemotes)) {
        if (hidden.contains(remote.name)) {
            continue;
        }
        ++shown;
        QString name = remote.name;
        name.replace(QLatin1Char('&'), QStringLiteral("&&"));
        QString type = remote.type;
        type.replace(QLatin1Char('&'), QStringLiteral("&&"));
        QAction *action =
            m_cloudStorageMenu->addAction(QIcon::fromTheme(QStringLiteral("folder-cloud")),
                                          type.isEmpty() ? name : i18nc("@action:inmenu %1 is an rclone remote, %2 its type", "%1 (%2)", name, type));
        action->setCheckable(true);
        const bool mounted = mounts->isMounted(remote.name);
        const bool served = mounts->isServed(remote.name);
        action->setChecked(mounted || served);
        action->setEnabled(!mounts->isStarting(remote.name));
        action->setToolTip(mounts->isStarting(remote.name) ? i18nc("@info:tooltip", "Opening…")
                               : mounted                   ? i18nc("@info:tooltip", "Mounted; open it in the active pane")
                               : served                    ? i18nc("@info:tooltip", "Served over WebDAV on this computer; open it in the active pane")
                               : serve                     ? i18nc("@info:tooltip", "Serve it over WebDAV on this computer and open it in the active pane")
                                                           : i18nc("@info:tooltip", "Mount it and open it in the active pane"));
        const QString id = remote.name;
        connect(action, &QAction::triggered, this, [this, id] {
            openRcloneRemote(id);
        });
    }
    if (shown == 0 && !m_rcloneRemotes.isEmpty()) {
        QAction *none = m_cloudStorageMenu->addAction(i18nc("@action:inmenu", "All rclone remotes are hidden"));
        none->setToolTip(i18nc("@info:tooltip", "Choose the remotes to show in Preferences › Remote and Cloud › rclone"));
        none->setEnabled(false);
    }
    const QStringList mounted = mounts->mountedRemotes();
    if (!mounted.isEmpty()) {
        m_cloudStorageMenu->addSection(i18nc("@title:menu", "Unmount"));
        for (const QString &remote : mounted) {
            QString name = remote;
            name.replace(QLatin1Char('&'), QStringLiteral("&&"));
            QAction *action = m_cloudStorageMenu->addAction(SymbolicIcon::fromTheme(QStringLiteral("media-eject")), name);
            action->setToolTip(i18nc("@info:tooltip", "Stop the mount Plasma Opus started"));
            connect(action, &QAction::triggered, this, [this, remote] {
                unmountRcloneRemote(remote);
            });
        }
    }
    const QStringList served = mounts->servedRemotes();
    if (!served.isEmpty()) {
        m_cloudStorageMenu->addSection(i18nc("@title:menu", "Stop Serving"));
        for (const QString &remote : served) {
            QString name = remote;
            name.replace(QLatin1Char('&'), QStringLiteral("&&"));
            QAction *action = m_cloudStorageMenu->addAction(SymbolicIcon::fromTheme(QStringLiteral("process-stop")), name);
            action->setToolTip(i18nc("@info:tooltip", "Stop the WebDAV server Plasma Opus started"));
            connect(action, &QAction::triggered, this, [this, remote] {
                unmountRcloneRemote(remote);
            });
        }
    }
}

void MainWindow::openRcloneRemote(const QString &remote)
{
    m_openAfterRcloneMount = remote;
    if (Settings::rcloneOpenMode() == Settings::EnumRcloneOpenMode::WebDav) {
        rcloneMounts()->serve(remote);
    } else {
        rcloneMounts()->mount(remote);
    }
}

void MainWindow::unmountRcloneRemote(const QString &remote)
{
    const bool served = rcloneMounts()->isServed(remote);
    if (!waitForCloudUploads(served
                                 ? i18nc("@info %1 is an rclone remote", "Files written to %1 are still uploading. Stopping now stops the uploads.", remote)
                                 : i18nc("@info %1 is an rclone remote", "Files written to %1 are still uploading. Unmounting now stops the uploads.", remote),
                             {remote},
                             false)) {
        return;
    }
    const QUrl servedUrl = rcloneMounts()->servedUrl(remote);
    const QString path = Rclone::mountPoint(remote);
    for (Pane *pane : {activePane(), inactivePane()}) {
        const QUrl url = pane->url();
        const bool inside = servedUrl.isEmpty() ? url.isLocalFile() && (url.toLocalFile() == path || url.toLocalFile().startsWith(path + QLatin1Char('/')))
                                                : url.scheme() == servedUrl.scheme() && url.host() == servedUrl.host() && url.port() == servedUrl.port();
        if (inside) {
            pane->setUrl(QUrl::fromLocalFile(QDir::homePath()));
        }
    }
    rcloneMounts()->unmount(remote);
}

namespace
{
const QString remoteMountPlacesKey = QStringLiteral("Places");

KConfigGroup remoteMountState()
{
    return KSharedConfig::openStateConfig()->group(QStringLiteral("RemoteMounts"));
}

/// Removes the Places entry this application added for @p url, leaving entries the user added.
void removeRemoteMountPlace(KFilePlacesModel *model, const QUrl &url)
{
    for (int row = model->rowCount() - 1; row >= 0; --row) {
        const QModelIndex index = model->index(row, 0);
        if (model->url(index).matches(url, QUrl::StripTrailingSlash)
            && model->bookmarkForIndex(index).metaDataItem(QStringLiteral("OnlyInApp")) == QCoreApplication::applicationName()) {
            model->removePlace(index);
        }
    }
    KConfigGroup state = remoteMountState();
    QStringList places = state.readEntry(remoteMountPlacesKey, QStringList());
    places.removeAll(url.toString());
    state.writeEntry(remoteMountPlacesKey, places);
    state.sync();
}
}

RemoteMountManager *MainWindow::remoteMounts()
{
    if (m_remoteMounts) {
        return m_remoteMounts;
    }
    m_remoteMounts = new RemoteMountManager(this);
    m_remoteMounts->setMountFolder(RemoteMounts::defaultsFromSettings().folder);
    m_remoteMounts->setUnmountOnExit(Settings::remoteDriveUnmountOnExit());
    if (m_cloudActivityDialog) {
        m_cloudActivityDialog->setRemoteMounts(m_remoteMounts);
    }
    connect(m_remoteMounts, &RemoteMountManager::mounted, this, [this](const QString &id, const QString &path) {
        const QString name = m_remoteMountNames.value(id);
        const QUrl url = QUrl::fromLocalFile(path);
        if (m_remoteMountPaths.value(id) != path) {
            m_remoteMountPaths.insert(id, path);
            // OnlyInApp keeps the entry out of other applications, which would keep showing it after the mount stops.
            m_placesModel->addPlace(name, url, QStringLiteral("folder-remote"), QCoreApplication::applicationName());
            KConfigGroup state = remoteMountState();
            QStringList places = state.readEntry(remoteMountPlacesKey, QStringList());
            if (!places.contains(url.toString())) {
                places.append(url.toString());
            }
            state.writeEntry(remoteMountPlacesKey, places);
            state.sync();
        }
        if (id != m_openAfterRemoteMount) {
            showStatusMessage(i18nc("@info:status %1 is a saved connection, %2 a folder", "Mounted %1 on %2", name, path));
            return;
        }
        m_openAfterRemoteMount.clear();
        activePane()->setUrl(url);
    });
    connect(m_remoteMounts, &RemoteMountManager::failed, this, [this](const QString &id, const QString &error) {
        const QString name = m_remoteMountNames.value(id);
        m_cloudActivity->addEvent(name.isEmpty() ? id : name, error);
        if (id != m_openAfterRemoteMount) {
            showStatusMessage(i18nc("@info:status %1 is a saved connection, %2 the error", "Could not mount %1: %2", name, error));
            return;
        }
        m_openAfterRemoteMount.clear();
        KMessageBox::error(this,
                           i18nc("@info %1 is a saved connection, %2 the error", "Could not mount %1:\n%2", name, error),
                           i18nc("@title:window", "Mount Remote Connection"));
    });
    connect(m_remoteMounts, &RemoteMountManager::unmounted, this, [this](const QString &id) {
        const QString path = m_remoteMountPaths.take(id);
        if (!path.isEmpty()) {
            removeRemoteMountPlace(m_placesModel, QUrl::fromLocalFile(path));
        }
        showStatusMessage(i18nc("@info:status %1 is a saved connection", "Unmounted %1", m_remoteMountNames.value(id)));
    });
    if (QCoreApplication *application = QCoreApplication::instance()) {
        // Mounts stop at quit without signals, so their Places entries are removed here.
        connect(application, &QCoreApplication::aboutToQuit, this, [this] {
            for (const QString &path : std::as_const(m_remoteMountPaths)) {
                removeRemoteMountPlace(m_placesModel, QUrl::fromLocalFile(path));
            }
            m_remoteMountPaths.clear();
        });
    }
    return m_remoteMounts;
}

void MainWindow::mountRemoteProfile(const RemoteProfiles::Profile &profile, bool open)
{
    m_remoteMountNames.insert(profile.id, profile.name.trimmed());
    if (open) {
        m_openAfterRemoteMount = profile.id;
    }
    remoteMounts()->mount(
        profile,
        RemoteMounts::loadOptions(KSharedConfig::openConfig()->group(QStringLiteral("RemoteMounts")), profile, RemoteMounts::defaultsFromSettings()));
}

void MainWindow::unmountRemoteProfile(const QString &id)
{
    const QString path = remoteMounts()->path(id);
    if (!path.isEmpty()) {
        for (Pane *pane : {activePane(), inactivePane()}) {
            const QUrl url = pane->url();
            if (url.isLocalFile() && (url.toLocalFile() == path || url.toLocalFile().startsWith(path + QLatin1Char('/')))) {
                pane->setUrl(QUrl::fromLocalFile(QDir::homePath()));
            }
        }
    }
    remoteMounts()->unmount(id);
}

void MainWindow::showRemoteMountOptions(const RemoteProfiles::Profile &profile)
{
    auto *dialog = new RemoteMountDialog(profile, KSharedConfig::openConfig()->group(QStringLiteral("RemoteMounts")), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &RemoteMountDialog::mountRequested, this, [this, profile] {
        mountRemoteProfile(profile, true);
    });
    dialog->show();
}

void MainWindow::mountRemoteProfilesAtStartup()
{
    // Places entries are removed at quit, so recorded entries are left from a session that didn't quit cleanly.
    const QStringList places = remoteMountState().readEntry(remoteMountPlacesKey, QStringList());
    for (const QString &place : places) {
        removeRemoteMountPlace(m_placesModel, QUrl(place));
    }
    const KConfigGroup options = KSharedConfig::openConfig()->group(QStringLiteral("RemoteMounts"));
    const QList<RemoteProfiles::Profile> profiles = RemoteProfiles::load(KSharedConfig::openConfig()->group(QStringLiteral("RemoteProfiles")));
    const RemoteMounts::Defaults defaults = RemoteMounts::defaultsFromSettings();
    for (const RemoteProfiles::Profile &profile : profiles) {
        const RemoteMounts::Options profileOptions = RemoteMounts::loadOptions(options, profile, defaults);
        // Mounts kept after an earlier session quit are taken over, so they can be opened and unmounted here.
        if (profileOptions.atStartup || remoteMounts()->isMountedAt(profile, profileOptions)) {
            mountRemoteProfile(profile, false);
        }
    }
    // Starting in a workspace activates it.
    mountRemoteProfilesForWorkspace(m_currentWorkspaceId);
}

void MainWindow::mountRemoteProfilesForWorkspace(const QString &workspaceId)
{
    if (!Settings::remoteDriveMountOnWorkspace() || workspaceId.isEmpty()) {
        return;
    }
    QList<QUrl> locations;
    const QList<WorkspaceLocations::Location> saved = m_workspaceLocations->locations();
    for (const WorkspaceLocations::Location &location : saved) {
        if (!location.ambiguous && m_workspaceCatalog.locationAssignments.value(location.id) == workspaceId) {
            locations.append(location.url);
        }
    }
    if (locations.isEmpty()) {
        return;
    }
    const KConfigGroup options = KSharedConfig::openConfig()->group(QStringLiteral("RemoteMounts"));
    const QList<RemoteProfiles::Profile> profiles = RemoteProfiles::load(KSharedConfig::openConfig()->group(QStringLiteral("RemoteProfiles")));
    const RemoteMounts::Defaults defaults = RemoteMounts::defaultsFromSettings();
    for (const RemoteProfiles::Profile &profile : profiles) {
        if (remoteMounts()->isMounted(profile.id) || remoteMounts()->isStarting(profile.id)) {
            continue;
        }
        const RemoteMounts::Options profileOptions = RemoteMounts::loadOptions(options, profile, defaults);
        // KIO FUSE chooses its own folder, so only remote locations show that it is needed.
        const QString mountPoint = profileOptions.backend == RemoteMounts::Backend::KioFuse
            ? QString()
            : RemoteMounts::mountPoint(profile, profileOptions, remoteMounts()->mountFolder());
        if (std::any_of(locations.cbegin(), locations.cend(), [&profile, &mountPoint](const QUrl &url) {
                return RemoteMounts::covers(profile, url, mountPoint);
            })) {
            mountRemoteProfile(profile, false);
        }
    }
}

void MainWindow::verifyChecksumFile()
{
    const KFileItemList items = activePane()->view()->selectedItems();
    if (items.size() != 1 || !items.first().url().isLocalFile() || !items.first().isFile() || items.first().isLink()) {
        KMessageBox::error(this, i18nc("@info", "Select one local checksum file that is not a symbolic link."));
        return;
    }
    auto *dialog = new ChecksumDialog(ChecksumDialog::Mode::Verify, {}, items.first().url().toLocalFile(), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::splitFile()
{
    const KFileItemList items = activePane()->view()->selectedItems();
    if (items.size() != 1 || !items.first().url().isLocalFile() || !items.first().isFile() || items.first().isLink()) {
        KMessageBox::error(this, i18nc("@info", "Select one local file that is not a symbolic link."));
        return;
    }
    auto *dialog = new FileSplitDialog(FileSplitDialog::Mode::Split, {items.first().url().toLocalFile()}, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::combineFiles()
{
    const KFileItemList items = activePane()->view()->selectedItems();
    if (items.isEmpty() || !std::all_of(items.cbegin(), items.cend(), [](const KFileItem &item) {
            return item.url().isLocalFile() && item.isFile() && !item.isLink();
        })) {
        KMessageBox::error(this, i18nc("@info", "Select local files that are not symbolic links."));
        return;
    }
    QStringList paths;
    for (const KFileItem &item : items) {
        paths.append(item.url().toLocalFile());
    }
    auto *dialog = new FileSplitDialog(FileSplitDialog::Mode::Combine, paths, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::encryptSelection(FileEncryption::Operation operation)
{
    using FileEncryption::Kind;
    using FileEncryption::Operation;
    // Turned-off actions are hidden, but a shortcut or caller may still reach them.
    const bool aesCrypt = operation == Operation::AesEncrypt || operation == Operation::AesDecrypt;
    if (!(aesCrypt ? Settings::aesCryptActions() : Settings::gnuPGActions())) {
        return;
    }
    QStringList paths;
    for (const KFileItem &item : activePane()->view()->selectedItems()) {
        if (!item.url().isLocalFile() || item.isDir()) {
            continue;
        }
        const QString path = item.url().toLocalFile();
        const Kind kind = operation == Operation::Decrypt || operation == Operation::AesDecrypt ? FileEncryption::identifyFile(path) : Kind::None;
        QString signature;
        QString data;
        if ((operation == Operation::Decrypt && kind != Kind::Encrypted && kind != Kind::Signed)
            || (operation == Operation::AesDecrypt && kind != Kind::AesCrypt)
            || (operation == Operation::Verify && !FileEncryption::verificationFiles(path, &signature, &data))) {
            continue;
        }
        paths.append(path);
    }
    if (paths.isEmpty()) {
        switch (operation) {
        case Operation::Decrypt:
            showStatusMessage(i18nc("@info:status", "Select files encrypted or signed with OpenPGP, such as .gpg or .asc files."));
            break;
        case Operation::AesDecrypt:
            showStatusMessage(i18nc("@info:status", "Select AES Crypt files, which end in .aes."));
            break;
        case Operation::Verify:
            showStatusMessage(i18nc("@info:status", "Select a signed file, a .sig or .asc signature, or a file with its signature beside it."));
            break;
        default:
            showStatusMessage(i18nc("@info:status", "Select local files."));
            break;
        }
        return;
    }
    auto *dialog = new FileEncryptionDialog(operation, paths, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::openSelectionInKleopatra(bool decrypt)
{
    using FileEncryption::Kind;
    if (!Settings::gnuPGActions()) {
        return;
    }
    const QString kleopatra = FileEncryption::findExecutable(FileEncryption::Tool::Kleopatra);
    if (kleopatra.isEmpty()) {
        showStatusMessage(FileEncryption::missingText(FileEncryption::Tool::Kleopatra));
        return;
    }
    QStringList paths;
    for (const KFileItem &item : activePane()->view()->selectedItems()) {
        if (!item.url().isLocalFile() || item.isDir()) {
            continue;
        }
        QString path = item.url().toLocalFile();
        if (decrypt) {
            const Kind kind = FileEncryption::identifyFile(path);
            QString signature;
            QString data;
            if (kind != Kind::Encrypted && kind != Kind::Signed && kind != Kind::Signature) {
                // Kleopatra finds the signed data from a detached signature, not the other way round.
                if (!FileEncryption::verificationFiles(path, &signature, &data)) {
                    continue;
                }
                path = signature;
            }
        }
        if (!paths.contains(path)) {
            paths.append(path);
        }
    }
    if (paths.isEmpty()) {
        showStatusMessage(decrypt ? i18nc("@info:status", "Select files encrypted or signed with OpenPGP, or their signatures.")
                                  : i18nc("@info:status", "Select local files."));
        return;
    }
    auto *job = new KIO::CommandLauncherJob(kleopatra, FileEncryption::kleopatraArguments(decrypt, paths), this);
    job->setDesktopName(QStringLiteral("org.kde.kleopatra"));
    connect(job, &KJob::result, this, [this](KJob *job) {
        if (job->error()) {
            showStatusMessage(i18nc("@info:status", "Could not open Kleopatra: %1", job->errorString()));
        }
    });
    job->start();
}

void MainWindow::updateEncryptionActions()
{
    using FileEncryption::Kind;
    using FileEncryption::Tool;
    KActionCollection *actions = actionCollection();
    const KFileItemList items = activePane()->view()->selectedItems();
    const bool localFiles = !items.isEmpty() && std::all_of(items.cbegin(), items.cend(), [](const KFileItem &item) {
        return item.url().isLocalFile() && !item.isDir();
    });
    bool openPgp = false;
    bool aesFile = false;
    bool verifiable = false;
    if (localFiles) {
        for (const KFileItem &item : items) {
            const QString path = item.url().toLocalFile();
            const Kind kind = FileEncryption::identifyFile(path);
            openPgp = openPgp || kind == Kind::Encrypted || kind == Kind::Signed;
            aesFile = aesFile || kind == Kind::AesCrypt;
            QString signature;
            QString data;
            verifiable = verifiable || FileEncryption::verificationFiles(path, &signature, &data);
        }
    }
    const bool gnupg = !FileEncryption::findExecutable(Tool::GnuPG).isEmpty();
    const bool aesCrypt = !FileEncryption::findExecutable(Tool::AesCrypt).isEmpty();
    const bool kleopatra = !FileEncryption::findExecutable(Tool::Kleopatra).isEmpty();
    const auto set = [actions](const char *name, bool enabled, const QString &tooltip) {
        QAction *action = actions->action(QLatin1String(name));
        action->setEnabled(enabled);
        action->setToolTip(tooltip);
    };
    const QString noGnupg = gnupg ? QString() : FileEncryption::missingText(Tool::GnuPG);
    set("encrypt_files",
        localFiles && gnupg,
        gnupg ? i18nc("@info:tooltip", "Encrypt copies of the selected files for chosen keys or with a password") : noGnupg);
    set("decrypt_files", localFiles && gnupg && openPgp, gnupg ? i18nc("@info:tooltip", "Decrypt copies of the selected OpenPGP files") : noGnupg);
    set("sign_files", localFiles && gnupg, gnupg ? i18nc("@info:tooltip", "Save a separate signature for each selected file") : noGnupg);
    set("verify_signatures",
        localFiles && gnupg && verifiable,
        gnupg ? i18nc("@info:tooltip", "Check signed files, or files with a .sig or .asc signature beside them") : noGnupg);
    const QString noAesCrypt = aesCrypt ? QString() : FileEncryption::missingText(Tool::AesCrypt);
    set("aes_encrypt_files",
        localFiles && aesCrypt,
        aesCrypt ? i18nc("@info:tooltip", "Encrypt copies of the selected files with a password, as .aes files") : noAesCrypt);
    set("aes_decrypt_files", localFiles && aesCrypt && aesFile, aesCrypt ? i18nc("@info:tooltip", "Decrypt copies of the selected .aes files") : noAesCrypt);
    actions->action(QStringLiteral("kleopatra_encrypt_sign"))->setVisible(kleopatra && Settings::gnuPGActions());
    actions->action(QStringLiteral("kleopatra_encrypt_sign"))->setEnabled(localFiles);
    actions->action(QStringLiteral("kleopatra_decrypt_verify"))->setVisible(kleopatra && Settings::gnuPGActions());
    actions->action(QStringLiteral("kleopatra_decrypt_verify"))->setEnabled(localFiles && (openPgp || verifiable));
}

void MainWindow::applyEncryptionSettings()
{
    KActionCollection *actions = actionCollection();
    const bool gnupg = Settings::gnuPGActions();
    const bool aesCrypt = Settings::aesCryptActions();
    // Hiding an action also turns off its shortcut; Kleopatra's actions also need Kleopatra.
    for (const char *name : {"encrypt_files", "decrypt_files", "sign_files", "verify_signatures"}) {
        actions->action(QLatin1String(name))->setVisible(gnupg);
    }
    for (const char *name : {"kleopatra_encrypt_sign", "kleopatra_decrypt_verify"}) {
        actions->action(QLatin1String(name))->setVisible(gnupg && !FileEncryption::findExecutable(FileEncryption::Tool::Kleopatra).isEmpty());
    }
    for (const char *name : {"aes_encrypt_files", "aes_decrypt_files"}) {
        actions->action(QLatin1String(name))->setVisible(aesCrypt);
    }
    actions->action(QStringLiteral("encryption"))->setVisible(gnupg || aesCrypt);
}

void MainWindow::compareFoldersFiltered()
{
    const QUrl first = activePane()->url();
    const QUrl second = inactivePane()->url();
    if (!isDualLayout() || inactivePane()->isHidden() || !synchronizableFolder(first) || !synchronizableFolder(second)) {
        showStatusMessage(i18nc("@info", "Choose two visible local or SSH folders to compare."));
        return;
    }
    auto *dialog = new FolderSyncDialog(first, second, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::comparePaneFolders()
{
    Pane *first = activePane();
    Pane *second = inactivePane();
    const QUrl firstUrl = first->url();
    const QUrl secondUrl = second->url();
    if (!isDualLayout() || second->isHidden() || firstUrl.isEmpty() || !firstUrl.isValid() || secondUrl.isEmpty() || !secondUrl.isValid()) {
        KMessageBox::error(this, i18nc("@info", "Show two panes with valid folders to compare."));
        return;
    }
    auto *dialog = new FolderComparisonDialog(firstUrl, secondUrl, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    const QPointer<Pane> firstPane(first);
    const QPointer<Pane> secondPane(second);
    connect(dialog,
            &FolderComparisonDialog::selectDifferences,
            this,
            [this, firstPane, secondPane, firstUrl, secondUrl](const QList<QUrl> &left, const QList<QUrl> &right) {
                if (!firstPane || !secondPane || !isDualLayout() || firstPane->isHidden() || secondPane->isHidden()
                    || firstPane->url().adjusted(QUrl::StripTrailingSlash) != firstUrl.adjusted(QUrl::StripTrailingSlash)
                    || secondPane->url().adjusted(QUrl::StripTrailingSlash) != secondUrl.adjusted(QUrl::StripTrailingSlash)) {
                    showStatusMessage(i18nc("@info:status", "Pane folders changed or are hidden; compare them again before selecting."));
                    return;
                }
                firstPane->view()->selectUrls(left);
                secondPane->view()->selectUrls(right);
                if (firstPane->view()->selectedItems().size() != left.size() || secondPane->view()->selectedItems().size() != right.size()) {
                    showStatusMessage(i18nc("@info:status", "Some differences could not be selected. Check pane filters and hidden files."));
                } else {
                    showStatusMessage(i18nc("@info:status", "Differences selected in both panes."));
                }
            });
    dialog->show();
}

void MainWindow::swapCurrentTabs()
{
    const QScopedValueRollback guard(m_syncingLinkedPanes, true);
    if (m_left->tabs()->swapCurrent(*m_right->tabs())) {
        std::swap(m_linkedLeftRoot, m_linkedRightRoot);
    }
}

void MainWindow::swapPanesAccordingToPreference()
{
    if (Settings::paneSwapMode() == Settings::EnumPaneSwapMode::WholeSides) {
        swapPaneSides();
    } else {
        swapCurrentTabs();
    }
}

void MainWindow::swapPaneSides()
{
    if (m_secondPaneInTab || m_rightWindow) {
        return;
    }
    const QList<int> sizes = m_splitter->sizes();
    m_splitter->insertWidget(0, m_right);
    std::swap(m_left, m_right);
    std::swap(m_linkedLeftRoot, m_linkedRightRoot);
    updateTabBarPlacement();
    m_splitter->setSizes(sizes);
    updateActions();
}

void MainWindow::moveSecondPaneToTab()
{
    if (m_secondPaneInTab) {
        return;
    }
    if (m_rightWindow) {
        moveSecondPaneToDual();
    }
    const int index = m_splitter->indexOf(m_right);
    if (index < 0) {
        return;
    }
    if (!m_rightPlaceholder) {
        m_rightPlaceholder = new QWidget;
        m_rightPlaceholder->setObjectName(QStringLiteral("secondPanePlaceholder"));
        m_rightPlaceholder->setMinimumSize(0, 0);
    }

    resetMovedPaneDisplayStyle();
    m_movingSecondPane = true;
    m_splitter->replaceWidget(index, m_rightPlaceholder);
    m_rightPlaceholder->hide();
    m_right->setParent(nullptr);
    m_paneTabs->addTab(m_right, i18nc("@title:tab", "Second Pane"));
    m_secondPaneInTab = true;
    m_paneTabs->tabBar()->show();
    m_paneTabs->setCurrentWidget(m_right);
    m_dualLayoutAction->setChecked(false);
    m_movingSecondPane = false;
    setActivePane(m_right);
    updateActions();
}

void MainWindow::popOutSecondPane()
{
    if (m_rightWindow) {
        return;
    }
    if (m_secondPaneInTab) {
        moveSecondPaneToDual();
    }
    const int index = m_splitter->indexOf(m_right);
    if (index < 0) {
        return;
    }
    resetMovedPaneDisplayStyle();
    if (!m_rightPlaceholder) {
        m_rightPlaceholder = new QWidget;
        m_rightPlaceholder->setObjectName(QStringLiteral("secondPanePlaceholder"));
        m_rightPlaceholder->setMinimumSize(0, 0);
    }

    m_movingSecondPane = true;
    m_splitter->replaceWidget(index, m_rightPlaceholder);
    m_rightPlaceholder->hide();
    m_right->setParent(nullptr);
    auto *window = new QMainWindow(this);
    window->setObjectName(QStringLiteral("secondPaneWindow"));
    window->setWindowFlag(Qt::Window, true);
    window->setWindowTitle(i18nc("@title:window", "Second Pane"));
    window->installEventFilter(this);
    window->setCentralWidget(m_right);
    m_rightWindow = window;
    m_dualLayoutAction->setChecked(false);
    m_movingSecondPane = false;
    setActivePane(m_right);
    window->resize(900, 600);
    window->show();
    updateActions();
}

void MainWindow::moveSecondPaneToPreferredDestination()
{
    if (Settings::paneMoveDestination() == Settings::EnumPaneMoveDestination::Window) {
        popOutSecondPane();
    } else {
        moveSecondPaneToTab();
    }
}

void MainWindow::resetMovedPaneDisplayStyle()
{
    if (Settings::paneMoveDisplayStyle() != Settings::EnumPaneMoveDisplayStyle::ResetToDefault) {
        return;
    }
    m_right->view()->setViewMode(static_cast<FolderView::ViewMode>(qBound(0, int(EffectiveAppearance::defaultViewMode()), int(FolderView::ViewMode::Cards))));
}

void MainWindow::moveSecondPaneToDual()
{
    if (!m_secondPaneInTab && !m_rightWindow) {
        return;
    }
    m_movingSecondPane = true;
    if (m_secondPaneInTab) {
        const int index = m_paneTabs->indexOf(m_right);
        if (index >= 0) {
            m_paneTabs->removeTab(index);
        }
        m_right->setParent(nullptr);
        m_secondPaneInTab = false;
        m_paneTabs->tabBar()->hide();
    } else if (m_rightWindow) {
        QMainWindow *window = m_rightWindow;
        window->removeEventFilter(this);
        window->takeCentralWidget();
        window->hide();
        window->deleteLater();
        m_rightWindow.clear();
        m_right->setParent(nullptr);
    }

    const int placeholderIndex = m_splitter->indexOf(m_rightPlaceholder);
    if (placeholderIndex >= 0) {
        m_splitter->replaceWidget(placeholderIndex, m_right);
        m_rightPlaceholder->deleteLater();
        m_rightPlaceholder = nullptr;
    } else {
        m_splitter->addWidget(m_right);
    }
    m_dualLayoutAction->setChecked(true);
    m_movingSecondPane = false;
    setActivePane(m_right);
    updateActions();
}

void MainWindow::syncLinkedPane(Pane *pane, const QUrl &url)
{
    if (!m_linkedPanesAction || !m_linkedPanesAction->isChecked() || m_syncingLinkedPanes) {
        return;
    }
    const bool left = pane == m_left;
    const auto result = LinkedNavigation::target(left ? m_linkedLeftRoot : m_linkedRightRoot, left ? m_linkedRightRoot : m_linkedLeftRoot, url);
    if (!result.error.isEmpty()) {
        m_linkedPanesAction->setChecked(false);
        showStatusMessage(i18nc("@info:status", "Pane navigation unlinked: %1", result.error));
        return;
    }
    const QScopedValueRollback guard(m_syncingLinkedPanes, true);
    (left ? m_right : m_left)->setUrl(result.target);
}

void MainWindow::copyToOtherPane()
{
    FileOperations::copy(urlsOf(activePane()->view()->selectedItems()), inactivePane()->url(), this);
}

void MainWindow::moveToOtherPane()
{
    FileOperations::move(urlsOf(activePane()->view()->selectedItems()), inactivePane()->url(), this);
}

void MainWindow::cut()
{
    FileOperations::copyToClipboard(urlsOf(activePane()->view()->selectedItems()), true);
}

void MainWindow::copy()
{
    FileOperations::copyToClipboard(urlsOf(activePane()->view()->selectedItems()), false);
}

void MainWindow::paste()
{
    const KFileItemList selected = activePane()->view()->selectedItems();
    const QUrl destination = selected.size() == 1 && selected.first().isDir() ? selected.first().targetUrl() : activePane()->url();
    FileOperations::paste(destination, this);
}

void MainWindow::rename()
{
    const KFileItemList items = activePane()->view()->selectedItems();
    if (items.size() > 1 && std::all_of(items.cbegin(), items.cend(), [](const KFileItem &item) {
            return item.url().isLocalFile();
        })) {
        batchRename();
    } else if (items.size() == 1 && activePane()->view()->renameInline()) {
        return;
    } else {
        FileOperations::rename(items, this);
    }
}

void MainWindow::batchRename()
{
    const KFileItemList items = activePane()->view()->selectedItems();
    QStringList paths;
    for (const KFileItem &item : items) {
        if (!item.url().isLocalFile()) {
            showStatusMessage(i18nc("@info", "Batch rename supports local items only."));
            return;
        }
        paths.append(item.url().toLocalFile());
    }
    if (paths.isEmpty()) {
        showStatusMessage(i18nc("@info", "Select at least one item to rename."));
        return;
    }
    auto *dialog = new BatchRenameDialog(paths, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::editMetadata()
{
    const KFileItemList items = activePane()->view()->selectedItems();
    if (items.size() != 1 || !items.first().url().isLocalFile() || !items.first().isFile() || items.first().isLink()) {
        showStatusMessage(i18nc("@info", "Select one local regular file to edit metadata."));
        return;
    }
    openMetadataEditor(items.first().url().toLocalFile());
}

void MainWindow::openMetadataEditor(const QString &path)
{
    auto *dialog = new MetadataEditDialog(path, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    auto *dock = new MetadataDock(dialog, this);
    dock->setObjectName(QStringLiteral("metadataEditorDock%1").arg(++m_metadataDockSerial));
    dock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setFeatures(QDockWidget::DockWidgetClosable);
    setDockTitleVisible(dock, Settings::embeddedPanelTitlesVisible());
    m_metadataDocks.append(dock);
    connect(dialog, &QDialog::finished, dock, &QObject::deleteLater);
    connect(dialog, &QDialog::finished, this, [this, dock] {
        m_metadataDocks.removeAll(dock);
    });
    connect(dock, &QDockWidget::visibilityChanged, this, [this, dock](bool visible) {
        if (!visible && dock->isHidden() && dock->property("opusPanelShown").toBool()) {
            savePanelDockSize(dock, QStringLiteral("MetadataDock"));
        }
    });
    connect(dialog, &MetadataEditDialog::placementRequested, this, [this, dialog, dock](MetadataEditDialog::Placement placement) {
        Settings::setMetadataPlacement(static_cast<Settings::EnumMetadataPlacement::type>(placement));
        Settings::self()->save();
        setMetadataPlacement(dialog, dock, placement);
    });
    setMetadataPlacement(dialog, dock, static_cast<MetadataEditDialog::Placement>(Settings::metadataPlacement()));
}

void MainWindow::setMetadataPlacement(MetadataEditDialog *dialog, QDockWidget *dock, MetadataEditDialog::Placement placement)
{
    if (!dock->isHidden()) {
        savePanelDockSize(dock, QStringLiteral("MetadataDock"));
    }
    dialog->hide();
    dock->hide();
    if (placement == MetadataEditDialog::Placement::Window) {
        dialog->setParent(this, Qt::Dialog);
        dialog->show();
    } else {
        addDockWidget(placement == MetadataEditDialog::Placement::Below ? Qt::BottomDockWidgetArea : Qt::RightDockWidgetArea, dock);
        dialog->setParent(dock, Qt::Widget);
        dock->setWidget(dialog);
        dock->show();
        dock->setProperty("opusPanelShown", true);
        dialog->show();
        restorePanelDockSize(dock, QStringLiteral("MetadataDock"));
    }
    dialog->setPlacement(placement);
}

void MainWindow::setPreviewPlacement(PreviewPanel::Placement placement)
{
    if (placement == m_previewPlacement && (placement == PreviewPanel::Placement::Pane || m_previewDock->widget() == m_previewPanel)) {
        m_previewPanel->setPlacement(placement);
        return;
    }
    const QScopedValueRollback<bool> moving(m_movingPreview, true);
    if (!m_previewDock->isHidden()) {
        savePanelDockSize(m_previewDock, QStringLiteral("PreviewDock"));
    }
    m_previewDock->hide();
    m_previewPanel->setParent(nullptr);
    m_previewPlacement = placement;
    if (placement == PreviewPanel::Placement::Pane) {
        m_splitter->insertWidget(2, m_previewPanel);
    } else {
        addDockWidget(placement == PreviewPanel::Placement::Below ? Qt::BottomDockWidgetArea : Qt::RightDockWidgetArea, m_previewDock);
        m_previewDock->setWidget(m_previewPanel);
    }
    m_previewPanel->setPlacement(placement);
    toggleLayout(m_dualLayoutAction->isChecked());
    restorePanelDockSize(m_previewDock, QStringLiteral("PreviewDock"));
}

void MainWindow::showTransferQueue()
{
    auto *dialog = new TransferQueueDialog(FileOperations::transferQueue(), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::showCustomCommand()
{
    showCustomCommandPreset({});
}

void MainWindow::showCustomCommandPreset(const QString &name)
{
    const QUrl folder = activePane()->url();
    if (!folder.isLocalFile()) {
        showStatusMessage(i18nc("@info", "Commands require a local working folder."));
        return;
    }
    QStringList paths;
    for (const KFileItem &item : activePane()->view()->selectedItems()) {
        if (!item.url().isLocalFile()) {
            showStatusMessage(i18nc("@info", "Commands require local selected items."));
            return;
        }
        paths.append(item.url().toLocalFile());
    }
    paths.sort();
    auto *dialog = new CustomCommandDialog(folder.toLocalFile(), paths, this);
    connect(dialog, &CustomCommandDialog::presetsChanged, this, &MainWindow::refreshCustomCommandActions);
    if (!name.isEmpty()) {
        dialog->loadPreset(name);
    }
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::refreshCustomCommandActions()
{
    QString error;
    const auto commands = CustomCommands::load(KSharedConfig::openConfig()->group(QStringLiteral("CustomCommands")), &error);
    if (!error.isEmpty()) {
        showStatusMessage(error);
        return;
    }
    QSet<QString> current;
    for (const auto &command : commands) {
        current.insert(command.name);
        QAction *action = m_customCommandActions.value(command.name);
        if (!action) {
            const QString id = QStringLiteral("custom_command_preset_")
                + QString::fromLatin1(QCryptographicHash::hash(command.name.toUtf8(), QCryptographicHash::Sha256).toHex());
            action = actionCollection()->addAction(id);
            action->setIcon(SymbolicIcon::fromTheme(QStringLiteral("system-run")));
            connect(action, &QAction::triggered, this, [this, name = command.name] {
                showCustomCommandPreset(name);
            });
            m_customCommandActions.insert(command.name, action);
            if (factory()) {
                // Saved after the window applied the stored shortcuts.
                WindowShortcuts::apply(action);
            }
        }
        // Configure Keyboard Shortcuts lists the text; toolbars show the name.
        action->setText(i18nc("@action %1 is a saved command's name", "Saved Command: %1", command.name));
        action->setIconText(command.name);
        action->setToolTip(i18nc("@info:tooltip", "Preview saved command: %1", command.name));
    }
    for (auto it = m_customCommandActions.begin(); it != m_customCommandActions.end();) {
        if (!current.contains(it.key())) {
            QAction *action = it.value();
            it = m_customCommandActions.erase(it);
            // A command saved again under this name starts without the deleted one's shortcuts.
            WindowShortcuts::forget(action->objectName());
            actionCollection()->removeAction(action);
        } else {
            ++it;
        }
    }
    if (factory()) {
        // A saved command's stored shortcut may be one of the extra navigation keys.
        WindowShortcuts::applyNavigationKeys(this);
    }
    if (m_active) {
        updateActions();
    }
}

void MainWindow::trash()
{
    FileOperations::trash(urlsOf(activePane()->view()->selectedItems()), this);
}

void MainWindow::deletePermanently()
{
    FileOperations::deletePermanently(urlsOf(activePane()->view()->selectedItems()), this);
}

void MainWindow::showProperties()
{
    FileOperations::showProperties(activePane()->view()->selectedItemsOrRoot(), this);
}

void MainWindow::openTerminal()
{
    const TerminalProviders::Provider provider = TerminalProviders::separateProvider();
    if (provider == TerminalProviders::Provider::SystemDefault) {
        FileOperations::openTerminal(activePane()->url(), this);
        return;
    }
    TerminalLauncher::open(provider, activePane()->url(), this, [window = QPointer<MainWindow>(this)](const QString &message) {
        if (window) {
            window->showStatusMessage(message);
        }
    });
}

void MainWindow::newFolder()
{
    m_newFileMenu->setWorkingDirectory(activePane()->url());
    m_newFileMenu->createDirectory();
}

void MainWindow::showNewMenu()
{
    m_newFileMenu->setWorkingDirectory(activePane()->url());
    m_newFileMenu->checkUpToDate();
    m_newFileMenu->menu()->popup(QCursor::pos());
}

void MainWindow::runYaziFileCommand(Pane *pane, YaziMode::Command command)
{
    if (!pane || !pane->yaziMode()) {
        return;
    }
    using YaziMode::Command;
    const QString name = YaziMode::commandAction(command);
    if (YaziMode::permanentDeletionActions().contains(name)) {
        pane->reportYaziMessage(i18nc("@info:status", "Yazi mode never deletes files permanently; move them to the trash instead."));
        return;
    }
    setActivePane(pane);
    updateActions();
    QAction *action = name.isEmpty() ? nullptr : actionCollection()->action(name);
    const KFileItemList selected = pane->view()->selectedItems();
    const KFileItem root = pane->view()->rootItem();
    const bool readOnly = root.isNull() || !root.isWritable();
    const auto unavailable = [pane, command, &selected](const QString &text) {
        QString title = KLocalizedString::removeAcceleratorMarker(text);
        title.remove(QStringLiteral("…"));
        title.remove(QStringLiteral("..."));
        const KFileItemListProperties props(selected);
        if (selected.isEmpty() && !title.isEmpty()) {
            pane->reportYaziMessage(i18nc("@info:status %1 is an action such as Move to Trash", "%1 needs a selected or focused item.", title));
        } else if (command == Command::Trash && props.supportsDeleting() && !props.isLocal()) {
            pane->reportYaziMessage(i18nc("@info:status", "Only local items can be moved to the trash."));
        } else if ((command == Command::Trash && !props.supportsDeleting())
                   || ((command == Command::Rename || command == Command::Cut || command == Command::MoveToOtherPane) && !props.supportsMoving())) {
            pane->reportYaziMessage(
                i18nc("@info:status %1 is an action such as Move to Trash", "%1 is not available: the items are read-only or in a virtual location.", title));
        } else {
            pane->reportYaziMessage(i18nc("@info:status %1 is an action such as Move to Trash", "%1 is not available here.", title));
        }
    };
    const QString folderReadOnly = i18nc("@info:status", "This folder is read-only or virtual, so nothing can be added to it.");
    switch (command) {
    case Command::Paste: {
        // Ctrl+V pastes into a single selected folder; as in Yazi, p pastes
        // into the current folder, since the focused item is selected as the
        // focus moves. KIO's paste asks about conflicts and records undo.
        bool clipboardHasItems = false;
        FileOperations::pasteActionText(root, &clipboardHasItems);
        if (!clipboardHasItems) {
            pane->reportYaziMessage(i18nc("@info:status", "The clipboard has nothing to paste. Copy or cut items first."));
        } else if (readOnly && !pane->view()->isFlat()) {
            pane->reportYaziMessage(folderReadOnly);
        } else if (!m_pasteAction->isEnabled() || !FileOperations::paste(pane->url(), this)) {
            pane->reportYaziMessage(i18nc("@info:status %1 is an action such as Move to Trash", "%1 is not available here.", i18nc("@action", "Paste")));
        }
        return;
    }
    case Command::Create:
        if (action && action->isEnabled() && readOnly) {
            pane->reportYaziMessage(folderReadOnly);
            return;
        }
        if (!action || !action->isEnabled()) {
            pane->reportYaziMessage(i18nc("@info:status %1 is an action such as Move to Trash", "%1 is not available here.", i18nc("@action", "Create New")));
            return;
        }
        m_newFileMenu->setWorkingDirectory(pane->url());
        m_newFileMenu->checkUpToDate();
        m_newFileMenu->menu()->popup(yaziMenuPosition(pane));
        return;
    case Command::OpenWith: {
        auto *menu = new QMenu(this);
        menu->setObjectName(QStringLiteral("yaziOpenWithMenu"));
        menu->setAttribute(Qt::WA_DeleteOnClose);
        if (!selected.isEmpty()) {
            m_fileItemActions->setItemListProperties(KFileItemListProperties(selected));
            m_fileItemActions->insertOpenWithActionsTo(nullptr, menu, QStringList());
        }
        if (menu->isEmpty()) {
            delete menu;
            unavailable(i18nc("@action", "Open With"));
            return;
        }
        menu->popup(yaziMenuPosition(pane));
        return;
    }
    case Command::CopyToOtherPane:
    case Command::MoveToOtherPane: {
        const QString title = KLocalizedString::removeAcceleratorMarker(action ? action->text() : QString());
        if (!isDualLayout() && !m_secondPaneInTab && !m_rightWindow) {
            pane->reportYaziMessage(
                i18nc("@info:status %1 is an action such as Copy to Other Pane", "%1 needs a second pane. Turn on the dual-pane layout first.", title));
            return;
        }
        const Pane *other = pane == m_left ? m_right : m_left;
        if (other->url().adjusted(QUrl::StripTrailingSlash) == pane->url().adjusted(QUrl::StripTrailingSlash)) {
            pane->reportYaziMessage(
                i18nc("@info:status %1 is an action such as Copy to Other Pane", "%1 is not available: the other pane shows the same folder.", title));
            return;
        }
        break;
    }
    case Command::Undo:
        if (!action || !action->isEnabled()) {
            pane->reportYaziMessage(i18nc("@info:status", "There is nothing to undo."));
            return;
        }
        break;
    default:
        break;
    }
    if (!action) {
        pane->reportYaziMessage(i18nc("@info:status", "This command is not available here."));
        return;
    }
    if (!action->isEnabled()) {
        unavailable(action->text());
        return;
    }
    action->trigger();
}

QPoint MainWindow::yaziMenuPosition(Pane *pane) const
{
    QAbstractItemView *items = pane->view()->currentView();
    const QRect rect = items->visualRect(items->currentIndex());
    const bool shown = rect.isValid() && items->viewport()->rect().intersects(rect);
    return items->viewport()->mapToGlobal(shown ? rect.bottomLeft() : QPoint(0, 0));
}

void MainWindow::toggleLayout(bool dual)
{
    const bool previewWasVisible = !m_previewPanel->isHidden();
    const bool leftWasVisible = !m_left->isHidden();
    const bool rightWasVisible = !m_secondPaneInTab && !m_rightWindow && !m_right->isHidden();
    const bool preview = m_previewPanelAction && m_previewPanelAction->isChecked();
    const bool editorEmbedded = m_editor
        && ((m_editorPlacement == EditorPanel::Placement::Panel && !m_editor->isHidden())
            || ((m_editorPlacement == EditorPanel::Placement::Below || m_editorPlacement == EditorPanel::Placement::Right) && !m_editorDock->isHidden()));
    const bool terminalShown = (m_terminalPanelAction && m_terminalPanelAction->isChecked()) || (m_searchPanelAction && m_searchPanelAction->isChecked());
    const bool previewInPane = preview && m_previewPlacement == PreviewPanel::Placement::Pane;
    m_left->setVisible((dual && (!previewInPane || editorEmbedded || terminalShown)) || activePane() == m_left);
    if (!m_secondPaneInTab && !m_rightWindow) {
        m_right->setVisible((dual && (!previewInPane || editorEmbedded || terminalShown)) || activePane() == m_right);
    }
    if (m_previewPlacement != PreviewPanel::Placement::Pane) {
        m_previewDock->setVisible(preview);
    }
    m_previewPanel->setVisible(preview);
    const bool rightIsVisible = !m_secondPaneInTab && !m_rightWindow && !m_right->isHidden();
    if (previewWasVisible != preview || leftWasVisible != !m_left->isHidden() || rightWasVisible != rightIsVisible) {
        restoreSplitterSizes(true, false);
    }
    if (m_linkedPanesAction && m_linkedPanesAction->isChecked() && (!dual || m_left->isHidden() || m_right->isHidden())) {
        m_linkedPanesAction->setChecked(false);
        showStatusMessage(i18nc("@info:status", "Pane navigation unlinked because both panes are not visible."));
    }
    updateActions();
}

void MainWindow::saveSplitterSizes()
{
    const auto state = KSharedConfig::openStateConfig();
    KConfigGroup group = state->group(QLatin1String(SplitterSizesGroup)).group(QLatin1String(isStackedLayout() ? "Stacked" : "SideBySide"));
    const auto save = [&group](QSplitter *splitter, const QList<QPair<QWidget *, QString>> &widgets) {
        const QList<int> sizes = splitter->sizes();
        int total = 0;
        int previousVisible = 0;
        bool savedHidden = false;
        for (int i = 0; i < splitter->count(); ++i) {
            if (!splitter->widget(i)->isHidden()) {
                total += sizes.at(i);
            }
        }
        for (const auto &[widget, key] : widgets) {
            if (!group.hasKey(key)) {
                continue;
            }
            const int index = splitter->indexOf(widget);
            if (index >= 0 && !widget->isHidden() && sizes.at(index) > 0) {
                previousVisible += qBound(1, group.readEntry(key, 2500), 10000);
            } else {
                savedHidden = true;
            }
        }
        if (total <= 0) {
            return;
        }
        const int budget = savedHidden && previousVisible > 0 ? previousVisible : 10000;
        for (const auto &[widget, key] : widgets) {
            const int index = splitter->indexOf(widget);
            if (index >= 0 && !widget->isHidden() && sizes.at(index) > 0) {
                group.writeEntry(key, qMax(1, int(qint64(sizes.at(index)) * budget / total)));
            }
        }
    };
    save(m_splitter,
         {{m_left, QStringLiteral("FirstPane")},
          {m_right, QStringLiteral("SecondPane")},
          {m_previewPanel, QStringLiteral("Preview")},
          {m_editor, QStringLiteral("Editor")}});
    save(m_workspaceSplitter,
         {{m_splitter, QStringLiteral("Workspace")}, {m_terminalPanel, QStringLiteral("Terminal")}, {m_searchPanel, QStringLiteral("Search")}});
    save(m_searchSplitter, {{m_workspaceSplitter, QStringLiteral("SearchWorkspace")}, {m_searchPanel, QStringLiteral("SeparateSearch")}});
    if (m_editorDock && !m_editorDock->isHidden() && !m_editorDock->isFloating()) {
        const bool right = dockWidgetArea(m_editorDock) == Qt::RightDockWidgetArea;
        const int dimension = right ? width() : height();
        if (dimension > 0) {
            group.writeEntry(right ? "EditorDockRight" : "EditorDockBelow",
                             qBound(1, int(qint64(right ? m_editorDock->width() : m_editorDock->height()) * 10000 / dimension), 6000));
        }
    }
    if (!m_previewDock->isHidden()) {
        savePanelDockSize(m_previewDock, QStringLiteral("PreviewDock"));
    }
    if (!m_tagDock->isHidden()) {
        savePanelDockSize(m_tagDock, QStringLiteral("TagDock"));
    }
    for (const auto &dock : std::as_const(m_metadataDocks)) {
        if (dock && !dock->isHidden()) {
            savePanelDockSize(dock, QStringLiteral("MetadataDock"));
        }
    }
    state->sync();
}

void MainWindow::restoreSplitterSizes(bool panes, bool workspace, bool defaultsIfMissing)
{
    const KConfigGroup group =
        KSharedConfig::openStateConfig()->group(QLatin1String(SplitterSizesGroup)).group(QLatin1String(isStackedLayout() ? "Stacked" : "SideBySide"));
    const auto restore = [this, &group, defaultsIfMissing](QSplitter *splitter, const QList<QPair<QWidget *, QString>> &widgets, const QList<int> &defaults) {
        QList<int> sizes(splitter->count(), 2500);
        bool saved = false;
        for (const auto &[widget, key] : widgets) {
            const int index = splitter->indexOf(widget);
            if (index >= 0 && group.hasKey(key)) {
                sizes[index] = qBound(1, group.readEntry(key, 2500), 10000);
                saved = true;
            }
        }
        if (saved) {
            const auto preservePrimaryArea = [&sizes, splitter](QWidget *primary, QWidget *secondary, int minimumTotalShare) {
                const int primaryIndex = splitter->indexOf(primary);
                const int secondaryIndex = splitter->indexOf(secondary);
                if (primaryIndex < 0 || secondaryIndex < 0 || primary->isHidden() || secondary->isHidden()) {
                    return;
                }
                int total = 0;
                int otherTotal = 0;
                for (int i = 0; i < splitter->count(); ++i) {
                    if (splitter->widget(i)->isHidden()) {
                        continue;
                    }
                    total += sizes.at(i);
                    if (i != primaryIndex && i != secondaryIndex) {
                        otherTotal += sizes.at(i);
                    }
                }
                if (total <= 0) {
                    return;
                }
                const int primaryBudget = qMax(total * minimumTotalShare / 10000, total - otherTotal);
                const int otherBudget = total - primaryBudget;
                const int otherRemaining = otherBudget;
                if (otherTotal > 0) {
                    for (int i = 0; i < splitter->count(); ++i) {
                        if (!splitter->widget(i)->isHidden() && i != primaryIndex && i != secondaryIndex) {
                            sizes[i] = qMax(1, int(qint64(sizes.at(i)) * otherRemaining / otherTotal));
                        }
                    }
                }
                const int primaryTotal = sizes.at(primaryIndex) + sizes.at(secondaryIndex);
                const int firstShare = primaryTotal > 0 ? int(qint64(primaryBudget) * sizes.at(primaryIndex) / primaryTotal) : primaryBudget / 2;
                const int minimumPane = primaryBudget / 4;
                sizes[primaryIndex] = qBound(minimumPane, firstShare, primaryBudget - minimumPane);
                sizes[secondaryIndex] = primaryBudget - sizes.at(primaryIndex);
            };
            if (splitter == m_splitter) {
                preservePrimaryArea(m_left, m_right, 6000);
            } else if (splitter == m_workspaceSplitter || splitter == m_searchSplitter) {
                // Embedded panels share at most 40% so the panes keep the primary area.
                const int workspaceIndex = splitter->indexOf(splitter == m_workspaceSplitter ? static_cast<QWidget *>(m_splitter) : m_workspaceSplitter);
                int panels = 0;
                for (int i = 0; i < splitter->count(); ++i) {
                    if (i != workspaceIndex && !splitter->widget(i)->isHidden()) {
                        panels += sizes.at(i);
                    }
                }
                if (workspaceIndex >= 0 && panels > 0) {
                    const int total = sizes.at(workspaceIndex) + panels;
                    const int primary = qMax(total * 6000 / 10000, sizes.at(workspaceIndex));
                    sizes[workspaceIndex] = primary;
                    for (int i = 0; i < splitter->count(); ++i) {
                        if (i != workspaceIndex && !splitter->widget(i)->isHidden()) {
                            sizes[i] = qMax(1, int(qint64(sizes.at(i)) * (total - primary) / panels));
                        }
                    }
                }
            }
            splitter->setSizes(sizes);
        } else if (defaultsIfMissing) {
            // Scale the default weights to the splitter so minimum sizes do not dominate them.
            const int extent = splitter->orientation() == Qt::Horizontal ? splitter->width() : splitter->height();
            int weight = 0;
            for (int i = 0; i < splitter->count(); ++i) {
                if (!splitter->widget(i)->isHidden()) {
                    weight += defaults.value(i);
                }
            }
            QList<int> scaled;
            for (int i = 0; i < splitter->count(); ++i) {
                scaled.append(weight > 0 && extent > 0 ? qMax(1, int(qint64(defaults.value(i)) * extent / weight)) : defaults.value(i));
            }
            splitter->setSizes(scaled);
        }
    };
    if (panes) {
        restore(m_splitter,
                {{m_left, QStringLiteral("FirstPane")},
                 {m_right, QStringLiteral("SecondPane")},
                 {m_previewPanel, QStringLiteral("Preview")},
                 {m_editor, QStringLiteral("Editor")}},
                QList<int>(m_splitter->count(), 1));
    }
    if (workspace) {
        restore(m_workspaceSplitter,
                {{m_splitter, QStringLiteral("Workspace")}, {m_terminalPanel, QStringLiteral("Terminal")}, {m_searchPanel, QStringLiteral("Search")}},
                {3, 1, 1});
        restore(m_searchSplitter, {{m_workspaceSplitter, QStringLiteral("SearchWorkspace")}, {m_searchPanel, QStringLiteral("SeparateSearch")}}, {3, 1});
    }
}

void MainWindow::restoreEditorDockSize()
{
    if (!m_editorDock || m_editorDock->isHidden() || m_editorDock->isFloating()) {
        return;
    }
    const KConfigGroup group =
        KSharedConfig::openStateConfig()->group(QLatin1String(SplitterSizesGroup)).group(QLatin1String(isStackedLayout() ? "Stacked" : "SideBySide"));
    const bool right = dockWidgetArea(m_editorDock) == Qt::RightDockWidgetArea;
    const char *key = right ? "EditorDockRight" : "EditorDockBelow";
    if (group.hasKey(key)) {
        const int fraction = qBound(1, group.readEntry(key, 2500), 6000);
        const int dimension = right ? width() : height();
        resizeDocks({m_editorDock}, {qMax(1, int(qint64(dimension) * fraction / 10000))}, right ? Qt::Horizontal : Qt::Vertical);
    }
}

void MainWindow::savePanelDockSize(QDockWidget *dock, const QString &name)
{
    if (!dock || dock->isFloating()) {
        return;
    }
    const bool right = dockWidgetArea(dock) == Qt::RightDockWidgetArea;
    const int dimension = right ? width() : height();
    if (dimension <= 0) {
        return;
    }
    const auto state = KSharedConfig::openStateConfig();
    KConfigGroup group = state->group(QLatin1String(SplitterSizesGroup)).group(QLatin1String(isStackedLayout() ? "Stacked" : "SideBySide"));
    group.writeEntry(name + (right ? QStringLiteral("Right") : QStringLiteral("Below")),
                     qBound(1, int(qint64(right ? dock->width() : dock->height()) * 10000 / dimension), 6000));
    state->sync();
}

void MainWindow::restorePanelDockSize(QDockWidget *dock, const QString &name)
{
    if (!dock || dock->isHidden() || dock->isFloating()) {
        return;
    }
    const KConfigGroup group =
        KSharedConfig::openStateConfig()->group(QLatin1String(SplitterSizesGroup)).group(QLatin1String(isStackedLayout() ? "Stacked" : "SideBySide"));
    const bool right = dockWidgetArea(dock) == Qt::RightDockWidgetArea;
    const QString key = name + (right ? QStringLiteral("Right") : QStringLiteral("Below"));
    if (group.hasKey(key)) {
        const int dimension = right ? width() : height();
        const int fraction = qBound(1, group.readEntry(key, 2500), 6000);
        resizeDocks({dock}, {qMax(1, int(qint64(dimension) * fraction / 10000))}, right ? Qt::Horizontal : Qt::Vertical);
    }
}

void MainWindow::movePaneDivider(int direction)
{
    QList<int> sizes = m_splitter->sizes();
    const int first = m_splitter->indexOf(m_left);
    const int second = m_splitter->indexOf(m_right);
    if (first < 0 || second != first + 1 || !m_right->isVisible()) {
        return;
    }
    const int total = sizes.at(first) + sizes.at(second);
    const bool horizontal = m_splitter->orientation() == Qt::Horizontal;
    const int minimum = std::max(horizontal ? m_left->minimumSizeHint().width() : m_left->minimumSizeHint().height(), 1);
    const int maximum = total - std::max(horizontal ? m_right->minimumSizeHint().width() : m_right->minimumSizeHint().height(), 1);
    if (maximum < minimum) {
        return;
    }
    // Move by a twentieth of the panes, as the arrow keys do for sliders.
    int step = std::max(total / 20, 1) * direction;
    if (horizontal && layoutDirection() == Qt::RightToLeft) {
        step = -step;
    }
    sizes[first] = std::clamp(sizes.at(first) + step, minimum, maximum);
    sizes[second] = total - sizes.at(first);
    m_splitter->setSizes(sizes);
}

void MainWindow::resetSplitterSizes()
{
    const auto state = KSharedConfig::openStateConfig();
    KConfigGroup group = state->group(QLatin1String(SplitterSizesGroup)).group(QLatin1String(isStackedLayout() ? "Stacked" : "SideBySide"));
    group.deleteGroup();
    group.writeEntry("EditorDockRight", 2500);
    group.writeEntry("EditorDockBelow", 2500);
    group.writeEntry("PreviewDockRight", 2500);
    group.writeEntry("PreviewDockBelow", 2500);
    group.writeEntry("MetadataDockRight", 2500);
    group.writeEntry("MetadataDockBelow", 2500);
    group.writeEntry("TagDockRight", 2000);
    group.writeEntry("TagDockBelow", 2000);
    state->sync();
    m_splitter->setSizes(QList<int>(m_splitter->count(), 1));
    m_workspaceSplitter->setSizes({3, 1, 1});
    m_searchSplitter->setSizes({3, 1});
    if (m_editorDock && !m_editorDock->isHidden() && !m_editorDock->isFloating()) {
        const bool right = dockWidgetArea(m_editorDock) == Qt::RightDockWidgetArea;
        resizeDocks({m_editorDock}, {qMax(1, (right ? width() : height()) / 4)}, right ? Qt::Horizontal : Qt::Vertical);
    }
    restorePanelDockSize(m_previewDock, QStringLiteral("PreviewDock"));
    restorePanelDockSize(m_tagDock, QStringLiteral("TagDock"));
    for (const auto &dock : std::as_const(m_metadataDocks)) {
        restorePanelDockSize(dock, QStringLiteral("MetadataDock"));
    }
}

void MainWindow::setStackedLayout(bool stacked)
{
    const bool changed = isStackedLayout() != stacked;
    if (changed) {
        if (m_editorDock && !m_editorDock->isHidden()) {
            savePanelDockSize(m_editorDock, QStringLiteral("EditorDock"));
        }
        if (!m_previewDock->isHidden()) {
            savePanelDockSize(m_previewDock, QStringLiteral("PreviewDock"));
        }
        if (!m_tagDock->isHidden()) {
            savePanelDockSize(m_tagDock, QStringLiteral("TagDock"));
        }
        for (const auto &dock : std::as_const(m_metadataDocks)) {
            if (dock && !dock->isHidden()) {
                savePanelDockSize(dock, QStringLiteral("MetadataDock"));
            }
        }
    }
    m_stackedLayoutAction->setChecked(stacked);
    m_splitter->setOrientation(stacked ? Qt::Vertical : Qt::Horizontal);
    updateTerminalPlacement();
    restoreSplitterSizes(true, true, changed);
    restoreEditorDockSize();
    restorePanelDockSize(m_previewDock, QStringLiteral("PreviewDock"));
    restorePanelDockSize(m_tagDock, QStringLiteral("TagDock"));
    for (const auto &dock : std::as_const(m_metadataDocks)) {
        restorePanelDockSize(dock, QStringLiteral("MetadataDock"));
    }
    updateTabBarPlacement();
}

void MainWindow::updateTabBarPlacement()
{
    const bool stacked = isStackedLayout();
    const bool together = stacked && Settings::stackedTabBarsTogether();
    const int first = together ? int(Pane::TabBarPosition::Bottom) : stacked ? Settings::stackedFirstTabPosition() : Settings::horizontalFirstTabPosition();
    const int second = together ? int(Pane::TabBarPosition::Top) : stacked ? Settings::stackedSecondTabPosition() : Settings::horizontalSecondTabPosition();
    m_left->setTabBarPosition(static_cast<Pane::TabBarPosition>(qBound(0, first, 3)));
    m_right->setTabBarPosition(static_cast<Pane::TabBarPosition>(qBound(0, second, 3)));
    auto *position = qobject_cast<KSelectAction *>(actionCollection()->action(QStringLiteral("tab_bar_position")));
    position->setCurrentItem(static_cast<int>(activePane()->tabBarPosition()));
    actionCollection()->action(QStringLiteral("stacked_tab_bars_together"))->setEnabled(stacked);
}

void MainWindow::updateTerminalPlacement()
{
    const auto placement = Settings::terminalPlacement();
    const bool right = placement == Settings::EnumTerminalPlacement::Right || (placement == Settings::EnumTerminalPlacement::Automatic && isStackedLayout());
    m_workspaceSplitter->setOrientation(right ? Qt::Horizontal : Qt::Vertical);
    updateSearchPlacement();
}

void MainWindow::updateSearchPlacement()
{
    if (!m_searchSplitter) {
        return;
    }
    const auto placement = Settings::searchPanelPlacement();
    const Qt::Orientation orientation = placement == Settings::EnumSearchPanelPlacement::Right ? Qt::Horizontal
        : placement == Settings::EnumSearchPanelPlacement::Below                               ? Qt::Vertical
                                                                                               : m_workspaceSplitter->orientation();
    m_searchSplitter->setOrientation(orientation);
    // A splitter has one orientation, so a panel placed across the terminal's direction needs the outer splitter.
    QSplitter *target = orientation == m_workspaceSplitter->orientation() ? m_workspaceSplitter : m_searchSplitter;
    if (m_searchPanel->parentWidget() == target) {
        return;
    }
    const bool hidden = m_searchPanel->isHidden();
    const QPointer<QWidget> focus = m_searchPanel->isAncestorOf(QApplication::focusWidget()) ? QApplication::focusWidget() : nullptr;
    target->addWidget(m_searchPanel);
    target->setStretchFactor(target->indexOf(m_searchPanel), 1);
    m_searchPanel->setVisible(!hidden);
    if (focus) {
        focus->setFocus(Qt::OtherFocusReason);
    }
}

void MainWindow::updatePreviewPanel()
{
    if (!m_previewPanelAction || !m_previewPanelAction->isChecked()) {
        return;
    }
    FolderView *view = activePane()->view();
    const KFileItemList selected = view->selectedItems();
    QList<QUrl> images;
    if (selected.size() == 1 && !selected.first().isDir() && previewMimeType(selected.first(), true).startsWith(QLatin1String("image/"))) {
        for (const KFileItem &item : view->itemsInViewOrder()) {
            if (!item.isDir() && (item.url() == selected.first().url() || previewMimeType(item).startsWith(QLatin1String("image/")))) {
                images.append(item.url());
            }
        }
    }
    m_previewPanel->setItems(selected, images);
}

void MainWindow::updateInformationPanel()
{
    updateTagPanel();
    if (!m_informationPanel) {
        return;
    }
    KFileItem item;
    if (Settings::informationHover() && !m_hoveredInformationItem.isNull()) {
        item = m_hoveredInformationItem;
    } else if (const KFileItemList selected = activePane()->view()->selectedItems(); selected.size() == 1) {
        item = selected.first();
    } else if (selected.isEmpty()) {
        item = activePane()->view()->currentItem();
    }
    m_informationPanel->setItem(item);
    updateNextcloudInformation(item);
}

bool MainWindow::isStackedLayout() const
{
    return m_splitter->orientation() == Qt::Vertical;
}

void MainWindow::updateTagPanel()
{
    if (!m_tagPanel || !m_tagDock || m_tagDock->isHidden() || !m_left) {
        return;
    }
    m_tagPanel->setContext(activePane()->url(), activePane()->view()->selectedItems());
}

void MainWindow::reloadLibraryViews(const QString &root)
{
    TagAssignment::invalidate(root);
    if (!m_tagDock->isHidden()) {
        m_tagPanel->reload();
    }
    updateInformationPanel();
}

void MainWindow::setTagPanelPlacement(TagPanel::Placement placement)
{
    const bool below = placement == TagPanel::Placement::Below;
    const bool shown = !m_tagDock->isHidden();
    if (shown) {
        savePanelDockSize(m_tagDock, QStringLiteral("TagDock"));
    }
    addDockWidget(below ? Qt::BottomDockWidgetArea : Qt::RightDockWidgetArea, m_tagDock);
    m_tagDock->setVisible(shown);
    m_tagPanel->setPlacement(placement);
    saveTagPanelPlacement(below);
    restorePanelDockSize(m_tagDock, QStringLiteral("TagDock"));
}

void MainWindow::applyTagSettings()
{
    const bool below = Settings::tagPanelPlacement() == Settings::EnumTagPanelPlacement::Below;
    if ((dockWidgetArea(m_tagDock) == Qt::BottomDockWidgetArea) != below) {
        setTagPanelPlacement(below ? TagPanel::Placement::Below : TagPanel::Placement::Right);
    }
    // Tag colours may have been shown or hidden.
    if (!m_tagDock->isHidden()) {
        m_tagPanel->reload();
    }
    for (TagManagerWidget *manager : findChildren<TagManagerWidget *>()) {
        manager->reload();
    }
    for (TagGroupManagerWidget *groups : findChildren<TagGroupManagerWidget *>()) {
        groups->reload();
    }
}

void MainWindow::saveTagPanelPlacement(bool below)
{
    const auto placement = below ? Settings::EnumTagPanelPlacement::Below : Settings::EnumTagPanelPlacement::Right;
    if (Settings::tagPanelPlacement() != placement) {
        Settings::setTagPanelPlacement(placement);
        Settings::self()->save();
    }
}

bool MainWindow::isDualLayout() const
{
    return m_dualLayoutAction->isChecked();
}

DesktopTray *MainWindow::desktopTray() const
{
    return m_desktopTray;
}

CloudSync::Scheduler *MainWindow::cloudSync() const
{
    return m_cloudSync;
}

void MainWindow::showCloudSync()
{
    m_desktopTray->setAttention({});
    if (!m_cloudSyncDialog) {
        m_cloudSyncDialog = new CloudSyncDialog(m_cloudSync, KSharedConfig::openConfig(), rcloneMounts(), this);
        m_cloudSyncDialog->setAttribute(Qt::WA_DeleteOnClose);
    }
    DesktopTray::bringToFront(m_cloudSyncDialog);
}

CloudActivity::Monitor *MainWindow::cloudActivity() const
{
    return m_cloudActivity;
}

void MainWindow::showCloudActivity()
{
    if (!m_cloudActivityDialog) {
        CloudActivityDialog::Sources sources;
        sources.monitor = m_cloudActivity;
        sources.queue = FileOperations::transferQueue();
        sources.scheduler = m_cloudSync;
        sources.rclone = m_rcloneMounts;
        sources.remoteMounts = m_remoteMounts;
        sources.config = KSharedConfig::openConfig();
        sources.cacheDir = CloudActivity::rcloneCacheDir();
        m_cloudActivityDialog = new CloudActivityDialog(sources, this);
        m_cloudActivityDialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(m_cloudActivityDialog, &CloudActivityDialog::limitsChanged, this, &MainWindow::applyCloudLimits);
        // Preferences › Remote and Cloud › Transfers and Cache shows the same keys.
        connect(m_cloudActivityDialog, &CloudActivityDialog::limitsChanged, this, [] {
            for (const QString &name : {QStringLiteral("CloudBandwidthLimitKiB"),
                                        QStringLiteral("CloudParallelTransfers"),
                                        QStringLiteral("CloudCacheSizeMiB"),
                                        QStringLiteral("CloudCacheMaxAgeHours")}) {
                Settings::self()->findItem(name)->readConfig(Settings::self()->config());
            }
        });
    }
    DesktopTray::bringToFront(m_cloudActivityDialog);
}

void MainWindow::applyCloudLimits(const CloudActivity::Limits &limits)
{
    m_cloudSync->setRcloneOptions(CloudActivity::rcloneTransferOptions(limits));
    if (m_rcloneMounts) {
        m_rcloneMounts->setMountOptions(CloudActivity::rcloneMountOptions(limits, CloudActivity::rcloneCacheDir()));
    }
}

bool MainWindow::waitForCloudUploads(const QString &text, const QStringList &remotes, bool syncRuns)
{
    const QString cacheDir = CloudActivity::rcloneCacheDir();
    QPointer<MainWindow> self(this);
    const auto status = [self, remotes, syncRuns, cacheDir] {
        CloudActivity::UploadStatus status;
        if (!self) {
            return status;
        }
        // rclone uploads only while a remote is mounted or served.
        QStringList checked = remotes;
        if (checked.isEmpty() && self->m_rcloneMounts) {
            checked = self->m_rcloneMounts->mountedRemotes() + self->m_rcloneMounts->servedRemotes();
        }
        for (const QString &remote : checked) {
            const CloudActivity::PendingUploads pending = CloudActivity::pendingUploads(cacheDir, remote);
            status.uploads.files += pending.files;
            status.uploads.bytes += pending.bytes;
        }
        if (syncRuns) {
            status.syncRuns = self->m_cloudSync->activeRuns();
        }
        return status;
    };
    return CloudUploadWaitDialog::wait(this, text, status);
}

KWinIntegration *MainWindow::kwinIntegration() const
{
    return m_kwin;
}

void MainWindow::setKWinBackend(std::unique_ptr<KWinBackend> backend)
{
    m_kwin->setBackend(std::move(backend));
}

void MainWindow::showKWinIntegration()
{
    showSettings();
    if (auto *dialog = qobject_cast<SettingsDialog *>(KConfigDialog::exists(QLatin1String(SettingsDialog::Name)))) {
        dialog->showKWinPage();
    }
}

void MainWindow::toggleDropDown()
{
    m_dropDown->toggle();
}

DropDownWindow *MainWindow::dropDownWindow() const
{
    return m_dropDown;
}

void MainWindow::refreshFolderColors()
{
    FolderColors::clearCache();
    for (Pane *pane : {m_left, m_right}) {
        pane->refreshFolderColors();
    }
    const auto views = findChildren<QAbstractItemView *>();
    for (QAbstractItemView *view : views) {
        view->viewport()->update();
    }
}

void MainWindow::applyFonts()
{
    const QFont list = AppFonts::fromSetting(EffectiveAppearance::fileListFont());
    const QFont tree = AppFonts::fromSetting(EffectiveAppearance::folderTreeFont());
    const QFont tabs = AppFonts::fromSetting(EffectiveAppearance::tabFont());
    const QFont status = AppFonts::fromSetting(EffectiveAppearance::statusBarFont());
    for (Pane *pane : {m_left, m_right}) {
        pane->view()->setListFont(list);
        pane->setTabFont(tabs);
        pane->statusBarWidget()->setFont(status);
    }
    for (FolderTree *folderTree : {m_folderTree, m_combinedPanel->folderTree()}) {
        folderTree->setFont(tree);
    }
    statusBar()->setFont(status);
}

void MainWindow::applySettings()
{
    // Saved theme profiles may have changed.
    updateWorkspaceAppearanceLayer();
    m_appliedAppearanceLayer = EffectiveAppearance::layer();
    applyAppearance();
    applyLocationPresentationDefaults();
    applyWorkspaceSelectorPlacement();
    scheduleWorkspaceSelectorRefresh();
    m_desktopTray->applySettings();
    m_activityController->applySettings();
    syncFileTypePlaces();
    actionCollection()->action(QStringLiteral("kdeconnect_send"))->setVisible(Settings::kdeConnectSharing());
    actionCollection()->action(QStringLiteral("send_by_email"))->setVisible(Settings::sendByEmail());
    actionCollection()->action(QStringLiteral("online_accounts"))->setVisible(Settings::onlineAccountsEnabled());
    // Hiding it also turns off its shortcut.
    actionCollection()->action(QStringLiteral("network_locations"))->setVisible(Settings::remoteShowNetwork());
    actionCollection()->action(QStringLiteral("cloud_storage"))->setVisible(Settings::rcloneEnabled());
    actionCollection()->action(QStringLiteral("plasma_vaults"))->setVisible(Settings::plasmaVaultMenu());
    applyEncryptionSettings();
    if (m_remoteMounts) {
        // Mounts already in place keep their folder and stay or stop at quit as they were started.
        m_remoteMounts->setMountFolder(RemoteMounts::defaultsFromSettings().folder);
        m_remoteMounts->setUnmountOnExit(Settings::remoteDriveUnmountOnExit());
    }
    QString autostartError;
    if (!RemoteMounts::setMountAtLogin(Settings::remoteDriveMountAtLogin(),
                                       QCoreApplication::applicationFilePath(),
                                       AppIdentity::desktopFileName(),
                                       &autostartError)) {
        showStatusMessage(autostartError);
    }
    // Both autostart entries start the same tray instance; later launches are ignored.
    if (!CloudSync::setStartAtLogin(Settings::cloudSyncAtLogin(), QCoreApplication::applicationFilePath(), AppIdentity::desktopFileName(), &autostartError)) {
        showStatusMessage(autostartError);
    }
    // Mounts and servers already running keep their options until they are mounted again.
    applyCloudLimits(CloudActivity::loadLimits(KSharedConfig::openConfig()->group(CloudActivity::limitsGroupName())));
    if (m_cloudActivityDialog) {
        m_cloudActivityDialog->reloadLimits();
    }
    QAction *terminalHistory = actionCollection()->action(QStringLiteral("search_terminal_history"));
    terminalHistory->setVisible(Settings::atuinHistory());
    terminalHistory->setEnabled(Settings::atuinHistory());
    if (m_mountBackend) {
        // Lists system file systems, or leaves them out, as now chosen.
        m_mountBackend->refresh();
    }
    if (!Settings::kdeConnectRememberDevice()) {
        KdeConnectShare::forgetDevice();
    }
    // The KWin script reads the window management and drop-down options at its next sync.
    m_kwin->requestSync();
    const bool toolbarsLocked = Settings::lockToolbars();
    QAction *customizeToolbar = actionCollection()->action(QStringLiteral("customize_main_toolbar"));
    customizeToolbar->setEnabled(!toolbarsLocked);
    if (toolbarsLocked) {
        customizeToolbar->setChecked(false);
    }
    for (Pane *pane : {m_left, m_right}) {
        if (QAction *configure = pane->toolbarAction(QStringLiteral("paneCustomizeToolbar"))) {
            configure->setEnabled(!toolbarsLocked);
        }
    }
    for (KToolBar *toolbar : toolBars()) {
        toolbar->setMovable(!toolbarsLocked);
    }

    const auto terminalOrientation = m_workspaceSplitter->orientation();
    const auto searchOrientation = m_searchSplitter->orientation();
    const QWidget *searchParent = m_searchPanel->parentWidget();
    updateTerminalPlacement();
    const bool searchMoved = searchParent != m_searchPanel->parentWidget();
    if (terminalOrientation != m_workspaceSplitter->orientation() || searchOrientation != m_searchSplitter->orientation() || searchMoved) {
        restoreSplitterSizes(false, true, searchMoved);
    }
    actionCollection()->action(QStringLiteral("dim_inactive_pane"))->setChecked(Settings::dimInactivePane());
    actionCollection()->action(QStringLiteral("stacked_tab_bars_together"))->setChecked(Settings::stackedTabBarsTogether());
    actionCollection()->action(QStringLiteral("show_tab_close_buttons"))->setChecked(Settings::showTabCloseButtons());
    updateTabBarPlacement();
    for (GalleryWindow *gallery : findChildren<GalleryWindow *>()) {
        gallery->applyPresentationSettings();
    }
    if (m_terminalPanelAction && m_terminalPanelAction->isChecked()) {
        m_terminalPanel->start(activePane()->url());
    }
    m_searchPanel->searchWidget()->applyDefaults();
    // Preferences › Applications › Editor may have changed the workspace vaults.
    if (!m_workspaceConfig->isDirty()) {
        m_workspaceConfig->reparseConfiguration();
        WorkspaceCatalog::Catalog stored;
        QString workspaceError;
        if (WorkspaceCatalog::load(m_workspaceConfig->group(QStringLiteral("Workspaces")), stored, workspaceError) && stored != m_workspaceCatalog) {
            reloadWorkspaceCatalog();
        }
    }
    // Including the editors that previews open.
    for (EditorPanel *editor : findChildren<EditorPanel *>()) {
        editor->applySettings();
    }
    const bool sizeDefaultsChanged =
        m_automaticLocalSizeDefault != Settings::automaticLocalFolderSizes() || m_automaticRemoteSizeDefault != Settings::automaticRemoteFolderSizes();
    m_automaticLocalSizeDefault = Settings::automaticLocalFolderSizes();
    m_automaticRemoteSizeDefault = Settings::automaticRemoteFolderSizes();
    applyPaneBackgrounds();
    m_yaziVimKeysAction->setChecked(Settings::yaziKeyPreset() == Settings::EnumYaziKeyPreset::Vim);
    m_vimNavigationAction->setChecked(Settings::vimNavigation());
    m_windowSequenceHints->reloadSettings();
    for (Pane *pane : {m_left, m_right}) {
        pane->setInactiveDimming(Settings::dimInactivePane(), Settings::inactivePaneOpacity());
        pane->reloadTabSettings();
        pane->reloadFilterSettings();
        pane->setNavigationBarVisible(Settings::showNavigationBar());
        pane->setNewTabsAfterCurrent(Settings::newTabsAfterCurrent());
        pane->reloadYaziPreviewSetting();
        pane->reloadYaziKeySettings();
        pane->reloadVimNavigationSetting();
        pane->reloadShortcutHintSettings();
        pane->setFollowSymlinks(Settings::followSymlinks());

        FolderView *view = pane->view();
        view->setInformationHoverEnabled(Settings::informationHover());
        view->reloadListingSettings();
        if (sizeDefaultsChanged) {
            view->refreshFolderSizePolicy();
        }
        if (!view->hasFolderFormat()) {
            view->setRatingColumn(Settings::showRatingColumn());
            view->setColumnVisible(KDirModel::Size, Settings::showSizeColumn());
            view->setColumnVisible(KDirModel::ModifiedTime, Settings::showModifiedColumn());
            view->setColumnVisible(KDirModel::Permissions, Settings::showPermissionsColumn());
            view->setColumnVisible(KDirModel::Owner, Settings::showOwnerColumn());
            view->setColumnVisible(KDirModel::Group, Settings::showGroupColumn() || Settings::showRatingColumn());
            view->setColumnVisible(KDirModel::Type, Settings::showTypeColumn());
        }
        view->setSortFoldersFirst(Settings::foldersFirst());
        view->setShowToolTips(Settings::showToolTips());
        view->setDragFromAnywhere(Settings::dragFromAnywhere());
        view->setItemSpacing(Density::itemSpace(itemDensity<Settings::EnumDetailsDensity>(Settings::detailsDensity()), Settings::detailsCustomSpace()),
                             Density::itemSpace(itemDensity<Settings::EnumIconsDensity>(Settings::iconsDensity()), Settings::iconsCustomSpace()));
        view->setPreviewPlugins(Settings::previewPlugins());
        view->setRemotePreviewsEnabled(Settings::showRemotePreviews());
    }
    m_toolbarCustomizer->applyAppearanceSettings();
    m_informationPanel->setFields(Settings::informationFields());
    if (!Settings::informationHover()) {
        m_hoveredInformationPane = nullptr;
        m_hoveredInformationItem = {};
    }
    updateTabBars();
    applyBarMargins();
    applyPanelSettings();
    applyEmbeddedPanelTitles();
    updateStatusBarIndicatorOrder();
    updateStatusBar();
    updateFreeSpace();
    applyTagSettings();
    applyNextcloudSettings();
    updateInformationPanel();

    QAction *switchPane = actionCollection()->action(QStringLiteral("switch_pane"));
    QList<QKeySequence> shortcuts = switchPane->shortcuts();
    shortcuts.removeAll(QKeySequence(Qt::Key_Tab));
    if (Settings::tabSwitchesPanes()) {
        shortcuts.prepend(QKeySequence(Qt::Key_Tab));
    }
    switchPane->setShortcuts(shortcuts);
    if (factory()) {
        WindowShortcuts::applyNavigationKeys(this);
    }
    setPreviewPlacement(static_cast<PreviewPanel::Placement>(Settings::previewPlacement()));
    m_previewPanel->applyPresentationSettings();
    updatePreviewPanel();
    updateDoubleShift();
    updateActions();
}

void MainWindow::updateDoubleShift()
{
    const bool wanted = Settings::doubleShiftSearch() && DoubleShiftDetector::unavailableReason().isEmpty();
    if (!wanted) {
        delete m_doubleShift;
        m_doubleShift = nullptr;
        return;
    }
    if (m_doubleShift) {
        return;
    }
    auto *detector = new DoubleShiftDetector(
        [this] {
            return windowHandle();
        },
        this);
    connect(detector, &DoubleShiftDetector::triggered, this, [this] {
        if (QAction *search = actionCollection()->action(QStringLiteral("search_folder")); search && search->isEnabled()) {
            search->trigger();
        }
    });
    m_doubleShift = detector;
}

void MainWindow::applyAppearance()
{
    applyColorScheme();
    applyWindowEffects();
    applyBackgroundOpacity();
    applyAppearanceStyle();
    applyFonts();
    refreshFolderColors();
    applyCustomStyleSheet();
    const bool translucency =
        EffectiveAppearance::windowOpacity() < 100 || EffectiveAppearance::paneOpacity() < 100 || EffectiveAppearance::panelOpacity() < 100;
    if (AppearanceDefaults::effectsSuspended()) {
        // applyColorScheme() reports the suspended effects instead.
    } else if (const QString reason = WindowEffects::unavailableReason(this, EffectiveAppearance::backgroundBlur(), translucency); !reason.isEmpty()) {
        statusBar()->showMessage(reason);
    }
}

void MainWindow::applyBarMargins()
{
    const int barMargin = Density::barMargin(barDensity(Settings::barDensity()), Settings::barCustomSpacing());
    const int spacingScale = AppearanceStyle::configured().spacingScale;
    const auto bars = toolBars();
    for (KToolBar *bar : bars) {
        setBarMargin(bar, barMargin, spacingScale);
    }
    setBarMargin(statusBar(), barMargin, spacingScale);
    for (Pane *pane : {m_left, m_right}) {
        setBarMargin(pane->navigationBar(), barMargin, spacingScale);
        // Pane status bars are already as tight as Compact, so they only follow the spacing scale.
        setBarMargin(pane->statusBarWidget(), -1, spacingScale);
    }
}

void MainWindow::applyPanelSettings()
{
    const bool combined = m_panelLayout == PanelLayout::Combined;
    const Qt::DockWidgetArea area = Settings::panelPosition() == Settings::EnumPanelPosition::right ? Qt::RightDockWidgetArea : Qt::LeftDockWidgetArea;
    m_autoFocusFolders = Settings::panelAutoFocus();
    addDockWidget(area, m_combinedDock);
    addDockWidget(area, m_placesDock);
    addDockWidget(area, m_foldersDock);
    splitDockWidget(m_placesDock, m_foldersDock, Qt::Vertical);
    m_combinedDock->setVisible(Settings::panelsVisible() && combined);
    m_placesDock->setVisible(Settings::panelsVisible() && !combined && Settings::showPlaces());
    m_foldersDock->setVisible(Settings::panelsVisible() && !combined);
    for (QDockWidget *dock : {m_combinedDock, m_placesDock, m_foldersDock}) {
        setDockTitleVisible(dock, Settings::panelTitlesVisible());
    }
    m_combinedPanel->setSectionVisibility(Settings::favoritesVisible(), Settings::placesVisible(), Settings::foldersVisible());
    if (m_active && m_autoFocusFolders) {
        syncFolderTrees();
    }
}

void MainWindow::applyEmbeddedPanelTitles()
{
    const bool visible = Settings::embeddedPanelTitlesVisible();
    m_terminalPanel->setTitleVisible(visible);
    m_searchPanel->setTitleVisible(visible);
    m_previewPanel->setTitleVisible(visible);
    setDockTitleVisible(m_previewDock, visible);
    setDockTitleVisible(m_editorDock, visible);
    setDockTitleVisible(m_informationDock, visible);
    setDockTitleVisible(m_tagDock, visible);
    m_tagPanel->setTitleVisible(visible);
    m_informationPanel->setTitleVisible(visible);
    for (const auto &dock : std::as_const(m_metadataDocks)) {
        setDockTitleVisible(dock, visible);
    }
    if (m_editor) {
        const bool embedded = m_editorPlacement == EditorPanel::Placement::Panel || m_editorPlacement == EditorPanel::Placement::Below
            || m_editorPlacement == EditorPanel::Placement::Right;
        m_editor->setPanelTitleVisible(!embedded || visible);
    }
}

void MainWindow::rememberPlacesShown(bool shown)
{
    Settings::setShowPlaces(shown);
    Settings::self()->save();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_placesDock && event->type() == QEvent::Close) {
        rememberPlacesShown(false);
    }
    if (watched == m_rightWindow && event->type() == QEvent::Close && !m_movingSecondPane) {
        moveSecondPaneToDual();
        return true;
    }
    return KXmlGuiWindow::eventFilter(watched, event);
}

void MainWindow::applyViewDefaults()
{
    const auto mode = static_cast<FolderView::ViewMode>(qBound(0, int(EffectiveAppearance::defaultViewMode()), int(FolderView::ViewMode::Cards)));
    const Qt::SortOrder order = Settings::sortDescending() ? Qt::DescendingOrder : Qt::AscendingOrder;
    for (Pane *pane : {m_left, m_right}) {
        pane->view()->setViewMode(mode);
        pane->view()->setSorting(Settings::sortColumn(), order);
        pane->view()->setShowHiddenFiles(Settings::showHiddenFiles());
    }
    m_previewsAction->setChecked(Settings::showPreviews());
    setPreviewsEnabled(Settings::showPreviews());
}

void MainWindow::showCommandPalette()
{
    if (m_commandPalette) {
        m_commandPalette->setFocus(Qt::ShortcutFocusReason);
        return;
    }
    Pane *pane = activePane();
    m_commandPalette = new CommandPaletteBar(this);
    m_commandPalette->setFallbackFocus(pane->view()->currentView());
    // Pane actions run in the pane that had the focus, even when closing the palette moved it.
    connect(m_commandPalette, &CommandPaletteBar::focusReturned, this, [this, pane = QPointer<Pane>(pane)] {
        if (pane) {
            setActivePane(pane);
        }
    });
    CommandPalette::Options options;
    options.savedCommands = Settings::commandPaletteSavedCommands();
    m_commandPalette->open(options);
}

void MainWindow::showKeyboardHints()
{
    if (m_keyboardHints->isActive()) {
        m_keyboardHints->cancel();
        return;
    }
    Pane *pane = activePane();
    // Labels take every key, so they never start while keys are text: in a text
    // field, an inline rename editor, the terminal or an input method's editor.
    QWidget *focus = QApplication::focusWidget();
    const bool typing = focus
        && (ShortcutGuard::focusContext(focus) == KeyBindings::Context::TextInput
            || (focus->testAttribute(Qt::WA_InputMethodEnabled) && !qobject_cast<QAbstractItemView *>(focus))
            || (m_terminalPanel && m_terminalPanel->isAncestorOf(focus)));
    QAbstractItemView *view = pane->view()->currentView();
    if (typing || (view && YaziKeys::isEditing(view))) {
        showStatusMessage(i18nc("@info:status", "Keyboard hints aren't shown while typing; move the keyboard focus out of the text first."));
        return;
    }
    m_shortcutReference->hide();
    QList<KFilePlacesView *> places;
    const QList<KFilePlacesView *> found = findChildren<KFilePlacesView *>();
    for (KFilePlacesView *placesView : found) {
        if (placesView->isVisible() && placesView->window() == this) {
            places.append(placesView);
        }
    }
    const auto layout = static_cast<KeyboardHints::Layout>(Settings::keyboardHintKeys());
    const auto action = Settings::keyboardHintItemAction() == Settings::EnumKeyboardHintItemAction::Open ? KeyboardHintOverlay::ItemAction::Open
                                                                                                         : KeyboardHintOverlay::ItemAction::Select;
    if (!m_keyboardHints->start(pane, places, KeyboardHints::alphabet(layout), action)) {
        showStatusMessage(i18nc("@info:status", "Nothing to label with keyboard hints."));
    }
}

void MainWindow::showKeyboardMap()
{
    if (!m_keyboardMap) {
        m_keyboardMap = new KeyboardMapDialog(this, this);
        m_keyboardMap->setAttribute(Qt::WA_DeleteOnClose);
    }
    m_keyboardMap->show();
    m_keyboardMap->raise();
    m_keyboardMap->activateWindow();
}

void MainWindow::showSettings()
{
    if (KConfigDialog::showDialog(QLatin1String(SettingsDialog::Name))) {
        return;
    }
    auto *dialog = new SettingsDialog(m_placesModel, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &KConfigDialog::settingsChanged, this, &MainWindow::applySettings);
    connect(dialog, &SettingsDialog::customCommandsChanged, this, &MainWindow::refreshCustomCommandActions);
    connect(dialog, &SettingsDialog::manageWorkspacesRequested, this, &MainWindow::showWorkspaces);
    connect(dialog, &SettingsDialog::showItemsRequested, this, [this](const QList<QUrl> &items) {
        showItems(items);
        activateWindow();
        raise();
    });
    connect(this, &MainWindow::workspaceCatalogChanged, dialog, &SettingsDialog::reloadWorkspaces);
    // Library changes in Preferences apply at once, to every window.
    connect(dialog, &SettingsDialog::libraryChanged, this, [](const QString &root) {
        const QList<KMainWindow *> windows = KMainWindow::memberList();
        for (KMainWindow *member : windows) {
            if (auto *window = qobject_cast<MainWindow *>(member)) {
                window->reloadLibraryViews(root);
                window->updateActions();
            }
        }
    });
    connect(dialog, &SettingsDialog::libraryAdded, this, [](const QString &root) {
        LibraryAutoRefresh::instance()->enqueue(root, true);
    });
    // The restored defaults are saved after the signal, so the panes of every window follow them once control returns.
    connect(dialog, &SettingsDialog::yaziDefaultsRestored, this, [] {
        const QList<KMainWindow *> windows = KMainWindow::memberList();
        for (KMainWindow *member : windows) {
            if (auto *window = qobject_cast<MainWindow *>(member)) {
                QTimer::singleShot(0, window, [window] {
                    for (Pane *pane : {window->m_left, window->m_right}) {
                        pane->followYaziSettings();
                    }
                });
            }
        }
    });
    dialog->show();
}

void MainWindow::showToolbarSettings()
{
    showSettings();
    if (auto *dialog = qobject_cast<SettingsDialog *>(KConfigDialog::exists(QLatin1String(SettingsDialog::Name)))) {
        dialog->showToolbarPage();
    }
}

void MainWindow::toggleViewMode()
{
    FolderView *view = activePane()->view();
    view->setViewMode(view->viewMode() == FolderView::ViewMode::Details ? FolderView::ViewMode::Icons : FolderView::ViewMode::Details);
    updateActions();
}

void MainWindow::setFlat(bool flat)
{
    const QString key = flatKey(activePane()->url());
    m_flatLocations.removeAll(key);
    if (flat) {
        m_flatLocations.append(key);
    }
    KConfigGroup group = KSharedConfig::openStateConfig()->group(QLatin1String(FlatGroup));
    group.writeEntry("Locations", m_flatLocations);
    group.sync();
    activePane()->view()->setFlat(flat);
    updateActions();
}

void MainWindow::syncFolderTrees()
{
    const QUrl url = activePane()->url();
    const bool hidden = activePane()->view()->showHiddenFiles();
    for (FolderTree *tree : {m_folderTree, m_combinedPanel->folderTree()}) {
        tree->setShowHiddenFolders(hidden);
        if (m_autoFocusFolders) {
            tree->reveal(url);
        }
    }
}

void MainWindow::showFolderTreeMenu(const QUrl &url, const QPoint &globalPos)
{
    QMenu menu(this);
    const KConfigGroup context = KSharedConfig::openConfig()->group(QStringLiteral("ContextMenu"));
    QAction *openInTab = context.readEntry("ShowOpenInNewTab", true)
        ? menu.addAction(QIcon::fromTheme(QStringLiteral("tab-new")), i18nc("@action:inmenu", "Open in New Tab"))
        : nullptr;
    QAction *openInOther = nullptr;
    if (context.readEntry("ShowOpenInOtherPane", true) && m_dualLayoutAction->isChecked()) {
        openInOther = menu.addAction(QIcon::fromTheme(QStringLiteral("view-split-left-right")), i18nc("@action:inmenu", "Open in Other Pane"));
    }
    QAction *favorite = menu.addAction(QIcon::fromTheme(QStringLiteral("bookmark-new")), i18nc("@action:inmenu", "Add to Favorites"));
    favorite->setEnabled(!m_favorites->contains(url));
    menu.addSeparator();
    QAction *properties = menu.addAction(QIcon::fromTheme(QStringLiteral("document-properties")), i18nc("@action:inmenu", "Properties"));

    QAction *chosen = menu.exec(globalPos);
    if (chosen && chosen == openInTab) {
        activePane()->openInNewTab(url, true);
    } else if (chosen && chosen == openInOther) {
        inactivePane()->setUrl(url);
    } else if (chosen == favorite) {
        m_favorites->add(url);
    } else if (chosen == properties) {
        FileOperations::showProperties(KFileItemList{KFileItem(url)}, this);
    }
}

void MainWindow::setShowHiddenFiles(bool show)
{
    activePane()->view()->setShowHiddenFiles(show);
    syncFolderTrees();
}

void MainWindow::setPreviewsEnabled(bool enabled)
{
    m_left->view()->setPreviewsEnabled(enabled);
    m_right->view()->setPreviewsEnabled(enabled);
}
