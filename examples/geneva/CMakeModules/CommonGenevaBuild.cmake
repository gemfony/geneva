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

#
# This module is needed to setup the Geneva build system, and may also be used
# to compile independent Geneva applications.
#

################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.1 FATAL_ERROR)

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
	SET( GENEVA_BUILD_TESTS TRUE )
ENDIF()

IF( NOT DEFINED GENEVA_STATIC )
	SET( GENEVA_STATIC FALSE )
ENDIF()

IF( NOT DEFINED GENEVA_WITH_MPI )
	SET( GENEVA_WITH_MPI FALSE )
ENDIF()

IF( NOT DEFINED CMAKE_VERBOSE_MAKEFILE )
	SET( CMAKE_VERBOSE_MAKEFILE FALSE )
ENDIF()

###############################################################################
# Include host and compiler identification functions

INCLUDE(IdentifySystemParameters)

################################################################################
# Validate the build type chosen by the user. This macro also sets
# the variable CMAKE_BUILD_TYPE accordingly.

VALIDATE_BUILD_TYPE()

################################################################################
# Identify the operating system

FIND_HOST_OS (
	"GENEVA_OS_NAME"
	"GENEVA_OS_VERSION"
)

###############################################################################
# Identify unsupported setups as early as possible

FLAG_UNSUPPORTED_SETUPS(
	${GENEVA_OS_NAME}
	${GENEVA_OS_VERSION}
	${GENEVA_BUILD_TYPE}
	${GENEVA_STATIC}
)

################################################################################
# Set the C++ standard to be used

# Geneva requires at least the C++14 Standard. The user may force another
# value at its own risk by setting the variable CMAKE_CXX_STANDARD.
IF( NOT DEFINED CMAKE_CXX_STANDARD )
	SET( CMAKE_CXX_STANDARD "14" )
ENDIF()

SET_CXX_STANDARD_FLAG()

################################################################################
# Set the compiler and linker flags

SET_COMPILER_FLAGS (
	${GENEVA_OS_NAME}
	${GENEVA_OS_VERSION}
	${GENEVA_BUILD_TYPE}
	${GENEVA_STATIC}
)

SET_LINKER_FLAGS (
	${GENEVA_OS_NAME}
	${GENEVA_OS_VERSION}
	${GENEVA_BUILD_TYPE}
	${GENEVA_STATIC}
)

################################################################################
# Set other necessary build flags

GET_BUILD_FLAGS (
	${GENEVA_OS_NAME}
	${GENEVA_OS_VERSION}
	${GENEVA_BUILD_TYPE}
	${GENEVA_STATIC}
	"PLATFORM_NEEDS_LIBRARY_LINKING"
)

################################################################################
# Set the build mode static or dynamic

IF ( GENEVA_STATIC )
	SET (BUILD_SHARED_LIBS OFF)
ELSE () # dynamic libraries
	SET (BUILD_SHARED_LIBS ON)
	# This preprocessor definition is required for knowing
	# if API-exporting is needed in the code or not
	ADD_DEFINITIONS("-DGEM_DYNAMIC")
ENDIF ()

################################################################################
# Set the preprocessor definition for enabling testing code

IF ( GENEVA_BUILD_TESTS )
	ADD_DEFINITIONS("-DGEM_TESTING")
ENDIF ()

################################################################################
# Define the required Boost environment

SET (Boost_USE_MULTITHREAD ON)
SET (Boost_ADDITIONAL_VERSIONS "1.57" "1.57.0" "1.58" "1.58.0" "1.59" "1.59.0")

IF ( GENEVA_STATIC )
	SET (Boost_USE_STATIC_LIBS ON)
ELSE () # Dynamic libraries
	SET (Boost_USE_STATIC_LIBS OFF)
	IF(WIN32)
		# Disable auto-linking
		ADD_DEFINITIONS("-DBOOST_ALL_DYN_LINK")

		# Boost::test_exec_monitor cannot be built as shared libraries,
		# which leads to problems with FindBoost under Windows when trying
		# to build the other libraries as dynamic. That case is unsupported.
		IF (GENEVA_BUILD_TESTS)
			MESSAGE (FATAL_ERROR "Building shared libraries with testing"
				 " code under Windows is currently not suported."
				 " Please set GENEVA_STATIC=TRUE or GENEVA_BUILD_TESTS=FALSE.")
		ENDIF ()

	ENDIF()
ENDIF ()

# The minimum Boost version required for building Geneva and Geneva applications
SET (GENEVA_MIN_BOOST_VERSION 1.57)

# These are the libraries required for any Geneva build
SET (
	GENEVA_BOOST_LIBS
	atomic
	date_time
	filesystem
	regex
	serialization
	system
	program_options
)

IF(WIN32)
	# Boost.Thread requires Boost.Chrono, required for linking in Windows
	SET (
		GENEVA_BOOST_LIBS
		${GENEVA_BOOST_LIBS}
		chrono
	)
ENDIF()

IF(GENEVA_BUILD_TESTS)
	SET (
		GENEVA_BOOST_LIBS
		${GENEVA_BOOST_LIBS}
		test_exec_monitor
		unit_test_framework
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
MESSAGE("")

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

# Add compile-time debug information about Boost's linked libraries
IF(WIN32 AND CMAKE_VERBOSE_MAKEFILE)
	ADD_DEFINITIONS(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})
ENDIF()

################################################################################
# The names of the Geneva libraries

SET ( COMMON_LIBNAME            "gemfony-common" )
SET ( HAP_LIBNAME               "gemfony-hap" )
SET ( COURTIER_LIBNAME          "gemfony-courtier" )
SET ( GENEVA_LIBNAME            "gemfony-geneva" )
SET ( GENEVA_INDIVIDUAL_LIBNAME "gemfony-geneva-individuals")

# The order of the entries is important, as it translates to the linking
# order in TARGET_LINK_LIBRARIES() later...
SET (
	GENEVA_LIBNAMES
	${GENEVA_INDIVIDUAL_LIBNAME}
	${GENEVA_LIBNAME}
	${COURTIER_LIBNAME}
	${HAP_LIBNAME}
	${COMMON_LIBNAME}
)

# This variable contains the library names. In case of an independent build,
# it is overwritten later by FindGeneva, with the list of full library paths.
# The function TARGET_LINK_LIBRARIES() can use either variant.
SET ( GENEVA_LIBRARIES ${GENEVA_LIBNAMES} )

################################################################################
# Setup the MPI mode if required

IF( GENEVA_WITH_MPI )
	IF( UNIX AND NOT ${GENEVA_OS_NAME} MATCHES "MacOSX" )
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
# Add additional libraries if required

IF(UNIX)
	FIND_LIBRARY( PTHREAD_LIBRARY NAMES pthread
		DOC "The threading library needed by Geneva"
	)
	IF( GENEVA_STATIC )
		FIND_LIBRARY( DL_LIBRARY NAMES dl
			DOC "The dl library needed for statically linking Geneva"
		)
		FIND_LIBRARY( Z_LIBRARY NAMES z
			DOC "The z library needed for statically linking Geneva"
		)
	ENDIF()
ENDIF()

################################################################################
# On a Unix-system, add a custom target to run a "make clean" and remove
# temporaries, so the configuration process may start fresh. The make
# command will not be available on windows.

IF (UNIX AND NOT TARGET "clean-cmake")
	ADD_CUSTOM_TARGET(
		"clean-cmake"
		COMMAND ${CMAKE_BUILD_TOOL} clean 2>&1 > /dev/null
		COMMAND ${CMAKE_COMMAND} -DOLD_CMAKE=${OLD_CMAKE} -P ${PROJECT_SOURCE_DIR}/CMakeModules/CleanCmakeTemporaries.cmake
	)
ENDIF ()

################################################################################
# Search for Geneva if this is an out-of-tree build of some Geneva application

