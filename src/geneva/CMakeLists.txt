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
# Geneva was started by Dr. Rüdiger Berlich and was later maintained together
# with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
# information on Gemfony scientific, see http://www.gemfomy.eu .
#
# The majority of files in Geneva was released under the Apache license v2.0
# in February 2020.
#
# See the NOTICE file in the top-level directory of the Geneva library
# collection for a list of contributors and copyright information.
#
################################################################################

SET ( OPTIMIZATIONSOURCES
	G_Interface_Rateable.cpp
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
    G_OptimizationAlgorithm_Base.cpp
    G_OptimizationAlgorithm_EvolutionaryAlgorithm.cpp
    G_OptimizationAlgorithm_EvolutionaryAlgorithm_Factory.cpp
    G_OptimizationAlgorithm_EvolutionaryAlgorithm_PersonalityTraits.cpp
    G_OptimizationAlgorithm_GradientDescent.cpp
    G_OptimizationAlgorithm_GradientDescent_Factory.cpp
    G_OptimizationAlgorithm_GradientDescent_PersonalityTraits.cpp
    G_OptimizationAlgorithm_ParChild.cpp
    G_OptimizationAlgorithm_ParChildT_PersonalityTraits.cpp
    G_OptimizationAlgorithm_ParameterScan.cpp
    G_OptimizationAlgorithm_ParameterScan_Factory.cpp
    G_OptimizationAlgorithm_ParameterScan_PersonalityTraits.cpp
    G_OptimizationAlgorithm_SimulatedAnnealing.cpp
    G_OptimizationAlgorithm_SimulatedAnnealing_Factory.cpp
    G_OptimizationAlgorithm_SimulatedAnnealing_PersonalityTraits.cpp
    G_OptimizationAlgorithm_SwarmAlgorithm.cpp
    G_OptimizationAlgorithm_SwarmAlgorithm_Factory.cpp
    G_OptimizationAlgorithm_SwarmAlgorithm_PersonalityTraits.cpp
    G_OA_ConjugateGradientDescent.cpp
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

# BUILD_SHARED_LIBS controls the behavior of ADD_LIBRARY if STATIC/SHARED omitted
IF ( GENEVA_STATIC )
	ADD_LIBRARY ( ${GENEVA_LIBNAME} STATIC ${OPTIMIZATIONSOURCES} )
ELSE ()
	ADD_LIBRARY ( ${GENEVA_LIBNAME} SHARED ${OPTIMIZATIONSOURCES} )
ENDIF ()

SET_TARGET_PROPERTIES(
    ${GENEVA_LIBNAME}
    PROPERTIES
    VERSION ${GENEVA_VERSION}
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
    MACOSX_RPATH ${INSTALL_PREFIX_LIBS}
)

# Enable API-exporting for this library
ADD_DEFINITIONS("-DGEM_GENEVA_EXPORTS")

IF ( PLATFORM_NEEDS_LIBRARY_LINKING )
	TARGET_LINK_LIBRARIES(
		${GENEVA_LIBNAME}
		${COURTIER_LIBNAME}
		${HAP_LIBNAME}
		${COMMON_LIBNAME}
		${Boost_LIBRARIES}
	)
ENDIF ()

INSTALL ( TARGETS ${GENEVA_LIBNAME} DESTINATION ${INSTALL_PREFIX_LIBS} )

