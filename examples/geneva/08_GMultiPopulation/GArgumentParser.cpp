/**
 * @file GArgumentParser.cpp
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

#include "GArgumentParser.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * Parses the command line
 */
bool parseCommandLine(
     int argc, char **argv
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
) {
   // Create the parser builder
   Gem::Common::GParserBuilder gpb;

   gpb.registerCLParameter<boost::uint16_t>(
         "nProducerThreads"
         , nProducerThreads
         , DEFAULTNPRODUCERTHREADS
         , "The amount of random number producer threads"
   );

   gpb.registerCLParameter<boost::uint16_t>(
         "nEvaluationThreads"
         , nEvaluationThreads
         , DEFAULTNEVALUATIONTHREADS
         , "The amount of threads processing individuals simultaneously"
   );

   gpb.registerCLParameter<std::size_t>(
         "populationSizeSuper"
         , populationSizeSuper
         , DEFAULTPOPULATIONSIZESUPER
         , "The desired size of the super population"
   );

   gpb.registerCLParameter<std::size_t>(
         "nParentsSuper"
         , nParentsSuper
         , DEFAULTNPARENTSSUPER
         , "The number of parents in the super population"
   );

   gpb.registerCLParameter<boost::uint32_t>(
         "maxIterationsSuper"
         , maxIterationsSuper
         , DEFAULTMAXITERATIONSSUPER
         , "Maximum number of iterations in the super population"
   );

   gpb.registerCLParameter<long>(
         "maxMinutesSuper"
         , maxMinutesSuper
         , DEFAULTMAXMINUTESSUPER
         , "The maximum number of minutes the optimization of the super population should run"
   );

   gpb.registerCLParameter<boost::uint32_t>(
         "reportIterationSuper"
         , reportIterationSuper
         , DEFAULTREPORTITERATIONSUPER
         , "The number of iterations after which information should be emitted in the super population"
   );

   gpb.registerCLParameter<duplicationScheme>(
         "rSchemeSuper"
         , rSchemeSuper
         , DEFAULTRSCHEMESUPER
         , "The recombination scheme of the evolutionary algorithm (super population)"
   );

   gpb.registerCLParameter<sortingModeMP>(
         "smodeSuper"
         , smodeSuper
         , DEFAULTSORTINGSCHEMESUPER
         , "Determines whether sorting is done in MUPLUSNU_SINGLEEVAL (0), MUCOMMANU_SINGLEEVAL (1)  or MUNU1PRETAIN (2) mode in the super population"
   );

   gpb.registerCLParameter<std::size_t>(
         "populationSizeSub"
         , populationSizeSub
         , DEFAULTPOPULATIONSIZESUB
         , "The desired size of the sub population"
   );

   gpb.registerCLParameter<std::size_t>(
         "nParentsSub"
         , nParentsSub
         , DEFAULTNPARENTSSUB
         , "The number of parents in the sub population"
   );

   gpb.registerCLParameter<boost::uint32_t>(
         "maxIterationsSub"
         , maxIterationsSub
         , DEFAULTMAXITERATIONSSUB
         , "Maximum number of iterations in the sub population"
   );

   gpb.registerCLParameter<long>(
         "maxMinutesSub"
         , maxMinutesSub
         , DEFAULTMAXMINUTESSUB
         , "The maximum number of minutes the optimization of the sub population should run"
   );

   gpb.registerCLParameter<boost::uint32_t>(
         "reportIterationSub"
         , reportIterationSub
         , DEFAULTREPORTITERATIONSUB
         , "The number of iterations after which information should be emitted in the sub population"
   );

   gpb.registerCLParameter<duplicationScheme>(
         "rSchemeSub"
         , rSchemeSub
         , DEFAULTRSCHEMESUB
         , "The recombination scheme of the evolutionary algorithm (sub population)"
   );

   gpb.registerCLParameter<sortingMode>(
         "smodeSub"
         , smodeSub
         , DEFAULTSORTINGSCHEMESUB
         , "Determines whether sorting is done in MUPLUSNU_SINGLEEVAL (0), MUCOMMANU_SINGLEEVAL (1)  or MUNU1PRETAIN (2) mode in the sub population"
   );


   // Parse the command line and leave if the help flag was given. The parser
   // will emit an appropriate help message by itself
   if(Gem::Common::GCL_HELP_REQUESTED == gpb.parseCommandLine(argc, argv, true /*verbose*/)) {
      return false; // Do not continue
   }

   return true;
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
