CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Define the Geneva root in case we compile the associated programs together with the main library
SET( GENEVA_ROOT "${PROJECT_SOURCE_DIR}" )

ADD_CUSTOM_TARGET( "examples_common"
	# Add all the available example's targets here
	DEPENDS GConfigFileCreation
	COMMENT "Building the Geneva examples."
)

ADD_SUBDIRECTORY (
  GConfigFileCreation
)
