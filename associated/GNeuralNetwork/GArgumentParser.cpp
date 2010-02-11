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
  namespace GenEvA
  {
	/************************************************************************************************/
    /**
     * Parses the architecture string and returns a std::vector holding the architecture
     *
     * @param architecture The architecture string
     * @return A std::vector<std::size_t> holding the size of each layer
     */
  std::vector<std::size_t> parseArchitectureString(const std::string& architecture) {
	  using namespace boost;

	  std::vector<std::size_t> result;
	  tokenizer<> t(architecture);
	  tokenizer<>::const_iterator tit;

	  for(tit=t.begin(); tit!=t.end();++tit){
		  try {
			  std::size_t nNodes = lexical_cast<std::size_t>(*tit);
			  result.push_back(nNodes);
		  }
		  catch(...) {
			  std::ostringstream error;
			  error << "In parseArchitectureString(const std::string&):" << std::endl
					<< "Error parsing the architecture string " << architecture << std::endl
					<< "for token " << *tit << std::endl;
			  throw(Gem::GenEvA::geneva_error_condition(error.str()));
		  }
	  }

	  return result;
  }

    /************************************************************************************************/
    /**
     * A function that parses the command line for all required parameters
     */
    bool parseCommandLine(
  		  int argc, char **argv
		, std::string& configFile
		, boost::uint16_t& parallelizationMode
		, bool& serverMode
		, std::string& ip
		, unsigned short& port
		, trainingDataType& tdt
		, std::string& trainingDataFile
		, std::size_t& nDataSets
		, std::vector<std::size_t>& architecture
    ) {
      std::string tmpArchitecture;

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
	  ("trainingDataType", po::value<trainingDataType>(&tdt)->default_value(DEFAULTTRAININGDATATYPE),
	   "The type of training data to be produced: 0 (none), 1 (hyper cube), 2 (hyper sphere), 3 (axis centric)")
	  ("trainingDataFile", po::value<std::string>(&trainingDataFile)->default_value(DEFAULTTRAININGDATAFILE),
	   "The name of the output file for the creation of training data")
	  ("nDataSets", po::value<std::size_t>(&nDataSets)->default_value(DEFAULTNDATASETS),
	   "The number of data sets to create")
	  ("architecture", po::value<std::string>(&tmpArchitecture)->default_value(DEFAULTARCHITECTURE),
	   "The architecture of the neural network (1 input layer, 0-n hidden layers, 1 output layer")
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

	if(parallelizationMode != DEFAULTPARALLELIZATIONMODE ||  ip != DEFAULTIP  ||  port != DEFAULTPORT || tdt != DEFAULTTRAININGDATATYPE){
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

	  architecture = parseArchitectureString(tmpArchitecture);

	  std::cout << std::endl
		    << "Running with the following command line options:" << std::endl
		    << "configFile = " << configFile << std::endl
		    << "parallelizationMode = " << parModeString << std::endl
		    << "serverMode = " << (serverMode?"true":"false") << std::endl
		    << "ip = " << ip << std::endl
		    << "port = " << port << std::endl;

	  if(tdt != 0) {
		  std::cout << "trainingDataType = " << tdt << std::endl
				    << "trainingDataFile = " << trainingDataFile << std::endl
				    << "nDataSets = " << nDataSets << std::endl;

		  std::cout << "nNodes[input layer] = " << architecture[0] << std::endl;
		  for(std::size_t layerCounter = 1; layerCounter<architecture.size()-1; layerCounter++) {
			  std::cout << "nNodes[hidden layer " << layerCounter << "] = " << architecture[layerCounter] << std::endl;
		  }
		  std::cout << "nNodes[output layer] = " << architecture.back() << std::endl;
	  }

	  std::cout << std::endl;
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
    bool parseConfigFile(
  		  const std::string& configFile
			, boost::uint16_t& nProducerThreads
			, boost::uint16_t& nEvaluationThreads
			, std::size_t& populationSize
			, std::size_t& nParents
			, boost::uint32_t& maxIterations
			, long& maxMinutes
			, boost::uint32_t& reportIteration
			, recoScheme& rScheme
			, sortingMode& smode
			, std::size_t& arraySize
			, boost::uint32_t& processingCycles
			, bool& returnRegardless
			, boost::uint32_t& waitFactor
			, transferFunction& tF
			, std::string& trainingInputData
			, std::string& resultProgram
			, std::string& visualizationFile
			, const bool& verbose
	) {
      boost::uint16_t recombinationScheme=0;

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
	  ("populationSize",po::value<std::size_t>(&populationSize)->default_value(DEFAULTPOPULATIONSIZE),
	   "The size of the super-population")
	  ("nParents",po::value<std::size_t>(&nParents)->default_value(DEFAULTNPARENTS),
	   "The number of parents in the population") // Needs to be treated separately
	  ("maxIterations", po::value<boost::uint32_t>(&maxIterations)->default_value(DEFAULTMAXITERATIONS),
	   "Maximum number of iterations in the population")
	  ("maxMinutes", po::value<long>(&maxMinutes)->default_value(DEFAULTMAXMINUTES),
	   "The maximum number of minutes the optimization of the population should run")
	  ("reportIteration",po::value<boost::uint32_t>(&reportIteration)->default_value(DEFAULTREPORTITERATION),
	   "The number of iterations after which information should be emitted in the super-population")
	  ("rScheme",po::value<boost::uint16_t>(&recombinationScheme)->default_value(DEFAULTRSCHEME),
	   "The recombination scheme for the super-population")
	  ("sortingScheme,o", po::value<sortingMode>(&smode)->default_value(DEFAULTSORTINGSCHEME),
	   "Determines whether sorting is done in MUCOMMANU (0), MUPLUSNU (1)  or MUNU1PRETAIN (2) mode")
	  ("arraySize", po::value<std::size_t>(&arraySize)->default_value(DEFAULTARRAYSIZE),
	   "The size of the buffer with random arrays in the random factory")
	  ("processingCycles", po::value<boost::uint32_t>(&processingCycles)->default_value(DEFAULTPROCESSINGCYCLES),
	   "The maximum number of cycles a client should perform mutations before it returns without success")
	  ("returnRegardless", po::value<bool>(&returnRegardless)->default_value(DEFAULTRETURNREGARDLESS),
	   "Specifies whether results should be returned even if they are not better than before")
	  ("waitFactor", po::value<boost::uint32_t>(&waitFactor)->default_value(DEFAULTGBTCWAITFACTOR),
	   "Influences the maximum waiting time of the GBrokerEA after the arrival of the first evaluated individuum")
	  ("transferFunction", po::value<Gem::GenEvA::transferFunction>(&tF)->default_value(DEFAULTTRANSFERFUNCTION),
	   "The transfer function used in the network: 0 (SIGMOID), 1(RBF)")
	  ("trainingInputData", po::value<std::string>(&trainingInputData)->default_value(DEFAULTTRAININGINPUTDATA),
	   "The name of the file with the training data")
	  ("resultProgram", po::value<std::string>(&resultProgram)->default_value(DEFAULTRESULTPROGRAM),
	   "The name of the result program")
	  ("visualizationFile", po::value<std::string>(&visualizationFile)->default_value(DEFAULTVISUALIZATIONFILE),
	   "The name of the visualization file")
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

	// Check the number of parents in the super-population
	if(2*nParents > populationSize){
	  std::cout << "Error: Invalid number of parents inpopulation" << std::endl
		    << "nParents       = " << nParents << std::endl
		    << "populationSize = " << populationSize << std::endl;

	  return false;
	}

	// Workaround for assigment problem with rScheme
	if(recombinationScheme==(boost::uint16_t)VALUERECOMBINE)
	  rScheme=VALUERECOMBINE;
	else if(recombinationScheme==(boost::uint16_t)RANDOMRECOMBINE)
	  rScheme=RANDOMRECOMBINE;
	else if(recombinationScheme==(boost::uint16_t)DEFAULTRECOMBINE)
	  rScheme=DEFAULTRECOMBINE;
	else {
	  std::cout << "Error: Invalid recombination scheme in population: " << recombinationScheme << std::endl;
	  return false;
	}

	if(waitFactor == 0) waitFactor = DEFAULTGBTCWAITFACTOR;

	if(verbose){
	  std::cout << std::endl
		    << "Running with the following options from " << configFile << ":" << std::endl
		    << "nProducerThreads = " << (boost::uint16_t)nProducerThreads << std::endl // boost::uint8_t not printable on gcc ???
		    << "populationSize = " << populationSize << std::endl
		    << "nParents = " << nParents << std::endl
		    << "maxIterations = " << maxIterations << std::endl
		    << "maxMinutes = " << maxMinutes << std::endl
		    << "reportIteration = " << reportIteration << std::endl
		    << "rScheme = " << (boost::uint16_t)rScheme << std::endl
		    << "sortingScheme = " << smode << std::endl
		    << "arraySize = " << arraySize << std::endl
		    << "processingCycles = " << processingCycles << std::endl
		    << "returnRegardless = " << (returnRegardless?"true":"false") << std::endl
		    << "waitFactor = " << waitFactor << std::endl
		    << "transferFunction = " << tF << std::endl
		    << "trainingInputData = " << trainingInputData << std::endl
		    << "resultProgram = " << resultProgram << std::endl
		    << "visualizationFile = " << visualizationFile << std::endl
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

  } /* namespace GenEvA */
} /* namespace Gem */
