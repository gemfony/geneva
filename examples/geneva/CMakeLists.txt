CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET( "examples-geneva"
	# Add all the available example's targets (EXECUTABLENAMEs) here
	DEPENDS GSimpleOptimizer 
		GParaboloid2D 
		GParameterObjectUsagePatterns 
		GMultiCriterionParabola 
		GDirectEA 
		GDirectSwarm 
		GFunctionMinimizer 
		GMultiPopulation 
		GNeuralNetwork
		GStarter
		GMetaOptimizer
		GExternalEvaluator
		GPluggableOptimizationMonitors
		GDependentConstraints
	COMMENT "Building all the examples for the Optimization library."
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
