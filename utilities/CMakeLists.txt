CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Define the Geneva root in case we compile the associated programs together with the main library
SET( GENEVA_ROOT "${PROJECT_SOURCE_DIR}" )

ADD_CUSTOM_TARGET( "utilities"
	# Add all the available example's targets here
	DEPENDS gcfc
	COMMENT "Building the Geneva utilities."
)

ADD_SUBDIRECTORY (
  GConfigurationFileCreator
)
