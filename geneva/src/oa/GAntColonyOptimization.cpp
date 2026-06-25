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

#include "geneva/oa/GAntColonyOptimization.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <numeric>
#include <random>
#include <tuple>
#include <vector>

#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/GenevaHelperFunctions.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/ind/GIndividualSlot.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::OptimizationAlgorithms::GAntColonyOptimization) // NOLINT

namespace Gem::Geneva::OptimizationAlgorithms {

using Gem::Geneva::activityMode;

/** @brief A dimensionless floor on the sampling sigma, as a fraction of a parameter's (normalized)
 *  range. Without it a collapsed archive (all members coincident) drives the mean-distance bandwidth to
 *  zero regardless of remaining search volume, so the sampler stops exploring and the run converges
 *  prematurely. The floor is universal because every parameter is normalized to the unit interval. It is
 *  small enough not to cap the achievable precision. */
constexpr double ACOR_SIGMA_FLOOR_FRACTION = 1.e-4;

/******************************************************************************/
/**
 * The default constructor.
 */
GAntColonyOptimization::GAntColonyOptimization() {
    this->setDefaultPopulationSize(archive_size_);
}

/******************************************************************************/
/**
 * Initialization with the archive size.
 */
GAntColonyOptimization::GAntColonyOptimization(std::size_t archive_size)
  : archive_size_((archive_size >= 2) ? archive_size : DEFAULTACORARCHIVESIZE) {
    this->setDefaultPopulationSize(archive_size_);
}

/******************************************************************************/
/** Sets the archive size k (the pheromone model size; at least 2). */
void GAntColonyOptimization::setArchiveSize(std::size_t archive_size) {
    if(archive_size < 2) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GAntColonyOptimization::setArchiveSize(std::size_t): Error!" << '\n'
            << "Got invalid archive size " << archive_size << " (need at least 2)." << '\n'
        );
    }
    archive_size_ = archive_size;
    this->setDefaultPopulationSize(archive_size_);
}

/******************************************************************************/
std::size_t GAntColonyOptimization::getArchiveSize() const {
    return archive_size_;
}

/******************************************************************************/
/** Sets the number of ants m constructed per iteration. */
void GAntColonyOptimization::setNAnts(std::size_t n_ants) {
    if(n_ants == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GAntColonyOptimization::setNAnts(std::size_t): Error!" << '\n'
            << "Got invalid number of ants (0)." << '\n'
        );
    }
    n_ants_ = n_ants;
}

/******************************************************************************/
std::size_t GAntColonyOptimization::getNAnts() const {
    return n_ants_;
}

/******************************************************************************/
/** Sets the locality / intensification parameter q (> 0). */
void GAntColonyOptimization::setQ(double q) {
    if(q <= 0.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GAntColonyOptimization::setQ(double): Error!" << '\n'
            << "q must be > 0, got " << q << '\n'
        );
    }
    q_ = q;
}

/******************************************************************************/
double GAntColonyOptimization::getQ() const {
    return q_;
}

/******************************************************************************/
/** Sets the evaporation / convergence-speed parameter xi (> 0). */
void GAntColonyOptimization::setXi(double xi) {
    if(xi <= 0.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GAntColonyOptimization::setXi(double): Error!" << '\n'
            << "xi must be > 0, got " << xi << '\n'
        );
    }
    xi_ = xi;
}

