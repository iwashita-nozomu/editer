// @dependency-start
// responsibility Implements lightweight Mado terminal command routing for popup actions.
// upstream implementation ../include/editor_proto/mado_command_router.hpp command router API
// upstream implementation ../include/editor_proto/terminal_session.hpp popup session API
// downstream implementation ../tests/cpp/prototype/mado_command_router_test.cpp validates command routing
// @dependency-end
#include "editor_proto/mado_command_router.hpp"

#include <algorithm>
#include <utility>

namespace editor_proto {

namespace {

bool is_option(std::string_view value) { return value.rfind("--", 0U) == 0U; }

}  // namespace

std::string normalize_mado_file_target(std::string_view target) {
  std::string normalized{target};
  normalized.erase(normalized.begin(), std::find_if(normalized.begin(), normalized.end(), [](char ch) {
    return ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r';
  }));
  normalized.erase(std::find_if(normalized.rbegin(), normalized.rend(), [](char ch) {
                     return ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r';
                   }).base(),
                   normalized.end());
  return normalized;
}

MadoCommand parse_mado_command(const std::vector<std::string>& argv) {
  if (argv.size() != 2U) return {};
  if (argv.front() != "mado") return {};

  const std::string target = normalize_mado_file_target(argv.back());
  if (target.empty() || is_option(target)) return {};
  return MadoCommand{.kind = MadoCommandKind::OpenFile, .target = target};
}

bool is_mado_file_command(const MadoCommand& command) noexcept {
  return command.kind == MadoCommandKind::OpenFile && !command.target.empty();
}

MadoRouteResult route_mado_command(PopupSessionChain& chain, std::optional<PopupId> parent_id,
                                   std::string root, const std::vector<std::string>& argv) {
  const MadoCommand command = parse_mado_command(argv);
  if (!is_mado_file_command(command)) return MadoRouteResult{.command = command, .popup_id = std::nullopt};

  const PopupId popup_id = chain.create_file_popup(parent_id, std::move(root), command.target);
  return MadoRouteResult{.command = command, .popup_id = popup_id};
}

}  // namespace editor_proto
