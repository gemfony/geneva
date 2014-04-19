###############################################################################
#
# Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
#
# See the AUTHORS file in the top-level directory for a list of authors.
#
# Contact: contact [at] gemfony (dot) eu
#
# This file is part of the Geneva library collection.
# Its purpose is to find the Geneva libraries and include files.
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

# clang settings
SET(CLANG_DEF_IDENTIFIER "Clang")
SET(CLANG_DEF_MIN_CXX11_VERSION "3.2")
SET(CLANG_DEF_MIN_CXX14_VERSION "3.4")
SET(CLANG_DEF_CXX98_STANDARD_FLAG "-std=c++98")
SET(CLANG_DEF_CXX11_STANDARD_FLAG "-std=c++11")
SET(CLANG_DEF_CXX14_STANDARD_FLAG "-std=c++14")

# gcc settings
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

# MSVC settings
SET(MSVC_DEF_IDENTIFIER "MSVC")
SET(MSVC_DEF_CXX98_STANDARD_FLAG "")
SET(MSVC_DEF_CXX11_STANDARD_FLAG "")
SET(MSVC_DEF_CXX14_STANDARD_FLAG "")

# Default compiler settings
SET(NONE_DEF_STANDARD_FLAG "")

###############################################################################
# Set the build type according to the GENEVA_BUILD_TYPE value, if defined.
# Otherwise use CMAKE_BUILD_TYPE if defined, or use build type 'Debug'
# as default.
#
MACRO (
	VALIDATE_BUILD_TYPE
)

	#--------------------------------------------------------------------------
	
	IF(GENEVA_BUILD_TYPE)
		IF(${GENEVA_BUILD_TYPE} STREQUAL "Debug")
			MESSAGE ("Setting the build type to Debug")
			SET(CMAKE_BUILD_TYPE "Debug")
		ELSEIF(${GENEVA_BUILD_TYPE} STREQUAL "Release")
			MESSAGE ("Setting the build type to Release")
			SET(CMAKE_BUILD_TYPE "Release")
		ELSE()
			MESSAGE (FATAL_ERROR "Unknown compilation mode GENEVA_BUILD_TYPE=${GENEVA_BUILD_TYPE}")
		ENDIF()
	ELSEIF(CMAKE_BUILD_TYPE) # CMAKE_BUILD_TYPE is set and not empty
		MESSAGE ("Using build type of CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
		SET(GENEVA_BUILD_TYPE "${CMAKE_BUILD_TYPE}")
	ELSE()
		MESSAGE ("Setting the build type to default value Debug")
		SET(CMAKE_BUILD_TYPE "Debug")
		SET(GENEVA_BUILD_TYPE "Debug")
	ENDIF()
	MESSAGE ("")
	
	#--------------------------------------------------------------------------

ENDMACRO()

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
		MESSAGE(FATAL_ERROR "Error: Requested C++ standard ${GENEVA_CXX_STANDARD_STRING_IN} is not supported")
	ENDIF()
	#--------------------------------------------------------------------------

ENDFUNCTION()

###############################################################################
# Assigns a standard string to an id
#
# 0 means cxx98
# 1 means cxx11
# 2 means cxx14
#
FUNCTION (
	 GET_STANDARD_STRING
	 GENEVA_CXX_STANDARD_ID_IN
	 GENEVA_CXX_STANDARD_STRING_OUT
)

	#--------------------------------------------------------------------------
	IF("${GENEVA_CXX_STANDARD_ID_IN}" MATCHES "2") 
        SET(${GENEVA_CXX_STANDARD_STRING_OUT}  "cxx14" PARENT_SCOPE)
	ELSEIF("${GENEVA_CXX_STANDARD_ID_IN}" MATCHES "1") 
        SET(${GENEVA_CXX_STANDARD_STRING_OUT}  "cxx11" PARENT_SCOPE)
	ELSEIF("${GENEVA_CXX_STANDARD_ID_IN}" MATCHES "0") 
        SET(${GENEVA_CXX_STANDARD_STRING_OUT}  "cxx98" PARENT_SCOPE)
	ELSE()
		MESSAGE(FATAL_ERROR "Error: Requested C++ standard ${GENEVA_CXX_STANDARD_ID_IN} is not supported")
	ENDIF()
	#--------------------------------------------------------------------------

ENDFUNCTION()

###############################################################################
# Checks if a desired c++ standard string matches the maximum supported version
#
FUNCTION (
	 CHECK_DESIRED_CXX_STANDARD
	 GENEVA_CXX_DESIRED_STANDARD_IN
 	 GENEVA_CXX_MAX_SUPPORTED_STANDARD_IN
	 GENEVA_CXX_DESIRED_STANDARD_SUPPORTED_OUT
)
	#--------------------------------------------------------------------------
	# Transform the two strings into numeric ids	
	GET_STANDARD_ID(${GENEVA_CXX_DESIRED_STANDARD_IN} GENEVA_CXX_DESIRED_STANDARD_NUMERIC)
	GET_STANDARD_ID(${GENEVA_CXX_MAX_SUPPORTED_STANDARD_IN} GENEVA_CXX_MAX_SUPPORTED_STANDARD_NUMERIC)

	# Compare the two numbers. The desired max supported id may
        # not be smaller than the desired id
	IF(${GENEVA_CXX_MAX_SUPPORTED_STANDARD_NUMERIC} LESS ${GENEVA_CXX_DESIRED_STANDARD_NUMERIC})
       SET(${GENEVA_CXX_DESIRED_STANDARD_SUPPORTED_OUT} "unsupported" PARENT_SCOPE)
	ELSE()
       SET(${GENEVA_CXX_DESIRED_STANDARD_SUPPORTED_OUT} "supported" PARENT_SCOPE)
	ENDIF()

    #--------------------------------------------------------------------------
	
ENDFUNCTION()

###############################################################################
# Tries to identify the operating system and version of the host system
#
FUNCTION (
	FIND_HOST_OS
	GENEVA_OS_NAME_OUT 
	GENEVA_OS_VERSION_OUT
)

    #--------------------------------------------------------------------------	
	IF(APPLE)
	    EXEC_PROGRAM(uname ARGS -r  OUTPUT_VARIABLE DARWIN_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
	    SET(${GENEVA_OS_NAME_OUT} "MacOSX" PARENT_SCOPE)
	    SET(${GENEVA_OS_VERSION_OUT} "${DARWIN_VERSION}" PARENT_SCOPE)
	ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	    EXEC_PROGRAM(uname ARGS -r  OUTPUT_VARIABLE LINUX_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
   	    SET(${GENEVA_OS_NAME_OUT} "Linux" PARENT_SCOPE)
	    SET(${GENEVA_OS_VERSION_OUT} "${LINUX_VERSION}" PARENT_SCOPE)
	ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
	    EXEC_PROGRAM(uname ARGS -r  OUTPUT_VARIABLE FREEBSD_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
   	    SET(${GENEVA_OS_NAME_OUT} "FreeBSD" PARENT_SCOPE)
	    SET(${GENEVA_OS_VERSION_OUT} "${FREEBSD_VERSION}" PARENT_SCOPE)	    
	ELSEIF(WIN32)
	    SET(${GENEVA_OS_NAME_OUT} "Windows" PARENT_SCOPE)
	    SET(${GENEVA_OS_VERSION_OUT} "unsupported" PARENT_SCOPE)
	ELSE()
   	    SET(${GENEVA_OS_NAME_OUT} "unsupported" PARENT_SCOPE)
	    SET(${GENEVA_OS_VERSION_OUT} "unsupported" PARENT_SCOPE)
	ENDIF()
	#-------------------------------------------------------------------------

ENDFUNCTION()

###############################################################################
# Checks if a desired c++ standard string matches the maximum supported version
#
FUNCTION (
	CHECK_DESIRED_CXX_STANDARD
    GENEVA_CXX_DESIRED_STANDARD_IN
 	GENEVA_CXX_MAX_SUPPORTED_STANDARD_IN
	GENEVA_CXX_DESIRED_STANDARD_SUPPORTED_OUT
)
	#--------------------------------------------------------------------------
	# Transform the two strings into numeric ids	
	GET_STANDARD_ID(${GENEVA_CXX_DESIRED_STANDARD_IN} GENEVA_CXX_DESIRED_STANDARD_NUMERIC)
	GET_STANDARD_ID(${GENEVA_CXX_MAX_SUPPORTED_STANDARD_IN} GENEVA_CXX_MAX_SUPPORTED_STANDARD_NUMERIC)

	# Compare the two numbers. The desired max supported id may
        # not be smaller than the desired id
	IF(${GENEVA_CXX_MAX_SUPPORTED_STANDARD_NUMERIC} LESS ${GENEVA_CXX_DESIRED_STANDARD_NUMERIC})
	       SET(${GENEVA_CXX_DESIRED_STANDARD_SUPPORTED_OUT} "unsupported" PARENT_SCOPE)
	ELSE()
	       SET(${GENEVA_CXX_DESIRED_STANDARD_SUPPORTED_OUT} "supported" PARENT_SCOPE)
	ENDIF()

    #--------------------------------------------------------------------------
	
ENDFUNCTION()

###############################################################################
# Tries to identify the operating system and version of the host system
#
FUNCTION (
	FIND_HOST_OS
	GENEVA_OS_NAME_OUT 
	GENEVA_OS_VERSION_OUT
)

    #--------------------------------------------------------------------------	
	IF(APPLE)
	    EXEC_PROGRAM(uname ARGS -r  OUTPUT_VARIABLE DARWIN_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
	    SET(${GENEVA_OS_NAME_OUT} "MacOSX" PARENT_SCOPE)
	    SET(${GENEVA_OS_VERSION_OUT} "${DARWIN_VERSION}" PARENT_SCOPE)
	ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	    EXEC_PROGRAM(uname ARGS -r  OUTPUT_VARIABLE LINUX_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
   	    SET(${GENEVA_OS_NAME_OUT} "Linux" PARENT_SCOPE)
	    SET(${GENEVA_OS_VERSION_OUT} "${LINUX_VERSION}" PARENT_SCOPE)
	ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
	    EXEC_PROGRAM(uname ARGS -r  OUTPUT_VARIABLE FREEBSD_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
   	    SET(${GENEVA_OS_NAME_OUT} "FreeBSD" PARENT_SCOPE)
	    SET(${GENEVA_OS_VERSION_OUT} "${FREEBSD_VERSION}" PARENT_SCOPE)	    
	ELSEIF(WIN32)
	    SET(${GENEVA_OS_NAME_OUT} "Windows" PARENT_SCOPE)
	    SET(${GENEVA_OS_VERSION_OUT} "unsupported" PARENT_SCOPE)
	ELSE()
   	    SET(${GENEVA_OS_NAME_OUT} "unsupported" PARENT_SCOPE)
	    SET(${GENEVA_OS_VERSION_OUT} "unsupported" PARENT_SCOPE)
	ENDIF()
	#-------------------------------------------------------------------------

ENDFUNCTION()

###############################################################################
# Identifies the compiler, version and the maximum fully supported C++ standard.
# GENEVA_COMPILER_NAME_OUT will be one of Clang, GNU, Intel or MSVC
# GENEVA_COMPILER_VERSION_OUT will only be filled for CMake versions >= 2.8.10
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
	# Set the the compiler version, if available
	SET(GENEVA_COMPILER_VERSION_LOCAL "")
	IF (${CMAKE_VERSION} VERSION_LESS 2.8.10)
		IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "${GNU_DEF_IDENTIFIER}")
			EXEC_PROGRAM(${CMAKE_CXX_COMPILER_ID} ARGS -dumpversion OUTPUT_VARIABLE GXX_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
			SET(GENEVA_COMPILER_VERSION_LOCAL "${GXX_VERSION}")
		ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "${CLANG_DEF_IDENTIFIER}")
			EXEC_PROGRAM(${CMAKE_CXX_COMPILER_ID} ARGS --version OUTPUT_VARIABLE CLANG_VERSION)
			STRING(REGEX REPLACE ".*based on LLVM ([0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION ${CLANG_VERSION})
			SET(GENEVA_COMPILER_VERSION_LOCAL "${CLANG_VERSION}")
		ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "${INTEL_DEF_IDENTIFIER}")
			EXEC_PROGRAM(${CMAKE_CXX_COMPILER_ID} ARGS -dumpversion OUTPUT_VARIABLE INTEL_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
			SET(GENEVA_COMPILER_VERSION_LOCAL "${INTEL_VERSION}")
		ELSE()
			SET(GENEVA_COMPILER_VERSION_LOCAL "unsupported")
		ENDIF()
	ELSE()
        # This flag is only supported as of CMake 2.8.10
		SET(GENEVA_COMPILER_VERSION_LOCAL "${CMAKE_CXX_COMPILER_VERSION}")	
	ENDIF()
		
	#--------------------------------------------------------------------------
	# Find out about the maximum C++ standard that is supported
	IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "${CLANG_DEF_IDENTIFIER}")
		# We assume that the compiler fully supports a given standard when it 
		# accepts the corresponding flag. Note that this is not entirely conclusive.
		# We might later have to use information about the compiler version.
		# However, we currently have no method to determine this information with 
		# versions of cmake older than 2.8.10 . Note that "clang -dumpversion" returns
		# the last gcc-version it pretends to be compatible with, not the "real" 
		# clang version. Hence it cannot be used. "clang --version" might be an option,
		# but we need to figure out the correct regexp. Plus, the output of this string
		# may change over time.
		CHECK_CXX_COMPILER_FLAG("${CLANG_DEF_CXX11_STANDARD_FLAG}" COMPILER_SUPPORTS_CXX11)
		CHECK_CXX_COMPILER_FLAG("${CLANG_DEF_CXX14_STANDARD_FLAG}" COMPILER_SUPPORTS_CXX14)
		IF(COMPILER_SUPPORTS_CXX14)
			SET(${GENEVA_MAX_CXX_STANDARD_OUT} "cxx14" PARENT_SCOPE)
		ELSEIF(COMPILER_SUPPORTS_CXX11)			
			SET(${GENEVA_MAX_CXX_STANDARD_OUT} "cxx11" PARENT_SCOPE)
		ELSE()
			SET(${GENEVA_MAX_CXX_STANDARD_OUT} "cxx98" PARENT_SCOPE)
		ENDIF()
	ELSEIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "${GNU_DEF_IDENTIFIER}")
		CHECK_CXX_COMPILER_FLAG("${GNU_DEF_CXX11_STANDARD_FLAG}" COMPILER_SUPPORTS_CXX11)
		CHECK_CXX_COMPILER_FLAG("${GNU_DEF_CXX14_STANDARD_FLAG}" COMPILER_SUPPORTS_CXX14)
		
		IF(COMPILER_SUPPORTS_CXX14 AND NOT ${GENEVA_COMPILER_VERSION_LOCAL} VERSION_LESS ${GNU_DEF_MIN_CXX14_VERSION})
			SET(${GENEVA_MAX_CXX_STANDARD_OUT} "cxx14" PARENT_SCOPE)
		ELSEIF(COMPILER_SUPPORTS_CXX11 AND NOT ${GENEVA_COMPILER_VERSION_LOCAL} VERSION_LESS ${GNU_DEF_MIN_CXX11_VERSION})
			SET(${GENEVA_MAX_CXX_STANDARD_OUT} "cxx11" PARENT_SCOPE)
		ELSE()
			SET(${GENEVA_MAX_CXX_STANDARD_OUT} "cxx98" PARENT_SCOPE)
		ENDIF()
	ELSEIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "${INTEL_DEF_IDENTIFIER}")
		CHECK_CXX_COMPILER_FLAG("${INTEL_DEF_CXX11_STANDARD_FLAG}" COMPILER_SUPPORTS_CXX11)
		CHECK_CXX_COMPILER_FLAG("${INTEL_DEF_CXX14_STANDARD_FLAG}" COMPILER_SUPPORTS_CXX14)
		IF(COMPILER_SUPPORTS_CXX14)
			SET(${GENEVA_MAX_CXX_STANDARD_OUT} "cxx14" PARENT_SCOPE)
		ELSEIF(COMPILER_SUPPORTS_CXX11)
			SET(${GENEVA_MAX_CXX_STANDARD_OUT} "cxx11" PARENT_SCOPE)
		ELSE()
			SET(${GENEVA_MAX_CXX_STANDARD_OUT} "cxx98" PARENT_SCOPE)
		ENDIF()
	ELSEIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "${MSVC_DEF_IDENTIFIER}")
		SET(${GENEVA_MAX_CXX_STANDARD_OUT} "unknown" PARENT_SCOPE)
	ELSE()
		SET(${GENEVA_MAX_CXX_STANDARD_OUT} "unknown" PARENT_SCOPE)
	ENDIF()
	
	#--------------------------------------------------------------------------	
	# Set the external compiler version
	SET(${GENEVA_COMPILER_VERSION_OUT} "${GENEVA_COMPILER_VERSION_LOCAL}" PARENT_SCOPE)
	
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

	SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "cxx98" PARENT_SCOPE)

	#--------------------------------------------------------------------------

	IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "${CLANG_DEF_IDENTIFIER}")
		IF("${GENEVA_CXX_STANDARD_IN}" STREQUAL "cxx14")
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${CLANG_DEF_CXX14_STANDARD_FLAG}" PARENT_SCOPE)
		ELSEIF("${GENEVA_CXX_STANDARD_IN}" STREQUAL "cxx11")
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${CLANG_DEF_CXX11_STANDARD_FLAG}" PARENT_SCOPE)
		ELSE()
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${CLANG_DEF_CXX98_STANDARD_FLAG}" PARENT_SCOPE)
		ENDIF()
	ELSEIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "${GNU_DEF_IDENTIFIER}")
		IF("${GENEVA_CXX_STANDARD_IN}" STREQUAL "cxx14")
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${GNU_DEF_CXX14_STANDARD_FLAG}" PARENT_SCOPE)
		ELSEIF("${GENEVA_CXX_STANDARD_IN}" STREQUAL "cxx11")
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${GNU_DEF_CXX11_STANDARD_FLAG}" PARENT_SCOPE)
		ELSE()
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${GNU_DEF_CXX98_STANDARD_FLAG}" PARENT_SCOPE)
		ENDIF()
	ELSEIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "${INTEL_DEF_IDENTIFIER}")
		IF("${GENEVA_CXX_STANDARD_IN}" STREQUAL "cxx14")
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${INTEL_DEF_CXX14_STANDARD_FLAG}" PARENT_SCOPE)
		ELSEIF("${GENEVA_CXX_STANDARD_IN}" STREQUAL "cxx11")
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${INTEL_DEF_CXX11_STANDARD_FLAG}" PARENT_SCOPE)
		ELSE()
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${INTEL_DEF_CXX98_STANDARD_FLAG}" PARENT_SCOPE)
		ENDIF()
	ELSEIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "${MSVC_DEF_IDENTIFIER}")
		IF("${GENEVA_CXX_STANDARD_IN}" STREQUAL "cxx14")
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${MSVC_DEF_CXX14_STANDARD_FLAG}" PARENT_SCOPE)
		ELSEIF("${GENEVA_CXX_STANDARD_IN}" STREQUAL "cxx11")
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${MSVC_DEF_CXX11_STANDARD_FLAG}" PARENT_SCOPE)
		ELSEIF("${GENEVA_CXX_STANDARD_IN}" STREQUAL "cxx98")
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${MSVC_DEF_CXX98_STANDARD_FLAG}" PARENT_SCOPE)
		ELSE() # may be "unknown" for MSVC ...
			SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${NONE_DEF_STANDARD_FLAG}" PARENT_SCOPE)
		ENDIF()
	ELSE()
		SET(${GENEVA_CXX_STANDARD_SWITCH_OUT} "${NONE_DEF_STANDARD_FLAG}" PARENT_SCOPE)
	ENDIF()
	
	#--------------------------------------------------------------------------	
	
