CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

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
	GBoostThreadConsumer.hpp
	GBoostThreadPopulation.hpp
	GBoundedBufferT.hpp
	GBoundedBufferWithIdT.hpp
	GBoundedDouble.hpp
	GBrokerT.hpp
	GBrokerPopulation.hpp
	GBufferPortT.hpp
	GCharFlipAdaptor.hpp
	GConsumer.hpp
	GDoubleCollection.hpp
	GDoubleGaussAdaptor.hpp
	GGaussAdaptorT.hpp
	GenevaExceptions.hpp
	GEnums.hpp
	GIndividualBroker.hpp
	GIndividual.hpp
	GIndividualSet.hpp
	GInt32Collection.hpp
	GInt32FlipAdaptor.hpp
	GInt32GaussAdaptor.hpp
	GIntFlipAdaptorT.hpp
	GLogger.hpp
	GLogTargets.hpp
	GMutableI.hpp
	GMutableSetT.hpp
	GNumCollectionT.hpp
	GObject.hpp
	GParameterBase.hpp
	GParameterBaseWithAdaptorsT.hpp
	GParameterCollectionT.hpp
	GParameterSet.hpp
	GParameterT.hpp
	GRandom.hpp
	GRandomFactory.hpp
	GRateableI.hpp
	GSerializableI.hpp
	GSerializationHelperFunctions.hpp
	GSingleton.hpp
	GThreadGroup.hpp
)

INSTALL ( FILES ${GENEVAINCLUDES} DESTINATION include 
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
