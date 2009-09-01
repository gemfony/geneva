PROJECT (geneva-opt)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# You can hard-code the Boost paths here, if you compiled Boost yourself and
# it was not deployed in one of the standard system locations. Otherwise,
# see the comments in the INSTALL file on how to set the path on the 
# command line.
#SET (BOOST_ROOT "/opt/boost140")
#SET (BOOST_INCLUDEDIR "/opt/boost140/include/boost-1_40")
#SET (BOOST_LIBRARYDIR "/opt/boost140/lib")

# Uncomment this if you want to see compiler arguments
# SET(CMAKE_VERBOSE_MAKEFILE ON)

# Uncomment to compile static libs
# SET( GENEVA_STATIC TRUE )

# Set the build type (default value is "Release")
# SET( CMAKE_BUILD_TYPE Debug )

# Change this setting to install in a different directory
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
	SET(CMAKE_INSTALL_PREFIX "/opt/geneva")
ENDIF()

# Make sure we first check our local modules directory
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")

# Set the build type to Release by default
IF(NOT DEFINED CMAKE_BUILD_TYPE)
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
	LINK_LIBRARIES (dl)
	LINK_LIBRARIES (z)	
	LINK_LIBRARIES (pthread)
ELSE()
	SET (BUILD_SHARED_LIBS ON)
	SET (Boost_USE_STATIC_LIBS OFF)
	LINK_LIBRARIES (pthread)
ENDIF()

SET (Boost_USE_MULTITHREAD ON)
SET (Boost_ADDITIONAL_VERSIONS "1.39" "1.39.0" "1.40" "1.40.0")

FIND_PACKAGE( Boost 1.36 REQUIRED COMPONENTS
	date_time
	filesystem
	program_options
	serialization
	system
	thread
)

# The test component is not necessary for the library, but needed for the tests
FIND_PACKAGE( Boost 1.36 COMPONENTS
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

ADD_SUBDIRECTORY (associated)
ADD_SUBDIRECTORY (CMakeModules)
ADD_SUBDIRECTORY (examples)
ADD_SUBDIRECTORY (include)
ADD_SUBDIRECTORY (licenses)
ADD_SUBDIRECTORY (src)
IF( Boost_TEST_EXEC_MONITOR_FOUND )
	ADD_SUBDIRECTORY (tests)
ENDIF()

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

#########################################################
# Code for package generation with CPack.
# This is experimental and the packages don't
# satisfy the Linux FHS yet. Use with care. 
# An RPM or DEB package can be created using
#   umask 022 && fakeroot make package

INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_NAME "geneva-opt")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The Geneva optimization library")

# The package description. Indentation is important!
SET(CPACK_AUX_PACKAGE_DESCRIPTION
 "This is a beta version of the Geneva optimization library. As the name
 suggests --it stands for 'Grid-enabled evolutionary algorithms'--, Geneva
 aims to provide the necessary tools to perform parametric optimization
 in parallel on devices ranging from multi-processor machines over
 clusters all the way to Grids and Cloud installations.
 .
 Geneva does not want to be the fastest library of evolutionary
 algorithms on single-processor machines, but is instead targeted at
 large scale problems, where the evaluation of a single individual will
 typically take longer than a few seconds."
)

SET(CPACK_PACKAGE_VENDOR "Gemfony scientific and Karlsruhe Institute of Technology")
SET(CPACK_PACKAGE_CONTACT "Gemfony scientific <info@gemfony.com>")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "6")
SET(CPACK_PACKAGE_VERSION_PATCH "1")
SET(CPACK_PACKAGING_INSTALL_PREFIX "/opt/geneva")

# DEB specific variables. Indentation is important in the description!
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}
 ${CPACK_AUX_PACKAGE_DESCRIPTION}"
)
SET(CPACK_DEBIAN_PACKAGE_SECTION "Science")
SET(CPACK_DEBIAN_PACKAGE_PRIORITY "Extra")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-dev (>= 1.38.1),
	libboost-date-time-dev (>= 1.38.1),
	libboost-filesystem-dev (>= 1.38.1),
	libboost-program-options-dev (>= 1.38.1),
	libboost-serialization-dev (>= 1.38.1),
	libboost-system-dev (>= 1.38.1),
	libboost-thread-dev (>= 1.38.1),
	libboost-test-dev (>= 1.38.1)"
)

# RPM specific variables
SET(CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_AUX_PACKAGE_DESCRIPTION})
SET(CPACK_RPM_PACKAGE_LICENSE "Affero GPL v3")
SET(CPACK_RPM_PACKAGE_GROUP "Sciences/Libraries")
SET(CPACK_RPM_PACKAGE_REQUIRES "boost-devel >= 1.36")

# Detect the distribution to autoselect the package generator...
EXECUTE_PROCESS(COMMAND lsb_release -is
	OUTPUT_VARIABLE CPACK_AUX_DISTRO
)
IF(CPACK_AUX_DISTRO MATCHES "^Debian" OR CPACK_AUX_DISTRO MATCHES "^Ubuntu")
	SET(CPACK_GENERATOR "DEB")
ELSE(CPACK_AUX_DISTRO MATCHES "^RedHat" OR CPACK_AUX_DISTRO MATCHES "^CentOS"
     CPACK_AUX_DISTRO MATCHES "^Scientific" OR CPACK_AUX_DISTRO MATCHES "^SUSE" )
	SET(CPACK_GENERATOR "RPM")
ENDIF()

INCLUDE(CPack)
