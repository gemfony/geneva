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

#include "geneva/oa/GNelderMead.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

#include "common/GLogger.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::OptimizationAlgorithms::GNelderMead) // NOLINT

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * The default constructor
 */
GNelderMead::GNelderMead()
  : GNelderMead(DEFAULTNMSIMPLICES) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with the number of simplices
 */
GNelderMead::GNelderMead(const std::size_t &n_simplices)
  : n_simplices_(n_simplices) { /* nothing */
}

/******************************************************************************/
/** Retrieves the number of simultaneous simplices */
std::size_t GNelderMead::getNSimplices() const {
    return n_simplices_;
}

/******************************************************************************/
/** Allows to set the number of simultaneous simplices */
void GNelderMead::setNSimplices(std::size_t n_simplices) {
    if(n_simplices == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GNelderMead::setNSimplices(std::size_t):" << '\n'
            << "Got invalid number of simplices (0)." << '\n'
        );
    }
    n_simplices_ = n_simplices;
}

/******************************************************************************/
/** Sets the reflection coefficient */
void GNelderMead::setAlpha(double alpha) {
    if(alpha <= 0.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GNelderMead::setAlpha(double): Error!" << '\n'
            << "alpha must be > 0, got " << alpha << '\n'
        );
    }
    alpha_ = alpha;
}

/******************************************************************************/
double GNelderMead::getAlpha() const {
    return alpha_;
}

/******************************************************************************/
/** Sets the expansion coefficient */
void GNelderMead::setGamma(double gamma) {
    if(gamma <= 1.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GNelderMead::setGamma(double): Error!" << '\n'
            << "gamma must be > 1, got " << gamma << '\n'
        );
    }
    gamma_ = gamma;
}

/******************************************************************************/
double GNelderMead::getGamma() const {
    return gamma_;
}

/******************************************************************************/
/** Sets the contraction coefficient */
void GNelderMead::setRho(double rho) {
    if(rho <= 0. || rho >= 1.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GNelderMead::setRho(double): Error!" << '\n'
            << "rho must be in ]0,1[, got " << rho << '\n'
        );
    }
    rho_ = rho;
}

/******************************************************************************/
double GNelderMead::getRho() const {
    return rho_;
}

/******************************************************************************/
/** Sets the shrink coefficient */
void GNelderMead::setSigma(double sigma) {
    if(sigma <= 0. || sigma >= 1.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GNelderMead::setSigma(double): Error!" << '\n'
            << "sigma must be in ]0,1[, got " << sigma << '\n'
        );
    }
    sigma_ = sigma;
}

/******************************************************************************/
double GNelderMead::getSigma() const {
    return sigma_;
}

/******************************************************************************/
/** Sets the relative size of the initial simplex */
void GNelderMead::setInitialEdge(double initial_edge) {
    if(initial_edge <= 0.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GNelderMead::setInitialEdge(double): Error!" << '\n'
            << "initial_edge must be > 0, got " << initial_edge << '\n'
        );
    }
    initial_edge_ = initial_edge;
}

/******************************************************************************/
double GNelderMead::getInitialEdge() const {
    return initial_edge_;
}

/******************************************************************************/
/** Number of population slots used per simplex (vertices + trial slots) */
std::size_t GNelderMead::simplexBlockSize() const {
    return n_fp_parms_first_ + 1 + NM_NTRIALS;
}

/******************************************************************************/
/** Population index of vertex v in simplex s */
std::size_t GNelderMead::vertexPos(std::size_t s, std::size_t v) const {
    return s * simplexBlockSize() + v;
}

/******************************************************************************/
/** Population index of trial slot t in simplex s */
std::size_t GNelderMead::trialPos(std::size_t s, std::size_t t) const {
    return s * simplexBlockSize() + (n_fp_parms_first_ + 1) + t;
}

/******************************************************************************/
/** Retrieve the number of processable items in the current iteration. */
std::size_t GNelderMead::getNProcessableItems_() const {
    return this->size(); // The whole population is (re-)evaluated every iteration
}

/******************************************************************************/
std::string GNelderMead::getAlgorithmPersonalityType_() const {
    return "PERSONALITY_NM";
}

/******************************************************************************/
std::string GNelderMead::getAlgorithmName_() const {
    return std::string("Nelder-Mead Simplex");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 */
void GNelderMead::compare_(
    const GObject &cp,
    const Gem::Common::expectation &e,
    const double & /*limit*/
) const {
    using namespace Gem::Common;

    const GNelderMead *p_load = Gem::Common::g_convert_and_compare<GObject, GNelderMead>(cp, this);

    GToken token("GNelderMead", e);

    Gem::Common::compare_base_t<GBase>(*this, *p_load, token);

    compare_t(IDENTITY(n_simplices_, p_load->n_simplices_), token);
    compare_t(IDENTITY(n_fp_parms_first_, p_load->n_fp_parms_first_), token);
    compare_t(IDENTITY(alpha_, p_load->alpha_), token);
    compare_t(IDENTITY(gamma_, p_load->gamma_), token);
    compare_t(IDENTITY(rho_, p_load->rho_), token);
    compare_t(IDENTITY(sigma_, p_load->sigma_), token);
    compare_t(IDENTITY(initial_edge_, p_load->initial_edge_), token);
    // dbl_lower_parameter_boundaries_, dbl_upper_parameter_boundaries_ and trials_pending_
    // are transient: recomputed in init() and not restored in load_(). Comparing
    // them would cause round-trip equality tests to fail spuriously.

    token.evaluate();
}

/******************************************************************************/
void GNelderMead::resetToOptimizationStart_() {
    dbl_lower_parameter_boundaries_.clear();
    dbl_upper_parameter_boundaries_.clear();
    trials_pending_ = false;

    GBase::resetToOptimizationStart_();
}

/******************************************************************************/
std::string GNelderMead::name_() const {
    return std::string("GNelderMead");
}

/******************************************************************************/
void GNelderMead::load_(const GObject *cp) {
    const GNelderMead *p_load = Gem::Common::g_convert_and_compare<GObject, GNelderMead>(cp, this);

    // First load the parent class'es data (this also copies all individuals).
    GBase::load_(cp);

    // ... and then our own (serialized) data
    n_simplices_ = p_load->n_simplices_;
    n_fp_parms_first_ = p_load->n_fp_parms_first_;
    alpha_ = p_load->alpha_;
    gamma_ = p_load->gamma_;
    rho_ = p_load->rho_;
    sigma_ = p_load->sigma_;
    initial_edge_ = p_load->initial_edge_;
    // dbl*ParameterBoundaries_ and trials_pending_ are transient and re-set in init().
}

/******************************************************************************/
GObject *GNelderMead::clone_() const {
    return new GNelderMead(*this);
}

/******************************************************************************/
/**
 * The actual business logic to be performed during each iteration.
 *
 * Ordering (mirrors the GGradientDescent decision/propose/evaluate pattern):
 *   1. apply the Nelder-Mead acceptance rules using the trials proposed and
 *      evaluated in the previous iteration (skipped until real trials exist),
 *   2. propose new reflection / expansion / contraction trial points,
 *   3. (re-)evaluate the whole population.
 *
 * @return The value of the best vertex found in this iteration
 */
std::tuple<double, double> GNelderMead::cycleLogic_() {
    if(afterFirstIteration() && trials_pending_) {
        this->applyNelderMeadDecision();
    }

    if(afterFirstIteration()) {
        this->proposeTrials();
        trials_pending_ = true;
    }

    runFitnessCalculation_();

    std::tuple<double, double> best_fitness =
        std::make_tuple(this->at(0)->getWorstCase(), this->at(0)->getWorstCase());
    std::tuple<double, double> fitness_candidate =
        std::make_tuple(this->at(0)->getWorstCase(), this->at(0)->getWorstCase());

    auto m = this->at(0)->getMaxMode();
    for(std::size_t s = 0; s < n_simplices_; s++) {
        for(std::size_t v = 0; v <= n_fp_parms_first_; v++) {
            auto ind = this->at(vertexPos(s, v));
            std::get<G_RAW_FITNESS>(fitness_candidate) = ind->raw_fitness(0);
            std::get<G_TRANSFORMED_FITNESS>(fitness_candidate) = ind->transformed_fitness(0);

            if(isBetter(
                   std::get<G_TRANSFORMED_FITNESS>(fitness_candidate),
                   std::get<G_TRANSFORMED_FITNESS>(best_fitness),
                   m
               )) {
                best_fitness = fitness_candidate;
            }
        }
    }

    return best_fitness;
}

/******************************************************************************/
/**
 * Proposes the reflection, expansion and inside-contraction trial points for
 * every simplex. The worst vertex and the centroid of the remaining vertices
 * are determined from the most recent evaluation; the three candidates are
 * written into the trial slots so they are evaluated in this iteration.
 */
void GNelderMead::proposeTrials() {
    for(std::size_t s = 0; s < n_simplices_; s++) {
        const std::size_t n_vert = n_fp_parms_first_ + 1;

        // Collect vertex parameter vectors and (minimization) fitnesses
        std::vector<std::vector<double>> vparm(n_vert);
        std::vector<double> vfit(n_vert);
        for(std::size_t v = 0; v < n_vert; v++) {
            auto ind = this->at(vertexPos(s, v));
            ind->streamline<double>(vparm[v], activityMode::ACTIVEONLY);
            // A vertex modified by a shrink in applyNelderMeadDecision() during
            // this same iteration has not been re-evaluated yet (its stored
            // result was invalidated). Parameters are always readable, but its
            // fitness is not; treat such a vertex as the worst so it becomes the
            // reflected point. It is re-evaluated by runFitnessCalculation_()
            // later in this iteration, so proper ranking resumes next cycle.
            if(ind->is_due_for_processing() || ind->has_errors()) {
                vfit[v] = std::numeric_limits<double>::max();
            }
            else {
                vfit[v] = minOnly_transformed_fitness(ind);
            }
        }

        // Worst vertex = largest minimization fitness
        std::size_t w = 0;
        for(std::size_t v = 1; v < n_vert; v++) {
            if(vfit[v] > vfit[w]) {
                w = v;
            }
        }

        // Centroid of all vertices except the worst
        std::vector<double> centroid(n_fp_parms_first_, 0.);
        for(std::size_t v = 0; v < n_vert; v++) {
            if(v == w) {
                continue;
            }
            for(std::size_t k = 0; k < n_fp_parms_first_; k++) {
                centroid[k] += vparm[v][k];
            }
        }
        const double denom = static_cast<double>(n_vert - 1);
        for(std::size_t k = 0; k < n_fp_parms_first_; k++) {
            centroid[k] /= denom;
        }

        const std::vector<double> &xw = vparm[w];

        std::vector<double> reflect(n_fp_parms_first_);
        std::vector<double> expand(n_fp_parms_first_);
        std::vector<double> contract(n_fp_parms_first_);
        for(std::size_t k = 0; k < n_fp_parms_first_; k++) {
            reflect[k] = centroid[k] + alpha_ * (centroid[k] - xw[k]);
            expand[k] = centroid[k] + gamma_ * (centroid[k] - xw[k]);
            contract[k] = centroid[k] + rho_ * (xw[k] - centroid[k]); // inside contraction
        }

        this->at(trialPos(s, NM_REFLECT))
            ->assignValueVector<double>(reflect, activityMode::ACTIVEONLY);
        this->at(trialPos(s, NM_EXPAND))
            ->assignValueVector<double>(expand, activityMode::ACTIVEONLY);
        this->at(trialPos(s, NM_CONTRACT))
            ->assignValueVector<double>(contract, activityMode::ACTIVEONLY);
    }
}

/******************************************************************************/
/**
 * Applies the standard Nelder-Mead acceptance rules. The reflection, expansion
 * and inside-contraction candidates were proposed in the previous iteration
 * (around the then-worst vertex) and have just been evaluated. Because the
 * vertices were not modified between proposal and this call, the worst vertex
 * recomputed here is the same one the trials were built for.
 *
 * Accepting a trial is done via GObject::load(): the vertex thereby also
 * inherits the trial's already-known fitness, which keeps proposeTrials() in
 * the same iteration consistent (no stale ranking except after a shrink).
 */
void GNelderMead::applyNelderMeadDecision() {
    for(std::size_t s = 0; s < n_simplices_; s++) {
        const std::size_t n_vert = n_fp_parms_first_ + 1;

        std::vector<double> vfit(n_vert);
        for(std::size_t v = 0; v < n_vert; v++) {
            vfit[v] = minOnly_transformed_fitness(this->at(vertexPos(s, v)));
        }

        // Best, worst and second-worst vertices (minimization fitness)
        std::size_t b = 0;
        std::size_t w = 0;
        for(std::size_t v = 1; v < n_vert; v++) {
            if(vfit[v] < vfit[b]) {
                b = v;
            }
            if(vfit[v] > vfit[w]) {
                w = v;
            }
        }
        std::size_t sw = (w == 0) ? 1 : 0;
        for(std::size_t v = 0; v < n_vert; v++) {
            if(v == w) {
                continue;
            }
            if(vfit[v] > vfit[sw]) {
                sw = v;
            }
        }

        const double f_best = vfit[b];
        const double f_worst = vfit[w];
        const double f_second = vfit[sw];

        const double f_r = minOnly_transformed_fitness(this->at(trialPos(s, NM_REFLECT)));
        const double f_e = minOnly_transformed_fitness(this->at(trialPos(s, NM_EXPAND)));
        const double f_c = minOnly_transformed_fitness(this->at(trialPos(s, NM_CONTRACT)));

        auto accept_trial_into_worst = [&](std::size_t trial_slot) {
            this->at(vertexPos(s, w))->GObject::load(this->at(trialPos(s, trial_slot)));
            this->at(vertexPos(s, w))
                ->getPersonalityTraits<GNelderMead_PersonalityTraits>()
                ->setPopulationPosition(vertexPos(s, w));
        };

        if(f_r < f_best) {
            // Reflection is the new best -> try to expand further
            if(f_e < f_r) {
                accept_trial_into_worst(NM_EXPAND);
            }
            else {
                accept_trial_into_worst(NM_REFLECT);
            }
        }
        else if(f_r < f_second) {
            // Reflection is an improvement (but not the best) -> accept it
            accept_trial_into_worst(NM_REFLECT);
        }
        else {
            // Contraction. Only the inside contraction is evaluated here; it is
            // accepted if it improves on both the worst vertex and the
            // reflection, otherwise the simplex is shrunk towards the best
            // vertex. (Outside contraction is intentionally approximated by the
            // reflection/inside-contraction pair -- see the accompanying docs.)
            const double accept_threshold = std::min(f_worst, f_r);
            if(f_c < accept_threshold) {
                accept_trial_into_worst(NM_CONTRACT);
            }
            else {
                // Shrink: move every non-best vertex towards the best vertex
                std::vector<double> xb;
                this->at(vertexPos(s, b))->streamline<double>(xb, activityMode::ACTIVEONLY);
                for(std::size_t v = 0; v < n_vert; v++) {
                    if(v == b) {
                        continue;
                    }
                    std::vector<double> xv;
                    this->at(vertexPos(s, v))
                        ->streamline<double>(xv, activityMode::ACTIVEONLY);
                    for(std::size_t k = 0; k < n_fp_parms_first_; k++) {
                        xv[k] = xb[k] + sigma_ * (xv[k] - xb[k]);
                    }
                    this->at(vertexPos(s, v))
                        ->assignValueVector<double>(xv, activityMode::ACTIVEONLY);
                }
            }
        }
    }
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 */
void GNelderMead::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    GBase::addConfigurationOptions_(gpb);

    gpb.registerFileParameter<std::size_t>(
        "n_simplices",
        DEFAULTNMSIMPLICES,
        [this](std::size_t ns) { this->setNSimplices(ns); }
    ) << "The number of simultaneous Nelder-Mead simplices";

    gpb.registerFileParameter<double>(
        "alpha",
        DEFAULTNMALPHA,
        [this](double a) { this->setAlpha(a); }
    ) << "The Nelder-Mead reflection coefficient (> 0)";

    gpb.registerFileParameter<double>(
        "gamma",
        DEFAULTNMGAMMA,
        [this](double g) { this->setGamma(g); }
    ) << "The Nelder-Mead expansion coefficient (> 1)";

    gpb.registerFileParameter<double>(
        "rho",
        DEFAULTNMRHO,
        [this](double r) { this->setRho(r); }
    ) << "The Nelder-Mead contraction coefficient (in ]0,1[)";

    gpb.registerFileParameter<double>(
        "sigma",
        DEFAULTNMSIGMA,
        [this](double si) { this->setSigma(si); }
    ) << "The Nelder-Mead shrink coefficient (in ]0,1[)";

    gpb.registerFileParameter<double>(
        "initial_edge",
        DEFAULTNMINITIALEDGE,
        [this](double ie) { this->setInitialEdge(ie); }
    ) << "Relative size of the initial simplex,"
      << '\n'
      << "as a fraction of each parameter's value range";
}

/******************************************************************************/
/**
 * Triggers fitness calculation of all individuals via the broker.
 */
void GNelderMead::runFitnessCalculation_() {
    using namespace Gem::Courtier;

    setProcessingFlag(this->data_cnt_, std::make_tuple(static_cast<std::size_t>(0), this->data_cnt_.size()));
    auto status = this->workOn(
        this->data_cnt_,
        true // resubmit unprocessed items
        ,
        "GNelderMead::runFitnessCalculation()"
    );

    if(not status.is_complete || status.has_errors) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GNelderMead::runFitnessCalculation(): Error!" << '\n'
            << "No complete set of items received or errors found in some individuals"
            << '\n'
        );
    }
}

/******************************************************************************/
/**
 * Does some preparatory work before the optimization starts
 */
void GNelderMead::init() {
    GBase::init();

    this->at(0)->boundaries(
        dbl_lower_parameter_boundaries_,
        dbl_upper_parameter_boundaries_,
        activityMode::ACTIVEONLY
    );

#ifdef DEBUG
    if(dbl_lower_parameter_boundaries_.size() != dbl_upper_parameter_boundaries_.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GNelderMead::init(): Error!" << '\n'
            << "Found invalid sizes: " << dbl_lower_parameter_boundaries_.size() << " / "
            << dbl_upper_parameter_boundaries_.size() << '\n'
        );
    }
#endif /* DEBUG */

    trials_pending_ = false;
    buildInitialSimplices();
    markIndividualPositions();
}

/******************************************************************************/
/**
 * Builds a non-degenerate initial simplex around each seed vertex. Vertex 0 of
 * every simplex is the (user-supplied or randomized) seed; the remaining n
 * vertices are obtained by perturbing one coordinate each. The perturbation is
 * a fraction (initial_edge_) of the parameter range where that range is finite,
 * and a robust absolute fallback otherwise.
 */
void GNelderMead::buildInitialSimplices() {
    for(std::size_t s = 0; s < n_simplices_; s++) {
        std::vector<double> p0;
        this->at(vertexPos(s, 0))->streamline<double>(p0, activityMode::ACTIVEONLY);

        for(std::size_t v = 1; v <= n_fp_parms_first_; v++) {
            std::vector<double> p = p0;
            const std::size_t k = v - 1; // coordinate perturbed for this vertex

            double edge;
            const double range =
                dbl_upper_parameter_boundaries_[k] - dbl_lower_parameter_boundaries_[k];
            if(std::isfinite(range) && range > 0.) {
                edge = initial_edge_ * range;
            }
            else {
                edge = (std::fabs(p0[k]) > 1e-12) ? initial_edge_ * std::fabs(p0[k])
                                                  : initial_edge_;
            }

            p[k] += edge;
            this->at(vertexPos(s, v))->assignValueVector<double>(p, activityMode::ACTIVEONLY);
        }

        // The trial slots start as copies of the seed; they are overwritten by
        // proposeTrials() before they are first used for a decision.
        for(std::size_t t = 0; t < NM_NTRIALS; t++) {
            this->at(trialPos(s, t))->assignValueVector<double>(p0, activityMode::ACTIVEONLY);
        }
    }
}

