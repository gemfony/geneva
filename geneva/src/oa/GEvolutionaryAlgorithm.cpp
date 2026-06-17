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

#include "geneva/oa/GEvolutionaryAlgorithm.hpp"

#include <atomic>
#include <memory>
#include "common/GCommonInterfaceT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "common/GParserBuilder.hpp"
#include "common/GThreadPool.hpp"
#include "courtier/GProcessingContainerT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/GenevaHelperFunctions.hpp"
#ifdef GEM_TESTING
#include "geneva/individuals/GTestIndividual1.hpp"
#endif /* GEM_TESTING */
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/oa/GEvolutionaryAlgorithm_PersonalityTraits.hpp"
#include "geneva/oa/GParChild.hpp"
#include "geneva/oa/GAdaption.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/par/GOptimizableEntityFixedSizePriorityQueue.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <future>
#include <iterator>
#include <ostream>
#include <sstream>
#include <tuple>
#include <vector>

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

/******************************************************************************/

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::OptimizationAlgorithms::GEvolutionaryAlgorithm) // NOLINT

/******************************************************************************/

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/**
 * @brief The default constructor. All initialization work of member variables
 * is done in the class body.
 *
 * The constructor body only ensures a valid default population size (100 individuals,
 * 1 parent) in case the user does not supply these values.
 */
GEvolutionaryAlgorithm::GEvolutionaryAlgorithm() {
    // Make sure we start with a valid population size if the user does not supply these values
    this->setPopulationSizes(100, 1);
}

/******************************************************************************/
/**
  * @brief Searches for compliance with expectations with respect to another object
  * of the same type
  *
  * @param cp A constant reference to another GEvolutionaryAlgorithm object (passed as a GOptimizationAlgorithmBase reference) to compare against
  * @param e The expected outcome of the comparison (e.g. equality or inequality)
  * @param limit The maximum deviation for floating point values (important for similarity checks; unused here)
  */
