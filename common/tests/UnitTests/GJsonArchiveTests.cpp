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

#include <cstdint>
#include <filesystem>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "common/GJsonArchive.hpp"

using Gem::Weft::base_object;
using Gem::Weft::GJsonIArchive;
using Gem::Weft::GJsonOArchive;

// ---------------------------------------------------------------------------
// Round-trip helper: save through serialized JSON *text* (the real checkpoint
// path -- also exercises boost::json's number round-trip guarantee), re-parse,
// load into a fresh T, and return it for an EQUAL check.

namespace {

template <typename T>
T roundtrip(const T &v) {
    GJsonOArchive oa;
    oa &v;
    std::string text = oa.str();

    GJsonIArchive ia(text);
    T out{};
    ia &out;
    return out;
}

struct Point {
    std::int32_t x = 0;
    double y = 0.0;
    std::string name;

    template <typename Archive>
    void serialize(Archive &ar, unsigned) {
        ar &Gem::Weft::make_nvp("x", x);
        ar &Gem::Weft::make_nvp("y", y);
        ar &Gem::Weft::make_nvp("name", name);
    }
    bool operator==(const Point &) const = default;
};

class Secret {
public:
    Secret() = default;
    Secret(int a, std::string b) : a_(a), b_(std::move(b)) {}
    bool operator==(const Secret &) const = default;

private:
    friend struct Gem::Weft::access;
    template <typename Archive>
    void serialize(Archive &ar, unsigned) {
        ar &Gem::Weft::make_nvp("a", a_);
        ar &Gem::Weft::make_nvp("b", b_);
    }
    int a_ = 0;
    std::string b_;
};

struct Base {
    int b = 0;
    template <typename Archive>
    void serialize(Archive &ar, unsigned) {
        ar &Gem::Weft::make_nvp("b", b);
    }
    bool operator==(const Base &) const = default;
};

struct Derived : Base {
    int d = 0;
    template <typename Archive>
    void serialize(Archive &ar, unsigned) {
        ar &base_object<Base>(*this);
        ar &Gem::Weft::make_nvp("d", d);
    }
    bool operator==(const Derived &) const = default;
};

enum class Color : std::uint8_t { red = 1, green = 2, blue = 250 };

} // namespace

// ---------------------------------------------------------------------------
// Scalars.

TEST_CASE("GJsonArchive: integers, bool and enum round-trip through text", "[common][archive][json]") {
    CHECK(roundtrip<std::int32_t>(-2000000000) == -2000000000);
    CHECK(roundtrip<std::uint64_t>(std::numeric_limits<std::uint64_t>::max()) ==
          std::numeric_limits<std::uint64_t>::max());
    CHECK(roundtrip<std::int64_t>(std::numeric_limits<std::int64_t>::min()) ==
          std::numeric_limits<std::int64_t>::min());
    CHECK(roundtrip<bool>(true) == true);
    CHECK(roundtrip<bool>(false) == false);
    CHECK(roundtrip<Color>(Color::blue) == Color::blue);
}

TEST_CASE("GJsonArchive: float and double round-trip through text", "[common][archive][json]") {
    CHECK(roundtrip<float>(3.14159f) == 3.14159f);
    CHECK(roundtrip<double>(2.718281828459045) == 2.718281828459045);
    CHECK(roundtrip<double>(std::numeric_limits<double>::min()) == std::numeric_limits<double>::min());
    CHECK(roundtrip<double>(-1.0 / 3.0) == -1.0 / 3.0);
}

TEST_CASE("GJsonArchive: long double round-trips bit-exactly via hex-float string", "[common][archive][json]") {
    long double a = 3.14159265358979323846264338327950288L;
    long double b = std::numeric_limits<long double>::min();
    long double c = -1.0L / 3.0L;
    CHECK(roundtrip<long double>(a) == a);
    CHECK(roundtrip<long double>(b) == b);
    CHECK(roundtrip<long double>(c) == c);
}

TEST_CASE("GJsonArchive: string and path round-trip", "[common][archive][json]") {
    CHECK(roundtrip<std::string>("") == "");
    CHECK(roundtrip<std::string>("hello \"world\"\n") == "hello \"world\"\n"); // escaping survives
    std::filesystem::path p{"/opt/geneva/config/GEvolutionaryAlgorithm.json"};
    CHECK(roundtrip<std::filesystem::path>(p) == p);
}

// ---------------------------------------------------------------------------
// Containers and composites.

TEST_CASE("GJsonArchive: containers round-trip", "[common][archive][json]") {
    CHECK(roundtrip<std::vector<int>>({1, 2, 3}) == std::vector<int>{1, 2, 3});
    CHECK(roundtrip<std::vector<int>>({}) == std::vector<int>{});
    std::vector<std::vector<double>> nested{{1.5, 2.5}, {}, {3.5}};
    CHECK(roundtrip(nested) == nested);
    std::set<int> s{5, 3, 9, 1};
    CHECK(roundtrip(s) == s);
    std::map<std::string, int> m{{"one", 1}, {"two", 2}};
    CHECK(roundtrip(m) == m);
    std::pair<int, std::string> pr{7, "seven"};
    CHECK(roundtrip(pr) == pr);
    std::tuple<int, double, std::string> t{1, 2.5, "x"};
    CHECK(roundtrip(t) == t);
}

// ---------------------------------------------------------------------------
// Serializable classes.

TEST_CASE("GJsonArchive: classes (public, private-via-access, base_object) round-trip", "[common][archive][json]") {
    Point p{42, 3.5, "origin"};
    CHECK(roundtrip(p) == p);

    Secret sec{99, "classified"};
    CHECK(roundtrip(sec) == sec);

    Derived d;
    d.b = 11;
    d.d = 22;
    CHECK(roundtrip(d) == d);

    std::vector<Point> pts{{1, 1.0, "a"}, {2, 2.0, "b"}};
    CHECK(roundtrip(pts) == pts);
}

// ---------------------------------------------------------------------------
// The JSON is actually human-readable: member names appear as keys.

TEST_CASE("GJsonArchive: output is keyed by member name (inspectable)", "[common][archive][json]") {
    GJsonOArchive oa;
    Point p{42, 3.5, "origin"};
    oa &p;
    std::string text = oa.str();
    CHECK(text.find("\"x\":") != std::string::npos);
    CHECK(text.find("\"y\":") != std::string::npos);
    CHECK(text.find("\"name\":") != std::string::npos);
    CHECK(text.find("\"origin\"") != std::string::npos);

    // The assembled tree is a JSON object with the three named members.
    const boost::json::value &v = oa.value();
    REQUIRE(v.is_object());
    CHECK(v.as_object().contains("x"));
    CHECK(v.as_object().contains("y"));
    CHECK(v.as_object().contains("name"));
}

TEST_CASE("GJsonArchive: a map serializes as an array of key/value objects", "[common][archive][json]") {
    GJsonOArchive oa;
    std::map<std::string, int> m{{"a", 1}};
    oa &m;
    const boost::json::value &v = oa.value();
    REQUIRE(v.is_array());
    REQUIRE(v.as_array().size() == 1);
    const boost::json::value &entry = v.as_array().at(0);
    REQUIRE(entry.is_object());
    CHECK(entry.as_object().contains("key"));
    CHECK(entry.as_object().contains("value"));
}
