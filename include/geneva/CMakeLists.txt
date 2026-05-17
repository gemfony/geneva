SET ( OPTIMIZATIONINCLUDES
	GConsumerStore.hpp
	GenevaHelperFunctionsT.hpp
    GenevaHelperFunctions.hpp
	GenevaInitializer.hpp
	GIndividualMultiConstraint.hpp
	GIndividualStandardConsumerInitializerT.hpp
	GIndividualStandardConsumers.hpp
    GMultiConstraintT.hpp
	Go2.hpp
    GObject.hpp
    GOptimizationEnums.hpp
	GPersonalityTraits.hpp
    GPluggableOptimizationMonitors.hpp
    GPostProcessorT.hpp
	GTestIndividual1.hpp
)

# Parameter / adaptor object hierarchy (namespace Gem::Geneva::Parameters),
# installed under geneva/par to mirror the `gpar` alias.
SET ( PARAM_INCLUDES
    par/GAdaptorT.hpp
    par/GBooleanAdaptor.hpp
    par/GBooleanCollection.hpp
    par/GBooleanObject.hpp
    par/GBooleanObjectCollection.hpp
    par/GConstrainedDoubleCollection.hpp
    par/GConstrainedDoubleObject.hpp
    par/GConstrainedDoubleObjectCollection.hpp
    par/GConstrainedFPNumCollectionT.hpp
    par/GConstrainedFPT.hpp
    par/GConstrainedInt32Object.hpp
    par/GConstrainedInt32ObjectCollection.hpp
    par/GConstrainedIntT.hpp
    par/GConstrainedNumCollectionT.hpp
    par/GConstrainedNumT.hpp
    par/GConstrainedValueLimitT.hpp
    par/GDoubleBiGaussAdaptor.hpp
    par/GDoubleCollection.hpp
    par/GDoubleGaussAdaptor.hpp
    par/GDoubleObject.hpp
    par/GDoubleObjectCollection.hpp
    par/GFPBiGaussAdaptorT.hpp
    par/GFPGaussAdaptorT.hpp
    par/GFPNumCollectionT.hpp
    par/GInt32Collection.hpp
    par/GInt32FlipAdaptor.hpp
    par/GInt32GaussAdaptor.hpp
    par/GInt32Object.hpp
    par/GInt32ObjectCollection.hpp
    par/GIntFlipAdaptorT.hpp
    par/GIntGaussAdaptorT.hpp
    par/GIntNumCollectionT.hpp
    par/GMutableParameterI.hpp
    par/GNumBiGaussAdaptorT.hpp
    par/GNumCollectionT.hpp
    par/GNumFlipAdaptorT.hpp
    par/GNumFPT.hpp
    par/GNumGaussAdaptorT.hpp
    par/GNumIntT.hpp
    par/GNumT.hpp
    par/GParameterBase.hpp
    par/GParameterBaseWithAdaptorsT.hpp
    par/GParameterCollectionT.hpp
    par/GParameterObjectCollection.hpp
    par/GParameterPropertyParser.hpp
    par/GParameterSet.hpp
    par/GParameterSetFactory.hpp
    par/GParameterSetFixedSizePriorityQueue.hpp
    par/GParameterSetMultiConstraint.hpp
    par/GParameterT.hpp
    par/GParameterTCollectionT.hpp
)

# Optimization-algorithm headers (namespace Gem::Geneva::OptimizationAlgorithms),
# installed under geneva/oa to mirror the `oa` alias.
SET ( OA_INCLUDES
    oa/GBase.hpp
    oa/GParChild.hpp
    oa/GInitializerT.hpp
    oa/GOAFactoryT.hpp
    oa/GFactoryStore.hpp
    oa/GBaseParChildPersonalityTraits.hpp
    oa/GConjugateGradientDescent.hpp
    oa/GConjugateGradientDescentFactory.hpp
    oa/GConjugateGradientDescent_PersonalityTraits.hpp
    oa/GEvolutionaryAlgorithm.hpp
    oa/GEvolutionaryAlgorithmFactory.hpp
    oa/GEvolutionaryAlgorithm_PersonalityTraits.hpp
    oa/GGradientDescent.hpp
    oa/GGradientDescentFactory.hpp
    oa/GGradientDescent_PersonalityTraits.hpp
    oa/GNelderMead.hpp
    oa/GNelderMeadFactory.hpp
    oa/GNelderMead_PersonalityTraits.hpp
    oa/GParameterScan.hpp
    oa/GParameterScanFactory.hpp
    oa/GParameterScan_PersonalityTraits.hpp
    oa/GSimulatedAnnealing.hpp
    oa/GSimulatedAnnealingFactory.hpp
    oa/GSimulatedAnnealing_PersonalityTraits.hpp
    oa/GSwarmAlgorithm.hpp
    oa/GSwarmAlgorithmFactory.hpp
    oa/GSwarmAlgorithm_PersonalityTraits.hpp
)

# Interface headers (namespace Gem::Geneva::Interface),
# installed under geneva/iface to mirror the `iface` alias.
SET ( IFACE_INCLUDES
    iface/GMutableI.hpp
    iface/GRateableI.hpp
    iface/GOptimizerIT.hpp
)

# Add MPI specific files
IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
	SET(OPTIMIZATIONINCLUDES
			${OPTIMIZATIONINCLUDES}
			GMPISubClientOptimizer.hpp
			GMPISubClientIndividual.hpp)
ENDIF()

# This is a workaround for a CLion-problem -- see CPP270 in the JetBrains issue tracker
ADD_CUSTOM_TARGET(CLION_ALL_GENEVA_HEADERS SOURCES ${OPTIMIZATIONINCLUDES} ${OA_INCLUDES} ${IFACE_INCLUDES} ${PARAM_INCLUDES})

SET_TARGET_PROPERTIES(
    CLION_ALL_GENEVA_HEADERS
    PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

INSTALL ( FILES ${OPTIMIZATIONINCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/geneva )
INSTALL ( FILES ${OA_INCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/geneva/oa )
INSTALL ( FILES ${IFACE_INCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/geneva/iface )
INSTALL ( FILES ${PARAM_INCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/geneva/par )

IF( GENEVA_BUILD_TESTS )
	ADD_SUBDIRECTORY ( tests )
ENDIF()
