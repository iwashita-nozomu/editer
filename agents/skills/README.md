# Shared Skill Canon

このディレクトリは、Codex を主 runtime としつつ、Claude や Copilot でも共有する skill 文書の人間向け正本です。
機械 discovery 用の `SKILL.md` は `.agents/skills/` と `.claude/skills/` に置き、判断基準と使い分けはここに集約します。

## Rules

- skill の目的、使う場面、関連正本は `agents/skills/` に書きます。
- `AGENTS.md` や `CLAUDE.md` には長い skill 説明を複製しません。
- `.agents/skills/` は Codex / Copilot の auto-discovery path です。
- `.claude/skills/` は Claude 互換 mirror です。
- 新しい skill を追加するときは `catalog.yaml` と対応文書を同時に更新します。
- この template では Python と Markdown を常に前提にするため、`python-review` と `md-style-check` は頻出 skill です。

## Skill Families

| Family | Purpose | Canonical Doc | Discovery Shim |
| ------ | ------- | ------------- | -------------- |
| `repo-onboarding` | unfamiliar repo の最短入口確認 | `agents/skills/repo-onboarding.md` | `.agents/skills/repo-onboarding/SKILL.md` |
| `agent-orchestration` | workflow family 選択と role/handoff 整理 | `agents/skills/agent-orchestration.md` | `.agents/skills/agent-orchestration/SKILL.md` |
| `artifact-placement` | task 文書、run artifact、repo 正本の置き分け | `agents/skills/artifact-placement.md` | `.agents/skills/artifact-placement/SKILL.md` |
| `subagent-bootstrap` | run bundle と specialist 起動 | `agents/skills/subagent-bootstrap.md` | `.agents/skills/subagent-bootstrap/SKILL.md` |
| `codex-cli` | Codex 用の入口と読順 | `agents/skills/codex-cli.md` | `.agents/skills/codex-cli/SKILL.md` |
| `codex-task-workflow` | Codex の context-independent task 実行 | `agents/skills/codex-task-workflow.md` | `.agents/skills/codex-task-workflow/SKILL.md` |
| `claude-code-cli` | Claude Code 用の入口差分 | `agents/skills/claude-code-cli.md` | `.agents/skills/claude-code-cli/SKILL.md` |
| `copilot-cli` | Copilot 用の入口差分 | `agents/skills/copilot-cli.md` | `.agents/skills/copilot-cli/SKILL.md` |
| `static-validation` | lint / test / docs / links の確認 | `agents/skills/static-validation.md` | `.agents/skills/static-validation/SKILL.md` |
| `python-review` | pyright / pytest / ruff を前提にした Python review | `agents/skills/python-review.md` | `.agents/skills/python-review/SKILL.md` |
| `md-style-check` | Markdown の体裁とリンク確認 | `agents/skills/md-style-check.md` | `.agents/skills/md-style-check/SKILL.md` |
| `docs-consistency-review` | 文書間の矛盾と stale route の確認 | `agents/skills/docs-consistency-review.md` | `.agents/skills/docs-consistency-review/SKILL.md` |
| `change-review` | findings-first review | `agents/skills/change-review.md` | `.agents/skills/change-review/SKILL.md` |
| `experiment-workflow` | question, protocol, run, report の整理 | `agents/skills/experiment-workflow.md` | `.agents/skills/experiment-workflow/SKILL.md` |
| `critical-review` | 過大主張、比較条件、根拠不足の確認 | `agents/skills/critical-review.md` | `.agents/skills/critical-review/SKILL.md` |
| `report-review` | experiment report の reader-facing review | `agents/skills/report-review.md` | `.agents/skills/report-review/SKILL.md` |
| `environment-maintenance` | Docker / CI / dependency / runtime 更新 | `agents/skills/environment-maintenance.md` | `.agents/skills/environment-maintenance/SKILL.md` |

## Codex Defaults

- Codex では `AGENTS.md` と `agents/canonical/CODEX_WORKFLOW.md` を先に読みます。
- task ごとの skill 選択は、このディレクトリか `catalog.yaml` を見て決めます。
- specialist を使う場合の Codex-specific routing は `agents/canonical/CODEX_SUBAGENTS.md` を見ます。

## Updating Skills

1. `agents/skills/<family>.md` を更新する
1. `agents/skills/catalog.yaml` を更新する
1. `.agents/skills/<family>/SKILL.md` を更新する
1. Claude mirror が必要なら `.claude/skills/<family>/SKILL.md` も更新する
1. 必要なら `agents/canonical/CODEX_WORKFLOW.md` と `agents/canonical/CODEX_SUBAGENTS.md` の routing を更新する
