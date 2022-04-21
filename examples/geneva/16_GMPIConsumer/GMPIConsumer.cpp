/**
 * @file GMPIConsumer.cpp
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

/*
 * *****************************************************************************
 * This example demonstrates how to use the MPI consumer with the geneva library
 * to solve an example optimization problem.
 *
 * The GMPIConsumerT combines the qualities of a brokered consumer and a corresponding
 * client. If running in server mode (internally that means it is the rank 0 node),
 * the consumer must be enrolled with the broker infrastructure of geneva.
 * If running in client mode (MPI rank 1-n) then the node just needs to run.
 * The consumer itself figures out its position in the computation cluster and will
 * connect to the master node (rank 0).
 * The master node will then wait for worker nodes to request work items. If a request
 * arrives it will retrieve a new work item from the broker and send it to the worker node.
 * Once the worker node has processed the work item it will send it back to the master node
 * and simultaneously request a new work item. At this point the master node will deliver the
 * processed work item to the broker and provide the worker with a new raw work item.
 *
 *
 * This example's basic structure is
 * taken from the example `06_DirectEA`. The example `06_DirectEA` concerning
 * consumers only provides the options of local serial execution, local multicore
 * execution and brokered execution using the GAsioConsumer. As the GMPIConsumerT
 * was developed at a later point in time and has another dependency (MPI library),
 * we decided to create this example separate example of how to use the GMPIConsumerT.
 * 
 * The example is best started with a runner program like `mpirun` like so:
 * `mpirun -np 4 ./GMPIConsumer
 * 
 * *****************************************************************************
 */

// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>
#include <chrono>

// Boost header files go here
#include <boost/lexical_cast.hpp>

// Geneva header files go here
#include "courtier/GMPIConsumerT.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/GenevaInitializer.hpp"
#include "geneva/G_OptimizationAlgorithm_EvolutionaryAlgorithm.hpp"

// The individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

using namespace Gem::Geneva;
using namespace Gem::Courtier;
using namespace Gem::Hap;
using namespace Gem::Common;

using namespace Gem::Common;
namespace po = boost::program_options;

/******************************************************************************/
// Default settings
const Gem::Common::serializationMode DEFAULTSERMODE = Gem::Common::serializationMode::TEXT;
const std::uint16_t DEFAULTNPRODUCERTHREADS = 10;
// number of threads in thread-pool of the MPI server that are used to handle connections with workers
const std::uint16_t DEFAULTNSERVERIOTHREADS = 0; // 0 means let GMPIConsumerT class make a suggestion
const std::size_t DEFAULTPOPULATIONSIZE06 = 100;
const std::size_t DEFAULTNPARENTS = 5; // Allow to explore the parameter space from many starting points
const std::uint32_t DEFAULTMAXITERATIONS = 200;
const long DEFAULTMAXMINUTES = 10;
const std::uint32_t DEFAULTREPORTITERATION = 1;
const duplicationScheme DEFAULTRSCHEME = duplicationScheme::VALUEDUPLICATIONSCHEME;
const sortingMode DEFAULTEAAPPSORTINGMODE = sortingMode::MUCOMMANU_SINGLEEVAL;
const std::size_t DEFAULTMAXRECONNECTS = 10;

/******************************************************************************/
/**
 * Parses the command line
 */
bool parseCommandLine(
        int argc,
        char **argv,
        Gem::Common::serializationMode &serMode,
        std::uint16_t &nProducerThreads,
        std::uint16_t &nServerIOThreads,
        std::size_t &populationSize,
        std::size_t &nParents,
        std::uint32_t &maxIterations,
        long &maxMinutes,
        std::uint32_t &reportIteration,
        duplicationScheme &rScheme,
        sortingMode &smode,
        std::size_t &maxReconnects) {
    // Create the parser builder
    Gem::Common::GParserBuilder gpb;

    gpb.registerCLParameter<Gem::Common::serializationMode>(
            "serializationMode", serMode, DEFAULTSERMODE,
            "Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)");

    gpb.registerCLParameter<std::uint16_t>(
            "nProducerThreads", nProducerThreads, DEFAULTNPRODUCERTHREADS,
            "The amount of random number producer threads");

    gpb.registerCLParameter<std::uint16_t>(
            "nServerIOThreads", nServerIOThreads, DEFAULTNSERVERIOTHREADS,
            "The number of threads in the thread pool of the MPI master node, which are used to handle incoming"
            "connections from worker nodes");

    gpb.registerCLParameter<std::size_t>(
            "populationSize", populationSize, DEFAULTPOPULATIONSIZE06, "The desired size of the population");

    gpb.registerCLParameter<std::size_t>(
            "nParents", nParents, DEFAULTNPARENTS, "The number of parents in the population");

    gpb.registerCLParameter<std::uint32_t>(
            "maxIterations", maxIterations, DEFAULTMAXITERATIONS, "Maximum number of iterations in the optimization");

    gpb.registerCLParameter<long>(
            "maxMinutes", maxMinutes, DEFAULTMAXMINUTES,
            "The maximum number of minutes the optimization of the population should run");

    gpb.registerCLParameter<std::uint32_t>(
            "reportIteration", reportIteration, DEFAULTREPORTITERATION,
            "The number of iterations after which information should be emitted in the population");

    gpb.registerCLParameter<duplicationScheme>(
            "rScheme", rScheme, DEFAULTRSCHEME, "The recombination scheme of the evolutionary algorithm");

    gpb.registerCLParameter<sortingMode>(
            "smode", smode, DEFAULTEAAPPSORTINGMODE,
            "Determines whether sorting is done in MUPLUSNU_SINGLEEVAL (0), MUCOMMANU_SINGLEEVAL (1) or MUNU1PRETAIN (2) mode");

    gpb.registerCLParameter<std::size_t>(
            "maxReconnects", maxReconnects, DEFAULTMAXRECONNECTS,
            "The number of times a client will try to reconnect when it couldn't reach the server");

    // Parse the command line and leave if the help flag was given. The parser
    // will emit an appropriate help message by itself
    if (Gem::Common::GCL_HELP_REQUESTED == gpb.parseCommandLine(argc, argv, true /*verbose*/)) {
        return false; // Do not continue
    }

    return true;
}

