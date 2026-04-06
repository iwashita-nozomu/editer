# docs-completeness-review

## Purpose

文書が存在するだけでなく、読者が作業や判断に必要な情報を欠かさず持っているかをレビューします。

## Use When

- README、設計文書、workflow 文書の不足確認
- 入口、前提、手順、出力先、禁止事項の欠落確認
- 実装変更後に docs が追随しているかの確認

## Core References

- `documents/coding-conventions-project.md`
- `documents/README.md`
- `agents/skills/docs-consistency-review.md`
- `agents/skills/md-style-check.md`

## Mandatory Checklist

- 文書だけで対象、目的、入口が分かる
- 手順文書なら入力、出力、主要コマンドがある
- 規約文書なら禁止、必須、許可、任意が明示されている
- 実験文書なら run path と report path がある

## Boundary

- Markdown の体裁だけを見るなら `md-style-check` を使います。
- 文書間の矛盾や stale route を潰すなら `docs-consistency-review` を使います。
