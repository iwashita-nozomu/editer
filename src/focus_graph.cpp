// @dependency-start
// responsibility Implements a minimal keyboard-only focus graph for Mado surfaces.
// upstream implementation ../include/editor_proto/focus_graph.hpp declares focus graph API
// upstream design ../documents/mado_editor_backend_design.md defines focus/navigation invariants
// downstream implementation ../tests/cpp/prototype/focus_graph_test.cpp validates focus graph behavior
// @dependency-end
#include "editor_proto/focus_graph.hpp"

#include <algorithm>

namespace editor_proto {

namespace {

bool has_text(std::string_view value) noexcept { return !value.empty(); }

bool has_focus_commands(const FocusCommandTable& commands) noexcept {
  return has_text(commands.focus_enter) && has_text(commands.focus_leave) &&
         has_text(commands.focus_cycle_next) && has_text(commands.focus_cycle_prev);
}

bool has_text_cursor_commands(const FocusCommandTable& commands) noexcept {
  return has_text(commands.cursor_up) && has_text(commands.cursor_down) &&
         has_text(commands.cursor_left) && has_text(commands.cursor_right);
}

bool has_log_selection_commands(const FocusCommandTable& commands) noexcept {
  return has_text(commands.log_scroll_up) && has_text(commands.log_scroll_down) &&
         has_text(commands.cursor_up) && has_text(commands.cursor_down) &&
         has_text(commands.log_select_line) && has_text(commands.selection_copy);
}

}  // namespace

bool FocusContractStatus::valid() const noexcept {
  return has_focus_commands && has_text_cursor_commands && has_log_selection_commands;
}

bool FocusGraph::register_surface(FocusNode node) {
  if (node.id == kInvalidFocusTargetId) return false;
  if (find(node.id) != nullptr) return false;
  if (node.parent_id.has_value() && find(*node.parent_id) == nullptr) return false;
  nodes_.push_back(std::move(node));
  cycle_order_.push_back(nodes_.back().id);
  return true;
}

FocusNode* FocusGraph::find(FocusTargetId id) noexcept {
  const auto found = std::find_if(nodes_.begin(), nodes_.end(), [id](const FocusNode& node) {
    return node.id == id;
  });
  if (found == nodes_.end()) return nullptr;
  return &*found;
}

const FocusNode* FocusGraph::find(FocusTargetId id) const noexcept {
  const auto found = std::find_if(nodes_.begin(), nodes_.end(), [id](const FocusNode& node) {
    return node.id == id;
  });
  if (found == nodes_.end()) return nullptr;
  return &*found;
}

std::size_t FocusGraph::size() const noexcept { return nodes_.size(); }

bool FocusGraph::focus(FocusTargetId id) {
  const FocusNode* node = find(id);
  if (node == nullptr || node->closed) return false;
  focused_id_ = id;
  return true;
}

std::optional<FocusTargetId> FocusGraph::focused_id() const noexcept { return focused_id_; }

std::optional<FocusTargetId> FocusGraph::cycle_next() { return cycle(true); }

std::optional<FocusTargetId> FocusGraph::cycle_prev() { return cycle(false); }

std::optional<FocusTargetId> FocusGraph::cycle(bool forward) {
  if (cycle_order_.empty()) {
    focused_id_.reset();
    return std::nullopt;
  }

  const auto is_open = [this](FocusTargetId id) {
    const FocusNode* node = find(id);
    return node != nullptr && !node->closed;
  };

  if (!focused_id_.has_value() || !is_open(*focused_id_)) {
    const auto first_open = std::find_if(cycle_order_.begin(), cycle_order_.end(), is_open);
    if (first_open == cycle_order_.end()) {
      focused_id_.reset();
      return std::nullopt;
    }
    focused_id_ = *first_open;
    return focused_id_;
  }

  auto current = std::find(cycle_order_.begin(), cycle_order_.end(), *focused_id_);
  if (current == cycle_order_.end()) {
    focused_id_.reset();
    return cycle(forward);
  }

  const std::size_t count = cycle_order_.size();
  for (std::size_t step = 0; step < count; ++step) {
    if (forward) {
      current = std::next(current);
      if (current == cycle_order_.end()) current = cycle_order_.begin();
    } else {
      current = current == cycle_order_.begin() ? std::prev(cycle_order_.end()) : std::prev(current);
    }

    const FocusNode* node = find(*current);
    if (node != nullptr && !node->closed) {
      focused_id_ = *current;
      return focused_id_;
    }
  }

  focused_id_.reset();
  return std::nullopt;
}

FocusCloseResult FocusGraph::close_surface(FocusTargetId id) {
  FocusNode* node = find(id);
  if (node == nullptr || node->closed) {
    return FocusCloseResult{.closed_id = id, .focus_restore_id = std::nullopt};
  }

  node->closed = true;
  std::optional<FocusTargetId> restore_id = node->parent_id;
  if (restore_id.has_value()) {
    const FocusNode* parent = find(*restore_id);
    if (parent == nullptr || parent->closed) restore_id = std::nullopt;
  }

  if (focused_id_ == id) focused_id_ = restore_id;
  return FocusCloseResult{.closed_id = id, .focus_restore_id = restore_id};
}

FocusContractStatus FocusGraph::validate_contract(FocusTargetId id) const noexcept {
  const FocusNode* node = find(id);
  if (node == nullptr || node->closed) return {};

  FocusContractStatus status;
  status.has_focus_commands = has_focus_commands(node->commands);
  status.has_text_cursor_commands = !node->text_like || has_text_cursor_commands(node->commands);
  status.has_log_selection_commands = !node->log_like || has_log_selection_commands(node->commands);
  return status;
}

}  // namespace editor_proto