void GEvolutionaryAlgorithm::compare_(
    const GOptimizationAlgorithmBase &cp // the other object
    ,
    const Gem::Common::expectation &e // the expectation for this object, e.g. equality
    ,
    const double & /*limit*/ // the limit for allowed deviations of floating point types
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GEvolutionaryAlgorithm reference independent of this object and convert the pointer
    const GEvolutionaryAlgorithm *p_load =
        Gem::Common::g_convert_and_compare<GOptimizationAlgorithmBase, GEvolutionaryAlgorithm>(cp, this);

    GToken token("GEvolutionaryAlgorithm", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GParChild>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(localMembers(), p_load->localMembers(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * @brief Resets the settings of this population to what was configured when
 * the optimize()-call was issued
 */
void GEvolutionaryAlgorithm::resetToOptimizationStart_() {
    // There is no more work to be done here, so we simply call the
    // function of the parent class
    GParChild::resetToOptimizationStart_();
}

/******************************************************************************/
/**
  * @brief Sets the sorting scheme. In MUPLUSNU_SINGLEEVAL, new parents will be selected from the entire
  * population, including the old parents. In MUCOMMANU_SINGLEEVAL new parents will be selected
  * from children only. MUNU1PRETAIN_SINGLEEVAL means that the best parent of the last generation
  * will also become a new parent (unless a better child was found). All other parents are
  * selected from children only.
  *
  * @param smode The desired sorting scheme
  */
void GEvolutionaryAlgorithm::setSortingScheme(const sortingMode smode) {
    sorting_mode_ = smode;
}

/******************************************************************************/
/**
  * @brief Retrieves information about the current sorting scheme (see
  * GEvolutionaryAlgorithm::setSortingScheme() for further information).
  *
  * @return The current sorting scheme
  */
sortingMode GEvolutionaryAlgorithm::getSortingScheme() const {
    return sorting_mode_;
}

/******************************************************************************/
/**
  * @brief Extracts all individuals on the pareto front
  *
  * @param pareto_inds An output vector that, on return, holds clones of all individuals currently tagged as being on the pareto front (cleared on entry)
  */
void GEvolutionaryAlgorithm::extractCurrentParetoIndividuals(
    std::vector<std::shared_ptr<gen::GOptimizableEntity>> &pareto_inds
) {
    // Make sure the vector is empty
    pareto_inds.clear();

    for(const auto &ind_ptr : *this) {
        if(ind_ptr->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()
               ->isOnParetoFront()) {
            pareto_inds.push_back(ind_ptr->individual().clone<gen::GOptimizableEntity>());
        }
    }
}

/******************************************************************************/
/**
  * @brief Adds the individuals of this iteration to a priority queue. The
  * queue will be sorted by the first evaluation criterion of the individuals
  * and may either have a limited or unlimited size, depending on user-
  * settings. The procedure is different for pareto optimization, as we only
  * want the individuals on the current pareto front to be added.
  *
  * @param best_individuals The priority queue of globally best individuals to be updated in place
  */
void GEvolutionaryAlgorithm::updateGlobalBestsPQ_(
    gen::GOptimizableEntityFixedSizePriorityQueue &best_individuals
) {
    constexpr bool replace = true;
    constexpr bool donotreplace = false;
    constexpr bool clone = true;

#ifdef DEBUG
    if(this->empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In G_OA_EvolutionaryAlgorithm::updateGlobalBestsPQ() :" << '\n'
            << "Tried to retrieve the best individuals even though the population is empty."
            << '\n'
        );
    }
#endif /* DEBUG */

    switch(sorting_mode_) {
    //----------------------------------------------------------------------------
    case sortingMode::MUPLUSNU_SINGLEEVAL:
    case sortingMode::MUNU1PRETAIN_SINGLEEVAL:
    case sortingMode::MUCOMMANU_SINGLEEVAL:
        best_individuals.add(
            this->data_cnt_.begin(),
            this->data_cnt_.begin() + this->getNParents(),
            clone,
            donotreplace
        );
        // GOptimizationAlgorithmBase::updateGlobalBestsPQ_(best_individuals);
        break;

    //----------------------------------------------------------------------------
    case sortingMode::MUPLUSNU_PARETO:
    case sortingMode::MUCOMMANU_PARETO: {
        // Retrieve all individuals on the pareto front
        std::vector<std::shared_ptr<gen::GOptimizableEntity>> pareto_inds;
        this->extractCurrentParetoIndividuals(pareto_inds);

        // We simply add all parent individuals to the queue. As we only want
        // the individuals on the current pareto front, we replace all members
        // of the current priority queue
        best_individuals.add(pareto_inds, clone, replace);
    } break;

        //----------------------------------------------------------------------------
    }
}

/******************************************************************************/
/**
 * @brief Adds the individuals of this iteration to a priority queue. The
 * queue will be sorted by the first evaluation criterion of the individuals
 * and will be cleared prior to adding the new individuals. This results in
 * the best individuals of the current iteration.
 *
 * @param best_individuals The priority queue of this iteration's best individuals to be updated in place
 */
void GEvolutionaryAlgorithm::updateIterationBestsPQ_(
    gen::GOptimizableEntityFixedSizePriorityQueue &best_individuals
) {
    constexpr bool clone = true;
    constexpr bool donotreplace = false;
    constexpr bool replace = true;

#ifdef DEBUG
    if(this->empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "G_OA_EvolutionaryAlgorithm::updateIterationBestsPQ() :" << '\n'
            << "Tried to retrieve the best individuals even though the population is empty."
            << '\n'
        );
    }
#endif /* DEBUG */

    switch(sorting_mode_) {
    //----------------------------------------------------------------------------
    case sortingMode::MUPLUSNU_SINGLEEVAL:
    case sortingMode::MUNU1PRETAIN_SINGLEEVAL:
    case sortingMode::MUCOMMANU_SINGLEEVAL: {
        best_individuals.add(
            this->data_cnt_.begin(),
            this->data_cnt_.begin() + this->getNParents(),
            clone,
            donotreplace
        );
        // GOptimizationAlgorithmBase::updateIterationBestsPQ_(best_individuals);
    } break;

    //----------------------------------------------------------------------------
    case sortingMode::MUPLUSNU_PARETO:
    case sortingMode::MUCOMMANU_PARETO: {
        // Retrieve all individuals on the pareto front
        std::vector<std::shared_ptr<gen::GOptimizableEntity>> pareto_inds;
        this->extractCurrentParetoIndividuals(pareto_inds);

        // We simply add all parent individuals to the queue. As we only want
        // the individuals on the current pareto front, we replace all members
        // of the current priority queue
        best_individuals.add(pareto_inds, clone, replace);
    } break;

        //----------------------------------------------------------------------------
    }
}

/******************************************************************************/
/**
  * @brief Adds local configuration options to a GParserBuilder object
  *
  * @param gpb The GParserBuilder object to which configuration options should be added
  */
void GEvolutionaryAlgorithm::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    // Call our parent class'es function (registers the shared n_adaption_threads option)
    GParChild::addConfigurationOptions_(gpb);

    // Add local data
    gpb.registerFileParameter<sortingMode>(
        "sorting_method" // The name of the variable
        ,
        DEFAULTEASORTINGMODE // The default value
        ,
        [this](sortingMode sm) { this->setSortingScheme(sm); }
    ) << "The sorting scheme. Options"
      << '\n'
      << "0: MUPLUSNU mode with a single evaluation criterion" << '\n'
      << "1: MUCOMMANU mode with a single evaluation criterion" << '\n'
      << "2: MUCOMMANU mode with single evaluation criterion," << '\n'
      << "   the best parent of the last iteration is retained" << '\n'
      << "   unless a better individual has been found" << '\n'
      << "3: MUPLUSNU mode for multiple evaluation criteria, pareto selection" << '\n'
      << "4: MUCOMMANU mode for multiple evaluation criteria, pareto selection";
}

/******************************************************************************/
/**
  * @brief Loads the data of another GEvolutionaryAlgorithm object.
 *
  * @param cp A pointer to another GEvolutionaryAlgorithm object (passed as a GOptimizationAlgorithmBase pointer)
  */
void GEvolutionaryAlgorithm::load_(const GOptimizationAlgorithmBase *cp) {
    // Check that we are dealing with a GEvolutionaryAlgorithm reference independent
    // of this object and convert the pointer
    const GEvolutionaryAlgorithm *p_load =
        Gem::Common::g_convert_and_compare<GOptimizationAlgorithmBase, GEvolutionaryAlgorithm>(cp, this);

    // First load the parent class's data ...
    GParChild::load_(cp);

    // ... and then our own data, derived from the single localMembers() declaration
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/******************************************************************************/
/**
 * @brief Some error checks related to population sizes
 *
 * Throws a geneva_exception if the number of parents is zero or if the population
 * size is too small for the currently selected sorting mode.
 */
void GEvolutionaryAlgorithm::populationSanityChecks_() const {
    // First check that we have been given a suitable value for the number of parents.
    // Note that a number of checks (e.g. population size != 0) has already been done
    // in the parent class.
    if(this->n_parents_ == 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In G_OA_EvolutionaryAlgorithm::populationSanityChecks(): Error!" << '\n'
            << "Number of parents is set to 0"
        );
    }

    // In MUCOMMANU_SINGLEEVAL mode we want to have at least as many children as parents,
    // whereas MUPLUSNU_SINGLEEVAL only requires the population size to be larger than the
    // number of parents. MUNU1PRETAIN has the same requirements as MUCOMMANU_SINGLEEVAL,
    // as it is theoretically possible that all children are better than the former
    // parents, so that the first parent individual will be replaced.
    std::size_t pop_size = this->getPopulationSize();
    if( // TODO: Why are PARETO modes missing here ?
        ((sorting_mode_ == sortingMode::MUCOMMANU_SINGLEEVAL ||
          sorting_mode_ == sortingMode::MUNU1PRETAIN_SINGLEEVAL) &&
         (pop_size < 2 * this->n_parents_)) ||
        (sorting_mode_ == sortingMode::MUPLUSNU_SINGLEEVAL && pop_size <= this->n_parents_)
    ) {
        std::ostringstream error; // NOLINT(cppcoreguidelines-init-variables)
        error << "In G_OA_EvolutionaryAlgorithm::populationSanityChecks() :" << '\n'
              << "Requested size of population is too small :" << pop_size << " "
              << this->n_parents_ << '\n'
              << "Sorting scheme is ";

        switch(sorting_mode_) {
        case sortingMode::MUPLUSNU_SINGLEEVAL:
            error << "MUPLUSNU_SINGLEEVAL" << '\n';
            break;
        case sortingMode::MUCOMMANU_SINGLEEVAL:
            error << "MUCOMMANU_SINGLEEVAL" << '\n';
            break;
        case sortingMode::MUNU1PRETAIN_SINGLEEVAL:
            error << "MUNU1PRETAIN" << '\n';
            break;
        case sortingMode::MUPLUSNU_PARETO:
            error << "MUPLUSNU_PARETO" << '\n';
            break;
        case sortingMode::MUCOMMANU_PARETO:
            error << "MUCOMMANU_PARETO" << '\n';
            break;
        };

        throw geneva_exception(g_error_streamer(DO_LOG, Gem::Common::timeAndPlace()) << error.str());
    }
}


/******************************************************************************/
/**
  * @brief We submit individuals to the broker connector and wait for processed items.
  *
  * Determines the evaluation range, submits that range for processing, removes any
  * unprocessed or erroneous work items, and repairs the population afterwards.
 */
void GEvolutionaryAlgorithm::runFitnessCalculation_() {
    //--------------------------------------------------------------------------------
    // Start by marking the work to be done in the individuals.
    // "range" will hold the start- and end-points of the range
    // to be worked on
    const std::tuple<std::size_t, std::size_t> range = getEvaluationRange_();

#ifdef DEBUG
    // There should be no situation in which a "clean" child is submitted
    // through this function. There MAY be situations, where in the first iteration
    // parents are clean, e.g. when they were extracted from another optimization.
    for(std::size_t i = this->getNParents(); i < this->size(); i++) {
        if(not this->at(i)->individual().is_due_for_processing()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GEvolutionaryAlgorithm::runFitnessCalculation(): Error!" << '\n'
                << "Tried to evaluate children in range " << std::get<0>(range) << " - "
                << std::get<1>(range) << '\n'
                << "but found \"clean\" individual in position " << i << '\n'
            );
        }
    }

    if(this->size() != this->getDefaultPopulationSize()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GEvolutionaryAlgorithm::runFitnessCalculation(): Error!" << '\n'
            << "Size of data vector (" << this->size() << ") should be "
            << this->getDefaultPopulationSize() << '\n'
        );
    }