/******************************************************************************/
void GNelderMead::finalize() {
    GBase::finalize();
}

/******************************************************************************/
std::shared_ptr<GPersonalityTraits> GNelderMead::getPersonalityTraits_() const {
    return std::make_shared<GNelderMead_PersonalityTraits>();
}

/******************************************************************************/
/**
 * The Nelder-Mead simplex is derivative free and has no internal structures
 * that need updating on a stall.
 */
void GNelderMead::actOnStalls_() {
    /* nothing */
}

/******************************************************************************/
/**
 * Resizes the population to the desired level and does some error checks. The
 * layout is n_simplices_ blocks of (n_fp_parms_first_ + 1) vertices plus
 * NM_NTRIALS speculative trial slots each.
 */
void GNelderMead::adjustPopulation_() {
    std::size_t n_start = this->size();

    if(n_start == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GNelderMead::adjustPopulation():" << '\n'
            << "You didn't add any individuals to the collection. We need at least one."
            << '\n'
        );
    }

    n_fp_parms_first_ = this->at(0)->countParameters<double>(activityMode::ACTIVEONLY);

    if(n_fp_parms_first_ == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GNelderMead::adjustPopulation():" << '\n'
            << "No floating point parameters in individual." << '\n'
        );
    }

    // Nelder-Mead is derivative-free but still operates only on the
    // floating-point (continuous) parameter space. Any integer / boolean
    // parameters are left unchanged -- this is normal, user-expected
    // behaviour, so it is merely logged (not warned about).
    {
        const std::size_t n_int_parms =
            this->at(0)->countParameters<std::int32_t>(activityMode::ACTIVEONLY);
        const std::size_t n_bool_parms =
            this->at(0)->countParameters<bool>(activityMode::ACTIVEONLY);
        if(n_int_parms + n_bool_parms > 0) {
            glogger
                << "In GNelderMead::adjustPopulation_(): Note:" << '\n'
                << "The individual carries " << n_int_parms << " integer and " << n_bool_parms
                << " boolean parameter(s) alongside " << n_fp_parms_first_
                << " floating point parameter(s)." << '\n'
                << "Nelder-Mead only operates on the floating point parameters;" << '\n'
                << "the other parameters are left unchanged." << '\n'
                << GLOGGING;
        }
    }

    const std::size_t block_size = n_fp_parms_first_ + 1 + NM_NTRIALS;
    const std::size_t total_size = n_simplices_ * block_size;

    GBase::setDefaultPopulationSize(total_size);

    // Make sure we have one (randomized) seed individual per simplex first.
    if(n_start < n_simplices_) {
        for(std::size_t i = 0; i < (n_simplices_ - n_start); i++) {
            this->push_back(this->at(0)->clone<gpar::GParameterSet>());
            this->back()->randomInit(activityMode::ACTIVEONLY);
        }
    }
    else if(n_start > n_simplices_) {
        this->resize(n_simplices_);
    }

    // The seeds currently sit at positions 0 .. n_simplices_-1. Re-order them so
    // that seed s ends up at vertexPos(s,0) and fill the rest of every block
    // with clones (the real initial simplex is constructed in init()).
    std::vector<std::shared_ptr<gpar::GParameterSet>> seeds;
    seeds.reserve(n_simplices_);
    for(std::size_t s = 0; s < n_simplices_; s++) {
        seeds.push_back(this->at(s)->clone<gpar::GParameterSet>());
    }

    this->clear();
    for(std::size_t s = 0; s < n_simplices_; s++) {
        this->push_back(seeds[s]); // vertex 0 of simplex s
        for(std::size_t r = 1; r < block_size; r++) {
            this->push_back(seeds[s]->clone<gpar::GParameterSet>());
        }
    }

#ifdef DEBUG
    if(this->size() != total_size) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GNelderMead::adjustPopulation():" << '\n'
            << "Population size is " << this->size() << '\n'
            << "but expected " << total_size << '\n'
        );
    }
#endif /* DEBUG */
}

/******************************************************************************/
/**
 * Lets all individuals know about their position in the population.
 */
void GNelderMead::markIndividualPositions() {
    for(std::size_t pos = 0; pos < this->size(); pos++) {
        this->at(pos)
            ->getPersonalityTraits<GNelderMead_PersonalityTraits>()
            ->setPopulationPosition(pos);
    }
}

/******************************************************************************/
bool GNelderMead::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    if(GBase::modify_GUnitTests_()) {
        result = true;
    }

    return result;
#else  /* GEM_TESTING */
    Gem::Common::condnotset("GNelderMead::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
void GNelderMead::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    GBase::specificTestsNoFailureExpected_GUnitTests_();
#else  /* GEM_TESTING */
    Gem::Common::condnotset(
        "GNelderMead::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/
void GNelderMead::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    GBase::specificTestsFailuresExpected_GUnitTests_();
#else  /* GEM_TESTING */
    Gem::Common::condnotset(
        "GNelderMead::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
