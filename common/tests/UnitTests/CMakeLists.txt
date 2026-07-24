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
# Helper: declare a Common-library Catch2 test executable. Each call
#
#     geneva_common_test(<exe-name> <source-files...>)
#
# creates an executable, links it against the Geneva libraries + Boost +
# Catch2::Catch2WithMain (+ MPI when GENEVA_BUILD_WITH_MPI_CONSUMER), registers
# it with CTest under the "common" label, installs it, and appends it to the
# `tests-common-unit` aggregate target. Used by every test below.

SET(_GENEVA_COMMON_TEST_TARGETS "")

FUNCTION(geneva_common_test name)
	ADD_EXECUTABLE(${name} ${ARGN})

	SET(_libs ${GENEVA_LIBRARIES} ${Boost_LIBRARIES} Catch2::Catch2WithMain)
	IF (GENEVA_BUILD_WITH_MPI_CONSUMER)
		LIST(APPEND _libs ${MPI_LIBRARIES})
	ENDIF ()
	TARGET_LINK_LIBRARIES(${name} ${_libs})

	ADD_TEST(${name} ${name})
	SET_TESTS_PROPERTIES(${name} PROPERTIES LABELS "common")
	INSTALL(TARGETS ${name}
		DESTINATION ${INSTALL_PREFIX_DATA}/tests/geneva/UnitTests)

	# Append to the parent-scope aggregate list.
	SET(_GENEVA_COMMON_TEST_TARGETS "${_GENEVA_COMMON_TEST_TARGETS};${name}"
		PARENT_SCOPE)
ENDFUNCTION()

################################################################################
# Test executables — one per production file, alphabetical.

geneva_common_test(GAgingStoreTTests              GAgingStoreTTests.cpp)
geneva_common_test(GBinaryArchiveTests            GBinaryArchiveTests.cpp)
geneva_common_test(GBlockingMPMCQueueTTests           GBlockingMPMCQueueTTests.cpp)
geneva_common_test(GCommonEnumsTests              GCommonEnumsTests.cpp)
geneva_common_test(GCompletionLatchTTests         GCompletionLatchTTests.cpp)
geneva_common_test(GContentAddressedStoreTTests   GContentAddressedStoreTTests.cpp)
geneva_common_test(GSPSCStagingRingTTests        GSPSCStagingRingTTests.cpp)
geneva_common_test(GThreadBudgetTests            GThreadBudgetTests.cpp)
geneva_common_test(GThreadSafeKeyedStoreTTests   GThreadSafeKeyedStoreTTests.cpp)
geneva_common_test(GThreadSafeSetTTests          GThreadSafeSetTTests.cpp)
geneva_common_test(GCommonHelperFunctionsTests    GCommonHelperFunctionsTests.cpp)
geneva_common_test(GCommonHelperFunctionsTTests   GCommonHelperFunctionsTTests.cpp)
geneva_common_test(GCommonInterfaceTTests         GCommonInterfaceTTests.cpp)
geneva_common_test(GCommonMathHelperFunctionsTTests
                                                  GCommonMathHelperFunctionsTTests.cpp)
geneva_common_test(GContainerTTests               GContainerTTests.cpp)
geneva_common_test(GDefaultValueTTests            GDefaultValueTTests.cpp)
geneva_common_test(GErrorStreamerTests            GErrorStreamerTests.cpp)
geneva_common_test(GExceptionsTests               GExceptionsTests.cpp)
geneva_common_test(GExpectationChecksTTests       GExpectationChecksTTests.cpp)
geneva_common_test(GFactoryTTests                 GFactoryTTests.cpp)
geneva_common_test(GFixedSizePriorityQueueTTests  GFixedSizePriorityQueueTTests.cpp)
geneva_common_test(GGlobalOptionsTTests           GGlobalOptionsTTests.cpp)
geneva_common_test(GJsonIOTests                   GJsonIOTests.cpp)
geneva_common_test(GLoadOnceCellTTests            GLoadOnceCellTTests.cpp)
geneva_common_test(GLoggerTests                   GLoggerTests.cpp)
geneva_common_test(GMPMCQueueConformanceTests     GMPMCQueueConformanceTests.cpp)
geneva_common_test(GMPMCQueueShutdownStressTests  GMPMCQueueShutdownStressTests.cpp)
geneva_common_test(GParserBuilderTests            GParserBuilderTests.cpp)
geneva_common_test(GPolymorphicRegistryTests      GPolymorphicRegistryTests.cpp)
geneva_common_test(GProviderStoreTTests           GProviderStoreTTests.cpp)
geneva_common_test(GSerializableFunctionObjectTTests
                                                  GSerializableFunctionObjectTTests.cpp)
geneva_common_test(GSerializationHelperFunctionsTTests
                                                  GSerializationHelperFunctionsTTests.cpp)
geneva_common_test(GSerializeTupleTTests          GSerializeTupleTTests.cpp)
geneva_common_test(GSingletonTTests               GSingletonTTests.cpp)
geneva_common_test(GThreadGroupTests              GThreadGroupTests.cpp)
geneva_common_test(GThreadPoolTests               GThreadPoolTests.cpp)
geneva_common_test(GTupleIOTests                  GTupleIOTests.cpp)
geneva_common_test(GTypeToStringTTests            GTypeToStringTTests.cpp)
geneva_common_test(GTypeTraitsTTests              GTypeTraitsTTests.cpp)
geneva_common_test(GUnitTestFrameworkTTests       GUnitTestFrameworkTTests.cpp)

################################################################################
# Aggregate `tests-common-unit` target builds every Common test exe at once.

ADD_CUSTOM_TARGET( "tests-common-unit"
	DEPENDS ${_GENEVA_COMMON_TEST_TARGETS}
	COMMENT "Building the unit tests for the \"common\" library."
)