ENDFUNCTION()

################################################################################
# Determines the actual standard to be used, depending on the desired standard
# and the maximum available standard
#
# The following cases exist:
# - The maximum supported standard is < the desired standard --> raise an error
# - The maximum supported standard is >= the desired standard --> set the desired standard
# - The auto-flag will set the compilation mode to the highest supported standard
#
FUNCTION (
	GET_ACTUAL_CXX_STANDARD
	GENEVA_CXX_DESIRED_STANDARD_IN
	GENEVA_MAX_CXX_STANDARD_IN
	GENEVA_ACTUAL_CXX_STANDARD_OUT
)

	#--------------------------------------------------------------------------	

	IF("${GENEVA_CXX_DESIRED_STANDARD_IN}" STREQUAL "auto")
        # Just assign the maximum supported standard
		SET(${GENEVA_ACTUAL_CXX_STANDARD_OUT} "${GENEVA_MAX_CXX_STANDARD_IN}" PARENT_SCOPE)
    ELSE()
        # Check that the desired C++ standard is indeed supported
        CHECK_DESIRED_CXX_STANDARD (
           ${GENEVA_CXX_DESIRED_STANDARD_IN}
           ${GENEVA_MAX_CXX_STANDARD_IN}
           GENEVA_CXX_DESIRED_STANDARD_SUPPORTED
       )
       
       IF("${GENEVA_CXX_DESIRED_STANDARD_SUPPORTED}" STREQUAL "unsupported")
           MESSAGE(FATAL_ERROR  "Error: Requested C++ standard is ${GENEVA_CXX_DESIRED_STANDARD_IN} while maximum supported standard is ${GENEVA_MAX_CXX_STANDARD_IN}")
       ENDIF()

	   SET(${GENEVA_ACTUAL_CXX_STANDARD_OUT} "${GENEVA_CXX_DESIRED_STANDARD_IN}" PARENT_SCOPE)
	ENDIF()
	 
	#--------------------------------------------------------------------------	

