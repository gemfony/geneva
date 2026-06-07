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
#include "geneva/par/GFloatGaussAdaptor.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/par/GAdaptorT.hpp"
#include "geneva/par/GFPGaussAdaptorT.hpp"
#include <memory>

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Parameters::GFloatGaussAdaptor) // NOLINT
namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * Initialization with a adaption probability
 *
 * @param ad_prob The adaption probability
 */
GFloatGaussAdaptor::GFloatGaussAdaptor(const double &ad_prob)
  : GFPGaussAdaptorT<float>(ad_prob) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with a number of values belonging to the width of the gaussian.
 */
GFloatGaussAdaptor::GFloatGaussAdaptor(
    const float &sigma,
    const float &sigma_sigma,
    const float &min_sigma,
    const float &max_sigma
)
  : GFPGaussAdaptorT<float>(sigma, sigma_sigma, min_sigma, max_sigma) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with a number of values belonging to the width of the gaussian and the
 * adaption probability.
 */
GFloatGaussAdaptor::GFloatGaussAdaptor(
    const float &sigma,
    const float &sigma_sigma,
    const float &min_sigma,
    const float &max_sigma,
    const double &ad_prob
)
  : GFPGaussAdaptorT<float>(sigma, sigma_sigma, min_sigma, max_sigma, ad_prob) { /* nothing */
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GAdaptorT<float, float>
 */
GAdaptorT<float, float> *GFloatGaussAdaptor::clone_() const {
    return new GFloatGaussAdaptor(*this);
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GAdaptorT<float, float> object
 * @param e The expected outcome of the comparison
 */
void GFloatGaussAdaptor::compare_(
    const GAdaptorT<float, float> &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GFloatGaussAdaptor reference independent of this object and convert the pointer
    const GFloatGaussAdaptor *p_load =
        Gem::Common::g_convert_and_compare<GAdaptorT<float, float>, GFloatGaussAdaptor>(cp, this);

    GToken token("GFloatGaussAdaptor", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GFPGaussAdaptorT<float>>(*this, *p_load, token);

    // ... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GFloatGaussAdaptor::name_() const {
    return std::string("GFloatGaussAdaptor");
}

/******************************************************************************/
/**
 * Loads the data of another GAdaptorT
 *
 * @param cp A copy of another GFloatGaussAdaptor object, camouflaged as a GAdaptorT<float, float>
 */
void GFloatGaussAdaptor::load_(const GAdaptorT<float, float> *cp) {
    // Convert the pointer to our target type and check for self-assignment
    const GFloatGaussAdaptor *p_load =
        Gem::Common::g_convert_and_compare<GAdaptorT<float, float>, GFloatGaussAdaptor>(cp, this);

    // Load our parent class'es data ...
    GFPGaussAdaptorT<float>::load_(cp);

    // ... no local data
}

/******************************************************************************/
/**
 * Retrieves the id of this adaptor
 *
 * @return The id of this adaptor
 */
Gem::Geneva::adaptorId GFloatGaussAdaptor::getAdaptorId_() const {
    return Gem::Geneva::adaptorId::GFLOATGAUSSADAPTOR;
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GFloatGaussAdaptor::modify_GUnitTests_() {
#ifdef GEM_TESTING

    bool result = false;

    // Call the parent class'es function
    if(GFPGaussAdaptorT<float>::modify_GUnitTests_()) {
        result = true;
    }

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GFloatGaussAdaptor::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GFloatGaussAdaptor::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent class'es function
    GFPGaussAdaptorT<float>::specificTestsNoFailureExpected_GUnitTests_();

    // --------------------------------------------------------------------------

    { // Check that the adaptor returns the correct adaptor id
        std::shared_ptr<GFloatGaussAdaptor> p_test = this->clone<GFloatGaussAdaptor>();

        INFO(
            "\n"
            << "p_test->getAdaptorId() = " << p_test->getAdaptorId()
            << "GFLOATGAUSSADAPTOR      = " << adaptorId::GFLOATGAUSSADAPTOR << "\n"
        );
        CHECK(p_test->getAdaptorId() == adaptorId::GFLOATGAUSSADAPTOR);
    }

    // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GFloatGaussAdaptor::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GFloatGaussAdaptor::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent class'es function
    GFPGaussAdaptorT<float>::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GFloatGaussAdaptor::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
