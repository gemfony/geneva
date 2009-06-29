CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_SUBDIRECTORY (thirdparty)

SET ( GENEVAINCLUDES
	GAsioHelperFunctions.hpp
	GAsioTCPClient.hpp
	GAsioTCPConsumer.hpp
	GAdaptorT.hpp
	GBasePopulation.hpp
	GBaseClient.hpp
	GBoolean.hpp
	GBooleanAdaptor.hpp
	GBooleanCollection.hpp
	GBooleanObjectCollection.hpp
	GBoolParameter.hpp
	GBoostThreadConsumer.hpp
	GBoostThreadPopulation.hpp
	GBoundedBufferT.hpp
	GBoundedBufferWithIdT.hpp
	GBoundedDouble.hpp
	GBoundedDoubleCollection.hpp
	GBoundedInt32.hpp
	GBoundedInt32Collection.hpp
	GBoundedNumT.hpp
	GBrokerT.hpp
	GBrokerPopulation.hpp
	GBufferPortT.hpp
	GChar.hpp
	GCharFlipAdaptor.hpp
	GCharObjectCollection.hpp
	GCharParameter.hpp
	GConsumer.hpp
	GDataExchange.hpp
	GDataExchangeException.hpp
	GDouble.hpp
	GDoubleCollection.hpp
	GDoubleGaussAdaptor.hpp
	GDoubleObjectCollection.hpp
	GDoubleParameter.hpp
	GenevaExceptions.hpp
	GGlobalOptionsT.hpp
	GEnums.hpp
	GGaussAdaptorT.hpp
	GGlobalDefines.hpp
	GHelperFunctionsT.hpp
	GIndividualBroker.hpp
	GIndividual.hpp
	GIndividualSet.hpp
	GInt32.hpp
	GInt32Collection.hpp
	GInt32FlipAdaptor.hpp
	GInt32GaussAdaptor.hpp
	GInt32ObjectCollection.hpp
	GIntFlipAdaptorT.hpp
	GLongParameter.hpp
	GMutableI.hpp
	GMutableSetT.hpp
	GNumCollectionT.hpp
	GNumericParameterT.hpp
	GObject.hpp
	GParameterBase.hpp
	GParameterBaseWithAdaptorsT.hpp
	GParameterCollectionT.hpp
	GParameterSet.hpp
	GParameterT.hpp
	GParameterTCollectionT.hpp
	GParameterValuePair.hpp
	GRandom.hpp
	GRandomFactory.hpp
	GRateableI.hpp
	GSerializableI.hpp
	GSerializationHelperFunctions.hpp
	GSingletonT.hpp
	GStdPtrVectorInterfaceT.hpp
	GStdSimpleVectorInterfaceT.hpp
	GThreadGroup.hpp
)

INSTALL ( FILES ${GENEVAINCLUDES} DESTINATION include 
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
