/**
 * @file GSerializeTupleT.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

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
