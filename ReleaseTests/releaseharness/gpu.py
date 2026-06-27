"""GPU detection.

A usable GPU normally means ``nvidia-smi`` returns success. On this host
``nvidia-smi`` may instead report a *driver/library version mismatch* while
CUDA compute still works (e.g. the CUDA example 15 runs). That degraded state
is therefore treated as *usable-with-warning*, not absent, so CUDA matrix
entries are not skipped purely because NVML is stale. When there is no GPU at
all, CUDA matrix entries are skipped gracefully rather than failed.
"""

from __future__ import annotations

import shutil
import subprocess


def _nvidia_smi() -> tuple[int, str]:
    try:
        proc = subprocess.run(
            ["nvidia-smi"], capture_output=True, text=True, timeout=10,
        )
        return proc.returncode, (proc.stdout + proc.stderr)
    except (OSError, subprocess.TimeoutExpired):
        return 1, ""


def gpu_available() -> bool:
    """True if a GPU is present and at least usable-with-warning.

    Returns True when nvidia-smi succeeds, OR when it fails specifically with a
    driver/library version mismatch (NVML stale but CUDA compute still works).
    """
    if shutil.which("nvidia-smi") is None:
        return False
    rc, out = _nvidia_smi()
    if rc == 0 and "Driver Version" in out:
        return True
    # Usable-with-warning: NVML mismatch but CUDA compute works.
    return "version mismatch" in out.lower()
