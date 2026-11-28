# Reported bugs

[![Docs index](https://img.shields.io/badge/docs-index-3daee9?style=flat-square)](README.md)
[![Project board](https://img.shields.io/badge/project-board-6e7781?style=flat-square&logo=github)](https://github.com/users/mikepadiernos/projects/3)
[![Edge cases](https://img.shields.io/badge/edge-cases-6e7781?style=flat-square)](EDGE.md)
[![Caveats](https://img.shields.io/badge/caveats-limitations-bf8700?style=flat-square)](CAVEATS.md)

This log records user-visible bugs and reliability issues with their project
status. Consult the [issue workflow](../CLAUDE.md#issues-and-caveats) for
tracking rules.

> [!IMPORTANT]
> A bug is complete only when its project status is **Completed** or
> **Merged**. Completed does not necessarily mean merged into `main`.
> [Edge cases](EDGE.md) are possible scenarios, not confirmed bugs.

> [!NOTE]
> 16 of 20 tracked bugs are merged into `main`. Four reports are open in
> Backlog: a failing `install.sh` build ([#785][issue-785]), abbreviated
> breadcrumbs spreading out their segments ([#756][issue-756]), slow opening
> of the app and Preferences ([#752][issue-752]) and a test reliability
> report ([#696][issue-696]). Issue states and project statuses were last
> checked against GitHub on 2026-11-28.

## Contents

- [Current tracked issues](#current-tracked-issues)
- [Status legend](#status-legend)
- [Open reports](#open-reports)
- [Completed reports](#completed-reports)

## Current tracked issues

Newest first. Each bug links to its report below.

| Issue             | Bug                                                                                                                    | Complete?   | Project status      |
| ----------------- | ---------------------------------------------------------------------------------------------------------------------- | ----------- | ------------------- |
| [#785][issue-785] | [`install.sh` build stops with "subcommand failed"](#install-build-failure)                                            | ![No][no]   | ![Backlog][backlog] |
| [#756][issue-756] | [Abbreviated breadcrumbs spread their segments across the location bar](#abbreviated-breadcrumb-spacing)               | ![No][no]   | ![Backlog][backlog] |
| [#752][issue-752] | [Opening the app and Preferences takes a long time](#slow-startup-and-preferences)                                     | ![No][no]   | ![Backlog][backlog] |
| [#696][issue-696] | [Checksum test times out waiting for the folder listing under load](#checksum-test-listing-timeout)                    | ![No][no]   | ![Backlog][backlog] |
| [#694][issue-694] | [`mainwindowtest` intermittently crashes after restoring a pane filter](#intermittent-mainwindowtest-crash)            | ![Yes][yes] | ![Merged][merged]   |
| [#682][issue-682] | [Icons view shows image thumbnails at Details size](#icons-view-thumbnail-size)                                        | ![Yes][yes] | ![Merged][merged]   |
| [#606][issue-606] | [Visual Markdown editing loses formatting with a fixed-pitch font](#visual-markdown-formatting-with-fixed-pitch-fonts) | ![Yes][yes] | ![Merged][merged]   |
| [#605][issue-605] | [Panes without a saved view mode open in Details](#default-view-for-restored-panes)                                    | ![Yes][yes] | ![Merged][merged]   |
| [#558][issue-558] | [Headers, titles and status bar need horizontal padding](#header-title-and-status-bar-padding)                         | ![Yes][yes] | ![Merged][merged]   |
| [#555][issue-555] | [Application icon accent colour and size](#application-icon-accent-color-and-size)                                     | ![Yes][yes] | ![Merged][merged]   |
| [#549][issue-549] | [Combined panel section spacing and title accents](#combined-panel-section-spacing)                                    | ![Yes][yes] | ![Merged][merged]   |
| [#533][issue-533] | [Embedded panel titles cannot be hidden like Explorer](#hide-embedded-panel-titles)                                    | ![Yes][yes] | ![Merged][merged]   |
| [#532][issue-532] | [Consistent panel placement beside or below the panes](#consistent-panel-placement)                                    | ![Yes][yes] | ![Merged][merged]   |
| [#476][issue-476] | [Toolbar context menus and symbolic icons](#toolbar-context-menus-and-symbolic-icons)                                  | ![Yes][yes] | ![Merged][merged]   |
| [#457][issue-457] | [Embedded terminal panel controls and background blur](#embedded-terminal-panel-controls-and-background-blur)          | ![Yes][yes] | ![Merged][merged]   |
| [#446][issue-446] | [Metadata editing fails with an unusable ExifTool installation](#metadata-editing-with-unusable-exiftool)              | ![Yes][yes] | ![Merged][merged]   |
| [#444][issue-444] | [KDE toolbar editor separators and spacers do not match the toolbar](#kde-toolbar-editor-separators-and-spacers)       | ![Yes][yes] | ![Merged][merged]   |
| [#383][issue-383] | [Editor settings should use the notepad symbolic icon](#editor-settings-icon)                                          | ![Yes][yes] | ![Merged][merged]   |
| [#378][issue-378] | [Embedded Konsole terminal does not honour profile background blur](#embedded-terminal-profile-blur)                   | ![Yes][yes] | ![Merged][merged]   |
| [#37][issue-37]   | [Combined panel header and accordion titles are misaligned](#combined-panel-header-and-accordion-alignment)            | ![Yes][yes] | ![Merged][merged]   |

## Status legend

Statuses use the project board's names, in workflow order.

| Status                      | Meaning                                                  |
| --------------------------- | -------------------------------------------------------- |
| ![Merged][merged]           | Merged into `main`                                       |
| ![Completed][completed]     | Closed as completed, but not necessarily merged          |
| ![Review][review]           | Implementation is ready for review                       |
| ![Development][development] | Work is in progress                                      |
| ![Backlog][backlog]         | Recorded on the project board; work has not started      |
| ![Untriaged][untriaged]     | Logged here but not yet created or assigned on the board |

## Open reports

Every report uses the same fields in the same order. "Not recorded" means the
field is not captured in this log; see the linked issue for any further
detail.

### Install build failure

- **Issue:** [#785][issue-785] ![Backlog][backlog]
- **Severity:** High (installation)
- **Summary:** Installing the latest `main` with `./install.sh` stops during
  the build with `ninja: build stopped: subcommand failed.`, so the installed
  version is not updated.
- **Evidence:** Not recorded. Ninja prints this line after a build command
  fails; the failing command and its error, after a `FAILED:` line, have not
  been captured yet. The reporter's commit, distribution and compiler are not
  yet known.
- **Steps to reproduce:** Update the checkout to the latest `main` and run
  `./install.sh`.
- **Expected behaviour:** The build completes and Plasma Opus is installed.
- **Actual behaviour:** The build stops before installing. It does not
  reproduce in the dev container (Fedora 44, GCC 16.2.1, Qt 6.11.2, KDE
  Frameworks 6.32): `install.sh` succeeds at `504456f` and `7d5aa14` from a
  clean tree, when updating an existing `build-plasma` tree from `e803e63`,
  and with every optional dependency disabled. The cause is therefore
  host-specific; running out of memory during the parallel Release build, or
  M28–M31 code using an API newer than the host's Qt or KDE Frameworks, are
  possible but unverified.
- **Resolution:** Not resolved. Next steps: capture the first error with
  `./install.sh 2>&1 | tee install.log` and `grep -n -A25 FAILED install.log`,
  and record the commit, distribution and `c++ --version`. Workarounds to
  try: `./install.sh --jobs 2`, or a fresh build tree with
  `OPUSFM_BUILD_DIR=build-plasma-new ./install.sh`.

### Abbreviated breadcrumb spacing

- **Issue:** [#756][issue-756] ![Backlog][backlog]
- **Severity:** Low (layout)
- **Summary:** With **Abbreviated Breadcrumbs** selected, the location bar
  spreads its path segments and separators evenly across its full width.
- **Evidence:** Not recorded.
- **Steps to reproduce:** Choose **Abbreviated Breadcrumbs** from a pane's
  location mode menu, then open a folder a few levels deep.
- **Expected behaviour:** The segments and separators stay together at the
  start of the bar, like full breadcrumbs, with the spare space at the end.
- **Actual behaviour:** Each segment and separator is spaced out evenly
  across the bar. Reading the code suggests one cause, not yet verified:
  `Pane::updateBreadcrumbs()` in `src/app/pane.cpp` adds the segments to a
  horizontal layout without a trailing stretch, so the layout shares the
  spare width among them.
- **Resolution:** Not resolved. Next steps: add a stretch after the last
  segment or left-align the items, and cover the layout with a test.

### Slow startup and Preferences

- **Issue:** [#752][issue-752] ![Backlog][backlog]
- **Severity:** Moderate (performance)
- **Summary:** Plasma Opus takes a long time to open, and so does its
  Preferences dialog.
- **Evidence:** Not recorded. Timings, hardware, session type, build type and
  the restored session or workspace are not yet known.
- **Steps to reproduce:** Launch Plasma Opus, then open **Settings › Configure
  Plasma Opus…** once the window is shown.
- **Expected behaviour:** The main window becomes usable quickly, and
  Preferences opens without a noticeable wait.
- **Actual behaviour:** Both take a long time to appear. Reading the code
  suggests one cause, not yet profiled: the Preferences dialog builds all of
  its pages, about 80 tabs, every time it opens, because it is deleted when
  closed. Startup costs have not been measured.
- **Resolution:** Not resolved. Next steps: measure cold and warm startup and
  the time to open Preferences in a Release build on Plasma, build settings
  pages when first shown and defer startup work the first window does not
  need. The planned M32.05 items [#477][issue-477], [#478][issue-478] and
  [#479][issue-479] cover this work.

### Checksum test listing timeout

- **Issue:** [#696][issue-696] ![Backlog][backlog]
- **Severity:** Low (test reliability)
- **Summary:** `createsAndVerifiesChecksumFiles()` in
  `tests/mainwindowtest.cpp` (line 3745 when reported) timed out while waiting
  for the folder view to show the new checksum file.
- **Evidence:** Not recorded.
- **Steps to reproduce:** Run `mainwindowtest` repeatedly in the dev container
  while other processes load the CPU. It failed in 1 of 8 loaded runs and in
  none of 18 runs without load.
- **Expected behaviour:** The view lists `source.txt` and the new manifest,
  and the test verifies the manifest.
- **Actual behaviour:** The view held 0 items, not 1, when the 5 s wait
  expired. This suggests the folder was still being relisted after the change
  rather than missing the new file; whether users can briefly see an empty
  folder is not yet known.
- **Resolution:** Not resolved. Next steps: confirm whether the change
  triggers a full relisting, wait for the listing to complete in the test, and
  consider keeping existing items visible while a folder refreshes.

## Completed reports

Newest first, using the same fields as open reports.

### Intermittent mainwindowtest crash

- **Issue:** [#694][issue-694] ![Merged][merged]
- **Severity:** Moderate (test reliability)
- **Summary:** `mainwindowtest` intermittently crashed with SIGSEGV when a main
  window was destroyed, first seen in `filterPersistsWithThePaneSession()`.
- **Evidence:** A `gdb` backtrace showed that destroying the places model
  stopped its shared directory lister job, which made a pane's lister report
  items. The pane's item-count signal then updated the status bar through a
  window member that had already been destroyed, because Qt destroys child
  widgets after the window's own members.
- **Steps to reproduce:** Run `mainwindowtest` repeatedly in the dev
  container. It crashed in about 3 of 8 runs, on feature branches and on
  unchanged `main`.
- **Expected behaviour:** Closing a window never crashes, and the test passes.
- **Actual behaviour:** The process crashed during window teardown.
- **Resolution:** The main window disconnects its panes and other children
  from itself before its members are destroyed. A regression test emits pane
  signals during child teardown.

### Icons view thumbnail size

- **Issue:** [#682][issue-682] ![Merged][merged]
- **Severity:** Moderate
- **Summary:** With previews enabled, image thumbnails in Icons view render at
  Details-view size, about 16 px, inside full-size icon cells.
- **Evidence:** Not recorded.
- **Steps to reproduce:** Enable **View › Previews**, switch a pane to Icons
  view and open a folder that contains images.
- **Expected behaviour:** Thumbnails fill the icon cell at the Icons view's
  size.
- **Actual behaviour:** Thumbnails appear at Details-view size. The views
  share one model, and every view's preview generator was enabled, so the
  smaller Details and Compact previews overwrote the Icons previews.
- **Resolution:** Only the current view's preview generator is enabled, and
  switching views refreshes the previews at the new view's size.

### Visual Markdown formatting with fixed-pitch fonts

- **Issue:** [#606][issue-606] ![Merged][merged]
- **Severity:** High (data loss)
- **Summary:** Saving a document edited in the visual Markdown editor writes
  all text as inline code and drops its formatting when the editor font is
  fixed-pitch, which is the default.
- **Evidence:** Not recorded.
- **Steps to reproduce:** With a fixed-pitch editor font, open a Markdown
  file, switch to visual editing, make a change and save.
- **Expected behaviour:** Headings, emphasis, lists and other formatting are
  preserved.
- **Actual behaviour:** With Qt 6.11, Markdown export treats fixed-pitch text
  as code.
- **Resolution:** The visual editor uses the system general font at the
  configured editor font size; the source editor keeps the fixed-pitch font.

### Default view for restored panes

- **Issue:** [#605][issue-605] ![Merged][merged]
- **Severity:** Moderate
- **Summary:** Restored session panes without a saved view mode open in
  Details instead of the configured default view mode.
- **Evidence:** Not recorded.
- **Steps to reproduce:** Set a default view mode other than Details, then
  restore a session whose panes have no `ViewMode` entry.
- **Expected behaviour:** Such panes use the configured default view mode.
- **Actual behaviour:** They open in Details because session loading fell
  back to view mode 0. This regressed when M15 made the default view
  configurable.
- **Resolution:** Session loading accepts the configured default view mode,
  which the main window passes when restoring sessions and window properties.

### Header, title and status-bar padding

- **Issue:** [#558][issue-558] ![Merged][merged]
- **Severity:** Not assessed
- **Summary:** Headers, titles and the status bar need more horizontal
  padding.
- **Evidence:** Not recorded.
- **Steps to reproduce:** Not recorded.
- **Expected behaviour:** Not recorded.
- **Actual behaviour:** Not recorded.
- **Resolution:** Application-owned headers, titles and status-bar content
  have at least eight logical pixels of horizontal padding. Native dock titles
  and column headers retain their active style; see the
  [padding caveat](CAVEATS.md#native-title-and-header-padding)
  ([#560][issue-560]).

### Application icon accent color and size

- **Issue:** [#555][issue-555] ![Merged][merged]
- **Severity:** Not assessed
- **Summary:** The application icon does not follow the Plasma accent colour
  and appears smaller than Dolphin's icon.
- **Evidence:** Not recorded.
- **Steps to reproduce:** Not recorded.
- **Expected behaviour:** The folder follows the active accent colour, has a
  comparable visual footprint, and displays a slightly smaller white
  golden-ratio motif.
- **Actual behaviour:** Not recorded.
- **Resolution:** The folder uses KDE's accent colour scheme, the icon's
  visual footprint is enlarged and the white motif is reduced. Icon renderers
  that do not apply colour schemes use the SVG's default blue.

### Combined panel section spacing

- **Issue:** [#549][issue-549] ![Merged][merged]
- **Severity:** Not assessed
- **Summary:** Large vertical gaps appear between combined-panel sections when
  some are expanded and others are collapsed. Section titles also display
  ampersands before letters.
- **Evidence:** The reporter's screenshot shows expanded Favorites and Remote
  sections with substantial empty space between neighbouring section headers.
- **Steps to reproduce:** Open the combined Explorer panel, expand some
  sections and collapse others, then observe the spacing between sections.
- **Expected behaviour:** Collapsed sections occupy only their header height;
  section headers follow the preceding content without excessive gaps.
  Expanded content remains usable, with appropriate scrolling and resizing,
  while section order, visibility, expanded state and navigation are
  preserved. In section titles, hide the ampersand before a letter and
  underline that letter in the active accent colour while retaining the normal
  text colour; preserve accessible titles and keyboard behaviour.
- **Actual behaviour:** Not recorded.
- **Resolution:** Section containers size to their content rather than
  absorbing spare vertical space. Mnemonic markers are hidden and their
  letters are underlined in the active accent colour, while accessible names
  and keyboard shortcuts are preserved.

### Hide embedded panel titles

- **Issue:** [#533][issue-533] ![Merged][merged]
- **Severity:** Not assessed
- **Summary:** Embedded panel titles should be hideable without losing
  controls, document status, metadata changes or terminal sessions.
- **Evidence:** Not recorded.
- **Steps to reproduce:** Not recorded.
- **Expected behaviour:** Not recorded.
- **Actual behaviour:** Not recorded.
- **Resolution:** **Preferences › Interface › Panel Appearance** offers a
  separate, persistent embedded-title preference for the terminal, preview,
  text editor, metadata editor and information panel. Controls remain
  available, document identity and unsaved status move out of the title area,
  and the information panel supplies a placement and hide menu when its dock
  title is hidden. Existing navigation title preferences and separate-window
  decorations remain unchanged.

### Consistent panel placement

- **Issue:** [#532][issue-532] ![Merged][merged]
- **Severity:** Not assessed
- **Summary:** Preview, text editor and metadata editor need placement below
  or right of the panes without losing their existing modes or content.
- **Evidence:** Not recorded.
- **Steps to reproduce:** Not recorded.
- **Expected behaviour:** Not recorded.
- **Actual behaviour:** Not recorded.
- **Resolution:** Preview and metadata editing offer below/right placement
  alongside their existing pane and separate-window modes. Editor placement
  and dock sizes are remembered. Moving metadata editing retains pending
  fields and preview/apply safeguards; active operations prevent closing.
  Existing navigation and information docks already allow right-side
  placement.

### Toolbar context menus and symbolic icons

- **Issue:** [#476][issue-476] ![Merged][merged]
- **Severity:** Not assessed
- **Summary:** The view mode button should use a symbolic icon consistent with
  other small toolbar controls. The reporter also observes a full-colour Open
  Editor icon and requests the symbolic icon used on the Editor settings page.
  The main and pane toolbars also need context menus with appropriate entries
  and access to toolbar editing settings.
- **Evidence:** Not recorded.
- **Steps to reproduce:** Not recorded.
- **Expected behaviour:** Prefer the active theme's symbolic icon at small
  sizes, with the existing regular-icon fallback for larger sizes or themes
  without a symbolic variant. Use the settings page's notepad icon for Open
  Editor and other applicable editor actions. Provide context menus for the
  main toolbar and each pane toolbar with relevant entries, including an
  option to open **Preferences › Interface › Toolbars**. Preserve existing
  action-specific context menus, view-mode, editor and toolbar customization
  behaviour.
- **Actual behaviour:** Investigation found that the customizer overwrote the
  pane view and sort buttons' symbolic icons from their empty widget-action
  icons. The separate `toggle_view_mode` action requested regular icons
  directly. `open_editor` requested `accessories-text-editor`, whereas the
  Editor settings page uses `view-pim-notes`. Other editor actions were
  reviewed for the same mismatch.
- **Resolution:** Preserve widget-action icons, use theme-aware view and
  editor icons, and provide toolbar-specific context menus with settings
  access and the existing KDE toolbar options.

### Embedded terminal panel controls and background blur

- **Issue:** [#457][issue-457] ![Merged][merged]
- **Severity:** Not assessed
- **Summary:** The embedded terminal lacks an accessible panel context menu
  and dismissal controls, while profile blur reportedly disappears after
  opening.
- **Evidence:** The reporter's screenshot shows the terminal and persistent
  "Terminal sessions remain open until their tabs are closed." message. The
  reporter observed the background initially blurred, then transparent without
  blur.
- **Steps to reproduce:** Not recorded.
- **Expected behaviour:** Provide panel controls independently of Konsole's
  terminal context menu, allow the informational message to be dismissed, and
  let users hide the panel without ending sessions. Honour the active
  profile's blur setting consistently when supported.
- **Actual behaviour:** Right-clicking offers the terminal's menu, the message
  cannot be dismissed, and the reporter could not find an in-panel way to
  close the panel without ending a session. Blur reportedly does not persist.
- **Resolution:** [#459][pr-459] adds panel controls to hide the panel without
  ending sessions and dismiss messages. When the Konsole profile enables blur,
  Plasma Opus reapplies a window-wide blur request during repaint so the
  active style cannot replace it with an incomplete region. The previous blur
  report was [#378][issue-378].

### Metadata editing with unusable ExifTool

- **Issue:** [#446][issue-446] ![Merged][merged]
- **Severity:** Not assessed
- **Summary:** Metadata editing failed when ExifTool could not load its Perl
  module.
- **Evidence:** The audit environment selected a system ExifTool with an
  incompatible Homebrew Perl. The reporter installed ExifTool 13.64 via mise;
  the shell resolves it successfully and the metadata editor and dialog suites
  pass in an isolated environment using that installation.
- **Steps to reproduce:** Not recorded.
- **Expected behaviour:** Not recorded.
- **Actual behaviour:** Not recorded.
- **Resolution:** [#451][pr-451] improved diagnostics to identify the
  executable and report silent failures. Desktop-launched Plasma Opus must
  also have the working executable on its `PATH`.

### KDE toolbar editor separators and spacers

- **Issue:** [#444][issue-444] ![Merged][merged]
- **Severity:** Moderate
- **Summary:** Separators and expanding spacers in KDE's toolbar editor did
  not match the live or restored toolbar.
- **Evidence:** The reporter's screenshots identify the KDE toolbar editor and
  the resulting toolbar. KDE replaced the toolbar on Apply, leaving Plasma Opus
  with the old toolbar and saved layout.
- **Steps to reproduce:** Not recorded.
- **Expected behaviour:** Not recorded.
- **Actual behaviour:** Not recorded.
- **Resolution:** [#454][pr-454] synchronizes the editor with the current
  layout and saves the rebuilt toolbar. [#452][pr-452] also preserves the
  default spacer when saving. Previously overridden layouts must be configured
  and applied again.

### Editor settings icon

- **Issue:** [#383][issue-383] ![Merged][merged]
- **Severity:** Not recorded
- **Summary:** The Editor settings page did not use the requested
  notepad-style symbolic icon.
- **Evidence:** The issue identifies the Editor page in the settings sidebar.
- **Steps to reproduce:** Not recorded.
- **Expected behaviour:** Use a notepad symbolic icon from the active icon
  theme.
- **Actual behaviour:** The page requested `accessories-text-editor` instead.
- **Resolution:** Marked Merged on the project board.

### Embedded terminal profile blur

- **Issue:** [#378][issue-378] ![Merged][merged]
- **Severity:** Not recorded
- **Summary:** The embedded terminal did not honour the active Konsole
  profile's background blur.
- **Evidence:** The issue describes comparing embedded and standalone Konsole
  sessions using a translucent profile with blur enabled.
- **Steps to reproduce:** Not recorded.
- **Expected behaviour:** Honour profile blur when the compositor supports it,
  without forcing blur when disabled.
- **Actual behaviour:** The embedded background was not blurred.
- **Resolution:** Marked Merged on the project board.

### Combined panel header and accordion alignment

- **Issue:** [#37][issue-37] ![Merged][merged]
- **Severity:** Not recorded
- **Summary:** The combined panel used the wrong header and centred accordion
  titles, with the caret on the left.
- **Evidence:** The issue includes the original screenshot.
- **Steps to reproduce:** Recorded in the issue.
- **Expected behaviour:** Show the explorer panel header, left-align section
  titles, place carets on the right, and make the relevant sections
  expandable.
- **Actual behaviour:** The header said Places, titles were centred and
  section expansion did not match the requested layout.
- **Resolution:** Marked Merged on the project board.

[merged]: https://img.shields.io/badge/Merged-8957e5?style=flat-square
[completed]: https://img.shields.io/badge/Completed-2ea44f?style=flat-square
[review]: https://img.shields.io/badge/Review-bf8700?style=flat-square
[development]: https://img.shields.io/badge/Development-1f6feb?style=flat-square
[backlog]: https://img.shields.io/badge/Backlog-6e7781?style=flat-square
[untriaged]: https://img.shields.io/badge/Untriaged-cf222e?style=flat-square
[yes]: https://img.shields.io/badge/Yes-2ea44f?style=flat-square
[no]: https://img.shields.io/badge/No-cf222e?style=flat-square
[issue-37]: https://github.com/mikepadiernos/plasma-opus/issues/37
[issue-378]: https://github.com/mikepadiernos/plasma-opus/issues/378
[issue-383]: https://github.com/mikepadiernos/plasma-opus/issues/383
[issue-444]: https://github.com/mikepadiernos/plasma-opus/issues/444
[issue-446]: https://github.com/mikepadiernos/plasma-opus/issues/446
[issue-457]: https://github.com/mikepadiernos/plasma-opus/issues/457
[issue-476]: https://github.com/mikepadiernos/plasma-opus/issues/476
[issue-477]: https://github.com/mikepadiernos/plasma-opus/issues/477
[issue-478]: https://github.com/mikepadiernos/plasma-opus/issues/478
[issue-479]: https://github.com/mikepadiernos/plasma-opus/issues/479
[issue-532]: https://github.com/mikepadiernos/plasma-opus/issues/532
[issue-533]: https://github.com/mikepadiernos/plasma-opus/issues/533
[issue-549]: https://github.com/mikepadiernos/plasma-opus/issues/549
[issue-555]: https://github.com/mikepadiernos/plasma-opus/issues/555
[issue-558]: https://github.com/mikepadiernos/plasma-opus/issues/558
[issue-560]: https://github.com/mikepadiernos/plasma-opus/issues/560
[issue-605]: https://github.com/mikepadiernos/plasma-opus/issues/605
[issue-606]: https://github.com/mikepadiernos/plasma-opus/issues/606
[issue-682]: https://github.com/mikepadiernos/plasma-opus/issues/682
[issue-694]: https://github.com/mikepadiernos/plasma-opus/issues/694
[issue-696]: https://github.com/mikepadiernos/plasma-opus/issues/696
[issue-752]: https://github.com/mikepadiernos/plasma-opus/issues/752
[issue-756]: https://github.com/mikepadiernos/plasma-opus/issues/756
[issue-785]: https://github.com/mikepadiernos/plasma-opus/issues/785
[pr-451]: https://github.com/mikepadiernos/plasma-opus/pull/451
[pr-452]: https://github.com/mikepadiernos/plasma-opus/pull/452
[pr-454]: https://github.com/mikepadiernos/plasma-opus/pull/454
[pr-459]: https://github.com/mikepadiernos/plasma-opus/pull/459
