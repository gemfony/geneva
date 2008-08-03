/**
 * @file
 */

/* GSimpleBasePopulation.cpp
 *
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 */

// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>

// Boost header files go here
#include <boost/function.hpp>

// GenEvA header files go here
#include "GBasePopulation.hpp"
#include "GDoubleCollection.hpp"
#include "GParameterSet.hpp"
#include "GEvaluator.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GBoostThreadPopulation.hpp"

using namespace boost;
using namespace Gem::GenEvA;
using namespace Gem::Util;
using namespace Gem::GLogFramework;

/************************************************************************************************/
/**
 * Emits information about the population it has been given
 */
void infoFunction(GBasePopulation * const gbp){
  bool isDirty = false;
  std::ostringstream information;
  information << std::setprecision(10)
	          << "h" << gbp << "->Fill(" << gbp->getGeneration() << ", " << gbp->at(0)->getCurrentFitness(isDirty) << ");";
  if(isDirty){
    information << "// dirty!";
  }

  information << std::endl;

  LOGGER.log(information.str(), Gem::GLogFramework::PROGRESS);
}

/************************************************************************************************/
/**
 * Set up an evaluation function. Note that we are over-cautious here. If you are
 * dealing with your own objects, you might want to deploy faster alternatives
 */
double parabola(const GParameterSet& gps){
	// Does gps have any data at all ?
	if(gps.empty()){
		std::cout << "In eval: Error! Supplied GParameterSet object" << std::endl
			      << "does not contain any data" << std::endl;
		std::terminate();
	}

	// Extract data - we know there is at least one shared_ptr<GParameterBase> registered.
	// gps is essentially a std::vector<boost::shared_ptr<GParameterBase> > . get() is
	// the shared_ptr way of retrieving the stored object.
	const GParameterBase *data_base = gps.at(0).get();

	// We know there should be a GDOubleCollection present - extract it.
	const GDoubleCollection *gdc_load = dynamic_cast<const GDoubleCollection*>(data_base);

	// Check that the conversion worked. dynamic_cast emits an empty pointer,
	// if this was not the case.
	if(!gdc_load){
		std::cout << "In eval: Conversion error!" << std::endl;
		std::terminate();
	}

	// Great - now we can do the actual calculations. We do this the fancy way ...
	double result = 0;
	std::vector<double>::const_iterator cit;
	for(cit=gdc_load->begin(); cit!=gdc_load->end(); ++cit){
		result += std::pow(*cit, 2); // Sum up the squares
	}

	return result;
}

/************************************************************************************************/
/**
 * The main function
 */
int main(int argc, char **argv){
	// Add some log levels to the logger
	LOGGER.addLogLevel(Gem::GLogFramework::CRITICAL);
	LOGGER.addLogLevel(Gem::GLogFramework::WARNING);
	LOGGER.addLogLevel(Gem::GLogFramework::INFORMATIONAL);
	LOGGER.addLogLevel(Gem::GLogFramework::PROGRESS);

	// Add log targets to the system
	LOGGER.addTarget(new Gem::GLogFramework::GDiskLogger("GSimpleBasePopulation.log"));
	LOGGER.addTarget(new Gem::GLogFramework::GConsoleLogger());

	// Random numbers are our most valuable good. Set the number of threads
	// 7 threads will be producing [0,1[ floating point values, 3 threads
	// will create gaussian random numbers
    GRANDOMFACTORY.setNProducerThreads(7,3);

	// Set up a GDoubleCollection with 1000 values, each initialized
	// with a random number in the range [-100,100[
	GDoubleCollection *gdc = new GDoubleCollection(1000,-100.,100.);

	// Set up and register an adaptor for the collection, so it
	// knows how to be mutated. We want a sigma of 0.5, sigma-adaption of 0.05 and
	// a minimum sigma of 0.02.
	GDoubleGaussAdaptor *gdga = new GDoubleGaussAdaptor(0.5,0.05,0.02,"gauss_mutation");
	gdc->addAdaptor(gdga);

	// Set up an evaluator and give it an evaluation function
	GEvaluator *ge = new GEvaluator();
	ge->registerEvalFunction(parabola);

	// Set up a GParameterSet object and register the evaluator
	GParameterSet *parabolaIndividual = new GParameterSet();
	parabolaIndividual->registerEvaluator(ge);

	// Add the double numbers to the parameter set
	parabolaIndividual->append(gdc);

	// Now we've got our first individual and can create a population.
	// You can choose between a simple, non-parallel population and a
	// multi-threaded population.

	// Uncomment the next line and comment out the GBoostThreadPopulation lines
	// to get the slower, serial execution.

	// GBasePopulation pop;

	GBoostThreadPopulation pop;
	pop.setNThreads(4);

	pop.append(parabolaIndividual);

	// Specify some population settings
	pop.setPopulationSize(100,5); // 100 individuals, 5 parents
	pop.setMaxGeneration(2000); // A maximum of 2000 generations is allowed
	pop.setMaxTime(0,0,5,0); // Calculation should be finished after 5 minutes
	pop.setReportGeneration(1); // Emit information during every generation
    pop.registerInfoFunction(infoFunction); // Determines how information is submitted
    pop.setRecombinationMethod(VALUERECOMBINE); // The best parents have higher chances of survival

    // Do the actual optimization
    pop.optimize();

    std::cout << "Done ..." << std::endl;

	return 0;
}
