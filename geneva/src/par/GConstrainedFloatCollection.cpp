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

#include "geneva/par/GConstrainedFloatCollection.hpp"

#include <cmath>
#include <limits>
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/par/GConstrainedFPNumCollectionT.hpp"
#include "geneva/par/GParameterBase.hpp"
#include "geneva/par/GParameterCollectionT.hpp"
#include "hap/GHapEnums.hpp"
#include "hap/GRandomBase.hpp"
#include "hap/GRandomT.hpp"
#include <cstddef>
#include <map>
#include <memory>
#include <random>
#include <vector>

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Parameters::GConstrainedFloatCollection) // NOLINT
namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * Initialize with the lower and upper boundaries for data members of this class and
 * a number of random values within this range. Note that all action will take place in the
 * range [lowerBoundary, upperBoundary[.
 *
 * @param size The desired size of the collection
 * @param lower_boundary The lower boundary for data members
 * @param upper_boundary The upper boundary for data members
 */
GConstrainedFloatCollection::GConstrainedFloatCollection(
    const std::size_t &size,
    const float &lower_boundary,
    const float &upper_boundary
)
  : GConstrainedFPNumCollectionT<float>(size, lower_boundary, upper_boundary) { /* nothing */
}

/******************************************************************************/
/**
 * Initialize with the lower and upper boundaries for data members of this class and
 * a fixed value for all items in the vector. Note that all action will take place in the
 * range [lowerBoundary, upperBoundary[.
 *
 * @param size The desired size of the collection
 * @param val The value to be assigned to all positions
 * @param lower_boundary The lower boundary for data members
 * @param upper_boundary The upper boundary for data members
 */
GConstrainedFloatCollection::GConstrainedFloatCollection(
    const std::size_t &size,
    const float &val,
    const float &lower_boundary,
    const float &upper_boundary
)
  : GConstrainedFPNumCollectionT<float>(size, val, lower_boundary, upper_boundary) { /* nothing */
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GParameterBase object
 * @param e The expected outcome of the comparison
 */
void GConstrainedFloatCollection::compare_(
    const GParameterBase &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GConstrainedFloatCollection reference independent of this object and convert the pointer
    const GConstrainedFloatCollection *p_load =
        Gem::Common::g_convert_and_compare<GParameterBase, GConstrainedFloatCollection>(cp, this);

    GToken token("GConstrainedFloatCollection", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GConstrainedFPNumCollectionT<float>>(*this, *p_load, token);

    // ... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GConstrainedFloatCollection::name_() const {
    return std::string("GConstrainedFloatCollection");
}

/******************************************************************************/
/**
 * Attach our local values to the vector. This is used to collect all parameters of this type
 * in the sequence in which they were registered.
 */
void GConstrainedFloatCollection::floatStreamline(
    std::vector<float> &par_vec,
    [[maybe_unused]] const activityMode & am
) const {
    for(const auto &val : *this) {
        par_vec.push_back(this->transfer(val));
    }
}

/******************************************************************************/

/******************************************************************************/
/**
 * Attach boundaries of type float to the vectors.
 */
void GConstrainedFloatCollection::floatBoundaries(
    std::vector<float> &l_bnd_vec,
    std::vector<float> &u_bnd_vec,
    [[maybe_unused]] const activityMode & am
) const {
    // Add a lower and upper boundary to the vectors
    // for each variable in the collection
    for(std::size_t pos = 0; pos < this->size(); pos++) {
        l_bnd_vec.push_back(this->getLowerBoundary());
        u_bnd_vec.push_back(this->getUpperBoundary());
    }
}

/******************************************************************************/
/**
 * Tell the audience that we own a number of float values
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of float parameters
 */
std::size_t GConstrainedFloatCollection::countFloatParameters(
    [[maybe_unused]] const activityMode & am
) const {
    return this->size();
}

/******************************************************************************/
/**
 * Assigns part of a value vector to the parameter. Note that we apply a transformation to the
 * vector, so that it lies inside of the allowed value range.
 */
void GConstrainedFloatCollection::assignFloatValueVector(
    const std::vector<float> &par_vec,
    std::size_t &pos,
    [[maybe_unused]] const activityMode & am
) {
    for(std::size_t i = 0; i < this->size(); i++) {
#ifdef DEBUG
        // Do we have a valid position ?
        if(pos >= par_vec.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GConstrainedFloatCollection::assignFloatValueVector(const "
                   "std::vector<float>&, std::size_t&):"
                << '\n'
                << "Tried to access position beyond end of vector: " << par_vec.size() << "/" << pos
                << '\n'
            );
        }
#endif

        this->setValue(i, this->transfer(par_vec[pos]));
        pos++;
    }
}

/******************************************************************************/

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
void GConstrainedFloatCollection::floatMultiplyByRandom(
    const float &min,
    const float &max,
    [[maybe_unused]] const activityMode & am
    ,
    Gem::Hap::GRandomBase &gr // NOLINT(misc-unused-parameters)
) {
    std::uniform_real_distribution<float> uniform_real_distribution(min, max);
    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<float>::setValue(
            pos,
            transfer(this->value(pos) * uniform_real_distribution(gr))
        );
    }
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
void GConstrainedFloatCollection::floatMultiplyByRandom(
    [[maybe_unused]] const activityMode & am
    ,
    Gem::Hap::GRandomBase &gr // NOLINT(misc-unused-parameters)
) {
    std::uniform_real_distribution<float> uniform_real_distribution(0., 1.);
    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<float>::setValue(
            pos,
            transfer(this->value(pos) * uniform_real_distribution(gr))
        );
    }
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
void GConstrainedFloatCollection::floatMultiplyBy(
    const float &val,
    const activityMode & /*am*/ // NOLINT(misc-unused-parameters)
) {
    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<float>::setValue(pos, transfer(val * this->value(pos)));
    }
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
/**
 * Initialization with a constant value
 */
void GConstrainedFloatCollection::floatFixedValueInit(
    const float &val,
    const activityMode & /*am*/ // NOLINT(misc-unused-parameters)
) {
    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<float>::setValue(pos, transfer(val));
    }
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GConstrainedFloatCollection::floatAdd(
    std::shared_ptr<GParameterBase> p_base,
    [[maybe_unused]] const activityMode & am
) {
    // We first need to convert p_base into the local type
    std::shared_ptr<GConstrainedFloatCollection> p =
        GParameterBase::parameterbase_cast<GConstrainedFloatCollection>(p_base);

    // Cross-check that the sizes match
    if(this->size() != p->size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GConstrainedFloatCollection::floatAdd():" << '\n'
            << "Sizes of vectors don't match: " << this->size() << "/" << p->size() << '\n'
        );
    }

    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<float>::setValue(pos, transfer(this->value(pos) + p->value(pos)));
    }
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
void GConstrainedFloatCollection::floatSubtract(
    std::shared_ptr<GParameterBase> p_base,
    [[maybe_unused]] const activityMode & am
) {
    // We first need to convert p_base into the local type
    std::shared_ptr<GConstrainedFloatCollection> p =
        GParameterBase::parameterbase_cast<GConstrainedFloatCollection>(p_base);

    // Cross-check that the sizes match
    if(this->size() != p->size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GConstrainedFloatCollection::floatSubtract():" << '\n'
            << "Sizes of vectors don't match: " << this->size() << "/" << p->size() << '\n'
        );
    }

    for(std::size_t pos = 0; pos < this->size(); pos++) {
        GParameterCollectionT<float>::setValue(pos, transfer(this->value(pos) - p->value(pos)));
    }
}

/******************************************************************************/
/**
 * Loads the data of another GConstrainedFloatCollection object,
 * camouflaged as a GParameterBase. We have no local data, so
 * all we need to do is to the standard identity check,
 * preventing that an object is assigned to itself.
 *
 * @param cp A copy of another GConstrainedFloatCollection object, camouflaged as a GParameterBase
 */
void GConstrainedFloatCollection::load_(const GParameterBase *cp) {
    // Convert the pointer to our target type and check for self-assignment
    const GConstrainedFloatCollection *p_load =
        Gem::Common::g_convert_and_compare<GParameterBase, GConstrainedFloatCollection>(cp, this);

    // Load our parent class'es data ...
    GConstrainedFPNumCollectionT<float>::load_(cp);

    // no local data ...
}

/******************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GParameterBase
 */
GParameterBase *GConstrainedFloatCollection::clone_() const {
    return new GConstrainedFloatCollection(*this);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GConstrainedFloatCollection::modify_GUnitTests_() {
#ifdef GEM_TESTING
    // A random generator
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

    bool result = false;

    // Call the parent classes' functions
    if(GConstrainedFPNumCollectionT<float>::modify_GUnitTests_()) {
        result = true;
    }

    this->randomInit(activityMode::ALLPARAMETERS, gr);
    result = true;

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GConstrainedFloatCollection::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GConstrainedFloatCollection::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Call the parent classes' functions
    GConstrainedFPNumCollectionT<float>::specificTestsNoFailureExpected_GUnitTests_();

    // Some parameters
    constexpr std::size_t defsize = 10;
    constexpr float defval = 1.;
    constexpr float defmin = -10.;
    constexpr float defmax = 10.;

    //---------------------------------------------------------------------

    { // Check that initialization with a fixed value-range yields the desired values
        std::shared_ptr<GConstrainedFloatCollection> p_test;

        CHECK_NOTHROW(
            p_test = std::make_shared<GConstrainedFloatCollection>(defsize, defmin, defmax)
        );
        CHECK((p_test->size() == defsize && defsize > 1));
        for(std::size_t i = 1; i < defsize; i++) { // Check that consecutive values are different
            CHECK(p_test->at(i) != p_test->at(i - 1));
        }
        CHECK(p_test->getLowerBoundary() == defmin);
        CHECK(
            p_test->getUpperBoundary() ==
            std::nextafter(defmax, -std::numeric_limits<float>::infinity())
        ); // The upper boundary is an open one
    }

    //---------------------------------------------------------------------

    { // Check that initialization with a fixed value and range yields the desired values
        std::shared_ptr<GConstrainedFloatCollection> p_test;

        CHECK_NOTHROW(
            p_test = std::make_shared<GConstrainedFloatCollection>(defsize, defval, defmin, defmax)
        );
        CHECK(p_test->size() == defsize);
        for(std::size_t i = 0; i < defsize; i++) {
            CHECK(p_test->at(i) == defval);
        }
        CHECK(p_test->getLowerBoundary() == defmin);
        CHECK(
            p_test->getUpperBoundary() ==
            std::nextafter(defmax, -std::numeric_limits<float>::infinity())
        ); // The upper boundary is an open one
    }

    //---------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GConstrainedFloatCollection::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GConstrainedFloatCollection::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Call the parent classes' functions
    GConstrainedFPNumCollectionT<float>::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GBrokerEA::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
