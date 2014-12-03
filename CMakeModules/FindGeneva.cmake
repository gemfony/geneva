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
# This file is part of the Geneva library collection's build system.
# Its purpose is to find the Geneva libraries and include files for
# building Geneva applications.
#
# This module accepts the following variables as hints for the search:
#
# GENEVA_ROOT       - The Geneva installation prefix where to search for libraries and includes
# GENEVA_INCLUDEDIR - The directory where the Geneva includes may be found
# GENEVA_LIBRARYDIR - The directory where the Geneva libraries may be found
#
#
# This module defines the following variables as result of the search:
#
# GENEVA_INCLUDE_DIR          - The directory below which Geneva includes may be found
# GENEVA_LIBRARY_DIR          - The directory below which the Geneva libraries may be found
#
# GENEVA_COMMON_LIBRARY       - Points to the library with common functionality needed by all parts of Geneva
# GENEVA_HAP_LIBRARY          - Points to the library for random number generation
# GENEVA_COURTIER_LIBRARY     - Points to the library for parallel/distributed computing
# GENEVA_OPTIMIZATION_LIBRARY - Points to the main geneva library itself, the optimization library
# GENEVA_INDIVIDUAL_LIBRARY   - Points to the library of sample individuals
#
# GENEVA_LIBRARIES            - A list containing all the Geneva libraries mentioned above
# GENEVA_LIBS                 - A list containing the names (not the paths!) of all the libraries found
#
# GENEVA_VERSION              - The Geneva version found by this module
#
# GENEVA_FOUND                - TRUE if all of the required Geneva components were found
#
#
# Example usages: FIND_PACKAGE(Geneva) or FIND_PACKAGE(Geneva REQUIRED)
#

###############################################################################

# Module for handling standard arguments given through FIND_PACKAGE()
INCLUDE(FindPackageHandleStandardArgs)

###############################################################################
# Analyze first the recieved variables

# Also check the environment variables if the CMake variables are not set
IF ((NOT GENEVA_ROOT) AND ENV{GENEVA_ROOT})
	SET (GENEVA_ROOT $ENV{GENEVA_ROOT})
ENDIF ()
IF ((NOT GENEVA_INCLUDEDIR) AND ENV{GENEVA_INCLUDEDIR})
	SET (GENEVA_INCLUDEDIR $ENV{GENEVA_INCLUDEDIR})
ENDIF ()
IF ((NOT GENEVA_LIBRARYDIR) AND ENV{GENEVA_LIBRARYDIR})
	SET (GENEVA_LIBRARYDIR $ENV{GENEVA_LIBRARYDIR})
ENDIF ()

# The more specific variables take precedence over the more generic ones
IF (GENEVA_ROOT)
	IF (NOT GENEVA_INCLUDEDIR)
		SET (GENEVA_INCLUDEDIR ${GENEVA_ROOT}/include)
	ENDIF ()
	IF (NOT GENEVA_LIBRARYDIR)
		SET (GENEVA_LIBRARYDIR ${GENEVA_ROOT}/lib)
	ENDIF ()
ENDIF ()

###############################################################################
# Find the include directories. We search for a single include
# directory only and assume that the others can be found at the
# same level 

SET (GENEVA_COMMON_HEADER_PATH "common/GGlobalDefines.hpp" )

# Enforce a search order, making sure we first search in the given
# location, and only if not found we search the normal system paths
FIND_PATH (
	GENEVA_INCLUDE_DIR
	NAMES ${GENEVA_COMMON_HEADER_PATH}
	PATHS ${GENEVA_INCLUDEDIR} NO_DEFAULT_PATH
)
FIND_PATH (
	GENEVA_INCLUDE_DIR
	NAMES ${GENEVA_COMMON_HEADER_PATH}
)

# Check that the path was indeed found
IF (NOT GENEVA_INCLUDE_DIR)
	MESSAGE (STATUS "The Geneva include directory was not found!")
ENDIF ()

