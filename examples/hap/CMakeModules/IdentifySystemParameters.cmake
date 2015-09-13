###############################################################################
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
###############################################################################

# Include guard
IF(NOT IDENTIFY_SYSTEM_PARAMETERS_INCLUDED)
SET(IDENTIFY_SYSTEM_PARAMETERS_INCLUDED TRUE)

###############################################################################
# Includes

INCLUDE(CheckCXXCompilerFlag)

###############################################################################
# Global variables

# Clang settings
SET(CLANG_DEF_IDENTIFIER "Clang")
SET(CLANG_DEF_MIN_CXX11_VERSION "3.2")
SET(CLANG_DEF_MIN_CXX14_VERSION "3.4")
SET(CLANG_DEF_CXX98_STANDARD_FLAG "-std=c++98")
SET(CLANG_DEF_CXX11_STANDARD_FLAG "-std=c++11")
SET(CLANG_DEF_CXX14_STANDARD_FLAG "-std=c++14")

# GCC settings
SET(GNU_DEF_IDENTIFIER "GNU")
SET(GNU_DEF_MIN_CXX11_VERSION "4.7")
SET(GNU_DEF_MIN_CXX14_VERSION "4.9")
SET(GNU_DEF_CXX98_STANDARD_FLAG "-std=c++98")
SET(GNU_DEF_CXX11_STANDARD_FLAG "-std=c++11")
SET(GNU_DEF_CXX14_STANDARD_FLAG "-std=c++14")

# Intel settings
SET(INTEL_DEF_IDENTIFIER "Intel")
SET(INTEL_DEF_CXX98_STANDARD_FLAG "-std=c++98")
SET(INTEL_DEF_CXX11_STANDARD_FLAG "-std=c++11")
SET(INTEL_DEF_CXX14_STANDARD_FLAG "-std=c++14")

# MS Visual C settings
SET(MSVC_DEF_IDENTIFIER "MSVC")
SET(MSVC_DEF_MIN_CXX11_VERSION "18.0")	# Aka MS Visual C++ 12.0, MS Visual Studio 2013, actually no full support...
SET(MSVC_DEF_MIN_CXX14_VERSION "21.0")	# Hopeless at the moment...
SET(MSVC_DEF_CXX98_STANDARD_FLAG "")
SET(MSVC_DEF_CXX11_STANDARD_FLAG "")
SET(MSVC_DEF_CXX14_STANDARD_FLAG "")

# Default compiler settings
SET(NONE_DEF_STANDARD_FLAG "")

