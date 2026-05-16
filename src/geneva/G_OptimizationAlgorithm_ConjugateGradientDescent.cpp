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

#include "geneva/G_OptimizationAlgorithm_ConjugateGradientDescent.hpp"

#include <limits>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GConjugateGradientDescent) // NOLINT

namespace Gem::Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GConjugateGradientDescent::GConjugateGradientDescent()
  : GConjugateGradientDescent(
        DEFAULTCGDSTARTINGPOINTS,
        DEFAULTCGDFINITESTEP,
        DEFAULTCGDSTEPSIZE
    ) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with the number of starting points and step parameters
 *
 * @param nStartingPoints The number of simultaneous starting points
 * @param finiteStep The size of the difference-quotient step
 * @param stepSize The multiplicative factor for the step along the search direction
 */
GConjugateGradientDescent::GConjugateGradientDescent(
    const std::size_t &nStartingPoints,
    const double &finiteStep,
    const double &stepSize
)
  : G_OptimizationAlgorithm_Base()
  , nStartingPoints_(nStartingPoints)
  , finiteStep_(finiteStep)
  , stepSize_(stepSize) { /* nothing */
}

/******************************************************************************/
/**
 * Retrieves the number of starting points of the algorithm
 */
std::size_t GConjugateGradientDescent::getNStartingPoints() const {
    return nStartingPoints_;
}

/******************************************************************************/
/**
 * Allows to set the number of starting points for the conjugate gradient descent
 */
void GConjugateGradientDescent::setNStartingPoints(std::size_t nStartingPoints) {
    if(nStartingPoints == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GConjugateGradientDescent::setNStartingPoints(const std::size_t&):" << '\n'
            << "Got invalid number of starting points." << '\n'
        );
    }

    nStartingPoints_ = nStartingPoints;
}

/******************************************************************************/
/**
 * Set the size of the finite step of the difference quotient
 */
void GConjugateGradientDescent::setFiniteStep(double finiteStep) {
    if(finiteStep <= 0. ||
       finiteStep > 1000.) { // Specified in per mill of the allowed or preferred value range
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GConjugateGradientDescent::setFiniteStep(double): Error!" << '\n'
            << "Invalid value of finiteStep: " << finiteStep << '\n'
            << "Must be in the range ]0.:1000.]" << '\n'
        );
    }

    finiteStep_ = finiteStep;

    // Keep adjustedFiniteStep_ consistent if called after init()
    updateDerivedQuantities();
}

/******************************************************************************/
/**
 * Retrieve the size of the finite step of the difference quotient
 */
double GConjugateGradientDescent::getFiniteStep() const {
    return finiteStep_;
}

/******************************************************************************/
/**
 * Sets a multiplier for the step along the search direction
 */
void GConjugateGradientDescent::setStepSize(double stepSize) {
    if(stepSize <= 0. ||
       stepSize > 1000.) { // Specified in per mill of the allowed or preferred value range
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GConjugateGradientDescent::setStepSize(double): Error!" << '\n'
            << "Invalid value of stepSize: " << stepSize << '\n'
            << "Must be in the range ]0.:1000.]" << '\n'
        );
    }

    stepSize_ = stepSize;
}

/******************************************************************************/
/**
 * Retrieves the current step size
 */
double GConjugateGradientDescent::getStepSize() const {
    return stepSize_;
}

/******************************************************************************/
/**
 * Retrieve the number of processable items in the current iteration.
 */
std::size_t GConjugateGradientDescent::getNProcessableItems_() const {
    return this->size(); // The entire population is (re-)evaluated every iteration
}

/******************************************************************************/
/**
 * Returns information about the type of optimization algorithm
 */
std::string GConjugateGradientDescent::getAlgorithmPersonalityType_() const {
    return "PERSONALITY_CGD";
}

/******************************************************************************/
/**
 * Returns the name of this optimization algorithm
 */
std::string GConjugateGradientDescent::getAlgorithmName_() const {
    return std::string("Conjugate Gradient Descent");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 */
void GConjugateGradientDescent::compare_(
    const GObject &cp,
    const Gem::Common::expectation &e,
    const double & /*limit*/
) const {
    using namespace Gem::Common;

    const GConjugateGradientDescent *p_load =
        Gem::Common::g_convert_and_compare<GObject, GConjugateGradientDescent>(cp, this);

    GToken token("GConjugateGradientDescent", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<G_OptimizationAlgorithm_Base>(*this, *p_load, token);

    // ... and then the local data
    compare_t(IDENTITY(nStartingPoints_, p_load->nStartingPoints_), token);
    compare_t(IDENTITY(nFPParmsFirst_, p_load->nFPParmsFirst_), token);
    compare_t(IDENTITY(finiteStep_, p_load->finiteStep_), token);
    compare_t(IDENTITY(stepSize_, p_load->stepSize_), token);
    // dblLowerParameterBoundaries_, dblUpperParameterBoundaries_, adjustedFiniteStep_,
    // prevGradient_, prevDirection_ and cgHistoryValid_ are transient: recomputed in
    // init() from the serialized fields above and not restored in load_(). Comparing
    // them would cause round-trip equality tests to fail spuriously.

    token.evaluate();
}

/******************************************************************************/
/**
 * Resets the settings of this population to what was configured when
 * the optimize()-call was issued
 */
void GConjugateGradientDescent::resetToOptimizationStart_() {
    dblLowerParameterBoundaries_.clear();
    dblUpperParameterBoundaries_.clear();
    adjustedFiniteStep_.clear();
    prevGradient_.clear();
    prevDirection_.clear();
    cgHistoryValid_.clear();

    G_OptimizationAlgorithm_Base::resetToOptimizationStart_();
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GConjugateGradientDescent::name_() const {
    return std::string("GConjugateGradientDescent");
}

/******************************************************************************/
/**
 * Loads the data of another population
 */
void GConjugateGradientDescent::load_(const GObject *cp) {
    const GConjugateGradientDescent *p_load =
        Gem::Common::g_convert_and_compare<GObject, GConjugateGradientDescent>(cp, this);

    // First load the parent class'es data (this also copies all individuals).
    G_OptimizationAlgorithm_Base::load_(cp);

    // ... and then our own (serialized) data
    nStartingPoints_ = p_load->nStartingPoints_;
    nFPParmsFirst_ = p_load->nFPParmsFirst_;
    finiteStep_ = p_load->finiteStep_;
    stepSize_ = p_load->stepSize_;
    // adjustedFiniteStep_, dbl*ParameterBoundaries_, prevGradient_, prevDirection_,
    // cgHistoryValid_ are transient and recomputed in init().
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
GObject *GConjugateGradientDescent::clone_() const {
    return new GConjugateGradientDescent(*this);
}

/******************************************************************************/
/**
 * The actual business logic to be performed during each iteration.
 *
 * The structure mirrors GGradientDescent: the conjugate-gradient step uses
 * the difference quotients evaluated in the *previous* iteration (they are
 * still stored on the individuals), then the children are rebuilt around the
 * stepped parents and the whole population is re-evaluated.
 *
 * @return The value of the best individual found in this iteration
 */
std::tuple<double, double> GConjugateGradientDescent::cycleLogic_() {
    if(afterFirstIteration()) {
        // Perform a conjugate-gradient step for every starting point. This
        // only makes sense once the difference quotients have been evaluated.
        this->updateParentIndividuals();
    }

    // Rebuild the difference-quotient children around the (stepped) parents
    this->updateChildParameters();

    // Trigger value calculation for all individuals (parents + children)
    runFitnessCalculation_();

    std::tuple<double, double> bestFitness =
        std::make_tuple(this->at(0)->getWorstCase(), this->at(0)->getWorstCase());
    std::tuple<double, double> fitnessCandidate =
        std::make_tuple(this->at(0)->getWorstCase(), this->at(0)->getWorstCase());

    GConjugateGradientDescent::iterator it;
    auto m = this->at(0)->getMaxMode(); // All individuals share the same max mode
    for(it = this->begin(); it != this->begin() + this->getNStartingPoints(); ++it) {
        std::get<G_RAW_FITNESS>(fitnessCandidate) = (*it)->raw_fitness(0);
        std::get<G_TRANSFORMED_FITNESS>(fitnessCandidate) = (*it)->transformed_fitness(0);

        if(isBetter(
               std::get<G_TRANSFORMED_FITNESS>(fitnessCandidate),
               std::get<G_TRANSFORMED_FITNESS>(bestFitness),
               m
           )) {
            bestFitness = fitnessCandidate;
        }
    }

    return bestFitness;
}

/******************************************************************************/
/**
 * Rebuilds the difference-quotient children of every starting point. Identical
 * in spirit to GGradientDescent::updateChildParameters(): for starting point i
 * and direction j the child at position
 *
 *   nStartingPoints_ + i * nFPParmsFirst_ + j
 *
 * is a copy of parent i with its j-th active parameter incremented by the
 * (range-scaled) finite step. This produces a forward difference quotient.
 */
void GConjugateGradientDescent::updateChildParameters() {
    for(std::size_t i = 0; i < nStartingPoints_; i++) {
        std::vector<double> parmVec;
        this->at(i)->streamline<double>(parmVec, activityMode::ACTIVEONLY);

        for(std::size_t j = 0; j < nFPParmsFirst_; j++) {
            std::size_t childPos = nStartingPoints_ + i * nFPParmsFirst_ + j;

            // Load the current "parent" into the "child"
            this->at(childPos)->GObject::load(this->at(i));

            // Update the child's position in the population
            this->at(childPos)
                ->getPersonalityTraits<GConjugateGradientDescent_PersonalityTraits>()
                ->setPopulationPosition(childPos);

            double origParmVal = parmVec[j];

            // Add the finite step to the feature vector's current parameter
            parmVec[j] += adjustedFiniteStep_[j];
            this->at(childPos)->assignValueVector<double>(parmVec, activityMode::ACTIVEONLY);

            // Restore the original value for the next direction
            parmVec[j] = origParmVal;
        }
    }
}

/******************************************************************************/
/**
 * Performs a non-linear conjugate-gradient step for every starting point.
 *
 * For each starting point the (proxy) gradient component in direction j is the
 * forward difference
 *
 *   g_j = f(x + h_j e_j) - f(x)
 *
 * (the same proxy used by GGradientDescent, i.e. not divided by h_j, so the
 * effective scaling matches the plain gradient descent). The Polak-Ribière+
 * coefficient and the new conjugate search direction are
 *
 *   beta = max(0, g . (g - g_prev) / (g_prev . g_prev))
 *   d    = -g + beta * d_prev          (with d = -g on the first step or on a restart)
 *
 * and the parameter vector is moved by stepRatio * d, with
 * stepRatio = stepSize_ / finiteStep_ (identical to GGradientDescent). A
 * non-positive / numerically unstable denominator triggers an automatic
 * restart (beta = 0), which keeps the method globally convergent.
 */
void GConjugateGradientDescent::updateParentIndividuals() {
    const long double stepRatio =
        ((long double)stepSize_) / ((long double)finiteStep_);

    for(std::size_t i = 0; i < nStartingPoints_; i++) {
        std::vector<double> parmVec;
        this->at(i)->streamline<double>(parmVec, activityMode::ACTIVEONLY);

#ifdef DEBUG
        if(this->at(i)->is_due_for_processing() || (this->at(i)->has_errors())) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GConjugateGradientDescent::updateParentIndividuals():" << '\n'
                << "Found individual in position " << i
                << " which is unprocessed or has errors" << '\n'
            );
        }
#endif /* DEBUG */

        const double parentFitness = minOnly_transformed_fitness(this->at(i));

        // 1) Assemble the forward-difference gradient proxy g_j
        std::vector<double> gradient(nFPParmsFirst_, 0.);
        for(std::size_t j = 0; j < nFPParmsFirst_; j++) {
            std::size_t childPos = nStartingPoints_ + i * nFPParmsFirst_ + j;
            gradient[j] =
                minOnly_transformed_fitness(this->at(childPos)) - parentFitness;
        }

        // 2) Compute the Polak-Ribière+ beta and the conjugate direction
        std::vector<double> direction(nFPParmsFirst_, 0.);
        double beta = 0.;
        if(cgHistoryValid_[i]) {
            long double numerator = 0.L;   // g . (g - g_prev)
            long double denominator = 0.L; // g_prev . g_prev
            for(std::size_t j = 0; j < nFPParmsFirst_; j++) {
                numerator += (long double)gradient[j] *
                             ((long double)gradient[j] - (long double)prevGradient_[i][j]);
                denominator +=
                    (long double)prevGradient_[i][j] * (long double)prevGradient_[i][j];
            }
            // Numerical-stability guard for the division. denominator is
            // g_{k-1}.g_{k-1}, which becomes vanishingly small near
            // convergence. Dividing by a tiny (but strictly positive)
            // denominator would blow beta up and destabilise the search
            // direction. We therefore restart (beta = 0, i.e. a steepest-
            // descent step) unless the denominator is (a) above the absolute
            // representable floor and (b) large enough relative to the
            // numerator that the quotient stays below a finite conjugate-weight
            // cap. Condition (b), denominator * betaMax > |numerator|,
            // guarantees |betaPR| < betaMax by construction.
            constexpr long double betaMax = 1.0e4L;
            const long double absNum = (numerator >= 0.L) ? numerator : -numerator;
            if(denominator > std::numeric_limits<long double>::min() &&
               denominator * betaMax > absNum) {
                long double betaPR = numerator / denominator;
                beta = (betaPR > 0.L)
                           ? Gem::Common::narrow_cast<double>(betaPR)
                           : 0.; // PR+ clamp == automatic restart
            }
            else {
                // Degenerate / near-zero previous gradient, or a denominator
                // too small relative to the numerator -> restart.
                beta = 0.;
            }
        }

        for(std::size_t j = 0; j < nFPParmsFirst_; j++) {
            direction[j] = -gradient[j] +
                           (cgHistoryValid_[i] ? beta * prevDirection_[i][j] : 0.);
        }

        // 3) Take the step x <- x + stepRatio * d
        try {
            for(std::size_t j = 0; j < nFPParmsFirst_; j++) {
                parmVec[j] += Gem::Common::narrow_cast<double>(
                    stepRatio * (long double)direction[j]
                );
            }
        }
        catch(std::overflow_error &e) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GConjugateGradientDescent::updateParentIndividuals(): Error!" << '\n'
                << "Bad conversion with message " << e.what() << '\n'
            );
        }

        // 4) Remember gradient/direction for the next conjugate step
        prevGradient_[i] = gradient;
        prevDirection_[i] = direction;
        cgHistoryValid_[i] = true;

        // Write the stepped parameter vector back into the parent
        this->at(i)->assignValueVector<double>(parmVec, activityMode::ACTIVEONLY);
    }
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 */
void GConjugateGradientDescent::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    // Call our parent class'es function
    G_OptimizationAlgorithm_Base::addConfigurationOptions_(gpb);

    gpb.registerFileParameter<std::size_t>(
        "nStartingPoints",
        DEFAULTCGDSTARTINGPOINTS,
        [this](std::size_t nsp) { this->setNStartingPoints(nsp); }
    ) << "The number of simultaneous conjugate gradient descents";

    gpb.registerFileParameter<double>(
        "finiteStep",
        DEFAULTCGDFINITESTEP,
        [this](double fs) { this->setFiniteStep(fs); }
    ) << "The size of the adjustment in the difference quotient,"
      << '\n'
      << "specified in per mill of the allowed or expected value" << '\n'
      << "range of a parameter";

    gpb.registerFileParameter<double>(
        "stepSize",
        DEFAULTCGDSTEPSIZE,
        [this](double ss) { this->setStepSize(ss); }
    ) << "The size of each step along the conjugate search"
      << '\n'
      << "direction, specified in per mill of the allowed or" << '\n'
      << "expected value range of a parameter";
}

