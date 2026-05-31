"""Backend abstraction.

A backend provides an isolated guest environment (container or VM) in which a
Geneva build + test sequence runs. Concrete backends implement image/instance
provisioning and command execution; the rest of the harness only talks to this
interface.
"""

from __future__ import annotations

import abc
import subprocess
from dataclasses import dataclass
from pathlib import Path

from ..model import BuildSpec, GuestOS


@dataclass
class Mount:
    """A host path mounted into the guest."""

    host: Path
    guest: str
    read_only: bool = False


@dataclass
class CommandResult:
    argv: list[str]
    returncode: int
    stdout: str
    stderr: str

    @property
    def ok(self) -> bool:
        return self.returncode == 0


class ContainerBackend(abc.ABC):
    """Abstract guest backend.

    The harness drives backends in two phases: ``build_image`` (or, for VMs,
    launch+provision) prepares a reusable environment per (OS, compiler); then
    ``run`` executes a command sequence inside it with the given mounts.
    """

    name: str = "abstract"

    def __init__(self, *, dry_run: bool = False, gpu_available: bool = False) -> None:
        self.dry_run = dry_run
        self.gpu_available = gpu_available

    # -- availability -----------------------------------------------------
    @abc.abstractmethod
    def is_available(self) -> bool:
        """Return True if this backend can be used on the current host."""

    # -- provisioning -----------------------------------------------------
    @abc.abstractmethod
    def image_tag(self, guest: GuestOS, spec: BuildSpec) -> str:
        """Stable image/instance identifier for a (guest, spec) combination."""

    @abc.abstractmethod
    def build_image(self, guest: GuestOS, spec: BuildSpec, *, images_dir: Path,
                    boost_root: str | None = None) -> CommandResult:
        """Build/prepare the guest environment. Idempotent where possible.

        ``boost_root`` overrides where Boost is installed inside the image; it
        must match the build-time BOOSTROOT. ``None`` uses the image default."""

    # -- execution --------------------------------------------------------
    @abc.abstractmethod
    def run(
        self,
        guest: GuestOS,
        spec: BuildSpec,
        argv: list[str],
        *,
        mounts: list[Mount],
        workdir: str,
        use_gpu: bool = False,
        timeout: int | None = None,
    ) -> CommandResult:
        """Run ``argv`` inside the guest and capture its output."""

    # -- teardown ---------------------------------------------------------
    def cleanup(self) -> None:  # pragma: no cover - backend specific
        """Remove any transient instances. Default: nothing to do."""

    # -- shared helper ----------------------------------------------------
    def _exec(self, argv: list[str], *, timeout: int | None = None) -> CommandResult:
        """Run a host-side command (the backend CLI). Honors dry-run."""
        if self.dry_run:
            return CommandResult(argv=argv, returncode=0,
                                 stdout="[dry-run] not executed", stderr="")
        try:
            proc = subprocess.run(
                argv,
                capture_output=True,
                text=True,
                timeout=timeout,
            )
            return CommandResult(argv=argv, returncode=proc.returncode,
                                 stdout=proc.stdout, stderr=proc.stderr)
        except FileNotFoundError as exc:
            return CommandResult(argv=argv, returncode=127, stdout="", stderr=str(exc))
        except subprocess.TimeoutExpired as exc:
            return CommandResult(argv=argv, returncode=124, stdout=exc.stdout or "",
                                 stderr=f"timeout after {timeout}s")
