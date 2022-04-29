/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <iostream>

// Boost header files go here
#include <boost/utility/identity_type.hpp>

// Geneva headers go here
#include "common/GCommonEnums.hpp"
#include "common/GLogger.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GIndividualStandardConsumerInitializerT.hpp"

#include "courtier/GBaseConsumerT.hpp"
#include "courtier/GWebsocketConsumerT.hpp"
#include "courtier/GAsioConsumerT.hpp"
#include "courtier/GStdThreadConsumerT.hpp"
#include "courtier/GSerialConsumerT.hpp"
#ifdef GENEVA_BUILD_WITH_MPI_CONSUMER
#include "courtier/GMPIConsumerT.hpp"
#endif // GENEVA_BUILD_WITH_MPI_CONSUMER

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
	 // Forward to base-class constructor
	 using Gem::Courtier::GWebsocketConsumerT<Gem::Geneva::GParameterSet>::GWebsocketConsumerT;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A consumer used for network communication, using GParameterSet-derivatives
 * and async communication on the client side, so that a keep-alive of the
 * connection is possible
 */
class GIndividualAsioConsumer final
	: public Gem::Courtier::GAsioConsumerT<Gem::Geneva::GParameterSet>
{
public:
	 // Forward to base-class constructor
	 using Gem::Courtier::GAsioConsumerT<Gem::Geneva::GParameterSet>::GAsioConsumerT;
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
	 // Forward to base-class constructor
	 using Gem::Courtier::GStdThreadConsumerT<Gem::Geneva::GParameterSet>::GStdThreadConsumerT;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A consumer used for serial execution (mostly needed for debugging purposes).
 * Its payload are GParameterSet-derivatives.
 */
class GIndividualSerialConsumer final
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
/**
 * A consumer used for network communication with MPI, using GParameterSet-derivatives.
 */
class GIndividualMPIConsumer final
        : public Gem::Courtier::GMPIConsumerT<Gem::Geneva::GParameterSet>{
public:
    // Forward to base-class constructor
    using Gem::Courtier::GMPIConsumerT<Gem::Geneva::GParameterSet>::GMPIConsumerT;
};


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

// Export of GCommandContainerT for Geneva individuals
BOOST_CLASS_EXPORT_KEY(BOOST_IDENTITY_TYPE((Gem::Courtier::GCommandContainerT<Gem::Geneva::GParameterSet, Gem::Courtier::networked_consumer_payload_command>)))

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/**
 * GMPIConsumerT can only be instantiated once, because multiple calls to MPI_Init or MPI_Finalize are not allowed.
 * Therefore if you are not totally sure that you will call the constructor exactly once, then rather use the provided
 * macro to acquire a singleton std::shared_ptr instance.
 */
#define GMPIConsumerInstance Gem::Common::GSingletonT<GIndividualMPIConsumer>::Instance(0)
