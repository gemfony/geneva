/**
 * @file GArgumentParser.cpp
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

#include "GArgumentParser.hpp"

namespace Gem {
namespace Common {
namespace Tests {

/************************************************************************************************/
/**
 * A function that parses the command line for all required parameters
 */
bool parseCommandLine(
		int argc
	  , char **argv
	  , std::string& resultFile
	  , std::size_t& nProducers
	  , std::size_t& nItems
	  , std::size_t& nConsumers
	  , long& timeoutMS
	  , long& maxRandomDelayMS
	  , long& startDelayMS
	  , bool& startAtOnce
) {
  try{
	// Check the command line options. Uses the Boost program options library.
	po::options_description desc("Usage: program [options]");
	desc.add_options()
	  ("help,h", "emit help message")
	  ("resultFile,r", po::value<std::string>(&resultFile)->default_value(DEFAULTRESULTFILEAP),
	   "The name of the file holding the test results")
	  ("nProducers,p", po::value<std::size_t>(&nProducers)->default_value(DEFAULTNPRODUCERSAP),
	   "The number of producers of items")
	  ("nItems,i", po::value<std::size_t>(&nItems)->default_value(DEFAULTNITEMS),
	   "The number of items to be created by each producer")
	  ("nConsumers,c", po::value<std::size_t>(&nConsumers)->default_value(DEFAULTNCONSUMERSAP),
	   "The number of consumers of items")
	  ("timeoutMS,t", po::value<long>(&timeoutMS)->default_value(DEFAULTTIMEOUTMS),
	   "The duration of the timeout in microseconds")
	  ("maxRandomDelayMS,m", po::value<long>(&maxRandomDelayMS)->default_value(DEFAULTMAXRANDOMDELAYMS),
	   "The maximum size of random delays in microseconds")
	  ("startDelayMS,s", po::value<long>(&startDelayMS)->default_value(DEFAULTSTARTDELAY),
	   "A delay between the start of the producer and the consumer")
	  ("startAtOnce,o", po::value<bool>(&startAtOnce)->default_value(DEFAULTSTARTATONCE),
	   "Indicates whether all threads should be started at once (if false, each producers and consumers will start simultaneously)")
	  ;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	// Emit a help message, if necessary
	if (vm.count("help")) {
		std::cerr << desc << std::endl;
		return false;
	}

	std::cout << std::endl
			<< "Running with the following command line options:" << std::endl
			<< "resultFile = " << resultFile << std::endl
			<< "nProducers = " << nProducers << std::endl
			<< "nItems = " << nItems << std::endl
			<< "nConsumers = " << nConsumers << std::endl
			<< "timeoutMS = " << timeoutMS << std::endl
			<< "maxRandomDelayMS = " << maxRandomDelayMS << std::endl
			<< "startDelayMS = " << startDelayMS << std::endl
			<< "startAtOnce = " << startAtOnce << std::endl
			<< std::endl;
  }
  catch(...){
	  std::cout << "Error parsing the command line" << std::endl;
	  return false;
  }

  return true;
}

/************************************************************************************************/

} /* namespace Tests */
} /* namespace Common */
} /* namespace Gem */