/******************************************************************************/
double GAntColonyOptimization::getXi() const {
    return xi_;
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object.
 */
void GAntColonyOptimization::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    // Call our parent class'es function
    GOptimizationAlgorithmT<GAntColonyOptimization>::addConfigurationOptions_(gpb);

    gpb.registerFileParameter<std::size_t>(
        "archive_size",
        DEFAULTACORARCHIVESIZE,
        [this](std::size_t k) { this->setArchiveSize(k); }
    ) << "The size k of the ACOR solution archive (the continuous pheromone model; at least 2)";

    gpb.registerFileParameter<std::size_t>(
        "n_ants",
        DEFAULTACORNANTS,
        [this](std::size_t m) { this->setNAnts(m); }
    ) << "The number of ants m constructed (and evaluated) per iteration";

    gpb.registerFileParameter<double>(
        "q",
        DEFAULTACORQ,
        [this](double q) { this->setQ(q); }
    ) << "The ACOR locality / intensification parameter q (> 0; small = strong intensification)";

    gpb.registerFileParameter<double>(
        "xi",
        DEFAULTACORXI,
        [this](double xi) { this->setXi(xi); }
    ) << "The ACOR evaporation / convergence-speed parameter xi (> 0)";
}

/******************************************************************************/
/**
 * Loads the data of another GAntColonyOptimization object.
 */
void GAntColonyOptimization::load_(const GOptimizationAlgorithmBase *cp) {
    const GAntColonyOptimization *p_load =
        Gem::Common::g_convert_and_compare<GOptimizationAlgorithmBase, GAntColonyOptimization>(cp, this);

    // First load the parent class's data (this also copies all individuals) ...
    GOptimizationAlgorithmT<GAntColonyOptimization>::load_(cp);

    // ... and then our own data, derived from the single localMembers() declaration. All other members
    // are transient and re-set in init().
    Gem::Common::g_load_members(localMembers_(*this), localMembers_(*p_load));
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object of the same type. Only the
 * scalar configuration is compared; the per-iteration archive state is transient (recomputed in init()
 * and not restored in load_()), so comparing it would cause round-trip equality tests to fail.
 */
void GAntColonyOptimization::compare_(
    const GOptimizationAlgorithmBase &cp,
    const Gem::Common::expectation &e,
    const double & /*limit*/
) const {
    using namespace Gem::Common;

    const GAntColonyOptimization *p_load =
        Gem::Common::g_convert_and_compare<GOptimizationAlgorithmBase, GAntColonyOptimization>(cp, this);

    GToken token("GAntColonyOptimization", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GOptimizationAlgorithmBase>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

    token.evaluate();
}

/******************************************************************************/
/**
 * Resets the settings of this population to what was configured when optimize() was issued.
 */
void GAntColonyOptimization::resetToOptimizationStart_() {
    n_fp_parms_ = 0;
    archive_parms_.clear();
    archive_fitness_.clear();
    selection_probabilities_.clear();

    GOptimizationAlgorithmT<GAntColonyOptimization>::resetToOptimizationStart_();
}

/******************************************************************************/
/**
 * Sizes the population to the archive size k and does error checks. Runs at setup, BEFORE
 * setIndividualPersonalities() and init(), so every slot (including the freshly cloned ones) receives a
 * personality. The working population hosts the k seed solutions for the first evaluation (and,
 * thereafter, the m ants in its first m slots).
 */
void GAntColonyOptimization::adjustPopulation_() {
    if(this->empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GAntColonyOptimization::adjustPopulation_(): Error!" << '\n'
            << "No individuals found in the population. You need to add at least" << '\n'
            << "one individual before the call to optimize()." << '\n'
        );
    }

    if(n_ants_ > archive_size_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GAntColonyOptimization::adjustPopulation_(): Error!" << '\n'
            << "The number of ants (" << n_ants_ << ") exceeds the archive size ("
            << archive_size_ << ")." << '\n'
        );
    }

    while(this->size() < archive_size_) {
        this->push_back(this->at(0)->individual().clone_unique());
    }
    if(this->size() > archive_size_) {
        this->resize(archive_size_);
    }
    this->setDefaultPopulationSize(archive_size_);
}

/******************************************************************************/
/**
 * Does some preparatory work before the optimization starts: determines the floating-point parameter
 * count, computes the ranked selection probabilities and seeds the initial archive into the first k population slots
 * (member 0 keeps the registered start individual; the rest are randomized uniformly inside the box).
 */
