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
  bool parseCommandLine(
  		int argc, char **argv
		  , std::string& configFile
		  , boost::uint16_t& parallelizationMode
		  , bool& serverMode
		  , std::string& ip
		  , unsigned short& port
		  , Gem::Common::serializationMode& serMode
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
			  ("serMode", po::value<Gem::Common::serializationMode>(&serMode)->default_value(DEFAULTSERMODE),
			   "Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)")
		  ;

		  po::variables_map vm;
		  po::store(po::parse_command_line(argc, argv, desc), vm);
		  po::notify(vm);

		  // Emit a help message, if necessary
		  if (vm.count("help")) {
			  std::cerr << desc << std::endl;
			  return false;
		  }

		  if (vm.count("parallelizationMode")) {
			  if(parallelizationMode > 2) {
				  std::cout << "Error: the \"-p\" or \"--parallelizationMode\" option may only assume the"<< std::endl
						  << "values 0 (serial), 1 (multi-threaded) or 2 (mt-consumer). Leaving ..." << std::endl;
				  return false;
			  }
		  }

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
				  << "serMode = " << serMode << std::endl
				  << std::endl;
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
		  , std::size_t& populationSizeSuper
		  , std::size_t& nParentsSuper
		  , boost::uint32_t& maxIterationsSuper
		  , long& maxMinutesSuper
		  , boost::uint32_t& reportIterationSuper
		  , recoScheme& rSchemeSuper
		  , sortingMode& smodeSuper
		  , std::size_t& populationSizeSub
		  , std::size_t& nParentsSub
		  , boost::uint32_t& maxIterationsSub
		  , long& maxMinutesSub
		  , boost::uint32_t& reportIterationSub
		  , recoScheme& rSchemeSub
		  , sortingMode& smodeSub
		  , std::size_t& arraySize
		  , boost::uint32_t& processingCycles
		  , bool& returnRegardless
		  , boost::uint32_t& waitFactor
		  , bool& productionPlace
		  , double& adProb
		  , boost::uint32_t& adaptionThreshold
		  , double& sigma
		  , double& sigmaSigma
		  , double& minSigma
		  , double& maxSigma
		  , std::size_t& parDim
		  , double& minVar
		  , double& maxVar
		  , demoFunction& df
    ) {
      boost::uint16_t recombinationSchemeSuper=0;
      boost::uint16_t recombinationSchemeSub=0;
      boost::uint16_t evalFunction=0;
      bool verbose = true;

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
	  ("populationSizeSuper",po::value<std::size_t>(&populationSizeSuper)->default_value(DEFAULTPOPULATIONSIZESUPER),
	   "The size of the super-population")
	  ("nParentsSuper",po::value<std::size_t>(&nParentsSuper)->default_value(DEFAULTNPARENTSSUPER),
	   "The number of parents in the super-population")
	  ("maxIterationsSuper", po::value<boost::uint32_t>(&maxIterationsSuper)->default_value(DEFAULTMAXITERATIONSSUPER),
	   "Maximum number of iterations in the super-population")
	  ("maxMinutesSuper", po::value<long>(&maxMinutesSuper)->default_value(DEFAULTMAXMINUTESSUPER),
	   "The maximum number of minutes the optimization of the super-population should run")
	  ("reportIterationSuper",po::value<boost::uint32_t>(&reportIterationSuper)->default_value(DEFAULTREPORTITERATIONSUPER),
	   "The number of iterations after which information should be emitted in the super-population")
	  ("rSchemeSuper",po::value<boost::uint16_t>(&recombinationSchemeSuper)->default_value(DEFAULTRSCHEMESUPER),
	   "The recombination scheme for the super-population")
	  ("sortingSchemeSuper", po::value<sortingMode>(&smodeSuper)->default_value(DEFAULTSORTINGSCHEMESUPER),
	   "Determines whether sorting is done in MUCOMMANU (0), MUPLUSNU (1)  or MUNU1PRETAIN (2) mode in the super-population")
	  ("populationSizeSub",po::value<std::size_t>(&populationSizeSub)->default_value(DEFAULTPOPULATIONSIZESUB),
	   "The size of the sub-population")
	  ("nParentsSub",po::value<std::size_t>(&nParentsSub)->default_value(DEFAULTNPARENTSSUB),
	   "The number of parents in the sub-population")
	  ("maxIterationsSub", po::value<boost::uint32_t>(&maxIterationsSub)->default_value(DEFAULTMAXITERATIONSSUB),
	   "Maximum number of iterations in the sub-population")
	  ("maxMinutesSub", po::value<long>(&maxMinutesSub)->default_value(DEFAULTMAXMINUTESSUB),
	   "The maximum number of minutes the optimization of the sub-population should run")
	  ("reportIterationSub",po::value<boost::uint32_t>(&reportIterationSub)->default_value(DEFAULTREPORTITERATIONSUB),
	   "The number of iterations after which information should be emitted in the sub-population")
	  ("rSchemeSub",po::value<boost::uint16_t>(&recombinationSchemeSub)->default_value(DEFAULTRSCHEMESUB),
	   "The recombination scheme for the sub-population")
	  ("sortingSchemeSub", po::value<sortingMode>(&smodeSub)->default_value(DEFAULTSORTINGSCHEMESUB),
	   "Determines whether sorting is done in MUCOMMANU (0), MUPLUSNU (1)  or MUNU1PRETAIN (2) mode in the sub-population")
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
      ("productionPlace", po::value<bool>(&productionPlace)->default_value(DEFAULTPRODUCTIONPLACE),
		"Whether production of random numbers should happen locally (0) or in the random number factory (1)")
	  ("adProb", po::value<double>(&adProb)->default_value(DEFAULTGDAADPROB),
		"Specifies the likelihood for adaptions to be actually carried out")
	  ("adaptionThreshold", po::value<boost::uint32_t>(&adaptionThreshold)->default_value(DEFAULTADAPTIONTHRESHOLD),
		"Number of calls to adapt() after which adaption parameters should be modified")
	  ("sigma", po::value<double>(&sigma)->default_value(DEFAULTSIGMA),
		"The width of the gaussian used for the adaption of double values")
	  ("sigmaSigma", po::value<double>(&sigmaSigma)->default_value(DEFAULTSIGMASIGMA),
		"The adaption rate of sigma")
	  ("minSigma", po::value<double>(&minSigma)->default_value(DEFAULTMINSIGMA),
		"The minimum allowed value for sigma")
	  ("maxSigma", po::value<double>(&maxSigma)->default_value(DEFAULTMAXSIGMA),
		"The maximum allowed value for sigma")
	  ("parDim", po::value<std::size_t>(&parDim)->default_value(DEFAULTPARDIM),
	   "The amount of variables in the parabola")
	  ("minVar", po::value<double>(&minVar)->default_value(DEFAULTMINVAR),
	   "The lower boundary for all variables")
	  ("maxVar", po::value<double>(&maxVar)->default_value(DEFAULTMAXVAR),
	   "The upper boundary for all variables")
	  ("evalFunction", po::value<boost::uint16_t>(&evalFunction),
		"The id of the evaluation function. Allowed values: 0 (parabola), 1 (noisy parabola), 2 (rosenbrock)")
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
	if(2*nParentsSuper > populationSizeSuper){
	  std::cout << "Error: Invalid number of parents in super population" << std::endl
		    << "nParentsSuper       = " << nParentsSuper << std::endl
		    << "populationSizeSuper = " << populationSizeSuper << std::endl;

	  return false;
	}

	// Check the number of parents in the sub-population
	if(2*nParentsSub > populationSizeSub){
	  std::cout << "Error: Invalid number of parents in sub population" << std::endl
		    << "nParentsSub       = " << nParentsSub << std::endl
		    << "populationSizeSub = " << populationSizeSub << std::endl;

	  return false;
	}

	// Workaround for assigment problem with rSchemeSuper
	if(recombinationSchemeSuper==(boost::uint16_t)VALUERECOMBINE)
	  rSchemeSuper=VALUERECOMBINE;
	else if(recombinationSchemeSuper==(boost::uint16_t)RANDOMRECOMBINE)
	  rSchemeSuper=RANDOMRECOMBINE;
	else if(recombinationSchemeSuper==(boost::uint16_t)DEFAULTRECOMBINE)
	  rSchemeSuper=DEFAULTRECOMBINE;
	else {
	  std::cout << "Error: Invalid recombination scheme in super-population: " << recombinationSchemeSuper << std::endl;
	  return false;
	}

	// Workaround for assigment problem with rSchemeSub
	if(recombinationSchemeSub==(boost::uint16_t)VALUERECOMBINE)
	  rSchemeSub=VALUERECOMBINE;
	else if(recombinationSchemeSub==(boost::uint16_t)RANDOMRECOMBINE)
	  rSchemeSub=RANDOMRECOMBINE;
	else if(recombinationSchemeSub==(boost::uint16_t)DEFAULTRECOMBINE)
	  rSchemeSub=DEFAULTRECOMBINE;
	else {
	  std::cout << "Error: Invalid recombination scheme in sub-population: " << recombinationSchemeSub << std::endl;
	  return false;
	}

	// Assign the evaluation function
	if(evalFunction == (boost::uint16_t)PARABOLA) df=PARABOLA;
	else if(evalFunction == (boost::uint16_t)NOISYPARABOLA) df=NOISYPARABOLA;
	else if(evalFunction == (boost::uint16_t)ROSENBROCK) df=ROSENBROCK;
	else {
		std::cout << "Error: Invalid evaluation function: " << evalFunction << std::endl;
		return false;
	}

	if(verbose){
		std::string eF;
		switch(evalFunction) {
		case PARABOLA:
			eF = "PARABOLA";
			break;
		case NOISYPARABOLA:
			eF = "NOISYPARABOLA";
			break;
		case ROSENBROCK:
			eF = "ROSENBROCK";
			break;
		}

	  std::cout << std::endl
		    << "Running with the following options from " << configFile << ":" << std::endl
		    << "nProducerThreads = " << (boost::uint16_t)nProducerThreads << std::endl // boost::uint8_t not printable on gcc ???

		    << "populationSizeSuper = " << populationSizeSuper << std::endl
		    << "nParentsSuper = " << nParentsSuper << std::endl
		    << "maxIterationsSuper = " << maxIterationsSuper << std::endl
		    << "maxMinutesSuper = " << maxMinutesSuper << std::endl
		    << "reportIterationSuper = " << reportIterationSuper << std::endl
		    << "rSchemeSuper = " << (boost::uint16_t)rSchemeSuper << std::endl
		    << "sortingSchemeSuper = " << smodeSuper << std::endl
			<< "populationSizeSub = " << populationSizeSub << std::endl
			<< "nParentsSub = " << nParentsSub << std::endl
			<< "maxIterationsSub = " << maxIterationsSub << std::endl
			<< "maxMinutesSub = " << maxMinutesSub << std::endl
			<< "reportIterationSub = " << reportIterationSub << std::endl
			<< "rSchemeSub = " << (boost::uint16_t)rSchemeSub << std::endl
			<< "sortingSchemeSub = " << smodeSub << std::endl
		    << "arraySize = " << arraySize << std::endl
		    << "processingCycles = " << processingCycles << std::endl
			<< "returnRegardless = " << (returnRegardless?"true":"false") << std::endl
	        << "waitFactor = " << waitFactor << std::endl
			<< "productionPlace = " << (productionPlace?"factory":"locally") << std::endl
 		    << "adProb = " << adProb << std::endl
			<< "adaptionThreshold = " << adaptionThreshold << std::endl
		    << "sigma = " << sigma << std::endl
		    << "sigmaSigma " << sigmaSigma << std::endl
		    << "minSigma " << minSigma << std::endl
		    << "maxSigma " << maxSigma << std::endl
		    << "parDim = " << parDim << std::endl
		    << "minVar = " << minVar << std::endl
		    << "maxVar = " << maxVar << std::endl
		    << "evalFunction = " << eF << std::endl
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
