/**
 * @file GDoubleBiGaussAdaptor.cpp
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

#include "geneva/GDoubleBiGaussAdaptor.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GDoubleBiGaussAdaptor)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GDoubleBiGaussAdaptor::GDoubleBiGaussAdaptor()
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GDoubleBiGaussAdaptor object
 */
GDoubleBiGaussAdaptor::GDoubleBiGaussAdaptor(const GDoubleBiGaussAdaptor& cp)
	: GFPBiGaussAdaptorT<double>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with a adaption probability
 *
 * @param adProb The adaption probability
 */
GDoubleBiGaussAdaptor::GDoubleBiGaussAdaptor(const double& adProb)
	: GFPBiGaussAdaptorT<double>(adProb)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GDoubleBiGaussAdaptor::~GDoubleBiGaussAdaptor()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GDoubleBiGaussAdaptor object
 * @return A constant reference to this object
 */
const GDoubleBiGaussAdaptor& GDoubleBiGaussAdaptor::operator=(const GDoubleBiGaussAdaptor& cp){
	GDoubleBiGaussAdaptor::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GDoubleBiGaussAdaptor::clone_() const {
	return new GDoubleBiGaussAdaptor(*this);
}

/******************************************************************************/
/**
 * Checks for equality with another GDoubleBiGaussAdaptor object
 *
 * @param  cp A constant reference to another GDoubleBiGaussAdaptor object
 * @return A boolean indicating whether both objects are equal
 */
bool GDoubleBiGaussAdaptor::operator==(const GDoubleBiGaussAdaptor& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GDoubleBiGaussAdaptor::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GDoubleBiGaussAdaptor object
 *
 * @param  cp A constant reference to another GDoubleBiGaussAdaptor object
 * @return A boolean indicating whether both objects are inequal
 */
bool GDoubleBiGaussAdaptor::operator!=(const GDoubleBiGaussAdaptor& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GDoubleBiGaussAdaptor::operator!=","cp", CE_SILENT);
}

/******************************************************************************/
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
boost::optional<std::string> GDoubleBiGaussAdaptor::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const {
    using namespace Gem::Common;

    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GDoubleBiGaussAdaptor>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GFPBiGaussAdaptorT<double>::checkRelationshipWith(cp, e, limit, "GDoubleBiGaussAdaptor", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GDoubleBiGaussAdaptor", caller, deviations, e);
}

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GDoubleBiGaussAdaptor object, camouflaged as a GObject
 */
void GDoubleBiGaussAdaptor::load_(const GObject* cp){
    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GDoubleBiGaussAdaptor>(cp);

	// Load our parent class'es data ...
	GFPBiGaussAdaptorT<double>::load_(cp);

	// ... no local data
}

/******************************************************************************/
/**
 * Retrieves the id of this adaptor
 *
 * @return The id of this adaptor
 */
Gem::Geneva::adaptorId GDoubleBiGaussAdaptor::getAdaptorId() const {
	return Gem::Geneva::GDOUBLEBIGAUSSADAPTOR;
}

/* ----------------------------------------------------------------------------------
 * - Tested in GDoubleBiGaussAdaptor::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GDoubleBiGaussAdaptor::modify_GUnitTests() {
#ifdef GEM_TESTING
   using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent class'es function
	if(GFPBiGaussAdaptorT<double>::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GDoubleBiGaussAdaptor::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GDoubleBiGaussAdaptor::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GFPBiGaussAdaptorT<double>::specificTestsNoFailureExpected_GUnitTests();

	// --------------------------------------------------------------------------

	{ // Check that the adaptor returns the correct adaptor id
		boost::shared_ptr<GDoubleBiGaussAdaptor> p_test = this->clone<GDoubleBiGaussAdaptor>();

		BOOST_CHECK_MESSAGE(
			p_test->getAdaptorId() == GDOUBLEBIGAUSSADAPTOR
			,  "\n"
			<< "p_test->getAdaptorId() = " << p_test->getAdaptorId()
			<< "GDOUBLEBIGAUSSADAPTOR     = " << GDOUBLEBIGAUSSADAPTOR << "\n"
		);
	}

	// --------------------------------------------------------------------------
	// Note to self: Test the effects of the adaptAdaptionProbability -- how often
	// are the adaption settings adapted for a specific probability ?
	// --------------------------------------------------------------------------

	// --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GDoubleBiGaussAdaptor::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GDoubleBiGaussAdaptor::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GFPBiGaussAdaptorT<double>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GDoubleBiGaussAdaptor::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
