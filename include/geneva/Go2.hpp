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

// Geneva headers go here
#include "common/GCommonEnums.hpp"
#include "common/GExceptions.hpp"
#include "common/GFactoryT.hpp"
#include "common/GParserBuilder.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GCourtierHelperFunctions.hpp"
#include "geneva/GConsumerStore.hpp"
#include "geneva/GIndividualStandardConsumerInitializerT.hpp"
#include "geneva/GIndividualStandardConsumers.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterObjectCollection.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/OptimizerIT.hpp"
#include "geneva/GBase.hpp"
#include "geneva/ConjugateGradientDescent_Factory.hpp"
#include "geneva/EvolutionaryAlgorithm_Factory.hpp"
#include "geneva/FactoryStore.hpp"
#include "geneva/GradientDescent_Factory.hpp"
#include "geneva/NelderMead_Factory.hpp"
#include "geneva/ParameterScan_Factory.hpp"
#include "geneva/SimulatedAnnealing_Factory.hpp"
#include "geneva/SwarmAlgorithm_Factory.hpp"
#include "geneva/GenevaHelperFunctionsT.hpp"
#include "geneva/GenevaInitializer.hpp"
#include "hap/GRandomFactory.hpp"
#include "hap/GRandomT.hpp"

namespace Gem::Geneva {

/******************************************************************************/
// Default values for the variables used by the optimizer
const std::string GO2_DEF_DEFAULTCONFIGFILE{"config/Go2.json"}; // NOLINT
const bool GO2_DEF_CLIENTMODE = false;
const execMode GO2_DEF_DEFAULPARALLELIZATIONMODE = execMode::MULTITHREADED;
const bool GO2_DEF_COPYBESTONLY = true;
const std::uint16_t GO2_DEF_NPRODUCERTHREADS = 0;
const std::uint32_t GO2_DEF_OFFSET = 0;
const std::string GO2_DEF_OPTALGS{""};        // NOLINT
const std::string GO2_DEF_NOCONSUMER{"none"}; // NOLINT
const bool GO2_DEF_COPYBESTINDIVIDUALSONLY = true;

/******************************************************************************/
/** @brief Set a number of parameters of the random number factory */
void setRNFParameters(std::uint16_t);

/******************************************************************************/
/** Syntactic sugar -- make the code easier to read */
using GOABase = oa::GBase;

/******************************************************************************/
/**
 * This class allows to "chain" a number of optimization algorithms so that a given
 * set of individuals can be optimized using more than one algorithm in sequence. The
 * class also hides the details of client/server mode, consumer initialization, etc.
 */
class Go2 // NOLINT(cppcoreguidelines-special-member-functions)
  : public Interface::OptimizerIT<Go2>
  , public Gem::Common::GPtrContainerT<GParameterSet> {
public:
    /** @brief The default constructor */
    Go2() = delete;

    /** @brief A constructor that first parses the command line for relevant parameters and allows to specify a default config file name */
    
    Go2(int,
        char **,
        std::string const &,
        boost::program_options::options_description const & =
            boost::program_options::options_description());
    /** @brief Deleted copy constructor */
    Go2(Go2 const &) = delete;

    /** @brief The (defaulted) destructor */
    ~Go2() override = default;

    /** @brief Triggers execution of the client loop */
    int clientRun();
    /** @brief Checks whether this object is running in client mode */
    bool clientMode() const;

    /** @brief Specifies whether only the best individuals of a population should be copied */
    void setCopyBestIndividualsOnly(bool);
    /** @brief Checks whether only the best individuals are copied */
    bool onlyBestIndividualsAreCopied() const;

    /** @brief Allows to add an optimization algorithm to the chain */
    void addAlgorithm(const std::shared_ptr<GOABase> &);
    /** @brief Makes it easier to add algorithms */
    Go2 &operator&(const std::shared_ptr<GOABase> &);
    /** @brief Allows to add an optimization algorithm through its mnemonic */
    void addAlgorithm(std::string const &);
    /** @brief Makes it easier to add algorithms */
    Go2 &operator&(std::string const &);

    /** @brief Retrieves the currently registered number of algorithms */
    std::size_t getNAlgorithms() const;

    /** @brief Allows to register a content creator */
    void
        registerContentCreator(const std::shared_ptr<Gem::Common::GFactoryT<GParameterSet>> &);

    /***************************************************************************/
    // The following is a trivial list of getters and setters
    void setClientMode(bool);
    bool getClientMode() const;

    std::uint16_t getNProducerThreads() const;

    void setOffset(std::uint32_t);
    std::uint32_t getIterationOffset() const;

    /** @brief Loads some configuration data from arguments passed on the command line (or another char ** that is presented to it) */
    void parseCommandLine(
        int,
        char **,
        boost::program_options::options_description const & =
            boost::program_options::options_description()
    );
    /** @brief Loads some configuration data from a configuration file */
    void parseConfigFile(std::filesystem::path const &);

    /** @brief Adds local configuration options to a GParserBuilder object */
    void addConfigurationOptions(Gem::Common::GParserBuilder &);

    /***************************************************************************/
    /** @brief Allows to register a default algorithm. */
    void registerDefaultAlgorithm(const std::shared_ptr<GOABase> &);
    /** @brief Allows to register a default algorithm. */
    void registerDefaultAlgorithm(std::string const &);

    /** @brief Allows to register a pluggable optimization monitor */
    void registerPluggableOM(const std::shared_ptr<oa::GBasePluggableOM> &);
    /** @brief Allows to reset the local pluggable optimization monitor */
    void resetPluggableOM();
    /** @brief Allows to check whether pluggable optimization monitors were registered */
    bool hasOptimizationMonitors() const;

    /** @brief Allows to set the maximum running time for a client */
    void setMaxClientTime(std::chrono::duration<double> max_duration);
    /** @brief Allows to retrieve the maximum running time for a client */
    std::chrono::duration<double> getMaxClientTime() const;

    /** @brief Retrieves the algorithms that were registered with this class */
    std::vector<std::shared_ptr<GOABase>> getRegisteredAlgorithms();

    /** @brief Retrieves the name of the used consumer */
    std::string getConsumerName();

protected:
    /***************************************************************************/
    /** @brief Triggers execution of the client loop */
    virtual int clientRun_();

    /** @brief Adds local configuration options to a GParserBuilder object */
    virtual void addConfigurationOptions_(Gem::Common::GParserBuilder &);

    /** @brief Retrieves the best individual found */
    std::shared_ptr<GParameterSet> getBestGlobalIndividual_() const final;
    /** @brief Retrieves a list of the best individuals found */
    std::vector<std::shared_ptr<GParameterSet>>
    getBestGlobalIndividuals_() const final;
    /** @brief Retrieves the best individual found */
    std::shared_ptr<GParameterSet> getBestIterationIndividual_() const final;
    /** @brief Retrieves a list of the best individuals found */
    std::vector<std::shared_ptr<GParameterSet>>
    getBestIterationIndividuals_() const final;

private:
    /***************************************************************************/

    /** @brief Returns one-word information about the type of optimization algorithm. */
    std::string getAlgorithmPersonalityType_() const final;
    /** @brief Returns the name of this optimization algorithm */
    std::string getAlgorithmName_() const final;

    /** @brief Satisfies a requirement of GOptimizerIT */
    void runFitnessCalculation_() final;

    /** @brief Retrieval of the current iteration */
    uint32_t getIteration_() const final;

    /***************************************************************************/
    /** @brief Sets the number of random number production threads */
    void setNProducerThreads(std::uint16_t);

    /** @brief Perform the actual optimization cycle */
    Go2 const *optimize_(std::uint32_t) final;

    /***************************************************************************/
    // Initialization code for the Geneva library
    GenevaInitializer gi_;

    /***************************************************************************/
    // These parameters can enter the object through the constructor
    bool client_mode_ =
        GO2_DEF_CLIENTMODE; ///< Specifies whether this object represents a network client
    std::string config_filename_ =
        GO2_DEF_DEFAULTCONFIGFILE; ///< Indicates where the configuration file is stored
    std::string consumer_name_ =
        GO2_DEF_NOCONSUMER; ///< The name of a consumer requested by the user on the command line

    //---------------------------------------------------------------------------
    // Parameters for the random number generator
    std::uint16_t n_producer_threads_ =
        GO2_DEF_NPRODUCERTHREADS; ///< The number of threads that will simultaneously produce random numbers

    //---------------------------------------------------------------------------
    // Parameters for clients
    std::chrono::duration<double> max_client_duration_ = Gem::Common::duration_from_string(
        DEFAULTDURATION
    ); ///< Maximum time-frame for a client to run

    //---------------------------------------------------------------------------
    // Internal parameters
    std::uint32_t offset_ =
        GO2_DEF_OFFSET;    ///< The offset to be used when starting a new optimization run
    bool sorted_ = false; ///< Indicates whether local individuals have been sorted
    std::uint32_t iterations_consumed_ =
        0; ///< The number of successive iterations performed by this object so far
    bool copyBestIndividualsOnly_ =
        GO2_DEF_COPYBESTINDIVIDUALSONLY; ///< Indicates whether only the best individuals of an optimization run are copied to the next algorithm
    //---------------------------------------------------------------------------
    // Name and path of a checkpoint file, if supplied by the user
    std::string cp_file_ = "empty";

    //---------------------------------------------------------------------------
    // The list of "chained" optimization algorithms
    std::vector<std::shared_ptr<GOABase>> algorithms_cnt_;
    // The default algorithm (if any)
    std::shared_ptr<GOABase> default_algorithm_;
    // A string representation of the default algorithm
    const std::string default_algorithm_str_ = DEFAULTOPTALG; ///< This is the last fall-back
    // Holds an object capable of producing objects of the desired type
    std::shared_ptr<Gem::Common::GFactoryT<GParameterSet>> content_creator_ptr_;
    // A user-defined means for information retrieval
    std::vector<std::shared_ptr<oa::GBasePluggableOM>> pluggable_monitors_cnt_;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva */
