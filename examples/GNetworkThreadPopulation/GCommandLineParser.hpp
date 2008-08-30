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
#include "GBasePopulation.hpp"

namespace Gem
{
namespace GenEvA
{

// Default settings
const std::size_t DEFAULTPARABOLADIMENSION=1000;
const double DEFAULTPARABOLAMIN=-100.;
const double DEFAULTPARABOLAMAX=100.;
const boost::uint16_t DEFAULTNPRODUCERTHREADS=10;
const std::size_t DEFAULTNPOPTHREADS=4;
const std::size_t DEFAULTPOPULATIONSIZE=100;
const std::size_t DEFAULTNPARENTS=5; // Allow to explore the parameter space from many starting points
const boost::uint32_t DEFAULTMAXGENERATIONS=2000;
const long DEFAULTMAXMINUTES=10;
const boost::uint32_t DEFAULTREPORTGENERATION=1;
const recoScheme DEFAULTRSCHEME=VALUERECOMBINE;
const bool DEFAULTVERBOSE=true;

namespace po = boost::program_options;

bool parseCommandLine(int argc, char **argv,
					  std::size_t& parabolaDimension,
					  double& parabolaMin,
					  double& parabolaMax,
					  boost::uint16_t& nProducerThreads,
					  std::size_t& nPopThreads,
					  std::size_t& populationSize,
					  std::size_t& nParents,
					  boost::uint32_t& maxGenerations,
					  long& maxMinutes,
					  boost::uint32_t& reportGeneration,
					  recoScheme& rScheme,
					  bool& verbose);

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GCOMMANDLINEPARSER_HPP_ */
