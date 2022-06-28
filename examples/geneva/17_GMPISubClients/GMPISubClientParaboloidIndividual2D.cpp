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
#include "courtier/GMPIHelperFunctions.hpp"

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

        // allocate memory for receiving the results from sub-clients
        char *receiveBuffer = new char[subGroupSize];

        // initialize request handles for async communication
        MPI_Request scatterRequest{};
        MPI_Request gatherRequest{};

        // NOTE: this process (root with rank=0) has two roles in MPI_Gather and scatter.
        // It has the root role and also has the role of a normal process.
        // Therefore, it will on scatter also receive one item and on gather send one item

        // NOTE: here we could also use blocking calls.
        // But the clients in the main program need non-blocking calls to implement a timeout
        // Blocking calls do not match non-blocking calls

        MPI_Iscatter(
                m_echoMessage.data(), // send substrings of the test message
                1, // send one char to each other process
                MPI_CHAR,
                receiveBuffer, // receive one character as the root process
                1, // send one character to every other process
                MPI_CHAR,
                0, // rank 0 (this process) is the root.
                communicator,
                &scatterRequest);

        // wait for completion of async call
        MPI_Status scatterStatus{};

        MPI_Wait(&scatterRequest, &scatterStatus);
        if (scatterStatus.MPI_ERROR != MPI_SUCCESS) {
            std::cout << "Error happened: " << std::endl
                      << mpiErrorString(scatterStatus.MPI_ERROR) << std::endl;
        }

        MPI_Igather(
                receiveBuffer, // send one character as the root process
                1, // send one character only
                MPI_CHAR,
                receiveBuffer,
                1, // collect all sent characters
                MPI_CHAR,
                0, //rank 0 (this process) is the root.
                communicator,
                &gatherRequest);

        // wait for completion of async call
        MPI_Status gatherStatus{};
        MPI_Wait(&gatherRequest, &gatherStatus);
        if (gatherStatus.MPI_ERROR != MPI_SUCCESS) {
            std::cout << "Error happened: " << std::endl
                      << mpiErrorString(gatherStatus.MPI_ERROR) << std::endl;
        }

        // verify the message has been echoed successfully
        for (std::size_t i{0}; i < subGroupSize; ++i) {
            if (receiveBuffer[i] != m_echoMessage[i]) {
                throw gemfony_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                                << "GMPISubClientParaboloidIndividual2D::fitnessCalculation(): Error!" << std::endl
                                << "the character `" << m_echoMessage[i] << "` has been sent and expected to be echoed"
                                << std::endl
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


    const std::int64_t POLL_INTERVAL_USEC{100};
    const std::int64_t TIMEOUT_USEC{1'000'000}; // one second

    using namespace Gem::Geneva;

    enum CompletionStatus {
        SUCCESS,
        TIMEOUT,
        ERROR
    };

    int GMPISubClientParaboloidIndividual2D::subClientJob(MPI_Comm communicator) {
        std::uint32_t count{0};

        while (true) {
            char message{}; // part of the message to receive

            int subGroupSize{0};
            int subGroupRank{0};
            MPI_Comm_size(communicator, &subGroupSize);
            MPI_Comm_rank(communicator, &subGroupRank);

            MPI_Request scatterRequest{};
            MPI_Request gatherRequest{};

            // create a message to be emitted in case of timeout
            std::stringstream timeoutMessage{};
            timeoutMessage << "Sub-client with rank=" << subGroupRank << " in communicator " << communicator
                           << "has received " << count << " messages so far and will now exit due to a timeout."
                           << std::endl
                           << "This is normal behaviour after the optimization has been finished." << std::endl
                           << "If it occurs mid-optimization it indicated unavailability of the Geneva-client."
                           << std::endl;

            // receive one character from the root process
            MPI_Iscatter(
                    nullptr, // we do not send as sub-client
                    1, // send one char to each other process
                    MPI_CHAR,
                    &message, // receive into the buffer
                    1, // send one character to every other process
                    MPI_CHAR,
                    0, // rank 0 (geneva client) is the root. The rank of this process is != 0
                    communicator,
                    &scatterRequest);


            switch (waitForRequestCompletion(scatterRequest)) {
                case SUCCESS:
                    break;
                case TIMEOUT: {
                    std::cout << timeoutMessage.str();
                    return 0;
                }
                case ERROR:
                    return -1;
            }

            // send the received character back to the root process
            MPI_Igather(
                    &message, // send the message, which we have received, back
                    1, // send one character only
                    MPI_CHAR,
                    nullptr, // we do not receive anything
                    1,
                    MPI_CHAR,
                    0, // rank 0 (geneva client) is the root. The rank of this process is != 0
                    communicator,
                    &gatherRequest);

            switch (waitForRequestCompletion(gatherRequest)) {
                case SUCCESS:
                    break;
                case TIMEOUT: {
                    std::cout << timeoutMessage.str();
                    return 0;
                }
                case ERROR:
                    return -1;
            }

            // increment successful message counter
            ++count;
        }
    }

    /**
     * Waits for an async request to be completed
     * @param request the request handle
     * @return false if an error occurred, otherwise true.
     */
    int GMPISubClientParaboloidIndividual2D::waitForRequestCompletion(MPI_Request &request) {
        int isCompleted{0};
        MPI_Status status{};
        std::chrono::microseconds timeElapsed{0};
        const auto timeStart = std::chrono::steady_clock::now();

        while (true) {
            MPI_Test(&request,
                     &isCompleted,
                     &status);

            if (isCompleted) {
                if (status.MPI_ERROR != MPI_SUCCESS) {
                    std::cout << "Error happened: " << std::endl
                              << mpiErrorString(status.MPI_ERROR) << std::endl;
                    return CompletionStatus::ERROR;
                }
                return CompletionStatus::SUCCESS;
            }

            // update elapsed time to compare with timeout
            auto currentTime = std::chrono::steady_clock::now();
            timeElapsed = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - timeStart);

            if (timeElapsed > std::chrono::microseconds{TIMEOUT_USEC}) {
                return CompletionStatus::TIMEOUT;
            }

            // sleep some time before polling again for completion status
            std::this_thread::sleep_for(std::chrono::microseconds(POLL_INTERVAL_USEC));
        }
    }

/********************************************************************************************/

} // namespace Gem::Geneva
