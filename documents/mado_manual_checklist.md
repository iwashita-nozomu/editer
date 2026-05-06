# Mado Manual Checklist
<!--
@dependency-start
responsibility Tracks human runtime checks for the Mado native editor prototype.
upstream implementation ../src/editor_gui_proto.cpp implements the runtime behavior under check
upstream design ../mado.yaml defines default font, keybindings, update, and logging paths
downstream design ../src/README.md links the Mado runtime checks
@dependency-end
-->

Use `mado --test-workspace` so the file picker opens `fixtures/mado_workspace`.

- [ ] Main window opens as native X11 and stays open until Close, window close control, Esc, or configured quit key.
- [ ] Startup opens `Mado Checklist` and `Mado Commands` as editor tabs.
- [ ] Window resize keeps the editor, inspector, status bar, file picker, and prompt readable.
- [ ] `Files`, configured open-files key, configured focus-files key, or `mado --files --test-workspace` opens a floating file picker with no sidebar.
- [ ] `Ctrl+Tab`, configured focus-editor key, configured focus-files key, and configured focus-prompt key switch keyboard focus between main, files, and prompt windows.
- [ ] File picker remains a fixed floating window and is not raised over other windows after every selection.
- [ ] Selecting `src/sample.cpp` loads it into the editor pane.
- [ ] Typing changes the file buffer, the active edit cursor is visible, and configured save key writes the file.
- [ ] Configured prompt key opens a new same-design floating terminal each time; multiple terminal windows can remain open together.
- [ ] Terminal header shows the active shell name, path, host, root, and root-derived accent color.
- [ ] Terminal shell starts with the test workspace as cwd.
- [ ] Running `mado --log --test-workspace` mirrors collected runtime events to stdout and the configured log file.
- [ ] Running `mado --log --log-test --log-file /tmp/mado-log-test.log --test-workspace` passes and prints the generated log lines.
- [ ] Running `mado --ssh-helper <host>` prints a shell function that can be used inside an SSH session to call host-side Mado.
- [ ] Commands run inside the terminal can find the current Mado binary through `PATH` and see `MADO_HOST_LAUNCH`.
- [ ] Running `ssh <host>` inside the terminal shows password, passphrase, or OTP prompts inside the Mado terminal window.
- [ ] Docker CLI can be launched from the terminal with a command such as `docker compose exec <service> sh`.
- [ ] Running a command appends output and auto-scrolls terminal output to the newest lines.
- [ ] Terminal Up/Down moves the output selection cursor; Shift+Up/Shift+Down extends the selected line range.
- [ ] Terminal Ctrl+C copies the selected output range to the system clipboard when `xclip`, `xsel`, or `wl-copy` is available, otherwise to Mado internal clipboard state.
- [ ] Runtime log is written under `.state/cpp-install/mado/logs/` or the `--log-file` path.
- [ ] Terminal shell start, terminal input, terminal output, file open, and file save events appear in the runtime log.
- [ ] Crash log exists at `.state/cpp-install/mado/logs/mado-crash.log` and records `event=crashlog.start`.
- [ ] Editing `mado.yaml` font or keybindings and running `mado --update` rebuilds, replaces runtime binary, and restarts.
- [ ] `mado --update --no-restart` rebuilds and replaces `.state/cpp-install/mado/bin/mado` without opening the GUI.
