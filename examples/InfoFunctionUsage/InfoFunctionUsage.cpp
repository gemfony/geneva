/**
 * @file InfoFunctionUsage.cpp
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
#include <string>

// Boost header files go here
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

// GenEvA header files go here
#include "GRandom.hpp"
#include "GBasePopulation.hpp"
#include "GIndividual.hpp"
#include "GDoubleCollection.hpp"
#include "GParameterSet.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GBoostThreadPopulation.hpp"

// The individual that should be optimized
// This is a "noisy" parabola
#include "GNoisyParabolaIndividual.hpp"

// Declares a function to parse the command line
#include "GCommandLineParser.hpp"

using namespace Gem::GenEvA;
using namespace Gem::Util;
using namespace Gem::GLogFramework;

/************************************************************************************************/
/**
 * GBasePopulation, as the base class of all population classes in GenEvA, allows to store functions
 * and function objects with the signature
 *
 * void infoFunction(const infoMode&, GBasePopulation * const);
 *
 * The function is called once before and after the optimization run, and in regular intervals
 * (as determined by the user) during the optimization.
 *
 * This example demonstrates how to use a function object to collect extensive information about the progress of
 * the optimization for later analysis. We search for the minimum of a simple function, such as provided by
 * the "parabola" or "noisy parabola" individuals.
 *
 * The output of this example is simply the XML representation of the optimizationData struct, as
 * provided by the Boost.Serialization library.
 */
class optimizationMonitor{
	struct individualData {
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version) {
			using boost::serialization::make_nvp;
			ar & make_nvp("parameters", parameters);
			ar & make_nvp("fitness", fitness);
			ar & make_nvp("isParent", isParent);
			ar & make_nvp("parentCounter",parentCounter);
			ar & make_nvp("isDirty", isDirty);
			ar & make_nvp("sigma", sigma);
			ar & make_nvp("sigmaSigma", sigmaSigma);
			ar & make_nvp("adaptionThreshold", adaptionThreshold);
			ar & make_nvp("adaptionCounter", adaptionCounter);
		}

		std::vector<double> parameters;
		double fitness;
		bool isParent;
		boost::uint32_t parentCounter;
		bool isDirty;
		double sigma;
		double sigmaSigma;
		boost::uint32_t adaptionThreshold;
		boost::uint32_t adaptionCounter;
	};

	struct generationData {
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version) {
			using boost::serialization::make_nvp;
			ar & make_nvp("iD", iD);
			ar & make_nvp("generation", generation);
		}

		std::vector<individualData> iD;
		boost::uint32_t generation;
	};

	struct optimizationData {
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version) {
			using boost::serialization::make_nvp;
			ar & make_nvp("gD", gD);
			ar & make_nvp("populationSize", populationSize);
			ar & make_nvp("nParents", nParents);
			ar & make_nvp("maxGenerations", maxGenerations);
			ar & make_nvp("reportGeneration", reportGeneration);
		}

		std::size_t populationSize;
		std::size_t nParents;
		boost::uint32_t maxGenerations;
		boost::uint32_t reportGeneration;

		std::vector<generationData> gD;
	};

