/**
 * @file GArgumentParser.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Boost headers go here
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#ifndef GARGUMENTPARSER_HPP_
#define GARGUMENTPARSER_HPP_

// Geneva headers go here
#include "common/GCommonEnums.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "geneva/GOptimizationEnums.hpp"

// The individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

using namespace Gem::Geneva;

namespace Gem {
namespace Tests {

// Default settings
const std::uint16_t DEFAULTNPRODUCERTHREADS=10;
const std::uint16_t DEFAULTNEVALUATIONTHREADS=4;
const std::size_t DEFAULTPOPULATIONSIZE=100;
const std::size_t DEFAULTNPARENTS=5; // Allow to explore the parameter space from many starting points
const std::uint32_t DEFAULTMAXITERATIONS=2000;
const long DEFAULTMAXMINUTES=10;
const std::uint32_t DEFAULTREPORTITERATION=1;
const duplicationScheme DEFAULTRSCHEME=duplicationScheme::VALUEDUPLICATIONSCHEME;
const bool DEFAULTVERBOSE=true;
const execMode DEFAULTPARALLELIZATIONMODE=execMode::EXECMODE_MULTITHREADED;
const bool DEFAULTUSECOMMONADAPTOR=false; // whether to use a common adaptor for all GParameterT objects
const unsigned short DEFAULTPORT=10000;
const std::string DEFAULTIP="localhost";
const std::string DEFAULTCONFIGFILE="./GBrokerOverhead.cfg";
const sortingMode DEFAULTSORTINGSCHEME=MUPLUSNU_SINGLEEVAL;
const std::uint32_t DEFAULTSTARTITERATION=0;
const std::size_t DEFAULTNBTCONSUMERTHREADS=2;
const std::uint32_t DEFAULTGBTCNPROCUNITS=1;
const std::size_t DEFAULTPARDIM=100;
const double DEFAULTMINVAR=-10.;
const double DEFAULTMAXVAR=10.;
const std::uint16_t DEFAULTEVALFUNCTION=0;
const std::uint32_t DEFAULTADAPTIONTHRESHOLD=1;
const double DEFAULTGDAADPROB=1.0;

namespace po = boost::program_options;

bool parseCommandLine(
	int argc, char **argv
	, std::string& configFile
	, execMode& parallelizationMode
);

bool parseConfigFile(
	const std::string& configFile
	, std::uint16_t& nProducerThreads
	, std::uint16_t& nEvaluationThreads
	, std::size_t& populationSize
	, std::size_t& nParents
	, std::uint32_t& maxIterations
	, long& maxMinutes
	, std::uint32_t& reportIteration
	, duplicationScheme& rScheme
	, sortingMode& smode
	, std::uint32_t& nProcessingUnits
	, double& adProb
	, std::uint32_t& adaptionThreshold
	, double& sigma
	, double& sigmaSigma
	, double& minSigma
	, double& maxSigma
	, std::size_t& parDim
	, double& minVar
	, double& maxVar
	, solverFunction& df
);

} /* namespace Tests */
} /* namespace Gem */

#endif /* GARGUMENTPARSER_HPP_ */
