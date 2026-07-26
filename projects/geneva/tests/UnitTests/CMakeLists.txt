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


#
# Deliberately broken modules, for the loader's negative paths. Each is a real shared object, built with
# exactly the recipe a genuine module uses (GENEVA_ADD_INDIVIDUAL_MODULE: no Geneva libraries linked,
# -UGEM_TESTING, pinned ABI flags), carrying one specific defect. The unit tests really dlopen them, so the
# rejection is proved through the whole loader path rather than against a hand-made in-process struct.
# Their paths are handed to the test executable as compile definitions -- nothing has to be installed, and
# the tests skip themselves if this wiring is ever dropped.
#
SET(GENEVA_BAD_MODULES
	GStaleAbiModule
	GUnknownKindModule
	GNoContributionsModule
	GNullFactoryModule)

FOREACH(_bad_module ${GENEVA_BAD_MODULES})
	GENEVA_ADD_INDIVIDUAL_MODULE(${_bad_module} badmodules/${_bad_module}.cpp)
	ADD_DEPENDENCIES(${EXECUTABLENAME} ${_bad_module})
ENDFOREACH()

TARGET_COMPILE_DEFINITIONS(${EXECUTABLENAME} PRIVATE
	"GENEVA_TEST_MODULE_STALE_ABI=\"$<TARGET_FILE:GStaleAbiModule>\""
	"GENEVA_TEST_MODULE_UNKNOWN_KIND=\"$<TARGET_FILE:GUnknownKindModule>\""
	"GENEVA_TEST_MODULE_NO_CONTRIBUTIONS=\"$<TARGET_FILE:GNoContributionsModule>\""
	"GENEVA_TEST_MODULE_NULL_FACTORY=\"$<TARGET_FILE:GNullFactoryModule>\""
	# A perfectly good shared library that is not a Geneva module at all -- the "you pointed me at
	# something else" path. Geneva's own library is the one guaranteed to be there.
	"GENEVA_TEST_NOT_A_MODULE=\"$<TARGET_FILE:${GENEVA_LIBNAME}>\"")

ADD_TEST(${EXECUTABLENAME} ${EXECUTABLENAME})
SET_TESTS_PROPERTIES(${EXECUTABLENAME} PROPERTIES LABELS "geneva")

INSTALL ( TARGETS ${EXECUTABLENAME} DESTINATION ${INSTALL_PREFIX_DATA}/tests/geneva/UnitTests )


#
# Take care of other particularities of this test
#

GENEVA_AGGREGATE_TARGET( "tests-geneva-unit" )
