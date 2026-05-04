# Codex Token Footprint Comparison
<!--
@dependency-start
responsibility Records Codex token footprint comparison evidence.
upstream implementation ../../tools/agent_tools/compare_codex_token_footprints.py generates this report
@dependency-end
-->

## Summary

- comparison_status: pass
- baseline_total_tokens: 42733
- candidate_total_tokens: 13094
- token_ratio: 0.306
- target_ratio: 0.500

## Sessions

| Label | Session File | input | cached input | output | reasoning output | total |
| ----- | ------------ | ----- | ------------ | ------ | ---------------- | ----- |
| baseline | /home/niwashita/.codex/sessions/2026/04/16/rollout-2026-04-16T21-26-34-019d9641-e7e0-7080-9051-4c7d21726ad5.jsonl | 41332 | 24192 | 1401 | 855 | 42733 |
| candidate | /home/niwashita/.codex/sessions/2026/04/05/rollout-2026-04-05T22-47-14-019d5de5-ce9e-78e0-bde7-5a1ba5121a36.jsonl | 13060 | 4480 | 34 | 23 | 13094 |

## Machine Status

- TOKEN_FOOTPRINT_COMPARISON=pass
- TOKEN_FOOTPRINT_RATIO=0.306
- TOKEN_FOOTPRINT_TARGET=0.500
