# Mado Terminal Popup Architecture
<!--
@dependency-start
responsibility Defines Mado terminal/file popup launch, parent restore, and bounded log retention behavior.
upstream implementation ../include/editor_proto/terminal_session.hpp declares popup session model
upstream implementation ../src/terminal_session.cpp implements popup session model
upstream design mado_plugin_policy.md defines plugin ownership boundaries
downstream design mado_command_list.md lists user-facing terminal commands
@dependency-end
-->

Mado terminal integration should keep the current screen stable when entering another environment.
SSH, WSL, devcontainer, Docker shell, and file open flows should create a new lightweight popup instead of replacing the current terminal view.

## Popup Chain Model

Mado uses direct up/down links, not a full tree traversal model.
Each popup has at most one parent and one active child:

- `parent_id`: the popup that launched this popup
- `child_id`: the current popup launched from this popup

This is enough for the common flow:

```text
local terminal -> remote terminal -> mado <filename> file popup
```

The chain may contain both terminal popups and file popups.
A terminal can launch a file popup with `mado <filename>`; the file popup records the parent terminal and should be raised as a new popup window.

## Close And Focus Restore

Closing a popup returns a `focus_restore_id`.
The GUI should raise that popup after the child exits.
When a remote terminal exits, the parent terminal that launched it should return to the front.
When a file popup closes, the terminal that opened it should return to the front.

Closing an upstream popup cascades to its active child chain.
For example, closing the local terminal also closes the remote terminal and any file popup opened from it.
The model keeps closed sessions and their logs available for diagnostics instead of deleting them immediately.

## Launch Latency Budget

Terminal and file popup launch should target 100 ms or less for the interactive handoff path.
The fast path should only allocate the session, link parent/child IDs, initialize the bounded log buffer, and create the popup shell.
Slow work such as file content loading, remote handshake, Docker inspection, devcontainer discovery, or large log scan should happen after the popup appears.

The default budget is represented by `kDefaultPopupLaunchBudgetMs`.

## Log Retention

Logs are collected per popup, not globally.
Each popup owns an independent bounded buffer.
The buffer size is configured through a memory byte limit and old entries are pruned first when the limit is exceeded.

The initial model stores these event classes:

- `info`
- `input`
- `output`
- `error`
- `exit`
- `file-open`

Future runtime config should expose this under the terminal plugin config, for example:

```yaml
plugins:
  mado.terminal:
    popup_launch_budget_ms: 100
    log_max_bytes: 65536
```

## Command Ownership

The terminal plugin owns popup-oriented terminal commands:

- `mado.terminal.open`
- `mado.terminal.run`
- `mado.terminal.open_file`
- `mado.terminal.close_chain`
- `mado.terminal.clear`
- `mado.terminal.copy_selection`

`mado <filename>` should route to `mado.terminal.open_file` when invoked from a Mado terminal context.
