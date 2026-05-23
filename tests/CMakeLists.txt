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

IF (NOT GENEVA_FULL_TREE_BUILD)

	PROJECT(Geneva_Tests_All)

	# For building the examples independently, we still assume that the
	# 'CMakeModules' folder can be found at the same place as the current
	# source folder... This avoids having to copy the same content over
	# and over again in all the examples.
	SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../CMakeModules")

	MESSAGE("\nPerforming an independent build of ${PROJECT_NAME}")
	INCLUDE(CommonGenevaBuild)

ENDIF ()

ADD_SUBDIRECTORY ( hap )
ADD_SUBDIRECTORY ( common )
ADD_SUBDIRECTORY ( courtier )
ADD_SUBDIRECTORY ( geneva )

################################################################################
# Aggregate "tests" target.
#
# Rather than an explicit, hand-maintained dependency list (which has silently
# drifted in the past -- a commented-out "tests-common-unit", a
# "tests-hap-docuprogs" that was never wired in, manual-test lists missing newly
# added entries), we recursively collect EVERY buildsystem target defined
# anywhere under tests/. This guarantees that, with GENEVA_BUILD_TESTS enabled,
# building "tests" (and therefore "gemfony-build-all", which depends on it)
# builds all existing -- and any future -- test executables.

FUNCTION(_geneva_collect_targets_recursive _out_var _dir)
	GET_PROPERTY(_subdirs DIRECTORY "${_dir}" PROPERTY SUBDIRECTORIES)
	GET_PROPERTY(_targets DIRECTORY "${_dir}" PROPERTY BUILDSYSTEM_TARGETS)
	SET(_acc ${_targets})
	FOREACH(_sub ${_subdirs})
		_geneva_collect_targets_recursive(_child "${_sub}")
		LIST(APPEND _acc ${_child})
	ENDFOREACH()
	SET(${_out_var} "${_acc}" PARENT_SCOPE)
ENDFUNCTION()

_geneva_collect_targets_recursive(_GENEVA_ALL_TEST_TARGETS "${CMAKE_CURRENT_SOURCE_DIR}")

ADD_CUSTOM_TARGET( "tests"
	DEPENDS ${_GENEVA_ALL_TEST_TARGETS}
	COMMENT "Building all the Geneva tests (auto-collected from tests/)."
)
