CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET ( GENEVASOURCES
	GAsioHelperFunctions
	GAsioTCPClient
	GAsioTCPConsumer
	GBaseClient
	GBasePopulation
	GBoolean
	GBooleanAdaptor
	GBooleanCollection
	GBooleanObjectCollection
	GBoolParameter
	GBoostThreadConsumer
	GBoostThreadPopulation
	GBoundedDouble
	GBoundedDoubleCollection
	GBoundedInt32
	GBoundedInt32Collection
	GBoundedNumT
	GBrokerPopulation
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
	GGaussAdaptorT
	GHelperFunctionsT
	GIndividual
	GIndividualSet
	GInt32
	GInt32Collection
	GInt32FlipAdaptor
	GInt32GaussAdaptor
	GInt32ObjectCollection
	GIntFlipAdaptorT
	GLongParameter
	GNumCollectionT
	GNumericParameterT
	GObject
	GParameterBase
	GParameterBaseWithAdaptorsT
	GParameterSet
	GParameterT
	GParameterValuePair
	GRandom
	GRandomFactory
	GSerializationHelperFunctions
	GThreadGroup
)

# BUILD_SHARED_LIBS controls the behavior of ADD_LIBRARY if STATIC/SHARED omitted
IF( GENEVA_STATIC )
	ADD_LIBRARY ( ${PROJECT_NAME} STATIC ${GENEVASOURCES} )
ELSE()
	ADD_LIBRARY ( ${PROJECT_NAME} SHARED ${GENEVASOURCES} )
ENDIF( GENEVA_STATIC )

SET_TARGET_PROPERTIES(${PROJECT_NAME}
	PROPERTIES VERSION 0.6.0
)

INSTALL ( TARGETS ${PROJECT_NAME} DESTINATION lib
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
