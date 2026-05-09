// @dependency-start
// responsibility Defines the stable core/plugin boundary and builtin plugin registry API.
// upstream design ../../documents/mado_plugin_policy.md plugin ownership and permission contract
// downstream implementation ../../src/plugin_registry.cpp implements registry behavior
// downstream implementation ../../tests/cpp/prototype/plugin_registry_test.cpp validates plugin manifests
// @dependency-end
#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace editor_proto {

enum class PluginKind {
  Core,
  Builtin,
  External,
};

enum class PluginPermission {
  WorkspaceRead,
  WorkspaceWrite,
  ProcessSpawn,
  TerminalPty,
  NetworkSsh,
  DockerAccess,
  HostLaunch,
};

struct PluginCommand {
  std::string id;
  std::string title;
  std::string owner_plugin_id;
};

struct PluginManifest {
  std::string id;
  std::string name;
  std::string version;
  PluginKind kind{PluginKind::Builtin};
  std::vector<PluginPermission> permissions;
  std::vector<PluginCommand> commands;
};

class PluginRegistry {
 public:
  void register_plugin(PluginManifest manifest);

  [[nodiscard]] const std::vector<PluginManifest>& plugins() const noexcept;
  [[nodiscard]] const PluginManifest* find_plugin(std::string_view id) const noexcept;
  [[nodiscard]] const PluginCommand* find_command(std::string_view id) const noexcept;
  [[nodiscard]] bool command_exists(std::string_view id) const noexcept;

 private:
  std::vector<PluginManifest> plugins_;
};

[[nodiscard]] const char* plugin_kind_name(PluginKind kind) noexcept;
[[nodiscard]] const char* plugin_permission_name(PluginPermission permission) noexcept;
[[nodiscard]] PluginManifest make_core_plugin_manifest();
[[nodiscard]] PluginManifest make_terminal_plugin_manifest();
[[nodiscard]] PluginManifest make_ssh_plugin_manifest();
[[nodiscard]] PluginManifest make_devcontainer_plugin_manifest();
[[nodiscard]] PluginRegistry make_default_plugin_registry();

}  // namespace editor_proto
