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

#ifndef GINDIVIDUALSTANDARDCONSUMERS_HPP_
#define GINDIVIDUALSTANDARDCONSUMERS_HPP_

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <iostream>

// Boost header files go here

// Geneva headers go here
#include "common/GLogger.hpp"
#include "courtier/GBaseConsumerT.hpp"
#include "courtier/GWebsocketConsumerT.hpp"
#include "courtier/GAsioSerialTCPConsumerT.hpp"
#include "courtier/GAsioAsyncTCPConsumerT.hpp"
#include "courtier/GStdThreadConsumerT.hpp"
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
 * and serial communication on the client side.
 */
class GIndividualWebsocketConsumer final
	: public Gem::Courtier::GWebsocketConsumerT<Gem::Geneva::GParameterSet>
{
public:
	 /** @brief The default constructor */
	 G_API_GENEVA GIndividualWebsocketConsumer() = default;
	 /** @brief The destructor */
	 G_API_GENEVA ~GIndividualWebsocketConsumer() override = default;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A consumer used for network communication, using GParameterSet-derivatives
 * and serial communication on the client side.
 */
class GIndividualSerialTCPConsumer final
	: public Gem::Courtier::GAsioSerialTCPConsumerT<Gem::Geneva::GParameterSet>
{
public:
	/** @brief The default constructor */
	G_API_GENEVA GIndividualSerialTCPConsumer() = default;
	/** @brief A constructor that takes a number of vital arguments */
	G_API_GENEVA GIndividualSerialTCPConsumer(
		const unsigned short&
		, const std::size_t& = 0
		, const Gem::Common::serializationMode& = Gem::Common::serializationMode::BINARY
	);
	/** @brief The destructor */
 	G_API_GENEVA ~GIndividualSerialTCPConsumer() override = default;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A consumer used for network communication, using GParameterSet-derivatives
 * and async communication on the client side, so that a keep-alive of the
 * connection is possible
 */
class GIndividualAsyncTCPConsumer final
	: public Gem::Courtier::GAsioAsyncTCPConsumerT<Gem::Geneva::GParameterSet>
{
public:
	 /** @brief The default constructor */
	 G_API_GENEVA GIndividualAsyncTCPConsumer() = default;
	 /** @brief A constructor that takes a number of vital arguments */
	 G_API_GENEVA GIndividualAsyncTCPConsumer(
		 const unsigned short&
		 , const std::size_t& = 0
		 , const Gem::Common::serializationMode& = Gem::Common::serializationMode::BINARY
	 );
	 /** @brief The destructor */
  	 G_API_GENEVA ~GIndividualAsyncTCPConsumer() override = default;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A consumer used for multi-threaded processing, using GParameterSet-derivatives
 */
class GIndividualThreadConsumer final
	: public Gem::Courtier::GStdThreadConsumerT<Gem::Geneva::GParameterSet>
{
public:
	/** @brief The default constructor */
	G_API_GENEVA GIndividualThreadConsumer() = default;
	/** @brief The desstructor */
 	G_API_GENEVA ~GIndividualThreadConsumer() = default;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A consumer used for serial execution (mostly needed for debugging purposes).
 * Its payload are GParameterSet-derivatives.
 */
class GIndividualSerialConsumer
	: public Gem::Courtier::GSerialConsumerT<Gem::Geneva::GParameterSet>
{
public:
	/** @brief The default constructor */
	G_API_GENEVA GIndividualSerialConsumer() = default;
	/** @brief The desstructor */
 	G_API_GENEVA ~GIndividualSerialConsumer() override = default;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GINDIVIDUALSTANDARDCONSUMERS_HPP_ */