void GAntColonyOptimization::init() {
    GOptimizationAlgorithmT<GAntColonyOptimization>::init();

    // ACOR samples in the normalized internal coordinate (boundary-agnostic): a bounded parameter occupies
    // the unit interval and an out-of-range sample is folded back by the genome on assignment, so only the
    // count of active floating point parameters is needed.
    n_fp_parms_ = this->at(0)->individual().countFPParameters(activityMode::ACTIVEONLY);

    if(n_fp_parms_ == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GAntColonyOptimization::init(): Error!" << '\n'
            << "The individual exposes no floating point parameters; Ant Colony Optimization" << '\n'
            << "(continuous) requires a continuous (double / float) genome." << '\n'
        );
    }

    computeSelectionProbabilities();
    seedInitialArchive();
    markIndividualPositions();
}

/******************************************************************************/
/**
 * Seeds the initial archive: archive member 0 keeps the registered start individual's parameters
 * (population slot 0); the remaining k-1 members are randomized uniformly within the parameter bounds.
 * The seeds are written into the first k population slots (where they are evaluated by the first
 * cycleLogic_() iteration). archive_parms_ is sized to k here; the fitnesses are filled after the first
 * evaluation.
 */
void GAntColonyOptimization::seedInitialArchive() {
    archive_parms_.assign(archive_size_, std::vector<double>(n_fp_parms_, 0.));
    archive_fitness_.assign(archive_size_, this->at(0)->individual().getWorstCase());

    // Member 0: the (user-supplied) start individual, unchanged.
    this->at(0)->individual().streamlineFPInternal(archive_parms_[0], activityMode::ACTIVEONLY);

    // Members 1..k-1: random restarts within the bounds (reuses the genome's randomInit channel).
    for(std::size_t l = 1; l < archive_size_; ++l) {
        this->at(l)->individual().randomInit(activityMode::ACTIVEONLY);
        this->at(l)->individual().streamlineFPInternal(archive_parms_[l], activityMode::ACTIVEONLY);
    }
}

/******************************************************************************/
/**
 * Does any necessary finalization work.
 */
void GAntColonyOptimization::finalize() {
    GOptimizationAlgorithmT<GAntColonyOptimization>::finalize();
}

/******************************************************************************/
/**
 * ACOR samples afresh from the archive each iteration and has no per-individual internal structures that
 * need updating on a stall.
 */
void GAntColonyOptimization::actOnStalls_() {
    /* nothing */
}

/******************************************************************************/
/**
 * Retrieve the number of processable items in the current iteration. In the very first iteration the
 * whole seeded archive (k individuals) is evaluated; afterwards only the m newly constructed ants are
 * evaluated.
 */
std::size_t GAntColonyOptimization::getNProcessableItems_() const {
    return this->afterFirstIteration() ? n_ants_ : archive_size_;
}

/******************************************************************************/
/**
 * Retrieve a GPersonalityTraits object belonging to this algorithm.
 */
std::shared_ptr<GPersonalityTraits> GAntColonyOptimization::getPersonalityTraits_() const {
    return std::make_shared<GAntColonyOptimization_PersonalityTraits>();
}

/******************************************************************************/
/**
 * Lets all individuals know about their position in the population.
 */
void GAntColonyOptimization::markIndividualPositions() {
    for(std::size_t pos = 0; pos < this->size(); ++pos) {
        this->at(pos)
            ->getPersonalityTraits<GAntColonyOptimization_PersonalityTraits>()
            ->setPopulationPosition(pos);
    }
}

/******************************************************************************/
/**
 * Computes the ranked Gaussian weights w_l and turns them into roulette-wheel selection probabilities
 * p_l (cached in selection_probabilities_). The weight of archive member l (rank l = 1 is best) is
 *
 *   w_l = (1 / (q k sqrt(2 pi))) exp( -(l-1)^2 / (2 q^2 k^2) ),  l = 1..k,
 *
 * and p_l = w_l / sum_j w_j. Because the normalization constant cancels in p_l, only the exponential
 * factor needs to be evaluated.
 */
