CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( OPTIMIZATIONSOURCES
    GAdaptorT
	GBaseEA
	GBaseGD
	GBasePS
	GBaseParChildPersonalityTraits
	GBaseSA
	GBaseSwarm
	GBooleanAdaptor
	GBooleanCollection
	GBooleanObject
	GBooleanObjectCollection
	GBrokerEA
	GBrokerGD
	GBrokerPS
	GBrokerSA
	GBrokerSwarm
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
	GEAPersonalityTraits
	GenevaHelperFunctionsT
	GenevaInitializer
	GEvolutionaryAlgorithmFactory
	GGDPersonalityTraits
	GGradientDescentFactory
	GIndividualMultiConstraint
	GIndividualStandardConsumers
	GInt32Collection
	GInt32FlipAdaptor
	GInt32GaussAdaptor
	GInt32Object
	GInt32ObjectCollection
	GMultiThreadedEA
	GMultiThreadedGD
	GMultiThreadedPS
	GMultiThreadedSA
	GMultiThreadedSwarm
	Go2
    G_OA_EvolutionaryAlgorithm
    G_OA_GradientDescent
    G_OA_ParameterScan
    G_OA_SimulatedAnnealing
    G_OA_SimulatedAnnealingFactory
    G_OA_SwarmAlgorithm
	GObject
	GOptimizableEntity
	GOptimizableI
	GOptimizationAlgorithmT
    GOptimizationAlgorithmT2
	GOptimizationEnums
	GParameterBase
	GParameterObjectCollection
	GParameterPropertyParser
	GParameterScanFactory
	GParameterSet
	GParameterSetParChild
	GParameterSetFixedSizePriorityQueue
	GParameterSetMultiConstraint
	GPersonalityTraits
    GPluggableOptimizationMonitors
    GPostProcessorT
	GPSPersonalityTraits
	GRateableI
	GSAPersonalityTraits
	GScanPar
	GSerialEA
	GSerialGD
	GSerialPS
	GSerialSA
	GSerialSwarm
	GSwarmAlgorithmFactory
	GSwarmPersonalityTraits
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

