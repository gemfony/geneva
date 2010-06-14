/**
 * @file GBoundedDouble.cpp
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

#include "GBoundedDouble.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GBoundedDouble)

namespace Gem {
namespace GenEvA {

/*******************************************************************************************/
/**
 * The default constructor
 */
GBoundedDouble::GBoundedDouble()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization with boundaries only. The value is set randomly.
 *
 * @param lowerBoundary The lower boundary of the value range
 * @param upperBoundary The upper boundary of the value range
 */
GBoundedDouble::GBoundedDouble(const double& lowerBoundary, const double& upperBoundary)
	: GBoundedNumT<double>(lowerBoundary, upperBoundary)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization with value and boundaries
 *
 * @param val Initialization value
 * @param lowerBoundary The lower boundary of the value range
 * @param upperBoundary The upper boundary of the value range
 */
GBoundedDouble::GBoundedDouble(const double& val,
		                       const double& lowerBoundary,
		                       const double& upperBoundary)
	: GBoundedNumT<double>(val, lowerBoundary, upperBoundary)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GBoundedDouble object
 */
GBoundedDouble::GBoundedDouble(const GBoundedDouble& cp)
	: GBoundedNumT<double>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GBoundedDouble::GBoundedDouble(const double& val)
	: GBoundedNumT<double>(val)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GBoundedDouble::~GBoundedDouble()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * An assignment operator for the contained value type
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
double GBoundedDouble::operator=(const double& val) {
	return GBoundedNumT<double>::operator=(val);
}

/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GBoundedDouble object
 * @return A constant reference to this object
 */
const GBoundedDouble& GBoundedDouble::operator=(const GBoundedDouble& cp){
	GBoundedDouble::load_(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GBoundedDouble::clone_() const {
	return new GBoundedDouble(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GBoundedDouble object
 *
 * @param  cp A constant reference to another GBoundedDouble object
 * @return A boolean indicating whether both objects are equal
 */
bool GBoundedDouble::operator==(const GBoundedDouble& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBoundedDouble::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GBoundedDouble object
 *
 * @param  cp A constant reference to another GBoundedDouble object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBoundedDouble::operator!=(const GBoundedDouble& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBoundedDouble::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GBoundedDouble::checkRelationshipWith(const GObject& cp,
		const Gem::Util::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Util;
    using namespace Gem::Util::POD;

	// Check for a possible self-assignment
    GObject::selfAssignmentCheck<GBoundedDouble>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GBoundedNumT<double>::checkRelationshipWith(cp, e, limit, "GBoundedDouble", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GBoundedDouble", caller, deviations, e);
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GBoundedDouble object, camouflaged as a GObject
 */
void GBoundedDouble::load_(const GObject* cp){
	// Check for a possible self-assignment
    GObject::selfAssignmentCheck<GBoundedDouble>(cp);

	// Load our parent class'es data ...
	GBoundedNumT<double>::load_(cp);

	// ... no local data
}

/*******************************************************************************************/
/**
 * Triggers random initialization of the parameter object
 */
void GBoundedDouble::randomInit_() {
	Gem::Util::GRandom gr(Gem::Util::RNRLOCAL);
	this->setExternalValue(gr.evenRandom(getLowerBoundary(), getUpperBoundary()));
}

/*******************************************************************************************/
/**
 * Initializes double-based parameters with a given value.
 *
 * @param val The value to use for the initialization
 */
void GBoundedDouble::fixedValueInit_(const double& val) {
	this->setExternalValue(val);
}


#ifdef GENEVATESTING

/*******************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBoundedDouble::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GBoundedNumT<double>::modify_GUnitTests()) result = true;

	return result;
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBoundedDouble::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GBoundedNumT<double>::specificTestsNoFailureExpected_GUnitTests();
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBoundedDouble::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GBoundedNumT<double>::specificTestsFailuresExpected_GUnitTests();
}

/*******************************************************************************************/

#endif /* GENEVATESTING */

} /* namespace GenEvA */
} /* namespace Gem */
