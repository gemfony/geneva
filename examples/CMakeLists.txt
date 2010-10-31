CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Define the Geneva root in case we compile the associated programs together with the main library
SET( GENEVA_ROOT "${PROJECT_SOURCE_DIR}" )

ADD_CUSTOM_TARGET( "examples"
	# Add all the available example's targets here
	DEPENDS 01_Optimizer GMultiPopulation GSimpleEA GSimpleSwarm GExternalEvaluator GNeuralNetwork GStartProject
	COMMENT "Building the Geneva examples."
)

ADD_SUBDIRECTORY (
	01_Optimizer
)

ADD_SUBDIRECTORY (
	SampleIndividuals
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
	GExternalEvaluator
)

ADD_SUBDIRECTORY (
	GNeuralNetwork
)

ADD_SUBDIRECTORY (
	GStartProject
)
