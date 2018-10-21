/**
 * @file
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

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
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GFSPQ", boost::serialization::base_object<Gem::Common::GFixedSizePriorityQueueT<GParameterSet>>(*this));
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief The default constructor */
	 G_API_GENEVA GParameterSetFixedSizePriorityQueue() = default;
	 /** @brief Initialization with the maximum size */
	 explicit G_API_GENEVA GParameterSetFixedSizePriorityQueue(const std::size_t&);
	 /** @brief The copy constructor */
	 G_API_GENEVA GParameterSetFixedSizePriorityQueue(const GParameterSetFixedSizePriorityQueue& cp) = default;
	 /** @brief The destructor */
	 G_API_GENEVA ~GParameterSetFixedSizePriorityQueue() = default;

	 /** @brief Checks whether no item has the dirty flag set */
	 G_API_GENEVA bool allClean(std::size_t&) const;
	 /** @brief Emits information about the "dirty flag" of all items */
	 G_API_GENEVA std::string getCleanStatus() const;

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 G_API_GENEVA void compare(
		 const Gem::Common::GFixedSizePriorityQueueT<GParameterSet>& // the other object
		 , const Gem::Common::expectation& // the expectation for this object, e.g. equality
		 , const double& // the limit for allowed deviations of floating point types
	 ) const override;

	 /** @brief Adds the items in the items_vec vector to the queue */
	 void add(
		 const std::vector<std::shared_ptr<GParameterSet>>& items_vec
		 , bool do_clone
		 , bool replace
	 ) override;

	 /** @brief Adds a single item to the queue */
	 void add(
		 std::shared_ptr<GParameterSet> item
		 , bool do_clone
	 ) override;

protected:
	 /***************************************************************************/
	 /** @brief Loads the data of another population */
	 G_API_GENEVA void load_(const Gem::Common::GFixedSizePriorityQueueT<GParameterSet> *) override;

	 /** @brief Evaluates a single work item, so that it can be sorted */
	 G_API_GENEVA double evaluation(const std::shared_ptr<GParameterSet>&) const override;
	 /** @brief Returns a unique id for a work item */
	 G_API_GENEVA std::string id(const std::shared_ptr<GParameterSet>&) const override;

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

