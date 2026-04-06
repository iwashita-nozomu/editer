---
name: environment-maintenance
description: Use when touching Docker, CI, dependencies, runtime compatibility, or repository-level development environment instructions.
---

# Environment Maintenance

1. Treat `docker/` as the shared environment canon.
1. Start from `agents/templates/environment_change_proposal.md` when proposing a new repo-wide tool or dependency.
1. Update dependency definitions and related docs in the same change.
1. Check CI and local validation commands together.
1. Use `documents/coding-conventions-project.md` and `documents/WORKFLOW_INVENTORY.md`.
1. Do not canonize host-global installs as the repository default.
