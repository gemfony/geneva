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

// Some defines for the individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

namespace Gem
{
  namespace Geneva
  {
    // Default settings
    const boost::uint16_t DEFAULTNPRODUCERTHREADS=10;
    const boost::uint16_t DEFAULTNEVALUATIONTHREADS=4;
    const std::size_t DEFAULTPOPULATIONSIZESUPER=10;
    const std::size_t DEFAULTNPARENTSSUPER=1;
    const boost::uint32_t DEFAULTMAXITERATIONSSUPER=20;
    const long DEFAULTMAXMINUTESSUPER=0;
    const boost::uint32_t DEFAULTREPORTITERATIONSUPER=1;
    const sortingModeMP DEFAULTSORTINGSCHEMESUPER=MUPLUSNU_SINGLEEVAL_MP;
    const duplicationScheme DEFAULTRSCHEMESUPER=VALUEDUPLICATIONSCHEME;
    const std::size_t DEFAULTPOPULATIONSIZESUB=100;
    const std::size_t DEFAULTNPARENTSSUB=5; // Allow to explore the parameter space from many starting points
    const boost::uint32_t DEFAULTMAXITERATIONSSUB=100;
    const long DEFAULTMAXMINUTESSUB=0;
    const boost::uint32_t DEFAULTREPORTITERATIONSUB=0;
    const sortingMode DEFAULTSORTINGSCHEMESUB=MUPLUSNU_SINGLEEVAL;
    const duplicationScheme DEFAULTRSCHEMESUB=VALUEDUPLICATIONSCHEME;
    const bool DEFAULTVERBOSE=true;
    const bool DEFAULTPARALLELIZATIONMODEAP=1;
    const bool DEFAULTUSECOMMONADAPTOR=false; // whether to use a common adaptor for all GParameterT objects
    const unsigned short DEFAULTPORT=10000;
    const std::string DEFAULTIP="localhost";
    const std::string DEFAULTCONFIGFILE="./GMultiPopulation.cfg";
    const boost::uint32_t DEFAULTSTARTITERATION=0;
    const std::size_t DEFAULTNBTCONSUMERTHREADS=2;
    const boost::uint32_t DEFAULTGBTCNPROCUNITS=1;
    const std::size_t DEFAULTPARDIM=100;
    const double DEFAULTMINVAR=-10.;
    const double DEFAULTMAXVAR=10.;
    const boost::uint16_t DEFAULTEVALFUNCTION=0;
    const boost::uint32_t DEFAULTADAPTIONTHRESHOLDAP=1;
    const double DEFAULTGDAADPROB=1.0;
    const bool DEFAULTRETURNREGARDLESS=true;
    const Gem::Common::serializationMode DEFAULTSERMODE=Gem::Common::SERIALIZATIONMODE_TEXT;

    namespace po = boost::program_options;

    bool parseCommandLine(
    		int argc, char **argv
		  , std::string& configFile
	);

    bool parseConfigFile(
    		const std::string& configFile
    	  , boost::uint16_t& nProducerThreads
		  , boost::uint16_t& nEvaluationThreads
		  , std::size_t& populationSizeSuper
		  , std::size_t& nParentsSuper
		  , boost::uint32_t& maxIterationsSuper
		  , long& maxMinutesSuper
		  , boost::uint32_t& reportIterationSuper
		  , duplicationScheme& rSchemeSuper
		  , sortingModeMP& smodeSuper
		  , std::size_t& populationSizeSub
		  , std::size_t& nParentsSub
		  , boost::uint32_t& maxIterationsSub
		  , long& maxMinutesSub
		  , boost::uint32_t& reportIterationSub
		  , duplicationScheme& rSchemeSub
		  , sortingMode& smodeSub
		  , bool& returnRegardless
		  , boost::uint32_t& nProcessingUnits
		  , double& adProb
		  , boost::uint32_t& adaptionThreshold
		  , double& sigma
		  , double& sigmaSigma
		  , double& minSigma
		  , double& maxSigma
		  , std::size_t& parDim
		  , double& minVar
		  , double& maxVar
		  , solverFunction& df
	);

  } /* namespace Geneva */
} /* namespace Gem */

#endif /* GARGUMENTPARSER_HPP_ */
