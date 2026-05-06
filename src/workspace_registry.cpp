// @dependency-start
// responsibility Implements workspace duplicate detection and scroll core behavior.
// upstream implementation ../include/editor_proto/workspace_registry.hpp declares registry and scroll APIs
// downstream implementation ../src/editor_proto_cli.cpp exposes core behavior from the CLI prototype
// downstream implementation ../src/editor_gui_proto.cpp uses core behavior in the native GUI prototype
// downstream implementation ../tests/cpp/prototype/workspace_registry_test.cpp validates registry and scroll behavior
// @dependency-end
#include "editor_proto/workspace_registry.hpp"

#include <algorithm>
#include <cctype>

namespace editor_proto {

std::string WorkspaceRegistry::normalize(std::string path) {
  std::replace(path.begin(), path.end(), '\\', '/');
  while (path.size() > 1 && path.back() == '/') path.pop_back();
  std::transform(path.begin(), path.end(), path.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return path;
}

DuplicateNotice WorkspaceRegistry::open_file(const std::string& path) {
  const auto canonical = normalize(path);
  if (open_files_.contains(canonical)) {
    return {true, canonical, {DuplicateAction::FocusExisting, DuplicateAction::OpenNewView}};
  }
  open_files_.insert(canonical);
  return {false, canonical, {}};
}

DuplicateNotice WorkspaceRegistry::add_directory_root(const std::string& path) {
  const auto canonical = normalize(path);
  if (roots_.contains(canonical)) {
    return {true, canonical, {DuplicateAction::FocusExisting, DuplicateAction::RegisterAlias}};
  }
  roots_.insert(canonical);
  return {false, canonical, {}};
}

bool WorkspaceRegistry::is_file_open(const std::string& path) const {
  return open_files_.contains(normalize(path));
}

bool WorkspaceRegistry::has_root(const std::string& path) const {
  return roots_.contains(normalize(path));
}

ScrollResult ScrollController::wheel_scroll(double current_offset, double wheel_delta) const {
  const double movement = wheel_delta * kWheelMultiplier;
  return {std::max(0.0, current_offset + movement), movement};
}

ScrollResult ScrollController::middle_button_pan(double current_offset, double pointer_delta) const {
  const double movement = pointer_delta * kPanMultiplier;
  return {std::max(0.0, current_offset + movement), movement};
}

}  // namespace editor_proto
