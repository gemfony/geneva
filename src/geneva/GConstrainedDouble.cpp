/**
 * @file GConstrainedDouble.cpp
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

#include "geneva/GConstrainedDouble.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GConstrainedDouble)

namespace Gem {
namespace Geneva {

/*******************************************************************************************/
/**
 * The default constructor
 */
GConstrainedDouble::GConstrainedDouble()
	: GConstrainedFPT<double>()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization with boundaries only. The value is set randomly.
 *
 * @param lowerBoundary The lower boundary of the value range
 * @param upperBoundary The upper boundary of the value range
 */
GConstrainedDouble::GConstrainedDouble(const double& lowerBoundary, const double& upperBoundary)
	: GConstrainedFPT<double>(lowerBoundary, upperBoundary)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization with value and boundaries
 *
 * @param val Initialization value
 * @param lowerBoundary The lower boundary of the value range
 * @param upperBoundary The upper boundary of the value range
 */
GConstrainedDouble::GConstrainedDouble (
		  const double& val
		, const double& lowerBoundary
		, const double& upperBoundary
)
	: GConstrainedFPT<double>(val, lowerBoundary, upperBoundary)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GConstrainedDouble object
 */
GConstrainedDouble::GConstrainedDouble(const GConstrainedDouble& cp)
	: GConstrainedFPT<double>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GConstrainedDouble::GConstrainedDouble(const double& val)
	: GConstrainedFPT<double>(val)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GConstrainedDouble::~GConstrainedDouble()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * An assignment operator for the contained value type
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
double GConstrainedDouble::operator=(const double& val) {
	return GConstrainedFPT<double>::operator=(val);
}

/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GConstrainedDouble object
 * @return A constant reference to this object
 */
const GConstrainedDouble& GConstrainedDouble::operator=(const GConstrainedDouble& cp){
	GConstrainedDouble::load_(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GConstrainedDouble::clone_() const {
	return new GConstrainedDouble(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GConstrainedDouble object
 *
 * @param  cp A constant reference to another GConstrainedDouble object
 * @return A boolean indicating whether both objects are equal
 */
bool GConstrainedDouble::operator==(const GConstrainedDouble& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to true)
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GConstrainedDouble::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GConstrainedDouble object
 *
 * @param  cp A constant reference to another GConstrainedDouble object
 * @return A boolean indicating whether both objects are inequal
 */
bool GConstrainedDouble::operator!=(const GConstrainedDouble& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality fulfilled, if no error text was emitted (which converts to true)
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GConstrainedDouble::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GConstrainedDouble::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

	// Check for a possible self-assignment
    GObject::selfAssignmentCheck<GConstrainedDouble>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GConstrainedFPT<double>::checkRelationshipWith(cp, e, limit, "GConstrainedDouble", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GConstrainedDouble", caller, deviations, e);
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GConstrainedDouble object, camouflaged as a GObject
 */
void GConstrainedDouble::load_(const GObject* cp){
	// Check for a possible self-assignment
    GObject::selfAssignmentCheck<GConstrainedDouble>(cp);

	// Load our parent class'es data ...
	GConstrainedFPT<double>::load_(cp);

	// ... no local data
}

/*******************************************************************************************/
/**
 * Triggers random initialization of the parameter object
 */
void GConstrainedDouble::randomInit_() {
	GConstrainedFPT<double>::randomInit_();
}


#ifdef GENEVATESTING

/*******************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GConstrainedDouble::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GConstrainedFPT<double>::modify_GUnitTests()) result = true;

	return result;
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GConstrainedDouble::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GConstrainedFPT<double>::specificTestsNoFailureExpected_GUnitTests();
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GConstrainedDouble::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GConstrainedFPT<double>::specificTestsFailuresExpected_GUnitTests();
}

/*******************************************************************************************/

#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
