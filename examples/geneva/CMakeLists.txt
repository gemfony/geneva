CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Define the Geneva root in case we compile the associated programs together with the main library
SET( GENEVA_ROOT "${PROJECT_SOURCE_DIR}" )

ADD_CUSTOM_TARGET( "examples_geneva"
	# Add all the available example's targets here
	DEPENDS 01_GSimpleOptimizer 02_GParaboloid2D 03_GParameterObjectUsagePatterns 04_GMultiCriterionParabola 06_GSimpleEA 06_GSimpleGD 06_GSimpleSwarm 07_GParaboloid 08_GMultiPopulation 09_GNeuralNetwork
	COMMENT "Building the Geneva optimization examples."
)

ADD_SUBDIRECTORY (
  01_GSimpleOptimizer
)

ADD_SUBDIRECTORY (
  02_GParaboloid2D
)

ADD_SUBDIRECTORY (
  03_GParameterObjectUsagePatterns
)

ADD_SUBDIRECTORY (
  04_GMultiCriterionParabola
)

ADD_SUBDIRECTORY (
	06_GDirectEA
)

ADD_SUBDIRECTORY (
	06_GDirectSwarm
)

ADD_SUBDIRECTORY (
  07_GFunctionMinimizer
)

ADD_SUBDIRECTORY (
  08_GMultiPopulation
)

ADD_SUBDIRECTORY (
  09_GNeuralNetwork
)

ADD_SUBDIRECTORY (
  10_GStarter
)

ADD_SUBDIRECTORY (
  11_GMetaOptimizer
)

ADD_SUBDIRECTORY (
  12_GExternalEvaluator
)