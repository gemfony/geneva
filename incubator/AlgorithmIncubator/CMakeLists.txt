cmake_minimum_required(VERSION 2.8)

project(OAInkubator)

set (REQUIRED_BOOST_LIBS
	random
)

set (MIN_BOOST_VERSION
	1.57
)

message("Searching for Boost...\n")
# This will invoke FindBoost.cmake
find_package(
	Boost
	${MIN_BOOST_VERSION} REQUIRED
	COMPONENTS ${REQUIRED_BOOST_LIBS}
)
MESSAGE("")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")

set(MAIN_SOURCES
        main.cpp
)

set(EXECUTABLENAME
	OAInkubator
)

add_subdirectory("${PROJECT_SOURCE_DIR}/incubatorlib")
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
include_directories (
	"${PROJECT_SOURCE_DIR}/incubatorlib"
	${Boost_INCLUDE_DIRS}
)
link_directories (
	"${PROJECT_SOURCE_DIR}/incubatorlib"

)

add_executable(${EXECUTABLENAME}
	${MAIN_SOURCES}
)

target_link_libraries (${EXECUTABLENAME}
	incubator
	${Boost_LIBRARIES}
)

INSTALL (TARGETS ${EXECUTABLENAME}
	DESTINATION /opt/${EXECUTABLENAME}
)
