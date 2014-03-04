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
#include <common/GParserBuilder.hpp>
#include <geneva/GOptimizationEnums.hpp>

namespace Gem {
namespace Geneva {

using namespace Gem::Common;
namespace po = boost::program_options;

/******************************************************************************/
// Default settings
const boost::uint16_t DEFAULTNEVALUATIONTHREADS=4;
const std::size_t DEFAULTPOPULATIONSIZESUPER=5;
const std::size_t DEFAULTNPARENTSSUPER=1;
const boost::uint32_t DEFAULTMAXITERATIONSSUPER=10;
const long DEFAULTMAXMINUTESSUPER=0;
const boost::uint32_t DEFAULTREPORTITERATIONSUPER=1;
const sortingModeMP DEFAULTSORTINGSCHEMESUPER=MUPLUSNU_SINGLEEVAL_MP;
const duplicationScheme DEFAULTRSCHEMESUPER=VALUEDUPLICATIONSCHEME;
const std::size_t DEFAULTPOPULATIONSIZESUB=22;
const std::size_t DEFAULTNPARENTSSUB=2; // Allow to explore the parameter space from many starting points
const boost::uint32_t DEFAULTMAXITERATIONSSUB=100;
const long DEFAULTMAXMINUTESSUB=0;
const boost::uint32_t DEFAULTREPORTITERATIONSUB=0;
const sortingMode DEFAULTSORTINGSCHEMESUB=MUCOMMANU_SINGLEEVAL;
const duplicationScheme DEFAULTRSCHEMESUB=VALUEDUPLICATIONSCHEME;

/******************************************************************************/
/**
 * Parses the command line
 */
bool parseCommandLine(
     int argc, char** argv
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
);

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GARGUMENTPARSER_HPP_ */
