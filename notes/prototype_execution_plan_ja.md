# プロトタイプ一時完成までの実行計画
<!--
@dependency-start
responsibility Captures the initial execution plan for the editor prototype.
upstream design editor_kickoff_questions_ja.md defines the Japanese editor requirements
downstream implementation ../include/editor_proto/workspace_registry.hpp declares core workspace APIs
downstream implementation ../src/workspace_registry.cpp implements core workspace behavior
downstream implementation ../src/editor_gui_proto.cpp implements the current native GUI prototype
@dependency-end
-->

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
