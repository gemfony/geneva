/**
 * @file GMetaOptimizer.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

// Standard header files go here
#include <iostream>

// Boost header files go here

// Geneva header files go here
#include <geneva/Go2.hpp>

// The individual that should be optimized
#include "GMetaOptimizerIndividual.hpp"
#include "GOptOptMonitor.hpp"

using namespace Gem::Geneva;

int main(int argc, char **argv) {
   Go2 go(argc, argv, "./config/Go2.json");

   //---------------------------------------------------------------------
   // Client mode
   if(go.clientMode()) {
      return go.clientRun();
   } // Execution will end here in client mode

   //---------------------------------------------------------------------

   // Create a factory for GMetaOptimizerIndividual objects and perform
   // any necessary initial work.
   boost::shared_ptr<GMetaOptimizerIndividualFactory> gmoi_ptr(
         new GMetaOptimizerIndividualFactory("./config/GMetaOptimizerIndividual.json")
   );

   // Create a factory for evolutionary algorithms in serial mode.
   // We choose serial execution, to allow the sub-populations to use multiple threads.
   GEvolutionaryAlgorithmFactory ea_factory(
      "./config/GEvolutionaryAlgorithm.json"
      , EXECMODE_SERIAL
      , gmoi_ptr
   );

   boost::shared_ptr<GBaseEA> ea_ptr = ea_factory.get<GBaseEA>();

   // Create an optimization monitor and register it with the optimizer
   boost::shared_ptr<GOptOptMonitor> mon_ptr(new GOptOptMonitor("./optProgress.C"));
   ea_ptr->registerOptimizationMonitor(mon_ptr);

   // Add an EA-object to the Go2 object
   go & ea_ptr;

   // Perform the actual optimization
   boost::shared_ptr<GMetaOptimizerIndividual> bestIndividual_ptr = go.optimize<GMetaOptimizerIndividual>();

   // Do something with the best result. Here we simply print the result to stdout.
   std::cout
   << "Best Result was:" << std::endl
   << *bestIndividual_ptr << std::endl;
}
