/**
 * @file GCourtierStandardTests.cpp
 */

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
#include <vector>

#include "courtier/GDemoProcessingContainers.hpp"
#include "courtier/GExecutorT.hpp"

using namespace Gem::Courtier;

/********************************************************************************************/
// GSerialExecutorT tests
//
// GSerialExecutorT processes work items inline during submit() — no broker or threads
// required. This makes it the simplest possible entry point for Courtier unit tests.
//
// Note: items are initialised with processingStatus::DO_IGNORE. They must be explicitly
// marked DO_PROCESS before workOn() — the executor silently skips anything else.

TEST_CASE("GSerialExecutorT: single item is processed", "[courtier][serial]") {
    auto executor = std::make_shared<GSerialExecutorT<GSimpleContainer>>();

    std::vector<std::shared_ptr<GSimpleContainer>> items;
    items.emplace_back(std::make_shared<GSimpleContainer>(3));
    items[0]->set_processing_status(processingStatus::DO_PROCESS);

    auto status = executor->workOn(items);

    REQUIRE(status.is_complete);
    REQUIRE_FALSE(status.has_errors);
    REQUIRE(items[0]->is_processed());
}

TEST_CASE("GSerialExecutorT: multiple items are all processed", "[courtier][serial]") {
    auto executor = std::make_shared<GSerialExecutorT<GSimpleContainer>>();

    const std::size_t N = 10;
    std::vector<std::shared_ptr<GSimpleContainer>> items;
    for(std::size_t i = 0; i < N; ++i) {
        auto item = std::make_shared<GSimpleContainer>(i + 1);
        item->set_processing_status(processingStatus::DO_PROCESS);
        items.emplace_back(std::move(item));
    }

    auto status = executor->workOn(items);

    REQUIRE(status.is_complete);
    REQUIRE_FALSE(status.has_errors);
    for(auto const &item : items) {
        REQUIRE(item->is_processed());
    }
}

TEST_CASE("GSerialExecutorT: empty work list completes without error", "[courtier][serial]") {
    auto executor = std::make_shared<GSerialExecutorT<GSimpleContainer>>();

    std::vector<std::shared_ptr<GSimpleContainer>> items;
    auto status = executor->workOn(items);

    REQUIRE(status.is_complete);
    REQUIRE_FALSE(status.has_errors);
}

TEST_CASE("GSerialExecutorT: GRandomNumberContainer items are processed", "[courtier][serial]") {
    auto executor = std::make_shared<GSerialExecutorT<GRandomNumberContainer>>();

    const std::size_t N = 5;
    std::vector<std::shared_ptr<GRandomNumberContainer>> items;
    for(std::size_t i = 0; i < N; ++i) {
        auto item = std::make_shared<GRandomNumberContainer>(10);
        item->set_processing_status(processingStatus::DO_PROCESS);
        items.emplace_back(std::move(item));
    }

    auto status = executor->workOn(items);

    REQUIRE(status.is_complete);
    REQUIRE_FALSE(status.has_errors);
    for(auto const &item : items) {
        REQUIRE(item->is_processed());
    }
}

TEST_CASE(
    "GSerialExecutorT: processing status transitions from DO_PROCESS to PROCESSED",
    "[courtier][serial]"
) {
    auto executor = std::make_shared<GSerialExecutorT<GSimpleContainer>>();

    auto item = std::make_shared<GSimpleContainer>(1);
    REQUIRE_FALSE(item->is_processed()); // newly created → DO_IGNORE, not processed

    item->set_processing_status(processingStatus::DO_PROCESS);
    std::vector<std::shared_ptr<GSimpleContainer>> items{item};
    executor->workOn(items);

    REQUIRE(item->is_processed());
}
