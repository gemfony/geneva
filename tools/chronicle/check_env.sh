#!/usr/bin/env bash
# check_env.sh — Phase -2 environment check for gemfony-chronicle development.
# Exit 0 if all required tools pass; exit 1 if any required tool is missing.

PASS=0; WARN=0; FAIL=0
REQUIRED_FAILED=()

ok()   { printf "[OK  ] %s\n" "$*"; PASS=$((PASS + 1)); }
warn() { printf "[WARN] %s\n" "$*"; WARN=$((WARN + 1)); }
fail() { printf "[FAIL] %s\n" "$*"; FAIL=$((FAIL + 1)); REQUIRED_FAILED+=("$*"); }

echo "============================================"
echo " gemfony-chronicle environment check"
echo "============================================"
echo ""

# ── 1. C++ compiler ──────────────────────────────────────────────────────────
COMPILER_OK=0
if command -v g++ &>/dev/null; then
    VER=$(g++ -dumpfullversion 2>/dev/null | cut -d. -f1)
    if [[ "${VER}" -ge 13 ]]; then
        ok "GCC ${VER} (g++)"; COMPILER_OK=1
    else
        fail "GCC ${VER} too old — need >= 13  (sudo apt install gcc-13 g++-13)"
    fi
fi
if command -v clang++ &>/dev/null; then
    VER=$(clang++ --version 2>/dev/null | head -1 | grep -oE '[0-9]+\.[0-9]+' | head -1 | cut -d. -f1)
    if [[ "${VER}" -ge 18 ]]; then
        ok "Clang ${VER} (clang++)"; COMPILER_OK=1
    else
        warn "Clang ${VER} found but < 18 — GCC will be used if available"
    fi
fi
if [[ ${COMPILER_OK} -eq 0 ]]; then
    fail "No supported compiler found — need GCC >= 13 or Clang >= 18"
fi

