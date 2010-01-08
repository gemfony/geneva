/**
 * @file GArgumentParser.hpp
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

#ifndef GARGUMENTPARSER_HPP_
#define GARGUMENTPARSER_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA headers go here
#include "GEnums.hpp"
#include "GSerializationHelperFunctions.hpp"

namespace Gem
{
  namespace GenEvA
  {
    // Default settings
    const boost::uint16_t DEFAULTNPRODUCERTHREADS=10;
    const boost::uint16_t DEFAULTNEVALUATIONTHREADS=4;
    const boost::uint32_t DEFAULTMAXGENERATIONS=2000;
    const long DEFAULTMAXMINUTES=10;
    const boost::uint32_t DEFAULTREPORTGENERATION=1;
    const bool DEFAULTVERBOSE=true;
    const bool DEFAULTPARALLELIZATIONMODE=1;
    const std::size_t DEFAULTARRAYSIZE=1000;
    const bool DEFAULTPRODUCTIONPLACE=true; // local production
    const bool DEFAULTUSECOMMONADAPTOR=false; // whether to use a common adaptor for all GParameterT objects
    const unsigned short DEFAULTPORT=10000;
    const std::string DEFAULTIP="localhost";
    const std::string DEFAULTCONFIGFILE="./GSimpleSwarm.cfg";
    const boost::uint32_t DEFAULTSTARTGENERATION=0;
    const boost::uint32_t DEFAULTPROCESSINGCYCLES=1;
    const bool DEFAULTRETURNREGARDLESS=true;
    const std::size_t DEFAULTNBTCONSUMERTHREADS=2;
    const boost::uint32_t DEFAULTGBTCWAITFACTOR=5;
    const std::size_t DEFAULTPARDIM=100;
    const double DEFAULTMINVAR=-10.;
    const double DEFAULTMAXVAR=10.;
    const std::size_t DEFAULTNNEIGHBORHOODS=5;
    const std::size_t DEFAULTNNEIGHBORHOODMEMBERS=20;

    namespace po = boost::program_options;

    bool parseCommandLine(int argc, char **argv,
			  std::string& configFile,			  
			  boost::uint16_t& parallelizationMode,
			  bool& serverMode,
			  std::string& ip,
			  unsigned short& port);

    bool parseConfigFile(const std::string& configFile,
			 boost::uint16_t& nProducerThreads,
			 boost::uint16_t& nEvaluationThreads,
			 std::size_t& nNeighborhoods,
			 std::size_t& nNeighborhoodMembers,
			 boost::uint32_t& maxGenerations,
			 long& maxMinutes,
			 boost::uint32_t& reportGeneration,
			 std::size_t& arraySize,
			 boost::uint32_t& processingCycles,
			 bool& returnRegardless,
			 boost::uint32_t& waitFactor,
			 std::size_t& parDim,
			 double& minVar,
			 double& maxVar);

  } /* namespace GenEvA */
} /* namespace Gem */

#endif /* GARGUMENTPARSER_HPP_ */
