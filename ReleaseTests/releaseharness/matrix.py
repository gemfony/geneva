"""Expand the configured axes into concrete jobs and apply filters."""

from __future__ import annotations

from .config import Config, FeatureSet
from .model import BuildSpec, BuildType, Compiler, Job


def _spec(cfg: Config, compiler: Compiler, build_type: BuildType,
          fs: FeatureSet, gpu_available: bool) -> BuildSpec:
    # CUDA is only enabled when the feature set wants it *and* a GPU exists.
    want_cuda = fs.cuda and gpu_available
    return BuildSpec(
        build_type=build_type,
        compiler=compiler,
        build_tests=fs.tests,
        build_examples=fs.examples,
        build_benchmarks=fs.benchmarks,
        with_mpi=fs.mpi,
        with_cuda_rng=want_cuda,
        skip_all_cuda=not want_cuda,
        boost_root=cfg.boost_root,
    )


def expand(cfg: Config, *, full: bool, gpu_available: bool) -> list[Job]:
    """Build the full job list for the requested tier.

    ``--quick`` (full=False) uses only ``short_build_types`` and the ``base``
    feature set. ``--full`` adds the long build types, the ``minimal`` build
    and the ``full`` (MPI/CUDA/benchmarks) feature set.
    """
    jobs: list[Job] = []
    base_fs = cfg.feature_sets["base"]

    build_types = list(cfg.short_build_types)
    if full:
        build_types += [b for b in cfg.long_build_types if b not in build_types]

    for guest in cfg.guests:
        for compiler in cfg.compilers:
            for bt in build_types:
                jobs.append(Job(guest=guest,
                                 spec=_spec(cfg, compiler, bt, base_fs, gpu_available),
                                 label="base"))

            if full:
                # Minimal build (all options off) — Debug only is enough.
                mini = cfg.feature_sets.get("minimal")
                if mini is not None:
                    jobs.append(Job(
                        guest=guest,
                        spec=_spec(cfg, compiler, BuildType.DEBUG, mini, gpu_available),
                        label="minimal"))
                # Full-feature build (MPI + benchmarks, CUDA if GPU) — Release.
                full_fs = cfg.feature_sets.get("full")
                if full_fs is not None:
                    jobs.append(Job(
                        guest=guest,
                        spec=_spec(cfg, compiler, BuildType.RELEASE, full_fs, gpu_available),
                        label="full"))

    return jobs


def _matches(job: Job, selectors: dict[str, str]) -> bool:
    for key, val in selectors.items():
        if key == "os" and job.guest.slug != val:
            return False
        if key == "compiler" and job.spec.compiler.value != val:
            return False
        if key == "buildtype" and job.spec.build_type.value != val:
            return False
        if key == "label" and job.label != val:
            return False
    return True


def apply_filter(jobs: list[Job], filter_str: str | None) -> list[Job]:
    """Filter jobs by a ``key=value,key=value`` selector string.

    Supported keys: ``os``, ``compiler``, ``buildtype``, ``label``.
    """
    if not filter_str:
        return jobs
    selectors: dict[str, str] = {}
    for part in filter_str.split(","):
        part = part.strip()
        if not part:
            continue
        if "=" not in part:
            raise ValueError(f"bad filter token {part!r}; expected key=value")
        key, val = part.split("=", 1)
        selectors[key.strip()] = val.strip()
    return [j for j in jobs if _matches(j, selectors)]
