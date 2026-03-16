/**
 * @file GMPISubClientIndividual.cpp
 */

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

#include "geneva/GMPISubClientIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GMPISubClientIndividual)

namespace Gem::Geneva {

    MPI_Comm GMPISubClientIndividual::getCommunicator() {
        return GMPISubClientIndividual::m_communicator;
    }

    void GMPISubClientIndividual::setCommunicator(const MPI_Comm &communicator) {
        m_communicator = communicator;
    }

    void GMPISubClientIndividual::setClientStatusRequest(const MPI_Request &request) {
        m_clientStatusRequest = request;
    }

    void GMPISubClientIndividual::setClientMode(const ClientMode &mode) {
        m_clientMode = mode;
    }

    ClientStatus GMPISubClientIndividual::getClientStatus() {
        // If the optimization is finished this means that no Individual is being processed.
        // Therefore, clients can only call this method if they are running
        if (m_clientMode == CLIENT) {
            return ClientStatus::RUNNING;
        }

        MPI_Status status{};
        int isCompleted{};

        MPI_Test(&m_clientStatusRequest, &isCompleted, &status);

        if (!isCompleted) {
            return ClientStatus::RUNNING;
        }

        if (status.MPI_ERROR != MPI_SUCCESS) {
            return ClientStatus::ERROR;
        }

        return ClientStatus::FINISHED;
    }

    ClientMode GMPISubClientIndividual::getClientMode() {
        return m_clientMode;
    }


} // namespace Gem::Geneva
