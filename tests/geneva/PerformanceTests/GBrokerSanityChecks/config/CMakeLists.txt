CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( CONFIGFILES
	GDelayIndividual.json
	GEvolutionaryAlgorithm.json
	Go2.json
)

# Copy the config files to the build tree for easier testing
FILE ( COPY ${CONFIGFILES} DESTINATION . )

INSTALL ( FILES ${CONFIGFILES} DESTINATION ${INSTALL_PREFIX_DATA}/tests/geneva/PerformanceTests/${EXECUTABLENAME}/config )
