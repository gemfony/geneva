/**
 * @file GOptimizer.cpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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

#include "geneva/GOptimizer.hpp"

namespace Gem {
namespace Geneva {

/**************************************************************************************/
/**
 * A constructor that first parses the command line for relevant parameters and then
 * loads data from a config file
 *
 * @param argc The number of command line arguments
 * @param argv An array with the arguments
 */
GOptimizer::GOptimizer(int argc, char **argv) {

}

/**************************************************************************************/
/**
 * The standard constructor. Loads the data from the configuration file
 *
 * @param ip Specifies under which address the server can be reached
 * @param port Specifies on which port the server answers
 * @param configFilename The name of the configuration file
 */
GOptimizer::GOptimizer(
		const personality& pers
		, const parMode& pm
		, const std::string& ip
		, const unsigned int& port
		, const std::string& configFilename
		, const bool& verbose
)
	: pers_(pers)
	, parMode_(pm)
	, ip_(ip)
	, port_(port)
	, configFilename_(configFilename)
	, verbose_(verbose)
{
	//--------------------------------------------
	// Load further configuration options from file
	parseConfigurationFile(configFilename_);

	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Set the number of threads. GRANDOMFACTORY is
	// a singleton that will be initialized by this call.
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads_);
	GRANDOMFACTORY->setArraySize(arraySize_);
}

/**************************************************************************************/
/**
 * Allows to register a function object that performs necessary initialization work.
 * It will be called both for servers (and serial/multithreaded execution) and for
 * clients. If you want a separate init function for the clients, register one with
 * registerClientInitFunction() and it will be called instead of the one registered
 * with this function.
 *
 * @param initFunction A function object performing necessary initialization work
 */
