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

# Optional gnuplot-validity test: feed the generated gnuplot script (GNUPLOT backend) to the real
# gnuplot interpreter to confirm it is valid gnuplot input. Enabled only when `gnuplot` is present and
# skipped gracefully otherwise. gnuplot renders reliably headless (no xvfb needed), so the check keys
# off BOTH a produced PNG and a zero exit code.
FIND_PROGRAM(GENEVA_GNUPLOT_EXECUTABLE NAMES gnuplot)
IF(GENEVA_GNUPLOT_EXECUTABLE)
	ADD_TEST(
		NAME GPlotDesignerGnuplotValidity
		COMMAND ${CMAKE_COMMAND}
			-DGEN_EXE=$<TARGET_FILE:${EXECUTABLENAME}>
			-DGNUPLOT_EXE=${GENEVA_GNUPLOT_EXECUTABLE}
			-DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}
			-P ${CMAKE_CURRENT_SOURCE_DIR}/RunGnuplotValidity.cmake
	)
	MESSAGE(STATUS "GPlotDesigner: gnuplot validity test ENABLED (gnuplot=${GENEVA_GNUPLOT_EXECUTABLE})")
ELSE()
	MESSAGE(STATUS "GPlotDesigner: gnuplot validity test skipped (gnuplot not found)")
ENDIF()

# Optional matplotlib-validity test: feed the generated Python/matplotlib script (MATPLOTLIB backend) to
# the real python3 + matplotlib to confirm it is valid, RENDERING matplotlib input. Enabled only when
# python3 is present AND `python3 -c "import matplotlib"` succeeds; skipped gracefully otherwise.
# matplotlib renders reliably headless via its Agg backend (no display/xvfb needed), so the check keys
# off a produced (non-trivial) PNG, a zero exit code AND the absence of a Python traceback / error.
FIND_PROGRAM(GENEVA_PYTHON3_EXECUTABLE NAMES python3)
IF(GENEVA_PYTHON3_EXECUTABLE)
	EXECUTE_PROCESS(
		COMMAND ${GENEVA_PYTHON3_EXECUTABLE} -c "import matplotlib"
		RESULT_VARIABLE GENEVA_MATPLOTLIB_RC
		OUTPUT_QUIET ERROR_QUIET
	)
ELSE()
	SET(GENEVA_MATPLOTLIB_RC 1)
ENDIF()
IF(GENEVA_PYTHON3_EXECUTABLE AND GENEVA_MATPLOTLIB_RC EQUAL 0)
	ADD_TEST(
		NAME GPlotDesignerMatplotlibValidity
		COMMAND ${CMAKE_COMMAND}
			-DGEN_EXE=$<TARGET_FILE:${EXECUTABLENAME}>
			-DPYTHON_EXE=${GENEVA_PYTHON3_EXECUTABLE}
			-DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}
			-P ${CMAKE_CURRENT_SOURCE_DIR}/RunMatplotlibValidity.cmake
	)
	MESSAGE(STATUS "GPlotDesigner: matplotlib validity test ENABLED (python3=${GENEVA_PYTHON3_EXECUTABLE})")
ELSE()
	MESSAGE(STATUS "GPlotDesigner: matplotlib validity test skipped (python3 and/or matplotlib not found)")
ENDIF()

INSTALL ( TARGETS ${EXECUTABLENAME} DESTINATION ${INSTALL_PREFIX_DATA}/tests/common/ManualTests/${EXECUTABLENAME} )
