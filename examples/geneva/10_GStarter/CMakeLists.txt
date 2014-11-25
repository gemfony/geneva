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

PROJECT (GStarter)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET ( EXECUTABLENAME GStarter )

SET ( ${EXECUTABLENAME}_SRCS
	GStarter
	GStarterIndividual
)

SET ( CONFIGFILES
	config/GEvolutionaryAlgorithm.json
	config/GStarterIndividual.json
	config/Go2.json
)

ADD_EXECUTABLE(${EXECUTABLENAME}
	${${EXECUTABLENAME}_SRCS}
)

# Copy the config files to the build tree for easier testing
FOREACH ( file IN LISTS CONFIGFILE )
	CONFIGURE_FILE ( ${file} ${file} COPYONLY )
ENDFOREACH ()

TARGET_LINK_LIBRARIES (${EXECUTABLENAME}
	${GENEVA_INDIVIDUAL_LIBNAME}
	${GENEVA_LIBNAME}
	${COURTIER_LIBNAME}
	${HAP_LIBNAME}
	${COMMON_LIBNAME}
	${Boost_LIBRARIES}
)

ADD_SUBDIRECTORY (config)

IF( GENEVA_BUILD_TESTS )
	ADD_SUBDIRECTORY (Tests)
ENDIF()

INSTALL ( TARGETS ${EXECUTABLENAME} DESTINATION examples/${PROJECT_NAME} )

INSTALL ( FILES ${CONFIGFILE} DESTINATION examples/${PROJECT_NAME}/config )

################################################################################
# Done
