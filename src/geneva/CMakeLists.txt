CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET ( OPTIMIZATIONSOURCES
	GBaseEA
	GBaseGD
	GBaseParChildPersonalityTraits
	GBaseSA
	GBaseSwarm
	GBooleanAdaptor
	GBooleanCollection
	GBooleanObject
	GBooleanObjectCollection
	GBrokerEA
	GBrokerGD
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
	Geneva
	GenevaHelperFunctionsT
	GEvolutionaryAlgorithmFactory
	GFloatBiGaussAdaptor
	GFloatCollection
	GFloatGaussAdaptor
	GFloatObject
	GFloatObjectCollection
	GGDPersonalityTraits
	GGradientDescentFactory
	GIndividual
	GInt32Collection
	GInt32FlipAdaptor
	GInt32GaussAdaptor
	GInt32Object
	GInt32ObjectCollection
	GMPEAPersonalityTraits
	GMultiThreadedEA
	GMultiThreadedGD
	GMultiThreadedSA
	GMultiThreadedSwarm
	Go2
	GObject
	GOptimizableI
	GOptimizationAlgorithmT
	GOptimizationEnums
	GParameterBase
	GParameterBaseWithAdaptorsT
	GParameterCollectionT
	GParameterObjectCollection
	GParameterSet
	GParameterT
	GPersonalityTraits
	GSAPersonalityTraits
	GSerialEA
	GSerialGD
	GSerialSA
	GSerialSwarm
	GSimulatedAnnealingFactory
	GSwarmAlgorithmFactory
	GSwarmPersonalityTraits
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

INSTALL ( TARGETS ${GENEVA_LIBNAME} DESTINATION lib
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
