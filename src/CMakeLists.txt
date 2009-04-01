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
	GBoolParameter
	GBoostThreadConsumer
	GBoostThreadPopulation
	GBoundedDouble
	GBoundedDoubleCollection
	GBrokerPopulation
	GChar
	GCharFlipAdaptor
	GCharParameter
	GDataExchange
	GDataExchangeException
	GDoubleCollection 
	GDoubleGaussAdaptor
	GDoubleParameter
	GGaussAdaptorT
	GIndividual
	GIndividualSet
	GInt32Collection
	GInt32FlipAdaptor
	GInt32GaussAdaptor
	GIntFlipAdaptorT
	GLogger
	GLogTargets
	GLongParameter
	GNumCollectionT
	GNumericParameterT.cpp
	GObject
	GParameterBase
	GParameterBaseWithAdaptorsT
	GParameterSet
	GParameterValuePair
	GRandom
	GRandomFactory
	GSerializationHelperFunctions
	GThreadGroup
)

ADD_LIBRARY ( ${PROJECT_NAME} SHARED ${GENEVASOURCES})
SET_TARGET_PROPERTIES(${PROJECT_NAME}
	PROPERTIES VERSION 0.5.1
)

INSTALL ( TARGETS ${PROJECT_NAME} DESTINATION lib
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
