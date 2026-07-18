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
#
# Standalone build for the GStarter example.
#
# This directory ships with the Geneva sources but is NOT compiled by the
# in-tree Geneva build. It is the canonical template showing how an external
# project consumes an *installed* Geneva. To build it you first need to install
# Geneva (see the project's CLAUDE.md / INSTALL notes), then point this project
# at that installation:
#
#   mkdir build && cd build
#   cmake -DCMAKE_PREFIX_PATH=<geneva-install-prefix> ..
#   make
#
# Geneva is located via find_package(Geneva). The preferred mechanism is the
# installed CMake config package (find_package(Geneva CONFIG)). If that is not
# available (an older install without the config package) the build falls back
# to the bundled FindGeneva.cmake module. In both cases the install prefix is
# supplied through CMAKE_PREFIX_PATH or the GENEVA_ROOT variable, e.g.
#
#   cmake -DGENEVA_ROOT=<geneva-install-prefix> ..
#
################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.27)

PROJECT(GStarter CXX)

# Allow GENEVA_ROOT to be used interchangeably with CMAKE_PREFIX_PATH so users
# can point at the installation either way.
IF (DEFINED GENEVA_ROOT)
	LIST(APPEND CMAKE_PREFIX_PATH "${GENEVA_ROOT}")
ENDIF ()

# Locate the bundled FindGeneva.cmake (in the repo-root CMakeModules) so the
# MODULE-mode fallback below can find it.
LIST(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../../../CMakeModules")

# Prefer the installed CMake config package (Geneva::* imported targets). Fall
# back to the FindGeneva.cmake module if no config package is present.
FIND_PACKAGE(Geneva CONFIG QUIET)
IF (NOT Geneva_FOUND)
	MESSAGE(STATUS "Geneva config package not found, falling back to FindGeneva.cmake (MODULE mode)")
	FIND_PACKAGE(Geneva MODULE REQUIRED)
ENDIF ()

################################################################################
# The GStarter executable.
#
# Linking Geneva::geneva transitively pulls in courtier/hap/common, the Geneva
# include directories, Boost, the C++23 requirement and (when the install was
# built with it) the MPI consumer dependencies.

# Package the GStarter individual BOTH ways from one source (the DA1b "build both" model), using the helper
# the installed Geneva CMake package exports -- find_package(Geneva) above brought GENEVA_DECLARE_INDIVIDUAL
# into scope, so this standalone/out-of-tree example authors its loadable module with the exact same one-line
# declaration the in-tree examples use. It produces a compiled-in object library (GStarterIndividual-obj) and
# a runtime-loadable module (libGStarterIndividual.so). The individual's serialization export lives in
# GStarterIndividual.cpp (SOURCES, in both targets); the module's fixed entry point is
# GStarterIndividualPlugin.cpp (PLUGIN, module only).
GENEVA_DECLARE_INDIVIDUAL(GStarterIndividual
	MODE    both
	SOURCES GStarterIndividual.cpp
	PLUGIN  GStarterIndividualPlugin.cpp
	CONFIG  ./config/GStarterIndividual.json)

# (1) The COMPILED-IN variant: the classic bespoke GStarter driver with the individual linked in. The object
# library is listed BEFORE Geneva::geneva so the linker keeps its Boost.Serialization instantiations (see the
# link-order note in GENEVA_DECLARE_INDIVIDUAL).
ADD_EXECUTABLE(GStarter GStarter.cpp)
TARGET_LINK_LIBRARIES(GStarter PRIVATE GStarterIndividual-obj Geneva::geneva)
TARGET_COMPILE_FEATURES(GStarter PRIVATE cxx_std_23)
INSTALL(TARGETS GStarter DESTINATION bin)

# (2) The LOADABLE variant's runner: a generic optimizer with NO problem compiled in, which loads the
# just-built module at runtime via --individual (the example-19 pattern). ENABLE_EXPORTS so a plugin loaded
# RTLD_GLOBAL resolves against one symbol namespace (single Boost.Serialization registry + Geneva singletons).
ADD_EXECUTABLE(GStarterGenericOptimizer GStarterGenericOptimizer.cpp)
SET_TARGET_PROPERTIES(GStarterGenericOptimizer PROPERTIES ENABLE_EXPORTS TRUE)
TARGET_LINK_LIBRARIES(GStarterGenericOptimizer PRIVATE Geneva::geneva)
TARGET_COMPILE_FEATURES(GStarterGenericOptimizer PRIVATE cxx_std_23)
ADD_DEPENDENCIES(GStarterGenericOptimizer GStarterIndividual)
INSTALL(TARGETS GStarterGenericOptimizer GStarterIndividual DESTINATION bin)

################################################################################
# Materialize the configuration files from code defaults at build time, so the
# example can be run directly from the build tree (GStarter expects
# ./config/*.json). find_package(Geneva) above exported the same
# GENEVA_MATERIALIZE_CONFIGS mechanism the in-tree examples use: after GStarter
# is built it runs once with --update-configs, which (re)creates every config
# file the example reads under <build>/config from the code's registered
# defaults. No hand-maintained config files are shipped.

GENEVA_MATERIALIZE_CONFIGS(GStarter bin/config)

################################################################################
# Optional unit test.
#
# The unit test uses Catch2 and the internal Geneva test scaffolding header
# Geneva_tests.hpp, which is part of the Geneva *sources* (not installed). The
# test target is therefore only built when both Catch2 and that header are
# available. A minimal Geneva install (without testing) still builds the
# GStarter executable above.

FIND_PACKAGE(Catch2 3 QUIET)

# Locate the internal test scaffolding header. By default it is taken from the
# in-tree location relative to this directory; a different Geneva source tree
# can be supplied via GENEVA_TEST_INCLUDE_DIR.
SET(GENEVA_TEST_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../../../tests/geneva/UnitTests"
	CACHE PATH "Directory holding the internal Geneva_tests.hpp test scaffolding header")

IF (Catch2_FOUND AND EXISTS "${GENEVA_TEST_INCLUDE_DIR}/Geneva_tests.hpp")
	MESSAGE(STATUS "Catch2 and Geneva test scaffolding found - building GStarterIndividualUnitTests")

	SET(TESTNAME GStarterIndividualUnitTests)

	ADD_EXECUTABLE(${TESTNAME}
		Tests/UnitTests/GStarterIndividualUnitTests.cpp
		GStarterIndividual.cpp
	)

	TARGET_LINK_LIBRARIES(${TESTNAME} PRIVATE
		Geneva::geneva
		Catch2::Catch2WithMain
	)

	TARGET_COMPILE_FEATURES(${TESTNAME} PRIVATE cxx_std_23)

	# The Geneva testing code (modify_GUnitTests / specificTests*) is compiled in
	# only when GEM_TESTING is defined; the in-tree build sets this globally via
	# CommonGenevaBuild.cmake when GENEVA_BUILD_TESTS is on.
	TARGET_COMPILE_DEFINITIONS(${TESTNAME} PRIVATE GEM_TESTING)

	TARGET_INCLUDE_DIRECTORIES(${TESTNAME} PRIVATE "${GENEVA_TEST_INCLUDE_DIR}")

	ENABLE_TESTING()
	ADD_TEST(NAME ${TESTNAME} COMMAND ${TESTNAME})
ELSE ()
	MESSAGE(STATUS "Catch2 and/or Geneva test scaffolding not found - skipping GStarterIndividualUnitTests")
ENDIF ()
