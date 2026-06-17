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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <filesystem>
#include <string>

// Boost header files go here
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_free.hpp>

/******************************************************************************/
/**
 * Non-intrusive (free) Boost.Serialization for std::filesystem::path.
 *
 * Boost.Serialization has no built-in support for std::filesystem::path. Rather
 * than having every path-holding class split its serialize() into a manual
 * save()/load() that converts the path to/from std::string by hand, this
 * one-time free serialization moves that asymmetry from the class to the type:
 * a path is stored as its string() representation and rebuilt on load. Any
 * class that includes this header can then serialize a std::filesystem::path
 * member directly through a plain symmetric serialize().
 */
namespace boost::serialization {

/**
 * @brief Boost.Serialization save hook for std::filesystem::path.
 *
 * Stores the path as its string() representation under the NVP key "path".
 *
 * @tparam Archive The Boost.Serialization output archive type
 * @param ar The archive the path is written to
 * @param p The path to be serialized
 * @param version The class version supplied by Boost.Serialization (unused)
 */
template <class Archive>
void save(Archive &ar, const std::filesystem::path &p, [[maybe_unused]] const unsigned int version) {
    std::string s = p.string();
    ar & boost::serialization::make_nvp("path", s);
}

/**
 * @brief Boost.Serialization load hook for std::filesystem::path.
 *
 * Reads the stored string from the NVP key "path" and rebuilds the path from it.
 *
 * @tparam Archive The Boost.Serialization input archive type
 * @param ar The archive the path is read from
 * @param p The path to be reconstructed from the archive (overwritten)
 * @param version The class version supplied by Boost.Serialization (unused)
 */
template <class Archive>
void load(Archive &ar, std::filesystem::path &p, [[maybe_unused]] const unsigned int version) {
    std::string s;
    ar & boost::serialization::make_nvp("path", s);
    p = std::filesystem::path(s);
}

} /* namespace boost::serialization */

BOOST_SERIALIZATION_SPLIT_FREE(std::filesystem::path) // NOLINT

/******************************************************************************/
