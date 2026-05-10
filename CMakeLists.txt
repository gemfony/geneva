################################################################################
#
# This file is part of the Geneva library collection. The following license
# applies to this file:
#
# ------------------------------------------------------------------------------
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ------------------------------------------------------------------------------
#
# Note that other files in the Geneva library collection may use a different
# license. Please see the licensing information in each file.
#
################################################################################
#
# See the NOTICE file in the top-level directory of the Geneva library
# collection for a list of contributors and copyright information.
#
################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.27 FATAL_ERROR)

MESSAGE ("")

PROJECT (Geneva_Library_Collection LANGUAGES CXX)

# Set ON to skip all CUDA compilation regardless of whether nvcc is present.
# Overrides GENEVA_USE_CUDA_RNG, CUDA_NVCC, and CUDA_ROOT.  Useful when a
# system CUDA installation is broken or incompatible (e.g. CUDA 13.1 + glibc 2.39).
option(GENEVA_SKIP_CUDA "Disable all CUDA compilation" OFF)

# Probe for an optional CUDA toolchain.  If nvcc is found, CUDA examples and
# benchmarks are added to the build automatically; if not, the rest of the
# build proceeds as pure C++.  GENEVA_USE_CUDA_RNG remains a separate opt-in
# flag (see genevaConfig.gcfg) because enabling it makes the Hap library
# depend on the CUDA runtime, reducing portability of programs linked with it.

