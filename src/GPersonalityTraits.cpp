/**
 * @file GPersonalityTraits.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "GPersonalityTraits.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GPersonalityTraits)

namespace Gem {
namespace GenEvA {

/*****************************************************************************/
/**
 * The default constructor
 */
GPersonalityTraits::GPersonalityTraits()
{ /* nothing */ }

/*****************************************************************************/
/**
 * The copy constructor
 */
GPersonalityTraits::GPersonalityTraits(const GPersonalityTraits& cp)
{ /* nothing */ }

/*****************************************************************************/
/**
 * The standard destructor. No local, dynamically allocated data,
 * hence it does nothing.
 */
GPersonalityTraits::~GPersonalityTraits()
{ /* nothing */ }

/*****************************************************************************/
/**
 * Checks for equality with another GPersonalityTraits object
 *
 * @param  cp A constant reference to another GPersonalityTraits object
 * @return A boolean indicating whether both objects are equal
 */
bool GPersonalityTraits::operator==(const GPersonalityTraits& cp) const {
	return GPersonalityTraits::isEqualTo(cp, boost::logic::indeterminate);
}

/*****************************************************************************/
/**
 * Checks for inequality with another GPersonalityTraits object
 *
 * @param  cp A constant reference to another GPersonalityTraits object
 * @return A boolean indicating whether both objects are inequal
 */
bool GPersonalityTraits::operator!=(const GPersonalityTraits& cp) const {
	return !GPersonalityTraits::isEqualTo(cp, boost::logic::indeterminate);
}

/*****************************************************************************/
/**
 * Checks for equality with another GPersonalityTraits object.
 *
 * @param  cp A constant reference to another GPersonalityTraits object
 * @return A boolean indicating whether both objects are equal
 */
bool GPersonalityTraits::isEqualTo(const GObject& cp, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GPersonalityTraits reference
	const GPersonalityTraits *p_load = GObject::conversion_cast(&cp,  this);

	// Check for equality of our parent class
	if(!GObject::isEqualTo(*p_load, expected)) return  false;

	// No local data

	return true;
}

/*****************************************************************************/
/**
 * Checks for similarity with another GPersonalityTraits object.
 *
 * @param  cp A constant reference to another GPersonalityTraits object
 * @param limit A double value specifying the acceptable level of differences of floating point values
 * @return A boolean indicating whether both objects are similar to each other
 */
bool GPersonalityTraits::isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GPersonalityTraits reference
	const GPersonalityTraits *p_load = GObject::conversion_cast(&cp,  this);

	// Check for equality of our parent class
	if(!GObject::isSimilarTo(*p_load, limit, expected)) return false;

	// No local data

	return true;
}

/*****************************************************************************/
/**
 * Loads the data of another GPersonalityTraits object
 *
 * @param cp A copy of another GPersonalityTraits object, camouflaged as a GObject
 */
void GPersonalityTraits::load(const GObject *cp) {
	const GPersonalityTraits *p_load = this->conversion_cast(cp, this);

	// Load the parent class'es data
	GObject::load(cp);

	// No local data
}

/*****************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
