PROJECT (GNeuralNetwork)

CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

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

IF(NOT DEFINED GENEVA_LIBRARY)
       SET (GENEVA_LIBRARY "geneva-opt")
ENDIF()

SET (Boost_USE_STATIC_LIBS OFF)
SET (LINK_FLAGS "-lpthread")
SET (CMAKE_EXE_LINKER_FLAGS "-lpthread")

SET (Boost_USE_MULTITHREADED ON)

IF(NOT DEFINED CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Release) # Set build type to Release by default
	# SET (CMAKE_BUILD_TYPE Debug) # Set build type to Debug by default
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

# The test component is not necessary for the application, but needed for the tests
FIND_PACKAGE( Boost 1.36 COMPONENTS
	test_exec_monitor
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

SET ( EXECUTABLENAME GNeuralNetwork )

SET ( ${EXECUTABLENAME}_SRCS
	  GArgumentParser
	  GNeuralNetwork
	  GNeuralNetworkIndividual
)

SET ( CONFIGFILE
	GNeuralNetwork.cfg
)

ADD_EXECUTABLE(${EXECUTABLENAME}
	${${EXECUTABLENAME}_SRCS}
)

INCLUDE_DIRECTORIES (AFTER
	${PROJECT_SOURCE_DIR}/
)

MESSAGE( "LIBS: ${Boost_LIBRARIES}")
TARGET_LINK_LIBRARIES (${EXECUTABLENAME}
	${GENEVA_LIBRARY}
	${Boost_LIBRARIES}
	pthread
)

# Tests will only be compiled of the test libraries were found
IF( Boost_TEST_EXEC_MONITOR_FOUND )
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
