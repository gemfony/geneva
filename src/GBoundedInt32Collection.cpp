/**
 * @file GBoundedInt32Collection.cpp
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

#include "GBoundedInt32Collection.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GBoundedInt32Collection)


namespace Gem {
namespace GenEvA {

/*******************************************************************************************/
/**
 * The default constructor
 */
GBoundedInt32Collection::GBoundedInt32Collection()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GBoundedInt32Collection object
 */
GBoundedInt32Collection::GBoundedInt32Collection(const GBoundedInt32Collection& cp)
	: GParameterTCollectionT<GBoundedInt32>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GBoundedInt32Collection::~GBoundedInt32Collection()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GBoundedInt32Collection object
 * @return A constant reference to this object
 */
const GBoundedInt32Collection& GBoundedInt32Collection::operator=(const GBoundedInt32Collection& cp){
	GBoundedInt32Collection::load(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GBoundedInt32Collection::clone() const {
	return new GBoundedInt32Collection(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GBoundedInt32Collection object
 *
 * @param  cp A constant reference to another GBoundedInt32Collection object
 * @return A boolean indicating whether both objects are equal
 */
bool GBoundedInt32Collection::operator==(const GBoundedInt32Collection& cp) const {
	return GBoundedInt32Collection::isEqualTo(cp, boost::logic::indeterminate);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GBoundedInt32Collection object
 *
 * @param  cp A constant reference to another GBoundedInt32Collection object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBoundedInt32Collection::operator!=(const GBoundedInt32Collection& cp) const {
	return !GBoundedInt32Collection::isEqualTo(cp, boost::logic::indeterminate);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GBoundedInt32Collection object.  If T is an object type,
 * then it must implement operator!= .
 *
 * @param  cp A constant reference to another GBoundedInt32Collection object
 * @return A boolean indicating whether both objects are equal
 */
bool GBoundedInt32Collection::isEqualTo(const GObject& cp, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterT reference
	const GBoundedInt32Collection *p_load = GObject::conversion_cast(&cp,  this);

	// Check equality of the parent class
	if(!GParameterTCollectionT<GBoundedInt32>::isEqualTo(*p_load, expected)) return false;

	// No local data

	return true;
}

/*******************************************************************************************/
/**
 * Checks for similarity with another GBoundedInt32Collection object.
 *
 * @param  cp A constant reference to another GBoundedInt32Collection object
 * @param limit A double value specifying the acceptable level of differences of floating point values
 * @return A boolean indicating whether both objects are similar to each other
 */
bool GBoundedInt32Collection::isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterT reference
	const GBoundedInt32Collection *p_load = GObject::conversion_cast(&cp,  this);

	// Check similarity of the parent class
	if(!GParameterTCollectionT<GBoundedInt32>::isSimilarTo(*p_load, limit, expected)) return false;

	// No local data

	return true;
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GBoundedInt32Collection object, camouflaged as a GObject
 */
void GBoundedInt32Collection::load(const GObject* cp){
	// Convert cp into local format (also checks for the type of cp)
	const GBoundedInt32Collection *p_load = GObject::conversion_cast(cp, this);

	// Load our parent class'es data ...
	GParameterTCollectionT<GBoundedInt32>::load(cp);

	// ... no local data
}

/*******************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
