#pragma once
// @dependency-start
// responsibility Maintains the repository-owned text surface include/project_template/jax_export_smoke.hpp.
// upstream implementation ../../CMakeLists.txt C++ build configuration
// downstream implementation ../../tests/cpp/smoke/jax_export_header_smoke.cpp smoke test
// @dependency-end

#include <cstddef>
#include <xla/ffi/api/c_api.h>
#include <xla/ffi/api/ffi.h>

namespace project_template {

inline constexpr bool xla_ffi_headers_available() noexcept {
  return sizeof(XLA_FFI_Api) > 0;
}

inline constexpr std::size_t xla_ffi_api_size() noexcept {
  return sizeof(XLA_FFI_Api);
}

}  // namespace project_template
