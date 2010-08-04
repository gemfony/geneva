CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET ( OPTIMIZATIONSOURCES
	GBooleanAdaptor
	GBoolean
	GBooleanCollection
	GBooleanObjectCollection
	GBoundedDouble
	GBoundedDoubleCollection
	GBoundedInt32
	GBoundedInt32Collection
	GBoundedNumT
	GBrokerEA
	GConstrainedInt32
	GDouble
	GDoubleCollection
	GDoubleGaussAdaptor
	GDoubleObjectCollection
	GEAPersonalityTraits
	GEvolutionaryAlgorithm
	GGaussAdaptorT
	GGDPersonalityTraits
	GIdentityAdaptorT
	GIndividual
	GInt32
	GInt32Collection
	GInt32FlipAdaptor
	GInt32GaussAdaptor
	GInt32ObjectCollection
	GIntFlipAdaptorT
	GMultiThreadedEA
	GObject
	GOptimizationAlgorithm
	GOptimizationEnums
	GParameterBase
	GParameterBaseWithAdaptorsT
	GParameterObjectCollection
	GParameterSet
	GPersonalityTraits
#	GSwarm
	GSwarmAdaptor
	GSwarmPersonalityTraits
)

# BUILD_SHARED_LIBS controls the behavior of ADD_LIBRARY if STATIC/SHARED omitted
IF ( GENEVA_STATIC )
	ADD_LIBRARY ( ${OPTIMIZATIONLIBNAME} STATIC ${OPTIMIZATIONSOURCES} )
ELSE ()
	ADD_LIBRARY ( ${OPTIMIZATIONLIBNAME} SHARED ${OPTIMIZATIONSOURCES} )
ENDIF ( GENEVA_STATIC )

SET_TARGET_PROPERTIES( ${OPTIMIZATIONLIBNAME}
	PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
)

INSTALL ( TARGETS ${OPTIMIZATIONLIBNAME} DESTINATION lib
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
