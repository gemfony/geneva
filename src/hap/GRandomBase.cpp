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
 * The standard constructor
 */
GRandomBase::GRandomBase()
	: min_value(GRandomBase::result_type(0.)), max_value(GRandomBase::result_type(1.)), fltGaussCache_(float(0.)),
	  dblGaussCache_(double(0.)), fltGaussCacheAvailable_(false), dblGaussCacheAvailable_(false) { /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GRandomBase::~GRandomBase() { /* nothing */ }

/******************************************************************************/
/**
 * Retrieves an uniform_01 item. Thus function, together with the min() and
 * max() functions make it possible to use GRandomBase as a generator for
 * boost's random distributions.
 *
 * @return A random number taken from the evenRandom function
 */
GRandomBase::result_type GRandomBase::operator()() {
	return this->uniform_01<GRandomBase::result_type>();
}

/******************************************************************************/
/**
 * Returns the minimum value returned by evenRandom(). This
 * makes it possible to use GRandomBase as a generator for Boost's random
 * distributions. The parantheses prevent Windows min-Macro from being
 * substituted. Using BOOST_PREVENT_MACRO_SUBSTITUTION would be another possiblity.
 *
 * @return The minimum value returned by evenRandom()
 */
GRandomBase::result_type (GRandomBase::min)() const {
	return min_value;
}

/******************************************************************************/
/**
 * Returns the maximum value returned by evenRandom(). This
 * makes it possible to use GRandomBase as a generator for Boost's random
 * distributions. The parantheses prevent Windows min-Macro from being
 * substituted. Using BOOST_PREVENT_MACRO_SUBSTITUTION would be another possiblity.
 *
 * @return The maximum value returned by evenRandom()
 */
GRandomBase::result_type (GRandomBase::max)() const {
	return max_value;
}

/******************************************************************************/
/**
 * This function returns true with a probability "probability", otherwise false.
 *
 * @param p The probability for the value "true" to be returned
 * @return A boolean value, which will be true with a user-defined likelihood
 */
bool GRandomBase::weighted_bool(const double &probability) {
#ifdef DEBUG
	assert(probability>=0. && probability<=1.);
#endif
	return uniform_01<double>() < probability;
}

/******************************************************************************/
/**
 * This function produces boolean values with a 50% likelihood each for
 * true and false.
 *
 * @return Boolean values with a 50% likelihood for true/false respectively
 */
bool GRandomBase::uniform_bool() {
	return this->weighted_bool(0.5);
}

/******************************************************************************/

} /* namespace Hap */
} /* namespace Gem */
