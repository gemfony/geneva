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
	SET(LIBRARIES_TO_LINK ${LIBRARIES_TO_LINK} ${MPI_LIBRARIES})
ENDIF()

# Helper: declare a courtier2 Catch2 test executable, register it with CTest under
# the "courtier2" label, install it.
FUNCTION(geneva_courtier2_test name)
	ADD_EXECUTABLE(${name} ${ARGN})
	TARGET_LINK_LIBRARIES(${name} ${LIBRARIES_TO_LINK})
	ADD_TEST(${name} ${name})
	SET_TESTS_PROPERTIES(${name} PROPERTIES LABELS "courtier2")
	INSTALL(TARGETS ${name} DESTINATION ${INSTALL_PREFIX_DATA}/tests/courtier2/UnitTests)
ENDFUNCTION()

geneva_courtier2_test(GCourtier2PolicyTests    GCourtier2PolicyTests.cpp)
geneva_courtier2_test(GCourtier2AsioTests      GCourtier2AsioTests.cpp)
geneva_courtier2_test(GCourtier2WebsocketTests GCourtier2WebsocketTests.cpp)

# The fatal reconciliation paths terminate the process (GTERMINATION -> std::terminate -> SIGABRT),
# so they are exercised as subprocesses. The "ok" control must succeed (exit 0); each fatal scenario
# must abort. CTest's WILL_FAIL does NOT cover signal deaths (only non-zero return codes), so the
# fatal scenarios are wrapped in a shell that asserts a non-zero/aborting exit.
ADD_EXECUTABLE(GCourtier2FatalExerciser GCourtier2FatalExerciser.cpp)
TARGET_LINK_LIBRARIES(GCourtier2FatalExerciser ${LIBRARIES_TO_LINK})
INSTALL(TARGETS GCourtier2FatalExerciser
		DESTINATION ${INSTALL_PREFIX_DATA}/tests/courtier2/UnitTests)

ADD_TEST(NAME GCourtier2Fatal_ok
		COMMAND GCourtier2FatalExerciser --scenario=ok)
SET_TESTS_PROPERTIES(GCourtier2Fatal_ok PROPERTIES LABELS "courtier2")

FOREACH(scenario fatal-unfixable fatal-none-fatal fatal-none-clone)
	ADD_TEST(NAME GCourtier2Fatal_${scenario}
			COMMAND sh -c "$<TARGET_FILE:GCourtier2FatalExerciser> --scenario=${scenario}; test $? -ne 0")
	SET_TESTS_PROPERTIES(GCourtier2Fatal_${scenario} PROPERTIES LABELS "courtier2")
ENDFOREACH()

# The MPI consumer fixes its rank layout at launch, so it is exercised as a multi-process program
# under mpirun (1 master + 2 workers) rather than as an in-process Catch2 test.
IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
	ADD_EXECUTABLE(GCourtier2MpiExerciser GCourtier2MpiExerciser.cpp)
	TARGET_LINK_LIBRARIES(GCourtier2MpiExerciser ${LIBRARIES_TO_LINK})
	INSTALL(TARGETS GCourtier2MpiExerciser
			DESTINATION ${INSTALL_PREFIX_DATA}/tests/courtier2/UnitTests)
	ADD_TEST(NAME GCourtier2MpiTest
			COMMAND ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} 3
					${MPIEXEC_PREFLAGS} $<TARGET_FILE:GCourtier2MpiExerciser> ${MPIEXEC_POSTFLAGS})
	SET_TESTS_PROPERTIES(GCourtier2MpiTest PROPERTIES LABELS "courtier2")
ENDIF()

GENEVA_AGGREGATE_TARGET( "tests-courtier2-unit" )
