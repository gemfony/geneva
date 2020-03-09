CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET ( PICFILES
	ml.ppm
)

# Copy the config files to the build tree for easier testing
FILE ( COPY ${PICFILES} DESTINATION . )

INSTALL ( FILES ${PICFILES} DESTINATION ${INSTALL_PREFIX_DATA}/tests/common/ManualTests/${EXECUTABLENAME}/pictures )
