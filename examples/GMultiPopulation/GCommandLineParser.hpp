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

#include <boost/cstdint.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
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
const std::size_t DEFAULTCONSUMERTHREADS=4;
const std::size_t DEFAULTNSUPERTHREADS=4;
const std::size_t DEFAULTSUPERPOPULATIONSIZE=20;
const std::size_t DEFAULTSUBPOPULATIONSIZE=10;
const std::size_t DEFAULTSUPERNPARENTS=5; // Allow to explore the parameter space from many starting points
const std::size_t DEFAULTSUBNPARENTS=1;
const boost::uint32_t DEFAULTSUPERMAXGENERATIONS=20;
const boost::uint32_t DEFAULTSUPERMAXGENERATIONS=100;
const long DEFAULTSUPERMAXMINUTES=10;
const long DEFAULTSUBMAXMINUTES=0;
const boost::uint32_t DEFAULTSUPERREPORTGENERATION=1;
const boost::uint32_t DEFAULTSUBREPORTGENERATION=0; // no reporting
const recoScheme DEFAULTSUPERRSCHEME=RANDOMRECOMBINE;
const recoScheme DEFAULTSUBRSCHEME=VALUERECOMBINE;
const bool DEFAULTVERBOSE=true;

namespace po = boost::program_options;

bool parseCommandLine(int argc, char **argv,
					  std::size_t& parabolaDimension,
					  double& parabolaMin,
					  double& parabolaMax,
					  boost::uint16_t& nProducerThreads,
					  std::size_t& nConsumerThreads,
					  std::site_t& nSuperThreads,
					  std::size_t& superPopulationSize,
					  std::size_t& superNParents,
					  std::size_t& subPopulationSize,
					  std::size_t& subNParents,
					  boost::uint32_t& superMaxGenerations,
					  boost::uint32_t& subMaxGenerations,
					  long& superMaxMinutes,
					  long& subMaxMinutes,
					  boost::uint32_t& superReportGeneration,
					  boost::uint32_t& subReportGeneration,
					  recoScheme& superRScheme,
					  recoScheme& subRScheme,
					  bool& verbose);

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GCOMMANDLINEPARSER_HPP_ */
