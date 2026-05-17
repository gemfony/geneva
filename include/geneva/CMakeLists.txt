SET ( OPTIMIZATIONINCLUDES
	GAdaptorT.hpp
	GBooleanAdaptor.hpp
	GBooleanCollection.hpp
	GBooleanObject.hpp
	GBooleanObjectCollection.hpp
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
	GenevaHelperFunctionsT.hpp
    GenevaHelperFunctions.hpp
	GenevaInitializer.hpp
	GFPBiGaussAdaptorT.hpp
	GFPGaussAdaptorT.hpp
	GFPNumCollectionT.hpp
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
    MutableI.hpp
    OptimizerIT.hpp
    RateableI.hpp
    GMultiConstraintT.hpp
    GMutableParameterI.hpp
	GNumBiGaussAdaptorT.hpp
	GNumGaussAdaptorT.hpp
	GNumCollectionT.hpp
	GNumFlipAdaptorT.hpp
	GNumFPT.hpp
	GNumIntT.hpp
	GNumT.hpp
        GBase.hpp
    ConjugateGradientDescent.hpp
    ConjugateGradientDescent_Factory.hpp
    ConjugateGradientDescent_PersonalityTraits.hpp
    EvolutionaryAlgorithm.hpp
    EvolutionaryAlgorithm_Factory.hpp
    EvolutionaryAlgorithm_PersonalityTraits.hpp
    GradientDescent.hpp
    GradientDescent_Factory.hpp
    GradientDescent_PersonalityTraits.hpp
    NelderMead.hpp
    NelderMead_Factory.hpp
    NelderMead_PersonalityTraits.hpp
    GParChild.hpp
    ParameterScan.hpp
    ParameterScan_Factory.hpp
    ParameterScan_PersonalityTraits.hpp
    SimulatedAnnealing.hpp
    SimulatedAnnealing_Factory.hpp
    SimulatedAnnealing_PersonalityTraits.hpp
    SwarmAlgorithm.hpp
    SwarmAlgorithm_Factory.hpp
    SwarmAlgorithm_PersonalityTraits.hpp
    GOAFactoryT.hpp
    FactoryStore.hpp
    GInitializerT.hpp
    ParChildT_PersonalityTraits.hpp
	Go2.hpp
    GObject.hpp
    GOptimizationEnums.hpp
	GParameterBase.hpp
	GParameterBaseWithAdaptorsT.hpp
	GParameterCollectionT.hpp
	GParameterObjectCollection.hpp
	GParameterPropertyParser.hpp
	GParameterSet.hpp
    GParameterSetFactory.hpp
	GParameterSetFixedSizePriorityQueue.hpp
	GParameterSetMultiConstraint.hpp
	GParameterT.hpp
	GParameterTCollectionT.hpp
	GPersonalityTraits.hpp
    GPluggableOptimizationMonitors.hpp
    GPostProcessorT.hpp
	GTestIndividual1.hpp
)

# Add MPI specific files
IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
	SET(OPTIMIZATIONINCLUDES
			${OPTIMIZATIONINCLUDES}
			GMPISubClientOptimizer.hpp
			GMPISubClientIndividual.hpp)
ENDIF()

# This is a workaround for a CLion-problem -- see CPP270 in the JetBrains issue tracker
ADD_CUSTOM_TARGET(CLION_ALL_GENEVA_HEADERS SOURCES ${OPTIMIZATIONINCLUDES})

SET_TARGET_PROPERTIES(
    CLION_ALL_GENEVA_HEADERS
    PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

INSTALL ( FILES ${OPTIMIZATIONINCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/geneva )

IF( GENEVA_BUILD_TESTS )
	ADD_SUBDIRECTORY ( tests )
ENDIF()
