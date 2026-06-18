/**
 * @file GMPIEvaluatedIndividual.cpp
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

#include "GMPIEvaluatedIndividual.hpp"

#include "geneva/oa/GAdaption.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GMPIEvaluatedIndividual) // NOLINT
namespace Gem::Geneva {

MPI_Comm GMPIEvaluatedIndividual::communicator{MPI_COMM_NULL};
/********************************************************************************************/
/**
 * The default constructor. This function will add two double parameters to this individual,
 * each of which has a constrained value range [-10:10].
 */
GMPIEvaluatedIndividual::GMPIEvaluatedIndividual()
  : gen::GFlatGenome()
  , M_PAR_MIN(-10.)
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
 * A standard copy constructor. All real work is done by the parent class.
 *
 * @param cp A copy of another GMPIEvaluatedIndividual
 */
GMPIEvaluatedIndividual::GMPIEvaluatedIndividual(const GMPIEvaluatedIndividual &cp)
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
GMPIEvaluatedIndividual::~GMPIEvaluatedIndividual() { /* nothing */
}

/**
 * Allows to set the communicator which is used inside of the fitness calculation function to communicate with sub-clients
 * @param c the new communicator
 */
void GMPIEvaluatedIndividual::setCommunicator(MPI_Comm c) {
    GMPIEvaluatedIndividual::communicator = c;
}

/********************************************************************************************/
/**
 * Loads the data of another GMPIEvaluatedIndividual, camouflaged as a GFlatGenome.
 *
 * @param cp A copy of another GMPIEvaluatedIndividual, camouflaged as a GFlatGenome
 */
void GMPIEvaluatedIndividual::load_(const gen::GOptimizableEntity *cp) {
    // Check that we are dealing with a GMPIEvaluatedIndividual reference independent of this object and convert the pointer
    const GMPIEvaluatedIndividual *p_load =
        Gem::Common::g_convert_and_compare<gen::GOptimizableEntity, GMPIEvaluatedIndividual>(cp, this);

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
gen::GFlatGenome *GMPIEvaluatedIndividual::clone_() const {
    return new GMPIEvaluatedIndividual(*this);
}

/********************************************************************************************/
/**
 * Builds the OA-owned adaption configuration: each of the two double parameters is its own Gauss group,
 * configured with the default GDoubleGaussAdaptor settings. The adaptor settings live solely on the
 * returned (OA-owned) config -- none reside on the individual.
 *
 * @param sample A sample flat genome whose group structure the config mirrors
 * @return A shared pointer to the populated OA-owned adaption config
 */
std::shared_ptr<OptimizationAlgorithms::GAdaptionConfigBase>
GMPIEvaluatedIndividual::buildAdaptionConfig(const gen::GFlatGenome &sample) {
    auto cfg = OptimizationAlgorithms::makeAdaptionConfig<OptimizationAlgorithms::GAdaptionConfigBase>(sample);
    for(std::size_t npar = 0; npar < cfg->doubleGroups().size(); npar++) {
        cfg->groupDouble(npar).gauss(DEFAULTSIGMA, DEFAULTSIGMASIGMA, DEFAULTMINSIGMA, DEFAULTMAXSIGMA, DEFAULTADPROB);
    }
    return cfg;
}

/********************************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @return The value of this object
 */
double GMPIEvaluatedIndividual::fitnessCalculation() {
    double result = 0.;         // Will hold the result
    std::vector<double> parVec; // Will hold the parameters

    this->streamline(parVec); // Retrieve the parameters

    // communicate with the sub-clients. In this example just use a useless barrier
    MPI_Barrier(communicator);

    // Do the actual calculation
    for(auto const &d : parVec) {
        result += d * d;
    }

    return result;
}

/********************************************************************************************/

} /* namespace Gem */
