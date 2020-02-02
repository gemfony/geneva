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
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
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
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>

/******************************************************************************/
/**
 * This file contains some helper functions needed for the serialization of std::tuple objects
 */

namespace boost {
namespace serialization {

/******************************************************************************/
// Note that this code is not satisfactory, as it does not address tuples of
// arbitrary size.

/******************************************************************************/
/**
 * Serialization of a std::tuple with a single element
 *
 * @param tpl The tuple to be serialized
 */
template<typename archive, typename T0>
void serialize(archive &ar, std::tuple<T0> &tpl, unsigned int) {
	using namespace boost;
	using boost::serialization::make_nvp;

	ar
		&make_nvp("tpl_0", std::get<0>(tpl));
}

/******************************************************************************/
/**
 * Serialization of a std::tuple with two elements
 *
 * @param tpl The tuple to be serialized
 */
template<typename archive, typename T0, typename T1>
void serialize(archive &ar, std::tuple<T0, T1> &tpl, unsigned int) {
	using namespace boost;
	using boost::serialization::make_nvp;

	ar
	& make_nvp("tpl_0", std::get<0>(tpl))
	& make_nvp("tpl_1", std::get<1>(tpl));
}

/******************************************************************************/
/**
 * Serialization of a std::tuple with three elements
 *
 * @param tpl The tuple to be serialized
 */
template<typename archive, typename T0, typename T1, typename T2>
void serialize(archive &ar, std::tuple<T0, T1, T2> &tpl, unsigned int) {
	using namespace boost;
	using boost::serialization::make_nvp;

	ar
	& make_nvp("tpl_0", std::get<0>(tpl))
	& make_nvp("tpl_1", std::get<1>(tpl))
	& make_nvp("tpl_2", std::get<2>(tpl));
}

/******************************************************************************/
/**
 * Serialization of a std::tuple with four elements
 *
 * @param tpl The tuple to be serialized
 */
template<typename archive, typename T0, typename T1, typename T2, typename T3>
void serialize(archive &ar, std::tuple<T0, T1, T2, T3> &tpl, unsigned int) {
	using namespace boost;
	using boost::serialization::make_nvp;

	ar
	& make_nvp("tpl_0", std::get<0>(tpl))
	& make_nvp("tpl_1", std::get<1>(tpl))
	& make_nvp("tpl_2", std::get<2>(tpl))
	& make_nvp("tpl_3", std::get<3>(tpl));
}

/******************************************************************************/
/**
 * Serialization of a std::tuple with five elements
 *
 * @param tpl The tuple to be serialized
 */
template<typename archive, typename T0, typename T1, typename T2, typename T3, typename T4>
void serialize(archive &ar, std::tuple<T0, T1, T2, T3, T4> &tpl, unsigned int) {
	using namespace boost;
	using boost::serialization::make_nvp;

	ar
	& make_nvp("tpl_0", std::get<0>(tpl))
	& make_nvp("tpl_1", std::get<1>(tpl))
	& make_nvp("tpl_2", std::get<2>(tpl))
	& make_nvp("tpl_3", std::get<3>(tpl))
	& make_nvp("tpl_4", std::get<4>(tpl));
}

/******************************************************************************/
/**
 * Serialization of a std::tuple with six elements
 *
 * @param tpl The tuple to be serialized
 */
template<typename archive, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
void serialize(archive &ar, std::tuple<T0, T1, T2, T3, T4, T5> &tpl, unsigned int) {
	using namespace boost;
	using boost::serialization::make_nvp;

	ar
	& make_nvp("tpl_0", std::get<0>(tpl))
	& make_nvp("tpl_1", std::get<1>(tpl))
	& make_nvp("tpl_2", std::get<2>(tpl))
	& make_nvp("tpl_3", std::get<3>(tpl))
	& make_nvp("tpl_4", std::get<4>(tpl))
	& make_nvp("tpl_5", std::get<5>(tpl));
}

/******************************************************************************/

} /* namespace serialization */
} /* namespace boost */
