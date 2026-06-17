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
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

// Geneva header files go here
#include "common/GFixedSizePriorityQueueT.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/GenevaHelperFunctions.hpp"

namespace Gem::Geneva::Genome {

/******************************************************************************/
// Forward declaration: the OA population is a vector of slots; the slot-population add() overload below
// clones each slot's individual across the ownership boundary into this (shared_ptr) archive.
class GIndividualSlot;

/******************************************************************************/
/**
 * This class implements a fixed size priority queue for GOptimizableEntity objects,
 * based on the maximization/minimization property and the current fitness of
 * the objects.
 */
class GOptimizableEntityFixedSizePriorityQueue // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Common::GFixedSizePriorityQueueT<GOptimizableEntity> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GFSPQ",
            boost::serialization::base_object<Gem::Common::GFixedSizePriorityQueueT<GOptimizableEntity>>(
                *this
            )
        );
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GOptimizableEntityFixedSizePriorityQueue() = default;

    /**
     * @brief Initialization with the maximum size
     *
     * @param maxSize The maximum number of items the priority queue is allowed to hold
     */
    explicit GOptimizableEntityFixedSizePriorityQueue(const std::size_t & maxSize);
    /**
     * @brief The copy constructor
     *
     * @param cp A constant reference to another GOptimizableEntityFixedSizePriorityQueue object to be copied
     */
    GOptimizableEntityFixedSizePriorityQueue(const GOptimizableEntityFixedSizePriorityQueue &cp) = default;
    /** @brief The destructor */
    ~GOptimizableEntityFixedSizePriorityQueue() override = default;

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
    std::string getCleanStatus() const;

    /**
     * @brief Adds items in a range to the priority queue
     *
     * @param begin A const_iterator pointing to the start of the range of items to be added
     * @param end A const_iterator pointing one past the end of the range of items to be added
     * @param do_clone If true, the items are cloned before being added; otherwise the shared pointers are stored as-is
     * @param replace If true, the queue's existing content is replaced rather than merged with the new items
     */
    void
    add(std::vector<std::shared_ptr<GOptimizableEntity>>::const_iterator begin,
        std::vector<std::shared_ptr<GOptimizableEntity>>::const_iterator end,
        bool do_clone,
        bool replace) override;

    /**
     * @brief Adds the items in the items_cnt container to the queue
     *
     * @param items_cnt A constant reference to a vector of items to be added to the queue
     * @param do_clone If true, the items are cloned before being added; otherwise the shared pointers are stored as-is
     * @param replace If true, the queue's existing content is replaced rather than merged with the new items
     */
    void
    add(std::vector<std::shared_ptr<GOptimizableEntity>> const &items_cnt,
        const bool do_clone,
        const bool replace) override;

    /**
     * @brief Adds a single item to the queue
     *
     * @param item A constant reference to the item to be added to the queue
     * @param do_clone If true, the item is cloned before being added; otherwise the shared pointer is stored as-is
     */
    void add(std::shared_ptr<GOptimizableEntity> const &item, const bool do_clone) override;

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
    add(std::vector<std::unique_ptr<GOptimizableEntity>>::const_iterator begin,
        std::vector<std::unique_ptr<GOptimizableEntity>>::const_iterator end,
        bool do_clone,
        bool replace);
    /**
     * @brief Adds the individuals of a unique_ptr population to the queue (cloning across the boundary)
     *
     * @param items_cnt A constant reference to a vector of unique_ptr-owned individuals to be added
     * @param do_clone If true, each individual is cloned before being added
     * @param replace If true, the queue's existing content is replaced rather than merged with the new items
     */
    void add(std::vector<std::unique_ptr<GOptimizableEntity>> const &items_cnt, bool do_clone, bool replace);
    /**
     * @brief Adds a single unique_ptr-owned individual to the queue (cloning across the boundary)
     *
     * @param item A constant reference to the unique_ptr-owned individual to be added
     * @param do_clone If true, the individual is cloned before being added
     */
    void add(std::unique_ptr<GOptimizableEntity> const &item, bool do_clone);
    /**
     * @brief Adds the individuals held by a SLOT population to the queue (cloning each slot's individual across the boundary)
     *
     * @param items_cnt A constant reference to a vector of GIndividualSlot objects whose held individuals are to be added
     * @param do_clone If true, each slot's individual is cloned before being added
     * @param replace If true, the queue's existing content is replaced rather than merged with the new items
     */
    void add(std::vector<std::unique_ptr<GIndividualSlot>> const &items_cnt, bool do_clone, bool replace);
    /**
     * @brief Adds the individuals held by a SLOT population sub-range [begin, end) to the queue (cloning each slot's individual across the boundary)
     *
     * @param begin A const_iterator pointing to the start of the GIndividualSlot range whose individuals are to be added
     * @param end A const_iterator pointing one past the end of the GIndividualSlot range whose individuals are to be added
     * @param do_clone If true, each slot's individual is cloned before being added
     * @param replace If true, the queue's existing content is replaced rather than merged with the new items
     */
    void
    add(std::vector<std::unique_ptr<GIndividualSlot>>::const_iterator begin,
        std::vector<std::unique_ptr<GIndividualSlot>>::const_iterator end,
        bool do_clone,
        bool replace);

protected:
    /***************************************************************************/
    /**
     * @brief Loads the data of another population
     *
     * @param cp A pointer to another GFixedSizePriorityQueueT (expected to be a GOptimizableEntityFixedSizePriorityQueue) whose data is to be loaded
     */
    void load_(const Gem::Common::GFixedSizePriorityQueueT<GOptimizableEntity> * cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GOptimizableEntityFixedSizePriorityQueue>(
        GOptimizableEntityFixedSizePriorityQueue const &,
        GOptimizableEntityFixedSizePriorityQueue const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type
     *
     * @param cp A constant reference to another GFixedSizePriorityQueueT object (the object to be compared against)
     * @param e The expectation for this comparison, e.g. equality or inequality
     * @param limit The maximum allowed deviation of floating point types still considered equal
     */
    void compare_(
        const Gem::Common::GFixedSizePriorityQueueT<GOptimizableEntity> & cp // the other object
        ,
        const Gem::Common::expectation & e // the expectation for this object, e.g. equality
        ,
        const double & limit // the limit for allowed deviations of floating point types
    ) const override;

    /**
     * @brief Checks whether an Item is valid
     *
     * @param item A constant reference to the work item to be checked
     * @return true if the item is valid (e.g. has no dirty flag set), false otherwise
     */
    bool isValid(const std::shared_ptr<GOptimizableEntity> & item) const override;
    /**
     * @brief Evaluates a single work item, so that it can be sorted
     *
     * @param item A constant reference to the work item to be evaluated
     * @return The fitness value of the item used as the sorting criterion within the priority queue
     */
    double evaluation(const std::shared_ptr<GOptimizableEntity> & item) const override;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /**
     * @brief Emits a name for this class / object
     *
     * @return A string holding the name of this class
     */
    std::string name_() const override;
    /**
     * @brief Creates a deep clone of this object
     *
     * @return A deep clone of this object, returned as a pointer to the GFixedSizePriorityQueueT base class
     */
    Gem::Common::GFixedSizePriorityQueueT<GOptimizableEntity> *clone_() const override;
};

/******************************************************************************/

} /* namespace Gem::Geneva::Genome */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Genome::GOptimizableEntityFixedSizePriorityQueue) // NOLINT
