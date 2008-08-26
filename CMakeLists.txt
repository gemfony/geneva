PROJECT (geneva)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Set the build type to Release by default
IF(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Release)
ENDIF()

IF(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	# Enforce proper C++. Use this for debug builds.
	ADD_DEFINITIONS ("-DDEBUG -g -Wall -ansi -pthread")
ELSEIF(CMAKE_BUILD_TYPE STREQUAL "Release")
	# Enforce proper C++. Use this for release builds.
	ADD_DEFINITIONS ("-O3 -Wall -ansi -pthread")
ENDIF()

# You can hard-code the paths here, if desired 
#SET (BOOST_ROOT "/opt/boost136")
#SET (BOOST_INCLUDEDIR /opt/boost136/include/boost-1_36/)
#SET (BOOST_LIBRARYDIR /opt/boost136/lib)

SET (Boost_USE_STATIC_LIBS OFF)
SET (Boost_USE_MULTITHREAD ON)

FIND_PACKAGE( Boost 1.36 REQUIRED COMPONENTS date_time system thread serialization program_options)

INCLUDE_DIRECTORIES ( 
	${PROJECT_SOURCE_DIR}/include 
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

SET ( INFORMATIONFILES
    AUTHORS
    CHANGES
    COPYING
    INSTALL
    README
)

INSTALL ( FILES ${INFORMATIONFILES} DESTINATION .
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)
