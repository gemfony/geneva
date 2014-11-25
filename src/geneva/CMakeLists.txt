CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( OPTIMIZATIONSOURCES
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
	GConstrainedNumCollectionT
	GConstrainedNumT
	GDoubleBiGaussAdaptor
	GDoubleCollection
	GDoubleGaussAdaptor
	GDoubleObject
	GDoubleObjectCollection
	GEAPersonalityTraits
	GenevaHelperFunctionsT
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
	GLineFit
	GLineFitIndividual
	GMPEAPersonalityTraits
	GMultiThreadedEA
	GMultiThreadedGD
	GMultiThreadedPS
	GMultiThreadedSA
	GMultiThreadedSwarm
	GNumT
	GNumCollectionT
	Go2
	GObject
	GOptimizableEntity
	GOptimizableI
	GOptimizationAlgorithmT
	GOptimizationEnums
	GParameterBase
	GParameterBaseWithAdaptorsT
	GParameterCollectionT
	GParameterObjectCollection
	GParameterPropertyParser
	GParameterScanFactory
	GParameterSet
	GParameterSetParChild
	GParameterSetFixedSizePriorityQueue
	GParameterSetMultiConstraint
	GParameterT
	GPersonalityTraits
	GPSPersonalityTraits
	GRateableI
	GSAPersonalityTraits
	GScanPar
	GSerialEA
	GSerialGD
	GSerialPS
	GSerialSA
	GSerialSwarm
	GSimulatedAnnealingFactory
	GSwarmAlgorithmFactory
	GSwarmPersonalityTraits
	GTestIndividual1
)

# BUILD_SHARED_LIBS controls the behavior of ADD_LIBRARY if STATIC/SHARED omitted
IF ( GENEVA_STATIC )
	ADD_LIBRARY ( ${GENEVA_LIBNAME} STATIC ${OPTIMIZATIONSOURCES} )
ELSE ()
	ADD_LIBRARY ( ${GENEVA_LIBNAME} SHARED ${OPTIMIZATIONSOURCES} )
ENDIF ( GENEVA_STATIC )

SET_TARGET_PROPERTIES( ${GENEVA_LIBNAME}
	PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
)

IF ( PLATFORM_NEEDS_LIBRARY_LINKING )
	TARGET_LINK_LIBRARIES(
		${GENEVA_LIBNAME}
		${COURTIER_LIBNAME}
		${HAP_LIBNAME}
		${COMMON_LIBNAME}
		${Boost_LIBRARIES}
	)
ENDIF ()

INSTALL ( TARGETS ${GENEVA_LIBNAME} DESTINATION lib )
