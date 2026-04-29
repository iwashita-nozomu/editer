// @dependency-start
// responsibility Tests behavior for jax export header smoke.
// upstream implementation ../../../include/project_template/jax_export_smoke.hpp header under test
// upstream implementation ../../../CMakeLists.txt C++ build configuration
// @dependency-end
#include <project_template/jax_export_smoke.hpp>

int main() {
  return project_template::xla_ffi_headers_available() ? 0 : 1;
}
