"""Containerfile (Dockerfile) generation for the release-test images.

A single image is produced per guest OS. That image carries BOTH toolchains
(a recent GCC and a recent Clang) plus CMake, Doxygen and, optionally,
OpenMPI, and it builds **Boost 1.91.0 from source in C++20 mode** once with the
system GCC.

Why one image for both compilers: on Linux, GCC and Clang share the same
``libstdc++`` C++ standard library, so a Boost built with GCC links cleanly
against Geneva object code produced by Clang. This is verified by actually
building Geneva with both compilers against the single GCC-built Boost (see the
release matrix). Only if a future Clang genuinely fails to link against the
GCC-built Boost would a per-compiler Boost be warranted.

The distro ``libboost-*-dev`` packages are intentionally NOT used: the Ubuntu
archives ship Boost 1.83/1.88/1.89, all older than the required 1.91 and not
guaranteed to be compiled in C++20 mode.
"""

from __future__ import annotations

from ..model import GuestOS

# Pinned Boost release built from source inside every image.
BOOST_VERSION = "1.91.0"
BOOST_UNDERSCORE = BOOST_VERSION.replace(".", "_")
BOOST_TARBALL = f"boost_{BOOST_UNDERSCORE}.tar.bz2"
BOOST_URL = (
    f"https://archives.boost.io/release/{BOOST_VERSION}/source/{BOOST_TARBALL}"
)

# Where the from-source Boost is installed inside the image.
IMAGE_BOOST_ROOT = "/opt/boost"

# Boost components Geneva links against (Boost.Test header is enough but we
# build the compiled libs to be safe).
BOOST_LIBRARIES = (
    "filesystem,program_options,regex,serialization,test,atomic"
)


def _base_packages(with_mpi: bool) -> str:
    """APT packages common to every image (toolchains + build tooling).

    Both GCC and Clang are installed so the one image serves both compilers.
    No Boost packages here — Boost is built from source below.
    """
    pkgs = [
        "ca-certificates",
        "curl",
        "wget",
        "git",
        "build-essential",   # gcc/g++ + make
        "g++",
        "clang",             # distro clang (>=18 on 24.04, >=21 on 26.04)
        "lld",
        "llvm",
        "libomp-dev",
        "cmake",             # 3.28 on 24.04, 4.x on 26.04 — both >= 3.27
        "ninja-build",
        "catch2",            # Catch2 v3 (test framework; CMake config + lib)
        "doxygen",
        "graphviz",
        "bzip2",
        "xz-utils",
        "python3",
    ]
    if with_mpi:
        pkgs += ["openmpi-bin", "libopenmpi-dev"]
    return " ".join(pkgs)


def containerfile(guest: GuestOS, *, with_mpi: bool = False,
                  boost_root: str | None = None) -> str:
    """Render the Containerfile for one guest OS.

    The image installs both toolchains and builds Boost 1.91 from source with
    GCC in C++20 mode (``b2 cxxstd=20``). Build jobs later mount the read-only
    Geneva source and a writable build dir and run ``prepareBuild.sh``.

    ``boost_root`` is where Boost is installed inside the image (and exported as
    ``BOOST_ROOT``); it MUST match the build-time ``BOOSTROOT`` derived from the
    same config value, so the from-source Boost is actually found. Defaults to
    IMAGE_BOOST_ROOT (``/opt/boost``).
    """
    boost_root = boost_root or IMAGE_BOOST_ROOT
    pkgs = _base_packages(with_mpi)
    return f"""\
FROM {guest.image}

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

# --- Toolchains and build tooling (both GCC and Clang) ---------------------
RUN apt-get update \\
 && apt-get install -y --no-install-recommends {pkgs} \\
 && rm -rf /var/lib/apt/lists/*

# --- Clang compiler-rt sanitizer runtimes (for the Sanitize build type) -----
# The distro `clang` package does not pull in libclang_rt.tsan/asan/etc., so a
# Clang Sanitize build fails at link ("cannot find libclang_rt.tsan-*.a").
# Install the runtime package matching the installed Clang major version.
RUN CLANG_MAJOR="$(clang --version | sed -nE 's/.*version ([0-9]+).*/\\1/p' | head -1)" \\
 && apt-get update \\
 && apt-get install -y --no-install-recommends "libclang-rt-${{CLANG_MAJOR}}-dev" \\
 && rm -rf /var/lib/apt/lists/*

# --- Boost {BOOST_VERSION} from source, C++20 ABI --------------------------------
# Built ONCE with the system GCC; GCC and Clang share libstdc++ on Linux so the
# same Boost serves both compilers. The distro libboost packages (1.83/1.88/
# 1.89) are deliberately NOT installed: Geneva requires Boost >= 1.91 in C++20.
RUN cd /tmp \\
 && curl -fsSL -o {BOOST_TARBALL} "{BOOST_URL}" \\
 && tar xf {BOOST_TARBALL} \\
 && cd boost_{BOOST_UNDERSCORE} \\
 && ./bootstrap.sh --prefix={boost_root} \\
        --with-libraries={BOOST_LIBRARIES} \\
 && ./b2 -j"$(nproc)" \\
        cxxstd=20 \\
        variant=release \\
        link=shared \\
        threading=multi \\
        runtime-link=shared \\
        install \\
 && cd / \\
 && rm -rf /tmp/boost_{BOOST_UNDERSCORE} /tmp/{BOOST_TARBALL}

ENV BOOST_ROOT={boost_root}
ENV LD_LIBRARY_PATH={boost_root}/lib:$LD_LIBRARY_PATH

WORKDIR /work
"""
