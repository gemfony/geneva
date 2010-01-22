/**
 * @file GBoolean.cpp
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

#include "GBoolean.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GBoolean)

namespace Gem {
namespace GenEvA {

/*******************************************************************************************/
/**
 * The default constructor
 */
GBoolean::GBoolean()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GBoolean object
 */
GBoolean::GBoolean(const GBoolean& cp)
	: GParameterT<bool>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GBoolean::GBoolean(const bool& val)
	: GParameterT<bool>(val)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GBoolean::~GBoolean()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * An assignment operator for the contained value type
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
const bool& GBoolean::operator=(const bool& val) {
	return GParameterT<bool>::operator=(val);
}


/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GBoolean object
 * @return A constant reference to this object
 */
const GBoolean& GBoolean::operator=(const GBoolean& cp){
	GBoolean::load(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GBoolean::clone() const {
	return new GBoolean(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GBoolean object
 *
 * @param  cp A constant reference to another GBoolean object
 * @return A boolean indicating whether both objects are equal
 */
bool GBoolean::operator==(const GBoolean& cp) const {
	return GBoolean::isEqualTo(cp, boost::logic::indeterminate);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GBoolean object
 *
 * @param  cp A constant reference to another GBoolean object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBoolean::operator!=(const GBoolean& cp) const {
	return !GBoolean::isEqualTo(cp, boost::logic::indeterminate);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GBoolean object.  If T is an object type,
 * then it must implement operator!= .
 *
 * @param  cp A constant reference to another GBoolean object
 * @return A boolean indicating whether both objects are equal
 */
bool GBoolean::isEqualTo(const GObject& cp, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterT reference
	const GBoolean *p_load = GObject::conversion_cast(&cp,  this);

	// Check equality of the parent class
	if(!GParameterT<bool>::isEqualTo(*p_load, expected)) return false;

	// No local data

	return true;
}

/*******************************************************************************************/
/**
 * Checks for similarity with another GBoolean object.
 *
 * @param  cp A constant reference to another GBoolean object
 * @param limit A double value specifying the acceptable level of differences of floating point values
 * @return A boolean indicating whether both objects are similar to each other
 */
bool GBoolean::isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterT reference
	const GBoolean *p_load = GObject::conversion_cast(&cp,  this);

	// Check similarity of the parent class
	if(!GParameterT<bool>::isSimilarTo(*p_load, limit, expected)) return false;

	// No local data

	return true;
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GBoolean object, camouflaged as a GObject
 */
void GBoolean::load(const GObject* cp){
	// Convert cp into local format (also checks for the type of cp)
	const GBoolean *p_load = GObject::conversion_cast(cp, this);

	// Load our parent class'es data ...
	GParameterT<bool>::load(cp);

	// ... no local data
}

/*******************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
