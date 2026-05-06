# Goal
<!--
@dependency-start
responsibility Defines the top-level goal loop contract for this repository.
upstream design README.md repository entrypoint
downstream implementation tools/agent_tools/goal_loop.py consumes this contract
@dependency-end
-->

## Loop Contract

- goal_status: achieved
- run_safety_cap: 3
- current_iteration: 0
- active_run_id:
- stop_reason:

## Objective

Keep the repository goal loop passable with current, inspectable evidence for
all exit criteria and backlog items.

## Exit Criteria

- [x] G1: Repository dependency review passes with `bash tools/agent_tools/run_repo_dependency_review.sh --fail-missing`.
- [x] G2: Code dependency extraction is reviewed with `bash tools/agent_tools/scan_code_dependencies.sh` for the affected surface.
- [x] G3: OOP/readability analysis is run with `python3 tools/agent_tools/analyze_oop_readability.py` and findings are fixed or documented.
- [x] G4: Repo-wide static analysis or CI passes with `make ci`, or the documented fallback `python3 -m pyright` plus `python3 -m ruff check python tests --select D,E,F,I,UP`.
- [x] G5: Objective-specific completion evidence is recorded.

## Backlog

- [x] B1: Build the prompt-to-artifact checklist that maps objective clauses to files, commands, gates, and completion evidence.
- [x] B2: Survey existing docs, tools, tests, and reusable surfaces before adding or deleting anything; list reuse, consolidation, and deletion candidates.
- [x] B3: Execute one cohesive implementation slice that advances the selected related surfaces together instead of stopping after one micro-fix.
- [x] B4: Run dependency review, code dependency scan, OOP/readability, and task-relevant prompt/doc/convention checks; fix any failure in the same iteration.
- [x] B5: Refresh the goal work breakdown, close completed backlog items with evidence, and continue immediately if NEXT_ACTION still reports run_next_iteration.

## Loop Log

- iteration evidence: current tree contains cleanup and prototype commits
  `0d7d94e` and `0e6236f`, with AgentCanon sync commits `5e6782e` and
  `d40e13d`; `main` is synchronized with `origin/main`.
- validation evidence: current-state audit ran
  `bash tools/agent_tools/run_repo_dependency_review.sh --fail-missing`
  (`REPO_DEPENDENCY_REVIEW=pass`), explicit affected-surface
  `bash tools/agent_tools/scan_code_dependencies.sh` (`CODE_DEPENDENCY_SCAN=pass
  files=7`), `python3 tools/agent_tools/analyze_oop_readability.py`
  (`OOP_READABILITY=pass`, score `94`, grade `low-risk`, warn/error density
  `0.0`), and `make ci` (`CI チェック完了: すべて成功`).
- closeout evidence: `reports/goal_work_breakdown.md` contains the current
  prompt-to-artifact completion audit checklist; `goal_loop.py status` and MCP
  `goal.loop_status` both report `NEXT_ACTION=close_goal_loop`.
