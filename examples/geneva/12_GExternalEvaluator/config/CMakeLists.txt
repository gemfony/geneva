CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET ( CONFIGFILES
	GEvolutionaryAlgorithm.json
	GExternalEvaluatorIndividual.json
	Go2.json
)

# Copy the config files to the build tree for easier testing
FILE ( COPY ${CONFIGFILES} DESTINATION . )

INSTALL ( FILES ${CONFIGFILES} DESTINATION ${INSTALL_PREFIX_DATA}/examples/${EXECUTABLENAME}/config )
