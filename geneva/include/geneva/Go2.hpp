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
#include <map>
#include <memory>
#include <mutex>
#include <string>

// Boost header files go here
#include <boost/program_options.hpp>

// Geneva headers go here
#include "common/GCommonEnums.hpp"
#include "common/GExceptions.hpp"
#include "common/GFactoryT.hpp"
#include "common/GParserBuilder.hpp"
#include "courtier/GCourtierHelperFunctions.hpp"
#include "courtier/GConsumerRegistry.hpp"
#include "geneva/GConsumerSetup.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GSigHupHandler.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/Interface/GOptimizerIT.hpp"
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/oa/GFactoryStore.hpp"
#include "geneva/GenevaInitializer.hpp"
#include "hap/GRandomFactory.hpp"
#include "hap/GRandomT.hpp"

namespace Gem::Geneva {

/******************************************************************************/
// Default values for the variables used by the optimizer
const std::string GO2_DEF_DEFAULTCONFIGFILE{"config/Go2.json"}; // NOLINT
constexpr bool GO2_DEF_CLIENTMODE = false;
constexpr execMode GO2_DEF_DEFAULPARALLELIZATIONMODE = execMode::MULTITHREADED;
constexpr bool GO2_DEF_COPYBESTONLY = true;
constexpr std::uint16_t GO2_DEF_NPRODUCERTHREADS = 0;
const std::string GO2_DEF_OPTALGS{""};        // NOLINT
const std::string GO2_DEF_NOCONSUMER{"none"}; // NOLINT
constexpr bool GO2_DEF_COPYBESTINDIVIDUALSONLY = true;

/******************************************************************************/
/**
 * @brief Sets the number of producer threads of the random number factory.
 *
 * @param nProducerThreads The number of threads the random number factory should use to produce random numbers
 */
void setRNFParameters(std::uint16_t);

/******************************************************************************/
/** Syntactic sugar -- make the code easier to read */
using GOABase = oa::GOptimizationAlgorithmBase;

/******************************************************************************/
/**
 * This class allows to "chain" a number of optimization algorithms so that a given
 * set of individuals can be optimized using more than one algorithm in sequence. The
 * class also hides the details of client/server mode, consumer initialization, etc.
 */
class Go2 // NOLINT(cppcoreguidelines-special-member-functions)
  : public Interface::GOptimizerIT<Go2>
  , public Gem::Common::GPtrContainerT<gen::GOptimizableEntity> {
public:
    /** @brief The default constructor */
    Go2() = delete;

    /**
     * @brief A constructor that first parses the command line for relevant parameters and allows to specify a default config file name.
     *
     * @param argc The number of command line arguments
     * @param argv The array of command line argument strings
     * @param configFilePath The name and location of the configuration file
     * @param userDescriptions Additional user-defined command line options (cmp. boost::program_options); defaults to an empty set
     */
    Go2(int,
        char **,
        std::string const &,
        boost::program_options::options_description const & =
            boost::program_options::options_description());
    /** @brief Deleted copy constructor */
    Go2(Go2 const &) = delete;

    /** @brief The (defaulted) destructor */
    ~Go2() override = default;

    /**
     * @brief Triggers execution of the client loop.
     * @return An integer return value (suitable for the main function) indicating the execution status
     */
    int clientRun();
    /**
     * @brief Checks whether this object is running in client mode.
     * @return True if this object represents a network client, false otherwise
     */
    bool clientMode() const;

    /**
     * @brief Specifies whether only the best individuals of a population should be copied to the next algorithm.
     * @param copyBestOnly If true, only the best individuals are carried over between chained algorithms
     */
    void setCopyBestIndividualsOnly(bool);
    /**
     * @brief Checks whether only the best individuals are copied.
     * @return True if only the best individuals are carried over between chained algorithms
     */
    bool onlyBestIndividualsAreCopied() const;

    /**
     * @brief Allows to add an optimization algorithm to the chain.
     * @param alg A shared pointer to the optimization algorithm to append to the chain
     */
    void addAlgorithm(const std::shared_ptr<GOABase> &);
    /**
     * @brief Makes it easier to add algorithms (operator form of addAlgorithm).
     * @param alg A shared pointer to the optimization algorithm to append to the chain
     * @return A reference to this object, allowing call chaining
     */
    Go2 &operator&(const std::shared_ptr<GOABase> &);
    /**
     * @brief Allows to add an optimization algorithm through its mnemonic.
     * @param mnemonic The mnemonic string identifying the algorithm to append (e.g. "ea", "sa")
     */
    void addAlgorithm(std::string const &);
    /**
     * @brief Makes it easier to add algorithms by mnemonic (operator form of addAlgorithm).
     * @param mnemonic The mnemonic string identifying the algorithm to append (e.g. "ea", "sa")
     * @return A reference to this object, allowing call chaining
     */
    Go2 &operator&(std::string const &);

    /** @brief Supplies a custom, ready-to-use courtier consumer (clone function set, and -- for
     *  networked consumers -- server started) for this run, OVERRIDING the mnemonic-based consumer
     *  selection. Used to plug in a custom consumer (e.g. a GPU consumer) that Go2 does not know how to
     *  build itself; it becomes the process's single consumer that every algorithm submits through. Call
     *  after construction and before optimize().
     *  @param consumer The ready-to-use consumer to register as the process consumer (ownership is taken via move). */
    void registerConsumer(
        std::shared_ptr<Gem::Courtier::GBaseConsumerT<gen::GOptimizableEntity>> consumer) {
        consumer_ = consumer;
        Gem::Courtier::GConsumerRegistryT<gen::GOptimizableEntity>::instance().setConsumer(
            std::move(consumer));
    }

    /**
     * @brief Retrieves the currently registered number of algorithms.
     * @return The number of algorithms in the chain
     */
    std::size_t getNAlgorithms() const;

    /**
     * @brief Allows to register a content creator (a factory producing the individuals to be optimized).
     * @param cc A shared pointer to the factory used to fill the initial population
     */
    void
        registerContentCreator(const std::shared_ptr<Gem::Common::GFactoryT<gen::GOptimizableEntity>> &);

    /***************************************************************************/
    // The following is a trivial list of getters and setters
    /**
     * @brief Sets whether this object runs in client mode.
     * @param clientMode If true, this object represents a network client
     */
    void setClientMode(bool);

    /**
     * @brief Retrieves the number of random number production threads.
     * @return The configured number of producer threads
     */
    std::uint16_t getNProducerThreads() const;

    /**
     * @brief Loads some configuration data from arguments passed on the command line (or another char ** presented to it).
     * @param argc The number of command line arguments
     * @param argv The array of command line argument strings
     * @param userOptions Additional user-defined command line options (cmp. boost::program_options); defaults to an empty set
     */
    void parseCommandLine(
        int,
        char **,
        boost::program_options::options_description const & =
            boost::program_options::options_description()
    );
    /**
     * @brief Loads some configuration data from a configuration file.
     * @param configFile The path to the configuration file to read
     */
    void parseConfigFile(std::filesystem::path const &);

    /**
     * @brief Adds local configuration options to a GParserBuilder object.
     * @param gpb The GParserBuilder object to which configuration options should be added
     */
    void addConfigurationOptions(Gem::Common::GParserBuilder &);

    /***************************************************************************/
    /**
     * @brief Allows to register a default algorithm via a shared pointer.
     * @param alg A shared pointer to the algorithm used when no other algorithm has been registered
     */
    void registerDefaultAlgorithm(const std::shared_ptr<GOABase> &);
    /**
     * @brief Allows to register a default algorithm via its mnemonic.
     * @param mnemonic The mnemonic of the algorithm used when no other algorithm has been registered
     */
    void registerDefaultAlgorithm(std::string const &);

    /**
     * @brief Allows to register a pluggable optimization monitor.
     * @param pluggableOM A shared pointer to the pluggable optimization monitor to register
     */
    void registerPluggableOM(const std::shared_ptr<oa::GBasePluggableOM> &);
    /** @brief Allows to reset the local pluggable optimization monitors */
    void resetPluggableOM();
    /**
     * @brief Allows to check whether pluggable optimization monitors were registered.
     * @return True if at least one pluggable optimization monitor is registered
     */
    bool hasOptimizationMonitors() const;

    /**
     * @brief Allows to set the maximum running time for a client.
     * @param max_duration The maximum time frame for which a client is allowed to run
     */
    void setMaxClientTime(std::chrono::duration<double> max_duration);
    /**
     * @brief Allows to retrieve the maximum running time for a client.
     * @return The maximum time frame for which a client is allowed to run
     */
    std::chrono::duration<double> getMaxClientTime() const;

    /**
     * @brief Retrieves the algorithms that were registered with this class.
     * @return A vector of shared pointers to the registered optimization algorithms
     */
    std::vector<std::shared_ptr<GOABase>> getRegisteredAlgorithms();

    /**
     * @brief Retrieves the name of the used consumer.
     * @return The name of the consumer requested by the user
     */
    std::string getConsumerName();

    /**
     * @brief Registers an OA-owned adaption configuration for an algorithm type. When an
     * algorithm of the given personality type (e.g. "PERSONALITY_EA") runs in the chain, Go2 hands it this
     * config, which it adopts (after validating it matches the population's genome) instead of deriving a
     * default from the genome layout. This separates adaption authoring (OA-owned, optionally from a config
     * file) from genome authoring (the individual's factory). Passing a null config clears the entry.
     *
     * @param oa_personality_type The algorithm personality type the config applies to (e.g. "PERSONALITY_EA")
     * @param config The OA-owned adaption configuration to install (a null pointer clears the entry)
     */
    void registerAdaptionConfig(
        const std::string &oa_personality_type,
        std::shared_ptr<oa::GAdaptionConfigBase> config
    );

protected:
    /***************************************************************************/
    /**
     * @brief Triggers execution of the client loop.
     * @return An integer return value (suitable for the main function) indicating the execution status
     */
    virtual int clientRun_();

    /**
     * @brief Adds local configuration options to a GParserBuilder object.
     * @param gpb The GParserBuilder object to which configuration options should be added
     */
    virtual void addConfigurationOptions_(Gem::Common::GParserBuilder &);

private:
    /***************************************************************************/

    // GOptimizerIT NVI hooks: keep these overrides private (do not
    // widen access -- matches oa::GOptimizationAlgorithmBase and the base's NVI contract).
    /**
     * @brief Retrieves the best individual found globally across the whole run.
     * @return A shared pointer to the globally best individual
     */
    std::shared_ptr<gen::GOptimizableEntity> getBestGlobalIndividual_() const final;
    /**
     * @brief Retrieves a list of the globally best individuals found.
     * @return A vector of shared pointers to the globally best individuals
     */
    std::vector<std::shared_ptr<gen::GOptimizableEntity>>
    getBestGlobalIndividuals_() const final;
    /**
     * @brief Retrieves the best individual found in the current iteration.
     * @return A shared pointer to the best individual of the current iteration
     */
    std::shared_ptr<gen::GOptimizableEntity> getBestIterationIndividual_() const final;
    /**
     * @brief Retrieves a list of the best individuals found in the current iteration.
     * @return A vector of shared pointers to the best individuals of the current iteration
     */
    std::vector<std::shared_ptr<gen::GOptimizableEntity>>
    getBestIterationIndividuals_() const final;

    /**
     * @brief Returns one-word information about the type of optimization algorithm.
     * @return A short string identifying the algorithm personality type
     */
    std::string getAlgorithmPersonalityType_() const final;
    /**
     * @brief Returns the name of this optimization algorithm.
     * @return The human-readable name of this optimization algorithm
     */
    std::string getAlgorithmName_() const final;

    /** @brief Satisfies a requirement of GOptimizerIT */
    void runFitnessCalculation_() final;

    /**
     * @brief Retrieval of the current iteration.
     * @return The continuous iteration count accumulated across the algorithm chain
     */
    uint32_t getIteration_() const final;

    /***************************************************************************/
    /**
     * @brief Sets the number of random number production threads.
     * @param nProducerThreads The number of threads used to produce random numbers
     */
    void setNProducerThreads(std::uint16_t);

    /**
     * @brief Performs the actual optimization cycle.
     * @param offset The iteration offset at which to start the optimization
     * @return A pointer to this object after optimization completed
     */
    Go2 const *optimize_(std::uint32_t) final;

    // --- optimize_ sub-steps (decomposition) ---
    /** @brief Adds the Geneva default algorithm if none have been registered */
    void ensureAlgorithmPresent();
    /**
     * @brief Loads a checkpoint or fills the population from the content creator.
     * @param offset The iteration offset supplied to the optimization run
     * @return The first algorithm's iteration offset (carried over from a resumed checkpoint, or the passed-in offset)
     */
    std::uint32_t prepareInitialPopulation(std::uint32_t offset);
    /**
     * @brief Runs the registered algorithms in sequence, threading the individuals between them.
     * @param first_algorithm_offset The iteration offset at which the first algorithm in the chain starts
     */
    void runAlgorithmChain(std::uint32_t first_algorithm_offset);
    /** @brief Sorts the collected individuals by their (min-only transformed) fitness */
    void sortIndividualsByFitness();

    // --- parseCommandLine sub-steps (decomposition) ---
    /**
     * @brief Emits the help message and exits the process, if --help / --showAll was requested.
     * @param vm The parsed command line variables map
     * @param general The general options group (e.g. help-related switches)
     * @param basic The basic options group
     * @param visible The options group shown in the standard help output
     * @param user_options The additional user-supplied options group
     * @param usage_string The usage line printed at the top of the help message
     */
    void emitHelpIfRequested(
        boost::program_options::variables_map const &vm,
        boost::program_options::options_description const &general,
        boost::program_options::options_description const &basic,
        boost::program_options::options_description const &visible,
        boost::program_options::options_description const &user_options,
        std::string const &usage_string
    ) const;
    /**
     * @brief Validates, initialises, configures and enrols the consumer chosen on the command line.
     * @param vm The parsed command line variables map from which the consumer choice is read
     */
    void setupChosenConsumer(boost::program_options::variables_map const &vm);
    /**
     * @brief Turns the comma-separated --optimizationAlgorithms list into algorithm objects.
     * @param vm The parsed command line variables map (source of further per-algorithm options)
     * @param optimization_algorithms The comma-separated list of algorithm mnemonics to instantiate
     */
    void parseRequestedAlgorithms(
        boost::program_options::variables_map const &vm,
        std::string const &optimization_algorithms
    );

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
    // courtier routing (the DEFAULT submission path). setupChosenConsumer() builds the consumer for the
    // chosen mnemonic through the shared factory buildConsumerSetup(), which registers it as the
    // process's single consumer (GConsumerRegistry); every algorithm reads it from there -- no per-OA
    // injection. A custom consumer can be supplied directly via registerConsumer() (e.g. a GPU consumer).
    /** @brief The single server-backed/local courtier consumer, shared across all algorithms. Held here
     *  so it (and any listening server) outlives the run and is torn down by RAII at Go2 destruction.
     *  Null when no courtier routing was built (an MPI worker rank). */
    std::shared_ptr<Gem::Courtier::GBaseConsumerT<gen::GOptimizableEntity>> consumer_;
    /** @brief Set on a courtier MPI WORKER rank: runs the courtier worker loop (clientRun_ invokes
     *  it instead of the legacy client). Type-erased so Go2.hpp needs no MPI headers; the captured
     *  consumer shared_ptr keeps the worker node alive. Empty on master / non-MPI / legacy paths. */
    std::function<void()> mpi_run_worker_;
    /** @brief The transport-agnostic spec for the chosen consumer, assembled from the command line in
     *  setupChosenConsumer(). Held so clientRun_() can build the matching networked client through the
     *  courtier setup layer (buildConsumerClient) without re-touching the command line or the
     *  concrete consumer types. */
    ConsumerSpec consumer_spec_;

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
    bool sorted_ = false; ///< Indicates whether local individuals have been sorted
    std::uint32_t total_iterations_ =
        0; ///< Continuous iteration count accumulated across the algorithm chain (for getIteration_ reporting only)
    bool copy_best_individuals_only_ =
        GO2_DEF_COPYBESTINDIVIDUALSONLY; ///< Indicates whether only the best individuals of an optimization run are copied to the next algorithm
    //---------------------------------------------------------------------------
    // Name and path of a checkpoint file, if supplied by the user
    std::string cp_file_ = "empty";

    //---------------------------------------------------------------------------
    // The list of "chained" optimization algorithms
    std::vector<std::shared_ptr<GOABase>> algorithms_cnt_;
    // OA-owned adaption configs keyed by algorithm personality type ("PERSONALITY_EA", …). Installed on
    // the matching algorithm before it runs in the chain; empty by default, in which
    // case each adapting algorithm derives its config from the genome layout.
    std::map<std::string, std::shared_ptr<oa::GAdaptionConfigBase>> adaption_config_registry_;
    // The default algorithm (if any)
    std::shared_ptr<GOABase> default_algorithm_;
    // A string representation of the default algorithm
    const std::string default_algorithm_str_ = DEFAULTOPTALG; ///< This is the last fall-back
    // Holds an object capable of producing objects of the desired type
    std::shared_ptr<Gem::Common::GFactoryT<gen::GOptimizableEntity>> content_creator_ptr_;
    // A user-defined means for information retrieval
    std::vector<std::shared_ptr<oa::GBasePluggableOM>> pluggable_monitors_cnt_;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva */
