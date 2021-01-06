/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <functional>
#include <mutex>

// Boost header files go here
#include <boost/algorithm/string.hpp>

// Geneva headers go here
#include "common/GCommonEnums.hpp"
#include "common/GFactoryT.hpp"
#include "common/GExceptions.hpp"
#include "common/GParserBuilder.hpp"
#include "hap/GRandomFactory.hpp"
#include "hap/GRandomT.hpp"
#include "courtier/GCourtierHelperFunctions.hpp"
#include "courtier/GBrokerT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GenevaHelperFunctionsT.hpp"
#include "geneva/G_Interface_OptimizerT.hpp"
#include "geneva/G_OptimizationAlgorithm_Base.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterObjectCollection.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/G_OptimizationAlgorithm_FactoryStore.hpp"
#include "geneva/GIndividualStandardConsumers.hpp"
#include "geneva/GIndividualStandardConsumerInitializerT.hpp"
#include "geneva/GConsumerStore.hpp"
#include "geneva/GenevaInitializer.hpp"
#include "geneva/G_OptimizationAlgorithm_EvolutionaryAlgorithm_Factory.hpp"
#include "geneva/G_OptimizationAlgorithm_GradientDescent_Factory.hpp"
#include "geneva/G_OptimizationAlgorithm_ParameterScan_Factory.hpp"
#include "geneva/G_OptimizationAlgorithm_SimulatedAnnealing_Factory.hpp"
#include "geneva/G_OptimizationAlgorithm_SwarmAlgorithm_Factory.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
// Default values for the variables used by the optimizer
const std::string GO2_DEF_DEFAULTCONFIGFILE{"config/Go2.json"}; // NOLINT
const bool GO2_DEF_CLIENTMODE=false;
const execMode GO2_DEF_DEFAULPARALLELIZATIONMODE=execMode::MULTITHREADED;
const bool GO2_DEF_COPYBESTONLY=true;
const std::uint16_t GO2_DEF_NPRODUCERTHREADS=0;
const std::uint32_t GO2_DEF_OFFSET=0;
const std::string GO2_DEF_OPTALGS{""}; // NOLINT
const std::string GO2_DEF_NOCONSUMER{"none"}; // NOLINT
const bool GO2_DEF_COPYBESTINDIVIDUALSONLY=true;

/******************************************************************************/
/** @brief Set a number of parameters of the random number factory */
G_API_GENEVA void setRNFParameters(std::uint16_t);

/******************************************************************************/
/** Syntactic sugar -- make the code easier to read */
using GOABase = Gem::Geneva::G_OptimizationAlgorithm_Base;

/******************************************************************************/
/**
 * This class allows to "chain" a number of optimization algorithms so that a given
 * set of individuals can be optimized using more than one algorithm in sequence. The
 * class also hides the details of client/server mode, consumer initialization, etc.
 */
