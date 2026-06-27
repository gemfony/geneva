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

// Standard headers go here
#include <tuple>

// Boost headers go here
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

/******************************************************************************/
/**
 * This file contains some helper functions needed for the serialization of std::tuple objects
 */

namespace boost::serialization {

/******************************************************************************/
// Note that this code is not satisfactory, as it does not address tuples of
// arbitrary size.

/******************************************************************************/
/**
 * @brief Serialization of a std::tuple with a single element.
 *
 * @tparam archive The Boost.Serialization archive type (input or output)
 * @tparam T0 The element type of the tuple
 * @param ar The archive the tuple element is read from / written to
 * @param tpl The tuple whose element is (de)serialized
 * @param version The Boost.Serialization class version (unused)
 */
template <typename archive, typename T0>
void serialize(archive &ar, std::tuple<T0> &tpl, [[maybe_unused]] unsigned int version) {
    using namespace boost;
    using boost::serialization::make_nvp;

    ar &make_nvp("tpl_0", std::get<0>(tpl));
}

/******************************************************************************/
/**
 * @brief Serialization of a std::tuple with two elements.
 *
 * @tparam archive The Boost.Serialization archive type (input or output)
 * @tparam T0 The type of the first tuple element
 * @tparam T1 The type of the second tuple element
 * @param ar The archive the tuple elements are read from / written to
 * @param tpl The tuple whose elements are (de)serialized
 * @param version The Boost.Serialization class version (unused)
 */
template <typename archive, typename T0, typename T1>
void serialize(archive &ar, std::tuple<T0, T1> &tpl, [[maybe_unused]] unsigned int version) {
    using namespace boost;
    using boost::serialization::make_nvp;

    ar &make_nvp("tpl_0", std::get<0>(tpl)) & make_nvp("tpl_1", std::get<1>(tpl));
}

/******************************************************************************/
/**
 * @brief Serialization of a std::tuple with three elements.
 *
 * @tparam archive The Boost.Serialization archive type (input or output)
 * @tparam T0 The type of the first tuple element
 * @tparam T1 The type of the second tuple element
 * @tparam T2 The type of the third tuple element
 * @param ar The archive the tuple elements are read from / written to
 * @param tpl The tuple whose elements are (de)serialized
 * @param version The Boost.Serialization class version (unused)
 */
template <typename archive, typename T0, typename T1, typename T2>
void serialize(archive &ar, std::tuple<T0, T1, T2> &tpl, [[maybe_unused]] unsigned int version) {
    using namespace boost;
    using boost::serialization::make_nvp;

    ar &make_nvp("tpl_0", std::get<0>(tpl)) & make_nvp("tpl_1", std::get<1>(tpl)) &
        make_nvp("tpl_2", std::get<2>(tpl));
}

/******************************************************************************/
/**
 * @brief Serialization of a std::tuple with four elements.
 *
 * @tparam archive The Boost.Serialization archive type (input or output)
 * @tparam T0 The type of the first tuple element
 * @tparam T1 The type of the second tuple element
 * @tparam T2 The type of the third tuple element
 * @tparam T3 The type of the fourth tuple element
 * @param ar The archive the tuple elements are read from / written to
 * @param tpl The tuple whose elements are (de)serialized
 * @param version The Boost.Serialization class version (unused)
 */
template <typename archive, typename T0, typename T1, typename T2, typename T3>
void serialize(archive &ar, std::tuple<T0, T1, T2, T3> &tpl, [[maybe_unused]] unsigned int version) {
    using namespace boost;
    using boost::serialization::make_nvp;

    ar &make_nvp("tpl_0", std::get<0>(tpl)) & make_nvp("tpl_1", std::get<1>(tpl)) &
        make_nvp("tpl_2", std::get<2>(tpl)) & make_nvp("tpl_3", std::get<3>(tpl));
}

/******************************************************************************/
/**
 * @brief Serialization of a std::tuple with five elements.
 *
 * @tparam archive The Boost.Serialization archive type (input or output)
 * @tparam T0 The type of the first tuple element
 * @tparam T1 The type of the second tuple element
 * @tparam T2 The type of the third tuple element
 * @tparam T3 The type of the fourth tuple element
 * @tparam T4 The type of the fifth tuple element
 * @param ar The archive the tuple elements are read from / written to
 * @param tpl The tuple whose elements are (de)serialized
 * @param version The Boost.Serialization class version (unused)
 */
template <typename archive, typename T0, typename T1, typename T2, typename T3, typename T4>
void serialize(archive &ar, std::tuple<T0, T1, T2, T3, T4> &tpl, [[maybe_unused]] unsigned int version) {
    using namespace boost;
    using boost::serialization::make_nvp;

    ar &make_nvp("tpl_0", std::get<0>(tpl)) & make_nvp("tpl_1", std::get<1>(tpl)) &
        make_nvp("tpl_2", std::get<2>(tpl)) & make_nvp("tpl_3", std::get<3>(tpl)) &
        make_nvp("tpl_4", std::get<4>(tpl));
}

/******************************************************************************/
/**
 * @brief Serialization of a std::tuple with six elements.
 *
 * @tparam archive The Boost.Serialization archive type (input or output)
 * @tparam T0 The type of the first tuple element
 * @tparam T1 The type of the second tuple element
 * @tparam T2 The type of the third tuple element
 * @tparam T3 The type of the fourth tuple element
 * @tparam T4 The type of the fifth tuple element
 * @tparam T5 The type of the sixth tuple element
 * @param ar The archive the tuple elements are read from / written to
 * @param tpl The tuple whose elements are (de)serialized
 * @param version The Boost.Serialization class version (unused)
 */
template <
    typename archive,
    typename T0,
    typename T1,
    typename T2,
    typename T3,
    typename T4,
    typename T5>
void serialize(archive &ar, std::tuple<T0, T1, T2, T3, T4, T5> &tpl, [[maybe_unused]] unsigned int version) {
    using namespace boost;
    using boost::serialization::make_nvp;

    ar &make_nvp("tpl_0", std::get<0>(tpl)) & make_nvp("tpl_1", std::get<1>(tpl)) &
        make_nvp("tpl_2", std::get<2>(tpl)) & make_nvp("tpl_3", std::get<3>(tpl)) &
        make_nvp("tpl_4", std::get<4>(tpl)) & make_nvp("tpl_5", std::get<5>(tpl));
}

/******************************************************************************/

} /* namespace boost::serialization */
