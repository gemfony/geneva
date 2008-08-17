project (geneva)
cmake_minimum_required(VERSION 2.6)

IF(UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3 -pthread")
ENDIF(UNIX)

SET (BOOST_ROOT "/opt/boost136")
SET (BOOST_INCLUDEDIR /opt/boost136/include/boost-1_36/)
SET (BOOST_LIBRARYDIR /opt/boost136/lib)

SET (Boost_USE_STATIC_LIBS OFF)
SET (Boost_USE_MULTITHREAD ON)

FIND_PACKAGE( Boost 1.36 REQUIRED COMPONENTS date_time system thread serialization)

add_subdirectory (src)
add_subdirectory (examples)
