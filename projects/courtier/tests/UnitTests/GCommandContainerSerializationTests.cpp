/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>

#include "common/GCommonEnums.hpp"
#include "courtier/GCommandContainerT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GDemoProcessingContainers.hpp"

using namespace Gem::Courtier;

namespace {

using test_container_type = GCommandContainerT<GSimpleContainer, networked_consumer_payload_command>;

/** @brief Builds a command container carrying a payload, so the archive has real content */
test_container_type makeContainer() {
    return {networked_consumer_payload_command::COMPUTE, std::make_unique<GSimpleContainer>(42)};
}

} // anonymous namespace

/******************************************************************************/
/**
 * container_to_string() must return a non-empty document in every (GArchive) mode.
 */
TEST_CASE("container_to_string emits a non-empty document in every mode", "[courtier][serialization]") {
    const auto container = makeContainer();

    REQUIRE_FALSE(container_to_string(container, Gem::Common::serializationMode::GEM_BINARY).empty());
    REQUIRE_FALSE(container_to_string(container, Gem::Common::serializationMode::GEM_JSON).empty());
}

/******************************************************************************/
/**
 * The companion round-trip: every serialization mode must survive a write/read cycle with the
 * command, the payload and the payload's stable lineage id intact.
 */
TEST_CASE("GCommandContainerT survives a round-trip in every serialization mode", "[courtier][serialization]") {
    const auto modes = {
        Gem::Common::serializationMode::GEM_BINARY, // GArchive flat-binary wire codec
        Gem::Common::serializationMode::GEM_JSON    // GArchive JSON wire codec
    };

    for(const auto mode : modes) {
        INFO("serialization mode: " << Gem::Common::serModeToString(mode));

        const auto original = makeContainer();
        REQUIRE(original.get_payload());
        const auto original_uuid = original.get_payload()->getSubmissionUuid();

        const std::string archive = container_to_string(original, mode);

        test_container_type restored{networked_consumer_payload_command::NONE};
        REQUIRE_NOTHROW(container_from_string(archive, restored, mode));

        CHECK(restored.get_command() == networked_consumer_payload_command::COMPUTE);
        REQUIRE(restored.get_payload());
        CHECK(restored.get_payload()->getSubmissionUuid() == original_uuid);
    }
}

/******************************************************************************/
