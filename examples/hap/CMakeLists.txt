CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Define the Geneva root in case we compile the associated programs together with the main library
SET( GENEVA_ROOT "${PROJECT_SOURCE_DIR}" )

ADD_CUSTOM_TARGET( "examples_hap"
	# Add all the available example's targets here
	DEPENDS 05_GHapUsagePatterns
	COMMENT "Building the Geneva hap examples."
)

ADD_SUBDIRECTORY (
  05_GHapUsagePatterns
)
