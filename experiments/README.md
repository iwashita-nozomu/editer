# Experiments Hub

`experiments/` は、server 上で回す実験コード、run ごとの生成物、1 run ごとの report をまとめる場所です。
この template では、topic ごとの実験コードと run artifact を同じ tree に寄せます。

## 標準構成

```text
experiments/
├── report/
│   ├── README.md
│   └── <run_name>.md
└── <topic>/
    ├── README.md
    ├── cases.py
    ├── experimentcode.py
    └── result/
        └── <run_name>/
```

## まず使うもの

- `_template/`
  - 新しい topic を始めるときの最小雛形です。
- `report/README.md`
  - run report の置き方です。
- `scripts/experiments/run_managed_experiment.py`
  - `run_manifest.json` と `run.log` を残しながら実験を実行する入口です。

## server 実行の既定

- fresh run は 1 つの `run_name` と 1 つの `result/<run_name>/` に閉じます。
- server 上の formal run は、できるだけ `run_managed_experiment.py` 経由で実行します。
- 主要生成物は次を基準にします。
  - `result/<run_name>/run_manifest.json`
  - `result/<run_name>/run.log`
  - `result/<run_name>/summary.json`
  - `result/<run_name>/cases.jsonl`
- 1 回の run report は `experiments/report/<run_name>.md` に置きます。

## topic の作り始め

```bash
cp -R experiments/_template experiments/<topic>
```

コピーしたら、少なくとも次をその topic に合わせて書き換えます。

- `README.md`
- `cases.py`
- `experimentcode.py`
- 標準コマンド
- `Question:`
- `Comparison Target:`

## 実行例

```bash
python3 scripts/experiments/run_managed_experiment.py \
  --topic _template \
  --variant smoke \
  -- \
  python3 experiments/_template/experimentcode.py \
    --run-dir {run_dir} \
    --limit 4
```

この wrapper は、command 実行前に result dir と report stub を初期化し、終了後に manifest を更新します。
