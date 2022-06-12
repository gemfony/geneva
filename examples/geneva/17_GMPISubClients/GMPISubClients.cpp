/**
 * @file GMPISubClients.cpp
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

// Standard header files go here
#include <iostream>

// Boost header files go here

// MPI header files go here
#include <mpi.h>

// Geneva header files go here
#include "geneva/GMPISubClientOptimizer.hpp"
#include "courtier/GMPIHelperFunctions.hpp"

// The individual that should be optimized
#include "GMPISubClientParaboloidIndividual2D.hpp"


const std::int64_t POLL_INTERVAL_USEC{100};
const std::int64_t TIMEOUT_USEC{1'000'000}; // one second

using namespace Gem::Geneva;

enum CompletionStatus {
    SUCCESS,
    TIMEOUT,
    ERROR
};

/**
 * Waits for an async request to be completed
 * @param request the request handle
 * @return false if an error occurred, otherwise true.
 */
int waitForRequestCompletion(MPI_Request &request) {
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

int subClientJob(MPI_Comm communicator) {
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

int main(int argc, char **argv) {
    GMPISubClientOptimizer optimizer(argc, argv, "config/GMPISubClientOptimizer.json");

    // register the sub-client job which is executed by sub-clients who need to communicate with geneva clients inside a subgroup
    optimizer.registerSubClientJob(subClientJob);

    //---------------------------------------------------------------------
    // Initialize a client, if requested
    if (optimizer.clientMode()) {
        return optimizer.clientRun();
    }

    //---------------------------------------------------------------------
    // Add individuals and algorithms and perform the actual optimization cycle

    // Make an individual known to the optimizer
    std::shared_ptr<GMPISubClientParaboloidIndividual2D> p(new GMPISubClientParaboloidIndividual2D());
    optimizer.push_back(p);

    // Add an evolutionary algorithm to the Go2 class.
    optimizer & "ea";

    // Perform the actual optimization
    std::shared_ptr<GMPISubClientParaboloidIndividual2D>
            bestIndividual_ptr = optimizer.optimize()->getBestGlobalIndividual<GMPISubClientParaboloidIndividual2D>();

    // Do something with the best result
}
