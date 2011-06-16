CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET ( OPTIMIZATIONSOURCES
	GBooleanAdaptor
	GBooleanCollection
	GBooleanObject
	GBooleanObjectCollection
	GBrokerConnector
	GBrokerEA
	GBrokerGD
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
	Geneva
	GEvaluator
	GenevaHelperFunctionsT
	GEvolutionaryAlgorithm
	GGDPersonalityTraits
	GGradientDescent
	GIndividual
	GInt32Collection
	GInt32FlipAdaptor
	GInt32GaussAdaptor
	GInt32Object
	GInt32ObjectCollection
	GMultiThreadedEA
	GMultiThreadedGD
	GMultiThreadedSwarm
	Go
	GObject
	GOptimizationAlgorithmT
	GOptimizationEnums
	GParameterBase
	GParameterBaseWithAdaptorsT
	GParameterObjectCollection
	GParameterSet
	GPersonalityTraits
	GSwarm
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
