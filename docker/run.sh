#!/usr/bin/env bash
# Start a Geneva build container with GPU, source tree, build directory,
# and Claude session data all mounted at their original absolute paths.
#
# Usage:
#   docker/run.sh                     # interactive bash shell
#   docker/run.sh bash                # same
#   docker/run.sh cmake <args...>     # run cmake directly
#   docker/run.sh make -j$(nproc)     # run make directly
#
# Environment variables (override defaults):
#   GENEVA_BUILD_DIR   out-of-source build directory (default: ~/build/geneva)
#   GENEVA_IMAGE       Docker image name            (default: geneva-build:latest)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "${SCRIPT_DIR}")"

BUILD_DIR="${GENEVA_BUILD_DIR:-${HOME}/build/geneva}"
IMAGE="${GENEVA_IMAGE:-geneva-build:latest}"
CLAUDE_DIR="${HOME}/.claude"

mkdir -p "${BUILD_DIR}"

exec docker run -it --rm \
    --gpus all \
    --shm-size=8g \
    -v "${REPO_ROOT}:${REPO_ROOT}" \
    -v "${BUILD_DIR}:${BUILD_DIR}" \
    -v "${CLAUDE_DIR}:${CLAUDE_DIR}" \
    -u "$(id -u):$(id -g)" \
    -e HOME="${HOME}" \
    -e GENEVA_BUILD_DIR="${BUILD_DIR}" \
    -w "${REPO_ROOT}" \
    "${IMAGE}" \
    "${@:-bash}"