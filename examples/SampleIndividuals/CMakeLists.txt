CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET ( SAMPLEINDIVIDUALSINCLUDES
    GExecIndividual.hpp
	GNeuralNetworkIndividual.hpp
	GNoisyParabolaIndividual.hpp
	GParabolaIndividual.hpp
	GProjectionIndividual.hpp
)

INSTALL ( FILES ${SAMPLEINDIVIDUALSINCLUDES} DESTINATION examples/include 
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
