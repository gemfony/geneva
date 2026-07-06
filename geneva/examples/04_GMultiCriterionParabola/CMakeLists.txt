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
# optimizer (example 19's GGenericOptimizer) via --individual, not by a bespoke main. Configuration is
# auto-created from code defaults on first run.
GENEVA_DECLARE_INDIVIDUAL(GMultiCriterionParabolaIndividual
	MODE    load
	SOURCES GMultiCriterionParabolaIndividual.cpp
	PLUGIN  GMultiCriterionParabolaIndividualPlugin.cpp
	CONFIG  ./config/GMultiCriterionParabolaIndividual.json)

IF (GENEVA_RUN_EXAMPLES_AS_TESTS)
	ADD_TEST(
		NAME GMultiCriterionParabola
		COMMAND $<TARGET_FILE:GGenericOptimizer> --individual $<TARGET_FILE:GMultiCriterionParabolaIndividual>
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
ENDIF ()

INSTALL ( TARGETS GMultiCriterionParabolaIndividual DESTINATION ${INSTALL_PREFIX_DATA}/examples/GMultiCriterionParabola )
