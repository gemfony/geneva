CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

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
	GConstrainedFloatCollection
	GConstrainedFloatObject
	GConstrainedFloatObjectCollection
	GConstrainedInt32Object
	GConstrainedInt32ObjectCollection
	GDoubleBiGaussAdaptor
	GDoubleCollection
	GDoubleGaussAdaptor
	GDoubleObject
	GDoubleObjectCollection
	GEAPersonalityTraits
	GenevaHelperFunctionsT
	GEvolutionaryAlgorithmFactory
	GFloatBiGaussAdaptor
	GFloatCollection
	GFloatGaussAdaptor
	GFloatObject
	GFloatObjectCollection
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
	GParameterSetFixedSizePriorityQueue
	GParameterSetMultiConstraint
	GParameterT
	GPersonalityTraits
	GPSPersonalityTraits
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

################################################################################
# Add additional libraries if this is MacOS
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	TARGET_LINK_LIBRARIES(
		${GENEVA_LIBNAME}
		${COURTIER_LIBNAME}
		${HAP_LIBNAME}
		${COMMON_LIBNAME}
	)
ENDIF()

################################################################################

INSTALL ( TARGETS ${GENEVA_LIBNAME} DESTINATION lib
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
