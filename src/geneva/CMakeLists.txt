CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( OPTIMIZATIONSOURCES
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
	GenevaInitializer
	GIndividualMultiConstraint
	GIndividualStandardConsumers
	GInt32Collection
	GInt32FlipAdaptor
	GInt32GaussAdaptor
	GInt32Object
	GInt32ObjectCollection
    G_Interface_Optimizer
    G_Interface_Rateable
	Go2
    G_OptimizationAlgorithm_Base
    G_OptimizationAlgorithm_EvolutionaryAlgorithm
    G_OptimizationAlgorithm_GradientDescent
    G_OptimimzationAlgorithm_ParameterScan
    G_OptimizationAlgorithm_SimulatedAnnealing
    G_OA_ConjugateGradientDescent
    G_OA_EvolutionaryAlgorithm_PersonalityTraits
    G_OA_EvolutionaryAlgorithm_Factory
    G_OA_GradientDescent_Factory
    G_OA_GradientDescent_PersonalityTraits
    G_OA_ParameterScan_Factory
    G_OA_ParameterScan_PersonalityTraits
    G_OA_ParChildT_PersonalityTraits
    G_OA_SimulatedAnnealing_Factory
    G_OA_SimulatedAnnealing_PersonalityTraits
    G_OA_SwarmAlgorithm
    G_OA_SwarmAlgorithm_Factory
    G_OA_SwarmAlgorithm_PersonalityTraits
	GObject
	GOptimizableEntity
	GOptimizationEnums
	GParameterBase
	GParameterObjectCollection
	GParameterPropertyParser
	GParameterSet
	GParameterSetFixedSizePriorityQueue
	GParameterSetMultiConstraint
	GPersonalityTraits
    GPluggableOptimizationMonitorsT
    GPostProcessorT
	GTestIndividual1
)

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
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED 14
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