#endif

    //--------------------------------------------------------------------------------
    // Submit the [start, end) evaluation range and wait for results. courtier marks the span
    // DO_PROCESS and reconciles it in place -- no per-item flagging needed.
    auto status = this->workOnPopulation(std::get<0>(range), std::get<1>(range));

    //--------------------------------------------------------------------------------
    // Take care of unprocessed items, if these exist
    if(not status.is_complete) {
        std::size_t n_erased =
            std::erase_if(this->data_cnt_, [this](const std::unique_ptr<gen::GIndividualSlot> &p) -> bool {
                return (p->individual().getProcessingStatus() == Gem::Courtier::processingStatus::DO_PROCESS);
            });

#ifdef DEBUG
        glogger << "In GEvolutionaryAlgorithm::runFitnessCalculation(): " << '\n'
                << "Removed " << n_erased << " unprocessed work items in iteration "
                << this->getIteration() << '\n'
                << GLOGGING;
#endif
    }

    // Remove items for which an error has occurred during processing
    if(status.has_errors) {
        std::size_t n_erased = std::erase_if(
            this->data_cnt_,
            [this](const auto &p) -> bool { return p->individual().has_errors(); }
        );

#ifdef DEBUG
        glogger << "In GEvolutionaryAlgorithm::runFitnessCalculation(): " << '\n'
                << "Removed " << n_erased << " erroneous work items in iteration "
                << this->getIteration() << '\n'
                << GLOGGING;
#endif
    }

    //--------------------------------------------------------------------------------
    // Now fix the population -- it may be smaller than its nominal size
    fixAfterJobSubmission();
}


/******************************************************************************/
/**
	* @brief Choose new parents, based on the selection scheme set by the user.
	*
	* Dispatches to the sorting routine matching the current sorting mode, marks the
	* selected parents, and resizes the population back to its nominal size.
	*/
