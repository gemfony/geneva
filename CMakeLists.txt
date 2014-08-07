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

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

################################################################################
# Make sure we first check our local modules directory
#
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")

################################################################################
# We create some examples and test programs as part of the build process.
# This variable allows their build-scripts to detect that they are being
# called from within the master build-script. This prevents redundant searches
# and checks and allows examples to be built independent of the main Geneva
# source tree. 

SET(GENEVA_DEPENDENT_BUILD TRUE)

###############################################################################
# Includes

INCLUDE(IdentifySystemParameters)

###############################################################################
# Set the library/package version

SET(VERSION_MAJOR "1")
SET(VERSION_MINOR "3")
SET(VERSION_PATCH "0")

SET(GENEVA_MIN_BOOST_VERSION 1.48)

################################################################################
#
# Set some default values for user-settings. Generally, these should be provided
# through a custom genevaConfig.gcfg, in conjunction with the prepareBuild.sh script.

IF( NOT DEFINED BOOST_ROOT )
	SET( BOOST_ROOT "/opt/boost155" )
ENDIF()
 
IF( NOT DEFINED BOOST_LIBRARYDIR )
	SET( BOOST_LIBRARYDIR "/opt/boost155/lib" )
ENDIF()

IF( NOT DEFINED BOOST_INCLUDEDIR )
	SET( BOOST_INCLUDEDIR "/opt/boost155/include/boost" )
ENDIF()

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

