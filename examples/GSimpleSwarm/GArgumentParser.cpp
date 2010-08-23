/**
 * @file GArgumentParser.cpp
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


#include "GArgumentParser.hpp"

namespace Gem
{
  namespace Geneva
  {
    /************************************************************************************************/
    /**
     * A function that parses the command line for all required parameters
     */
    bool parseCommandLine (
    		 int argc
    	   , char **argv
		   , std::string& configFile
		   , boost::uint16_t& parallelizationMode
		   , bool& serverMode
		   , std::string& ip
		   , unsigned short& port
    ) {
      try{
		// Check the command line options. Uses the Boost program options library.
		po::options_description desc("Usage: evaluator [options]");
		desc.add_options()
		  ("help,h", "emit help message")
		  ("configFile,c", po::value<std::string>(&configFile)->default_value(DEFAULTCONFIGFILE),
		   "The name of the configuration file holding further configuration options")
		  ("parallelizationMode,p", po::value<boost::uint16_t>(&parallelizationMode)->default_value(DEFAULTPARALLELIZATIONMODE),
		   "Whether or not to run this optimization in serial mode (0), multi-threaded (1) or networked (2) mode")
		  ("serverMode,s","Whether to run networked execution in server or client mode. The option only gets evaluated if \"--parallelizationMode=2\"")
		  ("ip",po::value<std::string>(&ip)->default_value(DEFAULTIP), "The ip of the server")
		  ("port",po::value<unsigned short>(&port)->default_value(DEFAULTPORT), "The port of the server")
		  ;

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		// Emit a help message, if necessary
		if (vm.count("help")) {
		  std::cerr << desc << std::endl;
		  return false;
		}

		serverMode=false;
		if (vm.count("parallelizationMode")) {
		  if(parallelizationMode > 2) {
			std::cout << "Error: the \"-p\" or \"--parallelizationMode\" option may only assume the"<< std::endl
				  << "values 0 (serial), 1 (multi-threaded) or 2 (networked). Leaving ..." << std::endl;
			return false;
		  }

		  if(parallelizationMode == 2) if(vm.count("serverMode")) serverMode = true;
		}

		if(parallelizationMode != DEFAULTPARALLELIZATIONMODE ||  ip != DEFAULTIP  ||  port != DEFAULTPORT){
		  std::string parModeString;
		  switch(parallelizationMode) {
		  case 0:
			parModeString = "serial";
			break;
		  case 1:
			parModeString = "multi-threaded";
			break;
		  case 2:
			parModeString = "networked";
			break;
		  };

		  std::cout << std::endl
				<< "Running with the following command line options:" << std::endl
				<< "configFile = " << configFile << std::endl
				<< "parallelizationMode = " << parModeString << std::endl
				<< "serverMode = " << (serverMode?"true":"false") << std::endl
				<< "ip = " << ip << std::endl
				<< "port = " << port << std::endl
				<< std::endl;
		}
      }
      catch(...){
		std::cout << "Error parsing the command line" << std::endl;
		return false;
      }

      return true;

    }

    /************************************************************************************************/
    /**
     * A function that parses a config file for further parameters
     */
    bool parseConfigFile (
   		 const std::string& configFile
	   , boost::uint16_t& nProducerThreads
	   , boost::uint16_t& nEvaluationThreads
	   , std::size_t& nNeighborhoods
	   , std::size_t& nNeighborhoodMembers
	   , boost::uint32_t& maxIterations
	   , long& maxMinutes
	   , boost::uint32_t& reportIteration
	   , std::size_t& arraySize
	   , boost::uint32_t& processingCycles
	   , bool& returnRegardless
	   , boost::uint32_t& waitFactor
	   , std::size_t& parDim
	   , double& minVar
	   , double& maxVar
	   , demoFunction& df
	   , double& cLocal
	   , double& cGlobal
	   , double& cDelta
    ) {
		  boost::uint16_t evalFunction = 0;
		  bool verbose;

		  // Check the name of the configuation file
		  if(configFile.empty() || configFile == "empty" || configFile == "unknown") {
		std::cerr << "Error: Invalid configuration file name given: \"" << configFile << "\"" << std::endl;
		return false;
      }

      try{
		// Check the configuration file line options. Uses the Boost program options library.
		po::options_description config("Allowed options");
		config.add_options()
		  ("nProducerThreads",po::value<boost::uint16_t>(&nProducerThreads)->default_value(DEFAULTNPRODUCERTHREADS),
		   "The amount of random number producer threads")
		  ("nEvaluationThreads",po::value<boost::uint16_t>(&nEvaluationThreads)->default_value(DEFAULTNEVALUATIONTHREADS),
		   "The amount of threads processing individuals simultaneously")
		  ("nNeighborhoods", po::value<std::size_t>(&nNeighborhoods)->default_value(DEFAULTNNEIGHBORHOODSAP),
		   "The number of neighborhoods in the population")
		  ("nNeighborhoodMembers", po::value<std::size_t>(&nNeighborhoodMembers)->default_value(DEFAULTNNEIGHBORHOODMEMBERSAP),
		   "The default number of members in each neighborhood")
		  ("maxIterations", po::value<boost::uint32_t>(&maxIterations)->default_value(DEFAULTMAXITERATIONS),
		   "Maximum number of iterations in the population")
		  ("maxMinutes", po::value<long>(&maxMinutes)->default_value(DEFAULTMAXMINUTES),
		   "The maximum number of minutes the optimization of the population should run")
		  ("reportIteration",po::value<boost::uint32_t>(&reportIteration)->default_value(DEFAULTREPORTITERATION),
		   "The number of iterations after which information should be emitted in the super-population")
		  ("arraySize", po::value<std::size_t>(&arraySize)->default_value(DEFAULTARRAYSIZE),
		   "The size of the buffer with random arrays in the random factory")
		  ("verbose",po::value<bool>(&verbose)->default_value(DEFAULTVERBOSE),
		   "Whether additional information should be emitted")
		  ("processingCycles", po::value<boost::uint32_t>(&processingCycles)->default_value(DEFAULTPROCESSINGCYCLES),
		   "The maximum number of cycles a client should perform adaptions before it returns without success")
		  ("returnRegardless", po::value<bool>(&returnRegardless)->default_value(DEFAULTRETURNREGARDLESS),
		   "Specifies whether results should be returned even if they are not better than before")
		  ("waitFactor", po::value<boost::uint32_t>(&waitFactor)->default_value(DEFAULTGBTCWAITFACTOR),
		   "Influences the maximum waiting time of the GBrokerEA after the arrival of the first evaluated individuum")
		  ("parDim", po::value<std::size_t>(&parDim)->default_value(DEFAULTPARDIM),
		   "The amount of variables in the parabola")
		  ("minVar", po::value<double>(&minVar)->default_value(DEFAULTMINVAR),
		   "The lower boundary for all variables")
		  ("maxVar", po::value<double>(&maxVar)->default_value(DEFAULTMAXVAR),
		   "The upper boundary for all variables")
		  ("evalFunction",po::value<boost::uint16_t>(&evalFunction),
		   "The id of the evaluation function.")
		  ("cLocal", po::value<double>(&cLocal)->default_value(DEFAULTCLOCALAP),
		   "A constant to be multiplied with the local direction vector")
		  ("cGlobal", po::value<double>(&cGlobal)->default_value(DEFAULTCGLOBALAP),
		   "A constant to be multiplied with the local direction vector")
		  ("cDelta", po::value<double>(&cDelta)->default_value(DEFAULTCDELTAAP),
		   "A constant to be multiplied with the old velocity vector")
		  ;

		po::variables_map vm;
		std::ifstream ifs(configFile.c_str());
		if(!ifs.good()) {
			std::cerr << "Error accessing configuration file " << configFile;
			return false;
		}

		store(po::parse_config_file(ifs, config), vm);
		notify(vm);

		// Emit a help message, if necessary
		if (vm.count("help")) {
			std::cout << config << std::endl;
			return false;
		}

		// Assign the demo function
		if(evalFunction > (boost::uint16_t)MAXDEMOFUNCTION) {
			std::cout << "Error: Invalid evaluation function: " << evalFunction
					<< std::endl;
			return false;
		}
		df=(demoFunction)evalFunction;

	
		if(verbose){
			std::cout << std::endl
					<< "Running with the following options from " << configFile << ":" << std::endl
					<< "nProducerThreads = " << (boost::uint16_t)nProducerThreads << std::endl // boost::uint8_t not printable on gcc ???
					<< "nNeighborhoods = " << nNeighborhoods << std::endl
					<< "nNeighborhoodMembers = " << nNeighborhoodMembers << std::endl
					<< "maxIterations = " << maxIterations << std::endl
					<< "maxMinutes = " << maxMinutes << std::endl
					<< "reportIteration = " << reportIteration << std::endl
					<< "arraySize = " << arraySize << std::endl
					<< "processingCycles = " << processingCycles << std::endl
					<< "returnRegardless = " << (returnRegardless?"true":"false") << std::endl
					<< "waitFactor = " << waitFactor << std::endl
					<< "parDim = " << parDim << std::endl
					<< "minVar = " << minVar << std::endl
					<< "maxVar = " << maxVar << std::endl
					<< std::endl << "evalFunction = " << GFunctionIndividual<>::getStringRepresentation(df) << std::endl
					<< "cLocal = " << cLocal << std::endl
					<< "cGlobal = " << cGlobal << std::endl
					<< "cDelta = " << cDelta << std::endl
					<< std::endl;
		}
      }
      catch(...){
    	  std::cout << "Error parsing the configuration file " << configFile << std::endl;
    	  return false;
      }

      return true;
    }

    /************************************************************************************************/

  } /* namespace Geneva */
} /* namespace Gem */
