PROJECT (GExternalEvaluator)

CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

# Make sure we first check our local modules directory
ADD_SUBDIRECTORY (CMakeModules)
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

# At least Boost 1.36 must be installed 
FIND_PACKAGE( Boost 1.36 REQUIRED COMPONENTS
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
	
	FIND_PACKAGE( Boost 1.36 REQUIRED COMPONENTS
		test_exec_monitor		
	)
ENDIF()

INCLUDE_DIRECTORIES (
	${PROJECT_SOURCE_DIR}
	${GENEVA_ROOT}/include
	${GENEVA_ROOT}/include/dataexchange
	${GENEVA_ROOT}/include/random
	${GENEVA_ROOT}/include/communication
	${GENEVA_ROOT}/include/optimization
	${GENEVA_ROOT}/include/thirdparty
#	${Boost_INCLUDE_DIRS}
)

LINK_DIRECTORIES (
	${GENEVA_ROOT}/lib
	${Boost_LIBRARY_DIRS}
)

SET ( EXECUTABLENAME GExternalEvaluator )

SET ( ${EXECUTABLENAME}_SRCS
	GArgumentParser
	GExternalEvaluator
	GExternalEvaluatorEnums
)

SET ( CONFIGFILE
	GExternalEvaluator.cfg
)

ADD_EXECUTABLE(${EXECUTABLENAME}
	${${EXECUTABLENAME}_SRCS}
)

SET (COMMON_LIBNAME "gemfony-common")
SET (DATAEXCHANGE_LIBNAME "gemfony-dataexchange")
SET (RANDOM_LIBNAME "gemfony-random")
SET (COMMUNICATION_LIBNAME "gemfony-communication")

# The name of the optimization lib depends on whether or not it contains test code
IF ( BUILDTESTCODE )
	SET ( OPTIMIZATION_LIBNAME "gemfony-optimization-testing" )
ELSE()
	SET ( OPTIMIZATION_LIBNAME "gemfony-optimization" )
ENDIF()

TARGET_LINK_LIBRARIES (${EXECUTABLENAME}
	${OPTIMIZATION_LIBNAME}
	${COMMUNICATION_LIBNAME}
	${DATAEXCHANGE_LIBNAME}
	${RANDOM_LIBNAME}
	${COMMON_LIBNAME}
	${Boost_LIBRARIES}
)

# Make sure the external evaluator gets compiled
ADD_SUBDIRECTORY (SampleEvaluator)

IF( BUILDTESTCODE )
	ADD_SUBDIRECTORY (Tests)
ENDIF()

INSTALL ( TARGETS ${EXECUTABLENAME} DESTINATION bin
	PERMISSIONS
	OWNER_READ OWNER_WRITE OWNER_EXECUTE
	GROUP_READ GROUP_EXECUTE
	WORLD_READ WORLD_EXECUTE
)

INSTALL ( FILES ${CONFIGFILES} DESTINATION .
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
