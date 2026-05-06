# src
<!--
@dependency-start
responsibility Documents src for this repository.
upstream implementation ../include/editor_proto/workspace_registry.hpp workspace registry API
upstream implementation ../include/editor_proto/duplicate_policy.hpp duplicate policy API
upstream design ../cmake/README.md CMake layout guidance
@dependency-end
-->

`src/` は editor prototype の C++ translation unit を置く場所です。
public API は `include/editor_proto/` に置き、実装は root `CMakeLists.txt` の
`editor_proto` target から build します。

現在の実装 surface は次です。

- `workspace_registry.cpp`: open file / directory root duplicate detection and scroll calculation
- `duplicate_policy.cpp`: duplicate notice default decision policy
- `editor_proto_cli.cpp`: runnable CLI prototype with `demo`, duplicate-open, directory, and scroll commands
- `editor_gui_proto.cpp`: runnable native X11 GUI prototype loaded from the system runtime

prototype smoke は次で実行します。

```bash
cmake -S . -B build/cpp/dev -G Ninja
cmake --build build/cpp/dev --target editor_proto_cli editor_gui_proto
build/cpp/dev/bin/editor_proto_cli demo
build/cpp/dev/bin/editor_gui_proto
build/cpp/dev/bin/mado --test-workspace
```

GUI prototype の名前は Mado で、短縮 command は `mado` です。
`mado.yaml` で app name、font size、keybindings、update path、runtime log path を上書きできます。
`mado --update` は CMake target `mado` を rebuild し、`.state/cpp-install/mado/bin/mado` を置き換えて再起動します。
CI や手動更新だけの確認では `mado --update --no-restart` を使います。
`mado --test-workspace` は `fixtures/mado_workspace` を root として開きます。
起動時は [Mado Manual Checklist](../documents/mado_manual_checklist.md) と
[Mado Command List](../documents/mado_command_list.md) を editor tab として開きます。
debug 用の実行ログは既定で `.state/cpp-install/mado/logs/` に出ます。
一時的な出力先は `mado --log-file /tmp/mado.log --test-workspace` で指定でき、`--no-log` で無効化できます。
`mado --log` は収集した runtime event を stdout にも出します。
`mado --log --log-test --log-file /tmp/mado-log-test.log --test-workspace` で headless log smoke を実行できます。

`editor_gui_proto` と `mado` は Close button、window close control、Esc、または configured quit key まで表示を続けます。
常駐 file pane は使わず、`Files` button または `Ctrl+O` で root 配下 file picker を floating window として開きます。
選択した file は editor pane で直接編集でき、`Ctrl+S` で実 file に保存します。
Prompt は常駐 pane ではなく、`Ctrl+@` または `Prompt` button で root directory の same-design floating prompt window を前面に開きます。
prompt では Enter または `Run` で root directory を cwd にして command を実行します。
prompt 出力は Up/Down で行カーソルを動かし、Shift+Up/Shift+Down で範囲選択、Ctrl+C で clipboard copy できます。
prompt shell は current Mado binary を `PATH` 先頭に置き、`MADO_HOST_LAUNCH` も渡します。
SSH session から host 側 Mado を呼ぶ shell 関数は `mado --ssh-helper <host>` で生成します。
人間が行う動作確認は [Mado Manual Checklist](../documents/mado_manual_checklist.md) に追記します。
