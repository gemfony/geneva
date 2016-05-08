/**
 * @file GRandomBase.cpp
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

#include "hap/GRandomBase.hpp"

namespace Gem {
namespace Hap {

/******************************************************************************/
/**
 * The standard constructor. Note that the seed "val" might just be ignored,
 * if random numbers are obtained from the global factory.
 */
GRandomBase::GRandomBase()
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GRandomBase::~GRandomBase()
{ /* nothing */ }

/******************************************************************************/
/**
 * Retrieves a raw random item. This function, together with the min() and
 * max() functions make it possible to use GRandomBase as a generator for
 * boost's random distributions.
 *
 * @return A "raw" random number suitable for a C++11 standard random engine
 */
GRandomBase::result_type GRandomBase::operator()() {
	return this->int_random();
}

/******************************************************************************/

} /* namespace Hap */
} /* namespace Gem */
