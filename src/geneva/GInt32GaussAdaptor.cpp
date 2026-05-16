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

#include "geneva/GInt32GaussAdaptor.hpp"

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GInt32GaussAdaptor) // NOLINT
namespace Gem::Geneva {

/******************************************************************************/
/**
 * The default constructor. Note that we need to use a different default value for sigma,
 * as there is a "natural" gap of 1 between integers, and the DEFAULTSIGMA might not be
 * suitable for us.
 */
GInt32GaussAdaptor::GInt32GaussAdaptor()
  : GIntGaussAdaptorT<std::int32_t>(
        DEFAULTINT32SIGMA,
        DEFAULTSIGMASIGMA,
        DEFAULTMINSIGMA,
        DEFAULTMAXSIGMA
    ) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with a adaption probability.  Note that we need to use a different default
 * value for sigma, as there is a "natural" gap of 1 between integers, and the DEFAULTSIGMA
 * might not be suitable for us.
 *
 * @param ad_prob The adaption probability
 */
GInt32GaussAdaptor::GInt32GaussAdaptor(const double &ad_prob)
  : GIntGaussAdaptorT<std::int32_t>(
        DEFAULTINT32SIGMA,
        DEFAULTSIGMASIGMA,
        DEFAULTMINSIGMA,
        DEFAULTMAXSIGMA,
        ad_prob
    ) { /* nothing */
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
GInt32GaussAdaptor::GInt32GaussAdaptor(
    const double &sigma,
    const double &sigma_sigma,
    const double &min_sigma,
    const double &max_sigma
)
  : GIntGaussAdaptorT<std::int32_t>(sigma, sigma_sigma, min_sigma, max_sigma) { /* nothing */
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
GInt32GaussAdaptor::GInt32GaussAdaptor(
    const double &sigma,
    const double &sigma_sigma,
    const double &min_sigma,
    const double &max_sigma,
    const double &ad_prob
)
  : GIntGaussAdaptorT<std::int32_t>(
        sigma,
        sigma_sigma,
        min_sigma,
        max_sigma,
        ad_prob
    ) { /* nothing */
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject *GInt32GaussAdaptor::clone_() const {
    return new GInt32GaussAdaptor(*this);
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
void GInt32GaussAdaptor::compare_(
    const GObject &cp,
    const Gem::Common::expectation &e,
    const double & /*limit*/
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GInt32GaussAdaptor reference independent of this object and convert the pointer
    const GInt32GaussAdaptor *p_load =
        Gem::Common::g_convert_and_compare<GObject, GInt32GaussAdaptor>(cp, this);

    GToken token("GInt32GaussAdaptor", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GIntGaussAdaptorT<std::int32_t>>(*this, *p_load, token);

    // ... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GInt32GaussAdaptor::name_() const {
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
    const GInt32GaussAdaptor *p_load =
        Gem::Common::g_convert_and_compare<GObject, GInt32GaussAdaptor>(cp, this);

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
Gem::Geneva::adaptorId GInt32GaussAdaptor::getAdaptorId_() const {
    return Gem::Geneva::adaptorId::GINT32GAUSSADAPTOR;
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
bool GInt32GaussAdaptor::modify_GUnitTests_() {
#ifdef GEM_TESTING

    bool result = false;

    // Call the parent class'es function
    if(GIntGaussAdaptorT<std::int32_t>::modify_GUnitTests_())
        result = true;

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GInt32GaussAdaptor::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GInt32GaussAdaptor::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent class'es function
    GIntGaussAdaptorT<std::int32_t>::specificTestsNoFailureExpected_GUnitTests_();

    // --------------------------------------------------------------------------

    { // Check that the adaptor returns the correct adaptor id
        std::shared_ptr<GInt32GaussAdaptor> p_test = this->clone<GInt32GaussAdaptor>();

        INFO(
            "\n"
            << "p_test->getAdaptorId() = " << p_test->getAdaptorId()
            << "GINT32GAUSSADAPTOR     = " << adaptorId::GINT32GAUSSADAPTOR << "\n"
        );
        CHECK(p_test->getAdaptorId() == adaptorId::GINT32GAUSSADAPTOR);
    }

    // --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GInt32GaussAdaptor::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GInt32GaussAdaptor::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Call the parent class'es function
    GIntGaussAdaptorT<std::int32_t>::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GInt32GaussAdaptor::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva */
