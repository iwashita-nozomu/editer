#!/usr/bin/env bash
# @dependency-start
# responsibility Runs generate runtime compose shell automation.
# upstream environment ../docker/packs/default.toml default devcontainer runtime pack
# upstream implementation ../tools/ci/render_devcontainer_compose.py renders compose file
# @dependency-end
set -euo pipefail

python3 tools/ci/render_devcontainer_compose.py --pack docker/packs/default.toml --output .devcontainer/docker-compose.generated.yml
