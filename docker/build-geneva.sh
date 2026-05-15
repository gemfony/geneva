#!/usr/bin/env bash
# Configure and build Geneva inside the container (or on the host).
#
# Usage:
#   docker/run.sh docker/build-geneva.sh [cmake-extra-args...]
#
# Examples:
#   docker/run.sh docker/build-geneva.sh
#   docker/run.sh docker/build-geneva.sh -DGENEVA_BUILD_EXAMPLES=TRUE

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "${SCRIPT_DIR}")"
BUILD_DIR="${GENEVA_BUILD_DIR:-${HOME}/build/geneva}"

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

cmake "${REPO_ROOT}" \
    -GNinja \
    -DGENEVA_BUILD_TYPE=Release \
    -DGENEVA_BUILD_TESTS=TRUE \
    -DGENEVA_BUILD_EXAMPLES=FALSE \
    -DGENEVA_BUILD_BENCHMARKS=FALSE \
    -DGENEVA_BUILD_WITH_MPI_CONSUMER=TRUE \
    -DUSECUDARNG=TRUE \
    -DCMAKE_CUDA_ARCHITECTURES=89 \
    -DBOOST_ROOT=/opt/boost \
    "$@"

ninja -j"$(nproc)"