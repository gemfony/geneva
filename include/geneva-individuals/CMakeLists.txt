CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET ( SAMPLEINDIVIDUALINCLUDES
	GExternalEvaluatorIndividual.hpp
	GExternalSetterIndividual.hpp
	GFunctionIndividual.hpp
	GNeuralNetworkIndividual.hpp
	GTestIndividual1.hpp
	GTestIndividual2.hpp
	GTestIndividual3.hpp
)

SET ( SAMPLEINDIVIDUALCONFIGS
)

# Copy the config files to the build tree for easier testing
IF( NOT OLD_CMAKE )
	FOREACH ( file IN LISTS SAMPLEINDIVIDUALCONFIGS )
		CONFIGURE_FILE ( ${file} . COPYONLY )
	ENDFOREACH ()
ENDIF()

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
