/**
 * @file GIndividualStandardConsumerInitializer.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <iostream>

// Boost header files go here
#include <boost/shared_ptr.hpp>

#ifndef GINDIVIDUALSTANDARDCONSUMERS_HPP_
#define GINDIVIDUALSTANDARDCONSUMERS_HPP_

// Geneva headers go here
#include "common/GLogger.hpp"
#include "courtier/GBaseConsumerT.hpp"
#include "courtier/GAsioTCPConsumerT.hpp"
#include "courtier/GBoostThreadConsumerT.hpp"
#include "courtier/GSerialConsumerT.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GIndividualStandardConsumerInitializerT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A consumer used for network communication, using GParameterSet-derivatives
 */
class G_API GIndividualTCPConsumer
   : public Gem::Courtier::GAsioTCPConsumerT<Gem::Geneva::GParameterSet>
{
public:
   /** @brief The default constructor */
   GIndividualTCPConsumer();
   /** @brief A constructor that takes a number of vital arguments */
   GIndividualTCPConsumer(
         const unsigned short&
         , const std::size_t& = 0
         , const Gem::Common::serializationMode& = Gem::Common::SERIALIZATIONMODE_BINARY
   );
   /** @brief The destructor */
   virtual ~GIndividualTCPConsumer();
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A consumer used for multi-threaded processing, using GParameterSet-derivatives
 */
class G_API GIndividualThreadConsumer
   : public Gem::Courtier::GBoostThreadConsumerT<Gem::Geneva::GParameterSet>
{
public:
   /** @brief The default constructor */
   GIndividualThreadConsumer();
   /** @brief The desstructor */
   virtual ~GIndividualThreadConsumer();
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A consumer used for serial execution (mostly needed for debugging purposes).
 * Its payload are GParameterSet-derivatives.
 */
class G_API GIndividualSerialConsumer
   : public Gem::Courtier::GSerialConsumerT<Gem::Geneva::GParameterSet>
{
public:
   /** @brief The default constructor */
   GIndividualSerialConsumer();
   /** @brief The desstructor */
   virtual ~GIndividualSerialConsumer();
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GINDIVIDUALSTANDARDCONSUMERS_HPP_ */
