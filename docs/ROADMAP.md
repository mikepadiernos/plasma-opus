# Plasma Opus roadmap

[![Docs index](https://img.shields.io/badge/docs-index-3daee9?style=flat-square)](README.md)
[![Scope](https://img.shields.io/badge/philosophy-scope-8957e5?style=flat-square)](philosophy/scope.md)
[![Inspirations](https://img.shields.io/badge/philosophy-inspirations-8957e5?style=flat-square)](philosophy/inspirations.md)
[![Project board](https://img.shields.io/badge/project-board-6e7781?style=flat-square&logo=github)](https://github.com/users/mikepadiernos/projects/3)

This is the authoritative milestone and architecture plan for Plasma Opus, a
dual-pane KDE Plasma file manager built with C++20, Qt 6 Widgets and KDE
Frameworks 6. It lists every planned feature in its intended order and records
which are complete. The [upstream inventories](README.md#behaviour-references)
record the Max File Manager behaviour that informs its parity goals.

> [!NOTE]
> The latest published MVP is
> [v0.2.0](https://github.com/mikepadiernos/plasma-opus/releases/tag/v0.2.0),
> released after M14. M15–M31 have since been merged into `main`; M32 is
> planned. See [pending tasks](TODO.md) for blocked work and branch-specific
> handoffs.

## At a glance

Status reflects the milestones' parent issues on the
[project board](https://github.com/users/mikepadiernos/projects/3): Merged
groups have every feature checked and merged into `main`, Development groups
have a milestone in progress, and Planned groups have not started. M27–M31
are merged, and their Plasma checks are recorded in [pending tasks](TODO.md).

| Milestones | Focus                                        | Status                                                                    | Jump to                         |
| ---------- | -------------------------------------------- | ------------------------------------------------------------------------- | ------------------------------- |
| M01–M04    | Core dual pane and listing depth             | ![Merged](https://img.shields.io/badge/Merged-8957e5?style=flat-square)   | [M01](#m01-core-dual-pane)      |
| M05–M08    | Preview, gallery and editor                  | ![Merged](https://img.shields.io/badge/Merged-8957e5?style=flat-square)   | [M05](#m05-preview-and-gallery) |
| M09–M14    | Tools, interface customization, translations | ![Merged](https://img.shields.io/badge/Merged-8957e5?style=flat-square)   | [M09](#m09-tools)               |
| M15–M16    | Workspaces and their settings                | ![Merged](https://img.shields.io/badge/Merged-8957e5?style=flat-square)   | [M15](#m15-workspaces)          |
| M17–M18    | Themes and their settings                    | ![Merged](https://img.shields.io/badge/Merged-8957e5?style=flat-square)   | [M17](#m17-themes)              |
| M19–M20    | Branding and its settings                    | ![Merged](https://img.shields.io/badge/Merged-8957e5?style=flat-square)   | [M19](#m19-branding)            |
| M21–M22    | Advanced search and its settings             | ![Merged](https://img.shields.io/badge/Merged-8957e5?style=flat-square)   | [M21](#m21-advanced-search)     |
| M23–M24    | Native tagging and its settings              | ![Merged](https://img.shields.io/badge/Merged-8957e5?style=flat-square)   | [M23](#m23-native-tagging)      |
| M25–M26    | Yazi mode and its settings                   | ![Merged](https://img.shields.io/badge/Merged-8957e5?style=flat-square)   | [M25](#m25-yazi-mode)           |
| M27–M31    | Integrations, Nextcloud and shortcuts        | ![Merged](https://img.shields.io/badge/Merged-8957e5?style=flat-square)   | [M27](#m27-integrations)        |
| M32        | Refinements                                  | ![Planned](https://img.shields.io/badge/Planned-6e7781?style=flat-square) | [M32](#m32-refinements)         |

## Contents

- [At a glance](#at-a-glance)
- [Architecture](#architecture) ·
  [Plasma-native decisions](#plasma-native-decisions)
- [Milestones](#milestones)
  - Core dual pane and listing depth: [M01](#m01-core-dual-pane) ·
    [M02](#m02-core-dual-pane-settings) · [M03](#m03-listing-depth) ·
    [M04](#m04-listing-depth-settings)
  - Preview, gallery and editor: [M05](#m05-preview-and-gallery) ·
    [M06](#m06-preview-and-gallery-settings) · [M07](#m07-editor) ·
    [M08](#m08-editor-settings)
  - Tools, interface customization and translations: [M09](#m09-tools) ·
    [M10](#m10-tools-settings) · [M11](#m11-ui-customizations) ·
    [M12](#m12-ui-customization-settings) · [M13](#m13-translations) ·
    [M14](#m14-translation-settings)
  - Workspaces: [M15](#m15-workspaces) · [M16](#m16-workspace-settings)
  - Themes: [M17](#m17-themes) · [M18](#m18-theme-settings)
  - Branding: [M19](#m19-branding) · [M20](#m20-branding-settings)
  - Advanced search: [M21](#m21-advanced-search) ·
    [M22](#m22-advanced-search-settings)
  - Native tagging: [M23](#m23-native-tagging) ·
    [M24](#m24-tagging-settings)
  - Yazi mode: [M25](#m25-yazi-mode) · [M26](#m26-yazi-mode-settings)
  - Integrations, Nextcloud and shortcuts: [M27](#m27-integrations) ·
    [M28](#m28-integration-settings) · [M29](#m29-nextcloud-integration) ·
    [M30](#m30-nextcloud-settings) · [M31](#m31-keyboard-shortcut-settings)
  - Refinements: [M32](#m32-refinements)
- [Feature references](#feature-references) · [Tracking](#tracking)

## Architecture

| Path                | Contents                                                                                                   |
| ------------------- | ---------------------------------------------------------------------------------------------------------- |
| `src/core/`         | The `opusfm_core` static library: logic without widgets                                                    |
| `src/app/`          | Widgets, built as the `opusfm_app` static library, and the `opus-file-manager` executable                  |
| `tests/`            | QtTest suites run through `ctest`; GUI tests use `QT_QPA_PLATFORM=offscreen`                               |
| `rust/searchquery/` | The M21 Rust query-parser pilot, built only with the `OPUSFM_RUST_PROTOTYPE` CMake option (off by default) |

Rust is a candidate for self-contained core logic, including measured
performance hotspots, not a wholesale rewrite of C++ features. Widgets, settings
and file operations stay in Qt/KDE, including KIO for local and remote
operations. The M21 Rust pilot can inform later work, such as M09 duplicate
detection and rename or sync planning; this evaluation does not block M09 or
change KIO's role. See the [pilot evaluation](rust-prototype.md) for its
findings and the criteria for later Rust components.

Qt Widgets is the user interface toolkit. Qt Quick may be embedded only for a
self-contained, optional component that Widgets cannot reasonably provide,
hosted in a `QQuickWidget` with a Widgets fallback. Embedded mpv playback in
M27 draws into a `QOpenGLWidget` instead, so no Qt Quick code ships. See
[Qt Widgets and Qt Quick](qt-quick.md) for the decision, the embedding rules
and the criteria for new candidates.

## Plasma-native decisions

Where the upstream file manager implements its own infrastructure, Plasma Opus
uses the matching Plasma or KDE Frameworks facility instead:

| Upstream approach                            | Plasma Opus approach                                                                                         |
| -------------------------------------------- | ------------------------------------------------------------------------------------------------------------ |
| Custom themes, title bars and window buttons | Plasma widget style, colour scheme, fonts and KWin decorations by default; optional app-local colours in M17 |
| Bundled icon pack                            | Active icon theme through `QIcon::fromTheme`                                                                 |
| Hand-written toolbar and shortcut editors    | KXmlGui, `KEditToolBar` and `KShortcutsDialog`                                                               |
| Settings in SQLite                           | KConfig; session state in the KConfig state file                                                             |
| Custom copy engine and undo                  | KIO jobs and `KIO::FileUndoManager`                                                                          |
| Custom places and device polling             | `KFilePlacesView` and Solid                                                                                  |
| Custom Open With and default-app logic       | `KFileItemActions`, `KOpenWithDialog` and `KApplicationTrader`                                               |
| Custom properties and checksums              | `KPropertiesDialog`                                                                                          |
| Custom clipboard helpers                     | `QClipboard` and `KIO::setClipboardDataCut`                                                                  |

Plasma Opus uses a supported subset of the upstream `.mfm.json` schema for local
per-folder metadata, including pins, labels, ratings, notes, colours and view
formats. See [metadata compatibility](CAVEATS.md#folder-metadata-and-listings)
for limitations on writes and unsupported behaviour.

## Milestones

Each milestone links to its parent issue on the project board, summarizes its
scope and lists its features, followed by links to any known caveats. Where
applicable, a feature milestone is followed by a settings milestone whose
options are stored in KConfig. Milestones appear in the intended work order;
within M15–M32, features are ordered by priority, effort and dependencies within
their groups.

A checked box marks a feature whose issue is Completed or Merged. An unchecked
box marks a feature that is not started, in progress, or implemented but still
in Review.

### M01: Core dual pane

**Project:**
[M01 milestone](https://github.com/mikepadiernos/plasma-opus/issues/12)

The foundation: a KXmlGui main window with two file panes built on KIO, the
Places panel, KDE file actions and session restore.

- [x] KXmlGui main window with hamburger menu, editable toolbar and shortcuts
      [#38]
- [x] Theme icons for app actions and pane controls, preferring symbolic
      variants when available and preserving file and device emblems [#39]
- [x] Two panes with tabs, back/forward history and a `KUrlNavigator` [#40]
- [x] Details and icon views over `KDirModel`, with previews [#41]
- [x] Places panel (`KFilePlacesView`: devices, mount, eject) [#42]
- [x] Open, Open With and service menus (`KFileItemActions`) [#43]
- [x] Copy and move to the other pane (<kbd>F5</kbd>/<kbd>F6</kbd>) [#44]
- [x] Cut, copy, paste and copy path via the clipboard [#45]
- [x] Drag and drop between panes and onto Places [#46]
- [x] Move to trash, delete, rename, new folder and new file templates [#47]
- [x] Undo through `KIO::FileUndoManager` [#48]
- [x] Show hidden files [#49]
- [x] Open terminal here [#50]
- [x] Properties dialog (`KPropertiesDialog`, includes checksums) [#51]
- [x] Free space in the status bar [#52]
- [x] Single instance (`KDBusService`) [#53]
- [x] Session restore of panes, tabs and history [#54]
- [x] Stacked dual-pane layout: panes split top/bottom with both tab bars
      meeting in the middle (top pane's tabs along its bottom edge, bottom
      pane's tabs along its top edge) [#55]

### M02: Core dual pane settings

**Project:**
[M02 milestone](https://github.com/mikepadiernos/plasma-opus/issues/13)

Settings for M01 features, modelled on Dolphin and Orcinus. Plasma Opus settings
are stored in KConfig; Places sections and trash/delete confirmations are shared
with Dolphin.

- **General**

  - [x] **Settings Dialog:** sidebar icon size (default 24 px) and title
        position (to the right of icons by default, or below) [#56]

- **Interface**

  - [x] **Folders and Tabs:** startup folders and session; one or two panes in
        new windows; tab bars; Tab switches panes; new-tab placement; navigation
        bars; symbolic-link following [#57]
  - [x] **Previews:** enablement, plugins, size limit and remote folders [#58]
  - [x] **Confirmations:** trash, delete, multiple tabs; executable handling;
        default drop action [#59]
  - [x] **Panels:** Places panel visibility and visible sections; optional focus
        return to the active pane after panel navigation [#60]
  - [x] **Status and Location Bars:** status-bar contents and bar density,
        including custom bar spacing [#61]

- **View**

  - [x] **General:** default view mode, sorting, hidden files and tooltips [#62]
  - [x] **Details:** icon size, columns, item density and dragging from anywhere
        in a row [#63]
  - [x] **Icons:** icon and preview size and item density [#64]

- **Context Menu**

  - [x] **General:** optional Copy To and Move To submenus, independently
        configurable alongside the additional KDE submenus [#65]
  - [x] **Actions:** choose which built-in actions appear (Plasma Opus actions;
        additional KDE transfer submenus are controlled separately) [#66]
  - [x] **Service Menus:** choose which installed service actions and plugins
        appear in Plasma Opus separately from Dolphin-shared settings [#67]

### M03: Listing depth

**Project:**
[M03 milestone](https://github.com/mikepadiernos/plasma-opus/issues/14)

Deepen each listing with extra columns, filtering, per-folder formats and
metadata, archive handling, richer tabs and folder sizes, drawing on Dolphin,
Krusader and Orcinus.

- [x] Combined Places/Folders panel: combines Places and folder navigation in
      one panel by default instead of showing separate Places and Folders
      panels. Favorites replace bookmarks. [#68]
- [x] Extra columns and Dolphin-style grouping [#69]
- [x] Filter syntax and type-ahead [#70]
- [x] Per-folder view format [#71]
- [x] Labels, ratings, notes, pins and colours [#72]
- [x] Folder look dialog (icon and colour) [#73]
- [x] Ignore list [#74]
- [x] Tree pane [#75]
- [x] Show Flat (<kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>F</kbd>): list every file below the current folder in one
      list without folders. Navigation, tabs and history remain rooted at that
      folder; sorting, filtering, previews and file operations continue to work.
      In details view, Type becomes Relative Path. The mode is remembered per
      folder; Create New, New Folder and Paste are disabled, and dropping files
      is refused. [#76]
- [x] Browse archives and extract files [#77]
- [x] Compress local files: choose an archive name, format and destination (this
      folder or the other pane); never overwrite an existing file. Run in the
      background and report success or errors in the status bar. Supported
      formats are ZIP, uncompressed ZIP, tar.gz, tar.xz and tar.zst. [#78]
- [x] Tabs: show folder icons; fill the tab-bar width equally as tabs are added;
      pin/lock behaviour; custom tab names; folder-based colours; restore tabs
      with the session [#79]
- [x] Select group: select or deselect by pattern, with saved selection filters
      (Krusader) [#80]
- [x] Occupied space of files, folders, archives and remote locations (Krusader)
      [#81]
- [x] Popular folders and a jump-back point (Krusader) [#82]
- [x] Archive integrity testing for local ZIP, TAR and 7z files, with password
      prompts for encrypted archives (Krusader) [#83]
- [x] Password-protected AES ZIP creation and extraction (Krusader) [#84]
- [x] Password-protected 7z extraction [#298]
- [x] Display style per pane (Orcinus) [#85]
- [x] Expandable folders inline in the file listing (Orcinus) [#86]
- [x] Persistent filters (Orcinus) [#87]
- [x] File-name highlighting (Orcinus) [#88]
- [x] Compact view with names beside icons and entries arranged in columns
      (Dolphin) [#89]
- [x] Optional selection markers and a selection mode with an action bar, so
      multiple items can be selected without modifier keys (Dolphin) [#90]
- [x] Details-view column-header controls to show or hide columns and fit column
      widths to their contents (Dolphin) [#91]
- [x] Folder-format rules for a folder and optionally its descendants, with
      explicit precedence over defaults; show which rule is active and allow a
      pane to temporarily lock its current format [#92]
- [x] Restore selection and scroll position when returning through a tab's
      navigation history, ignoring items that no longer exist [#93]
- [x] Calculate folder sizes in the background for display in the listing, on
      demand or automatically according to location and folder-format rules;
      optionally refresh calculated sizes periodically [#94]

**Caveats:** [Archives](CAVEATS.md#archives) and
[folder metadata and listings](CAVEATS.md#folder-metadata-and-listings). Tracked
in [#36], [#255], [#256], [#261], [#262], [#263] and [#265].

### M04: Listing depth settings

**Project:**
[M04 milestone](https://github.com/mikepadiernos/plasma-opus/issues/15)

Settings for M03 listing depth, stored in KConfig.

- **View**

  - [x] **General:** default visible columns and grouping; size format (human or
        bytes), date format and seconds [#95]
  - [x] **Icons:** name lines and metadata shown in icon view [#96]
  - [x] **Details:** enable inline folder expansion and choose automatic or
        manual column widths by default [#97]
  - [x] **Compact:** icon size, item spacing and additional information [#98]
  - [x] **Selection:** default selection markers and selection mode [#99]
  - [x] **Filters:** default scope, type-ahead activation, filter retention when
        changing folders, and file-name match highlighting [#100]
  - [x] **Folder formats:** manage inherited rules and default size calculation;
        allow separate local and remote policies [#101]
  - [x] **Folder sizes:** optional periodic refresh of calculated sizes [#102]
  - [x] **History:** restore selection and scroll position when navigating back
        or forward [#103]

- **Files**

  - [x] **Rules:** ignore list entries and file-type colours by extension [#104]
  - [x] **Metadata:** show or hide label, rating, colour and pin indicators;
        choose whether pinned items appear first without changing stored
        metadata [#105]
  - [x] **Selection filters:** manage saved selection patterns [#106]

- **Panels**

  - [x] **Combined Places/Folders:** show or hide the panel, choose its left or
        right position, configure its auto-focus behaviour, and show or hide
        each section independently [#107]
  - [x] **Titles:** independently show or hide navigation and embedded panel
        titles while retaining panel controls and document status [#108]

- **Archives**

  - [x] **Default format:** archive format used by Compress (ZIP, uncompressed
        ZIP, tar.gz, tar.xz or tar.zst); the format choice determines
        compression [#109]
  - [x] **Destination:** default to the current folder or the other pane when
        creating an archive, retaining the non-overwriting behaviour [#110]

- **Tabs**

  - [x] **Appearance:** show tab icons; fill the available tab-bar width and
        divide it equally among tabs as new tabs are created [#111]
  - [x] **Colours:** show or hide folder-based tab colours without removing them
        [#112]

**Caveats:** [Folder metadata and listings](CAVEATS.md#folder-metadata-and-listings),
tracked in [#262] and [#263].

### M05: Preview and gallery

**Project:**
[M05 milestone](https://github.com/mikepadiernos/plasma-opus/issues/16)

Preview files in a panel that follows the selection, browse images and videos
in a local gallery, and edit or convert images without leaving Plasma Opus.

- [x] Preview panel: images (up to 8 MiB and 40 megapixels) and UTF-8 text
      (first 1 MiB by default), PDF and office documents with an installed
      KPart, and audio/video with an installed Qt Multimedia backend, following
      the active selection (<kbd>F3</kbd>) [#113]
- [x] Remote previews: files on KIO locations are downloaded to a temporary file
      first, within the same image and text limits and up to 128 MiB for other
      files [#114]
- [x] Preview image navigation, gallery and Open controls; formatted HTML and
      Markdown with a source toggle (external resources and links are disabled)
      [#115]
- [x] Local gallery with zoom, a looping slideshow, thumbnails, image
      information, EXIF metadata, dominant colours and a luminance histogram
      (<kbd>Space</kbd>); follows the pane's sort order or the selected images
      [#116]
- [x] Animated images and video playback in the gallery [#117]
- [x] Rotate, flip, crop, resize and adjust colours (brightness, contrast,
      saturation, greyscale); save a PNG/JPEG/WebP copy or replace the original
      [#118]
- [x] Save-copy format selection and JPEG/WebP quality [#119]
- [x] Line-numbered, syntax-highlighted text previews and preview footer
      ID3/EXIF metadata [#120]
- [x] Mark images while browsing the gallery and apply file operations to the
      marked selection, without moving files until the user chooses an action
      (Directory Opus) [#121]
- [x] Batch image conversion with output format, dimensions and quality, a
      destination preview and non-overwriting names (Max) [#122]
- [x] Optional autoplay for audio and video in the preview panel; keep
      click-to-play available independently of gallery playback [#123]

**Caveats:** [Previews and gallery](CAVEATS.md#previews-and-gallery), tracked in
[#259], [#260], [#264], [#266], [#267] and [#303].

### M06: Preview and gallery settings

**Project:**
[M06 milestone](https://github.com/mikepadiernos/plasma-opus/issues/17)

Settings for M05 preview and gallery, stored in KConfig.

- **Opening**

  - [x] **Activation:** what activating a file opens (application, gallery or
        preview) [#124]

- **Preview**

  - [x] **Text:** size limit for text in the preview panel [#125]
  - [x] **Presentation:** default to formatted or source view for HTML and
        Markdown; show line numbers and syntax highlighting in text previews
        [#126]
  - [x] **Media:** autoplay audio and video in the preview panel or require play
        [#127]

- **Gallery**

  - [x] **Information:** image info panel shown by default [#128]
  - [x] **Display:** initial zoom (fit or actual size) and thumbnail visibility
        [#129]

- **Slideshow**

  - [x] **Playback:** slideshow interval and looping [#130]

- **Image conversion**

  - [x] **Output:** default format and quality for saved image copies [#131]
  - [x] **Batch:** default dimensions, format, quality and destination for batch
        conversion; keep output names non-overwriting [#132]

File activation follows the desktop's single- or double-click setting and also
applies to keyboard activation. Gallery mode opens local images and videos;
other files open in their default application. Folders and executable handling
are unchanged, and the preview panel's Open button opens the application.

**Caveats:** [Previews and gallery](CAVEATS.md#previews-and-gallery), tracked in
[#260], [#264], [#266] and [#303].

### M07: Editor

**Project:**
[M07 milestone](https://github.com/mikepadiernos/plasma-opus/issues/18)

Edit text files in an embedded KTextEditor-based editor, with drafts, Markdown
preview and PDF export.

- [x] KTextEditor-based editor: use it as a panel alongside Information or
      Preview, or as a separate embedded area below or to the right of both
      panes, like the embedded terminal. Neither embedded placement replaces a
      pane. Also support maximized and standalone editor windows. [#133]
- [x] Drafts [#134]
- [x] Markdown preview and PDF export [#135]

**Caveats:** [Editor](CAVEATS.md#editor), tracked in [#267] and [#307].

### M08: Editor settings

**Project:**
[M08 milestone](https://github.com/mikepadiernos/plasma-opus/issues/19)

Settings for M07 editor, stored in KConfig and KTextEditor configuration.

- **Editor**

  - [x] **Opening:** where files open (panel, separate embedded area, maximized
        or own window) [#136]
  - [x] **Embedded placement:** below or to the right of both panes when using a
        separate embedded area [#137]
  - [x] **Text:** font size and word wrap [#138]
  - [x] **Markdown:** default markdown mode (visual or source) [#139]
  - [x] **Drafts:** drafts on or off [#140]
  - [x] **Recovery:** draft autosave interval and retention; store drafts
        privately and allow them to be reviewed or discarded without overwriting
        the original files [#141]
  - [x] **PDF export:** default page size and margins for Markdown exports
        [#142]

- **KTextEditor**

  - [x] **Configuration:** KTextEditor's own configuration pages [#143]

**Caveats:** [Editor](CAVEATS.md#editor), tracked in [#267], [#307] and [#309].

### M09: Tools

**Project:**
[M09 milestone](https://github.com/mikepadiernos/plasma-opus/issues/20)

Add file-management tools: search, duplicate finding, folder comparison and
synchronization, batch renaming, metadata editing, scripts and custom commands,
a transfer queue and an embedded terminal.

- [x] Live search and indexed search: choose filename or content search and
      current-folder or indexed scope; indicate when locations are not indexed,
      show progress and allow cancellation (Dolphin, Max) [#144]
- [x] Search results: reveal an item in its containing folder and save searches;
      optionally collect references from different locations without moving the
      original files (Dolphin, Directory Opus) [#145]
- [x] Duplicate finder: preview groups and comparison criteria before selecting
      files for removal; never delete automatically (Directory Opus) [#146]
- [x] Duplicate selection: choose candidates by location or date while keeping
      at least one item per group in automatic selection; warn and require
      explicit confirmation if a manual selection would remove every item in a
      group [#147]
- [x] Search and selection filters for full path, parent folder and excluded
      subtrees, with an optional recursion-depth limit [#148]
- [x] Content chart (folder breakdown) [#149]
- [x] Folder sync: preview differences and planned operations before running;
      allow per-item direction changes and exclusions, with explicit
      confirmation of overwrites and deletions (Krusader) [#150]
- [x] Folder comparison and sync filters: include/exclude patterns, recursion,
      symlink handling and optional content comparison; save reusable profiles
      (Krusader) [#151]
- [x] File diff [#152]
- [x] Batch rename: preview old and new names, detect collisions before
      applying, and support metadata tokens for images and audio (Max) [#153]
- [x] Batch rename rules for regular expressions and reusable named presets
      [#154]
- [x] Symlinks [#155]
- [x] Listing export to CSV and PDF (Max) [#156]
- [x] Edit supported audio tags and image metadata, including a preview of
      changes and clear reporting when a format cannot be written (Directory
      Opus) [#157]
- [x] Scripts and custom commands [#158]
- [x] Command presets: save named script or custom-command configurations and
      placeholder defaults; let users review commands and arguments before
      running. Keep rename and synchronization profiles specific to their tools.
      [#159]
- [x] Compare folders across the two panes (Krusader) [#160]
- [x] Create and verify checksum files (Krusader) [#161]
- [x] Split and combine files (Krusader) [#162]
- [x] Transfer queue with per-job progress, speed, errors and pause, resume and
      cancellation controls where supported by the underlying KIO job (Krusader,
      Max) [#163]
- [x] Embedded terminal alongside both panes: below them or at the right of the
      app for portrait/stacked layouts; showing it never replaces either pane
      [#164]
- [x] Placeholders for scripts and custom commands [#165]
- [x] Linked panes and cushion treemap (Orcinus) [#166]

**Caveats:** [File comparison](CAVEATS.md#file-comparison), tracked in [#311];
[symbolic links](CAVEATS.md#symbolic-links), tracked in [#312];
[listing export](CAVEATS.md#listing-export), tracked in [#314];
[folder content chart](CAVEATS.md#folder-content-chart), tracked in [#315];
[checksum files](CAVEATS.md#checksum-files), tracked in [#316];
[pane folder comparison](CAVEATS.md#pane-folder-comparison), tracked in [#317];
[split and combine files](CAVEATS.md#split-and-combine-files), tracked in
[#318]; [embedded terminal](CAVEATS.md#embedded-terminal), tracked in [#321];
[linked pane navigation](CAVEATS.md#linked-pane-navigation), tracked in [#322];
[cushion treemap](CAVEATS.md#cushion-treemap), tracked in [#335];
[duplicate finder](CAVEATS.md#duplicate-finder), tracked in [#336];
[search](CAVEATS.md#search), tracked in [#362];
[batch rename](CAVEATS.md#batch-rename), tracked in [#372];
[custom commands](CAVEATS.md#custom-commands), tracked in [#373];
[metadata editing](CAVEATS.md#metadata-editing), tracked in [#374];
[transfer queue](CAVEATS.md#transfer-queue), tracked in [#375];
[filtered folder comparison](CAVEATS.md#filtered-folder-comparison), tracked in
[#376]; [folder synchronization](CAVEATS.md#folder-synchronization), tracked in
[#377].

### M10: Tools settings

**Project:**
[M10 milestone](https://github.com/mikepadiernos/plasma-opus/issues/21)

Settings for M09 tools, stored in KConfig.

- **Embedded terminal**

  - [x] **Placement:** below both panes or at the right of both panes for
        portrait/stacked layouts; the terminal does not replace a pane [#167]
  - [x] **Working folder:** optionally follow the active pane's local folder;
        report unsupported locations rather than silently switching folders
        [#168]

- **Search**

  - [x] **Defaults:** search scope and whether to search filenames or contents
        [#170]
  - [x] **Filters:** reusable path and subtree filters and recursion depth for
        search and selection [#171]
  - [x] **Saved searches:** manage saved searches and reusable result
        collections [#172]

- **Duplicate finder**

  - [x] **Defaults:** comparison criteria and selection preferences; retain the
        safeguard that automatic selection keeps at least one item per group
        [#173]

- **Content chart and cushion treemap**

  - [x] **Display:** choose whether to show the treemap and the default table
        sort order; keep scan errors and skipped entries visible [#354]

- **Linked panes**

  - [x] **Startup:** optionally link the initial pane folders; default to
        unlinked, validate local roots and report when linking is unavailable
        [#355]

- **File diff**

  - [x] **Display:** choose whether to wrap lines in the comparison output
        without changing the comparison itself [#356]

- **Pane folder comparison**

  - [x] **Results:** choose the initial result categories shown; keep errors and
        incomplete comparisons visible and require explicit selection in the
        panes [#357]

- **Folder sync**

  - [x] **Transfers:** skip existing, newer only, speed limit [#174]
  - [x] **Comparison:** default recursion, symlink and content-comparison
        options; saved synchronization profiles [#175]

- **Batch rename**

  - [x] **Presets:** manage named rename rules and choose a default preset;
        retain the M09 collision preview before applying changes [#176]

- **Listing export**

  - [x] **Defaults:** output format and export-specific columns, initially based
        on the current listing columns [#177]

- **Symbolic links**

  - [x] **Naming:** default link-name suffix, editable before creation; preserve
        existing destination checks and never overwrite an existing entry [#358]

- **Checksum files**

  - [x] **Defaults:** default manifest filename for creation, editable before
        saving; retain the supported SHA-256 format and existing validation
        [#359]

- **Split and combine**

  - [x] **Defaults:** default split-part size, editable for each operation;
        retain part-order review and never overwrite existing outputs [#360]

- **Metadata editing**

  - [x] **Fields:** choose the initially displayed editable audio and image
        fields when the editor is available; retain the change preview and
        report unsupported writes [#361]

- **Transfer queue**

  - [x] **Behaviour:** queue and concurrency defaults; conflict handling [#178]

- **Applications**

  - [x] **Associations:** manage desktop default applications by extension
        through KDE facilities; make changes to shared desktop defaults explicit
        [#179]

- **Automation**

  - [x] **Commands:** scripts, custom commands and presets [#180]

**Caveats:** The [M09 tools caveats](#m09-tools) also apply to these settings.

### M11: UI Customizations

**Project:**
[M11 milestone](https://github.com/mikepadiernos/plasma-opus/issues/22)

Let users rearrange the interface: toolbars, panes, panels, tab bars, status
bars, splitter sizes and the Information panel, with backup and restore of
app-owned settings and layouts.

- [x] Edit the main toolbar beyond the `KEditToolBar` dialog: drag entries on,
      off and around it directly, add separators and spacers, and choose icons,
      text or both per entry [#181]
- [x] Custom toolbar buttons for scripts, custom commands and presets [#182]
- [x] Pane toolbars instead of a fixed path row: each pane gets its own editable
      toolbar (back, forward, up, reload, location bar, view mode, zoom,
      sorting, previews, hidden files, filter, search) that acts on that pane,
      with the location bar dropping to a second row in a narrow pane [#183]
- [x] Move panes: send the second pane to a tab of its own or pop it out into a
      window, carrying its display style [#184]
- [x] Drag entries between the main toolbar, pane toolbars and hamburger menu
      [#185]
- [x] Customize the combined Places/Folders panel: reorder its sections, show or
      hide each section, and give sections custom titles [#186]
- [x] Back up and restore app-owned settings and layouts with a preview and
      confirmation; exclude recognized credentials and shared KDE settings, and
      avoid overwriting running configuration without a safe reload or restart
      [#187]
- [x] Swap panes: the current tabs or whole sides [#188]
- [x] Dim the inactive pane [#189]
- [x] Customize folder tab bars: place them above, below, left or right of each
      pane, with a together option for stacked panes; choose close-button
      visibility without bypassing pinned or locked tab protections. Keep
      keyboard and context-menu actions accessible. (Directory Opus) [#323]
- [x] Customize status bars: choose a shared bar or one per pane, and arrange
      available item counts, selection size and free-space indicators. Bind each
      pane's values to that pane and keep operation errors visible even when
      optional indicators are hidden. Reuse the existing status-bar preferences.
      (Directory Opus, Dolphin) [#324]
- [x] Save and reset pane and panel splitter sizes, including preview, editor
      and terminal areas, independently for side-by-side and stacked layouts.
      Restore sizes safely within the available window; preserve both file panes
      and coordinate with session restore and workspace layouts. (Max) [#325]
- [x] Customize the preview and gallery background for transparent images:
      follow the active Plasma theme, or choose light, dark or checkerboard.
      Keep the image data unchanged. (Directory Opus) [#326]
- [x] Customize the Information panel's visible metadata fields and their order,
      and optionally show information for a hovered item instead of the current
      selection. Make hover behaviour opt-in and retain selection-based
      fallback; show unavailable metadata without initiating additional remote
      downloads. (Dolphin) [#327]
- [x] Optional file-list minimap on the right edge of each explorer pane: show a
      compact overview of the current visible files and folders and the scroll
      viewport, with click or drag navigation. Follow the pane's current tab,
      filter, sort and view; keep normal scrolling and keyboard navigation
      available. Bound rendering work for large or remote listings and do not
      read file contents or trigger additional remote downloads. [#334]

**Caveats:** [File-list minimap](CAVEATS.md#file-list-minimap), tracked in
[#434]; [settings backups](CAVEATS.md#settings-backups), tracked in [#433].

### M12: UI Customization settings

**Project:**
[M12 milestone](https://github.com/mikepadiernos/plasma-opus/issues/23)

Settings for M11 UI customizations, stored in KConfig.

- **Toolbars**

  - [x] **Entries:** which entries the main toolbar and each pane toolbar show,
        and in what order; customize hamburger menu entries and their order
        [#190]
  - [x] **Appearance:** toolbar style and icon size [#191]
  - [x] **Entry appearance:** icon, text or both for individual toolbar entries
        [#192]
  - [x] **Editing:** lock toolbars [#193]

- **Panes**

  - [x] **Moving:** whether a moved pane opens as a tab or window, and whether
        it keeps or resets its display style [#194]
  - [x] **Swapping:** swap current tabs or whole pane sides by default [#195]
  - [x] **Inactive pane:** enable dimming and adjust its intensity [#196]

- **Combined panel**

  - [x] **Sections:** section order, visibility and custom titles, reusing the
        visibility settings from M04 [#197]

- **Tabs**

  - [x] **Placement and controls:** tab-bar position by pane orientation,
        together placement for stacked panes, and close-button visibility [#328]

- **Status bars**

  - [x] **Layout:** shared or per-pane bars, with visible indicators and their
        order; reuse existing item-count, selection-size and free-space settings
        [#329]

- **Splitter sizes**

  - [x] **Restore:** remember sizes per orientation and reset them to defaults,
        without overriding an explicitly applied workspace layout [#330]

- **Preview and gallery**

  - [x] **Background:** theme, light, dark or checkerboard for transparent
        images [#331]

- **Information panel**

  - [x] **Fields and focus:** visible metadata fields and order; opt-in hover
        information with selection fallback [#332]

- **File-list minimap**

  - [x] **Display:** enable or disable the minimap and choose its width [#333]

- **Defaults**

  - [x] **Reset:** restore the main toolbar, pane toolbars and hamburger menu
        [#198]
  - [x] **Backup:** export and restore app-owned settings and layouts with a
        preview and confirmation, excluding recognized credentials and shared
        KDE settings [#199]

**Caveats:** [File-list minimap](CAVEATS.md#file-list-minimap), tracked in
[#434]; [settings backups](CAVEATS.md#settings-backups), tracked in [#433].

### M13: Translations

**Project:**
[M13 milestone](https://github.com/mikepadiernos/plasma-opus/issues/26)

Make Plasma Opus translatable, mirror it for right-to-left languages and support
Jalali calendar dates.

- [x] Translation catalogues through `ki18n`, fed by the `i18n()` and `i18nc()`
      calls already used throughout the code; provide extraction and
      installation without requiring a translation into any particular language
      [#210]
- [x] Right-to-left layout, mirrored for languages that use it [#212]
- [x] Jalali (Persian) calendar dates [#213]

**Caveats:** [Jalali dates](CAVEATS.md#jalali-dates), tracked in [#436].

### M14: Translation settings

**Project:**
[M14 milestone](https://github.com/mikepadiernos/plasma-opus/issues/27)

Settings for M13 translations, stored in KConfig.

- **Language**

  - [x] **Interface:** follow the system language or select an interface
        language [#214]
  - [x] **Regional formats:** follow the system locale or select a locale for
        dates and numbers independently of the interface language [#215]

- **Dates**

  - [x] **Calendar:** calendar for dates: Gregorian or Jalali [#216]
  - [x] **Format:** custom date and time formats, with clear precedence over the
        listing date-format defaults from M04 [#217]

- **Layout**

  - [x] **Direction:** follow the language or force left-to-right or
        right-to-left [#218]
  - [x] **Digits:** digit shapes and custom numeral formats, including Western
        or Eastern Arabic-Indic digits [#219]

**Caveats:** [Jalali dates](CAVEATS.md#jalali-dates), tracked in [#436];
[translation format overrides](CAVEATS.md#translation-format-overrides), tracked
in [#440].

### M15: Workspaces

**Project:**
[M15 milestone](https://github.com/mikepadiernos/plasma-opus/issues/24)

Organize locations into focused workspace profiles. Each profile can include
features, layouts, a theme and other preferences; see the
[interface design](philosophy/designs.md#workspaces). Tag-group organization
belongs to the native tagging system.

- [x] Create, switch, rename, duplicate and delete named workspaces using a
      discoverable selector. Give each a full name and short label; duplication
      copies the layout without reassigning locations. [#200]
- [x] Assign saved locations to workspaces, with each location assigned to at
      most one workspace. Switching shows the active workspace's assignments
      without moving files; keep unassigned locations accessible. [#201]
- [x] Store workspace configuration locally without credentials. Workspaces
      organize the interface and search scope; they are not a security boundary.
      [#205]
- [x] Confirm before overwriting a saved workspace or discarding unsaved layout
      changes. Deleting a workspace removes associations, not the underlying
      files, locations or tags. [#204]
- [x] Scope global search to the active workspace's assigned locations by
      default, with an explicit option to search elsewhere. [#202]
- [x] Save layouts for workflows and integrations, including pane arrangements,
      views, toolbars, and placement or visibility of search, tagging, preview,
      editor and terminal areas as they become available. Report unavailable
      features or providers when applying the workspace. [#203]
- [x] Capture and restore pane and tab locations and active tabs as an explicit
      workspace layout snapshot. Preserve running jobs and confirm before
      replacing unsaved state. [#424]
- [x] Restore available components when locations or providers are unavailable.
      Identify components that could not be restored and offer a retry without
      silently replacing saved locations or changing files. [#425]
- [x] Show the active workspace and unsaved layout changes, with a
      keyboard-accessible selector and a confirmed action to restore the saved
      layout. [#423]
- [x] Extend each pane's location bar with Krusader-inspired breadcrumbs: show
      full segments or abbreviate ancestors to their first letter, retaining
      leading underscores (for example, `__path` becomes `__p`), while always
      showing the current folder's full name. Alternatively, show an editable
      full path. Navigate each breadcrumb using its actual location, reveal full
      names for abbreviations and save each pane's mode in the workspace layout.
      [#380]
- [x] Allow all, selected or none of a workspace's applicable behaviours,
      including location presentation, search scope, layouts, and pane and tab
      restoration. Preserve unselected state and existing defaults and reuse
      existing controls; M27 can apply separate choices per Activity. [#431]
- [x] Link reusable saved searches and folder comparison or synchronization
      profiles to workspaces for discovery, without copying profiles or changing
      shared defaults. Preserve and identify missing references; switching a
      workspace never runs a task. Keep synchronization previews and
      confirmations. [#585]
- [x] Import and export selected workspace profiles through the M12 backup
      workflow, with preview and collision handling. Exclude credentials and
      never execute commands or modify managed files during import. [#426]

**Caveats:** [Workspace layouts](CAVEATS.md#workspace-layouts), tracked in
[#700].

### M16: Workspace settings

**Project:**
[M16 milestone](https://github.com/mikepadiernos/plasma-opus/issues/25)

Settings for M15 workspaces, stored in KConfig.

- [x] **Management:** create, switch, rename, duplicate and delete workspaces;
      manage full names, short labels and locations. Preserve exclusive
      assignments and access to unassigned items; duplicating copies the layout
      without reassigning them. Confirm deletion and retain underlying files and
      locations. Store configuration locally without credentials and explain
      that workspaces are not a security boundary. [#206]
- [x] **Startup:** choose a startup workspace or restore the last used one
      without overriding M02 startup folder or session choices [#207]
- [x] **Search:** choose workspace-scoped or all-location search as the default,
      reusing M10 settings; allow searches outside the workspace [#208]
- [x] **Layouts:** choose which pane arrangements, views, toolbars and available
      integration panels to restore; confirm before overwriting saved layouts.
      [#209]
- [x] **Snapshots:** choose which pane and tab state to restore from an explicit
      workspace snapshot, respecting M02 startup and session choices. Preserve
      running jobs and confirm before replacing unsaved state. [#428]
- [x] **Recovery:** choose whether to prompt for retries when locations or
      providers are unavailable; identify what could not be restored and retain
      saved references until explicitly changed [#429]
- [x] **Indicators:** show the active workspace and unsaved layout state; choose
      the selector's placement without removing all ways to access it, and
      confirm before returning to the saved layout [#427]
- [x] **Pane location bars:** choose full breadcrumbs, abbreviated ancestors or
      an editable full path for each pane; restore these choices with the layout
      [#381]
- [x] **Features:** choose all, selected or none of a workspace's applicable
      behaviours, including location presentation, search scope, layouts, and
      pane and tab restoration. Preserve unselected state and existing defaults;
      reuse settings. [#432]
- [x] **Portability:** select profiles for import or export using M12 backup
      controls and review conflicts before applying changes. Exclude
      credentials; never execute commands or modify managed files during import.
      [#430]

**Caveats:** [Workspace settings](CAVEATS.md#workspace-settings), tracked in
[#609].

### M17: Themes

**Project:**
[M17 milestone](https://github.com/mikepadiernos/plasma-opus/issues/341)

Customize the appearance of Plasma Opus without changing the desktop's global
appearance. Every option follows Plasma by default, can be saved in a theme
profile and can be attached to a workspace; see the
[interface design](philosophy/designs.md#workspaces).

- **M17.01: Colour schemes**

  - [x] Follow the active Plasma colour scheme by default. Offer app-local
        light and dark presets and installed KDE schemes without changing the
        desktop's global appearance, widget style or window decorations. [#342]
  - [x] Apply app-local colours consistently across supported Plasma Opus
        windows, panes and dialogs, respecting accessibility settings, native
        icon themes and the appearance boundaries of embedded or external
        components. [#345]
  - [x] Provide an app-local colour editor with named palette roles and live
        preview. Preserve readable text, selection and focus indicators;
        validate contrast and allow changes to be reverted without saving.
        [#343]
  - [x] Follow Plasma's accent colour by default or choose an app-local accent
        for selection, focus, the active pane, progress and tab indicators.
        Validate contrast against the active colour scheme. [#611]
  - [x] Tint the background of all panes, or only the active or inactive pane,
        complementing M11 inactive pane dimming. Keep text, selection and file
        colours readable. [#612]

- **M17.02: Effects and shape**

  - [x] Optional background blur behind translucent windows and panels using
        KWin's blur effect through `KWindowEffects`; blur strength follows
        KWin's effect settings. Without KWin compositing, report blur as
        unavailable and keep an opaque background. If translucency cannot be
        changed live, apply it when the window next opens. [#613]
  - [x] Adjust background opacity separately for windows, panes and panels,
        with or without blur, while keeping text, icons and selection fully
        opaque. [#614]
  - [x] Choose a corner radius (none, small, medium, large or a custom value
        scaled with the display) for item highlights, cards, panels, input
        fields, tabs and tooltips, using custom delegates and app-local style
        rules. Leave window corners to KWin and the window decoration, and
        document controls that the widget style draws without a radius. [#615]

- **M17.03: Lines and dividers**

  - [x] Show, thin or hide visible dividers, including splitter handles, panel
        and toolbar separators, header and grid lines and pane frames. Hidden
        splitters keep a resize area, cursor feedback, keyboard resizing and
        the M11 size reset, and can optionally reveal themselves on hover.
        [#616]
  - [x] Show or hide tree lines in the tree pane and M03 inline folder
        expansion, and choose solid or dotted lines. Draw the lines in Plasma
        Opus when the widget style does not provide the chosen style. [#617]
  - [x] Colour tree lines with a single colour, a configurable palette by
        nesting depth or each folder's colour. Validate contrast and keep
        nesting distinguishable without relying on colour alone. [#618]

- **M17.04: Views**

  - [x] Add a Card view mode alongside Details, Icons and Compact. Show each
        item as a card with a preview or icon, its name and chosen metadata
        such as size, modification date, type, tags, rating and colour label.
        Support card size, columns and density, selection markers, inline
        renaming, drag and drop and keyboard navigation, and remember the mode
        wherever other view modes are remembered, including workspace layouts.
        [#619]
  - [x] Choose row striping in Details view, hover highlighting and a selection
        style such as filled, outlined, rounded or an accent bar. Keep focus and
        selection distinguishable for keyboard users. [#620]

- **M17.05: Folder colours**

  - [x] Extend M03 folder colours to tint folder icons consistently across
        views, the tree pane, places, breadcrumbs and tabs. Use the icon
        theme's coloured folder icons, such as Breeze's `folder-red`, when
        available and tint them otherwise; alternatively show a colour badge
        or coloured name. [#621]
  - [x] Colour folders by rules that match a path, name pattern or tag, with
        optional inheritance by subfolders. Explicit folder colours take
        precedence over rules, and rules never modify folders or their
        contents. [#622]

- **M17.06: Text and spacing**

  - [x] Follow Plasma's fonts by default or override the font family, size and
        weight for file lists, the tree pane, tabs and the status bar, scaled
        consistently with the display. [#623]
  - [x] Scale padding and spacing globally, extending M02 item density to
        toolbars, tabs, panels and the status bar. [#624]
  - [x] Style tabs and the status bar, including the tab shape, active tab
        indicator and the strength of M03 tab colours. [#625]

- **M17.07: Motion and accessibility**

  - [x] Follow Plasma's animation speed by default, or reduce or disable Plasma
        Opus transitions such as view changes, hover fades and panel
        animations. [#626]
  - [x] Offer to suspend blur, translucency, tints and custom corner radii with
        high-contrast colour schemes, and validate contrast for every custom
        colour. Provide a one-step reset of global appearance options to Plasma
        defaults from settings and the command line, and a way to start
        without workspace appearance overrides. [#627]

- **M17.08: Profiles and stylesheets**

  - [x] Save, import and export versioned app-owned theme profiles with
        palette choices, the appearance options in this milestone and optional
        stylesheets. Validate them, confirm overwrites and offer a safe reset
        to system appearance; do not execute code from theme files. Report
        unavailable themes and preserve saved references for retry. [#344]
  - [x] Support an optional app-local Qt Style Sheet (QSS, a CSS-like subset
        rather than full web CSS) entered in a text box or imported from a
        file. Allow preview, application, disabling, reversion and export
        separately or with a theme profile. Report invalid or unsupported
        input, bound resource use and prevent unapproved external resource
        loading. [#351]

- **M17.09: Workspaces**

  - [x] Attach a theme profile and selected appearance options, such as Card
        view, folder colour rules, blur, corner radius and divider visibility,
        to a workspace. Choose all, selected or none of them as M15 workspace
        behaviours and apply them live when switching workspaces. Layer them in
        a documented order from Plasma defaults through the theme profile and
        workspace overrides to the custom stylesheet. Preserve unselected
        global state, report unavailable themes or effects, such as blur
        without KWin, and keep saved references for retry; M27 can apply
        separate choices per Activity. [#628]

**Caveats:** [Themes](CAVEATS.md#themes), tracked in [#680].

### M18: Theme settings

**Project:**
[M18 milestone](https://github.com/mikepadiernos/plasma-opus/issues/352)

Settings for M17 themes, stored in KConfig. Workspaces can override them; see
M18.08.

- **M18.01: Themes**

  - [x] **Appearance:** follow the system or select an available app-local
        preset or installed KDE colour scheme [#348]
  - [x] **Custom colours:** edit, preview, save and reset app-local palette
        roles with contrast feedback and reversible preview [#349]
  - [x] **Profiles:** manage, import and export validated app-owned profiles and
        optional stylesheets with overwrite confirmation [#350]
  - [x] **Custom stylesheet:** enable or disable QSS, edit it in a text box,
        import or export it, and preview or revert changes [#353]
  - [x] **Reset:** restore global appearance options to Plasma defaults with
        confirmation [#629]

- **M18.02: Colours**

  - [x] **Accent:** follow Plasma or choose an app-local accent colour [#630]
  - [x] **Pane tint:** tint all, active or inactive pane backgrounds and adjust
        the strength [#631]
  - [x] **Folder colours:** choose an icon tint, badge or coloured name; manage
        colour rules, their order and subfolder inheritance [#632]

- **M18.03: Effects and shape**

  - [x] **Blur:** enable background blur where KWin supports it and show its
        availability [#633]
  - [x] **Opacity:** adjust window, pane and panel background opacity [#634]
  - [x] **Corner radius:** choose a preset or custom radius for items, cards,
        panels and controls [#635]

- **M18.04: Lines and dividers**

  - [x] **Dividers:** show, thin or hide splitter handles, separators, grid
        lines and frames; choose whether hidden splitters appear on hover [#636]
  - [x] **Tree lines:** show or hide them; choose solid or dotted lines and a
        single colour, a depth palette or folder colours [#637]

- **M18.05: Views**

  - [x] **Cards:** card size, columns, density and shown metadata; add Card to
        the M02 default view mode choices [#638]
  - [x] **Rows and selection:** row striping, hover highlighting and selection
        style [#639]

- **M18.06: Text and spacing**

  - [x] **Fonts:** follow Plasma or choose app-local fonts for file lists, the
        tree pane, tabs and the status bar [#640]
  - [x] **Spacing:** global padding and spacing scale [#641]
  - [x] **Tabs and status bar:** tab shape, active tab indicator and tab colour
        strength [#642]

- **M18.07: Accessibility**

  - [x] **Motion:** follow Plasma's animation speed or reduce or disable Plasma
        Opus animations [#643]
  - [x] **High contrast:** whether to suspend blur, translucency, tints and
        custom corner radii with high-contrast colour schemes [#644]

- **M18.08: Workspaces**

  - [x] **Theme:** let each workspace use an available theme profile or the
        global appearance [#645]
  - [x] **Overrides:** choose all, selected or none of the appearance options to
        save with a workspace, independently of other workspace features;
        preview them and report unavailable effects [#646]

**Caveats:** [Theme settings](CAVEATS.md#theme-settings), tracked in [#687].

### M19: Branding

**Project:**
[M19 milestone](https://github.com/mikepadiernos/plasma-opus/issues/337)

Give Plasma Opus a consistent identity: its name, description, logo and
application icon. File, device and action icons still come from the active icon
theme.

- [x] Use a consistent, translatable application name and description in the
      desktop entry, app metadata, About dialog and documentation. [#338]
- [x] Create original, appropriately licensed Plasma Opus logo and application
      icon assets in suitable sizes and light/dark variants. Continue to source
      file, device and action icons from the active desktop icon theme. [#339]
- [x] Present branding consistently in About and release materials. Offer an
      optional, accessible startup presentation that does not delay or block
      use. [#340]

**Caveats:** [Branding](CAVEATS.md#branding), tracked in [#689].

### M20: Branding settings

**Project:**
[M20 milestone](https://github.com/mikepadiernos/plasma-opus/issues/346)

Settings for M19 branding, stored in KConfig and separate from theme settings.

- **Branding**

  - [x] **Presentation:** enable or disable the optional startup presentation
        and select available branding variants without changing the app identity
        [#347]

**Caveats:** [Branding](CAVEATS.md#branding), tracked in [#689].

### M21: Advanced search

**Project:**
[M21 milestone](https://github.com/mikepadiernos/plasma-opus/issues/30)

Extend the M09 search tools and reuse their search scopes, filters and saved
searches rather than introducing a separate search system. M27 extends the Baloo
integration without blocking this milestone.

- [x] Indexing: configure search folders and exclusions, and use existing KDE
      facilities for the search shortcut. Use double <kbd>Shift</kbd> where
      supported and report when it is unavailable. [#169]
- [x] Provide an embedded search pane using existing supported placements.
      Placing it alongside file panes must not replace either pane. [#239]
- [x] Show M09 search results in the embedded panel, reusing progress,
      cancellation, error reporting and open and reveal actions without moving
      files [#243]
- [x] Offer an optional main-toolbar search widget to enter queries, search the
      shared scope (initially the active pane's location) and open the advanced
      search panel [#240]
- [x] Support keyboard navigation and focus between the toolbar widget, search
      panel and panes [#244]
- [x] Prototype query parsing and validation in Rust behind a narrow C++/Rust
      interface. Test boundary error handling, ownership, path handling and
      threading, and evaluate build and maintenance costs before broader Rust
      adoption; keep the search UI and KIO integration in C++. [#242]
- [x] Combine filename, path, file type, size, date and content criteria using
      AND, OR and NOT; validate queries and explain unsupported criteria [#241]

**Caveats:** [Search](CAVEATS.md#search), tracked in [#362].

### M22: Advanced search settings

**Project:**
[M22 milestone](https://github.com/mikepadiernos/plasma-opus/issues/31)

Settings for M21 advanced search, stored in KConfig.

- [x] **Defaults:** share M10 search scope, filter and saved-search settings
      between the toolbar and panel [#248]
- [x] **Panel:** choose an embedded-pane placement and whether to restore
      visibility at startup, preserving both file panes in shared placements
      [#245]
- [x] **Results:** choose the default presentation and whether to retain search
      history; allow history to be cleared [#247]
- [x] **Toolbar:** show or hide the search widget using M12 toolbar controls and
      choose whether submitting a query opens the panel [#246]

**Caveats:** [Search](CAVEATS.md#search), tracked in [#362].

### M23: Native tagging

**Project:**
[M23 milestone](https://github.com/mikepadiernos/plasma-opus/issues/32)

A native KDE tagging workflow inspired by TagStudio, integrated into Plasma Opus
rather than requiring TagStudio or changing the user's folder structure.

- [x] Evaluate Rust for tag relationships, library metadata and search using the
      M21 pilot. Keep Qt/KDE integration in C++ and use C++ for the core if
      build and interoperability costs are not justified. [#250]
- [x] Store library metadata in an open, documented format without requiring a
      sidecar per file; preserve existing `.mfm.json` metadata and report
      unsupported or read-only locations [#270]
- [x] Create opt-in libraries rooted in existing local folders; index files of
      any type without moving, renaming or duplicating them [#249]
- [x] Refresh libraries with progress and cancellation. Track moves and renames
      initiated by Plasma Opus, and offer review and manual relinking for
      missing or externally moved files without silently attaching metadata to
      uncertain matches. [#271]
- [x] Manage tags with stable identities, names, aliases, shorthand, colours and
      parent relationships; disambiguate duplicate names and prevent hierarchy
      cycles [#251]
- [x] Assign and remove multiple tags on selected files and folders from
      listings or previews, and show assignments in the file interface. Provide
      a searchable, keyboard-accessible tag manager as an embedded pane using
      existing supported placements without replacing file panes. [#253]
- [x] Search and filter by tags, inherited parent tags, untagged items, paths
      and file types using AND, OR, NOT and grouped expressions; reuse existing
      search and selection workflows [#254]
- [x] Organize tag groups independently of locations and workspaces. Until the
      native tag-group provider is available, keep all groups visible; recommend
      stable identities and no hidden or implicit assignment in [#252]. [#252]
- [x] Add editable text and date fields to library entries alongside existing
      labels, ratings and notes, without silently replacing or converting them
      [#268]
- [x] Read supported EXIF image metadata and ID3 audio tags for display and
      search. Let users map selected fields to library tags or fields with
      preview and conflict handling. Reuse M09 metadata editing for supported
      writes, with explicit consent and clear reporting of unsupported formats
      and failures. [#269]
- [x] Export and restore library metadata with preview and explicit
      confirmation; never modify or delete original files during either
      operation [#272]

**Caveats:** [Native tagging](CAVEATS.md#native-tagging), tracked in [#714].

### M24: Tagging settings

**Project:**
[M24 milestone](https://github.com/mikepadiernos/plasma-opus/issues/33)

Settings for M23 tagging, stored in KConfig.

- [x] **Privacy:** control library metadata and backup locations, explain what
      is stored and require explicit consent before writing tags into file
      metadata [#278]
- [x] **Libraries:** manage roots and exclusions, automatic refresh and scan
      limits [#273]
- [x] **Tags:** configure colours, display, parent-tag matching defaults and
      embedded-pane placement [#274]
- [x] **Search:** choose the default tag-search scope and matching behaviour
      [#277]
- [x] **Fields:** manage available text and date fields [#275]
- [x] **Embedded metadata:** select EXIF and ID3 fields to read or map to
      library metadata, and control refresh and conflict handling [#276]

**Caveats:** [Tagging settings](CAVEATS.md#tagging-settings), tracked in
[#716].

### M25: Yazi mode

**Project:**
[M25 milestone](https://github.com/mikepadiernos/plasma-opus/issues/407)

Offer an optional native Qt/KDE browsing mode inspired by
[Yazi](https://yazi-rs.github.io/docs/quick-start/), a terminal file manager.
Reuse Plasma Opus's file models and KDE services; this is not an embedded Yazi
process or a promise of Yazi configuration or plugin compatibility.

- [x] Allow the mode to be entered or left per pane, keeping it off by default.
      Preserve both panes and their tabs, locations, selections and normal view
      settings when switching; let M15 workspaces restore the mode when
      available. [#408]
- [x] Show parent and current folders with an optional preview in the active
      pane. Adapt or hide columns in narrow windows without replacing the other
      pane; keep navigation and preview accessible. [#409]
- [x] Provide keyboard-first navigation with optional Vim-style movement and
      multi-key actions. Reuse KDE shortcut facilities; show the active mode and
      do not intercept typing in editors, search fields or the terminal. [#410]
- [x] Support focused-item and multi-item selection, copy, move, rename, trash
      and other existing file actions through Plasma Opus and KIO. Retain
      conflict handling, destructive-action confirmations and undo; never add a
      shortcut that silently overwrites or permanently deletes files. [#411]
- [x] Reuse filtering, filename and content search, sorting, tabs and history.
      Show active filters and search progress, allow cancellation and report
      backend or remote-location limitations. [#412]
- [x] Reuse Plasma Opus previews and file information for the focused item, with
      cancellable work and existing size and remote-download limits. Report
      progress and errors through existing jobs and the transfer queue rather
      than duplicating operations. [#413]
- [x] Keep large and remote listings responsive and accessible to keyboard and
      assistive-technology users. Report unavailable operations and preserve
      ordinary browsing when a location is unsupported by the mode. [#414]
- [x] Store mode, column and preview choices in KConfig and available workspace
      layouts. Include Yazi mode in workspace snapshots and feature selection.
      Provide a discoverable toggle without changing existing bindings by
      default. [#415]

**Caveats:** [Yazi mode](CAVEATS.md#yazi-mode), tracked in [#723].

### M26: Yazi mode settings

**Project:**
[M26 milestone](https://github.com/mikepadiernos/plasma-opus/issues/416)

Settings for M25 Yazi mode, stored in KConfig and coordinated with workspace
layouts.

- [x] **Activation:** choose each pane's default mode and whether to restore its
      previous choice. Respect explicitly restored M15 layouts when available
      and keep the mode off by default. [#417]
- [x] **Columns:** choose visible folder-context and preview columns and their
      sizes; adapt to narrow windows without hiding the other file pane. [#418]
- [x] **Keyboard:** choose a standard or Yazi-inspired shortcut preset and
      review conflicts before applying it. Keep typing and accessibility
      controls usable; reuse KDE shortcut configuration. [#420]
- [x] **Preview:** choose whether to show the focused item's preview or file
      information. Reuse existing providers, size limits and remote-download
      policy rather than introducing conflicting settings. [#419]
- [x] **Workspace layouts:** when M15 is available, choose whether layouts
      restore Yazi mode and its column settings, including the Yazi-specific
      workspace feature choice, without overwriting an explicitly selected
      workspace layout or changing other pane views. [#421]
- [x] **Reset:** restore Yazi mode defaults without erasing saved workspace
      layouts or unrelated settings. [#422]

**Caveats:** [Yazi mode settings](CAVEATS.md#yazi-mode-settings), tracked in
[#725].

### M27: Integrations

**Project:**
[M27 milestone](https://github.com/mikepadiernos/plasma-opus/issues/34)

Integrate Plasma Opus with KDE Plasma and desktop services, archives and
application packages, search and indexing, terminals, editors and media, remote
storage, access control, development tooling and mail. Partial work and handoffs
for some features are recorded in [pending tasks](TODO.md).

- **M27.01: KDE Plasma and desktop services**

  - [x] Default file manager registration [#284]
  - [x] Provide the `org.freedesktop.FileManager1` D-Bus service [#279]
  - [x] Integrate with KDE Plasma's language preferences using M13 translation
        catalogues, M14 language and regional-format preferences, and supported
        KDE and Qt facilities. Follow Plasma when no app override is selected,
        fall back to source text if a Plasma Opus catalogue is unavailable, and
        offer access to Plasma's language settings when supported. Do not change
        shared desktop settings without explicit action or require a particular
        translation. [#437]
  - [x] Plasma notifications and jobs: integrate Plasma Opus-owned long-running
        operations with KDE's notification settings and offer relevant actions
        to return to the operation or inspect its result. Reuse KIO's existing
        progress reporting rather than creating duplicate notifications or
        controls. Respect notification settings and privacy; report failures and
        partial results without exposing sensitive paths unnecessarily. [#388]
  - [x] Provide system-tray integration and a global shortcut [#285]
  - [x] Plasma Activities: optionally associate M15 workspaces with Activities
        and configure default folders for each Activity and workspace
        association. For each association, use all, selected or none of the
        applicable workspace behaviours, including theme and layout, and
        Activity-specific default folders independently of other Activities;
        allow workspace defaults or explicit overrides. Offer to apply the
        selected choices when the Activity changes, without silently replacing
        restored tabs or unsaved layouts. Leave unselected behaviours and state
        unchanged, preserve unassociated workspaces and running operations, and
        never create, delete or switch an Activity without explicit user action.
        Handle an unavailable Activities service, missing folders and removed
        Activities without losing workspace data, and avoid duplicating KDE's
        KIO resource history. [#387]
  - [x] KDE Connect sharing: offer an explicit action to send selected files to
        an available paired device through supported KDE Connect interfaces.
        Show the selected device and transfer outcome; do not pair devices, send
        files or change shared KDE Connect settings automatically. Handle
        missing services, offline devices and unsupported locations visibly.
        [#389]
  - [x] KRunner integration: provide an optional runner for explicitly saved
        Plasma Opus workspaces and locations, with actions to open the chosen
        item in Plasma Opus. Respect workspace privacy, avoid exposing
        credentials or unselected history, and handle missing or stale targets
        without creating files or running commands. Use Plasma's supported
        runner interfaces and do not replace its existing file search. [#390]
  - [x] Download service menus from the existing Context Menu settings page, as
        in Dolphin, using KDE's KNewStuff service-menu provider. Support
        browsing, installation, updates and removal through the provider, and
        refresh the installed-action list without discarding pending settings
        changes. Explain that downloaded services can run commands and may be
        shared with other KDE applications; require explicit user action and
        report failures. Reuse the existing Plasma Opus-specific and
        Dolphin-shared visibility controls. [#369]
  - [x] Optional system file chooser that replaces application Open and Save
        dialogs, as in [Strata](https://github.com/lgse/strata), by serving the
        XDG portal file chooser. Implement
        `org.freedesktop.impl.portal.FileChooser` (`OpenFile`, `SaveFile` and
        `SaveFiles`) as a D-Bus-activated backend with a compact chooser that
        reuses Plasma Opus views, places and search. Honour multiple and folder
        selection, filters, choices, suggested names and folders, parent
        windows and request cancellation, and confirm overwrites when saving.
        Nothing changes without explicit opt-in through the M28 **File
        manager** settings, `install.sh --file-chooser` or
        `opus-file-manager --install-portal`. Restore the previous chooser from
        the same settings row, `install.sh --uninstall` or
        `opus-file-manager --uninstall-portal`. Setup installs per-user portal
        metadata and an activation service, prefers Plasma Opus ahead of
        `xdg-desktop-portal-kde` in the user's `kde-portals.conf` so the KDE
        chooser remains the fallback, and restarts the portal frontend after
        warning that open file dialogs may close. Restoring puts back the
        previous configuration, or removes only Plasma Opus if the user has
        changed it since. Keep this separate from default file manager
        registration and the `org.freedesktop.FileManager1` service. Explain
        that it affects applications that use the portal, such as Flatpak and
        Snap apps and portal-enabled GTK, Electron and browser apps; native Qt
        and KDE applications keep Plasma's in-process dialog. Do not change
        platform themes or environment variables, and report a missing portal
        service or setup failures. Plasma-session verification is a
        [pending handoff](TODO.md#system-file-chooser-on-plasma-286). [#286]
  - [x] Mount manager, like Krusader's
        [MountMan](https://docs.kde.org/trunk_kf6/en/krusader/krusader/mount-man.html):
        list mounted and fstab-defined file systems with their device, type,
        mount point, size, free space and a usage chart, optionally showing
        only removable devices. Open a file system in the active pane, and
        mount, unmount or eject it through Solid and UDisks2 with KDE's
        authorization prompts where required. Offer a toolbar quick-mount menu
        that shows each mount point's current state, and include network and
        FUSE mounts such as remote drives. Leave partitioning, formatting and
        fstab editing to KDE Partition Manager. Report busy devices, denied
        authorization and failed operations. (Krusader) Plasma-session
        verification is a
        [pending handoff](TODO.md#mount-manager-on-plasma-703). [#703]
  - [x] Optional KWin script for window management that Wayland applications
        cannot perform themselves: install, enable, update and remove a Plasma
        Opus KWin script package only through explicit action, using KPackage
        and KWin's scripting D-Bus interface. When applying an M15 workspace or
        Activity association, restore each Plasma Opus window's screen,
        virtual desktop, size, tile and keep-above state. Optionally place
        windows opened from Plasma Opus, such as terminals, editors and
        detached previews, beside the file manager using KWin's tiles, and
        apply an assigned workspace when the virtual desktop changes. Act only
        on Plasma Opus windows and communicate through a versioned D-Bus
        interface, checking the installed script version. Prefer
        KWindowSystem, XDG activation and window rules where they suffice, and
        open KWin's settings instead of editing window rules, other scripts or
        global KWin settings. Report when KWin or scripting is unavailable,
        such as on other compositors, without affecting file management.
        Plasma-session verification is a
        [pending handoff](TODO.md#kwin-script-on-plasma-707-708). [#707]
  - [x] Drop-down window, like Yakuake: optionally slide a Plasma Opus window
        in from a chosen screen edge and hide it again, toggled by a KDE global
        shortcut or by a screen edge or touch-screen edge registered by the
        KWin script. Keep it above other windows and out of the taskbar and
        pager, open it on the active screen and optionally hide it when it
        loses focus, but not while a dialog or confirmation needs attention.
        Restore its own layout or an assigned workspace, and fall back to a
        regular window when KWin or the script is unavailable. (Yakuake)
        Plasma-session verification is a
        [pending handoff](TODO.md#kwin-script-on-plasma-707-708). [#708]

- **M27.02: Archives and application packages**

  - [x] Integrate deeply with ARK [#280]
  - [x] Extend archive creation through Ark: create 7z, password-protected and
        split archives, encrypting file names where the format supports it,
        and add, rename or remove entries in existing archives. Keep M03's
        non-overwriting behaviour, confirm changes to existing archives and
        report formats that installed Ark backends can only extract, such as
        RAR. (MiXplorer) [#647]
  - [x] Mount and unmount ISO and other disk images through the Mount ISO
        action from Dolphin's plugins when installed, and show mounted images
        in Places through Solid. Report a missing plugin or denied
        authorization. (MiXplorer) [#648]
  - [x] Integrate deeply with AppImage, Flatpak and Snap: identify packages and
        show available application details and actions. Support AppImage
        launching and optional desktop integration, and use the respective
        package tools for supported Flatpak and Snap installation, updates and
        removal. Confirm changes, respect executable and sandbox permissions,
        and report missing tools, unsupported actions and errors. [#281]

- **M27.03: Search and indexing**

  - [x] Enhance Baloo integration using M09 indexed search and M21's search
        interface. Show service status, configured folders and exclusions, and
        available coverage or indexing progress. Distinguish configured scope
        from confirmed coverage and warn about missing or stale results. Offer
        KDE's shared indexing controls, reuse metadata from installed
        KFileMetaData extractors and identify missing extractors when
        detectable. Do not change shared settings, enable indexing or start a
        reindex without explicit action. [#441]
  - [x] Extend Baloo search with supported metadata criteria and optional fuzzy
        ranking. Explain that ranking cannot recover matches missing from the
        index or results truncated by existing limits. Offer an explicit
        live-search fallback for supported local folders, retaining its limits,
        progress, cancellation and error reporting; never silently broaden scope
        or substitute another search. [#442]
  - [x] Show file-type categories in Places, such as documents, images, audio,
        video, archives, ebooks, application packages and large files, as Baloo
        searches alongside KDE's existing **Search For** entries. Explain when
        indexing is off or incomplete and offer a live search of a chosen
        folder instead. (MiXplorer) [#649]
  - [x] Run indexed searches inside Baloo where possible: translate the M21
        query, scope and filters into one Baloo query using its AND, OR and
        property terms and its folder, file-type and date filters, rather than
        fetching one name or content term from the whole index and filtering
        up to 200,000 matches in Plasma Opus. Filter in Plasma Opus only for
        criteria Baloo cannot express, such as depth, exclusions, path
        selection and Plasma Opus tags, and check results against the full
        query. Return the same matches as the current search, apart from those
        previously lost to its limits; report when limits still apply, and use
        newer Baloo search scopes only when the installed version supports
        them. [#733]
  - [x] Share saved searches with KDE: offer to add a saved search that Baloo
        can represent to KDE's shared places as a search location, so Dolphin
        and file dialogs can reuse it. Explain which criteria, such as depth,
        exclusions or Plasma Opus tags, cannot be carried over, and add the
        entry only after the user confirms. Keep Plasma Opus saved searches as
        the source of truth, and do not update or remove shared entries
        without explicit action. [#734]

- **M27.04: Terminals and command history**

  - [x] Extend the existing open-terminal action and M09 embedded Konsole
        terminal with explicit separate Konsole, Yakuake or Kwi providers and an
        embedded Kwi KPart when available. Reuse the current Konsole integration
        and pane-preserving layout. Yakuake is a separate drop-down application,
        not an embeddable terminal; report unavailable providers and unsupported
        locations. [#289]
  - [x] Optional [Atuin](https://docs.atuin.sh/) integration for terminal
        command history: let users open interactive Atuin search in a supported
        terminal, without automatically executing a selected command. Respect
        the user's existing shell configuration; do not install or initialize
        Atuin, access its history database, or enable history synchronization
        without consent. Report missing tools and unsupported terminal sessions.
        [#364]

- **M27.05: Editors, documents and media**

  - [x] Editor integrations: offer KWrite and Kate as separate GUI editors, and
        Neovim, Rune, micro and nano in a supported embedded or separate
        terminal, as alternatives to the built-in editor. KWrite and Kate use
        KTextEditor themselves; they are separate applications, not replacement
        editor components. Preserve the built-in editor and its draft handling,
        pass file paths safely without shell interpolation, and report missing
        providers, launch failures and unsupported locations. [#319]
  - [x] Office documents: integrate viewing and editing for documents,
        spreadsheets and presentations, including OpenDocument and Microsoft
        Office formats. Reuse installed KParts for supported previews and offer
        an explicit action to open files in an available office application,
        such as LibreOffice. Evaluate embedded editing only through a supported
        provider; do not assume an installed viewer supports editing. Report
        unavailable providers, unsupported formats and locations, and launch or
        save failures. Preserve unsaved changes and existing file content; never
        silently convert or overwrite a document. [#674]
  - [x] For any viewable file, offer an edit action when a suitable tool is
        available. Use supported embedded providers or launch a separate editor;
        distinguish viewing from editing capabilities and report unavailable
        tools, unsupported locations and launch failures. Preserve unsaved
        changes and avoid silent conversion or overwrite. [#675]
  - [x] Edit text in the preview panel: when the preview panel shows a text
        file, offer an explicit Edit action that switches the preview in place
        to the built-in KTextEditor editor for that file, without opening the
        separate editor panel or changing the M08 opening choice. Load the
        whole file rather than the truncated preview, reuse M07 drafts,
        Markdown preview and KTextEditor settings, and return to the read-only
        preview after saving or closing. While changes are unsaved, keep the
        file open when the selection or active pane changes, and ask whether
        to save, discard or keep editing before switching files, closing the
        panel or quitting. Offer to move the session to the editor panel
        without losing changes. Keep previews read-only until editing is
        requested, never save the truncated preview text, and report
        read-only files, remote locations, size limits, binary content or
        unsupported encodings, and save failures. [#729]
  - [x] Extend the built-in editor with CommonMark and Obsidian-like wiki links
        and backlinks within a user-selected local vault. Preserve source and
        drafts, respect vault boundaries, and report ambiguous, missing or
        inaccessible targets and incomplete scans. See the
        [pending handoff](TODO.md#markdown-editor-and-backlinks-564). [#564]
  - [x] Let an assigned local location serve as an optional Obsidian-inspired
        vault for the built-in text editor. Reuse [#564] for source and visual
        Markdown editing, live preview, note navigation, wiki links, backlinks,
        tags and a note relationship view. Preserve drafts and unsaved edits
        during navigation and workspace switches; index only the chosen vault
        and report incomplete results and ambiguous or missing links. Use
        existing folders without moving notes, executing plugins or changing
        Obsidian configuration. Deleting a workspace must not delete its vault.
        See the [pending handoff](TODO.md#workspace-vault-editing-565). [#565]
  - [x] Offer optional mpv integration for embedded audio and video playback in
        the preview panel and gallery alongside Qt Multimedia, and an explicit
        action to open selected media in the separate mpv application. Preserve
        autoplay and remote preview behaviour; report unavailable backends,
        playback errors and unsupported locations. See the
        [pending handoff](TODO.md#mpv-integration-563). [#563]
  - [x] Preview ebooks, comics and help files, such as EPUB, MOBI, FictionBook,
        CBZ, DjVu and CHM, with Okular's KPart when installed, within the M05
        preview limits. Preview fonts with editable sample text through
        Plasma's font viewer part where available, and offer installation
        through KDE's font management. Report missing providers. (MiXplorer)
        [#650]

- **M27.06: Accounts, remote access and synchronization**

  - [x] Integrate advanced SSH, SFTP and WebDAV support: manage named profiles
        and browse remote locations in either pane using KDE's KIO workers. Use
        the existing transfer queue where available and offer a separate SSH
        terminal for SSH profiles. Support passwords and SSH key/agent
        authentication where available, use KDE credential facilities and
        respect SSH host-key and WebDAV TLS certificate verification. Report
        connection, authentication and unsupported-operation errors explicitly.
        [#282]
  - [x] Extend remote profiles to SMB and FTP or FTPS through KDE's KIO
        workers, and show services discovered on the local network through
        KIO's `network:/` location. Warn that plain FTP sends credentials
        unencrypted, and use KDE credential facilities. (MiXplorer) [#651]
  - [x] Optional [rclone](https://rclone.org/) integration for cloud providers
        without KDE support, such as Dropbox, OneDrive, Box, MEGA and pCloud.
        List the user's configured remotes through rclone and open one in
        either pane by serving it over WebDAV on the loopback interface or by
        mounting it on request. Never read, store or change rclone credentials
        or configuration; report missing tools, failed mounts and unsupported
        operations, and stop processes that Plasma Opus started. (MiXplorer)
        [#652]
  - [x] Mount remote storage as local folders, like
        [RaiDrive](https://www.raidrive.com/) drives, so applications without
        KIO support can open, edit and save files in place. Mount saved SSH,
        SFTP, WebDAV, FTP or FTPS and SMB profiles, Nextcloud, S3-compatible
        storage and KDE Online Accounts locations in an empty folder under the
        user's runtime directory or a chosen location, and show them in Places
        until unmounted. Prefer KDE's KIO FUSE, and offer installed backends as
        alternatives to rclone: sshfs for SFTP, davfs2 for WebDAV with its file
        locks, s3fs-fuse for S3 and the kernel CIFS client for SMB. Offer
        read-only mounts, mounting at login or when a workspace is activated,
        and unmounting on exit; stream media from a mount where the backend
        reads ranges rather than whole files. Never write credentials into
        fstab, backend configuration or Plasma Opus settings without explicit
        consent, and use KDE credential facilities where the backend accepts
        them. Report missing FUSE support or tools, mounts that need an fstab
        entry or administrator authorization, stale or busy mounts and failed
        unmounts; unmount only what Plasma Opus mounted. (RaiDrive) [#702]
  - [x] Temporary LAN sharing: serve selected files or a folder over HTTP or
        WebDAV to devices on the local network, with optional TLS, a QR code,
        a random access token, read-only access by default and an optional
        upload folder with conflict handling. Bind only to chosen interfaces,
        show active sessions and transfers, stop after a timeout or on exit,
        and never open firewall ports. Warn that transfers without TLS can be
        observed on the network, and point to KDE Connect and Samba sharing
        for paired devices and persistent shares. (MiXplorer) [#653]
  - [x] Integrate with KDE Online Accounts: discover available accounts and
        their supported file services, let users explicitly connect or open them
        in either pane through the appropriate KDE provider or KIO worker, and
        provide access to KDE's account-management interface. Reuse account
        identities for supported Nextcloud and remote integrations instead of
        duplicating credentials. Handle missing providers, denied access,
        expired authorization and removed accounts visibly; do not assume every
        account offers file access. Respect provider permissions, authentication
        and TLS verification, and never store tokens or passwords in URLs, logs
        or Plasma Opus configuration. [#370]
  - [x] Optional [rsync](https://rsync.samba.org/) backend for M09 folder sync:
        support local folders and SSH locations, reuse sync profiles and
        exclusions, and show an itemized dry run before confirmation. Disable
        deletion by default and explicitly confirm overwrites and deletions.
        Validate that the backend can honour the selected plan, including
        per-item directions; report unsupported options instead of silently
        changing the plan. Show progress, cancellation, errors and possible
        partial results; a preview is a snapshot, not a guarantee against
        subsequent filesystem changes. Use safe argument handling and SSH
        host-key verification. [#363]
  - [x] Two-way cloud sync, like
        [Syncative](https://github.com/Santuzius/syncative): pair any local
        folder, not only a single sync root, with a folder on a Nextcloud,
        WebDAV, SFTP, SMB, S3-compatible or KDE Online Accounts location or an
        rclone remote, and keep both sides up to date. Offer Sync, Cloud backup
        (upload only) and Local backup (download only) modes per pair, with
        deletions copied to a backup only when enabled. Reuse M09 sync filters
        and profiles for exclusions, and ask how to handle a pair nested in
        another. Keep a per-pair state snapshot to tell renames and deletions
        from new files. Never overwrite a file changed on both sides: let the
        user keep the local, remote or both copies, per file or for all.
        Preview the first sync and large deletions before applying them. Sync
        in the background while Plasma Opus or its tray icon runs, optionally
        from login, with pause and resume per pair or account and an option to
        pause on metered connections. Use KIO, or rclone bisync for rclone
        remotes, and keep credentials in KDE's facilities. Refuse pairs that
        overlap folders managed by the Nextcloud desktop client or another
        sync engine. Report quota, offline remotes, permission errors and
        partial runs. (Syncative) [#718]
  - [x] Cloud activity panel, like
        [SDrive](https://github.com/TitanniOpen/SDrive) and
        [DucksDrive](https://patopatetico2-lab.github.io/ducksdrive/): for
        remote drives and sync pairs, show connection state, quota where the
        provider reports it, pending uploads, current transfers with speed,
        remaining time and a speed graph, and recent errors, through the
        existing transfer queue and an optional tray panel. Offer pause and
        resume, a bandwidth limit, the number of parallel transfers, and a size
        limit for the local cache of mounted remotes that keeps recently opened
        files available offline. Before unmounting, exiting or logging out,
        wait for pending uploads with progress and an option to cancel, and
        report uploads that need recovery after a crash. Export connection and
        sync-pair definitions without credentials, and ask the user to sign in
        again after importing them. (SDrive, DucksDrive) [#719]

- **M27.07: Access control and automation**

  - [x] Support administrator mode through KIO's `admin:/` worker.
        Plasma-session verification is a
        [pending handoff](TODO.md#administrator-mode-on-plasma-288). [#288]
  - [x] Provide an MCP server [#287]
  - [x] Optional password-gated folders: let users require a password before
        opening selected local folders through Plasma Opus. This is a Plasma
        Opus-only convenience gate, not encryption or filesystem access control:
        clearly warn that other applications and direct filesystem access remain
        unrestricted. Do not change file permissions or contents. Apply the gate
        consistently across Plasma Opus navigation and alternate paths, store no
        plaintext password, and use a trusted password-verification mechanism
        and KDE credential facilities where appropriate. Report unavailable
        protection and failures rather than silently bypassing the gate; provide
        explicit password change and removal workflows. [#402]
  - [x] Plasma Vault integration: list encrypted vaults (CryFS, gocryptfs or
        EncFS) with their lock state, and open, close or create them through
        Plasma Vault's `org.kde.plasmavault` D-Bus interface, revealing an
        opened vault in a pane. Leave passwords and vault configuration to
        Plasma Vault, warn before closing a vault with files in use and report
        an unavailable service. Plasma-session verification is a
        [pending handoff](TODO.md#plasma-vault-on-plasma-654). (MiXplorer)
        [#654]
  - [x] File encryption: offer encrypt, decrypt, sign and verify actions for
        selected files through GnuPG, reusing Kleopatra's file actions when
        installed, and write new files without replacing the originals.
        Identify encrypted and signed files. Support AES Crypt (`.aes`) files,
        as used by MiXplorer, only through an installed `aescrypt` tool and
        without exposing passwords in command lines. Plasma-session
        verification is a
        [pending handoff](TODO.md#file-encryption-on-plasma-655). (MiXplorer)
        [#655]

- **M27.08: Development tooling**

  - [x] Use [mise](https://mise.jdx.dev/) for development tooling: manage
        project tool versions and provide configure, build, test and formatting
        tasks. Document setup and system-provided Qt, KDE and other
        dependencies, including versions needed to reproduce builds; keep the
        existing CMake workflow usable without mise. [#283]

- **M27.09: Plasma Impresario and mail**

  - [x] Send selected files by email: offer an explicit action that opens the
        user's preferred mail client, such as
        [Plasma Impresario](https://github.com/mikepadiernos/plasma-impresario),
        through KIO's `KEMailClientLauncherJob` or the XDG Email portal when
        sandboxed. Show the selected files, offer private local copies of
        remote files through KIO with consent, and never send mail. Report
        missing clients and clients that cannot accept attachments; Plasma
        Impresario currently ignores `mailto:` attachments. See the
        [companion workflows](integrations/plasma-impresario.md#compose-with-selected-files).
        Plasma-session verification is a
        [pending handoff](TODO.md#send-by-email-on-plasma-656). [#656]
  - [x] Mail attachments: verify dragging local files into Plasma Impresario's
        composer and offer private local copies for remote items. Recognize
        its private attachment drag format and explain that the attachment
        must be saved from the mail client instead of saving the reference as
        a file; accept real file content or URLs through the existing KIO drop
        and conflict handling when provided. Never contact a mail server.
        Plasma-session verification is a
        [pending handoff](TODO.md#mail-attachments-on-plasma-657). [#657]
  - [x] Mail file preview: show local `.eml` (`message/rfc822`) headers,
        plain-text body and attachment list in the preview panel without
        loading remote content, running scripts or decrypting messages. Offer
        explicit attachment extraction with normal conflict handling and
        opening through the registered application; never import messages.
        Plasma-session verification is a
        [pending handoff](TODO.md#mail-file-preview-on-plasma-658). [#658]
  - [x] Shared context with Plasma Impresario: use Plasma Activities, not a
        private API, as the only shared context with its planned
        account-to-workspace associations. Each application follows Activity
        changes with its own associations; do not read or change the other
        application's workspace, account or mail data.
        Plasma-session verification is a
        [pending handoff](TODO.md#shared-activity-context-on-plasma-659). [#659]

**Caveats:**
[Desktop integration](CAVEATS.md#desktop-integration), tracked in [#728];
[Password-gated folders](CAVEATS.md#password-gated-folders), tracked in
[#406].

### M28: Integration settings

**Project:**
[M28 milestone](https://github.com/mikepadiernos/plasma-opus/issues/35)

Settings for M27 integrations, stored in KConfig.

- **File manager**

  - [x] **Default:** show the current desktop file-manager association and offer
        an explicit action to set Plasma Opus as the user default; report
        failures. Plasma-session verification is a
        [pending handoff](TODO.md#default-file-manager-on-plasma-293). [#293]
  - [x] **System file chooser:** show whether Plasma Opus serves the XDG portal
        file chooser, with explicit actions to complete setup or restore the
        previous chooser; report failures. Plasma-session verification is a
        [pending handoff](TODO.md#file-chooser-settings-on-plasma-292). [#292]
  - [x] **System file chooser start folder:** when an application suggests no
        folder, start in the home folder or in the folder of the last accepted
        choice. Remember the last folder per application where the portal
        identifies it, in session state rather than settings. (Luciole)
        Plasma-session verification is a
        [pending handoff](TODO.md#chooser-start-folder-on-plasma-750).
        [#750]

- **Plasma notifications**

  - [x] **Operations:** choose which Plasma Opus-owned operation events produce
        notifications; use KDE's notification settings for shared controls.
        Plasma-session verification is a
        [pending handoff](TODO.md#operation-notifications-on-plasma-392).
        [#392]

- **Mount manager**

  - [x] **Display:** show only removable devices by default, hide pseudo, snap
        and other system file systems, and show the quick-mount menu on the
        toolbar [#705]

- **KWin scripts**

  - [x] **Window management:** show whether the Plasma Opus KWin script is
        installed and enabled, with explicit actions to install, update,
        disable or remove it; choose which behaviours it applies, such as
        workspace window restoration, tiling opened windows and following
        virtual desktops, and open KWin's settings for shared window rules.
        Plasma-session verification is a
        [pending handoff](TODO.md#kwin-script-settings-on-plasma-709).
        [#709]
  - [x] **Drop-down window:** enable it and choose its screen edge, width,
        height, screen, animation and trigger, and whether it hides when it
        loses focus. Plasma-session verification is a
        [pending handoff](TODO.md#drop-down-settings-on-plasma-710).
        [#710]

- **Baloo**

  - [x] **Search:** choose whether to offer fuzzy ranking and live-search
        fallback; reuse M10 and M22 settings and open KDE's shared indexing
        controls rather than duplicating or silently changing them.
        Plasma-session verification is a
        [pending handoff](TODO.md#baloo-search-settings-on-plasma-443).
        [#443]
  - [x] **Categories:** choose which file-type categories appear in Places and
        their criteria, such as the large-file size [#660]

- **Terminal**

  - [x] **Provider:** select Konsole, Yakuake or Kwi for separate sessions and
        Konsole or Kwi for embedded sessions, independently of M10 placement
        [#295]

- **Editor**

  - [x] **Provider:** select the built-in editor or an available KWrite, Kate,
        Neovim, Rune, micro or nano integration, plus the session type for
        terminal editors [#320]
  - [x] **Markdown:** choose a local vault and the Markdown editing and preview
        behaviour, retaining source and draft safeguards. [#577]
  - [x] **Workspace vault:** enable vault editing for a workspace and choose an
        assigned local location. Configure available editing, preview,
        navigation, wiki-link, backlink, tag and relationship-view options.
        Preserve drafts and notes and report unavailable roots or features and
        incomplete indexing. See the
        [pending handoff](TODO.md#workspace-vault-editing-565). [#566]
  - [x] **Preview editing:** choose whether text previews offer editing,
        whether double-clicking the preview starts editing, and whether to
        return to the preview after saving [#730]

- **Office documents**

  - [x] **Provider:** choose an available office application for editing and use
        installed preview providers where supported; report unavailable choices.
        Plasma-session verification is a
        [pending handoff](TODO.md#office-settings-on-plasma-676).
        [#676]

- **Ebooks and fonts**

  - [x] **Previews:** enable ebook, comic, help-file and font previews and set
        the font sample text [#661]

- **Media**

  - [x] **Backend:** choose Qt Multimedia or the optional embedded mpv backend
        for preview and gallery playback. Plasma-session verification is a
        [pending handoff](TODO.md#mpv-integration-563).
        [#578]

- **System tray**

  - [x] **Behaviour:** choose whether to minimize and close to the tray.
        Plasma-session verification is a
        [pending handoff](TODO.md#start-hidden-in-tray-on-plasma-290). [#290]

- **Shortcuts**

  - [x] **Global:** enable or disable the integration's global shortcut and
        choose its key combination through KDE facilities. Plasma-session
        verification is a
        [pending handoff](TODO.md#global-shortcut-on-plasma-291). [#291]

- **Remote connections**

  - [x] **Profiles:** manage SSH, SFTP, SMB, FTP or FTPS and WebDAV hosts,
        ports, usernames and connection defaults; use KDE credential facilities
        instead of storing credentials in KConfig or URLs. Plasma-session
        verification is a
        [pending handoff](TODO.md#remote-connection-settings-on-plasma-297).
        [#297]
  - [x] **Discovery:** show or hide services discovered on the local network
        [#662]

- **Cloud storage**

  - [x] **rclone:** enable the integration, choose which remotes to show and
        whether to serve them over WebDAV or mount them. Plasma-session
        verification is a
        [pending handoff](TODO.md#rclone-settings-on-plasma-663). [#663]
  - [x] **Remote drives:** choose the mount backend for each protocol from KIO
        FUSE and installed alternatives, the mount folder, read-only by
        default, and whether to mount at login or workspace activation and
        unmount on exit. Plasma-session verification is a
        [pending handoff](TODO.md#remote-drive-settings-on-plasma-704). [#704]
  - [x] **Sync pairs:** manage two-way sync pairs and their mode, filters,
        deletion handling and default conflict choice; choose whether to sync
        from login or on metered connections, and how often to check remotes
        that do not report changes. Plasma-session verification is a
        [pending handoff](TODO.md#cloud-sync-settings-on-plasma-720). [#720]
  - [x] **Transfers and cache:** set the bandwidth limit, parallel transfers,
        cache size and offline retention for mounted remotes, and whether to
        show the tray activity panel. Plasma-session verification is a
        [pending handoff](TODO.md#transfer-and-cache-settings-on-plasma-721).
        [#721]

- **LAN sharing**

  - [x] **Defaults:** protocol, TLS, network interfaces, port, timeout and
        whether uploads are allowed. Plasma-session verification is a
        [pending handoff](TODO.md#lan-sharing-settings-on-plasma-664). [#664]

- **Online accounts**

  - [x] **Access:** opt in and choose which supported accounts to show in Plasma
        Opus; manage accounts in KDE without deleting shared accounts when
        removing them from Plasma Opus. Plasma-session verification is a
        [pending handoff](TODO.md#online-account-settings-on-plasma-371).
        [#371]

- **Application packages**

  - [x] **Integration:** independently enable AppImage, Flatpak and Snap actions
        and configure optional AppImage desktop integration [#296]

- **Archives**

  - [x] **Creation:** default 7z or ZIP format, split size and whether to prompt
        for encryption, extending M04 archive defaults [#665]

- **rsync**

  - [x] **Backend:** enable the optional backend and choose safe sync defaults;
        reuse M10 profiles and preserve preview and confirmation requirements.
        Plasma-session verification is a
        [pending handoff](TODO.md#rsync-settings-on-plasma-366). [#366]

- **Atuin**

  - [x] **History:** enable the history action and select a supported terminal
        session without changing Atuin's history or sync settings [#367]

- **Plasma Activities**

  - [x] **Workspaces:** enable Activity associations; manage workspace mappings,
        Activity-specific default folders, feature selections and
        workspace-switch prompts without changing KDE's Activity settings. For
        each association, inherit workspace defaults or select all, individual
        or no behaviours. Plasma-session verification is a
        [pending handoff](TODO.md#activity-associations-on-plasma-391). [#391]

- **Mail**

  - [x] **Client:** show the current default mail client and open Plasma's
        Default Applications settings to change it; enable or disable the
        Send by Email action. Plasma-session verification is a
        [pending handoff](TODO.md#mail-client-settings-on-plasma-666).
        [#666]
  - [x] **Remote files:** choose whether Send by Email asks before preparing
        local copies of remote files, without allowing silent downloads [#667]
  - [x] **Mail file preview:** enable or disable `.eml` previews, within the
        existing preview size limits [#668]

- **KDE Connect**

  - [x] **Sharing:** enable the sharing action and choose whether to remember a
        device without changing pairing or KDE Connect settings.
        Plasma-session verification is a
        [pending handoff](TODO.md#kde-connect-settings-on-plasma-393). [#393]

- **KRunner**

  - [x] **Discovery:** enable the optional runner and choose which saved
        workspaces and locations it can expose. Plasma-session verification
        is a [pending handoff](TODO.md#krunner-settings-on-plasma-394).
        [#394]

- **MCP**

  - [x] **Access:** configure server access (off, read or write), token and
        audit log [#294]

- **Password-gated folders**

  - [x] **Access:** manage protected local folders and their passwords, with a
        clear warning that the gate applies only inside Plasma Opus [#403]

- **Encryption**

  - [x] **Plasma Vault:** show or hide vaults and their lock state.
        Plasma-session verification is a
        [pending handoff](TODO.md#plasma-vault-settings-on-plasma-669). [#669]
  - [x] **Files:** enable GnuPG and AES Crypt actions and choose how output
        files are named. Plasma-session verification is a
        [pending handoff](TODO.md#file-encryption-settings-on-plasma-670).
        [#670]

### M29: Nextcloud integration

**Project:**
[M29 milestone](https://github.com/mikepadiernos/plasma-opus/issues/586)

Provide optional native Nextcloud navigation, file management and digital asset
management, reusing M27 KDE Online Accounts and KIO WebDAV rather than replacing
the Nextcloud desktop client or its synchronization engine. See the
[Nextcloud integration](integrations/nextcloud.md) for app interfaces and
considered Calendar, Maps, Deck and Notes workflows.

- [x] Connect supported accounts through KDE Online Accounts or an explicit
      Nextcloud login. Use KDE credential facilities, verify TLS and report
      expired access, unavailable providers and server capabilities.
      Real-server verification is a
      [pending handoff](TODO.md#nextcloud-accounts-on-a-real-server-588).
      [#588]
- [x] Browse Nextcloud files in either pane and reuse supported KIO operations,
      confirmations, cancellation and error reporting. Respect server
      permissions and report unsupported operations and partial failures.
      Real-server verification is a
      [pending handoff](TODO.md#nextcloud-browsing-on-a-real-server-365).
      [#365]
- [x] Preview supported remote digital assets and inspect available server
      metadata, respecting existing preview limits and remote-download policy.
      Real-server verification is a
      [pending handoff](TODO.md#nextcloud-previews-on-a-real-server-589).
      [#589]
- [x] Search and organize assets using supported server metadata, tags and
      favorites. Keep server tags distinct from native local tags; identify
      unavailable features and incomplete results. Real-server verification
      is a
      [pending handoff](TODO.md#nextcloud-asset-search-on-a-real-server-590).
      [#590]
- [x] Browse Nextcloud Photos albums, shared albums and places as virtual
      folders, sorting and grouping by server capture date and location
      without downloading files. Add photos to albums or remove them with
      confirmation; removing a photo from an album never deletes the file.
      Respect shared-album permissions and report when Photos is unavailable.
      Real-server verification is a
      [pending handoff](TODO.md#nextcloud-photos-on-a-real-server-671).
      [#671]
- [x] View and manage supported Nextcloud shares with explicit authorization and
      confirmation before broadening access or revoking a share. Offer to send
      a reviewed share link through the M27 Send by Email action.
      Real-server verification is a
      [pending handoff](TODO.md#nextcloud-shares-on-a-real-server-591).
      [#591]
- [x] Open configured local sync folders and the existing desktop client,
      showing available status and distinguishing remote, local, offline and
      unknown state. Never start a second sync engine or change the client's
      configuration. Plasma verification is a
      [pending handoff](TODO.md#nextcloud-desktop-client-on-plasma-592).
      [#592]

### M30: Nextcloud settings

**Project:**
[M30 milestone](https://github.com/mikepadiernos/plasma-opus/issues/587)

Settings for M29 Nextcloud integration, stored in KConfig without credentials.

- [x] Configure account references, supported browsing and preview choices,
      asset metadata and search options, sharing controls and desktop-client
      integration. Reuse KDE Online Accounts, existing WebDAV profiles, remote
      preview policy and KDE credential facilities; explain unavailable server
      features and retain safe defaults. Real-server and Plasma verification
      is a [pending handoff](TODO.md#nextcloud-settings-on-plasma-368).
      [#368]
- [x] Show or hide Photos albums, shared albums and places, choose their
      default grouping and whether to confirm album changes. Real-server
      verification is a
      [pending handoff](TODO.md#nextcloud-photos-settings-on-a-real-server-672).
      [#672]

### M31: Keyboard shortcut settings

**Project:**
[M31 milestone](https://github.com/mikepadiernos/plasma-opus/issues/29)

Configure keyboard shortcuts, key chords, optional Vim-style navigation, a
shortcut hint pane, a command palette and keyboard hints across Plasma Opus,
using KDE shortcut facilities where supported.

- [x] Configure simultaneous key combinations and sequential key chords with
      conflict detection, clear feedback and reset to defaults; use KDE shortcut
      facilities where supported. Integrate advanced-search shortcuts, Yazi
      navigation and presets, and the integration global shortcut with these
      controls without overriding existing bindings. Plasma-session
      verification is a
      [pending handoff](TODO.md#keyboard-shortcuts-on-plasma-230). [#230]
- [x] Scope bindings to the applicable view, folder tree, viewer or application;
      detect conflicts between overlapping contexts and preserve typing and
      accessibility keys in input controls. Plasma-session verification is a
      [pending handoff](TODO.md#shortcut-contexts-on-plasma-233). [#233]
- [x] Store Vim-style mode and app-local bindings in KConfig; manage optional
      global shortcuts through KDE facilities and report registration failures
      or platform limitations. Plasma-session verification is a
      [pending handoff](TODO.md#shortcut-storage-on-plasma-238). [#238]
- [x] Allow alternative bindings for an action and configurable shortcuts for
      commands outside a toolbar, including custom commands. Plasma-session
      verification is a
      [pending handoff](TODO.md#alternate-and-command-shortcuts-on-plasma-231).
      [#231]
- [x] Show a searchable keyboard map of action names, descriptions, bindings and
      contexts. Search by action name or key combination and export a printable
      reference. Plasma-session verification is a
      [pending handoff](TODO.md#keyboard-map-on-plasma-232). [#232]
- [x] Provide shortcuts for pane and tab navigation, selection, filtering,
      preview and terminal focus as features become available, without
      overriding existing bindings by default. Plasma-session verification is
      a [pending handoff](TODO.md#navigation-shortcuts-on-plasma-236). [#236]
- [x] Offer optional Vim-style navigation in listings and panes with a
      discoverable shortcut reference. Keep it off by default and do not
      intercept typing in text fields or the embedded terminal.
      Plasma-session verification is a
      [pending handoff](TODO.md#vim-style-navigation-on-plasma-228). [#228]
- [x] Show the active Vim-style mode and provide a clear way to enter filtering
      or text without conflicting with navigation keys. Plasma-session
      verification is a
      [pending handoff](TODO.md#vim-style-mode-indicator-on-plasma-229).
      [#229]
- [x] Provide a native sliding or floating shortcut hint pane inspired by
      [which-key.nvim](https://github.com/folke/which-key.nvim). Show pending
      key sequences and context-appropriate continuations grouped with action
      descriptions; support keyboard navigation and dismissal. Plasma-session
      verification is a
      [pending handoff](TODO.md#shortcut-hints-on-plasma-234). [#234]
- [x] Configure the hint panel's enablement and display delay, and offer an
      on-demand shortcut reference. Cancel or time out incomplete sequences
      without unexpectedly running a conflicting action. Plasma-session
      verification is a
      [pending handoff](TODO.md#shortcut-hint-settings-on-plasma-235). [#235]
- [x] Offer a command palette through KDE's `KCommandBar`, as in Dolphin and
      Kate: search menu, toolbar and custom command actions by name and run
      them, showing each action's shortcut and recently used actions first.
      Skip disabled actions, apply pane actions to the active pane and leave
      shortcuts unchanged. (Lektra) Plasma-session verification is a
      [pending handoff](TODO.md#command-palette-on-plasma-761). [#761]
- [x] Offer keyboard hints: on a shortcut, label the visible items, breadcrumb
      segments, places and tabs of the active pane with short key sequences,
      and open or select a target when its keys are typed. Cancel with Escape,
      give labels accessible names and do not intercept typing in text fields
      or the embedded terminal. (Lektra) Plasma-session verification is a
      [pending handoff](TODO.md#keyboard-hints-on-plasma-762). [#762]
- [x] Import and export shortcut bindings with conflict preview; do not execute
      commands from imported files. Plasma-session verification is a
      [pending handoff](TODO.md#keymap-files-on-plasma-237). [#237]

### M32: Refinements

**Project:**
[M32 milestone](https://github.com/mikepadiernos/plasma-opus/issues/28)

Polish the existing interface across menus, settings, layouts and workflows.
Preserve user customizations and use KDE facilities; review future pages as they
become available without blocking this milestone on unimplemented work. See the
[editable M32 plans](refinements/README.md) for proposed sequencing and
subsection details.

- **M32.01: Menus**

  See the [menus plan](refinements/01-menus.md).

  - [ ] Synchronize menu and toolbar action state with the active pane,
        selection and location as tabs change or listings refresh. Keep
        context-menu actions targeted at the selection for which they were
        opened. [#482]
  - [ ] Review main, pane and context menus for logical groups, consistent
        action names and discoverable shortcuts. Reuse KXmlGui and KDE actions
        while preserving customized menus and destructive-action confirmations.
        [#458]
  - [ ] Make toolbar and pane actions consistent and discoverable, with overflow
        in narrow windows and a clear indication of the affected pane. Preserve
        toolbar customization and shortcut bindings. [#460]
  - [ ] Reorder, hide and restore Plasma Opus actions in context menus,
        extending the M02 action choices, with a reset to defaults. Keep KDE
        service-menu order and Dolphin-shared visibility separate. (MiXplorer)
        [#673]

- **M32.02: Layouts**

  See the [layouts plan](refinements/02-layouts.md).

  - [ ] Restore windows to a usable, visible position after monitor, resolution
        or scale changes while preserving saved geometry for when the display
        returns. Do not override placement decisions owned by the window
        manager. [#483]
  - [ ] Review splitter and panel behaviour when changing orientation,
        maximizing, hiding panels or swapping panes. Preserve each applicable
        saved layout and make it easy to recover a pane or panel that has become
        too small to use. [#484]
  - [ ] Adapt panes, panels, dialogs and menus to small windows, high DPI, long
        translations and right-to-left layouts. Keep both panes and essential
        controls accessible without discarding user-selected layouts. [#464]
  - [ ] Implement the five-region [interface design](philosophy/designs.md),
        with file panes in the middle column, sidebar panes in either side
        column and embedded panes in either side column, the middle column or
        footer. Preserve existing placements and saved layouts, and integrate
        the advanced-search panel and tag manager when available; see [#583].
        Review first-run pane, tab, panel, toolbar and window-layout defaults;
        apply revised defaults only to new configurations or an explicitly
        confirmed reset, never over saved layouts or sessions. [#583] [#461]
  - [ ] Extend the existing toolbar customizer so users can add, arrange and
        remove custom items, including saved commands and scripts, on the main
        toolbar and either pane toolbar. Preserve per-toolbar placement, labels
        and icons across restarts and report unavailable items without silently
        discarding them. [#492]
  - [ ] Provide a way to revert a toolbar's edits made during the current
        customization session and to reset an individual toolbar after
        confirmation. Keep other toolbars, menu entries and saved commands
        unchanged; explain which edits have already been saved. [#540]
  - [ ] Make built-in toolbar actions easy to find and add to existing
        customized toolbars, including actions introduced by updates. Offer a
        searchable, labelled reference as a sliding or floating pane without
        requiring a reset or automatically changing saved layouts. [#539]
  - [ ] Harmonize spacing, alignment, icons, labels and visual hierarchy across
        windows, panels, menus and dialogs. Follow the active KDE theme and icon
        conventions without hard-coding colours or replacing user-selected
        styling. [#462]

- **M32.03: Interaction**

  See the [interaction plan](refinements/03-interaction.md).

  - [ ] Prevent late listing, preview and search results from updating the wrong
        pane, tab or dialog after navigation, cancellation or closure. Retain
        the user's current selection and focus where possible. [#489]
  - [ ] Review destructive-operation confirmations to identify affected items,
        destination and consequences where applicable; retain KDE confirmation
        policies and use a safe default without introducing redundant prompts.
        [#487]
  - [ ] Clarify which pane supplies selected items and which location receives
        an operation before it runs, using accessible labels rather than colour
        alone. Allow changing the active pane without losing its selection and
        do not change KIO's operation or conflict semantics. [#542]
  - [ ] Make partial completion, skipped items and failures in batch operations
        understandable, with actionable details and a clear distinction between
        cancellation and rollback. Reuse existing KIO reporting where
        appropriate. [#488]
  - [ ] Explain loading, empty, filtered, unavailable and error states. Show
        progress and cancellation where supported, explain disabled actions and
        avoid duplicate KIO notifications or hidden failures. [#465]
  - [ ] Review keyboard navigation, focus restoration, screen-reader names and
        contrast across workflows. Reuse M31 shortcuts and KDE accessibility
        facilities without intercepting text or terminal input. [#463]
  - [ ] Give icon-only controls meaningful tooltips and accessible names,
        including relevant shortcuts and pane context where useful. Keep those
        hints current when the user changes shortcuts or toolbar configuration.
        [#485]
  - [ ] Make active file-list filters and their scope visible in the affected
        pane, including visible and hidden counts where available. Provide a
        clear way to remove the quick filter without implying that other filters
        were removed; distinguish filtering the current listing from searching
        other locations. [#543]
  - [ ] Display long, similar and bidirectional filenames clearly in listings
        and operation dialogs, with a way to inspect the full original name.
        Preserve the actual filename and avoid truncation that hides a
        meaningful extension. [#486]
  - [ ] Improve contextual help and discovery of pane actions, view modes and
        customization without mandatory onboarding or changes to existing
        settings. [#467]
  - [ ] Provide context-sensitive help from the main window, settings pages and
        supported dialogs using KDE help facilities and an accessible Help
        action. Open the relevant local documentation where available; explain
        missing help instead of silently failing, and avoid overriding user
        shortcut bindings. [#541]
  - [ ] Make adding and editing custom toolbar items discoverable and usable by
        keyboard as well as drag and drop. Pane toolbar items must use the
        intended pane and selection; preserve command preview and confirmation
        and do not delete the underlying saved command when removing a toolbar
        item. [#493]
  - [ ] Add subtle, consistent animations for UI transitions such as panels and
        dialogs where they improve orientation. Respect desktop animation and
        reduced-motion preferences where available, provide an app-level way to
        disable motion, and keep interactions responsive and states clear
        without it. Test both animated and motion-disabled behaviour. [#469]
  - [ ] After deleting, trashing or moving selected items out of a listing,
        select the next remaining item, or the previous one when none follows,
        so keyboard work can continue. Apply this only to the pane where the
        operation started; leave the selection alone when items disappear
        because of external changes. (Luciole) [#738]
  - [ ] Add a sort option that lists shown hidden files and folders after
        visible ones in every view; folders first applies within each group.
        Remember it with the other sort settings, including folder formats.
        (Luciole) [#739]
  - [ ] Add **Move into New Folder…** to the drop menu: ask for a folder name
        in the drop target, create the folder and move the dropped items into
        it. Extend KIO's drop menu rather than replacing it, never overwrite
        an existing folder, and record both steps in KIO's undo history.
        (Luciole) [#740]
  - [ ] Open an item's Properties by holding <kbd>Alt</kbd> with the usual
        open gesture: double-click, or a single click in single-click mode.
        Keep Alt-modified drag and drop and selection unchanged. (Luciole)
        [#741]
  - [ ] Drag a tab out of the tab bar to open it in a new window, keeping its
        location, history, view settings, name and pin, and close the original
        tab. Leave the last tab of a pane in place and keep the existing
        reopen-closed-tab history accurate. (Luciole) [#742]
  - [ ] Mark recursive folder sizes that are incomplete, for example because
        some subfolders could not be read, with ≥ in the Size column and a
        tooltip explaining why, so a partial total is not mistaken for the
        final size. (Luciole) [#743]
  - [ ] Add an operations panel that lists running and recently finished file
        operations with Cancel, Details, Copy Report and Retry Failed for
        items that failed or were skipped. Build on KIO jobs and the existing
        transfer queue, keep Plasma's job notifications, keep the history for
        the current session only and let users clear finished entries.
        (Luciole) [#744]
  - [ ] Before extracting an archive, estimate the space needed in the
        destination and any temporary folder, and warn when it may not fit,
        with a choice to continue. Explain that the check is an estimate and
        still report write errors, including for formats that do not record
        sizes. (Luciole) [#745]
  - [ ] Browse and extract single compressed files that are not tar archives,
        such as `dump.sql.gz`, `.bz2`, `.xz` and `.zst`, as one file named
        without the suffix. Reuse KArchive's compression devices, show the
        size as unknown until extracted and never overwrite an existing file.
        (Luciole) [#746]
  - [ ] Evaluate redo for undone file operations. KIO's undo manager has no
        redo, so a custom layer would repeat an undone operation only after
        checking that the affected items have not changed since, and refuse
        otherwise. Ship it only if a prototype proves reliable with KIO's undo
        history and remote locations. (Luciole) [#747]
  - [ ] Compare two selected images side by side in the gallery with linked
        zoom, pan and rotation, and step through either side independently.
        Keep the comparison local like the gallery, never modify the images
        and show a single image when only one is selected. (Lektra) [#763]
  - [ ] Open the document shown in the preview panel in an external viewer at
        the previewed page, and open two selected documents side by side, with
        viewers that support these on the command line, such as Okular and
        Lektra. Pass paths as arguments without a shell, use the page only
        where the preview provider reports it, and report a missing viewer,
        unsupported location or launch failure. Launch Lektra only as a
        separate application, because it is AGPL-licensed. (Lektra) [#764]
  - [ ] Add focused interaction tests for menu actions, default and restored
        layouts, keyboard and accessible navigation, responsive controls and
        error states; preserve existing user customizations in regression cases.
        [#468]

- **M32.04: Settings**

  See the [settings plan](refinements/04-settings.md).

  - [ ] Make settings persistence and recovery reliable across reopening the
        dialog, multiple windows and application restarts. Detect and report
        save failures or conflicting external changes without silently
        discarding user edits. [#397]
  - [ ] Review closing the settings dialog with pending changes, including
        window close and keyboard dismissal. Give a clear apply, discard or
        cancel choice where needed, without implying that immediate external
        changes are undone. [#490]
  - [ ] Explain Apply, OK and Cancel, including pending changes and restoration
        of live previews when changes are cancelled [#222]
  - [ ] Validate inputs with actionable inline errors and focus the affected
        control; explain unavailable or dependent options [#224]
  - [ ] Offer an explicit choice to restore defaults for the current page or all
        pages, with confirmation before discarding changes [#223]
  - [ ] Audit settings that open external KDE dialogs or immediately modify
        saved searches, collections, presets and commands. Make their scope and
        immediate effects clear before the action; do not imply that Cancel can
        undo changes owned by another component. [#396]
  - [ ] Clarify the boundaries of reset, backup and restore: preview affected
        app-owned settings, distinguish excluded credentials and shared KDE
        data, and preserve confirmation and restart requirements from M12.
        [#398]
  - [ ] Support keyboard navigation, visible focus, screen-reader labels and
        accessible error messages throughout the dialog [#225]
  - [ ] Adapt pages and navigation to small windows, high DPI, long translations
        and right-to-left layouts without clipping controls or labels [#226]
  - [ ] Search settings by name and description; show matching pages and open
        the relevant tab and control [#220]
  - [ ] Add keyboard-accessible filtering to existing long, searchable
        configuration lists where useful. Show when no entries match and let
        users clear the filter without changing hidden items or losing unsaved
        edits. [#545]
  - [ ] Explain setting scope and precedence where defaults can be overridden by
        folder formats, pane locks, operation choices, saved profiles or
        workspace layouts. Distinguish the saved default from the effective
        value without changing the underlying precedence rules. [#395]
  - [ ] Make page grouping, labels, descriptions and terminology consistent;
        explain settings that affect shared KDE configuration or require a
        restart [#221]
  - [ ] Provide concise contextual help where a setting's effects are not
        obvious [#227]
  - [ ] Let users mark frequently used settings pages for quick access and open
        a specific page from an appropriate interface action. Preserve page
        identity across navigation and restarts without changing settings
        values. [#544]
  - [ ] Keep settings pages responsive when loading large lists, profiles or
        provider data. Explain missing dependencies and permission failures
        without blocking the dialog or silently replacing saved choices. [#399]
  - [ ] Classify FUSE mounts without a local disk device, such as sshfs,
        rclone and gocryptfs, as remote for **Show remote previews** and
        automatic remote folder sizes, while FUSE file systems on a local
        disk, such as NTFS-3G, stay local. Explain the classification next to
        those settings. (Luciole) [#748]
  - [ ] Show the size of the thumbnail cache and offer to clear it after
        confirmation. Explain that the cache is shared with Dolphin and other
        applications and is rebuilt as folders are viewed. (Luciole) [#749]
  - [ ] Establish reusable search, accessibility, validation and help
        conventions for existing and future pages, including shortcuts, search,
        tagging, integrations, themes and branding. Review pages as implemented
        without blocking M32 on features that remain unavailable. [#400]
  - [ ] Add focused tests for settings search, keyboard access, validation,
        Apply/Cancel and defaults, persistence failures and shared or immediate
        changes across representative existing pages. Extend coverage as later
        settings pages are added. [#401]

- **M32.05: Performance and startup**

  See the [performance and startup plan](refinements/05-performance.md).

  - [ ] Profile and benchmark cold and warm startup through the first
        interactive window, settings opening and first-page readiness. Record
        representative configurations, timing distributions and memory use, then
        set measurable improvement targets against the baseline. [#477]
  - [ ] Reduce GUI-thread startup work by deferring nonessential initialization
        and loading where appropriate. Preserve restored sessions, pane
        readiness, error reporting and user-selected startup behaviour. [#478]
  - [ ] Speed up opening settings and switching pages through measured
        improvements such as lazy page creation and background loading where
        safe. Preserve search, validation, Apply/Cancel, immediate effects and
        unsaved changes, including for pages not yet opened. [#479]
  - [ ] Measure clean and incremental build times, identify bottlenecks and
        evaluate improvements against the same baseline without compromising
        build correctness or the existing CMake workflow. [#677]
  - [ ] Prototype and integrate Rust implementations for selected, profiled
        self-contained core hotspots where benchmarks demonstrate a benefit. Use
        the M21 pilot where applicable, a narrow safe C++/Rust interface and
        existing Qt/KDE facilities; account for build, packaging and dependency
        costs without rewriting widgets or replacing KIO. [#480]
  - [ ] Add reproducible performance checks and functional regression tests for
        startup, settings and any Rust-backed paths. Compare before and after
        under the same conditions, cover error and cancellation behaviour, and
        avoid hardware-dependent pass/fail thresholds in ordinary tests. [#481]

- **M32.06: Extensive functional and security testing**

  See the [testing plan](refinements/06-testing.md).

  Use the [edge-case inventory](EDGE.md) as input; its scenarios are not
  confirmed bugs and may overlap existing tests.

  - [ ] Inventory every implemented feature and map its supported entry points
        to reproducible automated or documented manual tests. Record gaps and
        unavailable features explicitly; use coverage reports to identify gaps
        without treating coverage as proof of correctness. Extend the matrix as
        functionality ships without blocking M32 on unimplemented milestones.
        [#470]
  - [ ] Review security-sensitive trust boundaries and test adversarial inputs
        for file operations, archives, paths and symbolic links, external
        commands, terminal integration and imported data. Verify validation,
        permissions, confirmations and error handling without executing
        untrusted code. [#473]
  - [ ] Test privacy and secret handling in settings, sessions, backups, logs,
        temporary files and remote connections. Use isolated test config, home
        and trash locations and synthetic credentials; never use real user data.
        [#474]
  - [ ] Run suitable static analysis and dependency checks and perform a
        documented security review. Evaluate tools such as clang-tidy and
        PVS-Studio, including licensing and integration costs, and use the C++
        Core Guidelines where compatible with Qt/KDE ownership and conventions.
        Track reproducible findings, severity and remediation separately; do not
        claim that testing proves the application is secure. [#475]
  - [ ] Run focused tests with AddressSanitizer and UndefinedBehaviorSanitizer
        on a supported toolchain. Evaluate ThreadSanitizer separately for
        concurrent paths and leak detection where supported. Consider
        MemorySanitizer or HWAddressSanitizer only where their toolchain and
        platform requirements can be met. Document unsupported environments and
        investigate findings before treating them as defects. [#678]
  - [ ] Evaluate running the applicable tests, formatting and analysis in CI
        using documented toolchains and dependencies. Keep sanitizer
        configurations separate where required and report unavailable checks
        explicitly. [#679]
  - [ ] Exercise workflows end to end alongside unit and integration tests.
        Cover successful operations, invalid input, cancellation, failures,
        recovery, persistence and feature interactions. Verify destructive
        operations and confirmations using isolated temporary fixtures. [#471]
  - [ ] Test accessibility, keyboard and pointer interaction, animations,
        layouts, translations and desktop configurations across a documented
        representative matrix. Include missing optional dependencies and local
        and remote failure scenarios; report environments that could not be
        exercised. [#472]
  - [ ] Conduct documented usability walkthroughs of common first-run and
        experienced workflows, including keyboard and assistive-technology use.
        Track confusing steps and regressions with reproducible examples and
        resolve or triage them. [#491]

## Feature references

The roadmap adapts workflows to KDE rather than copying platform-specific
interfaces. The additions above draw on these sources:

- [Dolphin view and selection](https://docs.kde.org/stable_kf6/en/dolphin/dolphin/dolphin-view.html)
  and
  [search](https://docs.kde.org/stable_kf6/en/dolphin/dolphin/quick-tips.html).
- [Dolphin panels](https://docs.kde.org/stable_kf6/en/dolphin/dolphin/panels.html)
  and
  [Directory Opus folder tabs](https://docs.dopus.com/doku.php?id=preferences:preferences_categories:folder_tabs:folder_tab_bar),
  [status bars](https://docs.dopus.com/doku.php?id=preferences:preferences_categories:file_displays:status_bar)
  and
  [viewer pane](https://docs.dopus.com/doku.php?id=basic_concepts:the_lister:viewer_pane).
- Directory Opus manual:
  [introduction](https://docs.dopus.com/doku.php?id=introduction),
  [preferences](https://docs.dopus.com/doku.php?id=preferences),
  [customization](https://docs.dopus.com/doku.php?id=customize:the_customize_dialog),
  [source and destination](https://docs.dopus.com/doku.php?id=basic_concepts:source_and_destination)
  and
  [filter bar](https://docs.dopus.com/doku.php?id=basic_concepts:searching_and_filtering:filter_bar).
- [Krusader synchronizer](https://docs.kde.org/trunk_kf6/en/krusader/krusader/synchronizer.html),
  [MountMan](https://docs.kde.org/trunk_kf6/en/krusader/krusader/mount-man.html)
  and
  [features](https://docs.kde.org/trunk_kf6/en/krusader/krusader/features.html).
- [Max File Manager](https://github.com/majidasgari/max-file-manager), alongside
  this project's [UI](parity/ui.md) and [backend](parity/backend.md)
  inventories.
- [KWin scripting](https://develop.kde.org/docs/plasma/kwin/), for window
  placement that Wayland applications cannot perform themselves, and
  [Yakuake](https://apps.kde.org/yakuake/)'s drop-down window.
- [Strata](https://github.com/lgse/strata), especially its opt-in system file
  chooser served through the XDG file chooser portal.
- [RaiDrive](https://www.raidrive.com/), which mounts cloud, S3-compatible and
  NAS storage as local drives, informing remote drives built on KIO FUSE and
  FUSE backends as alternatives to rclone.
- [Syncative](https://github.com/Santuzius/syncative),
  [SDrive](https://github.com/TitanniOpen/SDrive) and
  [DucksDrive](https://patopatetico2-lab.github.io/ducksdrive/), informing
  two-way cloud sync with backup modes and conflict choices, and the cloud
  activity panel for transfers, quota, pending uploads and cache limits.
- [MiXplorer](https://mixplorer.com/) and its
  [MiX Silver listing](https://play.google.com/store/apps/details?id=com.mixplorer.silver),
  an Android file manager whose encrypted containers, file encryption, LAN
  servers, cloud providers, archive editing, file-type categories and menu
  customization are adapted to KDE facilities.
- [Luciole](https://github.com/noctalia-dev/luciole), a Wayland file manager
  whose small workflow details, archive checks, operations panel and
  thumbnail policy inform M32 interaction and settings refinements and the
  file chooser's start folder.
- [Lektra](https://github.com/dheerajshenoy/lektra), a keyboard-driven
  document and image viewer whose command palette, link hints, synchronized
  views and page-aware command line inform M31 keyboard work and M32
  comparison and viewer handoff. Plasma Opus launches it only as a separate
  application, because it is AGPL-licensed.
- [Plasma Impresario](https://github.com/mikepadiernos/plasma-impresario), the
  companion KDE mail client, and its
  [Plasma Opus integration notes](https://github.com/mikepadiernos/plasma-impresario/blob/main/docs/integrations/plasma-opus.md).
- [TagStudio](https://github.com/TagStudioDev/TagStudio), especially its
  libraries, tag relationships and search.
- [TagSpaces workspaces](https://docs.tagspaces.org/workspaces/): workspace
  organization of locations, tag groups and search.
- [Directory Opus](https://www.gpsoft.com.au/) and its
  [file collections](https://docs.dopus.com/doku.php?id=basic_concepts:virtual_file_system:file_collections).
- Keyboard references:
  [Directory Opus keyboard map](https://resource.dopus.com/t/list-of-keyboard-shortcuts/26759),
  [Directory Opus shortcut editor](https://docs.dopus.com/doku.php?id=customize:the_customize_dialog:keys),
  [Krusader key bindings](https://docs.kde.org/trunk_kf6/en/krusader/krusader/key_bindings.html)
  and
  [Dolphin commands](https://docs.kde.org/stable_kf6/en/dolphin/dolphin/command-reference.html).
- Directory Opus Resource Centre:
  [folder formats](https://resource.dopus.com/t/folder-formats-detailed-guide/11718),
  [navigation history](https://resource.dopus.com/t/how-to-remember-scroll-positions-and-file-selections/1297),
  [folder sizes](https://resource.dopus.com/t/calculate-folder-sizes-automatically/2064),
  [duplicate selection](https://resource.dopus.com/t/find-duplicate-files/19799),
  [path filters](https://resource.dopus.com/t/how-to-filter-items-by-location-or-sub-folder/7644),
  [rename presets](https://resource.dopus.com/c/rename/30),
  [preview autoplay](https://resource.dopus.com/t/viewer-pane-turn-off-auto-play-for-video-and-audio/33976)
  and
  [configuration backup](https://resource.dopus.com/t/how-to-backup-or-locate-your-opus-configuration-simple/5465).

The existing Orcinus entries reflect earlier project notes; an authoritative
upstream source could not be verified for this review.

## Tracking

Follow the [project instructions](../CLAUDE.md) for issue tracking, completion,
branching and merging. Record blocked work in [pending tasks](TODO.md) and
document feature limitations in [caveats and limitations](CAVEATS.md). Native
dock and column-header spacing follows the
[active widget style](CAVEATS.md#native-title-and-header-padding) ([#560]).

Each feature item ends with a link to its board issue, such as [#44]. When
adding an item, create its issue first, then add the link and its definition at
the end of this file.

[#36]: https://github.com/mikepadiernos/plasma-opus/issues/36
[#38]: https://github.com/mikepadiernos/plasma-opus/issues/38
[#39]: https://github.com/mikepadiernos/plasma-opus/issues/39
[#40]: https://github.com/mikepadiernos/plasma-opus/issues/40
[#41]: https://github.com/mikepadiernos/plasma-opus/issues/41
[#42]: https://github.com/mikepadiernos/plasma-opus/issues/42
[#43]: https://github.com/mikepadiernos/plasma-opus/issues/43
[#44]: https://github.com/mikepadiernos/plasma-opus/issues/44
[#45]: https://github.com/mikepadiernos/plasma-opus/issues/45
[#46]: https://github.com/mikepadiernos/plasma-opus/issues/46
[#47]: https://github.com/mikepadiernos/plasma-opus/issues/47
[#48]: https://github.com/mikepadiernos/plasma-opus/issues/48
[#49]: https://github.com/mikepadiernos/plasma-opus/issues/49
[#50]: https://github.com/mikepadiernos/plasma-opus/issues/50
[#51]: https://github.com/mikepadiernos/plasma-opus/issues/51
[#52]: https://github.com/mikepadiernos/plasma-opus/issues/52
[#53]: https://github.com/mikepadiernos/plasma-opus/issues/53
[#54]: https://github.com/mikepadiernos/plasma-opus/issues/54
[#55]: https://github.com/mikepadiernos/plasma-opus/issues/55
[#56]: https://github.com/mikepadiernos/plasma-opus/issues/56
[#57]: https://github.com/mikepadiernos/plasma-opus/issues/57
[#58]: https://github.com/mikepadiernos/plasma-opus/issues/58
[#59]: https://github.com/mikepadiernos/plasma-opus/issues/59
[#60]: https://github.com/mikepadiernos/plasma-opus/issues/60
[#61]: https://github.com/mikepadiernos/plasma-opus/issues/61
[#62]: https://github.com/mikepadiernos/plasma-opus/issues/62
[#63]: https://github.com/mikepadiernos/plasma-opus/issues/63
[#64]: https://github.com/mikepadiernos/plasma-opus/issues/64
[#65]: https://github.com/mikepadiernos/plasma-opus/issues/65
[#66]: https://github.com/mikepadiernos/plasma-opus/issues/66
[#67]: https://github.com/mikepadiernos/plasma-opus/issues/67
[#68]: https://github.com/mikepadiernos/plasma-opus/issues/68
[#69]: https://github.com/mikepadiernos/plasma-opus/issues/69
[#70]: https://github.com/mikepadiernos/plasma-opus/issues/70
[#71]: https://github.com/mikepadiernos/plasma-opus/issues/71
[#72]: https://github.com/mikepadiernos/plasma-opus/issues/72
[#73]: https://github.com/mikepadiernos/plasma-opus/issues/73
[#74]: https://github.com/mikepadiernos/plasma-opus/issues/74
[#75]: https://github.com/mikepadiernos/plasma-opus/issues/75
[#76]: https://github.com/mikepadiernos/plasma-opus/issues/76
[#77]: https://github.com/mikepadiernos/plasma-opus/issues/77
[#78]: https://github.com/mikepadiernos/plasma-opus/issues/78
[#79]: https://github.com/mikepadiernos/plasma-opus/issues/79
[#80]: https://github.com/mikepadiernos/plasma-opus/issues/80
[#81]: https://github.com/mikepadiernos/plasma-opus/issues/81
[#82]: https://github.com/mikepadiernos/plasma-opus/issues/82
[#83]: https://github.com/mikepadiernos/plasma-opus/issues/83
[#84]: https://github.com/mikepadiernos/plasma-opus/issues/84
[#85]: https://github.com/mikepadiernos/plasma-opus/issues/85
[#86]: https://github.com/mikepadiernos/plasma-opus/issues/86
[#87]: https://github.com/mikepadiernos/plasma-opus/issues/87
[#88]: https://github.com/mikepadiernos/plasma-opus/issues/88
[#89]: https://github.com/mikepadiernos/plasma-opus/issues/89
[#90]: https://github.com/mikepadiernos/plasma-opus/issues/90
[#91]: https://github.com/mikepadiernos/plasma-opus/issues/91
[#92]: https://github.com/mikepadiernos/plasma-opus/issues/92
[#93]: https://github.com/mikepadiernos/plasma-opus/issues/93
[#94]: https://github.com/mikepadiernos/plasma-opus/issues/94
[#95]: https://github.com/mikepadiernos/plasma-opus/issues/95
[#96]: https://github.com/mikepadiernos/plasma-opus/issues/96
[#97]: https://github.com/mikepadiernos/plasma-opus/issues/97
[#98]: https://github.com/mikepadiernos/plasma-opus/issues/98
[#99]: https://github.com/mikepadiernos/plasma-opus/issues/99
[#100]: https://github.com/mikepadiernos/plasma-opus/issues/100
[#101]: https://github.com/mikepadiernos/plasma-opus/issues/101
[#102]: https://github.com/mikepadiernos/plasma-opus/issues/102
[#103]: https://github.com/mikepadiernos/plasma-opus/issues/103
[#104]: https://github.com/mikepadiernos/plasma-opus/issues/104
[#105]: https://github.com/mikepadiernos/plasma-opus/issues/105
[#106]: https://github.com/mikepadiernos/plasma-opus/issues/106
[#107]: https://github.com/mikepadiernos/plasma-opus/issues/107
[#108]: https://github.com/mikepadiernos/plasma-opus/issues/108
[#109]: https://github.com/mikepadiernos/plasma-opus/issues/109
[#110]: https://github.com/mikepadiernos/plasma-opus/issues/110
[#111]: https://github.com/mikepadiernos/plasma-opus/issues/111
[#112]: https://github.com/mikepadiernos/plasma-opus/issues/112
[#113]: https://github.com/mikepadiernos/plasma-opus/issues/113
[#114]: https://github.com/mikepadiernos/plasma-opus/issues/114
[#115]: https://github.com/mikepadiernos/plasma-opus/issues/115
[#116]: https://github.com/mikepadiernos/plasma-opus/issues/116
[#117]: https://github.com/mikepadiernos/plasma-opus/issues/117
[#118]: https://github.com/mikepadiernos/plasma-opus/issues/118
[#119]: https://github.com/mikepadiernos/plasma-opus/issues/119
[#120]: https://github.com/mikepadiernos/plasma-opus/issues/120
[#121]: https://github.com/mikepadiernos/plasma-opus/issues/121
[#122]: https://github.com/mikepadiernos/plasma-opus/issues/122
[#123]: https://github.com/mikepadiernos/plasma-opus/issues/123
[#124]: https://github.com/mikepadiernos/plasma-opus/issues/124
[#125]: https://github.com/mikepadiernos/plasma-opus/issues/125
[#126]: https://github.com/mikepadiernos/plasma-opus/issues/126
[#127]: https://github.com/mikepadiernos/plasma-opus/issues/127
[#128]: https://github.com/mikepadiernos/plasma-opus/issues/128
[#129]: https://github.com/mikepadiernos/plasma-opus/issues/129
[#130]: https://github.com/mikepadiernos/plasma-opus/issues/130
[#131]: https://github.com/mikepadiernos/plasma-opus/issues/131
[#132]: https://github.com/mikepadiernos/plasma-opus/issues/132
[#133]: https://github.com/mikepadiernos/plasma-opus/issues/133
[#134]: https://github.com/mikepadiernos/plasma-opus/issues/134
[#135]: https://github.com/mikepadiernos/plasma-opus/issues/135
[#136]: https://github.com/mikepadiernos/plasma-opus/issues/136
[#137]: https://github.com/mikepadiernos/plasma-opus/issues/137
[#138]: https://github.com/mikepadiernos/plasma-opus/issues/138
[#139]: https://github.com/mikepadiernos/plasma-opus/issues/139
[#140]: https://github.com/mikepadiernos/plasma-opus/issues/140
[#141]: https://github.com/mikepadiernos/plasma-opus/issues/141
[#142]: https://github.com/mikepadiernos/plasma-opus/issues/142
[#143]: https://github.com/mikepadiernos/plasma-opus/issues/143
[#144]: https://github.com/mikepadiernos/plasma-opus/issues/144
[#145]: https://github.com/mikepadiernos/plasma-opus/issues/145
[#146]: https://github.com/mikepadiernos/plasma-opus/issues/146
[#147]: https://github.com/mikepadiernos/plasma-opus/issues/147
[#148]: https://github.com/mikepadiernos/plasma-opus/issues/148
[#149]: https://github.com/mikepadiernos/plasma-opus/issues/149
[#150]: https://github.com/mikepadiernos/plasma-opus/issues/150
[#151]: https://github.com/mikepadiernos/plasma-opus/issues/151
[#152]: https://github.com/mikepadiernos/plasma-opus/issues/152
[#153]: https://github.com/mikepadiernos/plasma-opus/issues/153
[#154]: https://github.com/mikepadiernos/plasma-opus/issues/154
[#155]: https://github.com/mikepadiernos/plasma-opus/issues/155
[#156]: https://github.com/mikepadiernos/plasma-opus/issues/156
[#157]: https://github.com/mikepadiernos/plasma-opus/issues/157
[#158]: https://github.com/mikepadiernos/plasma-opus/issues/158
[#159]: https://github.com/mikepadiernos/plasma-opus/issues/159
[#160]: https://github.com/mikepadiernos/plasma-opus/issues/160
[#161]: https://github.com/mikepadiernos/plasma-opus/issues/161
[#162]: https://github.com/mikepadiernos/plasma-opus/issues/162
[#163]: https://github.com/mikepadiernos/plasma-opus/issues/163
[#164]: https://github.com/mikepadiernos/plasma-opus/issues/164
[#165]: https://github.com/mikepadiernos/plasma-opus/issues/165
[#166]: https://github.com/mikepadiernos/plasma-opus/issues/166
[#167]: https://github.com/mikepadiernos/plasma-opus/issues/167
[#168]: https://github.com/mikepadiernos/plasma-opus/issues/168
[#169]: https://github.com/mikepadiernos/plasma-opus/issues/169
[#170]: https://github.com/mikepadiernos/plasma-opus/issues/170
[#171]: https://github.com/mikepadiernos/plasma-opus/issues/171
[#172]: https://github.com/mikepadiernos/plasma-opus/issues/172
[#173]: https://github.com/mikepadiernos/plasma-opus/issues/173
[#174]: https://github.com/mikepadiernos/plasma-opus/issues/174
[#175]: https://github.com/mikepadiernos/plasma-opus/issues/175
[#176]: https://github.com/mikepadiernos/plasma-opus/issues/176
[#177]: https://github.com/mikepadiernos/plasma-opus/issues/177
[#178]: https://github.com/mikepadiernos/plasma-opus/issues/178
[#179]: https://github.com/mikepadiernos/plasma-opus/issues/179
[#180]: https://github.com/mikepadiernos/plasma-opus/issues/180
[#181]: https://github.com/mikepadiernos/plasma-opus/issues/181
[#182]: https://github.com/mikepadiernos/plasma-opus/issues/182
[#183]: https://github.com/mikepadiernos/plasma-opus/issues/183
[#184]: https://github.com/mikepadiernos/plasma-opus/issues/184
[#185]: https://github.com/mikepadiernos/plasma-opus/issues/185
[#186]: https://github.com/mikepadiernos/plasma-opus/issues/186
[#187]: https://github.com/mikepadiernos/plasma-opus/issues/187
[#188]: https://github.com/mikepadiernos/plasma-opus/issues/188
[#189]: https://github.com/mikepadiernos/plasma-opus/issues/189
[#190]: https://github.com/mikepadiernos/plasma-opus/issues/190
[#191]: https://github.com/mikepadiernos/plasma-opus/issues/191
[#192]: https://github.com/mikepadiernos/plasma-opus/issues/192
[#193]: https://github.com/mikepadiernos/plasma-opus/issues/193
[#194]: https://github.com/mikepadiernos/plasma-opus/issues/194
[#195]: https://github.com/mikepadiernos/plasma-opus/issues/195
[#196]: https://github.com/mikepadiernos/plasma-opus/issues/196
[#197]: https://github.com/mikepadiernos/plasma-opus/issues/197
[#198]: https://github.com/mikepadiernos/plasma-opus/issues/198
[#199]: https://github.com/mikepadiernos/plasma-opus/issues/199
[#200]: https://github.com/mikepadiernos/plasma-opus/issues/200
[#201]: https://github.com/mikepadiernos/plasma-opus/issues/201
[#202]: https://github.com/mikepadiernos/plasma-opus/issues/202
[#203]: https://github.com/mikepadiernos/plasma-opus/issues/203
[#204]: https://github.com/mikepadiernos/plasma-opus/issues/204
[#205]: https://github.com/mikepadiernos/plasma-opus/issues/205
[#206]: https://github.com/mikepadiernos/plasma-opus/issues/206
[#207]: https://github.com/mikepadiernos/plasma-opus/issues/207
[#208]: https://github.com/mikepadiernos/plasma-opus/issues/208
[#209]: https://github.com/mikepadiernos/plasma-opus/issues/209
[#210]: https://github.com/mikepadiernos/plasma-opus/issues/210
[#212]: https://github.com/mikepadiernos/plasma-opus/issues/212
[#213]: https://github.com/mikepadiernos/plasma-opus/issues/213
[#214]: https://github.com/mikepadiernos/plasma-opus/issues/214
[#215]: https://github.com/mikepadiernos/plasma-opus/issues/215
[#216]: https://github.com/mikepadiernos/plasma-opus/issues/216
[#217]: https://github.com/mikepadiernos/plasma-opus/issues/217
[#218]: https://github.com/mikepadiernos/plasma-opus/issues/218
[#219]: https://github.com/mikepadiernos/plasma-opus/issues/219
[#220]: https://github.com/mikepadiernos/plasma-opus/issues/220
[#221]: https://github.com/mikepadiernos/plasma-opus/issues/221
[#222]: https://github.com/mikepadiernos/plasma-opus/issues/222
[#223]: https://github.com/mikepadiernos/plasma-opus/issues/223
[#224]: https://github.com/mikepadiernos/plasma-opus/issues/224
[#225]: https://github.com/mikepadiernos/plasma-opus/issues/225
[#226]: https://github.com/mikepadiernos/plasma-opus/issues/226
[#227]: https://github.com/mikepadiernos/plasma-opus/issues/227
[#228]: https://github.com/mikepadiernos/plasma-opus/issues/228
[#229]: https://github.com/mikepadiernos/plasma-opus/issues/229
[#230]: https://github.com/mikepadiernos/plasma-opus/issues/230
[#231]: https://github.com/mikepadiernos/plasma-opus/issues/231
[#232]: https://github.com/mikepadiernos/plasma-opus/issues/232
[#233]: https://github.com/mikepadiernos/plasma-opus/issues/233
[#234]: https://github.com/mikepadiernos/plasma-opus/issues/234
[#235]: https://github.com/mikepadiernos/plasma-opus/issues/235
[#236]: https://github.com/mikepadiernos/plasma-opus/issues/236
[#237]: https://github.com/mikepadiernos/plasma-opus/issues/237
[#238]: https://github.com/mikepadiernos/plasma-opus/issues/238
[#239]: https://github.com/mikepadiernos/plasma-opus/issues/239
[#240]: https://github.com/mikepadiernos/plasma-opus/issues/240
[#241]: https://github.com/mikepadiernos/plasma-opus/issues/241
[#242]: https://github.com/mikepadiernos/plasma-opus/issues/242
[#243]: https://github.com/mikepadiernos/plasma-opus/issues/243
[#244]: https://github.com/mikepadiernos/plasma-opus/issues/244
[#245]: https://github.com/mikepadiernos/plasma-opus/issues/245
[#246]: https://github.com/mikepadiernos/plasma-opus/issues/246
[#247]: https://github.com/mikepadiernos/plasma-opus/issues/247
[#248]: https://github.com/mikepadiernos/plasma-opus/issues/248
[#249]: https://github.com/mikepadiernos/plasma-opus/issues/249
[#250]: https://github.com/mikepadiernos/plasma-opus/issues/250
[#251]: https://github.com/mikepadiernos/plasma-opus/issues/251
[#252]: https://github.com/mikepadiernos/plasma-opus/issues/252
[#253]: https://github.com/mikepadiernos/plasma-opus/issues/253
[#254]: https://github.com/mikepadiernos/plasma-opus/issues/254
[#255]: https://github.com/mikepadiernos/plasma-opus/issues/255
[#256]: https://github.com/mikepadiernos/plasma-opus/issues/256
[#259]: https://github.com/mikepadiernos/plasma-opus/issues/259
[#260]: https://github.com/mikepadiernos/plasma-opus/issues/260
[#261]: https://github.com/mikepadiernos/plasma-opus/issues/261
[#262]: https://github.com/mikepadiernos/plasma-opus/issues/262
[#263]: https://github.com/mikepadiernos/plasma-opus/issues/263
[#264]: https://github.com/mikepadiernos/plasma-opus/issues/264
[#265]: https://github.com/mikepadiernos/plasma-opus/issues/265
[#266]: https://github.com/mikepadiernos/plasma-opus/issues/266
[#267]: https://github.com/mikepadiernos/plasma-opus/issues/267
[#268]: https://github.com/mikepadiernos/plasma-opus/issues/268
[#269]: https://github.com/mikepadiernos/plasma-opus/issues/269
[#270]: https://github.com/mikepadiernos/plasma-opus/issues/270
[#271]: https://github.com/mikepadiernos/plasma-opus/issues/271
[#272]: https://github.com/mikepadiernos/plasma-opus/issues/272
[#273]: https://github.com/mikepadiernos/plasma-opus/issues/273
[#274]: https://github.com/mikepadiernos/plasma-opus/issues/274
[#275]: https://github.com/mikepadiernos/plasma-opus/issues/275
[#276]: https://github.com/mikepadiernos/plasma-opus/issues/276
[#277]: https://github.com/mikepadiernos/plasma-opus/issues/277
[#278]: https://github.com/mikepadiernos/plasma-opus/issues/278
[#279]: https://github.com/mikepadiernos/plasma-opus/issues/279
[#280]: https://github.com/mikepadiernos/plasma-opus/issues/280
[#281]: https://github.com/mikepadiernos/plasma-opus/issues/281
[#282]: https://github.com/mikepadiernos/plasma-opus/issues/282
[#283]: https://github.com/mikepadiernos/plasma-opus/issues/283
[#284]: https://github.com/mikepadiernos/plasma-opus/issues/284
[#285]: https://github.com/mikepadiernos/plasma-opus/issues/285
[#286]: https://github.com/mikepadiernos/plasma-opus/issues/286
[#287]: https://github.com/mikepadiernos/plasma-opus/issues/287
[#288]: https://github.com/mikepadiernos/plasma-opus/issues/288
[#289]: https://github.com/mikepadiernos/plasma-opus/issues/289
[#290]: https://github.com/mikepadiernos/plasma-opus/issues/290
[#291]: https://github.com/mikepadiernos/plasma-opus/issues/291
[#292]: https://github.com/mikepadiernos/plasma-opus/issues/292
[#293]: https://github.com/mikepadiernos/plasma-opus/issues/293
[#294]: https://github.com/mikepadiernos/plasma-opus/issues/294
[#295]: https://github.com/mikepadiernos/plasma-opus/issues/295
[#296]: https://github.com/mikepadiernos/plasma-opus/issues/296
[#297]: https://github.com/mikepadiernos/plasma-opus/issues/297
[#298]: https://github.com/mikepadiernos/plasma-opus/issues/298
[#303]: https://github.com/mikepadiernos/plasma-opus/issues/303
[#307]: https://github.com/mikepadiernos/plasma-opus/issues/307
[#309]: https://github.com/mikepadiernos/plasma-opus/issues/309
[#311]: https://github.com/mikepadiernos/plasma-opus/issues/311
[#312]: https://github.com/mikepadiernos/plasma-opus/issues/312
[#314]: https://github.com/mikepadiernos/plasma-opus/issues/314
[#315]: https://github.com/mikepadiernos/plasma-opus/issues/315
[#316]: https://github.com/mikepadiernos/plasma-opus/issues/316
[#317]: https://github.com/mikepadiernos/plasma-opus/issues/317
[#318]: https://github.com/mikepadiernos/plasma-opus/issues/318
[#319]: https://github.com/mikepadiernos/plasma-opus/issues/319
[#320]: https://github.com/mikepadiernos/plasma-opus/issues/320
[#321]: https://github.com/mikepadiernos/plasma-opus/issues/321
[#322]: https://github.com/mikepadiernos/plasma-opus/issues/322
[#323]: https://github.com/mikepadiernos/plasma-opus/issues/323
[#324]: https://github.com/mikepadiernos/plasma-opus/issues/324
[#325]: https://github.com/mikepadiernos/plasma-opus/issues/325
[#326]: https://github.com/mikepadiernos/plasma-opus/issues/326
[#327]: https://github.com/mikepadiernos/plasma-opus/issues/327
[#328]: https://github.com/mikepadiernos/plasma-opus/issues/328
[#329]: https://github.com/mikepadiernos/plasma-opus/issues/329
[#330]: https://github.com/mikepadiernos/plasma-opus/issues/330
[#331]: https://github.com/mikepadiernos/plasma-opus/issues/331
[#332]: https://github.com/mikepadiernos/plasma-opus/issues/332
[#333]: https://github.com/mikepadiernos/plasma-opus/issues/333
[#334]: https://github.com/mikepadiernos/plasma-opus/issues/334
[#335]: https://github.com/mikepadiernos/plasma-opus/issues/335
[#336]: https://github.com/mikepadiernos/plasma-opus/issues/336
[#338]: https://github.com/mikepadiernos/plasma-opus/issues/338
[#339]: https://github.com/mikepadiernos/plasma-opus/issues/339
[#340]: https://github.com/mikepadiernos/plasma-opus/issues/340
[#342]: https://github.com/mikepadiernos/plasma-opus/issues/342
[#343]: https://github.com/mikepadiernos/plasma-opus/issues/343
[#344]: https://github.com/mikepadiernos/plasma-opus/issues/344
[#345]: https://github.com/mikepadiernos/plasma-opus/issues/345
[#347]: https://github.com/mikepadiernos/plasma-opus/issues/347
[#348]: https://github.com/mikepadiernos/plasma-opus/issues/348
[#349]: https://github.com/mikepadiernos/plasma-opus/issues/349
[#350]: https://github.com/mikepadiernos/plasma-opus/issues/350
[#351]: https://github.com/mikepadiernos/plasma-opus/issues/351
[#353]: https://github.com/mikepadiernos/plasma-opus/issues/353
[#354]: https://github.com/mikepadiernos/plasma-opus/issues/354
[#355]: https://github.com/mikepadiernos/plasma-opus/issues/355
[#356]: https://github.com/mikepadiernos/plasma-opus/issues/356
[#357]: https://github.com/mikepadiernos/plasma-opus/issues/357
[#358]: https://github.com/mikepadiernos/plasma-opus/issues/358
[#359]: https://github.com/mikepadiernos/plasma-opus/issues/359
[#360]: https://github.com/mikepadiernos/plasma-opus/issues/360
[#361]: https://github.com/mikepadiernos/plasma-opus/issues/361
[#362]: https://github.com/mikepadiernos/plasma-opus/issues/362
[#363]: https://github.com/mikepadiernos/plasma-opus/issues/363
[#364]: https://github.com/mikepadiernos/plasma-opus/issues/364
[#365]: https://github.com/mikepadiernos/plasma-opus/issues/365
[#366]: https://github.com/mikepadiernos/plasma-opus/issues/366
[#367]: https://github.com/mikepadiernos/plasma-opus/issues/367
[#368]: https://github.com/mikepadiernos/plasma-opus/issues/368
[#369]: https://github.com/mikepadiernos/plasma-opus/issues/369
[#370]: https://github.com/mikepadiernos/plasma-opus/issues/370
[#371]: https://github.com/mikepadiernos/plasma-opus/issues/371
[#372]: https://github.com/mikepadiernos/plasma-opus/issues/372
[#373]: https://github.com/mikepadiernos/plasma-opus/issues/373
[#374]: https://github.com/mikepadiernos/plasma-opus/issues/374
[#375]: https://github.com/mikepadiernos/plasma-opus/issues/375
[#376]: https://github.com/mikepadiernos/plasma-opus/issues/376
[#377]: https://github.com/mikepadiernos/plasma-opus/issues/377
[#380]: https://github.com/mikepadiernos/plasma-opus/issues/380
[#381]: https://github.com/mikepadiernos/plasma-opus/issues/381
[#387]: https://github.com/mikepadiernos/plasma-opus/issues/387
[#388]: https://github.com/mikepadiernos/plasma-opus/issues/388
[#389]: https://github.com/mikepadiernos/plasma-opus/issues/389
[#390]: https://github.com/mikepadiernos/plasma-opus/issues/390
[#391]: https://github.com/mikepadiernos/plasma-opus/issues/391
[#392]: https://github.com/mikepadiernos/plasma-opus/issues/392
[#393]: https://github.com/mikepadiernos/plasma-opus/issues/393
[#394]: https://github.com/mikepadiernos/plasma-opus/issues/394
[#395]: https://github.com/mikepadiernos/plasma-opus/issues/395
[#396]: https://github.com/mikepadiernos/plasma-opus/issues/396
[#397]: https://github.com/mikepadiernos/plasma-opus/issues/397
[#398]: https://github.com/mikepadiernos/plasma-opus/issues/398
[#399]: https://github.com/mikepadiernos/plasma-opus/issues/399
[#400]: https://github.com/mikepadiernos/plasma-opus/issues/400
[#401]: https://github.com/mikepadiernos/plasma-opus/issues/401
[#402]: https://github.com/mikepadiernos/plasma-opus/issues/402
[#403]: https://github.com/mikepadiernos/plasma-opus/issues/403
[#406]: https://github.com/mikepadiernos/plasma-opus/issues/406
[#408]: https://github.com/mikepadiernos/plasma-opus/issues/408
[#409]: https://github.com/mikepadiernos/plasma-opus/issues/409
[#410]: https://github.com/mikepadiernos/plasma-opus/issues/410
[#411]: https://github.com/mikepadiernos/plasma-opus/issues/411
[#412]: https://github.com/mikepadiernos/plasma-opus/issues/412
[#413]: https://github.com/mikepadiernos/plasma-opus/issues/413
[#414]: https://github.com/mikepadiernos/plasma-opus/issues/414
[#415]: https://github.com/mikepadiernos/plasma-opus/issues/415
[#417]: https://github.com/mikepadiernos/plasma-opus/issues/417
[#418]: https://github.com/mikepadiernos/plasma-opus/issues/418
[#419]: https://github.com/mikepadiernos/plasma-opus/issues/419
[#420]: https://github.com/mikepadiernos/plasma-opus/issues/420
[#421]: https://github.com/mikepadiernos/plasma-opus/issues/421
[#422]: https://github.com/mikepadiernos/plasma-opus/issues/422
[#423]: https://github.com/mikepadiernos/plasma-opus/issues/423
[#424]: https://github.com/mikepadiernos/plasma-opus/issues/424
[#425]: https://github.com/mikepadiernos/plasma-opus/issues/425
[#426]: https://github.com/mikepadiernos/plasma-opus/issues/426
[#427]: https://github.com/mikepadiernos/plasma-opus/issues/427
[#428]: https://github.com/mikepadiernos/plasma-opus/issues/428
[#429]: https://github.com/mikepadiernos/plasma-opus/issues/429
[#430]: https://github.com/mikepadiernos/plasma-opus/issues/430
[#431]: https://github.com/mikepadiernos/plasma-opus/issues/431
[#432]: https://github.com/mikepadiernos/plasma-opus/issues/432
[#433]: https://github.com/mikepadiernos/plasma-opus/issues/433
[#434]: https://github.com/mikepadiernos/plasma-opus/issues/434
[#436]: https://github.com/mikepadiernos/plasma-opus/issues/436
[#437]: https://github.com/mikepadiernos/plasma-opus/issues/437
[#440]: https://github.com/mikepadiernos/plasma-opus/issues/440
[#441]: https://github.com/mikepadiernos/plasma-opus/issues/441
[#442]: https://github.com/mikepadiernos/plasma-opus/issues/442
[#443]: https://github.com/mikepadiernos/plasma-opus/issues/443
[#458]: https://github.com/mikepadiernos/plasma-opus/issues/458
[#460]: https://github.com/mikepadiernos/plasma-opus/issues/460
[#461]: https://github.com/mikepadiernos/plasma-opus/issues/461
[#462]: https://github.com/mikepadiernos/plasma-opus/issues/462
[#463]: https://github.com/mikepadiernos/plasma-opus/issues/463
[#464]: https://github.com/mikepadiernos/plasma-opus/issues/464
[#465]: https://github.com/mikepadiernos/plasma-opus/issues/465
[#467]: https://github.com/mikepadiernos/plasma-opus/issues/467
[#468]: https://github.com/mikepadiernos/plasma-opus/issues/468
[#469]: https://github.com/mikepadiernos/plasma-opus/issues/469
[#470]: https://github.com/mikepadiernos/plasma-opus/issues/470
[#471]: https://github.com/mikepadiernos/plasma-opus/issues/471
[#472]: https://github.com/mikepadiernos/plasma-opus/issues/472
[#473]: https://github.com/mikepadiernos/plasma-opus/issues/473
[#474]: https://github.com/mikepadiernos/plasma-opus/issues/474
[#475]: https://github.com/mikepadiernos/plasma-opus/issues/475
[#477]: https://github.com/mikepadiernos/plasma-opus/issues/477
[#478]: https://github.com/mikepadiernos/plasma-opus/issues/478
[#479]: https://github.com/mikepadiernos/plasma-opus/issues/479
[#480]: https://github.com/mikepadiernos/plasma-opus/issues/480
[#481]: https://github.com/mikepadiernos/plasma-opus/issues/481
[#482]: https://github.com/mikepadiernos/plasma-opus/issues/482
[#483]: https://github.com/mikepadiernos/plasma-opus/issues/483
[#484]: https://github.com/mikepadiernos/plasma-opus/issues/484
[#485]: https://github.com/mikepadiernos/plasma-opus/issues/485
[#486]: https://github.com/mikepadiernos/plasma-opus/issues/486
[#487]: https://github.com/mikepadiernos/plasma-opus/issues/487
[#488]: https://github.com/mikepadiernos/plasma-opus/issues/488
[#489]: https://github.com/mikepadiernos/plasma-opus/issues/489
[#490]: https://github.com/mikepadiernos/plasma-opus/issues/490
[#491]: https://github.com/mikepadiernos/plasma-opus/issues/491
[#492]: https://github.com/mikepadiernos/plasma-opus/issues/492
[#493]: https://github.com/mikepadiernos/plasma-opus/issues/493
[#539]: https://github.com/mikepadiernos/plasma-opus/issues/539
[#540]: https://github.com/mikepadiernos/plasma-opus/issues/540
[#541]: https://github.com/mikepadiernos/plasma-opus/issues/541
[#542]: https://github.com/mikepadiernos/plasma-opus/issues/542
[#543]: https://github.com/mikepadiernos/plasma-opus/issues/543
[#544]: https://github.com/mikepadiernos/plasma-opus/issues/544
[#545]: https://github.com/mikepadiernos/plasma-opus/issues/545
[#560]: https://github.com/mikepadiernos/plasma-opus/issues/560
[#563]: https://github.com/mikepadiernos/plasma-opus/issues/563
[#564]: https://github.com/mikepadiernos/plasma-opus/issues/564
[#565]: https://github.com/mikepadiernos/plasma-opus/issues/565
[#566]: https://github.com/mikepadiernos/plasma-opus/issues/566
[#577]: https://github.com/mikepadiernos/plasma-opus/issues/577
[#578]: https://github.com/mikepadiernos/plasma-opus/issues/578
[#583]: https://github.com/mikepadiernos/plasma-opus/issues/583
[#585]: https://github.com/mikepadiernos/plasma-opus/issues/585
[#588]: https://github.com/mikepadiernos/plasma-opus/issues/588
[#589]: https://github.com/mikepadiernos/plasma-opus/issues/589
[#590]: https://github.com/mikepadiernos/plasma-opus/issues/590
[#591]: https://github.com/mikepadiernos/plasma-opus/issues/591
[#592]: https://github.com/mikepadiernos/plasma-opus/issues/592
[#609]: https://github.com/mikepadiernos/plasma-opus/issues/609
[#611]: https://github.com/mikepadiernos/plasma-opus/issues/611
[#612]: https://github.com/mikepadiernos/plasma-opus/issues/612
[#613]: https://github.com/mikepadiernos/plasma-opus/issues/613
[#614]: https://github.com/mikepadiernos/plasma-opus/issues/614
[#615]: https://github.com/mikepadiernos/plasma-opus/issues/615
[#616]: https://github.com/mikepadiernos/plasma-opus/issues/616
[#617]: https://github.com/mikepadiernos/plasma-opus/issues/617
[#618]: https://github.com/mikepadiernos/plasma-opus/issues/618
[#619]: https://github.com/mikepadiernos/plasma-opus/issues/619
[#620]: https://github.com/mikepadiernos/plasma-opus/issues/620
[#621]: https://github.com/mikepadiernos/plasma-opus/issues/621
[#622]: https://github.com/mikepadiernos/plasma-opus/issues/622
[#623]: https://github.com/mikepadiernos/plasma-opus/issues/623
[#624]: https://github.com/mikepadiernos/plasma-opus/issues/624
[#625]: https://github.com/mikepadiernos/plasma-opus/issues/625
[#626]: https://github.com/mikepadiernos/plasma-opus/issues/626
[#627]: https://github.com/mikepadiernos/plasma-opus/issues/627
[#628]: https://github.com/mikepadiernos/plasma-opus/issues/628
[#629]: https://github.com/mikepadiernos/plasma-opus/issues/629
[#630]: https://github.com/mikepadiernos/plasma-opus/issues/630
[#631]: https://github.com/mikepadiernos/plasma-opus/issues/631
[#632]: https://github.com/mikepadiernos/plasma-opus/issues/632
[#633]: https://github.com/mikepadiernos/plasma-opus/issues/633
[#634]: https://github.com/mikepadiernos/plasma-opus/issues/634
[#635]: https://github.com/mikepadiernos/plasma-opus/issues/635
[#636]: https://github.com/mikepadiernos/plasma-opus/issues/636
[#637]: https://github.com/mikepadiernos/plasma-opus/issues/637
[#638]: https://github.com/mikepadiernos/plasma-opus/issues/638
[#639]: https://github.com/mikepadiernos/plasma-opus/issues/639
[#640]: https://github.com/mikepadiernos/plasma-opus/issues/640
[#641]: https://github.com/mikepadiernos/plasma-opus/issues/641
[#642]: https://github.com/mikepadiernos/plasma-opus/issues/642
[#643]: https://github.com/mikepadiernos/plasma-opus/issues/643
[#644]: https://github.com/mikepadiernos/plasma-opus/issues/644
[#645]: https://github.com/mikepadiernos/plasma-opus/issues/645
[#646]: https://github.com/mikepadiernos/plasma-opus/issues/646
[#647]: https://github.com/mikepadiernos/plasma-opus/issues/647
[#648]: https://github.com/mikepadiernos/plasma-opus/issues/648
[#649]: https://github.com/mikepadiernos/plasma-opus/issues/649
[#650]: https://github.com/mikepadiernos/plasma-opus/issues/650
[#651]: https://github.com/mikepadiernos/plasma-opus/issues/651
[#652]: https://github.com/mikepadiernos/plasma-opus/issues/652
[#653]: https://github.com/mikepadiernos/plasma-opus/issues/653
[#654]: https://github.com/mikepadiernos/plasma-opus/issues/654
[#655]: https://github.com/mikepadiernos/plasma-opus/issues/655
[#656]: https://github.com/mikepadiernos/plasma-opus/issues/656
[#657]: https://github.com/mikepadiernos/plasma-opus/issues/657
[#658]: https://github.com/mikepadiernos/plasma-opus/issues/658
[#659]: https://github.com/mikepadiernos/plasma-opus/issues/659
[#660]: https://github.com/mikepadiernos/plasma-opus/issues/660
[#661]: https://github.com/mikepadiernos/plasma-opus/issues/661
[#662]: https://github.com/mikepadiernos/plasma-opus/issues/662
[#663]: https://github.com/mikepadiernos/plasma-opus/issues/663
[#664]: https://github.com/mikepadiernos/plasma-opus/issues/664
[#665]: https://github.com/mikepadiernos/plasma-opus/issues/665
[#666]: https://github.com/mikepadiernos/plasma-opus/issues/666
[#667]: https://github.com/mikepadiernos/plasma-opus/issues/667
[#668]: https://github.com/mikepadiernos/plasma-opus/issues/668
[#669]: https://github.com/mikepadiernos/plasma-opus/issues/669
[#670]: https://github.com/mikepadiernos/plasma-opus/issues/670
[#671]: https://github.com/mikepadiernos/plasma-opus/issues/671
[#672]: https://github.com/mikepadiernos/plasma-opus/issues/672
[#673]: https://github.com/mikepadiernos/plasma-opus/issues/673
[#674]: https://github.com/mikepadiernos/plasma-opus/issues/674
[#675]: https://github.com/mikepadiernos/plasma-opus/issues/675
[#676]: https://github.com/mikepadiernos/plasma-opus/issues/676
[#677]: https://github.com/mikepadiernos/plasma-opus/issues/677
[#678]: https://github.com/mikepadiernos/plasma-opus/issues/678
[#679]: https://github.com/mikepadiernos/plasma-opus/issues/679
[#680]: https://github.com/mikepadiernos/plasma-opus/issues/680
[#687]: https://github.com/mikepadiernos/plasma-opus/issues/687
[#689]: https://github.com/mikepadiernos/plasma-opus/issues/689
[#700]: https://github.com/mikepadiernos/plasma-opus/issues/700
[#702]: https://github.com/mikepadiernos/plasma-opus/issues/702
[#703]: https://github.com/mikepadiernos/plasma-opus/issues/703
[#704]: https://github.com/mikepadiernos/plasma-opus/issues/704
[#705]: https://github.com/mikepadiernos/plasma-opus/issues/705
[#707]: https://github.com/mikepadiernos/plasma-opus/issues/707
[#708]: https://github.com/mikepadiernos/plasma-opus/issues/708
[#709]: https://github.com/mikepadiernos/plasma-opus/issues/709
[#710]: https://github.com/mikepadiernos/plasma-opus/issues/710
[#714]: https://github.com/mikepadiernos/plasma-opus/issues/714
[#716]: https://github.com/mikepadiernos/plasma-opus/issues/716
[#718]: https://github.com/mikepadiernos/plasma-opus/issues/718
[#719]: https://github.com/mikepadiernos/plasma-opus/issues/719
[#720]: https://github.com/mikepadiernos/plasma-opus/issues/720
[#721]: https://github.com/mikepadiernos/plasma-opus/issues/721
[#723]: https://github.com/mikepadiernos/plasma-opus/issues/723
[#725]: https://github.com/mikepadiernos/plasma-opus/issues/725
[#728]: https://github.com/mikepadiernos/plasma-opus/issues/728
[#729]: https://github.com/mikepadiernos/plasma-opus/issues/729
[#730]: https://github.com/mikepadiernos/plasma-opus/issues/730
[#733]: https://github.com/mikepadiernos/plasma-opus/issues/733
[#734]: https://github.com/mikepadiernos/plasma-opus/issues/734
[#738]: https://github.com/mikepadiernos/plasma-opus/issues/738
[#739]: https://github.com/mikepadiernos/plasma-opus/issues/739
[#740]: https://github.com/mikepadiernos/plasma-opus/issues/740
[#741]: https://github.com/mikepadiernos/plasma-opus/issues/741
[#742]: https://github.com/mikepadiernos/plasma-opus/issues/742
[#743]: https://github.com/mikepadiernos/plasma-opus/issues/743
[#744]: https://github.com/mikepadiernos/plasma-opus/issues/744
[#745]: https://github.com/mikepadiernos/plasma-opus/issues/745
[#746]: https://github.com/mikepadiernos/plasma-opus/issues/746
[#747]: https://github.com/mikepadiernos/plasma-opus/issues/747
[#748]: https://github.com/mikepadiernos/plasma-opus/issues/748
[#749]: https://github.com/mikepadiernos/plasma-opus/issues/749
[#750]: https://github.com/mikepadiernos/plasma-opus/issues/750
[#761]: https://github.com/mikepadiernos/plasma-opus/issues/761
[#762]: https://github.com/mikepadiernos/plasma-opus/issues/762
[#763]: https://github.com/mikepadiernos/plasma-opus/issues/763
[#764]: https://github.com/mikepadiernos/plasma-opus/issues/764