IF (NOT GENEVA_FULL_TREE_BUILD)

	# This command sets the variables
	#   GENEVA_INCLUDE_DIR, GENEVA_LIBRARY_DIR, GENEVA_LIBRARIES, etc.
	MESSAGE("Searching for Geneva...\n")
	FIND_PACKAGE (Geneva REQUIRED)
	MESSAGE("")

	IF (NOT GENEVA_FOUND)
		MESSAGE (FATAL_ERROR "Geneva not found, can't continue!")
	ENDIF ()

	IF (GENEVA_TESTING AND NOT GENEVA_BUILD_TESTS)
		# If Geneva was built with testing, the application could be built
		# without, but we would still need to add Boost's test_exec_monitor
		# library for avoiding linking errors... that case is unsupported for now.
		MESSAGE (FATAL_ERROR "Geneva was built with testing support,"
			             " building a Geneva application without testing"
			             " is not suported. Please set GENEVA_BUILD_TESTS=TRUE .")
	ELSEIF (GENEVA_BUILD_TESTS AND NOT GENEVA_TESTING)
		# This case doesn't really make sense...
		MESSAGE (FATAL_ERROR "Geneva was built without testing support,"
			             " cannot build a Geneva application with testing."
			             " Please set GENEVA_BUILD_TESTS=FALSE .")
	ENDIF ()

	INCLUDE_DIRECTORIES(${GENEVA_INCLUDE_DIR})

ENDIF ()

################################################################################
# Set the installation locations

IF (GENEVA_FULL_TREE_BUILD)

	IF (INSTALL_PREFIX_INCLUDES AND INSTALL_PREFIX_LIBS
			AND INSTALL_PREFIX_DOCS AND INSTALL_PREFIX_DATA)
		# All are set, we ignore CMAKE_INSTALL_PREFIX and install
		# each kind of files in its own location
		SET( INFO_INSTALL_PREFIX                       "\n\t\t(libs)\t\t${INSTALL_PREFIX_LIBS}" )
		SET( INFO_INSTALL_PREFIX "${INFO_INSTALL_PREFIX}\n\t\t(headers)\t${INSTALL_PREFIX_INCLUDES}" )
		SET( INFO_INSTALL_PREFIX "${INFO_INSTALL_PREFIX}\n\t\t(docs)\t\t${INSTALL_PREFIX_DOCS}" )
		SET( INFO_INSTALL_PREFIX "${INFO_INSTALL_PREFIX}\n\t\t(other files)\t${INSTALL_PREFIX_DATA}" )
	ELSEIF (NOT INSTALL_PREFIX_INCLUDES AND NOT INSTALL_PREFIX_LIBS
			AND NOT INSTALL_PREFIX_DOCS AND NOT INSTALL_PREFIX_DATA)
		# All unset, Geneva is installed as a standalone tree in CMAKE_INSTALL_PREFIX
		IF (NOT INSTALL_PREFIX_ROOT)
			IF (CMAKE_INSTALL_PREFIX)
				SET( INSTALL_PREFIX_ROOT ${CMAKE_INSTALL_PREFIX} )
			ELSE ()
				# If no value was set, use relative paths
				SET( INSTALL_PREFIX_ROOT "." )
			ENDIF ()
		ENDIF ()

		SET( INSTALL_PREFIX_INCLUDES "${INSTALL_PREFIX_ROOT}/include" )
		SET( INSTALL_PREFIX_LIBS     "${INSTALL_PREFIX_ROOT}/lib" )
		SET( INSTALL_PREFIX_DOCS     "${INSTALL_PREFIX_ROOT}" )
		SET( INSTALL_PREFIX_DATA     "${INSTALL_PREFIX_ROOT}" )
		SET( INFO_INSTALL_PREFIX     "${INSTALL_PREFIX_ROOT}" )
	ELSE ()
		# Inconsistent settings
		MESSAGE (FATAL_ERROR "Please set either all four or none of the"
			             " installation prefix values INSTALL_PREFIX_INCLUDES,"
			             " INSTALL_PREFIX_LIBS, INSTALL_PREFIX_DOCS, and INSTALL_PREFIX_DATA .")
	ENDIF ()