class Go2
	: public G_Interface_OptimizerT<Go2>
	, public Gem::Common::GPtrVectorT<GParameterSet, GObject>
{
public:
	 /** @brief The default constructor */
	 G_API_GENEVA Go2() = delete;

	 /** @brief A constructor that first parses the command line for relevant parameters and allows to specify a default config file name */
	 G_API_GENEVA Go2(
		 int
		 , char **
		 , std::string const &
		 , boost::program_options::options_description const & = boost::program_options::options_description()
	 );
	 /** @brief Deleted copy constructor */
	 G_API_GENEVA Go2(Go2 const &) = delete;

	 /** @brief The (defaulted) destructor */
	 G_API_GENEVA ~Go2() override = default;

	 /** @brief Triggers execution of the client loop */
	 G_API_GENEVA int clientRun();
	 /** @brief Checks whether this object is running in client mode */
	 G_API_GENEVA bool clientMode() const;

	 /** @brief Specifies whether only the best individuals of a population should be copied */
	 G_API_GENEVA void setCopyBestIndividualsOnly(bool);
	 /** @brief Checks whether only the best individuals are copied */
	 G_API_GENEVA bool onlyBestIndividualsAreCopied() const;

	 /** @brief Allows to add an optimization algorithm to the chain */
	 G_API_GENEVA void addAlgorithm(std::shared_ptr<GOABase>);
	 /** @brief Makes it easier to add algorithms */
	 G_API_GENEVA Go2& operator&(std::shared_ptr<GOABase>);
	 /** @brief Allows to add an optimization algorithm through its mnemonic */
	 G_API_GENEVA void addAlgorithm(std::string const &);
	 /** @brief Makes it easier to add algorithms */
	 G_API_GENEVA Go2& operator&(std::string const &);

	 /** @brief Retrieves the currently registered number of algorithms */
	 G_API_GENEVA std::size_t getNAlgorithms() const;

	 /** @brief Allows to register a content creator */
	 G_API_GENEVA void registerContentCreator(
		 std::shared_ptr<Gem::Common::GFactoryT<GParameterSet>>
	 );

	 /***************************************************************************/
	 // The following is a trivial list of getters and setters
	 G_API_GENEVA void setClientMode(bool);
	 G_API_GENEVA bool getClientMode() const;

	 G_API_GENEVA std::uint16_t getNProducerThreads() const;

	 G_API_GENEVA void setOffset(std::uint32_t);
	 G_API_GENEVA std::uint32_t getIterationOffset() const;

	 /** @brief Loads some configuration data from arguments passed on the command line (or another char ** that is presented to it) */
	 G_API_GENEVA void parseCommandLine(
		 int
		 , char **
		 , boost::program_options::options_description const & = boost::program_options::options_description()
	 );
	 /** @brief Loads some configuration data from a configuration file */
	 G_API_GENEVA void parseConfigFile(std::filesystem::path const &);

	 /** @brief Adds local configuration options to a GParserBuilder object */
	 G_API_GENEVA void addConfigurationOptions(Gem::Common::GParserBuilder &);

	 /***************************************************************************/
	 /** @brief Allows to register a default algorithm. */
	 G_API_GENEVA void registerDefaultAlgorithm(std::shared_ptr<GOABase>);
	 /** @brief Allows to register a default algorithm. */
	 G_API_GENEVA void registerDefaultAlgorithm(std::string const & default_algorithm);

	 /** @brief Allows to register a pluggable optimization monitor */
	 G_API_GENEVA void registerPluggableOM(std::shared_ptr<GBasePluggableOM>);
	 /** @brief Allows to reset the local pluggable optimization monitor */
	 G_API_GENEVA void resetPluggableOM();
	 /** @brief Allows to check whether pluggable optimization monitors were registered */
	 G_API_GENEVA bool hasOptimizationMonitors() const;

	 /** @brief Allows to set the maximum running time for a client */
	 G_API_GENEVA void setMaxClientTime(std::chrono::duration<double> maxDuration);
	 /** @brief Allows to retrieve the maximum running time for a client */
	 G_API_GENEVA std::chrono::duration<double> getMaxClientTime() const;

	 /** @brief Retrieves the algorithms that were registered with this class */
	 G_API_GENEVA std::vector<std::shared_ptr<GOABase>> getRegisteredAlgorithms();

protected:
	 /***************************************************************************/
	 /** @brief Retrieves the best individual found */
	 G_API_GENEVA std::shared_ptr<GParameterSet> getBestGlobalIndividual_() const final;
	 /** @brief Retrieves a list of the best individuals found */
	 G_API_GENEVA std::vector<std::shared_ptr<GParameterSet>> getBestGlobalIndividuals_() const final;
	 /** @brief Retrieves the best individual found */
	 G_API_GENEVA std::shared_ptr<GParameterSet> getBestIterationIndividual_() const final;
	 /** @brief Retrieves a list of the best individuals found */
	 G_API_GENEVA std::vector<std::shared_ptr<GParameterSet>> getBestIterationIndividuals_() const final;

private:
	 /***************************************************************************/

	 /** @brief Returns one-word information about the type of optimization algorithm. */
	 std::string getAlgorithmPersonalityType_() const final;
	 /** @brief Returns the name of this optimization algorithm */
	 G_API_GENEVA std::string getAlgorithmName_() const final;

	 /** @brief Satisfies a requirement of G_Interface_Optimizer */
	 G_API_GENEVA void runFitnessCalculation_() final;

	 /** @brief Retrieval of the current iteration */
	 G_API_GENEVA uint32_t getIteration_() const final;

	 /***************************************************************************/
	 /** @brief Sets the number of random number production threads */
	 void setNProducerThreads(std::uint16_t);

	 /** @brief Perform the actual optimization cycle */
	 G_API_GENEVA Go2 const * const optimize_(std::uint32_t) final;

	 /***************************************************************************/
	 // Initialization code for the Geneva library
	 GenevaInitializer m_gi;

	 /***************************************************************************/
	 // These parameters can enter the object through the constructor
	 bool m_client_mode = GO2_DEF_CLIENTMODE; ///< Specifies whether this object represents a network client
	 std::string m_config_filename = GO2_DEF_DEFAULTCONFIGFILE; ///< Indicates where the configuration file is stored
	 std::string m_consumer_name = GO2_DEF_NOCONSUMER; ///< The name of a consumer requested by the user on the command line

	 //---------------------------------------------------------------------------
	 // Parameters for the random number generator
	 std::uint16_t m_n_producer_threads = GO2_DEF_NPRODUCERTHREADS; ///< The number of threads that will simultaneously produce random numbers

	 //---------------------------------------------------------------------------
	 // Parameters for clients
	 std::chrono::duration<double> m_max_client_duration = Gem::Common::duration_from_string(DEFAULTDURATION); ///< Maximum time-frame for a client to run

	 //---------------------------------------------------------------------------
	 // Internal parameters
	 std::uint32_t m_offset = GO2_DEF_OFFSET; ///< The offset to be used when starting a new optimization run
	 bool m_sorted = false; ///< Indicates whether local individuals have been sorted
	 std::uint32_t m_iterations_consumed = 0; ///< The number of successive iterations performed by this object so far
	 bool m_copyBestIndividualsOnly = GO2_DEF_COPYBESTINDIVIDUALSONLY; ///< Indicates whether only the best individuals of an optimization run are copied to the next algorithm
	 //---------------------------------------------------------------------------
	 // Name and path of a checkpoint file, if supplied by the user
	 std::string m_cp_file = "empty";

	 //---------------------------------------------------------------------------
	 // The list of "chained" optimization algorithms
	 std::vector<std::shared_ptr<GOABase>> m_algorithms_cnt;
	 // The default algorithm (if any)
	 std::shared_ptr<GOABase> m_default_algorithm;
	 // A string representation of the default algorithm
	 const std::string m_default_algorithm_str = DEFAULTOPTALG; ///< This is the last fall-back
	 // Holds an object capable of producing objects of the desired type
	 std::shared_ptr<Gem::Common::GFactoryT<GParameterSet>> m_content_creator_ptr;
	 // A user-defined means for information retrieval
	 std::vector<std::shared_ptr<GBasePluggableOM>> m_pluggable_monitors_cnt;
         // Io per cpu option
         int m_ioc;
  
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

