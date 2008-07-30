project (geneva)
cmake_minimum_required(VERSION 2.6)
SET (BOOST_ROOT "/opt/boost136")
SET (BOOST_INCLUDEDIR /opt/boost136/include/boost-1_36/)
SET (Boost_USE_STATIC_LIBS OFF)
SET (Boost_USE_MULTITHREAD ON)
SET (Boost_ADDITIONAL_VERSION "1.36")

FIND_PACKAGE( Boost "1.36" REQUIRED )

add_subdirectory (src)
add_subdirectory (examples)
