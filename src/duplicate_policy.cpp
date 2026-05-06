// @dependency-start
// responsibility Implements duplicate-open decision policy for the editor prototype.
// upstream implementation ../include/editor_proto/duplicate_policy.hpp declares resolver API
// downstream implementation ../tests/cpp/prototype/duplicate_policy_test.cpp validates resolver choices
// @dependency-end
#include "editor_proto/duplicate_policy.hpp"

namespace editor_proto {

namespace {
DuplicateDecision choose_default(const DuplicateNotice& notice) {
  if (!notice.actions.empty()) return {notice.actions.front(), false};
  return {DuplicateAction::FocusExisting, false};
}
}  // namespace

DuplicateDecision DuplicateResolver::resolve_file_notice(const DuplicateNotice& notice) const {
  if (!notice.is_duplicate) return {DuplicateAction::OpenNewView, false};
  if (file_policy_ == DuplicatePolicy::FocusExisting) return {DuplicateAction::FocusExisting, true};
  if (file_policy_ == DuplicatePolicy::OpenNewView) return {DuplicateAction::OpenNewView, true};
  return choose_default(notice);
}

DuplicateDecision DuplicateResolver::resolve_root_notice(const DuplicateNotice& notice) const {
  if (!notice.is_duplicate) return {DuplicateAction::RegisterAlias, false};
  if (root_policy_ == DuplicatePolicy::FocusExisting) return {DuplicateAction::FocusExisting, true};
  if (root_policy_ == DuplicatePolicy::RegisterAlias) return {DuplicateAction::RegisterAlias, true};
  return choose_default(notice);
}

}  // namespace editor_proto
