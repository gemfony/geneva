PROJECT (geneva-opt)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Set the library/package version
SET(VERSION_MAJOR "0")
SET(VERSION_MINOR "9")
SET(VERSION_PATCH "4rc0")

###############################################################################
#
# User settings:
#   you can change some settings here, but we recommend that you use the
#   'prepareBuild.sh' script and the sample settings file 'genevaConfig.gcfg'
#   provided in the 'scripts' folder for building Geneva.
#

# You can hard-code the Boost paths here, if you compiled Boost yourself and
# it was not deployed in one of the standard system locations. Otherwise,
# see the comments in the INSTALL file on how to set the path on the
# command line.
#SET(BOOST_ROOT "/opt/boost143")
#SET(BOOST_INCLUDEDIR "/opt/boost143/include/boost")
#SET(BOOST_LIBRARYDIR "/opt/boost143/lib")

# Uncomment this if you want to see compiler options
#SET(CMAKE_VERBOSE_MAKEFILE ON)

# Uncomment to compile static libs
#SET(GENEVA_STATIC TRUE)

# Uncomment to build in 'Release' mode, default is 'Debug'
#SET(GENEVA_BUILD_TYPE Release)

# Uncomment to build tests
#SET(GENEVA_BUILD_TESTS TRUE)

# Change this setting to install in a different directory
#SET(CMAKE_INSTALL_PREFIX "/opt/geneva")

#
# End of user settings
#
###############################################################################

# Make sure we first check our local modules directory
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")

# Set the default install directory
IF( NOT DEFINED CMAKE_INSTALL_PREFIX )
	SET( CMAKE_INSTALL_PREFIX "/opt/geneva" )
ENDIF()

# Set the library type to shared by default
IF( NOT DEFINED GENEVA_STATIC )
	SET( GENEVA_STATIC FALSE )
ENDIF()

# Do not build tests by default
IF( NOT DEFINED GENEVA_BUILD_TESTS )
	SET( GENEVA_BUILD_TESTS FALSE )
ENDIF()

# The "FOREACH IN LISTS" construct is only available starting with CMake 2.8.
# Detect that case to avoid build errors in CMake 2.6.
IF( CMAKE_MAJOR_VERSION GREATER 1 AND CMAKE_MINOR_VERSION LESS 8 )
	SET( OLD_CMAKE TRUE )
	# We print a warning at end of the generation, to avoid it getting lost
ENDIF()

# Nice formatting
MESSAGE ("")

# Set the build type according to the GENEVA_BUILD_TYPE value, if defined.
# Otherwise use CMAKE_BUILD_TYPE if defined, or use build type 'Debug'
# as default.
IF(DEFINED GENEVA_BUILD_TYPE)
	IF(GENEVA_BUILD_TYPE STREQUAL "Debug")
		MESSAGE ("Setting the build type to Debug")
		SET(CMAKE_BUILD_TYPE Debug)
	ELSEIF(GENEVA_BUILD_TYPE STREQUAL "Release")
		MESSAGE ("Setting the build type to Release")
		SET(CMAKE_BUILD_TYPE Release)
	ELSE()
		MESSAGE (FATAL_ERROR "Unknown compilation mode GENEVA_BUILD_TYPE=${GENEVA_BUILD_TYPE}")
	ENDIF()
ELSEIF(DEFINED CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE STREQUAL "")
	MESSAGE ("Using provided value of CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
ELSE()
	MESSAGE ("Setting the build type to default value Debug")
	SET(CMAKE_BUILD_TYPE Debug)
ENDIF()
MESSAGE ("")

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
	IF(CMAKE_CXX_COMPILER MATCHES "icpc" OR CMAKE_CXX_COMPILER MATCHES "icc")
		SET( MSG_COMPILER "icpc/icc")
		# Specifics for Intel compiler (tested with 10.1). Removes two rather annoying
		# (and, according to the authors' opinion, irrelevant in this context) warnings
		ADD_DEFINITIONS ("-DDEBUG -g -Wall -Wno-unused -ansi -pthread -wd1572 -wd1418 -wd981 -wd444 -wd383")
	ELSE() # e.g. g++
		SET( MSG_COMPILER "gcc")
		# ADD_DEFINITIONS ("-DDEBUG -pg -g -fno-unsafe-math-optimizations -fno-finite-math-only -Wall -Wextra -Woverloaded-virtual -Wunused -ansi -std=c++98 -pedantic -pthread")
		ADD_DEFINITIONS ("-DDEBUG -g -fno-unsafe-math-optimizations -fno-finite-math-only -Wall -Wno-unused -ansi -pthread")
		# ADD_DEFINITIONS ("-DDEBUG -g -fno-unsafe-math-optimizations -fno-finite-math-only -Wall -Wno-unused -pthread -std=c++0x")
	ENDIF()
ELSEIF(CMAKE_BUILD_TYPE STREQUAL "Release")
	IF(CMAKE_CXX_COMPILER MATCHES "icpc" OR CMAKE_CXX_COMPILER MATCHES "icc")
		SET( MSG_COMPILER "icpc/icc")
		ADD_DEFINITIONS ("-O3 -pthread")
	ELSE() # e.g. g++
		SET( MSG_COMPILER "gcc")
		ADD_DEFINITIONS ("-fno-unsafe-math-optimizations -fno-finite-math-only -O3 -pthread")
		# ADD_DEFINITIONS ("-fno-unsafe-math-optimizations -fno-finite-math-only -g -O0 -pthread")
	ENDIF()
ELSE()
	MESSAGE (FATAL_ERROR "Unknown compilation mode CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
ENDIF()

# Make gcc output its error messages on a single line, so Eclipse-CDT can parse them
IF(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
ENDIF()
# Make g++ output its error messages on a single line, so Eclipse-CDT can parse them
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")
ENDIF()

# Note: -fno-unsafe-math-optimizations -fno-finite-math-only prevent false optimization of some critical
# code in GRandomBaseT inside uniform_real().

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
SET (Boost_ADDITIONAL_VERSIONS "1.49" "1.49.0")

FIND_PACKAGE( Boost 1.41 REQUIRED COMPONENTS
	date_time
	filesystem
	program_options
	regex
	serialization
	system
	thread
)

# Check whether we want to activate the test code and build tests
IF( GENEVA_BUILD_TESTS )
	ADD_DEFINITIONS ("-DGEM_TESTING")
	FIND_PACKAGE( Boost 1.41 REQUIRED COMPONENTS
		test_exec_monitor
	)
ENDIF()

INCLUDE_DIRECTORIES (
	${PROJECT_SOURCE_DIR}/include
	${Boost_INCLUDE_DIRS}
)

LINK_DIRECTORIES (
	${PROJECT_SOURCE_DIR}/src/common
	${PROJECT_SOURCE_DIR}/src/hap
	${PROJECT_SOURCE_DIR}/src/courtier
	${PROJECT_SOURCE_DIR}/src/geneva
	${Boost_LIBRARY_DIRS}
)

# The names of the libraries that will be built
SET ( COMMON_LIBNAME            "gemfony-common" )
SET ( HAP_LIBNAME               "gemfony-hap" )
SET ( COURTIER_LIBNAME          "gemfony-courtier" )
SET ( GENEVA_LIBNAME            "gemfony-geneva" )
SET ( GENEVA_INDIVIDUAL_LIBNAME "gemfony-geneva-individuals")

# Custom targets for building single libraries only
ADD_CUSTOM_TARGET( "common"       DEPENDS ${COMMON_LIBNAME}
				COMMENT "Building only the Common library." )
ADD_CUSTOM_TARGET( "hap"          DEPENDS ${HAP_LIBNAME}
				COMMENT "Building only the Hap library." )
ADD_CUSTOM_TARGET( "courtier"     DEPENDS ${COURTIER_LIBNAME}
				COMMENT "Building only the Courtier library." )
ADD_CUSTOM_TARGET( "geneva"       DEPENDS ${GENEVA_LIBNAME}
				COMMENT "Building only the Geneva library." )

# Print a summary of the build settings before going to the subfolders
MESSAGE ("\n========================================")
MESSAGE ("Building:")
IF( GENEVA_STATIC )
	MESSAGE ("\tstatic libraries")
ELSE()
	MESSAGE ("\tshared libraries")
ENDIF()
IF( GENEVA_BUILD_TESTS )
	MESSAGE ("\tand test code")
ELSE()
	MESSAGE ("\tbut no tests")
ENDIF()
MESSAGE ("\tin ${CMAKE_BUILD_TYPE} mode")
MESSAGE ("\tusing ${MSG_COMPILER} compiler settings")
MESSAGE ("\twith Boost include file location ${Boost_INCLUDE_DIRS}")
MESSAGE ("\twith Boost library location ${Boost_LIBRARY_DIRS}")
MESSAGE ("\twith install prefix ${CMAKE_INSTALL_PREFIX}")
IF( CMAKE_VERBOSE_MAKEFILE )
	MESSAGE ("\tand verbose CMake output")
ELSE()
	MESSAGE ("\tand sparse CMake output")
ENDIF()
MESSAGE ("========================================\n")

# Set a flag to avoid repeated messages in the subfolders
SET ( GENEVA_INFO_MSG TRUE )

ADD_SUBDIRECTORY (CMakeModules)
ADD_SUBDIRECTORY (src)
ADD_SUBDIRECTORY (utilities)
ADD_SUBDIRECTORY (examples)
ADD_SUBDIRECTORY (include)
ADD_SUBDIRECTORY (licenses)
ADD_SUBDIRECTORY (scripts)

IF( GENEVA_BUILD_TESTS )
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
	README.manual
)

INSTALL ( FILES ${INFORMATIONFILES} DESTINATION .
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)

#
# Warning for CMake 2.6, comes last to avoid the message being "lost" in the later output.
#
IF( OLD_CMAKE )
	MESSAGE( AUTHOR_WARNING ": You are using CMake 2.6. Some files will not be copied to the _build_ tree, "
				"in particular the configuration files for the examples will be missing. "
				"Updating to CMake 2.8 or later is recommended!" )
ENDIF()


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
SET(CPACK_PACKAGE_CONTACT "Gemfony scientific <contact@gemfony.com>")
SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
SET(CPACK_PACKAGING_INSTALL_PREFIX "/opt/geneva")

# DEB specific variables. Indentation is important in the description!
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}
 ${CPACK_AUX_PACKAGE_DESCRIPTION}"
)
SET(CPACK_DEBIAN_PACKAGE_SECTION "Science")
SET(CPACK_DEBIAN_PACKAGE_PRIORITY "Extra")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-dev (>= 1.41.0),
	libboost-date-time-dev (>= 1.41.0),
	libboost-filesystem-dev (>= 1.41.0),
	libboost-program-options-dev (>= 1.41.0),
	libboost-regex-dev (>= 1.41.0),
	libboost-serialization-dev (>= 1.41.0),
	libboost-system-dev (>= 1.41.0),
	libboost-thread-dev (>= 1.41.0),
	libboost-test-dev (>= 1.41.0)"
)

# RPM specific variables
SET(CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_AUX_PACKAGE_DESCRIPTION})
SET(CPACK_RPM_PACKAGE_LICENSE "Affero GPL v3")
SET(CPACK_RPM_PACKAGE_GROUP "Sciences/Libraries")
SET(CPACK_RPM_PACKAGE_REQUIRES "boost-devel >= 1.41")

# Detect the distribution to autoselect the package generator...
EXECUTE_PROCESS(COMMAND lsb_release -is
	OUTPUT_VARIABLE CPACK_AUX_DISTRO
)
IF(CPACK_AUX_DISTRO MATCHES "^Debian" OR CPACK_AUX_DISTRO MATCHES "^Ubuntu")
	SET(CPACK_GENERATOR "DEB")
ELSE(CPACK_AUX_DISTRO MATCHES "^RedHat" OR CPACK_AUX_DISTRO MATCHES "^CentOS" OR
     CPACK_AUX_DISTRO MATCHES "^Scientific" OR CPACK_AUX_DISTRO MATCHES "^SUSE" )
	SET(CPACK_GENERATOR "RPM")
ENDIF()

INCLUDE(CPack)
