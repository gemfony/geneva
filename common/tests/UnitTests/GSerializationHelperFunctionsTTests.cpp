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

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include "common/GCommonEnums.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GStdFilesystemPathSerialization.hpp"

using namespace Gem::Common;

// ---------------------------------------------------------------------------
// Tiny Boost.Serialization-friendly payload type used to exercise the
// sharedPtrToString / sharedPtrFromString round-trip.

namespace {

class TestPayload {
public:
    TestPayload() = default;
    TestPayload(int i, double d, std::string s) : i_(i), d_(d), s_(std::move(s)) {}

    int            i() const { return i_; }
    double         d() const { return d_; }
    std::string    s() const { return s_; }

    bool operator==(TestPayload const &o) const = default;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive &ar, [[maybe_unused]] unsigned int version) {
        ar &boost::serialization::make_nvp("i", i_);
        ar &boost::serialization::make_nvp("d", d_);
        ar &boost::serialization::make_nvp("s", s_);
    }

    int         i_{0};
    double      d_{0.0};
    std::string s_;
};

// Helpers that drive a free-function save/load round-trip through a specific
// archive family. Used by the tribool / chrono / atomic<bool> tests below.

template <class Save, class Load, class Value>
Value round_trip_text(Value const &in, Save save, Load load) {
    std::stringstream ss;
    {
        boost::archive::text_oarchive oa(ss);
        boost::serialization::save(oa, in, 0u);
    }
    Value out{};
    {
        boost::archive::text_iarchive ia(ss);
        boost::serialization::load(ia, out, 0u);
    }
    return out;
}

template <class Save, class Load, class Value>
Value round_trip_xml(Value const &in, Save save, Load load) {
    std::stringstream ss;
    {
        boost::archive::xml_oarchive oa(ss);
        boost::serialization::save(oa, in, 0u);
    }
    Value out{};
    {
        boost::archive::xml_iarchive ia(ss);
        boost::serialization::load(ia, out, 0u);
    }
    return out;
}

template <class Save, class Load, class Value>
Value round_trip_binary(Value const &in, Save save, Load load) {
    std::stringstream ss;
    {
        boost::archive::binary_oarchive oa(ss);
        boost::serialization::save(oa, in, 0u);
    }
    Value out{};
    {
        boost::archive::binary_iarchive ia(ss);
        boost::serialization::load(ia, out, 0u);
    }
    return out;
}

} // namespace

// ---------------------------------------------------------------------------
// sharedPtrToString / sharedPtrFromString round-trips through each archive
// family that the helpers support.

TEST_CASE("GSerializationHelperFunctionsT: shared_ptr round-trip via TEXT mode",
          "[common][serialization-helpers]") {
    auto in = std::make_shared<TestPayload>(17, 3.5, "alpha");

    const std::string s = sharedPtrToString(in, serializationMode::TEXT);
    REQUIRE_FALSE(s.empty());

    auto out = sharedPtrFromString<TestPayload>(s, serializationMode::TEXT);
    REQUIRE(out);
    CHECK(*out == *in);
}

TEST_CASE("GSerializationHelperFunctionsT: shared_ptr round-trip via XML mode",
          "[common][serialization-helpers]") {
    auto in = std::make_shared<TestPayload>(-3, 1.25, "beta");

    const std::string s = sharedPtrToString(in, serializationMode::XML);
    REQUIRE_FALSE(s.empty());
    CHECK(s.find("classHierarchyFromT_ptr") != std::string::npos);

    auto out = sharedPtrFromString<TestPayload>(s, serializationMode::XML);
    REQUIRE(out);
    CHECK(*out == *in);
}

TEST_CASE("GSerializationHelperFunctionsT: shared_ptr round-trip via BINARY mode",
          "[common][serialization-helpers]") {
    auto in = std::make_shared<TestPayload>(99, -2.5, "gamma");

    const std::string s = sharedPtrToString(in, serializationMode::BINARY);
    REQUIRE_FALSE(s.empty());

    auto out = sharedPtrFromString<TestPayload>(s, serializationMode::BINARY);
    REQUIRE(out);
    CHECK(*out == *in);
}

TEST_CASE("GSerializationHelperFunctionsT: sharedPtrFromString returns null on garbage input "
          "rather than throwing through",
          "[common][serialization-helpers]") {
    // The implementation catches archive_exception / std::exception, logs, and
    // returns an empty shared_ptr. (Only unknown-type catches throw a geneva_exception.)
    auto out = sharedPtrFromString<TestPayload>("not-a-valid-archive-blob",
                                                serializationMode::TEXT);
    CHECK_FALSE(out);
}

// ---------------------------------------------------------------------------
// tribool serialization round-trip

