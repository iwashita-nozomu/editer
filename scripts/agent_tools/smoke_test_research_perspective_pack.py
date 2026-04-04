#!/usr/bin/env python3
"""Smoke test the research perspective review pack runtime surfaces."""

from __future__ import annotations

import argparse
import shutil
import tempfile
from datetime import datetime, timezone
from pathlib import Path

from agent_team import create_run_bundle, load_team_config, resolve_role, resolve_role_write_scope


ROOT = Path(__file__).resolve().parents[2]
CODEX_AGENT_ROOT = ROOT / ".codex" / "agents"
TASK_CATALOG = ROOT / "agents" / "task_catalog.yaml"
BASE_RESEARCH_ROLE_IDS = (
    "researcher",
    "research_reviewer",
    "experimenter",
    "experiment_reviewer",
)
PERSPECTIVE_ROLE_IDS = (
    "reproducibility_reviewer",
    "scientific_computing_reviewer",
    "benchmark_reviewer",
    "artifact_reviewer",
    "fair_data_reviewer",
    "ml_science_reviewer",
)
ROLE_TO_ARTIFACT_KEY = {
    "reproducibility_reviewer": "reproducibility_review",
    "scientific_computing_reviewer": "scientific_computing_review",
    "benchmark_reviewer": "benchmark_review",
    "artifact_reviewer": "artifact_review",
    "fair_data_reviewer": "fair_data_review",
    "ml_science_reviewer": "ml_science_review",
}


def build_parser() -> argparse.ArgumentParser:
    """Create the CLI parser."""
    parser = argparse.ArgumentParser(
        description="Create a temporary run bundle and verify the research perspective review pack."
    )
    parser.add_argument(
        "--task",
        default="research perspective pack smoke test",
        help="Task label to embed in the temporary bundle.",
    )
    parser.add_argument(
        "--owner",
        default="codex",
        help="Owner label to embed in the temporary bundle.",
    )
    parser.add_argument(
        "--run-id",
        default="smoke-research-perspective-pack",
        help="Run id to use inside the temporary bundle.",
    )
    parser.add_argument(
        "--workspace-root",
        help="Optional workspace root. Defaults to a temporary workspace with WORKTREE_SCOPE.md.",
    )
    parser.add_argument(
        "--report-root",
        help="Optional report root. Defaults to a temporary report root.",
    )
    parser.add_argument(
        "--keep-temp",
        action="store_true",
        help="Keep temporary directories when defaults are used.",
    )
    return parser


def ensure_text_contains(content: str, needle: str) -> None:
    """Raise when one expected token is missing."""
    if needle not in content:
        raise RuntimeError(f"missing expected text: {needle}")


def prepare_workspace(workspace_root: Path) -> None:
    """Create a minimal workspace that satisfies manifest scope resolution."""
    (workspace_root / "python").mkdir(parents=True, exist_ok=True)
    (workspace_root / "documents").mkdir(parents=True, exist_ok=True)
    (workspace_root / "reports" / "runtime").mkdir(parents=True, exist_ok=True)
    (workspace_root / "WORKTREE_SCOPE.md").write_text(
        "\n".join(
            [
                "# Worktree Scope",
                "",
                "## Editable Directories",
                "- `python`",
                "- `documents`",
                "",
                "## Runtime Output Directories",
                "- `reports/runtime`",
                "",
            ]
        ),
        encoding="utf-8",
    )


def validate_task_catalog() -> None:
    """Check that the task catalog exposes the review pack."""
    content = TASK_CATALOG.read_text(encoding="utf-8")
    ensure_text_contains(content, "review_packs:")
    ensure_text_contains(content, "id: research_perspective_review")
    ensure_text_contains(content, "id: T9")
    for role_id in PERSPECTIVE_ROLE_IDS:
        ensure_text_contains(content, f"- {role_id}")


