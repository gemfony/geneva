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
# This code defines the following variables:
#
# GENEVA_FOUND    - TRUE if all components are found
#
# Example Usages:
#  FIND_PACKAGE(Geneva)
#
###############################################################################

# Handles standard arguments
include(FindPackageHandleStandardArgs)

# Search for the file GENEVAROOT in the search path
find_path(
  GENEVA_ROOT_DIR GENEVAROOT
  HINTS
    /opt/geneva/
    /usr/local/geneva
    /usr/geneva
    ENV GENEVA_ROOT
)

# find the include directories. We search for a single include
# directory only and assume that the others can be found on the
# same level 
find_path(
  GENEVA_INCLUDE_DIR geneva/GIndividual.hpp
  PATHS ${GENEVA_ROOT_DIR}/include
  )

# Find the libraries
find_library(
  GENEVA_BASE_LIBRARY 
  NAMES gemfony-geneva
  PATHS ${GENEVA_ROOT_DIR}/lib
  )
find_library(
  GENEVA_INDIVIDUAL_LIBRARY 
  NAMES gemfony-geneva-individuals
  PATHS ${GENEVA_ROOT_DIR}/lib
  )
find_library(
  GENEVA_COMMON_LIBRARY 
  NAMES gemfony-common
  PATHS ${GENEVA_ROOT_DIR}/lib
  )
find_library(
  GENEVA_HAP_LIBRARY 
  NAMES gemfony-hap
  PATHS ${GENEVA_ROOT_DIR}/lib
  )
find_library(
  GENEVA_COURTIER_LIBRARY 
  NAMES gemfony-courtier
  PATHS ${GENEVA_ROOT_DIR}/lib
  )

# Check that all files and directories were properly found
find_package_handle_standard_args (
  GENEVA 
  DEFAULT_MSG
  GENEVA_ROOT_DIR
  GENEVA_INCLUDE_DIR
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

mark_as_advanced(
  GENEVA_INCLUDE_DIR
  GENEVA_LIBRARY_DIR
  GENEVA_LIBRARIES
)
