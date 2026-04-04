# static-validation

## Purpose

変更内容に応じて最小限かつ十分な quality gate を選びます。

## Use When

- 何を検証すべきか決めたい
- docs / code / environment 変更の確認をそろえたい

## Standard Checks

- `make ci-quick`
- `make ci`
- `python3 scripts/tools/check_markdown_lint.py documents`
- `python3 scripts/tools/audit_and_fix_links.py documents`
