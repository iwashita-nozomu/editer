# worktree-start

## Purpose

新しい worktree を切った直後、または既存 worktree を引き継いだ直後に、scope、参照、action log、初期チェックを揃えます。

## Use When

- new / recreated worktree の kickoff
- stale な worktree の再開前確認
- `WORKTREE_SCOPE.md` の再作成や scope refresh

## Core References

- `documents/worktree-lifecycle.md`
- `documents/WORKTREE_SCOPE_TEMPLATE.md`
- `documents/BRANCH_SCOPE.md`
- `notes/worktrees/README.md`
- `scripts/setup_worktree.sh`

## Mandatory Checklist

- `WORKTREE_SCOPE.md` の `Branch` と `Worktree path` が current state と一致する
- `Required References Before Editing` に concrete file を書く
- `notes/worktrees/worktree_<topic>_YYYY-MM-DD.md` の path を決める
- carry-over target と runtime output directory を決める
- `git status --short --branch` を確認する
- `git worktree list --porcelain` を確認する

## Default Commands

- `bash scripts/setup_worktree.sh <branch-name> [worktree-path]`
- `git status --short --branch`
- `git worktree list --porcelain`
- `bash scripts/tools/check_worktree_scopes.sh`

## Boundary

- cleanup readiness や delete 可否の review は `worktree-health` を使います。
- repo-wide な routing や CI / Docker review は `project-review` を使います。
