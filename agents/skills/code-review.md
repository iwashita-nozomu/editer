# code-review

## Purpose

変更差分を correctness、設計、保守性の観点でレビューします。

## Use When

- PR 前後の変更レビュー
- doc / code / test の整合確認
- refactor での境界確認
- 規約逸脱や過剰修正の検出

## Core References

- `agents/skills/change-review.md`
- `agents/skills/python-review.md`
- `documents/REVIEW_PROCESS.md`

## Review Stance

- finding を先に出します。
- 壊れる理由、根拠不足、テスト不足、stale doc を優先して探します。
- 説明より evidence を重視します。

## Boundary

- この repo の findings-first review の正本は `change-review` です。
- Python 差分で pyright / pytest / ruff を強く見る場合は `python-review` を追加します。
- 実験主張の批判的評価は `critical-review` を使います。
