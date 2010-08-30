CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET ( OPTIMIZATIONINCLUDES
	GAdaptorT.hpp
	GBooleanAdaptor.hpp
	GBoolean.hpp
	GBooleanCollection.hpp
	GBooleanObjectCollection.hpp
	GBrokerConnector.hpp
	GBrokerEA.hpp
	GBrokerSwarm.hpp
	GConstrainedDouble.hpp
	GConstrainedDoubleObjectCollection.hpp
	GConstrainedFPT.hpp
	GConstrainedInt32.hpp
	GConstrainedInt32ObjectCollection.hpp
	GConstrainedIntegerT.hpp
#	GConstrainedNumCollectionT.hpp
	GConstrainedNumT.hpp
	GDouble.hpp
	GDoubleCollection.hpp
	GDoubleGaussAdaptor.hpp
	GDoubleObjectCollection.hpp
	GEAPersonalityTraits.hpp
	GEvolutionaryAlgorithm.hpp
	GGaussAdaptorT.hpp
	GGDPersonalityTraits.hpp
	GIndividual.hpp
	GInt32.hpp
	GInt32Collection.hpp
	GInt32FlipAdaptor.hpp
	GInt32GaussAdaptor.hpp
	GInt32ObjectCollection.hpp
	GIntFlipAdaptorT.hpp
	GMultiThreadedEA.hpp
	GMultiThreadedSwarm.hpp
	GMutableI.hpp
	GMutableSetT.hpp
	GNumCollectionFPT.hpp
	GNumCollectionT.hpp
	GNumFPT.hpp
	GNumT.hpp
	GObject.hpp
	GObjectExpectationChecksT.hpp
	GOptimizationAlgorithmT.hpp
	GOptimizationEnums.hpp
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
