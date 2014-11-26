CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( CONFIGFILES
	GEvolutionaryAlgorithm.json
	GFunctionIndividual.json
	Go2.json
	GOptimizationBenchmark.json
)

# Copy the config files to the build tree for easier testing
FOREACH ( file IN LISTS CONFIGFILES )
	CONFIGURE_FILE ( ${file} ${file} COPYONLY )
ENDFOREACH ()

INSTALL ( FILES ${CONFIGFILES} DESTINATION examples/${PROJECT_NAME} )
