/**
 * @file GOptimizer.hpp
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

// Standard header files go here
#include <vector>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost header files go here
#include <boost/utility.hpp>
#include <boost/function.hpp>
#include <boost/filesystem/operations.hpp>

#ifndef GOPTIMIZER_HPP_
#define GOPTIMIZER_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GEvolutionaryAlgorithm.hpp"
#include "geneva/GMultiThreadedEA.hpp"
#include "geneva/GBrokerEA.hpp"
#include "geneva/GSwarm.hpp"
#include "geneva/GMultiThreadedSwarm.hpp"
#include "geneva/GBrokerSwarm.hpp"
#include "geneva/GGradientDescent.hpp"
#include "geneva/GMultiThreadedGD.hpp"
#include "common/GExceptions.hpp"
#include "hap/GRandomT.hpp"

namespace Gem {
namespace Geneva {

/**************************************************************************************/
/**
 * This class acts as a wrapper around the various optimization algorithms in the
 * Geneva library. Its aim is to facilitate the usage of the various algorithms,
 * relieving users from having to write any other code than is needed by their parameter
 * descriptions. The class parses a configuration file covering the most common options of
 * the various optimization algorithms. The class will not touch the command line. The
 * user can make the name of a configuration file known to the class. If none is provided,
 * the class will attempt to load the data from a default file name.
 */
