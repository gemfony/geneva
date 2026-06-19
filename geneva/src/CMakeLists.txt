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

SET ( OPTIMIZATIONSOURCES
	Interface/GRateableI.cpp
	GenevaHelperFunctions.cpp
	GConsumerSetup.cpp
	GenevaInitializer.cpp
	GIndividualMultiConstraint.cpp
	GOptimizableEntityCommandContainerExport.cpp
	Go2.cpp
    oa/GOptimizationAlgorithmBase.cpp
    oa/GLineSearch.cpp
    oa/GHesseError.cpp
    oa/GEvolutionaryAlgorithm.cpp
    oa/GEvolutionaryAlgorithmFactory.cpp
    oa/GEvolutionaryAlgorithm_PersonalityTraits.cpp
    oa/GMetaEvolutionaryAlgorithm.cpp
    oa/GParChild.cpp
    oa/GBaseParChildPersonalityTraits.cpp
    oa/GSepCmaEvolutionStrategy.cpp
    oa/GSepCmaEvolutionStrategyFactory.cpp
    oa/GSepCmaEvolutionStrategy_PersonalityTraits.cpp
    oa/GStandardPSO2011.cpp
    oa/GStandardPSO2011Factory.cpp
    oa/GStandardPSO2011_PersonalityTraits.cpp
    oa/GAntColonyOptimization.cpp
    oa/GAntColonyOptimizationFactory.cpp
    oa/GAntColonyOptimization_PersonalityTraits.cpp
	GOptimizationEnums.cpp
	GSigHupHandler.cpp
	par/GParameterPropertyParser.cpp
	ind/GOptimizableEntity.cpp
	ind/GFlatGenome.cpp
	ind/GIndividualSlot.cpp
	par/GOptimizableEntityFixedSizePriorityQueue.cpp
	par/GOptimizableEntityMultiConstraint.cpp
	GPersonalityTraits.cpp
    GPluggableOptimizationMonitors.cpp
    GPostProcessorT.cpp
	individuals/GTestIndividual1.cpp
	individuals/GTestIndividual2.cpp
	individuals/GTestIndividual3.cpp
	individuals/GDelayIndividual.cpp
	individuals/GExternalEvaluatorIndividual.cpp
	individuals/GFunctionIndividual.cpp
	individuals/GLineFitIndividual.cpp
	individuals/GMetaOptimizerIndividualT.cpp
	individuals/GNeuralNetworkIndividual.cpp
)

# Add MPI specific files
IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
	SET(OPTIMIZATIONSOURCES
			${OPTIMIZATIONSOURCES}
			GMPISubClientIndividual.cpp)
ENDIF()

# Add MPI specific files
IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
	SET(OPTIMIZATIONSOURCES
			${OPTIMIZATIONSOURCES}
			GMPISubClientOptimizer.cpp)
ENDIF()

################################################################################
#
# Build-time algorithm selection ("slim core").
#
# Each option below gates the three source files of one optimization algorithm.
# All default to ON, so the stock build is byte-for-byte identical to before:
# every algorithm is compiled in and self-registers its factory with the
# global optimization-algorithm store. Setting an option to OFF omits that
# algorithm's sources; because registration happens from within those very
# translation units, the algorithm then never registers and its mnemonic
# simply disappears from the runtime selection (the '-a' option).
#
# Evolutionary algorithms (EA) are intentionally NOT optional and remain in the
# core source list above: GEvolutionaryAlgorithmFactory is referenced directly
# by the post-processor (GPostProcessorT) and the meta-optimizer individual
# (GMetaOptimizerIndividualT), both of which are always built. The shared
# parent-child base (GParChild / GBaseParChildPersonalityTraits, used by EA and
# SA) likewise stays in core.
#
# Consumers are not gated here: the courtier consumers are header-only
# templates with no .cpp to exclude, and the MPI consumer is already gated by
# GENEVA_BUILD_WITH_MPI_CONSUMER. Consumer plugin selection is deferred to a
# later plugin-architecture phase.
#
################################################################################

OPTION(GENEVA_ALGO_SA    "Build the Simulated Annealing algorithm"         ON)
OPTION(GENEVA_ALGO_SWARM "Build the Swarm algorithm"                       ON)
OPTION(GENEVA_ALGO_GD    "Build the Gradient Descent algorithm"            ON)
OPTION(GENEVA_ALGO_CGD   "Build the Conjugate Gradient Descent algorithm"  ON)
OPTION(GENEVA_ALGO_NM    "Build the Nelder-Mead algorithm"                 ON)
OPTION(GENEVA_ALGO_PS    "Build the Parameter Scan algorithm"              ON)

