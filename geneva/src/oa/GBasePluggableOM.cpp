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

#include "geneva/oa/GBasePluggableOM.hpp"

#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"

/******************************************************************************/

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
	 * Searches for compliance with expectations with respect to another object
	 * of the same type
	 *
	 * @param cp A constant reference to another GBasePluggableOM object
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 */
void GBasePluggableOM::compare_(
    const GBasePluggableOM &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GBasePluggableOM reference independent of this object and convert the pointer
    const auto *p_load =
        Gem::Common::g_convert_and_compare<GBasePluggableOM, GBasePluggableOM>(cp, this);

    GToken token("GBasePluggableOM", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<Gem::Common::GCommonInterfaceT<GBasePluggableOM>>(*this, *p_load, token);

    // ... and then our local data, derived from the single localMembers() declaration
    g_compare_members(this->localMembers_(), p_load->localMembers_(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * @brief Allows to set the use_raw_evaluation_ variable
 *
 * @param use_raw If true, the monitor reports raw (untransformed) fitness values instead of transformed ones
 */
void GBasePluggableOM::setUseRawEvaluation(bool use_raw) {
    use_raw_evaluation_ = use_raw;
}

/******************************************************************************/
/**
 * @brief Allows to retrieve the value of the use_raw_evaluation_ variable
 *
 * @return True if the monitor reports raw (untransformed) fitness values, false if it reports transformed ones
 */
bool GBasePluggableOM::getUseRawEvaluation() const {
    return use_raw_evaluation_;
}

/******************************************************************************/
/**
 * @brief Access to information about the current iteration. This is a wrapper
 * function to avoid public virtual.
 *
 * @param im The information mode (INFOINIT, INFOPROCESSING or INFOEND) describing the optimization phase
 * @param goa A pointer to the optimization algorithm currently being monitored (not owned)
 */
void GBasePluggableOM::informationFunction(
    infoMode im,
    GOptimizationAlgorithmBase const *const goa
) {
    informationFunction_(im, goa);
}

/******************************************************************************/
/**
 * @brief Loads the data of another object
 *
 * @param cp A pointer to another GBasePluggableOM object, camouflaged as a GBasePluggableOM (not owned)
 */
void GBasePluggableOM::load_(const GBasePluggableOM *cp) {
    // Check that we are dealing with a GBasePluggableOM reference independent of this object and convert the pointer
    const auto *p_load =
        Gem::Common::g_convert_and_compare<GBasePluggableOM, GBasePluggableOM>(cp, this);

    // This is the category root; there is no GObject parent class to load.

    // Our own data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(this->localMembers_(), p_load->localMembers_());
}

/******************************************************************************/
/**
 * @brief Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBasePluggableOM::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    // This is the category root; there is no modifiable parent class.

    this->setUseRawEvaluation(!this->getUseRawEvaluation());
    result = true;

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GBasePluggableOM", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // This is the category root; there is no parent class to test.

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBasePluggableOM::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // This is the category root; there is no parent class to test.

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GBasePluggableOM::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}


/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
