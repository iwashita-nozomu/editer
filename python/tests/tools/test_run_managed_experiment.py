"""Tests for the managed experiment run helper."""

from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

SCRIPT = (
    Path(__file__).resolve().parents[3]
    / "scripts"
    / "experiments"
    / "run_managed_experiment.py"
)
MARKER_COMMAND = (
    "from pathlib import Path; "
    "Path(r'{run_dir}/marker.txt').write_text('ok', encoding='utf-8')"
)


def build_repo(tmp_path: Path) -> Path:
    """Create a minimal fake repo layout for the helper."""
    repo_root = tmp_path / "repo"
    (repo_root / "experiments" / "demo_topic" / "result").mkdir(parents=True)
    (repo_root / "experiments" / "report").mkdir(parents=True)
    subprocess.run(["git", "init"], cwd=repo_root, check=True, capture_output=True, text=True)
    return repo_root


def test_run_managed_experiment_writes_manifest_log_and_report(tmp_path: Path) -> None:
    """The helper should create canonical files for one successful run."""
    repo_root = build_repo(tmp_path)
    run_name = "demo_topic_smoke_20260406T000000Z"

    result = subprocess.run(
        [
            sys.executable,
            str(SCRIPT),
            "--repo-root",
            str(repo_root),
            "--topic",
            "demo_topic",
            "--run-name",
            run_name,
            "--",
            sys.executable,
            "-c",
            MARKER_COMMAND,
        ],
        check=False,
        capture_output=True,
        text=True,
    )

    assert result.returncode == 0
    result_dir = repo_root / "experiments" / "demo_topic" / "result" / run_name
    manifest = json.loads((result_dir / "run_manifest.json").read_text(encoding="utf-8"))
    assert manifest["status"] == "completed"
    assert manifest["exit_code"] == 0
    assert (result_dir / "run.log").is_file()
    assert (result_dir / "marker.txt").read_text(encoding="utf-8") == "ok"
    report_path = repo_root / "experiments" / "report" / f"{run_name}.md"
    assert report_path.is_file()
    assert run_name in report_path.read_text(encoding="utf-8")


def test_run_managed_experiment_propagates_failure(tmp_path: Path) -> None:
    """The helper should return the child exit code and mark the run failed."""
    repo_root = build_repo(tmp_path)
    run_name = "demo_topic_fail_20260406T000000Z"

    result = subprocess.run(
        [
            sys.executable,
            str(SCRIPT),
            "--repo-root",
            str(repo_root),
            "--topic",
            "demo_topic",
            "--run-name",
            run_name,
            "--",
            sys.executable,
            "-c",
            "import sys; sys.exit(7)",
        ],
        check=False,
        capture_output=True,
        text=True,
    )

    assert result.returncode == 7
    manifest_path = (
        repo_root / "experiments" / "demo_topic" / "result" / run_name / "run_manifest.json"
    )
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    assert manifest["status"] == "failed"
    assert manifest["exit_code"] == 7
