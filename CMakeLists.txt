PROJECT (geneva)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# You can hard-code the Boost paths here, if you compiled Boost yourself and
# it was not deployed in one of the standard system locations. Otherwise,
# see the comments in the INSTALL file on how to set the path on the 
# command line.
#SET (BOOST_ROOT "/opt/boost136")
#SET (BOOST_INCLUDEDIR "/opt/boost136/include/boost-1_36")
#SET (BOOST_LIBRARYDIR "/opt/boost136/lib")

# Uncomment this if you want to see compiler arguments
# SET(CMAKE_VERBOSE_MAKEFILE ON)

# Uncomment to compile static libs
# SET( GENEVA_STATIC TRUE )

# Set the build type (default value is "Release")
# SET( CMAKE_BUILD_TYPE Debug )

# Make sure we first check our local modules directory
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")

# Set the build type to Release by default
IF(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Release)
ENDIF()

IF(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	# Use this for debug builds.    
	IF(CMAKE_CXX_COMPILER MATCHES "icpc" OR CMAKE_CXX_COMPILER MATCHES "icc")
		# Specifics for Intel compiler (tested with 10.1). Removes two rather annoying
		# (and, according to the authors' opinion, irrelevant in this context) warnings
		ADD_DEFINITIONS ("-DDEBUG -g -Wall -Wno-unused -ansi -pthread -wd1572 -wd1418 -wd981 -wd444 -wd383")
	ELSE() # e.g. g++
		# ADD_DEFINITIONS ("-DDEBUG -g -Wall -Wextra -Woverloaded-virtual -Wunused -ansi -std=c++98 -pedantic -pthread")
		ADD_DEFINITIONS ("-DDEBUG -g -Wall -Wno-unused -ansi -pthread")
	ENDIF()
ELSEIF(CMAKE_BUILD_TYPE STREQUAL "Release")
	# Use this for release builds.
	IF(CMAKE_CXX_COMPILER MATCHES "icpc" OR CMAKE_CXX_COMPILER MATCHES "icc")
		ADD_DEFINITIONS ("-O3 -pthread")
	ELSE() # e.g. g++
		ADD_DEFINITIONS ("-O3 -pthread")
	ENDIF()
ENDIF()

# Set the library type to shared by default
IF( NOT GENEVA_STATIC )
	SET( GENEVA_STATIC FALSE )
ENDIF()

IF( GENEVA_STATIC )
	SET (CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -lpthread")
	SET (LINK_FLAGS "-static-libgcc -lpthread")
	SET (BUILD_SHARED_LIBS OFF)
	SET (Boost_USE_STATIC_LIBS ON)
	# Why is this needed for static and not for shared libs??
	LINK_LIBRARIES (pthread)
ELSE()
	SET (BUILD_SHARED_LIBS ON)
	SET (Boost_USE_STATIC_LIBS OFF)
ENDIF()

SET (Boost_USE_MULTITHREAD ON)
SET (Boost_ADDITIONAL_VERSIONS "1.39" "1.39.0")

FIND_PACKAGE( Boost 1.36 REQUIRED COMPONENTS
	date_time
	system
	thread
	serialization
	program_options
	test_exec_monitor
)

INCLUDE_DIRECTORIES (
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/include/thirdparty
	${Boost_INCLUDE_DIRS}
)

LINK_DIRECTORIES (
	${PROJECT_SOURCE_DIR}/src
	${Boost_LIBRARY_DIRS}
)

ADD_SUBDIRECTORY (include)
ADD_SUBDIRECTORY (src)
ADD_SUBDIRECTORY (examples)
ADD_SUBDIRECTORY (tests)
ADD_SUBDIRECTORY (associated)
ADD_SUBDIRECTORY (CMakeModules)

SET ( INFORMATIONFILES
	AUTHORS
	CHANGES
	COPYING
	CREDITS
	FAQ
	INSTALL
	KNOWNBUGS
	RATEYOUREXPERIENCE
	README
	README.eclipse
)

INSTALL ( FILES ${INFORMATIONFILES} DESTINATION .
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
