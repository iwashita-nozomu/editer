// @dependency-start
// responsibility Validates workspace duplicate detection and scroll behavior for the C++ prototype.
// upstream implementation ../../../include/editor_proto/workspace_registry.hpp declares registry and scroll APIs
// upstream implementation ../../../src/workspace_registry.cpp implements registry and scroll behavior
// @dependency-end
#include "editor_proto/workspace_registry.hpp"

#include <cassert>

int main() {
  using namespace editor_proto;

  WorkspaceRegistry registry;

  auto f1 = registry.open_file("/Repo/src/main.cpp");
  assert(!f1.is_duplicate);
  auto f2 = registry.open_file("\\repo\\src\\main.cpp");
  assert(f2.is_duplicate);
  assert(f2.actions.size() == 2);
  assert(registry.is_file_open("/repo/src/main.cpp"));

  auto d1 = registry.add_directory_root("/Work/ProjectA/");
  assert(!d1.is_duplicate);
  auto d2 = registry.add_directory_root("/work/projecta");
  assert(d2.is_duplicate);
  assert(registry.has_root("/WORK/projectA"));

  assert(registry.open_file_count() == 1);
  assert(registry.root_count() == 1);

  ScrollController scroll;
  auto wheel = scroll.wheel_scroll(10.0, 2.0);
  assert(wheel.new_offset == 100.0);
  auto pan = scroll.middle_button_pan(wheel.new_offset, -10.0);
  assert(pan.new_offset == 60.0);

  auto clamped = scroll.middle_button_pan(5.0, -100.0);
  assert(clamped.new_offset == 0.0);

  return 0;
}