TEST_CASE("GSerializationHelperFunctionsT: tribool round-trips through each archive",
          "[common][serialization-helpers]") {
    auto save_tb = [](auto &ar, tribool const &v, unsigned int ver) {
        boost::serialization::save(ar, v, ver);
    };
    auto load_tb = [](auto &ar, tribool &v, unsigned int ver) {
        boost::serialization::load(ar, v, ver);
    };

    for(auto const &val : {tribool::False, tribool::True, tribool::Indeterminate}) {
        CHECK(round_trip_text  (val, save_tb, load_tb) == val);
        CHECK(round_trip_xml   (val, save_tb, load_tb) == val);
        CHECK(round_trip_binary(val, save_tb, load_tb) == val);
    }
}

// ---------------------------------------------------------------------------
// std::chrono::duration<double> serialization round-trip

TEST_CASE("GSerializationHelperFunctionsT: chrono::duration<double> round-trips",
          "[common][serialization-helpers]") {
    using D = std::chrono::duration<double>;
    auto save_d = [](auto &ar, D const &v, unsigned int ver) {
        boost::serialization::save(ar, v, ver);
    };
    auto load_d = [](auto &ar, D &v, unsigned int ver) {
        boost::serialization::load(ar, v, ver);
    };

    D in{12.5};
    CHECK(round_trip_text  (in, save_d, load_d).count() == in.count());
    CHECK(round_trip_xml   (in, save_d, load_d).count() == in.count());
    CHECK(round_trip_binary(in, save_d, load_d).count() == in.count());
}

// ---------------------------------------------------------------------------
// std::atomic<bool> serialization round-trip

TEST_CASE("GSerializationHelperFunctionsT: std::atomic<bool> round-trips",
          "[common][serialization-helpers]") {
    using AB = std::atomic<bool>;

    // std::atomic<bool> is non-copyable; build the in-fixture in-place and
    // compare via .load() rather than the helpers' templated value parameter.
    for(bool val : {false, true}) {
        AB in;
        in.store(val);

        // Text
        {
            std::stringstream ss;
            {
                boost::archive::text_oarchive oa(ss);
                boost::serialization::save(oa, in, 0u);
            }
            AB out;
            out.store(not val); // seed with the opposite to detect a no-op load
            {
                boost::archive::text_iarchive ia(ss);
                boost::serialization::load(ia, out, 0u);
            }
            CHECK(out.load() == val);
        }

        // XML
        {
            std::stringstream ss;
            {
                boost::archive::xml_oarchive oa(ss);
                boost::serialization::save(oa, in, 0u);
            }
            AB out;
            out.store(not val);
            {
                boost::archive::xml_iarchive ia(ss);
                boost::serialization::load(ia, out, 0u);
            }
            CHECK(out.load() == val);
        }

        // Binary
        {
            std::stringstream ss;
            {
                boost::archive::binary_oarchive oa(ss);
                boost::serialization::save(oa, in, 0u);
            }
            AB out;
            out.store(not val);
            {
                boost::archive::binary_iarchive ia(ss);
                boost::serialization::load(ia, out, 0u);
            }
            CHECK(out.load() == val);
        }
    }
}

// ---------------------------------------------------------------------------
// std::atomic<T> serialization round-trip for arbitrary value types T
// (the generic std::atomic<T> save/load added alongside the std::atomic<bool>
// overloads). Each value is serialized and deserialized through text, XML and
// binary archives; the deserialized value must match the original.

TEST_CASE("GSerializationHelperFunctionsT: std::atomic<T> round-trips",
          "[common][serialization-helpers]") {
    // std::atomic<T> is non-copyable; round-trip in place and compare via
    // .load(). 'seed' is stored into the output object first, so a no-op load
    // would be detected.
    auto check = [](auto val, auto seed) {
        using T = decltype(val);
        std::atomic<T> in;
        in.store(val);

        // Text
        {
            std::stringstream ss;
            { boost::archive::text_oarchive oa(ss); boost::serialization::save(oa, in, 0u); }
            std::atomic<T> out; out.store(seed);
            { boost::archive::text_iarchive ia(ss); boost::serialization::load(ia, out, 0u); }
            CHECK(out.load() == val);
        }
        // XML
        {
            std::stringstream ss;
            { boost::archive::xml_oarchive oa(ss); boost::serialization::save(oa, in, 0u); }
            std::atomic<T> out; out.store(seed);
            { boost::archive::xml_iarchive ia(ss); boost::serialization::load(ia, out, 0u); }
            CHECK(out.load() == val);
        }
        // Binary
        {
            std::stringstream ss;
            { boost::archive::binary_oarchive oa(ss); boost::serialization::save(oa, in, 0u); }
            std::atomic<T> out; out.store(seed);
            { boost::archive::binary_iarchive ia(ss); boost::serialization::load(ia, out, 0u); }
            CHECK(out.load() == val);
        }
    };

    SECTION("std::size_t (the GFactoryT::id_ case)") {
        check(std::size_t{0}, std::size_t{999});
        check(std::size_t{1}, std::size_t{0});
        check(std::size_t{1234567}, std::size_t{0});
        check((std::numeric_limits<std::size_t>::max)(), std::size_t{0});
    }
    SECTION("int (incl. negative)") {
        check(0, 7);
        check(-42, 0);
        check((std::numeric_limits<int>::max)(), 0);
        check((std::numeric_limits<int>::min)(), 0);
    }
    SECTION("unsigned long") {
        check(0ul, 5ul);
        check(4000000000ul, 0ul);
    }
    SECTION("std::int64_t") {
        check(std::int64_t{-9000000000LL}, std::int64_t{0});
        check((std::numeric_limits<std::int64_t>::max)(), std::int64_t{0});
    }
    SECTION("double (exactly representable values)") {
        check(0.0, 1.0);
        check(-2.5, 0.0);
        check(1024.0, 0.0);
    }
}

