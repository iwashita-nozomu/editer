# Canonical Skill Registry

## Curated Skills

- `repo-onboarding`
  - unfamiliar repo の入口確認
- `agent-orchestration`
  - workflow family 選択と handoff 整理
- `artifact-placement`
  - task 文書、run artifact、repo 正本の置き分け
- `subagent-bootstrap`
  - specialist 起動、report bundle、write-scope 整理
- `static-validation`
  - lint / test / link / CI 確認
- `python-review`
  - pyright / pytest / ruff を前提にした Python review
- `md-style-check`
  - Markdown の体裁とリンク確認
- `docs-consistency-review`
  - 文書間の矛盾と stale route の確認
- `change-review`
  - findings-first review
- `experiment-workflow`
  - question, protocol, run, report の整理
- `research-workflow`
  - 外部調査、比較設計、run loop、decision state の整理
- `critical-review`
  - fairness, overclaim, missing evidence の確認
- `report-review`
  - experiment report の reader-facing review
- `research-perspective-review`
  - 研究系変更を複数視点で並列レビュー
- `environment-maintenance`
  - Docker, CI, dependency, runtime 更新
- `codex-cli`
  - Codex 用の入口と skill path
- `codex-task-workflow`
  - Codex の context-independent task 実行フロー
- `claude-code-cli`
  - Claude Code 用の入口と subagent path
- `copilot-cli`
  - Copilot CLI / coding agent 用の入口と注意点

## Discovery Paths

- Codex / Copilot:
  - `.agents/skills/<skill>/SKILL.md`
- Claude:
  - `.claude/skills/<skill>/SKILL.md`

## Human Canon

- skill purpose and routing:
  - `agents/skills/README.md`
- machine-readable skill catalog:
  - `agents/skills/catalog.yaml`
