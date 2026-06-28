SET ( OPTIMIZATIONINCLUDES
    GenevaHelperFunctions.hpp
	GenevaInitializer.hpp
	GIndividualMultiConstraint.hpp
	GOptimizableEntityCommandContainerExport.hpp
    GMultiConstraintT.hpp
	Go2.hpp
	GConsumerSetup.hpp
    GOptimizationEnums.hpp
    GSigHupHandler.hpp
	GPersonalityTraits.hpp
    GPluggableOptimizationMonitors.hpp
    GPostProcessorT.hpp
)

# Parameter / adaptor object hierarchy (namespace Gem::Geneva::Genome),
# installed under geneva/par to mirror the `gen` alias.
SET ( PARAM_INCLUDES
    par/GParameterPropertyParser.hpp
    par/GOptimizableEntityFactory.hpp
    par/GOptimizableEntityFixedSizePriorityQueue.hpp
    par/GOptimizableEntityMultiConstraint.hpp
)

# Individual / genome headers (namespace Gem::Geneva::Genome), installed
# under geneva/ind. This is the public flat-genome authoring + storage API:
# the abstract individual base, the sole flat genome, the genome builder /
# layout / architecture / factory, the auxiliary store and the adaption
# kernels. All are pulled in transitively by the installed individuals/*.hpp,
# so every one must ship.
SET ( IND_INCLUDES
    ind/GAuxiliaryStore.hpp
    ind/GIndividualProcessingResult.hpp
    ind/GProblemPolicy.hpp
    ind/GFlatGenomeT.hpp
    ind/GOptimizableEntity.hpp
    ind/GAdaptionAuxKeys.hpp
    ind/GAdaptionKernels.hpp
    ind/GGenomeLayout.hpp
    ind/GGenomeLayoutSerialization.hpp
    ind/GGenomeBuilder.hpp
    ind/GGenomeArchitecture.hpp
    ind/GFlatGenome.hpp
    ind/GFlatIndividualFactory.hpp
)

# Optimization-algorithm headers (namespace Gem::Geneva::OptimizationAlgorithms),
# installed under geneva/oa to mirror the `oa` alias.
SET ( OA_INCLUDES
    oa/GOptimizationAlgorithmBase.hpp
    oa/GOptimizationAlgorithmT.hpp
    oa/GOptimizationAlgorithmFactoryT.hpp
    oa/GParChild.hpp
    oa/GAdaptionConfig.hpp
    oa/GParetoTools.hpp
    oa/GAdaption.hpp
    oa/GInitializerT.hpp
    oa/GOAFactoryT.hpp
    oa/GFactoryStore.hpp
    oa/GBaseParChildPersonalityTraits.hpp
    oa/GConjugateGradientDescent.hpp
    oa/GConjugateGradientDescentFactory.hpp
    oa/GConjugateGradientDescent_PersonalityTraits.hpp
    oa/GHesseError.hpp
    oa/GLineSearch.hpp
    oa/GEvolutionaryAlgorithm.hpp
    oa/GEvolutionaryAlgorithmFactory.hpp
    oa/GEvolutionaryAlgorithm_PersonalityTraits.hpp
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
    oa/GSepCmaEvolutionStrategy.hpp
    oa/GSepCmaEvolutionStrategyFactory.hpp
    oa/GSepCmaEvolutionStrategy_PersonalityTraits.hpp
    oa/GStandardPSO2011.hpp
    oa/GStandardPSO2011Factory.hpp
    oa/GStandardPSO2011_PersonalityTraits.hpp
    oa/GAntColonyOptimization.hpp
    oa/GAntColonyOptimizationFactory.hpp
    oa/GAntColonyOptimization_PersonalityTraits.hpp
    oa/GGeneralizedSimulatedAnnealing.hpp
    oa/GGeneralizedSimulatedAnnealingFactory.hpp
    oa/GGeneralizedSimulatedAnnealing_PersonalityTraits.hpp
)

# Interface headers (namespace Gem::Geneva::Interface), installed under
# geneva/Interface to mirror the nested `Interface` namespace.
SET ( IFACE_INCLUDES
    Interface/GMutableI.hpp
    Interface/GRateableI.hpp
    Interface/GOptimizerIT.hpp
)

# Reusable sample individuals (namespace Gem::Geneva::Individuals),
# installed under geneva/individuals to mirror the `gind` alias. The
# former geneva-individuals library was dissolved into the geneva
# library; these headers live on here.
SET ( INDIVIDUALS_INCLUDES
    individuals/GBenchmarkFunctions.hpp
    individuals/GDelayIndividual.hpp
    individuals/GExternalEvaluatorIndividual.hpp
    individuals/GFunctionIndividual.hpp
    individuals/GLineFitIndividual.hpp
    individuals/GMetaOptimizerIndividualT.hpp
    individuals/GNeuralNetworkIndividual.hpp
    individuals/GTestIndividual1.hpp
    individuals/GTestIndividual2.hpp
    individuals/GTestIndividual3.hpp
)

# Add MPI specific files
IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
	SET(OPTIMIZATIONINCLUDES
			${OPTIMIZATIONINCLUDES}
			GMPISubClientOptimizer.hpp
			GMPISubClientIndividual.hpp)
ENDIF()

# This is a workaround for a CLion-problem -- see CPP270 in the JetBrains issue tracker
ADD_CUSTOM_TARGET(CLION_ALL_GENEVA_HEADERS SOURCES ${OPTIMIZATIONINCLUDES} ${OA_INCLUDES} ${IFACE_INCLUDES} ${PARAM_INCLUDES} ${IND_INCLUDES} ${INDIVIDUALS_INCLUDES})

SET_TARGET_PROPERTIES(
    CLION_ALL_GENEVA_HEADERS
    PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

INSTALL ( FILES ${OPTIMIZATIONINCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/geneva )
INSTALL ( FILES ${OA_INCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/geneva/oa )
INSTALL ( FILES ${IFACE_INCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/geneva/Interface )
INSTALL ( FILES ${PARAM_INCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/geneva/par )
INSTALL ( FILES ${IND_INCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/geneva/ind )
INSTALL ( FILES ${INDIVIDUALS_INCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/geneva/individuals )

# GEqualityPrinter.hpp (the only former include/geneva/tests/ header) is
# pure test scaffolding and now lives next to its sole consumer in
# tests/geneva/UnitTests/ (internal only, never installed) — like
# Geneva_tests.hpp before it.
