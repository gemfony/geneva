/**
 * @file GConstrainedDoubleObjectCollection.cpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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

#include "geneva/GConstrainedDoubleObjectCollection.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GConstrainedDoubleObjectCollection)

namespace Gem {
namespace Geneva {

/*******************************************************************************************/
/**
 * The default constructor
 */
GConstrainedDoubleObjectCollection::GConstrainedDoubleObjectCollection()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GConstrainedDoubleObjectCollection object
 */
GConstrainedDoubleObjectCollection::GConstrainedDoubleObjectCollection(const GConstrainedDoubleObjectCollection& cp)
	: GParameterTCollectionT<GConstrainedDoubleObject>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GConstrainedDoubleObjectCollection::~GConstrainedDoubleObjectCollection()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GConstrainedDoubleObjectCollection object
 * @return A constant reference to this object
 */
const GConstrainedDoubleObjectCollection& GConstrainedDoubleObjectCollection::operator=(const GConstrainedDoubleObjectCollection& cp){
	GConstrainedDoubleObjectCollection::load_(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GConstrainedDoubleObjectCollection::clone_() const {
	return new GConstrainedDoubleObjectCollection(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GConstrainedDoubleObjectCollection object
 *
 * @param  cp A constant reference to another GConstrainedDoubleObjectCollection object
 * @return A boolean indicating whether both objects are equal
 */
bool GConstrainedDoubleObjectCollection::operator==(const GConstrainedDoubleObjectCollection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GConstrainedDoubleObjectCollection::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GConstrainedDoubleObjectCollection object
 *
 * @param  cp A constant reference to another GConstrainedDoubleObjectCollection object
 * @return A boolean indicating whether both objects are inequal
 */
bool GConstrainedDoubleObjectCollection::operator!=(const GConstrainedDoubleObjectCollection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GConstrainedDoubleObjectCollection::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GConstrainedDoubleObjectCollection::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

	// Check for a possible self-assignment
    GObject::selfAssignmentCheck<GConstrainedDoubleObjectCollection>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GParameterTCollectionT<GConstrainedDoubleObject>::checkRelationshipWith(cp, e, limit, "GConstrainedDoubleObjectCollection", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GConstrainedDoubleObjectCollection", caller, deviations, e);
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GConstrainedDoubleObjectCollection object, camouflaged as a GObject
 */
void GConstrainedDoubleObjectCollection::load_(const GObject* cp){
	// Check for a possible self-assignment
    GObject::selfAssignmentCheck<GConstrainedDoubleObjectCollection>(cp);

	// Load our parent class'es data ...
	GParameterTCollectionT<GConstrainedDoubleObject>::load_(cp);

	// ... no local data
}

#ifdef GENEVATESTING

/*******************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GConstrainedDoubleObjectCollection::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GParameterTCollectionT<GConstrainedDoubleObject>::modify_GUnitTests()) result = true;

	return result;
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GConstrainedDoubleObjectCollection::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GParameterTCollectionT<GConstrainedDoubleObject>::specificTestsNoFailureExpected_GUnitTests();
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GConstrainedDoubleObjectCollection::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GParameterTCollectionT<GConstrainedDoubleObject>::specificTestsFailuresExpected_GUnitTests();
}

/*******************************************************************************************/

#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
