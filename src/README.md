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
- `editor_gui_proto.cpp`: runnable browser GUI prototype served by a C++ localhost server

prototype smoke は次で実行します。

```bash
cmake -S . -B build/cpp/dev -G Ninja
cmake --build build/cpp/dev --target editor_proto_cli editor_gui_proto
build/cpp/dev/bin/editor_proto_cli demo
build/cpp/dev/bin/editor_gui_proto --port 8765
```
