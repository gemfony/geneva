/**
 * @file GMultiCriterionParabola.cpp
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
#include "GMultiCriterionParabolaIndividual.hpp"

using namespace Gem::Geneva;

int main(int argc, char **argv) {
    Go2 go(argc, argv, "./config/Go2.json");

    //---------------------------------------------------------------------
    // Client mode (networked)
    if(go.clientMode()) {
        go.clientRun();
        return 0;
    }

    //---------------------------------------------------------------------
    // Server mode, serial or multi-threaded execution

    // Create a factory for GMultiCriterionParabolaIndividual objects and perform
    // any necessary initial work.
    std::shared_ptr<GMultiCriterionParabolaIndividualFactory> gpi_ptr(
        new GMultiCriterionParabolaIndividualFactory(
            "./config/GMultiCriterionParabolaIndividual.json"
        )
    );

    // Add a content creator so Go2 can generate its own individuals, if necessary
    go.registerContentCreator(gpi_ptr);

    // The genome carries only structure; its Gauss adaptors live on an OA-owned config. Produce one
    // sample individual from the factory to author the config (all produced individuals share the same
    // genome geometry) and register it for the evolutionary algorithm.
    {
        auto sample = std::dynamic_pointer_cast<GMultiCriterionParabolaIndividual>(gpi_ptr->get());
        go.registerAdaptionConfig("PERSONALITY_EA", gpi_ptr->getAdaptionConfig(*sample));
    }

    // Add a default optimization algorithm to the Go2 object.
    // Note that this is the only algorithm that currently can handle multi-criterion optimization
    go.registerDefaultAlgorithm("ea");

    // Perform the actual optimization
    std::shared_ptr<GMultiCriterionParabolaIndividual> bestIndividual_ptr =
        go.optimize()->getBestGlobalIndividual<GMultiCriterionParabolaIndividual>();

    // Do something with the best result
    std::cout << bestIndividual_ptr << '\n';
}
