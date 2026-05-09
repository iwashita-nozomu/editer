// @dependency-start
// responsibility Validates the Mado plugin registry and builtin plugin manifests.
// upstream implementation ../../../include/editor_proto/plugin_registry.hpp declares the registry API
// upstream implementation ../../../src/plugin_registry.cpp implements builtin manifests
// downstream build ../../../CMakeLists.txt builds this executable as editor_proto_plugin_registry_test
// @dependency-end
#include "editor_proto/plugin_registry.hpp"

#include <cassert>
#include <string_view>

namespace {

bool has_permission(const editor_proto::PluginManifest& manifest,
                    editor_proto::PluginPermission permission) {
  for (const auto candidate : manifest.permissions) {
    if (candidate == permission) return true;
  }
  return false;
}

void assert_command_owner(const editor_proto::PluginRegistry& registry, std::string_view command_id,
                          std::string_view owner_plugin_id) {
  const editor_proto::PluginCommand* command = registry.find_command(command_id);
  assert(command != nullptr);
  assert(command->owner_plugin_id == owner_plugin_id);
}

}  // namespace

int main() {
  const editor_proto::PluginRegistry registry = editor_proto::make_default_plugin_registry();

  assert(registry.plugins().size() == 4U);
  assert(registry.find_plugin("mado.core") != nullptr);
  assert(registry.find_plugin("mado.terminal") != nullptr);
  assert(registry.find_plugin("mado.ssh") != nullptr);
  assert(registry.find_plugin("mado.devcontainer") != nullptr);
  assert(registry.find_plugin("mado.missing") == nullptr);

  const editor_proto::PluginManifest* core = registry.find_plugin("mado.core");
  assert(core != nullptr);
  assert(core->kind == editor_proto::PluginKind::Core);
  assert(core->permissions.empty());
  assert_command_owner(registry, "mado.file.save", "mado.core");
  assert_command_owner(registry, "mado.focus.cycle_next", "mado.core");
  assert_command_owner(registry, "mado.cursor.right", "mado.core");
  assert_command_owner(registry, "mado.selection.copy", "mado.core");

  const editor_proto::PluginManifest* terminal = registry.find_plugin("mado.terminal");
  assert(terminal != nullptr);
  assert(has_permission(*terminal, editor_proto::PluginPermission::TerminalPty));
  assert_command_owner(registry, "mado.terminal.open", "mado.terminal");
  assert_command_owner(registry, "mado.log.select_line", "mado.terminal");

  const editor_proto::PluginManifest* ssh = registry.find_plugin("mado.ssh");
  assert(ssh != nullptr);
  assert(has_permission(*ssh, editor_proto::PluginPermission::NetworkSsh));
  assert(has_permission(*ssh, editor_proto::PluginPermission::HostLaunch));
  assert_command_owner(registry, "mado.ssh.print_helper", "mado.ssh");

  const editor_proto::PluginManifest* devcontainer = registry.find_plugin("mado.devcontainer");
  assert(devcontainer != nullptr);
  assert(has_permission(*devcontainer, editor_proto::PluginPermission::DockerAccess));
  assert_command_owner(registry, "mado.devcontainer.open", "mado.devcontainer");

  assert(registry.command_exists("mado.window.quit"));
  assert(!registry.command_exists("mado.unknown"));
  assert(std::string_view(editor_proto::plugin_kind_name(editor_proto::PluginKind::Builtin)) ==
         "builtin");
  assert(std::string_view(editor_proto::plugin_permission_name(
             editor_proto::PluginPermission::DockerAccess)) == "docker.access");

  return 0;
}
