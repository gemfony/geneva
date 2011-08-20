PROJECT (GSimpleGD)

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

# If the build type CMAKE_BUILD_TYPE is not set already (ie, by the top
# level build configuration or by the user), use GENEVA_BUILD_TYPE
# to set it. Otherwise set 'Debug' as default.
IF(NOT CMAKE_BUILD_TYPE)
	IF(DEFINED GENEVA_BUILD_TYPE)
		IF(GENEVA_BUILD_TYPE STREQUAL "Debug")
			MESSAGE ("Setting the build type to Debug")
			SET(CMAKE_BUILD_TYPE Debug)
		ELSEIF(GENEVA_BUILD_TYPE STREQUAL "Release")
			MESSAGE ("Setting the build type to Release")
			SET(CMAKE_BUILD_TYPE Release)
		ELSE()
			MESSAGE (FATAL_ERROR "Unknown compilation mode " ${GENEVA_BUILD_TYPE})
		ENDIF()
	ELSE()
		MESSAGE ("Setting the build type to default value Debug")
		SET(CMAKE_BUILD_TYPE Debug)
	ENDIF()
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

# Check whether BUILDTESTCODE has been defined. If not, deactivate by default
IF( NOT DEFINED BUILDTESTCODE)
	SET( BUILDTESTCODE FALSE )
ENDIF()

# Check whether we want to activate the test code and build tests
IF( BUILDTESTCODE )
	ADD_DEFINITIONS ("-DGENEVATESTING")
	MESSAGE ("=======================")
	MESSAGE ("Building with test code")
	MESSAGE ("=======================")

	FIND_PACKAGE( Boost 1.40 REQUIRED COMPONENTS
		test_exec_monitor
	)
ENDIF()

INCLUDE_DIRECTORIES (
	${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../SampleIndividuals
	${GENEVA_ROOT}/include
#	${Boost_INCLUDE_DIRS}
)

LINK_DIRECTORIES (
	${GENEVA_ROOT}/lib
	${Boost_LIBRARY_DIRS}
)

SET ( EXECUTABLENAME GSimpleGD )

SET ( ${EXECUTABLENAME}_SRCS
	GArgumentParser
	GSimpleGD
)

SET ( CONFIGFILE
	GSimpleGD.cfg
	GFunctionIndividual.cfg
)

ADD_EXECUTABLE(${EXECUTABLENAME}
	${${EXECUTABLENAME}_SRCS}
)

# Copy the config files to the build tree for easier testing
IF( NOT OLD_CMAKE )
	FOREACH ( file IN LISTS CONFIGFILE )
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

IF( BUILDTESTCODE )
#	ADD_SUBDIRECTORY (Tests)
ENDIF()

INSTALL ( TARGETS ${EXECUTABLENAME} DESTINATION examples/${PROJECT_NAME}
	PERMISSIONS
	OWNER_READ OWNER_WRITE OWNER_EXECUTE
	GROUP_READ GROUP_EXECUTE
	WORLD_READ WORLD_EXECUTE
)

INSTALL ( FILES ${CONFIGFILE} DESTINATION examples/${PROJECT_NAME}
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
