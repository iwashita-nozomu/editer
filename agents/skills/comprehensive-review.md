# comprehensive-review

## Purpose

repo 全体を横断して、文書、skill、ツール、統合設定の破綻をまとめて検査します。

## Use When

- 文書体系の棚卸し
- skill 間の重複や未整合の確認
- 自動化や integration point の確認
- repo-wide な整理や workflow 改造の完了判定

## Core References

- `scripts/run_comprehensive_review.sh`
- `documents/WORKFLOW_INVENTORY.md`
- `agents/skills/project-review.md`

## Boundary

- 局所 diff のレビューだけなら `code-review` を使います。
- repo-wide review の最上位入口としては `project-review` を使います。
- 研究系の独立視点 review は `research-perspective-review` を使います。
