# Codex Subagents

この文書は、Codex を primary runtime とする場合の subagent routing と inventory の正本です。
shared workflow は `agents/canonical/CODEX_WORKFLOW.md` に置き、この文書は specialist 起動の補足に限定します。

## Principles

- parent agent が最終編集責任を持つ
- subagent は必要なときだけ explicit に使う
- 調査、レビュー、文書整備は分ける
- 再帰的 fan-out は避ける

## Built-In Or Project-Scoped Roles

- `explorer`
  - 読み取り専用で codebase / docs / workflow の調査を行う
- `reviewer`
  - 読み取り専用で diff と risk を findings-first で洗う
- `python_reviewer`
  - Python diff を型、pytest、ruff 前提で洗う
- `worker`
  - bounded な実装変更を切り出す
- `docs_workflow_steward`
  - agent 文書、workflow、adapter file の整理を行う
- `project_reviewer`
  - repo-wide な inventory と workflow health を確認する
- `report_reviewer`
  - experiment report の根拠と reader-facing quality を確認する

## Recommended Routing

| Task Shape | Default Subagent Pattern |
| ---------- | ------------------------ |
| 局所修正 | parent 直処理。Python 差分なら `python_reviewer`、必要なら `explorer` の後に `reviewer` |
| 文書整理 | `docs_workflow_steward`、必要なら `reviewer` |
| repo-wide 棚卸し | `project_reviewer`、必要なら `explorer` |
| bounded 実装切り出し | `worker` の前後に `reviewer` |
| 実験や調査つき改善 | `explorer` で前提確認後、parent 実装。report があるなら `report_reviewer`。必要なら run bundle を作る |

## Runtime Surfaces

- human canon: `agents/`
- skill shim: `.agents/skills/`
- Codex project config: `.codex/config.toml`
- Codex subagent definitions: `.codex/agents/*.toml`
