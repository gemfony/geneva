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
#include "common/GSingletonT.hpp"
#include "geneva/GIndividualStandardConsumerInitializerT.hpp"
#include "geneva/par/GParameterSet.hpp"

#include "courtier/consumers/GAsioConsumerT.hpp"
#include "courtier/consumers/GBaseConsumerT.hpp"
#include "courtier/consumers/GSerialConsumerT.hpp"
#include "courtier/consumers/GStdThreadConsumerT.hpp"
#include "courtier/consumers/GWebsocketConsumerT.hpp"
#ifdef GENEVA_BUILD_WITH_MPI_CONSUMER
#include "courtier/consumers/GMPIConsumerT.hpp"
#endif // GENEVA_BUILD_WITH_MPI_CONSUMER

namespace Gem::Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A consumer used for network communication, using GParameterSet-derivatives
 * and serial communication on the client side.
 */
class GIndividualWebsocketConsumer final
  : public cons::GWebsocketConsumerT<gpar::GParameterSet> {
public:
    // Forward to base-class constructor
    using cons::GWebsocketConsumerT<gpar::GParameterSet>::GWebsocketConsumerT;
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
  : public cons::GAsioConsumerT<gpar::GParameterSet> {
public:
    // Forward to base-class constructor
    using cons::GAsioConsumerT<gpar::GParameterSet>::GAsioConsumerT;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A consumer used for multi-threaded processing, using GParameterSet-derivatives
 */
class GIndividualThreadConsumer final
  : public cons::GStdThreadConsumerT<gpar::GParameterSet> {
public:
    // Forward to base-class constructor
    using cons::GStdThreadConsumerT<gpar::GParameterSet>::GStdThreadConsumerT;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A consumer used for serial execution (mostly needed for debugging purposes).
 * Its payload are GParameterSet-derivatives.
 */
class GIndividualSerialConsumer final
  : public cons::GSerialConsumerT<gpar::GParameterSet> {
public:
    /** @brief The default constructor */
    GIndividualSerialConsumer() = default;
    /** @brief The desstructor */
    ~GIndividualSerialConsumer() override = default;
};

#ifdef GENEVA_BUILD_WITH_MPI_CONSUMER
/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A consumer used for network communication with MPI, using GParameterSet-derivatives.
 */
class GIndividualMPIConsumer final
  : public cons::GMPIConsumerT<gpar::GParameterSet> {
public:
    // Forward to base-class constructor
    using cons::GMPIConsumerT<gpar::GParameterSet>::GMPIConsumerT;
};
#endif

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva */

// Export of GCommandContainerT for Geneva individuals
BOOST_CLASS_EXPORT_KEY(
    BOOST_IDENTITY_TYPE((Gem::Courtier::GCommandContainerT<
                         gpar::GParameterSet,
                         Gem::Courtier::networked_consumer_payload_command>))
) // NOLINT

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

#ifdef GENEVA_BUILD_WITH_MPI_CONSUMER
namespace Gem::Geneva {
/**
 * GMPIConsumerT can only be instantiated once, because multiple calls to MPI_Init
 * or MPI_Finalize are not allowed. The GIndividualMPIConsumer is therefore always
 * used as a singleton: mpiConsumerInstance() returns that single shared_ptr
 * instance. It replaces the former GMPIConsumerInstance macro.
 */
[[nodiscard]] inline std::shared_ptr<GIndividualMPIConsumer> mpiConsumerInstance() {
    return Gem::Common::GSingletonT<GIndividualMPIConsumer>::instance();
}
} /* namespace Gem::Geneva */
#endif
