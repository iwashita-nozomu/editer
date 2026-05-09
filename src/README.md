# src
<!--
@dependency-start
responsibility Documents src for this repository.
upstream implementation ../include/editor_proto/workspace_registry.hpp workspace registry API
upstream implementation ../include/editor_proto/duplicate_policy.hpp duplicate policy API
upstream implementation ../include/editor_proto/plugin_registry.hpp plugin boundary and builtin manifest API
upstream design ../documents/mado_plugin_policy.md Mado core/plugin ownership boundary
upstream design ../cmake/README.md CMake layout guidance
@dependency-end
-->

`src/` は editor prototype の C++ translation unit を置く場所です。
public API は `include/editor_proto/` に置き、実装は root `CMakeLists.txt` の
`editor_proto` target から build します。

現在の実装 surface は次です。

- `workspace_registry.cpp`: open file / directory root duplicate detection and scroll calculation
- `duplicate_policy.cpp`: duplicate notice default decision policy
- `plugin_registry.cpp`: Mado core/plugin boundary, builtin plugin manifests, and command ownership registry
- `editor_proto_cli.cpp`: runnable CLI prototype with `demo`, duplicate-open, directory, and scroll commands
- `editor_gui_proto.cpp`: runnable native X11 GUI prototype loaded from the system runtime

Mado の core/plugin 境界は [Mado Plugin Policy](../documents/mado_plugin_policy.md) を正本にします。
現時点では dynamic plugin loading ではなく、`editor_proto` に linked される builtin plugin manifest で
`mado.core`、`mado.terminal`、`mado.ssh`、`mado.devcontainer` の command ownership と permissions を固定します。
SSH、devcontainer、Docker、host launch のような外部連携は core に直接増やさず、builtin plugin の command owner を通して拡張します。

prototype smoke は次で実行します。

```bash
cmake -S . -B build/cpp/dev -G Ninja
cmake --build build/cpp/dev --target editor_proto_cli editor_gui_proto mado editor_proto_plugin_registry_test
build/cpp/dev/bin/editor_proto_cli demo
build/cpp/dev/bin/editor_proto_plugin_registry_test
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
クラッシュ追跡ログは通常ログとは別に常時 `.state/cpp-install/mado/logs/mado-crash.log` へ出ます。

`editor_gui_proto` と `mado` は Close button、window close control、Esc、または configured quit key まで表示を続けます。
常駐 file pane は使わず、`Files` button、`Ctrl+O`、または `mado --files` で root 配下 file picker を floating window として開きます。
選択した file は editor pane で直接編集でき、編集位置 cursor が表示され、`Ctrl+S` で実 file に保存します。
Prompt は常駐 pane ではなく、`Ctrl+@` または `Prompt` button で root directory の same-design floating terminal window を新規に開きます。
`Ctrl+@` を繰り返すと複数 terminal window を同時に開けます。
terminal では Enter または `Run` で root directory を cwd にした PTY shell へ入力を送ります。
terminal 出力は Up/Down で行カーソルを動かし、Shift+Up/Shift+Down で範囲選択、Ctrl+C で clipboard copy できます。
terminal shell は current Mado binary を `PATH` 先頭に置き、`MADO_HOST_LAUNCH` も渡します。
`ssh <host>` の認証 prompt は terminal window 内に表示され、その場で入力できます。
SSH session から host 側 Mado を呼ぶ shell 関数は `mado --ssh-helper <host>` で生成します。
人間が行う動作確認は [Mado Manual Checklist](../documents/mado_manual_checklist.md) に追記します。
