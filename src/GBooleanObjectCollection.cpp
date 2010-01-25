/**
 * @file GBooleanObjectCollection.cpp
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

#include "GBooleanObjectCollection.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GBooleanObjectCollection)

namespace Gem {
namespace GenEvA {

/*******************************************************************************************/
/**
 * The default constructor
 */
GBooleanObjectCollection::GBooleanObjectCollection()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GBooleanObjectCollection object
 */
GBooleanObjectCollection::GBooleanObjectCollection(const GBooleanObjectCollection& cp)
	: GParameterTCollectionT<GBoolean>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GBooleanObjectCollection::~GBooleanObjectCollection()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GBooleanObjectCollection object
 * @return A constant reference to this object
 */
const GBooleanObjectCollection& GBooleanObjectCollection::operator=(const GBooleanObjectCollection& cp){
	GBooleanObjectCollection::load(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GBooleanObjectCollection::clone_() const {
	return new GBooleanObjectCollection(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GBooleanObjectCollection object
 *
 * @param  cp A constant reference to another GBooleanObjectCollection object
 * @return A boolean indicating whether both objects are equal
 */
bool GBooleanObjectCollection::operator==(const GBooleanObjectCollection& cp) const {
	return GBooleanObjectCollection::isEqualTo(cp, boost::logic::indeterminate);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GBooleanObjectCollection object
 *
 * @param  cp A constant reference to another GBooleanObjectCollection object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBooleanObjectCollection::operator!=(const GBooleanObjectCollection& cp) const {
	return !GBooleanObjectCollection::isEqualTo(cp, boost::logic::indeterminate);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GBooleanObjectCollection object.  If T is an object type,
 * then it must implement operator!= .
 *
 * @param  cp A constant reference to another GBooleanObjectCollection object
 * @return A boolean indicating whether both objects are equal
 */
bool GBooleanObjectCollection::isEqualTo(const GObject& cp, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterT reference
	const GBooleanObjectCollection *p_load = GObject::conversion_cast(&cp,  this);

	// Check equality of the parent class
	if(!GParameterTCollectionT<GBoolean>::isEqualTo(*p_load, expected)) return false;

	// No local data

	return true;
}

/*******************************************************************************************/
/**
 * Checks for similarity with another GBooleanObjectCollection object.
 *
 * @param  cp A constant reference to another GBooleanObjectCollection object
 * @param limit A double value specifying the acceptable level of differences of floating point values
 * @return A boolean indicating whether both objects are similar to each other
 */
bool GBooleanObjectCollection::isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterT reference
	const GBooleanObjectCollection *p_load = GObject::conversion_cast(&cp,  this);

	// Check similarity of the parent class
	if(!GParameterTCollectionT<GBoolean>::isSimilarTo(*p_load, limit, expected)) return false;

	// No local data

	return true;
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GBooleanObjectCollection object, camouflaged as a GObject
 */
void GBooleanObjectCollection::load(const GObject* cp){
	// Convert cp into local format (also checks for the type of cp)
	const GBooleanObjectCollection *p_load = GObject::conversion_cast(cp, this);

	// Load our parent class'es data ...
	GParameterTCollectionT<GBoolean>::load(cp);

	// ... no local data
}

/*******************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
