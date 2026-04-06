#!/usr/bin/env python3
"""Run one experiment while recording canonical server-side run metadata."""

from __future__ import annotations

import argparse
import json
import os
import platform
import shlex
import shutil
import socket
import subprocess
import sys
import time
from datetime import datetime
from pathlib import Path


def repo_root_from_script() -> Path:
    """Return the repository root from this script location."""
    return Path(__file__).resolve().parents[2]


def utc_now() -> str:
    """Return the current UTC timestamp in ISO-8601 form."""
    return datetime.utcnow().replace(microsecond=0).isoformat() + "Z"


def compact_timestamp() -> str:
    """Return the compact timestamp used for run names."""
    return datetime.utcnow().strftime("%Y%m%dT%H%M%SZ")


def parse_args() -> argparse.Namespace:
    """Parse CLI arguments."""
    parser = argparse.ArgumentParser(
        description="Create one managed experiment run directory, manifest, and optional report stub."
    )
    parser.add_argument(
        "--repo-root",
        default=str(repo_root_from_script()),
        help="Repository root. Defaults to the path inferred from this script.",
    )
    parser.add_argument(
        "--topic",
        required=True,
        help="Experiment topic name under experiments/.",
    )
    parser.add_argument(
        "--run-name",
        help="Explicit run name. Defaults to <topic>_<variant>_<timestamp>.",
    )
    parser.add_argument(
        "--variant",
        default="manual",
        help="Variant label used when --run-name is omitted.",
    )
    parser.add_argument(
        "--report-path",
        help="Optional report path. Defaults to experiments/report/<run_name>.md.",
    )
    parser.add_argument(
        "--skip-report-init",
        action="store_true",
        help="Do not create a report stub when the report file is absent.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Create paths and manifest metadata but do not execute the command.",
    )
    parser.add_argument(
        "command",
        nargs=argparse.REMAINDER,
        help="Command to run. Tokens may use {run_dir}, {run_name}, {report_path}, {manifest_path}.",
    )
    return parser.parse_args()


