# Agent Task Workflows

この文書は、repo で使う workflow family の正本です。
task を細かく増やしすぎず、少数の family に寄せて運用します。

すべての family で、repo に持ち帰る実装パスは
[documents/implementation-waterfall-workflow.md](/mnt/l/workspace/project_template/documents/implementation-waterfall-workflow.md)
の段階ゲートに従います。

## 共通レビューゲート

1. Intake Review
   - `manager`
   - `manager_reviewer`
1. Requirements Review
   - `manager`
   - `manager_reviewer`
1. Design Review
   - `designer`
   - `design_reviewer`
1. Implementation Checkpoint Review
   - `implementer`
   - `change_reviewer`
1. Verification Review
   - `change_reviewer`
1. Final Acceptance Review
   - `final_reviewer`
1. Audit Review
   - `auditor`
1. Gate Closure
   - `verifier`

## Workflow Families

### 1. Scoped Change

対象:
- 局所バグ修正
- 小規模な docs/test 同期
- CI failure の切り分け

標準フロー:
1. 共通レビューゲートをそのまま 1 pass で通す

### 2. Research-Driven Change

対象:
- 外部調査を伴う実装
- benchmark や比較実験を根拠にした改善

追加ロール:
- `researcher`
- `research_reviewer`
- `experimenter`
- `experiment_reviewer`
- 必要に応じて `reproducibility_reviewer`
- 必要に応じて `scientific_computing_reviewer`
- 必要に応じて `benchmark_reviewer`
- 必要に応じて `artifact_reviewer`
- 必要に応じて `fair_data_reviewer`
- 必要に応じて `ml_science_reviewer`

特徴:
- research と experiment を evidence として回す
- overclaim review を明示的に挟む
- `report_rewrite_required`、`extra_validation_required`、`rerun_required` が残る限り loop を閉じない
- ただし、1 回の repo 変更は 1 回の waterfall pass として閉じる
- 各 pass で checkpoint review、verification review、final review、audit review を省略しない
- agent が code change と run を継続反復する場合は `experiment-change-loop` を追加する
- methodology、artifact、reporting policy を大きく変える場合は `research-perspective-review` を追加する
- repo-wide な research cleanup では task catalog の `T9` を基準に perspective reviewers をまとめて有効化する

### 3. Large Delivery

対象:
- 新機能追加
- 大規模 refactor
- 複数 chunk に分ける delivery

追加ロール:
- `scheduler`
- `schedule_reviewer`

特徴:
- milestone と chunk 境界を先に固定する
- 各 chunk は独立した waterfall pass として閉じる
- 各 chunk で checkpoint review を複数回に増やせる
- 逐次 review と最終 review を分ける

### 4. Platform And Environment

対象:
- Docker
- CI
- automation
- dependency / runtime upgrade

追加ロール:
- `infra_steward`
- `infra_reviewer`
- 必要に応じて `researcher`, `scheduler`, `experimenter`

特徴:
- rollout と rollback を先に考える
- repo ルール、環境、automation を同時に更新する
- 実装前に environment design を凍結し、acceptance gate で transition readiness を確認する
- `infra_reviewer` を design review と verification review の両方へ参加させる
- repo-wide な tool 導入案では理由、Docker 影響、validation、rollback を同時に残す

## 選び方

1. task が局所修正なら `Scoped Change`
1. 外部調査や比較実験が必要なら `Research-Driven Change`
1. chunk 設計が必要なら `Large Delivery`
1. 環境や automation を触るなら `Platform And Environment`

## 関連

- `agents/task_catalog.yaml`
- `agents/COMMUNICATION_PROTOCOL.md`
- `agents/canonical/ARTIFACT_PLACEMENT.md`
- `agents/canonical/CLI_ENTRYPOINTS.md`
- `documents/experiment-workflow.md`
- `documents/research-workflow.md`
