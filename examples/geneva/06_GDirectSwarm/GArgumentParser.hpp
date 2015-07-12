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

#ifndef GARGUMENTPARSER_HPP_
#define GARGUMENTPARSER_HPP_

// Geneva headers go here
#include <common/GCommonEnums.hpp>
#include <common/GSerializationHelperFunctionsT.hpp>
#include <geneva/GOptimizationEnums.hpp>

// The individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

namespace Gem {
namespace Geneva {

using namespace Gem::Common;
namespace po = boost::program_options;

/******************************************************************************/

const std::uint16_t DEFAULTPARALLELIZATIONMODEAP=EXECMODE_MULTITHREADED;
const unsigned short DEFAULTPORT=10000;
const std::string DEFAULTIP="localhost";
const std::uint32_t DEFAULTMAXSTALLS06=0;
const std::uint32_t DEFAULTMAXCONNECTIONATTEMPTS06=100;
const std::uint16_t DEFAULTNPRODUCERTHREADS=10;
const Gem::Common::serializationMode DEFAULTSERMODE=Gem::Common::serializationMode::SERIALIZATIONMODE_TEXT;
const bool DEFAULTADDLOCALCONSUMER=false;
const std::uint16_t DEFAULTNEVALUATIONTHREADS=4;
const std::uint32_t DEFAULTMAXITERATIONS=200;
const std::uint32_t DEFAULTREPORTITERATION=1;
const long DEFAULTMAXMINUTES=10;
const std::uint16_t DEFAULTXDIMAP=1024;
const std::uint16_t DEFAULTYDIMAP=1024;
const bool DEFAULTFOLLOWPROGRESS=false;
const std::size_t DEFAULTNNEIGHBORHOODSAP=5;
const std::size_t DEFAULTNNEIGHBORHOODMEMBERSAP=20;
const double DEFAULTCPERSONALAP=2.;
const double DEFAULTCNEIGHBORHOODAP=2.;
const double DEFAULTCGLOBALAP=1.;
const double DEFAULTCVELOCITYAP=0.4;
const bool DEFAULTALLRANDOMINIT=true;

/******************************************************************************/
/**
 * Parses the command line
 */
bool parseCommandLine(
	int argc, char **argv
	, std::uint16_t& parallelizationMode
	, bool& serverMode
	, std::string& ip
	, unsigned short& port
	, std::uint32_t& maxStalls
	, std::uint32_t& maxConnectionAttempts
	, Gem::Common::serializationMode& serMode
	, bool& addLocalConsumer
	, std::size_t& nNeighborhoods
	, std::size_t& nNeighborhoodMembers
	, double& cPersonal
	, double& cNeighborhood
	, double& cGlobal
	, double& cVelocity
	, updateRule& ur
	, bool& allRandomInit
	, std::uint16_t& nProducerThreads
	, std::uint16_t& nEvaluationThreads
	, std::uint32_t& maxIterations
	, long& maxMinutes
	, std::uint32_t& reportIteration
	, std::uint16_t& xDim
	, std::uint16_t& yDim
	, bool& followProgress
);

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GARGUMENTPARSER_HPP_ */