if(NOT GENEVA_SKIP_CUDA)
    # If only CMAKE_CUDA_COMPILER was supplied (e.g. by prepareBuild.sh), derive
    # CUDAToolkit_ROOT from it so find_package(CUDAToolkit) locates the matching
    # headers and libraries.  Assumes the standard NVIDIA layout: <root>/bin/nvcc.
    if(DEFINED CMAKE_CUDA_COMPILER AND NOT DEFINED CUDAToolkit_ROOT)
        get_filename_component(_geneva_cuda_bindir "${CMAKE_CUDA_COMPILER}" DIRECTORY)
        get_filename_component(_geneva_cuda_root   "${_geneva_cuda_bindir}"  DIRECTORY)
        set(CUDAToolkit_ROOT "${_geneva_cuda_root}"
            CACHE PATH "CUDA toolkit root (derived from CMAKE_CUDA_COMPILER)")
        unset(_geneva_cuda_bindir)
        unset(_geneva_cuda_root)
    endif()

    include(CheckLanguage)
    check_language(CUDA)
    if(CMAKE_CUDA_COMPILER)
        enable_language(CUDA)
        find_package(CUDAToolkit REQUIRED)
        message(STATUS "CUDA ${CMAKE_CUDA_COMPILER_VERSION} detected — CUDA examples and benchmarks will be built")
        message(STATUS "  Toolkit root: ${CUDAToolkit_ROOT}")

        # Suppress known Boost-internal false positives from NVCC.
        # NVCC does not honour -isystem suppression the way GCC does, so these
        # diagnostics fire on every Boost TU that pulls in mpl, mp11, fusion,
        # or the Boost.Archive serialization headers:
        #   68    – integer conversion sign change   (boost/mpl/print.hpp)
        #   186   – unsigned comparison with zero    (boost/mp11/detail/mp_count.hpp)
        #   191   – meaningless cast qualifier       (boost/archive/detail/iserializer.hpp)
        #   20012 – __host__/__device__ annotation ignored on defaulted function
        #           (boost/fusion/container/vector/vector.hpp, boost/fusion/tuple/tuple.hpp,
        #            boost/fusion/support/unused.hpp)
        add_compile_options($<$<COMPILE_LANGUAGE:CUDA>:-diag-suppress=68>
                            $<$<COMPILE_LANGUAGE:CUDA>:-diag-suppress=186>
                            $<$<COMPILE_LANGUAGE:CUDA>:-diag-suppress=191>
                            $<$<COMPILE_LANGUAGE:CUDA>:-diag-suppress=20012>)

        # Detect the CXX compiler's libstdc++.so location and cache it.
        # CUDA targets link it by full path to prevent the CUDA toolkit's bundled
        # (older) GCC from injecting an incompatible libstdc++ into the link step,
        # which would cause "undefined reference to __cxa_*@CXXABI_X.Y" failures.
        if(CMAKE_CXX_COMPILER)
            execute_process(
                COMMAND "${CMAKE_CXX_COMPILER}" -print-file-name=libstdc++.so
                OUTPUT_VARIABLE _cxx_libstdcxx_path
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            if(_cxx_libstdcxx_path AND NOT _cxx_libstdcxx_path STREQUAL "libstdc++.so")
                get_filename_component(_cxx_lib_dir "${_cxx_libstdcxx_path}" DIRECTORY)
                set(GENEVA_CXX_RUNTIME_LIB_DIR "${_cxx_lib_dir}" CACHE INTERNAL "CXX runtime lib dir for CUDA link fix")
            endif()
            unset(_cxx_libstdcxx_path)
            unset(_cxx_lib_dir)
        endif()
    else()
        message(STATUS "CUDA not found — building without CUDA examples and benchmarks")
    endif()
else()
    message(STATUS "CUDA disabled (GENEVA_SKIP_CUDA=ON) — skipping all CUDA compilation")
    if(GENEVA_USE_CUDA_RNG)
        message(WARNING "GENEVA_USE_CUDA_RNG is ON but GENEVA_SKIP_CUDA=ON overrides it")
    endif()
    set(GENEVA_USE_CUDA_RNG OFF CACHE BOOL "Use CUDA for RNG in Hap" FORCE)
endif()

################################################################################
# We create some examples and test programs as part of the build process.
# This variable allows their build-scripts to detect that they are being
# called from within the master build-script. This prevents redundant searches
# and checks, while still allowing examples to be built independently of the
# main Geneva source tree.

SET(GENEVA_FULL_TREE_BUILD TRUE)

################################################################################
# Set these variables to make all examples and/or benchmarks run as part of the
# testing with CTest.

# NOTE: Not yet functional

SET(GENEVA_RUN_EXAMPLES_AS_TESTS FALSE)
SET(GENEVA_RUN_BENCHMARKS_AS_TESTS FALSE)

################################################################################
# Different subcomponents of the Geneva binary tree can be installed in
# customized locations, by setting the "INSTALL_PREFIX_*" variables:
#   INSTALL_PREFIX_INCLUDES -> installation folder for the header files
#   INSTALL_PREFIX_LIBS     -> installation folder for the libraries
#   INSTALL_PREFIX_DOCS     -> installation folder for the documentation files
#   INSTALL_PREFIX_DATA     -> installation folder for other files like
#                              examples, tests and other scripts
# If none of these variables is set, the whole Geneva binary tree is installed
# (as expected) in "CMAKE_INSTALL_PREFIX". The default value for this last
# variable is set here, and further handled in the 'CommonGenevaBuild' module.
# Generally, this can be provided through a custom genevaConfig.gcfg, in
# conjunction with the prepareBuild.sh script.

IF( NOT CMAKE_INSTALL_PREFIX )
	SET( INSTALL_PREFIX_ROOT "/opt/geneva-${GENEVA_VERSION}" )
ENDIF()

################################################################################
# Make sure we first check our local modules directory

SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")

################################################################################
# Set preprocessor define to use in source code if building with MPI consumer
IF (GENEVA_BUILD_WITH_MPI_CONSUMER)
	ADD_COMPILE_DEFINITIONS(GENEVA_BUILD_WITH_MPI_CONSUMER)
ENDIF ()

###############################################################################
# Include the shared functionality module for any Geneva build

INCLUDE(CommonGenevaBuild)

###############################################################################
# Set the library/package version. Uneven minor numbers denote development
# versions while even minor numbers stand for production releases.
#
# IMPORTANT: When bumping the version here, also update the two macros
#   GENEVA_VERSION  and  MIN_BOOST_VERSION
# in include/common/GGlobalDefines.hpp accordingly.

SET(VERSION_MAJOR "1")
SET(VERSION_MINOR "12")
SET(VERSION_PATCH "0")

SET(GENEVA_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

################################################################################
# Include directories, libraries

INCLUDE_DIRECTORIES (
	${PROJECT_SOURCE_DIR}/include
)

# Custom targets for building single libraries only
ADD_CUSTOM_TARGET( "common"             DEPENDS ${COMMON_LIBNAME}            COMMENT "Building only the Common library." )
ADD_CUSTOM_TARGET( "hap"                DEPENDS ${HAP_LIBNAME}               COMMENT "Building only the Hap library." )
ADD_CUSTOM_TARGET( "courtier"           DEPENDS ${COURTIER_LIBNAME}          COMMENT "Building only the Courtier library." )
ADD_CUSTOM_TARGET( "geneva"             DEPENDS ${GENEVA_LIBNAME}            COMMENT "Building only the Geneva library." )
ADD_CUSTOM_TARGET( "geneva-individuals" DEPENDS ${GENEVA_INDIVIDUAL_LIBNAME} COMMENT "Building only the Geneva-Individuals library." )

# Custom target for building all the libraries but no examples nor tests
ADD_CUSTOM_TARGET( "core"               DEPENDS ${GENEVA_LIBNAMES}           COMMENT "Building all the Geneva libraries.")

################################################################################
# Add sub-directories

# Accessory files just get copied
ADD_SUBDIRECTORY (CMakeModules)
ADD_SUBDIRECTORY (licenses)
ADD_SUBDIRECTORY (scripts)

# The main code lays here, and the compilation takes place in 'src'
ADD_SUBDIRECTORY (include)
ADD_SUBDIRECTORY (src)

# Examples and tests are compiled at the end
IF( GENEVA_BUILD_TESTS )
	INCLUDE(CTest)
	ADD_SUBDIRECTORY (tests)
ENDIF()

# Examples are also run as tests if enabled with 'GENEVA_RUN_EXAMPLES_AS_TESTS'
IF( GENEVA_BUILD_EXAMPLES )
	ADD_SUBDIRECTORY (examples)
ENDIF()

# Benchmarks are also run as tests if enabled with GENEVA_RUN_BENCHMARKS_AS_TESTS
IF( GENEVA_BUILD_BENCHMARKS )
	ADD_SUBDIRECTORY (benchmarks)
ENDIF()

# Holds an incubator facility for new optimization algorithms
# ADD_SUBDIRECTORY (incubator)

################################################################################
# Custom target for all compilable items in the distribution
ADD_CUSTOM_TARGET(
	"gemfony-build-all" DEPENDS
	core
	benchmarks
	examples
	tests
	# incubator
	COMMENT "Build the entire Gemfony library collection"
)

################################################################################
# Custom target for all compilable items in the distribution
ADD_CUSTOM_TARGET(
	"install_gemfony-build-all"
	"${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target install
	DEPENDS gemfony-build-all
	COMMENT "Installing gemfony-build-all"
)

################################################################################
# Additional information files to be copied

SET ( INFORMATIONFILES
	CHANGES
	FAQ
	INSTALL
	LICENSE
	NOTICE
	README
	README.documentation
)

INSTALL ( FILES ${INFORMATIONFILES} DESTINATION ${INSTALL_PREFIX_DOCS} )

################################################################################
# Code for the creation of reference documentation for Geneva
#
FIND_PACKAGE(Doxygen)

IF( DOXYGEN_FOUND )
	CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	ADD_CUSTOM_TARGET (
		doc
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Creation of reference documentation for Geneva with 'make doc'" VERBATIM
	)
	SET(MSG "Reference documentation for Geneva may now be created with 'make doc'")
ELSE()
	SET(MSG "Doxygen not found. No support for the creation of reference documentation")
ENDIF()
MESSAGE("${MSG}\n")

################################################################################
# Code for package generation with CPack. This is experimental and the packages
# don't satisfy the Linux FHS yet. Use with care.
#
# RPM or DEB packages can be created using umask 022 && fakeroot make package.

INCLUDE(InstallRequiredSystemLibraries)
SET(CPACK_PACKAGE_NAME "geneva-opt")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The Geneva optimization library")

# The package description. Indentation is important!
SET(CPACK_AUX_PACKAGE_DESCRIPTION
 "This is the Geneva optimization library. As the name suggests --it stands for
 'Grid-enabled evolutionary algorithms'--, Geneva aims to provide the necessary
 tools to perform parametric optimization in parallel on devices ranging from
 multi-processor machines over clusters all the way to Grids and Cloud installations.
 Geneva does not want to be the fastest library of evolutionary algorithms on
 single-processor machines, but is instead targeted at large scale problems,
 where the evaluation of a single candidate solution will typically take longer
 than a few seconds."
)

SET(CPACK_PACKAGE_VENDOR "Gemfony scientific")
SET(CPACK_PACKAGE_CONTACT "Gemfony scientific <contact (AT) gemfony (DOT) eu>")
SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
SET(CPACK_PACKAGING_INSTALL_PREFIX "/opt/geneva")

# DEB specific variables. Indentation is important in the description!
SET(
	CPACK_DEBIAN_PACKAGE_DESCRIPTION
	"${CPACK_PACKAGE_DESCRIPTION_SUMMARY}\n ${CPACK_AUX_PACKAGE_DESCRIPTION}"
)
SET(CPACK_DEBIAN_PACKAGE_SECTION "Science")
SET(CPACK_DEBIAN_PACKAGE_PRIORITY "Extra")
SET(
	CPACK_DEBIAN_PACKAGE_DEPENDS
	"libboost-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0),
	libboost-filesystem-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0),
	libboost-program-options-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0),
	libboost-regex-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0),
	libboost-serialization-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0),
	libboost-system-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0),
	libcatch2-dev (>= 3.0)"
)

# RPM specific variables
SET(CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_AUX_PACKAGE_DESCRIPTION})
SET(CPACK_RPM_PACKAGE_LICENSE "Apache License 2.0")
SET(CPACK_RPM_PACKAGE_GROUP "Sciences/Libraries")
SET(CPACK_RPM_PACKAGE_REQUIRES "boost-devel >= ${GENEVA_MIN_BOOST_VERSION}")

# Detect the distribution to autos-elect the package generator...
EXECUTE_PROCESS(COMMAND lsb_release -is
	OUTPUT_VARIABLE CPACK_AUX_DISTRO
)
IF(CPACK_AUX_DISTRO MATCHES "^Debian" OR CPACK_AUX_DISTRO MATCHES "^Ubuntu")
	SET(CPACK_GENERATOR "DEB")
ELSE(CPACK_AUX_DISTRO MATCHES "^RedHat" OR CPACK_AUX_DISTRO MATCHES "^CentOS" OR
     CPACK_AUX_DISTRO MATCHES "^Scientific" OR CPACK_AUX_DISTRO MATCHES "^SUSE" )
	SET(CPACK_GENERATOR "RPM")
ENDIF()

INCLUDE(CPack)

################################################################################
# Done
