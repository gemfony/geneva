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

# Load-from-disk model: the multi-criterion problem is a runtime-loadable module optimized by the generic
# optimizer (example 19's GGenericOptimizer) via --individual, not by a bespoke main.
GENEVA_DECLARE_INDIVIDUAL(GMultiCriterionParabolaIndividual
	MODE    load
	SOURCES GMultiCriterionParabolaIndividual.cpp
	PLUGIN  GMultiCriterionParabolaIndividualPlugin.cpp
	CONFIG  ./config/GMultiCriterionParabolaIndividual.json)

# This problem exposes several conflicting evaluation criteria, so it must be optimized with a Pareto
# sorting scheme -- the generic optimizer's default EA config uses single-objective selection, which would
# drive only the first criterion. Because this example ships only a loadable module (it has no bespoke
# executable), the standard GENEVA_MATERIALIZE_CONFIGS -- which runs a LOCAL executable -- does not fit;
# instead the generic optimizer itself materializes the run-directory configs from its code defaults, and
# the shipped config-overrides.json overlays the Pareto sorting_method onto GEvolutionaryAlgorithm.json.
# (Kept inline rather than generalized into the shared macro until a second collapsed example needs it.)
SET(_mc_cfg_dir ${CMAKE_CURRENT_BINARY_DIR}/config)
SET(_mc_overrides ${CMAKE_CURRENT_SOURCE_DIR}/config/config-overrides.json)
FILE(MAKE_DIRECTORY ${_mc_cfg_dir})
# The materialization runs the freshly built generic optimizer against the freshly built module, so both
# must exist first (GGenericOptimizer is configured before this example in examples/CMakeLists.txt).
ADD_DEPENDENCIES(GMultiCriterionParabolaIndividual GGenericOptimizer)
IF(TARGET GConfigOverlay)
	ADD_DEPENDENCIES(GMultiCriterionParabolaIndividual GConfigOverlay)
ENDIF()
ADD_CUSTOM_COMMAND(TARGET GMultiCriterionParabolaIndividual POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E rm -rf ${_mc_cfg_dir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${_mc_cfg_dir}
	COMMAND $<TARGET_FILE:GGenericOptimizer> --individual $<TARGET_FILE:GMultiCriterionParabolaIndividual> --update-configs
	COMMAND $<TARGET_FILE:GConfigOverlay> ${_mc_cfg_dir} ${_mc_overrides}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Materializing GMultiCriterionParabola configs (Pareto sorting_method)"
	VERBATIM)

IF (GENEVA_RUN_EXAMPLES_AS_TESTS)
	ADD_TEST(
		NAME GMultiCriterionParabola
		COMMAND $<TARGET_FILE:GGenericOptimizer> --individual $<TARGET_FILE:GMultiCriterionParabolaIndividual>
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
ENDIF ()

INSTALL ( TARGETS GMultiCriterionParabolaIndividual DESTINATION ${INSTALL_PREFIX_DATA}/examples/GMultiCriterionParabola )
INSTALL ( DIRECTORY ${_mc_cfg_dir}/ DESTINATION ${INSTALL_PREFIX_DATA}/examples/GMultiCriterionParabola/config )
