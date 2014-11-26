CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_TARGET( "examples_common"
	# Add all the available example's targets here
	DEPENDS GConfigFileCreation
	COMMENT "Building the Geneva examples for the Common library."
)

ADD_SUBDIRECTORY( GConfigFileCreation )
