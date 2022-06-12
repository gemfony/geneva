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

// TODO: Think about putting this into the GMPIConsumer by adding a command line argument: --nSubClients and making it possible
//  to register a callback (the sub-client code)
// TODO: make the sub-clients in this example exit after the optimization has been done. This must be realized with async calls and timeouts

// Standard header files go here
#include <iostream>

// Boost header files go here

// MPI header files go here
#include <mpi.h>

// Geneva header files go here
#include "geneva/GMPISubClientOptimizer.hpp"

// The individual that should be optimized
#include "GMPISubClientParaboloidIndividual2D.hpp"

using namespace Gem::Geneva;

// TODO: use async calls with timeout in order to exit sub-clients properly
int subClientJob(MPI_Comm communicator) {
    while (true) {
        char message{}; // part of the message to receive

        int subGroupSize{0};
        int subGroupRank{0};
        MPI_Comm_size(communicator, &subGroupSize);
        MPI_Comm_rank(communicator, &subGroupRank);

        std::cout << "SubClientJob has sub-group rank=" << subGroupRank << std::endl;

        const int messageSize{subGroupSize - 1}; // send one character for every other process

        // receive one character from the root process
        MPI_Scatter(
                nullptr, // we do not send as sub-client
                1, // send one char to each other process
                MPI_CHAR,
                &message, // receive into the buffer
                1, // send one character to every other process
                MPI_CHAR,
                0, // rank 0 (geneva client) is the root. The rank of this process is != 0
                communicator);

        // send the received character back to the root process
        MPI_Gather(
                &message, // send the message, which we have received, back
                1, // send one character only
                MPI_CHAR,
                nullptr, // we do not receive anything
                1,
                MPI_CHAR,
                0, // rank 0 (geneva client) is the root. The rank of this process is != 0
                communicator);
    }

    return 0;
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
