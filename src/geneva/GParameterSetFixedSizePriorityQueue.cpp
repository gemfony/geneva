/**
 * @file GParameterSetFixedSizeSetPriorityQueue.cpp
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

#include "geneva/GParameterSetFixedSizePriorityQueue.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GParameterSetFixedSizePriorityQueue)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GParameterSetFixedSizePriorityQueue::GParameterSetFixedSizePriorityQueue()
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the maximum size
 */
GParameterSetFixedSizePriorityQueue::GParameterSetFixedSizePriorityQueue(const std::size_t& maxSize)
   : Gem::Common::GFixedSizePriorityQueueT<GParameterSet>(maxSize)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
GParameterSetFixedSizePriorityQueue::GParameterSetFixedSizePriorityQueue(
   const GParameterSetFixedSizePriorityQueue& cp
)
   : Gem::Common::GFixedSizePriorityQueueT<GParameterSet>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GParameterSetFixedSizePriorityQueue::~GParameterSetFixedSizePriorityQueue()
{ /* nothing */ }

/******************************************************************************/
/**
 * Copy the data of another GParameterSetFixedSizePriorityQueue over
 */
const GParameterSetFixedSizePriorityQueue& GParameterSetFixedSizePriorityQueue::operator=(
   const GParameterSetFixedSizePriorityQueue& cp
) {
   this->load(cp);
   return *this;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
boost::shared_ptr<Gem::Common::GFixedSizePriorityQueueT<GParameterSet> > GParameterSetFixedSizePriorityQueue::clone() {
   return boost::shared_ptr<GParameterSetFixedSizePriorityQueue>(new GParameterSetFixedSizePriorityQueue(*this));
}

/******************************************************************************/
/**
 * Load the data of another GParameterSetFixedSizePriorityQueue item
 */
void GParameterSetFixedSizePriorityQueue::load(
   const Gem::Common::GFixedSizePriorityQueueT<GParameterSet>& cp
){
   // If you need to load local data, convert cp accordingly
   GFixedSizePriorityQueueT<GParameterSet>::load(cp);
}

/******************************************************************************/
/**
 * Compares two work items
 */
bool GParameterSetFixedSizePriorityQueue::comparator(
   boost::shared_ptr<GParameterSet> x
   , boost::shared_ptr<GParameterSet> y
) {
   bool dirtyFlag_x, dirtyFlag_y;
   double fitness_x, fitness_y;

   fitness_x = x->getCachedFitness(dirtyFlag_x);
   fitness_y = y->getCachedFitness(dirtyFlag_y);

#ifdef DEBUG
   if(dirtyFlag_x || dirtyFlag_y) {
      glogger
      << "In GParameterSetFixedSizePriorityQueue::comparator(): Error!" << std::endl
      << "Found dirty individual -- x: " << dirtyFlag_x << " y: " << dirtyFlag_y << std::endl
      << GEXCEPTION;
   }
#endif

   if(true == x->getMaxMode()) { // maximization
      if(fitness_x > fitness_y) return true;
      else return false;
   } else { // minimization
      if(fitness_x < fitness_y) return true;
      else return false;
   }
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
