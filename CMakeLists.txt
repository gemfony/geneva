################################################################################
#
# Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
#
# See the AUTHORS file in the top-level directory for a list of authors.
#
# Contact: contact [at] gemfony (dot) eu
#
# This file is part of the Geneva library collection.
#
# Geneva was developed with kind support from Karlsruhe Institute of
# Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
# information about KIT and SCC can be found at http://www.kit.edu/english
# and http://scc.kit.edu .
#
# Geneva is free software: you can redistribute and/or modify it under
# the terms of version 3 of the GNU Affero General Public License
# as published by the Free Software Foundation.
#
# Geneva is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
#
# For further information on Gemfony scientific and Geneva, visit
# http://www.gemfony.eu .
#
################################################################################

PROJECT (geneva-library-collection)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Set the library/package version
SET(VERSION_MAJOR "1")
SET(VERSION_MINOR "1")
SET(VERSION_PATCH "0")

################################################################################
# Identify the Operating System. This is particularly needed for MacOS

SET(GENEVA_OS_NAME ${CMAKE_SYSTEM_NAME})

# If this is MacOS, find out about the version
IF("${GENEVA_OS_NAME}" MATCHES "Darwin")
   EXEC_PROGRAM(uname ARGS -v  OUTPUT_VARIABLE DARWIN_VERSION)
   STRING(REGEX MATCH "[0-9]+" DARWIN_VERSION ${DARWIN_VERSION})
   IF (DARWIN_VERSION GREATER 12)
     SET(GENEVA_OS_NAME "DarwinGE13")
   ELSE ()
   	 SET(GENEVA_OS_NAME "DarwinPre13")
   ENDIF ()
ENDIF()

################################################################################
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
################################################################################

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

################################################################################
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

################################################################################
# Set up compiler flags

IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
	SET( MSG_COMPILER "icpc/icc")
	SET (MSG_CXXSTANDARDMODE "Using Intel compiler, hence avoiding C++ standard settings")
		
	IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
		ADD_DEFINITIONS ("-DDEBUG -g")
	ELSEIF(CMAKE_BUILD_TYPE STREQUAL "Release")
		ADD_DEFINITIONS ("-DNDEBUG -O3")
	ENDIF()	
	
	# Specifics for Intel compiler (tested with 10.1). Removes two rather annoying
	# (and, according to the authors' opinion, irrelevant in this context) warnings
	ADD_DEFINITIONS ("-Wall -Wno-unused -ansi -pthread -wd1572 -wd1418 -wd981 -wd444 -wd383")
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	SET( MSG_COMPILER "CLang")
	
	IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
		ADD_DEFINITIONS ("-DDEBUG -g")
	ELSEIF(CMAKE_BUILD_TYPE STREQUAL "Release")
		ADD_DEFINITIONS ("-DNDEBUG -O3")
	ENDIF()	
	
	ADD_DEFINITIONS ("-Wall -Wno-unused -Wno-attributes -Wno-parentheses-equality -ansi -pthread")
	
	IF("${GENEVA_OS_NAME}" MATCHES "DarwinPre13") # MacOS prior to 10.9
		SET (MSG_CXXSTANDARDMODE "Using CLang version ${CMAKE_CXX_COMPILER_VERSION} on MacOS prior to 10.9, hence switching to C++98-mode")
		ADD_DEFINITIONS ("-std=c++98 -ftemplate-depth=512 -stdlib=libstdc++")
	ELSEIF("${GENEVA_OS_NAME}" MATCHES "DarwinGE13") # MacOS >= 10.9
		SET (MSG_CXXSTANDARDMODE "Using CLang version ${CMAKE_CXX_COMPILER_VERSION} on MacOS >= 10.9, hence switching to C++98-mode")
		ADD_DEFINITIONS ("-std=c++98 -ftemplate-depth=512 -stdlib=libstdc++")
	ELSEIF(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.1") # Older Clang-based systems
		SET (MSG_CXXSTANDARDMODE "Using CLang version ${CMAKE_CXX_COMPILER_VERSION} on ${CMAKE_SYSTEM_NAME}, hence switching to C++98-mode")
		# ADD_DEFINITIONS ("-std=c++98 -ftemplate-depth=512")
		ADD_DEFINITIONS ("-std=c++98 -ftemplate-depth=512 -stdlib=libstdc++")
	ELSE() # Newer, non-MacOS Clang-based systems
		SET (MSG_CXXSTANDARDMODE "Using Clang version ${CMAKE_CXX_COMPILER_VERSION}, switching to C++11-mode")
		ADD_DEFINITIONS ("-std=c++11")		
	ENDIF()
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	SET( MSG_COMPILER "GNU g++")

	# Make gcc/g++ output its error messages on a single line, so Eclipse-CDT can parse them
  	ADD_DEFINITIONS ("-fmessage-length=0")

	IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
		ADD_DEFINITIONS ("-DDEBUG -g")
	ELSEIF(CMAKE_BUILD_TYPE STREQUAL "Release")
		ADD_DEFINITIONS ("-DNDEBUG -O3")
	ENDIF()	

	# Note: -fno-unsafe-math-optimizations -fno-finite-math-only prevent false optimization of some critical
	# code in GRandomBaseT inside uniform_real(). Add "-Wall -Wextra -ansi -pedantic" to make tests even more thorough. This
	# will likely trigger many warnings in boost.
	ADD_DEFINITIONS ("-fno-unsafe-math-optimizations -fno-finite-math-only -Wno-unused -Wno-attributes -pthread -ftemplate-depth-1024")	
	
	# Switch to C++11-mode if the compiler version is suitable	
	EXEC_PROGRAM( 
		${CMAKE_CXX_COMPILER}
		ARGS -dumpversion
	 	OUTPUT_VARIABLE GCC_VERSION
	)
	IF(GCC_VERSION VERSION_GREATER "4.6.99")
		SET (MSG_CXXSTANDARDMODE "Using g++ version ${GCC_VERSION}, hence switching to C++11-mode")
		ADD_DEFINITIONS ("-std=c++11")
	ELSE()
		SET (MSG_CXXSTANDARDMODE "Using g++ version ${GCC_VERSION}, hence switching to C++98-mode")
		ADD_DEFINITIONS ("-std=c++98")
	ENDIF()
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	MESSAGE (FATAL_ERROR "Visual Studio C++ not yet supported")
ELSE()
	MESSAGE (FATAL_ERROR "Unknown compiler CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}")
ENDIF()

################################################################################
# Specify linker flags
IF( GENEVA_STATIC )	
	SET (CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -lpthread")
	SET (LINK_FLAGS "-static-libgcc -lpthread")
		
	# Some special linker flags for MacOS
	IF("${GENEVA_OS_NAME}" MATCHES "DarwinGE13") # MacOS >= 10.9
		SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libstdc++ -macosx_version_min=10.8")
		SET (LINK_FLAGS "${LINK_FLAGS} -stdlib=libstdc++ -macosx_version_min=10.8")		
	ENDIF()
	
	SET (BUILD_SHARED_LIBS OFF)
	SET (Boost_USE_STATIC_LIBS ON)
	LINK_LIBRARIES (dl)
	LINK_LIBRARIES (z)
	LINK_LIBRARIES (pthread)
ELSE() # Dynamic libraries
	# Some special linker flags for MacOS 10.9
	IF("${GENEVA_OS_NAME}" MATCHES "DarwinGE13") # MacOS >= 10.9
		SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lpthread -stdlib=libstdc++ -macosx_version_min=10.8")
		SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libstdc++ -macosx_version_min=10.8")
	ENDIF()

	SET (BUILD_SHARED_LIBS ON)
	SET (Boost_USE_STATIC_LIBS OFF)
	LINK_LIBRARIES (pthread)
ENDIF()

################################################################################
# Boost-specific code
SET (Boost_USE_MULTITHREAD ON)
SET (Boost_ADDITIONAL_VERSIONS "1.49" "1.49.0" "1.50" "1.50.0" "1.51" "1.51.0" "1.52" "1.52.0" "1.53" "1.53.0" "1.54" "1.54.0" "1.55.0")

FIND_PACKAGE( Boost 1.41 REQUIRED COMPONENTS
	date_time
	filesystem
	program_options
	regex
	serialization
	system
	thread
)

################################################################################
# Check whether we want to activate the test code and build tests
IF( GENEVA_BUILD_TESTS )
	ADD_DEFINITIONS ("-DGEM_TESTING")
	FIND_PACKAGE( Boost 1.41 REQUIRED COMPONENTS
		test_exec_monitor
	)
ENDIF()

################################################################################
# Add additional libraries if this is MacOS
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	LINK_LIBRARIES(
		${Boost_DATE_TIME_LIBRARY}
		${Boost_FILESYSTEM_LIBRARY}
		${BOOST_REGEX_LIBRARY}
		${Boost_SERIALIZATION_LIBRARY}
		${Boost_SYSTEM_LIBRARY}
		${Boost_THREAD_LIBRARY}
		${Boost_PROGRAM_OPTIONS_LIBRARY}
	)
	
	IF( GENEVA_BUILD_TESTS )
		LINK_LIBRARIES(
			${Boost_TEST_EXEC_MONITOR_LIBRARY}
		)
	ENDIF()
ENDIF()

################################################################################
# Include- and link-directories, libraries
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

################################################################################
# Custom targets for building single libraries only
ADD_CUSTOM_TARGET( "common"       DEPENDS ${COMMON_LIBNAME}
				COMMENT "Building only the Common library." )
ADD_CUSTOM_TARGET( "hap"          DEPENDS ${HAP_LIBNAME}
				COMMENT "Building only the Hap library." )
ADD_CUSTOM_TARGET( "courtier"     DEPENDS ${COURTIER_LIBNAME}
				COMMENT "Building only the Courtier library." )
ADD_CUSTOM_TARGET( "geneva"       DEPENDS ${GENEVA_LIBNAME}
				COMMENT "Building only the Geneva library." )

################################################################################
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
MESSAGE ("\t${MSG_CXXSTANDARDMODE}")
MESSAGE ("========================================\n")

################################################################################
# Add sub-directories

# Set a flag to avoid repeated messages in the sub-folders
SET ( GENEVA_INFO_MSG TRUE )

ADD_SUBDIRECTORY (CMakeModules)
ADD_SUBDIRECTORY (src)
ADD_SUBDIRECTORY (examples)
ADD_SUBDIRECTORY (include)
ADD_SUBDIRECTORY (licenses)
ADD_SUBDIRECTORY (scripts)

IF( GENEVA_BUILD_TESTS )
	INCLUDE(CTest)
	ADD_SUBDIRECTORY (tests)
ENDIF()

################################################################################
# Specify files to be copied
SET ( INFORMATIONFILES
	AUTHORS
	CHANGES
	COPYING
	CREDITS
	FAQ
	GENEVAROOT
	INSTALL
	KNOWNBUGS
	RATEYOUREXPERIENCE
	README
	README.eclipse
	README.documentation
)

INSTALL ( FILES ${INFORMATIONFILES} DESTINATION .
	PERMISSIONS
	OWNER_READ OWNER_WRITE
	GROUP_READ
	WORLD_READ
)

################################################################################
#
# Warning for CMake 2.6, comes last to avoid the message being "lost" in the later output.
#
IF( OLD_CMAKE )
	MESSAGE( AUTHOR_WARNING ": You are using CMake 2.6. Some files will not be copied to the _build_ tree, "
				"in particular the configuration files for the examples will be missing. "
				"Updating to CMake 2.8 or later is recommended!" )
ENDIF()

################################################################################
# Code for package generation with CPack. This is experimental and the packages 
# don't satisfy the Linux FHS yet. Use with care.
#  
# RPM or DEB packages can be created using umask 022 && fakeroot make package.

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
SET(CPACK_PACKAGE_CONTACT "Gemfony scientific <contact@gemfony.eu>")
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

################################################################################
# done
