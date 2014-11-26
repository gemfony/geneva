CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET( "examples_geneva"
	# Add all the available example's targets here
	DEPENDS 01_GSimpleOptimizer 
		02_GParaboloid2D 
		03_GParameterObjectUsagePatterns 
		04_GMultiCriterionParabola 
		06_GDirectEA 
		06_GDirectSwarm 
		07_GFunctionMinimizer 
		08_GMultiPopulation 
		09_GNeuralNetwork
		10_GStarter
		11_GMetaOptimizer
		12_GExternalEvaluator
		13_GPluggableOptimizationMonitors
		14_GDependentConstraints
	COMMENT "Building the Geneva examples for the optimization library."
)

ADD_SUBDIRECTORY( 01_GSimpleOptimizer )
ADD_SUBDIRECTORY( 02_GParaboloid2D )
ADD_SUBDIRECTORY( 03_GParameterObjectUsagePatterns )
ADD_SUBDIRECTORY( 04_GMultiCriterionParabola )
ADD_SUBDIRECTORY( 06_GDirectEA )
ADD_SUBDIRECTORY( 06_GDirectSwarm )
ADD_SUBDIRECTORY( 07_GFunctionMinimizer )
ADD_SUBDIRECTORY( 08_GMultiPopulation )
ADD_SUBDIRECTORY( 09_GNeuralNetwork )
ADD_SUBDIRECTORY( 10_GStarter )
ADD_SUBDIRECTORY( 11_GMetaOptimizer )
ADD_SUBDIRECTORY( 12_GExternalEvaluator )
ADD_SUBDIRECTORY( 13_GPluggableOptimizationMonitors )
ADD_SUBDIRECTORY( 14_GDependentConstraints )
