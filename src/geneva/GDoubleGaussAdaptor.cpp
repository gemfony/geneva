/**
 * @file GDoubleGaussAdaptor.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
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
 * http://www.gemfony.eu .
 */

#include "geneva/GDoubleGaussAdaptor.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GDoubleGaussAdaptor)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GDoubleGaussAdaptor::GDoubleGaussAdaptor() { /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GDoubleGaussAdaptor object
 */
GDoubleGaussAdaptor::GDoubleGaussAdaptor(const GDoubleGaussAdaptor &cp)
	: GFPGaussAdaptorT<double>(cp) { /* nothing */ }

/******************************************************************************/
/**
 * Initialization with a adaption probability
 *
 * @param adProb The adaption probability
 */
GDoubleGaussAdaptor::GDoubleGaussAdaptor(const double &adProb)
	: GFPGaussAdaptorT<double>(adProb) { /* nothing */ }

/********************************************************************************************/
/**
 * This constructor lets a user set all sigma parameters in one go.
 *
 * @param sigma The initial value for the sigma_ parameter
 * @param sigmaSigma The initial value for the sigmaSigma_ parameter
 * @param minSigma The minimal value allowed for sigma_
 * @param maxSigma The maximal value allowed for sigma_
 */
GDoubleGaussAdaptor::GDoubleGaussAdaptor(
	const double &sigma, const double &sigmaSigma, const double &minSigma, const double &maxSigma
)
	: GFPGaussAdaptorT<double>(sigma, sigmaSigma, minSigma, maxSigma) { /* nothing */ }

/********************************************************************************************/
/**
 * This constructor lets a user set all sigma parameters, as well as the adaption
 * probability in one go.
 *
 * @param sigma The initial value for the sigma_ parameter
 * @param sigmaSigma The initial value for the sigmaSigma_ parameter
 * @param minSigma The minimal value allowed for sigma_
 * @param maxSigma The maximal value allowed for sigma_
 * @param adProb The adaption probability
 */
GDoubleGaussAdaptor::GDoubleGaussAdaptor(
	const double &sigma, const double &sigmaSigma, const double &minSigma, const double &maxSigma, const double &adProb
)
	: GFPGaussAdaptorT<double>(sigma, sigmaSigma, minSigma, maxSigma, adProb) { /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GDoubleGaussAdaptor::~GDoubleGaussAdaptor() { /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GDoubleGaussAdaptor &GDoubleGaussAdaptor::operator=(
	const GDoubleGaussAdaptor &cp
) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject *GDoubleGaussAdaptor::clone_() const {
	return new GDoubleGaussAdaptor(*this);
}

/******************************************************************************/
/**
 * Checks for equality with another GDoubleGaussAdaptor object
 *
 * @param  cp A constant reference to another GDoubleGaussAdaptor object
 * @return A boolean indicating whether both objects are equal
 */
bool GDoubleGaussAdaptor::operator==(const GDoubleGaussAdaptor &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Checks for inequality with another GDoubleGaussAdaptor object
 *
 * @param  cp A constant reference to another GDoubleGaussAdaptor object
 * @return A boolean indicating whether both objects are inequal
 */
bool GDoubleGaussAdaptor::operator!=(const GDoubleGaussAdaptor &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GDoubleGaussAdaptor::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are indeed dealing with a GBaseEA reference
	const GDoubleGaussAdaptor *p_load = GObject::gobject_conversion<GDoubleGaussAdaptor>(&cp);

	GToken token("GDoubleGaussAdaptor", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GFPGaussAdaptorT<double> >(IDENTITY(*this, *p_load), token);

	// ... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GDoubleGaussAdaptor::name() const {
	return std::string("GDoubleGaussAdaptor");
}

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GDoubleGaussAdaptor object, camouflaged as a GObject
 */
void GDoubleGaussAdaptor::load_(const GObject *cp) {
	// Check that we are not accidently assigning this object to itself
	GObject::selfAssignmentCheck<GDoubleGaussAdaptor>(cp);

	// Load our parent class'es data ...
	GFPGaussAdaptorT<double>::load_(cp);

	// ... no local data
}

/******************************************************************************/
/**
 * Retrieves the id of this adaptor
 *
 * @return The id of this adaptor
 */
Gem::Geneva::adaptorId GDoubleGaussAdaptor::getAdaptorId() const {
	return Gem::Geneva::GDOUBLEGAUSSADAPTOR;
}

/* ----------------------------------------------------------------------------------
 * - Tested in GDoubleGaussAdaptor::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GDoubleGaussAdaptor::modify_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent class'es function
	if (GFPGaussAdaptorT<double>::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GDoubleGaussAdaptor::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GDoubleGaussAdaptor::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GFPGaussAdaptorT<double>::specificTestsNoFailureExpected_GUnitTests();

	// --------------------------------------------------------------------------

	{ // Check that the adaptor returns the correct adaptor id
		std::shared_ptr <GDoubleGaussAdaptor> p_test = this->clone<GDoubleGaussAdaptor>();

		BOOST_CHECK_MESSAGE(
			p_test->getAdaptorId() == GDOUBLEGAUSSADAPTOR, "\n"
																		  << "p_test->getAdaptorId() = " << p_test->getAdaptorId()
																		  << "GDOUBLEGAUSSADAPTOR     = " << GDOUBLEGAUSSADAPTOR << "\n"
		);
	}

	// --------------------------------------------------------------------------
	// Note to self: Test the effects of the adaptAdaptionProbability -- how often
	// are the adaption settings adapted for a specific probability ?
	// --------------------------------------------------------------------------

	// --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GDoubleGaussAdaptor::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GDoubleGaussAdaptor::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GFPGaussAdaptorT<double>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBrokerEA::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
