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

#include <sstream>
#include <string>

#include <boost/lexical_cast.hpp>

#include "common/GCommonEnums.hpp"

using namespace Gem::Common;

// Generic helper: stream the enum out, parse the integral form back in and
// expect bit-for-bit equality. The integral-only round-trip mirrors the
// boost::lexical_cast usage all callers go through.
template <class E>
E round_trip_int(E in) {
    std::ostringstream oss;
    oss << static_cast<ENUMBASETYPE>(in);   // numeric form, matches operator<< overloads
    std::istringstream iss(oss.str());
    E out{};
    iss >> out;
    return out;
}

TEST_CASE("ENUMBASETYPE: aliases std::uint16_t", "[common][enums]") {
    static_assert(std::is_same_v<ENUMBASETYPE, std::uint16_t>);
}

// ---------------------------------------------------------------------------
// parameter_source / sortOrder / dimensions / logType / triboolStates /
// serializationMode / expectation: numeric round-trip is the contract.

TEST_CASE("parameter_source: numeric round-trip preserves value", "[common][enums]") {
    for(auto e : {parameter_source::NETWORK,
                  parameter_source::COMMAND_LINE,
                  parameter_source::ENVIRONMENT_VARIABLE,
                  parameter_source::CONFIGURATION_FILE,
                  parameter_source::ASSIGNMENT}) {
        CHECK(round_trip_int(e) == e);
    }
}

TEST_CASE("sortOrder: numeric round-trip preserves value", "[common][enums]") {
    CHECK(round_trip_int(sortOrder::LOWERISBETTER)  == sortOrder::LOWERISBETTER);
    CHECK(round_trip_int(sortOrder::HIGHERISBETTER) == sortOrder::HIGHERISBETTER);
}

TEST_CASE("dimensions: numeric round-trip preserves value", "[common][enums]") {
    for(auto d : {dimensions::Dim1, dimensions::Dim2, dimensions::Dim3, dimensions::Dim4}) {
        CHECK(round_trip_int(d) == d);
    }
}

TEST_CASE("logType: numeric round-trip preserves value", "[common][enums]") {
    for(auto l : {logType::EXCEPTION, logType::TERMINATION, logType::WARNING,
                  logType::LOGGING,   logType::FILE,        logType::STDOUT,
                  logType::STDERR}) {
        CHECK(round_trip_int(l) == l);
    }
}

TEST_CASE("triboolStates: numeric round-trip preserves value", "[common][enums]") {
    for(auto t : {triboolStates::TBS_FALSE, triboolStates::TBS_TRUE, triboolStates::TBS_INDETERMINATE}) {
        CHECK(round_trip_int(t) == t);
    }
}

TEST_CASE("serializationMode: numeric round-trip preserves value", "[common][enums]") {
    for(auto m : {serializationMode::TEXT, serializationMode::XML, serializationMode::BINARY}) {
        CHECK(round_trip_int(m) == m);
    }
}

TEST_CASE("expectation: numeric round-trip preserves value", "[common][enums]") {
    for(auto e : {expectation::EQUALITY, expectation::FP_SIMILARITY, expectation::INEQUALITY}) {
        CHECK(round_trip_int(e) == e);
    }
}

// ---------------------------------------------------------------------------
// tribool prints names (not numeric form).

TEST_CASE("tribool::operator<<: prints the textual name", "[common][enums]") {
    std::ostringstream oss_true;
    std::ostringstream oss_false;
    std::ostringstream oss_indet;
    oss_true  << tribool::True;
    oss_false << tribool::False;
    oss_indet << tribool::Indeterminate;
    CHECK(oss_true.str()  == "True");
    CHECK(oss_false.str() == "False");
    CHECK(oss_indet.str() == "Indeterminate");
}

TEST_CASE("tribool::operator<<: out-of-range value is diagnosed (not silent)",
          "[common][enums]") {
    // Force an unknown enumerator (e.g. via a corrupted-stream load). The
    // stream-out must produce a recognisable diagnostic prefix rather than
    // an empty/silent output.
    auto bad = static_cast<tribool>(99);
    std::ostringstream oss;
    oss << bad;
    CHECK(oss.str().find("tribool::?") != std::string::npos);
    CHECK(oss.str().find("99")         != std::string::npos);
}

TEST_CASE("indeterminate() helper", "[common][enums]") {
    CHECK_FALSE(indeterminate(tribool::True));
    CHECK_FALSE(indeterminate(tribool::False));
    CHECK(indeterminate(tribool::Indeterminate));
}

// ---------------------------------------------------------------------------
// serModeToString: dedicated debug helper, plus the "unknown" fallback.

TEST_CASE("serModeToString: maps each mode to its name", "[common][enums]") {
    CHECK(serModeToString(serializationMode::TEXT)   == "TEXT");
    CHECK(serModeToString(serializationMode::XML)    == "XML");
    CHECK(serModeToString(serializationMode::BINARY) == "BINARY");
}

TEST_CASE("serModeToString: unknown value falls through to 'unknown'", "[common][enums]") {
    CHECK(serModeToString(static_cast<serializationMode>(99)) == "unknown");
}

// ---------------------------------------------------------------------------
// boost::lexical_cast: every enum that has operator<< / operator>> defined
// for stream interop should be lexical_cast-able both directions. This is
// the actual call-site pattern in the parser builder.

TEST_CASE("boost::lexical_cast: enums round-trip through string<->enum",
          "[common][enums][lexical-cast]") {
    auto check = [](auto in) {
        std::string s = boost::lexical_cast<std::string>(in);
        auto out      = boost::lexical_cast<decltype(in)>(s);
        CHECK(out == in);
    };

    check(parameter_source::COMMAND_LINE);
    check(sortOrder::HIGHERISBETTER);
    check(dimensions::Dim3);
    check(logType::WARNING);
    check(triboolStates::TBS_TRUE);
    check(serializationMode::XML);
    check(expectation::FP_SIMILARITY);
}
