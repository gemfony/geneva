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

#include "geneva/GIndividualStandardConsumers.hpp"

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
// Export of GCommandContainerT for Geneva individuals
BOOST_CLASS_EXPORT_IMPLEMENT(
    BOOST_IDENTITY_TYPE((Gem::Courtier::GCommandContainerT<
                         gpar::GParameterSet,
                         Gem::Courtier::networked_consumer_payload_command>))
) // NOLINT

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

namespace Gem::Geneva {
namespace {

/******************************************************************************/
/**
 * Self-registration of the standard consumers with the global consumer store at
 * library-load time, so that Go2 needs no explicit registration calls. The four
 * networked / threaded / serial consumers are default-constructible and are
 * registered as freshly built instances.
 */
GIndividualStandardConsumerInitializerT<GIndividualWebsocketConsumer> g_websocket_consumer_registrant;
GIndividualStandardConsumerInitializerT<GIndividualAsioConsumer>      g_asio_consumer_registrant;
GIndividualStandardConsumerInitializerT<GIndividualThreadConsumer>    g_thread_consumer_registrant;
GIndividualStandardConsumerInitializerT<GIndividualSerialConsumer>    g_serial_consumer_registrant;

#ifdef GENEVA_BUILD_WITH_MPI_CONSUMER
/**
 * The MPI consumer is a singleton (MPI must not be initialized or finalized more
 * than once), so it registers its existing singleton instance rather than a
 * freshly constructed one. This mirrors the former Go2 registration exactly:
 * MPI_Init / MPI_Finalize remain tied to consumer start / teardown, not to
 * registration, so the behaviour is unchanged.
 */
struct GMPIConsumerRegistrant {
    GMPIConsumerRegistrant() {
        auto provider = std::make_shared<GConsumerProviderT>(mpiConsumerInstance());
        consumerStore()->setOnce(provider->getMnemonic(), provider);
    }
};
GMPIConsumerRegistrant g_mpi_consumer_registrant;
#endif // GENEVA_BUILD_WITH_MPI_CONSUMER

/******************************************************************************/

} // anonymous namespace
} // namespace Gem::Geneva
