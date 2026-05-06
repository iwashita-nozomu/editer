// @dependency-start
// responsibility Validates duplicate-open policy decisions for the C++ prototype.
// upstream implementation ../../../include/editor_proto/duplicate_policy.hpp declares duplicate resolver API
// upstream implementation ../../../src/duplicate_policy.cpp implements resolver behavior
// @dependency-end
#include "editor_proto/duplicate_policy.hpp"

#include <cassert>

int main() {
  using namespace editor_proto;

  DuplicateResolver resolver;
  DuplicateNotice dup_file{true, "/a", {DuplicateAction::FocusExisting, DuplicateAction::OpenNewView}};
  auto d1 = resolver.resolve_file_notice(dup_file);
  assert(d1.action == DuplicateAction::FocusExisting);
  assert(!d1.remember_choice);

  resolver.set_file_policy(DuplicatePolicy::OpenNewView);
  auto d2 = resolver.resolve_file_notice(dup_file);
  assert(d2.action == DuplicateAction::OpenNewView);
  assert(d2.remember_choice);

  DuplicateNotice dup_root{true, "/r", {DuplicateAction::FocusExisting, DuplicateAction::RegisterAlias}};
  resolver.set_root_policy(DuplicatePolicy::RegisterAlias);
  auto d3 = resolver.resolve_root_notice(dup_root);
  assert(d3.action == DuplicateAction::RegisterAlias);

  return 0;
}
