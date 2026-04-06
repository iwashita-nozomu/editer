# change-review

## Purpose

diff を findings-first で読み、回帰、欠落テスト、古い文書を洗います。

## Use When

- code review
- doc review
- AI-generated diff review

## Core Reference

- `documents/REVIEW_PROCESS.md`

## Boundary

- `code-review` は Codex skill 名互換の入口です。
- Python 差分で型と test を強く見る場合は `python-review` を追加します。
