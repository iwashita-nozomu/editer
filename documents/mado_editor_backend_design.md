# Mado Editor Backend And Keyboard Focus Contract
<!--
@dependency-start
responsibility Defines the Neovim backend boundary, adapter responsibilities, and keyboard-only focus contract.
upstream implementation ../include/editor_proto/focus_graph.hpp declares the minimal focus graph model
upstream implementation ../src/focus_graph.cpp implements focus graph behavior
downstream design mado_command_list.md maps runtime command ownership and keyboard bindings
@dependency-end
-->

## Scope

Mado adopts Neovim as an editor engine, not as the outer UI shell.
Mado keeps ownership of windows, popup chains, keyboard navigation, terminal/file popup restore, and selectable runtime logs.

## Backend Boundary

Mado owns:

- window and popup lifecycle
- global focus routing across all interactive surfaces
- keyboard-only navigation and selection contracts
- terminal/file popup parent-child chain state
- CLI log visibility and selection
- plugin command ownership and dispatch

Neovim adapter owns:

- editor buffer mutation and undo/redo semantics
- modal editing primitives inside editor buffers
- cursor/selection state inside editor buffers
- syntax/highlighting/LSP integrations (future)
- text object operations exposed by Mado-owned command IDs

Neovim must run as an embedded/headless backend and never bypass Mado focus/popup ownership.

## Neovim Adapter Responsibilities

1. Keep one or more Neovim instances as backend sessions (single-instance per workspace root is the default proposal).
2. Expose buffer state changes to Mado renderer and accept Mado-issued command IDs.
3. Translate Neovim cursor/selection updates into Mado focus target updates for editor surfaces.
4. Keep popup launch handoff under the 100 ms target by deferring heavy backend operations until after popup creation.
5. Preserve terminal/file popup parent restore semantics when editor/file popups close.

## Keyboard-Only Focus And Navigation Contract

Every interactive surface must be reachable and operable without mouse input.
The contract applies to editor buffers, file picker, terminal popups, file popups, CLI/error logs, status panes, and plugin/command palette surfaces.

Required invariants:

1. Every window/popup has a stable focus target ID.
2. Every focusable surface has keyboard commands to enter/leave/cycle focus.
3. Every text-like surface has keyboard cursor movement commands.
4. Every log-like surface has keyboard scroll, cursor/line selection, and copy commands.
5. Closing a child popup restores focus to its parent target when available.
6. Mouse support is optional and never required for reachability.
7. Neovim integration cannot bypass Mado popup/focus routing.
8. CLI logs remain first-class selectable focus surfaces.

## Minimum Command IDs (Focus/Selection)

- `mado.focus.enter`
- `mado.focus.leave`
- `mado.focus.cycle_next`
- `mado.focus.cycle_prev`
- `mado.cursor.up`
- `mado.cursor.down`
- `mado.cursor.left`
- `mado.cursor.right`
- `mado.log.scroll_up`
- `mado.log.scroll_down`
- `mado.log.select_line`
- `mado.selection.copy`

These are the baseline IDs required before adding richer GUI behavior.
