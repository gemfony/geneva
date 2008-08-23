CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET ( GENEVAINCLUDES
	GAsioHelperFunctions.hpp
	GAsioTCPClient.hpp
	GAdaptorT.hpp
	GBasePopulation.hpp
	GBaseClient.hpp
	GBitCollection.hpp
	GBitFlipAdaptor.hpp
	GBit.hpp
	GBoostThreadConsumer.hpp
	GBoostThreadPopulation.hpp
	GBoundedBuffer.hpp
	GBoundedDouble.hpp
	GBroker.hpp
	GBrokerPopulation.hpp
	GBufferPort.hpp
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
	GRateableI.hpp
	GSerializableI.hpp
	GServerSession.hpp
	GThreadGroup.hpp
)

INSTALL ( FILES ${GENEVAINCLUDES} DESTINATION include 
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
