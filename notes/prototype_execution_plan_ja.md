# プロトタイプ一時完成までの実行計画

## ゴール（今回達成）
- ウィンドウ/ディレクトリ重複検知コアを実装
- 高速スクロール（中ボタンパン）コアを実装
- CLIから挙動確認できる試験用実行ファイルを用意
- C++自動テストで主要挙動を検証

## 実行ステップ
1. コアAPI定義（WorkspaceRegistry, ScrollController）
2. 実装（正規化・重複通知・スクロール演算）
3. CLIプロトタイプ追加（open-file/open-dir/scroll）
4. テスト拡張（重複検知、クランプ動作）
5. `cmake && build && ctest` で検証

## 次ステップ（v0.2）
- 実GUI層に接続（通知ダイアログ、ペイン管理）
- 永続化（settings/workspace/session）
- CLIエージェント連携のI/Oプロトコル固定
