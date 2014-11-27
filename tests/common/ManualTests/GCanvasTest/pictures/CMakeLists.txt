CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( PICFILES
	ml.ppm
)

# Copy the config files to the build tree for easier testing
FOREACH ( file IN LISTS PICFILES )
	CONFIGURE_FILE ( ${file} ${file} COPYONLY )
ENDFOREACH ()

INSTALL ( FILES ${PICFILES} DESTINATION tests/common/ManualTests/${EXECUTABLENAME}/pictures )