void GEvolutionaryAlgorithm::selectBest_() {
#ifdef DEBUG
    // We require at this stage that at least the default number of
    // children is present. If individuals can get lost in your setting,
    // you must add mechanisms to "repair" the population before this
    // function is called
    if((this->size() - this->n_parents_) < this->default_n_children_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In G_OA_EvolutionaryAlgorithm::select():" << '\n'
            << "Too few children. Got " << (this->size() - this->getNParents()) << "," << '\n'
            << "but was expecting at least " << this->getDefaultNChildren() << '\n'
        );
    }
#endif /* DEBUG */

    switch(sorting_mode_) {
    //----------------------------------------------------------------------------
    case sortingMode::MUPLUSNU_SINGLEEVAL: {
        this->sortMuPlusNuMode();
    } break;

    //----------------------------------------------------------------------------
    case sortingMode::MUNU1PRETAIN_SINGLEEVAL: {
        if(1 == n_parents_ || this->inFirstIteration()) {
            this->sortMuPlusNuMode();
        }
        else {
            this->sortMunu1pretainMode();
        }
    } break;

    //----------------------------------------------------------------------------
    case sortingMode::MUCOMMANU_SINGLEEVAL: {
        if(this->inFirstIteration()) {
            this->sortMuPlusNuMode();
        }
        else {
            this->sortMuCommaNuMode();
        }
    } break;

    //----------------------------------------------------------------------------
    case sortingMode::MUPLUSNU_PARETO:
        this->sortMuPlusNuParetoMode();
        break;

    //----------------------------------------------------------------------------
    case sortingMode::MUCOMMANU_PARETO: {
        if(this->inFirstIteration()) {
            this->sortMuPlusNuParetoMode();
        }
        else {
            this->sortMuCommaNuParetoMode();
        }
    } break;

        //----------------------------------------------------------------------------
    }

    // Let parents know they are parents
    this->markParents();

#ifdef DEBUG
    // Make sure our population is not smaller than its nominal size -- this
    // should have been taken care of in fixAfterJobSubmission() .
    if(this->size() < this->getDefaultPopulationSize()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In G_OA_EvolutionaryAlgorithm::selectBest(): Error!" << '\n'
            << "Size of population is smaller than expected: " << this->size() << " / "
            << this->getDefaultPopulationSize() << '\n'
        );
    }
#endif /* DEBUG */

    ////////////////////////////////////////////////////////////
    // At this point we have a sorted list of individuals and can take care of
    // too many members, so the next iteration finds a "standard" population. This
    // function will remove the last items.
    this->resize(this->getNParents() + this->getDefaultNChildren());

    // Everything should be back to normal ...
}

/******************************************************************************/
/**
  * @brief Retrieves the evaluation range in a given iteration and sorting scheme. Depending on the
  * iteration and sorting scheme, the start point will be different. The end-point is not meant
  * to be inclusive.
  *
  * @return A tuple holding the half-open [start, end) range of population positions inside which evaluation should take place
  */
std::tuple<std::size_t, std::size_t> GEvolutionaryAlgorithm::getEvaluationRange_() const {
    // We evaluate all individuals in the first iteration This happens so pluggable
    // optimization monitors do not need to distinguish between algorithms, and
    // MUCOMMANU selection may fall back to MUPLUSNU in the first iteration.
    return std::make_tuple<std::size_t, std::size_t>(
        this->inFirstIteration() ? static_cast<std::size_t>(0) : this->getNParents(),
        this->size()
    );
}

/******************************************************************************/
/**
  * @brief Retrieve a GPersonalityTraits object belonging to this algorithm
  *
  * @return A shared pointer to a freshly created GEvolutionaryAlgorithm_PersonalityTraits object
  */
std::shared_ptr<GPersonalityTraits> GEvolutionaryAlgorithm::getPersonalityTraits_() const {
    return std::make_shared<GEvolutionaryAlgorithm_PersonalityTraits>();
}

/******************************************************************************/
/**
 * @brief Selection, MUPLUSNU_SINGLEEVAL style. Note that not all individuals of the population (including parents)
 * are sorted -- only the n_parents best individuals are identified. The quality of the population can only
 * increase, but the optimization will stall more easily in MUPLUSNU_SINGLEEVAL mode.
 */
void GEvolutionaryAlgorithm::sortMuPlusNuMode() {
#ifdef DEBUG
    // Check that we do not accidently trigger value calculation
    std::size_t pos = 0;
    for(auto const &ind_ptr : *this) {
        if(ind_ptr->individual().is_due_for_processing()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GEvolutionaryAlgorithm::sortMuplusnuMode(): Error!" << '\n'
                << "In iteration " << GOptimizationAlgorithmBase::getIteration()
                << ": Found individual in position " << pos << '\n'
                << " that is unprocessed." << '\n'
            );
        }
        pos++;
    }
#endif /* DEBUG */

    // Only partially sort the arrays
    std::partial_sort(
        GOptimizationAlgorithmBase::data_cnt_.begin(),
        GOptimizationAlgorithmBase::data_cnt_.begin() + n_parents_,
        GOptimizationAlgorithmBase::data_cnt_.end(),
        [](const auto &x_ptr, const auto &y_ptr) -> bool {
            return minOnly_transformed_fitness(x_ptr->individual()) < minOnly_transformed_fitness(y_ptr->individual());
        }
    );
}

/******************************************************************************/
/**
 * @brief Selection, MUCOMMANU_SINGLEEVAL style. New parents are selected from children only. The quality
 * of the population may decrease occasionally from generation to generation, but the
 * optimization is less likely to stall.
 */