// ---------------------------------------------------------------------------
// std::chrono::high_resolution_clock::time_point round-trip
//
// The save/load helpers narrow the time_point to milliseconds, so we cannot
// compare the raw time_points directly. Instead, save A, load B, then save B
// again and check the wire form matches — that confirms the value is
// preserved at millisecond precision through both directions.

TEST_CASE("GSerializationHelperFunctionsT: time_point round-trips at millisecond precision",
          "[common][serialization-helpers]") {
    using TP = std::chrono::high_resolution_clock::time_point;

    TP in = std::chrono::high_resolution_clock::now();

    std::stringstream ss_a;
    {
        boost::archive::text_oarchive oa(ss_a);
        boost::serialization::save(oa, in, 0u);
    }

    TP out{};
    {
        std::stringstream ss_a_copy(ss_a.str());
        boost::archive::text_iarchive ia(ss_a_copy);
        boost::serialization::load(ia, out, 0u);
    }

    // Re-save the loaded value and compare wire forms.
    std::stringstream ss_b;
    {
        boost::archive::text_oarchive oa(ss_b);
        boost::serialization::save(oa, out, 0u);
    }

    CHECK(ss_a.str() == ss_b.str());
}

// ---------------------------------------------------------------------------
// std::filesystem::path free (non-intrusive) serialization round-trip.
//
// Boost has no built-in support for std::filesystem::path. The free serialization
// in GStdFilesystemPathSerialization.hpp stores a path as its string() and rebuilds
// it on load, which is what lets path-holding classes (e.g. oa::GOptimizationAlgorithmBase's
// cp_directory_path_) drop their hand-written save()/load() split. These tests
// exercise that free serialization directly, for several representative paths and
// through all three archive families.

TEST_CASE("GStdFilesystemPathSerialization: path round-trips through each archive",
          "[common][serialization-helpers]") {
    namespace fs = std::filesystem;

    auto save_p = [](auto &ar, fs::path const &v, unsigned int ver) {
        boost::serialization::save(ar, v, ver);
    };
    auto load_p = [](auto &ar, fs::path &v, unsigned int ver) {
        boost::serialization::load(ar, v, ver);
    };

    const std::vector<fs::path> paths{
        fs::path{},                              // empty path
        fs::path{"."},                           // single component
        fs::path{"results"},                     // relative, no separators
        fs::path{"some/relative/dir"},           // relative, multiple components
        fs::path{"/absolute/path/to/checkpoint"},// absolute
        fs::path{"with space/and-dash/file.cp"}, // spaces, dashes, extension
    };

    for(auto const &in : paths) {
        CHECK(round_trip_text  (in, save_p, load_p) == in);
        CHECK(round_trip_xml   (in, save_p, load_p) == in);
        CHECK(round_trip_binary(in, save_p, load_p) == in);
    }
}

TEST_CASE("GStdFilesystemPathSerialization: loaded path preserves its string()",
          "[common][serialization-helpers]") {
    namespace fs = std::filesystem;

    fs::path in{"/some/checkpoint/dir"};

    // Seed the destination with a clearly different value to prove the load
    // actually overwrites it rather than being a no-op.
    fs::path out{"unrelated"};

    std::stringstream ss;
    {
        boost::archive::text_oarchive oa(ss);
        boost::serialization::save(oa, in, 0u);
    }
    {
        boost::archive::text_iarchive ia(ss);
        boost::serialization::load(ia, out, 0u);
    }

    CHECK(out == in);
    CHECK(out.string() == in.string());
}
