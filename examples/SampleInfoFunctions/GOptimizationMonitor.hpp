/**
 * @file GOptimizationMonitor.hpp
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
#include <vector>
#include <fstream>

// Boost header files go here
#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

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


#ifndef GOPTIMIZATIONMONITOR_HPP_
#define GOPTIMIZATIONMONITOR_HPP_

// GenEvA header files go here
#include "GRandom.hpp"
#include "GBasePopulation.hpp"
#include "GIndividual.hpp"
#include "GDoubleCollection.hpp"
#include "GParameterSet.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"

namespace Gem
{
namespace GenEvA
{

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
 * the optimization for later analysis.
 *
 * The output of this example is simply the XML representation of the optimizationData struct, as
 * provided by the Boost.Serialization library.
 */
template <typename individualType>
class optimizationMonitor{
	/**
	 * A private struct that holds data specific to each individual
	 */
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
			ar & make_nvp("position", position);
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
		std::size_t position;
	};

	/**
	 * A private struct that holds data specific to all individuals of a given generation
	 */
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

	/**
	 * A private struct that holds all collected data for the entire optimization run
	 */
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
	/**
	 * The standard constructor. All collected data will to be written to file
	 *
	 * @param outputFile The name of the file to which the data should be written
	 */
	optimizationMonitor(const std::string& outputFile)
		 :outputFile_(outputFile)
	{ /* nothing */ }

	/**
	 * The function that does the actual collection of data. It can be called in
	 * three modes:
	 *
	 * INFOINIT: is called once before the optimization run.
	 * INFOPROCESSING: is called in regular intervals during the optimization, as determined by the user
	 * INFOEND: is called once after the optimization run
	 *
	 * @param im The current mode in which the function is called
	 * @param gbp A pointer to a GBasePopulation object for which information should be collected
	 */
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
				std::size_t position = 0;
				for(it=gbp->data.begin(); it!=gbp->data.end(); ++it){
					// We extract the data. (*it) is a boost::shared_ptr<GIndividual>,
					// so we need to convert it first.
					boost::shared_ptr<GDoubleCollection> gdc =
						(boost::dynamic_pointer_cast<individualType>(*it))->parameterbase_cast<GDoubleCollection>(0);

					individualData gdc_data;
					gdc_data.parameters = gdc->data; // data is itself a std::vector<double> in this case

					gdc_data.fitness = (*it)->fitness();
					gdc_data.isParent = (*it)->isParent();
					gdc_data.parentCounter = (*it)->getParentCounter();
					gdc_data.isDirty = (*it)->isDirty();

					gdc_data.position = position++;

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
			std::cout << "Fitness is " << gbp->data.at(0)->fitness() << std::endl;

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

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GOPTIMIZATIONMONITOR_HPP_ */
