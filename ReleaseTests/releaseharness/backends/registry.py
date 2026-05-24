"""Backend selection and host auto-detection."""

from __future__ import annotations

from .base import ContainerBackend
from .docker import DockerBackend
from .multipass import MultipassBackend
from .podman import PodmanBackend

_BACKENDS: dict[str, type[ContainerBackend]] = {
    "docker": DockerBackend,
    "podman": PodmanBackend,
    "multipass": MultipassBackend,
}

# Auto-detection preference order: Podman first (rootless, no daemon, no group
# membership needed), then Docker, then VMs last.
_AUTO_ORDER = ["podman", "docker", "multipass"]


def get_backend(name: str, *, dry_run: bool, gpu_available: bool) -> ContainerBackend:
    """Instantiate a backend by name, or auto-detect the first available one.

    With ``name == 'auto'`` the first installed-and-usable backend wins. When
    none is available a DockerBackend is still returned (so ``--dry-run`` works
    and ``run`` reports a clear unavailability error).
    """
    if name != "auto":
        if name not in _BACKENDS:
            raise ValueError(
                f"unknown backend {name!r}; choose from {sorted(_BACKENDS)} or 'auto'"
            )
        return _BACKENDS[name](dry_run=dry_run, gpu_available=gpu_available)

    for cand in _AUTO_ORDER:
        backend = _BACKENDS[cand](dry_run=dry_run, gpu_available=gpu_available)
        # In dry-run we still want availability reflected, but we must not block.
        if dry_run or backend.is_available():
            return backend
    # Nothing usable: fall back to Podman (the documented default) so that
    # dry-run works and `run` reports a clear unavailability error.
    return PodmanBackend(dry_run=dry_run, gpu_available=gpu_available)


def available_backends(*, gpu_available: bool = False) -> dict[str, bool]:
    """Probe each backend and report availability (for diagnostics)."""
    out: dict[str, bool] = {}
    for name, cls in _BACKENDS.items():
        try:
            out[name] = cls(dry_run=False, gpu_available=gpu_available).is_available()
        except Exception:  # pragma: no cover - defensive
            out[name] = False
    return out
