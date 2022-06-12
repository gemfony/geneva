/**
 * @file GMPISubClientParaboloidIndividual2D.cpp
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

#include "GMPISubClientParaboloidIndividual2D.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GMPISubClientParaboloidIndividual2D)

namespace Gem::Geneva {

/********************************************************************************************/
/**
 * The default constructor. This function will add two double parameters to this individual,
 * each of which has a constrained value range [-10:10].
 */
    GMPISubClientParaboloidIndividual2D::GMPISubClientParaboloidIndividual2D()
            : GMPISubClientIndividual(), M_PAR_MIN(-10.), M_PAR_MAX(10.) {
        for (std::size_t npar = 0; npar < 2; npar++) {
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
 * @param cp A copy of another GMPISubClientParaboloidIndividual2D
 */
    GMPISubClientParaboloidIndividual2D::GMPISubClientParaboloidIndividual2D(
            const GMPISubClientParaboloidIndividual2D &cp)
            : GMPISubClientIndividual(cp), M_PAR_MIN(-10.), M_PAR_MAX(10) { /* nothing */ }

/********************************************************************************************/
/**
 * Loads the data of another GMPISubClientParaboloidIndividual2D, camouflaged as a GObject.
 *
 * @param cp A copy of another GMPISubClientParaboloidIndividual2D, camouflaged as a GObject
 */
    void GMPISubClientParaboloidIndividual2D::load_(const GObject *cp) {
        // Check that we are dealing with a GMPISubClientParaboloidIndividual2D reference independent of this object and convert the pointer
        const GMPISubClientParaboloidIndividual2D *p_load = Gem::Common::g_convert_and_compare<GObject, GMPISubClientParaboloidIndividual2D>(
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
    GObject *GMPISubClientParaboloidIndividual2D::clone_() const {
        return new GMPISubClientParaboloidIndividual2D(*this);
    }

/********************************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @return The value of this object
 */
    double GMPISubClientParaboloidIndividual2D::fitnessCalculation() {
        double result = 0.; // Will hold the result
        std::vector<double> parVec; // Will hold the parameters

        // get information about current MPI environment
        MPI_Comm communicator = GMPISubClientIndividual::getCommunicator();
        int subGroupSize{0};
        int subGroupRank{0};
        MPI_Comm_size(communicator, &subGroupSize);
        MPI_Comm_rank(communicator, &subGroupRank);

        const int messageSize{subGroupSize - 1}; // send one character for every other process

        std::cout << "Geneva client has sub-group rank=" << subGroupRank << std::endl;

        MPI_Scatter(
                m_echoMessage.data(), // send substrings of the test message
                messageSize, // send one char to each other process
                MPI_CHAR,
                nullptr, // we do not receive anything here
                1, // send one character to every other process
                MPI_CHAR,
                0, // rank 0 (this process) is the root.
                communicator);

        // allocate memory for receiving the results from sub-clients
        char *receiveBuffer = new char[messageSize];

        MPI_Gather(
                nullptr, // the master node does not send anything
                1, // send one character only
                MPI_CHAR,
                receiveBuffer,
                messageSize, // collect all sent characters
                MPI_CHAR,
                0, //rank 0 (this process) is the root.
                communicator);

        // verify the message has been echoed successfully
        for (std::size_t i{0}; i < messageSize; ++i) {
            if (receiveBuffer[i] != m_echoMessage[i]) {
                throw gemfony_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                                << "GMPISubClientParaboloidIndividual2D::fitnessCalculation(): Error!" << std::endl
                                << "the character `" << m_echoMessage[i] << "` has been sent and expected to be echoed" << std::endl
                                << "but the character `" << receiveBuffer[i] << "` has been received." << std::endl
                );
            }
        }

        // release dynamic buffer
        delete[] receiveBuffer;


        // if everything has worked fine! :)
        // No we can do the real calculation which is taken from GParaboloidIndividual2D

        this->streamline(parVec); // Retrieve the parameters

        // Do the actual calculation
        for (auto const &d: parVec) {
            result += d * d;
        }

        return result;
    }

/********************************************************************************************/

} // namespace Gem::Geneva
