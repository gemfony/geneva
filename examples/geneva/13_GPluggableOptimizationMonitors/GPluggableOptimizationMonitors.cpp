/**
 * @file GPluggableOptimizationMonitors.cpp
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
#include <boost/bind.hpp>
#include <boost/ref.hpp>

// Geneva header files go here
#include "geneva/Go2.hpp"

// The individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

using namespace Gem::Geneva;
namespace po = boost::program_options;

int main(int argc, char **argv) {
   //---------------------------------------------------------------------------
   // We want to add additional command line options

   std::string monitorSpec = "empty";
   bool observeBoundaries = "false";

   std::vector<boost::shared_ptr<po::option_description> > od;

   boost::shared_ptr<po::option_description> monitorSpec_option(
      new po::option_description(
         "monitorSpec"
         , po::value<std::string>(&monitorSpec)->default_value(std::string("empty"))
         , "Allows you to specify variables to be monitored like this: \"d(var0 -10 10)\""
      )
   );
   od.push_back(monitorSpec_option);

   boost::shared_ptr<po::option_description> observeBoundaries_option(
      new po::option_description(
         "observeBoundaries"
         , po::value<bool>(&observeBoundaries)->implicit_value(true)->default_value(false) // This allows you say both --observeBoundaries and --observeBoundaries=true
         , "Only plot inside of specified boundaries (no effect, when monitorSpec hasn't been set)"
      )
   );
   od.push_back(observeBoundaries_option);

   Go2 go(argc, argv, "./config/Go2.json", od);

	//---------------------------------------------------------------------------
	// Client mode
	if(go.clientMode()) {
		return go.clientRun();
	} // Execution will end here in client mode

	//---------------------------------------------------------------------------
   // Create a factory for GFunctionIndividual objects and perform
   // any necessary initial work.
	boost::shared_ptr<GFunctionIndividualFactory>
	   gfi_ptr(new GFunctionIndividualFactory("./config/GFunctionIndividual.json"));

	//---------------------------------------------------------------------------
   // Register a progress plotter with the global optimization algorithm factory
	if(monitorSpec != "empty") {
	   boost::shared_ptr<GProgressPlotterT<GParameterSet, double> > progplot_ptr(new GProgressPlotterT<GParameterSet, double>());

	   progplot_ptr->setProfileSpec(monitorSpec);
	   progplot_ptr->setObserveBoundaries(observeBoundaries);

	   go.registerPluggableOM(
	      boost::bind(
	         &GProgressPlotterT<GParameterSet, double>::informationFunction
	         , progplot_ptr
	         , _1
	         , _2
	      )
	   );
	}

   //---------------------------------------------------------------------------

   // Add a content creator so Go2 can generate its own individuals, if necessary
   go.registerContentCreator(gfi_ptr);

	// Add a default optimization algorithm to the Go2 object. This is optional.
   // Indeed "ea" is the default setting anyway. However, if you do not like it, you
   // can register another default algorithm here, which will then be used, unless
   // you specify other algorithms on the command line. You can also add a smart
   // pointer to an optimization algorithm here instead of its mnemonic.
	go.registerDefaultAlgorithm("ea");

	// Perform the actual optimization
	boost::shared_ptr<GFunctionIndividual> p = go.optimize<GFunctionIndividual>();

	// Here you can do something with the best individual ("p") found.
	// We simply print its content here, by means of an operator<< implemented
	// in the GFunctionIndividual code.
	std::cout
	<< "Best result found:" << std::endl
	<< p << std::endl;
}
