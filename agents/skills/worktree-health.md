# worktree-health

## Purpose

現在の worktree が、scope、branch、未コミット差分、conflict risk の観点で健全かを確認します。

## Use When

- `WORKTREE_SCOPE.md` の存在と内容確認
- editable directories / runtime output directories の逸脱確認
- worktree の clean / dirty 状態確認
- conflict risk や carry-over 漏れの確認
- 削除前の健全性チェック

## Core References

- `documents/worktree-lifecycle.md`
- `documents/WORKTREE_SCOPE_TEMPLATE.md`
- `scripts/tools/check_worktree_scopes.sh`
- `scripts/agent_tools/validate_role_write_scope.py`

## Mandatory Checklist

- `WORKTREE_SCOPE.md` がある
- editable directories の外に repo edit が出ていない
- runtime output が許可ディレクトリに収まっている
- worktree の目的と branch 名が一致している
- carry-over すべき note や result が取り残されていない

## Boundary

- worktree 開始時の初期化は `worktree-start` を使います。
- repo 全体レビューは `project-review` を使います。
