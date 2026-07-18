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
# See the NOTICE file in the top-level directory of the Geneva library
# collection for a list of contributors and copyright information.
#
################################################################################

SET ( EXECUTABLENAME GParallelisationOverhead )

SET ( ${EXECUTABLENAME}_SRCS
	GParallelisationOverhead.cpp
)

ADD_EXECUTABLE(${EXECUTABLENAME}
	${${EXECUTABLENAME}_SRCS}
)

SET(LIBRARIES_TO_LINK
		${GENEVA_LIBRARIES}
		${Boost_LIBRARIES}
		# GDelayIndividual is no longer in the geneva library; this benchmark compiles it in via its object library.
		GDelayIndividual-obj)

# Add MPI Libraries in case building with MPI-consumer
IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
	SET(LIBRARIES_TO_LINK
			${LIBRARIES_TO_LINK}
			${MPI_LIBRARIES})
ENDIF()

TARGET_LINK_LIBRARIES (${EXECUTABLENAME}
		${LIBRARIES_TO_LINK})

GENEVA_MATERIALIZE_CONFIGS ( ${EXECUTABLENAME} ${INSTALL_PREFIX_DATA}/tests/geneva/PerformanceTests/${EXECUTABLENAME}/config )

IF( GENEVA_RUN_BENCHMARKS_AS_TESTS )
	ADD_TEST(${EXECUTABLENAME} ${EXECUTABLENAME})
	SET_TESTS_PROPERTIES(${EXECUTABLENAME} PROPERTIES TIMEOUT 2500)
ENDIF()

INSTALL ( TARGETS ${EXECUTABLENAME} DESTINATION ${INSTALL_PREFIX_DATA}/benchmarks/geneva/PerformanceTests/${EXECUTABLENAME} )
