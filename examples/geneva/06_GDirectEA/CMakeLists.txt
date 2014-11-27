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
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF (NOT GENEVA_FULL_TREE_BUILD)

	PROJECT(GDirectEA)

	# For building the examples independently, we still assume that the
	# 'CMakeModules' folder can be found at the same place as the current
	# source folder... This avoids having to copy the same content over
	# and over again in all the examples.
	SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../CMakeModules")

	MESSAGE("\nPerforming an independent build of ${PROJECT_NAME}")
	INCLUDE(CommonGenevaBuild)

ENDIF ()

SET ( EXECUTABLENAME GDirectEA )

SET ( ${EXECUTABLENAME}_SRCS
	GArgumentParser
	GDirectEA
)

ADD_EXECUTABLE(${EXECUTABLENAME}
	${${EXECUTABLENAME}_SRCS}
)

TARGET_LINK_LIBRARIES (${EXECUTABLENAME}
	${GENEVA_LIBRARIES}
	${Boost_LIBRARIES}
)

ADD_SUBDIRECTORY (config)

INSTALL ( TARGETS ${EXECUTABLENAME} DESTINATION examples/${EXECUTABLENAME} )
