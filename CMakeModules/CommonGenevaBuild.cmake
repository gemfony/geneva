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

CMAKE_MINIMUM_REQUIRED(VERSION 2.8 FATAL_ERROR)

# Include guard
IF(NOT COMMON_GENEVA_BUILD_INCLUDED)
SET(COMMON_GENEVA_BUILD_INCLUDED TRUE)

################################################################################
# Set some default values for user-settings. Generally, these should be provided
# through a custom genevaConfig.gcfg, in conjunction with the prepareBuild.sh script.

IF( NOT DEFINED GENEVA_BUILD_TYPE )
	SET( GENEVA_BUILD_TYPE "Release" )
ENDIF()

IF( NOT DEFINED GENEVA_BUILD_TESTS )
	SET( GENEVA_BUILD_TESTS FALSE )
ENDIF()

IF( NOT DEFINED GENEVA_CXX_STD )
	SET( GENEVA_CXX_STD "cxx98" )
ENDIF()

IF( NOT DEFINED GENEVA_STATIC )
	SET( GENEVA_STATIC 0 )
ENDIF()

IF( NOT DEFINED GENEVA_WITH_MPI )
	SET( GENEVA_WITH_MPI 0 )
ENDIF()

IF( NOT DEFINED CMAKE_VERBOSE_MAKEFILE )
	SET( CMAKE_VERBOSE_MAKEFILE 0 )
ENDIF()

###############################################################################
# Include host and compiler identification functions

INCLUDE(IdentifySystemParameters)

################################################################################
# Validate the build type chosen by the user (Release or Debug). This macro
# also sets CMAKE_BUILD_TYPE accordingly.

VALIDATE_BUILD_TYPE()

################################################################################
# Set the build mode static or dynamic

IF ( GENEVA_STATIC ) 
	SET (BUILD_SHARED_LIBS OFF)
ELSE () # dynamic libraries
	SET (BUILD_SHARED_LIBS ON)
ENDIF ()

################################################################################
# Identify the operating system and the compiler

FIND_HOST_OS (
	GENEVA_OS_NAME 
	GENEVA_OS_VERSION
)

FIND_HOST_COMPILER (
	GENEVA_COMPILER_NAME
	GENEVA_COMPILER_VERSION
	GENEVA_MAX_CXX_STANDARD
)

################################################################################
# Determine the C++ standard to be used, depending on the desired standard
# and the maximum standard allowed by the chosen compiler. This function
# will raise an error if the desired standard is higher than the maximum
# supported standard. It will also resolve the "auto" setting of the standard
# by setting it to the maximum supported standard.

GET_ACTUAL_CXX_STANDARD (
	${GENEVA_CXX_STD}
	${GENEVA_MAX_CXX_STANDARD}
	GENEVA_ACTUAL_CXX_STANDARD
)

###############################################################################
# Identify unsupported setups as early as possible

FLAG_UNSUPPORTED_SETUPS(
	GENEVA_OS_NAME 
	GENEVA_OS_VERSION
	GENEVA_COMPILER_NAME
	GENEVA_COMPILER_VERSION
	GENEVA_ACTUAL_CXX_STANDARD
	GENEVA_STATIC
)

################################################################################
# Set up compiler flags and add them to the compiler definitions

GET_COMPILER_FLAGS (
	${GENEVA_OS_NAME}
	${GENEVA_OS_VERSION}
	${GENEVA_COMPILER_NAME}
	${GENEVA_COMPILER_VERSION}
	${GENEVA_ACTUAL_CXX_STANDARD}
	${GENEVA_BUILD_TYPE}
	GENEVA_COMPILER_FLAGS_OUT
)

ADD_DEFINITIONS("${GENEVA_COMPILER_FLAGS_OUT}")

################################################################################
# Set other necessary build flags

GET_BUILD_FLAGS (
	${GENEVA_OS_NAME}
	${GENEVA_OS_VERSION}
	${GENEVA_COMPILER_NAME}
	${GENEVA_COMPILER_VERSION}
	${GENEVA_ACTUAL_CXX_STANDARD}
	${GENEVA_BUILD_TYPE}
	PLATFORM_NEEDS_LIBRARY_LINKING
)

################################################################################
# Define the required Boost environment

SET (Boost_USE_MULTITHREAD ON)
SET (Boost_ADDITIONAL_VERSIONS "1.57" "1.57.0" "1.58" "1.58.0")

IF ( GENEVA_STATIC ) 
	SET (Boost_USE_STATIC_LIBS ON)
ELSE () # dynamic libraries
	SET (Boost_USE_STATIC_LIBS OFF)
ENDIF ()

# This is the minimum Boost version Geneva compiles with
SET (GENEVA_MIN_BOOST_VERSION 1.48)

