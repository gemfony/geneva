"""Backend selection. Podman is the only supported backend (rootless, no
daemon, no group membership needed)."""

from __future__ import annotations

from .base import ContainerBackend
from .podman import PodmanBackend

_BACKENDS: dict[str, type[ContainerBackend]] = {
    "podman": PodmanBackend,
}


def get_backend(name: str, *, dry_run: bool, gpu_available: bool) -> ContainerBackend:
    """Instantiate the backend. Only Podman is supported; ``'auto'`` resolves
    to Podman. Any other name is rejected with a clear error."""
    if name not in ("podman", "auto"):
        raise ValueError(
            f"unknown backend {name!r}; only 'podman' (or 'auto') is supported"
        )
    return PodmanBackend(dry_run=dry_run, gpu_available=gpu_available)


def available_backends(*, gpu_available: bool = False) -> dict[str, bool]:
    """Probe the backend and report availability (for diagnostics)."""
    try:
        ok = PodmanBackend(dry_run=False, gpu_available=gpu_available).is_available()
    except Exception:  # pragma: no cover - defensive
        ok = False
    return {"podman": ok}
