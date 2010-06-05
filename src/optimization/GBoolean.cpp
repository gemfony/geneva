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
 * An assignment operator
 *
 * @param val The value to be assigned to this object
 * @return The value that was just assigned to this object
 */
bool GBoolean::operator=(const bool& val) {
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
	GBoolean::load_(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GBoolean::clone_() const {
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
	using namespace Gem::Util;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBoolean::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GBoolean object
 *
 * @param  cp A constant reference to another GBoolean object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBoolean::operator!=(const GBoolean& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBoolean::operator!=","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Random initialization. This is a helper function, without it we'd
 * have to say things like "myGBooleanObject.GParameterBase::randomInit();".
 */
void GBoolean::randomInit() {
	  GParameterBase::randomInit();
}

/*******************************************************************************************/
/**
 * Triggers random initialization of the parameter object, with a given likelihood structure
 */
void GBoolean::randomInit(const double& probability) {
  if(!GParameterBase::initializationBlocked()) randomInit_(probability);
}

/*******************************************************************************************/
/**
 * Triggers random initialization of the parameter object, with a given likelihood structure.
 * This function holds the actual initialization logic, used in the publicly accessible
 * GBoolean::randomInit(const double& probability) function.
 */
void GBoolean::randomInit_(const double& probability) {
  Gem::Util::GRandom gr(Gem::Util::RNRLOCAL);
  this->setValue(gr.boolRandom(probability));
}

/*******************************************************************************************/
/**
 * Triggers random initialization of the parameter object
 */
void GBoolean::randomInit_() {
	Gem::Util::GRandom gr(Gem::Util::RNRLOCAL);
	this->setValue(gr.boolRandom());
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
boost::optional<std::string> GBoolean::checkRelationshipWith(const GObject& cp,
		const Gem::Util::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Util;
    using namespace Gem::Util::POD;

	// Check for a possible self-assignment
	GObject::selfAssignmentCheck<GBoolean>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GParameterT<bool>::checkRelationshipWith(cp, e, limit, "GBoolean", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GBoolean", caller, deviations, e);
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GBoolean object, camouflaged as a GObject
 */
void GBoolean::load_(const GObject* cp){
	// Check for a possible self-assignment
	GObject::selfAssignmentCheck<GBoolean>(cp);

	// Load our parent class'es data ...
	GParameterT<bool>::load_(cp);

	// ... no local data
}

/*******************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBoolean::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GParameterT<bool>::modify_GUnitTests()) result = true;

	return result;
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBoolean::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GParameterT<bool>::specificTestsNoFailureExpected_GUnitTests();
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBoolean::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GParameterT<bool>::specificTestsFailuresExpected_GUnitTests();
}

/*******************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
