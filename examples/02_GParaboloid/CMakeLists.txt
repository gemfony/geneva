PROJECT (GParaboloid)

CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

# Make sure we first check our local modules directory
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")

# Check Geneva dirs
IF(NOT DEFINED GENEVA_ROOT)
	SET(GENEVA_ROOT "/opt/geneva")
ENDIF()

IF(NOT DEFINED GENEVA_INCLUDE_DIR)
	SET(GENEVA_INCLUDE_DIR "${GENEVA_ROOT}/include")
ENDIF()

SET (Boost_USE_STATIC_LIBS OFF)
SET (LINK_FLAGS "-lpthread")
SET (CMAKE_EXE_LINKER_FLAGS "-lpthread")

SET (Boost_USE_MULTITHREADED ON)

# Set the build type to Debug by default or in case GENEVA_BUILD_TYPE
# was set to Debug. Set it to Release if GENEVA_BUILD_TYPE was set
# accordingly. Complain in all other cases.
IF(NOT DEFINED GENEVA_BUILD_TYPE OR GENEVA_BUILD_TYPE STREQUAL "Release")
	MESSAGE ("Setting the build type to default value Release")
	SET(CMAKE_BUILD_TYPE Release)
ELSEIF(GENEVA_BUILD_TYPE STREQUAL "Debug")
	MESSAGE ("Setting the build type to Debug")
	SET(CMAKE_BUILD_TYPE Debug)
ELSE()
	MESSAGE (FATAL_ERROR "Unkown compilation mode " ${GENEVA_BUILD_TYPE})
ENDIF()

# At least Boost 1.40 must be installed
FIND_PACKAGE( Boost 1.40 REQUIRED COMPONENTS
	date_time
	system
	thread
	serialization
	program_options
	filesystem
)

INCLUDE_DIRECTORIES (
	${PROJECT_SOURCE_DIR}
	${GENEVA_ROOT}/include
	${Boost_INCLUDE_DIRS}
)

LINK_DIRECTORIES (
	${GENEVA_ROOT}/lib
	${Boost_LIBRARY_DIRS}
)

SET ( EXECUTABLENAME GParaboloid )

SET ( ${EXECUTABLENAME}_SRCS
	GParaboloid
	GParaboloidIndividual
)

SET ( CONFIGFILES
	GParaboloid.cfg
	GParaboloidIndividual.cfg
)

ADD_EXECUTABLE(${EXECUTABLENAME}
	${${EXECUTABLENAME}_SRCS}
)

# Copy the config files to the build tree for easier testing
IF( NOT OLD_CMAKE )
	FOREACH ( file IN LISTS CONFIGFILES )
		CONFIGURE_FILE ( ${file} . COPYONLY )
	ENDFOREACH ()
ENDIF()

SET ( COMMON_LIBNAME "gemfony-common" )
SET ( HAP_LIBNAME "gemfony-hap" )
SET ( COMMUNICATION_LIBNAME "gemfony-courtier" )
SET ( GENEVA_LIBNAME "gemfony-geneva" )
SET ( GENEVA_INDIVIDUAL_LIBNAME "gemfony-geneva-individuals")

TARGET_LINK_LIBRARIES (${EXECUTABLENAME}
	${GENEVA_LIBNAME}
	${GENEVA_INDIVIDUAL_LIBNAME}
	${COMMUNICATION_LIBNAME}
	${HAP_LIBNAME}
	${COMMON_LIBNAME}
	${Boost_LIBRARIES}
)

INSTALL ( TARGETS ${EXECUTABLENAME} DESTINATION examples/${PROJECT_NAME}
	PERMISSIONS
	OWNER_READ OWNER_WRITE OWNER_EXECUTE
	GROUP_READ GROUP_EXECUTE
	WORLD_READ WORLD_EXECUTE
)

INSTALL ( FILES ${CONFIGFILES} DESTINATION examples/${PROJECT_NAME}
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