void GEvolutionaryAlgorithm::sortMuCommaNuMode() {
#ifdef DEBUG
    if(GOptimizationAlgorithmBase::inFirstIteration()) {
        // Check that we do not accidentally trigger value calculation -- check the whole range
        typename GEvolutionaryAlgorithm::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            if((*it)->individual().is_due_for_processing()) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GEvolutionaryAlgorithm::sortMucommanuMode(): Error!" << '\n'
                    << "In iteration " << GOptimizationAlgorithmBase::getIteration()
                    << ": Found individual in position " << std::distance(this->begin(), it)
                    << '\n'
                    << " whose dirty flag is set." << '\n'
                );
            }
        }
    }
    else {
        // Check that we do not accidentally trigger value calculation -- check children only
        typename GEvolutionaryAlgorithm::iterator it;
        for(it = this->begin() + n_parents_; it != this->end(); ++it) {
            if((*it)->individual().is_due_for_processing()) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GEvolutionaryAlgorithm::sortMucommanuMode(): Error!" << '\n'
                    << "In iteration " << GOptimizationAlgorithmBase::getIteration()
                    << ": Found individual in position " << std::distance(this->begin(), it)
                    << '\n'
                    << " which is unprocessed." << '\n'
                );
            }
        }
    }
#endif /* DEBUG */

    // Only sort the children
    std::partial_sort(
        GOptimizationAlgorithmBase::data_cnt_.begin() + n_parents_,
        GOptimizationAlgorithmBase::data_cnt_.begin() + 2 * n_parents_,
        GOptimizationAlgorithmBase::data_cnt_.end(),
        [](const auto &x_ptr, const auto &y_ptr) -> bool {
            return minOnly_transformed_fitness(x_ptr->individual()) < minOnly_transformed_fitness(y_ptr->individual());
        }
    );

    std::swap_ranges(
        GOptimizationAlgorithmBase::data_cnt_.begin(),
        GOptimizationAlgorithmBase::data_cnt_.begin() + n_parents_,
        GOptimizationAlgorithmBase::data_cnt_.begin() + n_parents_
    );
}

/******************************************************************************/
/**
 * @brief Selection, MUNU1PRETAIN_SINGLEEVAL style. This is a hybrid between MUPLUSNU_SINGLEEVAL and MUCOMMANU_SINGLEEVAL
 * mode. If a better child was found than the best parent of the last generation,
 * all former parents are replaced. If no better child was found than the best
 * parent of the last generation, then this parent stays in place. All other parents
 * are replaced by the (nParents_-1) best children. The scheme falls back to MUPLUSNU_SINGLEEVAL
 * mode, if only one parent is available, or if this is the first generation (so we
 * do not accidentally trigger value calculation).
 */
void GEvolutionaryAlgorithm::sortMunu1pretainMode() {
#ifdef DEBUG
    // Check that we do not accidentally trigger value calculation
    typename GEvolutionaryAlgorithm::iterator it;
    for(it = this->begin() + n_parents_; it != this->end(); ++it) {
        if((*it)->individual().is_due_for_processing()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GEvolutionaryAlgorithm::sortMunu1pretainMode(): Error!" << '\n'
                << "In iteration " << GOptimizationAlgorithmBase::getIteration()
                << ": Found individual in position " << std::distance(this->begin(), it)
                << '\n'
                << " whose dirty flag is set." << '\n'
            );
        }
    }
#endif /* DEBUG */

    // Sort the children
    std::partial_sort(
        GOptimizationAlgorithmBase::data_cnt_.begin() + n_parents_,
        GOptimizationAlgorithmBase::data_cnt_.begin() + 2 * n_parents_,
        GOptimizationAlgorithmBase::data_cnt_.end(),
        [](const auto &x_ptr, const auto &y_ptr) -> bool {
            return minOnly_transformed_fitness(x_ptr->individual()) < minOnly_transformed_fitness(y_ptr->individual());
        }
    );

    // Retrieve the best child's and the last generation's best parent's fitness
    double best_tranformed_child_fitness_min_only = minOnly_transformed_fitness(
        (*(GOptimizationAlgorithmBase::data_cnt_.begin() + n_parents_))->individual()
    );
    double best_tranformed_parent_fitness_min_only =
        minOnly_transformed_fitness((*(GOptimizationAlgorithmBase::data_cnt_.begin()))->individual());

    // Leave the best parent in place, if no better child was found
    if(best_tranformed_child_fitness_min_only < best_tranformed_parent_fitness_min_only) {
        // A better child was found. Overwrite all parents
        std::swap_ranges(
            GOptimizationAlgorithmBase::data_cnt_.begin(),
            GOptimizationAlgorithmBase::data_cnt_.begin() + n_parents_,
            GOptimizationAlgorithmBase::data_cnt_.begin() + n_parents_
        );
    }
    else {
        std::swap_ranges(
            GOptimizationAlgorithmBase::data_cnt_.begin() + 1,
            GOptimizationAlgorithmBase::data_cnt_.begin() + n_parents_,
            GOptimizationAlgorithmBase::data_cnt_.begin() + n_parents_
        );
    }
}

/******************************************************************************/
/**
  * @brief Selection according to the pareto tag, also taking into account the parents of a population (i.e. in MUPLUSNU
  * mode). This is used in conjunction with multi-criterion optimization. See e.g.
  * http://en.wikipedia.org/wiki/Pareto_efficiency for a discussion of this topic.
  */
