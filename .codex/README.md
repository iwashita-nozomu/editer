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
- `reproducibility_reviewer`
- `scientific_computing_reviewer`
- `benchmark_reviewer`
- `artifact_reviewer`
- `fair_data_reviewer`
- `ml_science_reviewer`

## Smoke Test

subagent inventory や research perspective pack を触ったら、次で bundle と runtime surface を確認します。

```bash
python3 scripts/agent_tools/smoke_test_research_perspective_pack.py
```