void GAntColonyOptimization::computeSelectionProbabilities() {
    selection_probabilities_.assign(archive_size_, 0.);

    const double qk = q_ * static_cast<double>(archive_size_);
    const double denom_exp = 2. * qk * qk; // 2 q^2 k^2

    double sum = 0.;
    for(std::size_t l = 0; l < archive_size_; ++l) {
        // rank is l+1, so (rank-1) == l
        const auto rank_minus_one = static_cast<double>(l);
        const double w = std::exp(-(rank_minus_one * rank_minus_one) / denom_exp);
        selection_probabilities_[l] = w;
        sum += w;
    }

    if(sum > 0.) {
        for(double &p : selection_probabilities_) {
            p /= sum;
        }
    }
    else {
        // Degenerate fallback (should not happen for q,k > 0): uniform.
        const double uni = 1. / static_cast<double>(archive_size_);
        std::fill(selection_probabilities_.begin(), selection_probabilities_.end(), uni);
    }
}

/******************************************************************************/
/**
 * Picks an archive index by roulette-wheel selection over the cached probabilities p_l. Reuses the
 * inherited uniform real distribution (the same facility the swarm / sep-CMA-ES algorithms use for their
 * random coefficients).
 */
std::size_t GAntColonyOptimization::rouletteSelect() {
    const double r = uniform_real_distribution_(
        gr_,
        std::uniform_real_distribution<double>::param_type(0., 1.)
    );

    double cumulative = 0.;
    for(std::size_t l = 0; l < selection_probabilities_.size(); ++l) {
        cumulative += selection_probabilities_[l];
        if(r <= cumulative) {
            return l;
        }
    }
    // Floating-point round-off fallback: return the last member.
    return selection_probabilities_.empty() ? 0 : (selection_probabilities_.size() - 1);
}

/******************************************************************************/
/**
 * Sorts the archive (parameter vectors and their min-only fitnesses jointly) best-first, i.e. by
 * ascending min-only transformed fitness. Reuses the EA/ES notion of a ranked parent set: the archive
 * after sorting is exactly the ranked elite set ACOR samples from.
 */
void GAntColonyOptimization::sortArchive() {
    const std::size_t n = archive_fitness_.size();

    // Build an index permutation and sort it (cheaper than moving the vectors).
    std::vector<std::size_t> order(n);
    std::iota(order.begin(), order.end(), static_cast<std::size_t>(0));
    std::sort(order.begin(), order.end(), [this](std::size_t a, std::size_t b) {
        return archive_fitness_[a] < archive_fitness_[b]; // min-only: smaller is better
    });

    std::vector<std::vector<double>> sorted_parms;
    std::vector<double> sorted_fitness;
    sorted_parms.reserve(n);
    sorted_fitness.reserve(n);
    for(std::size_t idx : order) {
        sorted_parms.push_back(std::move(archive_parms_[idx]));
        sorted_fitness.push_back(archive_fitness_[idx]);
    }
    archive_parms_ = std::move(sorted_parms);
    archive_fitness_ = std::move(sorted_fitness);
}

/******************************************************************************/
/**
 * Constructs the m new ant solutions for this iteration and writes them into the first m population
 * slots. Each ant is built dimension by dimension and independently: a guiding archive member is chosen
 * by roulette wheel, its xi-scaled mean-distance bandwidth is computed, and the new coordinate is
 * sampled from a Gaussian kernel. This is an ES-style mutation that reuses the inherited RNG (gr_) and
 * std::normal_distribution, exactly as GSepCmaEvolutionStrategy / GStandardPSO2011 do.
 */
