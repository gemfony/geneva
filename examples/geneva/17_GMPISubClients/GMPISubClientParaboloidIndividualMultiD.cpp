/**
 * @file GMPISubClientParaboloidIndividualMultiD.cpp
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

#include "GMPISubClientParaboloidIndividualMultiD.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GMPISubClientParaboloidIndividualMultiD)

namespace Gem::Geneva {

/********************************************************************************************/
/**
 * The default constructor. This function will add a specified number of double parameters to this individual,
 * each of which has a constrained value range [-10:10].
 */
    GMPISubClientParaboloidIndividualMultiD::GMPISubClientParaboloidIndividualMultiD()
            : GMPISubClientIndividual(), M_PAR_MIN(-10.), M_PAR_MAX(10.) {
        for (std::size_t npar = 0; npar < m_nParameters; npar++) {
            // GConstrainedDoubleObject is constrained to [M_PAR_MIN:M_PAR_MAX[
            std::shared_ptr<GConstrainedDoubleObject> gcdo_ptr(new GConstrainedDoubleObject(M_PAR_MIN, M_PAR_MAX));
            // Add the parameters to this individual
            this->push_back(gcdo_ptr);
        }
    }

/********************************************************************************************/
/**
 * A standard copy constructor. All real work is done by the parent class.
 *
 * @param cp A copy of another GMPISubClientParaboloidIndividualMultiD
 */
    GMPISubClientParaboloidIndividualMultiD::GMPISubClientParaboloidIndividualMultiD(
            const GMPISubClientParaboloidIndividualMultiD &cp)
            : GMPISubClientIndividual(cp), M_PAR_MIN(-10.), M_PAR_MAX(10) { /* nothing */ }

/********************************************************************************************/
/**
 * Loads the data of another GMPISubClientParaboloidIndividualMultiD, camouflaged as a GObject.
 *
 * @param cp A copy of another GMPISubClientParaboloidIndividualMultiD, camouflaged as a GObject
 */
    void GMPISubClientParaboloidIndividualMultiD::load_(const GObject *cp) {
        // Check that we are dealing with a GMPISubClientParaboloidIndividualMultiD reference independent of this object and convert the pointer
        const GMPISubClientParaboloidIndividualMultiD *p_load = Gem::Common::g_convert_and_compare<GObject, GMPISubClientParaboloidIndividualMultiD>(
                cp, this);

        // Load our parent's data
        GMPISubClientIndividual::load_(cp);

        // No local data
        // sampleVariable = p_load->sampleVariable;
    }

/********************************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
    GObject *GMPISubClientParaboloidIndividualMultiD::clone_() const {
        return new GMPISubClientParaboloidIndividualMultiD(*this);
    }

/********************************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @return The value of this object
 */
    double GMPISubClientParaboloidIndividualMultiD::fitnessCalculation() {
        const uint32_t size{mpiSize(getCommunicator())}; // number of processes in the communicator
        double result = 0.; // Will hold the result
        std::vector<double> parVec; // Will hold the parameters

        // Retrieve the parameters
        this->streamline(parVec);

        // must create lValue binding
        std::optional<std::vector<double>> recvVecOpt{parVec};

        // distributed calculation of squares of individual parameters together with sub-clients
        MPITimeoutStatus status = distributedSolveWithTimeout(parVec, recvVecOpt, m_nParameters / size);

        if (status.timedOut) {
            std::cout << "Error: Sub-client unavailable: timeout triggered when communicating." << std::endl;
        } else if (!status.succeeded()) { // any error but a timeout
            std::cout << "MPI error occurred: " << std::endl
                      << mpiErrorString(status.status.MPI_ERROR) << std::endl;
        }

        // Calculate the sum of all individual results as the fitness value
        for (auto const &d: recvVecOpt.value()) {
            result += d;
        }

        return result;
    }


    int GMPISubClientParaboloidIndividualMultiD::subClientJob(MPI_Comm _communicator) {

        const uint32_t size{mpiSize(getCommunicator())};
        std::optional<std::vector<double>> dummyRecvVec{};


        while (true) {
            MPITimeoutStatus status = distributedSolveWithTimeout({}, dummyRecvVec, m_nParameters / size);

            if (status.timedOut) {
                std::cout << "Sub-client will exit due to a timeout." << std::endl
                          << "This is normal behaviour after the optimization has been finished." << std::endl
                          << "If it occurs mid-optimization it indicates unavailability of the Geneva-client."
                          << std::endl;
                return 0;
            } else if (!status.succeeded()) { // operation was not successful and has not timed out
                std::cout << "Error occurred: " << std::endl
                          << mpiErrorString(status.status.MPI_ERROR);
                return -1;
            }

            // continue with next iteration if no error or timeout occurred
        }

    }

    MPITimeoutStatus GMPISubClientParaboloidIndividualMultiD::distributedSolveWithTimeout(
            const std::optional<std::vector<double>> &sendVec,
            std::optional<std::vector<double>> &resultVec,
            const std::uint32_t &parsPerProc) {

        // vector to store the received subset of parameters in
        std::vector<double> parameterSubset{};
        parameterSubset.resize(parsPerProc);

        // completion status to return
        MPITimeoutStatus completionStatus{};

        // only the root process must pass a valid send address for scattering
        const double *scatterSendVecAddress{sendVec.has_value() ? sendVec.value().data() : nullptr};
        // only the root process must pass a valid send address for gathering
        double *gatherRecvVecAddress{resultVec.has_value() ? resultVec.value().data() : nullptr};

        // scatter data to all processes

        completionStatus = mpiScatterWithTimeout(scatterSendVecAddress,
                                                 parsPerProc,
                                                 parameterSubset.data(),
                                                 MPI_DOUBLE,
                                                 0,
                                                 getCommunicator(),
                                                 m_pollIntervalMSec,
                                                 m_pollTimeoutMSec);

        // return early with the error status if not completed successfully
        if (!completionStatus.succeeded()) {
            return completionStatus;
        }

        // actual calculation on the range of parameters assigned to this process
        for (double &par: parameterSubset) {
            // simulate longer time for the calculation of one parameter
            std::this_thread::sleep_for(std::chrono::milliseconds(m_delayPerParameterMSec));

            // square each parameter
            par = par * par;
        }

        // gather results from all processes
        completionStatus = mpiGatherWithTimeout(parameterSubset.data(),
                                                parsPerProc,
                                                gatherRecvVecAddress,
                                                MPI_DOUBLE,
                                                0,
                                                getCommunicator(),
                                                m_pollIntervalMSec,
                                                m_pollTimeoutMSec);

        return completionStatus;
    }


} // namespace Gem::Geneva
