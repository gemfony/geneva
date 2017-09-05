CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( OPTIMIZATIONINCLUDES
	GAdaptorT.hpp
	GBaseEA.hpp
	GBasePS.hpp
	GBaseParChildPersonalityTraits.hpp
	GBaseParChildT.hpp
	GBaseGD.hpp
	GBaseSA.hpp
	GBaseSwarm.hpp
	GBooleanAdaptor.hpp
	GBooleanCollection.hpp
	GBooleanObject.hpp
	GBooleanObjectCollection.hpp
	GBrokerEA.hpp
	GBrokerGD.hpp
	GBrokerPS.hpp
	GBrokerSA.hpp
	GBrokerSwarm.hpp
	GConstrainedDoubleCollection.hpp
	GConstrainedDoubleObject.hpp
	GConstrainedDoubleObjectCollection.hpp
	GConstrainedFPT.hpp
	GConstrainedFPNumCollectionT.hpp
	GConstrainedInt32Object.hpp
	GConstrainedInt32ObjectCollection.hpp
	GConstrainedIntT.hpp
	GConstrainedNumCollectionT.hpp
	GConstrainedNumT.hpp
	GConstrainedValueLimitT.hpp
	GConsumerStore.hpp
	GDoubleBiGaussAdaptor.hpp
	GDoubleCollection.hpp
	GDoubleGaussAdaptor.hpp
	GDoubleObject.hpp
	GDoubleObjectCollection.hpp
	GEAPersonalityTraits.hpp
	GenevaHelperFunctionsT.hpp
	GenevaInitializer.hpp
	GEvolutionaryAlgorithmFactory.hpp
	GFPBiGaussAdaptorT.hpp
	GFPGaussAdaptorT.hpp
	GFPNumCollectionT.hpp
	GGDPersonalityTraits.hpp
	GGradientDescentFactory.hpp
	GIndividualMultiConstraint.hpp
	GIndividualStandardConsumerInitializerT.hpp
	GIndividualStandardConsumers.hpp
	GInt32Collection.hpp
	GInt32FlipAdaptor.hpp
	GInt32GaussAdaptor.hpp
	GInt32Object.hpp
	GInt32ObjectCollection.hpp
	GIntFlipAdaptorT.hpp
	GIntGaussAdaptorT.hpp
	GIntNumCollectionT.hpp
    GMultiConstraintT.hpp
	GMultiThreadedEA.hpp
	GMultiThreadedGD.hpp
	GMultiThreadedPS.hpp
	GMultiThreadedSA.hpp
	GMultiThreadedSwarm.hpp
	GMutableI.hpp
    GMutableParameterI.hpp
	GMutableSetT.hpp
	GNumBiGaussAdaptorT.hpp
	GNumGaussAdaptorT.hpp
	GNumCollectionT.hpp
	GNumFlipAdaptorT.hpp
	GNumFPT.hpp
	GNumIntT.hpp
	GNumT.hpp
	GOAInitializerT.hpp
	Go2.hpp
	GOAFactoryStore.hpp
    GObject.hpp
	GOptimizableEntity.hpp
	GOptimizableI.hpp
	GOptimizationAlgorithmT.hpp
	GOptimizationAlgorithmFactoryT.hpp
	GOptimizationEnums.hpp
	GParameterBase.hpp
	GParameterBaseWithAdaptorsT.hpp
	GParameterCollectionT.hpp
	GParameterObjectCollection.hpp
	GParameterPropertyParser.hpp
	GParameterScanFactory.hpp
	GParameterSet.hpp
    GParameterSetFactory.hpp
	GParameterSetParChild.hpp
	GParameterSetFixedSizePriorityQueue.hpp
	GParameterSetMultiConstraint.hpp
	GParameterT.hpp
	GParameterTCollectionT.hpp
	GPersonalityTraits.hpp
    GPluggableOptimizationMonitors.hpp
    GPostProcessorT.hpp
	GPSPersonalityTraits.hpp
	GRateableI.hpp
	GSAPersonalityTraits.hpp
	GScanPar.hpp
	GSerialEA.hpp
	GSerialGD.hpp
	GSerialPS.hpp
	GSerialSA.hpp
	GSerialSwarm.hpp
	GSimulatedAnnealingFactory.hpp
	GSwarmAlgorithmFactory.hpp
	GSwarmPersonalityTraits.hpp
	GTestIndividual1.hpp
)

# This is a workaround for a CLion-problem -- see CPP270 in the JetBrains issue tracker
ADD_CUSTOM_TARGET(CLION_ALL_GENEVA_HEADERS SOURCES ${OPTIMIZATIONINCLUDES})

SET_TARGET_PROPERTIES(
    CLION_ALL_GENEVA_HEADERS
    PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED 14
)

INSTALL ( FILES ${OPTIMIZATIONINCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/geneva )

IF( GENEVA_BUILD_TESTS )
	ADD_SUBDIRECTORY ( tests )
ENDIF()
