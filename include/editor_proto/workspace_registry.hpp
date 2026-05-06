// @dependency-start
// responsibility Defines workspace duplicate detection and scroll core APIs.
// downstream implementation ../../src/workspace_registry.cpp implements registry and scroll behavior
// downstream implementation ../../src/editor_gui_proto.cpp uses workspace duplicate notices in the GUI
// downstream implementation ../../tests/cpp/prototype/workspace_registry_test.cpp validates core behavior
// @dependency-end
#pragma once

#include <string>
#include <unordered_set>
#include <vector>

namespace editor_proto {

enum class DuplicateAction {
  FocusExisting,
  OpenNewView,
  RegisterAlias,
};

struct DuplicateNotice {
  bool is_duplicate{false};
  std::string canonical_path;
  std::vector<DuplicateAction> actions;
};

class WorkspaceRegistry {
 public:
  DuplicateNotice open_file(const std::string& path);
  DuplicateNotice add_directory_root(const std::string& path);

  bool is_file_open(const std::string& path) const;
  bool has_root(const std::string& path) const;

  std::size_t open_file_count() const noexcept { return open_files_.size(); }
  std::size_t root_count() const noexcept { return roots_.size(); }

 private:
  static std::string normalize(std::string path);

  std::unordered_set<std::string> open_files_;
  std::unordered_set<std::string> roots_;
};

struct ScrollResult {
  double new_offset{0.0};
  double velocity{0.0};
};

class ScrollController {
 public:
  ScrollResult wheel_scroll(double current_offset, double wheel_delta) const;
  ScrollResult middle_button_pan(double current_offset, double pointer_delta) const;

 private:
  static constexpr double kWheelMultiplier = 45.0;
  static constexpr double kPanMultiplier = 4.0;
};

}  // namespace editor_proto
