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
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
     * The standard constructor. Initialization with the number of fitness
     * criteria, so GFlatGenome can set up its internal data structures.
     * This is the only "real" constructor, apart from the copy constructor.
     */
GMultiCriterionParabolaIndividual::GMultiCriterionParabolaIndividual(
    const std::size_t &nFitnessCriteria
)
  : gpar::GFlatGenome(nFitnessCriteria)
  , minima_(nFitnessCriteria) {
    /* nothing */
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
void GMultiCriterionParabolaIndividual::load_(const gpar::GOptimizableEntity *cp) {
    // Check that we are dealing with a GMultiCriterionParabolaIndividual reference independent of this object and convert the pointer
    const GMultiCriterionParabolaIndividual *p_load =
        Gem::Common::g_convert_and_compare<gpar::GOptimizableEntity, GMultiCriterionParabolaIndividual>(cp, this);

    // Load our parent's data ...
    gpar::GFlatGenome::load_(cp);

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
gpar::GFlatGenome *GMultiCriterionParabolaIndividual::clone_() const {
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
     * The standard constructor for this class
     *
     * @param cF The name of the configuration file
     */
GMultiCriterionParabolaIndividualFactory::GMultiCriterionParabolaIndividualFactory(
    std::filesystem::path const &cF
)
  : Gem::Common::GFactoryT<gpar::GOptimizableEntity>(cF)
  , par_min_(-10.)
  , par_max_(10.)
  , minima_string_("-1., 0., 1.")
  , nPar_(NPAR_MC) // The actual number will be determined by the external configuration file
  , firstParsed_(true) {
    /* nothing */
}

/******************************************************************************/
/**
     * The destructor
     */
GMultiCriterionParabolaIndividualFactory::~GMultiCriterionParabolaIndividualFactory() {
    /* nothing */
}

/******************************************************************************/
/**
     * Allows to describe configuration options in derived classes
     */
void GMultiCriterionParabolaIndividualFactory::describeLocalOptions_(
    Gem::Common::GParserBuilder &gpb
) {
    std::string comment;

    comment = "";
    comment += "The lower boundary of the parabola;";
    gpb.registerFileParameter(
        "par_min",
        par_min_.reference(),
        par_min_.value(),
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The upper boundary of the parabola;";
    gpb.registerFileParameter(
        "par_max",
        par_max_.reference(),
        par_max_.value(),
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "A list of optima, encoded as a string;";
    gpb.registerFileParameter(
        "minima",
        minima_string_.reference(),
        minima_string_.value(),
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
}

/******************************************************************************/
/**
     * Creates individuals of the desired type. The argument "id" gives the function a means
     * of detecting how often it has been called before. The id will be incremented for each call.
     * This can e.g. be used to act differently for the first call to this function.
     *
     * @param id The id of the individual to be created
     * @return An individual of the desired type
     */
std::shared_ptr<gpar::GOptimizableEntity> GMultiCriterionParabolaIndividualFactory::getObject_(
    Gem::Common::GParserBuilder &gpb,
    const std::size_t &id
) {
    // Will hold the result
    std::shared_ptr<GMultiCriterionParabolaIndividual> target(
        new GMultiCriterionParabolaIndividual()
    );

    // Make the object's local configuration options known
    target->addConfigurationOptions(gpb);

    return target;
}

/******************************************************************************/

void GMultiCriterionParabolaIndividualFactory::postProcess_(
    std::shared_ptr<gpar::GOptimizableEntity> &p_base
) {
    std::shared_ptr<GMultiCriterionParabolaIndividual> p =
        Gem::Common::convertSmartPointer<gpar::GOptimizableEntity, GMultiCriterionParabolaIndividual>(p_base);

    if(firstParsed_) {
        minima_ = Gem::Common::stringToDoubleVec(minima_string_);
        nPar_ = minima_.size();

        firstParsed_ = false;
    }

    p->setNStoredResults(nPar_);

    // Build a flat genome of nPar_ constrained doubles in [par_min_, par_max_], each its own Gauss
    // group with the default GDoubleGaussAdaptor configuration (the tree relied on the lazily installed
    // default adaptor: sigma 0.025 / sigma_sigma 0.2 / [0.001, 1] / ad_prob 1).
    gpar::GGenomeBuilder b;
    for(std::size_t npar = 0; npar < nPar_; npar++) {
        // structure only; the adaptor lives on the OA config (see getAdaptionConfig())
        b.addDouble(par_min_.value(), par_min_.value(), par_max_.value());
    }
    dynamic_cast<gpar::GFlatGenome &>(*p).setGenome(b.build());

    // Mirror the tree's per-parameter random initialization within bounds.
    p->randomInit(activityMode::ALLPARAMETERS);

    p->setMinima(minima_);
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
