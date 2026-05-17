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
    GMultiConstraintT.hpp
    GMutableParameterI.hpp
	GNumBiGaussAdaptorT.hpp
	GNumGaussAdaptorT.hpp
	GNumCollectionT.hpp
	GNumFlipAdaptorT.hpp
	GNumFPT.hpp
	GNumIntT.hpp
	GNumT.hpp
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
ADD_CUSTOM_TARGET(CLION_ALL_GENEVA_HEADERS SOURCES ${OPTIMIZATIONINCLUDES} ${OA_INCLUDES} ${IFACE_INCLUDES})

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

IF( GENEVA_BUILD_TESTS )
	ADD_SUBDIRECTORY ( tests )
ENDIF()
