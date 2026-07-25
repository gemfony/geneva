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

################################################################################
# Helper: declare a Weft-library Catch2 test executable. Each call
#
#     geneva_weft_test(<exe-name> <source-files...>)
#
# creates an executable, links it against ONLY gemfony-weft + Boost.JSON +
# Catch2::Catch2WithMain (deliberately NOT the rest of the Geneva libraries), so
# these tests double as a standing proof that Weft is self-contained: if a test
# accidentally reached into common it would fail to link. Each is registered with
# CTest under the "weft" label, installed, and appended to the `tests-weft-unit`
# aggregate target.

SET(_GENEVA_WEFT_TEST_TARGETS "")

FUNCTION(geneva_weft_test name)
	ADD_EXECUTABLE(${name} ${ARGN})

	TARGET_LINK_LIBRARIES(${name}
		${WEFT_LIBNAME}
		Boost::json
		Catch2::Catch2WithMain
	)

	ADD_TEST(${name} ${name})
	SET_TESTS_PROPERTIES(${name} PROPERTIES LABELS "weft")
	INSTALL(TARGETS ${name}
		DESTINATION ${INSTALL_PREFIX_DATA}/tests/weft/UnitTests)

	# Append to the parent-scope aggregate list.
	SET(_GENEVA_WEFT_TEST_TARGETS "${_GENEVA_WEFT_TEST_TARGETS};${name}"
		PARENT_SCOPE)
ENDFUNCTION()

################################################################################
# Test executables — one per production feature, alphabetical.

geneva_weft_test(GBinaryArchiveTests          GBinaryArchiveTests.cpp)
geneva_weft_test(GArchiveCoverageTests        GArchiveCoverageTests.cpp)
geneva_weft_test(GJsonArchiveTests            GJsonArchiveTests.cpp)
geneva_weft_test(GPolymorphicRegistryTests    GPolymorphicRegistryTests.cpp)

################################################################################
# Aggregate `tests-weft-unit` target builds every Weft test exe at once.

ADD_CUSTOM_TARGET( "tests-weft-unit"
	DEPENDS ${_GENEVA_WEFT_TEST_TARGETS}
	COMMENT "Building the unit tests for the \"weft\" library."
)
