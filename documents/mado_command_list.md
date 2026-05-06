# Mado Command List
<!--
@dependency-start
responsibility Lists Mado runtime commands, keyboard controls, and planned command surfaces.
upstream implementation ../src/editor_gui_proto.cpp implements the native Mado runtime commands
upstream design ../mado.yaml defines configurable keybindings and runtime paths
downstream design ../src/README.md documents how to run Mado
@dependency-end
-->

## Build And Run

- `cmake -S . -B build/cpp/dev -G Ninja`
- `cmake --build build/cpp/dev --target mado editor_gui_proto editor_proto_cli`
- `build/cpp/dev/bin/mado --test-workspace`
- `build/cpp/dev/bin/mado --root fixtures/mado_workspace`
- `build/cpp/dev/bin/mado --files --test-workspace`
- `build/cpp/dev/bin/mado --log --test-workspace`
- `build/cpp/dev/bin/mado --log-file /tmp/mado.log --test-workspace`
- `build/cpp/dev/bin/mado --log --log-test --log-file /tmp/mado-log-test.log --test-workspace`
- `build/cpp/dev/bin/mado --no-log --test-workspace`
- `build/cpp/dev/bin/mado --ssh-helper host.example`

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
- `mado --ssh-helper <host>`: print a shell function for an SSH session that calls the host-side Mado binary.
- `mado --host-launch --root <path>`: host-side entrypoint used by the SSH helper.
- Terminal shells inherit `PATH` with the current Mado binary directory first, plus `MADO_HOST_BINARY`, `MADO_HOST_ROOT`, and `MADO_HOST_LAUNCH`.
- SSH authentication prompts from commands such as `ssh <host>` stay inside the Mado terminal window because the shell runs through a PTY.
- Crash tracking is always enabled at `.state/cpp-install/mado/logs/mado-crash.log`, even when normal runtime logging is disabled.
- Docker CLI from the terminal can start with `docker compose exec <service> sh`; dedicated Docker child windows are planned.

## Planned Runtime Commands

- `mado --resume`: restore prior root, open files, prompt windows, and window placement from the session cache.
- `mado --resume <session-id>`: restore a named or timestamped session.
- `mado --roots`: list remembered roots and their cached child windows.
- `mado --forget <root-or-session>`: remove a cached root/session graph.
