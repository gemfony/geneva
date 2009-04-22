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
    # Use this for debug builds.    
    IF ($ENV{CXX} MATCHES "icpc" OR $ENV{CXX} MATCHES "icc")
    	# Specifics for Intel compiler (tested with 10.1). Removes two rather annoying
    	# (and, according to the authors' opinion, irrelevant in this context) warnings
    	ADD_DEFINITIONS ("-DDEBUG -g -Wall -Wno-unused -ansi -pthread -wd1572 -wd1418 -wd981 -wd444 -wd383")
    ELSEIF ($ENV{CXX} MATCHES "pgCC")
    	# NOTE: This does not work yet
    	ADD_DEFINITIONS ("-DDEBUG -g -lpthread -DBOOST_THREAD_POSIX -DBOOST_HAS_THREADS")
    ELSE()
        # ADD_DEFINITIONS ("-DDEBUG -g -Wall -Wextra -Woverloaded-virtual -Wunused -ansi -std=c++98 -pedantic -pthread")
    	ADD_DEFINITIONS ("-DDEBUG -g -Wall -Wno-unused -ansi -pthread")
    ENDIF()
ELSEIF(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Use this for release builds.
    IF ($ENV{CXX} MATCHES "icpc" OR $ENV{CXX} MATCHES "icc")
    	ADD_DEFINITIONS ("-O3 -pthread")
    ELSEIF ($ENV{CXX} MATCHES "pgCC")
    	ADD_DEFINITIONS ("-nfast -lpthread -DBOOST_THREAD_POSIX -DBOOST_HAS_THREADS")
    ELSE()
    	ADD_DEFINITIONS ("-O3 -pthread")
    ENDIF()
ENDIF()

SET (Boost_USE_STATIC_LIBS OFF)
SET (Boost_USE_MULTITHREAD ON)

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
