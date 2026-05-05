<!--
@dependency-start
responsibility Provides a Japanese slide deck explaining this project template repository.
upstream design ../README.md repository overview and operating entrypoints
upstream design ./SHARED_RUNTIME_SURFACES.md shared AgentCanon surface policy
upstream design ./REVIEW_PROCESS.md review and closeout gates
upstream environment ../docker/README.md canonical container and devcontainer policy
upstream design ../checklist.md repository audit checklist
@dependency-end
-->

# Project Template 説明スライド

この slide deck は、`project_template` の目的、構造、AgentCanon 連携、検証導線を短時間で説明するための資料です。
Markdown の `---` 区切りを slide 境界として扱います。

---

## 1. この Repo の目的

`project_template` は、研究開発 repo を最初から次の状態で始めるための template です。

- 実装、実験、文書、CI、Docker、agent runtime を 1 つの root に揃える
- repo 開始直後から `make ci`、`make agent-checks`、`make docs-check` を使える
- AgentCanon を submodule pin と root symlink surface で共有する
- template 固有の bootstrap と shared canon の責務を分離する

---

## 2. 想定する使い方

この template は、Python と Markdown を中心にした研究開発 repo を想定します。
C++、Docker、Jupyter、実験 registry、agent workflow は必要になったときに拡張できるように初期 surface を持ちます。

日常の入口は次です。

- 人間向け: `README.md`、`QUICK_START.md`
- agent 向け: `AGENTS.md`、`agents/README.md`
- 初期化: `scripts/start_repository.sh`
- 検証: `make ci`、`make agent-checks`、`make docs-check`

---

## 3. ディレクトリ全体像

repo root は、責務別に大きく分かれています。

- `python/`: Python 実装本体
- `tests/`: pytest と shared tooling tests
- `documents/`: durable な規約、設計、運用文書
- `notes/`: 後続作業で再利用する知見
- `experiments/`: 実験 topic、run artifact、report
- `docker/`: canonical container、runtime pack、devcontainer 元設定
- `vendor/agent-canon/`: shared agent canon の submodule pin
- `agents/`、`tools/`、`.agents/`、`.claude/`、`.codex/`: AgentCanon の root runtime view

---

## 4. Repo-Local と Shared Canon の境界

repo-local なものは root に置きます。

- project 固有の README、bootstrap script、Docker policy
- project code、test、experiment registry
- project 固有の監査表や説明資料

shared なものは `vendor/agent-canon/` を正本にします。

- agent workflow、skill、subagent profile
- shared tools、review gate、dependency review
- shared coding convention と runtime surface policy

root の symlink view は編集入口ではなく、runtime から見える互換 surface です。

---

## 5. AgentCanon の配置

AgentCanon は `vendor/agent-canon/` に submodule として pin します。
template はその pin を root の runtime surface として見せます。

- `agents/` -> `vendor/agent-canon/agents`
- `tools/` -> `vendor/agent-canon/tools`
- `.agents/` -> `vendor/agent-canon/.agents`
- `.claude/` -> `vendor/agent-canon/.claude`
- `.codex/` -> `vendor/agent-canon/.codex`
- `mcp/` -> `vendor/agent-canon/mcp`

shared workflow や tool を直す場合は、root view ではなく AgentCanon 側を正本として扱います。

---

## 6. GitHub Remote 方針

GitHub が canonical remote です。
local bare repo は互換 mirror または proposal target として扱います。

- Template canonical: `https://github.com/iwashita-nozomu/project_template.git`
- AgentCanon canonical: `https://github.com/iwashita-nozomu/agent-canon.git`
- Template local mirror: `/mnt/git/template.git`
- AgentCanon local mirror: `/mnt/git/agent-canon.git`

latest 判定は GitHub canonical を正本にし、local mirror だけを最新扱いにしません。

---

## 7. Bootstrap 導線

新規 repo は template clone から始めます。

```bash
make start-repository ARGS='--project-slug your-project --display-name "Your Project"'
```