ENDFUNCTION()

################################################################################
# Tries to determine linker and compiler flags for this platform and compiler.
#
# GENEVA_OS_NAME_IN  the type of the host OS
# GENEVA_OS_VERSION_IN the version of the host OS
# GENEVA_COMPILER_NAME_IN the name of the compiler to be used
# GENEVA_COMPILER_VERSION_IN the version of the compiler to be used
# GENEVA_ACTUAL_CXX_STANDARD_IN must be one of cxx98, cxx11, cxx14
# GENEVA_BUILD_MODE_IN must be one of "Release" or "Debug"
# GENEVA_DYNAMIC_MODE_IN must be one of "dynamic" or "static" 
# GENEVA_COMPILER_FLAGS_OUT the resulting flags
#
FUNCTION (
	GET_COMPILER_FLAGS
	GENEVA_OS_NAME_IN 
	GENEVA_OS_VERSION_IN
	GENEVA_COMPILER_NAME_IN
	GENEVA_COMPILER_VERSION_IN
	GENEVA_ACTUAL_CXX_STANDARD_IN
	GENEVA_BUILD_MODE_IN
	GENEVA_COMPILER_FLAGS_OUT	
)
	SET(GENEVA_COMPILER_FLAGS "")

	#--------------------------------------------------------------------------	
    # Retrieve the correct standard switch for our compiler
    
	GET_CXX_STANDARD_SWITCH (
        ${GENEVA_ACTUAL_CXX_STANDARD_IN}
		GENEVA_CXX_STANDARD_SWITCH
	)	 

	#--------------------------------------------------------------------------
	# Set the compiler flags with all values that were determined above. Note
	# that we overwrite existing settings inside of GENEVA_COMPILER_FLAGS_OUT
	SET (
	    GENEVA_COMPILER_FLAGS
	    "${GENEVA_CXX_STANDARD_SWITCH}" 
	)

	#--------------------------------------------------------------------------
	# Determine the other compiler flags. We organize this by compiler,  as the same compiler may 
	# be present on multiple platforms. The chosen switches are taylored for the use with geneva
	#	
	#*****************************************************************
	IF("${GENEVA_COMPILER_NAME_IN}" MATCHES "${INTEL_DEF_IDENTIFIER}")
		SET (
		    GENEVA_COMPILER_FLAGS
		    "${GENEVA_COMPILER_FLAGS} -Wall -Wno-unused -ansi -pthread -wd1572 -wd1418 -wd981 -wd444 -wd383"
		 )

		 IF("${GENEVA_BUILD_MODE_IN}" STREQUAL "Debug") 
		     SET (
		     	 GENEVA_COMPILER_FLAGS
		    	 "${GENEVA_COMPILER_FLAGS} -g -DDEBUG"
		     )	
         ELSEIF("${GENEVA_BUILD_MODE_IN" STREQUAL "Release")			    
		     SET (
		     	 GENEVA_COMPILER_FLAGS
		    	 "${GENEVA_COMPILER_FLAGS} -O3 -DNDEBUG"
		     )
		 ELSE()
		 	MESSAGE(FATAL_ERROR "Build mode ${GENEVA_BUILD_MODE_IN} is not supported")	
		 ENDIF()
		 
	#*****************************************************************
	ELSEIF("${GENEVA_COMPILER_NAME_IN}" MATCHES "${CLANG_DEF_IDENTIFIER}")

		SET (
		    GENEVA_COMPILER_FLAGS
		    "${GENEVA_COMPILER_FLAGS} -Wall -Wno-unused -Wno-attributes -Wno-parentheses-equality -ansi -pthread"
		 )

		 IF("${GENEVA_BUILD_MODE_IN}" STREQUAL "Debug") 
		     SET (
		     	GENEVA_COMPILER_FLAGS
		    	"${GENEVA_COMPILER_FLAGS} -g -DDEBUG"
		     )	
         ELSEIF("${GENEVA_BUILD_MODE_IN}" STREQUAL "Release")			    
		     SET (
		     	GENEVA_COMPILER_FLAGS
		    	"${GENEVA_COMPILER_FLAGS} -O3 -DNDEBUG"
		     )	
 		 ELSE()
		 	MESSAGE(FATAL_ERROR "Build mode ${GENEVA_BUILD_MODE_IN} is not supported")	
		 ENDIF()

		 IF("${GENEVA_OS_NAME_IN}" STREQUAL "MacOSX") # Special provisions for MacOS
	        SET (
     	    	GENEVA_COMPILER_FLAGS
    	     	"${GENEVA_COMPILER_FLAGS} -ftemplate-depth=512 -stdlib=libstdc++"
	        )
	     ELSEIF(${GENEVA_COMPILER_VERSION_IN} VERSION_LESS 3.1)
	        SET (
     	    	GENEVA_COMPILER_FLAGS
    	     	"${GENEVA_COMPILER_FLAGS} -stdlib=libstdc++"
	        )
	     ELSE() # Newer, non-MacOS Clang-based systems
			# No special provisions
		 ENDIF()

	#*****************************************************************
	ELSEIF("${GENEVA_COMPILER_NAME_IN}" MATCHES "${GNU_DEF_IDENTIFIER}")
        SET (
 	    	GENEVA_COMPILER_FLAGS
	     	"${GENEVA_COMPILER_FLAGS} -fmessage-length=0 -fno-unsafe-math-optimizations -fno-finite-math-only -Wno-unused -Wno-attributes -pthread -ftemplate-depth-1024"
        )
	
	 	IF("${GENEVA_BUILD_MODE_IN}" STREQUAL "Debug") 
		     SET (
		     	GENEVA_COMPILER_FLAGS
		    	"${GENEVA_COMPILER_FLAGS} -g -DDEBUG"
		     )	
         ELSEIF("${GENEVA_BUILD_MODE_IN}" STREQUAL "Release")			    
		     SET (
		     	GENEVA_COMPILER_FLAGS
		    	"${GENEVA_COMPILER_FLAGS} -O3 -DNDEBUG"
		     )	
 		 ELSE()
		 	MESSAGE(FATAL_ERROR "Build mode ${GENEVA_BUILD_MODE_IN} is not supported")	
		 ENDIF()
	
	#*****************************************************************
	ELSEIF("${GENEVA_COMPILER_NAME_IN}" MATCHES "${MSVC_DEF_IDENTIFIER}")
		# No special provisions yet
		
	#*****************************************************************
	ELSE() # unknown compiler / default setting
		# No special provisions yet
		
	ENDIF()
	#*****************************************************************
	
	#--------------------------------------------------------------------------
	# Set the compiler flags with all values that were determined above. Note
	# that we overwrite existing settings inside of GENEVA_COMPILER_FLAGS_OUT
	SET (${GENEVA_COMPILER_FLAGS_OUT} "${GENEVA_COMPILER_FLAGS}" PARENT_SCOPE)
	
	#--------------------------------------------------------------------------
		
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
	GENEVA_STATIC_IN
)

	#--------------------------------------------------------------------------
	
	IF("${GENEVA_OS_NAME_IN}" STREQUAL "MacOSX")
		# Only clang is currently supported on MacOS
		IF(NOT "${GENEVA_COMPILER_NAME_IN}" STREQUAL "${CLANG_DEF_IDENTIFIER}")
			MESSAGE("######################################################################################")
			MESSAGE("# Compiler ${GENEVA_COMPILER_NAME_IN} is not supported on MacOSX. Use Clang instead. #")
			MESSAGE("######################################################################################")
			MESSAGE(FATAL_ERROR)
		ENDIF()
		
		# Only MacOS X >= 10.9 Mavericks is supported
		IF(${GENEVA_OS_VERSION_IN} VERSION_LESS 13.0)
			MESSAGE("####################################################")
			MESSAGE("# Geneva only supports MacOS X >= 10.9 / Mavericks #")
			MESSAGE("####################################################")
			MESSAGE(FATAL_ERROR)
		ENDIF()
		
		# Static linking on MacOSX is not currently supported
		IF("${GENEVA_STATIC_IN}" STREQUAL "1")
			MESSAGE("##################################################################")
			MESSAGE("# Static linking is not currently supported by Geneva on MacOS X #")
			MESSAGE("##################################################################")
			MESSAGE(FATAL_ERROR)
		ENDIF()
		
		# Only C++98 is supported at the time being on MacOS X
		IF(NOT "${GENEVA_CXX_STANDARD_IN}" STREQUAL "cxx98")
			MESSAGE("#########################################")
			MESSAGE("# Geneva only supports C++98 on MacOS X #")
			MESSAGE("#########################################")
			MESSAGE(FATAL_ERROR)
		ENDIF()
		
	ELSEIF("${GENEVA_OS_NAME_IN}" STREQUAL "Linux")
		# No restrictions at the moment
	ELSEIF("${GENEVA_OS_NAME_IN}" STREQUAL "FreeBSD")
		# No restrictions at the moment
	ELSEIF("${GENEVA_OS_NAME_IN}" STREQUAL "Windows")
		MESSAGE("################################")
		MESSAGE("# Windows is not yet supported #")
		MESSAGE("################################")
		MESSAGE(FATAL_ERROR)
	ELSEIF("${GENEVA_OS_NAME_IN}" STREQUAL "unsupported")
		MESSAGE("#####################################")
		MESSAGE("# Operating system is not supported #")
		MESSAGE("#####################################")
		MESSAGE(FATAL_ERROR)
	ELSE()
		MESSAGE("#########################################")
		MESSAGE("# ${GENEVA_OS_NAME_IN} is not supported #")
		MESSAGE("#########################################")
		MESSAGE(FATAL_ERROR)
	ENDIF()
	
	#--------------------------------------------------------------------------

ENDFUNCTION()

###############################################################################
# End of the include-guard

ENDIF(NOT IDENTIFY_SYSTEM_PARAMETERS_INCLUDED)

###############################################################################
# done

