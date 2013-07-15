/**
 * @file GOptimizationAlgorithmT.cpp
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


#include "geneva/GOptimizationAlgorithmT.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GParameterSet.hpp"

/******************************************************************************/

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GOptimizationAlgorithmT<Gem::Geneva::GOptimizableEntity>::GOptimizationMonitorT)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::GOptimizationMonitorT)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Courtier::GBrokerConnector2T<Gem::Geneva::GParameterSet>)

/******************************************************************************/

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * Allows to perform initialization work before the optimization cycle starts. This
 * function will usually be overloaded by derived functions, which should however,
 * as one of their first actions, call this function. It is not recommended  to perform
 * any "real" optimization work here, such as evaluation of individuals. Use the
 * optimizationInit() function instead.
 */
template <> void GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::init() {
   // Tell all individuals in this collection to update their random number generators
   // with the one contained in GMutableSetT. Note: This will only have an effect on
   // GParameterSet objects, as GOptimizableEntity contains an empty function.
   GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::iterator it;
   for(it=this->begin(); it!=this->end(); ++it) {
      (*it)->updateRNGs();
   }
}

/******************************************************************************/
/**
 * Allows to perform any remaining work after the optimization cycle has finished.
 * This function will usually be overloaded by derived functions, which should however
 * call this function as one of their last actions. It is not recommended  to perform
 * any "real" optimization work here, such as evaluation of individuals. Use the
 * optimizationFinalize() function instead.
 */
template <> void GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::finalize() {
   // Tell all individuals in this collection to tell all GParameterBase derivatives
   // to again use their local generators.
   GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::iterator it;
   for(it=this->begin(); it!=this->end(); ++it) {
      (*it)->restoreRNGs();
   }
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
