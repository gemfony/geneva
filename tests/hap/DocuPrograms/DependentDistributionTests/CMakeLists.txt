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

IF (NOT GENEVA_FULL_TREE_BUILD)

	PROJECT(DependentDistributionTests)

	# For building the tests independently, we still assume that the
	# current source folder is still part of the Geneva tree structure,
	# i.e., the 'CMakeModules' folder can be found going up... This avoids
	# having to copy the same content over and over again in the tests.
	SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../../../../CMakeModules")

	MESSAGE("\nPerforming an independent build of ${PROJECT_NAME}")
	INCLUDE(CommonGenevaBuild)

ENDIF ()

SET ( EXECUTABLENAME DependentDistributionTests )

SET ( ${EXECUTABLENAME}_SRCS
	DependentDistributionTests
)

ADD_EXECUTABLE(${EXECUTABLENAME}
	${${EXECUTABLENAME}_SRCS}
)

TARGET_LINK_LIBRARIES (${EXECUTABLENAME}
	${GENEVA_LIBRARIES}
	${Boost_LIBRARIES}
)

#ADD_SUBDIRECTORY (config)

ADD_TEST(${EXECUTABLENAME} ${EXECUTABLENAME})

INSTALL ( TARGETS ${EXECUTABLENAME} DESTINATION ${INSTALL_PREFIX_DATA}/tests/hap/DocuPrograms/${EXECUTABLENAME} )
