/**
 * @file GPersonalityTraits.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "geneva/GPersonalityTraits.hpp"


/*********************************************************************************/

namespace Gem {
namespace Geneva {

/*****************************************************************************/
/**
 * The default constructor
 */
GPersonalityTraits::GPersonalityTraits()
{ /* nothing */ }

/*****************************************************************************/
/**
 * The copy constructor
 */
GPersonalityTraits::GPersonalityTraits(const GPersonalityTraits& cp)
{ /* nothing */ }

/*****************************************************************************/
/**
 * The standard destructor. No local, dynamically allocated data,
 * hence it does nothing.
 */
GPersonalityTraits::~GPersonalityTraits()
{ /* nothing */ }

/*****************************************************************************/
/**
 * Checks for equality with another GPersonalityTraits object
 *
 * @param  cp A constant reference to another GPersonalityTraits object
 * @return A boolean indicating whether both objects are equal
 */
bool GPersonalityTraits::operator==(const GPersonalityTraits& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GPersonalityTraits::operator==","cp", CE_SILENT);
}

/*****************************************************************************/
/**
 * Checks for inequality with another GPersonalityTraits object
 *
 * @param  cp A constant reference to another GPersonalityTraits object
 * @return A boolean indicating whether both objects are inequal
 */
bool GPersonalityTraits::operator!=(const GPersonalityTraits& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GPersonalityTraits::operator!=","cp", CE_SILENT);
}

/*****************************************************************************/
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
boost::optional<std::string> GPersonalityTraits::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

	// Check that we are not accidently assigning this object to itself
	GObject::selfAssignmentCheck<GPersonalityTraits>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GObject::checkRelationshipWith(cp, e, limit, "GPersonalityTraits", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GPersonalityTraits", caller, deviations, e);
}

/*****************************************************************************/
/**
 * Loads the data of another GPersonalityTraits object
 *
 * @param cp A copy of another GPersonalityTraits object, camouflaged as a GObject
 */
void GPersonalityTraits::load_(const GObject *cp) {
	// Check that we are not accidently assigning this object to itself
	GObject::selfAssignmentCheck<GPersonalityTraits>(cp);

	// Load the parent class'es data
	GObject::load_(cp);

	// No local data
}

#ifdef GENEVATESTING
/*****************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GPersonalityTraits::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GObject::modify_GUnitTests()) result = true;

	return result;
}

/*****************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GPersonalityTraits::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GObject::specificTestsNoFailureExpected_GUnitTests();

	// No local data -- nothing to test
}

/*****************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GPersonalityTraits::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GObject::specificTestsFailuresExpected_GUnitTests();

	// No local data -- nothing to test
}

/*****************************************************************************/
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
