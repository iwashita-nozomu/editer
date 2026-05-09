// @dependency-start
// responsibility Implements Mado plugin registry and builtin plugin manifests.
// upstream implementation ../include/editor_proto/plugin_registry.hpp registry API
// upstream design ../documents/mado_plugin_policy.md plugin ownership and permission contract
// downstream implementation ../tests/cpp/prototype/plugin_registry_test.cpp validates registry behavior
// @dependency-end
#include "editor_proto/plugin_registry.hpp"

#include <algorithm>
#include <utility>

namespace editor_proto {

void PluginRegistry::register_plugin(PluginManifest manifest) {
  plugins_.push_back(std::move(manifest));
}

const std::vector<PluginManifest>& PluginRegistry::plugins() const noexcept { return plugins_; }

const PluginManifest* PluginRegistry::find_plugin(std::string_view id) const noexcept {
  const auto found = std::find_if(plugins_.begin(), plugins_.end(), [id](const PluginManifest& plugin) {
    return plugin.id == id;
  });
  if (found == plugins_.end()) return nullptr;
  return &*found;
}

const PluginCommand* PluginRegistry::find_command(std::string_view id) const noexcept {
  for (const PluginManifest& plugin : plugins_) {
    const auto found = std::find_if(plugin.commands.begin(), plugin.commands.end(),
                                    [id](const PluginCommand& command) { return command.id == id; });
    if (found != plugin.commands.end()) return &*found;
  }
  return nullptr;
}

bool PluginRegistry::command_exists(std::string_view id) const noexcept { return find_command(id) != nullptr; }

const char* plugin_kind_name(PluginKind kind) noexcept {
  switch (kind) {
    case PluginKind::Core:
      return "core";
    case PluginKind::Builtin:
      return "builtin";
    case PluginKind::External:
      return "external";
  }
  return "unknown";
}

const char* plugin_permission_name(PluginPermission permission) noexcept {
  switch (permission) {
    case PluginPermission::WorkspaceRead:
      return "workspace.read";
    case PluginPermission::WorkspaceWrite:
      return "workspace.write";
    case PluginPermission::ProcessSpawn:
      return "process.spawn";
    case PluginPermission::TerminalPty:
      return "terminal.pty";
    case PluginPermission::NetworkSsh:
      return "network.ssh";
    case PluginPermission::DockerAccess:
      return "docker.access";
    case PluginPermission::HostLaunch:
      return "host.launch";
  }
  return "unknown";
}

PluginManifest make_core_plugin_manifest() {
  return PluginManifest{
      .id = "mado.core",
      .name = "Mado Core",
      .version = "0.1.0",
      .kind = PluginKind::Core,
      .permissions = {},
      .commands = {
          PluginCommand{"mado.files.open", "Open Files", "mado.core"},
          PluginCommand{"mado.file.save", "Save Active File", "mado.core"},
          PluginCommand{"mado.demo.run", "Run Core Demo", "mado.core"},
          PluginCommand{"mado.window.quit", "Quit Active Window", "mado.core"},
          PluginCommand{"mado.focus.editor", "Focus Editor", "mado.core"},
          PluginCommand{"mado.focus.notice", "Focus Notice", "mado.core"},
          PluginCommand{"mado.focus.files", "Focus Files", "mado.core"},
      },
  };
}

PluginManifest make_terminal_plugin_manifest() {
  return PluginManifest{
      .id = "mado.terminal",
      .name = "Mado Terminal",
      .version = "0.1.0",
      .kind = PluginKind::Builtin,
      .permissions = {PluginPermission::WorkspaceRead, PluginPermission::ProcessSpawn,
                      PluginPermission::TerminalPty},
      .commands = {
          PluginCommand{"mado.terminal.open", "Open Floating Terminal", "mado.terminal"},
          PluginCommand{"mado.terminal.run", "Run Terminal Input", "mado.terminal"},
          PluginCommand{"mado.terminal.open_file", "Open File From Terminal", "mado.terminal"},
          PluginCommand{"mado.terminal.close_chain", "Close Terminal Chain", "mado.terminal"},
          PluginCommand{"mado.terminal.clear", "Clear Terminal Output", "mado.terminal"},
          PluginCommand{"mado.terminal.copy_selection", "Copy Terminal Selection", "mado.terminal"},
      },
  };
}

PluginManifest make_ssh_plugin_manifest() {
  return PluginManifest{
      .id = "mado.ssh",
      .name = "Mado SSH Integration",
      .version = "0.1.0",
      .kind = PluginKind::Builtin,
      .permissions = {PluginPermission::WorkspaceRead, PluginPermission::ProcessSpawn,
                      PluginPermission::TerminalPty, PluginPermission::NetworkSsh,
                      PluginPermission::HostLaunch},
      .commands = {
          PluginCommand{"mado.ssh.print_helper", "Print SSH Helper", "mado.ssh"},
          PluginCommand{"mado.ssh.host_launch", "Launch Host Mado", "mado.ssh"},
      },
  };
}

PluginManifest make_devcontainer_plugin_manifest() {
  return PluginManifest{
      .id = "mado.devcontainer",
      .name = "Mado Dev Container Integration",
      .version = "0.1.0",
      .kind = PluginKind::Builtin,
      .permissions = {PluginPermission::WorkspaceRead, PluginPermission::WorkspaceWrite,
                      PluginPermission::ProcessSpawn, PluginPermission::DockerAccess},
      .commands = {
          PluginCommand{"mado.devcontainer.open", "Open Dev Container", "mado.devcontainer"},
          PluginCommand{"mado.devcontainer.rebuild", "Rebuild Dev Container", "mado.devcontainer"},
      },
  };
}

PluginRegistry make_default_plugin_registry() {
  PluginRegistry registry;
  registry.register_plugin(make_core_plugin_manifest());
  registry.register_plugin(make_terminal_plugin_manifest());
  registry.register_plugin(make_ssh_plugin_manifest());
  registry.register_plugin(make_devcontainer_plugin_manifest());
  return registry;
}

}  // namespace editor_proto
