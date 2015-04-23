/**
 * @file GMetaOptimizer.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
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
 * http://www.gemfony.eu .
 */

// Standard header files go here
#include <iostream>

// Boost header files go here

// Geneva header files go here
#include <geneva/Go2.hpp>

// The individual that should be optimized
#include "geneva-individuals/GMetaOptimizerIndividualT.hpp"

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
   signal(G_SIGHUP, GObject::sigHupHandler);

   //---------------------------------------------------------------------------
   // Create a factory for GFunctionIndividual objects and perform
   // any necessary initial work.
   std::shared_ptr<GFunctionIndividualFactory>
      gfi_ptr(new GFunctionIndividualFactory("./config/GFunctionIndividual.json"));

   // Create a factory for GMetaOptimizerIndividual objects and perform
   // any necessary initial work.
   std::shared_ptr<GMetaOptimizerIndividualFactoryT<GFunctionIndividual> > gmoi_ptr(
         new GMetaOptimizerIndividualFactoryT<GFunctionIndividual>("./config/GMetaOptimizerIndividual.json")
   );

   // Register the GFunctionIndividualFactory with the meta-optimizer,
   // so it can be handed to the meta-optimization individuals later
   gmoi_ptr->registerIndividualFactory(gfi_ptr);

   // Add a content creator so Go2 can generate its own individuals, if necessary
   go.registerContentCreator(gmoi_ptr);

   // Create an optimization monitor (targeted at evolutionary algorithms) and register
   // it with the global store. This step is OPTIONAL. We recommend checking the chapters
   // on writing custom progress monitors within the Geneva framework.
   GOAMonitorStore->setOnce(
      "ea"
      , std::shared_ptr<GOptOptMonitorT<GFunctionIndividual> >(new GOptOptMonitorT<GFunctionIndividual>("./optProgress.C"))
   );


   // Add a default optimization algorithm to the Go2 object
   go.registerDefaultAlgorithm("ea");

   // Perform the actual optimization
   std::shared_ptr<GMetaOptimizerIndividualT<GFunctionIndividual> > bestIndividual_ptr
      = go.optimize<GMetaOptimizerIndividualT<GFunctionIndividual> >();

   // Do something with the best result. Here we simply print the result to std-out.
   std::cout
   << "Best Result was:" << std::endl
   << *bestIndividual_ptr << std::endl;

   //---------------------------------------------------------------------------
}