# Slim-core builds and the test suite are mutually exclusive. The in-library
# unit-test fixtures (notably GTestIndividual1) and the algorithms' own
# GEM_TESTING self-tests reference the optimization algorithms by type, so a
# testing-enabled build needs every algorithm compiled in. Disabling any
# algorithm while GENEVA_BUILD_TESTS is ON would otherwise fail at link time
# with a cryptic undefined-reference error; catch it here with a clear message.
# (Production slim-core builds run with GENEVA_BUILD_TESTS=OFF anyway.)
IF (GENEVA_BUILD_TESTS AND NOT (GENEVA_ALGO_SA AND GENEVA_ALGO_SWARM
		AND GENEVA_ALGO_GD AND GENEVA_ALGO_CGD AND GENEVA_ALGO_NM
		AND GENEVA_ALGO_PS))
	MESSAGE(FATAL_ERROR
		"GENEVA_BUILD_TESTS=ON requires all GENEVA_ALGO_* options to be ON: the "
		"in-library test fixtures reference every optimization algorithm. For a "
		"slim-core build with an algorithm disabled, configure with "
		"-DGENEVA_BUILD_TESTS=OFF.")
ENDIF ()

IF(GENEVA_ALGO_SA)
	SET(OPTIMIZATIONSOURCES
			${OPTIMIZATIONSOURCES}
			oa/GSimulatedAnnealing.cpp
			oa/GSimulatedAnnealingFactory.cpp
			oa/GSimulatedAnnealing_PersonalityTraits.cpp)
ENDIF()

IF(GENEVA_ALGO_SWARM)
	SET(OPTIMIZATIONSOURCES
			${OPTIMIZATIONSOURCES}
			oa/GSwarmAlgorithm.cpp
			oa/GSwarmAlgorithmFactory.cpp
			oa/GSwarmAlgorithm_PersonalityTraits.cpp)
ENDIF()

# The "gd" (gradient descent) algorithm has been folded into conjugate gradient descent as its
# steepest-descent mode; the "gd" mnemonic survives as a backward-compatible alias (the factory
# produces a CGD forced into steepest descent), so this block keeps only the alias factory and the
# personality-traits marker, and requires the CGD algorithm to be built.
IF(GENEVA_ALGO_GD)
	IF(NOT GENEVA_ALGO_CGD)
		MESSAGE(FATAL_ERROR "GENEVA_ALGO_GD requires GENEVA_ALGO_CGD: 'gd' is now an alias for conjugate gradient descent in steepest-descent mode.")
	ENDIF()
	SET(OPTIMIZATIONSOURCES
			${OPTIMIZATIONSOURCES}
			oa/GGradientDescent_PersonalityTraits.cpp)
ENDIF()

IF(GENEVA_ALGO_CGD)
	SET(OPTIMIZATIONSOURCES
			${OPTIMIZATIONSOURCES}
			oa/GConjugateGradientDescent.cpp
			oa/GConjugateGradientDescentFactory.cpp
			oa/GConjugateGradientDescent_PersonalityTraits.cpp)
ENDIF()

IF(GENEVA_ALGO_NM)
	SET(OPTIMIZATIONSOURCES
			${OPTIMIZATIONSOURCES}
			oa/GNelderMead.cpp
			oa/GNelderMeadFactory.cpp
			oa/GNelderMead_PersonalityTraits.cpp)
ENDIF()

IF(GENEVA_ALGO_PS)
	SET(OPTIMIZATIONSOURCES
			${OPTIMIZATIONSOURCES}
			oa/GParameterScan.cpp
			oa/GParameterScanFactory.cpp
			oa/GParameterScan_PersonalityTraits.cpp)
ENDIF()

ADD_LIBRARY ( ${GENEVA_LIBNAME} SHARED ${OPTIMIZATIONSOURCES} )

SET_TARGET_PROPERTIES(
    ${GENEVA_LIBNAME}
    PROPERTIES
    VERSION ${GENEVA_VERSION}
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	EXPORT_NAME geneva
)



# Export the include directory and require C++20 transitively.
TARGET_INCLUDE_DIRECTORIES(
	${GENEVA_LIBNAME}
	PUBLIC
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/geneva/include>
	$<INSTALL_INTERFACE:include>
)
TARGET_COMPILE_FEATURES(${GENEVA_LIBNAME} PUBLIC cxx_std_20)

# Inter-library dependency: geneva builds on courtier (and transitively on hap
# and common). PUBLIC so the full include/Boost/C++20 interface propagates to
# downstream consumers that link Geneva::geneva.
TARGET_LINK_LIBRARIES(${GENEVA_LIBNAME} PUBLIC ${COURTIER_LIBNAME})

IF ( GENEVA_BUILD_TESTS )
	# When built with testing support (-DGEM_TESTING), the library embeds
	# Catch2-based test scaffolding (GTestIndividual*, GUnitTestFrameworkT) and
	# therefore references Catch2 symbols. Consumers of such a testing-enabled
	# install must link Catch2 too, so the requirement is propagated through the
	# public interface (both in-tree and exported). The installed config package
	# re-finds Catch2 accordingly (see GenevaConfig.cmake.in / GENEVA_TESTING).
	TARGET_LINK_LIBRARIES(
		${GENEVA_LIBNAME}
		INTERFACE
		Catch2::Catch2
	)
ENDIF ()

IF (GENEVA_BUILD_WITH_MPI_CONSUMER)
	TARGET_LINK_LIBRARIES(${GENEVA_LIBNAME} PRIVATE MPI::MPI_CXX)
ENDIF ()

INSTALL ( TARGETS ${GENEVA_LIBNAME} EXPORT GenevaTargets DESTINATION ${INSTALL_PREFIX_LIBS} )

