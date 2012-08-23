/**
 * @file GFloatGaussAdaptor.cpp
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

#include "geneva/GFloatGaussAdaptor.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GFloatGaussAdaptor)

namespace Gem {
namespace Geneva {

/*******************************************************************************************/
/**
 * The default constructor
 */
GFloatGaussAdaptor::GFloatGaussAdaptor()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GFloatGaussAdaptor object
 */
GFloatGaussAdaptor::GFloatGaussAdaptor(const GFloatGaussAdaptor& cp)
	: GFPGaussAdaptorT<float>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization with a adaption probability
 *
 * @param adProb The adaption probability
 */
GFloatGaussAdaptor::GFloatGaussAdaptor(const float& adProb)
	: GFPGaussAdaptorT<float>(adProb)
{ /* nothing */ }

/********************************************************************************************/
/**
 * This constructor lets a user set all sigma parameters in one go.
 *
 * @param sigma The initial value for the sigma_ parameter
 * @param sigmaSigma The initial value for the sigmaSigma_ parameter
 * @param minSigma The minimal value allowed for sigma_
 * @param maxSigma The maximal value allowed for sigma_
 */
GFloatGaussAdaptor::GFloatGaussAdaptor(
		const float& sigma
		, const float& sigmaSigma
		, const float& minSigma
		, const float& maxSigma
)
	: GFPGaussAdaptorT<float> (sigma, sigmaSigma, minSigma, maxSigma)
{ /* nothing */ }

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
GFloatGaussAdaptor::GFloatGaussAdaptor(
		const float& sigma
		, const float& sigmaSigma
		, const float& minSigma
		, const float& maxSigma
		, const float& adProb
)
	: GFPGaussAdaptorT<float> (sigma, sigmaSigma, minSigma, maxSigma, adProb)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GFloatGaussAdaptor::~GFloatGaussAdaptor()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GFloatGaussAdaptor object
 * @return A constant reference to this object
 */
const GFloatGaussAdaptor& GFloatGaussAdaptor::operator=(const GFloatGaussAdaptor& cp){
	GFloatGaussAdaptor::load_(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GFloatGaussAdaptor::clone_() const {
	return new GFloatGaussAdaptor(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GFloatGaussAdaptor object
 *
 * @param  cp A constant reference to another GFloatGaussAdaptor object
 * @return A boolean indicating whether both objects are equal
 */
bool GFloatGaussAdaptor::operator==(const GFloatGaussAdaptor& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GFloatGaussAdaptor::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GFloatGaussAdaptor object
 *
 * @param  cp A constant reference to another GFloatGaussAdaptor object
 * @return A boolean indicating whether both objects are inequal
 */
bool GFloatGaussAdaptor::operator!=(const GFloatGaussAdaptor& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GFloatGaussAdaptor::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GFloatGaussAdaptor::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const {
    using namespace Gem::Common;

    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GFloatGaussAdaptor>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GFPGaussAdaptorT<float>::checkRelationshipWith(cp, e, limit, "GFloatGaussAdaptor", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GFloatGaussAdaptor", caller, deviations, e);
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GFloatGaussAdaptor object, camouflaged as a GObject
 */
void GFloatGaussAdaptor::load_(const GObject* cp){
    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GFloatGaussAdaptor>(cp);

	// Load our parent class'es data ...
	GFPGaussAdaptorT<float>::load_(cp);

	// ... no local data
}

/*******************************************************************************************/
/**
 * Retrieves the id of this adaptor
 *
 * @return The id of this adaptor
 */
Gem::Geneva::adaptorId GFloatGaussAdaptor::getAdaptorId() const {
	return Gem::Geneva::GFLOATGAUSSADAPTOR;
}

/* ----------------------------------------------------------------------------------
 * - Tested in GFloatGaussAdaptor::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

#ifdef GEM_TESTING
/*******************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GFloatGaussAdaptor::modify_GUnitTests() {
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent class'es function
	if(GFPGaussAdaptorT<float>::modify_GUnitTests()) result = true;

	return result;
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GFloatGaussAdaptor::specificTestsNoFailureExpected_GUnitTests() {
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GFPGaussAdaptorT<float>::specificTestsNoFailureExpected_GUnitTests();

	//------------------------------------------------------------------------------

	{ // Check that the adaptor returns the correct adaptor id
		boost::shared_ptr<GFloatGaussAdaptor> p_test = this->clone<GFloatGaussAdaptor>();

		BOOST_CHECK_MESSAGE(
			p_test->getAdaptorId() == GFLOATGAUSSADAPTOR
			,  "\n"
			<< "p_test->getAdaptorId() = " << p_test->getAdaptorId()
			<< "GFLOATGAUSSADAPTOR     = " << GFLOATGAUSSADAPTOR << "\n"
		);
	}

	//------------------------------------------------------------------------------
	// Note to self: Test the effects of the adaptAdaptionProbability -- how often
	// are the adaption settings adapted for a specific probability ?
	//------------------------------------------------------------------------------

	//------------------------------------------------------------------------------
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GFloatGaussAdaptor::specificTestsFailuresExpected_GUnitTests() {
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GFPGaussAdaptorT<float>::specificTestsFailuresExpected_GUnitTests();
}

/*******************************************************************************************/
#endif /* GEM_TESTING */

} /* namespace Geneva */
} /* namespace Gem */
