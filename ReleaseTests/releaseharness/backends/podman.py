"""Podman backend (the default).

Podman is OCI-compatible and accepts the same CLI as Docker, so this is a thin
subclass overriding only the executable name and the GPU flag: Podman uses the
CDI device syntax (``--device nvidia.com/gpu=all``) rather than ``--gpus``.

Podman runs rootless here (no daemon, no docker group needed), which is why it
is the harness default. The single per-OS image (built once, both toolchains,
Boost 1.91 from source) is shared between GCC and Clang Geneva builds.
"""

from __future__ import annotations

from .docker import DockerBackend


class PodmanBackend(DockerBackend):
    name = "podman"
    executable = "podman"

    def _gpu_flags(self, use_gpu: bool) -> list[str]:
        # Requires a generated CDI spec on the host:
        #   sudo nvidia-ctk cdi generate --output=/etc/cdi/nvidia.yaml
        if use_gpu and self.gpu_available:
            return ["--device", "nvidia.com/gpu=all"]
        return []
