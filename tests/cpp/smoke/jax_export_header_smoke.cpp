#include <project_template/jax_export_smoke.hpp>

int main() {
  return project_template::xla_ffi_headers_available() ? 0 : 1;
}
