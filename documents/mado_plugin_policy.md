# Mado Plugin Policy
<!--
@dependency-start
responsibility Defines the Mado core/plugin ownership boundary and builtin plugin rules.
upstream implementation ../include/editor_proto/plugin_registry.hpp declares the plugin manifest and registry API
upstream implementation ../src/plugin_registry.cpp registers core, terminal, SSH, and devcontainer builtin plugin manifests
downstream design ../src/README.md documents the runtime architecture
downstream design mado_command_list.md lists plugin-owned runtime commands
@dependency-end
-->

Mado keeps editor state and window orchestration in core, and moves environment-specific behavior behind plugin manifests.
The first implementation uses builtin plugins linked into `editor_proto`; dynamic plugin loading is intentionally out of scope until the core API becomes stable.

## Core Ownership

`mado.core` owns the stable editor surface:

- editor buffers, tabs, dirty state, cursor state, and file saves
- workspace root selection and root-scoped file listing
- floating window creation and focus routing
- command dispatch and keybinding resolution
- runtime config parsing and runtime logging fanout
- plugin registry creation and plugin command lookup

Core must not grow direct feature-specific branches for SSH, Docker, devcontainer, or other host integrations. Those features register commands and permissions through plugin manifests.

## Plugin Ownership

Builtin and future external plugins own optional integration surfaces:

- `mado.terminal`: floating PTY terminal windows, terminal input execution, terminal selection, copy, and clear commands
- `mado.ssh`: SSH helper generation, host-side launch entrypoints, and SSH-oriented terminal integration
- `mado.devcontainer`: devcontainer detection, Docker-backed open/rebuild commands, and container workspace launch flows

Plugins may request core services only through public API surfaces. A plugin must not directly mutate editor buffers, tabs, focus state, or root state except through a core-owned command or API.

## Manifest Contract

Every plugin has a manifest with:

- stable `id`, using the `mado.<name>` namespace for builtin plugins
- user-facing `name`
- semantic `version`
- `kind`: `core`, `builtin`, or `external`
- declared permissions
- declared command IDs and titles

Builtin plugins use the same manifest contract as future external plugins. This keeps the boundary testable before dynamic loading exists.

## Permission Contract

Known permission names are:

- `workspace.read`: read workspace files or metadata
- `workspace.write`: write workspace files or generated config
- `process.spawn`: spawn child processes
- `terminal.pty`: create or operate PTY-backed terminal sessions
- `network.ssh`: initiate SSH-related flows
- `docker.access`: call Docker or devcontainer tooling
- `host.launch`: launch or re-enter a host-side Mado binary

Plugin implementations must log the plugin ID when exercising a permission that can spawn processes, touch the network, access Docker, or launch host commands.

## Command ID Contract

Command IDs are the stable routing layer between keybindings, UI buttons, CLI aliases, and plugin implementations.

- Core commands use `mado.<area>.<verb>`.
- Plugin commands use `mado.<plugin>.<verb>`.
- Existing UI shortcuts should be migrated to command IDs before feature-specific branching grows further.
- A command ID must not be reused for a different behavior after release.

Initial command ownership:

| Plugin | Commands |
| --- | --- |
| `mado.core` | `mado.files.open`, `mado.file.save`, `mado.demo.run`, `mado.window.quit`, `mado.focus.editor`, `mado.focus.notice`, `mado.focus.files` |
| `mado.terminal` | `mado.terminal.open`, `mado.terminal.run`, `mado.terminal.clear`, `mado.terminal.copy_selection` |
| `mado.ssh` | `mado.ssh.print_helper`, `mado.ssh.host_launch` |
| `mado.devcontainer` | `mado.devcontainer.open`, `mado.devcontainer.rebuild` |

## Configuration Contract

Plugin configuration lives under `plugins.<plugin-id>` in `mado.yaml` once runtime config support is added.
Until then, builtin plugins may keep existing command-line compatibility, but the documented owner should still be the plugin.

Example future shape:

```yaml
plugins:
  mado.ssh:
    enabled: true
    allowed_hosts:
      - "*"
  mado.devcontainer:
    enabled: true
    default_service: app
```

## Migration Rule

When a feature touches host tools, SSH, Docker, containers, or other external processes, first add or update its plugin manifest and command ownership. Then migrate implementation behind that command in a follow-up change.

This PR establishes the boundary and builtin manifests only. It does not change the runtime behavior of the existing Mado GUI.
