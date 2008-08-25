/**
 * @file GBasePopulationSerialization.cpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
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
#include <fstream>

// Boost header files go here
#include <boost/function.hpp>
#include <boost/cstdint.hpp>
#include <boost/utility.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/date_time.hpp>
#include <boost/lexical_cast.hpp>

// GenEvA header files go here
#include "GRandom.hpp"
#include "GBasePopulation.hpp"
#include "GDoubleCollection.hpp"
#include "GParameterSet.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GBoostThreadPopulation.hpp"
#include "GIndividual.hpp"

// The individual that should be optimized
// This is a simple parabola
#include "GParabolaIndividual.hpp"

// Declares a function to parse the command line
#include "GCommandLineParser.hpp"

using namespace Gem::GenEvA;
using namespace Gem::Util;
using namespace Gem::GLogFramework;

/************************************************************************************************/
/**
 * The main function. We search for the minimum of a parabola. In this test an entire population
 * is serialized and then again de-serialized. Subsequently an optimization run searches the
 * optimum of the de-serialized population. This is a variation of the GSimpleBasePopulation
 * example.
 */
int main(int argc, char **argv){
	 std::size_t parabolaDimension, nPopThreads;
	 std::size_t populationSize, nParents;
	 double parabolaMin, parabolaMax;
	 boost::uint16_t nProducerThreads;
	 boost::uint32_t maxGenerations, reportGeneration;
	 long maxMinutes;
	 bool verbose;
	 recoScheme rScheme;

	// Parse the command line
	if(!parseCommandLine(argc, argv,
		  			     parabolaDimension,
						 parabolaMin,
						 parabolaMax,
						 nProducerThreads,
						 nPopThreads,
						 populationSize,
						 nParents,
						 maxGenerations,
						 maxMinutes,
						 reportGeneration,
						 rScheme,
						 verbose))
	{ std::terminate(); }

	// Add some log levels to the logger
	LOGGER.addLogLevel(Gem::GLogFramework::CRITICAL);
	LOGGER.addLogLevel(Gem::GLogFramework::WARNING);
	LOGGER.addLogLevel(Gem::GLogFramework::INFORMATIONAL);
	LOGGER.addLogLevel(Gem::GLogFramework::PROGRESS);

	// Add log targets to the system
	LOGGER.addTarget(boost::shared_ptr<GBaseLogTarget>(new GDiskLogger("GBasePopulationSerialization.log")));
	LOGGER.addTarget(boost::shared_ptr<GBaseLogTarget>(new GConsoleLogger()));

	// Random numbers are our most valuable good. Set the number of threads
	GRANDOMFACTORY.setNProducerThreads(nProducerThreads);

	// Set up a single parabola individual
	boost::shared_ptr<GParabolaIndividual>
		parabolaIndividual(new GParabolaIndividual(parabolaDimension, parabolaMin, parabolaMax));

	// Now we've got our first individual and can create a population.
	GBoostThreadPopulation pop;
	pop.setNThreads(nPopThreads);

	pop.append(parabolaIndividual);

	// Specify some population settings
	pop.setPopulationSize(populationSize,nParents);
	pop.setMaxGeneration(maxGenerations);
	pop.setMaxTime(boost::posix_time::minutes(maxMinutes)); // Calculation should be finished after 5 minutes
	pop.setReportGeneration(reportGeneration); // Emit information during every generation
	pop.setRecombinationMethod(rScheme); // The best parents have higher chances of survival

	// Serialize
	pop.setSerializationMode(XMLSERIALIZATION);
	std::ostringstream popStream;
	popStream << pop.toString();

	// Write to file for later inspection
	std::ofstream popFile("pop.xml");
	popFile << popStream.str();
	popFile.close();

	GObject *local = (GObject *)NULL;
	std::istringstream istr(pop.toString());
	{
		boost::archive::xml_iarchive ia(istr);
		ia >> boost::serialization::make_nvp("classhierarchyFromGObject", local);
	} // note: explicit scope here is essential so the ia-destructor gets called

	GBoostThreadPopulation *newPop = dynamic_cast<GBoostThreadPopulation *>(local);

	// Do the actual optimization
	if(newPop)
		newPop->optimize();
	else{
		std::cout << "Error: Could not de-serialize the population" << std::endl;
	}

	delete newPop;

	return 0;
}
