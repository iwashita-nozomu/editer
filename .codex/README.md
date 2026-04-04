# Codex Project Setup

このディレクトリは、Codex を primary runtime として使うための project-scoped 設定置き場です。

## Layout

- `config.toml`
  - Codex の project 設定
- `agents/*.toml`
  - Codex 用 subagent 定義

## Shared Canon

- 共通入口は `AGENTS.md`
- workflow と skill の正本は `agents/`
- Codex-specific routing は `agents/canonical/CODEX_WORKFLOW.md` と `agents/canonical/CODEX_SUBAGENTS.md`

## Current Agents

- `explorer`
- `reviewer`
- `worker`
- `docs_workflow_steward`
- `project_reviewer`
- `python_reviewer`
- `report_reviewer`