def validate_runtime_surfaces(report_dir: Path, workspace_root: Path) -> None:
    """Check that config, agent inventory, templates, and bundle outputs align."""
    config = load_team_config()
    manifest_path = report_dir / config.artifacts["team_manifest"]
    manifest_text = manifest_path.read_text(encoding="utf-8")

    for role_id in PERSPECTIVE_ROLE_IDS:
        role = resolve_role(config, role_id)
        artifact_key = ROLE_TO_ARTIFACT_KEY[role_id]
        artifact_name = config.artifacts[artifact_key]
        artifact_path = report_dir / artifact_name
        codex_agent_path = CODEX_AGENT_ROOT / f"{role_id}.toml"

        if not codex_agent_path.is_file():
            raise RuntimeError(f"missing Codex agent definition: {codex_agent_path}")
        if not artifact_path.is_file():
            raise RuntimeError(f"missing generated artifact: {artifact_path}")
        if role.required_outputs != (artifact_name,):
            raise RuntimeError(
                f"role {role_id} required_outputs mismatch: {role.required_outputs} vs {artifact_name}"
            )
        if role.write_policy.allowed_artifacts != (artifact_key,):
            raise RuntimeError(
                f"role {role_id} artifact policy mismatch: {role.write_policy.allowed_artifacts}"
            )

        scope = resolve_role_write_scope(
            config=config,
            role=role,
            report_dir=report_dir,
            workspace_root=workspace_root,
        )
        if scope.mode != "artifacts_only":
            raise RuntimeError(f"role {role_id} should be artifacts_only, got {scope.mode}")
        if artifact_path.resolve() not in scope.allowed_files:
            raise RuntimeError(f"role {role_id} missing allowed file for {artifact_path}")

        ensure_text_contains(manifest_text, f"  - id: {role_id}")
        ensure_text_contains(manifest_text, f"      - {artifact_name}")


def main() -> int:
    """Run the smoke test."""
    args = build_parser().parse_args()
    temp_paths: list[Path] = []

    if args.workspace_root is None:
        workspace_root = Path(tempfile.mkdtemp(prefix="research-pack-workspace-"))
        temp_paths.append(workspace_root)
    else:
        workspace_root = Path(args.workspace_root).resolve()
        workspace_root.mkdir(parents=True, exist_ok=True)

    if args.report_root is None:
        report_root = Path(tempfile.mkdtemp(prefix="research-pack-reports-"))
        temp_paths.append(report_root)
    else:
        report_root = Path(args.report_root).resolve()
        report_root.mkdir(parents=True, exist_ok=True)

    try:
        prepare_workspace(workspace_root)
        validate_task_catalog()

        config = load_team_config()
        specialist_roles = tuple(
            resolve_role(config, role_id)
            for role_id in BASE_RESEARCH_ROLE_IDS + PERSPECTIVE_ROLE_IDS
        )
        roles = config.always_on_roles + specialist_roles
        created_at = datetime.now(timezone.utc).replace(microsecond=0)
        created_at_iso = created_at.isoformat().replace("+00:00", "Z")
        report_dir = (report_root / args.run_id).resolve()

        create_run_bundle(
            config=config,
            report_dir=report_dir,
            run_id=args.run_id,
            task=args.task,
            owner=args.owner,
            created_at_iso=created_at_iso,
            roles=roles,
            workspace_root=workspace_root.resolve(),
        )

        validate_runtime_surfaces(report_dir, workspace_root.resolve())

        print(f"RUN_ID={args.run_id}")
        print(f"REPORT_DIR={report_dir}")
        print(f"WORKSPACE_ROOT={workspace_root.resolve()}")
        print(f"ACTIVE_ROLES={','.join(role.id for role in roles)}")
        print("SMOKE_TEST=pass")
        return 0
    finally:
        if not args.keep_temp:
            for path in reversed(temp_paths):
                shutil.rmtree(path, ignore_errors=True)


if __name__ == "__main__":
    raise SystemExit(main())