# These are the libraries required for any Geneva build
SET (
	GENEVA_BOOST_LIBS
	date_time
	filesystem
	program_options
	regex
	serialization
	system
	thread	
)

# Build tests if requested by the user
IF( GENEVA_BUILD_TESTS )
	ADD_DEFINITIONS ("-DGEM_TESTING")
	SET (
		GENEVA_BOOST_LIBS
		${GENEVA_BOOST_LIBS}
		test_exec_monitor
	)
ENDIF()

# Add Boost MPI-Support if requested by the user
IF( GENEVA_WITH_MPI )
	SET (
		GENEVA_BOOST_LIBS
		${GENEVA_BOOST_LIBS}
		mpi
	)
ENDIF()

# Search for the required libraries
MESSAGE("Searching for Boost...\n")
FIND_PACKAGE( 
	Boost 
	${GENEVA_MIN_BOOST_VERSION} REQUIRED 
	COMPONENTS ${GENEVA_BOOST_LIBS}
)

################################################################################
# The MPI mode is currently only supported under Linux and BSD

IF( GENEVA_WITH_MPI )
	IF( UNIX AND NOT "${GENEVA_OS_NAME}" MATCHES "MacOSX" )
		# Search for MPI libraries and headers
		FIND_PACKAGE(MPI)

		IF( NOT MPI_CXX_FOUND )
			MESSAGE (FATAL_ERROR "You have requested MPI support, but"
				 " no suitable MPI libraries were found. Leaving!")
		ENDIF()
	ELSE()
		MESSAGE (FATAL_ERROR "The MPI-mode is currently only supported"
			 " under Linux and BSD. Leaving!")
	ENDIF()
ENDIF()

################################################################################
# Add additional libraries and compiler flags

IF(UNIX)
	#---------------------------------------------------------------------------
	IF("${GENEVA_OS_NAME}" MATCHES "MacOSX")
		IF( GENEVA_STATIC )	
			SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libstdc++")
		ELSE() # Dynamic libraries
			SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lpthread -stdlib=libstdc++")
			SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libstdc++")
		ENDIF()
	ENDIF()
	
	#---------------------------------------------------------------------------
	IF( GENEVA_STATIC )	
		LINK_LIBRARIES (dl z pthread)
	ELSE() # Dynamic linking
		LINK_LIBRARIES (pthread)
	ENDIF()
	#---------------------------------------------------------------------------
ELSEIF(WIN32)
	# Nothing yet
ENDIF()

################################################################################
# On a Unix-system, add a custom target to run a "make clean" and remove
# temporaries, so the configuration process may start fresh. The make
# command will not be available on windows.

IF(UNIX)
	ADD_CUSTOM_TARGET(
		"clean-cmake"
		COMMAND ${CMAKE_BUILD_TOOL} clean 2>&1 > /dev/null
		COMMAND ${CMAKE_COMMAND} -DOLD_CMAKE=${OLD_CMAKE} -P ${PROJECT_SOURCE_DIR}/CMakeModules/CleanCmakeTemporaries.cmake
	)
ENDIF()

################################################################################
# Print a summary of the build settings before continuing with the main script

MESSAGE ("\n========================================")
MESSAGE ("")
MESSAGE ("Building:")
IF (GENEVA_STATIC)
	MESSAGE ("\tstatic libraries")
ELSE ()
	MESSAGE ("\tshared libraries")
ENDIF ()
IF (GENEVA_BUILD_TESTS)
	MESSAGE ("\tand test code")
ELSE ()
	MESSAGE ("\tbut no tests")
ENDIF ()
MESSAGE ("\tin ${CMAKE_BUILD_TYPE} mode")
IF ("${GENEVA_BUILD_TYPE}" STREQUAL "Sanitize")
	MESSAGE ("\twith -fsanitize=thread (if available)")
ENDIF () 

MESSAGE ("\twith Boost include location:\t ${Boost_INCLUDE_DIRS}")
MESSAGE ("\twith Boost library location:\t ${Boost_LIBRARY_DIRS}")
MESSAGE ("\tto install into prefix:\t\t ${CMAKE_INSTALL_PREFIX}")
MESSAGE ("\tusing ${GENEVA_COMPILER_NAME} compiler v${GENEVA_COMPILER_VERSION}"
	 " with ${GENEVA_ACTUAL_CXX_STANDARD} standard")
IF( CMAKE_VERBOSE_MAKEFILE )
	MESSAGE ("\tproducing verbose CMake output")
ELSE()
	MESSAGE ("\tproducing sparse CMake output")
ENDIF()
MESSAGE ("\tfor operating system ${GENEVA_OS_NAME}")
MESSAGE ("")
MESSAGE ("========================================\n")

###############################################################################
# End of the include-guard

ENDIF(NOT COMMON_GENEVA_BUILD_INCLUDED)

################################################################################
# Done
