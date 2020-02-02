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
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
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
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>

// Geneva header files go here
#include "common/GFixedSizePriorityQueueT.hpp"
#include "geneva/GenevaHelperFunctions.hpp"
#include "geneva/GParameterSet.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class implements a fixed size priority queue for GParameterSet objects,
 * based on the maximization/minimization property and the current fitness of
 * the objects.
 */
class GParameterSetFixedSizePriorityQueue
    : public Gem::Common::GFixedSizePriorityQueueT<GParameterSet>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & make_nvp(
            "GFSPQ"
            , boost::serialization::base_object<Gem::Common::GFixedSizePriorityQueueT<GParameterSet>>(*this));
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    G_API_GENEVA GParameterSetFixedSizePriorityQueue() = default;

    /** @brief Initialization with the maximum size */
    explicit G_API_GENEVA GParameterSetFixedSizePriorityQueue(const std::size_t &);
    /** @brief The copy constructor */
    G_API_GENEVA GParameterSetFixedSizePriorityQueue(const GParameterSetFixedSizePriorityQueue &cp) = default;
    /** @brief The destructor */
    G_API_GENEVA ~GParameterSetFixedSizePriorityQueue() override = default;

    /** @brief Checks whether no item has the dirty flag set */
    G_API_GENEVA bool allClean(std::size_t &) const;
    /** @brief Emits information about the "dirty flag" of all items */
    G_API_GENEVA std::string getCleanStatus() const;

    /** @brief Adds the items in the items_cnt container to the queue */
    void add(
        std::vector<std::shared_ptr<GParameterSet>> const &items_cnt
        , bool do_clone
        , bool replace
    ) override;

    /** @brief Adds a single item to the queue */
    void add(
        std::shared_ptr<GParameterSet> const& item
        , bool do_clone
    ) override;

protected:
    /***************************************************************************/
    /** @brief Loads the data of another population */
    G_API_GENEVA void load_(const Gem::Common::GFixedSizePriorityQueueT<GParameterSet> *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GParameterSetFixedSizePriorityQueue>(
        GParameterSetFixedSizePriorityQueue const &
        , GParameterSetFixedSizePriorityQueue const &
        , Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    G_API_GENEVA void compare_(
        const Gem::Common::GFixedSizePriorityQueueT<GParameterSet> & // the other object
        , const Gem::Common::expectation & // the expectation for this object, e.g. equality
        , const double & // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Evaluates a single work item, so that it can be sorted */
    G_API_GENEVA double evaluation(const std::shared_ptr<GParameterSet> &) const override;
    /** @brief Returns a unique id for a work item */
    G_API_GENEVA std::string id(const std::shared_ptr<GParameterSet> &) const override;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    G_API_GENEVA bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    G_API_GENEVA void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /** @brief Emits a name for this class / object */
    G_API_GENEVA std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    G_API_GENEVA Gem::Common::GFixedSizePriorityQueueT<GParameterSet> *clone_() const override;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GParameterSetFixedSizePriorityQueue)

