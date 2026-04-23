# User Request Contract

- Run ID: 20260423-010426-tighten-markdown-math-checker-delimiters
- Task: tighten markdown math checker delimiters
- Owner: codex
- Created At (UTC): 2026-04-23T01:04:26Z

## Gate Status

- all_clauses_resolved: yes
- forbidden_drift_detected: no
- deferred_clause_ids:
- unresolved_clause_ids:

## Requirements Resolution Sweep

- Searched `memory/USER_PREFERENCES.md`, `documents/SHARED_RUNTIME_SURFACES.md`, `vendor/agent-canon/documents/SHARED_RUNTIME_SURFACES.md`, `vendor/agent-canon/documents/agent-canon-subtree-migration.md`, `tools/sync_agent_canon.sh`, `vendor/agent-canon/tools/sync_agent_canon.sh`, `tools/ci/run_docs_checks.sh`, `vendor/agent-canon/tools/docs/check_markdown_math.py`, and existing `tests/tools/` / `vendor/agent-canon/tests/tools/` patterns.
- Resolved from accumulated context:
  - the markdown math checker is a shared-canon tool, so the durable source of truth is `vendor/agent-canon/`
  - root test coverage for shared surfaces should be restored through the sync spec, not by leaving an unsynced root-only file
  - the existing checker and repo precedent already encode inline math as `$...$` and display math as `$$...$$`; the user wording was applied against that existing policy surface

## Resolved From Accumulated Context

| Clause ID | Resolved From | Evidence Path | Resolution | Remaining Risk |
| --------- | ------------- | ------------- | ---------- | -------------- |
| R-C1 | repo_or_code_precedent | `vendor/agent-canon/tools/docs/check_markdown_math.py`, `tools/ci/run_docs_checks.sh` | interpret the request as enforcing display `$$...$$` and inline `$...$` consistently across the markdown math checker | user wording was slightly ambiguous, but repo precedent was already aligned |
| R-C2 | repo_or_code_precedent | `vendor/agent-canon/documents/SHARED_RUNTIME_SURFACES.md`, `vendor/agent-canon/documents/agent-canon-subtree-migration.md`, `tools/sync_agent_canon.sh` | shared-canon tool changes must update the shared sync surface and root runtime view in the same pass | none |

## Must-Do Clauses

| Clause ID | Source Bucket | User Wording Or Evidence | Operational Interpretation | Owner Stage | Evidence Path | Status |
| --------- | ------------- | ------------------------- | -------------------------- | ----------- | ------------- | ------ |
| R-C1 | current_request | `MDの書式チェッカを修正して．独立数式は$$ $$，文中は$$に固定` | tighten the markdown math checker so display math is accepted only as `$$...$$` and inline math only as `$...$`, rejecting legacy and mixed delimiter usage | implementation, validation | `vendor/agent-canon/tools/docs/check_markdown_math.py`, `vendor/agent-canon/tests/tools/test_check_markdown_math.py` | resolved |
| R-C2 | repo_or_code_precedent | shared canon changes must use `vendor/agent-canon/` as source of truth | add durable regression coverage and sync metadata in the shared canon, then restore the root runtime view through the canon sync mechanism | implementation, validation, closeout | `vendor/agent-canon/tests/tools/test_check_markdown_math.py`, `vendor/agent-canon/documents/SHARED_RUNTIME_SURFACES.md`, `vendor/agent-canon/documents/agent-canon-subtree-migration.md`, `tools/sync_agent_canon.sh` | resolved |

## Must-Not-Do Clauses

| Clause ID | Source Bucket | Forbidden Drift | Why It Is Forbidden | Guard Stage | Evidence Path | Status |
| --------- | ------------- | --------------- | ------------------- | ----------- | ------------- | ------ |
| R-N1 | durable_user_preference | leave a root-only test file or repo-local-only checker behavior without shared-canon sync | the user consistently rejects duplicate or parallel truth surfaces; shared tooling must stay canonical in `vendor/agent-canon/` | implementation, validation | sync spec, `bash tools/sync_agent_canon.sh check` | resolved_clean |
| R-N2 | repo_or_code_precedent | accept legacy `\\(...\\)` / `\\[...\\]`, standalone `$...$` display lines, or inline `$$...$$` after the fix | the request is specifically about tightening delimiter policy, so mixed legacy behavior would leave the checker under-specified | implementation, test, validation | checker logic and regression tests | resolved_clean |

## Completion Evidence Clauses

| Clause ID | Source Bucket | Required Evidence | Where It Must Appear | Owner Stage | Status |
| --------- | ------------- | ----------------- | -------------------- | ----------- | ------ |
| R-E1 | current_request | regression tests covering accepted and rejected math delimiter cases | `vendor/agent-canon/tests/tools/test_check_markdown_math.py` and root symlink view `tests/tools/test_check_markdown_math.py` | implementation, validation | resolved |
| R-E2 | repo_or_code_precedent | docs/tooling validation for the updated checker and shared surface sync | `verification.txt` | validation | resolved |
| R-E3 | repo_or_code_precedent | closeout gate proving canonical tree completeness, commit, and push | `closeout_gate.md`, `verification.txt` | closeout | resolved |

## Source Bucket Rules

- Allowed buckets: `current_request`, `durable_user_preference`, `repo_or_code_precedent`, `domain_or_external_constraint`, `unknown_or_open_question`.
- Durable user preferences do not become task requirements unless the current request or repo evidence supports the conversion.
- Unknowns stay unresolved, deferred, or escalated; they are not converted into silent assumptions.
- Active must-do, must-not-do, and completion-evidence clauses must not use `unknown_or_open_question`; unresolved items must move to Deferred Or Rejected Clauses after the resolution sweep.
- Do not stop at the first ambiguity if accumulated notes, repo docs, local code, tests, or prior logs can resolve it without changing user intent.

## Deferred Or Rejected Clauses

| Clause ID | Reason | Escalation Or Follow-Up Path | Status |
| --------- | ------ | ---------------------------- | ------ |
| U-C1 | user wording for inline math omitted spaces and could be read ambiguously | resolved by existing checker policy and repo precedent; no divergent repo-wide convention was found | resolved |

## Update Rule

- Every planning, design, implementation, and review artifact must cite the clause IDs it covers.
- If active work does not map to at least one must-do clause, stop and escalate instead of continuing.
- Closeout stays locked until every must-do and completion-evidence clause is resolved and every must-not-do clause remains clean.