void GOptimizer::registerInitFunction(boost::function<void ()> initFunction) {
	// Do some error checking
	if(!initFunction) {
		std::ostringstream error;
		error << "In GOptimizer::registerInitFunction(): Error!" << std::endl
			  << "Empty function object provided." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	initFunction_ = initFunction;
}

/**************************************************************************************/
/**
 * Allows to register a function object that performs necessary initialization work
 * for networked clients. Note that, if no init function has been registered specifically
 * for the client, but one has been created using registerInitFunction(), then that
 * one will be called for the client.
 *
 * @param clientInitFunction A function object performing necessary initialization work for clients
 */
void GOptimizer::registerClientInitFunction(boost::function<void ()> clientInitFunction) {
	// Do some error checking
	if(!clientInitFunction) {
		std::ostringstream error;
		error << "In GOptimizer::registerClientInitFunction(): Error!" << std::endl
			  << "Empty function object provided." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	clientInitFunction_ = clientInitFunction;
}

/**************************************************************************************/
/**
 * Allows to register a function object that performs necessary finalization work.
 * It will be called both for servers (and serial/multithreaded execution) and for
 * clients. If you want a separate finalization function for the clients, register
 * one with registerClientFinalizationFunction() and it will be called instead of
 * the one registered with this function.
 *
 * @param finalizationFunction A function object performing necessary finalization work
 */
void GOptimizer::registerFinalizationFunction(boost::function<void ()> finalizationFunction) {
	// Do some error checking
	if(!finalizationFunction) {
		std::ostringstream error;
		error << "In GOptimizer::registerFinalizationFunction(): Error!" << std::endl
			  << "Empty function object provided." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	finalizationFunction_ = finalizationFunction_;
}

/**************************************************************************************/
/**
 * Allows to register a function object that performs necessary finalization work
 * for networked clients. Note that, if no finalization function has been registered specifically
 * for the client, but one has been created using registerFinalizationFunction(), then that
 * one will be called for the client.
 *
 * @param clientFinalizationFunction A function object performing necessary finalization work for clients
 */
void GOptimizer::registerClientFinalizationFunction(boost::function<void ()> clientFinalizationFunction) {
	// Do some error checking
	if(!clientFinalizationFunction) {
		std::ostringstream error;
		error << "In GOptimizer::registerClientFinalizationFunction(): Error!" << std::endl
			  << "Empty function object provided." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	clientFinalizationFunction_ = clientFinalizationFunction;
}

/**************************************************************************************/
/**
 * Allows to add individuals to the class. These will later be used to initialize the
 * optimization algorithms. Note that the class will take ownership of the objects by
 * cloning them. Note that we make no attempt to check whether the individuals registered
 * through this function are of the same type.
 *
 * @param ps_ptr A pointer to an individual, used to initialize other individuals
 */
void GOptimizer::registerParameterSet(boost::shared_ptr<GParameterSet> ps_ptr) {
	if(!ps_ptr) {
		std::ostringstream error;
		error << "In GOptimizer::registerParameterSet(): Error!" << std::endl
			  << "Empty pointer found." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	initialParameterSets_.push_back(ps_ptr->clone<GParameterSet>());
}

/**************************************************************************************/
/**
 * Allows to add a set of individuals to the class. These will later be used to initialize the
 * optimization algorithms. Note that the class will take ownership of the objects by
 * cloning them. Note that we make no attempt to check whether the individuals registered
 * through this function are of the same type.
 *
 * @param ps_ptr A collection of GParameterSet derivatives
 */
void GOptimizer::registerParameterSet(const std::vector<boost::shared_ptr<GParameterSet> >& ps_collection) {
	if(ps_collection.empty()) {
		std::ostringstream error;
		error << "In GOptimizer::registerParameterSet(std::vector<>): Error!" << std::endl
			  << "Found empty collection." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	for(std::size_t i=0; i<ps_collection.size(); i++) {
		if(!ps_collection[i]) {
			std::ostringstream error;
			error << "In GOptimizer::registerParameterSet(std::vector<>): Error!" << std::endl
				  << "Empty pointer found." << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		initialParameterSets_.push_back(ps_collection[i]->clone<GParameterSet>());
	}
}

/**************************************************************************************/
/**
 * Allows to specify an optimization monitor to be used with evolutionary algorithms.
 * Note that this function will take ownership of the monitor by cloning it.
 *
 * @param ea_om_ptr A pointer to an optimization monitor specific for evolutionary algorithms
 */
void GOptimizer::registerOptimizationMonitor(boost::shared_ptr<GEvolutionaryAlgorithm::GEAOptimizationMonitor> ea_om_ptr) {
	if(!ea_om_ptr) {
		std::ostringstream error;
		error << "In GOptimizer::registerOptimizationMonitor(): Error!" << std::endl
			  << "Empty optimization monitor pointer found for EA" << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	ea_om_ptr_ = ea_om_ptr->clone<GEvolutionaryAlgorithm::GEAOptimizationMonitor>();
}

/**************************************************************************************/
/**
 * Allows to specify an optimization monitor to be used with swarm algorithms.
 * Note that this function will take ownership of the monitor by cloning it.
 *
 * @param swarm_om_ptr A pointer to an optimization monitor specific for swarm algorithms
 */
void GOptimizer::registerOptimizationMonitor(boost::shared_ptr<GSwarm::GSwarmOptimizationMonitor> swarm_om_ptr) {
	if(!swarm_om_ptr) {
		std::ostringstream error;
		error << "In GOptimizer::registerOptimizationMonitor(): Error!" << std::endl
			  << "Empty optimization monitor pointer found for SWARM" << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	swarm_om_ptr_ = swarm_om_ptr->clone<GSwarm::GSwarmOptimizationMonitor>();
}

/**************************************************************************************/
/**
 * Allows to specify an optimization monitor to be used with gradient descents.
 * Note that this function will take ownership of the monitor by cloning it.
 *
 * @param gd_om_ptr A pointer to an optimization monitor specific for gradient descents
 */
void GOptimizer::registerOptimizationMonitor(boost::shared_ptr<GGradientDescent::GGDOptimizationMonitor> gd_om_ptr) {
	if(!gd_om_ptr) {
		std::ostringstream error;
		error << "In GOptimizer::registerOptimizationMonitor(): Error!" << std::endl
			  << "Empty optimization monitor pointer found for GD" << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	gd_om_ptr_ = gd_om_ptr->clone<GGradientDescent::GGDOptimizationMonitor>();
}

/**************************************************************************************/
/**
 * Triggers execution of the client loop
 */
void GOptimizer::clientRun() {
	// If initialization functions have been provided, call them as the first action.
	// We first check if a specific initialization function for clients has been
	// provided. If this is not the case, we try to execute the global initialization
	// function instead.
	if(clientInitFunction_) clientInitFunction_();
	else {
		if(initFunction_) initFunction_();
	}

	// Instantiate the client worker
    boost::shared_ptr<Gem::Courtier::GAsioTCPClientT<GIndividual> > p(new Gem::Courtier::GAsioTCPClientT<GIndividual>(ip_, boost::lexical_cast<std::string>(port_)));

    p->setMaxStalls(maxStalledDataTransfers_); // Set to 0 to allow an infinite number of stalls
    p->setMaxConnectionAttempts(maxConnectionAttempts_); // Set to 0 to allow an infinite number of failed connection attempts
    p->returnResultIfUnsuccessful(returnRegardless_);  // Prevent return of unsuccessful adaption attempts to the server

    // Start the actual processing loop
    p->run();

	// If finalization functions have been provided, call them as the last action.
	// We first check if a specific finalization function for clients has been
	// provided. If this is not the case, we try to execute the global finalization
	// function instead.
	if(clientFinalizationFunction_) clientFinalizationFunction_();
	else {
		if(finalizationFunction_) finalizationFunction_();
	}
}

/**************************************************************************************/
/**
 * Loads some configuration data from arguments passed on the command line
 *
 * @param argc The number of command line arguments
 * @param argv An array with the arguments
 */
void GOptimizer::parseCommandLine(int argc, char **argv) {
    namespace po = boost::program_options;

	try {
		std::string usageString = std::string("Usage: ") + argv[0] + " [options]";
		po::options_description desc(usageString.c_str());
		desc.add_options()
				("help,h", "emit help message")
				("optimizationConfig,o", po::value<std::string>(&configFilename_)->default_value(GO_DEF_DEFAULTCONFIGFILE),
				"The name of the file holding configuration information for optimization algorithms")
				("parallelizationMode,p", po::value<parMode>(&parMode_)->default_value(GO_DEF_DEFAULPARALLELIZATIONMODE),
				"Whether to perform the optimization in serial mode (0), multi-threaded (1) or networked (2) mode")
				("serverMode,s",
				"Whether to run networked execution in server or client mode. The option only gets evaluated if \"--parallelizationMode=2\"")(
				"ip", po::value<std::string>(&ip)->default_value(DEFAULTIP),
				"The ip of the server")("port",
				po::value<unsigned short>(&port)->default_value(DEFAULTPORT),
				"The port of the server")(
				"serMode",
				po::value<Gem::Common::serializationMode>(&serMode)->default_value(
						DEFAULTSERMODE),
				"Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)");

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);
	}
	catch(const po::error& e) {
		std::cerr << "Error parsing the command line:" << std::endl
				  << e.what() << std::endl;
		exit(1);
	}
	catch(...) {
		std::cerr << "Unknown error while parsing the command line" << configFile << std::endl;
		exit(1);
	}
}

/**************************************************************************************/
/**
 * Loads the configuration data from a given configuration file
 */
void GOptimizer::parseConfigurationFile(const std::string& configFile) {
	namespace bf = boost::filesystem;
    namespace po = boost::program_options;

	// Check the name of the configuration file
	if (!bf::exists(configFile)) {
		std::ostringstream error;
		error << "In GOptimizer::parseConfigurationFile(): Error!" << std::endl
			  << "Invalid file name given for configuration file: \"" << configFile << "\"" << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	try {
		po::options_description config(configFile.c_str());
		config.add_options()
		("maxStalledDataTransfers", po::value<boost::uint32_t>(&maxStalledDataTransfers_)->default_value(GO_DEF_MAXSTALLED))
		("maxConnectionAttempts", po::value<boost::uint32_t>(&maxConnectionAttempts_)->default_value(GO_DEF_MAXCONNATT))
		("returnRegardless", po::value<bool>(&returnRegardless_)->default_value(GO_DEF_RETURNREGARDLESS))
		("nProducerThreads", po::value<boost::uint16_t>(&nProducerThreads_)->default_value(GO_DEF_NPRODUCERTHREADS))
		("arraySize", po::value<std::size_t>(&arraySize_)->default_value(GO_DEF_ARRAYSIZE))
		("nEvaluationThreads", po::value<boost::uint16_t>(&nEvaluationThreads_)->default_value(GO_DEF_NEVALUATIONTHREADS))
		("serializationMode", po::value<Gem::Common::serializationMode>(&serializationMode_)->default_value(GO_DEF_SERIALIZATIONMODE))
		("waitFactor", po::value<boost::uint32_t>(&waitFactor_)->default_value(GO_DEF_WAITFACTOR))
		("maxIterations", po::value<boost::uint32_t>(&maxIterations_)->default_value(GO_DEF_MAXITERATIONS))
		("maxMinutes", po::value<long>(&maxMinutes_)->default_value(GO_DEF_MAXMINUTES))
		("reportIteration", po::value<boost::uint32_t>(&reportIteration_)->default_value(GO_DEF_REPORTITERATION))
		("eaPopulationSize", po::value<std::size_t>(&eaPopulationSize_)->default_value(GO_DEF_EAPOPULATIONSIZE))
		("eaNParents", po::value<std::size_t>(&eaNParents_)->default_value(GO_DEF_EANPARENTS))
		("eaRecombinationScheme", po::value<recoScheme>(&eaRecombinationScheme_)->default_value(GO_DEF_EARECOMBINATIONSCHEME))
		("eaSortingScheme", po::value<sortingMode>(&eaSortingScheme_)->default_value(GO_DEF_EASORTINGSCHEME))
		("eaTrackParentRelations", po::value<bool>(&eaTrackParentRelations_)->default_value(GO_DEF_EATRACKPARENTRELATIONS))
		("swarmNNeighborhoods", po::value<std::size_t>(&swarmNNeighborhoods_)->default_value(GO_DEF_SWARMNNEIGHBORHOODS))
		("swarmNNeighborhoodMembers", po::value<std::size_t>(&swarmNNeighborhoodMembers_)->default_value(GO_DEF_SWARMNNEIGHBORHOODMEMBERS))
		("swarmRandomFillUp", po::value<bool>(&swarmRandomFillUp_)->default_value(GO_DEF_SWARMRANDOMFILLUP))
		("swarmCLocal", po::value<float>(&swarmCLocal_)->default_value(GO_DEF_SWARMCLOCAL))
		("swarmCGlobal", po::value<float>(&swarmCGlobal_)->default_value(GO_DEF_SWARMCCGLOBAL))
		("swarmCDelta", po::value<float>(&swarmCDelta_)->default_value(GO_DEF_SWARMCCDELTA))
		("swarmUpdateRule", po::value<updateRule>(&swarmUpdateRule_)->default_value(GO_DEF_SWARMUPDATERULE))
		("gdNStartingPoints", po::value<std::size_t>(&gdNStartingPoints_)->default_value(GO_DEF_GDNSTARTINGPOINTS))
		("gdFiniteStep", po::value<float>(&gdFiniteStep_)->default_value(GO_DEF_GDFINITESTEP))
		("gdStepSize", po::value<float>(&gdStepSize_)->default_value(GO_DEF_GDSTEPSIZE))
		;

		po::variables_map vm;
		std::ifstream ifs(configFile.c_str());
		if (!ifs.good()) {
			std::cerr << "Error accessing configuration file " << configFile;
			exit(1);
		}

		store(po::parse_config_file(ifs, config), vm);
		notify(vm);

		if(verbose_) { // Let the audience know
			std::cout << "Found the following values in configuration file " << configFile << ":" << std::endl
					  << "maxStalledDataTransfers = " << maxStalledDataTransfers_ << std::endl;
		}
	}
	catch(const po::error& e) {
		std::cerr << "Error parsing the configuration file " << configFile << ":" << std::endl
				  << e.what() << std::endl;
		exit(1);
	}
	catch(...) {
		std::cerr << "Unknown error while parsing the configuration file " << configFile << std::endl;
		exit(1);
	}
}

/**************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
