/**
 * @file GInt32GaussAdaptor.cpp
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

#include "geneva/GInt32GaussAdaptor.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GInt32GaussAdaptor)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor. Note that we need to use a different default value for sigma,
 * as there is a "natural" gap of 1 between integers, and the DEFAULTSIGMA might not be
 * suitable for us.
 */
GInt32GaussAdaptor::GInt32GaussAdaptor()
	: GIntGaussAdaptorT<std::int32_t>(DEFAULTINT32SIGMA, DEFAULTSIGMASIGMA, DEFAULTMINSIGMA,
													DEFAULTMAXSIGMA) { /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GInt32GaussAdaptor object
 */
GInt32GaussAdaptor::GInt32GaussAdaptor(const GInt32GaussAdaptor &cp)
	: GIntGaussAdaptorT<std::int32_t>(cp) { /* nothing */ }

/******************************************************************************/
/**
 * Initialization with a adaption probability.  Note that we need to use a different default
 * value for sigma, as there is a "natural" gap of 1 between integers, and the DEFAULTSIGMA
 * might not be suitable for us.
 *
 * @param adProb The adaption probability
 */
GInt32GaussAdaptor::GInt32GaussAdaptor(const double &adProb)
	: GIntGaussAdaptorT<std::int32_t>(DEFAULTINT32SIGMA, DEFAULTSIGMASIGMA, DEFAULTMINSIGMA, DEFAULTMAXSIGMA,
													adProb) { /* nothing */ }

/********************************************************************************************/
/**
 * This constructor lets a user set all sigma parameters in one go.
 *
 * @param sigma The initial value for the sigma_ parameter
 * @param sigmaSigma The initial value for the sigmaSigma_ parameter
 * @param minSigma The minimal value allowed for sigma_
 * @param maxSigma The maximal value allowed for sigma_
 */
GInt32GaussAdaptor::GInt32GaussAdaptor(
	const double &sigma, const double &sigmaSigma, const double &minSigma, const double &maxSigma
)
	: GIntGaussAdaptorT<std::int32_t>(sigma, sigmaSigma, minSigma, maxSigma) { /* nothing */ }

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
GInt32GaussAdaptor::GInt32GaussAdaptor(
	const double &sigma, const double &sigmaSigma, const double &minSigma, const double &maxSigma, const double &adProb
)
	: GIntGaussAdaptorT<std::int32_t>(sigma, sigmaSigma, minSigma, maxSigma, adProb) { /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GInt32GaussAdaptor::~GInt32GaussAdaptor() { /* nothing */ }

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject *GInt32GaussAdaptor::clone_() const {
	return new GInt32GaussAdaptor(*this);
}

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GInt32GaussAdaptor &GInt32GaussAdaptor::operator=(const GInt32GaussAdaptor &cp) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GInt32GaussAdaptor object
 *
 * @param  cp A constant reference to another GInt32GaussAdaptor object
 * @return A boolean indicating whether both objects are equal
 */
bool GInt32GaussAdaptor::operator==(const GInt32GaussAdaptor &cp) const {
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
 * Checks for inequality with another GInt32GaussAdaptor object
 *
 * @param  cp A constant reference to another GInt32GaussAdaptor object
 * @return A boolean indicating whether both objects are inequal
 */
bool GInt32GaussAdaptor::operator!=(const GInt32GaussAdaptor &cp) const {
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
void GInt32GaussAdaptor::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GInt32GaussAdaptor reference independent of this object and convert the pointer
	const GInt32GaussAdaptor *p_load = Gem::Common::g_convert_and_compare<GObject, GInt32GaussAdaptor>(cp, this);

	GToken token("GInt32GaussAdaptor", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GIntGaussAdaptorT<std::int32_t>>(IDENTITY(*this, *p_load), token);

	// ... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GInt32GaussAdaptor::name() const {
	return std::string("GInt32GaussAdaptor");
}

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GInt32GaussAdaptor object, camouflaged as a GObject
 */
void GInt32GaussAdaptor::load_(const GObject *cp) {
	// Convert the pointer to our target type and check for self-assignment
	const GInt32GaussAdaptor * p_load = Gem::Common::g_convert_and_compare<GObject, GInt32GaussAdaptor>(cp, this);

	// Load our parent class'es data ...
	GIntGaussAdaptorT<std::int32_t>::load_(cp);

	// ... no local data
}

/******************************************************************************/
/**
 * Retrieves the id of this adaptor
 *
 * @return The id of this adaptor
 */
Gem::Geneva::adaptorId GInt32GaussAdaptor::getAdaptorId() const {
	return Gem::Geneva::GINT32GAUSSADAPTOR;
}

/* ----------------------------------------------------------------------------------
 * - Tested in GInt32GaussAdaptor::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GInt32GaussAdaptor::modify_GUnitTests() {
#ifdef GEM_TESTING

	bool result = false;

	// Call the parent class'es function
	if (GIntGaussAdaptorT<std::int32_t>::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GInt32GaussAdaptor::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GInt32GaussAdaptor::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING

	// Call the parent class'es function
	GIntGaussAdaptorT<std::int32_t>::specificTestsNoFailureExpected_GUnitTests();

	// --------------------------------------------------------------------------

	{ // Check that the adaptor returns the correct adaptor id
		std::shared_ptr <GInt32GaussAdaptor> p_test = this->clone<GInt32GaussAdaptor>();

		BOOST_CHECK_MESSAGE(
			p_test->getAdaptorId() == GINT32GAUSSADAPTOR, "\n"
																		 << "p_test->getAdaptorId() = " << p_test->getAdaptorId()
																		 << "GINT32GAUSSADAPTOR     = " << GINT32GAUSSADAPTOR << "\n"
		);
	}

	// --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GInt32GaussAdaptor::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GInt32GaussAdaptor::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GIntGaussAdaptorT<std::int32_t>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GInt32GaussAdaptor::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
