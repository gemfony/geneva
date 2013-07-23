/**
 * @file GBaseParChildT.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

#include "geneva/GBaseParChildT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * Retrieves the  best individual found. Note that this protected function will return the item itself.
 * Direct usage of this function should be avoided even by derived classes. We suggest to use the
 * function GOptimizableI::getBestIndividual<GParameterSet>() instead, which internally uses
 * this function and returns copies of the best individual, converted to the desired target type.
 *
 * @return The best individual found
 */
template <>
boost::shared_ptr<GParameterSet>
GBaseParChildT<GParameterSet>::customGetBestIndividual() {
#ifdef DEBUG
   if(this->empty()) {
      glogger
      << "In GBaseParChildT<GParameterSet>::customGetBestIndividual() :" << std::endl
      << "Tried to access individual at position 0 even though population is empty." << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

   return this->at(0);
}

/******************************************************************************/
/**
 * Retrieves a list of the best individuals found. This might just be one individual.
 *
 * @return A list of the best individuals found
 */
template <>
std::vector<boost::shared_ptr<GParameterSet> >
GBaseParChildT<GParameterSet>::customGetBestIndividuals() {
   // Some error checking
   if(nParents_ == 0) {
      glogger
      << "In GBaseParChildT<GParameterSet>::customGetBestIndividuals() :" << std::endl
      << "no parents found" << std::endl
      << GEXCEPTION;
   }

   std::vector<boost::shared_ptr<GParameterSet> > bestIndividuals;
   GBaseParChildT<GParameterSet>::iterator it;
   for(it=this->begin(); it!=this->begin()+nParents_; ++it) {
      bestIndividuals.push_back(*it);
   }

   return bestIndividuals;
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
