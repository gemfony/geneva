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

SET ( EXECUTABLENAME GenevaStandardTests )

SET ( ${EXECUTABLENAME}_SRCS
	GenevaStandardTests.cpp
	GLineSearchTests.cpp
	GHesseErrorTests.cpp
	GAuxiliaryStoreTests.cpp
	GAdaptionKernelTests.cpp
	GFlatGenomeTests.cpp
	GFlatOptimizationAlgorithmsTests.cpp
	GStandardPSO2011Tests.cpp
	GAntColonyOptimizationTests.cpp
	GGeneralizedSimulatedAnnealingTests.cpp
	GOABenchmark.cpp
	GEvolutionaryAlgorithmModesTests.cpp
	GAdaptionConfigTests.cpp
	GIndividualSlotTests.cpp
	GSharedConsumerTests.cpp
)

ADD_EXECUTABLE(${EXECUTABLENAME}
	${${EXECUTABLENAME}_SRCS}
)

SET(LIBRARIES_TO_LINK
		${GENEVA_LIBRARIES}
		${Boost_LIBRARIES}
		Catch2::Catch2WithMain)

# Add MPI Libraries in case building with MPI-consumer
IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
	SET(LIBRARIES_TO_LINK
			${LIBRARIES_TO_LINK}
			${MPI_LIBRARIES})
ENDIF()

TARGET_LINK_LIBRARIES (${EXECUTABLENAME}
		${LIBRARIES_TO_LINK})

#ADD_SUBDIRECTORY (config)

ADD_TEST(${EXECUTABLENAME} ${EXECUTABLENAME})
SET_TESTS_PROPERTIES(${EXECUTABLENAME} PROPERTIES LABELS "geneva")

INSTALL ( TARGETS ${EXECUTABLENAME} DESTINATION ${INSTALL_PREFIX_DATA}/tests/geneva/UnitTests )


#
# Take care of other particularities of this test
#

GENEVA_AGGREGATE_TARGET( "tests-geneva-unit" )