###############################################################################
# Find the libraries and set the related variables

# Note: libraries are listed in order of decreasing dependencies, which
# allows to use the GENEVA_LIBRARIES variable for linking with all of
# them as a block without linking order issues (hopefully)
SET ( NAMES
	"geneva-individuals"
	"geneva"
	"courtier"
	"hap"
	"common"
)

UNSET (GENEVA_LIBRARIES)
UNSET (GENEVA_LIBS)
FOREACH ( name IN LISTS NAMES )
	STRING (TOUPPER ${name} ucname)

	# Enforce a search order, making sure we first search in the given
	# location, and only if not found we search the normal system paths
	FIND_LIBRARY (
		GENEVA_${ucname}_LIBRARY 
		NAMES "gemfony-${name}"
		PATHS ${GENEVA_LIBRARYDIR} NO_DEFAULT_PATH
	)
	FIND_LIBRARY (
		GENEVA_${ucname}_LIBRARY 
		NAMES "gemfony-${name}"
	)

	IF (GENEVA_${ucname}_LIBRARY)
		SET (GENEVA_LIBRARIES ${GENEVA_LIBRARIES} ${GENEVA_${ucname}_LIBRARY})
		SET (GENEVA_LIBS ${GENEVA_LIBS} ${name})
	ENDIF ()
ENDFOREACH ()
UNSET (NAMES)

SET (GENEVA_OPTIMIZATION_LIBRARY ${GENEVA_GENEVA_LIBRARY})
SET (GENEVA_INDIVIDUAL_LIBRARY ${GENEVA_GENEVA-INDIVIDUALS_LIBRARY})

GET_FILENAME_COMPONENT (
	GENEVA_LIBRARY_DIR 
	${GENEVA_COMMON_LIBRARY} 
	PATH
)

###############################################################################
# Check that all files and directories were properly found

# This function sets the <PKG>_FOUND variable if all the given variables
# contain valid values. To declare Geneva as "FOUND" we need at least the
# 'common' library.
FIND_PACKAGE_HANDLE_STANDARD_ARGS (
	GENEVA
	DEFAULT_MSG
	GENEVA_INCLUDE_DIR
	GENEVA_LIBRARY_DIR
	GENEVA_COMMON_LIBRARY 
)

###############################################################################
# Check that the versions found are consistent

IF (GENEVA_INCLUDE_DIR)

	# Read the Geneva version string, which is of the form '#define GENEVA_VERSION 0150'
	SET ( _VER_PRE "#define GENEVA_VERSION" )
	FILE (
		STRINGS
		${GENEVA_INCLUDE_DIR}/${GENEVA_COMMON_HEADER_PATH}
		_RAW_VERSION
		REGEX "${_VER_PRE}"
		LIMIT_COUNT 100
		LIMIT_INPUT 10000
		LIMIT_OUTPUT 100
	)
	STRING ( REPLACE "${_VER_PRE} " "" _RAW_VERSION_2 ${_RAW_VERSION} )
	STRING ( STRIP ${_RAW_VERSION_2} _RAW_VERSION_3 )
	IF ( ${_RAW_VERSION_3} MATCHES "^[0-9][0-9][0-9][0-9]([a-z][a-z0-9]*)?$")
		STRING ( REGEX REPLACE "^([0-9][0-9])([0-9])([0-9].*$)" "\\1.\\2.\\3"
			GENEVA_VERSION ${_RAW_VERSION_3}
		)
		# Remove the initial 0
		STRING ( REGEX REPLACE "^0(.*)$" "\\1"
			GENEVA_VERSION ${GENEVA_VERSION}
		)
	ELSE ()
		MESSAGE (FATAL_ERROR "Could not determine the Geneva version!")
	ENDIF ()
	MESSAGE(STATUS "Found Geneva includes version ${GENEVA_VERSION}")

ENDIF ()

MESSAGE(STATUS "Found Geneva libraries: ${GENEVA_LIBS}")

###############################################################################
# Done
