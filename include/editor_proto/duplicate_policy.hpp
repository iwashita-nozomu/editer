#pragma once

#include "editor_proto/workspace_registry.hpp"

namespace editor_proto {

enum class DuplicatePolicy {
  AskEveryTime,
  FocusExisting,
  OpenNewView,
  RegisterAlias,
};

struct DuplicateDecision {
  DuplicateAction action;
  bool remember_choice{false};
};

class DuplicateResolver {
 public:
  DuplicateDecision resolve_file_notice(const DuplicateNotice& notice) const;
  DuplicateDecision resolve_root_notice(const DuplicateNotice& notice) const;

  void set_file_policy(DuplicatePolicy policy) { file_policy_ = policy; }
  void set_root_policy(DuplicatePolicy policy) { root_policy_ = policy; }

 private:
  DuplicatePolicy file_policy_{DuplicatePolicy::AskEveryTime};
  DuplicatePolicy root_policy_{DuplicatePolicy::AskEveryTime};
};

}  // namespace editor_proto
