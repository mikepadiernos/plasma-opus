# Plasma Opus

<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="src/app/branding/plasma-opus-logo-dark.svg">
    <img src="src/app/branding/plasma-opus-logo.svg" alt="Plasma Opus" width="480" height="100">
  </picture>
</p>

<p align="center">
  <strong>A dual-pane file manager for KDE Plasma</strong><br>
  Power-user workflows with native KDE file operations, Places and settings.
</p>

<p align="center">
  <a href="https://github.com/mikepadiernos/plasma-opus/releases/latest"><img alt="Latest release" src="https://img.shields.io/github/v/release/mikepadiernos/plasma-opus?style=flat-square&color=3daee9"></a>
  <img alt="C++20" src="https://img.shields.io/badge/C%2B%2B-20-00599C?style=flat-square&logo=cplusplus&logoColor=white">
  <img alt="Qt 6.8+" src="https://img.shields.io/badge/Qt-6.8%2B-41CD52?style=flat-square&logo=qt&logoColor=white">
  <img alt="KDE Frameworks 6.20+" src="https://img.shields.io/badge/KDE%20Frameworks-6.20%2B-1D99F3?style=flat-square&logo=kde&logoColor=white">
  <img alt="Linux and KDE Plasma" src="https://img.shields.io/badge/platform-Linux%20%C2%B7%20KDE%20Plasma-FCC624?style=flat-square&logo=linux&logoColor=black">
  <a href="https://github.com/users/mikepadiernos/projects/3"><img alt="Project board" src="https://img.shields.io/badge/project-board-8957e5?style=flat-square&logo=github"></a>
</p>

<p align="center">
  <a href="#build-and-run"><img alt="Get started" src="https://img.shields.io/badge/Get%20started-2ea44f?style=flat-square"></a>
  <a href="#features"><img alt="Features" src="https://img.shields.io/badge/Features-3daee9?style=flat-square"></a>
  <a href="#screenshots"><img alt="Screenshots" src="https://img.shields.io/badge/Screenshots-3daee9?style=flat-square"></a>
  <a href="#customize-the-interface"><img alt="Customize" src="https://img.shields.io/badge/Customize-3daee9?style=flat-square"></a>
  <a href="#safety-and-limitations"><img alt="Safety" src="https://img.shields.io/badge/Safety-bf8700?style=flat-square"></a>
  <a href="docs/ROADMAP.md"><img alt="Roadmap" src="https://img.shields.io/badge/Roadmap-8957e5?style=flat-square"></a>
  <a href="docs/README.md"><img alt="Docs" src="https://img.shields.io/badge/Docs-3daee9?style=flat-square"></a>
  <a href="docs/development.md"><img alt="Develop" src="https://img.shields.io/badge/Develop-2ea44f?style=flat-square"></a>
</p>

![Plasma Opus with stacked panes and an image preview](preview.png)

