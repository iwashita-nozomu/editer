# project-review

## Purpose

repo 全体を横断して、構成、文書、skills、ツール、静的健全性、運用 drift をまとめてレビューします。

## Use When

- repo-wide な棚卸し
- workflow、agent system、tools、docs をまたぐ全体レビュー
- 大きな整理や統合変更の完了判定
- stale 文書、重複定義、未参照資産、運用 drift の検出

## Core References

- `agents/skills/comprehensive-review.md`
- `agents/skills/project-health.md`
- `documents/REVIEW_PROCESS.md`
- `documents/AGENTS_COORDINATION.md`
- `documents/coding-conventions-project.md`

## Mandatory Phases

1. `Inventory`
1. `Static Health`
1. `Workflow Health`
1. `Tooling Health`
1. `Worktree Health`
1. `Follow-Up Decision`

## Boundary

- 局所 diff のレビューだけなら `code-review` を使います。
- Python 差分の静的解析中心レビューなら `python-review` を使います。
- 実験 result や report の妥当性は `critical-review` と `report-review` を使います。
