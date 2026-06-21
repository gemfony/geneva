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

SET ( EXECUTABLENAME GPlotDesignerTest )

SET ( ${EXECUTABLENAME}_SRCS
	GPlotDesignerTest.cpp
)

ADD_EXECUTABLE(${EXECUTABLENAME}
	${${EXECUTABLENAME}_SRCS}
)

SET(LIBRARIES_TO_LINK
		${GENEVA_LIBRARIES}
		${Boost_LIBRARIES})

# Add MPI Libraries in case building with MPI-consumer
IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
	SET(LIBRARIES_TO_LINK
			${LIBRARIES_TO_LINK}
			${MPI_LIBRARIES})
ENDIF()

TARGET_LINK_LIBRARIES (${EXECUTABLENAME}
		${LIBRARIES_TO_LINK})

#ADD_SUBDIRECTORY (config)

ADD_TEST(${EXECUTABLENAME} ${EXECUTABLENAME})

# Optional ROOT-validity test: feed the generated macro to the real ROOT interpreter to confirm it is
# valid ROOT input (GPlotDesigner is a ROOT-macro code generator). Enabled only when both `root` and
# `xvfb-run` are present -- ROOT needs a display even in batch mode on this host -- and skipped
# gracefully otherwise so ordinary builds are unaffected. The check keys off the rendered artifact, not
# ROOT's exit code (its headless teardown returns non-zero after a successful render).
FIND_PROGRAM(GENEVA_ROOT_EXECUTABLE NAMES root HINTS ENV ROOTSYS PATHS /opt/root/bin PATH_SUFFIXES bin)
FIND_PROGRAM(GENEVA_XVFB_RUN NAMES xvfb-run)
IF(GENEVA_ROOT_EXECUTABLE AND GENEVA_XVFB_RUN)
	ADD_TEST(
		NAME GPlotDesignerRootValidity
		COMMAND ${CMAKE_COMMAND}
			-DGEN_EXE=$<TARGET_FILE:${EXECUTABLENAME}>
			-DROOT_EXE=${GENEVA_ROOT_EXECUTABLE}
			-DXVFB_RUN=${GENEVA_XVFB_RUN}
			-DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}
			-P ${CMAKE_CURRENT_SOURCE_DIR}/RunRootValidity.cmake
	)
	MESSAGE(STATUS "GPlotDesigner: ROOT validity test ENABLED (root=${GENEVA_ROOT_EXECUTABLE})")
ELSE()
	MESSAGE(STATUS "GPlotDesigner: ROOT validity test skipped (root and/or xvfb-run not found)")
ENDIF()

INSTALL ( TARGETS ${EXECUTABLENAME} DESTINATION ${INSTALL_PREFIX_DATA}/tests/common/ManualTests/${EXECUTABLENAME} )
