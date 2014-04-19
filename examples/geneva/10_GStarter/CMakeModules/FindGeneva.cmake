###############################################################################
#
# Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
#
# See the AUTHORS file in the top-level directory for a list of authors.
#
# Contact: contact [at] gemfony (dot) eu
#
# This file is part of the Geneva library collection's build system.
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
# This code defines the following variables:
#
# GENEVA_ROOT_DIR             The directory below which Geneva can be found
# GENEVA_INCLUDE_DIR          The directory below which Geneva includes may be found
# GENEVA_LIBRARY_DIR          The directory where the Geneva libraries may be found
# GENEVA_CMAKE_MODULES_DIR    The directory holding additional CMake modules needed by Geneva-applications
# GENEVA_BASE_LIBRARY         Points to the geneva optimization library
# GENEVA_INDIVIDUAL_LIBRARY   Points to a library of sample individuals
# GENEVA_COMMON_LIBRARY       Points to a library with common functionality needed by all parts of Geneva
# GENEVA_HAP_LIBRARY          Points to a library for random number generation
# GENEVA_COURTIER_LIBRARY     Points to a library for parallel/distributed computing
# GENEVA_LIBRARIES            A list of all Geneva libraries
# GENEVA_MIN_BOOST_VERSION    The minimum boost version accepted for this Geneva release
# GENEVA_FOUND                TRUE if all of the above components were found
#
# Example usages: FIND_PACKAGE(Geneva) or FIND_PACKAGE(Geneva REQUIRED)
#
###############################################################################

# Handles standard arguments
include(FindPackageHandleStandardArgs)

###############################################################################
# Search for the file GENEVAROOT in the search path and an environment variable

find_path (
  GENEVA_ROOT_DIR GENEVAROOT
  PATHS
    /opt/geneva/
    /usr/local/geneva
    /usr/geneva
    ENV GENEVA_ROOT
)

# Check that the path was indeed found
IF ( NOT GENEVA_ROOT_DIR )
	MESSAGE(FATAL_ERROR "Geneva root directory was not found. Cannot continue!")
ENDIF()

###############################################################################
# find the include directories. We search for a single include
# directory only and assume that the others can be found on the
# same level 

find_path (
  GENEVA_INCLUDE_DIR geneva/GObject.hpp
  PATHS 
  	${GENEVA_ROOT_DIR}/include
)

# Check that the path was indeed found
IF ( NOT GENEVA_INCLUDE_DIR )
	MESSAGE(FATAL_ERROR "Geneva include directory was not found. Cannot continue!")
ENDIF()

###############################################################################
# Find the CMakeModules directory

find_path (
	GENEVA_CMAKE_MODULES_DIR IdentifySystemParameters.cmake
	PATHS
	${GENEVA_ROOT_DIR}/CMakeModules
)

# Check that the path was indeed found
IF ( NOT GENEVA_CMAKE_MODULES_DIR )
	MESSAGE(FATAL_ERROR "Geneva cmake module directory was not found. Cannot continue!")
ENDIF()

###############################################################################
# Find the libraries

SET ( GENEVA_LIBRARY_DIR ${GENEVA_ROOT_DIR}/lib )

find_library(
  GENEVA_BASE_LIBRARY 
  NAMES gemfony-geneva
  PATHS ${GENEVA_LIBRARY_DIR}
)
find_library(
  GENEVA_INDIVIDUAL_LIBRARY 
  NAMES gemfony-geneva-individuals
  PATHS ${GENEVA_LIBRARY_DIR}
)
find_library(
  GENEVA_COMMON_LIBRARY 
  NAMES gemfony-common
  PATHS ${GENEVA_LIBRARY_DIR}
)
find_library(
  GENEVA_HAP_LIBRARY 
  NAMES gemfony-hap
  PATHS ${GENEVA_LIBRARY_DIR}
)
find_library(
  GENEVA_COURTIER_LIBRARY 
  NAMES gemfony-courtier
  PATHS ${GENEVA_LIBRARY_DIR}
)

###############################################################################
# Check that all files and directories were properly found

find_package_handle_standard_args (
  GENEVA 
  DEFAULT_MSG
  GENEVA_ROOT_DIR
  GENEVA_INCLUDE_DIR
  GENEVA_LIBRARY_DIR
  GENEVA_CMAKE_MODULES_DIR
  GENEVA_BASE_LIBRARY 
  GENEVA_INDIVIDUAL_LIBRARY 
  GENEVA_COMMON_LIBRARY 
  GENEVA_HAP_LIBRARY
  GENEVA_COURTIER_LIBRARY
)

set (
  GENEVA_LIBRARIES
  ${GENEVA_BASE_LIBRARY}
  ${GENEVA_INDIVIDUAL_LIBRARY}
  ${GENEVA_COMMON_LIBRARY}
  ${GENEVA_HAP_LIBRARY}
  ${GENEVA_COURTIER_LIBRARY}
)

get_filename_component(
  GENEVA_LIBRARY_DIR 
  ${GENEVA_BASE_LIBRARY} 
  PATH
)

###############################################################################
# Specificy the minimum boost version accepted for this Geneva release
SET(GENEVA_MIN_BOOST_VERSION 1.48)

###############################################################################
# Mark as finished

mark_as_advanced(
  GENEVA_ROOT_DIR
  GENEVA_INCLUDE_DIR
  GENEVA_LIBRARY_DIR
  GENEVA_LIBRARIES
  GENEVA_BASE_LIBRARY
  GENEVA_INDIVIDUAL_LIBRARY
  GENEVA_COMMON_LIBRARY
  GENEVA_HAP_LIBRARY
  GENEVA_COURTIER_LIBRARY
  GENEVA_CMAKE_MODULES_DIR
  GENEVA_MIN_BOOST_VERSION
)

###############################################################################
# done
