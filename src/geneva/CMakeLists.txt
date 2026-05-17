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
	RateableI.cpp
	GAdaptorT.cpp
	GBooleanAdaptor.cpp
	GBooleanCollection.cpp
	GBooleanObject.cpp
	GBooleanObjectCollection.cpp
	GConstrainedDoubleCollection.cpp
	GConstrainedDoubleObject.cpp
	GConstrainedDoubleObjectCollection.cpp
	GConstrainedInt32Object.cpp
	GConstrainedInt32ObjectCollection.cpp
	GDoubleBiGaussAdaptor.cpp
	GDoubleCollection.cpp
	GDoubleGaussAdaptor.cpp
	GDoubleObject.cpp
	GDoubleObjectCollection.cpp
	GenevaHelperFunctionsT.cpp
	GenevaHelperFunctions.cpp
	GenevaInitializer.cpp
	GIndividualMultiConstraint.cpp
	GIndividualStandardConsumers.cpp
	GInt32Collection.cpp
	GInt32FlipAdaptor.cpp
	GInt32GaussAdaptor.cpp
	GInt32Object.cpp
	GInt32ObjectCollection.cpp
	Go2.cpp
    GBase.cpp
    ConjugateGradientDescent.cpp
    ConjugateGradientDescent_Factory.cpp
    ConjugateGradientDescent_PersonalityTraits.cpp
    EvolutionaryAlgorithm.cpp
    EvolutionaryAlgorithm_Factory.cpp
    EvolutionaryAlgorithm_PersonalityTraits.cpp
    GradientDescent.cpp
    GradientDescent_Factory.cpp
    GradientDescent_PersonalityTraits.cpp
    NelderMead.cpp
    NelderMead_Factory.cpp
    NelderMead_PersonalityTraits.cpp
    GParChild.cpp
    ParChildT_PersonalityTraits.cpp
    ParameterScan.cpp
    ParameterScan_Factory.cpp
    ParameterScan_PersonalityTraits.cpp
    SimulatedAnnealing.cpp
    SimulatedAnnealing_Factory.cpp
    SimulatedAnnealing_PersonalityTraits.cpp
    SwarmAlgorithm.cpp
    SwarmAlgorithm_Factory.cpp
    SwarmAlgorithm_PersonalityTraits.cpp
	GObject.cpp
	GOptimizationEnums.cpp
	GParameterBase.cpp
	GParameterObjectCollection.cpp
	GParameterPropertyParser.cpp
	GParameterSet.cpp
	GParameterSetFixedSizePriorityQueue.cpp
	GParameterSetMultiConstraint.cpp
	GPersonalityTraits.cpp
    GPluggableOptimizationMonitors.cpp
    GPostProcessorT.cpp
	GTestIndividual1.cpp
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

ADD_LIBRARY ( ${GENEVA_LIBNAME} SHARED ${OPTIMIZATIONSOURCES} )

SET_TARGET_PROPERTIES(
    ${GENEVA_LIBNAME}
    PROPERTIES
    VERSION ${GENEVA_VERSION}
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)



IF ( GENEVA_BUILD_TESTS )
	TARGET_LINK_LIBRARIES(
		${GENEVA_LIBNAME}
		INTERFACE
		Catch2::Catch2
	)
ENDIF ()

IF (GENEVA_BUILD_WITH_MPI_CONSUMER)
	TARGET_LINK_LIBRARIES(${GENEVA_LIBNAME} PRIVATE MPI::MPI_CXX)
ENDIF ()

INSTALL ( TARGETS ${GENEVA_LIBNAME} DESTINATION ${INSTALL_PREFIX_LIBS} )

