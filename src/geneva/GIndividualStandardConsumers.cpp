/**
 * @file GIndividualStandardConsumers.hpp
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

#include "geneva/GIndividualStandardConsumers.hpp"

namespace Gem {
namespace Geneva {


/******************************************************************************/
/**
 * This will register the consumers in the global consumer store
 */
GIndividualStandardConsumerInitializerT<Gem::Geneva::GParameterSet, GIndividualTCPConsumer> GTCPCStoreRegistrant;
GIndividualStandardConsumerInitializerT<Gem::Geneva::GParameterSet, GIndividualThreadConsumer> GBTCStoreRegistrant;
GIndividualStandardConsumerInitializerT<Gem::Geneva::GParameterSet, GIndividualSerialConsumer> GSCStoreRegistrant;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GIndividualTCPConsumer::GIndividualTCPConsumer()
   : Gem::Courtier::GAsioTCPConsumerT<Gem::Geneva::GParameterSet>()
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor that takes a number of vital arguments
 */
GIndividualTCPConsumer::GIndividualTCPConsumer (
      const unsigned short& port
      , const std::size_t& listenerThreads
      , const Gem::Common::serializationMode& sm
)
   : Gem::Courtier::GAsioTCPConsumerT<Gem::Geneva::GParameterSet>(port, listenerThreads, sm)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GIndividualTCPConsumer::~GIndividualTCPConsumer()
{ /* nothing */ }

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/*
 * The default constructor
 */
GIndividualThreadConsumer::GIndividualThreadConsumer()
   : Gem::Courtier::GBoostThreadConsumerT<Gem::Geneva::GParameterSet>()
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GIndividualThreadConsumer::~GIndividualThreadConsumer()
{ /* nothing */ }

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/*
 * The default constructor
 */
GIndividualSerialConsumer::GIndividualSerialConsumer()
   : Gem::Courtier::GSerialConsumerT<Gem::Geneva::GParameterSet>()
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GIndividualSerialConsumer::~GIndividualSerialConsumer()
{ /* nothing */ }

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

