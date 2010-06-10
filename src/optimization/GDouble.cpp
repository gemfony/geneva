/**
 * @file GDouble.cpp
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

#include "GDouble.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GDouble)

namespace Gem {
namespace GenEvA {

/*******************************************************************************************/
/**
 * The default constructor
 */
GDouble::GDouble()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GDouble object
 */
GDouble::GDouble(const GDouble& cp)
	: GNumT<double>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization by contained value
 *
 * @param val A value used for the initialization
 */
GDouble::GDouble(const double& val)
	: GNumT<double>(val)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization by random number in a given range
 *
 * @param lowerBoundary The lower boundary for the random number used in the initialization
 * @param upperBoundary The upper boundary for the random number used in the initialization
 */
GDouble::GDouble(const double& lowerBoundary, const double& upperBoundary)
	: GNumT<double>(lowerBoundary, upperBoundary)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GDouble::~GDouble()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * An assignment operator
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
double GDouble::operator=(const double& val) {
	return GNumT<double>::operator=(val);
}

/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GDouble object
 * @return A constant reference to this object
 */
const GDouble& GDouble::operator=(const GDouble& cp){
	GDouble::load_(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GDouble::clone_() const {
	return new GDouble(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GDouble object
 *
 * @param  cp A constant reference to another GDouble object
 * @return A boolean indicating whether both objects are equal
 */
bool GDouble::operator==(const GDouble& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GDouble::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GDouble object
 *
 * @param  cp A constant reference to another GDouble object
 * @return A boolean indicating whether both objects are inequal
 */
bool GDouble::operator!=(const GDouble& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GDouble::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GDouble::checkRelationshipWith(const GObject& cp,
		const Gem::Util::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Util;
    using namespace Gem::Util::POD;

    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GDouble>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GNumT<double>::checkRelationshipWith(cp, e, limit, "GDouble", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GDouble", caller, deviations, e);
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GDouble object, camouflaged as a GObject
 */
void GDouble::load_(const GObject* cp){
    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GDouble>(cp);

	// Load our parent class'es data ...
	GNumT<double>::load_(cp);

	// ... no local data
}

/*******************************************************************************************/
/**
 * Triggers random initialization of the parameter.
 */
void GDouble::randomInit_() {
	double lowerBoundary = getLowerInitBoundary();
	double upperBoundary = getUpperInitBoundary();

	Gem::Util::GRandom gr(Gem::Util::RNRLOCAL);

	setValue(gr.evenRandom(lowerBoundary, upperBoundary));
}

#ifdef GENEVATESTING
/*******************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GDouble::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GNumT<double>::modify_GUnitTests()) result = true;

	return result;
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GDouble::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GNumT<double>::specificTestsNoFailureExpected_GUnitTests();
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GDouble::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GNumT<double>::specificTestsFailuresExpected_GUnitTests();
}

/*******************************************************************************************/

#endif /* GENEVATESTING */

} /* namespace GenEvA */
} /* namespace Gem */
