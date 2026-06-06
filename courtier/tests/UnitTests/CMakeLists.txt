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

SET(LIBRARIES_TO_LINK
		${GENEVA_LIBRARIES}
		${Boost_LIBRARIES}
		Catch2::Catch2WithMain)

IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
	SET(LIBRARIES_TO_LINK
			${LIBRARIES_TO_LINK}
			${MPI_LIBRARIES})
ENDIF()

# Helper: declare a Courtier Catch2 test executable, register it with CTest under the
# "courtier" label, install it, and add it to the tests-courtier-unit aggregate.
FUNCTION(geneva_courtier_test name)
	ADD_EXECUTABLE(${name} ${ARGN})
	TARGET_LINK_LIBRARIES(${name} ${LIBRARIES_TO_LINK})
	ADD_TEST(${name} ${name})
	SET_TESTS_PROPERTIES(${name} PROPERTIES LABELS "courtier")
	INSTALL(TARGETS ${name} DESTINATION ${INSTALL_PREFIX_DATA}/tests/courtier/UnitTests)
ENDFUNCTION()

# Submission-path unit tests (formerly courtier2/tests/UnitTests, merged in at Stage E). The test
# executable / file names still carry the "2" until the Stage-E identifier rename (E2); the CTest
# labels and install paths are already "courtier".
geneva_courtier_test(GCourtier2PolicyTests      GCourtier2PolicyTests.cpp)
geneva_courtier_test(GCourtier2TimeoutTests     GCourtier2TimeoutTests.cpp)
geneva_courtier_test(GCourtier2ConcurrencyTests GCourtier2ConcurrencyTests.cpp)
geneva_courtier_test(GCourtier2AsioTests        GCourtier2AsioTests.cpp)
geneva_courtier_test(GCourtier2WebsocketTests   GCourtier2WebsocketTests.cpp)

# The fatal reconciliation paths exit the process cleanly with a non-zero status (via LOGEXIT, no
# core dump), so they are exercised as subprocesses: the "ok" control must succeed (exit 0); each
# fatal scenario must exit non-zero, which CTest's WILL_FAIL asserts directly.
ADD_EXECUTABLE(GCourtier2FatalExerciser GCourtier2FatalExerciser.cpp)
TARGET_LINK_LIBRARIES(GCourtier2FatalExerciser ${LIBRARIES_TO_LINK})
INSTALL(TARGETS GCourtier2FatalExerciser
		DESTINATION ${INSTALL_PREFIX_DATA}/tests/courtier/UnitTests)

ADD_TEST(NAME GCourtier2Fatal_ok COMMAND GCourtier2FatalExerciser --scenario=ok)
SET_TESTS_PROPERTIES(GCourtier2Fatal_ok PROPERTIES LABELS "courtier")

FOREACH(scenario fatal-unfixable fatal-none-fatal fatal-none-clone)
	ADD_TEST(NAME GCourtier2Fatal_${scenario}
			COMMAND GCourtier2FatalExerciser --scenario=${scenario})
	SET_TESTS_PROPERTIES(GCourtier2Fatal_${scenario} PROPERTIES LABELS "courtier" WILL_FAIL TRUE)
ENDFOREACH()

# The MPI consumer fixes its rank layout at launch, so it is exercised as a multi-process program
# under mpirun (1 master + 2 workers) rather than as an in-process Catch2 test.
IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
	ADD_EXECUTABLE(GCourtier2MpiExerciser GCourtier2MpiExerciser.cpp)
	TARGET_LINK_LIBRARIES(GCourtier2MpiExerciser ${LIBRARIES_TO_LINK})
	INSTALL(TARGETS GCourtier2MpiExerciser
			DESTINATION ${INSTALL_PREFIX_DATA}/tests/courtier/UnitTests)
	ADD_TEST(NAME GCourtier2MpiTest
			COMMAND ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} 3
					${MPIEXEC_PREFLAGS} $<TARGET_FILE:GCourtier2MpiExerciser> ${MPIEXEC_POSTFLAGS})
	SET_TESTS_PROPERTIES(GCourtier2MpiTest PROPERTIES LABELS "courtier")
ENDIF()

GENEVA_AGGREGATE_TARGET( "tests-courtier-unit" )
