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
	G_Interface_Rateable
	GAdaptorT
	GBooleanAdaptor
	GBooleanCollection
	GBooleanObject
	GBooleanObjectCollection
	GConstrainedDoubleCollection
	GConstrainedDoubleObject
	GConstrainedDoubleObjectCollection
	GConstrainedInt32Object
	GConstrainedInt32ObjectCollection
	GDoubleBiGaussAdaptor
	GDoubleCollection
	GDoubleGaussAdaptor
	GDoubleObject
	GDoubleObjectCollection
	GenevaHelperFunctionsT
	GenevaHelperFunctions
	GenevaInitializer
	GIndividualMultiConstraint
	GIndividualStandardConsumers
	GInt32Collection
	GInt32FlipAdaptor
	GInt32GaussAdaptor
	GInt32Object
	GInt32ObjectCollection
	Go2
    G_OptimizationAlgorithm_Base
    G_OptimizationAlgorithm_EvolutionaryAlgorithm
    G_OptimizationAlgorithm_EvolutionaryAlgorithm_Factory
    G_OptimizationAlgorithm_EvolutionaryAlgorithm_PersonalityTraits
    G_OptimizationAlgorithm_GradientDescent
    G_OptimizationAlgorithm_GradientDescent_Factory
    G_OptimizationAlgorithm_GradientDescent_PersonalityTraits
    G_OptimizationAlgorithm_ParChild
    G_OptimizationAlgorithm_ParChildT_PersonalityTraits
    G_OptimizationAlgorithm_ParameterScan
    G_OptimizationAlgorithm_ParameterScan_Factory
    G_OptimizationAlgorithm_ParameterScan_PersonalityTraits
    G_OptimizationAlgorithm_SimulatedAnnealing
    G_OptimizationAlgorithm_SimulatedAnnealing_Factory
    G_OptimizationAlgorithm_SimulatedAnnealing_PersonalityTraits
    G_OptimizationAlgorithm_SwarmAlgorithm
    G_OptimizationAlgorithm_SwarmAlgorithm_Factory
    G_OptimizationAlgorithm_SwarmAlgorithm_PersonalityTraits
    G_OA_ConjugateGradientDescent
	GObject
	GOptimizationEnums
	GParameterBase
	GParameterObjectCollection
	GParameterPropertyParser
	GParameterSet
	GParameterSetFixedSizePriorityQueue
	GParameterSetMultiConstraint
	GPersonalityTraits
    GPluggableOptimizationMonitors
    GPostProcessorT
	GTestIndividual1
)

# Add MPI specific files
IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
	SET(OPTIMIZATIONSOURCES
			${OPTIMIZATIONSOURCES}
			GMPISubClientOptimizer)
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

