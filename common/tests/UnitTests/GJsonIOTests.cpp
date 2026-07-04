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

#include <filesystem>
#include <fstream>
#include <string>

#include <boost/json.hpp>

#include "common/GExceptions.hpp"
#include "common/GJsonIO.hpp"

using namespace Gem::Common;
namespace json = boost::json;

namespace {
std::filesystem::path scratch(std::string const &tag) {
    auto base = std::filesystem::temp_directory_path() / "geneva_json_io_tests";
    std::filesystem::create_directories(base);
    return base / tag;
}
} // namespace

// ---------------------------------------------------------------------------
// prettyPrintJson layout: one member / element per line, four-space indent.

TEST_CASE("prettyPrintJson: objects and arrays print one entry per line",
          "[common][json-io][pretty]") {
    json::object obj;
    obj["comment"] = json::array{"line one", "line two", "line three"};
    obj["value"] = "42";

    const std::string out = prettyPrintJson(json::value(std::move(obj)));

    // Each comment element must be on its own line (hand-editing readability requirement).
    CHECK(out.find("\"line one\"") != std::string::npos);
    CHECK(out.find("\"line two\"") != std::string::npos);
    // The three comment strings appear on three distinct lines, in order.
    const auto p1 = out.find("line one");
    const auto p2 = out.find("line two");
    const auto p3 = out.find("line three");
    REQUIRE(p1 != std::string::npos);
    REQUIRE(p2 != std::string::npos);
    REQUIRE(p3 != std::string::npos);
    CHECK(p1 < p2);
    CHECK(p2 < p3);
    // A newline separates consecutive comment elements.
    CHECK(out.find('\n', p1) < p2);
    // Four-space indentation is used for top-level members.
    CHECK(out.find("\n    \"comment\"") != std::string::npos);
}

TEST_CASE("prettyPrintJson: empty containers are compact",
          "[common][json-io][pretty]") {
    CHECK(prettyPrintJson(json::value(json::object{})) == "{}");
    CHECK(prettyPrintJson(json::value(json::array{})) == "[]");
}

// ---------------------------------------------------------------------------
// Round-trip: pretty-print then parse yields an equal value.

TEST_CASE("parse/prettyPrint round-trip preserves structure and values",
          "[common][json-io][roundtrip]") {
    json::value original = json::parse(R"({
        "name": "geneva",
        "nested": { "a": "1", "b": "2" },
        "vec": ["10", "20", "30"],
        "typed_numbers": [1, 2, 3],
        "flag": true,
        "empty_obj": {},
        "empty_arr": []
    })");

    const std::string pretty = prettyPrintJson(original);
    json::value reparsed = parseJsonString(pretty, "round-trip");

    CHECK(reparsed == original);
}

// ---------------------------------------------------------------------------
// Inv 15 regression guard: the reason we left Boost.PropertyTree. A multi-element
// array must round-trip as N elements. The old ptree config format serialized
// vectors as duplicate "item" keys inside an object; a strict JSON parser collapses
// duplicate object keys to one (silent data loss). Using a JSON array instead, all
// elements survive.

TEST_CASE("multi-element array survives round-trip (no dup-key collapse)",
          "[common][json-io][roundtrip][regression]") {
    json::value v = json::array{"1", "1", "1"};
    json::value back = parseJsonString(prettyPrintJson(v), "vector");
    REQUIRE(back.is_array());
    CHECK(back.get_array().size() == 3);
    CHECK(back == v);
}

// ---------------------------------------------------------------------------
// Parser leniency: comments and trailing commas are tolerated.

TEST_CASE("parseJsonString: tolerates comments and trailing commas",
          "[common][json-io][lenient]") {
    json::value v = parseJsonString(R"({
        // a line comment
        "a": "1",
        "b": "2",   /* a block comment */
    })", "lenient");
    REQUIRE(v.is_object());
    CHECK(v.get_object().at("a").as_string() == "1");
    CHECK(v.get_object().at("b").as_string() == "2");
}

TEST_CASE("parseJsonString: throws a geneva_exception on malformed input",
          "[common][json-io][error]") {
    CHECK_THROWS_AS(parseJsonString("{ not valid ", "bad"), geneva_exception);
}

// ---------------------------------------------------------------------------
// File I/O: writeJsonFile then parseJsonFile round-trips and appends a newline.

TEST_CASE("writeJsonFile / parseJsonFile round-trip",
          "[common][json-io][file]") {
    const auto path = scratch("roundtrip.json");
    json::value v = json::object{{"k", "v"}, {"list", json::array{"a", "b"}}};

    writeJsonFile(path, v);
    CHECK(parseJsonFile(path) == v);

    // The file ends in a single trailing newline.
    std::ifstream ifs(path, std::ios::binary);
    std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    REQUIRE_FALSE(content.empty());
    CHECK(content.back() == '\n');

    std::filesystem::remove(path);
}

TEST_CASE("parseJsonFile: throws a geneva_exception for a missing file",
          "[common][json-io][error]") {
    CHECK_THROWS_AS(parseJsonFile(scratch("does_not_exist.json")), geneva_exception);
}
