/**
 * @file GMetaOptimizer.cpp
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
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

// Standard header files go here
#include <iostream>

// Boost header files go here

// Geneva header files go here
#include <geneva/Go2.hpp>

// The individual that should be optimized
#include "geneva/individuals/GMetaOptimizerIndividualT.hpp"

using namespace Gem::Geneva;

int main(int argc, char **argv) {
    Go2 go(argc, argv, "./config/Go2.json");

    //---------------------------------------------------------------------------
    // Client mode
    if(go.clientMode()) {
        return go.clientRun();
    } // Execution will end here in client mode

    //---------------------------------------------------------------------------
    // As we are dealing with a server, register a signal handler that allows us
    // to interrupt execution "on the run"
    signal(G_SIGHUP, Gem::Geneva::sigHupHandler);

    //---------------------------------------------------------------------------
    // Create a factory for GFunctionIndividual objects and perform
    // any necessary initial work.
    std::shared_ptr<gind::GFunctionIndividualFactory> gfi_ptr(
        new gind::GFunctionIndividualFactory("./config/GFunctionIndividual.json")
    );

    // Create a factory for GMetaOptimizerIndividual objects and perform
    // any necessary initial work.
    std::shared_ptr<gind::GMetaOptimizerIndividualFactoryT<gind::GFunctionIndividual>> gmoi_ptr(
        new gind::GMetaOptimizerIndividualFactoryT<gind::GFunctionIndividual>(
            "./config/GMetaOptimizerIndividual.json"
        )
    );

    // Add a pluggable optimization monitor (targeted at evolutionary algorithms) and register
    // it with Go2.
    go.registerPluggableOM(
        std::shared_ptr<gind::GOptOptMonitorT<gind::GFunctionIndividual>>(
            new gind::GOptOptMonitorT<gind::GFunctionIndividual>("./optProgress.C")
        )
    );

    // Register the GFunctionIndividualFactory with the meta-optimizer,
    // so it can be handed to the meta-optimization individuals later
    gmoi_ptr->registerIndividualFactory(gfi_ptr);

    // Add a content creator so Go2 can generate its own individuals, if necessary
    go.registerContentCreator(gmoi_ptr);

    // The meta genome carries only structure; its adaptors (n_parents flip, n_children integer-Gauss,
    // doubles Gauss) live on an OA-owned config the meta individual authors. Register it for the outer EA.
    {
        auto sample = gmoi_ptr->get_as<gind::GMetaOptimizerIndividualT<gind::GFunctionIndividual>>();
        go.registerAdaptionConfig("PERSONALITY_EA", sample->getAdaptionConfig());
    }

    // Add a default optimization algorithm to the Go2 object
    go.registerDefaultAlgorithm("ea");

    // Perform the actual optimization
    std::shared_ptr<gind::GMetaOptimizerIndividualT<gind::GFunctionIndividual>> bestIndividual_ptr =
        go.optimize()->getBestGlobalIndividual<gind::GMetaOptimizerIndividualT<gind::GFunctionIndividual>>();

    // Do something with the best result. Here we simply print the result to std-out.
    std::cout << "Best Result was:" << '\n' << *bestIndividual_ptr << '\n';

    //---------------------------------------------------------------------------
}
