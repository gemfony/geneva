/**
 * @file Go2.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <functional>
#include <mutex>

// Boost header files go here
#include <boost/algorithm/string.hpp>

#ifndef GO2_HPP_
#define GO2_HPP_

// Geneva headers go here
#include "common/GFactoryT.hpp"
#include "common/GExceptions.hpp"
#include "common/GParserBuilder.hpp"
#include "hap/GRandomFactory.hpp"
#include "hap/GRandomT.hpp"
#include "courtier/GCourtierHelperFunctions.hpp"
#include "courtier/GAsioSerialTCPConsumerT.hpp"
#include "courtier/GAsioAsyncTCPConsumerT.hpp"
#include "courtier/GStdThreadConsumerT.hpp"
#include "courtier/GSerialConsumerT.hpp"
#include "courtier/GBrokerT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GenevaHelperFunctionsT.hpp"
#include "geneva/G_Interface_Optimizer.hpp"
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
	: public G_Interface_Optimizer
	, public Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, GObject>
{
public:
	 /** @brief The default constructor */
	 G_API_GENEVA Go2() = delete;

	 /** @brief A constructor that first parses the command line for relevant parameters and allows to specify a default config file name */
	 G_API_GENEVA Go2(
		 int
		 , char **
		 , const std::string&
		 , const boost::program_options::options_description& = boost::program_options::options_description()
	 );
	 /** @brief Deleted copy constructor */
	 G_API_GENEVA Go2(const Go2&) = delete;

	 /** @brief The (defaulted) destructor */
	 G_API_GENEVA ~Go2() final = default;

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
	 G_API_GENEVA void addAlgorithm(const std::string&);
	 /** @brief Makes it easier to add algorithms */
	 G_API_GENEVA Go2& operator&(const std::string&);

	 /** @brief Retrieves the currently registered number of algorithms */
	 G_API_GENEVA std::size_t getNAlgorithms() const;

	 /** @brief Allows to register a content creator */
	 G_API_GENEVA void registerContentCreator(
		 std::shared_ptr<Gem::Common::GFactoryT<GParameterSet>>
	 );
	 /** @brief Perform the actual optimization cycle */
	 G_API_GENEVA void optimize(const std::uint32_t& = 0) final;

	 /***************************************************************************/
	 // The following is a trivial list of getters and setters
	 G_API_GENEVA void setClientMode(bool);
	 G_API_GENEVA bool getClientMode() const;

	 G_API_GENEVA std::uint16_t getNProducerThreads() const;

	 G_API_GENEVA void setOffset(const std::uint32_t&);
	 G_API_GENEVA std::uint32_t getIterationOffset() const;

	 /** @brief Retrieval of the current iteration */
	 G_API_GENEVA uint32_t getIteration() const final;

	 /** @brief Returns the name of this optimization algorithm */
	 G_API_GENEVA std::string getAlgorithmName() const final;

	 /** @brief Loads some configuration data from arguments passed on the command line (or another char ** that is presented to it) */
	 G_API_GENEVA void parseCommandLine(
		 int
		 , char **
		 , const boost::program_options::options_description& = boost::program_options::options_description()
	 );
	 /** @brief Loads some configuration data from a configuration file */
	 G_API_GENEVA void parseConfigFile(const std::string&);

	 /** @brief Adds local configuration options to a GParserBuilder object */
	 G_API_GENEVA void addConfigurationOptions(Gem::Common::GParserBuilder&);

	 /***************************************************************************/
	 /**
	  * Starts the optimization cycle and returns the best individual found, converted to
	  * the desired target type. This is a convenience overload of the corresponding
	  * GOptimizableI function.
	  *
	  * @return The best individual found during the optimization process, converted to the desired type
	  */
	 template <typename individual_type>
	 std::shared_ptr<individual_type> optimize() {
		 return G_Interface_Optimizer::optimize<individual_type>();
	 }

	 /***************************************************************************/
	 /**
	  * Starts the optimization cycle and returns the best individual found, converted to
	  * the desired target type. This function uses a configurable offset for the iteration
	  * counter. This is a convenience overload of the corresponding
	  * GOptimizableI function.
	  *
	  * @param offset An offset for the iteration counter
	  * @return The best individual found during the optimization process, converted to the desired type
	  */
	 template <typename individual_type>
	 std::shared_ptr<individual_type> optimize(
		 const std::uint32_t& offset
	 ) {
		 return G_Interface_Optimizer::optimize<individual_type>(offset);
	 }

	 /***************************************************************************/
	 /** @brief Allows to register a default algorithm. */
	 G_API_GENEVA void registerDefaultAlgorithm(std::shared_ptr<GOABase>);
	 /** @brief Allows to register a default algorithm. */
	 G_API_GENEVA void registerDefaultAlgorithm(const std::string& default_algorithm);

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
	 /**
	  * Re-implementation of a corresponding function in GStdPtrVectorInterface.
	  * Make the vector wrapper purely virtual allows the compiler to perform
	  * further optimizations.
	  */
	 void dummyFunction() final { /* nothing */ }

	 /***************************************************************************/
	 /** @brief Retrieves the best individual found */
	 G_API_GENEVA std::shared_ptr<GParameterSet> customGetBestGlobalIndividual() final;
	 /** @brief Retrieves a list of the best individuals found */
	 G_API_GENEVA std::vector<std::shared_ptr<GParameterSet>> customGetBestGlobalIndividuals() final;
	 /** @brief Retrieves the best individual found */
	 G_API_GENEVA std::shared_ptr<GParameterSet> customGetBestIterationIndividual() final;
	 /** @brief Retrieves a list of the best individuals found */
	 G_API_GENEVA std::vector<std::shared_ptr<GParameterSet>> customGetBestIterationIndividuals() final;

	 /** @brief Satisfies a requirement of G_Interface_Optimizer */
	 G_API_GENEVA void runFitnessCalculation() final;

private:
	 /***************************************************************************/
	 /** @brief Sets the number of random number production threads */
	 void setNProducerThreads(const std::uint16_t&);

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
	 std::vector<std::shared_ptr<GOABase>> m_algorithms_vec;
	 // The default algorithm (if any)
	 std::shared_ptr<GOABase> m_default_algorithm;
	 // A string representation of the default algorithm
	 const std::string m_default_algorithm_str = DEFAULTOPTALG; ///< This is the last fall-back
	 // Holds an object capable of producing objects of the desired type
	 std::shared_ptr<Gem::Common::GFactoryT<GParameterSet>> m_content_creator_ptr;
	 // A user-defined means for information retrieval
	 std::vector<std::shared_ptr<GBasePluggableOM>> m_pluggable_monitors_vec;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GO2_HPP_ */