###############################################################################
# Set the available build types for multi-config generators, and validate
# and set the chosen build type in single-config generators. In this last
# case the build type is set from the value of GENEVA_BUILD_TYPE if defined,
# otherwise the value of CMAKE_BUILD_TYPE is used if defined, or the
# value 'Debug' is used as default otherwise.
#
MACRO (
	VALIDATE_BUILD_TYPE
)

	#--------------------------------------------------------------------------
	SET(GENEVA_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;MinSizeRel;Sanitize")

	# Set the available build types on multi-config generators
	IF(CMAKE_CONFIGURATION_TYPES)
		# Due to issue http://www.cmake.org/Bug/view.php?id=5811,
		# the new variable is not used in the first run, only if it is
		# already in the cache... so we force a rerun
		IF (NOT "${CMAKE_CONFIGURATION_TYPES}" STREQUAL "${GENEVA_CONFIGURATION_TYPES}")
			SET(CMAKE_CONFIGURATION_TYPES ${GENEVA_CONFIGURATION_TYPES})
			SET(CMAKE_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES} CACHE
			    STRING "The available build types" FORCE)

			MESSAGE("\n\n")
			MESSAGE("#############################################################\n")
			MESSAGE("   The configuration type values changed, to generate")
			MESSAGE("   the right IDE project files CMake must be run again!\n")
			MESSAGE("   Please re-run the same command once more...\n")
			MESSAGE("#############################################################\n\n")
			MESSAGE(FATAL_ERROR "\nThis is NOT an error, but a request to re-run this command!\n")
		ENDIF()
	ENDIF()

	# CMake sets the variable 'NDEBUG' for Release modi, but Geneva uses
	# '#ifdef DEBUG' for compiling debugging code...
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

	# The Sanitize option falls back to Debug on unsupported platforms
	SET(CMAKE_CXX_FLAGS_SANITIZE ${CMAKE_CXX_FLAGS_DEBUG} CACHE
	    STRING "Flags used by the C++ compiler during sanitize builds" FORCE)
	SET(CMAKE_C_FLAGS_SANITIZE ${CMAKE_C_FLAGS_DEBUG} CACHE
	    STRING "Flags used by the C compiler during sanitize builds" FORCE)
	SET(CMAKE_EXE_LINKER_FLAGS_SANITIZE ${CMAKE_EXE_LINKER_FLAGS_DEBUG} CACHE
	    STRING "Flags used for linking binaries during sanitize builds" FORCE)
	SET(CMAKE_SHARED_LINKER_FLAGS_SANITIZE ${CMAKE_SHARED_LINKER_FLAGS_DEBUG} CACHE
	    STRING "Flags used by the shared libraries linker during sanitize builds" FORCE)
	MARK_AS_ADVANCED(
		CMAKE_CXX_FLAGS_SANITIZE
		CMAKE_C_FLAGS_SANITIZE
		CMAKE_EXE_LINKER_FLAGS_SANITIZE
		CMAKE_SHARED_LINKER_FLAGS_SANITIZE
	)
	# Update the documentation string of CMAKE_BUILD_TYPE for GUIs
	SET(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE
	    STRING "Choose the build type, options are: Debug Release RelWithDebInfo MinSizeRel Sanitize" FORCE)

	# The build type is not needed on multi-config generators
	IF(NOT CMAKE_CONFIGURATION_TYPES)
		IF(GENEVA_BUILD_TYPE)
			IF(NOT ";${GENEVA_CONFIGURATION_TYPES};" MATCHES ";${GENEVA_BUILD_TYPE};")
				MESSAGE (FATAL_ERROR "Unknown build type GENEVA_BUILD_TYPE=${GENEVA_BUILD_TYPE}!")
			ENDIF()
			SET(MSG "Setting the build type to ${GENEVA_BUILD_TYPE}")
			SET(CMAKE_BUILD_TYPE ${GENEVA_BUILD_TYPE})
			IF(${GENEVA_BUILD_TYPE} STREQUAL "Sanitize")
				SET(MSG "${MSG}\nThis will default to Debug on systems that do not support this setting")
			ENDIF()
		ELSEIF(CMAKE_BUILD_TYPE) # GENEVA_BUILD_TYPE is unset, but CMAKE_BUILD_TYPE is set and not empty
			IF(NOT ";${GENEVA_CONFIGURATION_TYPES};" MATCHES ";${CMAKE_BUILD_TYPE};")
				MESSAGE (FATAL_ERROR "Unknown build type CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}!")
			ENDIF()
			SET(MSG "Using build type CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
			SET(GENEVA_BUILD_TYPE "${CMAKE_BUILD_TYPE}")
		ELSE() # Both variables are unset
			SET(MSG "Setting the build type to default value Debug")
			SET(CMAKE_BUILD_TYPE "Debug")
			SET(GENEVA_BUILD_TYPE "Debug")
		ENDIF()
		MESSAGE ("\n${MSG}\n")
	ENDIF ()
	#--------------------------------------------------------------------------

ENDMACRO()

###############################################################################
# Identifies the operating system and version of the host system
#
FUNCTION (
	FIND_HOST_OS
	GENEVA_OS_NAME_OUT
	GENEVA_OS_VERSION_OUT
)

	#--------------------------------------------------------------------------
	#
	# Note: CMAKE_SYSTEM_VERSION is the output of 'uname -r', and
	#       CMAKE_SYSTEM_NAME is the output of 'uname -s' on systems that
	#       support it, at least in cmake 3.x
	#
	IF(APPLE)
		EXEC_PROGRAM(uname ARGS -r  OUTPUT_VARIABLE DARWIN_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
		SET(${GENEVA_OS_NAME_OUT} "MacOSX" PARENT_SCOPE)
		SET(${GENEVA_OS_VERSION_OUT} "${DARWIN_VERSION}" PARENT_SCOPE)
	ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
		EXEC_PROGRAM(uname ARGS -r  OUTPUT_VARIABLE LINUX_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
		SET(${GENEVA_OS_NAME_OUT} "Linux" PARENT_SCOPE)
		SET(${GENEVA_OS_VERSION_OUT} "${LINUX_VERSION}" PARENT_SCOPE)
	ELSEIF(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
		EXEC_PROGRAM(uname ARGS -r  OUTPUT_VARIABLE FREEBSD_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
		SET(${GENEVA_OS_NAME_OUT} "FreeBSD" PARENT_SCOPE)
		SET(${GENEVA_OS_VERSION_OUT} "${FREEBSD_VERSION}" PARENT_SCOPE)
	ELSEIF(CYGWIN)
		EXEC_PROGRAM(uname ARGS -r  OUTPUT_VARIABLE CYGWIN_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
		SET(${GENEVA_OS_NAME_OUT} "Cygwin" PARENT_SCOPE)
		SET(${GENEVA_OS_VERSION_OUT} "${CYGWIN_VERSION}" PARENT_SCOPE)
	ELSEIF(WIN32)
		SET(${GENEVA_OS_NAME_OUT} "Windows" PARENT_SCOPE)
		# No "WIN32_VERSION" variable, no way to get the Windows "marketing version"
		SET(${GENEVA_OS_VERSION_OUT} "NT ${CMAKE_SYSTEM_VERSION}" PARENT_SCOPE)
	ELSE()
		SET(${GENEVA_OS_NAME_OUT} "unsupported" PARENT_SCOPE)
		SET(${GENEVA_OS_VERSION_OUT} "unsupported" PARENT_SCOPE)
	ENDIF()
	#-------------------------------------------------------------------------

ENDFUNCTION()

###############################################################################
# Identifies the compiler, version and the maximum fully supported C++ standard.
# GENEVA_COMPILER_NAME_OUT will be one of Clang, GNU, Intel or MSVC.
#
FUNCTION (
	FIND_HOST_COMPILER
	GENEVA_COMPILER_NAME_OUT
	GENEVA_COMPILER_VERSION_OUT
	GENEVA_MAX_CXX_STANDARD_OUT
)

	#--------------------------------------------------------------------------
	# Set the compiler name
	SET(${GENEVA_COMPILER_NAME_OUT} "${CMAKE_CXX_COMPILER_ID}" PARENT_SCOPE)

	#--------------------------------------------------------------------------
	# This flag is only supported as of CMake 2.8.10
	SET(GENEVA_COMPILER_VERSION_LOCAL "${CMAKE_CXX_COMPILER_VERSION}")

	#--------------------------------------------------------------------------
	# Set the external compiler version
	SET(${GENEVA_COMPILER_VERSION_OUT} "${GENEVA_COMPILER_VERSION_LOCAL}" PARENT_SCOPE)

	#--------------------------------------------------------------------------
	# Find out about the maximum C++ standard that is supported. We assume
	# that the compiler fully supports a given standard when it accepts the
	# corresponding flag. Note that this is not entirely conclusive, and in
	# some cases we have to use information about the compiler version.
	IF (${CMAKE_CXX_COMPILER_ID} STREQUAL ${CLANG_DEF_IDENTIFIER})
		CHECK_CXX_COMPILER_FLAG("${CLANG_DEF_CXX11_STANDARD_FLAG}" COMPILER_SUPPORTS_CXX11)
		CHECK_CXX_COMPILER_FLAG("${CLANG_DEF_CXX14_STANDARD_FLAG}" COMPILER_SUPPORTS_CXX14)
		IF(COMPILER_SUPPORTS_CXX14 AND NOT ${GENEVA_COMPILER_VERSION_LOCAL} VERSION_LESS ${CLANG_DEF_MIN_CXX14_VERSION})
			SET(${GENEVA_MAX_CXX_STANDARD_OUT} "cxx14" PARENT_SCOPE)
		ELSEIF(COMPILER_SUPPORTS_CXX11 AND NOT ${GENEVA_COMPILER_VERSION_LOCAL} VERSION_LESS ${CLANG_DEF_MIN_CXX11_VERSION})
			SET(${GENEVA_MAX_CXX_STANDARD_OUT} "cxx11" PARENT_SCOPE)
		ELSE()
			SET(${GENEVA_MAX_CXX_STANDARD_OUT} "cxx98" PARENT_SCOPE)
		ENDIF()
	ELSEIF (${CMAKE_CXX_COMPILER_ID} STREQUAL ${GNU_DEF_IDENTIFIER})
		CHECK_CXX_COMPILER_FLAG("${GNU_DEF_CXX11_STANDARD_FLAG}" COMPILER_SUPPORTS_CXX11)
		CHECK_CXX_COMPILER_FLAG("${GNU_DEF_CXX14_STANDARD_FLAG}" COMPILER_SUPPORTS_CXX14)
		IF(COMPILER_SUPPORTS_CXX14 AND NOT ${GENEVA_COMPILER_VERSION_LOCAL} VERSION_LESS ${GNU_DEF_MIN_CXX14_VERSION})
			SET(${GENEVA_MAX_CXX_STANDARD_OUT} "cxx14" PARENT_SCOPE)
		ELSEIF(COMPILER_SUPPORTS_CXX11 AND NOT ${GENEVA_COMPILER_VERSION_LOCAL} VERSION_LESS ${GNU_DEF_MIN_CXX11_VERSION})
			SET(${GENEVA_MAX_CXX_STANDARD_OUT} "cxx11" PARENT_SCOPE)
		ELSE()
			SET(${GENEVA_MAX_CXX_STANDARD_OUT} "cxx98" PARENT_SCOPE)
		ENDIF()
	ELSEIF (${CMAKE_CXX_COMPILER_ID} STREQUAL ${INTEL_DEF_IDENTIFIER})
		CHECK_CXX_COMPILER_FLAG("${INTEL_DEF_CXX11_STANDARD_FLAG}" COMPILER_SUPPORTS_CXX11)
		CHECK_CXX_COMPILER_FLAG("${INTEL_DEF_CXX14_STANDARD_FLAG}" COMPILER_SUPPORTS_CXX14)
		IF(COMPILER_SUPPORTS_CXX14)
			SET(${GENEVA_MAX_CXX_STANDARD_OUT} "cxx14" PARENT_SCOPE)
		ELSEIF(COMPILER_SUPPORTS_CXX11)
			SET(${GENEVA_MAX_CXX_STANDARD_OUT} "cxx11" PARENT_SCOPE)
		ELSE()
			SET(${GENEVA_MAX_CXX_STANDARD_OUT} "cxx98" PARENT_SCOPE)
		ENDIF()
	ELSEIF (${CMAKE_CXX_COMPILER_ID} STREQUAL ${MSVC_DEF_IDENTIFIER})
		# MSVC doesn't support flags for enabling/disabling C++ standards
		# compliance, so no way to test it...
		IF(NOT ${GENEVA_COMPILER_VERSION_LOCAL} VERSION_LESS ${MSVC_DEF_MIN_CXX14_VERSION})
			SET(${GENEVA_MAX_CXX_STANDARD_OUT} "cxx14" PARENT_SCOPE)
		ELSEIF(NOT ${GENEVA_COMPILER_VERSION_LOCAL} VERSION_LESS ${MSVC_DEF_MIN_CXX11_VERSION})
			SET(${GENEVA_MAX_CXX_STANDARD_OUT} "cxx11" PARENT_SCOPE)
		ELSE()
			SET(${GENEVA_MAX_CXX_STANDARD_OUT} "cxx98" PARENT_SCOPE)
		ENDIF()
	ELSE()
		SET(${GENEVA_MAX_CXX_STANDARD_OUT} "unknown" PARENT_SCOPE)
	ENDIF()
	MESSAGE ("")
	#--------------------------------------------------------------------------

ENDFUNCTION()

###############################################################################
# Assigns an id to a given standard string
#
# 0 means cxx98
# 1 means cxx11
# 2 means cxx14
#
FUNCTION (
	GET_STANDARD_ID
	GENEVA_CXX_STANDARD_STRING_IN
	GENEVA_CXX_STANDARD_ID_OUT
)

	#--------------------------------------------------------------------------
	IF (${GENEVA_CXX_STANDARD_STRING_IN} MATCHES "cxx14")
		SET(${GENEVA_CXX_STANDARD_ID_OUT} "2" PARENT_SCOPE)
	ELSEIF (${GENEVA_CXX_STANDARD_STRING_IN} MATCHES "cxx11")
		SET(${GENEVA_CXX_STANDARD_ID_OUT} "1" PARENT_SCOPE)
	ELSEIF (${GENEVA_CXX_STANDARD_STRING_IN} MATCHES "cxx98")
		SET(${GENEVA_CXX_STANDARD_ID_OUT} "0" PARENT_SCOPE)
	ELSE()
		MESSAGE(FATAL_ERROR "Requested C++ standard GENEVA_CXX_STD=${GENEVA_CXX_STANDARD_STRING_IN}"
			" is not supported!")
	ENDIF()
	#--------------------------------------------------------------------------

ENDFUNCTION()

###############################################################################
# Checks if a desired c++ standard string matches the minimum required and
# the maximum supported version
#
FUNCTION (
	CHECK_DESIRED_CXX_STANDARD
	GENEVA_CXX_DESIRED_STANDARD_IN
	GENEVA_CXX_MIN_REQUIRED_STANDARD_IN
	GENEVA_CXX_MAX_SUPPORTED_STANDARD_IN
	GENEVA_CXX_DESIRED_STANDARD_SUPPORTED_OUT
)

	#--------------------------------------------------------------------------
	# Transform the two strings into numeric ids
	GET_STANDARD_ID(${GENEVA_CXX_DESIRED_STANDARD_IN} GENEVA_CXX_DESIRED_STANDARD_NUMERIC)
	GET_STANDARD_ID(${GENEVA_CXX_MIN_REQUIRED_STANDARD_IN} GENEVA_CXX_MIN_REQUIRED_STANDARD_NUMERIC)
	GET_STANDARD_ID(${GENEVA_CXX_MAX_SUPPORTED_STANDARD_IN} GENEVA_CXX_MAX_SUPPORTED_STANDARD_NUMERIC)

	# Check that the compiler maximum supported standard is at least equal
	# to the minimum standard required by Geneva
	IF(${GENEVA_CXX_MAX_SUPPORTED_STANDARD_NUMERIC} LESS ${GENEVA_CXX_MIN_REQUIRED_STANDARD_NUMERIC})
		MESSAGE(FATAL_ERROR "The maximum C++ standard supported by your"
			" compiler (${GENEVA_CXX_MAX_SUPPORTED_STANDARD_IN})"
			" is older than the minimum standard required to compile"
			" Geneva (${GENEVA_MIN_CXX_STANDARD_IN})!")
	ENDIF()

	# Compare the three numbers. The desired standard may not be lower
	# than the minimum required, nor higher than the maximum supported value.
	IF((${GENEVA_CXX_DESIRED_STANDARD_NUMERIC} LESS ${GENEVA_CXX_MIN_REQUIRED_STANDARD_NUMERIC})
	   OR
	   (${GENEVA_CXX_MAX_SUPPORTED_STANDARD_NUMERIC} LESS ${GENEVA_CXX_DESIRED_STANDARD_NUMERIC}))
		SET(${GENEVA_CXX_DESIRED_STANDARD_SUPPORTED_OUT} "unsupported" PARENT_SCOPE)
	ELSE()
		SET(${GENEVA_CXX_DESIRED_STANDARD_SUPPORTED_OUT} "supported" PARENT_SCOPE)
	ENDIF()
	#--------------------------------------------------------------------------

ENDFUNCTION()

################################################################################
# Determines the actual standard to be used, depending on the desired, the
# minimum required, and the maximum available standards
#
# The following cases exist:
# - The maximum supported standard is < the desired standard --> raise an error
# - The maximum supported standard is >= the desired standard --> set the desired standard
# - The auto-flag will set the compilation mode to the highest supported standard
#
FUNCTION (
	GET_ACTUAL_CXX_STANDARD
	GENEVA_CXX_DESIRED_STANDARD_IN
	GENEVA_MIN_CXX_STANDARD_IN
	GENEVA_MAX_CXX_STANDARD_IN
	GENEVA_ACTUAL_CXX_STANDARD_OUT
)

	#--------------------------------------------------------------------------
	IF(${GENEVA_CXX_DESIRED_STANDARD_IN} STREQUAL "auto")
		# Just assign the maximum supported standard
		SET(${GENEVA_ACTUAL_CXX_STANDARD_OUT} "${GENEVA_MAX_CXX_STANDARD_IN}" PARENT_SCOPE)
	ELSE()
		# Check that the desired C++ standard is indeed supported
		CHECK_DESIRED_CXX_STANDARD (
			${GENEVA_CXX_DESIRED_STANDARD_IN}
			${GENEVA_MIN_CXX_STANDARD_IN}
			${GENEVA_MAX_CXX_STANDARD_IN}
			GENEVA_CXX_DESIRED_STANDARD_SUPPORTED
		)

		IF(${GENEVA_CXX_DESIRED_STANDARD_SUPPORTED} STREQUAL "unsupported")
			MESSAGE(FATAL_ERROR "The requested C++ standard (${GENEVA_CXX_DESIRED_STANDARD_IN})"
				" is incompatible with the minimum required (${GENEVA_MIN_CXX_STANDARD_IN}),"
				" or the maximum supported standards (${GENEVA_MAX_CXX_STANDARD_IN})!")
		ENDIF()

		SET(${GENEVA_ACTUAL_CXX_STANDARD_OUT} "${GENEVA_CXX_DESIRED_STANDARD_IN}" PARENT_SCOPE)
	ENDIF()
	#--------------------------------------------------------------------------

ENDFUNCTION()

###############################################################################
# Retrieves the correct flag for a given C++ standard, based on the compiler
#
FUNCTION (
	GET_CXX_STANDARD_SWITCH
	GENEVA_CXX_STANDARD_IN
	GENEVA_CXX_STANDARD_SWITCH_OUT
)

	#--------------------------------------------------------------------------
	SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "cxx98" PARENT_SCOPE)

	IF (${CMAKE_CXX_COMPILER_ID} STREQUAL ${CLANG_DEF_IDENTIFIER})
		IF(${GENEVA_CXX_STANDARD_IN} STREQUAL "cxx14")
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${CLANG_DEF_CXX14_STANDARD_FLAG}" PARENT_SCOPE)
		ELSEIF(${GENEVA_CXX_STANDARD_IN} STREQUAL "cxx11")
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${CLANG_DEF_CXX11_STANDARD_FLAG}" PARENT_SCOPE)
		ELSE()
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${CLANG_DEF_CXX98_STANDARD_FLAG}" PARENT_SCOPE)
		ENDIF()
	ELSEIF (${CMAKE_CXX_COMPILER_ID} STREQUAL ${GNU_DEF_IDENTIFIER})
		IF(${GENEVA_CXX_STANDARD_IN} STREQUAL "cxx14")
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${GNU_DEF_CXX14_STANDARD_FLAG}" PARENT_SCOPE)
		ELSEIF(${GENEVA_CXX_STANDARD_IN} STREQUAL "cxx11")
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${GNU_DEF_CXX11_STANDARD_FLAG}" PARENT_SCOPE)
		ELSE()
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${GNU_DEF_CXX98_STANDARD_FLAG}" PARENT_SCOPE)
		ENDIF()
	ELSEIF (${CMAKE_CXX_COMPILER_ID} STREQUAL ${INTEL_DEF_IDENTIFIER})
		IF(${GENEVA_CXX_STANDARD_IN} STREQUAL "cxx14")
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${INTEL_DEF_CXX14_STANDARD_FLAG}" PARENT_SCOPE)
		ELSEIF(${GENEVA_CXX_STANDARD_IN} STREQUAL "cxx11")
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${INTEL_DEF_CXX11_STANDARD_FLAG}" PARENT_SCOPE)
		ELSE()
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${INTEL_DEF_CXX98_STANDARD_FLAG}" PARENT_SCOPE)
		ENDIF()
	ELSEIF (${CMAKE_CXX_COMPILER_ID} STREQUAL ${MSVC_DEF_IDENTIFIER})
		IF(${GENEVA_CXX_STANDARD_IN} STREQUAL "cxx14")
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${MSVC_DEF_CXX14_STANDARD_FLAG}" PARENT_SCOPE)
		ELSEIF(${GENEVA_CXX_STANDARD_IN} STREQUAL "cxx11")
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${MSVC_DEF_CXX11_STANDARD_FLAG}" PARENT_SCOPE)
		ELSEIF(${GENEVA_CXX_STANDARD_IN} STREQUAL "cxx98")
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${MSVC_DEF_CXX98_STANDARD_FLAG}" PARENT_SCOPE)
		ENDIF()
	ELSE()
		# Unsupported compiler
		SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${NONE_DEF_STANDARD_FLAG}" PARENT_SCOPE)
	ENDIF()
	#--------------------------------------------------------------------------

ENDFUNCTION()

################################################################################
# Sets the compiler flags for this platform and compiler
#
FUNCTION (
	SET_COMPILER_FLAGS
	GENEVA_OS_NAME_IN
	GENEVA_OS_VERSION_IN
	GENEVA_COMPILER_NAME_IN
	GENEVA_COMPILER_VERSION_IN
	GENEVA_ACTUAL_CXX_STANDARD_IN
	GENEVA_BUILD_MODE_IN
	GENEVA_STATIC_FLAG_IN
)

	#--------------------------------------------------------------------------
	# Retrieve the C++-standard switch for our compiler
	GET_CXX_STANDARD_SWITCH (
		${GENEVA_ACTUAL_CXX_STANDARD_IN}
		"GENEVA_CXX_STANDARD_SWITCH"
	)

	# We may use ADD_COMPILE_OPTIONS() with CMake 2.8.12 or newer.
	# We cannot redefine CMAKE_CXX_FLAGS in PARENT_SCOPE more than
	# once, so we use a local variable.
	SET(FLAGS_LOCAL "${CMAKE_CXX_FLAGS} ${GENEVA_CXX_STANDARD_SWITCH}")

	#--------------------------------------------------------------------------
	# Determine the other compiler flags. We organize this by compiler, as the
	# same compiler may be present on multiple platforms. The chosen switches
	# are tailored for the use with Geneva.
	#
	#*****************************************************************
	IF(GENEVA_COMPILER_NAME_IN MATCHES ${INTEL_DEF_IDENTIFIER})

		SET(FLAGS_LOCAL "${FLAGS_LOCAL} -Wall -Wno-unused -wd1572 -wd1418 -wd981 -wd444 -wd383 -pthread")

	#*****************************************************************
	ELSEIF(GENEVA_COMPILER_NAME_IN MATCHES ${CLANG_DEF_IDENTIFIER})

		SET(FLAGS_LOCAL "${FLAGS_LOCAL} -Wall -Wno-unused -Wno-attributes -Wno-parentheses-equality -Wno-deprecated-register -pthread")

		# CLang 3.0 does not seem to support -ftemplate-depth
		IF(${GENEVA_COMPILER_VERSION_IN} VERSION_GREATER 3.0)
			SET(FLAGS_LOCAL "${FLAGS_LOCAL} -ftemplate-depth=512")
		ENDIF()

		# For older Clang versions, or Clang on MacOSX we require the standard C++ library
		IF(${GENEVA_OS_NAME_IN} STREQUAL "MacOSX"
				OR ${GENEVA_COMPILER_VERSION_IN} VERSION_LESS 3.1)
			SET(FLAGS_LOCAL "${FLAGS_LOCAL} -stdlib=libstdc++")
		ENDIF()

		# Switch on Googles thread-sanitizer if this is a supported platform and the feature was requested.
		# Compare http://googletesting.blogspot.ru/2014/06/threadsanitizer-slaughtering-data-races.html
		IF(${GENEVA_COMPILER_VERSION_IN} VERSION_GREATER 3.2)
			SET(CMAKE_CXX_FLAGS_SANITIZE "${CMAKE_CXX_FLAGS_SANITIZE} -fsanitize=thread" PARENT_SCOPE)
		ENDIF()

	#*****************************************************************
	ELSEIF(GENEVA_COMPILER_NAME_IN MATCHES ${GNU_DEF_IDENTIFIER})

		SET(FLAGS_LOCAL "${FLAGS_LOCAL} -fmessage-length=0 -fno-unsafe-math-optimizations -fno-finite-math-only -pthread")

		# Gcc 4.4 or older does need a different option syntax
		IF(${GENEVA_COMPILER_VERSION_IN} VERSION_GREATER 4.5)
			SET(FLAGS_LOCAL "${FLAGS_LOCAL} -ftemplate-depth=1024")
		ELSE()
			SET(FLAGS_LOCAL "${FLAGS_LOCAL} -ftemplate-depth-1024")
		ENDIF()

		# GCC 4.8 on Cygwin does not provide the math constants (M_PI...) by
		# default (pure ANSI standard), unless _XOPEN_SOURCE=500 is set, see
		# http://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html
		IF(${GENEVA_OS_NAME_IN} STREQUAL "Cygwin")
			SET(FLAGS_LOCAL "${FLAGS_LOCAL} -D_XOPEN_SOURCE=500")
		ENDIF()

		# Switch on Googles thread-sanitizer if this is a supported platform and the feature was requested.
		# Compare http://googletesting.blogspot.ru/2014/06/threadsanitizer-slaughtering-data-races.html
		IF(${GENEVA_COMPILER_VERSION_IN} VERSION_GREATER 4.8)
			SET(CMAKE_CXX_FLAGS_SANITIZE "${CMAKE_CXX_FLAGS_SANITIZE} -fsanitize=thread" PARENT_SCOPE)
		ENDIF()

	#*****************************************************************
	ELSEIF(GENEVA_COMPILER_NAME_IN MATCHES ${MSVC_DEF_IDENTIFIER})

		# Compiling the most involved classes requires bigger object resources
		SET(FLAGS_LOCAL "${FLAGS_LOCAL} /bigobj")

	ENDIF()
	#--------------------------------------------------------------------------
	# We cannot redefine CMAKE_CXX_FLAGS in PARENT_SCOPE more than once
	SET(CMAKE_CXX_FLAGS "${FLAGS_LOCAL}" PARENT_SCOPE)

	#--------------------------------------------------------------------------

ENDFUNCTION()

################################################################################
# Sets the linker flags for this platform and compiler
#
FUNCTION (
	SET_LINKER_FLAGS
	GENEVA_OS_NAME_IN
	GENEVA_OS_VERSION_IN
	GENEVA_COMPILER_NAME_IN
	GENEVA_COMPILER_VERSION_IN
	GENEVA_ACTUAL_CXX_STANDARD_IN
	GENEVA_BUILD_MODE_IN
	GENEVA_STATIC_FLAG_IN
)

	#--------------------------------------------------------------------------
	IF(${GENEVA_OS_NAME_IN} MATCHES "MacOSX")
		SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libstdc++" PARENT_SCOPE)
		IF( NOT GENEVA_STATIC )
			SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libstdc++" PARENT_SCOPE)
			SET (MACOSX_RPATH 1)
		ENDIF()
	ENDIF()
	#--------------------------------------------------------------------------

ENDFUNCTION()

###############################################################################
# Determines other build flags for this platform and compiler
#
FUNCTION (
	GET_BUILD_FLAGS
	GENEVA_OS_NAME_IN
	GENEVA_OS_VERSION_IN
	GENEVA_COMPILER_NAME_IN
	GENEVA_COMPILER_VERSION_IN
	GENEVA_CXX_STANDARD_IN
	GENEVA_BUILD_MODE_IN
	GENEVA_STATIC_FLAG_IN
	PLATFORM_NEEDS_LIBRARY_LINKING_OUT
)

	#--------------------------------------------------------------------------
	IF(${GENEVA_OS_NAME_IN} MATCHES "MacOSX")
		SET(${PLATFORM_NEEDS_LIBRARY_LINKING_OUT} TRUE PARENT_SCOPE)
	ELSEIF(${GENEVA_OS_NAME_IN} STREQUAL "Cygwin")
		SET(${PLATFORM_NEEDS_LIBRARY_LINKING_OUT} TRUE PARENT_SCOPE)
	ELSEIF(${GENEVA_OS_NAME_IN} STREQUAL "Windows")
		SET(${PLATFORM_NEEDS_LIBRARY_LINKING_OUT} TRUE PARENT_SCOPE)
	ELSE()
		# Linux and other Unices do not need library linking
		SET(${PLATFORM_NEEDS_LIBRARY_LINKING_OUT} FALSE PARENT_SCOPE)
	ENDIF()
	#-------------------------------------------------------------------------

ENDFUNCTION()

###############################################################################
# Identifies unsupported setups as early as possible
#
FUNCTION (
	FLAG_UNSUPPORTED_SETUPS
	GENEVA_OS_NAME_IN
	GENEVA_OS_VERSION_IN
	GENEVA_COMPILER_NAME_IN
	GENEVA_COMPILER_VERSION_IN
	GENEVA_CXX_STANDARD_IN
	GENEVA_BUILD_MODE_IN
	GENEVA_STATIC_FLAG_IN
)

	#--------------------------------------------------------------------------
	IF(${GENEVA_OS_NAME_IN} STREQUAL "MacOSX")
		# Only clang is currently supported on MacOS
		IF(NOT ${GENEVA_COMPILER_NAME_IN} STREQUAL ${CLANG_DEF_IDENTIFIER})
			MESSAGE("######################################################################################")
			MESSAGE("# Compiler ${GENEVA_COMPILER_NAME_IN} is not supported on MacOSX. Use Clang instead. #")
			MESSAGE("######################################################################################")
			MESSAGE(FATAL_ERROR "Unsupported platform!")
		ENDIF()

		# Only MacOS X >= 10.9 Mavericks is supported
		IF(${GENEVA_OS_VERSION_IN} VERSION_LESS 13.0)
			MESSAGE("####################################################")
			MESSAGE("# Geneva only supports MacOS X >= 10.9 / Mavericks #")
			MESSAGE("####################################################")
			MESSAGE(FATAL_ERROR "Unsupported platform!")
		ENDIF()

		# Static linking on MacOSX is currently not supported
		IF(GENEVA_STATIC_FLAG_IN)
			MESSAGE("##################################################################")
			MESSAGE("# Static linking is currently not supported by Geneva on MacOS X #")
			MESSAGE("##################################################################")
			MESSAGE(FATAL_ERROR "Unsupported platform!")
		ENDIF()

	ELSEIF(${GENEVA_OS_NAME_IN} STREQUAL "Linux")
		# No restrictions at the moment
	ELSEIF(${GENEVA_OS_NAME_IN} STREQUAL "FreeBSD")
		# No restrictions at the moment
	ELSEIF(${GENEVA_OS_NAME_IN} STREQUAL "Cygwin")
		# No restrictions at the moment
	ELSEIF(${GENEVA_OS_NAME_IN} STREQUAL "Windows")
		MESSAGE("\n")
		MESSAGE("#########################################################")
		MESSAGE("# Geneva support for Windows is currently EXPERIMENTAL! #")
		MESSAGE("#########################################################")
		MESSAGE("\n")
	ELSEIF(${GENEVA_OS_NAME_IN} STREQUAL "unsupported")
		MESSAGE("#####################################")
		MESSAGE("# Operating system is not supported #")
		MESSAGE("#####################################")
		MESSAGE(FATAL_ERROR "Unsupported platform!")
	ELSE()
		MESSAGE("#########################################")
		MESSAGE("# ${GENEVA_OS_NAME_IN} is not supported #")
		MESSAGE("#########################################")
		MESSAGE(FATAL_ERROR "Unsupported platform!")
	ENDIF()
	#--------------------------------------------------------------------------

ENDFUNCTION()

###############################################################################
# End of the include-guard

ENDIF(NOT IDENTIFY_SYSTEM_PARAMETERS_INCLUDED)

###############################################################################
# Done
