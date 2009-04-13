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
	GIndividual
	GIndividualSet
	GInt32
	GInt32Collection
	GInt32FlipAdaptor
	GInt32GaussAdaptor
	GInt32ObjectCollection
	GIntFlipAdaptorT
	GLogger
	GLogTargets
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
	GStdSimpleVectorInterfaceT
	GThreadGroup
)

ADD_LIBRARY ( ${PROJECT_NAME} SHARED ${GENEVASOURCES})
SET_TARGET_PROPERTIES(${PROJECT_NAME}
	PROPERTIES VERSION 0.5.2
)

INSTALL ( TARGETS ${PROJECT_NAME} DESTINATION lib
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
