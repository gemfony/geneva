/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "geneva/par/GDoubleGaussAdaptor.hpp"

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Parameters::GDoubleGaussAdaptor) // NOLINT
namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * Initialization with a adaption probability
 *
 * @param ad_prob The adaption probability
 */
GDoubleGaussAdaptor::GDoubleGaussAdaptor(const double &ad_prob)
  : GFPGaussAdaptorT<double>(ad_prob) { /* nothing */
}

/********************************************************************************************/
/**
 * This constructor lets a user set all sigma parameters in one go.
 *
 * @param sigma The initial value for the sigma_ parameter
 * @param sigma_sigma The initial value for the sigmaSigma_ parameter
 * @param min_sigma The minimal value allowed for sigma_
 * @param max_sigma The maximal value allowed for sigma_
 */
GDoubleGaussAdaptor::GDoubleGaussAdaptor(
    const double &sigma,
    const double &sigma_sigma,
    const double &min_sigma,
    const double &max_sigma
)
  : GFPGaussAdaptorT<double>(sigma, sigma_sigma, min_sigma, max_sigma) { /* nothing */
}

/********************************************************************************************/
/**
 * This constructor lets a user set all sigma parameters, as well as the adaption
 * probability in one go.
 *
 * @param sigma The initial value for the sigma_ parameter
 * @param sigma_sigma The initial value for the sigmaSigma_ parameter
 * @param min_sigma The minimal value allowed for sigma_
 * @param max_sigma The maximal value allowed for sigma_
 * @param ad_prob The adaption probability
 */
GDoubleGaussAdaptor::GDoubleGaussAdaptor(
    const double &sigma,
    const double &sigma_sigma,
    const double &min_sigma,
    const double &max_sigma,
    const double &ad_prob
)
  : GFPGaussAdaptorT<double>(sigma, sigma_sigma, min_sigma, max_sigma, ad_prob) { /* nothing */
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GAdaptorT
 */
GAdaptorT<double> *GDoubleGaussAdaptor::clone_() const {
    return new GDoubleGaussAdaptor(*this);
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GAdaptorT object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GDoubleGaussAdaptor::compare_(
    const GAdaptorT<double> &cp,
    const Gem::Common::expectation &e,
    const double & /*limit*/
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GDoubleGaussAdaptor reference independent of this object and convert the pointer
    const GDoubleGaussAdaptor *p_load =
        Gem::Common::g_convert_and_compare<GAdaptorT<double>, GDoubleGaussAdaptor>(cp, this);

    GToken token("GDoubleGaussAdaptor", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GFPGaussAdaptorT<double>>(*this, *p_load, token);

    // ... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GDoubleGaussAdaptor::name_() const {
    return std::string("GDoubleGaussAdaptor");
}

/******************************************************************************/
/**
 * Loads the data of another GAdaptorT
 *
 * @param cp A copy of another GDoubleGaussAdaptor object, camouflaged as a GAdaptorT
 */
void GDoubleGaussAdaptor::load_(const GAdaptorT<double> *cp) {
    // Convert the pointer to our target type and check for self-assignment
    const GDoubleGaussAdaptor *p_load =
        Gem::Common::g_convert_and_compare<GAdaptorT<double>, GDoubleGaussAdaptor>(cp, this);

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
Gem::Geneva::adaptorId GDoubleGaussAdaptor::getAdaptorId_() const {
    return Gem::Geneva::adaptorId::GDOUBLEGAUSSADAPTOR;
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
bool GDoubleGaussAdaptor::modify_GUnitTests_() {
#ifdef GEM_TESTING

    bool result = false;

    // Call the parent class'es function
    if(GFPGaussAdaptorT<double>::modify_GUnitTests_()) {
        result = true;
    }

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GDoubleGaussAdaptor::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GDoubleGaussAdaptor::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent class'es function
    GFPGaussAdaptorT<double>::specificTestsNoFailureExpected_GUnitTests_();

    // --------------------------------------------------------------------------

    { // Check that the adaptor returns the correct adaptor id
        std::shared_ptr<GDoubleGaussAdaptor> p_test = this->clone<GDoubleGaussAdaptor>();

        INFO(
            "\n"
            << "p_test->getAdaptorId() = " << p_test->getAdaptorId()
            << "GDOUBLEGAUSSADAPTOR     = " << adaptorId::GDOUBLEGAUSSADAPTOR << "\n"
        );
        CHECK(p_test->getAdaptorId() == adaptorId::GDOUBLEGAUSSADAPTOR);
    }

    // --------------------------------------------------------------------------
    // Note to self: Test the effects of the adaptAdaptionProbability -- how often
    // are the adaption settings adapted for a specific probability ?
    // --------------------------------------------------------------------------

    // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GDoubleGaussAdaptor::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GDoubleGaussAdaptor::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent class'es function
    GFPGaussAdaptorT<double>::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GBrokerEA::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
