/**
 * @file GParameterSetFixedSizeSetPriorityQueue.hpp
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

// Standard header files go here

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

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

#ifndef GPARAMETERFIXEDSIZESETPRIORITYQUEUE_HPP_
#define GPARAMETERFIXEDSIZESETPRIORITYQUEUE_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include "common/GFixedSizePriorityQueueT.hpp"
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
     & make_nvp("GFSPQ", boost::serialization::base_object<Gem::Common::GFixedSizePriorityQueueT<GParameterSet> >(*this));
   }
   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief The default constructor */
   GParameterSetFixedSizePriorityQueue();
   /** @brief Initialization with the maximum size */
   explicit GParameterSetFixedSizePriorityQueue(const std::size_t&);
   /** @brief The copy constructor */
   GParameterSetFixedSizePriorityQueue(const GParameterSetFixedSizePriorityQueue&);
   /** @brief The destructor */
   ~GParameterSetFixedSizePriorityQueue();

   /** @brief Copy the data of another GParameterSetFixedSizePriorityQueue over */
   const GParameterSetFixedSizePriorityQueue& operator=(const GParameterSetFixedSizePriorityQueue&);

   /** @brief Loads the data of another GParameterSetFixedSizePriorityQueue object, camouflaged as a GFixedSizePriorityQueueT<T> */
   virtual void load(const Gem::Common::GFixedSizePriorityQueueT<GParameterSet>&);
   /** @brief Creates a deep clone of this object */
   virtual boost::shared_ptr<Gem::Common::GFixedSizePriorityQueueT<GParameterSet> > clone() const;

   /** @brief Checks whether no item has the dirty flag set */
   bool allClean(std::size_t&) const;
   /** @brief Emits information about the "dirty flag" of all items */
   std::string getCleanStatus() const;

protected:
   /** @brief Evaluates a single work item, so that it can be sorted */
   virtual double evaluation(const boost::shared_ptr<GParameterSet>&) const;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GParameterSetFixedSizePriorityQueue)

#endif /* GPARAMETERFIXEDSIZESETPRIORITYQUEUE_HPP_ */
