# Worktree Extraction Notes

このディレクトリには、削除予定の worktree から `main` へ吸い上げたメモを置きます。
研究・実験改造の全体手順は [documents/research-workflow.md](../../documents/research-workflow.md) を参照してください。

## Purpose

- 削除前の worktree にしか残っていない知見を `main` 側へ退避する
- 一時的な branch や tuning worktree の判断を、後で参照できる形にする
- worktree 自体は消しても、判断の履歴は失わないようにする

## Action Log

- このディレクトリの note は、carry-over 用 summary であると同時に worktree の action log の正本です。
- scope 更新、編集開始、テスト実行、実験開始 / 停止、最終判断は append-only で追記します。
- 1 行でよいので、何をしたか、何を見たか、次に何をするかが追える形にします。
- worktree 内で先に書く場合も、最終配置と同じ相対パスに置きます。

## What To Extract

最低限、次を整理してから worktree を消します。

- branch 名
- worktree の用途
- 関連する result や log の所在
- 主要な観測
- 次に残すべき `Idea:` / `Interpretation:` / `Consideration:`
- その worktree を一度しか開かなくても状況が分かる quick reference

## Naming

- ファイル名は `worktree_<topic>_YYYY-MM-DD.md` のように、対象が分かる形にします。
- unrelated な worktree を 1 つの file に混ぜません。
- タイトルは日付ではなく topic を主にします。

## Recommended Sections

- `Summary:`
- `Action Log:`
- `Question / Formulation:`
- `Observations:`
- `Carry-Over Targets:`
- `Branch Reflection:`
- `Idea:` / `Interpretation:` / `Consideration:`
