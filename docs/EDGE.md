# Edge cases

[![Docs index](https://img.shields.io/badge/docs-index-3daee9?style=flat-square)](README.md)
[![Testing plan](https://img.shields.io/badge/testing-plan-1f6feb?style=flat-square)](refinements/06-testing.md)
[![Reported bugs](https://img.shields.io/badge/reported-bugs-cf222e?style=flat-square)](BUGS.md)
[![Caveats](https://img.shields.io/badge/caveats-limitations-bf8700?style=flat-square)](CAVEATS.md)

This inventory lists candidate test scenarios from a targeted review of
implemented workflows and representative tests on 2026-11-20. It is not an
exhaustive audit or a claim that existing tests are missing. Each entry links
to an `[edge]` issue; consult the
[project board](https://github.com/users/mikepadiernos/projects/3) for status.

> [!NOTE]
> These scenarios are test candidates, not confirmed bugs. Existing
> restrictions and workarounds are in [caveats](CAVEATS.md); confirmed reports
> belong in [reported bugs](BUGS.md).

> [!WARNING]
> Use temporary fixtures, synthetic credentials and isolated configuration,
> home and trash locations. Never test destructive operations against real user
> data, a live Baloo index or production remote services. Check existing tests
> before adding coverage.

## Find an edge case

| Area                                                                             | Issues                                                                                        |
| -------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------- |
| [Navigation and sessions](#navigation-and-sessions)                              | [#494][issue-494], [#495][issue-495], [#496][issue-496], [#497][issue-497]                    |
| [File operations and transfers](#file-operations-and-transfers)                  | [#498][issue-498], [#499][issue-499], [#500][issue-500], [#501][issue-501]                    |
| [Rename, duplicates and synchronization](#rename-duplicates-and-synchronization) | [#502][issue-502], [#503][issue-503], [#504][issue-504], [#505][issue-505], [#506][issue-506] |
| [Archives, checksums and split files](#archives-checksums-and-split-files)       | [#507][issue-507], [#508][issue-508], [#509][issue-509], [#510][issue-510]                    |
| [Search and filters](#search-and-filters)                                        | [#511][issue-511], [#512][issue-512], [#513][issue-513], [#514][issue-514]                    |
| [Preview, editing and metadata](#preview-editing-and-metadata)                   | [#515][issue-515], [#516][issue-516], [#517][issue-517], [#518][issue-518]                    |
| [Settings, backups and commands](#settings-backups-and-commands)                 | [#519][issue-519], [#520][issue-520], [#521][issue-521], [#522][issue-522]                    |
| [Interface and environment](#interface-and-environment)                          | [#523][issue-523], [#524][issue-524], [#525][issue-525], [#526][issue-526]                    |

Every scenario has the same structure: **Exercise** describes the conditions
to set up, **Verify** the expected outcome to check, and an optional **Note**
records a known risk or precaution. Each area ends with the relevant source,
tests and caveats.

## Navigation and sessions

- **Invalid saved tab before the active tab** · [#494][issue-494]
  - **Exercise:** Restore tabs with an invalid URL before a valid active tab.
  - **Verify:** The intended selection, along with pinned, locked and named
    tabs.
  - **Note:** `sanitizePane` removes invalid URLs but clamps the original
    active index against the shortened list, so it can select a different
    surviving tab.
- **Unavailable locations** · [#495][issue-495]
  - **Exercise:** Restore local folders that were deleted or unmounted and
    remote tabs that are offline or require authentication.
  - **Verify:** Errors are visible, and other tabs and saved layouts survive.
- **Linked folder boundaries** · [#496][issue-496]
  - **Exercise:** Navigate through symlinks to locations inside and outside the
    linked roots, or remove a corresponding folder after linking.
  - **Verify:** Navigation cannot escape the roots, and a missing target does
    not unexpectedly move the other pane.
- **Changing tabs during loading** · [#497][issue-497]
  - **Exercise:** Switch, close or move tabs while a folder listing or history
    selection is still loading.
  - **Verify:** Late results do not change the wrong tab, pane, selection or
    scroll position.

**References:** [session](../src/core/session.cpp),
[linked navigation](../src/core/linkednavigation.cpp),
[pane](../src/app/pane.cpp) and [session tests](../tests/sessiontest.cpp).

## File operations and transfers

- **Conflicts and aliases** · [#498][issue-498]
  - **Exercise:** Copy or move items onto existing names, including dangling
    links, case differences, hard links and a source that resolves to the
    destination.
  - **Verify:** Each conflict policy, and that the operation reports what
    actually happened.
  - **Note:** **Create Symbolic Link** refuses a name that already exists in
    the chosen folder, including a dangling link, and never replaces an item
    that appears after the dialog closes. It refuses links between local and
    remote locations, between hosts and in locations whose KIO worker can't
    create links. Relative targets are computed from the resolved local
    folders, so they work through linked folders. On a remote host they are
    computed from the paths as shown, so a remote folder reached through a
    link on the server can make a relative link point elsewhere; absolute
    targets, the default, are unaffected.
    [Symbolic-link tests](../tests/symlinkstest.cpp) and
    [dialog tests](../tests/symlinkdialogtest.cpp) cover these; remote links
    are only planned against the SFTP worker, so check creation on a test SFTP
    server manually.
- **Partial queued work** · [#499][issue-499]
  - **Exercise:** Cancel a waiting, paused or running transfer, or disconnect a
    test destination mid-copy.
  - **Verify:** The final queue state, partial output and subsequent jobs.
  - **Note:** Do not assume cancellation rolls back KIO work.
- **Concurrent destination changes** · [#500][issue-500]
  - **Exercise:** Create or replace a destination after a transfer is queued
    but before it starts.
  - **Verify:** The selected conflict policy and any confirmation still apply
    to the actual destination.
- **Queue lifecycle** · [#501][issue-501]
  - **Exercise:** Change concurrency while jobs are paused or finishing, close
    the queue dialog, and clear finished jobs.
  - **Verify:** Running jobs remain controlled, and failures are not displayed
    as successes.

**References:** [file operations](../src/app/fileoperations.cpp),
[transfer queue](../src/core/transferqueue.cpp),
[symbolic links](../src/core/symlinks.cpp),
[queue tests](../tests/transferqueuetest.cpp) and
[transfer caveats](CAVEATS.md#transfer-queue).

## Rename, duplicates and synchronization

- **Rename preview becomes stale** · [#502][issue-502]
  - **Exercise:** Replace a source or create a destination after preview,
    including a dangling symlink or a name differing only in case.
  - **Verify:** Existing files are not overwritten, and completed earlier
    renames are reported if the operation stops.
- **Name boundaries** · [#503][issue-503]
  - **Exercise:** Preview empty, very long, Unicode-normalization, leading-dot
    and newline-containing names, as well as a folder and its child in the same
    selection.
  - **Verify:** Validation, and the actual filesystem's naming rules.
  - **Note:** Do not assume all filesystems behave alike.
- **Duplicate selection changes** · [#504][issue-504]
  - **Exercise:** Modify, replace or remove a candidate after scanning but
    before removal; include hard links and equal timestamps.
  - **Verify:** Revalidation and selection do not remove the last intended
    copy.
- **Stale sync plan** · [#505][issue-505]
  - **Exercise:** Change a source, target or parent directory after
    comparison, including replacing a directory with a symlink.
  - **Verify:** Stale actions are rejected, and completed actions and partial
    results are accurately reported.
- **Overlapping or aliased roots** · [#506][issue-506]
  - **Exercise:** Compare nested roots, symlinked roots and files sharing an
    inode.
  - **Verify:** Unsupported combinations are refused, and neither root is
    unintentionally overwritten.

**References:** [batch rename](../src/core/batchrename.cpp),
[duplicate finder](../src/core/duplicatefinder.cpp),
[folder sync](../src/core/foldersync.cpp),
[rename tests](../tests/batchrenametest.cpp) and
[sync tests](../tests/foldersynctest.cpp).

## Archives, checksums and split files

- **Unsafe archive entries** · [#507][issue-507]
  - **Exercise:** In disposable archives, use traversal and absolute names,
    duplicate entries, links, existing destination entries and malformed or
    truncated data.
  - **Verify:** Extraction stays within its destination, reports failures and
    does not silently overwrite files.
- **Protected archive failures** · [#508][issue-508]
  - **Exercise:** Try wrong passwords, unsupported entry types, missing
    optional tools and cancellation or write failure.
  - **Verify:** Error messages, and whether partial output remains.
  - **Note:** Encrypted ZIP entry names are not secret.
- **Checksum manifest boundaries** · [#509][issue-509]
  - **Exercise:** Use escaped newlines and backslashes in filenames, invalid
    UTF-8, malformed hashes, oversized lines, excessive entry counts and paths
    outside the manifest directory.
  - **Verify:** Rejected entries cannot cause an unrelated file to be read or
    overwritten.
- **Split and combine interruptions** · [#510][issue-510]
  - **Exercise:** Test empty files, exact part boundaries, missing or reordered
    parts, an existing output, a full destination and cancellation during
    writing.
  - **Verify:** Output ordering, error reporting and identification of
    incomplete files. Complete outputs, including archive volumes, get the
    normal creation permissions for the umask; incomplete ones stay
    owner-only.
  - **Note:** [Split tests](../tests/filesplittertest.cpp) and
    [archive tests](../tests/archivewritertest.cpp) cover output permissions
    under several umasks.

**References:** [archives](../src/core/archives.cpp),
[checksum handling](../src/core/checksums.cpp),
[split and combine](../src/core/filesplitter.cpp),
[archive tests](../tests/archivewritertest.cpp) and
[split tests](../tests/filesplittertest.cpp).

## Search and filters

- **Search limits and encodings** · [#511][issue-511]
  - **Exercise:** Produce results at and beyond the 5,000-result and
    200,000-scanned-entry limits, and content at and beyond the 2 MiB limit.
    Include UTF-8, invalid UTF-8 and binary files.
  - **Verify:** Limits, skipped files and incomplete results are visible.
- **Filters and traversal** · [#512][issue-512]
  - **Exercise:** Combine maximum depth, excluded subtrees, hidden entries,
    unreadable directories and symlink loops.
  - **Verify:** Exclusions are applied consistently, and errors are
    distinguished from no matches.
- **Index availability and staleness** · [#513][issue-513]
  - **Exercise:** Use an isolated Baloo fixture with disabled, incomplete or
    stale indexing and missing or unreadable index data. Search indexed types
    and dates for files changed or renamed since indexing, and mix indexed and
    unindexed workspace locations.
  - **Verify:** Notices and cancellation.
  - **Note:** Do not assume indexed results are complete, and check that a
    different search scope is not silently substituted.
- **Changing search state** · [#514][issue-514]
  - **Exercise:** Cancel a search while results arrive, start a new search or
    hide the search panel; delete or move a saved collection's referenced
    files.
  - **Verify:** Stale results are not attributed to the new search, and
    collection operations do not modify referenced files.

**References:** [live search](../src/core/livesearch.cpp),
[indexed search](../src/core/indexedsearch.cpp),
[search widget](../src/app/searchwidget.cpp),
[live-search tests](../tests/livesearchtest.cpp) and
[index tests](../tests/indexedsearchtest.cpp).

## Preview, editing and metadata

- **Preview boundaries** · [#515][issue-515]
  - **Exercise:** Test text truncated inside a multibyte character, invalid
    encoding, images at byte and pixel limits, corrupt files and remote URLs.
  - **Verify:** Truncation, unsupported content and errors remain distinct.
- **Changing or interrupted edits** · [#516][issue-516]
  - **Exercise:** Change a file externally while it is open, interrupt saving,
    or switch documents with unsaved visual Markdown edits.
  - **Verify:** Conflict handling, draft recovery and preservation of both the
    source file and unsaved content.
- **Metadata limits and concurrent edits** · [#517][issue-517]
  - **Exercise:** Use malformed or oversized `.mfm.json`, unknown fields and
    simultaneous changes from another process.
  - **Verify:** Errors are reported. Edits keep unknown keys, entries and
    format fields, and an edit that would exceed 4,096 bytes or replace an
    unsupported value is refused with the file unchanged. Key order and
    whitespace are not kept.
  - **Note:** [Folder metadata tests](../tests/foldermetadatatest.cpp)
    cover preserved data, refused writes, the size limit and validated reads;
    simultaneous changes from another process remain manual.
- **Media and optional tools** · [#518][issue-518]
  - **Exercise:** Test stale metadata previews, linked files, unsupported
    formats and missing, failing or incompatible ExifTool and image codecs.
  - **Verify:** Failures and any possible partial modifications are reported.
  - **Note:** Batch conversion never replaces an existing file. Where the
    destination doesn't support atomic no-replace renames, it publishes with
    a hard link; where it doesn't support hard links either, such as FAT,
    exFAT and some FUSE filesystems, it copies into a newly created file.
    During that copy, a partly written, owner-only file briefly exists under
    the final name, and a failed copy removes only the file it created.
    [Publish tests](../tests/filepublishtest.cpp) and
    [conversion tests](../tests/imageconversiontest.cpp) simulate these
    filesystems; check a real FAT or FUSE destination manually.

**References:** [preview](../src/core/previewcontent.cpp),
[editor](../src/app/editorpanel.cpp),
[folder metadata](../src/core/foldermetadata.cpp),
[folder metadata tests](../tests/foldermetadatatest.cpp),
[media metadata](../src/core/metadataeditor.cpp),
[image conversion](../src/core/imageconversion.cpp),
[file publication](../src/core/filepublish.cpp),
[preview tests](../tests/previewcontenttest.cpp) and
[metadata tests](../tests/metadataeditortest.cpp).

## Settings, backups and commands

- **Interrupted restore** · [#519][issue-519]
  - **Exercise:** Use an isolated configuration to simulate a corrupt backup,
    an unwritable destination or a failure between restoring settings and
    layout.
  - **Verify:** Errors, rollback and pending-restore state are clear.
- **Sensitive data in backups** · [#520][issue-520]
  - **Exercise:** Use synthetic secrets in varied key names, values and URLs.
  - **Verify:** The actual backup and restored configuration.
  - **Note:** Heuristic filtering is not a guarantee of secret removal.
- **Command argument boundaries** · [#521][issue-521]
  - **Exercise:** Use disposable helper programs and paths containing spaces,
    quotes, Unicode, leading dashes and line breaks.
  - **Verify:** Placeholder expansion preserves argument boundaries, and remote
    selections or missing required selections are rejected where unsupported.
- **Configuration changed elsewhere** · [#522][issue-522]
  - **Exercise:** Open settings in multiple windows and change shared KDE or
    immediately applied data while another dialog has pending edits.
  - **Verify:** Cancel and save failures do not imply changes were reverted
    when they were not.

**References:** [settings backup](../src/core/settingsbackup.cpp),
[custom commands](../src/core/customcommands.cpp),
[backup tests](../tests/settingsbackuptest.cpp),
[command tests](../tests/customcommandstest.cpp) and
[backup caveats](CAVEATS.md#settings-backups).

## Interface and environment

- **Responsive and accessible controls** · [#523][issue-523]
  - **Exercise:** Use narrow windows, large text, high DPI, right-to-left
    layouts, long translations and keyboard-only navigation.
  - **Verify:** Focus, labels, both panes and essential actions remain usable
    without discarding customized layouts.
- **Terminal availability and lifecycle** · [#524][issue-524]
  - **Exercise:** Test a missing Konsole component, unavailable local folders
    and remote locations; hide, reopen and move the terminal while sessions
    exist.
  - **Verify:** The reported state, and that hiding the panel does not
    unexpectedly end sessions.
- **Theme and motion** · [#525][issue-525]
  - **Exercise:** Switch themes and icon sizes, including themes without
    symbolic variants. Change **Animations** in **Preferences › Themes ›
    Motion and Accessibility** between Follow Plasma, Reduced and Off, and
    change Plasma's animation speed.
  - **Verify:** Transitions follow the desktop motion preference and the
    app's reduced and disabled settings, and state is not communicated by
    animation alone.
- **Locale boundaries** · [#526][issue-526]
  - **Exercise:** Use calendar and digit overrides around date boundaries,
    missing translations and mixed KDE and Plasma Opus formatting.
  - **Verify:** The documented distinction between Plasma Opus and
    KDE-controlled displays.

**References:** [terminal panel](../src/app/terminalpanel.cpp),
[terminal tests](../tests/terminalpaneltest.cpp),
[date tests](../tests/localizeddatestest.cpp) and
[translation caveats](CAVEATS.md#translation-format-overrides).

[issue-494]: https://github.com/mikepadiernos/plasma-opus/issues/494
[issue-495]: https://github.com/mikepadiernos/plasma-opus/issues/495
[issue-496]: https://github.com/mikepadiernos/plasma-opus/issues/496
[issue-497]: https://github.com/mikepadiernos/plasma-opus/issues/497
[issue-498]: https://github.com/mikepadiernos/plasma-opus/issues/498
[issue-499]: https://github.com/mikepadiernos/plasma-opus/issues/499
[issue-500]: https://github.com/mikepadiernos/plasma-opus/issues/500
[issue-501]: https://github.com/mikepadiernos/plasma-opus/issues/501
[issue-502]: https://github.com/mikepadiernos/plasma-opus/issues/502
[issue-503]: https://github.com/mikepadiernos/plasma-opus/issues/503
[issue-504]: https://github.com/mikepadiernos/plasma-opus/issues/504
[issue-505]: https://github.com/mikepadiernos/plasma-opus/issues/505
[issue-506]: https://github.com/mikepadiernos/plasma-opus/issues/506
[issue-507]: https://github.com/mikepadiernos/plasma-opus/issues/507
[issue-508]: https://github.com/mikepadiernos/plasma-opus/issues/508
[issue-509]: https://github.com/mikepadiernos/plasma-opus/issues/509
[issue-510]: https://github.com/mikepadiernos/plasma-opus/issues/510
[issue-511]: https://github.com/mikepadiernos/plasma-opus/issues/511
[issue-512]: https://github.com/mikepadiernos/plasma-opus/issues/512
[issue-513]: https://github.com/mikepadiernos/plasma-opus/issues/513
[issue-514]: https://github.com/mikepadiernos/plasma-opus/issues/514
[issue-515]: https://github.com/mikepadiernos/plasma-opus/issues/515
[issue-516]: https://github.com/mikepadiernos/plasma-opus/issues/516
[issue-517]: https://github.com/mikepadiernos/plasma-opus/issues/517
[issue-518]: https://github.com/mikepadiernos/plasma-opus/issues/518
[issue-519]: https://github.com/mikepadiernos/plasma-opus/issues/519
[issue-520]: https://github.com/mikepadiernos/plasma-opus/issues/520
[issue-521]: https://github.com/mikepadiernos/plasma-opus/issues/521
[issue-522]: https://github.com/mikepadiernos/plasma-opus/issues/522
[issue-523]: https://github.com/mikepadiernos/plasma-opus/issues/523
[issue-524]: https://github.com/mikepadiernos/plasma-opus/issues/524
[issue-525]: https://github.com/mikepadiernos/plasma-opus/issues/525
[issue-526]: https://github.com/mikepadiernos/plasma-opus/issues/526
