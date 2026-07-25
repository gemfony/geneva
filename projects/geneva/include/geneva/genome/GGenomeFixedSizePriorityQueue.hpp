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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here

// Boost header files go here

// Geneva header files go here
#include "common/GReflectiveInterfaceT.hpp"
#include "common/GFixedSizePriorityQueueT.hpp"
#include "geneva/genome/GGenome.hpp"
#include "geneva/GenevaHelperFunctions.hpp"

namespace Gem::Geneva::Genome {

/******************************************************************************/
/**
 * This class implements a fixed size priority queue for GGenome objects,
 * based on the maximization/minimization property and the current fitness of
 * the objects.
 */
class GGenomeFixedSizePriorityQueue // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Common::GReflectiveInterfaceT<GGenomeFixedSizePriorityQueue, Gem::Common::GFixedSizePriorityQueueT<GGenome>> {
    ///////////////////////////////////////////////////////////////////////
    // Gem::Weft::access default-constructs this concrete type on load;
    // GReflectiveInterfaceAccess lets the mixin reach the (empty) localMembers_().
    friend struct Gem::Common::GReflectiveInterfaceAccess;

    /** @brief This class adds no own members; the explicit empty declaration is required
     *  (the mixin's deleted fallback rejects a missing one). */
    template <typename Self>
    auto localMembers_(this Self &) {
        return std::make_tuple();
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The class name, consumed by the GReflectiveInterfaceT-generated name_() / compare token. */
    static constexpr std::string_view class_name = "GGenomeFixedSizePriorityQueue";

    /** @brief The default constructor */
    GGenomeFixedSizePriorityQueue() = default;

    /**
     * @brief Initialization with the maximum size
     *
     * @param maxSize The maximum number of items the priority queue is allowed to hold
     */
    explicit GGenomeFixedSizePriorityQueue(const std::size_t & max_size);
    /**
     * @brief The copy constructor
     *
     * @param cp A constant reference to another GGenomeFixedSizePriorityQueue object to be copied
     */
    GGenomeFixedSizePriorityQueue(const GGenomeFixedSizePriorityQueue &cp) = default;
    /** @brief The destructor */
    ~GGenomeFixedSizePriorityQueue() override = default;

    /**
     * @brief Checks whether no item has the dirty flag set
     *
     * @param pos If a dirty item is found, this is set to the position of the first such item
     * @return true if no item has its dirty flag set, false otherwise
     */
    bool allClean(std::size_t & pos) const;
    /**
     * @brief Emits information about the "dirty flag" of all items
     *
     * @return A string describing the dirty-flag status of all items held by the queue
     */
    [[nodiscard]] std::string getCleanStatus() const;

    /**
     * @brief Adds items in a range to the priority queue
     *
     * @param begin A const_iterator pointing to the start of the range of items to be added
     * @param end A const_iterator pointing one past the end of the range of items to be added
     * @param do_clone If true, the items are cloned before being added; otherwise the shared pointers are stored as-is
     * @param replace If true, the queue's existing content is replaced rather than merged with the new items
     */
    void
    add(std::vector<std::shared_ptr<GGenome>>::const_iterator begin,
        std::vector<std::shared_ptr<GGenome>>::const_iterator end,
        bool do_clone,
        bool do_replace) override;

    /**
     * @brief Adds the items in the items_cnt container to the queue
     *
     * @param items_cnt A constant reference to a vector of items to be added to the queue
     * @param do_clone If true, the items are cloned before being added; otherwise the shared pointers are stored as-is
     * @param replace If true, the queue's existing content is replaced rather than merged with the new items
     */
    void
    add(std::vector<std::shared_ptr<GGenome>> const &items_cnt,
        const bool do_clone,
        const bool do_replace) override;

    /**
     * @brief Adds a single item to the queue
     *
     * @param item A constant reference to the item to be added to the queue
     * @param do_clone If true, the item is cloned before being added; otherwise the shared pointer is stored as-is
     */
    void add(std::shared_ptr<GGenome> const &item_ptr, const bool do_clone) override;

    /***************************************************************************/
    // Boundary overloads for the unique_ptr population. The OA population now owns its individuals
    // by unique_ptr; this archive keeps its own (shared_ptr) clones, so these adapters clone each
    // individual across the ownership boundary and delegate to the shared_ptr implementations above.
    /**
     * @brief Adds a unique_ptr population sub-range to the queue (cloning across the boundary)
     *
     * @param begin A const_iterator pointing to the start of the unique_ptr-owned range to be added
     * @param end A const_iterator pointing one past the end of the unique_ptr-owned range to be added
     * @param do_clone If true, each individual is cloned before being added (always effectively cloned here, since ownership cannot be transferred)
     * @param replace If true, the queue's existing content is replaced rather than merged with the new items
     */
    void
    add(std::vector<std::unique_ptr<GGenome>>::const_iterator begin,
        std::vector<std::unique_ptr<GGenome>>::const_iterator end,
        bool do_clone,
        bool do_replace);
    /**
     * @brief Adds the individuals of a unique_ptr population to the queue (cloning across the boundary)
     *
     * @param items_cnt A constant reference to a vector of unique_ptr-owned individuals to be added
     * @param do_clone If true, each individual is cloned before being added
     * @param replace If true, the queue's existing content is replaced rather than merged with the new items
     */
    void add(std::vector<std::unique_ptr<GGenome>> const &items_cnt, bool do_clone, bool do_replace);
    /**
     * @brief Adds a single unique_ptr-owned individual to the queue (cloning across the boundary)
     *
     * @param item A constant reference to the unique_ptr-owned individual to be added
     * @param do_clone If true, the individual is cloned before being added
     */
    void add(std::unique_ptr<GGenome> const &item_ptr, bool do_clone);

protected:
    // load_(), compare_(), name_() and clone_() are generated by the
    // Gem::Common::GReflectiveInterfaceT base from class_name and the empty localMembers_().

    /***************************************************************************/
    /**
     * @brief Checks whether an Item is valid
     *
     * @param item A constant reference to the work item to be checked
     * @return true if the item is valid (e.g. has no dirty flag set), false otherwise
     */
    [[nodiscard]] bool isValid(const std::shared_ptr<GGenome> & item_ptr) const override;
    /**
     * @brief Evaluates a single work item, so that it can be sorted
     *
     * @param item A constant reference to the work item to be evaluated
     * @return The fitness value of the item used as the sorting criterion within the priority queue
     */
    [[nodiscard]] double evaluation(const std::shared_ptr<GGenome> & item_ptr) const override;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;
};

/******************************************************************************/

} /* namespace Gem::Geneva::Genome */

