/**
 * @file GParaboloidIndividual2D.hpp
 */

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

#include "GParaboloidIndividual2D.hpp"

#include "geneva/oa/GAdaption.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GParaboloidIndividual2D) // NOLINT
namespace Gem::Geneva {

/********************************************************************************************/
/**
 * The default constructor. This function will add two double parameters to this individual,
 * each of which has a constrained value range [-10:10].
 */
GParaboloidIndividual2D::GParaboloidIndividual2D()
  : M_PAR_MIN(-10.)
  , M_PAR_MAX(10.) {
    // Build a flat genome of two constrained doubles in [M_PAR_MIN, M_PAR_MAX[, each its own Gauss
    // group with the default GDoubleGaussAdaptor configuration (the tree relied on the lazily
    // installed default adaptor).
    gen::GGenomeBuilder b;
    for(std::size_t npar = 0; npar < 2; npar++) {
        b.addDouble(M_PAR_MIN, M_PAR_MIN, M_PAR_MAX); // structure only; the adaptor lives on the OA config
    }
    this->setGenome(b.build());

    // Mirror the tree's per-parameter random initialization within bounds.
    this->randomInit(activityMode::ALLPARAMETERS);
}

/********************************************************************************************/
/**
 * Builds the OA-owned adaption configuration for this genome: each of the two double parameters is its
 * own Gauss group, configured with the default GDoubleGaussAdaptor settings. The adaptor settings live
 * solely on the returned (OA-owned) config -- none reside on the individual.
 *
 * @param sample A sample flat genome whose group structure the config mirrors
 * @return A shared pointer to the populated OA-owned adaption config
 */
std::shared_ptr<OptimizationAlgorithms::GAdaptionConfigBase>
GParaboloidIndividual2D::buildAdaptionConfig(const gen::GFlatGenome &sample) {
    auto cfg = OptimizationAlgorithms::makeAdaptionConfig<OptimizationAlgorithms::GAdaptionConfigBase>(sample);
    for(std::size_t npar = 0; npar < 2; npar++) {
        cfg->groupDouble(npar).gauss(
            DEFAULTSIGMA, DEFAULTSIGMASIGMA, DEFAULTMINSIGMA, DEFAULTMAXSIGMA, DEFAULTADPROB
        );
    }
    return cfg;
}

/********************************************************************************************/
/**
 * A standard copy constructor. All real work is done by the parent class.
 *
 * @param cp A copy of another GParaboloidIndividual2D
 */
GParaboloidIndividual2D::GParaboloidIndividual2D(const GParaboloidIndividual2D &cp)
  : gen::GFlatGenome(cp)
  , M_PAR_MIN(-10.)
  , M_PAR_MAX(10) { /* nothing */
}

/********************************************************************************************/
/**
 * The standard destructor. Note that you do not need to care for the parameter objects
 * added in the constructor. Upon destruction, they will take care of releasing the allocated
 * memory.
 */
GParaboloidIndividual2D::~GParaboloidIndividual2D() { /* nothing */
}

/********************************************************************************************/
/**
 * Loads the data of another GParaboloidIndividual2D, camouflaged as a GFlatGenome.
 *
 * @param cp A copy of another GParaboloidIndividual2D, camouflaged as a GFlatGenome
 */
void GParaboloidIndividual2D::load_(const gen::GOptimizableEntity *cp) {
    // Check that we are dealing with a GParaboloidIndividual2D reference independent of this object and convert the pointer
    const GParaboloidIndividual2D *p_load =
        Gem::Common::g_convert_and_compare<gen::GOptimizableEntity, GParaboloidIndividual2D>(cp, this);

    // Load our parent's data
    gen::GFlatGenome::load_(cp);

    // No local data
    // sampleVariable = p_load->sampleVariable;
}

/********************************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GFlatGenome
 */
gen::GFlatGenome *GParaboloidIndividual2D::clone_() const {
    return new GParaboloidIndividual2D(*this);
}

/********************************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @return The value of this object
 */
double GParaboloidIndividual2D::fitnessCalculation() {
    double result = 0.;         // Will hold the result
    std::vector<double> parVec; // Will hold the parameters

    this->streamline(parVec); // Retrieve the parameters

    // Do the actual calculation
    for(auto const &d : parVec) {
        result += d * d;
    }

    return result;
}

/********************************************************************************************/

}
