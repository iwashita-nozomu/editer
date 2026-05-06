# Mado Manual Checklist
<!--
@dependency-start
responsibility Tracks human runtime checks for the Mado native editor prototype.
upstream implementation ../../src/editor_gui_proto.cpp implements the runtime behavior under check
upstream design ../../mado.yaml defines default font, keybindings, and update paths
downstream design ../../src/README.md links the test workspace command
@dependency-end
-->

Use `mado --test-workspace` so the file picker opens this directory.

- [ ] Main window opens as native X11 and stays open until Close, window close control, Esc, or configured quit key.
- [ ] Window resize keeps the editor, inspector, status bar, file picker, and prompt readable.
- [ ] `Files` or configured open-files key opens a floating file picker with no sidebar.
- [ ] Selecting `src/sample.cpp` loads it into the editor pane.
- [ ] Typing changes the file buffer and configured save key writes the file.
- [ ] Configured prompt key opens the same-design floating prompt in front of the editor.
- [ ] Prompt command runs with the test workspace as cwd.
- [ ] Editing `mado.yaml` font or keybindings and running `mado --update` rebuilds, replaces runtime binary, and restarts.
- [ ] `mado --update --no-restart` rebuilds and replaces `.state/cpp-install/mado/bin/mado` without opening the GUI.
