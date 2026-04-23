# Schedule

- Run ID: 20260423-010426-tighten-markdown-math-checker-delimiters
- Task: tighten markdown math checker delimiters
- Owner: codex

## Stage Plan

| Stage | Owner Agent | Review Agent | Inputs | Exit Criteria | Status |
| ----- | ----------- | ------------ | ------ | ------------- | ------ |
| context-and-reuse | parent codex | parent change-review mindset | user request, checker source, sync docs, existing tool-test patterns | request clauses, reuse decision, and canonical shared paths are fixed | complete |
| implementation | parent codex as single writer | parent document-flow and change-review mindset | checker source, test patterns, sync spec docs | checker logic, regression tests, and shared-surface metadata are patched without duplicate truth surfaces | complete |
| validation-and-closeout | parent codex verifier/auditor mindset | parent final-review mindset | patched files plus validation commands | sync check and required validation pass, closeout artifacts are populated, commit/push complete | complete |

## Clause Coverage

| Clause ID | Covered By Stage | Review Gate | Status |
| --------- | ---------------- | ----------- | ------ |
| R-C1 | context-and-reuse, implementation, validation-and-closeout | targeted regression tests plus docs check | complete |
| R-C2 | context-and-reuse, implementation, validation-and-closeout | shared surface sync check plus shared-canon PR gate | complete |
| R-N1 | implementation, validation-and-closeout | canonical-path and sync-surface review | complete |
| R-N2 | implementation, validation-and-closeout | checker regression suite | complete |
| R-E1 | implementation, validation-and-closeout | pytest evidence | complete |
| R-E2 | validation-and-closeout | `verification.txt` | complete |
| R-E3 | validation-and-closeout | `closeout_gate.md` and git evidence | complete |

## Planned Work Units

<!-- This table is the canonical task TODO surface. Keep concrete work units and statuses here until closeout. -->

| Unit ID | Clause IDs | Owner | Completion Evidence | Next Gate | Status |
| ------- | ---------- | ----- | ------------------- | --------- | ------ |
| W1 | R-C1,R-C2 | codex | clause freeze, checker/file inventory, shared-surface reuse decision in run bundle | implementation | complete |
| W2 | R-C1,R-N2,R-E1 | codex | patched `vendor/agent-canon/tools/docs/check_markdown_math.py` plus regression tests | validation | complete |
| W3 | R-C2,R-N1 | codex | patched sync spec and root symlink view for `tests/tools/test_check_markdown_math.py` | validation | complete |
| W4 | R-E2,R-E3 | codex | validation pass, closeout artifacts, commit, push | closeout | complete |

## Task Completion Boundary

- No user-facing completion before the checker logic, regression test, shared sync surface, validation commands, commit, and push are all complete.

## Explicit Subagents

- none; parent direct handling is justified here because the task is a single shared checker, one regression test, and one sync-spec update with a single write scope

## Reuse And Continuity Constraints

- follow the existing checker message style and CLI structure in `vendor/agent-canon/tools/docs/check_markdown_math.py`
- follow the established shared-surface mapping format in `tools/sync_agent_canon.sh`
- follow existing `tests/tools/` CLI-subprocess test style and keep `vendor/agent-canon/` as the source of truth

## Risks

- root-only tests would create repo-local drift for a shared tool; the sync spec must move in the same pass
- changing delimiter policy without tests risks silent regression in docs check behavior
- the inline-math wording in the user request is slightly ambiguous, so the implementation relies on existing repo policy rather than inventing a new convention
