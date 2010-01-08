CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET ( GENEVASOURCES
	GAsioHelperFunctions
	GAsioTCPClient
	GAsioTCPConsumer
	GBaseClient
	GBoolean
	GBooleanAdaptor
	GBooleanCollection
	GBooleanObjectCollection
	GBoolParameter
	GBoostThreadConsumer
	GBoundedDouble
	GBoundedDoubleCollection
	GBoundedInt32
	GBoundedInt32Collection
	GBoundedNumT
	GBrokerEA
	GChar
	GCharFlipAdaptor
	GCharObjectCollection
	GCharParameter
	GDataExchange
	GDataExchangeException
	GDouble
	GDoubleCollection 
	GDoubleGaussAdaptor
	GDoubleObjectCollection
	GDoubleParameter
	GEAPersonalityTraits
	GEvolutionaryAlgorithm
	GGaussAdaptorT
	GGDPersonalityTraits
	GHelperFunctionsT
	GIndividual
	GInt32
	GInt32Collection
	GInt32FlipAdaptor
	GInt32GaussAdaptor
	GInt32ObjectCollection
	GIntFlipAdaptorT
	GLongParameter
	GMultiThreadedEA
	GNumCollectionT
	GNumericParameterT
	GObject
	GOptimizationAlgorithm
	GParameterBase
	GParameterBaseWithAdaptorsT
	GParameterSet
	GParameterT
	GParameterValuePair
	GPersonalityTraits
	GRandom
	GRandomFactory
	GSerializationHelperFunctions
#	GSwarm
	GSwarmPersonalityTraits
	GThreadGroup
)

# BUILD_SHARED_LIBS controls the behavior of ADD_LIBRARY if STATIC/SHARED omitted
IF( GENEVA_STATIC )
	ADD_LIBRARY ( ${PROJECT_NAME} STATIC ${GENEVASOURCES} )
ELSE()
	ADD_LIBRARY ( ${PROJECT_NAME} SHARED ${GENEVASOURCES} )
ENDIF( GENEVA_STATIC )

SET_TARGET_PROPERTIES(${PROJECT_NAME}
	PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
)

INSTALL ( TARGETS ${PROJECT_NAME} DESTINATION lib
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
