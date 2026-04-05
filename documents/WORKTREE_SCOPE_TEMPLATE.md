# WORKTREE_SCOPE Template

このファイルは、他環境へ渡す worktree や、変更範囲を限定して使う worktree のためのテンプレートです。
実際に使うときは、このファイルを worktree root に `WORKTREE_SCOPE.md` として置きます。

## Worktree Summary

- Branch:
- Worktree path:
- Purpose:
- Owner or agent:

## Editable Directories

- `path/to/dir`
- `another/path`

## Runtime Output Directories

- `experiments/<topic>/result/`
- `experiments/<area>/<topic>/result/`

## Read-Only Or Avoid Directories

- `path/to/avoid`
- `another/path`

## Required References Before Editing

- [documents/worktree-lifecycle.md](/mnt/l/workspace/project_template/documents/worktree-lifecycle.md)
- [documents/coding-conventions-project.md](/mnt/l/workspace/project_template/documents/coding-conventions-project.md)
- `documents/...`
- `notes/...`
- `reviews/...`

## Main Carry-Over Targets

- `notes/worktrees/worktree_<topic>_YYYY-MM-DD.md`
- `notes/experiments/<topic>.md`
- `notes/experiments/results/<topic>_<date>.json`
- `notes/branches/<branch_topic>.md`

## Working Notes During Execution

- Action log path: `notes/worktrees/worktree_<topic>_YYYY-MM-DD.md`
- Experiment memo path: `notes/experiments/<topic>.md`
- Branch summary path: `notes/branches/<branch_topic>.md`
- worktree 内でも、最終配置と同じ相対パスで下書きする

## Required Checks Before Commit

- `pyright`
- `markdownlint`
- `pytest ...`

## Additional Rules

- ここに、この worktree 固有の制約を書きます。
- 例: テストは触らない、結果 JSON は commit しない、runner だけ変更する、など。
- 例: 変更した Markdown は `.markdownlint.json` を基準に確認する。
- 例: scope 更新、編集開始、テスト実行、実験開始 / 停止、carry-over 判断は action log に逐次追記する。
