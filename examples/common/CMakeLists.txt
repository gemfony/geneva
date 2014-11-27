CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET( "examples-common"
	# Add all the available example's targets here
	DEPENDS GConfigFileCreation
	COMMENT "Building all the examples for the Common library."
)

ADD_SUBDIRECTORY( GConfigFileCreation )
