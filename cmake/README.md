# CMake Layout
<!--
@dependency-start
responsibility Documents CMake Layout for this repository.
downstream implementation ../CMakeLists.txt CMake entrypoint
downstream implementation ../tests/cpp/prototype/workspace_registry_test.cpp workspace behavior coverage
downstream implementation ../tests/cpp/prototype/duplicate_policy_test.cpp duplicate policy coverage
@dependency-end
-->

この repo で C++ を使うときの CMake 正本は root の `CMakeLists.txt` です。

- root `CMakeLists.txt`
  - repo 全体の canonical entrypoint
- `cmake/`
  - 必要になった場合の `find_package` 補助や toolchain helper
- `include/`
  - public header
- `src/`
  - editor prototype の translation unit
- `lib/`
  - checked-in third-party source や手動 vendor する補助 library
- `tests/cpp/`
  - C++ test と smoke source

build は必ず out-of-source で行います。
既定の build tree は `build/cpp/<profile>/`、再利用する local install tree は `.state/cpp-install/<profile>/` です。

canonical C++ target は `editor_proto` と `editor_proto_cli` です。
