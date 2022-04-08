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
	GErrorCalculation.hpp
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
    G_Interface_Mutable.hpp
    G_Interface_OptimizerT.hpp
    G_Interface_Rateable.hpp
    GMultiConstraintT.hpp
    GMutableParameterI.hpp
	GNumBiGaussAdaptorT.hpp
	GNumGaussAdaptorT.hpp
	GNumCollectionT.hpp
	GNumFlipAdaptorT.hpp
	GNumFPT.hpp
	GNumIntT.hpp
	GNumT.hpp
    G_OptimizationAlgorithm_Base.hpp
    G_OptimizationAlgorithm_EvolutionaryAlgorithm.hpp
    G_OptimizationAlgorithm_EvolutionaryAlgorithm_Factory.hpp
    G_OptimizationAlgorithm_EvolutionaryAlgorithm_PersonalityTraits.hpp
    G_OptimizationAlgorithm_GradientDescent.hpp
    G_OptimizationAlgorithm_GradientDescent_Factory.hpp
    G_OptimizationAlgorithm_GradientDescent_PersonalityTraits.hpp
    G_OptimizationAlgorithm_ParChild.hpp
    G_OptimizationAlgorithm_ParameterScan.hpp
    G_OptimizationAlgorithm_ParameterScan_Factory.hpp
    G_OptimizationAlgorithm_ParameterScan_PersonalityTraits.hpp
    G_OptimizationAlgorithm_SimulatedAnnealing.hpp
    G_OptimizationAlgorithm_SimulatedAnnealing_Factory.hpp
    G_OptimizationAlgorithm_SimulatedAnnealing_PersonalityTraits.hpp
    G_OptimizationAlgorithm_SwarmAlgorithm.hpp
    G_OptimizationAlgorithm_SwarmAlgorithm_Factory.hpp
    G_OptimizationAlgorithm_SwarmAlgorithm_PersonalityTraits.hpp
    G_OA_ConjugateGradientDescent.hpp
    G_OptimizationAlgorithm_FactoryT.hpp
    G_OptimizationAlgorithm_FactoryStore.hpp
    G_OptimizationAlgorithm_InitializerT.hpp
    G_OptimizationAlgorithm_ParChildT_PersonalityTraits.hpp
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

# This is a workaround for a CLion-problem -- see CPP270 in the JetBrains issue tracker
ADD_CUSTOM_TARGET(CLION_ALL_GENEVA_HEADERS SOURCES ${OPTIMIZATIONINCLUDES})

SET_TARGET_PROPERTIES(
    CLION_ALL_GENEVA_HEADERS
    PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

INSTALL ( FILES ${OPTIMIZATIONINCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/geneva )

IF( GENEVA_BUILD_TESTS )
	ADD_SUBDIRECTORY ( tests )
ENDIF()
