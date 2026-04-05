# branch・worktree の例外運用

この文書は、`main` 以外で短期的に作業を切り分ける必要がある場合だけ参照します。
既定運用は `main` です。恒常的な branch 分割や worktree 常用は前提にしません。

## 使う場面

- 大きな refactor を安全に切り分けたい
- レビュー前に差分を明確に分けたい
- 長時間実験や破壊的な試行を一時的に隔離したい

## 作成前の確認

- `main` と対象 branch が `origin` と同期しているか確認します。
- 既存の worktree を使い回さず、新しい worktree を切ります。
- 長時間 run や巨大生成物が無いなら、まず `main` で進めることを優先します。

## ルール

- branch は短期で閉じます。
- 統合先は常に `main` です。
- 長期に残す知見は branch 名ではなく `documents/` または `notes/` に移します。
- 1 回の実験結果を branch 固有の台帳に依存させません。
- worktree root には必要に応じて `WORKTREE_SCOPE.md` を置き、テンプレートは [WORKTREE_SCOPE_TEMPLATE.md](/mnt/l/workspace/project_template/documents/WORKTREE_SCOPE_TEMPLATE.md) を使います。
- branch の役割と carry-over 先を残したい場合は [BRANCH_SCOPE.md](/mnt/l/workspace/project_template/documents/BRANCH_SCOPE.md) と `notes/branches/` を使います。
- 例外運用中の action log は `notes/worktrees/` に残します。

## 閉じる前の確認

- `main` に戻すコード、文書、知見がそろっているか
- 不要な branch 専用メモを残していないか
- 例外運用で得たルールを正本へ反映したか
- `main` に持ち帰る note と最小 final JSON の置き場が決まっているか
- worktree を消したあとも `main` から関連 note と結果を辿れるか
*** Add File: /mnt/l/workspace/project_template/notes/branches/README.md
# Branch Notes

`notes/branches/` は、branch ごとの要約と関連 note への入口をまとめるディレクトリです。
このカテゴリは例外運用用です。通常運用では branch 名そのものではなく `main` 上の code、document、note を見ます。

## 役割

- その branch が何のために存在したかを短く残す
- どこまで `main` に取り込まれたかを示す
- 先に読むべき note、result、report への入口をまとめる

## Format

- 1 branch 1 file を基本にします
- タイトルは branch 名そのものではなく topic-first で付けます
- branch 名、役割、現在の状態、読むべき note、主要な知見を最初に書きます
- active な branch は、対応する worktree action log と carry-over 先をここから辿れるようにします

## Retention Labels

- `persistent`
  - 継続的な入口または raw 結果の保管先として残します
- `keep-while-active`
  - 対応する worktree や実験が動いている間だけ残します
- `delete-ok`
  - 知見の吸収が終わっており、削除してよい branch です

## Workflow

1. branch を切ったら、必要に応じて同時にこのディレクトリへ branch summary を作ります。
1. `WORKTREE_SCOPE.md` で指定した action log と carry-over target を、この summary から辿れるようにします。
1. branch を閉じる前に、関連 note と final JSON のリンクを更新します。
