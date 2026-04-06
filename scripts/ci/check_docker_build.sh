#!/usr/bin/env bash
set -euo pipefail

WORKSPACE_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$WORKSPACE_ROOT"

BUILDER="${DOCKER_BUILDER:-auto}"
DOCKERFILE_PATH="${DOCKERFILE_PATH:-docker/Dockerfile}"
BUILD_CONTEXT="${DOCKER_BUILD_CONTEXT:-.}"
IMAGE_TAG="${DOCKER_IMAGE_TAG:-project-template:docker-build-check}"
PULL_BASE=0
NO_CACHE=0
SKIP_RUN=0
KEEP_IMAGE=0

show_help() {
  cat <<'EOF'
Usage: bash scripts/ci/check_docker_build.sh [OPTIONS]

Dockerfile build smoke check.

Options:
  --builder <auto|docker|podman>  Select the container builder. Default: auto
  --dockerfile <path>             Dockerfile path. Default: docker/Dockerfile
  --context <path>                Build context. Default: .
  --tag <image:tag>               Temporary image tag
  --pull                          Pull the latest base image before build
  --no-cache                      Build without using the cache
  --skip-run                      Skip the post-build runtime smoke check
  --keep-image                    Keep the built image after the check
  -h, --help                      Show this help
EOF
}

require_value() {
  local option_name="$1"
  if [[ $# -lt 2 || -z "${2:-}" ]]; then
    echo "Missing value for ${option_name}" >&2
    exit 1
  fi
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --builder)
      require_value "$1" "${2:-}"
      BUILDER="$2"
      shift 2
      ;;
    --dockerfile)
      require_value "$1" "${2:-}"
      DOCKERFILE_PATH="$2"
      shift 2
      ;;
    --context)
      require_value "$1" "${2:-}"
      BUILD_CONTEXT="$2"
      shift 2
      ;;
    --tag)
      require_value "$1" "${2:-}"
      IMAGE_TAG="$2"
      shift 2
      ;;
    --pull)
      PULL_BASE=1
      shift
      ;;
    --no-cache)
      NO_CACHE=1
      shift
      ;;
    --skip-run)
      SKIP_RUN=1
      shift
      ;;
    --keep-image)
      KEEP_IMAGE=1
      shift
      ;;
    -h|--help)
      show_help
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      show_help >&2
      exit 1
      ;;
  esac
done

if [[ ! -f "$DOCKERFILE_PATH" ]]; then
  echo "Dockerfile not found: $DOCKERFILE_PATH" >&2
  exit 1
fi

detect_builder() {
  case "$BUILDER" in
    auto)
      if command -v docker >/dev/null 2>&1; then
        echo "docker"
        return 0
      fi
      if command -v podman >/dev/null 2>&1; then
        echo "podman"
        return 0
      fi
      echo "Neither docker nor podman is available." >&2
      return 127
      ;;
    docker|podman)
      if ! command -v "$BUILDER" >/dev/null 2>&1; then
        echo "Requested builder is not available: $BUILDER" >&2
        return 127
      fi
      echo "$BUILDER"
      ;;
    *)
      echo "Unsupported builder: $BUILDER" >&2
      return 1
      ;;
  esac
}

CONTAINER_BUILDER="$(detect_builder)"
IMAGE_BUILT=0

cleanup() {
  if [[ "$IMAGE_BUILT" -eq 1 && "$KEEP_IMAGE" -eq 0 ]]; then
    "$CONTAINER_BUILDER" image rm -f "$IMAGE_TAG" >/dev/null 2>&1 || true
  fi
}

trap cleanup EXIT

BUILD_CMD=(
  "$CONTAINER_BUILDER"
  build
  -f "$DOCKERFILE_PATH"
  -t "$IMAGE_TAG"
)

if [[ "$PULL_BASE" -eq 1 ]]; then
  BUILD_CMD+=(--pull)
fi

if [[ "$NO_CACHE" -eq 1 ]]; then
  BUILD_CMD+=(--no-cache)
fi

BUILD_CMD+=("$BUILD_CONTEXT")

echo "Running Docker build check"
echo "  builder: $CONTAINER_BUILDER"
echo "  dockerfile: $DOCKERFILE_PATH"
echo "  context: $BUILD_CONTEXT"
echo "  tag: $IMAGE_TAG"
echo ""

"${BUILD_CMD[@]}"
IMAGE_BUILT=1

if [[ "$SKIP_RUN" -eq 0 ]]; then
  echo ""
  echo "Running container smoke check"
  "$CONTAINER_BUILDER" run --rm \
    -e GIT_SAFE_DIRECTORIES="/tmp/project-template-safe-dir" \
    "$IMAGE_TAG" /bin/bash -lc '
    set -euo pipefail
    python3 --version
    python3 -m pip --version
    node --version
    npm --version
    codex --version
    docker --version
    safe_dirs="$(git config --global --get-all safe.directory || true)"
    printf "%s\n" "$safe_dirs" | grep -Fx "/workspace" >/dev/null
    printf "%s\n" "$safe_dirs" | grep -Fx "/tmp/project-template-safe-dir" >/dev/null
  '
fi

echo ""
echo "Docker build check completed successfully"