# ── 2. CMake 3.27+ ───────────────────────────────────────────────────────────
if command -v cmake &>/dev/null; then
    VER=$(cmake --version 2>/dev/null | head -1 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+')
    MAJOR=$(echo "${VER}" | cut -d. -f1)
    MINOR=$(echo "${VER}" | cut -d. -f2)
    if [[ "${MAJOR}" -gt 3 || ( "${MAJOR}" -eq 3 && "${MINOR}" -ge 27 ) ]]; then
        ok "CMake ${VER}"
    else
        fail "CMake ${VER} too old — need >= 3.27  (pip install cmake --upgrade)"
    fi
else
    fail "CMake not found  (sudo apt install cmake  or  pip install cmake)"
fi

# ── 3. xxhash ────────────────────────────────────────────────────────────────
if pkg-config --modversion libxxhash &>/dev/null 2>&1; then
    VER=$(pkg-config --modversion libxxhash 2>/dev/null)
    ok "xxhash ${VER} (pkg-config)"
elif [[ -f /usr/include/xxhash.h || -f /usr/local/include/xxhash.h ]]; then
    ok "xxhash (header found; pkg-config not configured)"
else
    fail "xxhash not found  (sudo apt install libxxhash-dev)"
fi

# ── 4. Catch2 v3 ─────────────────────────────────────────────────────────────
CATCH2_OK=0
for PREFIX in /opt/catch2 /usr /usr/local; do
    if [[ -f "${PREFIX}/include/catch2/catch_all.hpp" ]]; then
        ok "Catch2 v3 (${PREFIX})"; CATCH2_OK=1; break
    fi
done
if [[ ${CATCH2_OK} -eq 0 ]]; then
    TMPDIR_CMAKE=$(mktemp -d)
    cat >"${TMPDIR_CMAKE}/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.14)
project(catch2_probe)
find_package(Catch2 3 REQUIRED)
EOF
    if cmake -S "${TMPDIR_CMAKE}" -B "${TMPDIR_CMAKE}/build" \
             -DCMAKE_PREFIX_PATH=/opt/catch2 \
             --log-level=ERROR -Wno-dev >/dev/null 2>&1; then
        ok "Catch2 v3 (found via CMake)"; CATCH2_OK=1
    fi
    rm -rf "${TMPDIR_CMAKE}"
fi
if [[ ${CATCH2_OK} -eq 0 ]]; then
    fail "Catch2 v3 not found — build:
       git clone --depth 1 -b v3.x https://github.com/catchorg/Catch2.git /tmp/Catch2
       cmake -S /tmp/Catch2 -B /tmp/Catch2/build -DCMAKE_INSTALL_PREFIX=/opt/catch2
       cmake --build /tmp/Catch2/build --parallel && sudo cmake --install /tmp/Catch2/build"
fi

# ── 5. ROOT 6.34+ ────────────────────────────────────────────────────────────
ROOT_BIN="/opt/root/bin/root"
THISROOT="/opt/root/bin/thisroot.sh"
if [[ -x "${ROOT_BIN}" && -f "${THISROOT}" ]]; then
    ROOT_VER_LINE=$(bash -c "source '${THISROOT}' 2>/dev/null; '${ROOT_BIN}' --version 2>&1" || true)
    ROOT_VER=$(echo "${ROOT_VER_LINE}" | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1)
    RMAJOR=$(echo "${ROOT_VER}" | cut -d. -f1)
    RMINOR=$(echo "${ROOT_VER}" | cut -d. -f2)
    if [[ -n "${ROOT_VER}" && ( "${RMAJOR}" -gt 6 || ( "${RMAJOR}" -eq 6 && "${RMINOR}" -ge 34 ) ) ]]; then
        ok "ROOT ${ROOT_VER} (${ROOT_BIN})"
    else
        fail "ROOT version check failed (got '${ROOT_VER}') — need >= 6.34"
    fi
else
    fail "ROOT not found at ${ROOT_BIN} — install from https://root.cern/install/"
fi

# ── Optional tools ───────────────────────────────────────────────────────────
echo ""
echo "Optional tools:"

if command -v clang-format &>/dev/null; then
    VER=$(clang-format --version 2>/dev/null | grep -oE '[0-9]+' | head -1)
    if [[ "${VER}" -ge 18 ]]; then
        ok "clang-format ${VER}"
    else
        warn "clang-format ${VER} < 18  (sudo apt install clang-format-18)"
    fi
else
    warn "clang-format not found  (sudo apt install clang-format-18)"
fi

if command -v clang-tidy &>/dev/null; then
    VER=$(clang-tidy --version 2>/dev/null | grep -oE '[0-9]+' | head -1)
    if [[ "${VER}" -ge 18 ]]; then
        ok "clang-tidy ${VER}"
    else
        warn "clang-tidy ${VER} < 18  (sudo apt install clang-tidy-18)"
    fi
else
    warn "clang-tidy not found  (sudo apt install clang-tidy-18)"
fi

if command -v xxd &>/dev/null; then
    ok "xxd"
elif command -v hexyl &>/dev/null; then
    ok "hexyl"
else
    warn "xxd/hexyl not found  (sudo apt install xxd)"
fi

if python3 -c "import numpy" &>/dev/null 2>&1; then
    ok "Python3 + numpy"
else
    warn "Python3 numpy missing  (sudo apt install python3-numpy)"
fi

# ── Summary ──────────────────────────────────────────────────────────────────
echo ""
echo "============================================"
echo " Required: ${PASS} OK, ${FAIL} FAILED"
echo " Optional: ${WARN} warnings"
echo "============================================"

if [[ ${FAIL} -gt 0 ]]; then
    echo ""
    echo "BLOCKED — fix the following before proceeding:"
    for item in "${REQUIRED_FAILED[@]}"; do
        echo "  * ${item}"
    done
    exit 1
fi

echo "All required tools present — environment OK."
exit 0