IF( NOT DEFINED CMAKE_INSTALL_PREFIX )
	SET( CMAKE_INSTALL_PREFIX "/opt/geneva-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
ENDIF()

# Nice formatting
MESSAGE ("")

################################################################################
# Validates the build type chosen by the user (Release or Debug). Also sets
# the CMAKE_BUILD_TYPE accordingly

VALIDATE_BUILD_TYPE()

################################################################################
# Set a number of variables related to static or dynamic build-mode

IF ( GENEVA_STATIC ) 
	SET (BUILD_SHARED_LIBS OFF)
ELSE() # dynamic libraries
	SET (BUILD_SHARED_LIBS ON)
ENDIF()

################################################################################
# Identify the operating system

FIND_HOST_OS (
	GENEVA_OS_NAME 
	GENEVA_OS_VERSION
)

################################################################################
# Identify the compiler

FIND_HOST_COMPILER (
	GENEVA_COMPILER_NAME
	GENEVA_COMPILER_VERSION
	GENEVA_MAX_CXX_STANDARD
)

SET(MSG_COMPILER ${GENEVA_COMPILER_NAME})

################################################################################
# Determine the C++ standard to be used (depending on the desired standard
# and the maximum supported standard by the chosen compiler. This function
# will raise an error if the desired standard is higher than the maximum
# supported standard. It will also resolve the "auto" setting of the standard
# by setting it to the maximum supported standard.

GET_ACTUAL_CXX_STANDARD (
	${GENEVA_CXX_STD}
	${GENEVA_MAX_CXX_STANDARD}
	GENEVA_ACTUAL_CXX_STANDARD
)

SET (MSG_CXXSTANDARDMODE "Using ${GENEVA_COMPILER_NAME} compiler with ${GENEVA_ACTUAL_CXX_STANDARD} standard")

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
# Boost-specific code

SET (Boost_USE_MULTITHREAD ON)
SET (Boost_ADDITIONAL_VERSIONS "1.49" "1.49.0" "1.50" "1.50.0" "1.51" "1.51.0" "1.52" "1.52.0" "1.53" "1.53.0" "1.54" "1.54.0" "1.55.0" "1.56.0" )

IF ( GENEVA_STATIC ) 
	SET (Boost_USE_STATIC_LIBS ON)
ELSE() # dynamic libraries
	SET (Boost_USE_STATIC_LIBS OFF)
ENDIF()  

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

# Only build tests if requested by the user
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
FIND_PACKAGE( 
	Boost 
	${GENEVA_MIN_BOOST_VERSION} REQUIRED 
	COMPONENTS ${GENEVA_BOOST_LIBS}
)

################################################################################
# Add additional libraries and flags, further error checks
#

# The MPI mode is currently only supported under Linux and BSD
IF( GENEVA_WITH_MPI )
    IF( UNIX AND NOT "${GENEVA_OS_NAME}" MATCHES "MacOSX" )
        # Search for MPI libraries and headers
        FIND_PACKAGE(MPI)
        
        # Has the library been found ? Complain if not
        IF( NOT MPI_CXX_FOUND )
            MESSAGE (FATAL_ERROR "You have requested MPI support, but no suitable MPI libraries were found. We cannot continue.")
        ENDIF()
        
    ELSE()
        # Complain
        MESSAGE (FATAL_ERROR "The MPI-mode is currently only supported under Linux and BSD")
    ENDIF()
ENDIF()

IF(UNIX)
	#---------------------------------------------------------------------------
	IF("${GENEVA_OS_NAME}" MATCHES "MacOSX")
		LINK_LIBRARIES(
			${Boost_DATE_TIME_LIBRARY}
			${Boost_FILESYSTEM_LIBRARY}
			${Boost_REGEX_LIBRARY}
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
# Include- and link-directories, libraries
#
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
# On a Unix-system, run a "make clean" and remove temporaries, so the configuration 
# process may start fresh. The make command will not be available on windows.
#
IF(UNIX)
	ADD_CUSTOM_TARGET(
		"clean-cmake" 
		COMMAND ${CMAKE_BUILD_TOOL} clean 2>&1 > /dev/null
		COMMAND ${CMAKE_COMMAND} -DOLD_CMAKE=${OLD_CMAKE} -P ${PROJECT_SOURCE_DIR}/CMakeModules/CleanCmakeTemporaries.cmake
	)
ENDIF()

################################################################################
# Custom targets for building single libraries only
#
ADD_CUSTOM_TARGET( "common"                   DEPENDS ${COMMON_LIBNAME}              COMMENT "Building only the Common library." )
ADD_CUSTOM_TARGET( "hap"                      DEPENDS ${HAP_LIBNAME}  	             COMMENT "Building only the Hap library." )
ADD_CUSTOM_TARGET( "courtier"                 DEPENDS ${COURTIER_LIBNAME}            COMMENT "Building only the Courtier library." )
ADD_CUSTOM_TARGET( "geneva"                   DEPENDS ${GENEVA_LIBNAME}              COMMENT "Building only the Geneva library." )
ADD_CUSTOM_TARGET( "geneva-individuals"       DEPENDS ${GENEVA_INDIVIDUAL_LIBNAME}   COMMENT "Building only the Geneva-Individuals library." )

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

IF("${GENEVA_BUILD_TYPE}" STREQUAL "Sanitize")
    MESSAGE ("\twith -fsanitize=thread (if available)")
ENDIF() 

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
MESSAGE ("\tBuilding for operating system ${GENEVA_OS_NAME}")
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
# Code for the creation of reference documentation for Geneva
#
FIND_PACKAGE(Doxygen)

IF( DOXYGEN_FOUND )
	CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	ADD_CUSTOM_TARGET (
		doc
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Creation of reference documentation for Geneva with 'make doc'" VERBATIM
	)
	MESSAGE("-- Reference documentation for Geneva may now be created with 'make doc'")
ELSE()
	MESSAGE("Doxygen not found. No support for the creation of reference documentation")
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
 "This is the Geneva optimization library. As the name suggests --it stands for 
 'Grid-enabled evolutionary algorithms'--, Geneva aims to provide the necessary 
 tools to perform parametric optimization in parallel on devices ranging from 
 multi-processor machines over clusters all the way to Grids and Cloud installations.
 Geneva does not want to be the fastest library of evolutionary algorithms on 
 single-processor machines, but is instead targeted at large scale problems, 
 where the evaluation of a single candidate solution will typically take longer 
 than a few seconds. Geneva was developed with kind support from Karlsruhe Institute
 of technology."
)

SET(CPACK_PACKAGE_VENDOR "Gemfony scientific")
SET(CPACK_PACKAGE_CONTACT "Gemfony scientific <contact@gemfony.eu>")
SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
SET(CPACK_PACKAGING_INSTALL_PREFIX "/opt/geneva")

# DEB specific variables. Indentation is important in the description!
SET(
	CPACK_DEBIAN_PACKAGE_DESCRIPTION 
	"${CPACK_PACKAGE_DESCRIPTION_SUMMARY} ${CPACK_AUX_PACKAGE_DESCRIPTION}"
)
SET(CPACK_DEBIAN_PACKAGE_SECTION "Science")
SET(CPACK_DEBIAN_PACKAGE_PRIORITY "Extra")
SET(
	CPACK_DEBIAN_PACKAGE_DEPENDS 
	"libboost-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0),
	libboost-date-time-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0),
	libboost-filesystem-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0),
	libboost-program-options-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0),
	libboost-regex-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0),
	libboost-serialization-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0),
	libboost-system-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0),
	libboost-thread-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0),
	libboost-test-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0)"
)

# RPM specific variables
SET(CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_AUX_PACKAGE_DESCRIPTION})
SET(CPACK_RPM_PACKAGE_LICENSE "Affero GPL v3")
SET(CPACK_RPM_PACKAGE_GROUP "Sciences/Libraries")
SET(CPACK_RPM_PACKAGE_REQUIRES "boost-devel >= ${GENEVA_MIN_BOOST_VERSION}")

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
