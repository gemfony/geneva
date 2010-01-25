/**
 * @file GBoundedInt32.cpp
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

#include "GBoundedInt32.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GBoundedInt32)

namespace Gem {
namespace GenEvA {

/*******************************************************************************************/
/**
 * The default constructor
 */
GBoundedInt32::GBoundedInt32()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization with boundaries only. The value is set randomly.
 *
 * @param lowerBoundary The lower boundary of the value range
 * @param upperBoundary The upper boundary of the value range
 */
GBoundedInt32::GBoundedInt32(const boost::int32_t& lowerBoundary, const boost::int32_t& upperBoundary)
	: GBoundedNumT<boost::int32_t>(lowerBoundary, upperBoundary)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization with value and boundaries
 *
 * @param val Initialization value
 * @param lowerBoundary The lower boundary of the value range
 * @param upperBoundary The upper boundary of the value range
 */
GBoundedInt32::GBoundedInt32(const boost::int32_t& val,
		                       const boost::int32_t& lowerBoundary,
		                       const boost::int32_t& upperBoundary)
	: GBoundedNumT<boost::int32_t>(val, lowerBoundary, upperBoundary)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GBoundedInt32 object
 */
GBoundedInt32::GBoundedInt32(const GBoundedInt32& cp)
	: GBoundedNumT<boost::int32_t>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GBoundedInt32::GBoundedInt32(const boost::int32_t& val)
	: GBoundedNumT<boost::int32_t>(val)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GBoundedInt32::~GBoundedInt32()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * An assignment operator for the contained value type
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
const boost::int32_t& GBoundedInt32::operator=(const boost::int32_t& val) {
	return GBoundedNumT<boost::int32_t>::operator=(val);
}

/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GBoundedInt32 object
 * @return A constant reference to this object
 */
const GBoundedInt32& GBoundedInt32::operator=(const GBoundedInt32& cp){
	GBoundedInt32::load(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GBoundedInt32::clone_() const {
	return new GBoundedInt32(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GBoundedInt32 object
 *
 * @param  cp A constant reference to another GBoundedInt32 object
 * @return A boolean indicating whether both objects are equal
 */
bool GBoundedInt32::operator==(const GBoundedInt32& cp) const {
	return GBoundedInt32::isEqualTo(cp, boost::logic::indeterminate);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GBoundedInt32 object
 *
 * @param  cp A constant reference to another GBoundedInt32 object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBoundedInt32::operator!=(const GBoundedInt32& cp) const {
	return !GBoundedInt32::isEqualTo(cp, boost::logic::indeterminate);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GBoundedInt32 object.  If T is an object type,
 * then it must implement operator!= .
 *
 * @param  cp A constant reference to another GBoundedInt32 object
 * @return A boolean indicating whether both objects are equal
 */
bool GBoundedInt32::isEqualTo(const GObject& cp, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterT reference
	const GBoundedInt32 *p_load = GObject::conversion_cast(&cp,  this);

	// Check equality of the parent class
	if(!GBoundedNumT<boost::int32_t>::isEqualTo(*p_load, expected)) return false;

	// No local data

	return true;
}

/*******************************************************************************************/
/**
 * Checks for similarity with another GBoundedInt32 object.
 *
 * @param  cp A constant reference to another GBoundedInt32 object
 * @param limit A double value specifying the acceptable level of differences of floating point values
 * @return A boolean indicating whether both objects are similar to each other
 */
bool GBoundedInt32::isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterT reference
	const GBoundedInt32 *p_load = GObject::conversion_cast(&cp,  this);

	// Check similarity of the parent class
	if(!GBoundedNumT<boost::int32_t>::isSimilarTo(*p_load, limit, expected)) return false;

	// No local data

	return true;
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GBoundedInt32 object, camouflaged as a GObject
 */
void GBoundedInt32::load(const GObject* cp){
	// Convert cp into local format (also checks for the type of cp)
	const GBoundedInt32 *p_load = GObject::conversion_cast(cp, this);

	// Load our parent class'es data ...
	GBoundedNumT<boost::int32_t>::load(cp);

	// ... no local data
}

/*******************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
