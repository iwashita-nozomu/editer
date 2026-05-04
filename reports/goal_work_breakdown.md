# Goal Work Breakdown
<!--
@dependency-start
responsibility Records executable TODO units derived from goal.md.
upstream implementation ../vendor/agent-canon/tools/agent_tools/goal_loop.py generates this plan
@dependency-end
-->

## Summary

- goal_file: `/mnt/l/workspace/project_template/vendor/agent-canon/goal.md`
- goal_status_field: `active`
- goal_loop_status: `continue`
- next_action: `run_next_iteration`
- open_exit_criteria: `9`
- open_backlog_items: `9`
- optional_goal_items: `5`

## Progress Log

This report is cumulative. Add one row per finished or validated slice so the
closeout path can show intent, evidence, and intermediate results in one place.

| Slice | Intent | Evidence | Result |
| ----- | ------ | -------- | ------ |
| Token efficiency | Reduce Codex footprint without regressing skill or behavior evals. | [reports/token_efficiency_comparison.md](token_efficiency_comparison.md) with `TOKEN_FOOTPRINT_RATIO=0.306`; `evaluate_skill_workflow_prompts.py` passed; `check_agent_runtime_alignment.py` passed. | `G9` has measured pass evidence, but the goal still has other open exit criteria. |

## Work Units

| Unit ID | Source | Work To Do | Evidence To Produce | Status |
| ------- | ------ | ---------- | ------------------- | ------ |
| GW1 | exit_criteria:G1 | The goal contract, workflow, and backlog are written from the TODO | specific artifact path, command output, or review decision | open |
| GW2 | exit_criteria:G2 | Model-routing workflow and prompt surfaces route simple coding to the | specific artifact path, command output, or review decision | open |
| GW3 | exit_criteria:G3 | Slide workflow uses a fixed PPT template and supports text, equation, | specific artifact path, command output, or review decision | open |
| GW4 | exit_criteria:G4 | Hypothesis-validation workflow enforces read-first survey, checklist | specific artifact path, command output, or review decision | open |
| GW5 | exit_criteria:G5 | Coding workflow prompts are revised when tools discard or rewrite | specific artifact path, command output, or review decision | open |
| GW6 | exit_criteria:G6 | The goal run produces a cumulative quantitative closeout report that | specific artifact path, command output, or review decision | open |
| GW7 | exit_criteria:G7 | Repository dependency review, prompt evals, and template `make ci` | `run_repo_dependency_review.sh` output | open |
| GW8 | exit_criteria:G8 | AgentCanon main, the template snapshot branch, and repo-local runtime | specific artifact path, command output, or review decision | open |
| GW9 | exit_criteria:G9 | The token-efficient workflow slice shows at least 50% lower token | `reports/token_efficiency_comparison.md`; `TOKEN_FOOTPRINT_RATIO=0.306` | evidence-recorded |
| GW10 | backlog:B1 | Draft the goal work breakdown from the TODO into checkable work | specific artifact path, command output, or review decision | open |
| GW11 | backlog:B2 | Survey existing routing, slide, hypothesis, coding, and reporting | specific artifact path, command output, or review decision | open |
| GW12 | backlog:B3 | Implement the model-routing slice and its eval coverage first. | specific artifact path, command output, or review decision | open |

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
