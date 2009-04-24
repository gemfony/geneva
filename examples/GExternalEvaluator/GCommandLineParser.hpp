/**
 * @file GCommandLineParser.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of the Geneva library.
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

// Standard headers go here

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Boost headers go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/program_options.hpp>

#ifndef GCOMMANDLINEPARSER_HPP_
#define GCOMMANDLINEPARSER_HPP_

// GenEvA headers go here
#include "GEnums.hpp"
#include "GSerializationHelperFunctions.hpp"

namespace Gem
{
namespace GenEvA
{

// Default settings
const std::size_t DEFAULTPOPSIZE=100;
const std::size_t DEFAULTNPARENTS=5;
const std::string DEFAULTPROGRAM="./evaluator/evaluator";
const boost::uint16_t DEFAULTNPRODUCERTHREADS=5;
const boost::uint16_t DEFAULTNPROCESSINGTHREADS=4;
const boost::uint32_t DEFAULTMAXGENERATIONS=2000;
const long DEFAULTMAXMINUTES=0;
const boost::uint32_t DEFAULTREPORTGENERATION=1;
const recoScheme DEFAULTRSCHEME=VALUERECOMBINE;
const bool DEFAULTVERBOSE=true;
const boost::uint32_t DEFAULTADAPTIONTHRESHOLD=1;
const boost::uint16_t DEFAULTPARALLEL=1;
const std::string DEFAULTEXTERNALARGUMENTS="empty";
const unsigned short DEFAULTPORT=10000;
const std::string DEFAULTIP="localhost";
const boost::uint32_t DEFAULTNEVALUATIONS=5;
const Gem::GenEvA::dataExchangeMode DEFAULTEXCHANGEMODE=Gem::GenEvA::BINARYEXCHANGE;
const bool DEFAULTSORTINGSCHEME=true; // MUPLUSNU
const boost::uint32_t DEFAULTINTERVAL=0;
const bool DEFAULTMAXIMIZE=false;

namespace po = boost::program_options;

bool parseCommandLine(int argc, char **argv,
					  std::string& program,
					  std::string& externalArguments,
					  std::size_t& popSize,
					  std::size_t& nParents,
					  boost::uint32_t& adaptionThreshold,
					  boost::uint16_t& nProducerThreads,
					  boost::uint16_t& nProcessingThreads,
					  boost::uint32_t& maxGenerations,
					  long& maxMinutes,
					  boost::uint32_t& reportGeneration,
					  recoScheme& rScheme,
					  boost::uint16_t& parallel,
					  bool& serverMode,
					  std::string& ip,
					  unsigned short& port,
					  double& sigma,
					  double& sigmaSigma,
					  double& minSigma,
					  double& maxSigma,
					  boost::uint32_t& nEvaluations,
					  Gem::GenEvA::dataExchangeMode& exchangeMode,
					  bool& sortingScheme,
					  boost::uint32_t& interval,
					  bool& maximize,
					  bool& verbose);

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GCOMMANDLINEPARSER_HPP_ */
