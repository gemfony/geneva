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
	GBoundedInt32::load_(&cp);
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
	using namespace Gem::Util;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to true)
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBoundedInt32::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GBoundedInt32 object
 *
 * @param  cp A constant reference to another GBoundedInt32 object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBoundedInt32::operator!=(const GBoundedInt32& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of inequality fulfilled, if no error text was emitted (which converts to true)
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBoundedInt32::operator!=","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks whether a given expectation for the relationship between this object and another object
 * is fulfilled.
 *
 * @param cp A constant reference to another object, camouflaged as a GObject
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 * @param caller An identifier for the calling entity
 * @param y_name An identifier for the object that should be compared to this one
 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
 */
boost::optional<std::string> GBoundedInt32::checkRelationshipWith(const GObject& cp,
		const Gem::Util::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Util;
    using namespace Gem::Util::POD;

	// Check for a possible self-assignment
    GObject::selfAssignmentCheck<GBoundedInt32>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GBoundedNumT<boost::int32_t>::checkRelationshipWith(cp, e, limit, "GBoundedInt32", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GBoundedInt32", caller, deviations, e);
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GBoundedInt32 object, camouflaged as a GObject
 */
void GBoundedInt32::load_(const GObject* cp){
	// Check for a possible self-assignment
    GObject::selfAssignmentCheck<GBoundedInt32>(cp);

	// Load our parent class'es data ...
	GBoundedNumT<boost::int32_t>::load_(cp);

	// ... no local data
}

/*******************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
