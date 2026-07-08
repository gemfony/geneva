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

# Dietrich is a leaf peer on top of common; this validity harness links only those
# two libraries (plus Boost), never hap / courtier / geneva.
TARGET_LINK_LIBRARIES (${EXECUTABLENAME}
		${DIETRICH_LIBNAME} ${COMMON_LIBNAME} ${Boost_LIBRARIES})

#ADD_SUBDIRECTORY (config)

ADD_TEST(${EXECUTABLENAME} ${EXECUTABLENAME})

# All tests below (this generator run plus every *Validity driver) execute the SAME
# generator in the SAME working directory (CMAKE_CURRENT_BINARY_DIR) and read/write the
# same fixed output files (result.C / result.gp / result.py / result.m / result_data.*).
# Two of them running concurrently under `ctest -j` would clobber those files or read one
# mid-write -> intermittent, spurious failures. A shared RESOURCE_LOCK makes CTest run
# them one-at-a-time (while still overlapping them with unrelated tests). Applied to each
# test right where it is added, so only actually-registered tests get the lock.
SET_TESTS_PROPERTIES(${EXECUTABLENAME} PROPERTIES RESOURCE_LOCK GPlotDesigner_shared_workdir)

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
	# Spinning up CERN ROOT (process start + macro compilation) makes this the single slowest test in the
	# whole suite (~2 min, dwarfing everything else). Label it "slow" so routine runs can drop it via
	# `ctest -LE slow` (fast turnaround for non-plotting work); the full release/CI gate still runs it.
	SET_TESTS_PROPERTIES(GPlotDesignerRootValidity PROPERTIES
		RESOURCE_LOCK GPlotDesigner_shared_workdir
		LABELS "slow")
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
	SET_TESTS_PROPERTIES(GPlotDesignerGnuplotValidity PROPERTIES RESOURCE_LOCK GPlotDesigner_shared_workdir)
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
	SET_TESTS_PROPERTIES(GPlotDesignerMatplotlibValidity PROPERTIES RESOURCE_LOCK GPlotDesigner_shared_workdir)
	MESSAGE(STATUS "GPlotDesigner: matplotlib validity test ENABLED (python3=${GENEVA_PYTHON3_EXECUTABLE})")
ELSE()
	MESSAGE(STATUS "GPlotDesigner: matplotlib validity test skipped (python3 and/or matplotlib not found)")
ENDIF()

# Optional Octave-validity test: feed the generated Octave/MATLAB script (OCTAVE backend) to the real
# `octave` to confirm it is valid, RENDERING input. The script uses only base Octave-and-MATLAB functions;
# the driver runs it headless (no window system) and keys off a produced (non-trivial) PNG, a zero exit
# code AND the absence of an Octave error. Enabled only when `octave` is present; skipped gracefully
# otherwise.
FIND_PROGRAM(GENEVA_OCTAVE_EXECUTABLE NAMES octave octave-cli)
IF(GENEVA_OCTAVE_EXECUTABLE)
	ADD_TEST(
		NAME GPlotDesignerOctaveValidity
		COMMAND ${CMAKE_COMMAND}
			-DGEN_EXE=$<TARGET_FILE:${EXECUTABLENAME}>
			-DOCTAVE_EXE=${GENEVA_OCTAVE_EXECUTABLE}
			-DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}
			-P ${CMAKE_CURRENT_SOURCE_DIR}/RunOctaveValidity.cmake
	)
	SET_TESTS_PROPERTIES(GPlotDesignerOctaveValidity PROPERTIES RESOURCE_LOCK GPlotDesigner_shared_workdir)
	MESSAGE(STATUS "GPlotDesigner: Octave validity test ENABLED (octave=${GENEVA_OCTAVE_EXECUTABLE})")
ELSE()
	MESSAGE(STATUS "GPlotDesigner: Octave validity test skipped (octave not found)")
ENDIF()

