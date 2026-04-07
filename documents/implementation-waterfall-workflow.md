# 実装ウォーターフォールワークフロー

この文書は、repo に変更を入れる実装プロセスを、段階ゲート付きのウォーターフォールとして進めるための正本です。
対象は `python/`、`documents/`、`agents/`、`docker/`、`scripts/` など、repo に持ち帰る変更全般です。

この repo では workflow family の選択は `agents/TASK_WORKFLOWS.md` を使いますが、実装そのものの進め方はこの文書を共通ルールにします。

## 1. 目的

- 要件が固まる前に code を書き始めない
- 設計が固まる前に実装を広げない
- 実装、review、verification を段階ゲートで区切る
- 変更要求 1 件につき 1 回の実装パスを閉じる
- 差し戻しが必要な場合は、どの段へ戻すかを明示する

## 2. 文献ベースの判断

この workflow は、純粋な無反復 waterfall ではなく、初期段階だけ限定的に戻りを許す phase-gated waterfall として定義します。

- Royce 1970:
  - 要件、分析、設計、実装、試験を段階化しつつ、設計先行、文書化、pilot model、test planning を強く要求しています。
  - 同時に、単純な一方向実装は risky だと明言しており、初期段階での制御された戻りを前提にします。
- NASA Systems Engineering Handbook Rev 2:
  - stakeholder expectations、technical requirements、logical decomposition、design solution、implementation、integration、verification、validation、transition を別プロセスとして扱います。
  - life-cycle review と technical review を decision gate として扱う考え方を採用します。
- NIST SP 800-218 / 800-218A:
  - secure software development practice は特定手法に依存せず、各 SDLC 実装へ統合すべきとしています。
  - そのため、この repo の waterfall でも security、provenance、AI 特有のリスク確認を verification gate に埋め込みます。

## 3. 適用範囲

- `Scoped Change`
- `Large Delivery`
- `Platform And Environment`
- `Research-Driven Change` のうち、repo へ持ち帰る各 code/doc/environment change

研究や実験の outer loop 自体は反復して構いません。ただし、1 回の change request を repo に入れる実装パスは、この文書の gate を順に通します。
言い換えると、研究は複数回の waterfall pass を並べて進め、1 pass の途中で要件や設計を曖昧なまま変形させません。

## 4. 標準ゲート

### Gate 0. Intake

目的:
- 変更要求を 1 件に固定する
- 影響範囲、非対象、受け入れ条件を固定する

最低限の記録:
- `Change Request:`
- `Scope:`
- `Non-Goals:`
- `Acceptance Criteria:`
- `Validation Plan:`

主担当:
- `manager`
- 必要に応じて `researcher`
- 必要に応じて `scheduler`
- 必要に応じて `infra_steward`

exit 条件:
- 何をもって完了とするかが 1 文で言える
- どの family で扱うかが決まっている
- 実装前に必要な review / validation が決まっている

### Gate 1. Requirements Freeze

目的:
- 解く問題、満たす条件、守る制約を固定する

最低限の記録:
- `Question:` または `Problem Statement:`
- `Acceptance Criteria:`
- `Constraints:`
- `Interfaces Affected:`
- `Rollback Trigger:`

主担当:
- `manager`
- `manager_reviewer`
- 必要に応じて `research_reviewer`

ルール:
- production code の変更は、この gate を通す前に始めません
- 例外は捨てる前提の pilot / prototype だけです

exit 条件:
- reviewer が scope の曖昧さを受け入れ可能と判断している
- 受け入れ条件が test、docs、runtime の観点で追える

### Gate 2. Design Freeze

目的:
- 実装方針、ファイル計画、検証計画を固定する

最低限の記録:
- `Design Summary:`
- `Files To Touch:`
- `Test / Check Plan:`
- `Migration Or Rollback Plan:`
- `Open Risks:`

主担当:
- `designer`
- `design_reviewer`
- 必要に応じて `infra_reviewer`

ルール:
- 要件の追加や変更が出たら Gate 1 に戻します
- 実装中に設計変更が必要だと分かったら Gate 2 を開き直します

exit 条件:
- どの file をどう変えるかが説明できる
- validation の入口が定まっている
- rollback か abort の条件が決まっている

