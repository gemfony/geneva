"""Multipass (VM) backend — STUB.

Multipass launches full Ubuntu VMs via QEMU/LXD. It is the fallback for cases
containers cannot cover: kernel-level features, true multi-distro VM isolation,
and (on hosts that support it) GPU passthrough. This backend is a structured
stub: the command flow is documented and wired, but provisioning a VM is a
LONG, side-effecting operation and is left as a TODO for a real release run.

Known limitation on this host: Multipass does not support consumer-GPU
passthrough out of the box on Linux, so CUDA-on-Multipass is treated as
unsupported and skipped by the harness.
"""

from __future__ import annotations

import shutil
from pathlib import Path

from ..model import BuildSpec, GuestOS
from .base import CommandResult, ContainerBackend, Mount


class MultipassBackend(ContainerBackend):
    name = "multipass"
    executable = "multipass"

    # Multipass only ships Ubuntu images; map our slugs to its release names.
    _RELEASE_MAP = {
        "ubuntu-26.04": "26.04",
        "ubuntu-24.04": "24.04",
    }

    def is_available(self) -> bool:
        if shutil.which(self.executable) is None:
            return False
        res = self._exec([self.executable, "version"], timeout=15)
        return res.ok

    def image_tag(self, guest: GuestOS, spec: BuildSpec) -> str:
        # For Multipass this is the VM instance name.
        return f"geneva-rt-{guest.slug}-{spec.compiler.value}".replace(".", "-")

    def build_image(self, guest: GuestOS, spec: BuildSpec, *, images_dir: Path) -> CommandResult:
        instance = self.image_tag(guest, spec)
        release = self._RELEASE_MAP.get(guest.slug, "lts")
        # Intended flow (TODO: implement provisioning + cloud-init):
        #   multipass launch <release> --name <instance> --cpus N --memory 8G --disk 40G
        #   multipass exec <instance> -- sudo apt-get update
        #   multipass exec <instance> -- sudo apt-get install -y <toolchain+cmake+catch2+mpi>
        #   then build Boost 1.91 from source in C++20 mode (NO distro libboost),
        #   mirroring backends/containerfile.py.
        argv = [self.executable, "launch", release, "--name", instance]
        if self.dry_run:
            res = self._exec(argv)
            res.stdout = (f"[dry-run] would launch Multipass VM {instance} "
                          f"from release {release} and provision the toolchain")
            return res
        return CommandResult(
            argv=argv, returncode=70,
            stdout="",
            stderr="MultipassBackend.build_image is a stub; implement VM "
                   "provisioning (launch + cloud-init) before a live run.",
        )

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
        instance = self.image_tag(guest, spec)
        # Intended flow:
        #   multipass mount <host> <instance>:<guest>   (per mount)
        #   multipass exec <instance> --working-directory <workdir> -- <argv>
        full = [self.executable, "exec", instance, "--working-directory", workdir, "--", *argv]
        if self.dry_run:
            res = self._exec(full)
            mount_desc = "; ".join(f"{m.host}->{m.guest}" for m in mounts)
            res.stdout = f"[dry-run] would mount [{mount_desc}] then exec in {instance}"
            return res
        return CommandResult(
            argv=full, returncode=70, stdout="",
            stderr="MultipassBackend.run is a stub; implement mount+exec before a live run.",
        )
