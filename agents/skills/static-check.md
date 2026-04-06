# static-check

## Purpose

実装変更の直後に、速く回せる基礎検査をまとめて扱います。

## Use When

- 型エラーの早期検出
- pytest の早期失敗確認
- Markdown とリンクの基礎確認
- Docker / 実行環境の破綻確認

## Core References

- `agents/skills/static-validation.md`
- `documents/WORKFLOW_INVENTORY.md`
- `scripts/ci/run_all_checks.sh`
- `scripts/ci/run_docs_checks.sh`
- `scripts/ci/check_docker_build.sh`

## Default Commands

- `make ci-quick`
- `make docs-check`
- `make docker-build-check`

## Boundary

- この repo では `static-validation` が基礎 gate の正本です。
- 深い diff review は `change-review` または `code-review` を使います。
