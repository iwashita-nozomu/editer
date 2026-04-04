# md-style-check

## Purpose

Markdown の体裁、見出し、リンク、可読性を崩さずに保ちます。

## Use When

- `.md` を触る
- 文書整理や report 整備を行う

## Required Checks

- `python3 scripts/tools/check_markdown_lint.py documents`
- 必要に応じて `python3 scripts/tools/audit_and_fix_links.py documents`

## Core References

- `documents/conventions/common/05_docs.md`
- `.markdownlint.json`