> [!NOTE]
> The latest published MVP is
> [v0.2.0](https://github.com/mikepadiernos/plasma-opus/releases/tag/v0.2.0),
> released after M14. Since then, `main` has gained M15–M31: workspaces
> (M15–M16), themes (M17–M18), branding (M19–M20), advanced search (M21–M22),
> native tagging (M23–M24), Yazi mode (M25–M26), desktop integrations
> (M27–M28) and Nextcloud integration (M29–M30), each followed by a settings
> milestone, and keyboard shortcut settings (M31). Features marked with a
> milestone badge below are on `main` but not in v0.2.0. See the
> [roadmap](docs/ROADMAP.md#at-a-glance) for status and the planned M32.

## Features

New windows start with stacked panes, visible tab bars and a preview on the
right. Pane navigation starts linked, and activating most files opens a
preview. Both behaviours can be changed in Preferences.

### Browse and preview

- **Panes and tabs:** side-by-side or stacked panes with tabs, history and
  session restoration.
- **Views:** Details, Icons, Compact, Card or flat views, with filtering and
  per-folder formats.
- **Preview and edit:** preview documents and media, use the image gallery,
  edit text with drafts and edit supported media metadata. Preview, text and
  metadata panels can be placed beside or below the panes.
- **Yazi mode**
  [![M25–M26](https://img.shields.io/badge/M25%E2%80%93M26-8957e5?style=flat-square)](docs/ROADMAP.md#m25-yazi-mode):
  an optional, per-pane [column layout](docs/yazi-mode.md) inspired by the Yazi
  terminal file manager, with parent, current and preview columns and
  keyboard-first navigation, including optional Vim-style keys. Toggle it with
  **View › Yazi Mode** and configure it on the **Yazi Mode** page in
  Preferences.

### Manage files

- **File operations:** KDE file actions, queued transfers, undo, batch rename,
  duplicate finding and listing export.
- **Archives and file tools:** browse and manage archives, including
  AES-encrypted ZIP creation and protected extraction. Create and verify
  SHA-256 checksum files, and split and combine files.
- **Compare and sync:** compare files or folders and preview per-item actions
  before synchronizing local folders.
- **Automate:** save scripts and commands, and preview them before running
  them.
- **Terminal:** embed Konsole below or beside the panes, with optional
  active-folder following. Hiding the panel retains its sessions.

### Find and organize

- **Search**
  [![M21–M22](https://img.shields.io/badge/M21%E2%80%93M22-8957e5?style=flat-square)](docs/ROADMAP.md#m21-advanced-search):
  search local folders or a Baloo index from the toolbar or an embedded search
  panel, combine name, path, type, size, date and content criteria with AND, OR
  and NOT, and save searches and collections.
- **Tagging**
  [![M23–M24](https://img.shields.io/badge/M23%E2%80%93M24-8957e5?style=flat-square)](docs/ROADMAP.md#m23-native-tagging):
  opt-in [tag libraries](docs/tagging.md) in existing folders, with
  hierarchical tags, groups, text and date fields, a tag panel, tag and
  embedded-metadata search criteria, and metadata import, export and restore.
  Use the **Tags** menu and the **Tagging** page in Preferences. Files are
  never moved or renamed.
- **Workspaces**
  [![M15–M16](https://img.shields.io/badge/M15%E2%80%93M16-8957e5?style=flat-square)](docs/ROADMAP.md#m15-workspaces):
  named workspaces with assigned locations, saved layouts, explicit pane and
  tab snapshots, workspace-scoped search, a startup choice, recovery options
  and profile import or export.

### Integrate with Plasma

- **Desktop services**
  [![M27–M28](https://img.shields.io/badge/M27%E2%80%93M28-8957e5?style=flat-square)](docs/integrations/kde-plasma.md):
  register as the default file manager, report progress through Plasma
  notifications, and offer a system tray icon, a global shortcut, a drop-down
  window, KRunner, KDE Connect, Activities and an optional system file chooser.
  Configure each integration on the **KDE Plasma**, **Applications**,
  **Remote and Cloud** and **Security** pages of Preferences.
- **Files and applications:** Ark archives, disk images, AppImage, Flatpak and
  Snap packages, Baloo search, office and ebook previews, editing in suitable
  applications, Markdown vaults with wiki links and tags, and optional embedded
  mpv playback.
- **Nextcloud**
  [![M29–M30](https://img.shields.io/badge/M29%E2%80%93M30-8957e5?style=flat-square)](docs/integrations/nextcloud.md):
  sign in to Nextcloud accounts with Login Flow, browse with the server's
  permissions, see server previews and metadata, search, favourite and tag on
  the server, open Photos albums and places sorted by capture date, manage
  links and shares with confirmation, and find the desktop client's folders
  with their sync status under **Go › Nextcloud**, configured under
  **Preferences › Remote and Cloud › Nextcloud**.
- **Remote and secure access:** named SSH, SFTP, WebDAV, SMB and FTP
  connections, rclone and KDE Online Accounts, temporary LAN sharing,
  administrator mode, Plasma Vault, file encryption and password-gated
  folders.

### Personalize

- **Themes**
  [![M17–M18](https://img.shields.io/badge/M17%E2%80%93M18-8957e5?style=flat-square)](docs/ROADMAP.md#m17-themes):
  app-local colour schemes, accents and pane tints, blur, opacity and corner
  radius, dividers and coloured tree lines, folder colour rules, fonts, spacing
  and motion. Save them as theme profiles with an optional stylesheet, or let
  each workspace apply its own appearance. Plasma's appearance remains the
  default.
- **Branding**
  [![M19–M20](https://img.shields.io/badge/M19%E2%80%93M20-8957e5?style=flat-square)](docs/ROADMAP.md#m19-branding):
  an original [name, logo and application icon](docs/BRANDING.md), and an
  optional startup presentation that never blocks use.
- **Languages:** translation catalogues, right-to-left layouts, Jalali dates
  and regional format overrides; see the [translation guide](po/README.md).
- **Keyboard**
  [![M31](https://img.shields.io/badge/M31-8957e5?style=flat-square)](docs/ROADMAP.md#m31-keyboard-shortcut-settings):
  change any action's [shortcuts](docs/keyboard.md), including key sequences,
  in KDE's shortcut editor. **Preferences › Keyboard** lists the changed and
  conflicting shortcuts, resets them to the defaults and shows the search,
  Yazi mode and global keys. Text fields and lists keep their typing and
  navigation keys. Shortcuts are stored in the configuration file and shared
  by every window, and the global shortcuts' status says when KDE refused a
  key or the session can't pass keys on. Saved commands can have shortcuts,
  and an action can have more than one alternate. **Help › Keyboard Map…**
  finds any action by name or by pressing its keys and exports a printable
  reference. Actions move the focus between panes, tabs and panels and change
  the selection or filter, with optional extra keys that never take a key
  already in use. Shortcuts export to a keymap file, and importing one shows
  the changes and conflicts before applying them. **Help › Command Palette…**
  finds menu, toolbar and saved command actions by name and runs them in the
  active pane, with recently used actions first. Optional Vim-style keys, such
  as `j`, `k`, `gg` and counts, move through the file views outside Yazi mode
  without taking keys from typing. Each pane then shows its mode, NORMAL,
  FILTER or INSERT; `/` or `i` filters, and Escape returns to the items.
  While a key sequence waits for its next key, a which-key style popup lists
  what can follow it, grouped with what each key does, without taking the
  focus. The **Hints** tab in **Preferences › Keyboard** turns the popup on or
  off and sets its delay and how long an unfinished sequence waits, and
  **Help › Shortcut Hints** shows the keys for the focused part of the window
  on demand. **Go › Keyboard Hints** labels the visible files, location
  segments, tabs and places with short home-row keys, and typing a label
  selects, opens or switches to it.

## Screenshots

Captured headless in the dev container with synthetic demo files, so KWin
decorations, blur and the Plasma theme are absent. See the
[screenshot notes](docs/screenshots/README.md).

| Dual panes                     | Card view                        |
| ------------------------------ | -------------------------------- |
| [![Dual panes][s1]][s1]        | [![Card view][s9]][s9]           |
| **Image preview**              | **Code preview**                 |
| [![Image preview][s2]][s2]     | [![Code preview][s3]][s3]        |
| **Embedded terminal**          | **Image gallery**                |
| [![Embedded terminal][s4]][s4] | [![Image gallery][s5]][s5]       |
| **Batch rename**               | **Content chart**                |
| [![Batch rename][s6]][s6]      | [![Content chart][s7]][s7]       |
| **Theme settings**             | **Branding settings**            |
| [![Theme settings][s8]][s8]    | [![Branding settings][s10]][s10] |

[s1]: docs/screenshots/dual-pane.png
[s2]: docs/screenshots/image-preview.png
[s3]: docs/screenshots/code-preview.png
[s4]: docs/screenshots/embedded-terminal.png
[s5]: docs/screenshots/image-gallery.png
[s6]: docs/screenshots/batch-rename.png
[s7]: docs/screenshots/content-chart.png
[s8]: docs/screenshots/settings-themes.png
[s9]: docs/screenshots/card-view.png
[s10]: docs/screenshots/settings-branding.png

## Build and run

### Requirements

Plasma Opus runs on Linux with KDE Plasma. Building it needs a C++20 compiler,
CMake 3.25+, Ninja, Qt 6.8+ and KDE Frameworks 6.20+. Run `./install.sh --deps`
to print the build and optional feature packages; it does not install them.

> [!TIP]
> To build for development, run the tests, use the headless dev container on
> other hosts such as macOS, run the optional mise tasks or contribute, see
> the [development guide](docs/development.md). It also describes the project
> workflow from issue to merge.

### Arguments

Pass folders or URLs as arguments: the first opens in the active pane, the
second in the other pane, and additional arguments in tabs in the active pane.
Without arguments, the configured startup or restored session is used. When
Plasma Opus is already running, the arguments open in the existing window.

### Install

Run `./install.sh` to build a release and install it for the current user
under `~/.local`; use `./install.sh --help` for other options, such as
`--system` and `--prefix`. `./install.sh --default` also makes Plasma Opus the
default folder handler and lets it answer **Show in folder** requests from other
applications through `org.freedesktop.FileManager1`. Installing also adds the
Plasma Opus KRunner plugin and its `opus-file-manager-krunner` helper, which
offer saved workspaces and Favorites in KRunner and Plasma Search, and the
`opus-file-manager-mcp` server, which lets MCP clients such as AI assistants
read files in your home folder by default; see
[KDE Plasma integration](docs/integrations/kde-plasma.md).
`./install.sh --file-chooser` also lets Plasma Opus answer the Open and Save
dialogs that Flatpak, Snap and other portal-enabled applications request
through the XDG file chooser portal, keeping KDE's chooser as the fallback;
run `opus-file-manager --install-portal` or `--uninstall-portal` to switch it
on or off for an existing install.

The installer uses `build-plasma` with tests disabled; set `OPUSFM_BUILD_DIR`
to choose another build tree. `./install.sh --uninstall` restores the previous
file chooser, then removes the files
installed from that build tree, so uninstall using the same build tree. If
Plasma Opus is the folder handler, it hands folders back to Dolphin when Dolphin
is installed.

The app icon, desktop entry and AppStream metadata are installed with the
application; file and action icons follow the active desktop theme. See
[branding](docs/BRANDING.md) for the logo and icon assets.

### Optional integrations

Baloo provides indexed search, Konsole provides the embedded terminal,
ExifTool supports media metadata editing, and libmpv offers embedded mpv
playback in previews and the gallery. See the [caveats](docs/CAVEATS.md) for
requirements and restrictions and the [translation guide](po/README.md) for
language setup.

## Customize the interface

- **Panes and panels:** swap current tabs or whole pane sides, dim the
  inactive pane, and save or reset splitter sizes by orientation. Change
  combined panel section order, visibility and titles; embedded panel titles
  can be hidden separately from navigation-panel titles.
- **Toolbars:** configure the main and pane toolbars and hamburger menu, choose
  button appearance and lock editing. Toolbar context menus provide editing and
  settings access; the main toolbar also retains KDE's toolbar options. Saved
  scripts and commands can become main-toolbar buttons; running them still
  requires preview and confirmation.
- **Tabs and status:** position tab bars per pane and layout, or keep them
  together between stacked panes. Show or hide close buttons and configure
  status-bar indicators and their order.
- **Views:** enable the
  [file-list minimap](docs/CAVEATS.md#file-list-minimap) and set its width.
  Choose Information panel fields and their order, with optional hover
  information. Set a theme, light, dark or checkerboard background for
  transparent images without changing the image.
- **Appearance:** the **Themes** page sets app-local colours, effects, lines,
  views, folder colours, text and motion, and manages theme profiles
  ([M17](docs/ROADMAP.md#m17-themes),
  [M18](docs/ROADMAP.md#m18-theme-settings)). The **Branding** page turns the
  optional startup presentation on and chooses its logo
  ([M20](docs/ROADMAP.md#m20-branding-settings)). Edit a workspace's theme
  profile and appearance overrides in **View › Workspaces…**.
- **Backup:** back up settings and window and panel layouts, or preview a
  restore that applies on the next launch. Review the
  [backup limitations](docs/CAVEATS.md#settings-backups) before sharing or
  relying on an archive.

Preferences also include
[tool defaults](docs/ROADMAP.md#m10-tools-settings),
[language and regional formats](docs/ROADMAP.md#m14-translation-settings),
[workspace settings](docs/ROADMAP.md#m16-workspace-settings),
[advanced search settings](docs/ROADMAP.md#m22-advanced-search-settings),
[tagging settings](docs/tagging.md#settings),
[Yazi mode settings](docs/yazi-mode.md#settings) and
[integration settings](docs/integrations/kde-plasma.md) on the **KDE Plasma**,
**Applications**, **Remote and Cloud** and **Security** pages
([M28](docs/ROADMAP.md#m28-integration-settings)).

> [!IMPORTANT]
> Path-filter, saved-search, collection, rename-preset, automation, remote
> connection, cloud sync pair and password-protected folder management changes
> take effect immediately, even if Preferences is cancelled. KDE File
> Associations changes shared desktop settings.

## Safety and limitations

> [!WARNING]
> Synchronization is not undoable and can leave partial results; automatic
> transfer overwrite can replace files without asking.

See [caveats and limitations](docs/CAVEATS.md) for restrictions and workarounds,
[reported bugs](docs/BUGS.md) for tracked issues, and [edge cases](docs/EDGE.md)
for possible scenarios that are not confirmed bugs.

## Documentation

| Area     | Guide                                                              | What it covers                                             |
| -------- | ------------------------------------------------------------------ | ---------------------------------------------------------- |
| Index    | [Documentation index](docs/README.md)                              | All guides, plans, integrations and upstream references    |
| Features | [Native tagging](docs/tagging.md)                                  | Library format, tags, fields, search, backups and settings |
| Features | [Yazi mode](docs/yazi-mode.md)                                     | Per-pane column browsing, keys, persistence and settings   |
| Features | [Translations](po/README.md)                                       | Catalogues, languages and regional formats                 |
| Features | [Branding](docs/BRANDING.md)                                       | Name, logo, icons and startup presentation                 |
| Features | [Screenshots](docs/screenshots/README.md)                          | Captured views and how they were made                      |
| Planning | [Roadmap](docs/ROADMAP.md)                                         | Architecture, milestone status and planned features        |
| Planning | [Project board](https://github.com/users/mikepadiernos/projects/3) | Issue status from Backlog to Merged                        |
| Planning | [Pending tasks](docs/TODO.md)                                      | Blocked work and handoffs                                  |
| Tracking | [Caveats](docs/CAVEATS.md)                                         | Limitations, dependencies and workarounds                  |
| Tracking | [Reported bugs](docs/BUGS.md)                                      | Bug reports and their status                               |
| Tracking | [Edge cases](docs/EDGE.md)                                         | Possible scenarios worth testing                           |
| Develop  | [Development guide](docs/development.md)                           | Building, testing, dev container and project workflow      |
