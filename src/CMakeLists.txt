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
	GBoostThreadConsumer
	GBoostThreadPopulation
	GBoundedDouble
	GBrokerPopulation
	GCharFlipAdaptor
	GDoubleCollection 
	GDoubleGaussAdaptor
	GGaussAdaptorT
	GIndividual
	GIndividualSet
	GInt32Collection
	GInt32FlipAdaptor
	GInt32GaussAdaptor
	GIntFlipAdaptorT
	GLogger
	GLogTargets
	GNumCollectionT
	GObject
	GParameterBase
	GParameterBaseWithAdaptorsT
	GParameterSet
	GRandom
	GRandomFactory
	GSerializationHelperFunctions
	GThreadGroup
)

ADD_LIBRARY ( ${PROJECT_NAME} SHARED ${GENEVASOURCES})
SET_TARGET_PROPERTIES(${PROJECT_NAME}
	PROPERTIES VERSION 0.5
)

INSTALL ( TARGETS ${PROJECT_NAME} DESTINATION lib
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
