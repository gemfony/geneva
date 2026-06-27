"""Geneva release-test harness package.

Drives the checks listed in ReleaseTestplan.md across a matrix of guest
operating systems, compilers and build types, inside containers (or VMs),
and produces a pass/fail report.
"""

__all__ = ["config", "matrix", "model", "report", "logging_util"]
