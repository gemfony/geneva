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

#include "geneva/individuals/GTestIndividual3.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include <cstddef>
#include <memory>
#include <vector>

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Individuals::GTestIndividual3) // NOLINT
namespace Gem::Geneva::Individuals {

constexpr std::size_t GTI_DEF_NITEMS = 300;
constexpr double GTI_DEF_SIGMA = 0.025;
constexpr double GTI_DEF_SIGMASIGMA = 0.1;
constexpr double GTI_DEF_MINSIGMA = 0.001;
constexpr double GTI_DEF_MAXSIGMA = 1.0;
constexpr double GTI_DEF_ADPROB = 0.05;

/******************************************************************************/
/**
 * The default constructor
 */
GTestIndividual3::GTestIndividual3() {
    using namespace Gem::Geneva;

    // Build a flat genome that reproduces the historical nested structure: GTI_DEF_NITEMS records,
    // each holding 10 constrained doubles in the order a[2], b, c[3], d[3], e (the order the tree's
    // GParameterObjectCollection streamlined them), all sharing the same Gauss adaptor config. A
    // collection becomes one shared-sigma group; a standalone object its own group. The flat
    // streamline order is therefore identical to the tree's, so getPlainData() reads it positionally.
    // Structure only -- this individual is never adapted (it is a genome / slot-scratch test fixture), so
    // no adaptor is attached and no OA adaption config is authored for it.
    gpar::GGenomeBuilder bld;

    for(std::size_t i_cnt = 0; i_cnt < GTI_DEF_NITEMS; i_cnt++) {
        bld.addDoubleGroup(2, 0., 1.); // a: middle of the circle
        bld.addDouble(0., 0., 0.3);    // b
        bld.addDoubleGroup(3, 0., 1.); // c: three angles
        bld.addDoubleGroup(3, 0., 1.); // d: three colors
        bld.addDouble(0.3, 0.3, 0.6);  // e: alpha channel
    }

    this->setGenome(bld.build());

    // Mirror the tree's per-parameter random initialization within bounds.
    this->randomInit(activityMode::ALLPARAMETERS);
}

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A constant reference to another GTestIndividual3 object
 */
GTestIndividual3::GTestIndividual3(const GTestIndividual3 &cp)
  : gpar::GFlatGenome(cp) { /* nothing */
}

/******************************************************************************/
/**
 * The standard destructor
 */
GTestIndividual3::~GTestIndividual3() { /* nothing */
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GTestIndividual3 object
 * @param e The expected outcome of the comparison
 */
void GTestIndividual3::compare_(
    const gpar::GOptimizableEntity &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;
    using namespace Gem::Geneva;

    // Check that we are dealing with a GTestIndividual3 reference independent of this object and convert the pointer
    const GTestIndividual3 *p_load =
        Gem::Common::g_convert_and_compare<gpar::GOptimizableEntity, GTestIndividual3>(cp, this);

    Gem::Common::GToken token("GTestIndividual3", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<gpar::GFlatGenome>(*this, *p_load, token);

    // ...no local data

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Loads the data of another GTestIndividual3, camouflaged as a GFlatGenome.
 *
 * @param cp A copy of another GTestIndividual3, camouflaged as a GFlatGenome
 */
void GTestIndividual3::load_(const gpar::GOptimizableEntity *cp) {
    using namespace Gem::Common;
    using namespace Gem::Geneva;

    // Check that we are dealing with a GTestIndividual3 reference independent of this object and convert the pointer
    const GTestIndividual3 *p_load =
        Gem::Common::g_convert_and_compare<gpar::GOptimizableEntity, GTestIndividual3>(cp, this);

    // Load our parent's data
    gpar::GFlatGenome::load_(cp);

    // no local data
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GFlatGenome
 */
gpar::GFlatGenome *GTestIndividual3::clone_() const {
    return new GTestIndividual3(*this);
}

/******************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @return The value of this object
 */
double GTestIndividual3::fitnessCalculation() {
    double result = 0.;

    // We just calculate the square of all double values
    std::vector<double> par_vec;
    this->streamline(par_vec);

    // Calculate the value of the parabola
    for(std::size_t i = 0; i < par_vec.size(); i++) {
        result += Gem::Common::gsquared(par_vec[i]);
    }

    return result;
}

/******************************************************************************/
/**
 * Get all data members of this class as a plain array
 */
std::shared_ptr<float> GTestIndividual3::getPlainData() const {
    using namespace Gem::Geneva;

    // The flat genome stores the 10 doubles of each record contiguously, in the same order the tree's
    // nested GParameterObjectCollection streamlined them (a[0], a[1], b, c[0..2], d[0..2], e). So record
    // i's field k sits at flat position i*10 + k -- a straight positional copy, no per-field decoding.
    std::vector<double> par_vec;
    this->streamline<double>(par_vec);

#ifdef DEBUG
    if(par_vec.size() != 10 * GTI_DEF_NITEMS) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GTestIndividual3::getPlainData(): Error!" << '\n'
            << "Invalid number of double parameters " << par_vec.size() << " / " << (10 * GTI_DEF_NITEMS)
            << '\n'
        );
    }
#endif /* DEBUG */

    // Note that we need to provide a deleter as we are dealing with an array. See e.g. http://stackoverflow.com/questions/13061979/shared-ptr-to-an-array-should-it-be-used
    std::shared_ptr<float> result(new float[10 * GTI_DEF_NITEMS], [](float *p) { delete[] p; });
    for(std::size_t m = 0; m < 10 * GTI_DEF_NITEMS; m++) {
        (result.get())[m] = Gem::Common::narrow<float>(par_vec[m]);
    }

    // Let the audience know
    return result;
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GTestIndividual3::modify_GUnitTests_() {
#ifdef GEM_TESTING

    bool result = false;

    // Call the parent classes' functions
    if(gpar::GFlatGenome::modify_GUnitTests_()) {
        result = true;
    }

    // Change the parameter settings
    result = true;

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GTestIndividual3::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GTestIndividual3::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    using namespace Gem::Geneva;

    // Call the parent classes' functions
    gpar::GFlatGenome::specificTestsNoFailureExpected_GUnitTests_();

    constexpr std::size_t ntests = 100;

    //------------------------------------------------------------------------------

    { // Test that repeated extraction of an object's data results in the same output
        std::shared_ptr<GTestIndividual3> p;
        std::shared_ptr<float> result_old;
        std::shared_ptr<float> result_new;

        CHECK_NOTHROW(p = std::make_shared<GTestIndividual3>());
        CHECK_NOTHROW(result_old = p->getPlainData());
        for(std::size_t i = 0; i < ntests; i++) {
            CHECK_NOTHROW(result_new = p->getPlainData());
            for(std::size_t m = 0; m < GTI_DEF_NITEMS * 10; m++) {
                CHECK((result_old.get())[m] == (result_new.get())[m]);
            }
        }
    }

    //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GTestIndividual3::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GTestIndividual3::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    using namespace Gem::Geneva;

    // Call the parent classes' functions
    gpar::GFlatGenome::specificTestsFailuresExpected_GUnitTests_();

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GTestIndividual3::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::Individuals */
