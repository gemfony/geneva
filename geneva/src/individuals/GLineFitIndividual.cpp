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

#include "geneva/individuals/GLineFitIndividual.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExpectationChecksT.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include "geneva/oa/GAdaption.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <tuple>
#include <vector>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Individuals::GLineFitIndividual) // NOLINT
namespace Gem::Geneva::Individuals {

/******************************************************************************/
/**
 * @brief The default constructor -- private, as it is only needed for (de-)serialization purposes.
 */
GLineFitIndividual::GLineFitIndividual() { /* nothing */
}

/******************************************************************************/
/**
 * @brief The standard constructor; builds the two-parameter (offset/slope) genome structure and
 * stores the data points to be fitted.
 *
 * @param data_points The set of (x, y) data points a line should be fitted through; stored for the
 *        fitness calculation
 */
GLineFitIndividual::GLineFitIndividual(const std::vector<std::tuple<double, double>> &data_points)
  : data_points_(data_points) {
    using namespace Gem::Geneva;

    // Two unbounded double parameters (the line's offset a and slope b), each its own Gauss group,
    // with a default unbounded init range of [0, 1]. The Gauss adaptor settings live on the OA-owned
    // config (see getAdaptionConfig()), not the genome layout.
    gen::GGenomeBuilder b;
    for(std::size_t i = 0; i < 2; i++) {
        b.addDouble(0.);
    }
    this->setGenome(b.build());
}

/******************************************************************************/
/**
 * @brief Builds the OA-owned adaption configuration: the line's offset a and slope b are each their own
 * Gauss group, configured with the settings below.
 *
 * @return A shared pointer to a freshly built adaption config whose double groups each carry the Gauss
 *         adaptor settings (sigma 0.025, sigma_sigma 0.1, min_sigma 0.0001, max_sigma 0.4, ad_prob 1.0)
 */
std::shared_ptr<OptimizationAlgorithms::GAdaptionConfigBase> GLineFitIndividual::getAdaptionConfig() const {
    auto cfg = OptimizationAlgorithms::makeAdaptionConfig<OptimizationAlgorithms::GAdaptionConfigBase>(*this);
    for(std::size_t i = 0; i < cfg->doubleGroups().size(); i++) {
        // sigma, sigma_sigma, min_sigma, max_sigma, ad_prob
        cfg->groupDouble(i).gauss(0.025, 0.1, 0.0001, 0.4, 1.);
    }
    return cfg;
}

/******************************************************************************/
/**
 * @brief The copy constructor.
 *
 * @param cp A constant reference to another GLineFitIndividual object to be copied
 */
GLineFitIndividual::GLineFitIndividual(const GLineFitIndividual &cp)
  : gen::GFlatGenome(cp)
  , data_points_(cp.data_points_) { /* nothing */
}

/******************************************************************************/
/**
 * @brief The standard destructor.
 */
GLineFitIndividual::~GLineFitIndividual() { /* nothing */
}

/******************************************************************************/
/**
 * @brief Searches for compliance with expectations with respect to another object of the same type.
 *
 * @param cp A constant reference to another object, camouflaged as a GOptimizableEntity, to compare against
 * @param e The expected outcome of the comparison (equality, inequality, ...)
 * @param limit The maximum acceptable deviation for (floating point) comparisons (unused here)
 */
void GLineFitIndividual::compare_(
    const gen::GOptimizableEntity &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GLineFitIndividual reference independent of this object and convert the pointer
    const GLineFitIndividual *p_load =
        Gem::Common::g_convert_and_compare<gen::GOptimizableEntity, GLineFitIndividual>(cp, this);

    GToken token("GLineFitIndividual", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<gen::GFlatGenome>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(localMembers(), p_load->localMembers(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * @brief Retrieves the tuple (a, b) of the line represented by this object.
 *
 * @return A tuple holding the line's offset a (first parameter) and slope b (second parameter)
 */
std::tuple<double, double> GLineFitIndividual::getLine() const {
    std::vector<double> par_vec;
    this->streamline(par_vec);
    return std::tuple<double, double>(par_vec.at(0), par_vec.at(1));
}

/******************************************************************************/
/**
 * @brief Loads the data of another GLineFitIndividual, camouflaged as a GOptimizableEntity.
 *
 * @param cp A pointer to another GLineFitIndividual, camouflaged as a GOptimizableEntity, to load from
 */
void GLineFitIndividual::load_(const gen::GOptimizableEntity *cp) {
    using namespace Gem::Common;
    using namespace Gem::Geneva;

    // Check that we are dealing with a GLineFitIndividual reference independent of this object and convert the pointer
    const GLineFitIndividual *p_load =
        Gem::Common::g_convert_and_compare<gen::GOptimizableEntity, GLineFitIndividual>(cp, this);

    // Load our parent's data
    gen::GFlatGenome::load_(cp);

    // and then our local data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/******************************************************************************/
/**
 * @brief Creates a deep clone of this object.
 *
 * @return A deep clone of this object, camouflaged as a GFlatGenome pointer
 */
gen::GFlatGenome *GLineFitIndividual::clone_() const {
    return new GLineFitIndividual(*this);
}

/******************************************************************************/
/**
 * @brief The actual fitness calculation takes place here; computes the root of the summed squared
 * deviation between the fitted line (a + b*x) and the stored data points.
 *
 * @return The fitness of this object: sqrt of the summed squared deviations of line and data points
 */
double GLineFitIndividual::fitnessCalculation() {
    double result = 0.;

    // We just calculate the square of all double values
    std::vector<double> par_vec;
    this->streamline(par_vec);

    double a = par_vec.at(0);
    double b = par_vec.at(1);

    // Sum up the square deviation of line and data points
    double deviation = 0.;
    for(const auto &data_point : data_points_) {
        deviation = (a + b * std::get<0>(data_point)) - std::get<1>(data_point);
        result += Gem::Common::gsquared(deviation);
    }

    return sqrt(result);
}

/******************************************************************************/
/**
 * @brief Applies modifications to this object. This is needed for testing purposes.
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GLineFitIndividual::modify_GUnitTests_() {
#ifdef GEM_TESTING

    bool result = false;

    // Call the parent classes' functions
    if(gen::GFlatGenome::modify_GUnitTests_()) {
        result = true;
    }

    // Change the parameter settings
    result = true;

    return result;
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("GLineFitIndividual::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to succeed. This is needed for testing purposes.
 */
void GLineFitIndividual::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    using namespace Gem::Geneva;

    // Call the parent classes' functions
    gen::GFlatGenome::specificTestsNoFailureExpected_GUnitTests_();

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GLineFitIndividual::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * @brief Performs self tests that are expected to fail. This is needed for testing purposes.
 */
void GLineFitIndividual::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    using namespace Gem::Geneva;

    // Call the parent classes' functions
    gen::GFlatGenome::specificTestsFailuresExpected_GUnitTests_();

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GLineFitIndividual::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Individuals */
