---
name: worktree-start
description: Use this skill when creating, recreating, or resuming a worktree and you need to lock scope, action-log paths, carry-over targets, and kickoff checks before editing.
---

# Worktree Start

1. Read `agents/skills/worktree-start.md`.
1. Refresh `WORKTREE_SCOPE.md` before the first edit. Fill concrete branch, path, editable directories, runtime outputs, references, carry-over targets, and required checks.
1. Create or update the action log from `notes/worktrees/WORKTREE_LOG_TEMPLATE.md`, then append the first kickoff entry before changing code or docs.
1. If the branch will survive multiple sessions or needs handoff, create or update `notes/branches/<branch_topic>.md`.
1. Run `git status --short --branch` and `git worktree list --porcelain`.
1. When multiple worktrees exist or the resumed state is unclear, run `bash scripts/tools/check_worktree_scopes.sh`.
1. If you see dirty state, stale scope, or conflict risk, record it in the action log before editing and switch to `worktree-health` if cleanup or drift review is needed.