bootstrap で確認することは次です。

- project slug と display name の置換
- AgentCanon pin と root symlink surface
- local bare mirror の opt-in
- quick CI と fresh clone validation
- repo 固有の README / Docker / documents の整合

---

## 8. Agent Runtime の開始点

agent は `AGENTS.md` を入口にします。
repo-changing task では、最初に `$agent-orchestration` で workflow family、skill、review roles を固定します。

重要な固定ルールは次です。

- implementation 前に既存実装と依存 surface を調査する
- subagent を stage ごとに分け、closeout 前に閉じる
- run bundle に user request contract、schedule、work log を残す
- dependency review、static analysis、diff review を closeout gate にする

---

## 9. MCP の役割

MCP は repo context と loop status の機械的な確認に使います。
canonical launcher は `.codex/config.toml` から `bash mcp/repo_mcp_server.sh` です。

監査時の固定確認は次です。

```bash
python3 tools/agent_tools/check_mcp_inventory.py --require repo_mcp_server
```

起動に失敗した場合は ad hoc process に置換せず、`.codex/`、`mcp/`、base command availability を直します。

---

## 10. Dependency Header と Graph

human-authored text file は先頭に dependency manifest を持ちます。
目的は、人間と agent が修正前に読むべき upstream / downstream を機械的に辿れるようにすることです。

closeout では差分限定ではなく全 repo を確認します。

```bash
bash tools/agent_tools/run_repo_dependency_review.sh --fail-missing
```

この gate は header 欠落、format drift、自己参照、循環参照、孤立 manifest を検出します。

---

## 11. Docker と Devcontainer

canonical runtime は `docker/Dockerfile` と `docker/packs/*.toml` です。
devcontainer も同じ pack から compose を生成します。

現在の container 方針は次です。

- `gh`、Codex CLI、Jupyter、Graphviz、rsync、Docker CLI を image に含める
- host `~/.codex`、`~/.config/gh`、`~/.ssh` は devcontainer に mount できる
- Codex 認証は host `~/.codex` を正本にし、container は mount された auth state を再利用する
- safe.directory は `/workspace` と `vendor/*` から動的登録する
- Template / AgentCanon 固有の local mirror path は Dockerfile に焼かない

---

## 12. Jupyter と実験

JupyterLab は canonical image に含めます。

```bash
make docker-jupyter
```

実験は registry と run artifact を分けて管理します。

- `experiments/registry.toml`: topic registry
- `experiments/_template/`: topic scaffold
- `experiments/report/`: run report
- `tools/experiments/update_latest_result.py`: latest result pointer

spot run や途中停止 run を正式 evidence にしません。

---

## 13. Review と Closeout

repo-changing task は、review と validation が揃うまで完了扱いにしません。

最低限の closeout gate は次です。

- `bash tools/agent_tools/run_repo_dependency_review.sh --fail-missing`
- `make agent-checks`
- `make ci`
- Markdown 変更時の `make docs-check`
- Docker 変更時の `make docker-build-check`
- latest diff の independent review
- commit / push 済みの clean worktree

---

## 14. 監査の見方

監査時は `checklist.md` を使います。
見る順番は、壊れると影響が大きい surface からです。

- Git remote と AgentCanon latest
- MCP と root runtime surface
- dependency header と graph
- Docker / devcontainer / Jupyter
- workflow、skill、eval、goal
- CI、static analysis、push evidence

監査結果は `pass`、`revise`、`blocked` に分けます。

---

## 15. この Repo の価値

この template の価値は、開発開始時点から「後で整える」項目を減らすことです。

- 実装前の調査、再利用、設計を workflow に組み込む
- agent runtime と人間向け docs を同じ repo state に揃える
- Docker と CI で実行環境を固定する
- dependency graph と review gate で横断変更の読み漏れを減らす
- Template と AgentCanon を GitHub canonical へ寄せ、派生 repo の運用を揃える
