.PHONY: git_init ci ci-quick dev-setup tools-help agent-checks

# Git 初期化（初回のみ）
git_init:
	bash scripts/git_init.sh
	@echo "✅ Git setup complete"

# ★推奨: 統合 CI（pytest + pyright + ruff）
ci:
	bash scripts/ci/run_all_checks.sh

# CI 高速モード（ruff skip）
ci-quick:
	bash scripts/ci/run_all_checks.sh --quick

# agent runtime / skill drift checks
agent-checks:
	python3 scripts/tools/mirror_skill_shims.py --target .claude/skills --prune --check
	python3 scripts/agent_tools/smoke_test_research_perspective_pack.py

# 開発環境初期化
dev-setup: git_init
	@echo "✅ Dev environment ready. Start with: make ci-quick"

# ツール情報表示
tools-help:
	@echo "=== Available Tools & Scripts ==="
	@echo ""
	@cat scripts/README.md | head -40
