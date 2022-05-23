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
#include "geneva/Go2.hpp"
#include "courtier/GMPIConsumerT.hpp"

// The individual that should be optimized
#include "GMPIEvaluatedIndividual.hpp"

using namespace Gem::Geneva;


const int DEFAULT_N_SUB_CLIENTS = 3;
const int MPI_GENEVA_COLOR = 42;


int runGeneva(int &argc, char **&argv) {
    Go2 go(argc, argv, "config/Go2.json");

    //---------------------------------------------------------------------
    // Initialize a client, if requested
    if (go.clientMode()) {
        return go.clientRun();
    }

    //---------------------------------------------------------------------
    // Add individuals and algorithms and perform the actual optimization cycle

    // Make an individual known to the optimizer
    std::shared_ptr<GMPIEvaluatedIndividual> p(new GMPIEvaluatedIndividual());
    go.push_back(p);

    // Add an evolutionary algorithm to the Go2 class.
    go & "ea";

    // Perform the actual optimization
    std::shared_ptr<GMPIEvaluatedIndividual>
            bestIndividual_ptr = go.optimize()->getBestGlobalIndividual<GMPIEvaluatedIndividual>();

    // Do something with the best result

    return 0;
}

void initializeMPI() {
    // initialize MPI by using the static method of the GMPIConsumer
    // this ensures that MPI is initialized in the by GMPIConsumerT required way
    Gem::Courtier::GMPIConsumerT<GParameterSet>::initializeMPI();
}

void finalizeMPI() {
    // finalize MPI if not already done.
    // The point in time when GMPIConsumerT calls MPI_Finalize() is an implementation detail.
    // So it is best practice for a user to check if MPI has already been finalized by GMPIConsumerT
    int isFinalized{0};
    MPI_Finalized(&isFinalized);

    if (!isFinalized) {
        MPI_Finalize();
    }
}

void runMPISubClient(MPI_Comm subClientComm) {
    // use mpi to communicate with the fitnessCalculation function of the individual.
    // In this case we just use a useless barrier as an example

    // NOTE: This loop will exit, because this is a quick and easy example
    while (true) {
        // you might use a std::cout and a sleep in order to check that the barrier works
        MPI_Barrier(subClientComm);
    }
}

// TODO: extract reusable class that handles the functionality needed in this example and works with a config file

int main(int argc, char **argv) {
    int nSubClients{DEFAULT_N_SUB_CLIENTS};

    initializeMPI();

    int worldRank{0};
    int worldSize{0};
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    MPI_Comm genevaComm{};
    MPI_Comm subClientComm{};

    // all geneva clients and the geneva server
    // In case of 17 processes with one server, 4 clients and 3 sub-clients the ranks [0, 4, 8, 12, 16]
    const bool runsGeneva = worldRank % (nSubClients + 1) == 0;
    const int subCommColor = (worldRank - 1) / (nSubClients + 1);

    if (worldRank != 0) { // the server is in no sub-client group
        std::cout << "worldRank=" << worldRank << " is in sub-client-group " << subCommColor << std::endl;
    }


    if (runsGeneva) {
        // Create a communicator to be used by GMPIConsumerT
        MPI_Comm_split(MPI_COMM_WORLD, MPI_GENEVA_COLOR, worldRank, &genevaComm);
        // Set the GMPIConsumer to use this inter-communicator
        Gem::Courtier::GMPIConsumerT<GParameterSet>::setMPICommunicator(genevaComm);

        if (worldRank == 0) { // we want rank 0 to be the geneva server
            // does not need to be in sub-client communicator
            MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, worldRank, &subClientComm);
        } else {
            // all geneva clients are in separate communicators by using down-rounding integer division
            MPI_Comm_split(MPI_COMM_WORLD, subCommColor, worldRank, &subClientComm);
            // notify the custom individual which communicator to use
            GMPIEvaluatedIndividual::setCommunicator(subClientComm);
        }

        // allowed to return without MPI_Finalize. Geneva processes are capable of finalizing mpi themselves
        return runGeneva(argc, argv);
    } else {
        // create the genevaCommunicator, because the call is collective, but not enter it (pass MPI_UNDEFINED)
        MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, worldRank, &genevaComm);

        // putting nSubClients in one communicator by using down-rounding integer division
        MPI_Comm_split(MPI_COMM_WORLD, subCommColor, worldRank, &subClientComm);

        runMPISubClient(subClientComm);
    }

    finalizeMPI();
}
