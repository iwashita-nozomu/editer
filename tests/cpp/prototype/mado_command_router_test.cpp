// @dependency-start
// responsibility Validates lightweight Mado terminal command routing to popup actions.
// upstream implementation ../../../include/editor_proto/mado_command_router.hpp declares router API
// upstream implementation ../../../src/mado_command_router.cpp implements router behavior
// downstream build ../../../CMakeLists.txt builds this executable as editor_proto_mado_command_router_test
// @dependency-end
#include "editor_proto/mado_command_router.hpp"

#include <cassert>
#include <string_view>
#include <vector>

int main() {
  const editor_proto::MadoCommand open = editor_proto::parse_mado_command({"mado", "src/main.cpp"});
  assert(open.kind == editor_proto::MadoCommandKind::OpenFile);
  assert(open.target == "src/main.cpp");
  assert(editor_proto::is_mado_file_command(open));

  const editor_proto::MadoCommand trimmed = editor_proto::parse_mado_command({"mado", "  README.md  "});
  assert(trimmed.kind == editor_proto::MadoCommandKind::OpenFile);
  assert(trimmed.target == "README.md");

  assert(editor_proto::parse_mado_command({"mado"}).kind == editor_proto::MadoCommandKind::None);
  assert(editor_proto::parse_mado_command({"mado", "--test-workspace"}).kind ==
         editor_proto::MadoCommandKind::None);
  assert(editor_proto::parse_mado_command({"other", "README.md"}).kind ==
         editor_proto::MadoCommandKind::None);

  editor_proto::PopupSessionChain chain;
  const editor_proto::PopupId terminal = chain.create_terminal(editor_proto::PopupLaunchRequest{
      .title = "local",
      .root = "/repo",
      .launch_context = "local shell",
  });

  const editor_proto::MadoRouteResult routed = editor_proto::route_mado_command(
      chain, terminal, "/repo", std::vector<std::string>{"mado", "src/main.cpp"});
  assert(routed.command.kind == editor_proto::MadoCommandKind::OpenFile);
  assert(routed.popup_id.has_value());

  const editor_proto::PopupSession* file = chain.find(*routed.popup_id);
  assert(file != nullptr);
  assert(file->kind == editor_proto::PopupKind::File);
  assert(file->parent_id == terminal);
  assert(file->file_path == "src/main.cpp");
  assert(file->launch_budget_ms <= 100U);
  assert(chain.child_of(terminal) == routed.popup_id);

  const editor_proto::PopupCloseResult close = chain.close_popup(*routed.popup_id);
  assert(close.focus_restore_id == terminal);

  return 0;
}