void GAntColonyOptimization::constructAnts() {
    for(std::size_t a = 0; a < n_ants_; ++a) {
        std::vector<double> x_new(n_fp_parms_, 0.);

        for(std::size_t i = 0; i < n_fp_parms_; ++i) {
            // 1. Choose a guiding archive member l by roulette wheel on {p_l}.
            const std::size_t l = rouletteSelect();
            const double mu = archive_parms_[l][i];

            // 2. xi-scaled average distance of member l to all others in dim i.
            double sum_dist = 0.;
            for(std::size_t e = 0; e < archive_size_; ++e) {
                if(e == l) {
                    continue;
                }
                sum_dist += std::fabs(archive_parms_[e][i] - mu);
            }
            // The mean-distance bandwidth, floored against a dimensionless fraction of the (normalized)
            // range so a collapsed archive cannot drive sigma to zero and stall the search prematurely.
            const double sigma = std::max(
                xi_ * sum_dist / static_cast<double>(archive_size_ - 1), ACOR_SIGMA_FLOOR_FRACTION
            );

            // 3. Sample the new coordinate ~ N(mu, sigma).
            std::normal_distribution<double> gauss(mu, sigma);
            x_new[i] = gauss(gr_);
        }

        // Write the sampled vector through the genome; the constrained parameter objects fold/clamp it
        // into the feasible box automatically. Mark the slot for (re)evaluation.
        this->at(a)->individual().assignFPValueVectorInternal(x_new, activityMode::ACTIVEONLY);
        this->at(a)->mark_as_due_for_processing();
    }
}

/******************************************************************************/
/**
 * Merges the freshly evaluated m ants (population slots 0..m-1) into the archive, re-sorts the union
 * best-first, and truncates back to the best k. The ant parameter vectors are read back from the genome
 * so that any constrained folding applied during evaluation is reflected in the archive. Reuses the
 * EA/ES min-only transformed fitness helper so that maxMode is handled transparently (the archive is
 * always ranked as a minimization).
 */
void GAntColonyOptimization::updateArchive() {
    for(std::size_t a = 0; a < n_ants_; ++a) {
        auto &ind = this->at(a)->individual();

        std::vector<double> parms;
        ind.streamlineFPInternal(parms, activityMode::ACTIVEONLY);

        const double fit = minOnly_transformed_fitness(ind);

        archive_parms_.push_back(std::move(parms));
        archive_fitness_.push_back(fit);
    }

    sortArchive();

    // Truncate (merge + keep best k).
    if(archive_parms_.size() > archive_size_) {
        archive_parms_.resize(archive_size_);
        archive_fitness_.resize(archive_size_);
    }
}

/******************************************************************************/
/**
 * Submits the relevant population sub-range to the one process consumer and waits for processed items.
 * In the first iteration the whole seeded archive (k individuals) is evaluated; afterwards only the m
 * newly constructed ants (population slots 0..m-1) are evaluated. Mirrors the stock EA's submission path
 * (workOnPopulation reconciles the contiguous range in place).
 */
void GAntColonyOptimization::runFitnessCalculation_() {
    const std::size_t n_eval = this->afterFirstIteration() ? n_ants_ : archive_size_;

    auto status = this->workOnPopulation(0, n_eval);

    if(not status.is_complete || status.has_errors) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GAntColonyOptimization::runFitnessCalculation_(): Error!" << '\n'
            << "No complete set of items received or errors found in some individuals." << '\n'
        );
    }
}

/******************************************************************************/
/**
 * The actual business logic to be performed during each iteration.
 *
 *   - first iteration: the archive was seeded in init() and written into the first k population slots;
 *     evaluate all k, read their fitnesses and sort the archive best-first.
 *   - later iterations: construct m new ants (sampling from the ranked archive), evaluate them, then
 *     merge them into the archive and truncate to k.
 *
 * The global best is retained across iterations by the base class's global priority queue, which is fed
 * the whole population every iteration; cycleLogic_ therefore reports the best (raw, transformed)
 * fitness among the individuals actually evaluated this iteration.
 *
 * @return The best fitness found in this iteration
 */
