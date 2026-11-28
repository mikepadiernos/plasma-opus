# ⚠️ Caveats and limitations

[![Docs index](https://img.shields.io/badge/docs-index-3daee9?style=flat-square)](README.md)
[![Roadmap](https://img.shields.io/badge/roadmap-M01%E2%80%93M32-8957e5?style=flat-square)](ROADMAP.md)
[![Reported bugs](https://img.shields.io/badge/reported-bugs-cf222e?style=flat-square)](BUGS.md)
[![Project board](https://img.shields.io/badge/project-board-6e7781?style=flat-square&logo=github)](https://github.com/users/mikepadiernos/projects/3)

Use this guide to find limitations, precautions and practical recommendations
for current features and, where marked, planned features. Tracked caveats link
to their issues and recommendation comments; alternatives supplement the primary
recommendation and external tools are separate applications. Confirmed defects
are in [reported bugs](BUGS.md), and untested scenarios in
[edge cases](EDGE.md).

> [!NOTE]
> Proposed improvements are not implemented features. The
> [roadmap](ROADMAP.md) defines scope and status.

> [!TIP]
> Each caveat opens with a short description and its tracking issue, lists
> the current limitations, then gives a **Recommendation** for what to do
> today with a link to the matching issue comment. Alternatives and external
> tools follow where useful; **Planned** and **Proposed improvements** are not
> yet implemented.

## Find a caveat

- **🗂️ Browsing and viewing:** [Archives](#archives),
  [folder metadata and listings](#folder-metadata-and-listings),
  [password-gated folders](#password-gated-folders),
  [previews and gallery](#previews-and-gallery), [editor](#editor),
  [file-list minimap](#file-list-minimap), [Yazi mode](#yazi-mode),
  [Yazi mode settings](#yazi-mode-settings)
- **🔎 Search and operations:** [Search](#search),
  [batch rename](#batch-rename), [metadata editing](#metadata-editing),
  [transfer queue](#transfer-queue), [custom commands](#custom-commands),
  [symbolic links](#symbolic-links), [listing export](#listing-export),
  [checksums](#checksum-files), [split and combine](#split-and-combine-files),
  [native tagging](#native-tagging), [tagging settings](#tagging-settings)
- **🔀 Comparison and analysis:**
  [Filtered folder comparison](#filtered-folder-comparison),
  [folder synchronization](#folder-synchronization),
  [file comparison](#file-comparison),
  [folder content chart](#folder-content-chart),
  [pane comparison](#pane-folder-comparison), [treemap](#cushion-treemap),
  [duplicate finder](#duplicate-finder)
- **🧭 Workspaces and panes:** [Workspace layouts](#workspace-layouts),
  [workspace settings](#workspace-settings),
  [linked navigation](#linked-pane-navigation)
- **🎨 Appearance:** [Themes](#themes), [theme settings](#theme-settings),
  [branding](#branding),
  [native title and header padding](#native-title-and-header-padding)
- **🌐 Languages:** [Jalali dates](#jalali-dates),
  [translation format overrides](#translation-format-overrides)
- **🛠️ Setup and tools:** [Desktop integration](#desktop-integration),
  [integration settings](#integration-settings),
  [Nextcloud integration](#nextcloud-integration),
  [Nextcloud settings](#nextcloud-settings),
  [keyboard shortcuts](#keyboard-shortcuts),
  [embedded terminal](#embedded-terminal),
  [settings backups](#settings-backups),
  [build and installation](#build-installation-and-references),
  [installing optional tools](#installing-optional-alternatives)

## Archives

Archive creation, extraction and integrity testing have these limits, each
tracked in its own issue:

- **Local files:** Archive creation and integrity testing require local files.
  When compressing a mixed selection, Plasma Opus skips items without local
  paths. Password-protected extraction also requires a local archive and
  destination. ([#265][issue-265])

  - **Recommendation:** Copy remote inputs to a local folder before creating,
    testing or extracting an archive; copy the result back afterward. For
    supported remote services, `rclone` is an alternative for transferring
    files.
    - Comment: [#265.7021749894][comment-265]
  - **Alternative:** Stage remote files locally with Dolphin.
    - Comment:
      [#265.7029669109](https://github.com/mikepadiernos/plasma-opus/issues/265#issuecomment-7029669109)
  - **Planned:** [M27 remote connection profiles](ROADMAP.md#m27-integrations)
    are not a commitment to support remote archive creation or testing.
    - Comment: [#265.7021749894][comment-265]

- **Password-protected archives:** Built-in creation supports AES-encrypted ZIP
  only and rejects symbolic links and other unsupported file types. ZIP entry
  names are not encrypted, and 7z archives are created without a password.
  Extraction rejects links, unsafe paths and unsupported entry types. If
  creation or extraction fails, a partial archive or extraction folder may
  remain. ([#255][issue-255], [#256][issue-256])

  - **Recommendation:** Avoid sensitive ZIP entry names, and check for partial
    output after failures before retrying. Use another trusted archive tool if
    the required format or input type is unsupported, such as 7-Zip for formats
    it supports. For encrypted filenames, 7-Zip can encrypt 7z headers; ZIP
    still exposes entry names. Check its handling of links and paths before use.
    - Comment: [#255.7021741358][comment-255]
    - Comment: [#256.7021742119][comment-256]
  - **Alternatives:**
    - Encrypt an entire archive separately with GnuPG.
      - Comment:
        [#255.7029665897](https://github.com/mikepadiernos/plasma-opus/issues/255#issuecomment-7029665897)
    - Extract into a fresh staging directory first.
      - Comment:
        [#256.7029666231](https://github.com/mikepadiernos/plasma-opus/issues/256#issuecomment-7029666231)
    - For encrypted 7z archives with encrypted file names, choose **More
      Options in Ark…** in the Compress dialog; see
      [Ark integration](#ark-integration).

- **Split archives:** **Split into volumes** writes the whole archive, then cuts
  it into raw numbered parts of at most the chosen size, such as
  `photos.7z.001`, as 7-Zip does. They are not ZIP spanned or RAR multi-volume
  archives, so only 7-Zip opens the first part directly; KIO archive browsing
  and most other tools need the parts joined first. Splitting needs free space
  for the whole archive and its parts, and creates the parts with owner-only
  permissions. An archive smaller than the volume size stays a single file. If
  splitting fails, the parts written so far are removed and the whole archive
  is kept. Whether to split by default, and the volume size, are set under
  **Preferences › Archives › Defaults**. ([#732][issue-732])

  - **Recommendation:** Keep the parts together and open the first with 7-Zip,
    or join them with **File › Combine Files**, naming the result after the
    original archive. Check free space before splitting, and adjust the parts'
    permissions before sharing them with other local users.
    - Comment: [#732.7200937321][comment-732]
  - **Alternative:** For RAR multi-volume archives, choose **More Options in
    Ark…**; Ark offers them only when RAR's proprietary `rar` program is
    installed.
    - Comment: [#732.7200937321][comment-732]

- **Encrypted 7z:** Extraction uses KArchive and can fail even for valid
  archives. Integrity testing uses the `7z` executable when available and
  otherwise falls back to KArchive. Passwords containing a newline, carriage
  return or NUL cannot be passed to `7z`. ([#36][issue-36], [#255][issue-255])

  - **Recommendation:** If extraction fails, try a trusted archive tool that
    supports encrypted 7z. For archives you create and intend to test with `7z`,
    choose a password without those characters. 7-Zip is one external option;
    enter passwords at its prompt rather than putting them in command arguments.
    - Comment: [#36.7021740511][comment-36]
    - Comment: [#255.7021741358][comment-255]
  - **Alternative:** Try standalone Ark with its 7z backend, or **File › Open
    in Ark**.
    - Comment:
      [#36.7029665511](https://github.com/mikepadiernos/plasma-opus/issues/36#issuecomment-7029665511)
  - **Ark:** **More Options in Ark…** creates encrypted 7z archives through
    Ark's dialog. It does not change built-in extraction or guarantee safe
    password handling; see [Ark integration](#ark-integration).
    - Comment: [#36.7021740511][comment-36]

### Ark integration

Plasma Opus hands archives to Ark, KDE's archiving tool, when it is installed
([#313][issue-313], [#280](https://github.com/mikepadiernos/plasma-opus/issues/280),
[#647](https://github.com/mikepadiernos/plasma-opus/issues/647)):

- **Current behaviour:** **File › Open in Ark** opens one local archive in
  Ark's own window, where entries can be added, renamed or removed and
  encrypted archives opened with Ark's password prompt. **Extract to Other
  Pane** uses Ark only for local formats that no KIO archive worker can
  browse, such as RAR. It creates a new, uniquely named folder in the other
  pane's local folder and runs Ark in batch mode there, so nothing existing is
  overwritten. **More Options in Ark…** in the Compress dialog opens Ark's own
  Compress dialog for the selection, with encrypted 7z, encrypted file names,
  compression levels and multi-volume archives where the installed backends
  support them. **File › Add to Archive…** adds the selection to an archive
  Ark can write, after a confirmation. Plasma Opus reads which formats Ark can
  open and write from Ark's installed backend plugins and the programs they
  need, and refuses formats Ark can only extract, such as RAR without `rar`.
- **Limits:** Ark reports its own progress, password prompts and errors;
  Plasma Opus reports only a missing Ark or a failed start. If Ark fails or is
  cancelled, the new folder stays, possibly empty. **Add to Archive** changes
  the archive in place, and Ark replaces an entry with the same name; entries
  are named relative to the first selected item's folder. Ark chooses where its
  Compress dialog saves, so the non-overwriting name and pane choices of the
  built-in Compress dialog do not apply. Remote archives and remote
  destinations are not handed to Ark. Formats KIO can browse, including 7z
  and ZIP, still use the built-in extraction, and **Extract
  Password-Protected Archive** still uses KArchive.
- **Password exposure:** Ark's
  [7z plugin](https://github.com/KDE/ark/blob/master/plugins/cli7zplugin/cliplugin.cpp)
  defines a `-p$Password` command-line switch. Plasma Opus never passes a
  password to Ark, but a password typed into Ark's prompt may appear in the
  `7z` process arguments, which other local users can read on some systems.

- **Recommendation:** For encrypted 7z archives that the built-in extraction
  cannot open, use **Open in Ark** with its 7z backend installed. On shared
  systems, avoid typing sensitive passwords into Ark, or use 7-Zip at its own
  prompt instead. Remove an empty folder left by a cancelled Ark extraction.
  Before **Add to Archive**, keep a copy of an archive you cannot recreate, and
  check that no entry with the same name should be kept.
  - Comment: [#313.7021757012][comment-313]
- **Alternative proposal:** Consider a user-controlled external archiver
  handoff.
  - Comment:
    [#313.7029672463](https://github.com/mikepadiernos/plasma-opus/issues/313#issuecomment-7029672463)

## Folder metadata and listings

Folder metadata, flat view and folder sizes have these limits, each tracked in
its own issue:

- **Metadata limits:** Metadata files are limited to 4,096 bytes, notes to 500
  characters and pins to 200 per folder. An edit that would exceed these limits
  or replace a stored value Plasma Opus doesn't support is refused, leaving
  `.mfm.json` unchanged. Folder-format rules are stored in local metadata;
  remote locations use defaults unless the pane's format is locked.
  ([#263][issue-263])

  - **Recommendation:** Keep metadata within these limits; lock the pane format
    when the same format is needed while browsing remote locations.
    - Comment: [#263.7021748197][comment-263]
  - **Alternative:** Keep longer notes in a separate file.
    - Comment:
      [#263.7029668469](https://github.com/mikepadiernos/plasma-opus/issues/263#issuecomment-7029668469)
  - **Native tagging:** [M23 library fields](#native-tagging) are separate
    from folder notes and do not remove these metadata limits.
    - Comment: [#263.7021748197][comment-263]

- **Flat view:** The `filenamesearch:/` KIO worker is required and is normally
  supplied by `kio-extras`. Create New, New Folder and Paste are disabled, and
  drops are refused while flat view is active. ([#261][issue-261])

  - **Recommendation:** Install the worker if flat view is unavailable, and
    leave flat view before creating, pasting or dropping files. For local
    filename searches, `fd` or GNU `find` is an alternative; neither replaces
    KIO flat view.
    - Comment: [#261.7021745774][comment-261]
  - **Alternative:** Use live search and reveal a result in its folder.
    - Comment:
      [#261.7029667831](https://github.com/mikepadiernos/plasma-opus/issues/261#issuecomment-7029667831)

- **Folder sizes:** Background calculations skip symbolic links to directories.
  Calculated sizes are snapshots and can become stale when changes occur outside
  watched directories. View settings offer optional periodic refresh for visible
  panes; it is off by default. Automatic calculation and periodic refresh can be
  costly, especially for remote locations. ([#262][issue-262])

  - **Recommendation:** Enable periodic refresh only when its cost is
    acceptable; GNU `du -L` can follow directory symlinks for local paths. Be
    mindful of cycles and repeated targets.
    - Comment: [#262.7021746538][comment-262]
  - **Alternative:** Take an on-demand size snapshot with GNU `du`.
    - Comment:
      [#262.7029668154](https://github.com/mikepadiernos/plasma-opus/issues/262#issuecomment-7029668154)

## Password-gated folders

**File › Password Protection** gates local folders behind a password inside
Plasma Opus only ([#406](https://github.com/mikepadiernos/plasma-opus/issues/406);
feature [#402](https://github.com/mikepadiernos/plasma-opus/issues/402)):

- **Current behaviour:** A protected folder starts locked in every new
  session. While it is locked, Plasma Opus shows the folder with a lock icon
  but does not list it, preview it, search inside it, open a terminal in it or
  copy, move, trash or delete it or anything inside it; its subfolders are
  hidden from the tree and Yazi columns. Unlocking lasts until **Lock
  Protected Folders**, **Lock** or **Lock All** under **Preferences ›
  Security › Password-Gated Folders**, or the end of the session; there is no
  idle auto-lock. The MCP server treats every protected folder as locked.
- **Not security:** The gate is not encryption or filesystem access control.
  Other file managers, terminals and processes with the same user
  permissions use the folder freely, and standard permissions and ACLs
  distinguish users and groups, not applications running with the same
  credentials. Removing the folder's entry from `opus-file-managerrc` removes
  the gate. Plasma Opus stores a salted PBKDF2-SHA256 verifier rather than the
  password, and does not use KWallet.
- **Leaks inside Plasma Opus:** A locked folder can still be dragged to
  another application, which then copies or opens it. Folder sizes,
  **Properties** counts, the trash, recent files, history, bookmarks and
  search collections can show sizes or names from inside it, and thumbnails
  cached before it was protected stay in the thumbnail cache.
- **Paths, not folders:** A gate follows a folder moved or renamed in Plasma
  Opus, but a folder moved by another application leaves its gate behind,
  unprotected at the new path. A gate on a deleted folder stays and gates any
  new folder created at the same path. Bind mounts, hard-linked files and
  other paths to the same data are not gated.

- **Recommendation:** Treat the gate as a privacy screen against casual
  browsing in Plasma Opus, not as protection. For confidential files, use a
  separate encrypted vault such as
  [KDE Plasma Vault](https://github.com/KDE/plasma-vault): applications with
  access can use files while the vault is unlocked and mounted, and the
  decrypted contents are unavailable through the mount when it is locked.
  Back up the data, protect the passphrase and handle any original
  unencrypted copies separately. Lock protected folders before leaving the
  desktop, move them only in Plasma Opus, and remove protection before
  deleting a folder.
  - Comment:
    [#406.7040077820](https://github.com/mikepadiernos/plasma-opus/issues/406#issuecomment-7040077820)
- **Alternatives:**
  - **Cooperative prompts:** Explore a shared gate only with file managers that
    explicitly support it. Unsupported apps and direct access can bypass it;
    cross-manager support is not established and this is not security.
    - Comment:
      [#406.7040078439](https://github.com/mikepadiernos/plasma-opus/issues/406#issuecomment-7040078439)
  - **Separate identities:** Use ownership and ACLs to restrict access to
    selected user or service identities. Apps with the same credentials retain
    the same ordinary access; this does not add a folder-password prompt.
    Privileged users can override controls, and administration requires care.
    - Comment:
      [#406.7040079139](https://github.com/mikepadiernos/plasma-opus/issues/406#issuecomment-7040079139)
  - **Application sandboxing:** Restrict a controlled set of sandboxed apps and
    grant chosen apps access through supported sandbox or portal mechanisms.
    Unsandboxed same-user processes can still access the folder; support and
    permissions vary, so this is not universal password enforcement.
    - Comment:
      [#406.7040079777](https://github.com/mikepadiernos/plasma-opus/issues/406#issuecomment-7040079777)
- **Proposed improvements:** Gate locked folders when dragging them out,
  hide their sizes and names in Properties, history and collections, add an
  idle auto-lock, and offer Plasma Vault for folders that need encryption.

## Previews and gallery

Previews, the gallery and batch image conversion have these limits, each
tracked in its own issue:

- **Preview dependencies:** Document previews require a suitable installed
  KPart, media playback requires a Qt Multimedia backend, and available metadata
  depends on KDE extraction plugins. ([#259][issue-259])

  - **Recommendation:** Install the relevant KPart, multimedia backend or
    extraction plugin for the file types you need. For supported media, FFmpeg's
    `ffplay`, when included in the installed build, is an external playback
    alternative.
    - Comment: [#259.7021744368][comment-259]
  - **Alternative:** Open the file with a separate application.
    - Comment:
      [#259.7029667136](https://github.com/mikepadiernos/plasma-opus/issues/259#issuecomment-7029667136)

- **Preview limits:** Images are limited to 8 MiB and 40 megapixels. Text
  previews use UTF-8 and default to the first 1 MiB; lines longer than 16,384
  characters are shown without syntax highlighting. Remote previews download to
  a temporary file and use the same limits, or 128 MiB for other file types.
  Since M29, the preview panel first asks a Nextcloud account's server for its
  preview image, up to 8 MiB, and downloads the file only when there is none,
  on request or when that is turned off under **Preferences › Remote and Cloud
  › Nextcloud** ([Nextcloud integration](#nextcloud-integration)).
  ([#259][issue-259], [#264][issue-264])

  - **Recommendation:** Open files in a suitable external application when
    previews are incomplete, exceed these limits or require another encoding.
    For local text files, `bat` or Neovim may be useful.
    - Comment: [#259.7021744368][comment-259]
    - Comment: [#264.7021749088][comment-264]
  - **Alternative:** Use Kate or KWrite for long-line text.
    - Comment:
      [#264.7029668772](https://github.com/mikepadiernos/plasma-opus/issues/264#issuecomment-7029668772)
  - **Planned:** [M27 editor integrations](ROADMAP.md#m27-integrations)
    launch separate editors; they do not increase Plasma Opus preview limits.
    - Comment: [#264.7021749088][comment-264]

- **Rendered previews:** HTML and Markdown previews disable links and external
  resources. ([#267][issue-267])

  - **Recommendation:** Open trusted documents in an appropriate external
    application if links or external resources are needed.
    - Comment: [#267.7021751490][comment-267]
  - **Alternative:** Use a separate browser for trusted rendered documents.
    - Comment:
      [#267.7029669771](https://github.com/mikepadiernos/plasma-opus/issues/267#issuecomment-7029669771)

- **Gallery:** The gallery supports local files only. Gallery images and batch
  conversion inputs are limited to 128 MiB and 40 megapixels. Saving edits or
  converting images does not preserve the original image metadata.
  ([#266][issue-266], [#260][issue-260])

  - **Recommendation:** Work on local copies within the limits and keep the
    originals when their metadata must be preserved. ExifTool can inspect and
    copy supported metadata separately; verify the result for the target format.
    ImageMagick can process supported local images outside Plasma Opus's
    gallery, subject to its own resource and format limits.
    - Comment: [#266.7021750778][comment-266]
    - Comment: [#260.7021745029][comment-260]
  - **Alternatives:**
    - Keep the metadata-bearing original alongside an edited copy.
      - Comment:
        [#260.7029667492](https://github.com/mikepadiernos/plasma-opus/issues/260#issuecomment-7029667492)
    - Try a separate image viewer for a local copy.
      - Comment:
        [#266.7029669439](https://github.com/mikepadiernos/plasma-opus/issues/266#issuecomment-7029669439)

- **Batch conversion:** Conversion reads files on disk, not unsaved gallery
  edits, and turns animated images into still images. The conversion dialog
  warns about both limitations. ([#303][issue-303])

  - **Recommendation:** Save gallery edits before conversion; use an
    animation-capable tool such as FFmpeg or ImageMagick when animation must be
    retained, and verify that the chosen output format supports it.
    - Comment: [#303.7021753023][comment-303]
  - **Alternative proposal:** Consider a future animation-aware conversion
    backend.
    - Comment:
      [#303.7029670421](https://github.com/mikepadiernos/plasma-opus/issues/303#issuecomment-7029670421)

- **Conversion output:** The destination filesystem must support atomic
  no-replace renames, which prevent overwriting files, including those created
  after preview. Unsupported filesystems cause conversion failures. Converted
  files have owner-only permissions; change permissions if needed before sharing
  them. ([#302][issue-302], [#304][issue-304])

  - **Recommendation:** Convert to a filesystem that supports atomic no-replace
    renames, then copy the output if needed. Review permissions before sharing;
    for local files, `chmod` can grant only the access needed.
    - Comment: [#302.7021752278][comment-302]
    - Comment: [#304.7021753785][comment-304]
  - **Alternatives:**
    - Use a separate conversion tool with a fresh destination.
      - Comment:
        [#302.7029670102](https://github.com/mikepadiernos/plasma-opus/issues/302#issuecomment-7029670102)
    - Use a targeted ACL when sharing converted output.
      - Comment:
        [#304.7029670734](https://github.com/mikepadiernos/plasma-opus/issues/304#issuecomment-7029670734)

## Editor

The text editor's drafts, Markdown handling and editing in the preview have
these limits, each tracked in its own issue:

- **Draft storage:** Drafts are limited to 8 MiB of UTF-8 text and are stored in
  the application's private data directory, not in the original file. If draft
  storage fails, save the document before switching files or closing.
  ([#307][issue-307])

  - **Recommendation:** Save work to the original file regularly and use an
    external editor such as Neovim for documents that exceed the draft limit.
    - Comment: [#307.7021754467][comment-307]
  - **Alternative:** Use a standalone editor and an explicit backup copy.
    - Comment:
      [#307.7029671086](https://github.com/mikepadiernos/plasma-opus/issues/307#issuecomment-7029671086)
  - **Planned:** [M27 editor integrations](ROADMAP.md#m27-integrations) are
    separate applications and do not expand Plasma Opus draft storage.
    - Comment: [#307.7021754467][comment-307]

- **Draft retention:** Drafts are retained indefinitely by default. An optional
  age limit removes expired drafts unless they are locked by an active editor.
  Legacy drafts from versions without locking remain until explicitly discarded.
  Restored drafts remain until discarded or expired, protecting another editor's
  changes during recovery. ([#307][issue-307])

  - **Recommendation:** Set an age limit if appropriate and explicitly discard
    unneeded legacy or restored drafts after checking their contents.
    - Comment: [#307.7021754467][comment-307]

- **Markdown resources:** Markdown preview, visual editing and PDF export do not
  load local or external resources. Links are also disabled in the preview.
  ([#267][issue-267])

  - **Recommendation:** Use a trusted external renderer when a document or PDF
    needs embedded resources or working links. Pandoc can render local
    resources; PDF output requires an appropriate PDF engine.
    - Comment: [#267.7021751490][comment-267]

- **Visual Markdown editing:** Qt's Markdown subset can normalize the source or
  remove unsupported markup and HTML. Visual editing requires confirmation and
  preserves a recovery snapshot when drafts are enabled. ([#309][issue-309])

  - **Recommendation:** Keep a backup and use source editing when preserving
    unsupported markup is important; Neovim is one external source editor.
    - Comment: [#309.7021755164][comment-309]
  - **Alternative:** Compare a saved copy against the original before adopting
    edits.
    - Comment:
      [#309.7029671476](https://github.com/mikepadiernos/plasma-opus/issues/309#issuecomment-7029671476)
  - **Planned:** [M27 editor integrations](ROADMAP.md#m27-integrations) do not
    change the built-in visual editor's Markdown limitations.
    - Comment: [#309.7021755164][comment-309]

- **Editing in the preview:** The preview's **Edit** action edits only local,
  writable text files up to 10 MiB. It refuses binary content and files that
  KTextEditor opens read-only, such as unsupported encodings or very long
  lines, and remote text previews report that they can't be edited there.
  Moving a session to the editor panel keeps the text and its unsaved state,
  but not the undo history, cursor position or visual Markdown mode. A file
  open in both the preview and the editor panel is two independent documents.
  ([#755][issue-755])

  - **Recommendation:** Use the preview for quick edits to local text files,
    and the editor panel or an external editor such as Kate or Neovim for
    remote, large, binary or unusually encoded files. Move a session to the
    editor panel before you need its undo history, and edit a file in only
    one place at a time.
    - Comment: [#755.7207460156][comment-755]

## File-list minimap

The file-list minimap has these limits
([#434](https://github.com/mikepadiernos/plasma-opus/issues/434)):

- **Sampling:** The minimap samples at most 512 top-level listing rows rather
  than showing every item or expanded child row.
- **Scrolling:** It tracks only the vertical scrollbar, so it cannot navigate
  horizontally scrolling columns in compact view.

- **Recommendation:** Use the ordinary scrollbar or keyboard navigation for
  compact view and expanded folders, and treat the minimap as a sampled overview
  rather than a complete list.
  - Comment:
    [#434.7040730100](https://github.com/mikepadiernos/plasma-opus/issues/434#issuecomment-7040730100)

## Yazi mode

The M25 [Yazi mode](yazi-mode.md) has these limits
([#723](https://github.com/mikepadiernos/plasma-opus/issues/723)):

- **Scope:** Yazi mode is a native Qt/KDE browsing mode inspired by Yazi. It
  doesn't run Yazi or read its configuration, keymaps, themes or plugins.
- **Locations:** Search results, recent items, tags, `remote:/` and
  application lists aren't supported; the pane falls back to the normal view
  there. Remote folders, archives, `trash:/` and `desktop:/` are limited: the
  parent column and preview may load slowly, and listings stop after 10
  seconds. Archives are recognised by file extension.
- **Preview:** A folder preview lists at most 500 entries. Remote files are
  downloaded for preview up to the existing size caps; there is no setting
  that stops remote preview downloads. Oversized text is truncated rather
  than refused. Hiding the preview stops media playback, and media may render
  poorly in a narrow column.
- **Keys:** With Vim-style keys, bound letters and digits no longer start
  type-ahead in the columns, <kbd>Space</kbd> no longer opens the gallery, and
  <kbd>Ctrl</kbd>+<kbd>D</kbd> (Add to Favorites) is overridden there.
  <kbd>Ctrl</kbd>+<kbd>C</kbd> doesn't cancel a search because it is Copy.
  With Standard keys, <kbd>Left</kbd> and <kbd>Right</kbd> mean Parent and
  Open in icon and card views. <kbd>Caps Lock</kbd> changes letter bindings.
- **File actions:** <kbd>c</kbd> and <kbd>m</kbd> copy or move straight to
  the other pane like <kbd>F5</kbd> and <kbd>F6</kbd>, so they follow the
  transfer setting **Existing destination**; with **Overwrite automatically**
  they overwrite without asking. <kbd>u</kbd> undoing a copy deletes the
  copies after KIO asks. Window shortcuts such as <kbd>Delete</kbd> don't fall
  back to the focused item when nothing is selected. Paste
  and trash don't appear in the Yazi transfer indicator.
- **Selection:** Marked items stay selected while moving, unlike the normal
  view. In grid views, range selection follows item order.
- **Search:** The status row only tracks searches started with <kbd>s</kbd>
  or <kbd>S</kbd> from that pane, and those keys change the Search panel's
  scope settings.
- **Persistence:** A pane follows the default column ratios until they are
  dragged; other panes pick up a new default only when settings are applied
  again. Existing workspaces tie Yazi state to the views option until their
  Yazi checkbox is changed. When startup opens the home folder or fixed
  folders, panes start with their default mode from **Preferences › Yazi
  Mode › Activation**, which is off unless changed.
- **Performance:** The first listing of very large folders (10,000+ items)
  is slow with or without Yazi mode.

- **Recommendation:**
  - **Scope:** Use Yazi itself, for example in the embedded terminal, when
    you need its plugins or configuration.
  - **Locations:** Browse unsupported locations in the normal view; the mode
    resumes at the next supported folder. Hide the preview with **View › Yazi
    Preview** in slow remote folders.
  - **Preview:** Switch the column to file information (<kbd>i</kbd> with
    Vim-style keys) on slow connections, and open large files with **Open
    With**.
  - **Keys:** Keep the Standard preset if you rely on type-ahead, the gallery
    on <kbd>Space</kbd> or <kbd>Ctrl</kbd>+<kbd>D</kbd>. Cancel searches with
    <kbd>Escape</kbd> or **Cancel**.
  - **File actions:** Keep **Existing destination** at **Ask** (the default)
    before using <kbd>c</kbd> and <kbd>m</kbd>. Select items explicitly before
    using window shortcuts.
  - **Selection:** Press <kbd>Escape</kbd> to clear marks before ordinary
    selection.
  - **Search:** Check the Search panel's scope after using <kbd>s</kbd> or
    <kbd>S</kbd>.
  - **Persistence:** Drag the columns once in each pane to fix its ratios,
    or use **Restore Yazi Mode Defaults** in Preferences to make every pane
    follow the defaults again. Turn on a workspace's **Yazi mode** option to
    restore Yazi state independently of views.
  - **Performance:** Filter or split very large folders.
  - Comment:
    [#723.7180859654](https://github.com/mikepadiernos/plasma-opus/issues/723#issuecomment-7180859654)
- **Proposed improvements:**
  - A remote preview download policy shared with the preview panel.
  - Track paste and trash jobs in the Yazi transfer indicator.
  - Faster first listings of very large folders.

## Yazi mode settings

The M26 [Yazi mode settings](yazi-mode.md#settings) under **Preferences ›
Yazi Mode** have these limits
([#725](https://github.com/mikepadiernos/plasma-opus/issues/725)):

- **Activation:** Each pane's default mode and restore option apply only
  when a window opens; open panes keep their current mode. The restore option
  matters only when **On startup** restores the last session. Sessions saved
  before M25 restore the mode as off. "First pane" is the left pane, or the
  top pane when the panes are stacked.
- **Columns:** Panes with their own widths, from dragging, their session or
  a workspace, keep them when the default widths change, so Apply can seem to
  do nothing. After a drag, the Columns tab shows the widths in pixels, such
  as 180, 540 and 360. The narrow-pane limits are fixed: the preview column
  hides below about 480 pixels and the parent column below about 320.
- **Keyboard:** Preset keys can't be edited one by one. The conflict review
  checks the window that opened Preferences, leaves out shortcuts that only
  work while their own widget has focus and sees shortcut edits only after
  they are saved. **View › Vim-Style Keys in Yazi Mode** switches presets
  without reviewing conflicts.
- **Preview:** There is no Yazi-specific size, provider or remote-download
  setting. Remote preview downloads stop at fixed caps (8 MiB for images,
  128 MiB for other files) and ignore **Show previews in remote folders**;
  **Skip files larger than** and **Show previews for** only affect
  thumbnails. The file-information fields are fixed. Changed presentation
  settings, such as line numbers, apply from the next previewed item.
- **Workspace layouts:** A pane that keeps its own Yazi state differs from
  the saved layout, so the workspace shows the modified marker (`*`) after
  switching. Restoring only the columns also changes panes that aren't in
  Yazi mode; the columns appear when the mode is turned on. The default for
  new workspaces stays under **Workspaces › Behaviors**.
- **Reset:** **Restore Yazi Mode Defaults** makes open panes drop their own
  column and preview choices on Apply, even if values are edited again before
  Apply, and can mark the active workspace as modified. It never turns a
  pane's Yazi mode on or off. The dialog's own **Defaults** button resets
  every page but keeps the panes' own choices.

- **Recommendation:**
  - **Activation:** Turn a pane's mode on or off with **View › Yazi Mode**
    after changing its default; the setting takes over at the next start.
  - **Columns:** Use **Restore Yazi Mode Defaults** when open panes should
    follow new default widths. Use **Restore Default Widths** on the Columns
    tab to return to 1 : 3 : 2 after a drag.
  - **Keyboard:** Change presets from the Keyboard tab rather than the View
    menu so conflicts are reviewed, and save edits in **Configure Keyboard
    Shortcuts…** before reviewing again.
  - **Preview:** Switch the preview column to file information on slow
    connections, and use **Maximum text in preview panel** under **Interface
    › Previews** to limit remote text downloads.
  - **Workspace layouts:** Save the workspace layout again when the modified
    marker should clear, or turn both Yazi options back on to restore the
    saved state.
  - **Reset:** Use the Yazi Mode button instead of **Defaults** when only
    Yazi settings should change.
  - Comment:
    [#725.7182507952](https://github.com/mikepadiernos/plasma-opus/issues/725#issuecomment-7182507952)
- **Proposed improvements:**
  - Editable Yazi key bindings, built on KDE shortcut configuration.
  - Conflict review for the View menu's key preset toggle.
  - A remote preview download policy shared with the preview panel.
  - Settings for the narrow-pane limits.
  - Apply activation defaults to open panes on request.

## Search

Live search, indexed search, the search panel and the toolbar search field
have these limits
([#362](https://github.com/mikepadiernos/plasma-opus/issues/362)):

- **Live search:** Scans local folders only and skips symbolic links. Content
  search covers UTF-8 files up to 2 MiB without NUL bytes. Search stops after
  5,000 matches or 200,000 entries, but lists each directory before applying the
  entry limit.
- **Indexed search:** Requires Baloo at build and runtime and searches its
  configured locations. It matches indexed terms rather than literal substrings,
  does not accept double quotes, and may omit files or return stale results.
  Baloo applies required content terms and alternatives, folder, image, audio
  and video types and modification dates itself; NOT, name, path, size,
  extension, tag and metadata criteria are still checked against the files it
  returns. Types and dates use the MIME type and modification time Baloo
  recorded, so files changed since indexing or with misleading names may be
  missed. Plasma Opus examines at most 200,000 Baloo matches, says so when it
  stops there, and returns at most 5,000 matches. A depth limit or workspace
  search asks Baloo about each location; a location with no indexed matches,
  such as an unindexed folder, makes the search ask the whole index instead,
  which is slower.
  ([#733](https://github.com/mikepadiernos/plasma-opus/issues/733))
- **Index status:** Coverage notices reflect configuration, not proof of
  indexing; exclusions and pending indexing can affect results. Baloo's query
  API does not report index-read failures, and a running query cannot be
  interrupted until it returns. **Indexing Status** reproduces Baloo's
  folder, hidden-item, pattern and file-type rules from KDE File Search
  settings; the indexer may still skip files it can't read, very large
  files or files it failed to extract. **Check Index Coverage** looks up only
  the folder's first 200 files, not subfolders, and a file in the index may
  still have stale contents. Missing extractors are detected only for
  document, text, image, audio, video and ebook types among those files.
  The indexer's state is read over D-Bus when the status is opened or the
  Baloo scope is chosen, then updated when the indexer reports a change; its
  remaining-time estimate is Baloo's own. Suspend, resume and checking for
  unindexed files ask the shared indexer, which may ignore them, for example
  on battery power. This has been verified in a headless container with a
  simulated indexer only, not yet against a running Baloo on a Plasma
  session.
  ([#441](https://github.com/mikepadiernos/plasma-opus/issues/441))
- **Indexed metadata and ranking:** A `meta:` term selects Baloo candidates
  only when the query has no name or content term. Baloo then matches whole
  indexed words in any indexed text and in the enabled text properties, not
  substrings, so `meta:Can` may not find Canon; every candidate is confirmed by
  reading its metadata, which is slower on large result sets. Numbers, dates
  and properties Baloo does not index, or files whose extractor is missing,
  can't be found this way, and Baloo limited to file names finds no metadata;
  add a name term or search live instead. Ranking compares file names only,
  not paths, contents or metadata, and only reorders the results found: it
  can't add files missing from the index or beyond the 5,000-result limit,
  and the order is by score, not relevance as Baloo computes it. Ranking and
  the live search are offered only while they are checked under **Preferences
  › KDE Plasma › Baloo**. **Search Current Folder Live** appears only after an
  indexed search that was not cancelled, for a local current folder or
  workspace locations, and switches the scope to the live search before
  running it, so later searches stay live until the scope is changed back.
  ([#442](https://github.com/mikepadiernos/plasma-opus/issues/442))
- **Shared searches:** A saved search shared with KDE Places is a Baloo
  search, so Dolphin and file dialogs list only indexed files, match whole
  words rather than parts of names or text, and use the MIME types and
  dates Baloo recorded. Top-level criteria Baloo can't express are dropped
  rather than approximated, so the place may list more files than the saved
  search: sizes, extensions, paths, tags, metadata numbers, NOT terms, name
  wildcards and exact names, today and yesterday, depth limits, path filters
  and exclusions. A search of several search folders covers every indexed
  location, since Baloo limits a search to one folder, and remote folders
  can't be shared. The place doesn't follow later changes to the saved
  search, and renaming or deleting the saved search leaves it in Places.
  ([#734](https://github.com/mikepadiernos/plasma-opus/issues/734))
- **Selection and exclusions:** Path-selection filters apply only to local
  top-level listings or the visible flat list, not expanded child rows. Excluded
  folders are entered one per line, so paths containing newlines cannot be
  entered; exclusions are lexical paths, not resolved links.
- **Results and collections:** Results can be opened or revealed in the active
  pane, but hidden or filtered items may not be visible and flat view must be
  turned off. A folder result opens in the active pane; a file opens in its
  application. The shared collection stores local URLs, not file contents;
  references can become stale.
- **Search panel:** The panel searches the folder it was opened for and does
  not follow later navigation; search again from a pane to change it. While a
  search runs, the folder can't change until the search finishes or is
  cancelled. By default the panel follows the embedded terminal's placement,
  below or beside the panes. Preferences' Search page can instead place it
  below or right of the panes; when that direction differs from the
  terminal's, the panel spans the panes and terminal together and its size is
  remembered separately. The panel is hidden at startup unless Preferences
  enables reopening it. Each new window then reopens it, without taking focus,
  if it was shown when last toggled in any window; this state is not kept per
  window or workspace. Workspace layouts don't record the search panel's
  placement or visibility. Hiding the panel does not cancel a running search.
- **Toolbar search field:** The optional Search Field toolbar entry is not on
  any toolbar by default; add it from Preferences' Search page, which places
  it before the Search button, or with Customize Main Toolbar. Both change the
  same main toolbar entries, so restoring default toolbars removes it, and
  the Preferences option is applied even when toolbars are locked. Pressing
  <kbd>Enter</kbd> searches the active pane's current folder with the text
  typed, but uses the other criteria, such as scope and filters, currently set
  in the search panel. While a search runs, <kbd>Enter</kbd> does not start
  another search. The field's Saved Searches menu runs a saved search with all
  its saved criteria, including its folder, and replaces the panel's criteria.
  Searching from the field shows the search panel unless Preferences turn this
  off; the status bar then reports how the search ended for five seconds, or
  why it could not start, but errors and skipped files are listed only in the
  panel. Empty text still shows the panel. The field can be placed on the main
  toolbar only, not on pane toolbars. It suggests recent queries while typing,
  and <kbd>Alt</kbd>+<kbd>Down</kbd> lists them.
- **Result presentation and history:** Results and collection references are
  listed by full path or by name with their folder; the choice next to the
  result tabs applies to the current panel and Preferences' Search page sets
  its default. Copying, opening and revealing always use the full path. Recent
  queries, up to 50, are shared by every window and kept as plain text in
  Plasma Opus's state file, not in a protected store. A query is remembered
  when its search starts, including searches started from the toolbar field or
  a saved search. Turning off Remember recent search queries forgets stored
  queries the next time a window applies Preferences, and Clear Search History
  forgets them at once; neither removes queries already shown in an open
  completion list or saved searches.
- **Search keyboard navigation:** <kbd>Down</kbd> in the toolbar field or the
  panel's query moves to the visible result list, or from the toolbar field to
  the query when there are no results. With the panel hidden, <kbd>Down</kbd>
  in the field shows it only when it holds results. <kbd>Escape</kbd> cancels
  a running search; otherwise it returns focus to the active pane and leaves
  the panel open. <kbd>Tab</kbd> moves through the panel in layout order, but
  between the toolbar, panel and panes it follows the window's widget order.
  There are no dedicated shortcuts for focusing the toolbar field or the
  panel.
- **Advanced queries:** AND, OR and NOT are operators only in upper case;
  quote a value that contains a colon, starts with `-` or is one of those
  words. Words without a field match file names, and unquoted name and path
  values treat `*` and `?` as wildcards. Size criteria match files only, so
  NOT size criteria also match folders. Sizes use binary units: 1K is 1,024
  bytes, and `size:10M` matches exactly 10 MiB. Dates use local time. An
  extension such as `type:pdf` compares names only; a MIME type is detected
  from the name and, if needed, the contents, which can slow live searches and
  miss misnamed files. Content criteria use live search's text limits;
  unreadable, binary and oversized files never contain a term, so NOT content
  criteria match them. Indexed queries need a name, content or meta term, or
  a group of content alternatives, that every result must match, which
  selects Baloo candidates. Content terms use Baloo's word matching rather
  than substrings. Baloo also applies required folder, image, audio and video
  types and modification dates; other criteria, and any criterion under NOT
  or in an OR group with other fields, are checked against the files found.
  The normalized query shown while typing uses English field names.
- **Search folders and exclusions:** The search folders and excluded folders
  set on Preferences' Search page apply only to Plasma Opus. They do not
  change what Baloo indexes, and an excluded folder is still searched by other
  applications. Search folders are searched live, so they share live search's
  local-only, symbolic-link and result limits. Exclusions are lexical paths:
  a symbolic link elsewhere does not inherit its target's exclusion.
- **Search defaults:** The default scope, search field and path filter on
  Preferences' Search page set the panel's initial criteria, which the toolbar
  field also uses. When they change, an open panel updates only criteria still
  showing the previous defaults, and waits until a running search finishes.
  The default path filter is stored by name: renaming or deleting the shared
  filter leaves it unavailable, so searches start without a path filter and
  report it until another filter is chosen. Settings from before M22 are
  migrated when Plasma Opus starts; the earlier Boolean keys are removed.
- **Double Shift:** Pressing <kbd>Shift</kbd> twice opens search only while a
  Plasma Opus main window has keyboard focus; it is not a global shortcut and
  can't be assigned in Configure Keyboard Shortcuts. It is turned off while
  Sticky Keys is on in Plasma's accessibility settings, and Plasma Opus can't
  detect input methods or remappings that intercept <kbd>Shift</kbd>.

- **Recommendation:** Check Baloo's indexing configuration and status in
  **File › Indexing Status…**, and use **Check Index Coverage** to confirm
  that a folder's files are indexed, or `balooctl6 status` for details Baloo
  doesn't expose to applications; use live search for a current local folder,
  or **Search Current Folder Live** after an indexed search, search smaller
  folders, inspect errors and repeat searches after files change. Use whole
  words in indexed `meta:` terms, and add a name term or search live for
  numbers, dates or partial words. Turn ranking off to keep the order in which
  results were found. Use flat view when selecting nested local items by path,
  and verify the resulting selection. Use GNU `find` for other local filename
  searches or ripgrep (`rg`) for content searches when Plasma Opus limits are
  unsuitable; check each tool's hidden-file, ignore, encoding and
  symbolic-link behaviour. If a revealed result is hidden or filtered, adjust
  the pane's visibility settings. Re-run saved searches and check collection
  references after files change. Use KDE File Search settings to change what
  Baloo indexes for every application. When pressing <kbd>Shift</kbd> twice is
  unavailable, use the Search shortcut
  (<kbd>Ctrl</kbd>+<kbd>Alt</kbd>+<kbd>F</kbd> by default), which can be
  changed in Configure Keyboard Shortcuts. To search another folder from the
  search panel, use Search from that pane's toolbar or the Search shortcut
  while the pane is active, or type in the toolbar search field. Check the
  panel's criteria before searching from the toolbar field. Press
  <kbd>Down</kbd> to reach the results and <kbd>Escape</kbd> to return to the
  active pane; click the panel or use <kbd>Tab</kbd> when keyboard focus needs
  to move elsewhere. Check the normalized query shown while typing to confirm
  grouping, use parentheses where AND and OR are mixed, and search the current
  folder or search folders when an advanced query has no required name,
  content or meta term. Search live when recently changed or misnamed files
  must match an indexed type or date, and add a name or content term when an
  indexed search reaches the 200,000-match limit. Read the criteria listed
  before sharing a search with KDE Places, share it again after changing it,
  and remove stale places from Places in any KDE application. After renaming
  or deleting a shared path filter, choose the default path filter again on
  Preferences' Search page. With several windows open, hide the search panel
  in the last window you use if it should stay closed at the next startup.
  Turn off Remember recent search queries before searching for sensitive text,
  or use Clear Search History afterwards. Choose Names and folders when long
  paths make results hard to scan. When toolbar searches leave the panel
  hidden, press <kbd>Down</kbd> in the field after a search to check its
  errors and results.
  - Comment: [#362.7023269364][comment-362]
- **Alternative:** Use a separately maintained locate database for filenames.
  - Comment:
    [#362.7029675758](https://github.com/mikepadiernos/plasma-opus/issues/362#issuecomment-7029675758)
- **Planned:** [M21 advanced search](ROADMAP.md#m21-advanced-search) reuses M09
  search; its new interface and criteria do not by themselves remove current
  backend limits.
  [M31 keyboard shortcut settings](ROADMAP.md#m31-keyboard-shortcut-settings)
  lists the Search shortcut and pressing <kbd>Shift</kbd> twice on
  **Preferences › Keyboard › Shortcuts**
  ([#230](https://github.com/mikepadiernos/plasma-opus/issues/230)) and in
  **Help › Keyboard Map…**
  ([#232](https://github.com/mikepadiernos/plasma-opus/issues/232)); **Show
  Search Panel**, which focuses the panel's query, can be given a shortcut in
  Configure Keyboard Shortcuts.
  [M15 workspaces](ROADMAP.md#m15-workspaces)
  plan search scoping, not a security boundary.
  - Comment: [#362.7023269364][comment-362]

## Batch rename

Batch rename has these limits
([#372](https://github.com/mikepadiernos/plasma-opus/issues/372)):

- **Inputs and destinations:** Only local items are supported. The destination
  filesystem must support Linux atomic no-replace renames. Preview rejects
  existing destinations, rename chains, swaps, and selections containing both a
  folder and its descendants.
- **Patterns and metadata:** Metadata tokens depend on installed KDE extractors
  and embedded fields; missing metadata and invalid names prevent application.
  Regular expressions are case-sensitive and apply to base names only,
  preserving extensions. Expensive expressions can delay a preview and cannot be
  cancelled.
- **Changes and recovery:** The preview is a snapshot. Source identity is
  checked before each rename, but changes can race those checks. Errors or
  cancellation stop the batch and can leave partial results. Renames are not
  registered with KIO undo. Per-folder Plasma Opus labels, colours, ratings,
  notes and pins remain keyed by old names.

- **Recommendation:** Review the preview and back up important items first. Use
  valid patterns and metadata fields present on every selected file; install KDE
  metadata extractors if needed. Use `(?i)` for case-insensitive regex matching
  and avoid expensive expressions. Use a filesystem that supports atomic
  no-replace renames. After an error or cancellation, inspect the displayed
  results and filesystem before retrying; manually rename completed items back
  if needed. Reapply per-folder annotations to new names and remove stale
  entries as appropriate.
  - Comment: [#372.7024683530][comment-372]
- **Alternative:** Try a separate batch renamer on copies.
  - Comment:
    [#372.7029676077](https://github.com/mikepadiernos/plasma-opus/issues/372#issuecomment-7029676077)
- **Native tagging:** [M23 libraries](#native-tagging) follow batch renames in
  Plasma Opus, keeping library tags and fields; existing per-folder annotations
  are not migrated.
  - Comment: [#372.7024683530][comment-372]

## Metadata editing

Embedded metadata editing has these limits
([#374](https://github.com/mikepadiernos/plasma-opus/issues/374)):

- **Formats and dependencies:** Edit one local JPEG or TIFF (description, artist
  and copyright) or MP3 (title, artist and album) at a time. ExifTool is
  required to read all supported formats and write images; MP3 writes require
  the KDE FileMetaData writer plugin. Unsupported formats, linked files and
  fields containing line breaks cannot be edited.
- **Preview and writes:** The preview covers only displayed values, not
  duplicate or unrelated metadata or media contents. File identity and
  timestamps are checked before writing but can race external changes. Writes
  are not undoable, and a failure or timeout may leave changes. The KDE writer
  does not report write errors: Plasma Opus re-reads displayed fields but cannot
  prove preservation of other data.

- **Recommendation:** Back up the original and review the preview. Install
  ExifTool and the KDE metadata writer plugin and ensure ExifTool runs in your
  environment. Use a regular local file without symbolic or hard links. Reopen
  and inspect media and metadata after writing or an error; use a dedicated
  metadata editor for other formats, fields, multiple values or stronger
  preservation guarantees.
  - Comment: [#374.7025134952][comment-374]
- **Alternative:** Try a dedicated tag editor for other audio fields.
  - Comment:
    [#374.7029676829](https://github.com/mikepadiernos/plasma-opus/issues/374#issuecomment-7029676829)
- **Native tagging:** [M23](#native-tagging) reads and maps selected embedded
  metadata into libraries and reuses this editor for supported writes; it adds
  no writable formats or preservation guarantees.
  - Comment: [#374.7025134952][comment-374]

## Transfer queue

The transfer queue has these limits
([#375](https://github.com/mikepadiernos/plasma-opus/issues/375)):

- **Coverage:** The queue handles pane, context-menu and gallery copy and move
  actions. Clipboard paste and drag-and-drop use KIO directly; they do not
  appear in the queue or use its conflict policy.
- **Scheduling and conflicts:** One job runs at a time by default; concurrency
  is configurable. Disabling queue scheduling starts jobs without a concurrency
  limit. Automatic overwrite replaces files and merges folders without asking.
- **Controls and lifetime:** Progress, speed and controls depend on the KIO job
  and worker. Cancellation may leave partial results. The queue exists only in
  memory: waiting jobs are discarded on application exit and active jobs may be
  cancelled. Plasma Opus warns before closing a window while transfers are
  pending. External KIO progress interfaces may control a job independently of
  the queue display. KIO records undo when a queued copy or move begins, not
  while waiting.

- **Recommendation:** Use queued pane, context-menu or gallery copy and move
  actions and choose concurrency appropriate for the destination. Keep the Ask
  conflict policy when confirmation is needed; review the destination before
  using automatic overwrite. Use the normal KIO progress interface for paste and
  drag-and-drop. Check partial results after cancellation or errors and keep
  Plasma Opus open until queued work finishes. Resume externally paused jobs
  from the external interface. Inspect the KIO undo stack after a transfer
  starts.
  - Comment: [#375.7025250130][comment-375]
- **Alternative proposal:** Consider adding paste and drop jobs to the queue.
  - Comment:
    [#375.7029677171](https://github.com/mikepadiernos/plasma-opus/issues/375#issuecomment-7029677171)
- **Planned:** [M27](ROADMAP.md#m27-integrations) proposes remote-profile
  transfers through the queue where available and Plasma Opus-owned job
  notifications; neither promises to route paste or drag-and-drop through the
  queue.
  - Comment: [#375.7025250130][comment-375]

## Custom commands

Custom commands have these limits
([#373](https://github.com/mikepadiernos/plasma-opus/issues/373)):

- **Inputs:** Commands require a local working folder and local selections.
  Placeholders occupy whole arguments; a single empty literal argument and line
  breaks in templates or defaults cannot be represented in the editor. Preset
  defaults are stored as plaintext.
- **Execution:** Commands launch directly without an implicit shell, but chosen
  programs may interpret arguments as code. They run with the user's permissions
  and are not registered with KIO undo.
- **Output and process lifetime:** Displayed output is limited to 256 KiB. Stop
  terminates the launched process but may leave descendants running; closing the
  application can terminate the child.

- **Recommendation:** Copy remote inputs locally. Review the executable and
  every argument, run only trusted programs, and avoid passing untrusted paths
  to code-evaluating arguments or saving credentials in presets. Redirect large
  output to a file if needed, and inspect or stop remaining descendants after
  cancellation. Back up files before destructive commands and use external tools
  for unsupported argument templates or undo.
  - Comment: [#373.7024944292][comment-373]
- **Alternative:** Use a reviewed script instead of a complex inline template.
  - Comment:
    [#373.7029676403](https://github.com/mikepadiernos/plasma-opus/issues/373#issuecomment-7029676403)

## Symbolic links

Symbolic-link creation has these limits ([#312][issue-312]):

- **Targets:** Only existing local files and folders are supported.
- **Destination:** The new link is created beside its target; another
  destination cannot be chosen.

- **Recommendation:** Use a suitable external tool when a different link
  destination or remote symbolic-link support is needed. For local paths, GNU
  `ln -s` can create a link at a chosen destination; remote support depends on
  the server and access method.
  - Comment: [#312.7021756400][comment-312]
- **Alternative:** Use an SFTP client with explicit symbolic-link support.
  - Comment:
    [#312.7029672139](https://github.com/mikepadiernos/plasma-opus/issues/312#issuecomment-7029672139)

## Listing export

Listing export has these limits
([#314](https://github.com/mikepadiernos/plasma-opus/issues/314)):

- **Rows and values:** Export uses the current filtered rows and displayed
  values, even in icon or compact view.
- **Columns:** By default it uses the visible details columns; export-specific
  columns can be selected in Preferences, but are limited to the supported
  listing fields.
- **Output:** Output is local only, and wide PDF tables may wrap or clip.
- **CSV formulas:** CSV prefixes cells that could be interpreted as
  spreadsheet formulas with an apostrophe.

- **Recommendation:** Check the filter and selected export columns before
  exporting, and reduce columns for wide PDF tables. Keep CSV formula protection
  when opening untrusted data in a spreadsheet; copy local output elsewhere if
  needed. GNU `find -print0` can produce a local, NUL-delimited filename list
  for compatible tools, but does not reproduce Plasma Opus filters, columns or
  CSV output.
  - Comment: [#314.7021757620][comment-314]
- **Alternative:** Produce a custom-field export with a separate script.
  - Comment:
    [#314.7029672795](https://github.com/mikepadiernos/plasma-opus/issues/314#issuecomment-7029672795)

## Checksum files

Checksum files have these limits
([#316](https://github.com/mikepadiernos/plasma-opus/issues/316)):

- **Format and inputs:** Checksum files use UTF-8 GNU-style SHA-256 manifests
  and local regular files in the same folder. Other algorithms, remote files,
  symbolic links, path entries and non-UTF-8 names are unsupported.
- **Size:** Manifests are limited to 10,000 entries.
- **Races:** Files can change during hashing; cancellation near completion can
  race with publication of the checksum file.

- **Recommendation:** Use local regular files in the same folder and keep
  manifests within 10,000 entries. Avoid changing files while hashing, and check
  whether an output file was published after cancellation. Use a trusted
  external checksum tool for unsupported algorithms or inputs. GNU coreutils
  provides `sha256sum` and other checksum commands for local files.
  - Comment: [#316.7021845379][comment-316]
- **Alternative:** Use a separate checksum tool for other algorithms.
  - Comment:
    [#316.7029673498](https://github.com/mikepadiernos/plasma-opus/issues/316#issuecomment-7029673498)

## Split and combine files

Split and combine supports local regular files and has these limits
([#318](https://github.com/mikepadiernos/plasma-opus/issues/318)):

- **Parts:** Parts are raw numbered files; combining uses the displayed order
  and cannot detect missing, altered or incorrectly ordered parts.
- **Interruptions:** Inputs can change during processing. Failures or
  cancellation can leave incomplete output, and many small parts can be
  expensive.
- **Output:** Existing outputs are never overwritten and new files are
  owner-only.

- **Recommendation:** Check the complete part list and order before combining,
  keep originals and verify the reconstructed file with a checksum. Avoid
  changing inputs while processing. Inspect and remove unwanted partial outputs
  after errors or cancellation, choose a sensible part size, and adjust output
  permissions if sharing is needed. GNU `split` and `cat` are external
  alternatives; verify part order and handle output safely to avoid overwriting
  existing files. GNU `sha256sum` can verify reconstructed files against a
  trusted checksum.
  - Comment: [#318.7022199214][comment-318]
- **Alternative:** Create separate recovery information with PAR2.
  - Comment:
    [#318.7029674177](https://github.com/mikepadiernos/plasma-opus/issues/318#issuecomment-7029674177)

## Native tagging

The M23 [native tagging](tagging.md) libraries have these limits
([#714](https://github.com/mikepadiernos/plasma-opus/issues/714)):

- **Libraries and indexing:** Libraries are local folders only. Symbolic links
  aren't indexed or followed, and names that aren't valid in the system
  encoding are skipped. A library is a single JSON document that is read and
  rewritten whole on each edit, so very large libraries make edits and
  searches slower. Saves detect changes from other windows and retry, but
  there is no file lock, so two processes saving at the same moment can still
  lose one edit.
- **Moves and renames:** Only moves and renames started in Plasma Opus are
  followed. Moves by other programs, copies and KIO undo aren't, and the next
  refresh marks the old paths missing. A folder moved into a library is
  indexed at its top level only until the next refresh. Tags aren't copied
  between libraries. When the rename dialog reports an error, the renames it
  completed aren't applied to the library. Moving or renaming a library's root
  folder leaves its registration pointing at the old path.
- **Showing tags:** Library tags appear in the tag panel and the Information
  panel only. Views have no Tags column or emblem; the cards view's tags field
  shows Baloo file tags, not library tags. The gallery window can't assign
  tags. The tag panel doesn't notice changes made by other windows or
  programs until the selection or folder changes.
- **Tag panel:** Creating a tag from the search field and assigning it are
  separate saves, so if the assignment fails the new tag is kept unassigned.
- **Tag groups:** Groups are reordered with Move Up and Move Down only, not by
  drag and drop. Groups are library data, independent of workspaces and
  locations, as recommended in
  [#252](https://github.com/mikepadiernos/plasma-opus/issues/252).
- **Fields:** Library fields can't be searched with query criteria. A field's
  type can't change while any item has a value for it.
- **Searching:** Tag criteria match only items inside registered libraries;
  `tagged:no` matches items outside them only when
  [tagging settings](#tagging-settings) allow it. A search uses a snapshot of
  each library taken when it starts. Indexed search still needs a name or
  content term. Select by Query can't use `content:`, and its `meta:` criteria
  read every listed file on the interface thread, so large folders can make
  the window unresponsive until it finishes.
- **Embedded metadata:** Reading uses KDE's extractors, so the available
  properties depend on the installed extractor plugins. `meta:` criteria read
  each candidate file and are slow without cheaper criteria. Writing embedded
  metadata is limited to single JPEG, TIFF and MP3 files through
  [metadata editing](#metadata-editing). The import dialog's field list isn't
  refreshed if fields are changed elsewhere while it's open.
- **Export and restore:** Restoring replaces the whole library document; it
  doesn't merge. Items are matched by path only. An export reflects the
  library as loaded, including repairs made when it was read. Backups made
  before the backup limit was added aren't pruned. Exports have no size limit.

- **Recommendation:**
  - **Libraries and indexing:** Keep libraries on local folders, and rename
    files with unusual encodings before indexing them. Split very large
    collections into several libraries, and avoid editing the same library
    from several windows or programs at once.
  - **Moves and renames:** Move and rename tagged items in Plasma Opus. After
    moving items with other programs, undoing a move or moving a folder into a
    library, use **Tags › Refresh Library** and **Review Missing Entries…** to
    relink. To move a library's root folder, remove the library, move the
    folder, then create the library again from the new location; its document
    is adopted with its tags.
  - **Showing tags:** Select items to see their tags in the tag panel or the
    Information panel. Change the selection to refresh the tag panel after
    editing in another window.
  - **Tag panel:** If creating and assigning a tag reports an error, check the
    new tag again to assign it.
  - **Fields:** To find items by a field, import or copy the value as a tag. To
    change a field's type, create a new field and copy the values to it.
  - **Searching:** Turn on **Include items outside libraries in tagged:no
    searches** in **Preferences › Tagging › Search**, or use `NOT tagged:yes`,
    to include items outside libraries, and repeat a search after editing
    tags. Use Search rather than Select by Query for content, and combine
    `meta:` with name or type criteria, as in `type:image/* meta:canon`.
  - **Embedded metadata:** Install KDE's metadata extractors, and ExifTool for
    writing images. Use a dedicated tag editor for other formats or several
    files. Reopen the import dialog after changing fields.
  - **Export and restore:** Export before restoring and review the preview.
    Set **Keep at most … backups per library** in **Preferences › Tagging ›
    Privacy**, and remove older backups without the library's hash in their
    name from the backup folder by hand.
  - Comment:
    [#714.7171889508](https://github.com/mikepadiernos/plasma-opus/issues/714#issuecomment-7171889508)
- **Proposed improvements:**
  - Show library tags in a Tags column or emblem, and allow tagging in the
    gallery.
  - Refresh the tag panel when the library document changes.
  - Index the contents of folders moved into a library, and follow renames of
    the library root.
  - Search library fields with query criteria.
  - Merge restores.

## Tagging settings

The M24 [tagging settings](tagging.md#settings) have these limits
([#716](https://github.com/mikepadiernos/plasma-opus/issues/716)):

- **Private storage:** A private library document is keyed by the library
  folder's path. Moving or renaming the folder leaves the document behind,
  and the library must be created again and restored.
- **Backup folder:** The setting accepts a URL, but only local folders are
  used. A remote location silently falls back to the default
  `library-backups` folder.
- **Preferences:** The Libraries and Fields tabs read every library document
  on the interface thread, so many or very large libraries make Preferences
  slower to open. **Add Library…** uses the storage choice and new-library
  fields as currently shown, even before Apply. The Fields tab's library
  picker doesn't show whether a library is read-only or unreadable.
- **Applying settings:** Apply updates the window Preferences was opened
  from. Other windows move their tag panel when they are opened again, and
  show colour and Information panel changes when those panels next refresh.
  Automatic refresh reports its results in the active window only.
- **Library scope:** Exclusions, hidden items, depth and item limits apply to
  refreshes only; entries already indexed keep their tags and fields.
- **Search:** With the whole-library scope, path filters and the depth limit
  count from the library folder, and the notice that the search widened
  appears only when it finishes. Searches already running keep the settings
  they started with. When `tagged:no` matches items outside libraries, a
  search outside any library matches everything it lists.
- **Fields:** New libraries get their own field identifiers, so fields with
  the same name are distinct between libraries. Invalid stored field
  templates are dropped silently.
- **Embedded metadata:** Disabled properties are hidden in the preview panel
  and gallery but still extracted there. Mappings are stored by field name
  and apply only in libraries with a field of that name and a compatible
  type. When Preferences and the import dialog are both open, whichever saves
  mappings last wins. Importing on refresh has no preview, covers only items
  that refresh added and reads every new file, so the first refresh of a
  large library is slow.

- **Recommendation:**
  - **Private storage:** To move a privately stored library's folder, remove
    the library, move the folder, create the library again at the new path
    and use **Tags › Restore Library Metadata…** with an export or the old
    private document. Alternatively, switch the library to **Keep Metadata in
    Library Folder** before moving it.
  - **Backup folder:** Choose a local folder for backups.
  - **Preferences:** Keep the number of registered libraries modest, and split
    very large collections. Apply the Privacy and Fields tabs before adding
    libraries if you want the choices saved.
  - **Applying settings:** Reopen other windows, or change their selection,
    after changing tagging settings. Keep the window you care about active
    while an automatic refresh runs, or refresh manually with **Tags ›
    Refresh Library**.
  - **Library scope:** Remove tags and field values from items before
    excluding them if you no longer want that metadata.
  - **Search:** With the whole-library scope, write path filters relative to
    the library folder. Start a new search after changing search settings.
    Leave `tagged:no` outside libraries off unless you search limited
    folders, or combine it with name or type criteria.
  - **Fields:** Set up fields for new libraries before creating them, and use
    the same names across libraries so saved mappings apply.
  - **Embedded metadata:** Disable unwanted properties to keep search fast.
    Create the mapped fields in each library, with a date type for date
    properties. Close the import dialog before changing mappings in
    Preferences. Leave importing on refresh off for the first refresh of a
    large library and run **Import Embedded Metadata…** with a preview
    instead.
  - Comment:
    [#716.7175386572](https://github.com/mikepadiernos/plasma-opus/issues/716#issuecomment-7175386572)
- **Proposed improvements:**
  - Key private documents by a stable library identifier and follow folder
    moves.
  - Read library state in Preferences in the background.
  - Apply tagging settings to every open window.
  - Offer to remove entries that new exclusions or limits leave out.
  - Share field identifiers between libraries, or map by identifier.

## Filtered folder comparison

Filtered folder comparison has these limits
([#376](https://github.com/mikepadiernos/plasma-opus/issues/376)):

- **Scope:** The built-in comparison requires two separate local folders; with
  a folder on an SSH host, the [rsync backend](#desktop-integration) compares
  instead. Patterns match
  relative paths case-sensitively; excluded folders are not traversed. Links can
  be skipped or compared by target, but are never followed. When links are
  skipped, the matching subtree on the other side is also skipped.
- **Results:** Comparison examines at most 100,000 entries per folder, and
  filenames must round-trip through Qt's local filename encoding. Without
  content comparison, matching size and modification time do not prove equal
  contents. Results are snapshots that concurrent changes can invalidate, and
  can produce a large list.

- **Recommendation:** Use separate local folders and choose relative-path
  patterns carefully; enable content comparison when matching bytes matter.
  Compare smaller trees if the entry limit is reached, and rerun after changes.
  Choose rsync for folders on SSH hosts, and use an external tool for other
  remote locations or unsupported filenames.
  - Comment: [#376.7026216329][comment-376]
- **Alternative:** Use a separate graphical directory comparison.
  - Comment:
    [#376.7029677475](https://github.com/mikepadiernos/plasma-opus/issues/376#issuecomment-7029677475)

## Folder synchronization

Folder synchronization has these limits
([#377](https://github.com/mikepadiernos/plasma-opus/issues/377)):

- **Supported operations:** The built-in backend is local-only; the optional
  [rsync backend](#desktop-integration) also reaches folders on SSH hosts, with
  its own limits. Synchronization copies, overwrites
  and permanently deletes regular files and creates missing folders. Links,
  directory deletion and type replacement are unsupported. Profiles save filters
  and comparison options, not item actions.
- **Metadata and filesystem:** Copies preserve basic permissions and timestamps,
  but not necessarily ownership, ACLs, extended attributes, sparse layout or
  other metadata. Linux `renameat2` with `RENAME_NOREPLACE` support is required.
- **Recovery and races:** Operations are not registered with KIO undo or
  transactional. Errors or cancellation can leave partial results, and the
  current operation may already have changed files. Private `.opus-sync-*`
  staging folders or backups may remain after errors or crashes. Snapshot and
  identity checks reduce but cannot eliminate races with other processes
  changing or moving items.

- **Recommendation:** Back up important data first, stop other writers and avoid
  moving the folders during synchronization. Review every planned overwrite and
  permanent deletion, inspect the completed-operation report and any reported
  `.opus-sync-*` folders after errors, and compare again before retrying.
  Choose rsync for folders on SSH hosts, and use another tool for links,
  directory deletion, type replacement or metadata that must be preserved.
  - Comment: [#377.7026633904][comment-377]
- **Alternative:** Run rsync yourself with a reviewed dry run for options
  Plasma Opus doesn't offer.
  - Comment:
    [#377.7029677824](https://github.com/mikepadiernos/plasma-opus/issues/377#issuecomment-7029677824)

## File comparison

File comparison has these limits ([#311][issue-311]):

- **Requirements:** Comparison requires the `diff` executable and two local
  regular files.
- **Output:** Unified output is decoded as UTF-8, and comparison stops if
  output exceeds 4 MiB.
- **Binary files:** Binary differences are reported without a visual binary
  diff.

- **Recommendation:** Copy remote files locally and use another comparison tool
  for non-UTF-8 text, large output or a visual binary diff. Difftastic can
  compare supported text files; `cmp` can check whether binary files differ, but
  does not provide a visual diff.
  - Comment: [#311.7021755780][comment-311]
- **Alternative:** Use a separate graphical comparison tool.
  - Comment:
    [#311.7029671783](https://github.com/mikepadiernos/plasma-opus/issues/311#issuecomment-7029671783)

## Folder content chart

The folder content chart has these limits
([#315](https://github.com/mikepadiernos/plasma-opus/issues/315)):

- **Breakdown:** The chart shows measured logical sizes by immediate entry,
  not kind or extension breakdowns or a largest-file drilldown.
- **Entries:** It includes hidden entries, skips top-level symbolic links and
  entries without a known size, and relies on KIO workers for recursive
  traversal.
- **Scans:** Scans can be cancelled but have no entry or time limit and can be
  costly; results are snapshots.

- **Recommendation:** Scan smaller folders when possible, cancel expensive scans
  and rescan when current results are needed. Use another tool for extension
  breakdowns, largest-file analysis or linked-directory sizes. `ncdu` can
  explore local directory usage; GNU `du -L` can follow directory symlinks,
  subject to cycles and repeated targets.
  - Comment: [#315.7021758275][comment-315]
- **Alternative:** Inspect local disk usage with QDirStat.
  - Comment:
    [#315.7029673162](https://github.com/mikepadiernos/plasma-opus/issues/315#issuecomment-7029673162)

## Pane folder comparison

Pane folder comparison has these limits
([#317](https://github.com/mikepadiernos/plasma-opus/issues/317)):

- **Scope:** Comparison covers immediate entries only. Files are compared by
  size and modification time, not contents; subfolders are not compared.
- **Results:** Results are snapshots, and hidden or filtered entries may not
  be selectable.
- **Cost:** Large or remote listings may be costly.

- **Recommendation:** Use file comparison for individual files or filtered
  folder comparison for local recursive comparisons, enabling content comparison
  when needed. For other cases, use a trusted recursive tool; GNU `diff -r`
  compares local directory trees. Show hidden files and clear pane filters
  before selecting differences; compare again after changes.
  - Comment: [#317.7021904155][comment-317]
- **Alternative:** Inspect a folder-sync comparison without applying changes.
  - Comment:
    [#317.7029673828](https://github.com/mikepadiernos/plasma-opus/issues/317#issuecomment-7029673828)

## Cushion treemap

The treemap uses the folder content chart measurements: tiles represent
measured immediate entries and their relative logical sizes. It has these
limits ([#335](https://github.com/mikepadiernos/plasma-opus/issues/335)):

- **Nesting:** Nested folders are aggregated rather than shown simultaneously;
  activate a folder in the list to rescan it.
- **Visibility:** Empty, unmeasured and skipped entries have no visible tile,
  and very small shares may be difficult to see.
- **Measurements:** Measurements are snapshots and the
  [folder content chart](#folder-content-chart) scan limits also apply.

- **Recommendation:** Activate folders in the list to inspect their contents,
  and use the size table for empty, skipped or very small entries. Scan smaller
  folders and rescan when current results are needed. QDirStat is an external
  option for simultaneously visualizing nested disk usage.
  - Comment: [#335.7022647720][comment-335]
- **Alternative:** Use Filelight for a separate nested radial view.
  - Comment:
    [#335.7029675089](https://github.com/mikepadiernos/plasma-opus/issues/335#issuecomment-7029675089)

## Duplicate finder

The duplicate finder recursively scans local folders for known regular files,
groups equal sizes and SHA-256 hashes, and allows checked file URLs to be
copied for manual review. Candidates can be selected by modification time or
folder and moved to Trash after confirmation. It has these limits
([#336](https://github.com/mikepadiernos/plasma-opus/issues/336)):

- **Changes:** It rechecks affected groups before asking, but files can change
  afterward.
- **Entry types:** Symbolic links are skipped; hard links may appear as
  separate paths, and unknown entry types are skipped.
- **Scans and hashes:** Errors are shown, but scans are unbounded and matching
  hashes are not a guarantee of byte identity.

- **Recommendation:** Review groups and errors, verify current file contents and
  paths independently, and keep at least one confirmed copy and backups before
  moving files to Trash. Scan smaller local folders if a scan is expensive.
  `jdupes` is an external option for comparing local files; its default listing
  is non-destructive, but deletion and other modifying options require care.
  - Comment: [#336.7022793474][comment-336]
- **Alternative:** Compare candidate files byte-for-byte before deletion.
  - Comment:
    [#336.7029675367](https://github.com/mikepadiernos/plasma-opus/issues/336#issuecomment-7029675367)

## Workspace layouts

Workspace layouts, merged in M15, save pane and tab locations, active tabs,
view and location-bar modes, toolbar action lists, splitter sizes, terminal
orientation, detached-pane geometry, and supported panel placement and
visibility. They have these limits
([#700](https://github.com/mikepadiernos/plasma-opus/issues/700)):

- **Drafts, sessions and jobs:** Layouts do not persist editor drafts or
  terminal sessions. Running jobs remain alive when switching in the current
  process but are not reconstructed from a saved layout.
- **Startup:** The selected workspace is marked current on startup, while its
  saved layout remains unapplied until the user switches to it.
- **Prerequisites:** A visible terminal requires an existing session; metadata
  and toolbar editors must be closed before saving or switching. Incompatible
  linked panes must be unlinked first.
- **Older snapshots:** Older snapshots may require replacement before they can
  be applied.

- **Recommendation:** Save editor drafts separately and start a terminal session
  before switching to a layout that shows it. Finish metadata and toolbar edits,
  and unlink panes when prompted. Replace older snapshots with **Save Current
  Layout**. Explicitly switch to apply a saved layout after startup.
  - Comment:
    [#700.7163113449](https://github.com/mikepadiernos/plasma-opus/issues/700#issuecomment-7163113449)

## Workspace settings

The M16 settings have these limits
([#609](https://github.com/mikepadiernos/plasma-opus/issues/609)):

- **Panels:** The Layouts panels option restores the saved window state as one
  unit, including main toolbar positions and dock placement. The toolbars
  option only controls toolbar entries.
- **Remote locations:** Recovery checks only that a remote location's KIO
  protocol is installed. It never contacts the server, so a known but
  unreachable remote location is not listed and fails when its tab loads.
- **Unsaved state:** The unsaved marker and the **When unsaved** switch
  confirmation compare the whole captured layout. This includes parts that
  the workspace's behaviors or the restore options would not restore.
- **Import conflicts:** You choose which profiles to import one by one, but
  the replace or keep choice applies to all selected conflicts.
- **Default behaviors:** These apply only to workspaces created in the
  workspace manager. Duplicated and imported workspaces keep their own.

- **Recommendation:**
  - **Panels:** To keep the current docks and toolbar positions while
    restoring toolbar entries, clear the panels option.
  - **Remote locations:** Connect to them before switching, then reload any
    remote tab that was unreachable.
  - **Unsaved state:** Use **Save Current Layout** after deliberate changes,
    or set the switch confirmation to **Never** or **Always** if it asks too
    often.
  - **Import conflicts:** Import conflicting profiles in separate passes when
    some should replace existing workspaces and others should be kept.
  - **Behaviors:** Review the behaviors of duplicated and imported workspaces
    in the workspace manager.
  - Comment:
    [#609.7144292691](https://github.com/mikepadiernos/plasma-opus/issues/609#issuecomment-7144292691)
- **Proposed improvements:**
  - Split window state so that toolbar positions follow the toolbars option.
  - Optionally probe remote locations with a bounded, cancellable KIO stat.
  - Compare only the restorable parts of a layout when marking unsaved state.
  - Choose to replace or keep each conflicting profile separately.

## Linked pane navigation

Linked navigation supports existing local folder trees rooted at the two
starting folders and has these limits
([#322](https://github.com/mikepadiernos/plasma-opus/issues/322)):

- **Unlinking:** Panes unlink when navigation leaves either tree, a matching
  folder is missing, a symbolic link resolves outside its tree, or a pane is
  hidden. Tab and history changes also participate and may unlink.
- **Races:** Filesystem changes can invalidate folders between checks and
  navigation.

- **Recommendation:** Start from corresponding local folders with matching
  subfolder names. Check both panes after navigating, especially when
  directories may change; relink at a new pair of roots when needed. Navigate
  remote folders independently.
  - Comment: [#322.7022490696][comment-322]
- **Alternative:** Navigate the two locations independently.
  - Comment:
    [#322.7029674758](https://github.com/mikepadiernos/plasma-opus/issues/322#issuecomment-7029674758)

## Themes

The M17 theme options have these limits
([#680](https://github.com/mikepadiernos/plasma-opus/issues/680)):

- **Blur and translucency:** Background blur uses KWin's blur effect through
  `KWindowEffects`. Without KWin compositing it is reported as unavailable and
  the background stays opaque. Blur and translucency have been tested only
  headlessly, without a Plasma session.
- **Stylesheets:** A custom Qt stylesheet is limited to 64 KiB and 2,000 rules.
  `@import`, `qproperty-` declarations and `url()` references other than Qt
  resources (`:/` or `qrc:/`) are refused, so local and remote images cannot
  be used. Qt's parser is lenient, so some unsupported selectors or properties
  are ignored without an error. The stylesheet is applied to each main window
  and the dialogs it owns.
- **Theme profiles:** Invalid saved profiles are skipped when loading, with a
  count but without naming them. Profiles leave out folder colour rules and the
  high-contrast safeguard. Unavailable colour schemes and fonts in a profile
  fall back to Plasma's until they are installed.
- **Settings dialog and workspaces:** The settings dialog always edits the
  global appearance. While the active workspace overrides an option, the dialog
  shows the global value without a notice, and its changes appear only where
  the workspace does not override them.
- **Other windows:** Changes to a theme profile attached to a workspace reach
  other open windows only when they next apply settings or switch workspaces.
- **View mode:** A workspace's view mode override sets the panes' view mode
  when switching to the workspace, not at startup, where restored panes keep
  their saved view modes.
- **Folder colour rules:** A workspace keeps its own copy of folder colour
  rules, taken from the global rules rather than edited per workspace.
- **App-wide:** The active workspace is shared by all windows, so its
  appearance applies to every window.

- **Recommendation:**
  - **Blur and translucency:** Enable compositing and KWin's blur effect in
    System Settings. If blur is unavailable, lower opacity only as far as text
    stays readable, or keep backgrounds opaque.
  - **Stylesheets:** Keep stylesheets small and appearance-only, using colours,
    borders and Qt resource images. Check the preview before applying, and
    export the stylesheet before large edits so that you can revert.
  - **Theme profiles:** Export profiles you rely on as backups and re-import
    them if the profile count drops. Keep folder colour rules in the global
    settings or attach them to a workspace.
  - **Settings dialog and workspaces:** Change overridden options in
    **Workspaces… › Workspace Appearance**, or clear the override there first.
    Start with `--no-workspace-appearance` to see only the global appearance.
  - **Other windows:** After editing an attached profile, apply settings or
    switch workspaces in the other windows.
  - **View mode:** Switch to the workspace again, or choose the view mode for
    each pane.
  - **Folder colour rules:** Edit the global rules, then choose **Copy of the
    global rules** in the workspace's appearance and save it.
  - **App-wide:** Use separate workspaces for different appearances; M27 can
    apply separate choices per Activity.
  - Comment:
    [#680.7148716042](https://github.com/mikepadiernos/plasma-opus/issues/680#issuecomment-7148716042)
- **Proposed improvements:**
  - Mark options that the active workspace overrides in the settings dialog.
  - Notify other windows when theme profiles or workspace appearance change.
  - Name skipped theme profiles and offer to export or remove them.
  - Edit folder colour rules per workspace.

## Theme settings

The M18 theme settings have these limits
([#687](https://github.com/mikepadiernos/plasma-opus/issues/687)):

- **Corner radius:** A corner radius other than **Follow widget style** rounds
  item highlights, cards, text fields, tabs and tooltips. Buttons, combo boxes
  and panel frames keep the widget style's corners.
- **Plasma accent:** Colours that follow the Plasma accent are updated live
  only when Plasma announces a change to `kdeglobals`. An accent change written
  without a change notification appears after the next settings change or
  restart.
- **Workspace preview:**
  - Previewing a workspace's appearance doesn't switch the view mode of open
    panes.
  - Applying settings while a preview is shown replaces the preview.
  - The preview is shown in the window that opened the workspace manager,
    although the workspace appearance layer is shared by all windows.
- **All options:** When a workspace saves all appearance options, options that
  have no row in the workspace appearance editor take the workspace's saved
  values, or the current global values when they are first saved. They can't
  be edited individually there.
- **Stylesheet:** A workspace's own options never include the custom
  stylesheet. Only a theme profile attached to the workspace can supply one.
- **Blur and translucency:** These are reported per workspace only when the
  workspace saves those options; otherwise the global values apply.
- **Plasma session:** The theme settings have been tested only headlessly,
  without a Plasma session.

- **Recommendation:**
  - **Corner radius:** Choose the radius closest to the widget style's own
    corners so that rounded and unrounded controls look consistent.
  - **Plasma accent:** If the accent changes without updating the window,
    apply settings again or restart Plasma Opus.
  - **Workspace preview:** Preview before applying other settings, then choose
    **Revert** or save. To check view modes, switch to the workspace after
    saving it.
  - **All options:** Set those options in the settings dialog first, then save
    the workspace with **All options**, or use **Selected options** to save only
    the options you edit.
  - **Stylesheet:** Save a theme profile with the stylesheet and attach it to
    the workspaces that need it.
  - Comment:
    [#687.7149624993](https://github.com/mikepadiernos/plasma-opus/issues/687#issuecomment-7149624993)
- **Proposed improvements:**
  - Round buttons, combo boxes and panel frames with the chosen radius.
  - Preview in every window and keep the preview after applying settings.
  - Show every saved option in the workspace appearance editor.
  - Allow a per-workspace stylesheet.

## Branding

The M19 branding and its M20 settings have these limits
([#689](https://github.com/mikepadiernos/plasma-opus/issues/689)):

- **High contrast:** The startup presentation uses the colour scheme's window
  and text colours with a solid border, but the folder and the word "Opus"
  keep the accent colour, which can contrast less with the card than the text
  does. The version and description are always drawn in the text colour.
- **Screen readers:** The presentation has an accessible name (name and
  version) and description, but it is not announced when it appears, so it
  doesn't interrupt reading.
- **Duration:** The presentation stays for two seconds; the duration can't be
  changed.
- **Icon themes and other desktops:** The small and symbolic icons install to
  `hicolor`, so an icon theme that ships its own `org.kde.opusfilemanager` icon
  takes precedence. The folder follows the Plasma accent and the symbolic icon
  follows the text colour only where KDE's icon loader applies the colour
  scheme; elsewhere they use their built-in Breeze colours.
- **Release notes:** The v0.1.0 and v0.2.0 release notes predate the
  [release notes template](releases/TEMPLATE.md) and were not rewritten.
- **Plasma session:** The presentation, its settings preview and the icons
  have been tested only headlessly, without a Plasma session.

- **Recommendation:**
  - **High contrast:** Read the version and description, which use the text
    colour. If the accent contrasts poorly with the card, choose a stronger
    accent in Plasma's System Settings, or leave the startup presentation off
    (the default) in **Preferences › Branding › Presentation**.
  - **Screen readers:** Use **Help › About Plasma Opus** for the name, version
    and description.
  - **Duration:** Press any key, click or scroll to dismiss the presentation
    early. Input still reaches the window. If it distracts, turn it off in
    **Preferences › Branding › Presentation**.
  - **Icon themes and other desktops:** To see the Plasma Opus icon under a
    theme that overrides it, remove the theme's `org.kde.opusfilemanager` icon
    or choose another icon theme. Outside Plasma, expect the Breeze blue folder.
  - **Release notes:** Start new releases from the template.
  - **Plasma session:** Check the presentation's appearance on Plasma,
    including fractional scaling and Wayland, before relying on it in
    screenshots or release materials.
  - Comment:
    [#689.7150293713](https://github.com/mikepadiernos/plasma-opus/issues/689#issuecomment-7150293713)
- **Proposed improvements:**
  - Draw the folder and "Opus" in the text colour under high contrast.
  - Offer an option to announce the presentation to screen readers.
  - Make the duration configurable.

## Native title and header padding

Native Qt/KDE dock titles and column headers follow the active widget style
([#560](https://github.com/mikepadiernos/plasma-opus/issues/560)):

- **Padding:** Their horizontal padding can be less than eight logical
  pixels. For example, Breeze's column-label rectangle starts six pixels from
  the section edge.

- **Recommendation:** Keep native rendering and choose a widget style whose
  spacing is comfortable for your needs. Check the active style after changing
  desktop appearance; Plasma Opus does not guarantee an eight-pixel minimum for
  these controls.
  - Comment:
    [#560.7071231166](https://github.com/mikepadiernos/plasma-opus/issues/560#issuecomment-7071231166)

## Jalali dates

With a Persian regional locale or an explicit Jalali calendar override, Plasma
Opus's date displays use the Jalali calendar. This has these limits
([#436](https://github.com/mikepadiernos/plasma-opus/issues/436)):

- **Listing format:** With the automatic calendar, the ISO 8601 listing format
  remains Gregorian; an explicit calendar override takes precedence.
- **KDE-provided dates:** KDE controls dates in its own dialogs and tooltips,
  and the additional file information shown under icons in icon and compact
  views uses KDE's `KFileItemDelegate`; these dates may remain Gregorian.

- **Recommendation:** Use Details view or the Plasma Opus Information panel for
  Jalali dates. Select Jalali explicitly, or use a Persian regional locale with
  the automatic calendar and a Locale listing format. Verify the calendar before
  interpreting KDE-provided dates.
  - Comment:
    [#436.7041290164](https://github.com/mikepadiernos/plasma-opus/issues/436#issuecomment-7041290164)

## Translation format overrides

The regional locale and custom digit shapes do not control every display
([#440](https://github.com/mikepadiernos/plasma-opus/issues/440)):

- **Digit shapes:** Plasma Opus applies digit shapes to its file dates,
  listing sizes, preview line numbers and selected gallery and chart fields.
- **Other displays:** Other text and numbers, editable Qt controls,
  KDE-provided details and dialogs, and separately embedded applications can
  use their own locale or desktop settings.
- **C/POSIX message locale:** An explicitly selected non-English interface
  language also requires an installed non-C UTF-8 message locale; Plasma Opus
  warns and follows the system language if none can be used.

- **Recommendation:** Use Details view or the Information panel for overridden
  file dates and verify other displays against the desktop locale. Configure
  desktop regional settings for consistency elsewhere. If the selected language
  is unavailable under C/POSIX, install a non-C UTF-8 system locale and restart
  Plasma Opus.
  - Comment:
    [#440.7045227580](https://github.com/mikepadiernos/plasma-opus/issues/440#issuecomment-7045227580)

## Desktop integration

The M27 desktop integrations have these limits
([#728](https://github.com/mikepadiernos/plasma-opus/issues/728)):

- **🔗 FileManager1 service:** Plasma Opus claims
  `org.freedesktop.FileManager1` only when it starts while it is the default
  folder handler, or when it becomes the default under **Preferences › KDE
  Plasma › File Manager**, and never replaces another owner, such as a
  running Dolphin. Changing the default handler in System Settings while it
  runs takes effect at the next start. The per-user activation file stays
  when another default is chosen in System Settings, so requests made while
  no file manager runs still start Plasma Opus until the file is removed
  under **Preferences › KDE Plasma › File Manager**. Requests open tabs in
  the active pane of the existing window; missing items are skipped and
  reported, and flat view opens the folder without a selection.
  ([#279](https://github.com/mikepadiernos/plasma-opus/issues/279))
- **🌐 Language preferences:** Plasma Opus reads Plasma's language list
  only at start. Changes in Plasma's Region & Language settings reach running
  applications through `LANGUAGE` only after logging in again, and Plasma Opus
  falls back to `plasma-localerc` only when `LANGUAGE` is unset. Listed
  languages without a Plasma Opus catalogue are skipped; when none has one,
  the original English text is shown.
  ([#437](https://github.com/mikepadiernos/plasma-opus/issues/437))
- **🔔 Notifications and jobs:** only the checksum, folder synchronization,
  duplicate finder, file split, batch rename, batch image conversion and
  tag-library refresh dialogs report to Plasma's job progress. Automatic
  library refreshes, previews, gallery loads, searches and archive tools do
  not. Plasma's job entry cannot bring the dialog back to the front; it offers
  only its own actions, such as opening the destination folder. Outside a
  Plasma session, with no job-view service running, nothing is reported and
  the dialogs remain the only progress display. Choose which of these
  operations report under **Preferences › KDE Plasma › Notifications**
  ([Integration settings](#integration-settings)).
  ([#388](https://github.com/mikepadiernos/plasma-opus/issues/388))
- **📌 System tray and global shortcut:** the tray icon is on by default;
  minimizing and closing to the tray are off until turned on under
  **Preferences › KDE Plasma › System Tray**. On Wayland, KWin does not tell
  applications when they are minimized, so minimizing to the tray works only
  on X11. KWin's focus-stealing prevention may keep a window shown from the
  tray or the global shortcut behind the active window and highlight it in
  the task manager instead. When another application already uses
  Meta+Alt+O, the shortcut stays unassigned. A window hidden in the tray
  keeps its tabs and running operations; for starting hidden and the
  shortcut settings, see [Integration settings](#integration-settings).
  ([#285](https://github.com/mikepadiernos/plasma-opus/issues/285))
- **🧭 Plasma Activities:** associations are off by default and are only
  offered, in a message above the panes, when the current Activity changes;
  the Activity that is current when Plasma Opus starts is not offered. An
  association whose workspace is already active offers only its default
  folders, and its own behaviours apply only when its workspace is switched
  to. Behaviours chosen by an association last until another workspace is
  chosen, and they are not saved with the session. Default folders always
  open in new tabs, so tabs accumulate with each switch. Only local folders
  are checked before they are offered; a missing remote folder fails when
  its tab opens. Associations are kept for removed Activities and missing
  workspaces until they are cleared under **Preferences › KDE Plasma ›
  Activities**. Without the Activities service, nothing is offered and
  removed Activities cannot be told apart from existing ones.
  ([#387](https://github.com/mikepadiernos/plasma-opus/issues/387))
- **📱 KDE Connect sharing:** **File › Send to Device…** sends only local
  regular files; folders, remote KIO locations and special files are listed
  as unsupported and skipped. A successful reply from KDE Connect means it
  accepted the files, not that they arrived: the transfer's progress, result
  and any failure appear only in KDE Connect's notifications, not in Plasma
  Opus. Device states are read when the dialog opens or KDE Connect reports a
  change, with short blocking D-Bus calls. The action is not in the file
  context menu, where KDE Connect's own **Send to** service menu may already
  appear.
  ([#389](https://github.com/mikepadiernos/plasma-opus/issues/389))
- **🔎 KRunner:** the plugin works only from an installed build: KRunner
  finds it through the installed plugin file and starts the helper through
  D-Bus activation, so a build run from its tree offers nothing. Only saved
  workspaces and Favorites are offered, matched by name; locations inside a
  workspace, history and other places are not. Remote Favorites are offered
  without checking that they can be reached, and fail when their tab opens.
  Choosing a workspace always asks before switching. The plugin is enabled
  by default once installed, and turning its results off under
  **Preferences › KDE Plasma › KRunner** leaves it loaded; only Plasma's
  search settings turn it off. The helper keeps running after KRunner first
  starts it, reads the settings again whenever a search session ends, and
  is not stopped when Plasma Opus quits. KRunner may need restarting before
  a newly installed plugin appears.
  ([#390](https://github.com/mikepadiernos/plasma-opus/issues/390))
- **⬇️ Service-menu downloads:** downloading reuses Dolphin's
  `servicemenu.knsrc` provider and its `servicemenuinstaller`, so it needs
  Dolphin installed, and what it installs, updates or removes is shared with
  Dolphin's own download window. Plasma Opus does not review downloaded
  service menus; they come from the KDE Store and can run any command.
  Installation errors are shown in the download window, and the tab names
  only the entries that failed. Service menus installed outside the download
  window, or by another application while the settings are open, appear only
  after the window is used again or the settings are reopened.
  ([#369](https://github.com/mikepadiernos/plasma-opus/issues/369))
- **🗂️ System file chooser:** setup is per user, through
  `--install-portal`, `./install.sh --file-chooser` or **Preferences › KDE
  Plasma › File Manager**. It writes only `kde-portals.conf`, so it applies
  in Plasma sessions and not in other desktops. xdg-desktop-portal reads
  per-user portal descriptions only from version 1.21.0; older versions
  ignore Plasma Opus and fall back to the next chooser in the preference,
  normally KDE's. Applications that open their own dialog without the portal
  keep it, including most native Qt and KDE applications. The chooser has no
  tabs, previews, side panels or indexed search, only a name filter over the
  current folder, which also lists matching files when choosing a folder.
  Only local files are accepted; remote locations can be browsed but not
  returned. A modal request keeps the dialog over its window, but the
  application's own window is not blocked. Setup and uninstalling restart
  xdg-desktop-portal, which closes file dialogs open in other applications.
  This has been verified in a headless container only, not yet on a Plasma
  session ([pending task](TODO.md#system-file-chooser-on-plasma-286)).
  ([#286](https://github.com/mikepadiernos/plasma-opus/issues/286))
- **💽 Mount manager:** the dialog checks for mounts made by other programs
  every three seconds while it is open, and sizes and free space are read
  only when it opens, on **Refresh** and after a mount. Eject works only for
  optical drives; USB drives can be unmounted but not powered off. File
  systems that Solid does not manage can be mounted only when their fstab
  entry allows users, with `user`, `users`, `owner` or `group`, and unmounted
  only when it allows users or they are FUSE mounts; `/` can never be
  unmounted. autofs and systemd automount points show as not mounted until
  they are first accessed. Pseudo, snap, container and portal file systems
  are hidden unless shown under **Preferences › KDE Plasma › Mount Manager**,
  and are then listed but never mounted or unmounted. Plasma's and UDisks2's
  authorization prompts have been verified in a headless container only, not
  yet on a Plasma session.
  ([#703](https://github.com/mikepadiernos/plasma-opus/issues/703))
- **💽 Disk images:** mounting needs the Mount ISO plugin from Dolphin's
  plugins, usually packaged as `dolphin-plugins`; without it the actions
  explain what is missing and do nothing. The plugin blocks the window while
  UDisks2 attaches and mounts the image, for up to about 20 seconds, and
  reports no errors, so the reason shown after a failure is inferred from
  what Solid reports afterwards and may be inaccurate. Mounting and
  unmounting may ask for authorization through polkit. A refused mount can
  leave the image attached as a loop device without a mount point; choosing
  **Mount Disk Image** again detaches it and retries. Solid hides loop
  devices whose image is outside the home folder, so those images do not
  appear in Places even when mounted. Only images Dolphin's plugin
  recognizes are offered, and only one at a time. This has been verified in
  a headless container with a simulated plugin only, not yet on a Plasma
  session.
  ([#648](https://github.com/mikepadiernos/plasma-opus/issues/648))
- **📦 Application packages:** packages are recognized by MIME type and by
  the standard installation folders, `~/.local/share/flatpak`,
  `/var/lib/flatpak` and `/snap`; custom Flatpak installations are not.
  A Flatpak bundle's application ID is not read, so its installed state and
  details are unknown until it is installed and opened from its folder.
  Sandbox permissions are shown only after installing, since Flatpak
  reports them only for installed applications; installing grants what the
  application requests. Local snap files are installed with `--dangerous`,
  without a Store signature check or automatic updates, and snaps that need
  classic confinement fail to install. Tools run without a terminal, so
  Flatpak and snapd may ask for authorization through polkit, but any other
  prompt fails the change, and progress is not shown beyond a busy
  indicator. **Add to Application Menu** runs a type 2 AppImage's runtime
  to extract its desktop file and icon; type 1 AppImages, and others whose
  files cannot be read, get their file name and a generic icon. Menu entries
  point to the AppImage's current path and break when it is moved or
  deleted. AppImage updates need AppImageUpdate's `appimageupdatetool`.
  AppImage, Flatpak and Snap actions, adding AppImages to the menu and
  reading their name and icon can be turned off under **Preferences ›
  Applications › Packages**; see
  [Package settings](#integration-settings). This has been verified in a
  headless container with simulated package tools only, not yet with real
  `flatpak`, `snap` or Discover on a Plasma session.
  ([#281](https://github.com/mikepadiernos/plasma-opus/issues/281))
- **🪟 KWin script:** the script has been tested only against a simulated
  KWin in the headless container, not yet on a Plasma session
  ([pending task](TODO.md#kwin-script-on-plasma-707-708)). Its use of KWin's
  tiling API, assigning a tile or `frameGeometry`, and its X11 and Wayland
  behaviour are unverified, and KWin's scripting API can change between
  Plasma releases. Requests reach the script through its **Plasma Opus:
  Process Window Requests** shortcut, which appears, unassigned, in KWin's
  shortcut list. Only the main window's placement is kept, one per workspace,
  without its maximized or full-screen state; a placement is kept only once
  the window has been moved, resized or retiled with that workspace current.
  Tiled companions are only Plasma Opus's own detached windows, such as the
  gallery and the popped-out second pane; terminals, editors and other
  applications it opens are separate processes and are not placed.
  Windows are matched by desktop file name and process id, so a sandbox that
  hides the real process id, such as Flatpak, prevents matching. Virtual
  desktops can be assigned only after the script has reported them. The
  script is installed and its window management chosen under **Preferences ›
  KDE Plasma › KWin**.
  ([#707](https://github.com/mikepadiernos/plasma-opus/issues/707))
- **⬇️ Drop-down window:** it is off by default, and its global shortcut
  is unassigned until one is chosen. Only the main window drops down, and
  while it is on, its placement is not kept for workspaces. Without the
  script it opens as a regular window, wherever KWin places it, and stays in
  the taskbar. Hiding on focus loss waits 200 ms and does not hide while a
  dialog, menu or another Plasma Opus window is active; closing the window
  still quits or hides it to the tray as usual instead of rolling it up. The
  slide animation needs KWin's **Sliding popups** effect. The screen and
  touch-screen edges are shared with KWin's own edge actions, and only one
  can claim an edge. It is turned on and configured under **Preferences › KDE
  Plasma › KWin**.
  ([#708](https://github.com/mikepadiernos/plasma-opus/issues/708))
- **🗂️ File-type categories:** Baloo classifies indexed files by their
  detected content, while the live fallback matches MIME types guessed from
  names and file extensions, so the two can list different files, such as a
  renamed file or an ebook saved without its extension. **Ebooks** lists
  only EPUB, Mobipocket, FictionBook, comic book archives and DjVu, and
  **Application Packages** only Flatpak, AppImage, Debian, RPM and Snap
  packages. **Large Files** is always a live search, since Baloo doesn't
  index file sizes. The entries are saved in Plasma's
  shared `user-places.xbel`, marked for Plasma Opus only, with their titles
  in the language in use when they were added. They also show in the KDE
  file dialogs Plasma Opus opens, where they can't be opened. KDE's older
  **Search For** entries, where a distribution still adds them, appear beside
  them. A removed entry isn't added again until its category is checked
  under **Preferences › KDE Plasma › Baloo**.
  ([#649](https://github.com/mikepadiernos/plasma-opus/issues/649))
- **💻 Terminal providers:** **Open Terminal With** only lists the
  default terminal, Konsole, Yakuake and Kwi, and finds them through `PATH`,
  so a terminal installed only as a Flatpak shows as not installed. It opens
  the active pane's folder, also from a selected folder's context menu.
  Yakuake has no option to start in a folder, so Plasma Opus adds a session
  and types `cd` into it; a shell that is slow to start or a profile that
  runs another program can miss or misread it, and the command can appear in
  shell history unless leading spaces are ignored. Plasma Opus can't tell
  whether Yakuake's window is shown, so it doesn't open or close it, and
  waits 15 seconds for a Yakuake it started. A running Kwi decides how a
  second request opens. **Open Terminal With** doesn't remember its choice;
  **Open Terminal Here** opens the terminal chosen under **Preferences ›
  Applications › Terminal**.
  ([#289](https://github.com/mikepadiernos/plasma-opus/issues/289))

- **📝 Editor integrations:** **Edit With** only lists KWrite, Kate, Neovim,
  Rune, micro and nano, and finds them through `PATH`, so an editor
  installed only as a Flatpak shows as not installed. Rune is looked up as
  `rune` and hasn't been tested. Neovim, Rune, micro and nano open in a new
  embedded terminal tab, so they need the Konsole or Kwi component, unless
  **Terminal editors run in** under **Preferences › Applications › Editor**
  chooses a separate terminal, and they only open local files; KWrite and
  Kate also open remote files through KIO. Folders can't be opened. The
  editor's tab starts in the first file's folder, but a terminal profile with
  its own start folder takes precedence, and the tab closes when the editor
  exits. The editors start with their own configuration, and the **Edit
  With** choice isn't remembered between uses.
  ([#319](https://github.com/mikepadiernos/plasma-opus/issues/319))

- **📄 Office documents:** Plasma Opus doesn't embed an office editor. The
  preview panel shows office documents read-only, and only when a document
  viewer component for the format, such as Calligra's, is installed; otherwise
  it shows the file's details. **Open in Office Application** lists only
  LibreOffice, Calligra and ONLYOFFICE, and finds them through `PATH`, so an
  application installed only as a Flatpak or AppImage shows as not
  installed. It covers OpenDocument and Microsoft Office documents,
  spreadsheets and presentations; RTF, CSV and OpenDocument drawings aren't
  included. LibreOffice and ONLYOFFICE only open local files, and Calligra
  needs a separate Words, Sheets or Stage program for each kind. The
  application handles saving and format conversion, and Plasma Opus doesn't
  track whether a document is open or has unsaved changes. The application
  that **Edit** uses and whether office documents are previewed are chosen
  under **Preferences › Applications › Office**
  ([#676](https://github.com/mikepadiernos/plasma-opus/issues/676)).
  ([#674](https://github.com/mikepadiernos/plasma-opus/issues/674))

- **✏️ Edit action:** **Edit** chooses a tool by file type rather than by
  the file associations in System Settings. Text files open in the editor
  chosen under **Preferences › Applications › Editor**, the built-in editor
  unless changed, and office documents in the application chosen under
  **Preferences › Applications › Office**, the first installed one unless
  changed; other types take the first installed tool in a fixed order, so
  their editor can't be chosen. It covers text, office documents,
  SVG drawings, raster images and audio; videos, PDF documents, archives and
  other types can be previewed but not edited. Separate tools are found
  through `PATH`, so Flatpak and AppImage installations show as not
  installed, and only the built-in editor, KWrite, Kate, KolourPaint and
  Calligra open remote files. It edits one file at a
  time. The chosen tool handles saving: some image and audio editors save to
  their own format or export, and an image editor may not support every
  format the preview shows. Plasma Opus doesn't track whether a separate
  tool has unsaved changes.
  ([#675](https://github.com/mikepadiernos/plasma-opus/issues/675))

- **📚 Ebook and font previews:** Ebooks, comics, DjVu documents and help
  files need Okular's viewer component, and the preview only offers what the
  component shows in a narrow panel, without its toolbars or reading
  position. Okular's support for each format depends on its installed
  backends. Fonts are previewed with Qt rather than Plasma's font viewer
  component, without a character map or detailed font information. A font
  whose family name matches an installed font may show the installed one,
  font collections show only their first face, and fonts over 64 MiB aren't
  previewed. Web fonts can be previewed but not installed, and
  **Install Font…** needs Plasma's `kfontinst` in `PATH`; the installer
  decides whether the font is installed for the user or system-wide.
  ([#650](https://github.com/mikepadiernos/plasma-opus/issues/650))

- **🔐 Remote connections:** A profile's private key is passed only to SSH terminals: KIO's SFTP worker
  uses the SSH agent and its own key lookup, so browsing may still ask for a
  password. SSH terminals need `ssh` in `PATH` and the Konsole component, and
  start in the remote home folder rather than the profile's folder.
  Connections aren't tested when saved, and errors appear only once a pane
  opens them. Removing a connection doesn't ask for confirmation and leaves
  any password saved in KWallet. Profiles don't appear in Places.
  ([#282](https://github.com/mikepadiernos/plasma-opus/issues/282))

- **🌍 Network shares and FTP:** KDE ships no FTPS worker, so FTPS profiles
  open only when a third-party worker is installed. Plain FTP profiles can't
  require encryption, and KIO's FTP worker doesn't upgrade them to TLS.
  **Go › Network** shows what KIO's `network:/` worker or Dolphin's `remote:/`
  folder discovers, which depends on Avahi and, for Windows shares, WS-Discovery
  being installed and allowed through the firewall. Plasma Opus doesn't add
  discovered services to saved connections or Places. SMB sign-in, including
  domains and Kerberos, follows KIO's SMB worker.
  ([#651](https://github.com/mikepadiernos/plasma-opus/issues/651))

- **☁️ rclone cloud storage:** Mounting a remote needs FUSE and `fusermount3`;
  without them, serve remotes over WebDAV instead, as described under [rclone
  settings](#integration-settings). Mounts last
  only while Plasma Opus runs, including hidden in the tray, so applications
  editing files on a mount lose access when it quits, and a busy mount may
  stay behind after rclone stops. A mount left by an earlier session opens
  but can't be unmounted from the menu. Remotes with an encrypted rclone
  configuration fail unless `RCLONE_CONFIG_PASS` is set in the environment
  Plasma Opus starts with. Mounting waits up to 20 seconds, and rclone's
  cache settings, other than caching writes, are its defaults. Mounts don't
  appear in Places, and the list refreshes each time the menu opens.
  ([#652](https://github.com/mikepadiernos/plasma-opus/issues/652))

- **💽 Remote drives:** KIO FUSE can't mount read-only or in a chosen folder,
  and can't unmount a single connection: **Unmount** only removes it from
  Plasma Opus and Places, and its folder stays available until KIO FUSE
  stops. sshfs needs `ksshaskpass` to ask for passwords and new host keys;
  without it, only the SSH agent and keys sign in, and unknown hosts are
  refused. davfs2 and CIFS mount only through a user-mountable fstab entry
  that an administrator adds, usually with the user in the `davfs2` group
  and credentials in the backend's own secrets file, and read-only comes
  from that entry. S3-compatible storage and KDE Online Accounts locations
  aren't offered; S3 works through **Cloud Storage** with an rclone remote.
  Mounting at login means starting Plasma Opus in the system tray at login,
  and mounting when a workspace is activated follows its assigned locations,
  as described under [Remote drive settings](#integration-settings). Changed
  mount options apply after unmounting. sshfs, davfs2 and CIFS mounts end
  when Plasma Opus quits unless those settings keep them, and a connection
  mounted outside Plasma Opus is taken over when it is mounted, so it can be
  unmounted from it too. Media streams only where the backend reads ranges,
  which sshfs, CIFS and KIO FUSE's SFTP and SMB workers do.
  ([#702](https://github.com/mikepadiernos/plasma-opus/issues/702))
- **📡 Temporary LAN sharing:** Anyone who sees a link can use it until
  sharing stops: there is no password, and a single link can't be revoked.
  Without HTTPS, others on the network can read links and files in transit.
  The HTTPS certificate is self-signed and made for each share, so browsers
  warn that it isn't trusted; HTTPS needs `openssl`, and QR codes need KDE's
  Prison library when Plasma Opus is built. Firewalls such as firewalld and
  ufw may block the port, which Plasma Opus doesn't open. WebDAV covers
  browsing, downloading and uploading files only: folders can't be created,
  renamed or deleted, nothing is locked, and uploads must declare their size,
  so clients that need locking, such as Windows' WebDAV redirector, may
  connect read-only or not at all. Uploads go only to the upload folder and
  its existing subfolders, and a leading dot is removed from their names.
  Special files aren't served. Each response closes its connection, so
  folders with many small files load more slowly, and at most 32 connections
  are served at once. Shares aren't announced on the network, the address
  list isn't refreshed when the network changes, and sharing ends when the
  dialog closes. The starting choices are under **Preferences › Remote and
  Cloud › LAN Sharing**, as described under [LAN sharing
  settings](#integration-settings).
  ([#653](https://github.com/mikepadiernos/plasma-opus/issues/653))
- **👤 KDE Online Accounts:** Only Google Drive, through kio-gdrive, and
  Nextcloud and ownCloud storage, through WebDAV, open as folders; other
  services, such as contacts and YouTube, and providers without a KDE file
  service aren't listed as folders. Plasma Opus doesn't pass account
  credentials to KIO, so WebDAV storage asks for the password the first
  time unless Plasma's Online Accounts integration has already given it to
  KDE's password server. Since M29, a Nextcloud account signed in under
  **Go › Nextcloud › Manage Nextcloud Accounts…** gives KDE's password
  server an app password instead
  ([Nextcloud integration](#nextcloud-integration)). Expired sign-ins and
  denied access appear as KIO
  errors in the pane, not in the menu. Accounts aren't shown in Places or
  offered as remote drives, and the menu reads accounts only when it opens,
  and only after opting in under **Preferences › KDE Plasma › Online
  Accounts**. The menu needs Plasma Opus built with libaccounts-qt6, and
  **Manage Online Accounts…** needs System Settings with its Online Accounts
  page.
  ([#370](https://github.com/mikepadiernos/plasma-opus/issues/370))
- **🔁 rsync backend:** Each run copies or deletes in one direction, so plans
  that work both ways are refused, and it deletes only regular files missing
  from the source; folders, links and type changes aren't synchronized. It
  works between a local folder and a folder on an SSH host that has rsync
  installed, not between two hosts or with other KIO locations. Every
  comparison, dry run and transfer opens new SSH connections, so passwords
  may be asked for several times, and without ksshaskpass a host that needs
  a password or an unknown host key fails. Remote comparisons use rsync's
  listings: times have one-second precision, symbolic links appear only when
  link targets are compared, and filter patterns with `?`, `\` or a leading
  or trailing `/` are refused. Copies keep modification times but not
  permissions, ownership, ACLs or extended attributes. The dry run is a
  snapshot, runs aren't added to KIO's undo history, and canceling or an error
  leaves the files already transferred changed. Whether rsync may delete files,
  its copy policy and the profile it loads are set under **Preferences › Remote
  and Cloud › Sync**, as described under [rsync
  settings](#integration-settings).
  ([#363](https://github.com/mikepadiernos/plasma-opus/issues/363))
- **🔄 Cloud sync:** Pairs synchronize only while Plasma Opus runs; **Synchronize
  in the background after login**, or the matching setting under **Preferences ›
  Remote and Cloud › Sync**, starts it in the system tray, as described under
  [Cloud sync settings](#integration-settings). Each run of a network-folder
  pair lists both folders in full and compares sizes and modification times, not
  contents, so an edit that keeps the size and time is missed, and a remote that
  changes modification times, or an interrupted upload, can cause conflicts on
  the next run. Renamed files are detected, but a renamed folder is copied anew
  and the old one deleted. **Keep Both** saves the remote version beside the
  local file, and the next run uploads that copy. A file and a folder with the
  same name are left for you to rename, and a remote subfolder that can't be
  read is skipped for that run. rclone pairs use `rclone bisync`: the first run
  and any filter change need `--resync`, which copies newer files both ways
  without deleting anything; the review shows a dry run, so changes made after
  it are synchronized too; and the report gives no counts. Excluding a nested
  pair whose folder name contains `*`, `?` or `[` fails for rclone pairs. Only
  Nextcloud, ownCloud, Syncthing and Dropbox folders are recognized as owned by
  another client. There are no desktop notifications: runs that need review or
  found conflicts are shown in the tray and the status bar. Pausing on metered
  connections relies on Qt's network information, usually from NetworkManager.
  ([#718](https://github.com/mikepadiernos/plasma-opus/issues/718))
- **📊 Cloud activity:** Speeds and remaining times are shown only for
  transfers in the transfer queue; rclone mounts and sync runs show their
  status, not their speed, and **Pause All** doesn't stop mounts from
  uploading or sync runs you start yourself. The bandwidth, parallel-transfer
  and cache limits apply only to rclone, not to pairs that synchronize network
  folders through KIO, and take effect at a pair's next run and a remote's
  next mount. Files waiting to upload are found in rclone's cache metadata in
  Plasma Opus's cache folder, so mounts started outside Plasma Opus aren't
  counted, and a change to rclone's metadata format would hide them. The
  offline cache keeps files already opened, but listing a folder or opening an
  uncached file still needs the remote. Unmounting, closing and quitting wait
  for uploads, but logging out waits only when the session lets applications
  ask; otherwise uploads stop and resume at the next mount. Space is shown
  only for mounted remotes whose provider reports it. Exported definitions
  contain no passwords, and imported sync pairs start paused. The limits,
  and how long cached files are kept, are also under **Preferences › Remote
  and Cloud › Transfers and Cache**, as described under [Transfer and cache
  settings](#integration-settings).
  ([#719](https://github.com/mikepadiernos/plasma-opus/issues/719))
- **🛡️ Administrator mode:** **Act as Administrator** needs the separate
  `kio-admin` package and is reported as missing without it. Items in the
  `admin:` view are not local files, so **Move to Trash**, terminals, custom
  commands, checksums, batch rename, symbolic links, metadata editing, split
  and combine and editing in the preview are unavailable there, and deleted
  items skip the trash. Copied or dragged items carry `admin:` URLs that only
  KIO-based applications open. How often the password is asked and how long
  an authorization lasts are decided by `kio-admin` and Polkit; stopping
  administrator mode does not revoke an authorization already granted.
  Entering it in a pane with linked navigation unlinks the panes. The
  container used for development has no `kio-admin` or Polkit agent, so the
  prompt and elevated operations await
  [verification on Plasma](TODO.md#administrator-mode-on-plasma-288).
  ([#288](https://github.com/mikepadiernos/plasma-opus/issues/288))
- **🤖 MCP server:** `opus-file-manager-mcp` speaks only the stdio
  transport, so a client must run on the same computer and start the server
  itself; there is no HTTP transport, so no access token is used. Access,
  folders and the audit log are set under **Preferences › Security › MCP**
  ([MCP settings](#integration-settings)). It reads only UTF-8 text, at most
  1 MiB per call, and refuses binary files and other encodings. Listings and
  searches return at most 1,000 items, and a search stops after visiting
  200,000 entries. Moves work only within one file system, and a failed folder
  copy leaves a partial copy. Items outside the allowed folders, including the
  targets of symbolic links, are refused, but links copied inside a folder keep
  pointing where they did. Tools work on local files only, not KIO locations,
  and changes are made directly rather than as Plasma Opus jobs, so they can't
  be undone in Plasma Opus. The audit log records paths but no contents, and
  grows until it is deleted.
  ([#287](https://github.com/mikepadiernos/plasma-opus/issues/287))
- **🔐 Plasma Vault:** **Go › Plasma Vaults** needs Plasma Vault and its
  kded6 module, and reports Plasma Vault as unavailable without them; opening
  the menu may start kded6. Passwords, creating, configuring and closing are
  left to Plasma Vault and its dialogs, so its errors appear only as its own
  message in the status bar and the menu. Importing and deleting vaults are
  not offered, and vaults limited to other Activities are listed too. An
  opened vault is shown in the active pane only when it was opened from
  Plasma Opus; if Plasma Vault's password prompt is cancelled without a
  status change, the vault is still shown when it is later opened elsewhere.
  Before closing, Plasma Opus lists this user's processes whose working
  folder or open files are inside the vault, but not memory-mapped files or
  other users' processes, and tabs still showing the vault list a missing
  folder after it closes. Plasma Opus never forces a vault closed. The
  container used for development has no Plasma Vault, so the real service
  awaits [verification on Plasma](TODO.md#plasma-vault-on-plasma-654).
  ([#654](https://github.com/mikepadiernos/plasma-opus/issues/654))
- **🔏 File encryption:** **File › Encryption** needs GnuPG for OpenPGP
  files and the separate `aescrypt` tool for `.aes` files; Kleopatra's
  entries appear only when Kleopatra is installed. Plasma Opus runs GnuPG
  without asking questions, so it encrypts only for keys certified as
  belonging to their owner and lists other keys as unavailable. Passwords
  for keys and password-only encryption are asked for by GnuPG's own prompt,
  and the AES Crypt password is held briefly in a private file, readable only
  by you, under the runtime folder, then overwritten and removed. Results
  are written as new files beside the originals, which are never replaced or
  deleted. Binary OpenPGP and AES Crypt files are recognized by their suffix
  and first bytes, and text (`.asc`) files only when they have an encryption
  or signature suffix. Folders and remote files are not offered, and
  verifying a signature checks it against the data file beside it with the
  same name. The container used for development has no `aescrypt` or
  Kleopatra, so both await
  [verification on Plasma](TODO.md#file-encryption-on-plasma-655).
  ([#655](https://github.com/mikepadiernos/plasma-opus/issues/655))
- **📧 Send by Email:** **File › Send by Email…** opens a new message in
  the default mail client, chosen in System Settings › Default
  Applications, and never sends it. **Preferences › Applications › Mail**
  names the client, turns the action off and can skip remote files
  ([Mail client and remote file settings](#integration-settings)). Folders
  and missing, unreadable or special files are listed and skipped. Unless
  remote files are skipped, a remote file is attached only after you allow
  a private local copy, which is downloaded into a temporary folder that
  only you can read and kept until Plasma Opus quits, because the mail
  client may read it later. Most clients receive the
  files as `attach` parameters in a `mailto:` URL, and Thunderbird through
  its own command line; whether they are attached is up to the client, so
  Plasma Opus reports that the composer opened, not what it attached. A
  client that is registered but fails to start is not reported. Plasma
  Impresario currently ignores attachments from other applications, so
  Plasma Opus warns before and after opening it and asks you to drag the
  files from the dialog's list into its composer instead. Inside
  a Flatpak or Snap sandbox, the XDG Email portal attaches the files
  instead, and Plasma Opus reports only that the portal accepted the
  request. The container used for development has no mail client or
  portal, so both await
  [verification on Plasma](TODO.md#send-by-email-on-plasma-656).
  ([#656](https://github.com/mikepadiernos/plasma-opus/issues/656))
- **📎 Mail attachments:** Local files in the **Send by Email** list can be
  dragged into any composer. A remote file can be dragged only after its
  private local copy is downloaded, and not at all while **Preferences ›
  Applications › Mail** skips remote files; ticking the consent box starts the
  download, and unticking it stops the download or removes the copies.
  Copies that are attached or dragged out are kept until Plasma Opus quits,
  because Plasma Opus can't tell when the mail client has finished reading
  them; others are removed when the dialog closes. Dropping a Plasma
  Impresario attachment on a pane is refused with a status-bar explanation,
  because the drag carries only a private reference to the message on the
  mail server, not the file; Plasma Opus never contacts the mail server. A
  drop that carries the file content and a suggested filename is saved
  through KIO, which asks for the file name. The container used for
  development has no Plasma Impresario, so real drags await
  [verification on Plasma](TODO.md#mail-attachments-on-plasma-657).
  ([#657](https://github.com/mikepadiernos/plasma-opus/issues/657))
- **📨 Mail file preview:** The preview panel shows a local `.eml` file's
  From, To, Cc, Date and Subject headers, its plain-text body and its
  attachments. An HTML-only message is reduced to plain text; images, links
  and other remote content are never loaded, and scripts never run.
  Encrypted parts are not decrypted and signatures are not verified; the
  panel says so instead. Messages over 64 MiB, and messages on remote
  locations, are not parsed; a remote `.eml` still shows as raw text.
  Clearing **Preview mail files (.eml) in the preview panel** under
  **Preferences › Applications › Mail** shows local messages as raw text
  too ([Mail preview settings](#integration-settings)).
  **Extract All** and **Extract Selected** save the attachments into the
  folder that holds the message, through KIO's conflict handling, and
  report an error instead if the message changed after it was previewed.
  **Open** hands a read-only private copy to the attachment's registered
  application, chosen from the copy's content rather than the type the
  message declares, and never runs programs. Those copies are kept until
  the window closes. Opening the message itself uses the normal **Open**
  action on the file; Plasma Impresario doesn't open mail files, and Plasma
  Opus never imports messages into it. The container used for development
  has no registered applications or mail client, so opening awaits
  [verification on Plasma](TODO.md#mail-file-preview-on-plasma-658).
  ([#658](https://github.com/mikepadiernos/plasma-opus/issues/658))
- **🪢 Shared Activity context:** Plasma Activities is the only context
  Plasma Opus shares with Plasma Impresario. Each application follows
  Activity changes with its own associations, and Plasma Opus never reads or
  changes Plasma Impresario's workspaces, accounts or mail. Visited folders
  and opened files are reported to the current Activity, so Plasma lists
  them in its recent files and locations and other applications can offer
  them. Reporting is on by default and covers local folders and network
  locations such as SFTP and SMB, without passwords. Temporary files, open
  CryFS, gocryptfs, EncFS, securefs and eCryptfs folders, searches, the trash
  and administrator locations are never reported. Plasma Activities decides
  what it keeps, so blocking Plasma Opus or clearing history happens in
  System Settings. Plasma Impresario's account-to-workspace associations
  aren't released yet, and the container used for development has no
  Activities service, so both await
  [verification on Plasma](TODO.md#shared-activity-context-on-plasma-659).
  ([#659](https://github.com/mikepadiernos/plasma-opus/issues/659))
- **📓 Markdown vault links:** The editor's **Vault** menu, or **Vault
  folder** under **Preferences › Applications › Editor**, chooses one local
  folder whose notes `[[wiki links]]` resolve against; every editor uses the
  same folder. Links name a note anywhere in the
  vault, a vault-relative path such as `Projects/Plan`, or a path relative
  to the note that starts with `./` or `../`; names are matched without
  regard to case. When several notes share a name, the one in the linking
  note's folder wins, and otherwise the link is reported as ambiguous with
  the candidates listed. Only `.md`, `.markdown` and `.mkd` files are
  indexed, up to 10,000 notes of at most 4 MiB of UTF-8 each; hidden
  folders such as `.obsidian` are skipped, symbolic links are followed only
  when they stay inside the vault, and links never open anything outside
  it. The backlinks list and the missing-link count are refreshed when a
  note is opened, saved or the vault is rescanned, not while typing, and
  the panel reports unreadable notes and stopped scans. The source is never
  rewritten: the preview renders a copy with the links made clickable, but
  only for a saved note inside the vault. A link's `#heading` and the
  embedded content of `![[embeds]]` are not shown; following either opens
  the whole note or file, and attachments open in their registered
  application without running programs. The preview is Qt's Markdown
  renderer, built on the md4c parser: its **CommonMark** and **GitHub
  Markdown** dialects parse as those specifications do, but raw HTML,
  footnotes and math are shown as text, so it is not a complete CommonMark
  renderer. ([#564](https://github.com/mikepadiernos/plasma-opus/issues/564))
- **🏷️ Workspace vaults and tags:** **Use as Editor Vault** in **Workspaces**
  makes one of a workspace's assigned locations the vault of every editor while
  that workspace is active; the editor's own choice is kept and returns when the
  workspace has no vault, its vault is turned off or the workspace is left.
  While the workspace's vault location can't be found, such as after it is
  removed from Favorites, the editor says why instead of using its own vault.
  Only one vault applies at a time, shared by
  all windows of a Plasma Opus process, and only local folders that resolve to a
  single saved location qualify, so remote locations and ambiguous saved names
  can't be chosen. While the workspace's vault applies, **Vault › Choose Vault
  Folder** and **Vault › Close Vault** are disabled. The **Links** and **Tags**
  tabs read the open note's text when it is opened, saved or the vault is
  rescanned, while the other notes listed under a tag come from the last scan.
  Tags are inline `#tag` or nested `#area/topic` words outside code, and `tags:`
  or `tag:` in YAML front matter; tags that are only digits are ignored and case
  is not significant. Tags can't be renamed or searched across the vault, and
  there is no graph view. **Workspace Vault** under **Preferences ›
  Applications › Editor** turns a workspace's vault off or gives it its own
  options; see **Workspace vault settings** under
  [Integration settings](#integration-settings).
  ([#565](https://github.com/mikepadiernos/plasma-opus/issues/565))
- **🎞️ Embedded mpv playback:** **Audio and video playback** under **Preferences
  › Applications › Media**, or the backend button beside the playback controls,
  chooses Qt Multimedia, the default, or mpv for the preview panel and gallery.
  mpv needs a build with libmpv and Qt OpenGL Widgets; otherwise the choice is
  disabled and Qt Multimedia plays with a notice. Video through mpv also needs
  OpenGL: when Plasma Opus can't create an OpenGL context, or libmpv can't draw
  into it, Qt Multimedia plays the video and the notice says why. Audio needs no
  OpenGL. The embedded player ignores your `mpv.conf`, scripts and `youtube-dl`
  or `yt-dlp`, uses safe hardware decoding, and only plays local files: remote
  previews play their temporary download, within the 128 MiB remote preview
  limit. Its controls are play, pause and seeking; there is no volume, track,
  subtitle or speed control, and mpv's keyboard shortcuts don't apply. If libmpv
  only starts with the C numeric locale, Plasma Opus switches the C library's
  number parsing to it; Qt's number formatting is unaffected. Playback errors
  appear where the preview or video would be. The separate **Open with mpv**
  action opens selected local audio and video in the mpv application. The
  container used for development has no OpenGL, so drawing video on Wayland and
  X11 awaits [verification on Plasma](TODO.md#mpv-integration-563).
  ([#563](https://github.com/mikepadiernos/plasma-opus/issues/563))

- **Recommendation:**
  - **FileManager1 service:** Choose **Set Plasma Opus as Default** and
    **Install Activation File** under **Preferences › KDE Plasma › File
    Manager**, or run `./install.sh --default`. If Dolphin is running, quit
    it and restart Plasma Opus so it claims the service. To hand requests
    back, choose **Remove Activation File** there, or run
    `./install.sh --uninstall`, as well as changing the default in System
    Settings.
  - **Language preferences:** After changing Plasma's languages, log out and
    back in, or select the language under **Settings › Translation ›
    Language and Layout** and restart Plasma Opus. The status line on that page
    shows which language is in effect.
  - **Notifications and jobs:** Keep the Plasma Opus dialog open, or switch
    to it from the task manager, to read error details or change options.
    To hide the progress of particular operations, clear them under
    **Preferences › KDE Plasma › Notifications**. To hide it for every
    application, turn off application progress in notifications under
    **System Settings › Notifications**.
  - **System tray and global shortcut:** To hide the window instead of
    closing it, check **Hide the window in the system tray when it is
    closed** under **Preferences › KDE Plasma › System Tray**; on Wayland,
    prefer this to hiding when minimized. Clear **Show an icon in the system
    tray** there to remove the icon. If the shortcut does nothing, choose a
    key combination under **Preferences › KDE Plasma › Shortcuts**, and keep
    the icon always visible through the tray's **Configure System Tray…**
    entries.
  - **Plasma Activities:** Turn associations on and choose each Activity's
    workspace, behaviours and folders under **Preferences › KDE Plasma ›
    Activities**. To apply the current Activity's association at any time,
    including at start or after choosing **Not Now**, use **View › Apply
    Activity Association**. Close default-folder tabs you no longer need, and
    clear associations of removed Activities once they are not coming back.
  - **KDE Connect sharing:** To send a folder, compress it first with
    **Compress…** and send the archive; copy remote files to a local folder
    before sending. Watch KDE Connect's notification to confirm delivery. If a
    paired device shows as offline, open KDE Connect on both devices and press
    **Refresh**; if it shows file sharing as turned off, enable the **Share and
    receive** plugin for it in KDE Connect's settings, which **Configure KDE
    Connect…** under **Preferences › KDE Plasma › KDE Connect** opens.
  - **KRunner:** Install with `./install.sh`, then run `kquitapp6 krunner`
    or log out and back in if **Plasma Opus** is missing from **System
    Settings › Search › Plasma Search**. Clear what should not be offered
    under **Preferences › KDE Plasma › KRunner**, or turn the plugin itself
    off in System Settings. Add a folder to Favorites to make it searchable.
  - **Service-menu downloads:** Install Dolphin to enable **Download New
    Services…**; Plasma Opus does not need to be the default file manager.
    Read a service menu's description and comments before installing it, and
    hide any unwanted action under **Settings › Context Menu › Service
    Menus** instead of removing it if Dolphin should keep it. Reopen the
    settings to see service menus installed by other means.
  - **System file chooser:** Choose **Set Up** under **Preferences › KDE
    Plasma › File Manager**, or run `opus-file-manager --install-portal`,
    from an installed build in a Plasma session with xdg-desktop-portal
    1.21.0 or later, after closing open file dialogs. If applications still
    show KDE's chooser, check `xdg-desktop-portal --version` and log out and
    back in. Copy remote files to a local folder before choosing them. To go
    back, choose **Restore Previous Chooser**, or run
    `opus-file-manager --uninstall-portal` or `./install.sh --uninstall`,
    which restores the previous `kde-portals.conf`.
  - **Mount manager:** Press **Refresh** for current sizes. To power off a
    USB drive, use Plasma's **Disks & Devices** applet. To let Plasma Opus
    mount an fstab entry without root, add `user` or `users` to its options
    in KDE Partition Manager, or let UDisks2 manage the device instead. Open
    an automount point's folder once to mount it.
  - **Disk images:** Install `dolphin-plugins` from Discover or your
    distribution, and keep images inside the home folder so they appear in
    Places once mounted. Wait for the window to respond after choosing
    **Mount Disk Image**. If an image stays attached without a mount point,
    choose **Unmount Disk Image**, or **Mount Disk Image** again to retry.
  - **Application packages:** Install the `flatpak` or `snap` packages, and
    Discover for **Open in Discover**. Install a Flatpak bundle, then open
    its folder under `~/.local/share/flatpak/app` to review its permissions,
    and adjust them with Plasma's Flatpak permission settings in System
    Settings or with Flatseal. Prefer installing snaps from the Store
    in Discover so they are verified and updated; for a snap that needs
    classic confinement, run `snap install --classic` in a terminal. Remove
    an AppImage from the application menu before moving or deleting it, and
    add it again from its new location. Turn off the package types you don't
    use under **Preferences › Applications › Packages**.
  - **KWin script:** Install it from **Preferences › KDE Plasma › KWin** in a
    Plasma session, then move or resize the window once with each workspace
    current so its placement is kept. If the status says the script has not
    contacted Plasma Opus, press **Refresh**, or **Disable** and **Enable** it
    there. Run Plasma Opus outside Flatpak for window management. Leave the
    processing shortcut unassigned. Use **Configure Window Rules…** there for
    placements the script does not restore, such as maximized state.
  - **Drop-down window:** Turn it on and choose a global shortcut under
    **Preferences › KDE Plasma › KWin**, and install the KWin script there
    so it keeps above other windows and out of the taskbar. Enable **Sliding
    popups** under **System Settings › Window Management › Desktop Effects**
    for the animation. If an edge does nothing, free it under **System
    Settings › Mouse & Touchpad › Screen Edges** or **Touch Screen**.
  - **File-type categories:** Open a category from the main window's Places,
    not from a file dialog. If indexed results look incomplete, open
    **File › Indexing Status…** to check the indexer and the folder's
    coverage, or choose **Search a Folder Live…** to read the files directly.
    Clear a category you don't use under **Preferences › KDE Plasma ›
    Baloo**, and check it there to restore it. For other sizes or formats,
    set **Large files are over** there, or search with `size>` or `type:` in
    the search panel.
  - **Terminal providers:** Install the terminal from your distribution so
    its command is in `PATH`, or keep **Default Terminal** chosen for **Open
    Terminal Here** under **Preferences › Applications › Terminal** to follow
    the terminal chosen under **System Settings › Default Applications**.
    After opening a Yakuake session, press Yakuake's shortcut, F12 by
    default, if its window is hidden, and check the folder before running
    commands. To keep the `cd` out of shell history, set
    `HISTCONTROL=ignorespace` in Bash, or `setopt HIST_IGNORE_SPACE` in Zsh.
  - **Editor integrations:** Install the editor from your distribution so
    its command is in `PATH`, and install the Konsole component for terminal
    editors, or choose **Separate terminal** for them under **Preferences ›
    Applications › Editor**. To edit remote files in a terminal editor, copy
    them to a local folder first, or use KWrite or Kate. Save your work
    before closing the editor's terminal tab.
  - **Office documents:** Install LibreOffice, Calligra or ONLYOFFICE from
    your distribution so its command is in `PATH`, choose it for **Edit**
    under **Preferences › Applications › Office**, and install Calligra for
    previews. To open remote documents in LibreOffice or
    ONLYOFFICE, copy them to a local folder first, or use Calligra. Open RTF
    and CSV files with **Open With**.
  - **Edit action:** Install a listed tool from your distribution, choose
    the editor for text files under **Preferences › Applications › Editor**
    and the office application under **Preferences › Applications ›
    Office**, use **Edit With** or **Open in Office Application** to pick a specific
    editor, or **Open With** for other applications. Copy remote files to a
    local folder before editing them in a local-only tool, and check the
    format when saving from an image or audio editor.
  - **Ebook and font previews:** Install Okular for ebook, comic, DjVu and
    help-file previews, and open long documents in Okular to read them. To
    compare a font with an installed one of the same name, or inspect its
    characters, open it in Plasma's font viewer, KFontView.
  - **Remote connections:** Install `kio-extras` for SFTP and OpenSSH for
    SSH terminals. Load keys into the SSH agent, with `ssh-add`, so browsing
    and terminals sign in without a password. Prefer WebDAV over HTTPS, and
    remove saved passwords you no longer need in KDE Wallet Manager, which
    **Open KDE Wallet Manager…** under **Preferences › Remote and Cloud ›
    Connections** opens. After an SSH terminal connects, `cd` to the folder you
    need.
  - **Network shares and FTP:** Prefer SFTP or WebDAV over HTTPS to plain
    FTP. For FTPS, install a third-party KIO worker or use `lftp` in the
    terminal. If **Network** stays empty, install `kio-extras` with Avahi and
    `kdsoap-ws-discovery-client`, and allow mDNS and WS-Discovery through
    the firewall, or save a connection to the host directly.
  - **rclone cloud storage:** Install rclone and FUSE 3 from your
    distribution and add remotes with `rclone config`; without FUSE, choose
    **Serve it over WebDAV on this computer only** under **Preferences ›
    Remote and Cloud › rclone**. Close files on a
    mount before unmounting it or quitting Plasma Opus. If a mount stays
    behind, run `fusermount3 -u` on its folder under
    `$XDG_RUNTIME_DIR/opus-file-manager/rclone`. For mounts that outlive
    Plasma Opus or start at login, use rclone's own systemd service.
  - **Remote drives:** Keep the default KIO FUSE for occasional access; it
    reuses passwords saved in KWallet. Choose sshfs and install `ksshaskpass`
    for SSH servers that need read-only or a fixed folder. For WebDAV or
    Windows shares with a fixed folder, ask an administrator to add the line
    **Mount Options…** shows to `/etc/fstab`. Close files before unmounting.
    If a folder is reported as stale, run `fusermount3 -u` on it. To keep
    sshfs, davfs2 and CIFS mounts after quitting or mount them at login,
    choose so under **Preferences › Remote and Cloud › Remote Drives**; for
    mounts needed before you log in, use a systemd mount unit or fstab entry
    with `auto`.
  - **Temporary LAN sharing:** Keep HTTPS on and, before accepting the
    certificate, check that the fingerprint the browser shows matches the
    dialog's. Share only on trusted networks, with the shortest practical
    time limit, and pass the link through the QR code or a private channel.
    If clients can't connect, choose a fixed port under **Preferences ›
    Remote and Cloud › LAN Sharing** and allow it, for example with `sudo
firewall-cmd --add-port=PORT/tcp` until the next restart or `sudo ufw
allow PORT/tcp`, removing the ufw rule afterwards. For paired phones, KDE
    Connect is simpler; for lasting shares, use Samba sharing in the folder's
    **Properties** dialog.
  - **KDE Online Accounts:** Install `kio-gdrive` for Google Drive and
    `kaccounts-providers` for Nextcloud and ownCloud, add accounts in
    **System Settings › Online Accounts**, then check **Show the folders of
    KDE Online Accounts in Plasma Opus** under **Preferences › KDE Plasma ›
    Online Accounts**. When KIO reports an expired
    sign-in, sign in again on that page. For WebDAV storage, let KWallet
    remember the password when KIO asks, or, for Nextcloud, sign in under
    **Go › Nextcloud › Manage Nextcloud Accounts…** to use an app password.
    For providers without a KDE file service, use **Go › Cloud Storage**
    with rclone.
  - **rsync backend:** Install rsync on both computers. Use SSH keys with an
    agent, or install `ksshaskpass`, and connect to the host once in a
    terminal to accept its host key. Plan one direction per run, read rsync's
    changes in the confirmation's details, and compare again after each run;
    for changes in both directions, run two plans. Back up data whose
    permissions or ownership matter, and allow deletions under **Preferences ›
    Remote and Cloud › Sync** only for folders you have backed up.
  - **Cloud sync:** Add pairs under **File › Cloud Sync…** or **Preferences ›
    Remote and Cloud › Sync**, and check **Synchronize in the background after
    login** so they run on their intervals. Read the first run's review and any
    large deletion before choosing **Synchronize**. Choose a backup mode when
    only one side should change, keep pairs out of folders another client
    synchronizes, and resolve conflicts in the review rather than leaving them
    skipped. Merge and delete conflict copies once you have compared them.
    Rename one side of a file-and-folder conflict, and move whole folders with
    the remote's own tools to avoid copying them again. Check the tray icon or
    the dialog for pairs that need attention.
  - **Cloud activity:** Open **File › Cloud Activity…**, or **Cloud
    Activity…** in the tray menu, to follow transfers and errors. Set a
    bandwidth limit when uploads slow other traffic and an offline cache large
    enough for the files you reopen, in the window or under **Preferences ›
    Remote and Cloud › Transfers and Cache**, then unmount and mount remotes
    again for the limits to apply. Let the upload wait finish before unmounting
    or logging out; if you continue without waiting, mount the remote again
    later to finish uploading. Before importing definitions on another computer,
    configure the same rclone remotes there, then sign in to each connection and
    resume the imported pairs under **File › Cloud Sync…**.
  - **Administrator mode:** Install `kio-admin` from Discover or your
    distribution. Act as an administrator only for the change you need,
    copy a file to a backup name in the same folder before editing it, then
    press **Stop Acting as Administrator**. For commands, use `sudo` in a
    normal terminal instead. Lock the screen when leaving the computer,
    because Polkit may keep the authorization for a short time.
  - **MCP server:** Keep the default read-only access, and allow only the
    folders a client needs, such as a single project folder, under
    **Preferences › Security › MCP**. Turn on write access only while a task
    needs it, start other clients with `--read-only`, and restart the client
    after raising the access. Choose **Show Audit Log** there to see what a
    client did, restore items from the trash, and move large files between
    drives yourself.
  - **Plasma Vault:** Install Plasma Vault and a backend such as gocryptfs
    from Discover or your distribution. Close files and terminals in a vault
    before **Close Vault**; if Plasma Vault still refuses, use its system
    tray applet to force the vault closed. Leave other tabs showing a vault
    before closing it.
  - **File encryption:** Certify the keys you encrypt for in Kleopatra
    first, or use **Sign and Encrypt with Kleopatra…** for keys you have not
    certified. Install `aescrypt` only when you exchange `.aes` files, and
    delete the original yourself once you have checked the encrypted copy.
    Keep a signature beside the file it signs, with the same name plus `.sig`
    or `.asc`.
  - **Send by Email:** Choose a client that accepts attachments, such as
    KMail or Thunderbird, under **System Settings › Default Applications ›
    Email client**, which **Configure Default Applications…** under
    **Preferences › Applications › Mail** opens. With Plasma Impresario,
    drag the files from the **Send by Email** list into its composer
    instead. Send a message that uses local copies of remote files before
    quitting Plasma Opus, or copy the files into a local folder first.
  - **Mail attachments:** To keep an attachment you received, save it from
    Plasma Impresario, then drag the saved file into Plasma Opus.
  - **Mail file preview:** Read encrypted, signed or formatted messages in
    your mail client. Copy a remote `.eml` file into a local folder to
    preview it, and use **Extract Selected** rather than **Open** to keep an
    attachment, because opened copies are removed when the window closes.
  - **Shared Activity context:** To keep a location out of Plasma's
    history, turn off **Report visited folders and opened files to Plasma
    Activities** under **Preferences › KDE Plasma › Activities**, or choose
    what Plasma remembers, and clear it, under **System Settings › Recent
    Files**. Keep private files in a Plasma Vault, whose open folders are
    never reported.
  - **Markdown vault links:** Give notes unique names or link to them by
    path, such as `[[Projects/Plan]]`, and choose **Vault › Rescan Vault**
    after changing notes in another application. Split vaults with more
    than 10,000 notes, and convert notes that aren't UTF-8. Use Obsidian or
    another Markdown application to view embeds, headings and HTML.
  - **Workspace vaults and tags:** Keep notes in a local folder and assign
    it to the workspace before choosing **Use as Editor Vault**; for notes
    on a server, sync them to a local folder first. Save a note, or choose
    **Vault › Rescan Vault**, to refresh its links and tags. Use Obsidian or
    another Markdown application to rename tags or browse a graph.
  - **Embedded mpv playback:** Use **Open with mpv** from the context menu
    for volume, tracks, subtitles and mpv's own configuration. If video stays
    black or the notice reports OpenGL, choose **Qt Multimedia** with the
    backend button or under **Preferences › Applications › Media**.
  - Comment:
    [#728.7196797397](https://github.com/mikepadiernos/plasma-opus/issues/728#issuecomment-7196797397)
- **Proposed improvements:**
  - Claim or release the service when the default handler changes in
    System Settings while Plasma Opus runs.
  - Watch `plasma-localerc` and offer a restart when Plasma's language list
    changes.
  - A job action that raises the dialog that started the operation.
  - Show only the current Activity's vaults, and move tabs showing a vault
    away when it closes.
  - Encrypt folders through an archive, offer to remove originals after a
    checked encryption, and pass AES Crypt passwords through a pipe where the
    tool allows it.
  - Offer to compress a selected folder and attach the archive, and ask the
    XDG Email portal for its response to report cancelled requests.
  - Extract mail attachments into a chosen folder, preview remote `.eml`
    files through a size-limited download, and show inline images only on
    request.
  - Jump to a link's heading, show embedded notes and images in the
    preview, create a missing note from its link, refresh backlinks while
    typing and watch the vault for changes, and an M28 setting for the
    vault's note limit.
  - Refresh links and tags while typing, rename a tag across the vault,
    search notes by tag, a graph view, and a vault per window.
  - Volume, track and subtitle controls and mpv's keyboard shortcuts in the
    embedded player, an option to read the user's mpv configuration, and
    playing remote files without a full download.
  - Report files opened through **Open With** and from previews, exclude
    chosen folders from reporting, and an option under **Preferences › KDE
    Plasma › Activities** to report only while associations are on.
  - Reuse an open tab showing an Activity's default folder instead of
    opening another, and offer the current Activity's association at start
    as an option.
  - Keep association behaviours with the session.
  - Offer to compress a selected folder and send the archive, and report
    KDE Connect's transfer result in the dialog when its share plugin exposes
    one.
  - Let KRunner match saved locations assigned to a workspace, check remote
    Favorites before offering them, and stop the helper after a period
    without searches.
  - Ship a Plasma Opus provider and installer so downloads work without
    Dolphin, and watch the service-menu folders to refresh the list while
    the settings are open.
  - A version check that warns before setup on xdg-desktop-portal older
    than 1.21.0, previews and indexed search in the chooser, and returning
    remote files through a local copy.
  - Watch mount changes instead of polling, refresh sizes periodically, and
    power off USB drives after unmounting.
  - Mount disk images through UDisks2 directly, without blocking the window
    and with its actual error messages, and show images outside the home
    folder in Places once mounted.
  - Read a Flatpak bundle's ID and permissions before installing, show tool
    progress, update or remove AppImage menu entries when the file moves,
    and recognize custom Flatpak installations.
  - Verify the script on X11 and Wayland with each supported Plasma release,
    keep maximized and full-screen state, and record a placement when a
    workspace is saved as well as when the window moves.
  - Ask KWin to place other applications started from Plasma Opus, such as
    terminals and editors, through XDG activation tokens.
  - Roll the drop-down window up instead of closing it, offer a default
    shortcut when it is free, and let a detached window drop down instead
    of the main window.
  - Retranslate the file-type category titles when the language changes,
    hide the entries from file dialogs, and confirm live results by content
    as Baloo does.
  - Find Flatpak terminals, show Yakuake's window through a dedicated D-Bus
    call when it offers one, and open a selected folder rather than the
    pane's folder from its context menu.
  - Find Flatpak editors and add other editors.
  - Find Flatpak office applications and offer RTF and CSV files to the
    chosen one.
  - A setting to choose the image, SVG and audio editors, and use the file
    associations' editors where available.
  - Use Plasma's font viewer component when available, preview every face
    of a font collection, and remember the reading position in ebook
    previews.
  - Show saved connections in Places, test a connection before saving it,
    start SSH terminals in the profile's folder, confirm removals and offer
    to forget the saved password, and import hosts from `~/.ssh/config`.
  - Save a discovered share as a connection from **Network**, and hide FTPS
    when no worker handles it.
  - Keep mounts running in the tray, show mounts in Places, and choose
    rclone's cache mode in the M28 settings.
  - Mount KDE Online Accounts and S3 locations, remount with changed
    options, and pass KWallet passwords to davfs2 and CIFS with consent.
  - Keep sharing from the system tray after the dialog closes, announce
    shares over DNS-SD, add an optional password and revocable links,
    keep connections alive, create folders over WebDAV, and refresh
    addresses when the network changes.
  - Show online account folders in Places, mount them as remote drives,
    refresh the menu when accounts change, and hand the app passwords of
    Online Accounts' Nextcloud accounts to KIO with consent, as **Go ›
    Nextcloud** already does for accounts signed in there.
  - Share one SSH connection between rsync runs, offer to keep permissions
    and links, and delete folders missing from the source.
  - Detect folder renames, compare checksums where the remote offers them,
    watch local folders instead of waiting for the interval, send desktop
    notifications, and show transfers and quota per pair.
  - Show the speed of rclone mounts and sync runs through rclone's remote
    control, limit the bandwidth of KIO pairs, delay logout while uploads are
    pending, and mark folders to keep available offline.
  - Mark administrator tabs in the tab bar, offer **Open as Administrator**
    for a selected folder in a new tab, and allow trash, checksums and other
    local-file actions through the `admin:` worker where KIO supports them.
  - An optional Streamable HTTP transport on the loopback interface, with an
    access token kept in KWallet; reading other text encodings and images;
    moving between file systems through KIO jobs that can be undone; and a
    size limit for the audit log.

## Integration settings

The M28 integration settings have these limits
([#759](https://github.com/mikepadiernos/plasma-opus/issues/759)):

- **🥇 Default file manager:** **Set Plasma Opus as Default** under
  **Preferences › KDE Plasma › File Manager** needs an installed Plasma Opus
  desktop entry, so it is unavailable when running from a build tree. It
  writes the user's `mimeapps.list`, as System Settings does. A
  desktop-specific list, such as `kde-mimeapps.list`, that names another
  default takes precedence; it is reported, not changed. There is no action
  to hand the default back. Plasma Opus claims `org.freedesktop.FileManager1`
  at once only when no other application, such as a running Dolphin, owns
  it; otherwise it takes over when it next starts after that application
  quits. The activation file can be installed only while Plasma Opus is the
  default, and a file that another application wrote is left alone. After a
  change, Plasma Opus asks the session bus to reload its activation files,
  which dbus-broker needs. The development container has no Plasma session,
  so the tab is verified only by tests there
  ([Plasma check](TODO.md#default-file-manager-on-plasma-293)).
  ([#293](https://github.com/mikepadiernos/plasma-opus/issues/293))
- **🚪 System file chooser:** the **System File Chooser** group under
  **Preferences › KDE Plasma › File Manager** reads the per-user setup only:
  it cannot tell whether xdg-desktop-portal is 1.21.0 or later, which
  per-user portal descriptions need, or whether it has already read the
  change. **Set Up** needs an installed `opus-file-manager-portal` next to
  Plasma Opus and a running or activatable xdg-desktop-portal; outside a
  Plasma session it is allowed but takes effect only in Plasma sessions,
  which the group notes. Both actions ask first, because restarting
  xdg-desktop-portal through `systemctl --user` closes file dialogs open in
  other applications; when the restart fails, the change stays and applies
  after logging out and back in. The dialog waits for the restart. The
  container has no session bus or Plasma session
  ([Plasma check](TODO.md#file-chooser-settings-on-plasma-292)).
  ([#292](https://github.com/mikepadiernos/plasma-opus/issues/292))
- **🏠 System file chooser start folder:** **Start folder** applies only
  when the application suggests no folder or file, and only to the Plasma
  Opus chooser. Folders are remembered by the application ID that
  xdg-desktop-portal passes on; applications it cannot identify, typically
  unsandboxed ones, share one folder. Only local folders are remembered,
  and only while **Folder of the last choice** is selected, so switching to
  it starts from the home folder until the next accepted choice. A
  remembered folder that no longer exists falls back to the home folder.
  The folder of a chosen folder is the folder itself; for files, it is the
  folder of the first file. The setting applies from the next request after
  **Apply** or **OK**. The container has no xdg-desktop-portal, so it is
  verified there over a private session bus only
  ([Plasma check](TODO.md#chooser-start-folder-on-plasma-750)).
  ([#750](https://github.com/mikepadiernos/plasma-opus/issues/750))
- **📣 Operation notifications:** the checkboxes under **Preferences › KDE
  Plasma › Notifications** cover only the seven Plasma Opus operations that
  report to Plasma; file copies, moves and deletions are KIO jobs that only
  Plasma's settings control. A change applies to operations started after
  **Apply** or **OK**; a running operation keeps reporting, or not, until it
  ends. An operation that does not report still shows its progress and
  errors in its dialog, but nothing in Plasma tells you when it ends.
  **Configure Plasma Notifications…** opens Plasma's Notifications settings
  through `kcmshell6`, or System Settings without it, and asks for the
  Plasma Opus entry. Plasma Opus has no notification events of its own, so
  Plasma's options there apply to all of its reports together. When the
  settings are not installed, the tab says so. The container has no Plasma
  session, so opening the Plasma Opus entry and the reports are unverified
  there ([Plasma check](TODO.md#operation-notifications-on-plasma-392)).
  ([#392](https://github.com/mikepadiernos/plasma-opus/issues/392))
- **📥 Start hidden in the tray:** **Start with the window hidden in the
  system tray** under **Preferences › KDE Plasma › System Tray** applies only
  when Plasma Opus starts without folders or a workspace to open, and only
  when the tray icon is on and a tray can already show it; otherwise the
  window is shown. At login, Plasma's tray may not be ready when Plasma Opus
  starts, so the window can still appear then. The setting does not start
  Plasma Opus at login; that is up to Plasma's Autostart settings. Starting
  Plasma Opus again shows the hidden window. Minimizing to the tray still
  works only on X11, which the tab notes on Wayland. The container has no
  Plasma session, so starting hidden with Plasma's tray is unverified there
  ([Plasma check](TODO.md#start-hidden-in-tray-on-plasma-290)).
  ([#290](https://github.com/mikepadiernos/plasma-opus/issues/290))
- **⌨️ Global shortcut:** **Show or hide Plasma Opus with a global
  shortcut** under **Preferences › KDE Plasma › Shortcuts** registers the
  shortcut with KDE's global shortcut service, KGlobalAccel, only while it
  is checked. Clearing it removes the shortcut from Plasma's Shortcuts
  settings; Plasma Opus keeps the key and assigns it again when the
  shortcut is turned back on. A key chosen in the tab replaces the one in
  Plasma's settings. KDE leaves a key unassigned when another application
  already uses it, unless you agree to take it over when choosing it; the
  tab then says that no key is assigned. Without KDE's global shortcut
  service, as outside Plasma, the shortcut does nothing and the tab says
  so. The tab checks the chosen key only against other global and standard
  shortcuts, not against Plasma Opus's own window shortcuts; **Preferences ›
  Keyboard › Shortcuts** lists those conflicts. The container
  has no global shortcut service
  ([Plasma check](TODO.md#global-shortcut-on-plasma-291)).
  ([#291](https://github.com/mikepadiernos/plasma-opus/issues/291))
- **🗺️ Activity associations:** the editor under **Preferences › KDE Plasma ›
  Activities** lists the existing Activities only while Plasma's Activities
  service is running; without it, only Activities with a saved association are
  listed, under their saved names, and they can still be edited. The tab cannot
  create, rename or remove Activities: **Configure Plasma Activities…** opens
  Plasma's Activities settings for that, through `kcmshell6` or System Settings,
  and the tab says so when they are not installed. Associations can be edited
  only while **Associate workspaces and default folders with Plasma Activities**
  is checked. Changes are saved with **Apply** or **OK** and discarded by
  **Cancel**. **Restore Defaults** turns associations and their offers off but
  keeps the associations, as it keeps workspaces, so they are cleared one at a
  time with **Clear Association**. Individual behaviours can be chosen only for
  an association that names a workspace, and a folder address that is not valid
  is reported in the tab and not kept. The container has no Activities service
  or System Settings, so the Activity list and opening Plasma's Activities
  settings are unverified there
  ([Plasma check](TODO.md#activity-associations-on-plasma-391)).
  ([#391](https://github.com/mikepadiernos/plasma-opus/issues/391))
- **📲 KDE Connect sharing:** clearing **Offer File › Send to Device… to send
  files with KDE Connect** under **Preferences › KDE Plasma › KDE Connect**
  hides the action from the menus and toolbars; KDE Connect's own **Send to**
  entry in the context menu, where installed, is not affected. When **Choose the
  device the files were last sent to again** is cleared, the dialog chooses the
  first device, by name, that can receive files, and the device used last is
  forgotten once the change is applied. Only the device's KDE Connect identifier
  is remembered, in the Plasma Opus state file. **Configure KDE Connect…** opens
  KDE Connect's settings through `kcmshell6` or System Settings, and the tab
  says so when they are not installed. The container has no KDE Connect, so
  opening its settings is unverified there
  ([Plasma check](TODO.md#kde-connect-settings-on-plasma-393)).
  ([#393](https://github.com/mikepadiernos/plasma-opus/issues/393))
- **🚀 KRunner discovery:** clearing **Offer saved workspaces and Favorites in
  KRunner and Plasma Search** under **Preferences › KDE Plasma › KRunner** stops
  the plugin's results, but KRunner still loads the plugin and starts its
  helper; only Plasma's search settings turn the plugin off. **Configure Plasma
  Search…** opens them through `kcmshell6` or System Settings, and the tab says
  so when they are not installed. A change applies from KRunner's next search
  session, because the helper reads the settings again only when a session
  ends. Workspaces and Favorites added later are offered until they are cleared
  in the tab. A cleared workspace is remembered by its id, and a cleared
  Favorite by an id derived from its address that does not contain it, so a
  removed Favorite added again stays cleared, and the ids of removed workspaces
  and Favorites are kept. Locations assigned to a workspace cannot be chosen,
  because the plugin does not offer them. The container has no KRunner or
  System Settings, so KRunner following the choice and opening Plasma's search
  settings are unverified there
  ([Plasma check](TODO.md#krunner-settings-on-plasma-394)).
  ([#394](https://github.com/mikepadiernos/plasma-opus/issues/394))
- **🪪 Online Accounts access:** **Show the folders of KDE Online Accounts in
  Plasma Opus** under **Preferences › KDE Plasma › Online Accounts** is off by
  default, so **Go › Online Accounts** and the accounts menu of cloud sync
  pairs are hidden, and no account is read, until it is checked; existing
  cloud sync pairs keep their folders. The tab lists only accounts with file
  storage, by name and provider, with their folders in the tooltip, and names
  the other accounts below the list. Clearing an account only hides it in
  Plasma Opus: the account, its sign-in and its services stay in KDE Online
  Accounts for other applications, and Plasma Opus never removes accounts.
  Accounts are remembered by their KDE Online Accounts id, so an account that
  is removed and added again is shown again, and the ids of removed accounts
  are kept. Accounts added later are shown until they are cleared. The list is
  read when the tab is shown, when the option is checked and with **Refresh
  List**, not when accounts change while the tab is open. **Manage Online
  Accounts…** opens KDE's Online Accounts settings through `kcmshell6` or
  System Settings, and the tab says so when they are not installed. Without
  libaccounts-qt6, the tab says that Plasma Opus was built without Online
  Accounts support. The container has no Plasma session or KDE account
  providers, so accounts set up in System Settings and opening its Online
  Accounts page are unverified there
  ([Plasma check](TODO.md#online-account-settings-on-plasma-371)).
  ([#371](https://github.com/mikepadiernos/plasma-opus/issues/371))
- **🔌 Mount manager display:** **Show only removable devices** under
  **Preferences › KDE Plasma › Mount Manager** is on by default and sets how
  the Mount Manager's own checkbox starts each time it opens; changing that
  checkbox no longer lasts beyond the dialog. It also limits **Go › Mount
  Points**, which then shows **No removable devices** when none is listed;
  the Mount Manager remains the way to reach fixed disks, fstab entries and
  network mounts. A device counts as removable when Solid reports its drive
  as removable or hot-pluggable, so file systems that Solid does not manage,
  such as network and FUSE mounts, never do. Clearing **Hide pseudo, snap
  and other system file systems** lists the kernel, memory, automount, snap,
  container and document portal mounts that Plasma Opus recognizes, with a
  tooltip saying so; Plasma Opus never mounts or unmounts them, and an
  automount shows its `autofs` entry in place of the file system mounted below
  it. **Show the Mount Points menu on the main toolbar** adds or removes the
  toolbar entry, as the toolbar editor does, and puts it back after **Preview**,
  or before **Search** when **Preview** is not on the toolbar; it needs the main
  window, and Go › Mount Points stays in the menu either way. The settings apply
  from the next time the menu or dialog opens after **Apply** or **OK**. The
  container has no removable devices, so the filter is verified there with test
  devices only.
  ([#705](https://github.com/mikepadiernos/plasma-opus/issues/705))
- **🧩 KWin script management:** the **KWin Script** group under **Preferences ›
  KDE Plasma › KWin** installs nothing until **Install and Enable** is chosen,
  and then installs the script for the current user only. Without KWin or its
  scripting interface, as outside Plasma, the tab says so and cannot install the
  script. **Update** appears only when the installed script is older than the
  one included with Plasma Opus. **Disable** keeps the script installed, and
  **Remove…** asks first and removes only the Plasma Opus script and its entry
  in KWin's settings. A failed action is reported in the tab with its error. The
  status follows the script once it contacts Plasma Opus, but changes made
  elsewhere, such as in KWin's script settings, show only after **Refresh**. The
  tab manages the script only when Preferences is opened from a Plasma Opus
  window. The options in the **Window Management** group take effect only while
  the script is running, from its next request after **Apply** or **OK**; while
  it is disabled or removed, they are kept but do nothing. Virtual desktops are
  listed only after the running script has reported them, and workspaces can be
  assigned to them only while **Apply the workspace assigned to the virtual
  desktop that becomes current** is checked. A desktop that KWin no longer lists
  keeps its assignment, and a deleted workspace is shown as missing. **Restore
  Defaults** resets the options but keeps the assignments, which are cleared by
  choosing **None**. **Configure KWin Scripts…** and **Configure Window Rules…**
  open KWin's settings through `kcmshell6` or System Settings, and the tab says
  so when they are not installed; Plasma Opus never changes KWin's window rules.
  The container has no KWin or System Settings, so the script actions are
  verified there against a simulated KWin only, and opening KWin's settings is
  unverified ([Plasma check](TODO.md#kwin-script-settings-on-plasma-709)).
  ([#709](https://github.com/mikepadiernos/plasma-opus/issues/709))
- **🪂 Drop-down window settings:** the **Drop-Down Window** group under
  **Preferences › KDE Plasma › KWin** replaces the **Configure KWin
  Integration** dialog, and **Settings › Configure KWin Integration…** now opens
  this tab. Its options are sent to the KWin script after **Apply** or **OK**;
  the screen edge, size and screen apply the next time the window is shown, and
  they and the screen and touch-screen triggers work only while the script is
  running. The **Screen** list shows the screens connected while Preferences is
  open; a chosen screen that is disconnected stays chosen as not connected, and
  the window opens on the active screen until it returns. The global shortcut
  is assigned in KDE's global shortcut service, not saved with the other
  settings, and can be changed only when Preferences is opened from a Plasma
  Opus window. The tab says when the service is not running or when KDE left
  the shortcut unassigned because another application uses the key, and
  **Restore Defaults** clears it. The container has no KWin or global shortcut
  service, so the settings are verified there against a simulated KWin and
  shortcut only ([Plasma check](TODO.md#drop-down-settings-on-plasma-710)).
  ([#710](https://github.com/mikepadiernos/plasma-opus/issues/710))
- **🔦 Baloo search settings:** **Offer ranking results by name similarity in
  the search panel** and **Offer to search live after an indexed search, for
  matches the index lacks** under **Preferences › KDE Plasma › Baloo** are on
  by default and take effect after **Apply** or **OK**; a search still running
  uses them when it finishes. Clearing the first hides **Rank results by
  similarity** in the search panel and lists results in the order found, but
  keeps the panel's own choice for when ranking is offered again. Clearing the
  second stops **Search Current Folder Live** and **Search Workspace Locations
  Live** from appearing, and hides one already shown; the live scope itself
  stays available. The tab reads whether file indexing is on and the
  indexer's state each time it is shown, but doesn't follow later changes
  while it stays open. **Configure File Search…** opens KDE's File Search
  settings through `kcmshell6` or System Settings, and the tab says so when
  they are not installed; Plasma Opus never changes what Baloo indexes. The
  default scope, search folders and exclusions stay under **Preferences ›
  Tools › Search**, which **Search Defaults…** shows. The container has no
  running indexer or System Settings, so the indexer state and opening File
  Search settings are unverified there
  ([Plasma check](TODO.md#baloo-search-settings-on-plasma-443)).
  ([#443](https://github.com/mikepadiernos/plasma-opus/issues/443))
- **🗃️ File-type category settings:** The categories checked under
  **Preferences › KDE Plasma › Baloo** are added to Places, and the cleared
  ones removed, after **Apply** or **OK**. A category checked again gets a
  new entry at the end of **Search For**, so its earlier position and any
  title or icon edited in Places are lost. Removing an entry from Places
  clears its category, also in an open Preferences window, but Places'
  **Hide** action keeps it checked. The entries live in Plasma's shared
  `user-places.xbel`, so deleting or replacing that file clears every
  category. **Large files are over** takes whole MiB from 1 MiB to 1 TiB and
  applies to the next **Large Files** search; the other categories have no
  criteria to set, and their file types are fixed.
  ([#660](https://github.com/mikepadiernos/plasma-opus/issues/660))
- **🐚 Terminal provider settings:** **Separate terminal** under
  **Preferences › Applications › Terminal** chooses what **Open Terminal
  Here** opens, on the toolbar and in the folder context menus, after
  **Apply** or **OK**. **Tools › Open Terminal With** still offers every
  terminal, and running an executable in a terminal still uses KDE's default
  terminal. The tab marks the terminals that aren't installed, checking
  `PATH` and the installed components each time it is shown, so a terminal
  installed only as a Flatpak shows as not installed. A chosen terminal that
  is later removed is reported by **Open Terminal Here** instead of replaced
  by another, and Yakuake opens with the same limits as from **Open Terminal
  With**. **Embedded terminal** moved here from **Tools › Embedded
  Terminal**, where the panel's placement and folder following remain, and
  like the panel menu's **Terminal Component** applies to new sessions only.
  ([#295](https://github.com/mikepadiernos/plasma-opus/issues/295))
- **📜 Atuin history settings:** Clearing **Offer Search Command History
  with Atuin** under **Preferences › Applications › Terminal** hides the
  action, with its shortcut and any toolbar button, and the terminal panel's
  entry after **Apply** or **OK**. **Search in** chooses the current session,
  a new one when the current session is running a program, or always a new
  one. A new session starts in the active pane's folder, or in the current
  session's folder when the search is chosen from the panel's menu, and stays
  open afterwards; a session opened by showing the panel for the search
  counts as the new one. Whether a session is busy comes from the terminal
  component, so a busy Kwi session can still receive the search, and a new
  session that is already busy, for example because the shell profile starts
  a program, is reported rather than replaced. The tab reports whether Atuin,
  its history and the embedded terminal component are found each time it is
  shown. Plasma Opus never changes Atuin's history, sync, key binding or
  shell settings.
  ([#367](https://github.com/mikepadiernos/plasma-opus/issues/367))
- **🖋️ Text editor settings:** **Text editor** under **Preferences ›
  Applications › Editor** chooses what **Edit** opens text files in, including
  Markdown notes, after **Apply** or **OK**; office documents, SVG drawings,
  images and audio keep their own tools, and **Edit With** still offers every
  editor. Only the built-in editor offers drafts, the Markdown preview and
  vault links. The tab marks the editors that aren't installed, checking
  `PATH` each time it is shown, so an editor installed only as a Flatpak
  shows as not installed. A chosen editor that is later removed is reported
  by **Edit** instead of replaced by the built-in editor, and a chosen
  terminal editor reports remote files rather than opening them.
  **Terminal editors run in** applies to **Edit** and **Edit With**.
  **Separate terminal** opens the terminal chosen under **System Settings ›
  Default Applications**, rather than the one chosen for **Open Terminal
  Here**, starting in the first file's folder unless its profile sets
  another; KDE starts Konsole with `--noclose`, so its window stays open
  after the editor exits. Plasma Opus doesn't track whether an editor in a
  separate window has unsaved changes.
  ([#320](https://github.com/mikepadiernos/plasma-opus/issues/320))
- **📔 Markdown settings:** The **Markdown** group under **Preferences ›
  Applications › Editor** applies to the built-in editor, including the
  editors that previews open, after **Apply** or **OK**. **Vault folder** is
  the folder that the editor's **Vault** menu chooses, so a folder chosen
  there while Preferences is open is replaced by the one the dialog shows if
  other changes are applied afterwards. One vault folder applies to every
  window, it must be local, and an unavailable folder, such as one on a
  disconnected drive, is reported in the tab and in the editor, and read again
  when settings are applied or the vault is rescanned. **Preview dialect** is
  the dialect each editor starts with; the editor's own dialect list changes
  it only for that editor, until the setting changes. **Show the preview when
  a Markdown file opens** applies to files opened afterwards. With **Follow
  wiki links** off, the preview shows wiki links as written and the **Links**
  list doesn't open them, but backlinks still count them. The vault panel
  keeps its title and status when all its lists are off. The default
  Markdown mode stays under **Preferences › Editor › Editor**; visual editing
  still asks first, keeps a draft of the source and changes the file only on
  save. The 10,000-note limit is fixed.
  ([#577](https://github.com/mikepadiernos/plasma-opus/issues/577))
- **📒 Workspace vault settings:** The **Workspace Vault** group under
  **Preferences › Applications › Editor** stores each workspace's vault
  switch, location and own Markdown options with the workspace after **Apply**
  or **OK**; **Restore Defaults** leaves them alone, and a duplicated
  workspace keeps the options but not the location. The location can only be
  one of the workspace's assigned saved locations, assigned with **Manage
  Workspaces…**; removed, remote and ambiguous ones can't be chosen, but the
  workspace's current one stays shown with why it can't be used. Turning the
  vault off keeps its location and options, and the editor uses the vault
  folder from the **Markdown** group. While the active workspace's vault
  location can't be found, the editor reports why rather than using another
  vault; a missing folder is read again when settings are applied or the
  vault is rescanned. The workspace's own options, the Markdown mode, preview
  dialect, preview on open, wiki links and vault panel lists, apply only
  while it is active with its vault on, and replace the **Markdown** group's
  options and the default Markdown mode for its notes. The group counts the
  vault's notes in the background, reporting the 10,000-file limit and
  skipped notes, but doesn't follow changes to the notes while it stays open.
  Changes not yet applied replace those made meanwhile to the same workspace
  in **Workspaces**, and saving stops with a report if another window changed
  the workspaces at the same moment. The **Backlinks**, **Links** and **Tags**
  lists are the only relationship views; there is no graph view. Notes and
  drafts are never changed by these settings.
  ([#566](https://github.com/mikepadiernos/plasma-opus/issues/566))
- **👁️ Preview editing settings:** The **Preview Editing** group under
  **Preferences › Applications › Editor** applies to the preview panel after
  **Apply** or **OK**; the compact Yazi-mode preview never offers editing.
  Editing in the preview always uses the built-in editor, whichever editor is
  chosen for **Edit**, and keeps the limits described under
  [Editing in the preview](#editor). Turning editing off hides **Edit** but
  leaves a file already being edited open until it is closed or moved to the
  editor panel. With **Start editing when the preview is double-clicked** on,
  a double-click in a text preview starts editing instead of selecting a
  word, opens the whole file at its start rather than at the clicked line,
  and reports files that can't be edited there, such as remote ones. With
  **Return to the preview after saving** off, a saved file stays open for
  editing until **Close Editor** or until another file is selected.
  ([#730](https://github.com/mikepadiernos/plasma-opus/issues/730))
- **🏢 Office settings:** **Office application** under **Preferences ›
  Applications › Office** chooses what **Edit** opens office documents in
  after **Apply** or **OK**; **First installed** keeps the order LibreOffice,
  Calligra, ONLYOFFICE, and **Open in Office Application** still offers
  every application. The tab marks the applications that aren't installed,
  checking `PATH` each time it is shown, so one installed only as a Flatpak
  shows as not installed. Calligra counts as installed when any of its
  Words, Sheets or Stage programs is, and the tab names the missing ones. A
  chosen application that is later removed, or that can't open a remote
  document, is reported by **Edit** rather than replaced. **Preview office
  documents in the preview panel** applies to the selection after **Apply**
  or **OK**; when it is off, office documents, including remote ones, which
  are then not downloaded, show a message instead of a viewer. The tab lists
  the viewer component that previews text documents, spreadsheets and
  presentations, checking one OpenDocument and one Microsoft Office type of
  each, but can't choose it: the preview uses the one that **File
  Associations** ranks first, which **File Associations…** opens. The
  container has no viewer components or System Settings, so the listed
  viewers and opening File Associations are verified there only by their
  absence ([Plasma check](TODO.md#office-settings-on-plasma-676)).
  ([#676](https://github.com/mikepadiernos/plasma-opus/issues/676))
- **🔤 Ebook and font settings:** The checkboxes under **Preferences ›
  Applications › Ebooks and Fonts** turn previews of ebooks and DjVu
  documents, comic books, help files and fonts on or off after **Apply** or
  **OK**; ebooks and DjVu documents share one checkbox. A format that is off
  shows a message in the preview panel instead, and remote files of that
  format are then not downloaded. The tab lists the viewer component that
  previews each kind, checking a few representative types, but can't
  choose it: the preview uses the one that **File Associations** ranks
  first, which **File Associations…** opens. **Sample text** is the text
  every font preview shows, and leaving it empty shows the translated
  default. Editing the text in a font preview changes the setting at once,
  also in an open Preferences window unless it was edited there as well, in
  which case **Apply** keeps the Preferences text. Sample text remembered by
  an earlier version becomes the setting the first time a font is
  previewed, unless one is already set. The tab reports when `kfontinst` is
  missing, checking `PATH` each time it is shown. The container has no
  viewer components or `kfontinst`, so the listed viewers are verified there
  only by their absence; they are found as in the **Office** tab.
  ([#661](https://github.com/mikepadiernos/plasma-opus/issues/661))
- **📺 Media settings:** **Audio and video playback** moved from
  **Preferences › Interface › Previews** to **Preferences › Applications ›
  Media**, and is still the setting that the backend button beside the
  playback controls changes. A choice applies after **Apply** or **OK**, or
  at once from the button, and open previews and the gallery reopen their
  file with the new backend, from the start, playing on if they were
  playing. A choice made with the button while Preferences is open is shown
  there unless the setting was changed there as well, in which case
  **Apply** keeps the Preferences choice. The tab says what plays audio and
  video: without mpv support in the build, **mpv** is marked as not built in
  and can't be chosen; with mpv chosen but no OpenGL, it says Qt Multimedia
  plays video, checking OpenGL once per session. It also reports whether the
  mpv application that **Open with mpv** starts is in `PATH`. The tab has no
  volume or other mpv options.
  ([#578](https://github.com/mikepadiernos/plasma-opus/issues/578))
- **🧳 Package settings:** The **AppImages**, **Flatpak** and **Snap**
  checkboxes under **Preferences › Applications › Packages** apply after
  **Apply** or **OK**. A type that is off is no longer recognized as a
  package, so **Application Package…** is disabled for it and the file opens
  like any other; an open **Application Package** window keeps describing
  it, without actions. They don't hide Flatpak or Snap from Discover, the
  **Application Packages** category in Places or the package tools
  themselves. Clearing **Offer to add AppImages to the application menu**
  hides **Add to Application Menu** but keeps **Remove from Application
  Menu** for entries already added, which the tab counts. Clearing **Read
  the AppImage's own name, icon and categories** adds AppImages with their
  file name and a generic icon without running their runtime or marking
  them executable; it doesn't change entries already added, and **Run**
  still runs the AppImage. The tab reports whether `flatpak`, `snap`,
  Discover and `appimageupdatetool` are in `PATH`, checking each time it is
  shown. In the container it is verified with the tools missing or
  simulated.
  ([#296](https://github.com/mikepadiernos/plasma-opus/issues/296))
- **🗜️ Archive settings:** The **Creation** group under **Preferences ›
  Archives › Defaults**, an existing page rather than a new **Applications**
  tab, sets only what the **Compress** dialog starts with; each archive can
  still be changed there. The settings apply from the next **Compress** after
  **Apply** or **OK**. The volume size, from 1 MiB, applies only when
  splitting is checked, here or in the dialog, and split archives keep the
  limits of [Split archives](#archives). Clearing **Offer a password for ZIP
  archives** hides the password fields, so ZIP archives are written
  unencrypted; there is no setting to require a password, and 7z archives
  are always written without one. The settings don't apply to **More Options
  in Ark…** or **File › Add to Archive…**, which use Ark's own options. The
  group reports whether Ark is installed and can write encrypted 7z archives,
  checking when Preferences opens. The container has no Ark, so only the
  missing-Ark report is verified there.
  ([#665](https://github.com/mikepadiernos/plasma-opus/issues/665))
- **📮 Mail client settings:** The **Client** group under **Preferences ›
  Applications › Mail** names the default mail client, the `mailto:`
  handler that KDE's application list resolves, and reports when none is set
  or when the client can't receive attached files, such as Plasma
  Impresario. It reads the client again each time the tab is shown, so it
  doesn't follow a change made in System Settings while the tab stays open.
  **Configure Default Applications…** opens Plasma's Default Applications
  settings, or reports that they aren't installed; Plasma Opus never changes
  the default itself. Inside a Flatpak or Snap sandbox the tab can't see the
  client and says that the desktop portal chooses it. Clearing **Offer Send
  by Email for selected files** hides **File › Send by Email…** and leaves it
  out of the context menus after **Apply** or **OK**; a **Send by Email**
  window that is already open stays usable. The container has no mail client
  or System Settings, so the client is verified with a fake one and opening
  Default Applications only by its absence
  ([Plasma check](TODO.md#mail-client-settings-on-plasma-666)).
  ([#666](https://github.com/mikepadiernos/plasma-opus/issues/666))
- **📤 Remote mail file settings:** **Remote files** in the **Remote Files**
  group under **Preferences › Applications › Mail** chooses whether **Send by
  Email** asks before downloading private local copies of remote files, such as
  files on network locations or inside archives, or skips them. Asking is the
  default and happens each time, in the **Send by Email** window; no choice
  downloads copies without asking. Skipped files are listed with the reason and
  can't be dragged into a composer either. Files on a network share mounted into
  a local folder count as local files. The setting applies to **Send by Email**
  windows opened after **Apply** or **OK**, and the group is disabled while
  **Offer Send by Email for selected files** is cleared.
  ([#667](https://github.com/mikepadiernos/plasma-opus/issues/667))
- **📬 Mail preview settings:** **Preview mail files (.eml) in the preview
  panel**, in the **Mail File Preview** group under **Preferences › Applications
  › Mail**, turns the message view of local `.eml` files on or off in the
  preview panel and in Yazi mode; both reload after **Apply** or **OK**. When
  it is off, mail files show as raw text, like other text files, including their
  headers and encoded attachments, cut at **Maximum text in preview panel**
  under **Preferences › Interface › Previews**. The limits themselves aren't
  settings: messages over 64 MiB aren't parsed, the text of a parsed message is
  cut at the same text limit, and remote messages always show as raw text. The
  setting doesn't change folder thumbnails or how mail files open.
  ([#668](https://github.com/mikepadiernos/plasma-opus/issues/668))
- **🔑 Remote connection settings:** The **Saved Connections** group under
  **Preferences › Remote and Cloud › Connections** is the editor of **Go ›
  Remote Connections › Manage Remote Connections…**, so **Save** and **Remove**
  take effect immediately, even if Preferences is cancelled, and removing a
  connection also forgets its mount options. Each keeps connections saved in the
  other meanwhile but lists them only when it is shown again, and editing the
  same connection in both keeps the last save. **Defaults for New Connections**
  sets the protocol, user and SSH sign-in method that new connections start
  with: in the tab as soon as they are edited, and in the **Manage Remote
  Connections** window after **Apply** or **OK**. Saved connections keep their
  own settings. The SSH sign-in method applies only when SFTP is chosen, and a
  user name that a connection can't use is reported and left out. There are no
  defaults for the host, port, folder or private key. Passwords are never stored
  in Plasma Opus's configuration or in URLs; KIO asks for them when needed and
  keeps them in KDE Wallet when you choose to. **Open KDE Wallet Manager…**
  opens KDE Wallet Manager to review or remove them, or reports that it isn't
  installed. The container has no KDE Wallet Manager or KDE password server, so
  saving passwords and opening the manager are unverified there ([Plasma
  check](TODO.md#remote-connection-settings-on-plasma-297)).
  ([#297](https://github.com/mikepadiernos/plasma-opus/issues/297))
- **📶 Network discovery settings:** **Show the shares and services discovered on
  the local network** under **Preferences › Remote and Cloud › Connections**
  shows or hides **Go › Network** in the Go menu and in **Go › Remote
  Connections**, together with its shortcut, after **Apply** or **OK**. It
  doesn't change what KIO discovers, and a `network:/` or `remote:/` location
  typed in the location bar, bookmarked or restored in a tab still opens. The
  Network place in the Remote section of the Places panel belongs to KDE's
  places shared with Dolphin, so the setting leaves it alone: hide it from its
  context menu, or hide the whole Remote section under **Preferences › Interface
  › Panels**. The group reports when no KIO worker lists the local network.
  ([#662](https://github.com/mikepadiernos/plasma-opus/issues/662))
- **🪣 rclone settings:** Under **Preferences › Remote and Cloud › rclone**,
  clearing **Offer the remotes configured in rclone in Go › Cloud Storage**
  hides the menu and its shortcut after **Apply** or **OK**, but remotes already
  mounted or served stay open until Plasma Opus quits, as the menu that stops
  them is hidden. Clearing a remote only leaves it out of the menu's list: cloud
  sync still offers it, and a cleared remote that is open still appears under
  **Unmount** or **Stop Serving**. The tab reads the remotes from rclone when it
  is shown or on **Refresh List**, and remotes added later are shown until they
  are cleared. **Open a remote** applies to remotes opened afterwards; an open
  remote keeps its mode until it is stopped. A served remote listens only on
  127.0.0.1, so other computers can't reach it, but other users of this computer
  can try its password. Only applications that use KIO open its files in place;
  others get a temporary copy, so mount remotes for them. Each remote has a port
  derived from its name; when that port is busy, another is used, and KDE's
  password server keeps a separate password for it, so KDE Wallet can hold
  several Plasma Opus entries for 127.0.0.1. The password is in rclone's
  environment, which other processes of the same user can read, and without KDE
  Wallet it lasts only until the password server stops. The container has no
  rclone or KDE password server, so serving a real remote and the password
  hand-off are tested only with a fake rclone ([Plasma
  check](TODO.md#rclone-settings-on-plasma-663)).
  ([#663](https://github.com/mikepadiernos/plasma-opus/issues/663))
- **🗄️ Remote drive settings:** The backends, mount folder and read-only choice
  under **Preferences › Remote and Cloud › Remote Drives** apply to connections
  whose **Mount Options…** were never saved; a connection with saved options
  keeps them. KIO FUSE ignores the mount folder, read-only and unmount settings:
  it chooses its own folder, can't mount read-only, and its mounts last until
  KIO FUSE stops. A backend the tab marks as not installed fails when a
  connection mounts, and davfs2 and CIFS still need an fstab entry for each
  mount folder, so choosing another folder needs new entries. A changed folder
  applies to new mounts, and a chosen folder isn't made private, so other users
  can open the mounts if its permissions allow. While **Start Plasma Opus in the
  system tray at login** is checked, each **Apply** or **OK** writes its
  autostart entry again, even if it was removed in Plasma's Autostart settings,
  and only connections chosen to **Mount when Plasma Opus starts** are mounted.
  Activating a workspace mounts a connection only when a Favorite or Place
  assigned to it is on the connection's server, port and folder, or inside the
  folder an sshfs, davfs2 or CIFS mount uses; failures appear only in the status
  bar and the Cloud Activity window. Clearing **Unmount sshfs, davfs2 and CIFS
  mounts when Plasma Opus quits** lets sshfs mounts started afterwards run on
  their own; sshfs mounts started before still end at quit, while davfs2 and
  CIFS mounts follow the setting when Plasma Opus quits. A mount left in place
  is taken over when Plasma Opus starts only if it is in the folder that its
  options and the current mount folder give, and a mount taken over is never
  unmounted at quit. A kept sshfs mount whose connection drops stays stale until
  it is unmounted. The container has no sshfs, FUSE, KIO FUSE or Plasma session,
  so mounting, keeping and taking over mounts and starting at login are tested
  only with fakes ([Plasma check](TODO.md#remote-drive-settings-on-plasma-704)).
  ([#704](https://github.com/mikepadiernos/plasma-opus/issues/704))
- **🔃 Cloud sync settings:** The pair list under **Preferences › Remote and
  Cloud › Sync** is the one in **File › Cloud Sync…**: adding, editing,
  removing, pausing and synchronizing pairs take effect immediately, even if
  Preferences is cancelled, and the list appears only when Preferences is opened
  from a Plasma Opus window. No supported remote reports changes, so every pair
  is checked on its interval, and each check of a network-folder pair lists both
  folders in full; a short default interval means more traffic for large
  folders. The default interval applies to pairs that use it, as new pairs do;
  pairs made before it existed keep their own interval until edited. A pair's
  conflict choice applies to background and **Sync Now** runs; the first run and
  large deletions still stop for review, and conflicts between a file and a
  folder still wait. **Keep Local Version** and **Keep Remote Version** replace
  the other version without a copy in the trash. rclone pairs pass the choice to
  `rclone bisync --conflict-resolve`, which needs rclone 1.66 or later and keeps
  the replaced version under a conflict name, while **Ask When Reviewing** and
  **Keep Both** keep both versions under bisync's conflict names without
  stopping for review. Pausing on metered connections works only where Qt's
  network information reports it, usually through NetworkManager; the tab says
  when it isn't reported, and background runs then aren't paused. Starting at
  login uses its own autostart entry beside the one **Remote Drives** writes;
  both start the same tray instance. While the setting is checked, each
  **Apply** or **OK** writes the entry again, even if it was removed in Plasma's
  Autostart settings, and an entry made from **File › Cloud Sync…** before the
  setting existed turns it on. The container has no Plasma session or
  NetworkManager, so starting at login and metered detection are tested only
  with a test-mode autostart folder and a fake check ([Plasma
  check](TODO.md#cloud-sync-settings-on-plasma-720)).
  ([#720](https://github.com/mikepadiernos/plasma-opus/issues/720))
- **🪞 rsync settings:** The **rsync Backend** group under **Preferences › Remote
  and Cloud › Sync** sets how **File › Compare and Synchronize Folders…**
  starts; a dialog already open keeps its backend, filters and copy policy,
  although its runs follow the current deletion setting. The comparison profile
  is loaded only when the dialog starts with rsync, for a folder on an SSH host
  or with **Synchronize local folders with rsync by default**; switching to
  rsync later keeps the dialog's filters and applies only the copy policy, and
  switching back applies the built-in default from **Tools › Folder Sync**. A
  deleted profile isn't loaded: the dialog reports it and the tab lists it as
  **Unavailable profile**. While deletions aren't allowed, rsync offers no
  **Delete** actions, planned deletions become **Skip** when switching to rsync,
  and plans with deletions are refused; once allowed, rsync's deletions are
  permanent and skip the trash. Turning rsync off also stops comparisons with
  folders on SSH hosts, which only rsync reaches. The dry run and confirmation
  can't be turned off, the copy speed limit under **Tools › Folder Sync** is
  shared with the built-in backend, and cloud sync pairs never use rsync. The
  container has rsync but no SSH host or ksshaskpass, so the settings are tested
  with local folders and an unreachable SSH address ([Plasma
  check](TODO.md#rsync-settings-on-plasma-366)).
  ([#366](https://github.com/mikepadiernos/plasma-opus/issues/366))
- **🚚 Transfer and cache settings:** The limits under **Preferences › Remote and
  Cloud › Transfers and Cache** are the ones in **File › Cloud Activity…**:
  applying Preferences updates the window, and a change in the window updates
  the settings, but Preferences, while open, keeps showing the values it opened
  with and writes them back when another change is applied. The limits apply
  only to rclone: to remotes that **Go › Cloud Storage** mounts or serves, when
  they are next mounted or served, and to rclone sync pairs at their next run;
  remotes already mounted keep their options until unmounted. KIO FUSE has no
  bandwidth, transfer or cache options, sshfs and CIFS mounts ignore the limits,
  davfs2 keeps its own cache settings in `davfs2.conf`, and cloud sync pairs
  with network folders and the transfer queue go through KIO without limits. The
  cache size applies to each mounted remote separately, so several mounts can
  use a multiple of it, and rclone can exceed it briefly while files are open.
  How long cached files are kept applies only with a cache size: files not
  opened for that long leave the cache at rclone's next check, even when it has
  room, while files waiting to upload stay. Hiding **Cloud Activity…** removes
  it only from the tray menu, which appears only while the tray icon is shown or
  pairs synchronize in the background; **File › Cloud Activity…** stays. The
  container has no rclone, FUSE or Plasma tray, so the limits are tested as the
  generated rclone options and the tray menu with a fake tray host ([Plasma
  check](TODO.md#transfer-and-cache-settings-on-plasma-721)).
  ([#721](https://github.com/mikepadiernos/plasma-opus/issues/721))
- **🏘️ LAN sharing settings:** The choices under **Preferences › Remote and
  Cloud › LAN Sharing** apply when **File › Share on Local Network…** opens;
  running shares and open windows keep their choices. Choices made in the window
  apply to that share only, and the window no longer remembers the last share's
  port, HTTPS, time limit or upload folder. Interfaces are matched by name, so
  an interface that is renamed or added later starts checked, and cleared
  interfaces that disappear stay cleared. Public addresses always start
  unchecked. With **Browsers only**, file managers can't browse folders but can
  still download a file through its link; with **File managers only (WebDAV)**,
  browsers get no folder pages or upload form but can still download files
  through their links. The HTTPS choice has no effect without `openssl`. A fixed
  port that is already in use stops sharing from starting, so only one share at
  a time can use it, and ports below 1024 normally need administrator rights.
  The time limit is 1 minute to 24 hours, with no unlimited choice. The upload
  folder must exist and be writable, and clients can browse it while uploads are
  allowed. The container has only loopback and its own network interface, so
  interfaces are tested with loopback and fake interfaces ([Plasma
  check](TODO.md#lan-sharing-settings-on-plasma-664)).
  ([#664](https://github.com/mikepadiernos/plasma-opus/issues/664))
- **🛂 MCP settings:** **Preferences › Security › MCP** sets what
  `opus-file-manager-mcp` lets MCP clients do, the folders they may use and the
  audit log for every client at once; only the `--read-only` option limits a
  single client. Running servers read the settings again for each request, so
  lowering the access or removing a folder applies to the next request, but a
  client lists the tools added by raising the access only after it restarts the
  server, which does not announce tool changes. There is no access token: any
  program running as you can start the server, and could read your files
  anyway, so the settings limit what clients are offered rather than what local
  programs can do. Folders that don't exist are skipped, and password-protected
  folders are refused even while unlocked in Plasma Opus. **Show Audit Log**
  selects the log in a new tab of the main window once a call is recorded; the
  log is never rotated or cleared.
  ([#294](https://github.com/mikepadiernos/plasma-opus/issues/294))
- **🔒 Password-gated folder settings:** **Preferences › Security ›
  Password-Gated Folders** protects, unlocks, locks, changes and removes
  password-protected folders at once in every window, so cancelling Preferences
  does not undo them. Its folder chooser is the standard KDE dialog, which does
  not know about the gates and lists the contents of locked folders, as do the
  other folder choosers in Preferences. A protected folder that no longer exists
  stays listed as missing, and its protection applies to a new folder created
  at the same path. The root folder, the home folder and the folders that
  contain it can't be protected, and the state and passwords apply only inside
  Plasma Opus; see [Password-gated folders](#password-gated-folders).
  ([#403](https://github.com/mikepadiernos/plasma-opus/issues/403))
- **🏦 Plasma Vault settings:** **Preferences › Security › Encryption**
  reads Plasma Vault's vaults when the tab is shown, so Plasma Vault and kded6
  may start then, and a cleared vault stays cleared by its encrypted data
  folder, even after it is renamed. Clearing a vault or the menu changes only
  Plasma Opus and hides the vault's messages from its status bar: the vault
  stays open or closed, and Plasma Vault's applet and other applications
  still list it. Leaving out the state removes it from the names and
  icons of open and closed vaults only; each vault's submenu still offers
  **Open Vault…** or **Close Vault** by its state, and errors and missing
  vaults are always shown. The development container has no Plasma Vault, so
  the real service awaits
  [verification on Plasma](TODO.md#plasma-vault-settings-on-plasma-669).
  ([#669](https://github.com/mikepadiernos/plasma-opus/issues/669))
- **🗝️ File encryption settings:** the **Files** group under **Preferences ›
  Security › Encryption** hides actions, not tools: GnuPG, Kleopatra and
  `aescrypt` stay installed, the information panel still names encrypted and
  signed files, and service menus that other applications add to the context
  menu are hidden separately under **Preferences › Context Menu › Service
  Menus**. The GnuPG setting also covers Kleopatra's two actions. The text
  format only chooses how GnuPG's **Encrypt** and **Sign** windows open, and
  windows already open keep their choice. The `.pgp` suffix applies only to
  binary files that Plasma Opus encrypts with GnuPG: text files end in `.asc`,
  signatures in `.sig` or `.asc` and AES Crypt files in `.aes`, and
  Kleopatra's own windows follow Kleopatra's settings. The decrypted mark is
  translated with the interface language and goes before the extension that
  the shared MIME database knows, such as `.tar.gz`, or else the last one, and
  a name that is taken still gets a number. The development container has no
  Kleopatra or `aescrypt`, so both await
  [verification on Plasma](TODO.md#file-encryption-settings-on-plasma-670).
  ([#670](https://github.com/mikepadiernos/plasma-opus/issues/670))

- **Recommendation:**
  - **Default file manager:** Install Plasma Opus with `./install.sh`, then
    choose **Set Plasma Opus as Default** and **Install Activation File**
    under **Preferences › KDE Plasma › File Manager**. If a running Dolphin
    keeps answering **Show in folder**, quit it and restart Plasma Opus. To
    hand folders back, choose another file manager with **Open Default
    Applications Settings**, then choose **Remove Activation File**.
  - **System file chooser:** In a Plasma session with xdg-desktop-portal
    1.21.0 or later, close open file dialogs, then choose **Set Up**, or
    **Complete Setup** after editing `kde-portals.conf`, under **Preferences
    › KDE Plasma › File Manager**. If the restart fails or applications still
    show KDE's chooser, log out and back in. Choose **Restore Previous
    Chooser** to go back.
  - **System file chooser start folder:** Set **Start folder** to **Folder
    of the last choice** under **Preferences › KDE Plasma › File Manager** to
    return to where each application last opened or saved files. Choose
    **Forget Remembered Folders** to clear the remembered folders, for
    example after reorganizing them.
  - **Operation notifications:** Clear the operations whose progress you do
    not want in Plasma under **Preferences › KDE Plasma › Notifications**,
    and follow them in their dialogs instead. Choose **Configure Plasma
    Notifications…** to change how Plasma shows the remaining reports, for
    example to keep finished operations out of the notification history.
  - **Start hidden in the tray:** Check **Start with the window hidden in the
    system tray** under **Preferences › KDE Plasma › System Tray** and add
    Plasma Opus under **System Settings › Autostart** to have it wait in the
    tray after login; choose its tray icon to show the window. Check **Hide
    the window in the system tray when it is closed** there as well to keep
    it running after closing the window.
  - **Global shortcut:** Choose the key combination under **Preferences ›
    KDE Plasma › Shortcuts**, and pick another one when the tab says no key
    is assigned. Clear **Show or hide Plasma Opus with a global shortcut**
    there if the key is needed elsewhere, and use the tray icon instead.
  - **Activity associations:** Check **Associate workspaces and default
    folders with Plasma Activities** under **Preferences › KDE Plasma ›
    Activities**, choose each Activity's workspace, behaviours and default
    folders in the list there, then choose **Apply**. Use **Configure Plasma
    Activities…** to add or rename Activities, and **Clear Association** to
    drop the association of an Activity that is not coming back.
  - **KDE Connect sharing:** Clear **Choose the device the files were last
    sent to again** under **Preferences › KDE Plasma › KDE Connect** when
    several people or devices share the computer, so each send starts from
    the first available device. Clear **Offer File › Send to Device… to send
    files with KDE Connect** if you send files with KDE Connect's own **Send
    to** entry instead, and use **Configure KDE Connect…** to pair devices or
    turn on file sharing for them.
  - **KRunner discovery:** Clear the workspaces and Favorites that should not
    appear in search results under **Preferences › KDE Plasma › KRunner**, for
    example private folders when the screen is shared, then start a new
    search. Clear **Offer saved workspaces and Favorites in KRunner and Plasma
    Search** to pause all results, and use **Configure Plasma Search…** to turn
    the Plasma Opus plugin off entirely.
  - **Online Accounts access:** Check **Show the folders of KDE Online
    Accounts in Plasma Opus** under **Preferences › KDE Plasma › Online
    Accounts** to browse account storage, and clear the accounts that should
    not appear in **Go › Online Accounts**, for example work accounts on a
    shared screen. Use **Manage Online Accounts…** to add accounts or sign in
    again, and remove an account there, not in Plasma Opus, to remove it from
    every application.
  - **Mount manager display:** Keep **Show only removable devices** checked
    under **Preferences › KDE Plasma › Mount Manager** for a short **Go ›
    Mount Points** menu of USB drives and discs, and use **File › Mount
    Manager…** for other disks. Clear **Hide pseudo, snap and other system
    file systems** only to check what is mounted where, and clear **Show the
    Mount Points menu on the main toolbar** if you reach devices from the
    Places panel instead.
  - **KWin script management:** Choose **Install and Enable** under
    **Preferences › KDE Plasma › KWin** in a Plasma session, and **Update** when
    it appears after upgrading Plasma Opus. Clear the behaviours you do not want
    under **Window Management**, for example **Place detached previews, editors
    and panes beside the main window** if you arrange windows yourself, and
    choose **Disable** to pause the script while keeping it installed. Use
    **Configure Window Rules…** for rules that should also apply to other
    applications.
  - **Drop-down window settings:** Check **Slide the window in from a screen
    edge and hide it again** under **Preferences › KDE Plasma › KWin**, choose
    a global shortcut that the tab reports as working, and keep **Screen** at
    **Active screen** if you move between monitors. Check **Hide when another
    application is activated** to use it like Yakuake, and clear **Slide in and
    out** to have the window appear and disappear at once.
  - **Baloo search settings:** Keep both options checked under **Preferences
    › KDE Plasma › Baloo** while the tab reports that file indexing is on, and
    use **Search Current Folder Live** when an indexed search misses recent or
    excluded files. Clear **Offer to search live after an indexed search** if
    you only search indexed locations, and clear the ranking option to keep a
    simpler search panel. Use **Configure File Search…** to change what Baloo
    indexes, and **Search Defaults…** for the scope, search folders and
    exclusions of Plasma Opus.
  - **File-type category settings:** Clear the categories you don't use
    under **Preferences › KDE Plasma › Baloo** rather than hiding them in
    Places, and check one again to restore its entry. Raise **Large files are
    over** to list only the biggest files, such as disk images and videos,
    when freeing space, and search with `size>` in the search panel for a
    one-off size.
  - **Terminal provider settings:** Choose the terminal you use most as
    **Separate terminal** under **Preferences › Applications › Terminal**,
    for example Yakuake for a drop-down terminal, or keep **Default
    Terminal** to follow **System Settings › Default Applications**. Pick one
    the tab doesn't mark as not installed, and use **Tools › Open Terminal
    With** for a one-off choice. Choose **Embedded terminal** there as well,
    then open a new session in the terminal panel.
  - **Atuin history settings:** Choose **The current session, or a new one
    when it is busy** under **Preferences › Applications › Terminal** to
    search history while a long command runs, or **A new session** to keep
    each search apart from your work. Clear **Offer Search Command History
    with Atuin** if you use Atuin's own key binding, such as Ctrl+R, in the
    shell instead, and change Atuin's history and sync settings with `atuin`
    and its `config.toml`.
  - **Text editor settings:** Keep **Built-in editor** under **Preferences ›
    Applications › Editor** to keep drafts, the Markdown preview and vault
    links, or choose the editor you use most, such as Kate, from those the
    tab doesn't mark as not installed, and use **Edit With** for a one-off
    choice. Choose **Separate terminal** for terminal editors if you prefer a
    full-sized window or the Konsole component isn't installed.
  - **Markdown settings:** Choose the vault folder under **Preferences ›
    Applications › Editor** or with the editor's **Vault** menu, not both
    while Preferences is open. Keep **Follow wiki links** on to move between
    notes, and turn off the vault panel lists you don't use to save room.
    Keep **Source** as the default Markdown mode under **Preferences › Editor
    › Editor** for notes with HTML or other formatting that visual editing
    can't keep.
  - **Workspace vault settings:** Give each project's workspace its own vault
    under **Preferences › Applications › Editor**, and turn it off rather
    than choosing **None** when you want the editor's own vault for a while.
    Give it its own options only where its notes differ, such as **Visual**
    mode for plain notes. If its location is reported as unavailable, add the
    folder back to Favorites or Places, or reconnect its drive; its notes and
    drafts are kept.
  - **Preview editing settings:** Keep **Offer Edit in text previews** on
    under **Preferences › Applications › Editor** for quick changes to notes
    and configuration files, and turn on **Start editing when the preview is
    double-clicked** if you edit previews more often than you select words in
    them. Turn off **Return to the preview after saving** when you save often
    while editing, and choose **Close Editor** when you're done. Turn editing
    off if you only read files in the preview; **Edit** still opens the
    editor chosen above.
  - **Office settings:** Choose the office application you use most under
    **Preferences › Applications › Office**, from those the tab doesn't mark
    as not installed, and use **Open in Office Application** for a one-off
    choice. Choose Calligra if you edit remote documents. Install Calligra
    for previews, raise its viewer under **File Associations…** if another
    one is listed first, and turn previews off on slow or metered network
    folders.
  - **Ebook and font settings:** Install Okular if **Preferences ›
    Applications › Ebooks and Fonts** says no viewer is installed, and turn
    off previews of formats you only read in Okular or that sit on slow or
    metered network folders. Set a sample text in your language, or with the
    characters you need to compare, there or in any font preview.
  - **Media settings:** Keep **Qt Multimedia** under **Preferences ›
    Applications › Media** unless the tab says mpv plays audio and video, and
    choose **mpv** for formats Qt Multimedia can't play. Install the mpv
    application for **Open with mpv** when you need volume, tracks or
    subtitles.
  - **Package settings:** Turn off the package types you don't use under
    **Preferences › Applications › Packages**, so their files open in their
    usual application, and install the tools the tab reports as missing for
    the rest. Clear **Read the AppImage's own name, icon and categories**
    before adding AppImages you haven't run yet, and add them again with it
    checked once you trust them.
  - **Archive settings:** Set the format and volume size you use most under
    **Preferences › Archives › Defaults**, such as 7z in volumes that fit your
    storage or upload limit, and change them in **Compress** for a one-off
    archive. Keep **Offer a password for ZIP archives** checked unless you
    never encrypt archives, and use **More Options in Ark…** for encrypted 7z
    archives when the group says Ark can write them.
  - **Mail client settings:** Check the client named under **Preferences ›
    Applications › Mail** before sending files, and choose one that accepts
    attachments, such as KMail or Thunderbird, with **Configure Default
    Applications…**. Clear **Offer Send by Email for selected files** if you
    attach files from your mail client instead.
  - **Remote mail file settings:** Keep **Ask before downloading private local
    copies** to attach remote files when you need them, and choose **Send only
    local files and skip remote files** if Send by Email should never download
    anything; then copy the remote files you need into a local folder first.
  - **Mail preview settings:** Keep **Preview mail files (.eml) in the preview
    panel** checked to read messages without opening them, and clear it when you
    need a message's raw headers, such as to check where it came from. Raise
    **Maximum text in preview panel** under **Preferences › Interface ›
    Previews** to read long messages.
  - **Remote connection settings:** Manage saved connections under **Preferences
    › Remote and Cloud › Connections** or in **Manage Remote Connections…**, and
    edit each connection in only one of them at a time. Set **Defaults for New
    Connections** to the protocol and user name you use on most servers, and
    choose **SSH key or agent** when your keys are loaded with `ssh-add`. Review
    saved passwords with **Open KDE Wallet Manager…** and remove those of
    connections you no longer use.
  - **Network discovery settings:** Keep **Show the shares and services
    discovered on the local network** checked on home and office networks with
    shares to browse, and clear it on untrusted networks or when you connect
    only through saved connections. If you clear it, also hide the Network place
    from its context menu in the Places panel.
  - **rclone settings:** Mount remotes when FUSE is installed and you open their
    files in applications that don't use KIO; serve them over WebDAV when FUSE
    is missing or mounting is blocked, such as in a sandbox. Clear the remotes
    you don't open from Plasma Opus, such as those used only by cloud sync, and
    stop open remotes before turning the integration off. When KDE Wallet asks
    to open as a remote is served, allow it so that the password is kept.
  - **Remote drive settings:** Keep KIO FUSE as the default backend, as it
    reuses passwords saved in KDE Wallet, and choose sshfs for SSH servers whose
    mounts must be read-only, in a fixed folder or kept after quitting. Choose a
    mount folder only when applications expect a fixed path, and make it
    readable only by you on a shared computer. Check **Start Plasma Opus in the
    system tray at login** only when connections are set to **Mount when Plasma
    Opus starts**, and assign a project's server locations to its workspace to
    mount them when it is activated. Clear **Unmount sshfs, davfs2 and CIFS
    mounts when Plasma Opus quits** only when applications use the mounts while
    Plasma Opus is closed, and unmount them before leaving the network.
  - **Cloud sync settings:** Keep **Ask When Reviewing** for pairs whose files
    are edited on both sides, and choose **Keep Both** to keep background runs
    going without losing either version; choose **Keep Local Version** or **Keep
    Remote Version** only when one side is authoritative, with a backup of the
    other. Use rclone 1.66 or later for rclone pairs. Keep the default interval
    at 15 minutes or more for large folders and give busy pairs their own
    shorter interval in **Edit…**. Check **Start Plasma Opus in the system tray
    at login to synchronize pairs in the background** to run pairs without
    opening a window, and keep pausing on metered connections checked on mobile
    data.
  - **rsync settings:** Keep deletions turned off and **Only replace older
    files** chosen unless one folder should mirror the other, and allow
    deletions only for folders you have backed up. Save a profile with the
    filters you use for SSH hosts in **File › Compare and Synchronize Folders…**
    and choose it as the comparison profile, and turn on rsync for local folders
    only when you want rsync's itemized dry run for them too.
  - **Transfer and cache settings:** Set a bandwidth limit below your upload
    speed when mounted remotes slow other traffic, and keep four or fewer
    parallel transfers for providers that limit requests. Size the offline cache
    for the files you reopen, divided among the remotes you mount at once, and
    keep cached files for 24 hours or more to work offline through a day. Change
    the limits with Preferences closed when using **File › Cloud Activity…**,
    then unmount and mount remotes again. For davfs2 mounts, set the cache in
    `davfs2.conf`.
  - **LAN sharing settings:** Keep HTTPS on and the time limit at 30 minutes or
    less, and clear VPN and virtual-machine interfaces so shares start only on
    your home or office network. Choose a fixed port above 1024 when a firewall
    needs a rule for it, and allow uploads by default only if you often receive
    files, into a dedicated folder rather than Downloads. Choose **Browsers
    only** when no one connects with a file manager.
  - **MCP settings:** Keep **Read files and folders** and add only the folders
    a client needs, such as one project folder, under **Preferences › Security
    › MCP**. Choose **Read and change files and folders** only while a task
    needs it, add `--read-only` to the configuration of clients that only
    read, and restart a client after raising the access. Keep the audit log on,
    check it with **Show Audit Log** and delete it when it grows large.
  - **Password-gated folder settings:** Check **Preferences › Security ›
    Password-Gated Folders** for folders left unlocked and choose **Lock All**
    before leaving the desktop. Choose **Remove Protection…** before deleting
    or moving a protected folder outside Plasma Opus, and keep confidential
    files in a Plasma Vault instead.
  - **Plasma Vault settings:** Clear the vaults you open only from Plasma
    Vault's applet under **Preferences › Security › Encryption**, and turn
    off the state while sharing your screen. Clear **Offer Plasma
    Vault's encrypted vaults in Go › Plasma Vaults** if you don't use Plasma
    Vault.
  - **File encryption settings:** Clear **Offer AES Crypt's encrypt and
    decrypt actions in File › Encryption** unless you exchange `.aes` files,
    and keep the GnuPG actions to check signatures. Choose **Marked as
    decrypted** when you decrypt files beside others you keep, so decrypted
    copies are easy to find and delete, and choose `.pgp` only when the people
    you send files to expect it.
  - Comment:
    [#759.7210936724](https://github.com/mikepadiernos/plasma-opus/issues/759#issuecomment-7210936724)
- **Proposed improvements:**
  - Remember the file manager that Plasma Opus replaced and offer to restore
    it, offer to remove a conflicting `kde-mimeapps.list` entry, and release
    the service when another default is chosen while Plasma Opus runs.
  - Check the xdg-desktop-portal version before setup, confirm that the
    restarted portal answers with Plasma Opus, and restart it without
    blocking the dialog.
  - Remember remote folders through KIO, keep separate folders for opening
    and saving, and keep remembering while **Home folder** is selected so
    that switching options keeps the last folders.
  - Report automatic library refreshes to Plasma with their own setting,
    and show Plasma's notification options for Plasma Opus in the tab.
  - Wait briefly for Plasma's tray to appear before showing the window at
    login, and offer to start Plasma Opus at login from the **System Tray**
    tab.
  - Name the application that already uses a key when KDE leaves the
    shortcut unassigned, and check the chosen key against Plasma Opus's own
    window shortcuts.
  - Clear all Activity associations at once, show the behaviours an
    association inherits from its workspace, and copy an association to
    another Activity.
  - Show KDE Connect's paired devices and the remembered one in the
    **KDE Connect** tab, with an action to forget it, and offer **Send to
    Device…** in the file context menu as an option.
  - Offer locations assigned to a workspace in the **KRunner** tab, show
    whether a workspace or Favorite is offered where it is managed, and
    forget the cleared ids of removed workspaces and Favorites.
  - Follow account changes while the **Online Accounts** tab is open, forget
    the cleared ids of removed accounts, and choose individual services of
    an account, such as only its Drive.
  - Remember the Mount Manager's removable-only choice as an option,
    recognize system file systems that Plasma Opus does not know yet from
    their mount flags, and list network mounts with removable devices as an
    option.
  - Follow changes made in KWin's script settings without **Refresh**, list
    virtual desktops before the script has run, and forget the assignments of
    desktops that were removed.
  - Choose the drop-down window's screen by its position as well as its
    output name, preview its size on the screen, and name the application that
    already uses a shortcut key when KDE leaves it unassigned.
  - Follow the indexer's state while the **Baloo** tab is open, show which
    folders KDE File Search indexes there, and offer ranking by path or
    modification time as well as by name.
  - Restore a category at its earlier position in Places, treat Places'
    **Hide** action like clearing the category, and offer criteria for the
    other categories, such as extra file extensions or a modification date.
  - Fall back to KDE's default terminal when the chosen one is removed, use
    the separate terminal for executables run in a terminal as well, and
    follow terminals installed or removed while the **Terminal** tab is open.
  - Close a session opened only for the search once it ends, place the chosen
    command at the prompt rather than printing it, and offer the search in a
    separate terminal as well as the embedded one.
  - Fall back to the built-in editor when the chosen editor is removed,
    choose an editor for each kind of text, such as Markdown, and close a
    separate terminal when its editor exits.
  - Follow a vault folder chosen in the editor while Preferences is open,
    keep a dialect chosen in the editor as the setting, and list recent vaults
    to switch between.
  - Assign a location to a workspace from the **Workspace Vault** group,
    follow changes to the vault's notes while the group is open, and show a
    graph of how notes link.
  - Start editing at the double-clicked line, offer editing in the Yazi-mode
    preview, and open the editor chosen for **Edit** from the preview.
  - Choose the office application for each kind of document, find Flatpak
    office applications, and choose the preview's viewer component in the
    **Office** tab rather than in File Associations.
  - Choose the viewer component for each kind of document in the **Ebooks
    and Fonts** tab, turn DjVu previews on or off apart from ebooks, and
    offer sample texts for other writing systems.
  - Keep the playback position when the backend changes, test drawing video
    with mpv from the **Media** tab, and add mpv's options there once the
    embedded player has volume, track and subtitle controls.
  - Choose where AppImage menu entries and icons are written, list and remove
    the added entries from the **Packages** tab, and turn off the Flatpak
    and Snap entries of the **Application Packages** category in Places with
    their actions.
  - Offer encrypted 7z archives in **Compress** itself, remember the last
    format, volume size and location as an option, and offer common volume
    sizes, such as the largest file a FAT32 drive holds.
  - Follow a mail client chosen in System Settings while the **Mail** tab is
    open, and list there which installed mail clients accept attachments.
  - Show the size of remote files before downloading copies, set a size limit
    for them, and remember the consent for trusted locations.
  - Show a message's raw source beside its parsed view without changing the
    setting, and a separate size limit for mail previews.
  - Follow changes made in the **Manage Remote Connections** window while the
    **Connections** tab is open, set a default port and private key for new SSH
    connections, and forget a removed connection's saved password with consent.
  - Hide the Network place in Plasma Opus's Places panel without hiding it in
    Dolphin, and offer the hosts discovered on the local network when adding a
    connection.
  - Stop open remotes when the integration is turned off, follow remotes added
    with `rclone config` while the **rclone** tab is open, choose how each
    remote opens, and offer **Stop Serving** in the Cloud Activity window.
  - Keep mounts after quitting for each connection, move mounts in place when
    the mount folder changes, let sshfs mounts started before keeping mounts was
    chosen stay, leave the login entry alone once it is removed in Plasma's
    Autostart settings, and notify when a workspace's connection fails to mount.
  - Watch local folders and ask remotes that offer change notifications, such as
    Nextcloud and rclone's polling backends, instead of checking on an interval;
    keep a trash copy of the version a conflict choice replaces; check the
    rclone version before offering a conflict choice for rclone pairs; and leave
    the cloud sync login entry alone once it is removed in Plasma's Autostart
    settings.
  - Load the chosen profile whenever the dialog switches to rsync, follow
    changes made in Preferences while the dialog is open, move rsync's deletions
    to the trash through a backup folder, and give rsync its own speed limit.
  - Apply new limits to running mounts through rclone's remote control instead
    of waiting for the next mount, follow changes made in **File › Cloud
    Activity…** while Preferences is open, show how much of the cache each
    remote uses, and limit the bandwidth of KIO FUSE mounts and KIO pairs.
  - Choose interfaces by the network they connect to rather than their name, try
    the next port when the fixed port is in use, report in Preferences when the
    port is already taken, and offer sharing until the window closes without a
    time limit.
  - Announce tool changes to running MCP clients when the access changes,
    choose the access and folders for each client, and rotate or limit the
    audit log and view it in Preferences.
  - Hide locked folders from the folder choosers in Preferences, offer to
    remove the protection of a missing folder, and set an idle auto-lock there.
  - Close or open a vault from Preferences, forget cleared vaults that Plasma
    Vault no longer has, and leave out vaults of other Activities.
  - Choose the default keys to encrypt for and sign with, decrypt into a
    chosen folder, and follow Kleopatra's own file name settings.

## Nextcloud integration

The M29 Nextcloud integration has these limits
([#767](https://github.com/mikepadiernos/plasma-opus/issues/767)):

- **🎫 Nextcloud accounts:** **Go › Nextcloud › Manage Nextcloud Accounts…**
  adds accounts only with Nextcloud's Login Flow in a web browser; there is
  no form for a user name and password. The app password goes to KDE's
  password server, `kpasswdserver`, filed under the account's WebDAV address
  and the realm that the server's theming name suggests, `Nextcloud` by
  default. KIO's WebDAV worker finds it only when that realm matches the
  server's `WWW-Authenticate` realm; otherwise KIO asks for the password once.
  Nextcloud's OCS endpoints send no authentication challenge, so the dialog's
  own requests send the app password themselves, through KIO, as a Basic
  `Authorization` header. Those requests verify the certificate without
  offering to accept an invalid one, so a self-signed server fails in the
  dialog. Unencrypted `http` is allowed only for servers on the same
  computer, and Login Flow is refused when the server's polling address is
  unencrypted or on another host, as behind some reverse proxies. Sign-in
  waits at most 20 minutes. Accounts from KDE Online Accounts are listed but
  their credentials aren't handed to KIO, so **Check** sees only what the
  server shows anonymously, and KIO asks for the password when their files
  open. Photos is detected by reading its WebDAV folder, collaborative tags
  and sharing from the capabilities, and file search is assumed from the
  `dav` app on Nextcloud 15 or later without trying a search. **Remove…**
  asks `kpasswdserver` to forget the app password; whether that also removes
  the KDE Wallet entry is unverified. Accounts aren't added to Places. The
  container has no Nextcloud server, KDE Wallet or Plasma session
  ([real-server check](TODO.md#nextcloud-accounts-on-a-real-server-588)).
  ([#588](https://github.com/mikepadiernos/plasma-opus/issues/588))
- **⛅ Nextcloud browsing:** Server permissions are read with a separate
  `PROPFIND` once a folder's listing finishes, so actions stay enabled until
  it answers, and changes made on the server apply only after the folder is
  reloaded. Only the folder shown and its direct items are checked: items in
  expanded subfolders and flat views aren't. Only the main window's actions
  and their shortcuts are disabled; drag and drop and other ways of starting
  an operation aren't checked, so the server refuses them and KIO reports
  each failed item, usually as a generic access error without Nextcloud's
  reason. When the permissions can't be read, for example when KDE's
  password server has no app password for an account from KDE Online
  Accounts, nothing is disabled. Accounts aren't added to Places. Checked
  only against a fake server
  ([real-server check](TODO.md#nextcloud-browsing-on-a-real-server-365)).
  ([#365](https://github.com/mikepadiernos/plasma-opus/issues/365))
- **🌄 Nextcloud previews:** Only the preview panel asks the server for
  previews. Thumbnails in the panes, the gallery and Yazi mode's preview still
  use KIO's thumbnailers or download the file, as for other remote folders.
  Each item's first preview waits for a `PROPFIND` that reads its file ID
  before the preview request. A server preview is a still image of at most
  2,048 pixels, so animations, later pages, video and audio need **Download
  Full Preview**, which applies the usual download limits. When the server
  has no preview, its preview is over 8 MiB or the request fails, the panel
  downloads the file as before, without saying why. The information panel
  reads metadata for each item it shows, including hovered items when it
  follows the pointer, and reads it again after a minute, or the time set
  under **Preferences › Remote and Cloud › Nextcloud**, so favourites and
  tags changed elsewhere can take that long to appear. Which
  `nc:metadata-*` properties exist depends on the server version and its
  apps; values are shown as the server sends them, without units, up to 60
  lines. Server tags appear only in the **Nextcloud server** section and are
  never written to Plasma Opus tag libraries. Checked only against a fake
  server and recorded Nextcloud 26 and 29 responses
  ([real-server check](TODO.md#nextcloud-previews-on-a-real-server-589)).
  ([#589](https://github.com/mikepadiernos/plasma-opus/issues/589))
- **🔖 Nextcloud search and tags:** **Search and Organize…** searches only
  file names, kinds and favourites, not photo metadata, EXIF values or
  content, and lists at most 500 results by default, or up to 5,000 as set
  under **Preferences › Remote and Cloud › Nextcloud**, newest first, without
  sorting them another way. Nextcloud doesn't report whether it supports
  WebDAV search, so it is assumed from Nextcloud 15 and a missing search shows
  as an error. Favourites and tag listings have no server-side limit, so the
  server sends every match before the list is cut to the limit. The context
  menu offers both favourite actions until the item's metadata has been read,
  and the panes don't mark favourites or show server tags. **Create Tag…**
  creates the tag on the server as soon as it is confirmed, so it remains
  when the dialog is cancelled. Changes made in the web interface appear
  after the next search or after a minute in the information panel. Checked
  only against a fake server and recorded Nextcloud responses
  ([real-server check](TODO.md#nextcloud-asset-search-on-a-real-server-590)).
  ([#590](https://github.com/mikepadiernos/plasma-opus/issues/590))
- **📸 Nextcloud Photos:** **Photos…** browses albums, shared albums and
  places in its own window, not in a pane, so pane actions, Plasma Opus tags
  and the gallery don't apply there; **Show in Pane** reveals one photo's
  file at a time, and only for photos in your own files. Each collection is
  read with one `PROPFIND` of depth 1, without paging, so a large album takes
  a while to list. Capture dates come from
  `nc:metadata-photos-original_date_time` or the EXIF `DateTimeOriginal`, and
  places from `nc:metadata-photos-place`; the server sets them only after
  indexing a photo, so other photos are listed last as unknown, without
  falling back to file dates. Places are read from `places/`, or from
  `locations/` when that is missing. Albums can be created but not renamed or
  deleted. Adding sends a `COPY` per file and counts a photo already in the
  album as added; removing sends a `DELETE` per photo, only for an entry
  directly in an album. Shared albums that don't report `oc:permissions` offer
  adding and removing, and the server's refusal is shown. Property names,
  entry names and shared-album permissions are unverified against a real
  server; checked only against a fake server
  ([real-server check](TODO.md#nextcloud-photos-on-a-real-server-671)).
  ([#671](https://github.com/mikepadiernos/plasma-opus/issues/671))
- **🤝 Nextcloud shares:** **Shares…** creates and changes only public links
  and shares with users and groups; email, federated, team and Talk shares
  are listed and can be revoked, but are managed in the web interface. User
  and group IDs must be typed exactly, as names aren't suggested, and labels,
  notes, hidden downloads and custom link tokens aren't offered. The editor
  enforces the server's link password, link expiry and upload rules, but not
  its expiry rules for user and group shares, which the server reports when
  it refuses a share. KIO doesn't pass on the server's response to a change or
  a revocation, so the list is read again afterwards to show the server's
  state. Each view is read with one request, without paging, and shares with
  you can't be declined or left here. **Send Link by Email…** only opens a
  message with the link, and only while **Offer Send by Email for selected
  files** is on; the password must be sent another way. Permission
  combinations, such as file drops and editable file links, and the server's
  error messages are unverified against a real server; checked only against
  a fake server
  ([real-server check](TODO.md#nextcloud-shares-on-a-real-server-591)).
  ([#591](https://github.com/mikepadiernos/plasma-opus/issues/591))
- **🖥️ Nextcloud desktop client:** **Go › Nextcloud › Desktop Client** reads
  only the client's configuration in `~/.config/Nextcloud/nextcloud.cfg` and
  its local socket in `$XDG_RUNTIME_DIR/Nextcloud/socket`, so the Flatpak and
  Snap builds of the client, ownCloud's client and a client with another
  configuration folder aren't found; the submenu is shown only when the
  configuration exists or the client is installed. Each folder's status is
  asked once each time **Go › Nextcloud** opens and isn't updated while the
  menu stays open. It is the folder's overall status: files' status, conflicts,
  the client's sign-in state and its error messages aren't shown, and a folder
  reached through a symbolic link can report **Unknown**. A folder with
  virtual files is called **Remote** even when some files are already
  downloaded, and **Offline** covers both a client that isn't running and a
  folder paused in the configuration, which the client may write only after a
  while. **Open on Server** needs an account added to Plasma Opus with the
  same server address and user. **Open Desktop Client** starts the client when
  it isn't running; synchronization is paused, resumed and set up only in the
  client. The configuration keys and socket messages follow the client's
  source and are checked only against a recorded configuration and a fake
  socket ([Plasma check](TODO.md#nextcloud-desktop-client-on-plasma-592)).
  ([#592](https://github.com/mikepadiernos/plasma-opus/issues/592))

- **Recommendation:**
  - **Nextcloud accounts:** Sign in under **Go › Nextcloud › Manage
    Nextcloud Accounts…** with an `https` address whose certificate the
    system trusts, and keep KDE Wallet enabled so the app password survives
    restarts. If KIO asks for a password when the files open, enter the
    Nextcloud password, or an app password when two-factor authentication is
    on, and let KDE Wallet remember it. Choose **Check** when files stop
    opening, and sign in again when it reports expired access. To remove an
    account, choose **Revoke and Remove** so its app password stops working.
  - **Nextcloud browsing:** Open files from **Go › Nextcloud**, and read a
    disabled action's tooltip as the server's answer: ask the share's owner
    for more access, or copy the items to a folder you own. Reload the folder
    after its sharing changes. If nothing is disabled in a read-only share,
    choose **Check** under **Manage Nextcloud Accounts…** and sign in again
    so the permissions can be read. Prefer the menu actions to drag and drop
    in shared folders, and read KIO's error report when some items fail.
  - **Nextcloud previews:** Keep the preview panel open to browse photos and
    documents without downloading them, and choose **Download Full Preview**
    only for the file you need to play or read in full. Show **View ›
    Information Panel** to check a file's owner, sharing, tags and photo
    metadata on the server; open the file in Nextcloud's web interface when a
    property you need isn't listed.
  - **Nextcloud search and tags:** Use **Search and Organize…** to find files
    by name across the account and to list favourites or tagged files, then
    **Show in Pane** to work with them. Narrow a search that reports only the
    first results, or raise the limit under **Preferences › Remote and Cloud ›
    Nextcloud** when narrowing it doesn't help. Check the tag list before
    choosing **Create Tag…**, since every user sees a new tag, and remove
    unwanted tags in Nextcloud's web interface. Use Plasma Opus tags for
    private labels and Nextcloud tags only for labels to share with other
    users of the server.
  - **Nextcloud Photos:** Use **Go › Nextcloud › _account_ › Photos…** to
    review albums and places by capture date and place, then **Show in
    Pane** to work with a photo's file. Use **Remove from Album…**, which
    never deletes the file, to tidy an album, and delete a file only from its
    folder in a pane. Before adding files to a shared album, read who the
    confirmation says will see them. Rename or delete albums and change their
    collaborators in the Photos app of the web interface, and wait for photos
    listed without a capture date to be indexed there.
  - **Nextcloud shares:** Share an item with **Nextcloud › Nextcloud
    Shares…** in its context menu, and read the confirmation's summary of who
    gets which access and until when before agreeing. Prefer links with a
    password and an expiry, send the password another way than the link, and
    use **Send Link by Email…** to review a link before writing to someone.
    Review **Shared by me** now and then and revoke links no longer needed.
    Manage email, federated and Talk shares, labels and notes in the web
    interface.
  - **Nextcloud desktop client:** Open synchronized folders from **Go ›
    Nextcloud › Desktop Client**, and read a folder's state before working
    without a connection: only **Local** folders hold full copies, while files
    in **Remote** folders open only when the server can be reached, unless the
    client has made them available offline. When a folder shows **Offline**,
    choose **Open Desktop Client** to start the client, and pause, resume or
    add folders there. Add the same account under **Manage Nextcloud
    Accounts…** to also open the server's copy, and keep two-way cloud sync
    pairs out of the client's folders.
  - Comment:
    [#767.7225982354](https://github.com/mikepadiernos/plasma-opus/issues/767#issuecomment-7225982354)
- **Proposed improvements:**
  - Hand the app password to KIO under the realm the server announces, sign
    in to accounts from KDE Online Accounts through their own credentials,
    offer to trust a self-signed certificate after showing it, confirm file
    search with a `SEARCH` request, and add accounts to Places.
  - Check drag and drop against the same permissions, read them for flat
    views and expanded subfolders, and show Nextcloud's own reason for a
    refusal from the server's error response.
  - Use server previews for the panes' thumbnails and the gallery, read the
    file ID from the folder listing to save a request, say why a server
    preview was unavailable, and refresh the metadata when the server's ETag
    changes.
  - Search photo metadata and tags on the server, page through results
    beyond the limit, sort them by any column, confirm search support with a
    trial request, mark favourites and server tags in the panes, and offer
    to delete a tag created in a cancelled dialog.
  - Browse albums in a pane as virtual folders, rename and delete albums with
    confirmation, page through large albums, fall back to file dates for
    photos without a capture date, and send album photos through **Send by
    Email** with a location warning.
  - Suggest users and groups from the server's sharee search, manage labels,
    notes, hidden downloads and email shares, enforce the server's expiry
    rules for user and group shares, read the server's response to a change
    directly, and accept or leave shares with you.
  - Find the Flatpak and Snap builds of the desktop client, update folder
    status while the menu is open, show files' status in the panes, report
    the client's sign-in state and errors, and tell downloaded virtual files
    from those still on the server.

## Nextcloud settings

The M30 Nextcloud settings have these limits
([#769](https://github.com/mikepadiernos/plasma-opus/issues/769)):

- **🎛️ Nextcloud settings:** **Preferences › Remote and Cloud › Nextcloud**
  doesn't list or edit accounts itself: **Manage Nextcloud Accounts…** opens
  the M29 dialog, and only when Preferences was opened from a main window.
  The **Accounts** group counts only the accounts saved in Plasma Opus and
  says whether KDE Online Accounts are read, without counting theirs. Changes
  apply to menus and panels from the next item shown, and to **Search and
  Organize…** and **Shares…** windows opened afterwards; open windows keep
  their result limit and link expiry. Turning favourites and tags off removes
  them only from the context menu; **Search and Organize…** still lists
  favourites and tags and changes favourites. Turning shares off hides
  **Shares…** and **Nextcloud Shares…** without changing existing shares.
  The link expiry applies only to new public links, not to user and group
  shares, and a server's enforced maximum wins. With server previews off,
  the preview panel downloads files within the usual limits, and folder
  views' thumbnails still follow **Show previews in remote folders** under
  **Interface › Previews**. With the permission check off, every action
  stays enabled and the server refuses what it doesn't allow. The
  **Desktop Client** group reads only the client's default configuration
  path, as the menu does. Checked only against fake servers, a recorded
  client configuration and a fake socket
  ([Plasma check](TODO.md#nextcloud-settings-on-plasma-368)).
  ([#368](https://github.com/mikepadiernos/plasma-opus/issues/368))
- **🏞️ Nextcloud Photos settings:** the **Photos** group only hides albums,
  shared albums or places in Plasma Opus; nothing changes on the server, and
  the Photos app of the web interface still lists them. Hiding your albums
  also removes **New Album…** from both windows, so albums can then be
  created only in the web interface. An album counts as shared when it is
  listed under **Shared albums** or the server reports collaborators for it;
  one of your albums shared without `nc:collaborators` in the server's
  response counts as unshared, so its changes aren't confirmed when
  confirmation is turned off. A grouping chosen in Preferences replaces the
  one last chosen in the window each time it opens, though **Group by** still
  changes it for that window. The settings apply to **Photos…** and **Add to
  Nextcloud Album…** windows opened afterwards; open windows keep theirs.
  Checked only against a fake server
  ([real-server check](TODO.md#nextcloud-photos-settings-on-a-real-server-672)).
  ([#672](https://github.com/mikepadiernos/plasma-opus/issues/672))

- **Recommendation:**
  - **Nextcloud settings:** Keep the defaults unless you need otherwise. Keep
    the permission check on when working in shared folders, and turn the
    server's previews off only to preview files exactly as stored. Raise
    **Search and Organize lists at most** only when narrowing a search
    doesn't help, since the server then sends more at once. Set **New public
    links expire after** so links end on their own, and still review each
    link's expiry before creating it. Turn the desktop client's status off
    when the client isn't used. Reopen **Search and Organize…** and
    **Shares…** windows after changing their settings.
  - **Nextcloud Photos settings:** Hide the kinds of collection you don't use
    to keep **Photos…** and **Add to Nextcloud Album…** short, and keep your
    albums shown when you create albums from Plasma Opus. Choose a fixed
    grouping only when you always browse photos the same way. Keep asking
    before album changes unless you often tidy albums only you use; changes
    to shared albums are always confirmed. Reopen **Photos…** and **Add to
    Nextcloud Album…** windows after changing these settings.
  - Comment:
    [#769.7229818538](https://github.com/mikepadiernos/plasma-opus/issues/769#issuecomment-7229818538)
- **Proposed improvements:**
  - List and edit accounts on the tab itself and count those from KDE Online
    Accounts, apply a new limit and expiry to open windows, offer a setting
    for the favourites and tags of **Search and Organize…**, apply a default
    expiry to user and group shares, and find the desktop client's
    configuration in its Flatpak and Snap builds.
  - Apply the Photos settings to open windows, offer **New Album…** while
    your albums are hidden, and read each album's sharing from the server to
    confirm changes to every shared album.

## Keyboard shortcuts

The M31 keyboard shortcut settings have these limits
([#771](https://github.com/mikepadiernos/plasma-opus/issues/771)):

- **🎹 Shortcut settings:** **Preferences › Keyboard › Shortcuts** summarizes
  the window's shortcuts; KDE's **Configure Keyboard Shortcuts…** still edits
  them, and the tab only lists, resets, imports and exports them. It needs
  Preferences opened from a main window and sees shortcut edits once they
  are saved. Pressing
  <kbd>Shift</kbd> twice, the keys of the Yazi mode presets and the gallery's
  keys aren't listed or editable there, though **Help › Keyboard Map…** lists
  them; the search panel's own keys and the pane toolbars' buttons are in
  neither. **Reset All Shortcuts…** keeps global shortcuts, which are
  changed on **Preferences › KDE Plasma** or in System Settings. Global
  shortcuts appear, and their conflicts with window shortcuts are found, only
  while KDE's global shortcut service runs; the container has none
  ([Plasma check](TODO.md#keyboard-shortcuts-on-plasma-230)).
  ([#230](https://github.com/mikepadiernos/plasma-opus/issues/230))
- **🎯 Shortcut contexts:** Every action in **Configure Keyboard Shortcuts…**
  applies to the whole window; it can't be limited to the file views, the
  folder tree or the gallery. Instead, the focused text field keeps a fixed
  list of typing and editing keys, including <kbd>Tab</kbd>, and lists and
  the folder tree keep the arrow keys, <kbd>Home</kbd>, <kbd>End</kbd>,
  <kbd>Page Up</kbd> and <kbd>Page Down</kbd>. That list follows KDE's
  standard editing keys, not a changed key scheme or the field's own extra
  keys, and only a shortcut's first key is checked. Default shortcuts such as
  <kbd>Space</kbd>, <kbd>Backspace</kbd> and <kbd>Delete</kbd> still run in
  file views and aren't reported. <kbd>Escape</kbd>, <kbd>F10</kbd> and
  <kbd>Menu</kbd> given to an action are reported but not kept back.
  Read-only text, combo boxes without a text field and the embedded terminal
  are left alone; the terminal keeps whichever keys it chooses. The gallery's
  keys are fixed. Keyboard layouts other than the container's are unverified
  ([Plasma check](TODO.md#shortcut-contexts-on-plasma-233)).
  ([#233](https://github.com/mikepadiernos/plasma-opus/issues/233))
- **💾 Shortcut storage:** Window shortcuts are kept in the `Shortcuts`
  group of `opus-file-managerrc`. Only shortcuts of the current KDE shortcut
  scheme are moved there from the layout file KDE's editor writes; other
  schemes aren't supported. An older Plasma Opus reads only the layout file,
  so it starts with the default shortcuts after a downgrade. When something
  else, such as an older version, writes shortcuts into the layout file
  again, they are moved at the next start and replace the stored ones. The
  Vim-style navigation keys are fixed, so only whether they are on is
  stored. Global shortcuts depend on KDE's
  global shortcut service; whether a Wayland desktop other than Plasma passes
  keys to it is guessed from `XDG_CURRENT_DESKTOP`, and a key KDE refuses is
  reported only after **Apply**. Neither can be checked in the container
  ([Plasma check](TODO.md#shortcut-storage-on-plasma-238)).
  ([#238](https://github.com/mikepadiernos/plasma-opus/issues/238))
- **🔀 Alternates and commands:** **Configure Keyboard Shortcuts…** shows
  and changes only an action's first two shortcuts; further alternates can
  be added only in the `Shortcuts` group of `opus-file-managerrc`, and
  **Preferences › Keyboard › Shortcuts** lists them. Saved commands are
  listed as **Saved Command:** and a name; a shortcut is kept with the
  command's name, so renaming a command loses it, and another open window
  lists a command saved elsewhere only when its own commands next change.
  The entries of **Open Terminal With**, **Edit With**, **Open With**,
  service menus and other submenus can't have shortcuts, nor can the pane
  toolbars' buttons. Real keyboards in Plasma are unverified
  ([Plasma check](TODO.md#alternate-and-command-shortcuts-on-plasma-231)).
  ([#231](https://github.com/mikepadiernos/plasma-opus/issues/231))
- **📇 Keyboard map:** **Help › Keyboard Map…** only shows keys; KDE's
  **Configure Keyboard Shortcuts…** still changes them. It lists the window's
  actions, pressing <kbd>Shift</kbd> twice, the gallery's keys, the chosen
  Yazi mode preset and the global shortcuts KDE has registered, but not the
  search panel's own keys, the keys text fields and lists keep, the embedded
  terminal's or dialogs' keys, or the pane toolbars' buttons. The Yazi mode
  keys are listed whether or not a pane uses Yazi mode. Descriptions are the
  actions' tooltips, so many actions have none. **Find by key** records keys
  only once it is clicked, and can't record <kbd>Shift</kbd> alone, so
  pressing <kbd>Shift</kbd> twice is found only by typing it; keys the
  desktop takes first, such as global shortcuts, may never reach it. The
  reference is exported only as an HTML page or an A4 PDF of the listed
  actions, without a print dialog or a choice of page size. Real keyboards,
  layouts and the Help menu on Plasma are unverified
  ([Plasma check](TODO.md#keyboard-map-on-plasma-232)).
  ([#232](https://github.com/mikepadiernos/plasma-opus/issues/232))
- **🕹️ Navigation shortcuts:** The actions added for moving between panes,
  tabs and panels, changing the selection and clearing the filter have no
  shortcut by default, and the extra keys on **Preferences › Keyboard ›
  Navigation** are off. When they are on, a key is left out only if another
  window action or a global shortcut Plasma Opus registered has it; KWin's,
  Plasma's and other applications' global shortcuts aren't checked, so the
  desktop may still take one first. While the embedded terminal has the
  focus, it keeps keys with a single modifier, so <kbd>Alt</kbd>+<kbd>1</kbd>,
  <kbd>Alt</kbd>+<kbd>2</kbd>, <kbd>Ctrl</kbd>+<kbd>Page Up</kbd> and
  <kbd>Ctrl</kbd>+<kbd>Page Down</kbd> don't leave it.
  <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>A</kbd> follows KDE's standard
  **Deselect**, whereas Dolphin uses it for **Invert Selection**. **Invert
  Selection** changes only the items shown, so in the details view it skips
  the contents of collapsed folders. **Focus Preview Panel** only says so in
  the status bar when the panel has nothing that takes the focus. When KDE's
  editor gives an extra key to another action, the extra key is dropped until
  it is free again. Real keyboards, layouts and the desktop's keys on Plasma
  are unverified
  ([Plasma check](TODO.md#navigation-shortcuts-on-plasma-236)).
  ([#236](https://github.com/mikepadiernos/plasma-opus/issues/236))
- **🧾 Keymap files:** **Import…** and **Export…** on **Preferences ›
  Keyboard › Shortcuts** read and write only Plasma Opus keymap files, not
  the shortcut scheme files of KDE's editor or Plasma Impresario's keymaps.
  An export lists only the actions whose shortcuts differ from their
  defaults, with the saved keyboard settings, and leaves out global
  shortcuts, so importing it keeps the other customized shortcuts unless
  **Give actions that the file doesn't list their default shortcuts** is
  chosen. A saved command's shortcut is matched by a hash of the command's
  name, so it is imported only where a command of the same name is saved;
  others are listed under **Not Imported**. Imported shortcuts are saved
  at once and can't be undone, whereas the file's keyboard settings wait for
  **Apply** or **OK** in Preferences and are lost on **Cancel**. The preview
  works out the extra navigation keys from the current setting; when the
  file also changes it, the extra keys are added, only where free, or
  removed after **Apply**, and the preview doesn't list them. The file
  dialogs and other open windows on Plasma are unverified
  ([Plasma check](TODO.md#keymap-files-on-plasma-237)).
  ([#237](https://github.com/mikepadiernos/plasma-opus/issues/237))
- **🪄 Command palette:** **Help › Command Palette…** searches only the
  menu bar's menus, the window's toolbars and the saved commands; actions
  that have only a shortcut, the pane toolbars' buttons, context menus,
  service menus, the gallery and Yazi mode keys aren't listed. Menus are read
  as they are when it opens: a menu that fills itself only as it is shown is
  filled when empty, but one with old entries may show them until it is
  opened. Only an action's first shortcut is shown. The recently used
  actions are KDE's: the last six are kept by their names in the `General`
  group of `opus-file-managerstaterc`, so renaming an action, a saved
  command or a language change loses them, actions of the same name share
  an entry, and they come first only until you type. KDE also adds **Command
  Bar: Clear History** to the list. Saved commands open in **Scripts and
  Custom Commands** instead of running. Whether the embedded terminal passes
  <kbd>Ctrl</kbd>+<kbd>Alt</kbd>+<kbd>I</kbd> on, other keyboard layouts,
  the palette's look and the screen reader on Plasma are unverified
  ([Plasma check](TODO.md#command-palette-on-plasma-761)).
  ([#761](https://github.com/mikepadiernos/plasma-opus/issues/761))
- **🐇 Vim-style navigation:** **Preferences › Keyboard › Vim Mode** and
  **View › Vim-Style Navigation** turn on a fixed set of Vim-style keys, off
  by default, in the file views of panes outside Yazi mode. The keys can't
  be changed or added to, a count only repeats movements, and there are no
  marks, registers, visual selection, operators such as `dd` or `:`
  commands. The folder tree, the places panel, the search panel and other
  lists don't use the keys, and a pane in Yazi mode uses its own preset.
  While the keys are on, their letters and the digits 1 to 9 no longer find
  items by their first characters, though other letters still do, and
  capital letters follow <kbd>Shift</kbd>, so <kbd>Caps Lock</kbd> changes
  them. Plain keys take over a window shortcut that uses them while a file
  view has the focus, whereas keys with <kbd>Ctrl</kbd>, <kbd>Alt</kbd> or
  <kbd>Meta</kbd> give way, so with the default shortcuts
  <kbd>Ctrl</kbd>+<kbd>D</kbd> stays **Add to Favorites**. Only the **Vim
  Mode** tab lists these conflicts, once shortcut edits are saved; the
  **Shortcuts** tab and the command palette don't include the keys. Input
  methods that compose text from keys typed in a file view, other keyboard
  layouts and screen readers on Plasma are unverified
  ([Plasma check](TODO.md#vim-style-navigation-on-plasma-228)).
  ([#228](https://github.com/mikepadiernos/plasma-opus/issues/228))
- **🚦 Vim-style modes:** While the Vim-style keys are on, each pane's
  indicator shows **NORMAL**, **FILTER** or **INSERT** only for the pane's
  own widgets: with the focus in the folder tree, another panel, the
  embedded terminal or a dialog, it shows **NORMAL**, and **INSERT** covers
  only the pane's text fields, such as the location bar or a rename editor.
  The modes are fixed: there is no visual or command mode, the indicator
  shows only the keys a sequence waits after, not the filter's text, and
  it can't be hidden while the keys are on. <kbd>Escape</kbd> keeps the
  filter only outside Yazi mode, and can't be set to clear it; press
  <kbd>\\</kbd> in the items for that. Only the pane being typed in announces
  a new mode, and moving between panes isn't announced. The indicator's look
  in Plasma themes and Orca's announcements are unverified
  ([Plasma check](TODO.md#vim-style-mode-indicator-on-plasma-229)).
  ([#229](https://github.com/mikepadiernos/plasma-opus/issues/229))
- **💬 Shortcut hints:** The hint popup lists only what can follow a
  Vim-style or Yazi mode sequence or count and the first key combinations of
  window shortcuts of several; it doesn't list the keys text fields, lists,
  the gallery, the search panel or the embedded terminal keep, or global
  shortcuts. It floats at the bottom of the active pane, even when the
  sequence was begun in another panel, and there is no sliding side pane.
  Qt matches window shortcut sequences without saying that one is pending,
  so Plasma Opus infers it from the key events it sees: a key combination
  held until it repeats, or a modifier released before its key in a sequence
  of three or more, can make the popup list the wrong keys or close early,
  though Qt still runs the shortcut typed. While a window sequence waits, Qt
  takes every key, so only scrolling turns the popup's pages; moving the
  focus ends Qt's sequence through a function Qt exports for its own tests,
  which a later Qt could change. Window shortcuts are grouped by their menu
  bar menu and described by their names, so actions in no menu are under
  **Other actions**. The popup's place, look and motion in Plasma
  themes and Orca's announcements are unverified
  ([Plasma check](TODO.md#shortcut-hints-on-plasma-234)).
  ([#234](https://github.com/mikepadiernos/plasma-opus/issues/234))
- **⏱️ Hint settings and reference:** **Preferences › Keyboard › Hints**
  sets one delay and one key sequence timeout for every kind of sequence;
  they can't differ between the Vim-style keys, Yazi mode and window
  shortcuts. The timeout now also ends window shortcuts of several key
  combinations, which Qt alone would keep waiting for: with the hints off, a
  slow second key combination finds the sequence cancelled, and the key runs
  on its own. Plasma Opus can time out only the window sequences it
  infers (see Shortcut hints), and ends Qt's sequence through the same
  function Qt exports for its tests. A delay as long as the timeout or
  longer is allowed, with a note, and then no hints show. **Help › Shortcut
  Hints** has no default shortcut, so it is reached from the Help menu, the
  command palette or a key you give it. It lists the keys for where the
  focus is when it opens and doesn't follow the focus; it leaves out the
  keys text fields, lists, the gallery and the search panel keep and global
  shortcuts, and works out which Vim-style or Yazi mode keys and window
  shortcuts win from their first key only. In a file view or the folder
  tree, the arrow keys turn its pages rather than moving through the items
  until it closes; elsewhere only scrolling turns them. The tab and the
  reference on Plasma are unverified
  ([Plasma check](TODO.md#shortcut-hint-settings-on-plasma-235)).
  ([#235](https://github.com/mikepadiernos/plasma-opus/issues/235))
- **🪧 Keyboard hints:** **Go › Keyboard Hints** labels only the active
  pane's visible files and folders, its location segments and other tabs,
  and the entries of a **Places** panel docked in the window; a floating
  Places panel, the folder tree, the other pane, the preview panel, toolbars
  and menus get no labels. In Yazi mode, only the current column is
  labelled, not the parent or preview column, and the location bar's
  segments that KDE folds into its drop-down in a narrow window aren't
  labelled. Labels are made of the Latin home-row letters of the chosen
  layout, so with a non-Latin keyboard layout typing a label cancels
  instead; input methods are turned off while labels show. A file or folder
  label can only select or open; it can't add to the selection. Any change
  that could move a target cancels the labels, including a folder that
  loads more items while they show. A full location segment is opened by
  sending it <kbd>Enter</kbd>, as KDE's location bar has no public way to
  activate one. The labels' place over the views, tabs, location bar and
  **Places** in Plasma themes, input methods, the focus returning on Wayland
  and Orca's announcements are unverified
  ([Plasma check](TODO.md#keyboard-hints-on-plasma-762)).
  ([#762](https://github.com/mikepadiernos/plasma-opus/issues/762))

- **Recommendation:**
  - **Shortcut settings:** Change shortcuts in **Configure Keyboard
    Shortcuts…**, then check **Preferences › Keyboard › Shortcuts** for
    conflicts; give one of the actions in each conflict another shortcut.
    Prefer key sequences, such as <kbd>Ctrl</kbd>+<kbd>K</kbd>,
    <kbd>Ctrl</kbd>+<kbd>C</kbd>, to taking a key another action uses, and
    don't give a single combination that begins one of your sequences to
    another action. Change global shortcuts where they are configured, and
    open Preferences from a Plasma Opus window when reviewing shortcuts.
  - **Shortcut contexts:** Give window actions shortcuts with <kbd>Ctrl</kbd>
    or <kbd>Alt</kbd>, a function key, or a key sequence, so they work
    wherever the focus is; plain letters, arrows and editing keys only run
    them outside text fields and lists. Leave <kbd>Escape</kbd>,
    <kbd>F10</kbd>, <kbd>Menu</kbd> and <kbd>Tab</kbd> to keyboard
    navigation, and press <kbd>Escape</kbd> or click a pane before using a
    plain-key shortcut from a text field.
  - **Shortcut storage:** Change window shortcuts only in **Configure
    Keyboard Shortcuts…**, with **Reset All Shortcuts…** or by importing a
    keymap, keep using the Default shortcut scheme, and copy
    `opus-file-managerrc` before downgrading. Check the status under each
    global shortcut after **Apply**, and choose another key when KDE didn't
    accept one. Outside a Plasma session on Wayland, give the command
    `opus-file-manager` a key in the desktop's own shortcut settings instead;
    it shows the running window but doesn't hide it.
  - **Alternates and commands:** Give a saved command a shortcut in
    **Configure Keyboard Shortcuts…** under **Saved Command:**, and give it
    again after renaming the command. For a terminal or editor from a
    submenu, choose it on **Preferences › Applications › Terminal** or
    **Editor** and use the shortcut of **Open Terminal Here** or **Edit**, or
    save a command that starts it. Add more than two shortcuts for an action
    only in the configuration file, and check them on **Preferences ›
    Keyboard › Shortcuts**, as KDE's editor doesn't show them.
  - **Keyboard map:** Before giving a key to an action, look it up in
    **Help › Keyboard Map…** with **Find by key** to see what it already
    runs and which sequences it begins. To find an action without a key,
    clear **Only actions with shortcuts**, then give it one in **Configure
    Keyboard Shortcuts…**. To print, filter the list to what you need and
    export a PDF, or export an HTML page and print it from a web browser on
    any page size. For the keys the map doesn't list, see
    [Where shortcuts apply](keyboard.md#where-shortcuts-apply).
  - **Navigation shortcuts:** Turn on the extra keys on **Preferences ›
    Keyboard › Navigation** and check each one's status there; for an action
    whose key wasn't added, or to move the focus out of the embedded
    terminal, give it a key with two modifiers or a key sequence in
    **Configure Keyboard Shortcuts…**. Look a key up in **Help › Keyboard
    Map…** before choosing it, and change it if Plasma or KWin takes it
    first. If you are used to Dolphin, give **Invert Selection**
    <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>A</kbd> yourself; **Deselect All**
    then loses its extra key. Expand folders in the details view before
    inverting the selection if their contents should change too.
  - **Keymap files:** Export your shortcuts before importing a keymap; to go
    back, import that file with **Give actions that the file doesn't list
    their default shortcuts** chosen. Read **Conflicts After Importing** and
    **Not Imported** before **Apply**, then choose **Apply** or **OK** in
    Preferences to keep the keyboard settings. Save commands under the same
    names on both computers before importing their shortcuts, and set global
    shortcuts where they are configured.
  - **Command palette:** Use **Help › Command Palette…**, or
    <kbd>Ctrl</kbd>+<kbd>Alt</kbd>+<kbd>I</kbd>, to find and run menu and
    toolbar actions by name, and give the ones you use often a shortcut in
    **Configure Keyboard Shortcuts…**. For actions it doesn't list, use the
    pane's toolbar or context menu, or look the key up in **Help › Keyboard
    Map…**. If the embedded terminal keeps
    <kbd>Ctrl</kbd>+<kbd>Alt</kbd>+<kbd>I</kbd>, click a pane first or give
    the palette another key. Turn off **List saved commands** on
    **Preferences › Keyboard › Command Palette** to keep them out of the
    list, and use **Clear History** there after renaming actions or changing
    the language.
  - **Vim-style navigation:** Before turning the keys on, read the
    **Conflicts** list on **Preferences › Keyboard › Vim Mode**; give an
    action whose plain-key shortcut they take a key with <kbd>Ctrl</kbd> or
    <kbd>Alt</kbd>, and use <kbd>J</kbd> and <kbd>K</kbd> while a window
    shortcut keeps <kbd>Ctrl</kbd>+<kbd>D</kbd> or
    <kbd>Ctrl</kbd>+<kbd>U</kbd>. Press <kbd>/</kbd> to filter by name, or
    turn the keys off with **View › Vim-Style Navigation** to find items by
    typing. Keep <kbd>Caps Lock</kbd> off and switch an input method to
    direct input while moving with the keys. For the same keys in a pane in
    Yazi mode, choose the Yazi-style keys on **Preferences › Yazi Mode ›
    Keyboard**.
  - **Vim-style modes:** Check a pane's indicator before typing: press
    <kbd>/</kbd> or <kbd>i</kbd> to filter, and <kbd>Escape</kbd> to return
    to the items with the filter kept, then <kbd>\\</kbd> to clear it. When
    the indicator shows **NORMAL** but keys don't move through the items,
    click the file view or press <kbd>Escape</kbd> in the field that has the
    focus. Use the filter bar or **Find by key** in the keyboard map rather
    than the indicator to see the filter's text or a key's command.
  - **Shortcut hints:** Pause after the first key of a sequence, such as
    <kbd>g</kbd> or <kbd>Ctrl</kbd>+<kbd>K</kbd>, to see what can follow it,
    and press <kbd>Escape</kbd> to cancel it. Turn the popup's pages with
    <kbd>Page Up</kbd> and <kbd>Page Down</kbd>, or scroll over it while a
    count or a window shortcut waits. Type window shortcuts of several key
    combinations without holding keys until they repeat; if the popup lists
    keys that don't fit, press <kbd>Escape</kbd> and type the shortcut again.
    For keys the popup doesn't list, use **Help › Keyboard Map…**.
  - **Hint settings and reference:** On **Preferences › Keyboard › Hints**,
    keep the hint delay shorter than the key sequence timeout; if you type
    sequences slowly with the hints off, choose a longer timeout or
    **Never**, and press <kbd>Escape</kbd> to cancel one. Give **Help ›
    Shortcut Hints** a key that **Find by key** in **Help › Keyboard Map…**
    shows is free, or run it from the command palette; press
    <kbd>Escape</kbd> to close it in a file view, and open it again after
    moving the focus. For keys it doesn't list, use the keyboard map.
  - **Keyboard hints:** Press <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>J</kbd>,
    or choose **Go › Keyboard Hints**, from a file view rather than a text
    field; press <kbd>Escape</kbd> first if the location bar or the filter
    bar has the focus. Choose your keyboard layout's home row under **Label
    keys** on **Preferences › Keyboard › Keyboard Hints**, and switch to a
    Latin keyboard layout before typing a label. Hold <kbd>Shift</kbd> on a
    label's last key to open the item, or choose **Open it** there if you
    mostly open. Wait for a large folder to finish loading before showing
    the labels, and dock the **Places** panel to label its entries. To
    select several items, type a label, then extend the selection with
    <kbd>Shift</kbd> and the arrow keys.
  - Comment:
    [#771.7231940238](https://github.com/mikepadiernos/plasma-opus/issues/771#issuecomment-7231940238)
- **Proposed improvements:**
  - Show the Yazi mode preset keys and the gallery's keys on the tab as well
    as in the keyboard map, review shortcuts before they are saved, and reset
    global shortcuts from the tab.
  - Let an action be limited to the file views, the folder tree or the
    gallery, read the keys text fields keep from the active key scheme, and
    let the gallery's keys be changed.
  - Store the shortcuts of every KDE shortcut scheme, ask KDE's global
    shortcut service whether a key is free before **Apply**.
  - Show every alternate in the editor, keep a command's shortcut when it is
    renamed, and give submenu entries such as each terminal or editor their
    own actions.
  - List the search panel's keys and the keys text fields and lists keep in
    the keyboard map, mark the Yazi mode keys while no pane uses Yazi mode,
    print through KDE's print dialog, and let **Find by key** take keys as
    soon as the map opens.
  - Check KDE's global shortcut service and KWin for clashes before adding
    an extra navigation key, offer keys the embedded terminal passes on, and
    let each extra key be turned on or chosen per action.
  - Read and write KDE's shortcut scheme files, include global shortcuts,
    undo an import, show the extra navigation keys a changed setting adds in
    the preview, and agree a shared keymap profile with Plasma Impresario.
  - List the pane toolbars' buttons, context and service menu entries and
    shortcut-only actions in the command palette, show every shortcut of an
    action, keep the recently used actions by action name rather than text,
    and keep them first among matches while typing.
  - Let the Vim-style keys be changed and extended, use them in the folder
    tree and other lists, list them on the **Shortcuts** tab and in the
    command palette, and add visual selection and operators such as `yy`
    and `p`.
  - Add visual and command modes, show the filter's text and the mode in the
    window's status bar, let the indicator be hidden, and let
    <kbd>Escape</kbd> in the filter bar be set to clear the filter.
  - Ask Qt to report pending window shortcut sequences rather than inferring
    them, let keys turn the popup's pages while one waits, list the keys
    text fields, lists, the gallery and the search panel keep, and offer the
    hints as a sliding side pane as well.
  - Set the delay and timeout per kind of key sequence, let the shortcut
    reference follow the focus and be searched, turn its pages with keys in
    text fields and the terminal, and offer a free default key for it if one
    is found.
  - Label the other pane, the folder tree, the preview panel, toolbars and
    floating **Places** panels, and the Yazi mode parent column; let a label
    add an item to the selection; follow the keyboard layout's own letters,
    including non-Latin layouts, rather than a fixed list; keep the labels
    through incremental loading by relabelling; and ask KDE for a public way
    to activate a location bar segment.

## Embedded terminal

The embedded terminal requires the Konsole or Kwi terminal component and has
these limits
([#321](https://github.com/mikepadiernos/plasma-opus/issues/321)):

- **Terminal component:** **Terminal Component** in the panel's menu, like
  **Embedded terminal** under **Preferences › Applications › Terminal**,
  applies to new sessions; open sessions keep the component they started
  with. When the chosen component isn't installed, the panel says so instead
  of loading the other. Kwi's component doesn't report its blur setting, so
  the panel isn't blurred behind it.
- **Atuin history search:** The search is typed into the shell of the
  current session, or of a new one when chosen under **Preferences ›
  Applications › Terminal**, so it needs the shell to be idle and to accept
  `atuin search --interactive`; a session running another program is
  refused unless a new session is chosen for it, but a prompt with
  half-typed text gets the search appended to it. The chosen
  command is printed, not placed at the prompt, unless the shell's own Atuin
  key binding is used. Plasma Opus only checks that the history database
  file exists, honouring `ATUIN_DB_PATH`, `ATUIN_DATA_DIR`,
  `ATUIN_CONFIG_DIR` and a top-level `db_path` in `config.toml`; Atuin can
  still fail to open it. The typed line can appear in shell history unless
  leading spaces are ignored. Kwi's component may not report a running
  program, so a busy Kwi session can receive the search.
  ([#364](https://github.com/mikepadiernos/plasma-opus/issues/364))

- **Working directory:** By default, new sessions start in the active local
  folder, or in the home folder with a visible notice for remote locations,
  and pane navigation does not change their directory.
- **Folder following:** Optional folder following instead switches between
  retained sessions or creates new ones; it does not change a running shell's
  directory. Remote or missing folders are reported without switching
  sessions.
- **Session lifetime:** Sessions accumulate until their tabs are closed.
  Hiding the panel keeps them running; closing the window ends them and can
  affect running programs.

- **Recommendation:** Install the Konsole terminal component for embedded
  sessions; on Arch Linux it is supplied by the `konsole` package. To use Kwi
  instead, install it and choose **Kwi** under the panel menu's **Terminal
  Component** or **Preferences › Applications › Terminal**, then open a new
  session. For remote locations, connect using a suitable terminal tool or
  use a separate terminal configured for that service. Check the terminal
  working directory before running commands, close unneeded session tabs,
  and finish or safely stop programs before closing a tab or the window.
  Before searching history with Atuin, clear the prompt and finish running
  programs, or choose a new session for the search under **Preferences ›
  Applications › Terminal**; to edit a chosen command at the prompt instead,
  use the key binding from `atuin init`, such as Ctrl+R, in the shell.
  - Comment: [#321.7022392302][comment-321]
- **Alternative:** Use a terminal multiplexer for independently retained
  sessions.
  - Comment:
    [#321.7029674455](https://github.com/mikepadiernos/plasma-opus/issues/321#issuecomment-7029674455)
- **Planned:** [M27](ROADMAP.md#m27-integrations) proposes separate SSH
  sessions for SSH profiles; it does not promise embedded remote working
  directories or persistent sessions after exit.
  - Comment: [#321.7022392302][comment-321]

## Settings backups

Settings backups have these limits
([#433](https://github.com/mikepadiernos/plasma-opus/issues/433)):

- **Encryption:** Backups are unencrypted archives.
- **Credential filtering:** Filtering recognizes certain key names and value
  patterns, but cannot guarantee that all sensitive data is removed; saved
  searches and other settings may still contain private text or paths. A
  matching value can instead exclude an entire settings entry.
- **Excluded data:** Current session tabs and expanded folder locations, saved
  custom commands and shared KDE settings are excluded; restoring preserves
  the corresponding current state rather than recovering it from the backup.

- **Recommendation:** Review backup contents privately before sharing or
  restoring, protect the archive with restrictive access and encryption, and
  avoid putting secrets in settings. Keep separate protected backups of excluded
  commands and other needed data; after restoration, verify important settings
  and recreate excluded items as needed.
  - Comment:
    [#433.7040727966](https://github.com/mikepadiernos/plasma-opus/issues/433#issuecomment-7040727966)

## Build, installation and references

For build requirements and installer behaviour, see
[Build and run](../README.md#build-and-run). For the scope of the upstream
inventories, see [Behaviour references](README.md#behaviour-references).

## Installing optional alternatives

These tools can be used independently for the workarounds above. Some are also
required by particular Plasma Opus features, as noted in their caveats; none is
a dependency of every feature. Install only what you need and check each tool's
format, platform and security limitations. For example, `difftastic` provides
`difft` for text comparison and `ncdu` provides `ncdu` for browsing directory
usage.

On Arch Linux, install an official package with `sudo pacman -S <package>`;
`paru -S <package>` can also install repository packages. With Homebrew, use
`brew install <formula>`. For the listed mise registry names,
`mise use -g <name>@latest` installs the tool and adds it to your global mise
configuration; ensure mise is activated in your shell. A dash means no mise
option is listed here, not that installation through another backend is
impossible.

| Tool          | Arch package          | Homebrew formula | mise name     |
| ------------- | --------------------- | ---------------- | ------------- |
| 7-Zip         | `7zip`                | `sevenzip`       | `7zip`        |
| rclone        | `rclone`              | `rclone`         | `rclone`      |
| fd            | `fd`                  | `fd`             | `fd`          |
| bat           | `bat`                 | `bat`            | `bat`         |
| Neovim        | `neovim`              | `neovim`         | `neovim`      |
| Difftastic    | `difftastic`          | `difftastic`     | `difftastic`  |
| Pandoc        | `pandoc-cli`          | `pandoc`         | `pandoc`      |
| ImageMagick   | `imagemagick`         | `imagemagick`    | `imagemagick` |
| FFmpeg        | `ffmpeg`              | `ffmpeg`         | -             |
| ExifTool      | `perl-image-exiftool` | `exiftool`       | `exiftool`    |
| ncdu          | `ncdu`                | `ncdu`           | -             |
| GNU coreutils | `coreutils`           | `coreutils`      | `coreutils`   |
| GNU diffutils | `diffutils`           | `diffutils`      | -             |
| GNU findutils | `findutils`           | `findutils`      | -             |
| ripgrep       | `ripgrep`             | `ripgrep`        | `ripgrep`     |
| jdupes        | `jdupes`              | `jdupes`         | -             |
| QDirStat      | `qdirstat`            | -                | -             |

Homebrew's GNU coreutils commands are normally prefixed with `g`, for example
`gdu` and `gsha256sum`; Homebrew's GNU diffutils commands likewise use names
such as `gdiff` and `gcmp`, and GNU findutils uses `gfind`. The mise `coreutils`
entry installs uutils coreutils, not GNU coreutils; check compatibility before
relying on GNU-specific behaviour. KDE components such as Ark, KParts and KIO
workers should be installed through the system's KDE packages, not treated as
standalone CLI replacements. The PDF engine used by Pandoc is a separate
dependency and varies with the chosen output method.

[issue-36]: https://github.com/mikepadiernos/plasma-opus/issues/36
[issue-255]: https://github.com/mikepadiernos/plasma-opus/issues/255
[issue-256]: https://github.com/mikepadiernos/plasma-opus/issues/256
[issue-259]: https://github.com/mikepadiernos/plasma-opus/issues/259
[issue-260]: https://github.com/mikepadiernos/plasma-opus/issues/260
[issue-261]: https://github.com/mikepadiernos/plasma-opus/issues/261
[issue-262]: https://github.com/mikepadiernos/plasma-opus/issues/262
[issue-263]: https://github.com/mikepadiernos/plasma-opus/issues/263
[issue-264]: https://github.com/mikepadiernos/plasma-opus/issues/264
[issue-265]: https://github.com/mikepadiernos/plasma-opus/issues/265
[issue-266]: https://github.com/mikepadiernos/plasma-opus/issues/266
[issue-267]: https://github.com/mikepadiernos/plasma-opus/issues/267
[issue-302]: https://github.com/mikepadiernos/plasma-opus/issues/302
[issue-303]: https://github.com/mikepadiernos/plasma-opus/issues/303
[issue-304]: https://github.com/mikepadiernos/plasma-opus/issues/304
[issue-307]: https://github.com/mikepadiernos/plasma-opus/issues/307
[issue-309]: https://github.com/mikepadiernos/plasma-opus/issues/309
[issue-311]: https://github.com/mikepadiernos/plasma-opus/issues/311
[issue-312]: https://github.com/mikepadiernos/plasma-opus/issues/312
[issue-313]: https://github.com/mikepadiernos/plasma-opus/issues/313
[issue-732]: https://github.com/mikepadiernos/plasma-opus/issues/732
[issue-755]: https://github.com/mikepadiernos/plasma-opus/issues/755
[comment-36]: https://github.com/mikepadiernos/plasma-opus/issues/36#issuecomment-7021740511
[comment-255]: https://github.com/mikepadiernos/plasma-opus/issues/255#issuecomment-7021741358
[comment-256]: https://github.com/mikepadiernos/plasma-opus/issues/256#issuecomment-7021742119
[comment-259]: https://github.com/mikepadiernos/plasma-opus/issues/259#issuecomment-7021744368
[comment-260]: https://github.com/mikepadiernos/plasma-opus/issues/260#issuecomment-7021745029
[comment-261]: https://github.com/mikepadiernos/plasma-opus/issues/261#issuecomment-7021745774
[comment-262]: https://github.com/mikepadiernos/plasma-opus/issues/262#issuecomment-7021746538
[comment-263]: https://github.com/mikepadiernos/plasma-opus/issues/263#issuecomment-7021748197
[comment-264]: https://github.com/mikepadiernos/plasma-opus/issues/264#issuecomment-7021749088
[comment-265]: https://github.com/mikepadiernos/plasma-opus/issues/265#issuecomment-7021749894
[comment-266]: https://github.com/mikepadiernos/plasma-opus/issues/266#issuecomment-7021750778
[comment-267]: https://github.com/mikepadiernos/plasma-opus/issues/267#issuecomment-7021751490
[comment-302]: https://github.com/mikepadiernos/plasma-opus/issues/302#issuecomment-7021752278
[comment-303]: https://github.com/mikepadiernos/plasma-opus/issues/303#issuecomment-7021753023
[comment-304]: https://github.com/mikepadiernos/plasma-opus/issues/304#issuecomment-7021753785
[comment-307]: https://github.com/mikepadiernos/plasma-opus/issues/307#issuecomment-7021754467
[comment-309]: https://github.com/mikepadiernos/plasma-opus/issues/309#issuecomment-7021755164
[comment-311]: https://github.com/mikepadiernos/plasma-opus/issues/311#issuecomment-7021755780
[comment-312]: https://github.com/mikepadiernos/plasma-opus/issues/312#issuecomment-7021756400
[comment-313]: https://github.com/mikepadiernos/plasma-opus/issues/313#issuecomment-7021757012
[comment-314]: https://github.com/mikepadiernos/plasma-opus/issues/314#issuecomment-7021757620
[comment-315]: https://github.com/mikepadiernos/plasma-opus/issues/315#issuecomment-7021758275
[comment-316]: https://github.com/mikepadiernos/plasma-opus/issues/316#issuecomment-7021845379
[comment-317]: https://github.com/mikepadiernos/plasma-opus/issues/317#issuecomment-7021904155
[comment-318]: https://github.com/mikepadiernos/plasma-opus/issues/318#issuecomment-7022199214
[comment-321]: https://github.com/mikepadiernos/plasma-opus/issues/321#issuecomment-7022392302
[comment-322]: https://github.com/mikepadiernos/plasma-opus/issues/322#issuecomment-7022490696
[comment-335]: https://github.com/mikepadiernos/plasma-opus/issues/335#issuecomment-7022647720
[comment-336]: https://github.com/mikepadiernos/plasma-opus/issues/336#issuecomment-7022793474
[comment-362]: https://github.com/mikepadiernos/plasma-opus/issues/362#issuecomment-7023269364
[comment-372]: https://github.com/mikepadiernos/plasma-opus/issues/372#issuecomment-7024683530
[comment-373]: https://github.com/mikepadiernos/plasma-opus/issues/373#issuecomment-7024944292
[comment-374]: https://github.com/mikepadiernos/plasma-opus/issues/374#issuecomment-7025134952
[comment-375]: https://github.com/mikepadiernos/plasma-opus/issues/375#issuecomment-7025250130
[comment-376]: https://github.com/mikepadiernos/plasma-opus/issues/376#issuecomment-7026216329
[comment-377]: https://github.com/mikepadiernos/plasma-opus/issues/377#issuecomment-7026633904
[comment-732]: https://github.com/mikepadiernos/plasma-opus/issues/732#issuecomment-7200937321
[comment-755]: https://github.com/mikepadiernos/plasma-opus/issues/755#issuecomment-7207460156
