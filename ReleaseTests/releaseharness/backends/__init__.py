"""Container/VM backends for the release-test harness."""

from .base import CommandResult, ContainerBackend, Mount
from .registry import available_backends, get_backend

__all__ = [
    "CommandResult",
    "ContainerBackend",
    "Mount",
    "available_backends",
    "get_backend",
]
