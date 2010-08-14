CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Define the Geneva root in case we compile the associated programs together with the main library
SET( GENEVA_ROOT "${PROJECT_SOURCE_DIR}" )

ADD_CUSTOM_TARGET( "examples"
	DEPENDS GMultiPopulation GSimpleEA GExternalEvaluator GNeuralNetwork GStartProject
	COMMENT "Building only the Geneva examples."
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

#ADD_SUBDIRECTORY (
#	GSimpleSwarm
#)

ADD_SUBDIRECTORY (
	GExternalEvaluator
)

ADD_SUBDIRECTORY (
	GNeuralNetwork
)

ADD_SUBDIRECTORY (
	GStartProject
)