# Optional data-validity test: confirm the raw series data GPlotDesigner exports (via the DATA backend /
# GDataEmitter) is correct in BOTH formats -- the binary numpy .npz (built by hand) and the CSV. The check
# loads the .npz with numpy.load() and asserts a known series' shape and a couple of values round-trip
# BIT-EXACTLY, then parses a couple of CSV values back. Enabled only when python3 is present AND
# `python3 -c "import numpy"` succeeds; skipped gracefully otherwise. (GENEVA_PYTHON3_EXECUTABLE is found
# above by the matplotlib block.)
IF(GENEVA_PYTHON3_EXECUTABLE)
	EXECUTE_PROCESS(
		COMMAND ${GENEVA_PYTHON3_EXECUTABLE} -c "import numpy"
		RESULT_VARIABLE GENEVA_NUMPY_RC
		OUTPUT_QUIET ERROR_QUIET
	)
ELSE()
	SET(GENEVA_NUMPY_RC 1)
ENDIF()
IF(GENEVA_PYTHON3_EXECUTABLE AND GENEVA_NUMPY_RC EQUAL 0)
	ADD_TEST(
		NAME GPlotDesignerDataValidity
		COMMAND ${CMAKE_COMMAND}
			-DGEN_EXE=$<TARGET_FILE:${EXECUTABLENAME}>
			-DPYTHON_EXE=${GENEVA_PYTHON3_EXECUTABLE}
			-DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}
			-P ${CMAKE_CURRENT_SOURCE_DIR}/RunDataValidity.cmake
	)
	SET_TESTS_PROPERTIES(GPlotDesignerDataValidity PROPERTIES RESOURCE_LOCK GPlotDesigner_shared_workdir)
	MESSAGE(STATUS "GPlotDesigner: data validity test ENABLED (python3=${GENEVA_PYTHON3_EXECUTABLE})")
ELSE()
	MESSAGE(STATUS "GPlotDesigner: data validity test skipped (python3 and/or numpy not found)")
ENDIF()

# Optional render-validity test: feed a DATA export (the raw series data + self-describing manifest the
# DATA backend writes) to the EXTERNAL renderer scripts/geneva_plot_render.py and confirm it reproduces a
# figure -- i.e. that rendering can live entirely outside the C++ library. Enabled only when python3 +
# numpy + matplotlib are all present; skipped gracefully otherwise. Keys off a produced (non-trivial) PNG,
# a zero exit code AND the absence of a Python traceback.
IF(GENEVA_PYTHON3_EXECUTABLE AND GENEVA_NUMPY_RC EQUAL 0 AND GENEVA_MATPLOTLIB_RC EQUAL 0)
	ADD_TEST(
		NAME GPlotDesignerRenderValidity
		COMMAND ${CMAKE_COMMAND}
			-DGEN_EXE=$<TARGET_FILE:${EXECUTABLENAME}>
			-DRENDER_PY=${CMAKE_SOURCE_DIR}/dietrich/scripts/geneva_plot_render.py
			-DPYTHON_EXE=${GENEVA_PYTHON3_EXECUTABLE}
			-DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}
			-P ${CMAKE_CURRENT_SOURCE_DIR}/RunRenderValidity.cmake
	)
	SET_TESTS_PROPERTIES(GPlotDesignerRenderValidity PROPERTIES RESOURCE_LOCK GPlotDesigner_shared_workdir)
	MESSAGE(STATUS "GPlotDesigner: render validity test ENABLED (python3=${GENEVA_PYTHON3_EXECUTABLE})")
ELSE()
	MESSAGE(STATUS "GPlotDesigner: render validity test skipped (python3, numpy and/or matplotlib not found)")
ENDIF()

INSTALL ( TARGETS ${EXECUTABLENAME} DESTINATION ${INSTALL_PREFIX_DATA}/tests/dietrich/ManualTests/${EXECUTABLENAME} )
