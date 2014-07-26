CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Define the Geneva root in case we compile the associated programs together with the main library
SET( GENEVA_ROOT "${PROJECT_SOURCE_DIR}" )

ADD_SUBDIRECTORY (
	common
)

ADD_SUBDIRECTORY (
	geneva
)

ADD_SUBDIRECTORY (
	hap
)