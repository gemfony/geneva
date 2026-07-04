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
#include <catch2/catch_template_test_macros.hpp>

#include <sstream>
#include <string>
#include <tuple>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/nvp.hpp>

#include "common/GSerializeTupleT.hpp"

// ---------------------------------------------------------------------------
// Round-trip helper. Each archive family needs a slightly different ctor
// argument set, so we keep helpers per family rather than a generic one.

namespace {

template <class Tuple>
Tuple round_trip_text(Tuple const &in) {
    std::stringstream ss;
    {
        boost::archive::text_oarchive oa(ss);
        oa &boost::serialization::make_nvp("t", const_cast<Tuple &>(in));
    }
    Tuple out{};
    {
        boost::archive::text_iarchive ia(ss);
        ia &boost::serialization::make_nvp("t", out);
    }
    return out;
}

template <class Tuple>
Tuple round_trip_xml(Tuple const &in) {
    std::stringstream ss;
    {
        boost::archive::xml_oarchive oa(ss);
        oa &boost::serialization::make_nvp("t", const_cast<Tuple &>(in));
    }
    Tuple out{};
    {
        boost::archive::xml_iarchive ia(ss);
        ia &boost::serialization::make_nvp("t", out);
    }
    return out;
}

template <class Tuple>
Tuple round_trip_binary(Tuple const &in) {
    std::stringstream ss;
    {
        boost::archive::binary_oarchive oa(ss);
        oa &boost::serialization::make_nvp("t", const_cast<Tuple &>(in));
    }
    Tuple out{};
    {
        boost::archive::binary_iarchive ia(ss);
        ia &boost::serialization::make_nvp("t", out);
    }
    return out;
}

} // namespace

// ---------------------------------------------------------------------------
// One-element tuple

TEST_CASE("GSerializeTupleT: 1-element tuple round-trips through every archive family",
          "[common][serialize-tuple]") {
    auto in = std::make_tuple(42);

    CHECK(round_trip_text  (in) == in);
    CHECK(round_trip_xml   (in) == in);
    CHECK(round_trip_binary(in) == in);
}

// ---------------------------------------------------------------------------
// Two-element tuple

TEST_CASE("GSerializeTupleT: 2-element tuple round-trips through every archive family",
          "[common][serialize-tuple]") {
    auto in = std::make_tuple(3, 4.5);
    CHECK(round_trip_text  (in) == in);
    CHECK(round_trip_xml   (in) == in);
    CHECK(round_trip_binary(in) == in);
}

// ---------------------------------------------------------------------------
// Three-element tuple, including a non-trivial std::string element to
// exercise allocation/streaming paths.

TEST_CASE("GSerializeTupleT: 3-element tuple incl. std::string round-trips",
          "[common][serialize-tuple]") {
    auto in = std::make_tuple(1, 2.5, std::string{"hello"});
    CHECK(round_trip_text  (in) == in);
    CHECK(round_trip_xml   (in) == in);
    CHECK(round_trip_binary(in) == in);
}

// ---------------------------------------------------------------------------
// Four-element tuple

TEST_CASE("GSerializeTupleT: 4-element tuple round-trips",
          "[common][serialize-tuple]") {
    auto in = std::make_tuple(1, 2, 3.5, std::string{"abc"});
    CHECK(round_trip_text  (in) == in);
    CHECK(round_trip_xml   (in) == in);
    CHECK(round_trip_binary(in) == in);
}

// ---------------------------------------------------------------------------
// Five-element tuple

TEST_CASE("GSerializeTupleT: 5-element tuple round-trips",
          "[common][serialize-tuple]") {
    auto in = std::make_tuple(1, 2.5, std::string{"x"}, std::int64_t{99}, false);
    CHECK(round_trip_text  (in) == in);
    CHECK(round_trip_xml   (in) == in);
    CHECK(round_trip_binary(in) == in);
}

// ---------------------------------------------------------------------------
// Six-element tuple — the largest currently-supported arity. If the file
// ever grows to support 7+ elements, add cases for those here.

TEST_CASE("GSerializeTupleT: 6-element tuple round-trips",
          "[common][serialize-tuple]") {
    auto in = std::make_tuple(1, 2.5, std::string{"x"}, std::int64_t{99}, true, std::string{"y"});
    CHECK(round_trip_text  (in) == in);
    CHECK(round_trip_xml   (in) == in);
    CHECK(round_trip_binary(in) == in);
}

// ---------------------------------------------------------------------------
// Ensure each archive actually produces a non-empty payload (sanity-check
// that the serialize() overload was selected, not a no-op fallback).

TEST_CASE("GSerializeTupleT: each archive writes a non-empty payload",
          "[common][serialize-tuple]") {
    auto in = std::make_tuple(1, std::string{"abc"});

    std::stringstream text_ss;
    std::stringstream xml_ss;
    std::stringstream bin_ss;
    {
        boost::archive::text_oarchive oa(text_ss);
        oa &boost::serialization::make_nvp("t", in);
    }
    {
        boost::archive::xml_oarchive oa(xml_ss);
        oa &boost::serialization::make_nvp("t", in);
    }
    {
        boost::archive::binary_oarchive oa(bin_ss);
        oa &boost::serialization::make_nvp("t", in);
    }

    CHECK_FALSE(text_ss.str().empty());
    CHECK_FALSE(xml_ss.str().empty());
    CHECK_FALSE(bin_ss.str().empty());

    // Spot-check XML for the expected element names.
    CHECK(xml_ss.str().contains("tpl_0"));
    CHECK(xml_ss.str().contains("tpl_1"));
}
