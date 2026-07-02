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

#include "geneva/ind/GOptimizableEntity.hpp"

#include "geneva/GFaultInjector.hpp" // the pluggable, process-global evaluation fault injector

#include <chrono>
#include <cmath>
#include <limits>
#include <sstream>

#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GParserBuilder.hpp"

namespace Gem::Geneva::Genome {

/******************************************************************************/
/**
 * @brief The default constructor: a single fitness criterion and a fresh private policy.
 */
GOptimizableEntity::GOptimizableEntity() = default;

/******************************************************************************/
/**
 * @brief Initialization with the number of fitness criteria.
 * @param n_fitness_criteria The number of fitness criteria this candidate evaluates to
 */
GOptimizableEntity::GOptimizableEntity(const std::size_t n_fitness_criteria)
  : stored_results_cnt_(n_fitness_criteria, individual_processing_result()) {
    /* nothing */
}

/******************************************************************************/
/**
 * @brief The copy constructor. The shared policy is copied by shared pointer (1:N), the
 * pre-/post-processors are deep-cloned.
 * @param cp The other candidate whose data is copied
 */
GOptimizableEntity::GOptimizableEntity(GOptimizableEntity const &cp)
  : Gem::Courtier::GProcessable(cp)
  , Gem::Common::GCommonInterfaceT<GOptimizableEntity>(cp)
  , Interface::GRateableI(cp)
  , pre_processing_disabled_(cp.pre_processing_disabled_)
  , post_processing_disabled_(cp.post_processing_disabled_)
  , stored_results_cnt_(cp.stored_results_cnt_)
  , policy_(cp.policy_) // shared 1:N -- the clone references the same policy
  , assigned_iteration_(cp.assigned_iteration_)
  , validity_level_(cp.validity_level_)
  // The OA-owned scratch is deep-copied (a clone mid-optimization keeps the live personality + adaption
  // state, e.g. an EA child inheriting its parent's sigma).
  , scratch_(cp.scratch_ ? std::make_unique<GAuxiliaryStore>(*cp.scratch_) : nullptr) {
    Gem::Common::copyCloneableSmartPointer(cp.pre_processor_ptr_, pre_processor_ptr_);
    Gem::Common::copyCloneableSmartPointer(cp.post_processor_ptr_, post_processor_ptr_);
}

/******************************************************************************/
/**
 * @brief Installs the shared problem policy (the 1:N feasibility/ranking rules).
 * @param policy The shared policy to reference (must not be empty)
 */
void GOptimizableEntity::setPolicy(std::shared_ptr<GProblemPolicy> policy) {
    if(not policy) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GOptimizableEntity::setPolicy(): Error!" << '\n'
            << "Tried to install an empty problem policy" << '\n'
        );
    }
    policy_ = std::move(policy);
}

/******************************************************************************/
/**
 * @brief Resets every stored result to a default-constructed value.
 */
void GOptimizableEntity::clear_stored_results_vec() {
    // Cannot use range-based for here, as the value type might be a proxy (kept for parity with the
    // historical processing container).
    for(auto it = stored_results_cnt_.begin(); it != stored_results_cnt_.end(); ++it) {
        *it = individual_processing_result();
    }
}

/******************************************************************************/
/**
 * @brief Sets the vector of stored results to a given collection and marks the candidate PROCESSED.
 * @param result_cnt The new result vector (size must match the configured number of stored results)
 * @return The first stored result after the assignment
 */
individual_processing_result
GOptimizableEntity::markAsProcessedWith(std::vector<individual_processing_result> const &result_cnt) {
#ifdef DEBUG
    if(result_cnt.size() != stored_results_cnt_.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GOptimizableEntity::markAsProcessedWith(): Vector dimensions" << '\n'
            << "do not fit: " << result_cnt.size() << " / " << stored_results_cnt_.size() << '\n'
        );
    }
#endif

    stored_results_cnt_ = result_cnt;
    stored_error_descriptions_.clear();
    processing_status_ = Gem::Courtier::processingStatus::PROCESSED;

    return this->stored_results_cnt_.at(0);
}

/******************************************************************************/
/**
 * @brief Read-only retrieval of a stored result. Throws if the PROCESSED flag is not set.
 * @param id The position of the stored result to return
 * @return The stored result at position id
 */
individual_processing_result GOptimizableEntity::getStoredResult(const std::size_t id) const {
    if(not this->is_processed()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GOptimizableEntity::getStoredResult(): Tried to" << '\n'
            << "retrieve stored result while the PROCESSED flag was not set" << '\n'
        );
    }

    return stored_results_cnt_.at(id);
}

/******************************************************************************/
/**
 * @brief Performs the evaluation of this candidate (see the header for the full sequence).
 * @param res_vec Optional pre-computed raw results; if empty, fitnessCalculation() is invoked
 * @return The first stored result after processing
 */
individual_processing_result
GOptimizableEntity::process(const std::vector<individual_processing_result> &res_vec) {
    using Gem::Courtier::processingStatus;

    if(processingStatus::DO_PROCESS != processing_status_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GOptimizableEntity::process(): Function called while processing_status_ was set to "
            << processing_status_ << '\n'
            << "Expected " << processingStatus::DO_PROCESS << '\n'
        );
    }

    stored_error_descriptions_.clear();
    this->clear_stored_results_vec();

    std::ostringstream error_description_stream; // NOLINT(cppcoreguidelines-init-variables)

    // Consult the process-global fault injector once (no-op unless a GFaultInjector is registered -- a
    // single null-pointer check on the default path). A THROW fault is raised inside the try below so it
    // surfaces as EXCEPTION_CAUGHT; a FLAG_ERROR fault is applied AFTER the try/catch (the catch would
    // otherwise force EXCEPTION_CAUGHT, and the try's terminal PROCESSED assignment would clobber it).
    GFaultInjector::Fault injected_fault = GFaultInjector::Fault::NONE;
    if(GFaultInjector *injector = GFaultInjectorRegistry::get(); injector != nullptr) {
        injected_fault = injector->evaluate(*this, this->getRandomEngine());
    }

    try {
        if(injected_fault == GFaultInjector::Fault::THROW) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "Fault injected during GOptimizableEntity::process() (THROW)" << '\n'
            );
        }

        const auto start_time = std::chrono::high_resolution_clock::now();
        this->preProcess_();
        const auto after_pre_processing = std::chrono::high_resolution_clock::now();

        this->runEvaluation_(res_vec);

        // The fitness has now been computed, so the work item is processed. Mark it PROCESSED before
        // post-processing: a post-processor refines an ALREADY-EVALUATED item and rejects a dirty one.
        // If processing flagged an error, the error status is left intact.
        if(not this->has_errors()) {
            processing_status_ = processingStatus::PROCESSED;
        }

        const auto after_processing = std::chrono::high_resolution_clock::now();
        this->postProcess_();
        const auto after_post_processing = std::chrono::high_resolution_clock::now();

        pre_processing_time_ =
            std::chrono::duration<double>(after_pre_processing - start_time).count();
        processing_time_ =
            std::chrono::duration<double>(after_processing - after_pre_processing).count();
        post_processing_time_ =
            std::chrono::duration<double>(after_post_processing - after_processing).count();

        processing_status_ = processingStatus::PROCESSED;
    }
    catch(std::exception &e) {
        processing_status_ = processingStatus::EXCEPTION_CAUGHT;
        error_description_stream << "In GOptimizableEntity::process():" << '\n'
                                 << "Processing has thrown an exception with message" << '\n'
                                 << e.what() << '\n'
                                 << "We will rethrow this exception" << '\n';
    }
    catch(...) {
        processing_status_ = processingStatus::EXCEPTION_CAUGHT;
        error_description_stream << "In GOptimizableEntity::process():" << '\n'
                                 << "Processing has thrown an unknown exception." << '\n';
    }

    // Apply an injected FLAG_ERROR fault now, after the try/catch, so it surfaces as ERROR_FLAGGED
    // (rather than being overridden by the catch's EXCEPTION_CAUGHT or the try's terminal PROCESSED).
    if(injected_fault == GFaultInjector::Fault::FLAG_ERROR && not this->has_errors()) {
        this->force_set_error("Fault injected during GOptimizableEntity::process() (FLAG_ERROR)\n");
    }

    if(this->has_errors()) { // Either an exception was caught or the user flagged an error
        pre_processing_time_ = 0.;
        processing_time_ = 0.;
        post_processing_time_ = 0.;

        this->clear_stored_results_vec();

        if(processingStatus::EXCEPTION_CAUGHT == processing_status_) {
            stored_error_descriptions_ += error_description_stream.str();
        }

        throw Gem::Courtier::g_processing_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace()) << stored_error_descriptions_
        );
    }

    return this->stored_results_cnt_.at(0);
}

/******************************************************************************/
/**
 * @brief The evaluation body run inside process(): feasibility check + fitnessCalculation()/res_vec
 * adoption + the evaluation-policy transform.
 * @param res_vec Optional pre-computed raw results
 */
void GOptimizableEntity::runEvaluation_(const std::vector<individual_processing_result> &res_vec) {
    // Find out whether this is a valid solution (must be called first, to fill validity_level_).
    if(this->fulfillsConstraints(validity_level_) ||
       evaluationPolicy::USESIMPLEEVALUATION == this->getEvaluationPolicy()) {
        double main_raw_result = 0.;

        try {
            if(not res_vec.empty()) {
                if(res_vec.size() != this->getNStoredResults()) {
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                        << "In GOptimizableEntity::runEvaluation_(): Error!" << '\n'
                        << "res_vec has invalid size. Got " << res_vec.size() << '\n'
                        << "Expected " << this->getNStoredResults() << '\n'
                    );
                }

                main_raw_result = res_vec.begin()->rawFitness();

                std::size_t pos = 0;
                for(const auto &res : res_vec) {
                    if(pos == 0) {
                        ++pos;
                        continue; // skip the main raw result
                    }
                    this->setResult(pos, res.rawFitness());
                    ++pos;
                }
            }
            else {
                // With multiple fitness criteria, fitnessCalculation() also sets the additional raw values.
                main_raw_result = this->fitnessCalculation();
            }
        }
        catch(...) {
            this->setAllFitnessTo(this->getWorstCase());
            throw;
        }

        this->setResult(0, main_raw_result);
        this->modifyStoredResult(0).setTransformedFitnessToRaw();

        if(this->error_flagged_by_user()) {
            // The user indicated a problem without throwing: worst-case the whole quality surface.
            this->setAllFitnessTo(this->getWorstCase());
        }
        else {
            for(std::size_t i = 0; i < this->getNStoredResults(); i++) {
                if(evaluationPolicy::USESIGMOID == this->getEvaluationPolicy()) {
                    this->modifyStoredResult(i).setTransformedFitnessWith(
                        [this](const double raw_value) { return policy_->sigmoidTransform(raw_value); }
                    );
                }
                else {
                    this->modifyStoredResult(i).setTransformedFitnessToRaw();
                }
            }
        }
    }
    else {
        // Some constraints were violated. Act on the chosen policy.
        if(evaluationPolicy::USEWORSTCASEFORINVALID == this->getEvaluationPolicy()) {
            this->setAllFitnessTo(this->getWorstCase());
        }
        else if(evaluationPolicy::USESIGMOID == this->getEvaluationPolicy()) {
            double uniform_fitness_value = 0.;
            const double barrier = this->getBarrier();
            if(maxMode::MAXIMIZE == this->getMaxMode()) {
                uniform_fitness_value = (std::numeric_limits<double>::max() == validity_level_)
                                            ? this->getWorstCase()
                                            : -validity_level_ * barrier;
            }
            else {
                uniform_fitness_value = (std::numeric_limits<double>::max() == validity_level_)
                                            ? this->getWorstCase()
                                            : validity_level_ * barrier;
            }
            this->setAllFitnessTo(this->getWorstCase(), uniform_fitness_value);
        }
    }
}

/******************************************************************************/
/**
 * @brief Sets the fitness from a vector of externally-computed raw values (the GPU consumer / external
 * evaluation), applying the feasibility check and evaluation-policy transform, then marking PROCESSED.
 * @param f_cnt A vector of raw fitness values (size must match the criteria count)
 */
void GOptimizableEntity::setFitness_(std::vector<double> const &f_cnt) {
#ifdef DEBUG
    if(f_cnt.size() != this->getNStoredResults()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GOptimizableEntity::setFitness_(): Error!" << '\n'
            << "Invalid size of fitness vector: " << f_cnt.size()
            << ", expected: " << this->getNStoredResults() << '\n'
        );
    }
#endif /* DEBUG */

    if(this->fulfillsConstraints(validity_level_) ||
       evaluationPolicy::USESIMPLEEVALUATION == this->getEvaluationPolicy()) {
        std::vector<individual_processing_result> processing_results(
            f_cnt.size(),
            individual_processing_result()
        );

        std::size_t pos = 0;
        for(auto &p : processing_results) {
            p.reset(f_cnt.at(pos));

            if(evaluationPolicy::USESIGMOID == this->getEvaluationPolicy()) {
                p.setTransformedFitnessWith(
                    [this](const double raw_value) { return policy_->sigmoidTransform(raw_value); }
                );
            }
            else {
                p.setTransformedFitnessToRaw();
            }
            ++pos;
        }

        this->markAsProcessedWith(processing_results);
    }
    else {
        if(evaluationPolicy::USEWORSTCASEFORINVALID == this->getEvaluationPolicy()) {
            this->setAllFitnessTo(this->getWorstCase());
        }
        else if(evaluationPolicy::USESIGMOID == this->getEvaluationPolicy()) {
            double uniform_fitness_value = 0.;
            const double barrier = this->getBarrier();
            if(maxMode::MAXIMIZE == this->getMaxMode()) {
                uniform_fitness_value = (std::numeric_limits<double>::max() == validity_level_)
                                            ? this->getWorstCase()
                                            : -validity_level_ * barrier;
            }
            else {
                uniform_fitness_value = (std::numeric_limits<double>::max() == validity_level_)
                                            ? this->getWorstCase()
                                            : validity_level_ * barrier;
            }
            this->setAllFitnessTo(this->getWorstCase(), uniform_fitness_value);
        }
    }
}

/******************************************************************************/
/**
 * @brief Registers a (raw) result value of the fitness calculation at a given criterion position.
 * @param id The position of the fitness criterion
 * @param value The raw fitness value to register
 */
void GOptimizableEntity::setResult(const std::size_t id, const double value) {
#ifdef DEBUG
    if(id >= this->getNStoredResults()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GOptimizableEntity::setResult(): Error!" << '\n'
            << "Invalid position in vector: " << id << " (expected min 0 and max "
            << this->getNStoredResults() - 1 << ")" << '\n'
        );
    }
#endif /* DEBUG */

    this->modifyStoredResult(id).reset(value);
}

/******************************************************************************/
/**
 * @brief Retrieve the (raw, transformed) fitness tuple at a given evaluation position.
 * @param id The evaluation position (fitness criterion index)
 * @return A (raw, transformed) fitness tuple at the requested position
 */
std::tuple<double, double> GOptimizableEntity::getFitnessTuple(const std::uint32_t id) const {
    return std::make_tuple<double, double>(this->raw_fitness(id), this->transformed_fitness(id));
}

/******************************************************************************/
/**
 * @brief Checks whether this candidate is at least as good as a set of raw boundaries.
 * @param boundaries One boundary value per fitness criterion
 * @return true if every raw fitness is at least as good as its boundary
 */
bool GOptimizableEntity::isGoodEnough(std::vector<double> const &boundaries) {
#ifdef DEBUG
    if(boundaries.size() != this->getNStoredResults()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GOptimizableEntity::isGoodEnough(): Error!" << '\n'
            << "Number of boundaries does not match number of fitness criteria" << '\n'
        );
    }
    if(not this->is_processed()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GOptimizableEntity::isGoodEnough(): Error!" << '\n'
            << "Trying to compare fitness values although the individual isn't processed" << '\n'
        );
    }
#endif /* DEBUG */

    if(maxMode::MAXIMIZE == this->getMaxMode()) {
        for(std::size_t i = 0; i < boundaries.size(); i++) {
            if(this->raw_fitness(i) < boundaries.at(i)) {
                return false;
            }
        }
    }
    else {
        for(std::size_t i = 0; i < boundaries.size(); i++) {
            if(this->raw_fitness(i) > boundaries.at(i)) {
                return false;
            }
        }
    }

    return true;
}

/******************************************************************************/
/**
 * @brief Checks whether this candidate is a valid solution (meant for processed candidates).
 * @return true if the validity level is <= 1 (all constraints fulfilled)
 */
bool GOptimizableEntity::isValid() const {
#ifdef DEBUG
    if(this->is_due_for_processing() || this->has_errors()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GOptimizableEntity::isValid():" << '\n'
            << "Function was called for unprocessed or erroneous individual" << '\n'
        );
    }
#endif

    return validity_level_ <= 1.;
}

/******************************************************************************/
/**
 * @brief Randomly initializes the parameters (marking the item for reprocessing on change).
 * @param am The activity mode selecting which parameters are (re-)initialized
 * @return true if at least one parameter was changed
 */
bool GOptimizableEntity::randomInit(activityMode const &am) {
    bool modifications_made = this->randomInit_(am);
    if(modifications_made) {
        this->mark_as_due_for_processing();
    }
    return modifications_made;
}

