// @dependency-start
// responsibility Validates the minimal keyboard-only focus graph and contract invariants.
// upstream implementation ../../../include/editor_proto/focus_graph.hpp declares focus graph API
// upstream implementation ../../../src/focus_graph.cpp implements focus graph behavior
// downstream build ../../../CMakeLists.txt builds this executable as editor_proto_focus_graph_test
// @dependency-end
#include "editor_proto/focus_graph.hpp"

#include <cassert>
#include <string>
#include <string_view>

namespace {

editor_proto::FocusCommandTable make_full_commands() {
  return editor_proto::FocusCommandTable{
      .focus_enter = std::string(editor_proto::kCommandFocusEnter),
      .focus_leave = std::string(editor_proto::kCommandFocusLeave),
      .focus_cycle_next = std::string(editor_proto::kCommandFocusCycleNext),
      .focus_cycle_prev = std::string(editor_proto::kCommandFocusCyclePrev),
      .cursor_up = std::string(editor_proto::kCommandCursorUp),
      .cursor_down = std::string(editor_proto::kCommandCursorDown),
      .cursor_left = std::string(editor_proto::kCommandCursorLeft),
      .cursor_right = std::string(editor_proto::kCommandCursorRight),
      .log_scroll_up = std::string(editor_proto::kCommandLogScrollUp),
      .log_scroll_down = std::string(editor_proto::kCommandLogScrollDown),
      .log_select_line = std::string(editor_proto::kCommandLogSelectLine),
      .selection_copy = std::string(editor_proto::kCommandSelectionCopy),
  };
}

}  // namespace

int main() {
  editor_proto::FocusGraph graph;

  assert(graph.register_surface(editor_proto::FocusNode{
      .id = 1U,
      .kind = editor_proto::FocusSurfaceKind::EditorBuffer,
      .parent_id = std::nullopt,
      .text_like = true,
      .log_like = false,
      .commands = make_full_commands(),
  }));
  assert(graph.register_surface(editor_proto::FocusNode{
      .id = 2U,
      .kind = editor_proto::FocusSurfaceKind::TerminalPopup,
      .parent_id = 1U,
      .text_like = true,
      .log_like = true,
      .commands = make_full_commands(),
  }));
  assert(graph.register_surface(editor_proto::FocusNode{
      .id = 3U,
      .kind = editor_proto::FocusSurfaceKind::CliLog,
      .parent_id = 2U,
      .text_like = false,
      .log_like = true,
      .commands = make_full_commands(),
  }));

  assert(graph.size() == 3U);
  assert(!graph.register_surface(editor_proto::FocusNode{
      .id = 2U,
      .kind = editor_proto::FocusSurfaceKind::FilePopup,
      .commands = make_full_commands(),
  }));
  assert(!graph.register_surface(editor_proto::FocusNode{
      .id = 4U,
      .kind = editor_proto::FocusSurfaceKind::FilePopup,
      .parent_id = 99U,
      .commands = make_full_commands(),
  }));

  assert(graph.focus(2U));
  assert(graph.focused_id() == 2U);
  assert(graph.cycle_next() == 3U);
  assert(graph.cycle_next() == 1U);
  assert(graph.cycle_prev() == 3U);

  const editor_proto::FocusContractStatus editor_status = graph.validate_contract(1U);
  assert(editor_status.valid());

  editor_proto::FocusNode* log_node = graph.find(3U);
  assert(log_node != nullptr);
  log_node->commands.log_select_line.clear();
  const editor_proto::FocusContractStatus invalid_log_status = graph.validate_contract(3U);
  assert(!invalid_log_status.valid());
  assert(invalid_log_status.has_focus_commands);
  assert(invalid_log_status.has_text_cursor_commands);
  assert(!invalid_log_status.has_log_selection_commands);
  log_node->commands.log_select_line = std::string(editor_proto::kCommandLogSelectLine);

  assert(graph.focus(3U));
  const editor_proto::FocusCloseResult close = graph.close_surface(3U);
  assert(close.closed_id == 3U);
  assert(close.focus_restore_id == 2U);
  assert(graph.focused_id() == 2U);

  const editor_proto::FocusNode* closed = graph.find(3U);
  assert(closed != nullptr);
  assert(closed->closed);
  assert(!graph.focus(3U));

  assert(std::string_view(editor_proto::kCommandFocusEnter) == "mado.focus.enter");
  assert(std::string_view(editor_proto::kCommandSelectionCopy) == "mado.selection.copy");
  return 0;
}