ELSE ()

	# This is an out-of-tree build of some Geneva application, only the
	# INSTALL_PREFIX_DATA or CMAKE_INSTALL_PREFIX are needed
	IF (NOT INSTALL_PREFIX_DATA)
		IF (CMAKE_INSTALL_PREFIX)
			SET( INSTALL_PREFIX_DATA "${CMAKE_INSTALL_PREFIX}" )
		ELSE ()
			# If no value was set, use relative paths
			SET( INSTALL_PREFIX_DATA "." )
		ENDIF ()
	ENDIF ()
	SET( INFO_INSTALL_PREFIX "${INSTALL_PREFIX_DATA}" )

ENDIF ()

################################################################################
# Print a summary of the build settings before continuing with the main script

MESSAGE ("========================================")
MESSAGE ("")
MESSAGE ("Building:")
IF (GENEVA_FULL_TREE_BUILD)
	MESSAGE ("\tthe Geneva library collection")
ELSE ()
	MESSAGE ("\ta Geneva application")
ENDIF ()
IF (GENEVA_STATIC)
	MESSAGE ("\tstatically linked")
ELSE ()
	MESSAGE ("\tdynamically linked")
ENDIF ()
IF (GENEVA_BUILD_TESTS)
	MESSAGE ("\tincluding testing code")
ELSE ()
	MESSAGE ("\twithout testing code")
ENDIF ()
# Don't print the useless build type on multi-config generators
IF(NOT CMAKE_CONFIGURATION_TYPES)
	MESSAGE ("\tin ${CMAKE_BUILD_TYPE} mode")
	IF ("${GENEVA_BUILD_TYPE}" STREQUAL "Sanitize")
		MESSAGE ("\twith -fsanitize=thread (if available)")
	ENDIF ()
ENDIF ()

MESSAGE ("\twith Boost include location:\t ${Boost_INCLUDE_DIRS}")
MESSAGE ("\twith Boost library location:\t ${Boost_LIBRARY_DIRS}")
IF (NOT GENEVA_FULL_TREE_BUILD)
	MESSAGE ("\twith Geneva include location:\t ${GENEVA_INCLUDE_DIR}")
	MESSAGE ("\twith Geneva library location:\t ${GENEVA_LIBRARY_DIR}")
ENDIF ()
MESSAGE ("\tusing compiler:\t\t\t ${CMAKE_CXX_COMPILER_ID} v${CMAKE_CXX_COMPILER_VERSION}")
# Don't try to access the build type on multi-config generators
IF(NOT CMAKE_CONFIGURATION_TYPES)
	STRING (TOUPPER ${CMAKE_BUILD_TYPE} B_MODE)
	SET (CMAKE_CXX_FLAGS_ALL "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${B_MODE}}")
	STRING (STRIP "${CMAKE_CXX_FLAGS_ALL}" CMAKE_CXX_FLAGS_STRIPPED)
	STRING (REGEX REPLACE "[ \t]+" "\n\t\t\t\t\t " CMAKE_CXX_FLAGS_SEP ${CMAKE_CXX_FLAGS_STRIPPED})
	MESSAGE ("\twith C++ compiler flags:\t ${CMAKE_CXX_FLAGS_SEP}")
ENDIF ()
# Don't print linker options if empty
STRING (STRIP "${CMAKE_EXE_LINKER_FLAGS}" CMAKE_L_FLAGS_STRIPPED)
IF(NOT "${CMAKE_L_FLAGS_STRIPPED}" STREQUAL "")
	STRING (REGEX REPLACE "[ \t]+" "\n\t\t\t\t\t " CMAKE_L_FLAGS_SEP ${CMAKE_L_FLAGS_STRIPPED})
	MESSAGE ("\twith extra linker flags:\t ${CMAKE_L_FLAGS_SEP}")
ENDIF ()
IF( CMAKE_VERBOSE_MAKEFILE )
	MESSAGE ("\tproducing verbose CMake output")
ELSE()
	MESSAGE ("\tproducing sparse CMake output")
ENDIF()
MESSAGE ("\tfor operating system:\t\t ${GENEVA_OS_NAME}")
MESSAGE ("\tto install into prefix:\t\t ${INFO_INSTALL_PREFIX}")
MESSAGE ("")
MESSAGE ("========================================\n")

###############################################################################
# End of the include-guard

ENDIF(NOT COMMON_GENEVA_BUILD_INCLUDED)

################################################################################
# Done