/******************************************************************************/
/**
 * Triggers fitness calculation of all individuals via the broker.
 */
void GConjugateGradientDescent::runFitnessCalculation_() {
    using namespace Gem::Courtier;

#ifdef DEBUG
    std::size_t pos = 0;
    for(const auto &item_ptr : *this) {
        if(this->afterFirstIteration() && !item_ptr->is_due_for_processing()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GConjugateGradientDescent::runFitnessCalculation():" << '\n'
                << "Found individual on position " << pos
                << " which is not due for processing" << '\n'
            );
        }
        pos++;
    }
#endif /* DEBUG */

    setProcessingFlag(this->data_cnt_, std::make_tuple(std::size_t(0), this->data_cnt_.size()));
    auto status = this->workOn(
        this->data_cnt_,
        true // resubmit unprocessed items
        ,
        "GConjugateGradientDescent::runFitnessCalculation()"
    );

    // A conjugate-gradient method needs a complete set of evaluated solutions.
    if(not status.is_complete || status.has_errors) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GConjugateGradientDescent::runFitnessCalculation(): Error!" << '\n'
            << "No complete set of items received or errors found in some individuals"
            << '\n'
        );
    }
}

/******************************************************************************/
/**
 * Does some preparatory work before the optimization starts
 */
void GConjugateGradientDescent::init() {
    // To be performed before any other action
    G_OptimizationAlgorithm_Base::init();

    // Extract the boundaries of all active parameters
    this->at(0)->boundaries(
        dblLowerParameterBoundaries_,
        dblUpperParameterBoundaries_,
        activityMode::ACTIVEONLY
    );

#ifdef DEBUG
    if(dblLowerParameterBoundaries_.size() != dblUpperParameterBoundaries_.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GConjugateGradientDescent::init(): Error!" << '\n'
            << "Found invalid sizes: " << dblLowerParameterBoundaries_.size() << " / "
            << dblUpperParameterBoundaries_.size() << '\n'
        );
    }

    if(stepSize_ <= 0. || stepSize_ > 1000.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GConjugateGradientDescent::init(): Error!" << '\n'
            << "Invalid value of stepSize_: " << stepSize_ << '\n'
            << "Must be in the range ]0.:1000.]" << '\n'
        );
    }

    if(finiteStep_ <= 0. || finiteStep_ > 1000.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GConjugateGradientDescent::init(): Error!" << '\n'
            << "Invalid value of finiteStep_: " << finiteStep_ << '\n'
            << "Must be in the range ]0.:1000.]" << '\n'
        );
    }
