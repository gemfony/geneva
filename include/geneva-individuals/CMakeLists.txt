CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( SAMPLEINDIVIDUALINCLUDES
	GExternalEvaluatorIndividual.hpp
	GFunctionIndividual.hpp
	GMetaOptimizerIndividualT.hpp
	GNeuralNetworkIndividual.hpp
	GTestIndividual2.hpp
	GTestIndividual3.hpp
)

SET ( SAMPLEINDIVIDUALCONFIGS
)

# Copy the config files to the build tree for easier testing
FOREACH ( file IN LISTS SAMPLEINDIVIDUALCONFIGS )
	CONFIGURE_FILE ( ${file} . COPYONLY )
ENDFOREACH ()

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
