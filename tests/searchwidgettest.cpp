// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchwidget.h"

#include "favoritesmodel.h"
#include "folderview.h"
#include "indexbackend.h"
#include "indexstatusdialog.h"
#include "libraryregistry.h"
#include "mainwindow.h"
#include "pane.h"
#include "savedpathfilters.h"
#include "searchcollection.h"
#include "searchdefaults.h"
#include "searchhistory.h"
#include "searchpanel.h"
#include "settings.h"
#include "workspacelocations.h"

#include <KActionCollection>
#include <KFilePlacesModel>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KToolBar>

#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QCompleter>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QScopeGuard>
#include <QScrollArea>
#include <QSignalSpy>
#include <QSpinBox>
#include <QSplitter>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTabBar>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QWidgetAction>

namespace
{
class FakeIndexBackend : public IndexBackend
{
public:
    using IndexBackend::IndexBackend;

    IndexStatus::Config config() const override
    {
        ++reads;
        return settings;
    }
    IndexStatus::Service service() const override
    {
        return indexer;
    }
    bool isIndexed(const QString &) const override
    {
        return false;
    }
    bool hasExtractor(const QString &) const override
    {
        return true;
    }
    bool suspend(QString *) override
    {
        return false;
    }
    bool resume(QString *) override
    {
        return false;
    }
    bool checkUnindexedFiles(QString *) override
    {
        return false;
    }
    bool openSettings(QString *) override
    {
        return false;
    }

    IndexStatus::Config settings;
    IndexStatus::Service indexer;
    mutable int reads = 0;
};
}

class SearchWidgetTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void showsIndexWarningsInBalooScope()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const auto config = KSharedConfig::openConfig(root.filePath(QStringLiteral("searchrc")), KConfig::SimpleConfig);
        FakeIndexBackend backend;
        backend.settings.available = true;
        backend.settings.enabled = true;
        backend.settings.includeFolders = {root.path()};
        backend.indexer.running = true;
        backend.indexer.state = int(IndexStatus::State::Idle);
        SearchWidget widget(QUrl::fromLocalFile(root.path()), nullptr, config);
        widget.setIndexBackend(&backend);
        widget.show();
        auto *scope = widget.findChild<QComboBox *>(QStringLiteral("searchScope"));
        auto *notice = widget.findChild<QLabel *>(QStringLiteral("searchIndexNotice"));
        auto *status = widget.findChild<QPushButton *>(QStringLiteral("searchIndexStatus"));
        QVERIFY(scope && notice && status);
        QCOMPARE(scope->currentIndex(), 0);
        QVERIFY(!notice->isVisible() && !status->isVisible());
        // The live scope doesn't read the index.
        QCOMPARE(backend.reads, 0);

        scope->setCurrentIndex(1);
        QVERIFY(status->isVisible());
        QVERIFY(!notice->isVisible());
        QCOMPARE(backend.reads, 1);

        backend.indexer.state = int(IndexStatus::State::Suspended);
        Q_EMIT backend.serviceChanged();
        QVERIFY(notice->isVisible());
        QVERIFY(notice->text().contains(QStringLiteral("suspended")));

        // Changing folders reuses the state read last.
        QVERIFY(widget.setFolder(QUrl::fromLocalFile(QStringLiteral("/srv"))));
        QCOMPARE(backend.reads, 2);
        QVERIFY(notice->text().contains(QStringLiteral("outside the indexed folders")));

        status->click();
        auto *dialog = widget.findChild<IndexStatusDialog *>();
        QVERIFY(dialog);
        QVERIFY(dialog->findChild<QLabel *>(QStringLiteral("indexFolderCoverage"))->text().contains(QStringLiteral("outside")));
        dialog->close();

        scope->setCurrentIndex(0);
        QVERIFY(!notice->isVisible() && !status->isVisible());
    }

    void usesSavedSearchDefaults()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const auto config = KSharedConfig::openConfig(root.filePath(QStringLiteral("searchrc")), KConfig::SimpleConfig);
        auto defaults = config->group(QStringLiteral("Search"));
        defaults.writeEntry("DefaultSearchScope", QStringLiteral("Indexed"));
        defaults.writeEntry("DefaultSearchField", QStringLiteral("Contents"));
        QVERIFY(defaults.sync());
        SearchWidget indexed(QUrl::fromLocalFile(root.path()), nullptr, config);
        auto *scope = indexed.findChild<QComboBox *>(QStringLiteral("searchScope"));
        auto *field = indexed.findChild<QComboBox *>(QStringLiteral("searchField"));
        QVERIFY(scope && field);
        QCOMPARE(scope->currentIndex(), 1);
        QCOMPARE(field->currentIndex(), 1);
        QVERIFY(field->currentText().contains(QStringLiteral("Indexed")));
        QVERIFY(indexed.findChild<QLabel *>(QStringLiteral("searchStatus"))->text().contains(QStringLiteral("Baloo")));
        defaults.writeEntry("DefaultSearchScope", QStringLiteral("SearchFolders"));
        defaults.writeEntry("DefaultSearchField", QStringLiteral("Query"));
        QVERIFY(defaults.sync());
        SearchWidget folders(QUrl::fromLocalFile(root.path()), nullptr, config);
        QCOMPARE(folders.findChild<QComboBox *>(QStringLiteral("searchScope"))->currentIndex(), 2);
        QCOMPARE(folders.findChild<QComboBox *>(QStringLiteral("searchField"))->currentIndex(), 2);
        defaults.deleteEntry("DefaultSearchScope");
        defaults.deleteEntry("DefaultSearchField");
        QVERIFY(defaults.sync());
        SearchWidget live(QUrl::fromLocalFile(root.path()), nullptr, config);
        QCOMPARE(live.findChild<QComboBox *>(QStringLiteral("searchScope"))->currentIndex(), 0);
        QCOMPARE(live.findChild<QComboBox *>(QStringLiteral("searchField"))->currentIndex(), 0);
    }

    void readsAndMigratesLegacySearchDefaults()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const auto config = KSharedConfig::openConfig(root.filePath(QStringLiteral("searchrc")), KConfig::SimpleConfig);
        auto defaults = config->group(QStringLiteral("Search"));
        defaults.writeEntry("SearchIndexedByDefault", true);
        defaults.writeEntry("SearchContentsByDefault", true);
        QCOMPARE(SearchDefaults::read(defaults).scope, SearchDefaults::Scope::Indexed);
        QCOMPARE(SearchDefaults::read(defaults).field, LiveSearch::Field::Contents);
        SearchWidget legacy(QUrl::fromLocalFile(root.path()), nullptr, config);
        QCOMPARE(legacy.findChild<QComboBox *>(QStringLiteral("searchScope"))->currentIndex(), 1);
        QCOMPARE(legacy.findChild<QComboBox *>(QStringLiteral("searchField"))->currentIndex(), 1);

        QVERIFY(SearchDefaults::migrate(defaults));
        QVERIFY(!defaults.hasKey("SearchIndexedByDefault"));
        QVERIFY(!defaults.hasKey("SearchContentsByDefault"));
        QCOMPARE(defaults.readEntry("DefaultSearchScope", QString()), QStringLiteral("Indexed"));
        QCOMPARE(defaults.readEntry("DefaultSearchField", QString()), QStringLiteral("Contents"));
        QVERIFY(!SearchDefaults::migrate(defaults));

        // A newer value wins over a leftover legacy key.
        defaults.writeEntry("DefaultSearchScope", QStringLiteral("SearchFolders"));
        defaults.writeEntry("SearchIndexedByDefault", true);
        QVERIFY(SearchDefaults::migrate(defaults));
        QCOMPARE(SearchDefaults::read(defaults).scope, SearchDefaults::Scope::SearchFolders);
        // Unknown values fall back to the defaults; indexes are accepted like KConfigSkeleton.
        defaults.writeEntry("DefaultSearchScope", QStringLiteral("Elsewhere"));
        defaults.writeEntry("DefaultSearchField", 2);
        QCOMPARE(SearchDefaults::read(defaults).scope, SearchDefaults::Scope::CurrentFolder);
        QCOMPARE(SearchDefaults::read(defaults).field, LiveSearch::Field::Query);
    }

    void appliesDefaultPathFilterAndKeepsChangedCriteria()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const auto config = KSharedConfig::openConfig(root.filePath(QStringLiteral("searchrc")), KConfig::SimpleConfig);
        QString error;
        PathFilter reports;
        reports.pathText = QStringLiteral("reports");
        reports.maxDepth = 2;
        PathFilter drafts;
        drafts.parentText = QStringLiteral("drafts");
        QVERIFY(SavedPathFilters::save(config->group(QStringLiteral("SavedPathFilters")),
                                       {{QStringLiteral("Reports"), reports}, {QStringLiteral("Drafts"), drafts}},
                                       &error));
        auto defaults = config->group(QStringLiteral("Search"));
        defaults.writeEntry("DefaultSearchPathFilter", QStringLiteral("Reports"));
        QVERIFY(defaults.sync());
        SearchWidget search(QUrl::fromLocalFile(root.path()), nullptr, config);
        auto *path = search.findChild<QLineEdit *>(QStringLiteral("searchPathFilter"));
        auto *parent = search.findChild<QLineEdit *>(QStringLiteral("searchParentFilter"));
        auto *depth = search.findChild<QSpinBox *>(QStringLiteral("searchMaxDepth"));
        auto *scope = search.findChild<QComboBox *>(QStringLiteral("searchScope"));
        auto *field = search.findChild<QComboBox *>(QStringLiteral("searchField"));
        auto *status = search.findChild<QLabel *>(QStringLiteral("searchStatus"));
        QVERIFY(path && parent && depth && scope && field && status);
        QCOMPARE(path->text(), QStringLiteral("reports"));
        QCOMPARE(depth->value(), 2);

        // Unchanged criteria follow new defaults; the scope the user chose is kept.
        scope->setCurrentIndex(2);
        defaults.writeEntry("DefaultSearchScope", QStringLiteral("Indexed"));
        defaults.writeEntry("DefaultSearchField", QStringLiteral("Query"));
        defaults.writeEntry("DefaultSearchPathFilter", QStringLiteral("Drafts"));
        search.applyDefaults();
        QCOMPARE(scope->currentIndex(), 2);
        QCOMPARE(field->currentIndex(), 2);
        QCOMPARE(path->text(), QString());
        QCOMPARE(parent->text(), QStringLiteral("drafts"));
        QCOMPARE(depth->value(), -1);

        // An edited filter is kept, and a missing default filter is reported.
        path->setText(QStringLiteral("mine"));
        defaults.writeEntry("DefaultSearchPathFilter", QStringLiteral("Removed"));
        search.applyDefaults();
        QCOMPARE(path->text(), QStringLiteral("mine"));
        QVERIFY(status->text().contains(QStringLiteral("Removed")));
        SearchWidget missing(QUrl::fromLocalFile(root.path()), nullptr, config);
        QCOMPARE(missing.findChild<QLineEdit *>(QStringLiteral("searchPathFilter"))->text(), QString());
        QVERIFY(missing.findChild<QLabel *>(QStringLiteral("searchStatus"))->text().contains(QStringLiteral("Removed")));
    }

    void runsSavedSearchesByName()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QFile file(root.filePath(QStringLiteral("needle.txt")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
        const auto config = KSharedConfig::openConfig(root.filePath(QStringLiteral("searchrc")), KConfig::SimpleConfig);
        SearchWidget search(QUrl::fromLocalFile(QStringLiteral("/")), nullptr, config);
        QVERIFY(search.savedSearchNames().isEmpty());
        QString error;
        SavedSearches::Search saved;
        saved.name = QStringLiteral("Needles & pins");
        saved.text = QStringLiteral("needle");
        saved.folder = QUrl::fromLocalFile(root.path());
        QVERIFY(SavedSearches::save(config->group(QStringLiteral("SavedSearches")), {saved}, &error));
        QCOMPARE(search.savedSearchNames(), QStringList{saved.name});
        QCOMPARE(search.findChild<QComboBox *>(QStringLiteral("searchSaved"))->count(), 1);
        QVERIFY(!search.runSavedSearch(QStringLiteral("Missing")));
        QVERIFY(search.findChild<QLabel *>(QStringLiteral("searchStatus"))->text().contains(QStringLiteral("no longer exists")));
        QVERIFY(search.runSavedSearch(saved.name));
        QCOMPARE(search.folder(), saved.folder);
        QCOMPARE(search.findChild<QLineEdit *>(QStringLiteral("searchQuery"))->text(), QStringLiteral("needle"));
        QTRY_VERIFY(!search.isRunning());
        auto *results = search.findChild<QTreeWidget *>(QStringLiteral("searchResults"));
        // The results arrive in a queued slot after the worker finishes.
        QTRY_COMPARE(results->topLevelItemCount(), 1);
        QCOMPARE(results->topLevelItem(0)->text(0), file.fileName());
    }

    void presentsResultsByDefaultAndKeepsFullPaths()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QFile file(root.filePath(QStringLiteral("needle.txt")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
        const auto config = KSharedConfig::openConfig(root.filePath(QStringLiteral("searchrc")), KConfig::SimpleConfig);
        auto defaults = config->group(QStringLiteral("Search"));
        defaults.writeEntry("DefaultSearchResultPresentation", QStringLiteral("NamesAndFolders"));
        QVERIFY(defaults.sync());
        QCOMPARE(SearchDefaults::read(defaults).presentation, SearchDefaults::Presentation::NamesAndFolders);
        SearchWidget search(QUrl::fromLocalFile(root.path()), nullptr, config);
        auto *presentation = search.findChild<QComboBox *>(QStringLiteral("searchPresentation"));
        auto *results = search.findChild<QTreeWidget *>(QStringLiteral("searchResults"));
        auto *collection = search.findChild<QTreeWidget *>(QStringLiteral("searchCollection"));
        QVERIFY(presentation && results && collection);
        QCOMPARE(presentation->currentIndex(), int(SearchDefaults::Presentation::NamesAndFolders));
        QCOMPARE(results->columnCount(), 2);

        search.findChild<QLineEdit *>(QStringLiteral("searchQuery"))->setText(QStringLiteral("needle"));
        search.findChild<QPushButton *>(QStringLiteral("searchStart"))->click();
        QTRY_VERIFY(search.findChild<QPushButton *>(QStringLiteral("searchStart"))->isEnabled());
        QCOMPARE(results->topLevelItemCount(), 1);
        QCOMPARE(results->topLevelItem(0)->text(0), QStringLiteral("needle.txt"));
        QCOMPARE(results->topLevelItem(0)->text(1), QFileInfo(file.fileName()).path());
        QCOMPARE(results->topLevelItem(0)->toolTip(0), file.fileName());

        // Actions keep using the full path.
        results->setCurrentItem(results->topLevelItem(0));
        search.findChild<QPushButton *>(QStringLiteral("searchCopy"))->click();
        QCOMPARE(QApplication::clipboard()->text(), QUrl::fromLocalFile(file.fileName()).toString(QUrl::FullyEncoded));
        QSignalSpy reveal(&search, &SearchWidget::revealRequested);
        search.findChild<QPushButton *>(QStringLiteral("searchReveal"))->click();
        QCOMPARE(reveal.size(), 1);
        QCOMPARE(reveal.first().first().toUrl(), QUrl::fromLocalFile(file.fileName()));
        search.findChild<QPushButton *>(QStringLiteral("searchCollect"))->click();
        QCOMPARE(collection->topLevelItemCount(), 1);
        QCOMPARE(collection->topLevelItem(0)->text(0), QStringLiteral("needle.txt"));

        // Switching presentation relists the existing items.
        presentation->setCurrentIndex(int(SearchDefaults::Presentation::FullPaths));
        QCOMPARE(results->columnCount(), 1);
        QCOMPARE(results->topLevelItem(0)->text(0), file.fileName());
        QCOMPARE(collection->topLevelItem(0)->text(0), file.fileName());

        // A changed choice is kept when the default changes; an unchanged one follows it.
        search.applyDefaults();
        QCOMPARE(presentation->currentIndex(), int(SearchDefaults::Presentation::FullPaths));
        SearchWidget fresh(QUrl::fromLocalFile(root.path()), nullptr, config);
        auto *freshPresentation = fresh.findChild<QComboBox *>(QStringLiteral("searchPresentation"));
        defaults.writeEntry("DefaultSearchResultPresentation", QStringLiteral("FullPaths"));
        fresh.applyDefaults();
        QCOMPARE(freshPresentation->currentIndex(), int(SearchDefaults::Presentation::FullPaths));
    }

    void retainsAndClearsSearchHistory()
    {
        auto historyGroup = KSharedConfig::openStateConfig()->group(QStringLiteral("SearchHistory"));
        historyGroup.deleteGroup();
        QVERIFY(historyGroup.sync());
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const auto config = KSharedConfig::openConfig(root.filePath(QStringLiteral("searchrc")), KConfig::SimpleConfig);
        SearchWidget search(QUrl::fromLocalFile(root.path()), nullptr, config);
        auto *query = search.findChild<QLineEdit *>(QStringLiteral("searchQuery"));
        auto *status = search.findChild<QLabel *>(QStringLiteral("searchStatus"));
        QVERIFY(query && status && query->completer());
        QVERIFY(search.history().isEmpty());
        for (const QString &text : {QStringLiteral("alpha"), QStringLiteral("beta"), QStringLiteral(" alpha ")}) {
            query->setText(text);
            search.findChild<QPushButton *>(QStringLiteral("searchStart"))->click();
            QTRY_VERIFY(search.findChild<QPushButton *>(QStringLiteral("searchStart"))->isEnabled());
        }
        QCOMPARE(search.history(), (QStringList{QStringLiteral("alpha"), QStringLiteral("beta")}));
        query->clear();
        search.findChild<QPushButton *>(QStringLiteral("searchStart"))->click();
        QVERIFY(status->text().contains(QStringLiteral("Enter text")));
        QCOMPARE(search.history().size(), 2);

        // Another widget sees the shared history and lists it with Alt+Down.
        SearchWidget other(QUrl::fromLocalFile(root.path()), nullptr, config);
        auto *otherQuery = other.findChild<QLineEdit *>(QStringLiteral("searchQuery"));
        QCOMPARE(otherQuery->completer()->model()->rowCount(), 2);
        other.show();
        QVERIFY(QTest::qWaitForWindowExposed(&other));
        otherQuery->setFocus();
        QTest::keyClick(otherQuery, Qt::Key_Down, Qt::AltModifier);
        QTRY_VERIFY(otherQuery->completer()->popup()->isVisible());
        QCOMPARE(otherQuery->completer()->completionCount(), 2);
        otherQuery->completer()->popup()->hide();

        // Turning retention off discards the history and stops recording.
        auto defaults = config->group(QStringLiteral("Search"));
        defaults.writeEntry("RetainSearchHistory", false);
        QVERIFY(!SearchHistory::retained(defaults));
        search.applyDefaults();
        QVERIFY(SearchHistory::load(KSharedConfig::openStateConfig()->group(QStringLiteral("SearchHistory"))).isEmpty());
        QCOMPARE(query->completer()->model()->rowCount(), 0);
        query->setText(QStringLiteral("gamma"));
        search.findChild<QPushButton *>(QStringLiteral("searchStart"))->click();
        QTRY_VERIFY(search.findChild<QPushButton *>(QStringLiteral("searchStart"))->isEnabled());
        QVERIFY(search.history().isEmpty());
        query->setFocus();
        QTest::keyClick(query, Qt::Key_Down, Qt::AltModifier);
        QVERIFY(status->text().contains(QStringLiteral("not remembered")));
        defaults.deleteEntry("RetainSearchHistory");
    }

    void usesApplicationSettings()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        Settings::setDefaultSearchScope(Settings::EnumDefaultSearchScope::Indexed);
        Settings::setDefaultSearchField(Settings::EnumDefaultSearchField::Contents);
        Settings::self()->save();
        SearchWidget dialog(QUrl::fromLocalFile(root.path()));
        QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("searchScope"))->currentIndex(), 1);
        QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("searchField"))->currentIndex(), 1);
        Settings::setDefaultSearchScope(Settings::EnumDefaultSearchScope::CurrentFolder);
        Settings::setDefaultSearchField(Settings::EnumDefaultSearchField::FileName);
        Settings::self()->save();
    }

    void loadsSharedPathFilter()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("parent/excluded")));
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("parent/deep/nested")));
        for (const QString &name : {QStringLiteral("parent/report.txt"),
                                    QStringLiteral("parent/excluded/report.txt"),
                                    QStringLiteral("parent/deep/nested/report.txt"),
                                    QStringLiteral("parent/other.txt")}) {
            QFile file(root.filePath(name));
            QVERIFY(file.open(QIODevice::WriteOnly));
        }
        const auto config = KSharedConfig::openConfig(root.filePath(QStringLiteral("searchrc")), KConfig::SimpleConfig);
        QString error;
        const PathFilter filter{QStringLiteral("report"), QStringLiteral("parent"), {root.filePath(QStringLiteral("parent/excluded"))}, 1};
        QVERIFY(SavedPathFilters::save(config->group(QStringLiteral("SavedPathFilters")), {{QStringLiteral("Work"), filter}}, &error));
        SearchWidget dialog(QUrl::fromLocalFile(root.path()), nullptr, config);
        QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("searchFilterPresets"))->currentText(), QStringLiteral("Work"));
        dialog.findChild<QPushButton *>(QStringLiteral("searchLoadFilterPreset"))->click();
        QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("searchPathFilter"))->text(), filter.pathText);
        QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("searchParentFilter"))->text(), filter.parentText);
        QCOMPARE(dialog.findChild<QPlainTextEdit *>(QStringLiteral("searchExcludedFolders"))->toPlainText(), filter.excludedFolders.first());
        QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("searchMaxDepth"))->value(), 1);
        dialog.findChild<QLineEdit *>(QStringLiteral("searchQuery"))->setText(QStringLiteral(".txt"));
        auto *start = dialog.findChild<QPushButton *>(QStringLiteral("searchStart"));
        start->click();
        QTRY_VERIFY(start->isEnabled());
        auto *results = dialog.findChild<QTreeWidget *>(QStringLiteral("searchResults"));
        QCOMPARE(results->topLevelItemCount(), 1);
        QCOMPARE(results->topLevelItem(0)->text(0), root.filePath(QStringLiteral("parent/report.txt")));
        QVERIFY(SavedPathFilters::save(config->group(QStringLiteral("SavedPathFilters")), {}, &error));
        dialog.findChild<QPushButton *>(QStringLiteral("searchLoadFilterPreset"))->click();
        QVERIFY(dialog.findChild<QLabel *>(QStringLiteral("searchFilterPresetsStatus"))->text().contains(QStringLiteral("no longer exists")));
        dialog.findChild<QPushButton *>(QStringLiteral("searchRefreshFilterPresets"))->click();
        QVERIFY(!dialog.findChild<QPushButton *>(QStringLiteral("searchLoadFilterPreset"))->isEnabled());
    }

    void searchesAndReportsFailures()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QFile file(root.filePath(QStringLiteral("example.txt")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write("matching content"), 16);
        file.close();
        SearchWidget dialog(QUrl::fromLocalFile(root.path()));
        auto *query = dialog.findChild<QLineEdit *>(QStringLiteral("searchQuery"));
        auto *field = dialog.findChild<QComboBox *>(QStringLiteral("searchField"));
        auto *start = dialog.findChild<QPushButton *>(QStringLiteral("searchStart"));
        auto *results = dialog.findChild<QTreeWidget *>(QStringLiteral("searchResults"));
        auto *status = dialog.findChild<QLabel *>(QStringLiteral("searchStatus"));
        QVERIFY(query && field && start && results && status);
        start->click();
        QVERIFY(status->text().contains(QStringLiteral("Enter text")));
        query->setText(QStringLiteral("example"));
        start->click();
        QTRY_VERIFY(start->isEnabled());
        QVERIFY(dialog.findChild<QProgressBar *>(QStringLiteral("searchProgress"))->isHidden());
        QCOMPARE(results->topLevelItemCount(), 1);
        QCOMPARE(results->topLevelItem(0)->text(0), file.fileName());
        results->setCurrentItem(results->topLevelItem(0));
        dialog.findChild<QPushButton *>(QStringLiteral("searchCopy"))->click();
        QCOMPARE(QApplication::clipboard()->text(), QUrl::fromLocalFile(file.fileName()).toString(QUrl::FullyEncoded));
        field->setCurrentIndex(1);
        query->setText(QStringLiteral("content"));
        start->click();
        QTRY_VERIFY(start->isEnabled());
        QCOMPARE(results->topLevelItemCount(), 1);
        QCOMPARE(results->topLevelItem(0)->text(0), file.fileName());
    }

    void runsAdvancedQueries()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        for (const QString &name : {QStringLiteral("report.pdf"), QStringLiteral("report.txt")}) {
            QFile file(root.filePath(name));
            QVERIFY(file.open(QIODevice::WriteOnly));
        }
        SearchWidget dialog(QUrl::fromLocalFile(root.path()));
        auto *query = dialog.findChild<QLineEdit *>(QStringLiteral("searchQuery"));
        auto *field = dialog.findChild<QComboBox *>(QStringLiteral("searchField"));
        auto *start = dialog.findChild<QPushButton *>(QStringLiteral("searchStart"));
        auto *results = dialog.findChild<QTreeWidget *>(QStringLiteral("searchResults"));
        auto *status = dialog.findChild<QLabel *>(QStringLiteral("searchStatus"));
        QVERIFY(query && field && start && results && status);
        QCOMPARE(field->count(), 3);
        field->setCurrentIndex(2);
        query->setFocus();
        QTest::keyClicks(query, QStringLiteral("report OR"));
        QVERIFY(status->text().contains(QStringLiteral("after OR")));
        start->click();
        QVERIFY(start->isEnabled());
        QVERIFY(status->text().contains(QStringLiteral("after OR")));
        QCOMPARE(results->topLevelItemCount(), 0);
        query->clear();
        QTest::keyClicks(query, QStringLiteral("report -type:txt"));
        QVERIFY(status->text().contains(QStringLiteral("(name:\"report\" AND NOT type:\"txt\")")));
        start->click();
        QTRY_VERIFY(start->isEnabled());
        QCOMPARE(results->topLevelItemCount(), 1);
        QCOMPARE(results->topLevelItem(0)->text(0), root.filePath(QStringLiteral("report.pdf")));
    }

    void widensTagSearchesToTheLibrary()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("sub")));
        for (const QString &name : {QStringLiteral("top.txt"), QStringLiteral("sub/inner.txt")}) {
            QFile file(root.filePath(name));
            QVERIFY(file.open(QIODevice::WriteOnly));
        }
        QCOMPARE(LibraryRegistry::create(root.path()), QString());
        const auto restore = qScopeGuard([&root] {
            LibraryRegistry::remove(root.path());
            Settings::setTagSearchScope(Settings::EnumTagSearchScope::CurrentFolder);
            Settings::self()->save();
        });
        SearchWidget dialog(QUrl::fromLocalFile(root.filePath(QStringLiteral("sub"))));
        auto *query = dialog.findChild<QLineEdit *>(QStringLiteral("searchQuery"));
        auto *field = dialog.findChild<QComboBox *>(QStringLiteral("searchField"));
        auto *start = dialog.findChild<QPushButton *>(QStringLiteral("searchStart"));
        auto *results = dialog.findChild<QTreeWidget *>(QStringLiteral("searchResults"));
        auto *status = dialog.findChild<QLabel *>(QStringLiteral("searchStatus"));
        QVERIFY(query && field && start && results && status);
        field->setCurrentIndex(2);
        const auto found = [results] {
            QStringList paths;
            for (int row = 0; row < results->topLevelItemCount(); ++row) {
                paths.append(results->topLevelItem(row)->text(0));
            }
            paths.sort();
            return paths;
        };
        const QString inner = root.filePath(QStringLiteral("sub/inner.txt"));

        // By default, tag searches stay in the current folder.
        query->setText(QStringLiteral("tagged:no type:file"));
        start->click();
        QTRY_VERIFY(start->isEnabled());
        QCOMPARE(found(), QStringList{inner});
        QVERIFY(!status->text().contains(QStringLiteral("whole tag library")));

        // With the library scope, they start in the library folder and say so.
        Settings::setTagSearchScope(Settings::EnumTagSearchScope::LibraryRoot);
        Settings::self()->save();
        start->click();
        QTRY_VERIFY(start->isEnabled());
        QCOMPARE(found(), (QStringList{inner, root.filePath(QStringLiteral("top.txt"))}));
        QVERIFY(status->text().contains(QStringLiteral("Searched the whole tag library in ") + root.path()));

        // Queries without tag criteria still search the current folder.
        query->setText(QStringLiteral("type:file"));
        start->click();
        QTRY_VERIFY(start->isEnabled());
        QCOMPARE(found(), QStringList{inner});
        QVERIFY(!status->text().contains(QStringLiteral("whole tag library")));
    }

    void searchesWorkspaceLocationsOrExplicitlyOutside()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const QStringList folders{QStringLiteral("work"), QStringLiteral("unassigned"), QStringLiteral("other")};
        for (const QString &folder : folders) {
            QVERIFY(QDir(root.path()).mkpath(folder));
            QFile file(root.filePath(folder + QStringLiteral("/needle.txt")));
            QVERIFY(file.open(QIODevice::WriteOnly));
        }
        const auto config = KSharedConfig::openConfig(root.filePath(QStringLiteral("searchrc")), KConfig::SimpleConfig);
        FavoritesModel favorites(config->group(QStringLiteral("Favorites")));
        KFilePlacesModel places;
        for (const QString &folder : folders) {
            QVERIFY(favorites.add(QUrl::fromLocalFile(root.filePath(folder))));
        }
        WorkspaceLocations locations(&favorites, &places);
        WorkspaceCatalog::Catalog catalog;
        QString error;
        QVERIFY(WorkspaceCatalog::create(catalog, QStringLiteral("Work"), QStringLiteral("W"), error));
        QVERIFY(WorkspaceCatalog::create(catalog, QStringLiteral("Other"), QStringLiteral("O"), error));
        QString active = catalog.workspaces.first().id;
        QVERIFY(WorkspaceCatalog::assignLocation(catalog, WorkspaceCatalog::locationId(QUrl::fromLocalFile(root.filePath(folders[0]))), active, error));
        QVERIFY(WorkspaceCatalog::assignLocation(catalog,
                                                 WorkspaceCatalog::locationId(QUrl::fromLocalFile(root.filePath(folders[2]))),
                                                 catalog.workspaces.last().id,
                                                 error));
        SearchWidget dialog(QUrl::fromLocalFile(root.path()), nullptr, config, [&] {
            SearchWidget::WorkspaceScope scope;
            scope.active = !active.isEmpty();
            for (const auto &location : locations.locations()) {
                if (!active.isEmpty() && catalog.locationAssignments.value(location.id) == active) {
                    scope.locations.append(location.url);
                }
            }
            return scope;
        });
        auto *workspaceOnly = dialog.findChild<QCheckBox *>(QStringLiteral("searchWorkspaceOnly"));
        QVERIFY(workspaceOnly);
        QVERIFY(workspaceOnly->isChecked());
        auto *start = dialog.findChild<QPushButton *>(QStringLiteral("searchStart"));
        auto *results = dialog.findChild<QTreeWidget *>(QStringLiteral("searchResults"));
        dialog.findChild<QLineEdit *>(QStringLiteral("searchQuery"))->setText(QStringLiteral("needle"));
        start->click();
        QTRY_VERIFY(start->isEnabled());
        QStringList paths;
        for (int i = 0; i < results->topLevelItemCount(); ++i) {
            paths.append(results->topLevelItem(i)->text(0));
        }
        paths.sort();
        QCOMPARE(paths, (QStringList{root.filePath(QStringLiteral("work/needle.txt"))}));
        workspaceOnly->setChecked(false);
        start->click();
        QTRY_VERIFY(start->isEnabled());
        QCOMPARE(results->topLevelItemCount(), 3);
        workspaceOnly->setChecked(true);
        active = catalog.workspaces.last().id;
        start->click();
        QTRY_VERIFY(start->isEnabled());
        QCOMPARE(results->topLevelItemCount(), 1);
        paths.clear();
        for (int i = 0; i < results->topLevelItemCount(); ++i) {
            paths.append(results->topLevelItem(i)->text(0));
        }
        QVERIFY(paths.contains(root.filePath(QStringLiteral("other/needle.txt"))));
        QVERIFY(!paths.contains(root.filePath(QStringLiteral("work/needle.txt"))));
        QVERIFY(!paths.contains(root.filePath(QStringLiteral("unassigned/needle.txt"))));
        active.clear();
        start->click();
        QTRY_VERIFY(start->isEnabled());
        QCOMPARE(results->topLevelItemCount(), 3);
    }

    void workspaceSearchDefaultFollowsPreference()
    {
        QTemporaryDir root;
        const auto config = KSharedConfig::openConfig(root.filePath(QStringLiteral("searchrc")), KConfig::SimpleConfig);
        const auto activeScope = [] {
            SearchWidget::WorkspaceScope scope;
            scope.active = true;
            return scope;
        };
        {
            SearchWidget dialog(QUrl::fromLocalFile(root.path()), nullptr, config, activeScope);
            auto *workspaceOnly = dialog.findChild<QCheckBox *>(QStringLiteral("searchWorkspaceOnly"));
            QVERIFY(workspaceOnly);
            QVERIFY(workspaceOnly->isChecked());
        }
        config->group(QStringLiteral("Search")).writeEntry("WorkspaceSearchByDefault", false);
        SearchWidget dialog(QUrl::fromLocalFile(root.path()), nullptr, config, activeScope);
        auto *workspaceOnly = dialog.findChild<QCheckBox *>(QStringLiteral("searchWorkspaceOnly"));
        auto *scope = dialog.findChild<QComboBox *>(QStringLiteral("searchScope"));
        QVERIFY(workspaceOnly && scope);
        QVERIFY(!workspaceOnly->isChecked());
        QCOMPARE(scope->itemText(0), QStringLiteral("Current folder (live)"));
        workspaceOnly->setChecked(true);
        QCOMPARE(scope->itemText(0), QStringLiteral("Workspace locations (live)"));
    }

    void workspaceSearchReportsUnsupportedLocations()
    {
        QTemporaryDir root;
        const auto config = KSharedConfig::openConfig(root.filePath(QStringLiteral("searchrc")), KConfig::SimpleConfig);
        QFile file(root.filePath(QStringLiteral("needle.txt")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QList<QUrl> locations{QUrl(QStringLiteral("sftp://user:secret@example.invalid/path")), QUrl::fromLocalFile(root.path())};
        SearchWidget dialog(QUrl::fromLocalFile(root.path()), nullptr, config, [&] {
            return SearchWidget::WorkspaceScope{true, locations};
        });
        dialog.findChild<QLineEdit *>(QStringLiteral("searchQuery"))->setText(QStringLiteral("needle"));
        auto *start = dialog.findChild<QPushButton *>(QStringLiteral("searchStart"));
        start->click();
        QTRY_VERIFY(start->isEnabled());
        QVERIFY(dialog.findChild<QLabel *>(QStringLiteral("searchStatus"))->text().contains(QStringLiteral("incomplete")));
        auto *errors = dialog.findChild<QTreeWidget *>(QStringLiteral("searchErrors"));
        QCOMPARE(errors->topLevelItemCount(), 1);
        QVERIFY(!errors->topLevelItem(0)->text(0).contains(QStringLiteral("secret")));
        QCOMPARE(dialog.findChild<QTreeWidget *>(QStringLiteral("searchResults"))->topLevelItemCount(), 1);
        locations.clear();
        start->click();
        QTRY_VERIFY(start->isEnabled());
        QCOMPARE(dialog.findChild<QTreeWidget *>(QStringLiteral("searchResults"))->topLevelItemCount(), 0);
        QCOMPARE(errors->topLevelItemCount(), 1);
        QVERIFY(errors->topLevelItem(0)->text(0).contains(QStringLiteral("no searchable")));
        locations = {QUrl::fromLocalFile(root.filePath(QStringLiteral("missing")))};
        start->click();
        QTRY_VERIFY(start->isEnabled());
        QCOMPARE(errors->topLevelItemCount(), 1);
        QVERIFY(errors->topLevelItem(0)->text(0).contains(QStringLiteral("existing local folder")));
    }

    void searchesSearchFoldersAndSkipsExclusions()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        for (const QString &folder : {QStringLiteral("docs/cache"), QStringLiteral("music"), QStringLiteral("elsewhere")}) {
            QVERIFY(QDir(root.path()).mkpath(folder));
            QFile file(root.filePath(folder + QStringLiteral("/needle.txt")));
            QVERIFY(file.open(QIODevice::WriteOnly));
        }
        QFile docs(root.filePath(QStringLiteral("docs/needle.txt")));
        QVERIFY(docs.open(QIODevice::WriteOnly));
        const auto config = KSharedConfig::openConfig(root.filePath(QStringLiteral("searchrc")), KConfig::SimpleConfig);
        SearchWidget dialog(QUrl::fromLocalFile(root.filePath(QStringLiteral("elsewhere"))), nullptr, config);
        auto *scope = dialog.findChild<QComboBox *>(QStringLiteral("searchScope"));
        QCOMPARE(scope->count(), 3);
        scope->setCurrentIndex(2);
        dialog.findChild<QLineEdit *>(QStringLiteral("searchQuery"))->setText(QStringLiteral("needle"));
        auto *start = dialog.findChild<QPushButton *>(QStringLiteral("searchStart"));
        auto *results = dialog.findChild<QTreeWidget *>(QStringLiteral("searchResults"));
        auto *errors = dialog.findChild<QTreeWidget *>(QStringLiteral("searchErrors"));
        start->click();
        QTRY_VERIFY(start->isEnabled());
        QCOMPARE(results->topLevelItemCount(), 0);
        QCOMPARE(errors->topLevelItemCount(), 1);
        QVERIFY(errors->topLevelItem(0)->text(0).contains(QStringLiteral("No search folders")));

        auto search = config->group(QStringLiteral("Search"));
        search.writeEntry("SearchFolders", QStringList{root.filePath(QStringLiteral("docs")), root.filePath(QStringLiteral("music"))});
        search.writeEntry("SearchExclusions", QStringList{root.filePath(QStringLiteral("docs/cache"))});
        start->click();
        QTRY_VERIFY(start->isEnabled());
        QStringList paths;
        for (int i = 0; i < results->topLevelItemCount(); ++i) {
            paths.append(results->topLevelItem(i)->text(0));
        }
        paths.sort();
        QCOMPARE(paths, (QStringList{root.filePath(QStringLiteral("docs/needle.txt")), root.filePath(QStringLiteral("music/needle.txt"))}));
        QCOMPARE(errors->topLevelItemCount(), 0);

        // Exclusions apply to the current folder scope too.
        search.writeEntry("SearchExclusions", QStringList{root.filePath(QStringLiteral("elsewhere"))});
        scope->setCurrentIndex(0);
        start->click();
        QTRY_VERIFY(start->isEnabled());
        QCOMPARE(results->topLevelItemCount(), 0);

        scope->setCurrentIndex(2);
        QTimer::singleShot(0, [] {
            auto *input = qobject_cast<QInputDialog *>(QApplication::activeModalWidget());
            QVERIFY(input);
            input->setTextValue(QStringLiteral("Folders"));
            input->accept();
        });
        dialog.findChild<QPushButton *>(QStringLiteral("searchSave"))->click();
        SearchWidget restored(QUrl::fromLocalFile(root.path()), nullptr, config);
        QCOMPARE(restored.findChild<QComboBox *>(QStringLiteral("searchScope"))->currentIndex(), 0);
        restored.findChild<QPushButton *>(QStringLiteral("searchLoad"))->click();
        QCOMPARE(restored.findChild<QComboBox *>(QStringLiteral("searchScope"))->currentIndex(), 2);
    }

    void mainWindowScopesSearchToWorkspace()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("work")));
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("other")));
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("unassigned")));
        for (const QString &folder : {QStringLiteral("work"), QStringLiteral("other"), QStringLiteral("unassigned")}) {
            QFile file(root.filePath(folder + QStringLiteral("/needle.txt")));
            QVERIFY(file.open(QIODevice::WriteOnly));
        }
        const auto config = KSharedConfig::openConfig();
        const auto backup = KSharedConfig::openConfig(root.filePath(QStringLiteral("backup")), KConfig::SimpleConfig);
        auto favorites = config->group(QStringLiteral("Favorites"));
        auto workspaces = config->group(QStringLiteral("Workspaces"));
        auto savedFavorites = backup->group(QStringLiteral("Favorites"));
        auto savedWorkspaces = backup->group(QStringLiteral("Workspaces"));
        favorites.copyTo(&savedFavorites);
        workspaces.copyTo(&savedWorkspaces);
        favorites.deleteGroup();
        workspaces.deleteGroup();
        const auto restore = qScopeGuard([config, savedFavorites, savedWorkspaces]() mutable {
            config->reparseConfiguration();
            auto favorites = config->group(QStringLiteral("Favorites"));
            auto workspaces = config->group(QStringLiteral("Workspaces"));
            favorites.deleteGroup();
            workspaces.deleteGroup();
            savedFavorites.copyTo(&favorites);
            savedWorkspaces.copyTo(&workspaces);
            config->sync();
        });
        FavoritesModel model(favorites);
        QVERIFY(model.add(QUrl::fromLocalFile(root.filePath(QStringLiteral("work")))));
        QVERIFY(model.add(QUrl::fromLocalFile(root.filePath(QStringLiteral("other")))));
        QVERIFY(model.add(QUrl::fromLocalFile(root.filePath(QStringLiteral("unassigned")))));
        WorkspaceCatalog::Catalog catalog;
        QString error;
        QVERIFY(WorkspaceCatalog::create(catalog, QStringLiteral("Work"), QStringLiteral("W"), error));
        QVERIFY(WorkspaceCatalog::create(catalog, QStringLiteral("Other"), QStringLiteral("O"), error));
        QVERIFY(WorkspaceCatalog::create(catalog, QStringLiteral("Empty"), QStringLiteral("E"), error));
        QVERIFY(WorkspaceCatalog::activate(catalog, catalog.workspaces.first().id, error));
        QVERIFY(WorkspaceCatalog::assignLocation(catalog,
                                                 WorkspaceCatalog::locationId(QUrl::fromLocalFile(root.filePath(QStringLiteral("work")))),
                                                 catalog.workspaces.first().id,
                                                 error));
        QVERIFY(WorkspaceCatalog::assignLocation(catalog,
                                                 WorkspaceCatalog::locationId(QUrl::fromLocalFile(root.filePath(QStringLiteral("other")))),
                                                 catalog.workspaces.at(1).id,
                                                 error));
        QVERIFY(WorkspaceCatalog::save(workspaces, catalog, error));
        QVERIFY(config->sync());
        for (int workspace = 0; workspace < 4; ++workspace) {
            if (workspace == 3) {
                catalog.activeId.clear();
            } else {
                QVERIFY(WorkspaceCatalog::activate(catalog, catalog.workspaces.at(workspace).id, error));
            }
            QVERIFY(WorkspaceCatalog::save(workspaces, catalog, error));
            QVERIFY(config->sync());
            MainWindow window;
            window.openUrls({QUrl::fromLocalFile(root.path())});
            Pane *activePane = window.activePane();
            Pane *inactivePane = window.inactivePane();
            inactivePane->setUrl(QUrl::fromLocalFile(root.filePath(QStringLiteral("other"))));
            QTRY_COMPARE(inactivePane->url(), QUrl::fromLocalFile(root.filePath(QStringLiteral("other"))));
            const QList<QAction *> entries{window.actionCollection()->action(QStringLiteral("search_folder")),
                                           activePane->toolbarAction(QStringLiteral("paneSearch")),
                                           inactivePane->toolbarAction(QStringLiteral("paneSearch"))};
            for (int entry = 0; entry < entries.size(); ++entry) {
                QVERIFY(entries[entry]);
                entries[entry]->trigger();
                auto *dialog = window.findChild<SearchWidget *>(QStringLiteral("searchWidget"));
                QVERIFY(dialog);
                auto *workspaceOnly = dialog->findChild<QCheckBox *>(QStringLiteral("searchWorkspaceOnly"));
                QVERIFY(workspaceOnly && workspaceOnly->isChecked());
                QCOMPARE(workspaceOnly->isHidden(), workspace == 3);
                dialog->findChild<QComboBox *>(QStringLiteral("searchScope"))->setCurrentIndex(0);
                dialog->findChild<QLineEdit *>(QStringLiteral("searchQuery"))->setText(QStringLiteral("needle"));
                auto *start = dialog->findChild<QPushButton *>(QStringLiteral("searchStart"));
                start->click();
                QTRY_VERIFY(start->isEnabled());
                auto *results = dialog->findChild<QTreeWidget *>(QStringLiteral("searchResults"));
                const int broaderCount = entry == 2 ? 1 : 3;
                QCOMPARE(results->topLevelItemCount(), workspace == 3 ? broaderCount : workspace == 2 ? 0 : 1);
                if (workspace < 2) {
                    const QString folder = workspace == 0 ? QStringLiteral("work") : QStringLiteral("other");
                    QCOMPARE(results->topLevelItem(0)->text(0), root.filePath(folder + QStringLiteral("/needle.txt")));
                } else if (workspace == 2) {
                    auto *errors = dialog->findChild<QTreeWidget *>(QStringLiteral("searchErrors"));
                    QCOMPARE(errors->topLevelItemCount(), 1);
                    QVERIFY(errors->topLevelItem(0)->text(0).contains(QStringLiteral("no searchable")));
                }
                if (workspace != 3) {
                    workspaceOnly->setChecked(false);
                    start->click();
                    QTRY_VERIFY(start->isEnabled());
                    QCOMPARE(results->topLevelItemCount(), broaderCount);
                    workspaceOnly->setChecked(true);
                }
            }
        }
    }

    void refusesRemoteFolders()
    {
        SearchWidget dialog(QUrl(QStringLiteral("sftp://example.invalid/path")));
        dialog.findChild<QLineEdit *>(QStringLiteral("searchQuery"))->setText(QStringLiteral("x"));
        dialog.findChild<QPushButton *>(QStringLiteral("searchStart"))->click();
        QVERIFY(dialog.findChild<QLabel *>(QStringLiteral("searchStatus"))->text().contains(QStringLiteral("local folders only")));
    }

    void indexedScopeReportsInvalidQueries()
    {
        SearchWidget dialog(QUrl(QStringLiteral("sftp://example.invalid/path")));
        auto *scope = dialog.findChild<QComboBox *>(QStringLiteral("searchScope"));
        auto *start = dialog.findChild<QPushButton *>(QStringLiteral("searchStart"));
        auto *errors = dialog.findChild<QTreeWidget *>(QStringLiteral("searchErrors"));
        QVERIFY(scope && start && errors);
        scope->setCurrentIndex(1);
        dialog.findChild<QLineEdit *>(QStringLiteral("searchQuery"))->setText(QStringLiteral("a\"b"));
        start->click();
        QTRY_VERIFY(start->isEnabled());
        QCOMPARE(errors->topLevelItemCount(), 1);
        QVERIFY(errors->topLevelItem(0)->text(0).contains(QStringLiteral("double quotes")));
        QVERIFY(dialog.findChild<QLabel *>(QStringLiteral("searchStatus"))->text().contains(QStringLiteral("incomplete")));
        dialog.findChild<QSpinBox *>(QStringLiteral("searchMaxDepth"))->setValue(0);
        dialog.findChild<QLineEdit *>(QStringLiteral("searchQuery"))->setText(QStringLiteral("term"));
        start->click();
        QVERIFY(dialog.findChild<QLabel *>(QStringLiteral("searchStatus"))->text().contains(QStringLiteral("local current folder")));
    }

    void ranksResultsBySimilarity()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const QStringList names{QStringLiteral("misreported.txt"),
                                QStringLiteral("annual-report.txt"),
                                QStringLiteral("report.txt"),
                                QStringLiteral("reports.txt")};
        for (const QString &name : names) {
            QFile file(root.filePath(name));
            QVERIFY(file.open(QIODevice::WriteOnly));
        }
        const auto config = KSharedConfig::openConfig(root.filePath(QStringLiteral("searchrc")), KConfig::SimpleConfig);
        QStringList ranked;
        QStringList unranked;
        QString rankedStatus;
        QString unrankedStatus;
        for (const bool rank : {true, false}) {
            SearchWidget widget(QUrl::fromLocalFile(root.path()), nullptr, config);
            auto *check = widget.findChild<QCheckBox *>(QStringLiteral("searchRank"));
            auto *query = widget.findChild<QLineEdit *>(QStringLiteral("searchQuery"));
            auto *start = widget.findChild<QPushButton *>(QStringLiteral("searchStart"));
            auto *results = widget.findChild<QTreeWidget *>(QStringLiteral("searchResults"));
            auto *status = widget.findChild<QLabel *>(QStringLiteral("searchStatus"));
            QVERIFY(check && query && start && results && status);
            // The choice persists between panels.
            QCOMPARE(check->isChecked(), !rank);
            check->setChecked(rank);
            query->setText(QStringLiteral("report"));
            start->click();
            QTRY_VERIFY_WITH_TIMEOUT(start->isEnabled(), 5000);
            QStringList &paths = rank ? ranked : unranked;
            for (int i = 0; i < results->topLevelItemCount(); ++i) {
                paths.append(QFileInfo(results->topLevelItem(i)->data(0, Qt::UserRole).toString()).fileName());
            }
            (rank ? rankedStatus : unrankedStatus) = status->text();
            QCOMPARE(config->group(QStringLiteral("Search")).readEntry("RankResults", !rank), rank);
        }
        QCOMPARE(
            ranked,
            (QStringList{QStringLiteral("report.txt"), QStringLiteral("reports.txt"), QStringLiteral("annual-report.txt"), QStringLiteral("misreported.txt")}));
        QVERIFY(rankedStatus.contains(QStringLiteral("ranked by name similarity")));
        // Clearing it keeps the order in which results were found.
        QCOMPARE(unranked.size(), 4);
        QVERIFY(!unrankedStatus.contains(QStringLiteral("ranked")));
    }

    void offersLiveFallbackAfterIndexedSearch()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QFile file(root.filePath(QStringLiteral("fallbackneedle.txt")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
        const auto config = KSharedConfig::openConfig(root.filePath(QStringLiteral("searchrc")), KConfig::SimpleConfig);
        SearchWidget widget(QUrl::fromLocalFile(root.path()), nullptr, config);
        widget.show();
        auto *scope = widget.findChild<QComboBox *>(QStringLiteral("searchScope"));
        auto *query = widget.findChild<QLineEdit *>(QStringLiteral("searchQuery"));
        auto *start = widget.findChild<QPushButton *>(QStringLiteral("searchStart"));
        auto *results = widget.findChild<QTreeWidget *>(QStringLiteral("searchResults"));
        auto *fallback = widget.findChild<QPushButton *>(QStringLiteral("searchLiveFallback"));
        QVERIFY(scope && query && start && results && fallback);
        QVERIFY(!fallback->isVisible());
        query->setText(QStringLiteral("fallbackneedle"));
        start->click();
        QTRY_VERIFY(start->isEnabled());
        QVERIFY(!fallback->isVisible());

        scope->setCurrentIndex(1);
        start->click();
        QTRY_VERIFY_WITH_TIMEOUT(start->isEnabled(), 10000);
        QVERIFY(fallback->isVisible());
        QCOMPARE(KLocalizedString::removeAcceleratorMarker(fallback->text()), QStringLiteral("Search Current Folder Live"));
        scope->setCurrentIndex(2);
        QVERIFY(!fallback->isVisible());
        scope->setCurrentIndex(1);
        start->click();
        QTRY_VERIFY_WITH_TIMEOUT(start->isEnabled(), 10000);
        QVERIFY(fallback->isVisible());

        fallback->click();
        QCOMPARE(scope->currentIndex(), 0);
        QVERIFY(!fallback->isVisible());
        QTRY_VERIFY(start->isEnabled());
        QCOMPARE(results->topLevelItemCount(), 1);
        QCOMPARE(results->topLevelItem(0)->data(0, Qt::UserRole).toString(), file.fileName());
        QVERIFY(!fallback->isVisible());
    }

    void honoursTheOfferedSearchExtras()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        for (const QString &name : {QStringLiteral("misreported.txt"), QStringLiteral("report.txt")}) {
            QFile file(root.filePath(name));
            QVERIFY(file.open(QIODevice::WriteOnly));
        }
        const auto config = KSharedConfig::openConfig(root.filePath(QStringLiteral("searchrc")), KConfig::SimpleConfig);
        KConfigGroup search = config->group(QStringLiteral("Search"));
        QVERIFY(SearchDefaults::read(search).offerRanking);
        QVERIFY(SearchDefaults::read(search).offerLiveFallback);
        search.writeEntry("RankResults", true);
        search.writeEntry("SearchOfferRanking", false);
        search.writeEntry("SearchOfferLiveFallback", false);
        QVERIFY(!SearchDefaults::read(search).offerRanking);
        QVERIFY(!SearchDefaults::read(search).offerLiveFallback);

        SearchWidget widget(QUrl::fromLocalFile(root.path()), nullptr, config);
        widget.show();
        auto *rank = widget.findChild<QCheckBox *>(QStringLiteral("searchRank"));
        auto *scope = widget.findChild<QComboBox *>(QStringLiteral("searchScope"));
        auto *query = widget.findChild<QLineEdit *>(QStringLiteral("searchQuery"));
        auto *start = widget.findChild<QPushButton *>(QStringLiteral("searchStart"));
        auto *results = widget.findChild<QTreeWidget *>(QStringLiteral("searchResults"));
        auto *status = widget.findChild<QLabel *>(QStringLiteral("searchStatus"));
        auto *fallback = widget.findChild<QPushButton *>(QStringLiteral("searchLiveFallback"));
        QVERIFY(rank && scope && query && start && results && status && fallback);
        // The panel's own choice is kept, but not used while ranking isn't offered.
        QVERIFY(rank->isHidden());
        QVERIFY(rank->isChecked());
        query->setText(QStringLiteral("report"));
        start->click();
        QTRY_VERIFY_WITH_TIMEOUT(start->isEnabled(), 5000);
        QCOMPARE(results->topLevelItemCount(), 2);
        QVERIFY(!status->text().contains(QStringLiteral("ranked")));
        scope->setCurrentIndex(1);
        start->click();
        QTRY_VERIFY_WITH_TIMEOUT(start->isEnabled(), 10000);
        QVERIFY(!fallback->isVisible());

        search.writeEntry("SearchOfferRanking", true);
        search.writeEntry("SearchOfferLiveFallback", true);
        widget.applyDefaults();
        QVERIFY(!rank->isHidden());
        start->click();
        QTRY_VERIFY_WITH_TIMEOUT(start->isEnabled(), 10000);
        QVERIFY(fallback->isVisible());
        scope->setCurrentIndex(0);
        start->click();
        QTRY_VERIFY_WITH_TIMEOUT(start->isEnabled(), 5000);
        QVERIFY(status->text().contains(QStringLiteral("ranked by name similarity")));
        QCOMPARE(QFileInfo(results->topLevelItem(0)->data(0, Qt::UserRole).toString()).fileName(), QStringLiteral("report.txt"));

        // Turning the fallback off hides an offer already shown.
        scope->setCurrentIndex(1);
        start->click();
        QTRY_VERIFY_WITH_TIMEOUT(start->isEnabled(), 10000);
        QVERIFY(fallback->isVisible());
        search.writeEntry("SearchOfferLiveFallback", false);
        widget.applyDefaults();
        QVERIFY(!fallback->isVisible());
    }

    void mainWindowOpensSearchPanel()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        MainWindow window;
        window.resize(1200, 900);
        window.openUrls({QUrl::fromLocalFile(root.path()), QUrl::fromLocalFile(root.path())});
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(true);
        QAction *action = window.actionCollection()->action(QStringLiteral("search_folder"));
        auto *toggle = window.actionCollection()->action(QStringLiteral("show_search_panel"));
        auto *panel = window.findChild<SearchPanel *>(QStringLiteral("searchPanel"));
        auto *workspace = window.findChild<QSplitter *>(QStringLiteral("workspaceSplitter"));
        QVERIFY(action && toggle && toggle->isCheckable() && panel && workspace);
        QVERIFY(panel->isHidden());
        QCOMPARE(workspace->indexOf(panel), 2);
        QCOMPARE(window.findChildren<SearchWidget *>().size(), 1);

        action->trigger();
        QVERIFY(toggle->isChecked());
        QVERIFY(panel->isVisible());
        QVERIFY(window.activePane()->isVisible());
        QVERIFY(window.inactivePane()->isVisible());
        auto *search = panel->searchWidget();
        QCOMPARE(search->folder(), QUrl::fromLocalFile(root.path()));
        QTRY_COMPARE(QApplication::focusWidget(), search->findChild<QLineEdit *>(QStringLiteral("searchQuery")));
        QVERIFY2(workspace->sizes().at(0) > workspace->sizes().at(2),
                 qPrintable(QDebug::toString(workspace->sizes()) + QDebug::toString(panel->minimumSizeHint())));

        // Showing the preview in a pane keeps both panes while an embedded panel is visible.
        auto *preview = window.actionCollection()->action(QStringLiteral("show_preview_panel"));
        QVERIFY(preview);
        preview->setChecked(true);
        QVERIFY(window.activePane()->isVisible());
        QVERIFY(window.inactivePane()->isVisible());
        preview->setChecked(false);

        panel->findChild<QToolButton *>(QStringLiteral("searchPanelClose"))->click();
        QVERIFY(!toggle->isChecked());
        QVERIFY(panel->isHidden());
        QTRY_VERIFY(window.activePane()->isAncestorOf(QApplication::focusWidget()));
        toggle->setChecked(true);
        QVERIFY(panel->isVisible());
        QCOMPARE(window.findChildren<SearchWidget *>().size(), 1);

        window.actionCollection()->action(QStringLiteral("show_embedded_terminal"))->setChecked(true);
        QCOMPARE(workspace->indexOf(window.findChild<QWidget *>(QStringLiteral("terminalPanel"))), 1);
        QVERIFY(panel->isVisible());
        const QList<int> sizes = workspace->sizes();
        QVERIFY2(sizes.at(0) * 10 >= (sizes.at(0) + sizes.at(1) + sizes.at(2)) * 5, qPrintable(QDebug::toString(sizes)));
        window.actionCollection()->action(QStringLiteral("show_embedded_terminal"))->setChecked(false);
    }

    void searchPanelPlacementAndStartupVisibility()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const QUrl url = QUrl::fromLocalFile(root.path());
        Settings::setTerminalPlacement(Settings::EnumTerminalPlacement::Below);
        Settings::setSearchPanelPlacement(Settings::EnumSearchPanelPlacement::Right);
        Settings::setRestoreSearchPanel(false);
        {
            MainWindow window;
            window.resize(1200, 900);
            window.openUrls({url, url});
            window.show();
            QVERIFY(QTest::qWaitForWindowExposed(&window));
            window.setStackedLayout(false);
            window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(true);
            auto *panel = window.findChild<SearchPanel *>(QStringLiteral("searchPanel"));
            auto *workspace = window.findChild<QSplitter *>(QStringLiteral("workspaceSplitter"));
            auto *outer = window.findChild<QSplitter *>(QStringLiteral("searchSplitter"));
            auto *terminal = window.findChild<QWidget *>(QStringLiteral("terminalPanel"));
            QVERIFY(panel && workspace && outer && terminal);

            // Right of the panes while the terminal stays below them.
            QCOMPARE(workspace->orientation(), Qt::Vertical);
            QCOMPARE(outer->orientation(), Qt::Horizontal);
            QCOMPARE(outer->indexOf(workspace), 0);
            QCOMPARE(outer->indexOf(panel), 1);
            QCOMPARE(workspace->indexOf(panel), -1);
            window.actionCollection()->action(QStringLiteral("show_search_panel"))->setChecked(true);
            window.actionCollection()->action(QStringLiteral("show_embedded_terminal"))->setChecked(true);
            QVERIFY(panel->isVisible());
            QVERIFY(terminal->isVisible());
            QVERIFY(window.activePane()->isVisible());
            QVERIFY(window.inactivePane()->isVisible());
            const QList<int> outerSizes = outer->sizes();
            QVERIFY2(outerSizes.at(0) * 10 >= (outerSizes.at(0) + outerSizes.at(1)) * 6 - 10, qPrintable(QDebug::toString(outerSizes)));

            // Following the terminal moves the panel beside it and keeps it visible.
            Settings::setSearchPanelPlacement(Settings::EnumSearchPanelPlacement::WithTerminal);
            window.applySettings();
            QCOMPARE(workspace->indexOf(panel), 2);
            QCOMPARE(outer->indexOf(panel), -1);
            QVERIFY(panel->isVisible());

            // An explicit placement matching the terminal also shares its splitter.
            Settings::setSearchPanelPlacement(Settings::EnumSearchPanelPlacement::Below);
            window.applySettings();
            QCOMPARE(workspace->indexOf(panel), 2);
            Settings::setTerminalPlacement(Settings::EnumTerminalPlacement::Right);
            window.applySettings();
            QCOMPARE(workspace->orientation(), Qt::Horizontal);
            QCOMPARE(outer->orientation(), Qt::Vertical);
            QCOMPARE(outer->indexOf(panel), 1);
            QVERIFY(panel->isVisible());
            QVERIFY(window.activePane()->isVisible());
            QVERIFY(window.inactivePane()->isVisible());
            window.actionCollection()->action(QStringLiteral("show_embedded_terminal"))->setChecked(false);
        }
        Settings::setTerminalPlacement(Settings::EnumTerminalPlacement::Automatic);
        Settings::setSearchPanelPlacement(Settings::EnumSearchPanelPlacement::WithTerminal);

        // The panel was open when the window closed, but is restored only when enabled.
        {
            MainWindow window;
            window.openUrls({url});
            QTest::qWait(50);
            QVERIFY(!window.actionCollection()->action(QStringLiteral("show_search_panel"))->isChecked());
        }
        Settings::setRestoreSearchPanel(true);
        {
            MainWindow window;
            window.openUrls({url});
            window.show();
            QVERIFY(QTest::qWaitForWindowExposed(&window));
            auto *panel = window.findChild<SearchPanel *>(QStringLiteral("searchPanel"));
            QTRY_VERIFY(window.actionCollection()->action(QStringLiteral("show_search_panel"))->isChecked());
            QVERIFY(panel->isVisible());
            QCOMPARE(panel->searchWidget()->folder(), url);
            QVERIFY(!panel->isAncestorOf(QApplication::focusWidget()));
            window.actionCollection()->action(QStringLiteral("show_search_panel"))->setChecked(false);
        }
        {
            MainWindow window;
            window.openUrls({url});
            QTest::qWait(50);
            QVERIFY(!window.actionCollection()->action(QStringLiteral("show_search_panel"))->isChecked());
        }
        Settings::setRestoreSearchPanel(false);
    }

    void searchPanelScopeFollowsRequestingPane()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("left")));
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("right")));
        const QUrl left = QUrl::fromLocalFile(root.filePath(QStringLiteral("left")));
        const QUrl right = QUrl::fromLocalFile(root.filePath(QStringLiteral("right")));
        MainWindow window;
        window.openUrls({left, right});
        window.actionCollection()->action(QStringLiteral("dual_layout"))->setChecked(true);
        Pane *inactive = window.inactivePane();
        QTRY_COMPARE(inactive->url(), right);
        inactive->toolbarAction(QStringLiteral("paneSearch"))->trigger();
        auto *search = window.findChild<SearchWidget *>(QStringLiteral("searchWidget"));
        QVERIFY(search);
        QCOMPARE(window.activePane(), inactive);
        QCOMPARE(search->folder(), right);
        QCOMPARE(search->findChild<QLabel *>(QStringLiteral("searchFolder"))->text(), right.toDisplayString());
    }

    void tabOrderFollowsLayout()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        SearchWidget widget(QUrl::fromLocalFile(root.path()));
        widget.resize(500, 900);
        widget.show();
        QVERIFY(QTest::qWaitForWindowExposed(&widget));
        widget.focusQuery();
        QTRY_COMPARE(QApplication::focusWidget(), widget.findChild<QLineEdit *>(QStringLiteral("searchQuery")));
        QList<QWidget *> visited;
        for (int i = 0; i < 40; ++i) {
            QTest::keyClick(QApplication::focusWidget(), Qt::Key_Tab);
            QWidget *focused = QApplication::focusWidget();
            QVERIFY(focused);
            if (focused->objectName() == QLatin1String("searchQuery")) {
                break;
            }
            QVERIFY2(!visited.contains(focused), qPrintable(QStringLiteral("Tab stayed on ") + focused->objectName() + QStringLiteral(" after ") + [&visited] {
                QStringList names;
                for (QWidget *w : visited) {
                    names.append(w->objectName());
                }
                return names.join(QLatin1Char(','));
            }()));
            QVERIFY2(!qobject_cast<QScrollArea *>(focused), qPrintable(focused->objectName()));
            visited.append(focused);
        }
        QCOMPARE(QApplication::focusWidget()->objectName(), QStringLiteral("searchQuery"));
        const auto position = [&visited, &widget](const QString &name) {
            return visited.indexOf(widget.findChild<QWidget *>(name));
        };
        QCOMPARE(position(QStringLiteral("searchField")), 0);
        QVERIFY(position(QStringLiteral("searchExcludedFolders")) >= 0);
        QVERIFY(position(QStringLiteral("searchExcludedFolders")) < position(QStringLiteral("searchMaxDepth")));
        QVERIFY(position(QStringLiteral("searchSaved")) < position(QStringLiteral("searchSave")));
        auto *tabs = widget.findChild<QTabWidget *>(QStringLiteral("searchResultTabs"));
        QVERIFY(tabs);
        QVERIFY(visited.indexOf(tabs->tabBar()) >= 0);
        QVERIFY(position(QStringLiteral("searchSave")) < visited.indexOf(tabs->tabBar()));
        QVERIFY(visited.indexOf(tabs->tabBar()) < position(QStringLiteral("searchResults")));
        QVERIFY(position(QStringLiteral("searchResults")) < position(QStringLiteral("searchStart")));
    }

    void toolbarSearchFieldSearchesActivePane()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("left")));
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("right")));
        QFile needle(root.filePath(QStringLiteral("left/needle.txt")));
        QVERIFY(needle.open(QIODevice::WriteOnly));
        needle.close();
        const QUrl left = QUrl::fromLocalFile(root.filePath(QStringLiteral("left")));
        const QUrl right = QUrl::fromLocalFile(root.filePath(QStringLiteral("right")));
        const auto config = KSharedConfig::openConfig();
        KConfigGroup customization = config->group(QStringLiteral("ToolbarCustomization"));
        const auto backup = KSharedConfig::openConfig(root.filePath(QStringLiteral("toolbar-backup")));
        KConfigGroup savedCustomization = backup->group(QStringLiteral("ToolbarCustomization"));
        customization.copyTo(&savedCustomization);
        customization.deleteGroup();
        const auto restoreCustomization = qScopeGuard([&] {
            customization.deleteGroup();
            savedCustomization.copyTo(&customization);
            config->sync();
        });
        MainWindow window;
        window.resize(1200, 900);
        window.openUrls({left, right});
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_COMPARE(window.activePane()->url(), left);

        auto *action = qobject_cast<QWidgetAction *>(window.actionCollection()->action(QStringLiteral("search_toolbar")));
        QVERIFY(action);
        QVERIFY(!action->text().isEmpty());
        QVERIFY(!action->icon().isNull());
        QToolBar *toolbar = window.toolBar();
        QVERIFY(toolbar);
        QVERIFY(!toolbar->actions().contains(action));
        window.actionCollection()->action(QStringLiteral("customize_main_toolbar"))->trigger();
        auto *actionList = window.findChild<QListWidget *>(QStringLiteral("toolbarActionList"));
        QVERIFY(actionList);
        const auto listed = [actionList] {
            for (int row = 0; row < actionList->count(); ++row) {
                if (actionList->item(row)->data(Qt::UserRole + 1).toString() == QLatin1String("search_toolbar")) {
                    return true;
                }
            }
            return false;
        };
        QVERIFY(listed());
        toolbar->addAction(action);
        QTRY_VERIFY(!listed());
        QTRY_VERIFY(customization.readEntry("Actions", QStringList()).contains(QStringLiteral("action:search_toolbar")));
        window.actionCollection()->action(QStringLiteral("customize_main_toolbar"))->trigger();
        auto *field = toolbar->findChild<QLineEdit *>(QStringLiteral("toolbarSearch"));
        QVERIFY(field);
        QTRY_VERIFY(field->isVisible());

        auto *panel = window.findChild<SearchPanel *>(QStringLiteral("searchPanel"));
        auto *toggle = window.actionCollection()->action(QStringLiteral("show_search_panel"));
        QVERIFY(panel && toggle);
        SearchWidget *search = panel->searchWidget();
        QVERIFY(search->setFolder(right));

        // Empty text only opens the panel and focuses its query.
        field->setFocus();
        QTest::keyClick(field, Qt::Key_Return);
        QVERIFY(toggle->isChecked());
        QCOMPARE(search->folder(), left);
        QTRY_COMPARE(QApplication::focusWidget(), search->findChild<QLineEdit *>(QStringLiteral("searchQuery")));

        toggle->setChecked(false);
        QVERIFY(search->setFolder(right));
        field->setFocus();
        QTest::keyClicks(field, QStringLiteral("needle"));
        QTest::keyClick(field, Qt::Key_Return);
        QVERIFY(toggle->isChecked());
        QVERIFY(panel->isVisible());
        QCOMPARE(search->folder(), left);
        QCOMPARE(search->findChild<QLineEdit *>(QStringLiteral("searchQuery"))->text(), QStringLiteral("needle"));
        auto *results = search->findChild<QTreeWidget *>(QStringLiteral("searchResults"));
        QVERIFY(results);
        QTRY_COMPARE(results->topLevelItemCount(), 1);
        QVERIFY(!search->isRunning());
        QCOMPARE(QApplication::focusWidget(), field);

        // Down moves from the field to the results, Escape returns to the active pane.
        QTest::keyClick(field, Qt::Key_Down);
        QCOMPARE(QApplication::focusWidget(), results);
        QVERIFY(results->currentItem());
        QTest::keyClick(results, Qt::Key_Escape);
        QVERIFY(panel->isVisible());
        QVERIFY(window.activePane()->isAncestorOf(QApplication::focusWidget()));
        auto *query = search->findChild<QLineEdit *>(QStringLiteral("searchQuery"));
        search->focusQuery();
        QTest::keyClick(query, Qt::Key_Down);
        QCOMPARE(QApplication::focusWidget(), results);
        field->setFocus();
        QTest::keyClick(field, Qt::Key_Escape);
        QVERIFY(window.activePane()->isAncestorOf(QApplication::focusWidget()));

        // Without results, Down moves to the panel's query; with the panel hidden it stays put.
        results->clear();
        field->setFocus();
        QTest::keyClick(field, Qt::Key_Down);
        QCOMPARE(QApplication::focusWidget(), query);
        toggle->setChecked(false);
        field->setFocus();
        QTest::keyClick(field, Qt::Key_Down);
        QCOMPARE(QApplication::focusWidget(), field);

        // Defaults changed in Preferences reach the open panel used by the field.
        auto *fieldChoice = search->findChild<QComboBox *>(QStringLiteral("searchField"));
        QCOMPARE(fieldChoice->currentIndex(), 0);
        Settings::setDefaultSearchField(Settings::EnumDefaultSearchField::Contents);
        Settings::self()->save();
        window.applySettings();
        QCOMPARE(fieldChoice->currentIndex(), 1);
        Settings::setDefaultSearchField(Settings::EnumDefaultSearchField::FileName);
        Settings::self()->save();
        window.applySettings();
        QCOMPARE(fieldChoice->currentIndex(), 0);

        // The field's menu runs the panel's saved searches.
        KConfigGroup savedGroup = config->group(QStringLiteral("SavedSearches"));
        KConfigGroup savedBackup = backup->group(QStringLiteral("SavedSearches"));
        savedGroup.copyTo(&savedBackup);
        const auto restoreSaved = qScopeGuard([&] {
            savedGroup.deleteGroup();
            savedBackup.copyTo(&savedGroup);
            config->sync();
        });
        SavedSearches::Search saved;
        saved.name = QStringLiteral("Needles");
        saved.text = QStringLiteral("needle");
        saved.folder = left;
        QString error;
        QVERIFY(SavedSearches::save(savedGroup, {saved}, &error));
        results->clear();
        auto *savedAction = field->findChild<QAction *>(QStringLiteral("toolbarSavedSearches"));
        QVERIFY(savedAction);
        savedAction->trigger();
        QMenu *menu = nullptr;
        QTRY_VERIFY((menu = field->findChild<QMenu *>(QStringLiteral("toolbarSavedSearchesMenu"))));
        QCOMPARE(menu->actions().size(), 1);
        QCOMPARE(menu->actions().constFirst()->text(), saved.name);
        menu->actions().constFirst()->trigger();
        menu->close();
        QVERIFY(toggle->isChecked());
        QTRY_COMPARE(results->topLevelItemCount(), 1);
        QCOMPARE(search->findChild<QComboBox *>(QStringLiteral("searchSaved"))->currentText(), saved.name);

        // Preferences can leave the panel hidden; the status bar then reports the outcome.
        Settings::setToolbarSearchOpensPanel(false);
        Settings::self()->save();
        const auto restoreOpening = qScopeGuard([] {
            Settings::setToolbarSearchOpensPanel(true);
            Settings::self()->save();
        });
        toggle->setChecked(false);
        results->clear();
        field->setFocus();
        field->clear();
        QTest::keyClicks(field, QStringLiteral("needle"));
        QTest::keyClick(field, Qt::Key_Return);
        QVERIFY(!toggle->isChecked());
        QTRY_COMPARE(results->topLevelItemCount(), 1);
        QTRY_VERIFY(window.statusBar()->currentMessage().contains(QStringLiteral("1 match")));
        QCOMPARE(QApplication::focusWidget(), field);
        // Down then shows the panel with its results.
        QTest::keyClick(field, Qt::Key_Down);
        QVERIFY(toggle->isChecked());
        QCOMPARE(QApplication::focusWidget(), results);

        // Saved searches follow the same choice, and a search that can't start is reported.
        toggle->setChecked(false);
        window.statusBar()->clearMessage();
        savedAction->trigger();
        QTRY_VERIFY((menu = field->findChild<QMenu *>(QStringLiteral("toolbarSavedSearchesMenu"))));
        menu->actions().constFirst()->trigger();
        menu->close();
        QVERIFY(!toggle->isChecked());
        QTRY_VERIFY(!window.statusBar()->currentMessage().isEmpty());
        QTRY_VERIFY(search->findChild<QPushButton *>(QStringLiteral("searchStart"))->isEnabled());
        search->findChild<QComboBox *>(QStringLiteral("searchField"))->setCurrentIndex(2);
        window.statusBar()->clearMessage();
        field->setFocus();
        field->setText(QStringLiteral("(unbalanced"));
        QTest::keyClick(field, Qt::Key_Return);
        QVERIFY(!toggle->isChecked());
        QCOMPARE(window.statusBar()->currentMessage(), search->status());
        QVERIFY(!search->status().isEmpty());
    }

    void opensResultsAndRevealsWithKeyboard()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("match-folder")));
        const QString filePath = root.filePath(QStringLiteral("match-folder/match-file.txt"));
        QFile file(filePath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(root.path())});
        window.actionCollection()->action(QStringLiteral("search_folder"))->trigger();
        auto *search = window.findChild<SearchWidget *>(QStringLiteral("searchWidget"));
        QVERIFY(search);
        auto *open = search->findChild<QPushButton *>(QStringLiteral("searchOpen"));
        auto *start = search->findChild<QPushButton *>(QStringLiteral("searchStart"));
        auto *results = search->findChild<QTreeWidget *>(QStringLiteral("searchResults"));
        auto *status = search->findChild<QLabel *>(QStringLiteral("searchStatus"));
        QVERIFY(open && start && results && status);
        QVERIFY(!open->isEnabled());
        QSignalSpy opened(search, &SearchWidget::openRequested);
        QSignalSpy revealed(search, &SearchWidget::revealRequested);
        search->search(QStringLiteral("match"));
        QTRY_VERIFY(start->isEnabled());
        QCOMPARE(results->topLevelItemCount(), 2);
        QTreeWidgetItem *folderItem = results->findItems(root.filePath(QStringLiteral("match-folder")), Qt::MatchExactly).value(0);
        QTreeWidgetItem *fileItem = results->findItems(filePath, Qt::MatchExactly).value(0);
        QVERIFY(folderItem && fileItem);

        results->setCurrentItem(fileItem);
        QTest::keyClick(results, Qt::Key_Return, Qt::ControlModifier);
        QCOMPARE(revealed.size(), 1);
        QCOMPARE(revealed.takeFirst().first().toUrl(), QUrl::fromLocalFile(filePath));
        QTest::keyClick(results, Qt::Key_Return);
        QCOMPARE(opened.size(), 1);
        QCOMPARE(opened.takeFirst().first().toUrl(), QUrl::fromLocalFile(filePath));

        Pane *pane = window.activePane();
        results->setCurrentItem(folderItem);
        QVERIFY(open->isEnabled());
        open->click();
        QCOMPARE(opened.size(), 1);
        QTRY_COMPARE(pane->url(), QUrl::fromLocalFile(root.filePath(QStringLiteral("match-folder"))));
        opened.clear();

        QVERIFY(QFile::remove(filePath));
        results->setCurrentItem(fileItem);
        open->click();
        QVERIFY(opened.isEmpty());
        QVERIFY(status->text().contains(QStringLiteral("no longer exists")));
    }

    void setFolderIsIgnoredWhileSearching()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        SearchWidget search(QUrl::fromLocalFile(root.path()));
        QVERIFY(search.setFolder(QUrl::fromLocalFile(QDir::rootPath())));
        QCOMPARE(search.folder(), QUrl::fromLocalFile(QDir::rootPath()));
        search.search(QStringLiteral("a-name-that-should-not-exist-anywhere"));
        QVERIFY(search.isRunning());
        QVERIFY(!search.setFolder(QUrl::fromLocalFile(root.path())));
        QCOMPARE(search.folder(), QUrl::fromLocalFile(QDir::rootPath()));
        search.findChild<QPushButton *>(QStringLiteral("searchCancel"))->click();
        QTRY_VERIFY(!search.isRunning());
        QVERIFY(search.setFolder(QUrl::fromLocalFile(root.path())));
    }

    void preparesFolderSearchesAndReportsTheirProgress()
    {
        SearchWidget remote(QUrl(QStringLiteral("sftp://example.invalid/path")));
        auto *scope = remote.findChild<QComboBox *>(QStringLiteral("searchScope"));
        QVERIFY(scope);
        scope->setCurrentIndex(1);
        QVERIFY(!remote.searchesCurrentFolder());
        QVERIFY(remote.locationLimitation().isEmpty());

        // The Yazi search keys pick the live current-folder scope and the field.
        QVERIFY(remote.prepareFolderSearch(LiveSearch::Field::Contents));
        QVERIFY(remote.searchesCurrentFolder());
        QCOMPARE(remote.field(), LiveSearch::Field::Contents);
        QVERIFY(remote.locationLimitation().contains(QStringLiteral("local folders only")));
        QVERIFY(remote.prepareFolderSearch(LiveSearch::Field::FileName));
        QCOMPARE(remote.field(), LiveSearch::Field::FileName);

        // A search the location can't run is rejected with the panel's message.
        QSignalSpy rejected(&remote, &SearchWidget::startRejected);
        QSignalSpy remoteStarted(&remote, &SearchWidget::started);
        QVERIFY(!remote.search(QStringLiteral("x")));
        QCOMPARE(rejected.count(), 1);
        QCOMPARE(rejected.at(0).at(0).toString(), remote.status());
        QVERIFY(remote.status().contains(QStringLiteral("local folders only")));
        QCOMPARE(remoteStarted.count(), 0);

        // A local search starts, can't be re-prepared while it runs and reports its end.
        SearchWidget local(QUrl::fromLocalFile(QDir::rootPath()));
        QVERIFY(local.prepareFolderSearch(LiveSearch::Field::FileName));
        QVERIFY(local.locationLimitation().isEmpty());
        QSignalSpy started(&local, &SearchWidget::started);
        QSignalSpy finished(&local, &SearchWidget::finished);
        QVERIFY(local.search(QStringLiteral("a-name-that-should-not-exist-anywhere")));
        QCOMPARE(started.count(), 1);
        QVERIFY(!local.prepareFolderSearch(LiveSearch::Field::Contents));
        QCOMPARE(local.field(), LiveSearch::Field::FileName);
        local.cancel();
        QTRY_COMPARE(finished.count(), 1);
        QVERIFY(!local.isRunning());
    }

    void revealsResultInActivePaneAndReportsMissingItems()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QVERIFY(QDir(root.path()).mkdir(QStringLiteral("sub")));
        const QString filePath = root.filePath(QStringLiteral("sub/find-me.txt"));
        QFile file(filePath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
        MainWindow window;
        window.openUrls({QUrl::fromLocalFile(root.path())});
        window.actionCollection()->action(QStringLiteral("search_folder"))->trigger();
        auto *dialog = window.findChild<SearchWidget *>(QStringLiteral("searchWidget"));
        QVERIFY(dialog);
        auto *query = dialog->findChild<QLineEdit *>(QStringLiteral("searchQuery"));
        auto *start = dialog->findChild<QPushButton *>(QStringLiteral("searchStart"));
        auto *reveal = dialog->findChild<QPushButton *>(QStringLiteral("searchReveal"));
        auto *results = dialog->findChild<QTreeWidget *>(QStringLiteral("searchResults"));
        QVERIFY(query && start && reveal && results);
        QVERIFY(!reveal->isEnabled());
        query->setText(QStringLiteral("find-me"));
        start->click();
        QTRY_VERIFY(start->isEnabled());
        QCOMPARE(results->topLevelItemCount(), 1);
        results->setCurrentItem(results->topLevelItem(0));
        QVERIFY(reveal->isEnabled());
        Pane *pane = window.activePane();
        reveal->click();
        QTRY_COMPARE(pane->url(), QUrl::fromLocalFile(root.filePath(QStringLiteral("sub"))));
        QTRY_COMPARE(pane->view()->selectedItems().size(), 1);
        QCOMPARE(pane->view()->selectedItems().first().url(), QUrl::fromLocalFile(filePath));
        QSignalSpy status(pane, &Pane::statusMessage);
        pane->view()->setNameFilter(NameFilter(QStringLiteral("not-present"), NameFilter::Scope::FilesAndFolders));
        reveal->click();
        QCOMPARE(status.size(), 1);
        QVERIFY(status.takeFirst().first().toString().contains(QStringLiteral("filtered")));
        pane->view()->setNameFilter(NameFilter());
        pane->view()->setFlat(true);
        reveal->click();
        QCOMPARE(status.size(), 1);
        QVERIFY(status.takeFirst().first().toString().contains(QStringLiteral("flat view")));
        pane->view()->setFlat(false);
        QVERIFY(QFile::remove(filePath));
        reveal->click();
        QCOMPARE(status.size(), 1);
        QVERIFY(status.first().first().toString().contains(QStringLiteral("no longer available")));
    }

    void savesAndRestoresSearchCriteria()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QTemporaryDir configDir;
        QVERIFY(configDir.isValid());
        QFile file(root.filePath(QStringLiteral("result.txt")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write("needle"), 6);
        file.close();
        const auto config = KSharedConfig::openConfig(configDir.filePath(QStringLiteral("searchesrc")), KConfig::SimpleConfig);
        {
            SearchWidget dialog(QUrl::fromLocalFile(root.path()), nullptr, config);
            dialog.findChild<QLineEdit *>(QStringLiteral("searchQuery"))->setText(QStringLiteral("needle"));
            dialog.findChild<QComboBox *>(QStringLiteral("searchField"))->setCurrentIndex(1);
            dialog.findChild<QLineEdit *>(QStringLiteral("searchPathFilter"))->setText(QStringLiteral("result"));
            dialog.findChild<QLineEdit *>(QStringLiteral("searchParentFilter"))->setText(root.path());
            dialog.findChild<QPlainTextEdit *>(QStringLiteral("searchExcludedFolders"))->setPlainText(root.filePath(QStringLiteral("excluded")));
            dialog.findChild<QSpinBox *>(QStringLiteral("searchMaxDepth"))->setValue(0);
            QTimer::singleShot(0, [] {
                auto *input = qobject_cast<QInputDialog *>(QApplication::activeModalWidget());
                QVERIFY(input);
                input->setTextValue(QStringLiteral("Documents"));
                input->accept();
            });
            dialog.findChild<QPushButton *>(QStringLiteral("searchSave"))->click();
            QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("searchSaved"))->count(), 1);
        }
        auto defaults = config->group(QStringLiteral("Search"));
        defaults.writeEntry("DefaultSearchScope", QStringLiteral("Indexed"));
        defaults.writeEntry("DefaultSearchField", QStringLiteral("FileName"));
        QVERIFY(defaults.sync());
        config->reparseConfiguration();
        SearchWidget restored(QUrl::fromLocalFile(QStringLiteral("/")), nullptr, config);
        QCOMPARE(restored.findChild<QComboBox *>(QStringLiteral("searchSaved"))->count(), 1);
        QCOMPARE(restored.findChild<QComboBox *>(QStringLiteral("searchScope"))->currentIndex(), 1);
        QCOMPARE(restored.findChild<QComboBox *>(QStringLiteral("searchField"))->currentIndex(), 0);
        restored.findChild<QPushButton *>(QStringLiteral("searchLoad"))->click();
        QCOMPARE(restored.findChild<QLineEdit *>(QStringLiteral("searchQuery"))->text(), QStringLiteral("needle"));
        QCOMPARE(restored.findChild<QComboBox *>(QStringLiteral("searchField"))->currentIndex(), 1);
        QCOMPARE(restored.findChild<QComboBox *>(QStringLiteral("searchScope"))->currentIndex(), 0);
        QCOMPARE(restored.findChild<QLineEdit *>(QStringLiteral("searchPathFilter"))->text(), QStringLiteral("result"));
        QCOMPARE(restored.findChild<QLineEdit *>(QStringLiteral("searchParentFilter"))->text(), root.path());
        QCOMPARE(restored.findChild<QPlainTextEdit *>(QStringLiteral("searchExcludedFolders"))->toPlainText(), root.filePath(QStringLiteral("excluded")));
        QCOMPARE(restored.findChild<QSpinBox *>(QStringLiteral("searchMaxDepth"))->value(), 0);
        auto *start = restored.findChild<QPushButton *>(QStringLiteral("searchStart"));
        start->click();
        QTRY_VERIFY(start->isEnabled());
        auto *results = restored.findChild<QTreeWidget *>(QStringLiteral("searchResults"));
        QCOMPARE(results->topLevelItemCount(), 1);
        QCOMPARE(results->topLevelItem(0)->text(0), file.fileName());
        auto *deleteSaved = restored.findChild<QPushButton *>(QStringLiteral("searchDelete"));
        QTimer::singleShot(0, [] {
            auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            QVERIFY(box);
            box->button(QMessageBox::No)->click();
        });
        deleteSaved->click();
        QCOMPARE(restored.findChild<QComboBox *>(QStringLiteral("searchSaved"))->count(), 1);
        QTimer::singleShot(0, [] {
            auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            QVERIFY(box);
            box->button(QMessageBox::Yes)->click();
        });
        deleteSaved->click();
        QCOMPARE(restored.findChild<QComboBox *>(QStringLiteral("searchSaved"))->count(), 0);
        QVERIFY(QFile::exists(file.fileName()));
    }

    void savingFromTwoDialogsPreservesBothSearches()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const auto config = KSharedConfig::openConfig(root.filePath(QStringLiteral("searchesrc")), KConfig::SimpleConfig);
        SearchWidget first(QUrl::fromLocalFile(root.path()), nullptr, config);
        SearchWidget second(QUrl::fromLocalFile(root.path()), nullptr, config);
        const auto save = [](SearchWidget &dialog, const QString &name) {
            dialog.findChild<QLineEdit *>(QStringLiteral("searchQuery"))->setText(QStringLiteral("term"));
            QTimer::singleShot(0, [name] {
                auto *input = qobject_cast<QInputDialog *>(QApplication::activeModalWidget());
                QVERIFY(input);
                input->setTextValue(name);
                input->accept();
            });
            dialog.findChild<QPushButton *>(QStringLiteral("searchSave"))->click();
        };
        save(first, QStringLiteral("One"));
        save(second, QStringLiteral("Two"));
        SearchWidget restored(QUrl::fromLocalFile(root.path()), nullptr, config);
        auto *saved = restored.findChild<QComboBox *>(QStringLiteral("searchSaved"));
        QCOMPARE(saved->count(), 2);
        QCOMPARE(saved->itemText(0), QStringLiteral("One"));
        QCOMPARE(saved->itemText(1), QStringLiteral("Two"));
    }

    void updatesSavedSearchAndDetectsStaleEdits()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const auto config = KSharedConfig::openConfig(root.filePath(QStringLiteral("searchesrc")), KConfig::SimpleConfig);
        const auto group = config->group(QStringLiteral("SavedSearches"));
        QString error;
        QVERIFY(
            SavedSearches::save(group,
                                {{QStringLiteral("Work"), QStringLiteral("old"), QUrl::fromLocalFile(root.path()), LiveSearch::Field::FileName, false, {}, {}}},
                                &error));
        const QString stableId = SavedSearches::load(group, &error).first().id;
        SearchWidget first(QUrl::fromLocalFile(root.path()), nullptr, config);
        SearchWidget second(QUrl::fromLocalFile(root.path()), nullptr, config);
        first.findChild<QLineEdit *>(QStringLiteral("searchQuery"))->setText(QStringLiteral("new"));
        first.findChild<QPushButton *>(QStringLiteral("searchUpdate"))->click();
        auto saved = SavedSearches::load(group, &error);
        QCOMPARE(saved.first().text, QStringLiteral("new"));
        QCOMPARE(saved.first().id, stableId);
        second.findChild<QLineEdit *>(QStringLiteral("searchQuery"))->setText(QStringLiteral("stale"));
        second.findChild<QPushButton *>(QStringLiteral("searchUpdate"))->click();
        QVERIFY(second.findChild<QLabel *>(QStringLiteral("searchStatus"))->text().contains(QStringLiteral("changed elsewhere")));
        QCOMPARE(SavedSearches::load(group, &error).first().text, QStringLiteral("new"));
        second.findChild<QPushButton *>(QStringLiteral("searchLoad"))->click();
        QCOMPARE(second.findChild<QLineEdit *>(QStringLiteral("searchQuery"))->text(), QStringLiteral("new"));
        second.findChild<QLineEdit *>(QStringLiteral("searchQuery"))->setText(QStringLiteral("latest"));
        second.findChild<QPushButton *>(QStringLiteral("searchUpdate"))->click();
        saved = SavedSearches::load(group, &error);
        QCOMPARE(saved.first().text, QStringLiteral("latest"));
        QCOMPARE(saved.first().id, stableId);
    }

    void savesLoadsAndUpdatesNamedCollections()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const auto config = KSharedConfig::openConfig(root.filePath(QStringLiteral("searchesrc")), KConfig::SimpleConfig);
        const auto group = config->group(QStringLiteral("SearchCollection"));
        const QUrl a = QUrl::fromLocalFile(root.filePath(QStringLiteral("a")));
        const QUrl b = QUrl::fromLocalFile(root.filePath(QStringLiteral("b")));
        QString error;
        QVERIFY(SearchCollection::save(group, {a}, &error));
        SearchWidget dialog(QUrl::fromLocalFile(root.path()), nullptr, config);
        dialog.findChild<QTabWidget *>(QStringLiteral("searchResultTabs"))->setCurrentIndex(1);
        QTimer::singleShot(0, [] {
            auto *input = qobject_cast<QInputDialog *>(QApplication::activeModalWidget());
            QVERIFY(input);
            input->setTextValue(QStringLiteral("First"));
            input->accept();
        });
        dialog.findChild<QPushButton *>(QStringLiteral("searchSaveCollection"))->click();
        QCOMPARE(SearchCollection::loadSaved(group, &error).first().urls, (QList<QUrl>{a}));
        QVERIFY(SearchCollection::save(group, {b}, &error));
        QTimer::singleShot(0, [] {
            auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            QVERIFY(box);
            box->button(QMessageBox::No)->click();
        });
        dialog.findChild<QPushButton *>(QStringLiteral("searchLoadCollection"))->click();
        QCOMPARE(SearchCollection::load(group, &error), (QList<QUrl>{b}));
        QTimer::singleShot(0, [] {
            auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            QVERIFY(box);
            box->button(QMessageBox::Yes)->click();
        });
        dialog.findChild<QPushButton *>(QStringLiteral("searchLoadCollection"))->click();
        QCOMPARE(SearchCollection::load(group, &error), (QList<QUrl>{a}));
        QVERIFY(SearchCollection::save(group, {b}, &error));
        QTimer::singleShot(0, [] {
            auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            QVERIFY(box);
            box->button(QMessageBox::Yes)->click();
        });
        dialog.findChild<QPushButton *>(QStringLiteral("searchUpdateCollection"))->click();
        QCOMPARE(SearchCollection::loadSaved(group, &error).first().urls, (QList<QUrl>{b}));
        QVERIFY(SearchCollection::save(group, {a}, &error));
        QTimer::singleShot(0, [&] {
            QString updateError;
            QVERIFY(SearchCollection::saveSaved(group, {{QStringLiteral("First"), {a, b}}}, &updateError));
            auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            QVERIFY(box);
            box->button(QMessageBox::Yes)->click();
        });
        dialog.findChild<QPushButton *>(QStringLiteral("searchLoadCollection"))->click();
        QVERIFY(dialog.findChild<QLabel *>(QStringLiteral("searchStatus"))->text().contains(QStringLiteral("changed elsewhere")));
        QCOMPARE(SearchCollection::load(group, &error), (QList<QUrl>{a}));
        config->reparseConfiguration();
        SearchWidget restored(QUrl::fromLocalFile(root.path()), nullptr, config);
        QCOMPARE(restored.findChild<QComboBox *>(QStringLiteral("searchSavedCollections"))->currentText(), QStringLiteral("First"));
        QCOMPARE(SearchCollection::load(group, &error), (QList<QUrl>{a}));
    }

    void collectsReferencesAcrossSearchesWithoutMovingFiles()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QVERIFY(QDir(root.path()).mkdir(QStringLiteral("first")));
        QVERIFY(QDir(root.path()).mkdir(QStringLiteral("second")));
        const QString first = root.filePath(QStringLiteral("first/match-a"));
        const QString second = root.filePath(QStringLiteral("second/match-b"));
        QFile a(first);
        QFile b(second);
        QVERIFY(a.open(QIODevice::WriteOnly));
        a.close();
        QVERIFY(b.open(QIODevice::WriteOnly));
        b.close();
        const auto config = KSharedConfig::openConfig(root.filePath(QStringLiteral("searchesrc")), KConfig::SimpleConfig);
        for (const QString &folder : {root.filePath(QStringLiteral("first")), root.filePath(QStringLiteral("second"))}) {
            SearchWidget dialog(QUrl::fromLocalFile(folder), nullptr, config);
            dialog.findChild<QLineEdit *>(QStringLiteral("searchQuery"))->setText(QStringLiteral("match"));
            auto *start = dialog.findChild<QPushButton *>(QStringLiteral("searchStart"));
            start->click();
            QTRY_VERIFY(start->isEnabled());
            auto *results = dialog.findChild<QTreeWidget *>(QStringLiteral("searchResults"));
            QCOMPARE(results->topLevelItemCount(), 1);
            results->setCurrentItem(results->topLevelItem(0));
            dialog.findChild<QPushButton *>(QStringLiteral("searchCollect"))->click();
        }
        config->reparseConfiguration();
        SearchWidget dialog(QUrl::fromLocalFile(root.path()), nullptr, config);
        dialog.findChild<QTabWidget *>(QStringLiteral("searchResultTabs"))->setCurrentIndex(1);
        auto *collection = dialog.findChild<QTreeWidget *>(QStringLiteral("searchCollection"));
        QCOMPARE(collection->topLevelItemCount(), 2);
        QCOMPARE(collection->topLevelItem(0)->text(0), first);
        QCOMPARE(collection->topLevelItem(1)->text(0), second);
        collection->setCurrentItem(collection->topLevelItem(0));
        QSignalSpy reveal(&dialog, &SearchWidget::revealRequested);
        dialog.findChild<QPushButton *>(QStringLiteral("searchReveal"))->click();
        QCOMPARE(reveal.size(), 1);
        QCOMPARE(reveal.first().first().toUrl(), QUrl::fromLocalFile(first));
        dialog.findChild<QPushButton *>(QStringLiteral("searchUncollect"))->click();
        QCOMPARE(collection->topLevelItemCount(), 1);
        QVERIFY(QFile::exists(first) && QFile::exists(second));
    }
};

QTEST_MAIN(SearchWidgetTest)
#include "searchwidgettest.moc"
