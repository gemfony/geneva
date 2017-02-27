/**
 * @file GIndividualStandardConsumers.hpp
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

#include "geneva/GIndividualStandardConsumers.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GIndividualSerialTCPConsumer::GIndividualSerialTCPConsumer()
	: Gem::Courtier::GAsioSerialTCPConsumerT<Gem::Geneva::GParameterSet>()
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor that takes a number of vital arguments
 */
GIndividualSerialTCPConsumer::GIndividualSerialTCPConsumer(
	const unsigned short &port
	, const std::size_t &listenerThreads
	, const Gem::Common::serializationMode &sm
)
	: Gem::Courtier::GAsioSerialTCPConsumerT<Gem::Geneva::GParameterSet>(port, listenerThreads, sm)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GIndividualSerialTCPConsumer::~GIndividualSerialTCPConsumer()
{ /* nothing */ }


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GIndividualAsyncTCPConsumer::GIndividualAsyncTCPConsumer()
	: Gem::Courtier::GAsioAsyncTCPConsumerT<Gem::Geneva::GParameterSet>()
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor that takes a number of vital arguments
 */
GIndividualAsyncTCPConsumer::GIndividualAsyncTCPConsumer(
	const unsigned short &port
	, const std::size_t &listenerThreads
	, const Gem::Common::serializationMode &sm
)
	: Gem::Courtier::GAsioAsyncTCPConsumerT<Gem::Geneva::GParameterSet>(port, listenerThreads, sm)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GIndividualAsyncTCPConsumer::~GIndividualAsyncTCPConsumer()
{ /* nothing */ }

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/*
 * The default constructor
 */
GIndividualThreadConsumer::GIndividualThreadConsumer()
	: Gem::Courtier::GStdThreadConsumerT<Gem::Geneva::GParameterSet>()
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

