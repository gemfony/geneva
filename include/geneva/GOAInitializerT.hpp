/**
 * @file GOAInitializerT.hpp
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

// Standard header files go here
#include <iostream>

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/assert.hpp>

#ifndef GOAINITIALIZERT_HPP_
#define GOAINITIALIZERT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GLogger.hpp"
#include "common/GGlobalOptionsT.hpp"
#include "geneva/GOAFactoryStore.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"
#include "geneva/GOptimizationAlgorithmFactoryT.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GOAFactoryStore.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This base class takes care of adding optimization algorithm factories to
 * the global algorithm store
 */
template <typename oaf_type>
class GOAInitializerT {
   // Make sure oaf_type has the expected type
   BOOST_MPL_ASSERT((boost::is_base_of<GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet> > , oaf_type>));

public:
   /** @brief The initializing constructor */
   inline GOAInitializerT() {
      // Create a smart pointer holding the algorithm
      boost::shared_ptr<GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet> > > p(new oaf_type());
      std::string mnemomic = p->getMnemomic();

      // We require that the algorithm has a default
      // constructor and the static "nickname" data member
      if(!GOAFactoryStore->setOnce(mnemomic, p)) { // Algorithm factory already exists in the store
         glogger
         << "In GOAInitializerT<op_type>::GOAInitializerT(): Error!" << std::endl
         << "Identifier " << mnemomic << " already exists in store." << std::endl
         << GTERMINATION;
      } else {
         std::cout << "Registered factory for algorithm \"" << mnemomic << "\" in the store." << std::endl;
      }
   }
   /** @brief An empty destructor */
   virtual inline ~GOAInitializerT() { /* nothing */ }
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GOAINITIALIZERT_HPP_ */
