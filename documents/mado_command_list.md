# Mado Command List
<!--
@dependency-start
responsibility Lists Mado runtime commands, keyboard controls, and planned command surfaces.
upstream implementation ../src/editor_gui_proto.cpp implements the native Mado runtime commands
upstream implementation ../include/editor_proto/plugin_registry.hpp declares stable plugin command ownership
upstream design ../documents/mado_plugin_policy.md defines Mado core/plugin command boundaries
upstream design ../mado.yaml defines configurable keybindings and runtime paths
downstream design ../src/README.md documents how to run Mado
@dependency-end
-->

## Build And Run

- `cmake -S . -B build/cpp/dev -G Ninja`
- `cmake --build build/cpp/dev --target mado editor_gui_proto editor_proto_cli editor_proto_plugin_registry_test`
- `build/cpp/dev/bin/mado --test-workspace`
- `build/cpp/dev/bin/mado --root fixtures/mado_workspace`
- `build/cpp/dev/bin/mado --files --test-workspace`
- `build/cpp/dev/bin/mado --log --test-workspace`
- `build/cpp/dev/bin/mado --log-file /tmp/mado.log --test-workspace`
- `build/cpp/dev/bin/mado --log --log-test --log-file /tmp/mado-log-test.log --test-workspace`
- `build/cpp/dev/bin/mado --no-log --test-workspace`
- `build/cpp/dev/bin/mado --ssh-helper host.example`

## Plugin Command Ownership

Mado runtime commands are now grouped by plugin owner. The current registry is builtin and linked into `editor_proto`; dynamic plugin loading is not required for these command IDs.

| Owner | Command IDs |
| --- | --- |
| `mado.core` | `mado.files.open`, `mado.file.save`, `mado.demo.run`, `mado.window.quit`, `mado.focus.enter`, `mado.focus.leave`, `mado.focus.cycle_next`, `mado.focus.cycle_prev`, `mado.focus.editor`, `mado.focus.notice`, `mado.focus.files`, `mado.cursor.up`, `mado.cursor.down`, `mado.cursor.left`, `mado.cursor.right`, `mado.selection.copy` |
| `mado.terminal` | `mado.terminal.open`, `mado.terminal.run`, `mado.terminal.clear`, `mado.log.scroll_up`, `mado.log.scroll_down`, `mado.log.select_line`, `mado.terminal.copy_selection` |
| `mado.ssh` | `mado.ssh.print_helper`, `mado.ssh.host_launch` |
| `mado.devcontainer` | `mado.devcontainer.open`, `mado.devcontainer.rebuild` |

See [Mado Plugin Policy](mado_plugin_policy.md) for the ownership, permission, and migration rules.

## Update

- `build/cpp/dev/bin/mado --update`
- `build/cpp/dev/bin/mado --update --no-restart`

## Keyboard

- `Ctrl+O`: open the fixed floating file picker.
- `Ctrl+S`: save the active editor file.
- `Ctrl+@`: open a new floating terminal for the current root. Repeating it creates another terminal window.
- `Ctrl+D`: run the core duplicate/root/scroll demo.
- `Ctrl+Q`: quit the active Mado window.
- `Ctrl+1`: focus the main editor window.
- `Ctrl+2`: focus the notice pane.
- `Ctrl+3`: focus the file picker window.
- `Ctrl+4`: focus the current terminal window, creating one if none exists.
- `Ctrl+Tab`: cycle main -> files -> terminal -> main.
- `Up` / `Down` in terminal: move the output selection cursor.
- `Shift+Up` / `Shift+Down` in terminal: extend the output selection range.
- `Ctrl+C` in terminal: copy the selected terminal output range.
- `Ctrl+L` in terminal: clear terminal output.

## Shell And Host Launch

- `mado --log`: write every collected runtime event to stdout as well as the configured log file.
- `mado --log-test`: run the headless runtime-log smoke test and verify required log events.
- `mado --files`: open the floating Files window during startup.
- `mado --ssh-helper <host>`: print a shell function for an SSH session that calls the host-side Mado binary. Command owner: `mado.ssh.print_helper`.
- `mado --host-launch --root <path>`: host-side entrypoint used by the SSH helper. Command owner: `mado.ssh.host_launch`.
- Terminal shells inherit `PATH` with the current Mado binary directory first, plus `MADO_HOST_BINARY`, `MADO_HOST_ROOT`, and `MADO_HOST_LAUNCH`.
- SSH authentication prompts from commands such as `ssh <host>` stay inside the Mado terminal window because the shell runs through a PTY.
- Crash tracking is always enabled at `.state/cpp-install/mado/logs/mado-crash.log`, even when normal runtime logging is disabled.
- Docker CLI from the terminal can start with `docker compose exec <service> sh`; dedicated Docker child windows are planned and should be owned by `mado.devcontainer` or a narrower Docker plugin.

## Planned Runtime Commands

- `mado --resume`: restore prior root, open files, prompt windows, and window placement from the session cache.
- `mado --resume <session-id>`: restore a named or timestamped session.
- `mado --roots`: list remembered roots and their cached child windows.
- `mado --forget <root-or-session>`: remove a cached root/session graph.
