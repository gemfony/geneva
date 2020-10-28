################################################################################
#
# This file is part of the Geneva library collection. The following license
# applies to this file:
#
# ------------------------------------------------------------------------------
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ------------------------------------------------------------------------------
#
# Note that other files in the Geneva library collection may use a different
# license. Please see the licensing information in each file.
#
################################################################################
#
# Geneva was started by Dr. Rüdiger Berlich and was later maintained together
# with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
# information on Gemfony scientific, see http://www.gemfomy.eu .
#
# The majority of files in Geneva was released under the Apache license v2.0
# in February 2020.
#
# See the NOTICE file in the top-level directory of the Geneva library
# collection for a list of contributors and copyright information.
#
################################################################################

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
SET(CLANG_DEF_MIN_CXX14_VERSION "3.4")
SET(CLANG_DEF_MIN_CXX1Z_VERSION "3.9")
SET(CLANG_DEF_CXX14_STANDARD_FLAG "-std=c++14")
SET(CLANG_DEF_CXX1Z_STANDARD_FLAG "-std=c++1z")

# Apple Clang settings
SET(APPLECLANG_DEF_IDENTIFIER "AppleClang")
SET(APPLECLANG_DEF_MIN_CXX14_VERSION "7.3")
SET(APPLECLANG_DEF_CXX14_STANDARD_FLAG "-std=c++14")

# GCC settings
SET(GNU_DEF_IDENTIFIER "GNU")
SET(GNU_DEF_MIN_CXX14_VERSION "4.9")
SET(GNU_DEF_MIN_CXX1Z_VERSION "6.0")
SET(GNU_DEF_CXX14_STANDARD_FLAG "-std=c++14")
SET(GNU_DEF_CXX1Z_STANDARD_FLAG "-std=c++1z")

# Intel settings
SET(INTEL_DEF_IDENTIFIER "Intel")
SET(INTEL_DEF_MIN_CXX14_VERSION "16.0")
SET(INTEL_DEF_CXX14_STANDARD_FLAG "-std=c++14")

