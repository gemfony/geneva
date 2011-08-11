/**
 * @file Go.hpp
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

// Standard header files go here

// Boost header files go here

#ifndef GO_HPP_
#define GO_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "hap/GRandomT.hpp"
#include "courtier/GAsioTCPConsumerT.hpp"
#include "courtier/GAsioTCPClientT.hpp"
#include "geneva/GMutableSetT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GMutableSetT.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GSerialEA.hpp"
#include "geneva/GMultiThreadedEA.hpp"
#include "geneva/GBrokerEA.hpp"
#include "geneva/GSerialSwarm.hpp"
#include "geneva/GMultiThreadedSwarm.hpp"
#include "geneva/GBrokerSwarm.hpp"
#include "geneva/GSerialGD.hpp"
#include "geneva/GMultiThreadedGD.hpp"
#include "geneva/GBrokerGD.hpp"
#include "geneva/GenevaHelperFunctionsT.hpp"

namespace Gem {
namespace Geneva {

/**************************************************************************************/
// Default values for the variables used by the optimizer
const personality GO_DEF_PERSONALITY=EA;
const parMode GO_DEF_PARALLELIZATIONMODE=MULTITHREADED;
const std::string GO_DEF_DEFAULTCONFIGFILE="optimizationAlgorithm.cfg";
const bool GO_DEF_SERVERMODE=true;
const parMode GO_DEF_DEFAULPARALLELIZATIONMODE=MULTITHREADED;
const Gem::Common::serializationMode GO_DEF_SERIALIZATIONMODE=Gem::Common::SERIALIZATIONMODE_BINARY;
const std::string GO_DEF_IP="localhost";
const unsigned int GO_DEF_PORT=10000;
const bool GO_DEF_DEFAULTVERBOSE=false;
const bool GO_DEF_COPYBESTONLY=true;
const boost::uint16_t GO_DEF_MAXSTALLED=0;
const boost::uint16_t GO_DEF_MAXCONNATT=100;
const bool GO_DEF_RETURNREGARDLESS=true;
const boost::uint16_t GO_DEF_NPRODUCERTHREADS=0;
const std::size_t GO_DEF_ARRAYSIZE=1000;
const boost::uint16_t GO_DEF_NEVALUATIONTHREADS=0;
const boost::uint32_t GO_DEF_NPROCUNITS=0;
const boost::uint32_t GO_DEF_MAXITERATIONS=1000;
const boost::uint32_t GO_DEF_MAXSTALLITERATIONS=0;
const long GO_DEF_MAXMINUTES=0;
const boost::uint32_t GO_DEF_REPORTITERATION=1;
const boost::uint32_t GO_DEF_OFFSET=0;
const bool GO_DEF_CONSUMERINITIALIZED=false;
const std::size_t GO_DEF_EAPOPULATIONSIZE=100;
const std::size_t GO_DEF_EANPARENTS=1;
const recoScheme GO_DEF_EARECOMBINATIONSCHEME=VALUERECOMBINE;
const sortingMode GO_DEF_EASORTINGSCHEME=MUCOMMANU_SINGLEEVAL;
const bool GO_DEF_EATRACKPARENTRELATIONS=false;
const std::size_t GO_DEF_EAGROWTHRATE=0;
const std::size_t GO_DEF_EAMAXPOPSIZE=0;
const std::size_t GO_DEF_SWARMNNEIGHBORHOODS=5;
const std::size_t GO_DEF_SWARMNNEIGHBORHOODMEMBERS=10;
const bool GO_DEF_SWARMRANDOMFILLUP=1;
const float GO_DEF_SWARMCPERSONAL=0.05;
const float GO_DEF_SWARMCNEIGHBORHOOD=2.;
const float GO_DEF_SWARMCVELOCITY=0.4;
const updateRule GO_DEF_SWARMUPDATERULE=CLASSIC;
const std::size_t GO_DEF_GDNSTARTINGPOINTS=1;
const float GO_DEF_GDFINITESTEP=0.0000001;
const float GO_DEF_GDSTEPSIZE=0.1;

/**************************************************************************************/
/**
 * This class acts as a wrapper around the various optimization algorithms in the
 * Geneva library. Its aim is to facilitate the usage of the various algorithms,
 * relieving users from having to write much code beyond what is needed by their parameter
 * descriptions. The class parses a configuration file covering the most common options of
 * the various optimization algorithms. The class will not touch the command line. The
 * user can make the name of a configuration file known to the class. If none is provided,
 * the class will attempt to load the data from a default file name.
 */
