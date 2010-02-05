/**
 * @file GOptimizationMonitor.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
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

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA header files go here
#include "GEvolutionaryAlgorithm.hpp"
#include "GEnums.hpp"

namespace Gem
{
namespace GenEvA
{

/************************************************************************************************/
/**
 * GEvolutionaryAlgorithm, as the base class of all population classes in GenEvA, allows to store functions
 * and function objects with the signature
 *
 * void infoFunction(const infoMode&, GEvolutionaryAlgorithm * const);
 *
 * The function is called once before and after the optimization run, and in regular intervals
 * (as determined by the user) during the optimization.
 *
 * This example demonstrates how to use a function object to collect extensive information about the progress of
 * the optimization for later analysis.
 *
 * The output of this example is simply the XML representation of the optimizationData struct, as
 * provided by the Boost.Serialization library. The object simply takes a snapshot of the population during each
 * invocation. This makes it possible to follow the path of the optimization later-on.
 *
 * Note that the collected data can get very large in size. It is thus recommended not to use this facilty
 * for long optimization runs.
 */
class optimizationMonitor{
	/**
	 * A private struct that holds a snapshot of the monitored population at the time of a given generation
	 */
	struct generationData {
		template<class Archive>
		void serialize(Archive & ar, const unsigned int) {
			using boost::serialization::make_nvp;
			ar & make_nvp("pop", pop);
			ar & make_nvp("generation", generation);
		}

		boost::shared_ptr<GEvolutionaryAlgorithm> pop;
		boost::uint32_t generation;
	};

	/**
	 * A private struct that holds all collected snapshots for the entire optimization run
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
	optimizationMonitor(const std::string& outputFile, serializationMode serMode = XMLSERIALIZATION)
		 :outputFile_(outputFile),
		  serMode_(serMode)
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
	 * @param gbp A pointer to a GEvolutionaryAlgorithm object for which information should be collected
	 */
	void informationFunction(const infoMode& im, GEvolutionaryAlgorithm * const gbp){
		switch(im){
		case INFOINIT: // extract the population constraints
			oD_.populationSize = gbp->getDefaultPopulationSize();
			oD_.nParents = gbp->getNParents();
			oD_.maxGenerations = gbp->getMaxIteration();
			oD_.reportGeneration = gbp->getReportIteration();
			break;

		case INFOPROCESSING: // Collect information about the current population
			{ // Scope needed, as we have local data
				generationData genDat;
				genDat.generation = gbp->getIteration();

				boost::shared_ptr<GEvolutionaryAlgorithm> pop(new GEvolutionaryAlgorithm());
				pop->load(*gbp);
				genDat.pop = pop;

				oD_.gD.push_back(genDat);
			}

			// Emit a minimum of information to the audience
			std::cout << "Current fitness is " << gbp->at(0)->fitness() << std::endl;

			break;

		case INFOEND: // You could easily write out the data in your own format here
			{
				std::cout << "Writing result to disk ..." << std::endl;
				std::ofstream fstr(outputFile_.c_str());

				switch(serMode_)
				{
				case TEXTSERIALIZATION:
					{
						boost::archive::text_oarchive oa(fstr);
						oa << boost::serialization::make_nvp("optimizationData" , oD_);
					}
					break;

				case XMLSERIALIZATION:
					{
						boost::archive::xml_oarchive oa(fstr);
						oa << boost::serialization::make_nvp("optimizationData" , oD_);
					}
					break;

				case BINARYSERIALIZATION:
					{
						boost::archive::binary_oarchive oa(fstr);
						oa << boost::serialization::make_nvp("optimizationData" , oD_);
					}
					break;
				}

				fstr.close();
			}
			break;
		}
	}

private:
	optimizationMonitor(){} ///< Intentionally private

	std::string outputFile_;
	optimizationData oD_;
	serializationMode serMode_;
};

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GOPTIMIZATIONMONITOR_HPP_ */