# MS Visual C settings
SET(MSVC_DEF_IDENTIFIER "MSVC")
SET(MSVC_DEF_MIN_CXX14_VERSION "21.0")	# Aka MS Visual C++ 14.0, MS Visual Studio 2015
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
		EXEC_PROGRAM(/usr/bin/uname ARGS -r  OUTPUT_VARIABLE DARWIN_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
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

################################################################################
# Sets the compiler flags for this platform and compiler
#
FUNCTION (
	SET_COMPILER_FLAGS
	GENEVA_OS_NAME_IN
	GENEVA_OS_VERSION_IN
	GENEVA_BUILD_MODE_IN
	GENEVA_STATIC_FLAG_IN
)

	# We may use ADD_COMPILE_OPTIONS() but that sets the options in the current
	# scope only. We cannot redefine CMAKE_CXX_FLAGS in PARENT_SCOPE more than
	# once, so we use a local variable.
	SET(FLAGS_LOCAL "${CMAKE_CXX_FLAGS}")

	#--------------------------------------------------------------------------
	# Determine the necessary compiler flags. We organize this by compiler, as
	# the same compiler may be present on multiple platforms. The chosen switches
	# are tailored for the use with Geneva.
	#
	#*****************************************************************
	IF(CMAKE_CXX_COMPILER_ID MATCHES ${INTEL_DEF_IDENTIFIER})

		SET(FLAGS_LOCAL "${FLAGS_LOCAL} -Wall -Wno-unused -wd1572 -wd1418 -wd981 -wd444 -wd383 -pthread")

	#*****************************************************************
	ELSEIF(CMAKE_CXX_COMPILER_ID MATCHES ${CLANG_DEF_IDENTIFIER} OR CMAKE_CXX_COMPILER_ID MATCHES ${APPLECLANG_DEF_IDENTIFIER})

		SET(FLAGS_LOCAL "${FLAGS_LOCAL} -Wall -Wno-unused -Wno-attributes -Wno-parentheses-equality -Wno-deprecated-register")
		SET(FLAGS_LOCAL "${FLAGS_LOCAL} -ftemplate-depth=512 -pthread")

		# For Clang on MacOSX we require the standard C++ library
		IF(${GENEVA_OS_NAME_IN} STREQUAL "MacOSX")
			SET(FLAGS_LOCAL "${FLAGS_LOCAL} -stdlib=libc++")
		ELSEIF(${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 3.7)
			# Avoid https://llvm.org/bugs/show_bug.cgi?id=18402
			# when using older libstdc++ versions
			SET(FLAGS_LOCAL "${FLAGS_LOCAL} -stdlib=libc++")
		ENDIF()

		# Set the parameters for Google's thread-sanitizer
		# See http://googletesting.blogspot.ru/2014/06/threadsanitizer-slaughtering-data-races.html
		SET(CMAKE_CXX_FLAGS_SANITIZE "${CMAKE_CXX_FLAGS_SANITIZE} -fsanitize=thread" PARENT_SCOPE)

	#*****************************************************************
	ELSEIF(CMAKE_CXX_COMPILER_ID MATCHES ${GNU_DEF_IDENTIFIER})

		SET(FLAGS_LOCAL "${FLAGS_LOCAL} -fno-unsafe-math-optimizations -fno-finite-math-only")
		SET(FLAGS_LOCAL "${FLAGS_LOCAL} -fmessage-length=0 -ftemplate-depth=1024 -pthread")

		# GCC 4.8 on Cygwin does not provide the math constants (M_PI...) by
		# default (pure ANSI standard), unless _XOPEN_SOURCE=500 is set, see
		# http://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html
		IF(${GENEVA_OS_NAME_IN} STREQUAL "Cygwin")
			SET(FLAGS_LOCAL "${FLAGS_LOCAL} -D_XOPEN_SOURCE=500")
		ENDIF()

		# Set the parameters for Google's thread-sanitizer
		# See http://googletesting.blogspot.ru/2014/06/threadsanitizer-slaughtering-data-races.html
		SET(CMAKE_CXX_FLAGS_SANITIZE "${CMAKE_CXX_FLAGS_SANITIZE} -fsanitize=thread" PARENT_SCOPE)

	#*****************************************************************
	ELSEIF(CMAKE_CXX_COMPILER_ID MATCHES ${MSVC_DEF_IDENTIFIER})

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
	GENEVA_BUILD_MODE_IN
	GENEVA_STATIC_FLAG_IN
)

	#--------------------------------------------------------------------------
	IF(CMAKE_CXX_COMPILER_ID MATCHES ${CLANG_DEF_IDENTIFIER} OR CMAKE_CXX_COMPILER_ID MATCHES ${APPLECLANG_DEF_IDENTIFIER})

		# For Clang on MacOSX we require the standard C++ library
		IF(${GENEVA_OS_NAME_IN} STREQUAL "MacOSX")
			SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++" PARENT_SCOPE)
			IF(NOT GENEVA_STATIC)
				SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++" PARENT_SCOPE)
				SET (MACOSX_RPATH 1)
			ENDIF()
		ELSEIF(${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 3.7)
			# Avoid https://llvm.org/bugs/show_bug.cgi?id=18402
			# when using older libstdc++ versions
			SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++" PARENT_SCOPE)
			IF(NOT GENEVA_STATIC)
				SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++" PARENT_SCOPE)
			ENDIF()
		ENDIF()

	#*****************************************************************
	ELSEIF(CMAKE_CXX_COMPILER_ID MATCHES ${GNU_DEF_IDENTIFIER})

		# For GCC version < 9.0 add the filesystem library explicitely
		IF(${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 9.0)
			#SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++fs" PARENT_SCOPE)
			SET(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -lstdc++fs" PARENT_SCOPE)
			IF(NOT GENEVA_STATIC)
				#SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lstdc++fs" PARENT_SCOPE)
				SET(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -lstdc++fs" PARENT_SCOPE)
			ENDIF()
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
	GENEVA_BUILD_MODE_IN
	GENEVA_STATIC_FLAG_IN
)

	#--------------------------------------------------------------------------
	IF(${GENEVA_OS_NAME_IN} STREQUAL "MacOSX")
		# Only clang is currently supported on MacOS
		IF(NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL ${CLANG_DEF_IDENTIFIER} AND NOT CMAKE_CXX_COMPILER_ID STREQUAL ${APPLECLANG_DEF_IDENTIFIER})
			MESSAGE("####################################################################################")
			MESSAGE("# Compiler ${CMAKE_CXX_COMPILER_ID} is not supported on MacOSX. Use Clang instead. #")
			MESSAGE("####################################################################################")
			MESSAGE(FATAL_ERROR "Unsupported platform ${CMAKE_CXX_COMPILER_ID} !")
		ENDIF()

		# Only MacOS X >= 10.9 Mavericks is supported
		IF(${GENEVA_OS_VERSION_IN} VERSION_LESS 13.0)
			MESSAGE("####################################################")
			MESSAGE("# Geneva only supports MacOS X >= 10.9 / Mavericks #")
			MESSAGE("####################################################")
			MESSAGE(FATAL_ERROR "Unsupported platform Darwin ${GENEVA_OS_VERSION_IN} !")
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
	# Enforce a minimum compiler version
	IF (${CMAKE_CXX_COMPILER_ID} STREQUAL ${CLANG_DEF_IDENTIFIER})
		SET(COMPILER_MIN_VER 3.5)
	ELSEIF (${CMAKE_CXX_COMPILER_ID} STREQUAL ${APPLECLANG_DEF_IDENTIFIER})
		SET(COMPILER_MIN_VER 7.3)
	ELSEIF (${CMAKE_CXX_COMPILER_ID} STREQUAL ${GNU_DEF_IDENTIFIER})
		SET(COMPILER_MIN_VER 4.8)
	ELSEIF (${CMAKE_CXX_COMPILER_ID} STREQUAL ${INTEL_DEF_IDENTIFIER})
		SET(COMPILER_MIN_VER 16.0)
	ELSEIF (${CMAKE_CXX_COMPILER_ID} STREQUAL ${MSVC_DEF_IDENTIFIER})
		SET(COMPILER_MIN_VER 19.0)
	ELSE()
		# Unsupported compiler
		MESSAGE("########################")
		MESSAGE("# Unsupported compiler #")
		MESSAGE("########################")
		MESSAGE(FATAL_ERROR "Unsupported compiler ${CMAKE_CXX_COMPILER_ID} with version ${CMAKE_CXX_COMPILER_VERSION}!")
	ENDIF()

	IF(${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS ${COMPILER_MIN_VER})
		MESSAGE("#######################################################")
		MESSAGE("# Compiler version is not supported, version too old! #")
		MESSAGE("#######################################################")
		MESSAGE(FATAL_ERROR "Unsupported compiler version ${COMPILER_MIN_VER}!")
	ENDIF()
	#--------------------------------------------------------------------------

ENDFUNCTION()

###############################################################################
# End of the include-guard

ENDIF(NOT IDENTIFY_SYSTEM_PARAMETERS_INCLUDED)

###############################################################################
# Done
