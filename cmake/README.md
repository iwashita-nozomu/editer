# CMake Layout
<!--
@dependency-start
responsibility Documents CMake Layout for this repository.
downstream implementation ../CMakeLists.txt CMake entrypoint
downstream implementation ../tests/cpp/smoke/jax_export_header_smoke.cpp smoke build coverage
@dependency-end
-->

この template で C++ を使うときの CMake 正本は root の `CMakeLists.txt` です。

- root `CMakeLists.txt`
  - repo 全体の canonical entrypoint
- `cmake/`
  - `find_package` 補助、toolchain helper、JAX / OpenXLA 連携 helper
- `include/`
  - public header 兼 template 既定の header-only 実装
- `src/`
  - header-only で収まらない特例実装
- `lib/`
  - checked-in third-party source や手動 vendor する補助 library
- `tests/cpp/`
  - C++ test と smoke source

build は必ず out-of-source で行います。
既定の build tree は `build/cpp/<profile>/`、再利用する local install tree は `.state/cpp-install/<profile>/`、`jax.export` artifact cache は `.state/jax-export/<profile>/` です。

template の canonical C++ target は `INTERFACE` library を既定にします。