/******************************************************************************/
/**
 * @brief Returns all raw fitness results in a std::vector.
 * @return A vector of all stored raw fitness results
 */
std::vector<double> GOptimizableEntity::raw_fitness_vec_() const {
    std::vector<double> result_vec;
    for(std::size_t i = 0; i < this->getNStoredResults(); i++) {
        result_vec.push_back(this->raw_fitness(i));
    }
    return result_vec;
}

/******************************************************************************/
/**
 * @brief Returns all transformed fitness results in a std::vector.
 * @return A vector of all stored transformed fitness results
 */
std::vector<double> GOptimizableEntity::transformed_fitness_vec_() const {
    std::vector<double> result_vec;
    for(std::size_t i = 0; i < this->getNStoredResults(); i++) {
        result_vec.push_back(this->transformed_fitness(i));
    }
    return result_vec;
}

/******************************************************************************/
/**
 * @brief Runs the registered pre-processor (if allowed) on this candidate.
 */
void GOptimizableEntity::preProcess_() {
    if(this->mayBePreProcessed() && pre_processor_ptr_) {
        (*pre_processor_ptr_)(*this);
    }
}

/******************************************************************************/
/**
 * @brief Runs the registered post-processor (if allowed) on this candidate.
 */
void GOptimizableEntity::postProcess_() {
    if(this->mayBePostProcessed() && post_processor_ptr_) {
        (*post_processor_ptr_)(*this);
    }
}

/******************************************************************************/
/**
 * @brief Sets every raw fitness to raw_value and every transformed fitness to transformed_value.
 * @param raw_value The raw value
 * @param transformed_value The transformed value
 */
void GOptimizableEntity::setAllFitnessTo(const double raw_value, const double transformed_value) {
    for(std::size_t i = 0; i < this->getNStoredResults(); i++) {
        this->modifyStoredResult(i).reset(raw_value);
        this->modifyStoredResult(i).setTransformedFitnessTo(transformed_value);
    }
}

/******************************************************************************/
/**
 * @brief @return The sum of all stored transformed fitness values.
 */
double GOptimizableEntity::sumCombiner() const {
    double result = 0.;
    for(std::size_t id = 0; id < this->getNStoredResults(); id++) {
        result += this->transformed_fitness(id);
    }
    return result;
}

/******************************************************************************/
/**
 * @brief @return The sum of the absolute values of all stored transformed fitness values.
 */
double GOptimizableEntity::fabsSumCombiner() const {
    double result = 0.;
    for(std::size_t id = 0; id < this->getNStoredResults(); id++) {
        result += std::abs(this->transformed_fitness(id));
    }
    return result;
}

/******************************************************************************/
/**
 * @brief @return The square root of the sum of squares of all stored transformed fitness values.
 */
double GOptimizableEntity::squaredSumCombiner() const {
    double result = 0.;
    for(std::size_t id = 0; id < this->getNStoredResults(); id++) {
        result += Gem::Common::gsquared(this->transformed_fitness(id));
    }
    return sqrt(result);
}

/******************************************************************************/
/**
 * @brief @param weights The per-criterion weights. @return The square root of the weighed sum of squares.
 */
double GOptimizableEntity::weighedSquaredSumCombiner(std::vector<double> const &weights) const {
    if(this->getNStoredResults() != weights.size()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GOptimizableEntity::weighedSquaredSumCombiner(): Error!" << '\n'
            << "Sizes of results and the weights vector don't match: " << this->getNStoredResults()
            << " / " << weights.size() << '\n'
        );
    }

    double result = 0.;
    auto cit_weights = weights.begin();
    for(std::size_t id = 0; id < this->getNStoredResults(); id++, ++cit_weights) {
        result += Gem::Common::gsquared((*cit_weights) * this->transformed_fitness(id));
    }
    return sqrt(result);
}

/******************************************************************************/
/**
 * @brief Adds local configuration options (eval policy, sigmoid, max mode, adaption limits).
 * @param gpb The parser builder the configuration options are registered with
 */
