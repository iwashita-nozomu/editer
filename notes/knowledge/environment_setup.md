# Environment Setup

## 共通方針

- 共通開発環境は `docker/` を基準にします。
- 環境更新時は、設定だけでなく関連文書も同時に更新します。

## Python を使う場合

- 依存の正本は `docker/Dockerfile` と `docker/requirements.txt` です。
- import path やテスト実行前提は Python 用文書とスクリプトの規約に合わせます。

## 実験前チェック

- 作業ツリーが期待した状態か確認します。
- 出力先が意図したディレクトリか確認します。
- 実験条件と環境変数の前提を run 前に固定します。
