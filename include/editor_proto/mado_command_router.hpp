// @dependency-start
// responsibility Routes lightweight Mado terminal commands such as `mado <filename>` to popup session actions.
// upstream implementation terminal_session.hpp popup session API
// downstream implementation ../../src/mado_command_router.cpp implements command routing
// downstream implementation ../../tests/cpp/prototype/mado_command_router_test.cpp validates command routing
// @dependency-end
#pragma once

#include "editor_proto/terminal_session.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace editor_proto {

enum class MadoCommandKind {
  None,
  OpenFile,
};

struct MadoCommand {
  MadoCommandKind kind{MadoCommandKind::None};
  std::string target;
};

struct MadoRouteResult {
  MadoCommand command;
  std::optional<PopupId> popup_id;
};

[[nodiscard]] MadoCommand parse_mado_command(const std::vector<std::string>& argv);
[[nodiscard]] bool is_mado_file_command(const MadoCommand& command) noexcept;
[[nodiscard]] MadoRouteResult route_mado_command(PopupSessionChain& chain,
                                                 std::optional<PopupId> parent_id,
                                                 std::string root,
                                                 const std::vector<std::string>& argv);
[[nodiscard]] std::string normalize_mado_file_target(std::string_view target);

}  // namespace editor_proto
