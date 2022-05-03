#!/bin/bash

#
# This file is copied inside of the container and used to execute the genva build.
# You may adjust the file in case you want to build a different setup
#

# configure build of geneva
cmake3 \
-DCMAKE_BUILD_TYPE=Debug \
-DCMAKE_MAKE_PROGRAM=ninja \
-DGENEVA_BUILD_TYPE=Debug \
-DGENEVA_BUILD_TESTS=1 \
-DGENEVA_BUILD_EXAMPLES=1 \
-DGENEVA_BUILD_BENCHMARKS=1 \
-DGENEVA_STATIC=0 \
-DCMAKE_VERBOSE_MAKEFILE=0 \
-DCMAKE_INSTALL_PREFIX=/opt/geneva \
-DGENEVA_BUILD_WITH_OPENCL_EXAMPLES=0 \
-DGENEVA_BUILD_WITH_MPI_CONSUMER=1 \
-G Ninja \
-S ./geneva \
-B ./geneva/docker/docker-geneva-build \

# build geneva into the docker-geneva-build directory, which is inside the shared volume.
# The built binaries will therefore also be availible on the host system.
cmake3 \
--build ./geneva/docker/docker-geneva-build \
--target all \
--parallel 2