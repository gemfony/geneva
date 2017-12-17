/**
 * @file GTypeTraitsT.hpp
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

#ifndef INCLUDE_COMMON_GTYPETRAITST_HPP_
#define INCLUDE_COMMON_GTYPETRAITST_HPP_

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <vector>
#include <deque>
#include <sstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <typeinfo>

// Boost headers go here
#include <boost/cast.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>

// Geneva headers go here

namespace Gem {
namespace Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
// This simple class is used to simplify detection of classes that have the common
// interface of Gemfony libraries.
class gemfony_common_interface_indicator {};

/**
 * A type trait helping to check whether a class has the Gemfony Scientific library
 * interface. The simple convention is that the base class of a hierarchy must
 * (we recommend) privately inherit from common_gemfony_iterface .
 */
template<typename T>
struct has_gemfony_common_interface {
	enum {
		value = std::is_base_of<gemfony_common_interface_indicator, T>::value
	};
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A type trait helping to check whether a class has a compare function.
 */
template<typename T>
class has_compare_member {
	using yes = char;
	using no = long;

	template<typename C>
	static yes test(decltype(&C::compare));

	template<typename C>
	static no test(...);

public:
	enum {
		value = sizeof(test<T>(0)) == sizeof(char)
	};
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A type trait helping to check whether a class has a clone function.
 */
template<typename T>
class has_clone_member {
	using yes = char;
	using no = long;

	template<typename C>
	static yes test(decltype(&C::clone));

	template<typename C>
	static no test(...);

public:
	enum {
		value = sizeof(test<T>(0)) == sizeof(char)
	};
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A type trait helping to check whether a class has a load function.
 */
template<typename T>
class has_load_member {
	using yes = char;
 	using no = long;

	template<typename C>
	static yes test(decltype(&C::load));

	template<typename C>
	static no test(...);

public:
	enum {
		value = sizeof(test<T>(0)) == sizeof(char)
	};
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* INCLUDE_COMMON_GTYPETRAITST_HPP_ */