public:
	optimizationMonitor(const std::string& outputFile)
		 :outputFile_(outputFile)
	{ /* nothing */ }

	void informationFunction(const infoMode& im, GBasePopulation * const gbp){
		switch(im){
		case INFOINIT: // extract the population constraints
			oD_.populationSize = gbp->getDefaultPopulationSize();
			oD_.nParents = gbp->getNParents();
			oD_.maxGenerations = gbp->getMaxGeneration();
			oD_.reportGeneration = gbp->getReportGeneration();
			break;

		case INFOPROCESSING: // Collect information about the current population
			{ // Scope needed, as we have local data
				generationData genDat;
				genDat.generation = gbp->getGeneration();

				std::vector<boost::shared_ptr<GIndividual> >::iterator it;
				for(it=gbp->data.begin(); it!=gbp->data.end(); ++it){
					// We extract the data. (*it) is a boost::shared_ptr<GIndividual>,
					// so we need to convert it first.
					boost::shared_ptr<GDoubleCollection> gdc =
						(boost::dynamic_pointer_cast<GParabolaIndividual>(*it))->parameterbase_cast<GDoubleCollection>(0);

					individualData gdc_data;
					gdc_data.parameters = gdc->data; // data is itself a std::vector<double> in this case

					gdc_data.fitness = (*it)->fitness();
					gdc_data.isParent = (*it)->isParent();
					gdc_data.parentCounter = (*it)->getParentCounter();
					gdc_data.isDirty = (*it)->isDirty();

					// Extract the adaptor from gdc
					boost::shared_ptr<GDoubleGaussAdaptor> gda =
						gdc->adaptor_cast<GDoubleGaussAdaptor>(GDoubleGaussAdaptor::adaptorName());

					// Retrieve mutation data
					gdc_data.sigma = gda->getSigma();
					gdc_data.sigmaSigma = gda->getSigmaAdaptionRate();
					gdc_data.adaptionThreshold = gda->getAdaptionThreshold();
					gdc_data.adaptionCounter = gda->getAdaptionCounter();


					genDat.iD.push_back(gdc_data);
				}

				oD_.gD.push_back(genDat);
			}

			// Emit a minimum of information to the audience
			std::cout << "Fitness is " << data.at(0).fitness() << std::endl;

			break;

		case INFOEND: // You could easily write out the data in your own format here
			{
				std::ofstream fstr(outputFile_.c_str());
				boost::archive::xml_oarchive oa(fstr);
				oa << boost::serialization::make_nvp("optimizationData" , oD_);
				fstr.close();
			}
			break;
		}
	}

private:
	optimizationMonitor(){} ///< Intentionally private

	std::size_t populationSize;
	std::size_t nParents;
	boost::uint32_t maxGenerations;
	boost::uint32_t reportGeneration;
	std::string outputFile_;
	optimizationData oD_;
};

/************************************************************************************************/
/**
 * The main function - similar to the GBasePopulation example.
 */
int main(int argc, char **argv){
	 std::size_t nPopThreads;
	 std::size_t populationSize, nParents;
	 double parabolaMin, parabolaMax;
	 boost::uint16_t nProducerThreads;
	 boost::uint32_t maxGenerations, reportGeneration;
	 boost::uint32_t adaptionThreshold;
	 long maxMinutes;
	 bool verbose;
	 recoScheme rScheme;

	// Parse the command line
	if(!parseCommandLine(argc, argv,
						 parabolaMin,
						 parabolaMax,
						 adaptionThreshold,
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
	LOGGER.addTarget(boost::shared_ptr<GBaseLogTarget>(new GDiskLogger("InfoFunctionUsage.log")));
	LOGGER.addTarget(boost::shared_ptr<GBaseLogTarget>(new GConsoleLogger()));

	// Random numbers are our most valuable good. Set the number of threads
	GRANDOMFACTORY.setNProducerThreads(nProducerThreads);

	// Set up a single parabola individual. Dimension is hardwired to 2, as we might
	// want to visualize the results later.
	boost::shared_ptr<GParabolaIndividual> noisyParabolaIndividual(new GNoisyParabolaIndividual(2, parabolaMin, parabolaMax, adaptionThreshold));

	// Create the optimizationMonitor
	boost::shared_ptr<optimizationMonitor> om(new optimizationMonitor("optimization.xml"));

	// Create the population
	GBoostThreadPopulation pop;
	pop.setNThreads(nPopThreads);

	// Register the monitor with the population. boost::bind knows how to handle a shared_ptr.
	pop.registerInfoFunction(boost::bind(&optimizationMonitor::informationFunction, om, _1, _2));

	pop.append(noisyParabolaIndividual);

	// Specify some population settings
	pop.setPopulationSize(populationSize,nParents);
	pop.setMaxGeneration(maxGenerations);
	pop.setMaxTime(boost::posix_time::minutes(maxMinutes)); // Calculation should be finished after 5 minutes
	pop.setReportGeneration(reportGeneration); // Emit information during every generation
	pop.setRecombinationMethod(rScheme); // The best parents have higher chances of survival

	// Do the actual optimization
	pop.optimize();

	// At this point we should have a file named "optimization.xml" in the same directory as this executable.

	std::cout << "Done ..." << std::endl;

	return 0;
}
