CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET ( OPTIMIZATIONINCLUDES
	GAdaptorT.hpp
	GBooleanAdaptor.hpp
	GBoolean.hpp
	GBooleanCollection.hpp
	GBooleanObjectCollection.hpp
	GBoundedDouble.hpp
	GBoundedDoubleCollection.hpp
	GBoundedInt32.hpp
	GBoundedInt32Collection.hpp
	GBoundedNumT.hpp
	GBrokerEA.hpp
	GDouble.hpp
	GDoubleCollection.hpp
	GDoubleGaussAdaptor.hpp
	GDoubleObjectCollection.hpp
	GEAPersonalityTraits.hpp
	GEvolutionaryAlgorithm.hpp
	GGaussAdaptorT.hpp
	GGDPersonalityTraits.hpp
	GIdentityAdaptorT.hpp
	GIndividual.hpp
	GIndividualBroker.hpp
	GIndividualSerializationHelperFunctions.hpp
	GInt32.hpp
	GInt32Collection.hpp
	GInt32FlipAdaptor.hpp
	GInt32GaussAdaptor.hpp
	GInt32ObjectCollection.hpp
	GIntFlipAdaptorT.hpp
	GMultiThreadedEA.hpp
	GMutableI.hpp
	GMutableSetT.hpp
	GNumCollectionT.hpp
	GObject.hpp
	GObjectExpectationChecksT.hpp
	GOptimizationAlgorithm.hpp
	GParameterBase.hpp
	GParameterBaseWithAdaptorsT.hpp
	GParameterCollectionT.hpp
	GParameterObjectCollection.hpp
	GParameterSet.hpp
	GParameterT.hpp
	GParameterTCollectionT.hpp
	GPersonalityTraits.hpp
	GRateableI.hpp
	GSerializableI.hpp
	GStdSimpleVectorInterfaceT.hpp
	GStdPtrVectorInterfaceT.hpp
#	GSwarm.hpp
	GSwarmAdaptor.hpp
	GSwarmPersonalityTraits.hpp
)

INSTALL ( FILES ${OPTIMIZATIONINCLUDES} DESTINATION include/optimization
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
