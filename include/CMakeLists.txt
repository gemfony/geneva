CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

ADD_SUBDIRECTORY (thirdparty)

SET ( GENEVAINCLUDES
	GAsioHelperFunctions.hpp
	GAsioTCPClient.hpp
	GAsioTCPConsumer.hpp
	GAdaptorT.hpp
	GBaseClient.hpp
	GBoolean.hpp
	GBooleanAdaptor.hpp
	GBooleanCollection.hpp
	GBooleanObjectCollection.hpp
	GBoolParameter.hpp
	GBoostThreadConsumer.hpp
	GBoundedBufferT.hpp
	GBoundedBufferWithIdT.hpp
	GBoundedDouble.hpp
	GBoundedDoubleCollection.hpp
	GBoundedInt32.hpp
	GBoundedInt32Collection.hpp
	GBoundedNumT.hpp
	GBrokerEA.hpp
	GBrokerT.hpp
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
	GEAPersonalityTraits.hpp
	GenevaExceptions.hpp
	GEnums.hpp
	GEvolutionaryAlgorithm.hpp
	GGaussAdaptorT.hpp
	GGDPersonalityTraits.hpp
	GGlobalDefines.hpp
	GGlobalOptionsT.hpp
	GHelperFunctionsT.hpp
	GIdentityAdaptorT.hpp
	GIndividualBroker.hpp
	GIndividual.hpp
	GInt32.hpp
	GInt32Collection.hpp
	GInt32FlipAdaptor.hpp
	GInt32GaussAdaptor.hpp
	GInt32ObjectCollection.hpp
	GIntFlipAdaptorT.hpp
	GLongParameter.hpp
	GMultiThreadedEA.hpp
	GMutableI.hpp
	GMutableSetT.hpp
	GNumCollectionT.hpp
	GNumericParameterT.hpp
	GObject.hpp
	GOptimizationAlgorithm.hpp
	GParameterBase.hpp
	GParameterBaseWithAdaptorsT.hpp
	GParameterCollectionT.hpp
	GParameterSet.hpp
	GParameterT.hpp
	GParameterTCollectionT.hpp
	GParameterValuePair.hpp
	GPersonalityTraits.hpp
	GRandom.hpp
	GRandomFactory.hpp
	GRateableI.hpp
	GSeedManager.hpp
	GSerializableI.hpp
	GSerializationHelperFunctions.hpp
	GSingletonT.hpp
	GStdPtrVectorInterfaceT.hpp
	GStdSimpleVectorInterfaceT.hpp
#	GSwarm.hpp
	GSwarmPersonalityTraits.hpp
	GThreadGroup.hpp
)

INSTALL ( FILES ${GENEVAINCLUDES} DESTINATION include 
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