/******************************************************************************/
/**
 * The main function.
 */
int main(int argc, char **argv) {
    Gem::Common::serializationMode serMode;
    std::uint16_t nProducerThreads;
    std::uint16_t nServerIOThreads;
    std::size_t populationSize;
    std::size_t nParents;
    std::uint32_t maxIterations;
    long maxMinutes;
    std::uint32_t reportIteration;
    duplicationScheme rScheme;
    sortingMode smode;
    std::size_t maxReconnects;

    /****************************************************************************/
    // Initialization of Geneva
    GenevaInitializer gi;

    /****************************************************************************/
    // Retrieve all necessary configuration data from the command line

    if (!parseCommandLine(
            argc,
            argv,
            serMode,
            nProducerThreads,
            nServerIOThreads,
            populationSize,
            nParents,
            maxIterations,
            maxMinutes,
            reportIteration,
            rScheme,
            smode,
            maxReconnects)) {
        exit(1);
    }

    /****************************************************************************/
    // Random numbers are our most valuable good. Set the number of threads
    GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

    // Instantiate the MPI consumer.
    // Internally calls MPI_Init and finds out whether it is the master node (rank 0) or not
    auto consumer_ptr = std::make_shared<GMPIConsumerT<GParameterSet>>(serMode, nServerIOThreads /* &argc, &argv */ /* optional */);

    /****************************************************************************/
    // If this is supposed to be a client start an MPI consumer client
    if (consumer_ptr->isWorkerNode()) {
        consumer_ptr->run();

        return 0;
    }

    // If this is supposed to be the master node (server), then add it to the broker.
    // This will allow the consumer to pull raw work items from the broker and put processed work items back.
    GBROKER(Gem::Geneva::GParameterSet)->enrol_consumer(consumer_ptr);

    /****************************************************************************/
    // We can now start creating populations. We refer to them through the base class

    // Create a factory for GFunctionIndividual objects and perform
    // any necessary initial work.
    GFunctionIndividualFactory gfi("./config/GFunctionIndividual.json");

    // Create the first set of parent individuals. Initialization of parameters is done randomly.
    std::vector<std::shared_ptr<GFunctionIndividual>> parentIndividuals;
    for (std::size_t p = 0; p < nParents; p++) {
        parentIndividuals.push_back(gfi.get_as<GFunctionIndividual>());
    }

    /****************************************************************************/

    // Create an empty population
    auto pop_ptr = std::make_shared<GEvolutionaryAlgorithm>();

    // General settings
    pop_ptr->setPopulationSizes(populationSize, nParents);
    pop_ptr->setMaxIteration(maxIterations);
    pop_ptr->setMaxTime(std::chrono::minutes(maxMinutes));
    pop_ptr->setReportIteration(reportIteration);
    pop_ptr->setRecombinationMethod(rScheme);
    pop_ptr->setSortingScheme(smode);

    // Add individuals to the population.
    for (const auto &i: parentIndividuals) {
        pop_ptr->push_back(i);
    }

    // set executor mode in the producer/optimization algorithm
    pop_ptr->registerExecutor(execMode::BROKER,
                              "./config/GBrokerExecutor.json");

    /****************************************************************************/
    // Perform the actual optimization
    pop_ptr->optimize();

    // Retrieve the best individual found
    auto p = pop_ptr->getBestGlobalIndividual<GFunctionIndividual>();

    // Here you can do something with the best individual ("p") found.
    // We simply print its content here, by means of an operator<< implemented
    // in the GFunctionIndividual code.
    std::cout
            << "Best result found:" << std::endl
            << p << std::endl;

    return 0;
}
