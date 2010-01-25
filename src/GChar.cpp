/**
 * @file GChar.cpp
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

#include "GChar.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GChar)

namespace Gem {
namespace GenEvA {

/*******************************************************************************************/
/**
 * The default constructor
 */
GChar::GChar()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GChar object
 */
GChar::GChar(const GChar& cp)
	: GParameterT<char>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GChar::GChar(const char& val)
	: GParameterT<char>(val)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GChar::~GChar()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * An assignment operator for the contained value type
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
const char& GChar::operator=(const char& val) {
	return GParameterT<char>::operator=(val);
}


/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GChar object
 * @return A constant reference to this object
 */
const GChar& GChar::operator=(const GChar& cp){
	GChar::load(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GChar::clone_() const {
	return new GChar(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GChar object
 *
 * @param  cp A constant reference to another GChar object
 * @return A boolean indicating whether both objects are equal
 */
bool GChar::operator==(const GChar& cp) const {
	return GChar::isEqualTo(cp, boost::logic::indeterminate);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GChar object
 *
 * @param  cp A constant reference to another GChar object
 * @return A boolean indicating whether both objects are inequal
 */
bool GChar::operator!=(const GChar& cp) const {
	return !GChar::isEqualTo(cp, boost::logic::indeterminate);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GChar object.  If T is an object type,
 * then it must implement operator!= .
 *
 * @param  cp A constant reference to another GChar object
 * @return A boolean indicating whether both objects are equal
 */
bool GChar::isEqualTo(const GObject& cp, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterT reference
	const GChar *p_load = GObject::conversion_cast(&cp,  this);

	// Check equality of the parent class
	if(!GParameterT<char>::isEqualTo(*p_load, expected)) return false;

	// No local data

	return true;
}

/*******************************************************************************************/
/**
 * Checks for similarity with another GChar object.
 *
 * @param  cp A constant reference to another GChar object
 * @param limit A double value specifying the acceptable level of differences of floating point values
 * @return A boolean indicating whether both objects are similar to each other
 */
bool GChar::isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterT reference
	const GChar *p_load = GObject::conversion_cast(&cp,  this);

	// Check similarity of the parent class
	if(!GParameterT<char>::isSimilarTo(*p_load, limit, expected)) return false;

	// No local data

	return true;
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GChar object, camouflaged as a GObject
 */
void GChar::load(const GObject* cp){
	// Convert cp into local format (also checks for the type of cp)
	const GChar *p_load = GObject::conversion_cast(cp, this);

	// Load our parent class'es data ...
	GParameterT<char>::load(cp);

	// ... no local data
}

/*******************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
