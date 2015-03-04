/**
 * @file GHapEnums.hpp
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
#include <istream>
#include <ostream>

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/cast.hpp>

#ifndef GHAPENUMS_HPP_
#define GHAPENUMS_HPP_

// Hap headers go here

namespace Gem {
namespace Hap {

/******************************************************************************/
/**
 * Allowed specializations of Gem::Hap::GRandomT<T>
 */
enum G_API RANDFLAVOURS {
    RANDOMPROXY = 0 // random numbers are taken from the factory
  , RANDOMLOCAL = 1 // random numbers are produced locally, using a seed taken from the seed manager or provided to the constructor
};

/******************************************************************************/

/** @brief Puts a Gem::Hap::RANDFLAVOURS into a stream. Needed also for boost::lexical_cast<> */
G_API std::ostream& operator<<(std::ostream&, const Gem::Hap::RANDFLAVOURS&);
/** @brief Reads a Gem::Hap::RANDFLAVOURS item from a stream. Needed also for boost::lexical_cast<> */
G_API std::istream& operator>>(std::istream&, Gem::Hap::RANDFLAVOURS&);

/******************************************************************************/

} /* namespace Hap */
} /* namespace Gem */

#endif /* GHAPENUMS_HPP_ */
