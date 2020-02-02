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

CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

MESSAGE ("")

PROJECT (Geneva_Library_Collection)

################################################################################
# We create some examples and test programs as part of the build process.
# This variable allows their build-scripts to detect that they are being
# called from within the master build-script. This prevents redundant searches
# and checks, while still allowing examples to be built independently of the
# main Geneva source tree.

SET(GENEVA_FULL_TREE_BUILD TRUE)

################################################################################
# Set this variable to make all examples run as part of the testing with CTest.

#SET(GENEVA_RUN_EXAMPLES_AS_TESTS TRUE)

################################################################################
# Different subcomponents of the Geneva binary tree can be installed in
# customized locations, by setting the "INSTALL_PREFIX_*" variables:
#   INSTALL_PREFIX_INCLUDES -> installation folder for the header files
#   INSTALL_PREFIX_LIBS     -> installation folder for the libraries
#   INSTALL_PREFIX_DOCS     -> installation folder for the documentation files
#   INSTALL_PREFIX_DATA     -> installation folder for other files like
#                              examples, tests and other scripts
# If none of these variables is set, the whole Geneva binary tree is installed
# (as expected) in "CMAKE_INSTALL_PREFIX". The default value for this last
# valiable is set here, and further handled in the 'CommonGenevaBuild' module.
# Generally, this can be provided through a custom genevaConfig.gcfg, in
# conjunction with the prepareBuild.sh script.

IF( NOT CMAKE_INSTALL_PREFIX )
	SET( INSTALL_PREFIX_ROOT "/opt/geneva-${GENEVA_VERSION}" )
ENDIF()

################################################################################
# Make sure we first check our local modules directory

SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")

###############################################################################
# Include the shared functionality module for any Geneva build

INCLUDE(CommonGenevaBuild)

###############################################################################
# Set the library/package version. Uneven minor numbers denote development
# versions while even minor numbers stand for production releases.

SET(VERSION_MAJOR "1")
SET(VERSION_MINOR "9")
SET(VERSION_PATCH "2")

SET(GENEVA_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

################################################################################
# Include directories, libraries

INCLUDE_DIRECTORIES (
	${PROJECT_SOURCE_DIR}/include
)

# Custom targets for building single libraries only
ADD_CUSTOM_TARGET( "common"             DEPENDS ${COMMON_LIBNAME}            COMMENT "Building only the Common library." )
ADD_CUSTOM_TARGET( "hap"                DEPENDS ${HAP_LIBNAME}               COMMENT "Building only the Hap library." )
ADD_CUSTOM_TARGET( "courtier"           DEPENDS ${COURTIER_LIBNAME}          COMMENT "Building only the Courtier library." )
ADD_CUSTOM_TARGET( "geneva"             DEPENDS ${GENEVA_LIBNAME}            COMMENT "Building only the Geneva library." )
ADD_CUSTOM_TARGET( "geneva-individuals" DEPENDS ${GENEVA_INDIVIDUAL_LIBNAME} COMMENT "Building only the Geneva-Individuals library." )

# Custom target for building all the libraries but no examples nor tests
ADD_CUSTOM_TARGET( "core"               DEPENDS ${GENEVA_LIBNAMES}           COMMENT "Building all the Geneva libraries.")

################################################################################
# Add sub-directories

# Accessory files just get copied
ADD_SUBDIRECTORY (CMakeModules)
ADD_SUBDIRECTORY (licenses)
ADD_SUBDIRECTORY (scripts)

# The main code lays here, and the compilation takes place in 'src'
ADD_SUBDIRECTORY (include)
ADD_SUBDIRECTORY (src)

# Examples and tests are compiled at the end
IF( GENEVA_BUILD_TESTS )
	INCLUDE(CTest)
	ADD_SUBDIRECTORY (tests)
ENDIF()

# Examples are also run as tests if enabled with 'GENEVA_RUN_EXAMPLES_AS_TESTS'
ADD_SUBDIRECTORY (examples)

# Holds an incubator facility for new optimization algorihms
# ADD_SUBDIRECTORY (incubator)

################################################################################
# Custom target for all compilable items in the distribution
ADD_CUSTOM_TARGET(
    "gemfony-build-all"  DEPENDS
    core
    examples
    tests
    # incubator
    COMMENT "Build the entire Gemfony library collection"
)

################################################################################
# Custom target for all compilable items in the distribution
ADD_CUSTOM_TARGET(
    "install_gemfony-build-all"
    "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target install
    DEPENDS gemfony-build-all
    COMMENT "Installing gemfony-build-all"
)

################################################################################
# Additional information files to be copied

SET ( INFORMATIONFILES
	AUTHORS
	CHANGES
	COPYING
	CREDITS
	FAQ
	INSTALL
	INSTALL.Windows
	KNOWNBUGS
	RATEYOUREXPERIENCE
	README
	README.documentation
)

INSTALL ( FILES ${INFORMATIONFILES} DESTINATION ${INSTALL_PREFIX_DOCS} )

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
	SET(MSG "Reference documentation for Geneva may now be created with 'make doc'")
ELSE()
	SET(MSG "Doxygen not found. No support for the creation of reference documentation")
ENDIF()
MESSAGE("${MSG}\n")

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
 than a few seconds."
)

SET(CPACK_PACKAGE_VENDOR "Gemfony scientific")
SET(CPACK_PACKAGE_CONTACT "Gemfony scientific <contact (AT) gemfony (DOT) eu>")
SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
SET(CPACK_PACKAGING_INSTALL_PREFIX "/opt/geneva")

# DEB specific variables. Indentation is important in the description!
SET(
	CPACK_DEBIAN_PACKAGE_DESCRIPTION
	"${CPACK_PACKAGE_DESCRIPTION_SUMMARY}\n ${CPACK_AUX_PACKAGE_DESCRIPTION}"
)
SET(CPACK_DEBIAN_PACKAGE_SECTION "Science")
SET(CPACK_DEBIAN_PACKAGE_PRIORITY "Extra")
SET(
	CPACK_DEBIAN_PACKAGE_DEPENDS
	"libboost-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0),
	libboost-filesystem-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0),
	libboost-program-options-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0),
	libboost-regex-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0),
	libboost-serialization-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0),
	libboost-system-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0),
	libboost-test-dev (>= ${GENEVA_MIN_BOOST_VERSION}.0)"
)

# RPM specific variables
SET(CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_AUX_PACKAGE_DESCRIPTION})
SET(CPACK_RPM_PACKAGE_LICENSE "Apache License 2.0")
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
# Done
