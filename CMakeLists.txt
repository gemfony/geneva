PROJECT (geneva)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# You can hard-code the Boost paths here, if you compiled Boost yourself and
#   it was not deployed in one of the stadard system locations
#SET (BOOST_ROOT "/opt/boost136")
#SET (BOOST_INCLUDEDIR "/opt/boost136/include/boost-1_36")
#SET (BOOST_LIBRARYDIR "/opt/boost136/lib")

# Set once for Eclipse to find the boost libraries, you can then comment
#   it out again
#SET(CMAKE_VERBOSE_MAKEFILE ON)

# Set the build type to Release by default
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()

IF(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    # Enforce proper C++. Use this for debug builds.
    # ADD_DEFINITIONS ("-DDEBUG -g -Wall -Wextra -Woverloaded-virtual -Wno-unused -ansi -std=c++98 -pedantic -pthread")
    ADD_DEFINITIONS ("-DDEBUG -g -Wall -ansi -pthread")
ELSEIF(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Use this for release builds.
    ADD_DEFINITIONS ("-O3 -pthread")
ENDIF()

SET (Boost_USE_STATIC_LIBS OFF)
SET (Boost_USE_MULTITHREAD ON)

SET (Boost_ADDITIONAL_VERSIONS "1.38.0" "1.38" "1.37.0" "1.37")

FIND_PACKAGE( Boost 1.38 REQUIRED COMPONENTS 
	date_time 
	system 
	thread 
	serialization 
	program_options 
	test_exec_monitor 
)

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
ADD_SUBDIRECTORY (associated)

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
