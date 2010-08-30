CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET ( OPTIMIZATIONSOURCES
	GBooleanAdaptor
	GBoolean
	GBooleanCollection
	GBooleanObjectCollection
	GBrokerConnector
	GBrokerEA
	GBrokerSwarm
	GConstrainedDouble
	GConstrainedDoubleObjectCollection
	GConstrainedInt32
	GConstrainedInt32ObjectCollection
	GDouble
	GDoubleCollection
	GDoubleGaussAdaptor
	GDoubleObjectCollection
	GEAPersonalityTraits
	GEvolutionaryAlgorithm
	GGDPersonalityTraits
	GIndividual
	GInt32
	GInt32Collection
	GInt32FlipAdaptor
	GInt32GaussAdaptor
	GInt32ObjectCollection
	GIntFlipAdaptorT
	GMultiThreadedEA
	GMultiThreadedSwarm
	GObject
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
