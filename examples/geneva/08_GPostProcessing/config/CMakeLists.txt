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
FILE ( COPY ${CONFIGFILES} DESTINATION . )

INSTALL ( FILES ${CONFIGFILES} DESTINATION ${INSTALL_PREFIX_DATA}/examples/${EXECUTABLENAME}/config )
