CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET ( PROGFILES
	evaluator.py
)

# Copy the scripts to the build tree for easier testing
FILE ( COPY ${PROGFILES} DESTINATION . )

INSTALL ( PROGRAMS ${PROGFILES} DESTINATION ${INSTALL_PREFIX_DATA}/examples/${EXECUTABLENAME}/evaluator )