class Go
	: public GMutableSetT<GParameterSet>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GMutableSetT_GParameterSet", boost::serialization::base_object<GMutableSetT<GParameterSet> >(*this))
		 & BOOST_SERIALIZATION_NVP(pers_)
	  	 & BOOST_SERIALIZATION_NVP(parMode_)
	     & BOOST_SERIALIZATION_NVP(serverMode_)
	     & BOOST_SERIALIZATION_NVP(serializationMode_)
	     & BOOST_SERIALIZATION_NVP(ip_)
	     & BOOST_SERIALIZATION_NVP(port_)
	     & BOOST_SERIALIZATION_NVP(configFilename_)
	     & BOOST_SERIALIZATION_NVP(verbose_)
	     & BOOST_SERIALIZATION_NVP(ea_om_ptr_)
	     & BOOST_SERIALIZATION_NVP(swarm_om_ptr_)
	     & BOOST_SERIALIZATION_NVP(gd_om_ptr_)
	     & BOOST_SERIALIZATION_NVP(copyBestOnly_)
	     & BOOST_SERIALIZATION_NVP(maxStalledDataTransfers_)
	     & BOOST_SERIALIZATION_NVP(maxConnectionAttempts_)
	     & BOOST_SERIALIZATION_NVP(returnRegardless_)
	     & BOOST_SERIALIZATION_NVP(nProducerThreads_)
	     & BOOST_SERIALIZATION_NVP(arraySize_)
	     & BOOST_SERIALIZATION_NVP(nEvaluationThreads_)
	     & BOOST_SERIALIZATION_NVP(serializationMode_)
	     & BOOST_SERIALIZATION_NVP(nProcessingUnits_)
	     & BOOST_SERIALIZATION_NVP(maxIterations_)
	     & BOOST_SERIALIZATION_NVP(maxStallIteration_)
	     & BOOST_SERIALIZATION_NVP(maxMinutes_)
	     & BOOST_SERIALIZATION_NVP(reportIteration_)
	     & BOOST_SERIALIZATION_NVP(offset_)
	     & BOOST_SERIALIZATION_NVP(eaPopulationSize_)
	     & BOOST_SERIALIZATION_NVP(eaNParents_)
	     & BOOST_SERIALIZATION_NVP(eaRecombinationScheme_)
	     & BOOST_SERIALIZATION_NVP(eaSortingScheme_)
	     & BOOST_SERIALIZATION_NVP(eaTrackParentRelations_)
	     & BOOST_SERIALIZATION_NVP(eaGrowthRate_)
	     & BOOST_SERIALIZATION_NVP(eaMaxPopSize_)
	     & BOOST_SERIALIZATION_NVP(swarmNNeighborhoods_)
	     & BOOST_SERIALIZATION_NVP(swarmNNeighborhoodMembers_)
	     & BOOST_SERIALIZATION_NVP(swarmRandomFillUp_)
	     & BOOST_SERIALIZATION_NVP(swarmCPersonal_)
	     & BOOST_SERIALIZATION_NVP(swarmCNeighborhood_)
	     & BOOST_SERIALIZATION_NVP(swarmCVelocity_)
	     & BOOST_SERIALIZATION_NVP(swarmUpdateRule_)
	     & BOOST_SERIALIZATION_NVP(gdNStartingPoints_)
	     & BOOST_SERIALIZATION_NVP(gdFiniteStep_)
	     & BOOST_SERIALIZATION_NVP(gdStepSize_)
	     & BOOST_SERIALIZATION_NVP(bestIndividual_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	Go();
	/** @brief A constructor that first parses the command line for relevant parameters and then loads data from a config file */
	Go(int, char **, const std::string& = GO_DEF_DEFAULTCONFIGFILE);
	/** @brief A constructor that is given the usual command line parameters, then loads the rest of the data from a config file */
	Go(
		const personality& pers
		, const parMode&
		, const bool&
		, const Gem::Common::serializationMode&
		, const std::string&
		, const unsigned short&
		, const std::string&
		, const bool&
	);
	/** @brief A copy constructor */
	Go(const Go&);

	/** @brief The destructor */
	virtual ~Go();

	/** @brief Standard assignment operator */
	const Go& operator=(const Go&);

	/** @brief Checks for equality with another Go object */
	bool operator==(const Go&) const;
	/** @brief Checks for inequality with another Go object */
	bool operator!=(const Go&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&
			, const Gem::Common::expectation&
			, const double&
			, const std::string&
			, const std::string&
			, const bool&
	) const;

	/** @brief Allows to specify an optimization monitor to be used with evolutionary algorithms */
	void registerOptimizationMonitor(boost::shared_ptr<GBaseEA::GEAOptimizationMonitor>);
	/** @brief Allows to specify an optimization monitor to be used with swarm algorithms */
	void registerOptimizationMonitor(boost::shared_ptr<GSerialSwarm::GSwarmOptimizationMonitor>);
	/** @brief Allows to specify an optimization monitor to be used with gradient descents */
	void registerOptimizationMonitor(boost::shared_ptr<GBaseGD::GGDOptimizationMonitor>);

	/** @brief Triggers execution of the client loop */
	bool clientRun();

	/** @brief Checks whether server mode has been requested for this object */
	bool serverMode() const;
	/** @brief Checks whether this object is running in client mode */
	bool clientMode() const;

	/* @brief Specifies whether only the best individuals of a population should be copied */
	void setCopyBestIndividualsOnly(const bool&);
	/** @brief Checks whether only the best individuals are copied */
	bool onlyBestIndividualsAreCopied() const;

	/** @brief Allows to randomly initialize parameter members. Unused in this wrapper object */
	virtual void randomInit();
	/** @brief Triggers fitness calculation (i.e. optimization) for this object */
	virtual double fitnessCalculation();

	/**************************************************************************************/
	// The following is a trivial list of getters and setters
	void setPersonality(const personality&);
	personality getPersonality() const;

	void setParallelizationMode(const parMode&);
	parMode getParallelizationMode() const;

	void setServerMode(const bool&);
	bool getServerMode() const;

	void setSerializationMode(const Gem::Common::serializationMode&);
	Gem::Common::serializationMode getSerializationMode() const;

	void setServerIp(const std::string&);
	std::string getServerIp() const;

	void setServerPort(const unsigned short&);
	unsigned short getServerPort() const;

	void setConfigFileName(const std::string&);
	std::string getConfigFileName() const;

	void setVerbosity(const bool&);
	bool getVerbosity() const;

	void setMaxStalledDataTransfers(const boost::uint32_t&);
	boost::uint32_t getMaxStalledDataTransfers() const;

	void setMaxConnectionAttempts(const boost::uint32_t&);
	boost::uint32_t getMaxConnectionAttempts() const;

	void setReturnRegardless(const bool&);
	bool getReturnRegardless() const;

	void setNProducerThreads(const boost::uint16_t&);
	boost::uint16_t getNProducerThreads() const;

	void setArraySize(const std::size_t&);
	std::size_t getArraySize() const;

	void setNEvaluationThreads(const boost::uint16_t&);
	boost::uint16_t getNEvaluationThreads() const;

	void setNProcessingUnits(const boost::uint32_t&);
	boost::uint32_t getNProcessingUnits() const;

	void setMaxIterations(const boost::uint32_t&);
	boost::uint32_t getMaxIterations() const;

	void setMaxStallIteration(const boost::uint32_t&);
	boost::uint32_t getMaxStallIteration() const;

	void setMaxMinutes(const long&);
	long getMaxMinutes() const;

	void setReportIteration(const boost::uint32_t&);
	boost::uint32_t getReportIteration() const;

	void setOffset(const boost::uint32_t&);
	boost::uint32_t getOffset() const;

	void setEAPopulationSize(const std::size_t&);
	std::size_t getEAPopulationSize() const;

	void setEANParents(const std::size_t&);
	std::size_t getEANParents() const;

	void setEARecombinationScheme(const recoScheme&);
	recoScheme getEARecombinationScheme() const;

	void setEASortingScheme(const sortingMode&);
	sortingMode getEASortingScheme() const;

	void setEATrackParentRelations(const bool&);
	bool getEATrackParentRelations() const;

	void setEAGrowthRate(const std::size_t&);
	std::size_t getEAGrowthRate() const;

	void setEAMaxPopSize(const std::size_t&);
	std::size_t getEAMaxPopSize() const;

	void setSwarmNNeighborhoods(const std::size_t&);
	std::size_t getSwarmNNeighborhoods() const;

	void setSwarmNNeighborhoodMembers(const std::size_t&);
	std::size_t getSwarmNNeighborhoodMembers() const;

	void setSwarmRandomFillUp(const bool&);
	bool getSwarmRandomFillUp() const;

	void setSwarmCPersonal(const float&);
	float getSwarmCPersonal() const;

	void setSwarmCNeighborhood(const float&);
	float getSwarmCNeighborhood() const;

	void setSwarmCVelocity(const float&);
	float getSwarmCVelocity() const;

	void setSwarmUpdateRule(const updateRule&);
	updateRule getSwarmUpdateRule() const;

	void setGDNStartingPoints(const std::size_t&);
	std::size_t getGDNStartingPoints() const;

	void setGDFiniteStep(const float&);
	float getGDFiniteStep() const;

	void setGDStepSize(const float&);
	float getGDStepSize() const;

	/**************************************************************************************/
	/**
	 * Retrieves a copy of the best individual (if any) and converts it to the
	 * desired target type.
	 *
	 * @return A converted shared_ptr to a copy of the best individual of the population
	 */
	template <typename ind_type>
	inline boost::shared_ptr<ind_type> getBestIndividual(
			typename boost::enable_if<boost::is_base_of<GParameterSet, ind_type> >::type* dummy = 0
	){
#ifdef DEBUG
		// Check that bestIndividual_ actually points somewhere
		if(!bestIndividual_) {
			raiseException(
					"In Go::getBestIndividual<>() : Error" << std::endl
					<< "Tried to access uninitialized best individual."
			);
		}
#endif /* DEBUG */

		return bestIndividual_->clone<ind_type>();
	}

	/**************************************************************************************/
	/**
	 * Starts the optimization cycle, using the optimization algorithm that has been requested.
	 * Returns the best individual found, converted to the desired type. Note that after a call
	 * to this function, both the number of individuals stored in this object and their content
	 * will have changed. The function can only be called if ind_type is a derivative of GParameterSet.
	 * This enables us to store the best individual in a smart pointer to a GParameterSet object.
	 *
	 * @param offset An offset for the iteration counter
	 * @return The best individual found during the optimization process, converted to the desired type
	 */
	template <typename ind_type>
	boost::shared_ptr<ind_type> optimize(
			const boost::uint32_t& offset = 0
			, typename boost::enable_if<boost::is_base_of<GParameterSet, ind_type> >::type* dummy = 0
	) {
		boost::shared_ptr<ind_type> result;

		// We need at least one individual to start with
		if(this->empty()) {
			raiseException(
					"In Go::optimize():" << std::endl
					<< "You need to register at least one individual." << std::endl
					<< "Found none."
			);
		}

		// Which algorithm are we supposed to use ?
		switch(pers_) {
		case EA: // Evolutionary algorithms
			result = eaOptimize<ind_type>(offset);
			break;

		case SWARM: // Swarm algorithms
			result = swarmOptimize<ind_type>(offset);
			break;

		case GD: // Gradient descents
			result = gdOptimize<ind_type>(offset);
			break;

		case NONE:
			{
				raiseException(
						"In Go::optimize():" << std::endl
						<< "No optimization algorithm was specified."
				);
			}
			break;
		};

		// Create a copy of the best individual for later use
		bestIndividual_ = result->GObject::clone<GParameterSet>();

		// Let the audience know
		return result;
	}

	/**************************************************************************************/
	/**
	 * Outputs a configuration file with default values
	 *
	 * @param configFile The name of the file to which the configuration should be written
	 */
	static void writeConfigurationFile(const std::string& configFile) {
		std::ofstream cf(configFile.c_str());
		if(!cf) {
			raiseException(
					"In Go::writeConfigurationFile() :" << std::endl
					<< "Could not open output file " << configFile
			);
		}

		cf << "################################################################" << std::endl
		   << "# This is a configuration file for the optimization            #" << std::endl
		   << "# algorithms implemented in the Geneva library.                #" << std::endl
		   << "# It is meant to be accessed through the Go class              #" << std::endl
		   << "#                                                              #" << std::endl
		   << "# This file was automatically created by the Geneva library    #" << std::endl
		   << "################################################################" << std::endl
		   << "#" << std::endl
	       << "# General options applicable to all optimization algorithms" << std::endl
	       << std::endl
	       << "# The maximum number of data transfers without result." << std::endl
	       << "# 0 means \"no limit\"" << std::endl
	       << "maxStalledDataTransfers = " << GO_DEF_MAXSTALLED << std::endl
	       << std::endl
	       << "# The maximum number of failed connection attempts between" << std::endl
	       << "# client and server. 0 means \"no limit\"" << std::endl
	       << "maxConnectionAttempts = " << GO_DEF_MAXCONNATT << std::endl
	       << std::endl
	       << "# Specifies whether the optimizer should copy only the best individuals" << std::endl
	       << "# at the end of the optimization or the entire population" << std::endl
	       << "copyBestOnly = " << GO_DEF_COPYBESTONLY << std::endl
	       << std::endl
	       << "# Indicates whether clients should return their payload even" << std::endl
	       << "# if no better result was found" << std::endl
	       << "returnRegardless = " << GO_DEF_RETURNREGARDLESS << std::endl
	       << std::endl
	       << "# Determines the number of threads simultaneously producing" << std::endl
	       << "# random numbers. 0 means \"automatic\"" << std::endl
	       << "nProducerThreads = " << GO_DEF_NPRODUCERTHREADS << std::endl
	       << std::endl
	       << "# Specifies the number of entries in random number packages" << std::endl
	       << "# coming from the factory" << std::endl
	       << "arraySize = " << GO_DEF_ARRAYSIZE << std::endl
	       << std::endl
	       << "# Determines the number of threads simultaneously performing" << std::endl
	       << "# evaluations in multi-threaded mode. 0 means \"automatic\"" << std::endl
	       << "nEvaluationThreads = " << GO_DEF_NEVALUATIONTHREADS << std::endl
	       << std::endl
	       << "# Specifies whether client-server transfers should be done in" << std::endl
	       << "# text-mode (0), xml-mode (1) or binary-mode (2)" << std::endl
	       << "serializationMode = " << GO_DEF_SERIALIZATIONMODE << std::endl
	       << std::endl
	       << "# Influences for how long the server should wait for arrivals" << std::endl
           << "# in networked mode" << std::endl
	       << "nProcessingUnits = " << GO_DEF_NPROCUNITS << std::endl
	       << std::endl
	       << "# Indicates the maximum number of iterations in the optimization" << std::endl
	       << "maxIterations = " << GO_DEF_MAXITERATIONS << std::endl
	       << std::endl
	       << "# The maximum amount of iterations without improvement before the current" << std::endl
	       << "# optimization algorithm halts" << std::endl
	       << "maxStallIteration = " << GO_DEF_MAXSTALLITERATIONS << std::endl
	       << std::endl
	       << "# Specifies the maximum amount of time that may pass before the" << std::endl
	       << "# optimization ends. 0 mean \"no limit\""
	       << "maxMinutes = " << GO_DEF_MAXMINUTES << std::endl
	       << std::endl
	       << "# Specifies in which intervals information should be emitted" << std::endl
	       << "reportIteration = " << GO_DEF_REPORTITERATION << std::endl
	       << std::endl
	       << "# An offset used for the iteration counter. " << std::endl
	       << "# Useful when starting several successive optimization runs" << std::endl
	       << "offset = " << GO_DEF_OFFSET << std::endl
	       << std::endl
	       << std::endl
	       << "#######################################################" << std::endl
	       << "# Options applicable to evolutionary algorithms" << std::endl
	       << "#" << std::endl
	       << std::endl
	       << "# The size of the entire population in evolutionary algorithms" << std::endl
	       << "eaPopulationSize = " << GO_DEF_EAPOPULATIONSIZE << std::endl
	       << std::endl
	       << "# The number of parents in the evolutionary algorithm" << std::endl
	       << "eaNParents = " << GO_DEF_EANPARENTS << std::endl
	       << std::endl
	       << "# The type of recombination scheme: DEFAULTRECOMBINE (0)," << std::endl
	       << "# RANDOMRECOMBINE (1) or VALUERECOMBINE(2)" << std::endl
	       << "eaRecombinationScheme = " << GO_DEF_EARECOMBINATIONSCHEME << std::endl
	       << std::endl
	       << "# The sorting scheme: MUPLUSNU_SINGLEEVAL (0), MUCOMMANU_SINGLEEVAL (1) or MUNU1PRETAIN (2)" << std::endl
	       << "eaSortingScheme = " << GO_DEF_EASORTINGSCHEME << std::endl
	       << std::endl
	       << "# Indicates whether the algorithm should track relationships" << std::endl
	       << "# between old parents and new children" << std::endl
	       << "eaTrackParentRelations = " << GO_DEF_EATRACKPARENTRELATIONS << std::endl
	       << std::endl
	       << "# The amount of individuals to be added in each iteration. Set to 0" << std::endl
	       << "# to disable growth" << std::endl
	       << "eaGrowthRate = " << GO_DEF_EAGROWTHRATE << std::endl
	       << std::endl
	       << "# The maximum allowed size of the population if growth is enabled" << std::endl
	       << "eaMaxPopSize = " << GO_DEF_EAMAXPOPSIZE << std::endl
	       << std::endl
	       << std::endl
	       << "#######################################################" << std::endl
	       << "# Options applicable to swarm algorithms" << std::endl
	       << "#" << std::endl
	       << std::endl
	       << "# The number of neighborhoods in swarm algorithms" << std::endl
	       << "swarmNNeighborhoods = " << GO_DEF_SWARMNNEIGHBORHOODS << std::endl
	       << std::endl
	       << "# The number of members in each neighborhood" << std::endl
	       << "swarmNNeighborhoodMembers = " << GO_DEF_SWARMNNEIGHBORHOODMEMBERS << std::endl
	       << std::endl
	       << "# Indicates whether all individuals of a neighborhood should" << std::endl
	       << "# start at the same or a random position" << std::endl
	       << "swarmRandomFillUp = " << GO_DEF_SWARMRANDOMFILLUP << std::endl
	       << std::endl
	       << "# A multiplicative factor for personal updates" << std::endl
	       << "swarmCPersonal = " << GO_DEF_SWARMCPERSONAL << std::endl
	       << std::endl
	       << "# A multiplicative factor for local updates" << std::endl
	       << "swarmCNeighborhood = " << GO_DEF_SWARMCNEIGHBORHOOD << std::endl
	       << std::endl
	       << "# A multiplicative factor for velocities" << std::endl
	       << "swarmCVelocity = " << GO_DEF_SWARMCVELOCITY << std::endl
	       << std::endl
	       << "# Indicates whether the linear (0) or classic (1)" << std::endl
	       << "# update rule should be used" << std::endl
	       << "swarmUpdateRule = " << GO_DEF_SWARMUPDATERULE << std::endl
	       << std::endl
	       << std::endl
	       << "#######################################################" << std::endl
	       << "# Options applicable to gradient descents" << std::endl
	       << "#" << std::endl
	       << std::endl
	       << "# Indicates how many simultaneous gradient descents should" << std::endl
	       << "# be started" << std::endl
	       << "gdNStartingPoints = " << GO_DEF_GDNSTARTINGPOINTS << std::endl
	       << std::endl
	       << "# Specifies the size of the finite step in each direction" << std::endl
	       << "gdFiniteStep = " << GO_DEF_GDFINITESTEP << std::endl
	       << std::endl
	       << "# Specifies the size of the step made into the direction" << std::endl
	       << "# of steepest descent" << std::endl
	       << "gdStepSize = " << GO_DEF_GDSTEPSIZE << std::endl;

		// Clean up
		cf.close();
	}

	/**************************************************************************************/
	/** @brief Loads the configuration data from a given configuration file */
	void parseConfigurationFile(const std::string&);
	/** @brief Loads some configuration data from arguments passed on the command line (or another char ** that is presented to it) */
	void parseCommandLine(int, char **);

protected:
	/** @brief Loads the data of another Go object */
	virtual void load_(const GObject *);
	/** @brief Creates a deep clone of this object */
	virtual GObject *clone_() const;

private:
	/**************************************************************************************/
	/**
	 * Performs an EA optimization cycle
	 *
	 * @param offset An offset for the iteration counter
	 * @return The best individual found during the optimization process
	 */
	template <typename ind_type>
	boost::shared_ptr<ind_type> eaOptimize(const boost::uint32_t& offset = 0) {
		// This smart pointer will hold the different types of evolutionary algorithms
		boost::shared_ptr<GBaseEA> ea_ptr;

		switch(parMode_) {
		//----------------------------------------------------------------------------------
		case SERIAL:
		{
			// Create an empty population
			ea_ptr = boost::shared_ptr<GSerialEA>(new GSerialEA());
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
			if(!consumerInitialized_) {
				// Create a network consumer and enrol it with the broker
				boost::shared_ptr<Gem::Courtier::GAsioTCPConsumerT<GIndividual> > gatc(new Gem::Courtier::GAsioTCPConsumerT<GIndividual>(
						port_
						, 0
						, serializationMode_
						)
				);
				GBROKER(Gem::Geneva::GIndividual)->enrol(gatc);

				consumerInitialized_ = true;
			}

			// Create the actual broker population
			boost::shared_ptr<GBrokerEA> eaBroker_ptr(new GBrokerEA());

			// Assignment to the base pointer
			ea_ptr = eaBroker_ptr;
		}
		break;

		//----------------------------------------------------------------------------------
		};

		// Specify some specific EA settings
		ea_ptr->setDefaultPopulationSize(eaPopulationSize_,eaNParents_);
		ea_ptr->setRecombinationMethod(eaRecombinationScheme_);
		ea_ptr->setSortingScheme(eaSortingScheme_);
		ea_ptr->setLogOldParents(eaTrackParentRelations_);
		ea_ptr->setPopulationGrowth(eaGrowthRate_, eaMaxPopSize_);

		// Set some general population settings
		ea_ptr->setMaxIteration(maxIterations_);
		ea_ptr->setMaxStallIteration(maxStallIteration_);
		ea_ptr->setMaxTime(boost::posix_time::minutes(maxMinutes_));
		ea_ptr->setReportIteration(reportIteration_);

		// Register the optimization monitor, if one has been provided
		if(ea_om_ptr_) ea_ptr->registerOptimizationMonitor(ea_om_ptr_);

		// Transfer the initial parameter sets to the population
		Go::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			// Note, there will not be a big space overhead here,
			// as what is being copied are smart pointers, not
			// the individuals themselves.
			ea_ptr->push_back(*it);
		}

		// Clean our own vector
		this->clear();

		// Do the actual optimization
		ea_ptr->optimize(offset);

		// Copy all or the best set of individuals back into our own vector
		if(copyBestOnly_) {
			for(std::size_t i=0; i<ea_ptr->getNParents(); i++) {
#ifdef DEBUG
				this->push_back(boost::dynamic_pointer_cast<GParameterSet>(ea_ptr->at(i)));
#else
				this->push_back(boost::static_pointer_cast<GParameterSet>(ea_ptr->at(i)));
#endif /* DEBUG */
			}
		} else {
			for(std::size_t i=0; i<ea_ptr->size(); i++) {
#ifdef DEBUG
				this->push_back(boost::dynamic_pointer_cast<GParameterSet>(ea_ptr->at(i)));
#else
				this->push_back(boost::static_pointer_cast<GParameterSet>(ea_ptr->at(i)));
#endif /* DEBUG */
			}
		}

		// Retrieve the best individual found
		boost::shared_ptr<ind_type> result = ea_ptr->GOptimizableI::getBestIndividual<ind_type>();

		// Adapt the local offset_ variable in case we intend to start another optimization run
		offset_ = ea_ptr->getIteration() + 1;

		// Make sure ea_ptr is clean again
		ea_ptr->clear();

		// Return the best individual found
		return result;
	}

	/**************************************************************************************/
	/**
	 * Performs a swarm optimization cycle
	 *
	 * @param offset An offset for the iteration counter
	 * @return The best individual found during the optimization process
	 */
	template <typename ind_type>
	boost::shared_ptr<ind_type> swarmOptimize(const boost::uint32_t& offset = 0) {
		// This smart pointer will hold the different types of evolutionary algorithms
		boost::shared_ptr<GSerialSwarm> swarm_ptr;

		switch(parMode_) {
		//----------------------------------------------------------------------------------
		case SERIAL:
		{
			swarm_ptr = boost::shared_ptr<GSerialSwarm>(new GSerialSwarm(swarmNNeighborhoods_, swarmNNeighborhoodMembers_));
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
			if(!consumerInitialized_) {
				// Create a network consumer and enrol it with the broker
				boost::shared_ptr<Gem::Courtier::GAsioTCPConsumerT<GIndividual> > gatc(new Gem::Courtier::GAsioTCPConsumerT<GIndividual>(
						port_
						, 0
						, serializationMode_
						)
				);
				GBROKER(Gem::Geneva::GIndividual)->enrol(gatc);

				consumerInitialized_ = true;
			}

			// Create the actual broker population
			boost::shared_ptr<GBrokerSwarm> swarmBroker_ptr(new GBrokerSwarm(swarmNNeighborhoods_, swarmNNeighborhoodMembers_));

			// Assignment to the base pointer
			swarm_ptr = swarmBroker_ptr;
		}
		break;

		//----------------------------------------------------------------------------------
		};

		// Specify some specific swarm settings
		if(swarmRandomFillUp_) {
			swarm_ptr->setNeighborhoodsRandomFillUp();
		}
		else {
			swarm_ptr->setNeighborhoodsEqualFillUp();
		}
		swarm_ptr->setCPersonal(swarmCPersonal_);
		swarm_ptr->setCNeighborhood(swarmCNeighborhood_);
		swarm_ptr->setCVelocity(swarmCVelocity_);
		swarm_ptr->setUpdateRule(swarmUpdateRule_);

		// Set some general population settings
		swarm_ptr->setMaxIteration(maxIterations_);
		swarm_ptr->setMaxStallIteration(maxStallIteration_);
		swarm_ptr->setMaxTime(boost::posix_time::minutes(maxMinutes_));
		swarm_ptr->setReportIteration(reportIteration_);

		// Register the optimization monitor (if one has been provided)
		if(swarm_om_ptr_) swarm_ptr->registerOptimizationMonitor(swarm_om_ptr_);

		// Transfer the initial parameter sets to the population
		Go::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			// Note, there will not be a big space overhead here,
			// as what is being copied are smart pointers, not
			// the individuals themselves.
			swarm_ptr->push_back(*it);
		}

		// Clean our own vector
		this->clear();

		// Do the actual optimization
		swarm_ptr->optimize(offset);

		// Copy all or the best set of individuals back into our own vector
		if(copyBestOnly_) {
			// Copy the local bests. One of them should be identical to the global best
			for(std::size_t i=0; i<swarm_ptr->getNNeighborhoods(); i++) {
				this->push_back(swarm_ptr->getBestNeighborhoodIndividual<GParameterSet>(i));
			}
		} else {
			// First copy the local bests
			for(std::size_t i=0; i<swarm_ptr->getNNeighborhoods(); i++) {
				this->push_back(swarm_ptr->getBestNeighborhoodIndividual<GParameterSet>(i));
			}

			// Then copy all other individuals
			for(std::size_t i=0; i<swarm_ptr->size(); i++) {
				this->push_back(swarm_ptr->at(i));
			}
		}

		// Retrieve the best individual found
		boost::shared_ptr<ind_type> result = swarm_ptr->GOptimizableI::getBestIndividual<ind_type>();

		// Adapt the local offset_ variable in case we intend to start another optimization run
		offset_ = swarm_ptr->getIteration() + 1;

		// Make sure ea_ptr is clean again
		swarm_ptr->clear();

		// Return the best individual found
		return result;
	}

	/**************************************************************************************/
	/**
	 * Performs a GD optimization cycle
	 *
	 * @param offset An offset for the iteration counter
	 * @return The best individual found during the optimization process
	 */
	template <typename ind_type>
	boost::shared_ptr<ind_type> gdOptimize(const boost::uint32_t& offset = 0) {
		// This smart pointer will hold the different types of evolutionary algorithms
		boost::shared_ptr<GBaseGD> gd_ptr;

		switch(parMode_) {
		//----------------------------------------------------------------------------------
		case SERIAL:
		{
			// Create an empty population
			gd_ptr = boost::shared_ptr<GSerialGD>(new GSerialGD(gdNStartingPoints_, gdFiniteStep_, gdStepSize_));
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
			if(!consumerInitialized_) {
				// Create a network consumer and enrol it with the broker
				boost::shared_ptr<Gem::Courtier::GAsioTCPConsumerT<GIndividual> > gatc(new Gem::Courtier::GAsioTCPConsumerT<GIndividual>(
						port_
						, 0
						, serializationMode_
						)
				);
				GBROKER(Gem::Geneva::GIndividual)->enrol(gatc);

				consumerInitialized_ = true;
			}

			// Create the actual broker population
			boost::shared_ptr<GBrokerGD> gdBroker_ptr(new GBrokerGD(gdNStartingPoints_, gdFiniteStep_, gdStepSize_));

			// Assignment to the base pointer
			gd_ptr = gdBroker_ptr;
		}
		break;

		//----------------------------------------------------------------------------------
		};

		// Set some general population settings
		gd_ptr->setMaxIteration(maxIterations_);
		gd_ptr->setMaxStallIteration(maxStallIteration_);
		gd_ptr->setMaxTime(boost::posix_time::minutes(maxMinutes_));
		gd_ptr->setReportIteration(reportIteration_);

		// Register the optimization monitor (if one has been provided)
		if(gd_om_ptr_) gd_ptr->registerOptimizationMonitor(gd_om_ptr_);

		// Transfer the initial parameter sets to the population. Note:
		// It doesn't make sense to transfer more items than starting
		// points in a gradient descent.
		Go::iterator it;
		for(it=this->begin(); it!=this->begin() + gdNStartingPoints_; ++it) {
			// Note, there will not be a big space overhead here,
			// as what is being copied are smart pointers, not
			// the individuals themselves.
			gd_ptr->push_back(*it);
		}

		// Clean our own vector
		this->clear();

		// Do the actual optimization
		gd_ptr->optimize(offset);

		// Copy the best individuals over. Note: Copying the entire population doesn't
		// make much sense for a gradient descent, but Geneva still gives you the freedom
		// to do so
		if(copyBestOnly_) {
			// Copy the starting points only.
			for(std::size_t i=0; i<gdNStartingPoints_; i++) {
				this->push_back(gd_ptr->at(i));
			}
		} else {
			// First copy the starting points.
			for(std::size_t i=0; i<gdNStartingPoints_; i++) {
				this->push_back(gd_ptr->at(i));
			}

			// Then copy all other individuals
			for(std::size_t i=gdNStartingPoints_; i<gd_ptr->size(); i++) {
				this->push_back(gd_ptr->at(i));
			}
		}

		// Retrieve the best individual found
		boost::shared_ptr<ind_type> result = gd_ptr->GOptimizableI::getBestIndividual<ind_type>();

		// Adapt the local offset_ variable in case we intend to start another optimization run
		offset_ = gd_ptr->getIteration() + 1;

		// Make sure ea_ptr is clean again
		gd_ptr->clear();

		// Return the best individual found
		return result;
	}

	/**********************************************************************/
	// These parameters can enter the object through the constructor
	personality pers_; ///< Indicates which optimization algorithm should be used
	parMode parMode_; ///< The chosen parallelization mode
	bool serverMode_; ///< Specifies whether this object is in server (true) or client (false) mode
	Gem::Common::serializationMode serializationMode_; ///< Indicates whether serialization should be done in Text, XML or Binary form
    std::string ip_; ///< Where the server can be reached
    unsigned short port_; ///< The port on which the server answers
	std::string configFilename_; ///< Indicates where the configuration file is stored
	bool verbose_; ///< Whether additional information should be emitted, e.g. when parsing configuration files

	boost::shared_ptr<GBaseEA::GEAOptimizationMonitor> ea_om_ptr_; ///< Holds a specific optimization monitor used for evolutionary algorithms
	boost::shared_ptr<GSerialSwarm::GSwarmOptimizationMonitor> swarm_om_ptr_; ///< Holds a specific optimization monitor used for swarm algorithms
	boost::shared_ptr<GBaseGD::GGDOptimizationMonitor> gd_om_ptr_; ///< Holds a specific optimization monitor used for gradient descents

	//----------------------------------------------------------------------------------------------------------------
	// These parameters can be read from a configuration file

	// Steering parameters of the optimizer
	bool copyBestOnly_;

	// General parameters
    boost::uint32_t maxStalledDataTransfers_; ///< Specifies how often a client may try to unsuccessfully retrieve data from the server (0 means endless)
    boost::uint32_t maxConnectionAttempts_; ///< Specifies how often a client may try to connect unsuccessfully to the server (0 means endless)
    bool returnRegardless_; ///< Specifies whether unsuccessful processing attempts should be returned to the server
    boost::uint16_t nProducerThreads_; ///< The number of threads that will simultaneously produce random numbers
    std::size_t arraySize_; ///< The size of the random number packages being transferred to the proxy RNGs
    boost::uint16_t nEvaluationThreads_; ///< The number of threads used for evaluations in multithreaded execution
    boost::uint32_t nProcessingUnits_; ///< Influences the timeout in each iteration on the server side in networked execution
    boost::uint32_t maxIterations_; ///< The maximum number of iterations of the optimization algorithms
    boost::uint32_t maxStallIteration_; ///< The maximum number of generations without improvement, after which optimization is stopped
    long maxMinutes_; ///< The maximum duration of the optimization
    boost::uint32_t reportIteration_; ///< The number of iterations after which information should be emitted
    boost::uint32_t offset_; ///< The offset to be used when starting a new optimization run
    bool consumerInitialized_; ///< Determines whether a consumer has already been started. Note that this variable will not be serialized

    // EA parameters
    std::size_t eaPopulationSize_; ///< The desired size of EA populations
    std::size_t eaNParents_; ///< The number of parents in an EA population
    recoScheme eaRecombinationScheme_; ///< The recombination scheme in EA
    sortingMode eaSortingScheme_; ///< The sorting scheme in EA (MUCOMMANU_SINGLEEVAL etc.)
    bool eaTrackParentRelations_; ///< Whether relations between children and parents should be tracked in EA
    std::size_t eaGrowthRate_; ///< The growth rate of the population per iteration
    std::size_t eaMaxPopSize_; ///< The maximum population size of population growth is enabled

    // SWARM parameters
    std::size_t swarmNNeighborhoods_; ///< The number of neighborhoods in a swarm algorithm
    std::size_t swarmNNeighborhoodMembers_; ///< The number of members in each neighborhood
    bool swarmRandomFillUp_; ///< Specifies whether neighborhoods are filled up with random values
    float swarmCPersonal_; ///< A factor for multiplication of personal bests
    float swarmCNeighborhood_; ///< A factor for multiplication of local bests
	float swarmCVelocity_; ///< A factor for multiplication of velocities
	updateRule swarmUpdateRule_; ///< Specifies how the parameters are updated

    // Gradient descent parameters
    std::size_t gdNStartingPoints_;
    float gdFiniteStep_;
    float gdStepSize_;

    //----------------------------------------------------------------------------------------------------------------
    boost::shared_ptr<GParameterSet> bestIndividual_; ///< The best individual found during an optimization
};

/**************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Go)

#endif /* GO_HPP_ */