class GOptimizer
	:boost::noncopyable
{
public:
	/** @brief The standard constructor. Loads the data from the configuration file */
	explicit GOptimizer(
			const personality& pers = EA
			, const parMode& pm = MULTITHREADED
			, const std::string& ip = "localhost"
			, const unsigned int& port = 10000
			, const std::string& fileName = "optimizationAlgorithm.cfg"
			, const bool& verbose = false
	);

	/** @brief Allows to register a function object that performs necessary initialization work */
	void registerInitFunction(boost::function<void ()>);
	/** @brief Allows to register a function object that performs necessary initialization work for the client */
	void registerClientInitFunction(boost::function<void ()>);
	/** @brief Allows to register a function object that performs necessary finalization work */
	void registerFinalizationFunction(boost::function<void ()>);
	/** @brief Allows to register a function object that performs necessary finalization work for the client */
	void registerClientFinalizationFunction(boost::function<void ()>);

	/** @brief Allows to add individuals to the class. */
	void registerParameterSet(boost::shared_ptr<GParameterSet>);
	/** @brief Allows to add a set of individuals to the class. */
	void registerParameterSet(const std::vector<boost::shared_ptr<GParameterSet> >&);

	/** @brief Allows to specify an optimization monitor to be used with evolutionary algorithms */
	void registerOptimizationMonitor(boost::shared_ptr<GEvolutionaryAlgorithm::GEAOptimizationMonitor>);
	/** @brief Allows to specify an optimization monitor to be used with swarm algorithms */
	void registerOptimizationMonitor(boost::shared_ptr<GSwarm::GSwarmOptimizationMonitor>);
	/** @brief Allows to specify an optimization monitor to be used with gradient descents */
	void registerOptimizationMonitor(boost::shared_ptr<GGradientDescent::GGDOptimizationMonitor>);

	/**************************************************************************************/
	/**
	 * Starts the optimization cycle, using the optimization algorithm that has been requested.
	 * Returns the best individual found, converted to the desired type.
	 *
	 * @return The best individual found during the optimization process, converted to the desired type
	 */
	template <typename ind_type>
	boost::shared_ptr<ind_type> optimize() {
		boost::shared_ptr<ind_type> result;

		// If an initialization function has been provided, call it as the first action
		if(initFunction_) initFunction_();

#ifdef DEBUG
		// We need at least one individual to start with
		if(initialParameterSets_.empty()) {
			std::ostringstream error;
			error << "In GOptimizer::optimize(): Error!" << std::endl
					<< "You need to register at least one individual." << std::endl
					<< "Found none." << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}
#endif

		// Which algorithm are we supposed to use ?
		switch(pers_) {
		case EA: // Evolutionary algorihms
		{
			result = eaOptimize<ind_type>();
			if(finalizationFunction_) finalizationFunction_();
			return result;
		}
		break;

		case SWARM: // Swarm algorithms
		{
			result = swarmOptimize<ind_type>();
			if(finalizationFunction_) finalizationFunction_();
			return result;
		}
		break;

		case GD: // Gradient descents
		{
			result = gdOptimize<ind_type>();
			if(finalizationFunction_) finalizationFunction_();
			return result;
		}
		break;

		case NONE:
		{
			std::ostringstream error;
			error << "In GOptimizer::optimize(): Error!" << std::endl
					<< "No optimization algorithm was specified." << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}
		break;
		};

		// Make the compiler happy
		return boost::shared_ptr<ind_type>(); // Empty smart pointer
	}

	/**************************************************************************************/

	/** @brief Triggers execution of the client loop */
	void clientRun();

private:
	/** @brief The default constructor. Intentionally private and undefined */
	GOptimizer();

	/** @brief Loads the configuration data from a given configuration file */
	void loadConfigurationData(const std::string&);

	/**************************************************************************************/
	/**
	 * Performs an EA optimization cycle
	 *
	 * @return The best individual found during the optimization process
	 */
	template <typename ind_type>
	boost::shared_ptr<ind_type> eaOptimize() {
		// This smart pointer will hold the different types of evolutionary algorithms
		boost::shared_ptr<GEvolutionaryAlgorithm> ea_ptr;

		switch(parMode_) {
		//----------------------------------------------------------------------------------
		case SERIAL:
		{
			// Create an empty population
			ea_ptr = boost::shared_ptr<GEvolutionaryAlgorithm>(new GEvolutionaryAlgorithm());
		}
		break;

		//----------------------------------------------------------------------------------
		case MULTITHREADED:
		{
			// Create the multi-threaded population
			boost::shared_ptr<GMultiThreadedEA> eaPar_ptr(new GMultiThreadedEA());

			// Population-specific settings
			eaPar_ptr->setNThreads(nEvaluationThreads_);

			// Assignment to the base pointer
			ea_ptr = eaPar_ptr;
		}
		break;

		//----------------------------------------------------------------------------------
		case ASIONETWORKED:
		{
			// Create a network consumer and enrol it with the broker
			boost::shared_ptr<GAsioTCPConsumerT<GIndividual> > gatc(new GAsioTCPConsumerT<GIndividual>(port_));
			gatc->setSerializationMode(serializationMode);
			GINDIVIDUALBROKER->enrol(gatc);

			// Create the actual broker population
			boost::shared_ptr<GBrokerEA> eaBroker_ptr(new GBrokerEA());
			eaBroker_ptr->setWaitFactor(waitFactor_);

			// Assignment to the base pointer
			ea_ptr = eaBroker_ptr;
		}
		break;

		//----------------------------------------------------------------------------------
		};

		// Transfer the initial parameter sets to the population
		for(std::size_t p = 0 ; p<initialParameterSets_.size(); p++) {
			ea_ptr->push_back(initialParameterSets_[p]);
		}
		initialParameterSets_.clear();

		// Specify some specific EA settings
		ea_ptr->setDefaultPopulationSize(eaPopulationSize_,eaNParents_);
		ea_ptr->setRecombinationMethod(eaRecombinationScheme_);
		ea_ptr->setSortingScheme(eaSortingScheme_);
		ea_ptr->setLogOldParents(eaTrackParentRelations_);

		// Set some general population settings
		ea_ptr->setMaxIteration(maxIterations_);
		ea_ptr->setMaxTime(boost::posix_time::minutes(maxMinutes_));
		ea_ptr->setReportIteration(reportIteration_);

		// Register the optimization monitor, if one has been provided
		if(ea_om_ptr_) ea_ptr->registerOptimizationMonitor(ea_om_ptr_);

		// Do the actual optimization
		ea_ptr->optimize();

		// Return the best individual found
		return ea_ptr->getBestIndividual<ind_type>();
	}

	/**************************************************************************************/
	/**
	 * Performs a swarm optimization cycle
	 *
	 * @return The best individual found during the optimization process
	 */
	template <typename ind_type>
	boost::shared_ptr<ind_type> swarmOptimize() {
		// This smart pointer will hold the different types of evolutionary algorithms
		boost::shared_ptr<GSwarm> swarm_ptr;

		switch(parMode_) {
		//----------------------------------------------------------------------------------
		case SERIAL:
		{
			swarm_ptr = boost::shared_ptr<GSwarm>(new GSwarm(swarmNNeighborhoods_, swarmNNeighborhoodMembers_));
		}
		break;

		//----------------------------------------------------------------------------------

		case MULTITHREADED:
		{
			// Create the multi-threaded population
			boost::shared_ptr<GMultiThreadedSwarm> swarmPar_ptr(new GMultiThreadedSwarm(swarmNNeighborhoods_, swarmNNeighborhoodMembers_));

			// Population-specific settings
			swarmPar_ptr->setNThreads(nEvaluationThreads_);

			// Assignment to the base pointer
			swarm_ptr = swarmPar_ptr;
		}
		break;

		//----------------------------------------------------------------------------------

		case ASIONETWORKED:
		{
			// Create a network consumer and enrol it with the broker
			boost::shared_ptr<GAsioTCPConsumerT<GIndividual> > gatc(new GAsioTCPConsumerT<GIndividual>(port_));
			GINDIVIDUALBROKER->enrol(gatc);

			// Create the actual broker population
			boost::shared_ptr<GBrokerSwarm> swarmBroker_ptr(new GBrokerSwarm(swarmNNeighborhoods_, swarmNNeighborhoodMembers_));
			swarmBroker_ptr->setWaitFactor(waitFactor_);

			// Assignment to the base pointer
			swarm_ptr = swarmBroker_ptr;
		}
		break;

		//----------------------------------------------------------------------------------
		};

		// Specify some specific swarm settings
		if(swarmRandomFillUp_) {
			swarm_ptr->sedNeighborhoodsRandomFillUp();
		}
		else {
			swarm_ptr->setNeighborhoodsEqualFillUp();
		}
		swarm_ptr->setCLocal(swarmCLocal_);
		swarm_ptr->setCGlobal(swarmCGlobal_);
		swarm_ptr->setCDelta(swarmCDelta_);
		swarm_ptr->setUpdateRule(swarmUpdateRule_);

		// Set some general population settings
		swarm_ptr->setMaxIteration(maxIterations_);
		swarm_ptr->setMaxTime(boost::posix_time::minutes(maxMinutes_));
		swarm_ptr->setReportIteration(reportIteration_);

		// Register the optimization monitor (if one has been provided)
		if(swarm_om_ptr_) swarm_ptr->registerOptimizationMonitor(swarm_om_ptr_);

		// Return the best individual found
		return swarm_ptr->getBestIndividual<ind_type>();
	}

	/**************************************************************************************/
	/**
	 * Performs a GD optimization cycle
	 *
	 * @return The best individual found during the optimization process
	 */
	template <typename ind_type>
	boost::shared_ptr<ind_type> gdOptimize() {
		// This smart pointer will hold the different types of evolutionary algorithms
		boost::shared_ptr<GGradientDescent> gd_ptr;

		switch(parMode_) {
		//----------------------------------------------------------------------------------
		case SERIAL:
		{
			// Create an empty population
			gd_ptr = boost::shared_ptr<GGradientDescent>(new GGradientDescent(gdNStartingPoints_, gdFiniteStep_, gdStepSize_));
		}
		break;

		//----------------------------------------------------------------------------------
		case MULTITHREADED:
		{
		  // Create the multi-threaded population
		  boost::shared_ptr<GMultiThreadedGD> gdPar_ptr(new GMultiThreadedGD(gdNStartingPoints_, gdFiniteStep_, gdStepSize_));

		  // Population-specific settings
		  gdPar_ptr->setNThreads(nEvaluationThreads_);

		  // Assignment to the base pointer
		  gd_ptr = gdPar_ptr;
		}
		break;

		//----------------------------------------------------------------------------------
		case ASIONETWORKED:
		{
			std::ostringstream error;
			error << "In GOptimizer::gdOptimize(): Error!" << std::endl
				  << "ASIONETWORKED mode not implemented yet for gradient descents." << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}
		break;

		//----------------------------------------------------------------------------------
		};

		// Set some general population settings
		gd_ptr->setMaxIteration(maxIterations_);
		gd_ptr->setMaxTime(boost::posix_time::minutes(maxMinutes_));
		gd_ptr->setReportIteration(reportIteration_);

		// Register the optimization monitor (if one has been provided)
		if(gd_om_ptr_) swarm_ptr->registerOptimizationMonitor(gd_om_ptr_);

		// Return the best individual found
		return gd_ptr->getBestIndividual<ind_type>();
	}

	/**********************************************************************/
	// These parameters enter the object through the constructor
	personality pers_; ///< Indicates which optimization algorithm should be used
	parMode parMode_; ///< The chosen parallelization mode
    std::string ip_; ///< Where the server can be reached
    unsigned short port_; ///< The port on which the server answers
	std::string configFilename_; ///< Indicates where the configuration file is stored
	bool verbose_; ///< Whether additional information should be emitted, e.g. when parsing configuration files

	// Parameters registered through member functions
	boost::function<void ()> initFunction_; ///< Actions to be performed before the optimization starts
	boost::function<void ()> clientInitFunction_; ///< Actions to be performed for clients before the optimization starts
	boost::function<void ()> finalizationFunction_; ///< Actions to be performed after the optimization has ended
	boost::function<void ()> clientFinalizationFunction_; ///< Actions to be performed for clients after the client loop has ended
	std::vector<boost::shared_ptr<GParameterSet> > initialParameterSets_;  ///< Holds the individuals used for the initialization of the algorithm
	boost::shared_ptr<GEvolutionaryAlgorithm::GEAOptimizationMonitor> ea_om_ptr_; ///< Holds a specific optimization monitor used for evolutionary algorithms
	boost::shared_ptr<GSwarm::GSwarmOptimizationMonitor> swarm_om_ptr_; ///< Holds a specific optimization monitor used for swarm algorithms
	boost::shared_ptr<GGradientDescent::GGDOptimizationMonitor> gd_om_ptr_; ///< Holds a specific optimization monitor used for gradient descents

	//----------------------------------------------------------------------------------------------------------------
	// These parameters are read from a configuration file

	// General parameters
    boost::uint32_t maxStalledDataTransfers_; ///< Specifies how often a client may try to unsuccessfully retrieve data from the server (0 means endless)
    boost::uint32_t maxConnectionAttempts_; ///< Specifies how often a client may try to connect unsuccessfully to the server (0 means endless)
    bool returnRegardless_; ///< Specifies whether unsuccessful processing attempts should be returned to the server
    boost::uint16_t nProducerThreads_; ///< The number of threads that will simultaneously produce random numbers
    std::size_t arraySize_; ///< The size of the random number packages being transferred to the proxy RNGs
    boost::uint16_t nEvaluationThreads_; ///< The number of threads used for evaluations in multithreaded execution
    Gem::Common::serializationMode serializationMode_; ///< The mode used for the (de-)serialization of objects
    boost::uint32_t waitFactor_; ///< Influences the timeout in each iteration on the server side in networked execution
    boost::uint32_t maxIterations_; ///< The maximum number of iterations of the optimization algorithms
    long maxMinutes_; ///< The maximum duration of the optimization
    boost::uint32_t reportIteration_; ///< The number of iterations after which information should be emitted

    // EA parameters
    std::size_t eaPopulationSize_; ///< The desired size of EA populations
    std::size_t eaNParents_; ///< The number of parents in an EA population
    recoScheme eaRecombinationScheme_; ///< The recombination scheme in EA
    sortingMode eaSortingScheme_; ///< The sorting scheme in EA (MUCOMMANU etc.)
    bool eaTrackParentRelations_; ///< Whether relations between children and parents should be tracked in EA

    // SWARM parameters
    std::size_t swarmNNeighborhoods_; ///< The number of neighborhoods in a swarm algorithm
    std::size_t swarmNNeighborhoodMembers_; ///< The number of members ine ach neighborhood
    bool swarmRandomFillUp_; ///< Specifies whether neighborhoods are filled up with random values

    // Gradient descent parameters
    std::size_t gdNStartingPoints_;
    float gdFiniteStep_;
    float gdStepSize_;
};

/**************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GOPTIMIZER_HPP_ */
