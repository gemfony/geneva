CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Define the Geneva root in case we compile the associated programs together with the main library
SET( GENEVA_ROOT "${PROJECT_SOURCE_DIR}" )

ADD_CUSTOM_TARGET( "examples_geneva"
	# Add all the available example's targets here
	DEPENDS 01_GSimpleOptimizer 02_GParaboloid2D 03_GParameterObjectUsagePatterns 04_GMultiCriterionParabola 05_GHapUsagePatterns 06_GParaboloid GMultiPopulation GSimpleEA GSimpleGD GSimpleSwarm GNeuralNetwork GStartProject
	COMMENT "Building the Geneva examples."
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
  05_GHapUsagePatterns
)

ADD_SUBDIRECTORY (
  06_GParaboloid
)

ADD_SUBDIRECTORY (
	GMultiPopulation
)

ADD_SUBDIRECTORY (
	GSimpleEA
)

ADD_SUBDIRECTORY (
	GSimpleGD
)

ADD_SUBDIRECTORY (
	GSimpleSwarm
)

ADD_SUBDIRECTORY (
	GNeuralNetwork
)

ADD_SUBDIRECTORY (
	GStartProject
)
