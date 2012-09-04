/**
 * @file GArgumentParser.hpp
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


// Standard headers go here
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Boost headers go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#ifndef GARGUMENTPARSER_HPP_
#define GARGUMENTPARSER_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include <common/GCommonEnums.hpp>
#include <common/GSerializationHelperFunctionsT.hpp>
#include <geneva/GOptimizationEnums.hpp>

// The individual that should be optimized
#include <geneva-individuals/GNeuralNetworkIndividual.hpp>

namespace Gem
{
  namespace Geneva
  {
    // Default settings
    const boost::uint16_t DEFAULTNPRODUCERTHREADS=10;
    const boost::uint16_t DEFAULTNEVALUATIONTHREADS=4;
    const std::size_t DEFAULTPOPULATIONSIZE=100;
    const std::size_t DEFAULTNPARENTS=5; // Allow to explore the parameter space from many starting points
    const boost::uint32_t DEFAULTMAXITERATIONS=2000;
    const long DEFAULTMAXMINUTES=10;
    const boost::uint32_t DEFAULTREPORTITERATION=1;
    const duplicationScheme DEFAULTRSCHEME=VALUEDUPLICATIONSCHEME;
    const bool DEFAULTVERBOSE=true;
    const bool DEFAULTPARALLELIZATIONMODEAP=1;
    const std::size_t DEFAULTARRAYSIZE=1000;
    const bool DEFAULTPRODUCTIONPLACE=true; // local production
    const bool DEFAULTUSECOMMONADAPTOR=false; // whether to use a common adaptor for all GParameterT objects
    const unsigned short DEFAULTPORT=10000;
    const std::string DEFAULTIP="localhost";
    const std::string DEFAULTCONFIGFILE="./GNeuralNetwork.cfg";
    const sortingMode DEFAULTSORTINGSCHEME=MUPLUSNU_SINGLEEVAL;
    const boost::uint32_t DEFAULTSTARTITERATION=0;
    const bool DEFAULTRETURNREGARDLESS=true;
    const std::size_t DEFAULTNBTCONSUMERTHREADS=2;
    const boost::uint32_t DEFAULTGBTCNPROCUNITS=1;
    const Gem::Geneva::trainingDataType DEFAULTTRAININGDATATYPE=Gem::Geneva::TDTNONE;
    const std::string DEFAULTTRAININGDATAFILE="geneva_training.dat";
    const std::size_t DEFAULTNDATASETS=2000;
    const std::string DEFAULTARCHITECTURE="2 4 4 1";
    const std::string DEFAULTTRAININGINPUTDATA="training.dat";
    const std::string DEFAULTRESULTPROGRAM="trainedNetwork.hpp";
    const std::string DEFAULTVISUALIZATIONFILE="visualization.C";
    const transferFunction DEFAULTTRANSFERFUNCTION=Gem::Geneva::SIGMOID;
    const double DEFAULTNNSIGMA=2;
    const double DEFAULTNNSIGMASIGMA=0.8;
    const double DEFAULTNNMINSIGMA=0.001;
    const double DEFAULTNNMAXSIGMA=2.;
    const double DEFAULTNNADPROB=0.05;
    const Gem::Common::serializationMode DEFAULTSERMODE=Gem::Common::SERIALIZATIONMODE_TEXT;

    namespace po = boost::program_options;

    bool parseCommandLine(
    		  int argc, char **argv
			, std::string& configFile
			, boost::uint16_t& parallelizationMode
			, bool& serverMode
			, std::string& ip
			, unsigned short& port
		    , Gem::Common::serializationMode& serMode
			, trainingDataType& tdt
			, std::string& trainingDataFile
			, std::size_t& nDataSets
			, std::vector<std::size_t>& architecture
	);

    bool parseConfigFile(
    		  const std::string& configFile
			, boost::uint16_t& nProducerThreads
			, boost::uint16_t& nEvaluationThreads
			, std::size_t& populationSize
			, std::size_t& nParents
			, boost::uint32_t& maxIterations
			, long& maxMinutes
			, boost::uint32_t& reportIteration
			, duplicationScheme& rScheme
			, sortingMode& smode
			, std::size_t& arraySize
			, bool& returnRegardless
			, boost::uint32_t& nProcessingUnits
			, transferFunction& tF
			, std::string& trainingInputData
			, std::string& resultProgram
			, std::string& visualizationFile
			, double& sigma
			, double& sigmaSigma
			, double& minSigma
			, double& maxSigma
			, double& adProb
			, const bool& verbose
	);

  } /* namespace Geneva */
} /* namespace Gem */

#endif /* GARGUMENTPARSER_HPP_ */