#endif /* DEBUG */

    updateDerivedQuantities();
    resetCGState();
    markIndividualPositions();
}

/******************************************************************************/
/**
 * Recomputes the per-parameter difference-quotient step from finiteStep_ and
 * the extracted parameter ranges. Before init() the boundary vectors are
 * empty, so adjustedFiniteStep_ is simply cleared and init() fills it once the
 * boundaries are known.
 */
void GConjugateGradientDescent::updateDerivedQuantities() {
    try {
        adjustedFiniteStep_.clear();
        long double finiteStepRatio = ((long double)finiteStep_) / ((long double)1000.);
        for(std::size_t pos = 0; pos < dblLowerParameterBoundaries_.size(); pos++) {
            long double parameterRange =
                (long double)dblUpperParameterBoundaries_[pos] -
                (long double)dblLowerParameterBoundaries_[pos];
            adjustedFiniteStep_.push_back(
                Gem::Common::narrow_cast<double>(finiteStepRatio * parameterRange)
            );
        }
    }
    catch(std::overflow_error &e) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GConjugateGradientDescent::updateDerivedQuantities(): Error!" << '\n'
            << "Bad conversion with message " << e.what() << '\n'
        );
    }
}

/******************************************************************************/
/**
 * (Re-)initialises the per-starting-point conjugate-gradient memory. Called
 * from init() once nFPParmsFirst_ is known (it is set in adjustPopulation_,
 * which runs before init()).
 */
