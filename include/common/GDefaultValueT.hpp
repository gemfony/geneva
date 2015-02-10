/**
 * @file GDefaultValueT.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <string>

// Boost headers go here


#ifndef GDEFAULTVALUE_HPP_
#define GDEFAULTVALUE_HPP_

// Geneva headers go here

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * This function allows to specify default values for specific types
 * through specializations.
 */
template <typename T>
struct G_API GDefaultValueT {
   static T value() {
      return T(0);
   }
};

/******************************************************************************/
/**
 * Specialization for T == bool
 */
template <>
struct G_API GDefaultValueT<bool> {
   static bool value() {
      return true;
   }
};

/******************************************************************************/
/**
 * Specialization for T == std::string
 */
template <>
struct G_API GDefaultValueT<std::string> {
   static std::string value() {
      return std::string();
   }
};
/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GDEFAULTVALUE_HPP_ */
