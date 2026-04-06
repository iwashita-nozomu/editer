# project-health

## Purpose

日次・週次・継続運用での健康状態を監視し、automation の壊れ方を早めに見つけます。

## Use When

- project health の監視
- CI / CD 健全性確認
- routine maintenance の起点作り
- 運用上の drift 検出

## Core References

- `documents/WORKFLOW_INVENTORY.md`
- `scripts/run_comprehensive_review.sh`
- `scripts/docker_dependency_validator.py`

## Boundary

- 変更差分のレビューは `code-review` を使います。
- repo-wide review の最上位入口としては `project-review` を使います。