### Gate 3. Implementation

目的:
- 凍結済みの設計を実装へ落とす

主担当:
- `implementer`

ルール:
- 実装は 1 つの change request に閉じます
- docs と tests は同じ pass で更新します
- 途中で scope を広げません
- 設計を変えたくなったら Gate 2 へ戻します

exit 条件:
- 差分が requirements / design に一致している
- planned checks を実行できる状態になっている

### Gate 4. Verification

目的:
- 差分が設計どおりで、回帰やリスクが許容範囲に収まっているか確認する

主担当:
- `change_reviewer`
- 必要に応じて `python-review`
- 必要に応じて `md-style-check`
- 必要に応じて `critical-review`

最低限の確認:
- code / docs diff review
- validation plan の実行
- security / safety / provenance の確認

ルール:
- 設計を変えない軽微修正だけは、この gate 内で戻して構いません
- 新しい requirement や設計変更が必要なら Gate 1 または Gate 2 に戻します

exit 条件:
- `required_change` が解消している
- 実行した checks と未実行理由が説明できる

### Gate 5. Acceptance And Transition

目的:
- 受け入れ条件を満たした変更だけを close する

主担当:
- `final_reviewer`
- `verifier`

最低限の確認:
- acceptance criteria の達成
- repo 正本の同期
- closeout command の実行
- commit / push の成否確認

exit 条件:
- final reviewer が change 全体を受け入れ可能と判断している
- verifier が gate を閉じている

## 5. 差し戻しルール

- requirement の抜けやスコープ変更:
  - Gate 1 へ戻す
- 設計不整合、file plan の見直し、rollback 方針の欠落:
  - Gate 2 へ戻す
- 実装ミスや test failure だが設計は維持できる:
  - Gate 3 に戻して修正する
- 実験結果やユーザー要望で別仮説になった:
  - 既存 pass を閉じ、新しい change request として Gate 0 からやり直す

## 6. Pilot / Prototype の扱い

Royce の "do it twice" を踏まえ、この repo では pilot / prototype を次の条件で許可します。

- Gate 1 または Gate 2 のための学習目的である
- production path に直接 merge しない
- 何を確かめたかを記録する
- pilot の結果で要件か設計を更新したら、そのあとで本実装の waterfall pass を開始する

pilot は本実装の抜け道ではなく、requirements/design の凍結精度を上げる前段とみなします。

## 7. Family ごとの使い分け

### Scoped Change

- Gate 1 から Gate 5 をそのまま 1 pass で通します
- artifact は最小限で構いませんが、requirements/design/verification の区別は崩しません

### Research-Driven Change

- literature survey、baseline run、比較設計は Gate 0-2 の入力です
- 1 回の code change は 1 回の waterfall pass で実装します
- `rerun_required` や新仮説が出たら、新しい pass としてやり直します

### Large Delivery

- `scheduler` が chunk を先に固定します
- 各 chunk は独立した waterfall pass として閉じます
- chunk 間の横断変更は、次 chunk の Gate 1 に持ち越します

### Platform And Environment

- Gate 2 で rollout / rollback / environment impact を必ず固定します
- Gate 4-5 で `docker/`、CI、runtime pack、関連 README の同期を確認します

## 8. closeout の必須項目

- 実行した validation
- 未実行 validation と理由
- 更新した repo 正本
- commit hash
- push の成否

## 関連正本

- [agents/TASK_WORKFLOWS.md](/mnt/l/workspace/project_template/agents/TASK_WORKFLOWS.md)
- [agents/canonical/CODEX_WORKFLOW.md](/mnt/l/workspace/project_template/agents/canonical/CODEX_WORKFLOW.md)
- [documents/WORKFLOW_GUIDE.md](/mnt/l/workspace/project_template/documents/WORKFLOW_GUIDE.md)
- [documents/research-workflow.md](/mnt/l/workspace/project_template/documents/research-workflow.md)
- [documents/experiment-workflow.md](/mnt/l/workspace/project_template/documents/experiment-workflow.md)
- [documents/workflow-references.md](/mnt/l/workspace/project_template/documents/workflow-references.md)
