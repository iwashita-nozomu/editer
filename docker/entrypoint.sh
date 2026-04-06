#!/usr/bin/env bash
set -euo pipefail

add_safe_directory() {
  local candidate="$1"
  if [[ -z "$candidate" ]]; then
    return 0
  fi
  if git config --global --get-all safe.directory 2>/dev/null | grep -Fxq "$candidate"; then
    return 0
  fi
  git config --global --add safe.directory "$candidate"
}

if [[ "${AUTO_GIT_SAFE_DIRECTORY:-1}" != "0" ]]; then
  add_safe_directory "${PWD:-}"
fi

if [[ -n "${GIT_SAFE_DIRECTORIES:-}" ]]; then
  IFS=':' read -r -a safe_directories <<< "${GIT_SAFE_DIRECTORIES}"
  for safe_directory in "${safe_directories[@]}"; do
    add_safe_directory "$safe_directory"
  done
fi

exec "$@"
