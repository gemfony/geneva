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

# --- GCanvasTest: manual smoke-test (load/add/save a PPM image) ---------------

SET ( EXECUTABLENAME GCanvasTest )

SET ( ${EXECUTABLENAME}_SRCS
	GCanvasTest.cpp
	../GCanvas.cpp
)

ADD_EXECUTABLE(${EXECUTABLENAME}
	${${EXECUTABLENAME}_SRCS}
)

TARGET_INCLUDE_DIRECTORIES(${EXECUTABLENAME} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/..
)

SET(LIBRARIES_TO_LINK
		${GENEVA_LIBRARIES}
		${Boost_LIBRARIES})

IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
	SET(LIBRARIES_TO_LINK ${LIBRARIES_TO_LINK} ${MPI_LIBRARIES})
ENDIF()

TARGET_LINK_LIBRARIES(${EXECUTABLENAME} ${LIBRARIES_TO_LINK})

ADD_SUBDIRECTORY(pictures)

# --- GCanvasTests: Catch2 unit tests ------------------------------------------

SET ( UNITTESTNAME GCanvasTests )

SET ( ${UNITTESTNAME}_SRCS
	GCanvasTests.cpp
	../GCanvas.cpp
)

ADD_EXECUTABLE(${UNITTESTNAME}
	${${UNITTESTNAME}_SRCS}
)

TARGET_INCLUDE_DIRECTORIES(${UNITTESTNAME} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/..
)

TARGET_LINK_LIBRARIES(${UNITTESTNAME}
		${GENEVA_LIBRARIES}
		${Boost_LIBRARIES}
		Catch2::Catch2WithMain)

ADD_TEST(${UNITTESTNAME} ${UNITTESTNAME})

INSTALL ( TARGETS ${EXECUTABLENAME} ${UNITTESTNAME}
	DESTINATION ${INSTALL_PREFIX_DATA}/examples/GCUDAWorker/tests )
