/**
 * @file GIndividualStandardConsumerInitializerT.hpp
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

// For Microsoft-compatible compilers
#include "common/GWindowsDefines.hpp"

// Standard header files go here
#include <iostream>

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/assert.hpp>


#ifndef GINDIVIDUALSTANDARDCONSUMERINITIALIZERT_HPP_
#define GINDIVIDUALSTANDARDCONSUMERINITIALIZERT_HPP_

// Geneva headers go here
#include "common/GLogger.hpp"
#include "common/GGlobalOptionsT.hpp"
#include "courtier/GBaseConsumerT.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GConsumerStore.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This base class takes care of adding GParameterSet-based consumer objects
 * to a global store
 */
template <typename pl_type, typename c_type> // pl_type stands for "payload type"; c_type stands for consumer type
class GIndividualStandardConsumerInitializerT {
   // Make sure pl_type is a derivative of Gem::Geneva::GParameterSet
   BOOST_MPL_ASSERT((boost::is_base_of<Gem::Geneva::GParameterSet , pl_type>));

public:
   /** @brief The initializing constructor */
   inline GIndividualStandardConsumerInitializerT() {
      // Create a smart pointer holding the consumer
      boost::shared_ptr<Gem::Courtier::GBaseConsumerT<pl_type> > p(new c_type());
      std::string mnemonic = p->getMnemonic();

      // We require that the consumer has a default constructor and the static "nickname" data member
      if(!GConsumerStore->setOnce(mnemonic, p)) { // The consumer already exists in the store
         glogger
         << "In GIndividualStandardConsumerInitializerT<pl_type, c_type>::GIndividualStandardConsumerInitializerT(): Error!" << std::endl
         << "Consumer with identifier " << mnemonic << " already exists in store." << std::endl
         << GTERMINATION;
      }
   }
   /** @brief An empty destructor */
   virtual inline ~GIndividualStandardConsumerInitializerT() { /* nothing */ }
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GINDIVIDUALSTANDARDCONSUMERINITIALIZERT_HPP_ */
