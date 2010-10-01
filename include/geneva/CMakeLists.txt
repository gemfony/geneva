CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET ( OPTIMIZATIONINCLUDES
	GAdaptorT.hpp
	GBooleanAdaptor.hpp
	GBooleanCollection.hpp
	GBooleanObject.hpp
	GBooleanObjectCollection.hpp
	GBrokerConnector.hpp
	GBrokerEA.hpp
	GBrokerGD.hpp
	GBrokerSwarm.hpp
	GConstrainedDoubleObject.hpp
	GConstrainedDoubleObjectCollection.hpp
	GConstrainedFPT.hpp
	GConstrainedInt32Object.hpp
	GConstrainedInt32ObjectCollection.hpp
	GConstrainedIntT.hpp
#	GConstrainedNumCollectionT.hpp
	GConstrainedNumT.hpp
	GConstrainedValueLimit.hpp
	GDoubleCollection.hpp
	GDoubleGaussAdaptor.hpp
	GDoubleObject.hpp
	GDoubleObjectCollection.hpp
	GEAPersonalityTraits.hpp
	GEvolutionaryAlgorithm.hpp
	GFPNumCollectionT.hpp
	GGDPersonalityTraits.hpp
	GGradientDescent.hpp
	GIndividual.hpp
	GInt32Collection.hpp
	GInt32FlipAdaptor.hpp
	GInt32GaussAdaptor.hpp
	GInt32Object.hpp
	GInt32ObjectCollection.hpp
	GIntFlipAdaptorT.hpp
	GIntNumCollectionT.hpp
	GMultiThreadedEA.hpp
	GMultiThreadedGD.hpp
	GMultiThreadedSwarm.hpp
	GMutableI.hpp
	GMutableSetT.hpp
	GNumGaussAdaptorT.hpp
	GNumCollectionT.hpp
	GNumFPT.hpp
	GNumIntT.hpp
	GNumT.hpp
	GObject.hpp
	GObjectExpectationChecksT.hpp
	GOptimizationAlgorithmT.hpp
	GOptimizationEnums.hpp
	GOptimizationMonitorT.hpp
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
	GSwarm.hpp
	GSwarmPersonalityTraits.hpp
)

INSTALL ( FILES ${OPTIMIZATIONINCLUDES} DESTINATION include/geneva
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
