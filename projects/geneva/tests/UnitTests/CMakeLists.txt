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
	GGenomeTests.cpp
	GGenomeProcessingTests.cpp
	GNormalizedGenomeTests.cpp
	GOptimizationAlgorithmsTests.cpp
	GStandardPSO2011Tests.cpp
	GAntColonyOptimizationTests.cpp
	GGeneralizedSimulatedAnnealingTests.cpp
	GOABenchmark.cpp
	GEvolutionaryAlgorithmModesTests.cpp
	GAdaptionConfigTests.cpp
	GSharedConsumerTests.cpp
	GProgressPlotterTests.cpp
	GMultiConstraintTests.cpp
	GLateReturnIntegrationTests.cpp
	GFaultInjectorTests.cpp
	GNeuralNetworkDataTests.cpp
)

ADD_EXECUTABLE(${EXECUTABLENAME}
	${${EXECUTABLENAME}_SRCS}
)

SET(LIBRARIES_TO_LINK
		${GENEVA_LIBRARIES}
		${Boost_LIBRARIES}
		Catch2::Catch2WithMain
		# These individuals are no longer compiled into the geneva library; the tests construct and serialize
		# them directly, so they link the individuals' object libraries (which carry their BOOST_CLASS_EXPORT_IMPLEMENT).
		GLineFitIndividual-obj
		GNeuralNetworkIndividual-obj
		GExternalEvaluatorIndividual-obj
		GDelayIndividual-obj
		GFunctionIndividual-obj
		GMetaOptimizerIndividualT-obj
		# GTestIndividual2/3 are packaged as object libraries (GTestIndividual1 stays in the GEM_TESTING library).
		GTestIndividual2-obj
		GTestIndividual3-obj)

# Add MPI Libraries in case building with MPI-consumer
IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
	SET(LIBRARIES_TO_LINK
			${LIBRARIES_TO_LINK}
			${MPI_LIBRARIES})
ENDIF()

TARGET_LINK_LIBRARIES (${EXECUTABLENAME}
		${LIBRARIES_TO_LINK})


ADD_TEST(${EXECUTABLENAME} ${EXECUTABLENAME})
SET_TESTS_PROPERTIES(${EXECUTABLENAME} PROPERTIES LABELS "geneva")

INSTALL ( TARGETS ${EXECUTABLENAME} DESTINATION ${INSTALL_PREFIX_DATA}/tests/geneva/UnitTests )


#
# Take care of other particularities of this test
#

GENEVA_AGGREGATE_TARGET( "tests-geneva-unit" )
