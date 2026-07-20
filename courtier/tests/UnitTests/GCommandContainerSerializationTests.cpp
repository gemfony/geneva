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
 * Regression guard: container_to_string() must return a COMPLETE archive document.
 *
 * A boost output archive may append trailing content in its destructor -- xml_oarchive writes
 * the closing </boost_serialization> root tag there. Reading the underlying stream from inside
 * the archive's scope (`return oss.str();` next to a live `oa`) therefore yields a truncated
 * document. The damage is not contained to the writer: the truncated text still de-serializes
 * successfully, and ~xml_iarchive then fails while winding up and throws from an implicitly
 * noexcept destructor -- std::terminate, which container_from_string()'s try/catch cannot
 * intercept. XML is user-selectable (--asio_serializationMode / --beast_serializationMode /
 * --mpi_serializationMode), so this is reachable, not merely theoretical.
 *
 * The well-formedness assertions below are REQUIREs on purpose: on the unfixed code they must
 * stop the test case before the round-trip section terminates the whole test binary.
 */
TEST_CASE("container_to_string emits a complete archive document in every mode", "[courtier][serialization]") {
    const auto container = makeContainer();

    SECTION("XML carries its closing root tag") {
        const std::string xml =
            container_to_string(container, Gem::Common::serializationMode::XML);

        REQUIRE_FALSE(xml.empty());
        INFO("emitted XML tail: " << xml.substr(xml.size() > 120 ? xml.size() - 120 : 0));
        REQUIRE(xml.find("<boost_serialization") != std::string::npos);
        REQUIRE(xml.find("</boost_serialization>") != std::string::npos);
    }

    SECTION("TEXT and BINARY are non-empty") {
        REQUIRE_FALSE(container_to_string(container, Gem::Common::serializationMode::TEXT).empty());
        REQUIRE_FALSE(
            container_to_string(container, Gem::Common::serializationMode::BINARY).empty()
        );
    }
}

/******************************************************************************/
/**
 * The companion round-trip: every serialization mode must survive a write/read cycle with the
 * command, the payload and the payload's stable lineage id intact. On the unfixed code the XML
 * arm of this test terminates the process rather than failing -- which is precisely why the
 * document-completeness case above exists and runs first.
 */
TEST_CASE("GCommandContainerT survives a round-trip in every serialization mode", "[courtier][serialization]") {
    const auto modes = {
        Gem::Common::serializationMode::TEXT,
        Gem::Common::serializationMode::XML,
        Gem::Common::serializationMode::BINARY
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
