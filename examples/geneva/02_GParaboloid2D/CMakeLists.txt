PROJECT (GParaboloid2D)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

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
SET (LINK_FLAGS "${LINK_FLAGS} -lpthread")
SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread")

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

# At least Boost 1.41 must be installed
FIND_PACKAGE( Boost 1.41 REQUIRED COMPONENTS
	date_time
	filesystem
	program_options
	serialization
	system
	thread
)

# Check whether we want to activate the test code and build tests, if not done yet
IF ( NOT GENEVA_INFO_MSG AND GENEVA_BUILD_TESTS )
	ADD_DEFINITIONS ("-DGEM_TESTING")
	FIND_PACKAGE( Boost 1.41 REQUIRED COMPONENTS
		test_exec_monitor
	)
ENDIF()

# Print a summary of the build settings if it was not printed already
IF ( NOT GENEVA_INFO_MSG )
	# Print info messages only once
	SET ( GENEVA_INFO_MSG TRUE )
	MESSAGE ("\n========================================")
	MESSAGE ("Building:")
	IF( GENEVA_BUILD_TESTS )
		MESSAGE ("\twith test code")
	ELSE()
		MESSAGE ("\twithout tests")
	ENDIF()
	MESSAGE ("\tin ${CMAKE_BUILD_TYPE} mode")
#	MESSAGE ("\tusing ${MSG_COMPILER} compiler settings")
	MESSAGE ("\twith Boost include file location ${Boost_INCLUDE_DIRS}")
	MESSAGE ("\twith Boost library location ${Boost_LIBRARY_DIRS}")
	MESSAGE ("\twith install prefix ${CMAKE_INSTALL_PREFIX}")
	IF( CMAKE_VERBOSE_MAKEFILE )
		MESSAGE ("\tand verbose CMake output")
	ELSE()
		MESSAGE ("\tand sparse CMake output")
	ENDIF()
	MESSAGE ("========================================\n")
ENDIF()

INCLUDE_DIRECTORIES (
	${PROJECT_SOURCE_DIR}
	${GENEVA_ROOT}/include
	${Boost_INCLUDE_DIRS}
)

LINK_DIRECTORIES (
	${GENEVA_ROOT}/lib
	${Boost_LIBRARY_DIRS}
)

SET ( EXECUTABLENAME GParaboloid2D )

SET ( ${EXECUTABLENAME}_SRCS
	GParaboloid2D
	GParaboloidIndividual2D
)

SET ( CONFIGFILE
	config/GEvolutionaryAlgorithm.json
	config/Go2.json
)

ADD_EXECUTABLE(${EXECUTABLENAME}
	${${EXECUTABLENAME}_SRCS}
)

# Copy the config files to the build tree for easier testing
IF( NOT OLD_CMAKE )
	FOREACH ( file IN LISTS CONFIGFILE )
		CONFIGURE_FILE ( ${file} ${file} COPYONLY )
	ENDFOREACH ()
ENDIF()

SET ( COMMON_LIBNAME "gemfony-common" )
SET ( HAP_LIBNAME "gemfony-hap" )
SET ( COURTIER_LIBNAME "gemfony-courtier" )
SET ( GENEVA_LIBNAME "gemfony-geneva" )
SET ( GENEVA_INDIVIDUAL_LIBNAME "gemfony-geneva-individuals")

TARGET_LINK_LIBRARIES (${EXECUTABLENAME}
	${GENEVA_LIBNAME}
	${GENEVA_INDIVIDUAL_LIBNAME}
	${COURTIER_LIBNAME}
	${HAP_LIBNAME}
	${COMMON_LIBNAME}
	${Boost_LIBRARIES}
)

IF( GENEVA_BUILD_TESTS )
#	ADD_SUBDIRECTORY (Tests)
ENDIF()

ADD_SUBDIRECTORY (config)

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
