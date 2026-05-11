// @dependency-start
// responsibility Defines a minimal keyboard-only focus graph contract for Mado surfaces.
// upstream design ../../documents/mado_editor_backend_design.md editor backend boundary and focus contract
// downstream implementation ../../src/focus_graph.cpp implements focus graph behavior
// downstream implementation ../../tests/cpp/prototype/focus_graph_test.cpp validates contract invariants
// @dependency-end
#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace editor_proto {

using FocusTargetId = std::size_t;
constexpr FocusTargetId kInvalidFocusTargetId = 0;

constexpr std::string_view kCommandFocusEnter = "mado.focus.enter";
constexpr std::string_view kCommandFocusLeave = "mado.focus.leave";
constexpr std::string_view kCommandFocusCycleNext = "mado.focus.cycle_next";
constexpr std::string_view kCommandFocusCyclePrev = "mado.focus.cycle_prev";
constexpr std::string_view kCommandCursorUp = "mado.cursor.up";
constexpr std::string_view kCommandCursorDown = "mado.cursor.down";
constexpr std::string_view kCommandCursorLeft = "mado.cursor.left";
constexpr std::string_view kCommandCursorRight = "mado.cursor.right";
constexpr std::string_view kCommandLogScrollUp = "mado.log.scroll_up";
constexpr std::string_view kCommandLogScrollDown = "mado.log.scroll_down";
constexpr std::string_view kCommandLogSelectLine = "mado.log.select_line";
constexpr std::string_view kCommandSelectionCopy = "mado.selection.copy";

enum class FocusSurfaceKind {
  EditorBuffer,
  FilePicker,
  TerminalPopup,
  FilePopup,
  CliLog,
  ErrorLog,
  StatusPane,
  CommandPalette,
  PluginSurface,
};

struct FocusCommandTable {
  std::string focus_enter;
  std::string focus_leave;
  std::string focus_cycle_next;
  std::string focus_cycle_prev;
  std::string cursor_up;
  std::string cursor_down;
  std::string cursor_left;
  std::string cursor_right;
  std::string log_scroll_up;
  std::string log_scroll_down;
  std::string log_select_line;
  std::string selection_copy;
};

struct FocusNode {
  FocusTargetId id{kInvalidFocusTargetId};
  FocusSurfaceKind kind{FocusSurfaceKind::EditorBuffer};
  std::optional<FocusTargetId> parent_id;
  bool text_like{};
  bool log_like{};
  bool closed{};
  FocusCommandTable commands;
};

struct FocusContractStatus {
  bool has_focus_commands{};
  bool has_text_cursor_commands{true};
  bool has_log_selection_commands{true};

  [[nodiscard]] bool valid() const noexcept;
};

struct FocusCloseResult {
  FocusTargetId closed_id{kInvalidFocusTargetId};
  std::optional<FocusTargetId> focus_restore_id;
};

class FocusGraph {
 public:
  bool register_surface(FocusNode node);

  [[nodiscard]] FocusNode* find(FocusTargetId id) noexcept;
  [[nodiscard]] const FocusNode* find(FocusTargetId id) const noexcept;
  [[nodiscard]] std::size_t size() const noexcept;

  [[nodiscard]] bool focus(FocusTargetId id);
  [[nodiscard]] std::optional<FocusTargetId> focused_id() const noexcept;
  [[nodiscard]] std::optional<FocusTargetId> cycle_next();
  [[nodiscard]] std::optional<FocusTargetId> cycle_prev();

  [[nodiscard]] FocusCloseResult close_surface(FocusTargetId id);
  [[nodiscard]] FocusContractStatus validate_contract(FocusTargetId id) const noexcept;

 private:
  [[nodiscard]] std::optional<FocusTargetId> cycle(bool forward);

  std::vector<FocusNode> nodes_;
  std::vector<FocusTargetId> cycle_order_;
  std::optional<FocusTargetId> focused_id_;
};

}  // namespace editor_proto