std::tuple<double, double> GAntColonyOptimization::cycleLogic_() {
    if(this->afterFirstIteration()) {
        // Sample m new ants from the current (ranked) archive, then evaluate and merge.
        constructAnts();
        runFitnessCalculation_();
        updateArchive();
    }
    else {
        // Initial archive evaluation: slots 0..k-1 already hold the seeds.
        runFitnessCalculation_();

        for(std::size_t l = 0; l < archive_size_; ++l) {
            auto &ind = this->at(l)->individual();
            ind.streamlineFPInternal(archive_parms_[l], activityMode::ACTIVEONLY);
            archive_fitness_[l] = minOnly_transformed_fitness(ind);
        }
        sortArchive();
    }

    // Report the best (raw, transformed) fitness among the individuals actually evaluated this iteration,
    // using the standard EA/ES ranking helpers (isBetter / getMaxMode).
    const auto m = this->at(0)->individual().getMaxMode();
    const std::size_t n_eval = this->afterFirstIteration() ? n_ants_ : archive_size_;

    std::tuple<double, double> best_fitness = std::make_tuple(
        this->at(0)->individual().getWorstCase(),
        this->at(0)->individual().getWorstCase()
    );

    for(std::size_t pos = 0; pos < n_eval; ++pos) {
        auto &ind = this->at(pos)->individual();
        if(ind.is_due_for_processing() || ind.has_errors()) {
            continue;
        }
        if(isBetter(
               ind.transformed_fitness(0),
               std::get<G_TRANSFORMED_FITNESS>(best_fitness),
               m
           )) {
            best_fitness = ind.getFitnessTuple();
        }
    }

    return best_fitness;
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes.
 */
bool GAntColonyOptimization::modify_GUnitTests_() {
#ifdef GEM_TESTING
    bool result = false;

    if(GOptimizationAlgorithmT<GAntColonyOptimization>::modify_GUnitTests_()) {
        result = true;
    }

    this->setArchiveSize(this->getArchiveSize() + 1);
    result = true;

    return result;
#else  /* GEM_TESTING */
    Gem::Common::condnotset("GAntColonyOptimization::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes.
 */
void GAntColonyOptimization::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    GOptimizationAlgorithmT<GAntColonyOptimization>::specificTestsNoFailureExpected_GUnitTests_();

    { // Test setting and retrieval of basic strategy parameters
        std::shared_ptr<GAntColonyOptimization> p_test = this->clone<GAntColonyOptimization>();

        CHECK_NOTHROW(p_test->setArchiveSize(25));
        CHECK(p_test->getArchiveSize() == 25);

        CHECK_NOTHROW(p_test->setNAnts(4));
        CHECK(p_test->getNAnts() == 4);

        CHECK_NOTHROW(p_test->setQ(0.5));
        CHECK(p_test->getQ() == 0.5);

        CHECK_NOTHROW(p_test->setXi(0.7));
        CHECK(p_test->getXi() == 0.7);
    }

    { // Setting invalid strategy parameters must throw
        std::shared_ptr<GAntColonyOptimization> p_test = this->clone<GAntColonyOptimization>();
        CHECK_THROWS(p_test->setArchiveSize(1)); // need at least 2 archive members
        CHECK_THROWS(p_test->setNAnts(0));       // need at least 1 ant
        CHECK_THROWS(p_test->setQ(0.));          // q must be > 0
        CHECK_THROWS(p_test->setXi(-1.));        // xi must be > 0
    }
#else  /* GEM_TESTING */
    Gem::Common::condnotset(
        "GAntColonyOptimization::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes.
 */
void GAntColonyOptimization::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    GOptimizationAlgorithmT<GAntColonyOptimization>::specificTestsFailuresExpected_GUnitTests_();
#else  /* GEM_TESTING */
    Gem::Common::condnotset(
        "GAntColonyOptimization::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
