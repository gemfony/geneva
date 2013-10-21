/**
 * @file GDependentConstraints.cpp
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
   // We want to add an additional command line option

   bool printValid = false;
   bool useTrueFitness = false;

   std::vector<boost::shared_ptr<po::option_description> > od;

   boost::shared_ptr<po::option_description> printValid_option(
      new po::option_description(
         "validOnly"
         , po::value<bool>(&printValid)->implicit_value(true)->default_value(false) // This allows you say both --validOnly and --validOnly=true
         , "Enforces output of valid solutions only"
      )
   );
   od.push_back(printValid_option);

   boost::shared_ptr<po::option_description> printTrue_option(
      new po::option_description(
         "useTrueFitness"
         , po::value<bool>(&useTrueFitness)->implicit_value(true)->default_value(false) // This allows you say both --useTrueFitness and --useTrueFitness=true
         , "Plot untransformed fitness value, even if a transformation takes place for the purpose of optimization"
      )
   );
   od.push_back(printTrue_option);


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
	boost::shared_ptr<GProgressPlotterT<GParameterSet> > progplot_ptr(new GProgressPlotterT<GParameterSet>());

	progplot_ptr->addProfileVar("d", 0); // first double parameter
   progplot_ptr->addProfileVar("d", 1); // second double parameter
   progplot_ptr->setMonitorValidOnly(printValid); // Only record valid parameters, when printValid is set to true
   progplot_ptr->setUseTrueEvaluation(useTrueFitness); // Use untransformed evaluation values for logging

   go.registerPluggableOM(
      boost::bind(
         &GProgressPlotterT<GParameterSet>::informationFunction
         , progplot_ptr
         , _1
         , _2
      )
   );

   //---------------------------------------------------------------------------
   // Add a number of start values to the go object.
   // We also add a constraint definition here
   for(std::size_t i=0; i<10; i++) {
      boost::shared_ptr<GFunctionIndividual> p = gfi_ptr->get<GFunctionIndividual>();
      boost::shared_ptr<GDoubleSumConstraint> constraint_ptr(new GDoubleSumConstraint(1.));
      p->registerConstraint(constraint_ptr);
      go.push_back(p);
   }

   //---------------------------------------------------------------------------

	// Perform the actual optimization
	boost::shared_ptr<GFunctionIndividual> p = go.optimize<GFunctionIndividual>();

	// Here you can do something with the best individual ("p") found.
	// We simply print its content here, by means of an operator<< implemented
	// in the GFunctionIndividual code.
	std::cout
	<< "Best result found:" << std::endl
	<< p << std::endl;
}