void GConjugateGradientDescent::resetCGState() {
    prevGradient_.assign(nStartingPoints_, std::vector<double>(nFPParmsFirst_, 0.));
    prevDirection_.assign(nStartingPoints_, std::vector<double>(nFPParmsFirst_, 0.));
    cgHistoryValid_.assign(nStartingPoints_, false);
}

/******************************************************************************/
/**
 * Does any necessary finalization work
 */
void GConjugateGradientDescent::finalize() {
    G_OptimizationAlgorithm_Base::finalize();
}

/******************************************************************************/
/**
 * Retrieve a GPersonalityTraits object belonging to this algorithm
 */
std::shared_ptr<GPersonalityTraits> GConjugateGradientDescent::getPersonalityTraits_() const {
    return std::make_shared<GConjugateGradientDescent_PersonalityTraits>();
}

/******************************************************************************/
/**
 * Gives individuals an opportunity to update their internal structures. A
 * conjugate gradient descent is largely deterministic; nothing to do here.
 */
void GConjugateGradientDescent::actOnStalls_() {
    /* nothing */
}

/******************************************************************************/
/**
 * Resizes the population to the desired level and does some error checks.
 * The layout is identical to GGradientDescent:
 * nStartingPoints_ * (nFPParmsFirst_ + 1) individuals.
 */
