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

#include "geneva/GParameterSetFixedSizePriorityQueue.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GParameterSetFixedSizePriorityQueue) // NOLINT

namespace Gem::Geneva {

/******************************************************************************/
/**
	 * Initialization with the maximum size. The GParameterSetFixedSizePriorityQueue is
	 * targetted at optimization algorithms, which only understand "minimization". Hence
	 * "lower is better" is the only allowed mode of operation of this priority queue.
	 */
GParameterSetFixedSizePriorityQueue::GParameterSetFixedSizePriorityQueue(
    const std::size_t &max_size
)
  : Gem::Common::GFixedSizePriorityQueueT<GParameterSet>(
        max_size,
        Gem::Common::sortOrder::LOWERISBETTER
    ) { /* nothing */
}

/******************************************************************************/
/**
	 * Emits a name for this class / object
	 */
std::string GParameterSetFixedSizePriorityQueue::name_() const {
    return std::string("GParameterSetFixedSizePriorityQueue");
}

/******************************************************************************/
/**
	 * Searches for compliance with expectations with respect to another object
	 * of the same type
	 *
	 * @param cp A constant reference to another GParameterSetFixedSizePriorityQueue object
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 */
void GParameterSetFixedSizePriorityQueue::compare_(
    const Gem::Common::GFixedSizePriorityQueueT<GParameterSet> &cp // the other object
    ,
    const Gem::Common::expectation &e // the expectation for this object, e.g. equality
    ,
    const double & /*limit*/ // the limit for allowed deviations of floating point types
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GParameterSetFixedSizePriorityQueue reference independent of this object and convert the pointer
    const GParameterSetFixedSizePriorityQueue *p_load =
        Gem::Common::g_convert_and_compare(cp, this);

    GToken token("GBasGParameterSetFixedSizePriorityQueueeEA", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<Gem::Common::GFixedSizePriorityQueueT<GParameterSet>>(
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
	 * Loads the data of another GParameterSetFixedSizePriorityQueue object, camouflaged as a GFixedSizePriorityQueueT<GParameterSet>
	 */
void GParameterSetFixedSizePriorityQueue::load_(
    const Gem::Common::GFixedSizePriorityQueueT<GParameterSet> *cp
) { // NOLINT(misc-unused-parameters)
    // Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
    // *** currently not needed ***
    // const GParameterSetFixedSizePriorityQueue *p_load = Gem::Common::g_convert_and_compare(cp, this);

    // Load our parent class'es data ...
    Gem::Common::GFixedSizePriorityQueueT<GParameterSet>::load_(cp);

    // ... no local data
}

/******************************************************************************/
/**
	 * Creates a deep clone of this object
	 */
Gem::Common::GFixedSizePriorityQueueT<GParameterSet> *
GParameterSetFixedSizePriorityQueue::clone_() const {
    return new GParameterSetFixedSizePriorityQueue(*this);
}

/******************************************************************************/
/**
	 * Checks whether no item has the dirty flag set
	 */
bool GParameterSetFixedSizePriorityQueue::allClean(std::size_t &pos) const {
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
std::string GParameterSetFixedSizePriorityQueue::getCleanStatus() const {
    std::size_t pos = 0;
    std::ostringstream oss; // NOLINT(cppcoreguidelines-init-variables)
    for(const auto &item_ptr : data_deq_) {
        oss << "(" << pos++ << ", " << (not item_ptr->is_processed() ? "d" : "c") << ") ";
    }

    return oss.str();
}

/******************************************************************************/
/**
	 * Checks whether an Item is valid, i.e. holds a GParameterSet item and has
	 * already been evaluated.
	 */
bool GParameterSetFixedSizePriorityQueue::isValid(
    const std::shared_ptr<GParameterSet> &item_ptr
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
double GParameterSetFixedSizePriorityQueue::evaluation(
    const std::shared_ptr<GParameterSet> &item_ptr
) const {
    return minOnly_transformed_fitness(item_ptr);
}

/******************************************************************************/
/**
	 * Adds items in a range to the priority queue
	 */
void GParameterSetFixedSizePriorityQueue::GParameterSetFixedSizePriorityQueue::add(
    std::vector<std::shared_ptr<GParameterSet>>::const_iterator begin,
    std::vector<std::shared_ptr<GParameterSet>>::const_iterator end,
    bool do_clone,
    bool do_replace
) {
    // Create a std::vector containing only processed items. We only want
    // to add "clean" (i.e. processed) individuals to the queue.
    std::vector<std::shared_ptr<GParameterSet>> processed_cnt(std::distance(begin, end));
    auto it = std::copy_if(
        begin,
        end,
        processed_cnt.begin(),
        [](const std::shared_ptr<GParameterSet> &item_ptr) { return item_ptr->is_processed(); }
    );
    processed_cnt.resize(std::distance(processed_cnt.begin(), it));

    // Some error checking -- it should not happen that no processed items are found
    if(processed_cnt.empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterSetFixedSizePriorityQueue::add(range): Error!" << '\n'
            << "Container is empty when it should not be!" << '\n'
        );
    }

    Gem::Common::GFixedSizePriorityQueueT<GParameterSet>::add(
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
void GParameterSetFixedSizePriorityQueue::add(
    std::vector<std::shared_ptr<GParameterSet>> const &items_cnt,
    const bool do_clone,
    const bool do_replace
) {
    // Create a std::vector containing only processed items. We only want
    // to add "clean" (i.e. processed) individuals to the queue.
    std::vector<std::shared_ptr<GParameterSet>> processed_cnt(items_cnt.size());
    auto it = std::copy_if(
        items_cnt.begin(),
        items_cnt.end(),
        processed_cnt.begin(),
        [](const std::shared_ptr<GParameterSet> &item_ptr) { return item_ptr->is_processed(); }
    );
    processed_cnt.resize(std::distance(processed_cnt.begin(), it));

    // Some error checking -- it should not happen that no processed items are found
    if(processed_cnt.empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GParameterSetFixedSizePriorityQueue::add(vec): Error!" << '\n'
            << "Container is empty when it should not be!" << '\n'
        );
    }

    Gem::Common::GFixedSizePriorityQueueT<GParameterSet>::add(processed_cnt, do_clone, do_replace);
}

/******************************************************************************/
/**
	 * Adds a single item to the queue. his overload makes sure
	 * that only processed items (i.e. without errors and with the PROCESSED flag) are
	 * entered into the priority queue.
	 */
void GParameterSetFixedSizePriorityQueue::add(
    std::shared_ptr<GParameterSet> const &item_ptr,
    const bool do_clone
) {
    if(item_ptr && item_ptr->is_processed()) {
        Gem::Common::GFixedSizePriorityQueueT<GParameterSet>::add(item_ptr, do_clone);
    }
}

/******************************************************************************/
/** @brief Applies modifications to this object. This is needed for testing purposes */
bool GParameterSetFixedSizePriorityQueue::modify_GUnitTests_() {
#ifdef GEM_TESTING

    bool result = false;

    // Call the parent classes' functions
    if(Gem::Common::GFixedSizePriorityQueueT<GParameterSet>::modify_GUnitTests_()) {
        result = true;
    }

    return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GParameterSetFixedSizePriorityQueueT<GParameterSet>::modify_GUnitTests",
        "GEM_TESTING"
    );
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
void GParameterSetFixedSizePriorityQueue::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent classes' functions
    Gem::Common::GFixedSizePriorityQueueT<
        GParameterSet>::specificTestsNoFailureExpected_GUnitTests_();

    //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GParameterSetFixedSizePriorityQueueT<GParameterSet>::specificTestsNoFailureExpected_"
        "GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
void GParameterSetFixedSizePriorityQueue::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent classes' functions
    Gem::Common::GFixedSizePriorityQueueT<
        GParameterSet>::specificTestsFailuresExpected_GUnitTests_();

    //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GParameterSetFixedSizePriorityQueueT<GParameterSet>::specificTestsFailuresExpected_"
        "GUnitTests_",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Geneva */
