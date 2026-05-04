# Goal Work Breakdown
<!--
@dependency-start
responsibility Records executable TODO units derived from goal.md.
upstream implementation ../vendor/agent-canon/tools/agent_tools/goal_loop.py generates this plan
@dependency-end
-->

## Summary

- goal_file: `/mnt/l/workspace/project_template/vendor/agent-canon/goal.md`
- goal_status_field: `achieved`
- goal_loop_status: `achieved`
- next_action: `close_goal_loop`
- open_exit_criteria: `0`
- open_backlog_items: `0`
- optional_goal_items: `5`

## Progress Log

This report is cumulative. Each row records intent, evidence, and the
resulting slice-level outcome that contributed to closeout.

| Slice | Intent | Evidence | Result |
| ----- | ------ | -------- | ------ |
| Model routing | Route simple coding to the cheapest suitable model while keeping code-reading on Spark. | `vendor/agent-canon/agents/TASK_WORKFLOWS.md`; `vendor/agent-canon/agents/canonical/CODEX_WORKFLOW.md`; `vendor/agent-canon/tools/agent_tools/check_agent_runtime_alignment.py`. | `G2` is covered by the routing workflow and runtime alignment checks. |
| Slide workflow | Standardize slide production on a fixed template with slot mapping and layout review. | `vendor/agent-canon/agents/workflows/slide-production-workflow.md`; `vendor/agent-canon/agents/evals/skill_workflow_prompt_eval.toml`; `vendor/agent-canon/documents/codex-configuration-slides.md`. | `G3` has a dedicated workflow surface and prompt-eval coverage. |
| Hypothesis validation | Require read-first survey, checklist creation, tool selection, and rejection analysis before edits. | `vendor/agent-canon/agents/workflows/hypothesis-validation-workflow.md`; `vendor/agent-canon/agents/evals/skill_workflow_prompt_eval.toml`; `vendor/agent-canon/agents/workflows/adaptive-improvement-workflow.md`. | `G4` and the prompt-repair side of `G5` are covered by workflow and eval hooks. |
| AgentCanon unification | Keep vendor main, the template snapshot, and runtime views aligned through the canonical update lane. | `vendor/agent-canon` main at `d2a63dc`; root gitlink update; `check_agent_runtime_alignment.py` pass. | `G8` is complete and the shared canon lane is synchronized. |
| Token efficiency | Reduce Codex footprint without regressing skill or behavior evals. | [reports/token_efficiency_comparison.md](token_efficiency_comparison.md) with `TOKEN_FOOTPRINT_RATIO=0.306`; `evaluate_skill_workflow_prompts.py` passed; `check_agent_runtime_alignment.py` passed. | `G9` has measured pass evidence and the target ratio is below 0.5. |
| Cumulative report | Keep a single progress report that records per-slice intent, path, evidence, and intermediate results. | This section, the token-efficiency report, and the goal work breakdown. | `G6` is now represented as a cumulative closeout artifact. |
| Closeout | Confirm repo-wide gates, dependency review, and goal-loop completion. | `run_repo_dependency_review.sh --fail-missing`; `check_convention_compliance.py`; `make ci`; `goal_loop.py status`. | `NEXT_ACTION=close_goal_loop` and the loop can close. |

## Work Units

| Unit ID | Source | Work To Do | Evidence To Produce | Status |
| ------- | ------ | ---------- | ------------------- | ------ |
| none | none | No unchecked goal items. | closeout evidence | complete |

## Optional Goal Item Catalog

Optional catalog items are not emitted as `GW*` work units by default.
Promote one by copying it into `Exit Criteria` or `Backlog` when the
current objective requires it.

| Item ID | Text | Status |
| ------- | ---- | ------ |
| O1 | (research) External web research is required, with source links and current-date verification recorded in the run bundle. | available |
| O2 | (benchmark) Benchmark or experiment evidence is required, with reproducible commands, seeds, environment, and comparison artifacts. | available |
| O3 | (docs) Long-form documentation, slide, or user-guide review is required before closeout. | available |
| O4 | (release) Release, branch-integration, push, or downstream template snapshot coordination is required. | available |
| O5 | (subagents) Explicit read-only specialist review or implementation handoff is required for the goal. | available |

## Schedule Transfer Rule

- Copy every open `GW*` row into the run bundle `schedule.md` before editing.
- Do not start implementation from a bare objective without this breakdown.
- If `NEXT_ACTION=run_next_iteration`, create the next iteration slice from the first open work unit.
