# subagent-bootstrap

## Purpose

specialist delegation が必要な task で、run bundle、役割分担、write-scope を崩さずに起動します。

## Use When

- run artifact を残したい
- specialist を使う
- reviewer / implementer の責務を分けたい

## Core References

- `agents/TASK_WORKFLOWS.md`
- `agents/COMMUNICATION_PROTOCOL.md`
- `agents/canonical/CODEX_SUBAGENTS.md`
- `scripts/agent_tools/bootstrap_agent_run.py`

## Standard Command

```bash
python3 scripts/agent_tools/bootstrap_agent_run.py \
  --task "short task summary" \
  --owner "codex"
```