void GOptimizableEntity::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    // Call our CRTP base class's function (the category root has no GObject parent).
    Gem::Common::GCommonInterfaceT<GOptimizableEntity>::addConfigurationOptions_(gpb);

    gpb.registerFileParameter<evaluationPolicy>(
        "eval_policy",
        Gem::Geneva::evaluationPolicy::USESIMPLEEVALUATION,
        [this](const evaluationPolicy ep) { this->setEvaluationPolicy(ep); }
    ) << "Specifies which strategy should be used to calculate the evaluation:" << '\n'
      << "0 (a.k.a. USESIMPLEEVALUATION): Always call the evaluation function, even for invalid solutions"
      << '\n'
      << "1 (a.k.a. USEWORSTCASEFORINVALID) : Assign the worst possible value to our fitness and evaluate only valid solutions"
      << '\n'
      << "2 (a.k.a. USESIGMOID): Assign a multiple of validity_level_ and sigmoid barrier to invalid solutions, apply a sigmoid function to valid evaluations"
      << '\n';

    gpb.registerFileParameter<double>(
        "steepness",
        Gem::Geneva::FITNESSSIGMOIDSTEEPNESS,
        [this](const double ss) { this->setSteepness(ss); }
    ) << "When using a sigmoid function to transform the individual's fitness," << '\n'
      << "this parameter influences the steepness of the function at the center of the sigmoid." << '\n'
      << "The parameter must have a value > 0.";

    gpb.registerFileParameter<double>(
        "barrier",
        Gem::Geneva::WORSTALLOWEDVALIDFITNESS,
        [this](const double barrier) { this->setBarrier(barrier); }
    ) << "When using a sigmoid function to transform the individual's fitness," << '\n'
      << "this parameter sets the upper/lower boundary of the sigmoid." << '\n'
      << "The parameter must have a value > 0.;";

    gpb.registerFileParameter<maxMode>(
        "maxmode",
        maxMode::MINIMIZE,
        [this](const maxMode mm) { this->setMaxMode(mm); }
    ) << "Specifies whether the individual should be maximized (1) or minimized (0)" << '\n'
      << "Note that minimization is the by far most common option.";
}

/******************************************************************************/
/**
 * @brief Loads the data of another GOptimizableEntity.
 * @param cp The source candidate whose data is copied into this one
 */
void GOptimizableEntity::load_(const GOptimizableEntity *cp) {
    const auto *p_load =
        Gem::Common::g_convert_and_compare<GOptimizableEntity, GOptimizableEntity>(cp, this);

    // Copy the non-generic processing lifecycle state (status, errors, routing counters, timing).
    Gem::Courtier::GProcessable::operator=(*p_load);

    // The plain local members (veto flags, feasibility / best-known state).
    Gem::Common::g_load_members(this->localMembers_(), p_load->localMembers_());

    // The result store is copied directly (it is serialized/loaded but not among the compared members).
    stored_results_cnt_ = p_load->stored_results_cnt_;

    // The cloneable pre-/post-processors are deep-cloned; the shared policy is referenced (1:N).
    Gem::Common::copyCloneableSmartPointer(p_load->pre_processor_ptr_, pre_processor_ptr_);
    Gem::Common::copyCloneableSmartPointer(p_load->post_processor_ptr_, post_processor_ptr_);
    policy_ = p_load->policy_;

    // The OA-owned scratch is deep-copied (it is serialized but not among the compared members).
    if(p_load->scratch_) {
        scratch_ = std::make_unique<GAuxiliaryStore>(*p_load->scratch_);
    }
    else {
        scratch_.reset();
    }
}

/******************************************************************************/
/**
 * @brief Searches for compliance with expectations with respect to another candidate.
 * @param cp The other candidate to compare against
 * @param e The expectation (e.g. equality)
 * @param limit The limit for allowed floating-point deviations
 */
void GOptimizableEntity::compare_(
    GOptimizableEntity const &cp,
    Gem::Common::expectation const &e,
    [[maybe_unused]] double const &limit
) const {
    using namespace Gem::Common;

    const auto *p_load =
        Gem::Common::g_convert_and_compare<GOptimizableEntity, GOptimizableEntity>(cp, this);

    GToken token("GOptimizableEntity", e);

    // Compare our CRTP base data (the category root has no GObject parent) ...
    Gem::Common::compare_base_t<Gem::Common::GCommonInterfaceT<GOptimizableEntity>>(*this, *p_load, token);

    // ... and the plain local data, derived from the single localMembers() declaration. The shared policy
    // is referenced 1:N (compared by configuration is the OA-setup concern, not per-individual equality).
    Gem::Common::g_compare_members(this->localMembers_(), p_load->localMembers_(), token);

    token.evaluate();
}

/******************************************************************************/
} /* namespace Gem::Geneva::Genome */
