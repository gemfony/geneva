CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( CONFIGFILES
	GEvolutionaryAlgorithm.json
	GFunctionIndividual.json
	GGradientDescent.json
	Go2.json
	GParameterScan.json
	GSimulatedAnnealing.json
	GSwarmAlgorithm.json
)

# Copy the config files to the build tree for easier testing
FOREACH ( file IN LISTS CONFIGFILES )
	CONFIGURE_FILE ( ${file} ${file} COPYONLY )
ENDFOREACH ()

INSTALL ( FILES ${CONFIGFILES} DESTINATION examples/${PROJECT_NAME}/config )
