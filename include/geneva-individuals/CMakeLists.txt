CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET ( SAMPLEINDIVIDUALINCLUDES
	GDelayIndividual.hpp
	GExternalEvaluatorIndividual.hpp
	GFunctionIndividual.hpp
	GIndividualFactoryT.hpp
	GNeuralNetworkIndividual.hpp
# GOpenCLIndividual.hpp
	GTestIndividual1.hpp
)

SET ( SAMPLEINDIVIDUALCONFIGS
  GFunctionIndividual.cfg
)

INSTALL ( FILES ${SAMPLEINDIVIDUALINCLUDES} DESTINATION include/geneva-individuals
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)

INSTALL ( FILES ${SAMPLEINDIVIDUALCONFIGS} DESTINATION include/geneva-individuals
  PERMISSIONS
  OWNER_READ OWNER_WRITE
  GROUP_READ
  WORLD_READ
)