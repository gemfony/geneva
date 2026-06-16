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

#include "geneva/par/GOptimizableEntityFixedSizePriorityQueue.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GFixedSizePriorityQueueT.hpp"
#include "common/GLogger.hpp"
#include "geneva/GenevaHelperFunctions.hpp"
#include "geneva/ind/GIndividualSlot.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Genome::GOptimizableEntityFixedSizePriorityQueue) // NOLINT

namespace Gem::Geneva::Genome {

/******************************************************************************/
/**
	 * Initialization with the maximum size. The GOptimizableEntityFixedSizePriorityQueue is
	 * targetted at optimization algorithms, which only understand "minimization". Hence
	 * "lower is better" is the only allowed mode of operation of this priority queue.
	 */
GOptimizableEntityFixedSizePriorityQueue::GOptimizableEntityFixedSizePriorityQueue(
    const std::size_t &max_size
)
  : Gem::Common::GFixedSizePriorityQueueT<GOptimizableEntity>(
        max_size,
        Gem::Common::sortOrder::LOWERISBETTER
    ) { /* nothing */
}

/******************************************************************************/
/**
	 * Emits a name for this class / object
	 */
std::string GOptimizableEntityFixedSizePriorityQueue::name_() const {
    return std::string("GOptimizableEntityFixedSizePriorityQueue");
}

/******************************************************************************/
/**
	 * Searches for compliance with expectations with respect to another object
	 * of the same type
	 *
	 * @param cp A constant reference to another GOptimizableEntityFixedSizePriorityQueue object
	 * @param e The expected outcome of the comparison
	 */
void GOptimizableEntityFixedSizePriorityQueue::compare_(
    const Gem::Common::GFixedSizePriorityQueueT<GOptimizableEntity> &cp // the other object
    ,
    const Gem::Common::expectation &e // the expectation for this object, e.g. equality
    ,
    const double & /*limit*/ // the limit for allowed deviations of floating point types
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GOptimizableEntityFixedSizePriorityQueue reference independent of this object and convert the pointer
    const GOptimizableEntityFixedSizePriorityQueue *p_load =
        Gem::Common::g_convert_and_compare(cp, this);

    GToken token("GOptimizableEntityFixedSizePriorityQueue", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<Gem::Common::GFixedSizePriorityQueueT<GOptimizableEntity>>(
        *this,
        *p_load,
        token
    );

    // ... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
	 * Loads the data of another GOptimizableEntityFixedSizePriorityQueue object, camouflaged as a GFixedSizePriorityQueueT<GOptimizableEntity>
	 */
void GOptimizableEntityFixedSizePriorityQueue::load_(
    const Gem::Common::GFixedSizePriorityQueueT<GOptimizableEntity> *cp
) { // NOLINT(misc-unused-parameters)
    // Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
    // *** currently not needed ***
    // const GOptimizableEntityFixedSizePriorityQueue *p_load = Gem::Common::g_convert_and_compare(cp, this);

    // Load our parent class'es data ...
    Gem::Common::GFixedSizePriorityQueueT<GOptimizableEntity>::load_(cp);

    // ... no local data
}

/******************************************************************************/
/**
	 * Creates a deep clone of this object
	 */
Gem::Common::GFixedSizePriorityQueueT<GOptimizableEntity> *
GOptimizableEntityFixedSizePriorityQueue::clone_() const {
    return new GOptimizableEntityFixedSizePriorityQueue(*this);
}

/******************************************************************************/
/**
	 * Checks whether no item has the dirty flag set
	 */
bool GOptimizableEntityFixedSizePriorityQueue::allClean(std::size_t &pos) const {
    pos = 0;
    for(const auto &item_ptr : data_deq_) {
        if(not item_ptr->is_processed()) {
            return false;
        }
        pos++;
    }

    return true;
}

/******************************************************************************/
/**
	 * Emits information about the "dirty flag" of all items
	 */
std::string GOptimizableEntityFixedSizePriorityQueue::getCleanStatus() const {
    std::size_t pos = 0;
    std::ostringstream oss; // NOLINT(cppcoreguidelines-init-variables)
    for(const auto &item_ptr : data_deq_) {
        oss << "(" << pos++ << ", " << (not item_ptr->is_processed() ? "d" : "c") << ") ";
    }

    return oss.str();
}

/******************************************************************************/
/**
	 * Checks whether an Item is valid, i.e. holds a GOptimizableEntity item and has
	 * already been evaluated.
	 */
bool GOptimizableEntityFixedSizePriorityQueue::isValid(
    const std::shared_ptr<GOptimizableEntity> &item_ptr
) const {
    if(not item_ptr) {
        return false; // Empty
    }
    if(not item_ptr->is_processed()) {
        return false;
    } // The item has not been worked on

    // Everything ok
    return true;
}

/******************************************************************************/
/**
	 * Evaluates a single work item, so that it can be sorted. Note that this function
	 * will throw in DEBUG mode, if the dirty flag of item is set. Note that the function
	 * uses the primary evaluation criterion only.
	 */
double GOptimizableEntityFixedSizePriorityQueue::evaluation(
    const std::shared_ptr<GOptimizableEntity> &item_ptr
) const {
    return minOnly_transformed_fitness(*item_ptr);
}

/******************************************************************************/
/**
	 * Adds items in a range to the priority queue
	 */
void GOptimizableEntityFixedSizePriorityQueue::add(
    std::vector<std::shared_ptr<GOptimizableEntity>>::const_iterator begin,
    std::vector<std::shared_ptr<GOptimizableEntity>>::const_iterator end,
    bool do_clone,
    bool do_replace
) {
    // Create a std::vector containing only processed items. We only want
    // to add "clean" (i.e. processed) individuals to the queue.
    std::vector<std::shared_ptr<GOptimizableEntity>> processed_cnt(std::distance(begin, end));
    auto it = std::copy_if(
        begin,
        end,
        processed_cnt.begin(),
        [](const std::shared_ptr<GOptimizableEntity> &item_ptr) { return item_ptr->is_processed(); }
    );
    processed_cnt.resize(std::distance(processed_cnt.begin(), it));

    // Some error checking -- it should not happen that no processed items are found
    if(processed_cnt.empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GOptimizableEntityFixedSizePriorityQueue::add(range): Error!" << '\n'
            << "Container is empty when it should not be!" << '\n'
        );
    }

    Gem::Common::GFixedSizePriorityQueueT<GOptimizableEntity>::add(
        processed_cnt.begin(),
        processed_cnt.end(),
        do_clone,
        do_replace
    );
}

/******************************************************************************/
/**
	 * Adds the items in the items_cnt vector to the queue. This overload makes sure
	 * that only processed items (i.e. without errors and with the PROCESSED flag) are
	 * entered into the priority queue.
	 */
void GOptimizableEntityFixedSizePriorityQueue::add(
    std::vector<std::shared_ptr<GOptimizableEntity>> const &items_cnt,
    const bool do_clone,
    const bool do_replace
) {
    // Create a std::vector containing only processed items. We only want
    // to add "clean" (i.e. processed) individuals to the queue.
    std::vector<std::shared_ptr<GOptimizableEntity>> processed_cnt(items_cnt.size());
    auto it = std::copy_if(
        items_cnt.begin(),
        items_cnt.end(),
        processed_cnt.begin(),
        [](const std::shared_ptr<GOptimizableEntity> &item_ptr) { return item_ptr->is_processed(); }
    );
    processed_cnt.resize(std::distance(processed_cnt.begin(), it));

    // Some error checking -- it should not happen that no processed items are found
    if(processed_cnt.empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GOptimizableEntityFixedSizePriorityQueue::add(vec): Error!" << '\n'
            << "Container is empty when it should not be!" << '\n'
        );
    }

    Gem::Common::GFixedSizePriorityQueueT<GOptimizableEntity>::add(processed_cnt, do_clone, do_replace);
}

/******************************************************************************/
/**
	 * Adds a single item to the queue. his overload makes sure
	 * that only processed items (i.e. without errors and with the PROCESSED flag) are
	 * entered into the priority queue.
	 */
void GOptimizableEntityFixedSizePriorityQueue::add(
    std::shared_ptr<GOptimizableEntity> const &item_ptr,
    const bool do_clone
) {
    if(item_ptr && item_ptr->is_processed()) {
        Gem::Common::GFixedSizePriorityQueueT<GOptimizableEntity>::add(item_ptr, do_clone);
    }
}

/******************************************************************************/
/**
	 * Boundary overload: adds the individuals of a unique_ptr-owned population. The population owns its
	 * individuals by unique_ptr, while this archive keeps its own shared_ptr clones, so we clone each
	 * individual across the ownership boundary and hand the (already-cloned) shared_ptrs to the
	 * shared_ptr overload with do_clone == false -- the archive co-owns the clones directly, no second
	 * copy. (do_clone is intentionally ignored: cloning at the boundary is exactly what do_clone asks for.)
	 */
void GOptimizableEntityFixedSizePriorityQueue::add(
    std::vector<std::unique_ptr<GOptimizableEntity>> const &items_cnt,
    const bool /* do_clone */,
    const bool do_replace
) {
    std::vector<std::shared_ptr<GOptimizableEntity>> bridge;
    bridge.reserve(items_cnt.size());
    for(auto const &item_ptr : items_cnt) {
        if(item_ptr) {
            bridge.push_back(item_ptr->clone<GOptimizableEntity>());
        }
    }
    this->add(bridge, false, do_replace);
}

/******************************************************************************/
/**
	 * Boundary overload: adds the individuals held by a SLOT population. The population owns its slots by
	 * unique_ptr, each slot owns its individual; this archive keeps its own shared_ptr clones, so we clone
	 * each slot's individual across the ownership boundary and delegate to the shared_ptr overload (which
	 * co-owns the clones directly). (do_clone is intentionally ignored: cloning at the boundary is exactly
	 * what do_clone asks for.)
	 */
void GOptimizableEntityFixedSizePriorityQueue::add(
    std::vector<std::unique_ptr<GIndividualSlot>> const &items_cnt,
    const bool /* do_clone */,
    const bool do_replace
) {
    std::vector<std::shared_ptr<GOptimizableEntity>> bridge;
    bridge.reserve(items_cnt.size());
    for(auto const &slot_ptr : items_cnt) {
        if(slot_ptr && slot_ptr->hasIndividual()) {
            bridge.push_back(slot_ptr->individual().clone<GOptimizableEntity>());
        }
    }
    this->add(bridge, false, do_replace);
}

/******************************************************************************/
/**
	 * Boundary overload: adds the individuals held by a SLOT population sub-range [begin, end). See the
	 * slot-vector overload above.
	 */
void GOptimizableEntityFixedSizePriorityQueue::add(
    std::vector<std::unique_ptr<GIndividualSlot>>::const_iterator begin,
    std::vector<std::unique_ptr<GIndividualSlot>>::const_iterator end,
    const bool /* do_clone */,
    const bool do_replace
) {
    std::vector<std::shared_ptr<GOptimizableEntity>> bridge;
    bridge.reserve(static_cast<std::size_t>(std::distance(begin, end)));
    for(auto it = begin; it != end; ++it) {
        if(*it && (*it)->hasIndividual()) {
            bridge.push_back((*it)->individual().clone<GOptimizableEntity>());
        }
    }
    this->add(bridge, false, do_replace);
}

/******************************************************************************/
/**
	 * Boundary overload: adds a unique_ptr population sub-range [begin, end). See the vector overload above.
	 */
void GOptimizableEntityFixedSizePriorityQueue::add(
    std::vector<std::unique_ptr<GOptimizableEntity>>::const_iterator begin,
    std::vector<std::unique_ptr<GOptimizableEntity>>::const_iterator end,
    const bool /* do_clone */,
    const bool do_replace
) {
    std::vector<std::shared_ptr<GOptimizableEntity>> bridge;
    bridge.reserve(static_cast<std::size_t>(std::distance(begin, end)));
    for(auto it = begin; it != end; ++it) {
        if(*it) {
            bridge.push_back((*it)->clone<GOptimizableEntity>());
        }
    }
    this->add(bridge, false, do_replace);
}

/******************************************************************************/
/**
	 * Boundary overload: adds a single unique_ptr-owned individual. See the vector overload above.
	 */
void GOptimizableEntityFixedSizePriorityQueue::add(
    std::unique_ptr<GOptimizableEntity> const &item_ptr,
    const bool /* do_clone */
) {
    if(item_ptr && item_ptr->is_processed()) {
        this->add(item_ptr->clone<GOptimizableEntity>(), false);
    }
}

/******************************************************************************/
/** @brief Applies modifications to this object. This is needed for testing purposes */
bool GOptimizableEntityFixedSizePriorityQueue::modify_GUnitTests_() {
#ifdef GEM_TESTING

    bool result = false;

    // Call the parent classes' functions
    if(Gem::Common::GFixedSizePriorityQueueT<GOptimizableEntity>::modify_GUnitTests_()) {
        result = true;
    }

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GOptimizableEntityFixedSizePriorityQueue<GOptimizableEntity>::modify_GUnitTests",
        "GEM_TESTING"
    );
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
void GOptimizableEntityFixedSizePriorityQueue::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent classes' functions
    Gem::Common::GFixedSizePriorityQueueT<
        GOptimizableEntity>::specificTestsNoFailureExpected_GUnitTests_();

    //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GOptimizableEntityFixedSizePriorityQueue<GOptimizableEntity>::specificTestsNoFailureExpected_"
        "GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
void GOptimizableEntityFixedSizePriorityQueue::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent classes' functions
    Gem::Common::GFixedSizePriorityQueueT<
        GOptimizableEntity>::specificTestsFailuresExpected_GUnitTests_();

    //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GOptimizableEntityFixedSizePriorityQueue<GOptimizableEntity>::specificTestsFailuresExpected_"
        "GUnitTests_",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva::Genome */
