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

# This example has TWO artifacts: a runtime-loadable individual (the problem, built as a shared object)
# and a GENERIC optimizer that loads it via --individual and has no problem compiled in.

SET(LIBRARIES_TO_LINK
		${GENEVA_LIBRARIES}
		${Boost_LIBRARIES})

IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
	SET(LIBRARIES_TO_LINK ${LIBRARIES_TO_LINK} ${MPI_LIBRARIES})
ENDIF()

# (1) The loadable individual plugin -> libGLoadableParaboloid.so. It carries the problem code + its
# serialization GUID + the plugin entry points. A plugin resolves Geneva's symbols from the HOST PROCESS at
# load time (the loader uses RTLD_GLOBAL), so it links none of the Geneva libraries -- it only needs their
# HEADERS at compile time. This also sidesteps the non-PIC static Catch2 that a testing-enabled Geneva
# pulls in as a usage requirement, which cannot be linked into a shared object.
ADD_LIBRARY(GLoadableParaboloid MODULE GLoadableParaboloidPlugin.cpp)
TARGET_INCLUDE_DIRECTORIES(GLoadableParaboloid PRIVATE
	${PROJECT_SOURCE_DIR}/common/include
	${PROJECT_SOURCE_DIR}/hap/include
	${PROJECT_SOURCE_DIR}/courtier/include
	${PROJECT_SOURCE_DIR}/dietrich/include
	${PROJECT_SOURCE_DIR}/geneva/include
	${Boost_INCLUDE_DIRS})
# MODULE libraries drop the "lib" prefix by default; keep it so the .so is named libGLoadableParaboloid.so.
SET_TARGET_PROPERTIES(GLoadableParaboloid PROPERTIES PREFIX "lib")

# (2) The generic optimizer -> loads a problem at runtime. ENABLE_EXPORTS so a plugin loaded RTLD_GLOBAL
# resolves against one symbol namespace (single Boost.Serialization registry + single Geneva singletons).
ADD_EXECUTABLE(GGenericOptimizer GGenericOptimizer.cpp)
SET_TARGET_PROPERTIES(GGenericOptimizer PROPERTIES ENABLE_EXPORTS TRUE)
TARGET_LINK_LIBRARIES(GGenericOptimizer ${LIBRARIES_TO_LINK})

# Materialize the configuration from code defaults at build time. This plugin loader only knows its full
# configuration once an individual is loaded, so materialize with the just-built plugin (and depend on it so
# it exists first).
ADD_DEPENDENCIES(GGenericOptimizer GLoadableParaboloid)
GENEVA_MATERIALIZE_CONFIGS(GGenericOptimizer
	${INSTALL_PREFIX_DATA}/examples/GLoadableIndividual/config
	--individual $<TARGET_FILE:GLoadableParaboloid>)

# End-to-end test: the generic optimizer loads the just-built individual plugin and optimizes it. This is
# also the empirical check that the plugin and process share ONE set of singletons / one Boost registry.
IF(GENEVA_RUN_EXAMPLES_AS_TESTS)
	ADD_TEST(
		NAME GLoadableIndividual
		COMMAND GGenericOptimizer --individual $<TARGET_FILE:GLoadableParaboloid>
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
ENDIF()

INSTALL(TARGETS GGenericOptimizer GLoadableParaboloid
	DESTINATION ${INSTALL_PREFIX_DATA}/examples/GLoadableIndividual)
