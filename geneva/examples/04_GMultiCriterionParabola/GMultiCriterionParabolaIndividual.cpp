/**
 * @file GMultiCriterionParabolaIndividual.cpp
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

#include "GMultiCriterionParabolaIndividual.hpp"

#include "geneva/oa/GAdaption.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GMultiCriterionParabolaIndividual) // NOLINT
namespace Gem::Geneva {
/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
     * Provide an easy way to print the individual's content
     */
std::ostream &operator<<(std::ostream &s, const Gem::Geneva::GMultiCriterionParabolaIndividual &f) {
    std::vector<double> parVec;
    f.streamline(parVec);

    for(std::size_t i = 0; i < f.getNStoredResults(); i++) {
        std::cout << "Raw fitness " << i << ": " << f.raw_fitness(i) << '\n';
    }

    std::vector<double>::iterator it;
    for(it = parVec.begin(); it != parVec.end(); ++it) {
        std::cout << std::distance(parVec.begin(), it) << ": " << *it << '\n';
    }

    return s;
}

/******************************************************************************/
/**
     * Provide an easy way to print the individual's content through a smart-pointer
     */
std::ostream &operator<<(
    std::ostream &s,
    const std::shared_ptr<Gem::Geneva::GMultiCriterionParabolaIndividual> &f_ptr
) {
    return operator<<(s, *f_ptr);
}

/******************************************************************************/
/**
     * Assigns a number of minima to this object
     */
void GMultiCriterionParabolaIndividual::setMinima(const std::vector<double> &minima) {
#ifdef DEBUG
    if(minima.size() != this->getNStoredResults()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GMultiCriterionParabolaIndividual::setMinima(...): Error!" << '\n'
            << "Invalid size of minima vector. Expected " << this->getNStoredResults() << '\n'
            << "but got " << minima.size() << '\n'
        );
    }
#endif /* DEBUG */

    minima_ = minima;
}

/******************************************************************************/
/**
     * Loads the data of another GMultiCriterionParabolaIndividual, camouflaged as a GFlatGenome.
     *
     * @param cp A copy of another GMultiCriterionParabolaIndividual, camouflaged as a GFlatGenome
     */
void GMultiCriterionParabolaIndividual::load_(const gen::GOptimizableEntity *cp) {
    // Check that we are dealing with a GMultiCriterionParabolaIndividual reference independent of this object and convert the pointer
    const GMultiCriterionParabolaIndividual *p_load =
        Gem::Common::g_convert_and_compare<gen::GOptimizableEntity, GMultiCriterionParabolaIndividual>(cp, this);

    // Load our parent's data ...
    gen::GFlatGenome::load_(cp);

#ifdef DEBUG
    if((p_load->minima_).size() != minima_.size() ||
       (p_load->minima_).size() != this->getNStoredResults()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GMultiCriterionParabolaIndividual::setMinima(...): Error!" << '\n'
            << "Invalid size of minima vector. Expected " << minima_.size() << "/"
            << this->getNStoredResults() << '\n'
            << "but got " << (p_load->minima_).size() << '\n'
        );
    }
#endif /* DEBUG */

    // Load local data
    minima_ = p_load->minima_;
}

/******************************************************************************/
/**
     * Creates a deep clone of this object
     *
     * @return A deep clone of this object, camouflaged as a GFlatGenome
     */
gen::GFlatGenome *GMultiCriterionParabolaIndividual::clone_() const {
    return new GMultiCriterionParabolaIndividual(*this);
}

/******************************************************************************/
/**
     * The actual fitness calculation takes place here.
     *
     * @return The value of this object
     */
double GMultiCriterionParabolaIndividual::fitnessCalculation() {
    double main_result = 0.;    // Will hold the main result
    std::vector<double> parVec; // Will hold the individual parameters

    this->streamline(parVec); // Retrieve the parameters

    // Do the actual calculations. Note that the first calculation
    // counts as the main result and that we can register other,
    // secondary evaluation criteria.
    main_result = Gem::Common::gsquared(parVec[0] - minima_[0]);
    for(std::size_t i = 1; i < parVec.size(); i++) {
        setResult(i, Gem::Common::gsquared(parVec[i] - minima_[i]));
    }

    return main_result;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Registers the config-file options, binding them to the passed Config. This is the body of the former
 * GMultiCriterionParabolaIndividualFactory::describeLocalOptions_ (now binding plain Config fields instead
 * of GOneTimeRefParameterT references).
 */
void GMultiCriterionParabolaIndividual::describeConfig(Gem::Common::GParserBuilder &gpb, Config &c) {
    std::string comment;

    comment = "";
    comment += "The lower boundary of the parabola;";
    gpb.registerFileParameter<double>(
        "par_min", c.par_min, c.par_min, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "The upper boundary of the parabola;";
    gpb.registerFileParameter<double>(
        "par_max", c.par_max, c.par_max, Gem::Common::VAR_IS_ESSENTIAL, comment
    );

    comment = "";
    comment += "A list of optima, encoded as a string;";
    gpb.registerFileParameter<std::string>(
        "minima", c.minima, c.minima, Gem::Common::VAR_IS_ESSENTIAL, comment
    );
}

/******************************************************************************/
/**
 * Builds the flat genome's STRUCTURE only (the genome-building body of the former
 * GMultiCriterionParabolaIndividualFactory::postProcess_): one constrained double per minimum, in
 * [par_min, par_max]. The start value is the lower perimeter; the optimization algorithm
 * random-initialises within bounds. The Gauss adaptor settings live on the OA-owned config (see
 * getAdaptionConfig()), not in the structure-only genome layout.
 */
gen::GenomeData GMultiCriterionParabolaIndividual::buildGenome(const Config &c) {
    const std::vector<double> minima = Gem::Common::stringToDoubleVec(c.minima);

    gen::GGenomeBuilder b;
    for(std::size_t npar = 0; npar < minima.size(); npar++) {
        // structure only; the adaptor lives on the OA config (see getAdaptionConfig())
        b.addDouble(c.par_min, c.par_min, c.par_max);
    }
    return b.build();
}

/******************************************************************************/
/**
 * Per-object post-config hook (the per-object body of the former postProcess_): the number of evaluation
 * criteria equals the number of parabolas (= the number of minima), and the per-criterion minima are
 * stored on the individual for fitnessCalculation().
 */
void GMultiCriterionParabolaIndividual::applyConfig(
    GMultiCriterionParabolaIndividual &ind,
    const Config &c
) {
    const std::vector<double> minima = Gem::Common::stringToDoubleVec(c.minima);
    ind.setNStoredResults(minima.size());
    ind.setMinima(minima);
}

/******************************************************************************/
/**
 * Builds the OA-owned adaption configuration for this genome: each of the nPar_ double parameters is its
 * own Gauss group, configured with the default GDoubleGaussAdaptor settings the tree relied upon. The
 * adaptor settings live on the OA-owned config, not in the shared, structure-only genome layout.
 */
std::shared_ptr<OptimizationAlgorithms::GAdaptionConfigBase>
GMultiCriterionParabolaIndividual::getAdaptionConfig() const {
    auto cfg = OptimizationAlgorithms::makeAdaptionConfig<OptimizationAlgorithms::GAdaptionConfigBase>(*this);
    for(std::size_t npar = 0; npar < cfg->doubleGroups().size(); npar++) {
        cfg->groupDouble(npar).gauss(
            DEFAULTSIGMA, DEFAULTSIGMASIGMA, DEFAULTMINSIGMA, DEFAULTMAXSIGMA, DEFAULTADPROB
        );
    }
    return cfg;
}

/******************************************************************************/
} // namespace Gem::Geneva
