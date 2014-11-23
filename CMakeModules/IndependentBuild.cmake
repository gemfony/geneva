###############################################################################
#
# Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
#
# See the AUTHORS file in the top-level directory for a list of authors.
#
# Contact: contact [at] gemfony (dot) eu
#
# This file is part of the Geneva library collection's build-system.
# Its purpose is to support independent programs based on Geneva in the
# compilation process.
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
###############################################################################
# Includes

INCLUDE(IdentifySystemParameters)

###############################################################################
#
# This macro performs the necessary steps to setup the build system to compile
# independent Geneva applications.
#
MACRO (
	SETUP_BUILD_SYSTEM
)

	#--------------------------------------------------------------------------
	
	# Set some default values for user-settings. Generally, these should be provided
	# through a custom genevaConfig.gcfg, in conjunction with the prepareBuild.sh script.
	
	#IF( NOT DEFINED BOOST_ROOT )
	#	SET( BOOST_ROOT "/opt/boost155" )
	#ENDIF()
	 
	#IF( NOT DEFINED BOOST_LIBRARYDIR )
	#	SET( BOOST_LIBRARYDIR "/opt/boost155/lib" )
	#ENDIF()
	
	#IF( NOT DEFINED BOOST_INCLUDEDIR )
	#	SET( BOOST_INCLUDEDIR "/opt/boost155/include/boost" )
	#ENDIF()
	
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
	
	# Validates the build type chosen by the user (Release or Debug). Also sets
	# the CMAKE_BUILD_TYPE accordingly

	VALIDATE_BUILD_TYPE()
	
	# Set a number of variables related to static or dynamic build-mode

	IF ( GENEVA_STATIC ) 
		SET (BUILD_SHARED_LIBS OFF)
	ELSE() # dynamic libraries
		SET (BUILD_SHARED_LIBS ON)
	ENDIF()
	
	# Identify the operating system

	FIND_HOST_OS (
		GENEVA_OS_NAME 
		GENEVA_OS_VERSION
	)
	
	# Identify the compiler

	FIND_HOST_COMPILER (
		GENEVA_COMPILER_NAME
		GENEVA_COMPILER_VERSION
		GENEVA_MAX_CXX_STANDARD
	)

	SET(MSG_COMPILER ${GENEVA_COMPILER_NAME})
	
	# Identify unsupported setups as early as possible
	FLAG_UNSUPPORTED_SETUPS(
		GENEVA_OS_NAME 
		GENEVA_OS_VERSION
		GENEVA_COMPILER_NAME
		GENEVA_COMPILER_VERSION
		GENEVA_ACTUAL_CXX_STANDARD
		GENEVA_STATIC
	)
	
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
	
	# Boost-specific code

	SET (Boost_USE_MULTITHREAD ON)
	SET (Boost_ADDITIONAL_VERSIONS "1.57" "1.57.0" "1.58" "1.58.0")
	
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
	
	# Add MPI-Support if requested by the user
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
	
	#--------------------------------------------------------------------------

ENDMACRO()
