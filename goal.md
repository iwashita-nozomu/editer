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

Grow AgentCanon skills, tools, workflows, and evals until no new prompt/eval improvement candidates remain, while eliminating duplicate eval definitions and duplicate workflow/skill coverage.

## Exit Criteria

- [x] G1: Repository dependency review passes with `bash tools/agent_tools/run_repo_dependency_review.sh --fail-missing`.
- [x] G2: Code dependency extraction is reviewed with `bash tools/agent_tools/scan_code_dependencies.sh` for the affected surface.
- [x] G3: OOP/readability analysis is run with `python3 tools/agent_tools/analyze_oop_readability.py` and findings are fixed or documented.
- [x] G4: Repo-wide static analysis or CI passes with `make ci`, or the documented fallback `python3 -m pyright` plus `python3 -m ruff check python tests --select D,E,F,I,UP`.
- [x] G5: Objective-specific completion evidence is recorded.
- [x] G6: Skill/workflow prompt evals pass and produce no duplicate eval IDs, duplicate checklist IDs within an eval, duplicate target entries, or uncovered changed prompt surfaces.
- [x] G7: Any growth candidate reported by the eval/tool/workflow audit is either implemented, explicitly rejected with evidence, or removed as duplicate.

## Backlog

- [x] B1: Add a mechanical eval-manifest audit to the existing prompt eval runner instead of creating a parallel checker.
- [x] B2: Run the audit, remove duplicate eval coverage if found, and add missing eval coverage only for concrete uncovered prompt surfaces.
- [x] B3: Update workflow / tool docs so future goal-driven growth starts from audit output rather than a vague objective.
- [x] B4: Rerun the audit until it reports no growth candidates and no duplicates.

## Loop Log

- 2026-05-02 iteration 0:
  - Implemented AgentCanon prompt eval manifest audit in `tools/agent_tools/evaluate_skill_workflow_prompts.py`.
  - Consolidated duplicate explicit eval targets for `.codex/config.toml` and `agents/canonical/CODEX_WORKFLOW.md`.
  - Added eval coverage for audit close gates in `agents/evals/skill_workflow_prompt_eval.toml`.
  - Updated adaptive workflow and tool documentation to require `EVAL_AUDIT_STATUS=pass` and `EVAL_GROWTH_CANDIDATES=0`.
  - Refined OOP readability analyzer so function-local accumulator mutation is not treated as an external effect boundary.
  - Evidence so far: dependency review pass, code dependency scan pass, OOP/readability pass with score 87, prompt eval pass with 405/405 checks and 0 growth candidates.
- 2026-05-02 closeout:
  - Template snapshot synced through AgentCanon `67ceaf0`.
  - Final gates passed: `run_repo_dependency_review.sh --fail-missing`, `make agent-checks`, `make docs-check`, and `make ci`.
  - `make ci` evidence: 175 pytest tests plus 6 subtests passed, pyright 0 errors, pydocstyle pass, ruff pass.