void GEvolutionaryAlgorithm::sortMuPlusNuParetoMode() {
    typename GEvolutionaryAlgorithm::iterator it;
    typename GEvolutionaryAlgorithm::iterator it_cmp;

    // A PARETO sorting mode was requested. With only a single evaluation criterion Pareto
    // selection is degenerate (the front collapses to the single best individual), so we fall back
    // to the single-eval MUPLUSNU sort. That fallback is mathematically correct, but a user who
    // selected a PARETO mode almost certainly intended multi-objective optimization -- warn once so
    // a misconfigured individual (returning only one fitness) is not silently treated as single-eval.
    it = this->begin();
    if(not(*it)->individual().hasMultipleFitnessCriteria()) {
        static std::atomic<bool> warned{false};
        if(not warned.exchange(true)) {
            glogger << "In GEvolutionaryAlgorithm::sortMuPlusNuParetoMode(): Warning!" << '\n'
                    << "A PARETO sorting mode was selected, but the individuals expose only a single" << '\n'
                    << "fitness criterion. Pareto selection therefore degenerates to single-objective" << '\n'
                    << "MUPLUSNU selection. If you intended multi-objective optimization, make your" << '\n'
                    << "individual return multiple fitness criteria; otherwise select a single-evaluation" << '\n'
                    << "sorting mode to silence this warning." << '\n'
                    << GWARNING;
        }
        this->sortMuPlusNuMode();
        return;
    }

    // Mark all individuals as being on the pareto front initially
    for(const auto &ind : *this) {
        ind
            ->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()
            ->resetParetoTag();
    }

    // Compare all parameters
    for(it = this->begin(); it != this->end(); ++it) {
        for(it_cmp = it + 1; it_cmp != this->end(); ++it_cmp) {
            // If we already know that this individual is *not*
            // on the front we do not have to do any tests
            if(not(*it_cmp)
                      ->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()
                      ->isOnParetoFront()) {
                continue;
            }

            // Check if it dominates it_cmp. If so, mark it accordingly
            if(aDominatesB((*it)->individualPtr(), (*it_cmp)->individualPtr())) {
                (*it_cmp)
                    ->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()
                    ->setIsNotOnParetoFront();
            }

            // If a it dominated by it_cmp, we mark it accordingly and break the loop
            if(aDominatesB((*it_cmp)->individualPtr(), (*it)->individualPtr())) {
                (*it)
                    ->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()
                    ->setIsNotOnParetoFront();
                break;
            }
        }
    }

    // At this point we have tagged all individuals according to whether or not they are
    // on the pareto front. Lets sort them accordingly, bringing individuals with the
    // pareto tag to the front of the collection.
    sort(
        this->begin(),
        this->end(),
        [](const auto &x, const auto &y) {
            return x->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()
                       ->isOnParetoFront() >
                   y->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()
                       ->isOnParetoFront();
        }
    );

    // Count the number of individuals on the pareto front
    std::size_t n_individuals_on_pareto_front = 0;
    for(const auto &ind : *this) {
        if(ind
               ->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()
               ->isOnParetoFront()) {
            n_individuals_on_pareto_front++;
        }
    }

    // If the number of individuals on the pareto front exceeds the number of parents, we
    // do not want to introduce a bias by selecting only the first nParent individuals. Hence
    // we randomly shuffle them. Note that not all individuals on the pareto front might survive,
    // as subsequent iterations will only take into account parents for the reproduction step.
    // If fewer individuals are on the pareto front than there are parents, then we want the
    // remaining parent positions to be filled up with the non-pareto-front individuals with
    // the best minOnly_fitness(0), i.e. with the best "master" fitness (transformed to take into
    // account minimization and maximization).
    if(n_individuals_on_pareto_front > this->getNParents()) {
        // randomly shuffle pareto-front individuals to avoid a bias
        std::shuffle(this->begin(), this->begin() + n_individuals_on_pareto_front, this->gr_);
    }
    else if(n_individuals_on_pareto_front < this->getNParents()) {
        // Sort the non-pareto-front individuals according to their master fitness
        std::partial_sort(
            this->begin() + n_individuals_on_pareto_front,
            this->begin() + this->n_parents_,
            this->end(),
            [](const auto &x_ptr,
               const auto &y_ptr) -> bool {
                return minOnly_transformed_fitness(x_ptr->individual()) < minOnly_transformed_fitness(y_ptr->individual());
            }
        );
    }

    // Finally, we sort the parents only according to their master fitness. This is meant
    // to give some sense to the value recombination scheme. It won't change much in case of the
    // random recombination scheme.
    std::sort(
        this->begin(),
        this->begin() + this->n_parents_,
        [](const auto &x_ptr, const auto &y_ptr) -> bool {
            return minOnly_transformed_fitness(x_ptr->individual()) < minOnly_transformed_fitness(y_ptr->individual());
        }
    );
}

/******************************************************************************/
/**
 * @brief Selection according to the pareto tag, not taking into account the parents of a population (i.e. in MUCOMMANU
  * mode). This is used in conjunction with multi-criterion optimization. See e.g.
  * http://en.wikipedia.org/wiki/Pareto_efficiency for a discussion of this topic.
  */
