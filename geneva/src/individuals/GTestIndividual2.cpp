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

#include "geneva/individuals/GTestIndividual2.hpp"
#include "common/GCommonEnums.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include "geneva/oa/GAdaption.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"
#include <cstddef>
#include <istream>
#include <memory>
#include <ostream>
#include <vector>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Individuals::GTestIndividual2) // NOLINT
namespace Gem::Geneva::Individuals {

/******************************************************************************/
/**
 * Puts a Gem::Geneva::Individuals::PERFOBJECTTYPE item into a stream
 *
 * @param o The ostream the item should be added to
 * @param lt the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::Individuals::PERFOBJECTTYPE &lt) {
    Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(lt);
    o << tmp;
    return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::Individuals::PERFOBJECTTYPE item from a stream
 *
 * @param i The stream the item should be read from
 * @param lt The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::Individuals::PERFOBJECTTYPE &lt) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;

#ifdef DEBUG
    lt = Gem::Common::narrow<Gem::Geneva::Individuals::PERFOBJECTTYPE>(tmp);
#else
    lt = static_cast<Gem::Geneva::Individuals::PERFOBJECTTYPE>(tmp);
#endif /* DEBUG */

    return i;
}

/******************************************************************************/
/**
 * The default constructor -- private, as it is only needed for (de-)serialization purposes
 */
GTestIndividual2::GTestIndividual2() { /* nothing */
}

/******************************************************************************/
/**
 * The standard constructor
 */
GTestIndividual2::GTestIndividual2(const std::size_t &n_objects, const PERFOBJECTTYPE &otype) {
    using namespace Gem::Geneva;

    gpar::GGenomeBuilder b;

    // Fill with the requested amount of data of the requested type
    switch(otype) {
    case PERFOBJECTTYPE::PERFGDOUBLEOBJECT: {
        // n unbounded double scalars, each with its own Gauss adaptor (GDoubleObject + adaptor).
        for(std::size_t i = 0; i < n_objects; i++) {
            b.addDouble(0.);
        }
    } break;

    case PERFOBJECTTYPE::PERFGCONSTRDOUBLEOBJECT: {
        // n constrained double scalars [-10, 10], each with its own Gauss adaptor.
        for(std::size_t i = 0; i < n_objects; i++) {
            b.addDouble(0., -10., 10.);
        }
    } break;

    case PERFOBJECTTYPE::PERFGCONSTRAINEDDOUBLEOBJECTCOLLECTION: {
        // n constrained double parameters [-10, 10], each its own adaption group
        // (GConstrainedDoubleObjectCollection of GConstrainedDoubleObject, each with own adaptor).
        b.addDoubleArray(n_objects, -10., 10.);
    } break;

    case PERFOBJECTTYPE::PERFGDOUBLECOLLECTION: {
        // n unbounded double parameters sharing one adaptor (GDoubleCollection).
        b.addDoublePlainGroup(n_objects, -10., 10.);
    } break;

    case PERFOBJECTTYPE::PERFGCONSTRAINEDDOUBLECOLLECTION: {
        // n constrained double parameters [-10, 10] sharing one adaptor (GConstrainedDoubleCollection).
        b.addDoubleGroup(n_objects, -10., 10.);
    } break;

    default: {
        glogger << "In GTestIndividual2::GTestIndividual2(): Error!" << '\n'
                << "Invalid object type requested: " << otype << '\n'
                << GTERMINATION;
    } break;
    }

    this->setGenome(b.build());
}

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A constant reference to another GTestIndividual2 object
 */
GTestIndividual2::GTestIndividual2(const GTestIndividual2 &cp)
  : gpar::GFlatGenome(cp) { /* nothing */
}

/******************************************************************************/
/**
 * The standard destructor
 */
GTestIndividual2::~GTestIndividual2() { /* nothing */
}

/******************************************************************************/
/**
 * Builds the OA-owned adaption configuration: every double group of this genome gets the Gauss adaptor the
 * constructor formerly baked into the layout (sigma 0.025 / sigma_sigma 0.1 / [0, 1] / ad_prob 1).
 */
std::shared_ptr<OptimizationAlgorithms::GAdaptionConfigBase> GTestIndividual2::getAdaptionConfig() const {
    namespace oa = Gem::Geneva::OptimizationAlgorithms;
    auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(*this);
    for(std::size_t i = 0; i < cfg->doubleGroups().size(); i++) {
        cfg->groupDouble(i).gauss(0.025, 0.1, 0., 1., 1.);
    }
    return cfg;
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GFlatGenome object
 * @param e The expected outcome of the comparison
 */
void GTestIndividual2::compare_(
    const gpar::GOptimizableEntity &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;
    using namespace Gem::Geneva;

    // Check that we are dealing with a GTestIndividual2 reference independent of this object and convert the pointer
    const GTestIndividual2 *p_load =
        Gem::Common::g_convert_and_compare<gpar::GOptimizableEntity, GTestIndividual2>(cp, this);

    Gem::Common::GToken token("GTestIndividual2", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<gpar::GFlatGenome>(*this, *p_load, token);

    // ...no local data

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Loads the data of another GTestIndividual2, camouflaged as a GFlatGenome.
 *
 * @param cp A copy of another GTestIndividual2, camouflaged as a GFlatGenome
 */
void GTestIndividual2::load_(const gpar::GOptimizableEntity *cp) {
    using namespace Gem::Common;
    using namespace Gem::Geneva;

    // Check that we are dealing with a GTestIndividual2 reference independent of this object and convert the pointer
    const GTestIndividual2 *p_load =
        Gem::Common::g_convert_and_compare<gpar::GOptimizableEntity, GTestIndividual2>(cp, this);

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
gpar::GFlatGenome *GTestIndividual2::clone_() const {
    return new GTestIndividual2(*this);
}

/******************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @return The value of this object
 */
double GTestIndividual2::fitnessCalculation() {
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
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GTestIndividual2::modify_GUnitTests_() {
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
    Gem::Common::condnotset("GTestIndividual2::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GTestIndividual2::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    using namespace Gem::Geneva;

    // Call the parent classes' functions
    gpar::GFlatGenome::specificTestsNoFailureExpected_GUnitTests_();

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GTestIndividual2::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GTestIndividual2::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    using namespace Gem::Geneva;

    // Call the parent classes' functions
    gpar::GFlatGenome::specificTestsFailuresExpected_GUnitTests_();

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GTestIndividual2::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::Individuals */
