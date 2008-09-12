CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET ( GENEVAINCLUDES
	GAsioHelperFunctions.hpp
	GAsioTCPClient.hpp
	GAsioTCPConsumer.hpp
	GAdaptorT.hpp
	GBasePopulation.hpp
	GBaseClient.hpp
	GBitCollection.hpp
	GBitFlipAdaptor.hpp
	GBit.hpp
	GBoostThreadConsumer.hpp
	GBoostThreadPopulation.hpp
	GBoundedBufferT.hpp
	GBoundedDouble.hpp
	GBrokerT.hpp
	GBrokerPopulation.hpp
	GBufferPortT.hpp
	GConsumer.hpp
	GDoubleCollection.hpp
	GDoubleGaussAdaptor.hpp
	GenevaExceptions.hpp
	GEnums.hpp
	GIndividualBroker.hpp
	GIndividual.hpp
	GIndividualSet.hpp
	GLogger.hpp
	GLogTargets.hpp
	GMutableI.hpp
	GMutableSetT.hpp
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
	GThreadGroup.hpp
)

INSTALL ( FILES ${GENEVAINCLUDES} DESTINATION include 
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
