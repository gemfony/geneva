# syntax=docker/dockerfile:1.7
################################################################################
# Geneva build container
#
# Base:  nvidia/cuda:12.6-devel-ubuntu24.04
# GPU:   RTX 4090  (CUDA compute capability 8.9)
# Boost: 1.87.0  (built from source; Ubuntu 24.04 ships 1.83, Geneva needs 1.90+)
#
# Host requirements
# -----------------
#   nvidia-container-toolkit installed and configured
#   NVIDIA driver >= 535
#
# Build image
# -----------
#   docker build -t geneva-build:latest docker/
#
# Run (use the helper script)
# ---------------------------
#   docker/run.sh [cmake|make|bash|...]
################################################################################

FROM nvidia/cuda:12.6-devel-ubuntu24.04

ARG BOOST_VERSION=1.87.0
ARG BOOST_TAG=1_87_0
ARG DEBIAN_FRONTEND=noninteractive

# ── System packages ────────────────────────────────────────────────────────────
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        gcc-13 \
        g++-13 \
        cmake \
        ninja-build \
        git \
        wget \
        ca-certificates \
        python3 \
        catch2 \
        libopenmpi-dev \
        openmpi-bin \
        doxygen \
        libssl-dev \
        zlib1g-dev \
        libbz2-dev \
        liblzma-dev \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100 \
    && update-alternatives --install /usr/bin/cc  cc  /usr/bin/gcc-13 100 \
    && update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-13 100 \
    && rm -rf /var/lib/apt/lists/*

# ── Boost from source ──────────────────────────────────────────────────────────
# Ubuntu 24.04 ships Boost 1.83; Geneva requires >= 1.90.
RUN wget -q \
        "https://archives.boost.io/release/${BOOST_VERSION}/source/boost_${BOOST_TAG}.tar.gz" \
    && tar xf "boost_${BOOST_TAG}.tar.gz" \
    && cd "boost_${BOOST_TAG}" \
    && ./bootstrap.sh \
        --prefix=/opt/boost \
        --with-libraries=atomic,chrono,date_time,filesystem,iostreams,program_options,regex,serialization,system,test,thread \
    && ./b2 \
        -j"$(nproc)" \
        cxxflags="-std=c++20" \
        variant=release \
        link=shared \
        install \
    && cd / \
    && rm -rf "boost_${BOOST_TAG}" "boost_${BOOST_TAG}.tar.gz"

# Make Boost visible to the dynamic linker and pkg-config
RUN echo "/opt/boost/lib" > /etc/ld.so.conf.d/boost.conf && ldconfig

ENV BOOST_ROOT=/opt/boost
ENV LD_LIBRARY_PATH=/opt/boost/lib${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}

# ── Default working directory ──────────────────────────────────────────────────
WORKDIR /workspace