"""Docker/OCI backend (shared implementation; Podman subclasses this).

Builds ONE image per guest OS. Each image carries both toolchains (a recent
GCC and Clang), CMake, Doxygen and, optionally, OpenMPI, and builds Boost
1.91.0 from source in C++20 mode (see ``containerfile.py``). Build/test
commands then run in ephemeral containers with the Geneva source tree bind-
mounted read-only and a writable per-job build directory mounted from the work
area.

The image is keyed on (guest OS, MPI on/off) only — NOT on the compiler —
because the single GCC-built Boost serves both GCC and Clang builds of Geneva.
"""

from __future__ import annotations

import shutil
from pathlib import Path

from ..model import BuildSpec, GuestOS
from .base import CommandResult, ContainerBackend, Mount
from .containerfile import containerfile


class DockerBackend(ContainerBackend):
    name = "docker"
    executable = "docker"

    def is_available(self) -> bool:
        if shutil.which(self.executable) is None:
            return False
        # `info` fails fast if the daemon/engine is not reachable or access is
        # denied. Diagnostics in diagnostics.py classify the failure mode.
        res = self._exec([self.executable, "info"], timeout=20)
        return res.ok

    def image_tag(self, guest: GuestOS, spec: BuildSpec) -> str:
        # One image per OS (and MPI variant); shared across compilers.
        return f"geneva-rt/{guest.slug}{'-mpi' if spec.with_mpi else ''}"

    def build_image(self, guest: GuestOS, spec: BuildSpec, *, images_dir: Path) -> CommandResult:
        tag = self.image_tag(guest, spec)
        body = containerfile(guest, with_mpi=spec.with_mpi)
        df_path = images_dir / f"Containerfile.{guest.slug}{'-mpi' if spec.with_mpi else ''}"
        if not self.dry_run:
            df_path.write_text(body, encoding="utf-8")
        argv = [self.executable, "build", "-t", tag, "-f", str(df_path), str(images_dir)]
        if self.dry_run:
            res = self._exec(argv)
            res.stdout = f"[dry-run] would write {df_path} and build {tag}\n{body}"
            return res
        # Building Boost from source can take a while; allow up to an hour.
        return self._exec(argv, timeout=3600)

    def _gpu_flags(self, use_gpu: bool) -> list[str]:
        # Docker: requires nvidia-container-toolkit on the host.
        if use_gpu and self.gpu_available:
            return ["--gpus", "all"]
        return []

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
        tag = self.image_tag(guest, spec)
        cmd: list[str] = [self.executable, "run", "--rm", "-w", workdir]
        cmd += self._gpu_flags(use_gpu)
        for m in mounts:
            opt = "ro" if m.read_only else "rw"
            cmd += ["-v", f"{m.host}:{m.guest}:{opt}"]
        cmd.append(tag)
        cmd += argv
        return self._exec(cmd, timeout=timeout)
