# Scope and objective

[![Docs index](https://img.shields.io/badge/docs-index-3daee9?style=flat-square)](../README.md)
[![Next: Inspirations](https://img.shields.io/badge/next-Inspirations-3daee9?style=flat-square)](inspirations.md)
[![Roadmap](https://img.shields.io/badge/roadmap-M01%E2%80%93M32-8957e5?style=flat-square)](../ROADMAP.md)

What Plasma Opus is for, who it serves and where its boundaries lie. The rest
of the philosophy series builds on this scope.

**Philosophy series:** **1. Scope** → [2. Inspirations](inspirations.md) →
[3. Interface design](designs.md)

> [!NOTE]
> Milestones M01–M31 are merged into `main`; M32 is planned.
> The [roadmap](../ROADMAP.md) is the source of truth for feature status.

## Contents

- [Objective](#objective)
- [Audience and workflows](#audience-and-workflows)
- [Product scope](#product-scope)
- [Design boundaries](#design-boundaries)

## Objective

Plasma Opus brings power-user file management to Linux in an application
designed for KDE Plasma. Its central idea is the dual-pane workspace: keep two
locations available, navigate each through tabs and history, and make operations
between them direct and predictable.

The project combines this workflow with KDE desktop integration rather than
recreating a desktop environment inside the file manager. Max File Manager
provides the primary behaviour and feature-parity reference; ideas from other
file managers broaden the workflows described in
[Inspirations](inspirations.md).

## Audience and workflows

Plasma Opus is for people who manage files frequently and benefit from seeing
source and destination locations together. It combines browsing, preview and
transfer workflows with power-user tools. See the
[feature overview](../../README.md#features) for current capabilities.

## Product scope

- **Platform:** Linux, with KDE Plasma as the target desktop.
- **Desktop integration:** use KDE services for file operations, Places,
  properties, settings, icons and application integration wherever they fit.
  Planned desktop integrations are described in the
  [KDE Plasma integration plan](../integrations/kde-plasma.md).
- **Compatibility:** support a subset of upstream `.mfm.json` metadata and keep
  the rest unchanged when writing. See
  [metadata limitations](../CAVEATS.md#folder-metadata-and-listings) and the
  [behaviour references](../README.md#behaviour-references).
- **Delivery:** track tested, incremental milestones in the
  [roadmap](../ROADMAP.md). Planned features are not necessarily implemented.

## Design boundaries

- Plasma Opus is Linux-only; a cross-platform port is not a goal.
- Plasma Opus complements the KDE desktop and does not replace, remove or mask
  Dolphin.
- Prefer existing KDE facilities for common desktop functions. Implement Plasma
  Opus-specific logic where the workflow has no suitable KDE counterpart.
- User data and behaviour should remain explicit and testable. Core logic
  belongs outside widgets when practical, and tests must not modify a real
  user's files, home directory or configuration.

---

Next: [Inspirations](inspirations.md) →
