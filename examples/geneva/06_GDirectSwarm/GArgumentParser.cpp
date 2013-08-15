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
   , boost::uint16_t& parallelizationMode
   , bool& serverMode
   , std::string& ip
   , unsigned short& port
   , boost::uint32_t& maxStalls
   , boost::uint32_t& maxConnectionAttempts
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
   , boost::uint16_t& nProducerThreads
   , boost::uint16_t& nEvaluationThreads
   , boost::uint32_t& maxIterations
   , long& maxMinutes
   , boost::uint32_t& reportIteration
   , boost::uint16_t& xDim
   , boost::uint16_t& yDim
   , bool& followProgress
){
   // Create the parser builder
   Gem::Common::GParserBuilder gpb;

   gpb.registerCLParameter<boost::uint16_t>(
         "parallelizationMode,p"
         , parallelizationMode
         , DEFAULTPARALLELIZATIONMODEAP
         , "Whether to run the optimization in serial (0), multi-threaded (1) or networked (2) mode"
   );

   gpb.registerCLParameter<bool>(
         "serverMode,s"
         , serverMode
         , false // Use client mode, if no server option is specified
         , "Whether to run networked execution in server or client mode. The option only has an effect if \"--parallelizationMode=2\". You can either say \"--server=true\" or just \"--server\"."
         , GCL_IMPLICIT_ALLOWED // Permit implicit values, so that we can say --server instead of --server=true
         , true // Use server mode, of only -s or --server was specified
   );

   gpb.registerCLParameter<std::string>(
         std::string("ip")
         , ip
         , DEFAULTIP
         , "The ip of the server"
   );

   gpb.registerCLParameter<unsigned short>(
         "port"
         , port
         , DEFAULTPORT
         , "The port on the server"
   );

   gpb.registerCLParameter<boost::uint32_t>(
         "maxStalls"
         , maxStalls
         , DEFAULTMAXSTALLS06
         , "The number of stalled data transfers (i.e. transfers without a useful work item returned) before the client terminates in networked mode"
   );

   gpb.registerCLParameter<boost::uint32_t>(
         "maxConnectionAttempts"
         , maxConnectionAttempts
         , DEFAULTMAXCONNECTIONATTEMPTS06
         , "The number of connection attempts from client to server, before the client terminates in networked mode"
   );

   gpb.registerCLParameter<Gem::Common::serializationMode>(
         "serializationMode"
         , serMode
         , DEFAULTSERMODE
         , "Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)"
   );

   gpb.registerCLParameter<bool>(
         "addLocalConsumer"
         , addLocalConsumer
         , DEFAULTADDLOCALCONSUMER // Use client mode, if no server option is specified
         , "Whether or not a local consumer should be added to networked execution. You can use this option with or without arguments."
         , GCL_IMPLICIT_ALLOWED // Permit implicit values, so that we can say --server instead of --server=true
         , true // Use a local consumer if the option --addLocalConsumer was given without arguments
   );

   gpb.registerCLParameter<std::size_t>(
         "nNeighborhoods"
         , nNeighborhoods
         , DEFAULTNNEIGHBORHOODSAP
         , "The number of neighborhoods in the population"
   );

   gpb.registerCLParameter<std::size_t>(
         "nNeighborhoodMembers"
         , nNeighborhoodMembers
         , DEFAULTNNEIGHBORHOODMEMBERSAP
         , "The default number of members in each neighborhood"
   );

   gpb.registerCLParameter<double>(
         "cPersonal"
         , cPersonal
         , DEFAULTCPERSONALAP
         , "A constant to be multiplied with the personal direction vector"
   );

   gpb.registerCLParameter<double>(
         "cNeighborhood"
         , cNeighborhood
         , DEFAULTCNEIGHBORHOODAP
         , "A constant to be multiplied with the neighborhood direction vector"
   );

   gpb.registerCLParameter<double>(
         "cGlobal"
         , cGlobal
         , DEFAULTCGLOBALAP
         , "A constant to be multiplied with the global direction vector"
   );

   gpb.registerCLParameter<double>(
         "cVelocity"
         , cVelocity
         , DEFAULTCVELOCITYAP
         , "A constant to be multiplied with the old velocity vector"
   );

   gpb.registerCLParameter<updateRule>(
         "updateRule"
         , ur
         , DEFAULTUPDATERULE
         , "Use linear (0) or classical (1) update rule"
   );

   gpb.registerCLParameter<bool>(
         "allRandomInit"
         , allRandomInit
         , DEFAULTALLRANDOMINIT
         , "If set, all individuals will be initialized randomly. If 0, all individuals in one neighborhood will have the same start value"
         , GCL_IMPLICIT_ALLOWED // Permit implicit values, so that we can say --server instead of --server=true
         , true // Always randomly initialize if set
   );

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
         , "The amount of threads processing individuals simultaneously in multi-threaded mode"
   );

   gpb.registerCLParameter<boost::uint32_t>(
         "maxIterations"
         , maxIterations
         , DEFAULTMAXITERATIONS
         , "Maximum number of iterations in the optimization"
   );

   gpb.registerCLParameter<boost::uint32_t>(
         "reportIteration"
         , reportIteration
         , DEFAULTREPORTITERATION
         , "The number of iterations after which information should be emitted in the population"
   );

   gpb.registerCLParameter<long>(
         "maxMinutes"
         , maxMinutes
         , DEFAULTMAXMINUTES
         , "The maximum number of minutes the optimization of the population should run"
   );

   gpb.registerCLParameter<boost::uint16_t>(
         "xDim"
         , xDim
         , DEFAULTXDIMAP
         , "The x-dimension of the canvas for the result print(s)"
   );

   gpb.registerCLParameter<boost::uint16_t>(
         "yDim"
         , yDim
         , DEFAULTYDIMAP
         , "The y-dimension of the canvas for the result print(s)"
   );

   gpb.registerCLParameter<bool>(
         "followProgress"
         , followProgress
         , DEFAULTFOLLOWPROGRESS // Use client mode, if no server option is specified
         , "Specifies whether snapshots should be taken in regular intervals. You can use this option with or without arguments."
         , GCL_IMPLICIT_ALLOWED // Permit implicit values, so that we can say --server instead of --server=true
         , true // Take snapshots, if --followProgress was specified without arguments
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