void GEvolutionaryAlgorithm::sortMuCommaNuParetoMode() {
    typename GEvolutionaryAlgorithm::iterator it;
    typename GEvolutionaryAlgorithm::iterator it_cmp;

    // See sortMuPlusNuParetoMode() for the rationale: a single evaluation criterion makes Pareto
    // selection degenerate, so we fall back to the single-eval MUCOMMANU sort -- correct, but warn
    // once so a misconfigured single-fitness individual is not silently treated as single-eval.
    it = this->begin();
    if(not(*it)->individual().hasMultipleFitnessCriteria()) {
        static std::atomic<bool> warned{false};
        if(not warned.exchange(true)) {
            glogger << "In GEvolutionaryAlgorithm::sortMuCommaNuParetoMode(): Warning!" << '\n'
                    << "A PARETO sorting mode was selected, but the individuals expose only a single" << '\n'
                    << "fitness criterion. Pareto selection therefore degenerates to single-objective" << '\n'
                    << "MUCOMMANU selection. If you intended multi-objective optimization, make your" << '\n'
                    << "individual return multiple fitness criteria; otherwise select a single-evaluation" << '\n'
                    << "sorting mode to silence this warning." << '\n'
                    << GWARNING;
        }
        this->sortMuCommaNuMode();
        return;
    }

    // Mark the last iterations parents as not being on the pareto front
    for(it = this->begin(); it != this->begin() + this->n_parents_; ++it) {
        (*it)
            ->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()
            ->setIsNotOnParetoFront();
    }

    // Mark all children as being on the pareto front initially
    for(it = this->begin() + this->n_parents_; it != this->end(); ++it) {
        (*it)
            ->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()
            ->resetParetoTag();
    }

    // Compare all parameters of all children
    for(it = this->begin() + this->n_parents_; it != this->end(); ++it) {
        for(it_cmp = it + 1; it_cmp != this->end(); ++it_cmp) {
            // If we already know that this individual is *not*
            // on the front we do not have to do any tests
            if(not(*it_cmp)
                      ->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()
                      ->isOnParetoFront()) {
                continue;
            }

            // Check if it dominates it_cmp. If so, mark it accordingly
            if(aDominatesB((*it)->individualPtr(), (*it_cmp)->individualPtr())) {
                (*it_cmp)
                    ->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()
                    ->setIsNotOnParetoFront();
            }

            // If a it dominated by it_cmp, we mark it accordingly and break the loop
            if(aDominatesB((*it_cmp)->individualPtr(), (*it)->individualPtr())) {
                (*it)
                    ->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()
                    ->setIsNotOnParetoFront();
                break;
            }
        }
    }

    // At this point we have tagged all children according to whether or not they are
    // on the pareto front. Lets sort them accordingly, bringing individuals with the
    // pareto tag to the front of the population. Note that parents have been manually
    // tagged as not being on the pareto front in the beginning of this function, so
    // sorting the individuals according to the pareto tag will move former parents out
    // of the parents section.
    sort(
        this->begin(),
        this->end(),
        [](const auto &x, const auto &y) {
            return x->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()
                       ->isOnParetoFront() >
                   y->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()
                       ->isOnParetoFront();
        }
    );

    // Count the number of individuals on the pareto front
    std::size_t n_individuals_on_pareto_front = 0;
    for(const auto &ind : *this) {
        if(ind
               ->template getPersonalityTraits<GEvolutionaryAlgorithm_PersonalityTraits>()
               ->isOnParetoFront()) {
            n_individuals_on_pareto_front++;
        }
    }

    // If the number of individuals on the pareto front exceeds the number of parents, we
    // do not want to introduce a bias by selecting only the first nParent individuals. Hence
    // we randomly shuffle them. Note that not all individuals on the pareto front might survive,
    // as subsequent iterations will only take into account parents for the reproduction step.
    // If fewer individuals are on the pareto front than there are parents, then we want the
    // remaining parent positions to be filled up with the non-pareto-front individuals with
    // the best minOnly_fitness(0), i.e. with the best "master" fitness, transformed to take into account
    // minimization and maximization. Note that, unlike MUCOMMANU_SINGLEEVAL
    // this implies the possibility that former parents are "elected" as new parents again.
    if(n_individuals_on_pareto_front > this->getNParents()) {
        // randomly shuffle pareto-front individuals to avoid a bias
        std::shuffle(this->begin(), this->begin() + n_individuals_on_pareto_front, this->gr_);
    }
    else if(n_individuals_on_pareto_front < this->getNParents()) {
        // Sort the non-pareto-front individuals according to their master fitness
        std::partial_sort(
            this->begin() + n_individuals_on_pareto_front,
            this->begin() + this->n_parents_,
            this->end(),
            [](const auto &x_ptr,
               const auto &y_ptr) -> bool {
                return minOnly_transformed_fitness(x_ptr->individual()) < minOnly_transformed_fitness(y_ptr->individual());
            }
        );
    }

    // Finally, we sort the parents only according to their master fitness. This is meant
    // to give some sense to the value recombination scheme. It won't change much in case of the
    // random recombination scheme.
    std::sort(
        this->begin(),
        this->begin() + this->n_parents_,
        [](const auto &x_ptr, const auto &y_ptr) -> bool {
            return minOnly_transformed_fitness(x_ptr->individual()) < minOnly_transformed_fitness(y_ptr->individual());
        }
    );
}

/******************************************************************************/
/**
  * @brief Determines whether the first individual dominates the second.
  *
  * @param x_ptr The individual that is assumed to dominate (none of its fitness criteria may be worse than the corresponding criterion of y_ptr)
  * @param y_ptr The individual that is assumed to be dominated
  * @return A boolean indicating whether the first individual dominates the second
  */
