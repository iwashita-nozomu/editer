# Codex Subagents

この文書は、Codex を primary runtime とする場合の subagent routing と inventory の正本です。
shared workflow は `agents/canonical/CODEX_WORKFLOW.md` に置き、この文書は specialist 起動の補足に限定します。

## Principles

- parent agent が最終編集責任を持つ
- subagent は必要なときだけ explicit に使う
- 調査、レビュー、文書整備は分ける
- 再帰的 fan-out は避ける

## Built-In Or Project-Scoped Roles

- `explorer`
  - 読み取り専用で codebase / docs / workflow の調査を行う
- `reviewer`
  - 読み取り専用で diff と risk を findings-first で洗う
- `python_reviewer`
  - Python diff を型、pytest、ruff 前提で洗う
- `worker`
  - bounded な実装変更を切り出す
- `docs_workflow_steward`
  - agent 文書、workflow、adapter file の整理を行う
- `project_reviewer`
  - repo-wide な inventory と workflow health を確認する
- `report_reviewer`
  - experiment report の根拠と reader-facing quality を確認する
- `reproducibility_reviewer`
  - provenance、seed、command、environment、rerunability を確認する
- `scientific_computing_reviewer`
  - incremental change、testing、automation、prototype discipline を確認する
- `benchmark_reviewer`
  - fairness、case mix、confounder、benchmark anti-pattern を確認する
- `artifact_reviewer`
  - code、script、raw result、environment、artifact package の十分性を確認する
- `fair_data_reviewer`
  - metadata、命名、result path、再利用性を確認する
- `ml_science_reviewer`
  - assumptions、limitations、uncertainty、reader-facing reporting を確認する

## Recommended Routing

| Task Shape | Default Subagent Pattern |
| ---------- | ------------------------ |
| 局所修正 | parent 直処理。Python 差分なら `python_reviewer`、必要なら `explorer` の後に `reviewer` |
| 文書整理 | `docs_workflow_steward`、必要なら `reviewer` |
| repo-wide 棚卸し | `project_reviewer`、必要なら `explorer`。research / experiment scope なら perspective reviewers を並列 |
| bounded 実装切り出し | `worker` の前後に `reviewer` |
| 実験や調査つき改善 | `explorer` で前提確認後、parent 実装。report があるなら `report_reviewer`。methodology や artifact policy が大きく動くなら perspective reviewers も使う。必要なら run bundle を作る |

## Research Perspective Review Pack

- `reproducibility_reviewer` に provenance、seed、command、environment、rerunability を見させる
- `scientific_computing_reviewer` に incremental change、testing、automation、prototype discipline を見させる
- `benchmark_reviewer` に fairness、case mix、confounder、benchmark anti-pattern を見させる
- `artifact_reviewer` に code、script、raw result、environment、artifact package の十分性を見させる
- `fair_data_reviewer` に metadata、命名、result path、再利用性を見させる
- `ml_science_reviewer` に assumptions、limitations、uncertainty、reader-facing reporting を見させる
- parent が findings を `fix now`、`follow-up`、`delete-ok` に再分類して反映する

## Runtime Surfaces

- human canon: `agents/`
- skill shim: `.agents/skills/`
- Codex project config: `.codex/config.toml`
- Codex subagent definitions: `.codex/agents/*.toml`

## Smoke Test

runtime inventory や review pack を変えたら、まず次を実行します。

```bash
python3 scripts/agent_tools/smoke_test_research_perspective_pack.py
```

この smoke test は次を確認します。

- `agents/agents_config.json` に perspective reviewers と artifact mapping がある
- `agents/task_catalog.yaml` に `research_perspective_review` pack と `T9` がある
- `.codex/agents/*.toml` に対応 subagent 定義がある
- temporary run bundle を作って、各 perspective review artifact と `team_manifest.yaml` が実際に生成される