void GConjugateGradientDescent::adjustPopulation_() {
    std::size_t nStart = this->size();

    if(nStart == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GConjugateGradientDescent::adjustPopulation():" << '\n'
            << "You didn't add any individuals to the collection. We need at least one."
            << '\n'
        );
    }

    nFPParmsFirst_ = this->at(0)->countParameters<double>(activityMode::ACTIVEONLY);

    if(nFPParmsFirst_ == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GConjugateGradientDescent::adjustPopulation():" << '\n'
            << "No floating point parameters in individual." << '\n'
        );
    }

#ifdef DEBUG
    for(std::size_t i = 1; i < this->size(); i++) {
        if(this->at(i)->countParameters<double>(activityMode::ACTIVEONLY) != nFPParmsFirst_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GConjugateGradientDescent::adjustPopulation():" << '\n'
                << "Found individual in position " << i << " with different" << '\n'
                << "number of floating point parameters than the first one: "
                << this->at(i)->countParameters<double>(activityMode::ACTIVEONLY) << "/"
                << nFPParmsFirst_ << '\n'
            );
        }
    }
#endif

    G_OptimizationAlgorithm_Base::setDefaultPopulationSize(
        nStartingPoints_ * (nFPParmsFirst_ + 1)
    );

    // Create the requested number of (randomized) starting points
    if(nStart < nStartingPoints_) {
        for(std::size_t i = 0; i < (nStartingPoints_ - nStart); i++) {
            this->push_back(this->at(0)->clone<GParameterSet>());
            this->back()->randomInit(activityMode::ACTIVEONLY);
        }
    }
    else {
        this->resize(nStartingPoints_);
    }

    // Add the difference-quotient children for every starting point
    for(std::size_t i = 0; i < nStartingPoints_; i++) {
        for(std::size_t j = 0; j < nFPParmsFirst_; j++) {
            this->push_back(this->at(i)->clone<GParameterSet>());
        }
    }

#ifdef DEBUG
    if(this->size() != nStartingPoints_ * (nFPParmsFirst_ + 1)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GConjugateGradientDescent::adjustPopulation():" << '\n'
            << "Population size is " << this->size() << '\n'
            << "but expected " << nStartingPoints_ * (nFPParmsFirst_ + 1) << '\n'
        );
    }
#endif /* DEBUG */
}

/******************************************************************************/
/**
 * Lets all individuals know about their position in the population.
 */
void GConjugateGradientDescent::markIndividualPositions() {
    for(std::size_t pos = 0; pos < this->size(); pos++) {
        this->at(pos)
            ->getPersonalityTraits<GConjugateGradientDescent_PersonalityTraits>()
            ->setPopulationPosition(pos);
    }
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GConjugateGradientDescent::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    if(G_OptimizationAlgorithm_Base::modify_GUnitTests_())
        result = true;

    return result;
#else  /* GEM_TESTING */
    Gem::Common::condnotset("GConjugateGradientDescent::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GConjugateGradientDescent::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    G_OptimizationAlgorithm_Base::specificTestsNoFailureExpected_GUnitTests_();
#else  /* GEM_TESTING */
    Gem::Common::condnotset(
        "GConjugateGradientDescent::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GConjugateGradientDescent::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    G_OptimizationAlgorithm_Base::specificTestsFailuresExpected_GUnitTests_();
#else  /* GEM_TESTING */
    Gem::Common::condnotset(
        "GConjugateGradientDescent::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva */
