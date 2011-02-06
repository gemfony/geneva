/**
 * @file GConstrainedDoubleCollection.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt) and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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

#include "geneva/GConstrainedDoubleCollection.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GConstrainedDoubleCollection)

namespace Gem {
namespace Geneva {

/*******************************************************************************************/
/**
 * The default constructor.
 */
GConstrainedDoubleCollection::GConstrainedDoubleCollection()
	: GConstrainedFPNumCollectionT<double> ()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialize the lower and upper boundaries for data members of this class
 *
 * @param lowerBoundary The lower boundary for data members
 * @param upperBoundary The upper boundary for data members
 */
GConstrainedDoubleCollection::GConstrainedDoubleCollection (
		const double& lowerBoundary
		, const double& upperBoundary
)
	: GConstrainedFPNumCollectionT<double> (lowerBoundary, upperBoundary)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The standard copy constructor
 *
 * @param cp A copy of another GConstrainedDoubleCollection object
 */
GConstrainedDoubleCollection::GConstrainedDoubleCollection(const GConstrainedDoubleCollection& cp)
	: GConstrainedFPNumCollectionT<double> (cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The standard destructor
 */
GConstrainedDoubleCollection::~GConstrainedDoubleCollection()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The standard assignment operator.
 *
 * @param cp A copy of another GDoubleCollection object
 * @return A constant reference to this object
 */
const GConstrainedDoubleCollection& GConstrainedDoubleCollection::operator=(const GConstrainedDoubleCollection& cp){
	GConstrainedDoubleCollection::load_(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Checks for equality with another GConstrainedDoubleCollection object
 *
 * @param  cp A constant reference to another GConstrainedDoubleCollection object
 * @return A boolean indicating whether both objects are equal
 */
bool GConstrainedDoubleCollection::operator==(const GConstrainedDoubleCollection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GConstrainedDoubleCollection::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GConstrainedDoubleCollection object
 *
 * @param  cp A constant reference to another GConstrainedDoubleCollection object
 * @return A boolean indicating whether both objects are inequal
 */
bool GConstrainedDoubleCollection::operator!=(const GConstrainedDoubleCollection& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GConstrainedDoubleCollection::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GConstrainedDoubleCollection::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const	{
	using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GConstrainedDoubleCollection  *p_load = GObject::conversion_cast<GConstrainedDoubleCollection>(&cp);

	// Will hold possible deviations from the expectation, including explanations
	std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GConstrainedFPNumCollectionT<double>::checkRelationshipWith(cp, e, limit, "GConstrainedDoubleCollection", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GConstrainedDoubleCollection", caller, deviations, e);
}

/*******************************************************************************************/
/**
 * Loads the data of another GConstrainedDoubleCollection object,
 * camouflaged as a GObject. We have no local data, so
 * all we need to do is to the standard identity check,
 * preventing that an object is assigned to itself.
 *
 * @param cp A copy of another GConstrainedDoubleCollection object, camouflaged as a GObject
 */
void GConstrainedDoubleCollection::load_(const GObject *cp){
	// Convert cp into local format
	const GConstrainedDoubleCollection *p_load = GObject::conversion_cast<GConstrainedDoubleCollection>(cp);

	// Load our parent class'es data ...
	GConstrainedFPNumCollectionT<double>::load_(cp);

	// no local data ...
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GConstrainedDoubleCollection::clone_() const {
	return new GConstrainedDoubleCollection(*this);
}

#ifdef GENEVATESTING
/*******************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GConstrainedDoubleCollection::modify_GUnitTests() {
	bool result = false;

	// Call the parent classes' functions
	if(GConstrainedFPNumCollectionT<double>::modify_GUnitTests()) result = true;

	return result;
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GConstrainedDoubleCollection::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent classes' functions
	GConstrainedFPNumCollectionT<double>::specificTestsNoFailureExpected_GUnitTests();
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GConstrainedDoubleCollection::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent classes' functions
	GConstrainedFPNumCollectionT<double>::specificTestsFailuresExpected_GUnitTests();
}

/*******************************************************************************************/

#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