bool GEvolutionaryAlgorithm::aDominatesB(
    const std::unique_ptr<gen::GOptimizableEntity> &x_ptr,
    const std::unique_ptr<gen::GOptimizableEntity> &y_ptr
) const {
    std::size_t n_criteria_x =
        x_ptr->getNStoredResults(); // NOLINT(cppcoreguidelines-init-variables)

#ifdef DEBUG
    std::size_t n_criteria_y =
        y_ptr->getNStoredResults(); // NOLINT(cppcoreguidelines-init-variables)
    if(n_criteria_x != n_criteria_y) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In G_OA_EvolutionaryAlgorithm::aDominatesB(): Error!" << '\n'
            << "Number of fitness criteria differ: " << n_criteria_x << " / " << n_criteria_y
            << '\n'
        );
    }
#endif

    // x dominates y if none of its fitness criteria is worse than the corresponding criterion of y.
    auto m = x_ptr->getMaxMode();
    for(std::size_t i = 0; i < n_criteria_x; i++) {
        if(isWorse(x_ptr->transformed_fitness(i), y_ptr->transformed_fitness(i), m)) {
            return false;
        }
    }

    return true;
}

/******************************************************************************/
/**
  * @brief Applies modifications to this object. This is needed for testing purposes
  *
  * @return A boolean which indicates whether modifications were made
  */
bool GEvolutionaryAlgorithm::modify_GUnitTests_() {
#ifdef GEM_TESTING

    bool result = false;

    // Call the parent class'es function
    if(GParChild::modify_GUnitTests_()) {
        result = true;
    }

    if(sortingMode::MUPLUSNU_SINGLEEVAL == this->getSortingScheme()) {
        this->setSortingScheme(sortingMode::MUCOMMANU_SINGLEEVAL);
    }
    else {
        this->setSortingScheme(sortingMode::MUPLUSNU_SINGLEEVAL);
    }
    result = true;

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("G_OA_EvolutionaryAlgorithm::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
  * @brief Fills the collection with individuals.
  *
  * @param n_individuals The number of individuals (GTestIndividual1 instances) that should be added to the collection
  */
void GEvolutionaryAlgorithm::fillWithObjects(const std::size_t &n_individuals) {
#ifdef GEM_TESTING
    // Clear the collection, so we can start fresh
    CHECK_NOTHROW(this->clear());

    // Add some some
    for(std::size_t i = 0; i < n_individuals; i++) {
        this->push_back(std::make_unique<gen::GIndividualSlot>(
            std::make_unique<Gem::Geneva::Individuals::GTestIndividual1>()));
    }

    // Make sure we have unique data items
    for(const auto &ind_ptr : *this) {
        ind_ptr->individual().randomInit(activityMode::ALLPARAMETERS);
    }

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset("G_OA_EvolutionaryAlgorithm::fillWithObjects", "GEM_TESTING");
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
  * @brief Performs self tests that are expected to succeed. This is needed for testing purposes
  */
void GEvolutionaryAlgorithm::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Call the parent class'es function
    GParChild::specificTestsNoFailureExpected_GUnitTests_();

    //------------------------------------------------------------------------------

    {
        // Call the parent class'es function
        std::shared_ptr<GEvolutionaryAlgorithm> p_test =
            this->template clone<GEvolutionaryAlgorithm>();

        // Fill p_test with individuals
        p_test->fillWithObjects(100);

        // Run the parent class'es tests
        p_test->GParChild::specificTestsNoFailureExpected_GUnitTests_();
    }

    //------------------------------------------------------------------------------

    {
        // Check setting and retrieval of the population size and number of parents/children
        std::shared_ptr<GEvolutionaryAlgorithm> p_test =
            this->template clone<GEvolutionaryAlgorithm>();

        // Set the default population size and number of children to different numbers
        for(std::size_t n_children = 5; n_children < 10; n_children++) {
            for(std::size_t n_parents = 1; n_parents < n_children; n_parents++) {
                // Clear the collection
                CHECK_NOTHROW(p_test->clear());

                // Add the required number of individuals
                p_test->fillWithObjects(n_parents + n_children);

                CHECK_NOTHROW(p_test->setPopulationSizes(n_parents + n_children, n_parents));

                // Check that the number of parents is as expected
                INFO(
                    "p_test->getNParents() == " << p_test->getNParents() << ", n_parents = "
                                                << n_parents << ", size = " << p_test->size()
                );
                CHECK(p_test->getNParents() == n_parents);

                // Check that the actual number of children has the same value
                INFO(
                    "p_test->getNChildren() = " << p_test->getNChildren()
                                                << ", n_children = " << n_children
                );
                CHECK(p_test->getNChildren() == n_children);
            }
        }
    }

    //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GEvolutionaryAlgorithm::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
  * @brief Performs self tests that are expected to fail. This is needed for testing purposes
  */
void GEvolutionaryAlgorithm::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    // Call the parent class'es function
    GParChild::specificTestsFailuresExpected_GUnitTests_();

#else  /* GEM_TESTING */
    Gem::Common::condnotset(
        "GEvolutionaryAlgorithm::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/**
 * @brief Streams a summary of the parent individuals' fitness values to an output stream.
 *
 * For each parent individual, the raw and transformed fitness values are printed on one line.
 *
 * @param os The output stream to write to
 * @param pop The GEvolutionaryAlgorithm population whose parents should be summarised
 * @return A reference to the output stream, for chaining
 */
std::ostream &operator<<(std::ostream &os, const GEvolutionaryAlgorithm &pop) {
    os << '\n' << '\n';
    for(auto it = pop.begin(); it != pop.begin() + pop.getNParents(); ++it) {
        os << (*it)->individual().raw_fitness() << " " << (*it)->individual().transformed_fitness() << '\n';
    }
    os << "***************************************" << '\n';

    return os;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} // namespace Gem::Geneva::OptimizationAlgorithms
