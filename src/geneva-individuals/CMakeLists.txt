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

SET ( SAMPLEINDIVIDUALSOURCES
    GDelayIndividual.cpp
	GExternalEvaluatorIndividual.cpp
	GFunctionIndividual.cpp
	GLineFitIndividual.cpp
	GMetaOptimizerIndividualT.cpp
	GNeuralNetworkIndividual.cpp
	GTestIndividual2.cpp
	GTestIndividual3.cpp
)

set_source_files_properties(
    ${SAMPLEINDIVIDUALSOURCES}
    PROPERTIES
    LANGUAGE CXX
    CXX_STANDARD 20
)

ADD_LIBRARY ( ${GENEVA_INDIVIDUAL_LIBNAME} SHARED ${SAMPLEINDIVIDUALSOURCES} )

SET_TARGET_PROPERTIES(
    ${GENEVA_INDIVIDUAL_LIBNAME}
    PROPERTIES
    VERSION ${GENEVA_VERSION}
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
    MACOSX_RPATH ${INSTALL_PREFIX_LIBS}
)

# Enable API-exporting for this library
ADD_DEFINITIONS("-DGEM_INDIVIDUALS_EXPORTS")

IF ( PLATFORM_NEEDS_LIBRARY_LINKING )
	TARGET_LINK_LIBRARIES(
		${GENEVA_INDIVIDUAL_LIBNAME}
		${GENEVA_LIBNAME}
		${COURTIER_LIBNAME}
		${HAP_LIBNAME}
		${COMMON_LIBNAME}
		${Boost_LIBRARIES}
	)
ENDIF ()

INSTALL ( TARGETS ${GENEVA_INDIVIDUAL_LIBNAME} DESTINATION ${INSTALL_PREFIX_LIBS} )