def command_version(name: str) -> str | None:
    """Return one-line version text for a command when available."""
    if shutil.which(name) is None:
        return None
    result = subprocess.run(
        [name, "--version"],
        check=False,
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        return None
    output = "\n".join(part.strip() for part in (result.stdout, result.stderr) if part.strip())
    if not output:
        return None
    return output.splitlines()[0]


def git_value(repo_root: Path, *args: str) -> str | None:
    """Return one git value or None when unavailable."""
    result = subprocess.run(
        ["git", *args],
        cwd=repo_root,
        check=False,
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        return None
    value = result.stdout.strip()
    return value or None


def render_report_stub(
    *,
    topic: str,
    run_name: str,
    report_path: Path,
    result_dir: Path,
    manifest_path: Path,
    command: list[str],
    created_at: str,
    branch: str | None,
    commit: str | None,
) -> str:
    """Render one initial run report."""
    command_text = shlex.join(command) if command else "(dry-run)"
    branch_text = branch or "(unknown)"
    commit_text = commit or "(unknown)"
    return f"""# {run_name}

- Topic: {topic}
- Created At (UTC): {created_at}
- Result Dir: {result_dir}
- Run Manifest: {manifest_path}
- Branch: {branch_text}
- Commit: {commit_text}

## Question

<!-- What empirical question does this run answer? -->

## Comparison Target

<!-- main, baseline, previous run, or external reference. -->

## Protocol

- Command: `{command_text}`
- Report Path: `{report_path}`

## Results

<!-- Fill in summary.json, cases.jsonl, and the main observations after the run. -->

## Reproducibility Record

- `run_manifest.json`
- `run.log`
- `summary.json`
- `cases.jsonl`

## Critical Review Notes

<!-- What this run still does not justify. -->
"""


def build_manifest(
    *,
    repo_root: Path,
    topic: str,
    run_name: str,
    topic_dir: Path,
    result_dir: Path,
    report_path: Path,
    manifest_path: Path,
    command: list[str],
    created_at: str,
    status: str,
) -> dict[str, object]:
    """Build one manifest dictionary."""
    branch = git_value(repo_root, "branch", "--show-current")
    commit = git_value(repo_root, "rev-parse", "HEAD")
    git_dirty = git_value(repo_root, "status", "--short")
    return {
        "topic": topic,
        "run_name": run_name,
        "status": status,
        "created_at_utc": created_at,
        "repo_root": str(repo_root),
        "topic_dir": str(topic_dir),
        "result_dir": str(result_dir),
        "report_path": str(report_path),
        "manifest_path": str(manifest_path),
        "command": command,
        "server_context": {
            "hostname": socket.gethostname(),
            "platform": platform.platform(),
            "cwd": os.getcwd(),
            "user": os.environ.get("USER") or os.environ.get("USERNAME") or "(unknown)",
        },
        "tool_versions": {
            "python": platform.python_version(),
            "codex": command_version("codex"),
            "docker": command_version("docker"),
        },
        "git": {
            "branch": branch,
            "commit": commit,
            "dirty": bool(git_dirty),
            "status_short": git_dirty.splitlines() if git_dirty else [],
        },
    }


def write_manifest(path: Path, manifest: dict[str, object]) -> None:
    """Write one manifest JSON file."""
    path.write_text(json.dumps(manifest, indent=2, ensure_ascii=True) + "\n", encoding="utf-8")


def format_command(command: list[str], placeholders: dict[str, str]) -> list[str]:
    """Format command tokens with run placeholders."""
    if command and command[0] == "--":
        command = command[1:]
    return [token.format(**placeholders) for token in command]


def stream_command(command: list[str], *, cwd: Path, env: dict[str, str], log_path: Path) -> int:
    """Run one command, teeing output to stdout and the log file."""
    with log_path.open("w", encoding="utf-8") as log_handle:
        log_handle.write("$ " + shlex.join(command) + "\n")
        log_handle.flush()

        process = subprocess.Popen(
            command,
            cwd=cwd,
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        try:
            assert process.stdout is not None
            for line in process.stdout:
                sys.stdout.write(line)
                log_handle.write(line)
            return process.wait()
        except KeyboardInterrupt:
            process.terminate()
            try:
                return process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
                return 130


def main() -> int:
    """Run the CLI."""
    args = parse_args()
    repo_root = Path(args.repo_root).resolve()
    topic_dir = repo_root / "experiments" / args.topic
    if not topic_dir.is_dir():
        print(f"topic directory does not exist: {topic_dir}", file=sys.stderr)
        return 2

    run_name = args.run_name or f"{args.topic}_{args.variant}_{compact_timestamp()}"
    result_dir = topic_dir / "result" / run_name
    report_path = (
        Path(args.report_path).resolve()
        if args.report_path
        else (repo_root / "experiments" / "report" / f"{run_name}.md")
    )
    manifest_path = result_dir / "run_manifest.json"
    log_path = result_dir / "run.log"

    result_dir.mkdir(parents=True, exist_ok=True)
    report_path.parent.mkdir(parents=True, exist_ok=True)

    created_at = utc_now()
    placeholders = {
        "repo_root": str(repo_root),
        "topic_dir": str(topic_dir),
        "run_name": run_name,
        "run_dir": str(result_dir),
        "report_path": str(report_path),
        "manifest_path": str(manifest_path),
        "log_path": str(log_path),
    }
    command = format_command(args.command, placeholders)

    manifest = build_manifest(
        repo_root=repo_root,
        topic=args.topic,
        run_name=run_name,
        topic_dir=topic_dir,
        result_dir=result_dir,
        report_path=report_path,
        manifest_path=manifest_path,
        command=command,
        created_at=created_at,
        status="initialized" if args.dry_run else "running",
    )
    write_manifest(manifest_path, manifest)

    if not args.skip_report_init and not report_path.exists():
        git_info = manifest["git"]
        if not isinstance(git_info, dict):
            raise TypeError("git manifest entry must be a dictionary")
        report_path.write_text(
            render_report_stub(
                topic=args.topic,
                run_name=run_name,
                report_path=report_path,
                result_dir=result_dir,
                manifest_path=manifest_path,
                command=command,
                created_at=created_at,
                branch=git_info.get("branch") if isinstance(git_info.get("branch"), str) else None,
                commit=git_info.get("commit") if isinstance(git_info.get("commit"), str) else None,
            ),
            encoding="utf-8",
        )

    if args.dry_run:
        print(f"run_name={run_name}")
        print(f"result_dir={result_dir}")
        print(f"report_path={report_path}")
        print(f"manifest_path={manifest_path}")
        return 0

    if not command:
        print("a command is required unless --dry-run is used", file=sys.stderr)
        return 2

    env = dict(os.environ)
    env.update(
        {
            "EXPERIMENT_RUN_NAME": run_name,
            "EXPERIMENT_TOPIC": args.topic,
            "EXPERIMENT_RUN_DIR": str(result_dir),
            "EXPERIMENT_REPORT_PATH": str(report_path),
            "EXPERIMENT_RUN_MANIFEST": str(manifest_path),
            "EXPERIMENT_RUN_LOG": str(log_path),
        }
    )

    start_monotonic = time.monotonic()
    exit_code = stream_command(command, cwd=repo_root, env=env, log_path=log_path)
    finished_at = utc_now()
    manifest["finished_at_utc"] = finished_at
    manifest["duration_seconds"] = round(time.monotonic() - start_monotonic, 3)
    manifest["exit_code"] = exit_code
    manifest["status"] = "completed" if exit_code == 0 else "failed"
    write_manifest(manifest_path, manifest)
    return exit_code


if __name__ == "__main__":
    raise SystemExit(main())
