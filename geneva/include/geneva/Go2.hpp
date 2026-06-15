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
/** @brief Set a number of parameters of the random number factory */
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
  , public Gem::Common::GPtrContainerT<gpar::GOptimizableEntity> {
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

    /** @brief Supplies a custom, ready-to-use courtier broker (its consumer already registered, clone
     *  function set, and -- for networked consumers -- server started) for this run, OVERRIDING the
     *  mnemonic-based consumer selection. Used to plug in a custom consumer (e.g. a GPU consumer) that
     *  Go2 does not know how to build itself; the broker is injected into every algorithm. Call after
     *  construction and before optimize(). */
    void registerBroker(
        std::shared_ptr<Gem::Courtier::GBrokerT<gpar::GOptimizableEntity>> broker) {
        broker_ = std::move(broker);
    }

    /** @brief Retrieves the currently registered number of algorithms */
    std::size_t getNAlgorithms() const;

    /** @brief Allows to register a content creator */
    void
        registerContentCreator(const std::shared_ptr<Gem::Common::GFactoryT<gpar::GOptimizableEntity>> &);

    /***************************************************************************/
    // The following is a trivial list of getters and setters
    void setClientMode(bool);

    std::uint16_t getNProducerThreads() const;

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

    /**
     * @brief Registers an OA-owned adaption configuration for an algorithm type (Phase 8 step 4). When an
     * algorithm of the given personality type (e.g. "PERSONALITY_EA") runs in the chain, Go2 hands it this
     * config, which it adopts (after validating it matches the population's genome) instead of deriving a
     * default from the genome layout. This separates adaption authoring (OA-owned, optionally from a config
     * file) from genome authoring (the individual's factory). Passing a null config clears the entry.
     */
    void registerAdaptionConfig(
        const std::string &oa_personality_type,
        std::shared_ptr<oa::GAdaptionConfigBase> config
    );

protected:
    /***************************************************************************/
    /** @brief Triggers execution of the client loop */
    virtual int clientRun_();

    /** @brief Adds local configuration options to a GParserBuilder object */
    virtual void addConfigurationOptions_(Gem::Common::GParserBuilder &);

private:
    /***************************************************************************/

    // GOptimizerIT NVI hooks: keep these overrides private (do not
    // widen access -- matches oa::GOptimizationAlgorithmBase and the base's NVI contract).
    /** @brief Retrieves the best individual found */
    std::shared_ptr<gpar::GOptimizableEntity> getBestGlobalIndividual_() const final;
    /** @brief Retrieves a list of the best individuals found */
    std::vector<std::shared_ptr<gpar::GOptimizableEntity>>
    getBestGlobalIndividuals_() const final;
    /** @brief Retrieves the best individual found */
    std::shared_ptr<gpar::GOptimizableEntity> getBestIterationIndividual_() const final;
    /** @brief Retrieves a list of the best individuals found */
    std::vector<std::shared_ptr<gpar::GOptimizableEntity>>
    getBestIterationIndividuals_() const final;

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

    // --- optimize_ sub-steps (decomposition) ---
    /** @brief Adds the Geneva default algorithm if none have been registered */
    void ensureAlgorithmPresent();
    /** @brief Loads a checkpoint or fills the population from the content creator; returns the first algorithm's iteration offset */
    std::uint32_t prepareInitialPopulation(std::uint32_t offset);
    /** @brief Runs the registered algorithms in sequence, threading the individuals between them */
    void runAlgorithmChain(std::uint32_t first_algorithm_offset);
    /** @brief Sorts the collected individuals by their (min-only transformed) fitness */
    void sortIndividualsByFitness();

    // --- parseCommandLine sub-steps (decomposition) ---
    /** @brief Emits the help message and exits the process, if --help / --showAll was requested */
    void emitHelpIfRequested(
        boost::program_options::variables_map const &vm,
        boost::program_options::options_description const &general,
        boost::program_options::options_description const &basic,
        boost::program_options::options_description const &visible,
        boost::program_options::options_description const &user_options,
        std::string const &usage_string
    ) const;
    /** @brief Validates, initialises, configures and enrols the consumer chosen on the command line */
    void setupChosenConsumer(boost::program_options::variables_map const &vm);
    /** @brief Turns the comma-separated --optimizationAlgorithms list into algorithm objects */
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
    // courtier routing (the DEFAULT submission path). setupChosenConsumer() builds the consumer for
    // the chosen mnemonic through the shared factory buildConsumerSetup() and stores the result here;
    // runAlgorithmChain() injects broker_ into every algorithm. Consumers without a courtier form
    // yet (e.g. cuda) stay on the legacy path until ported. A custom broker can be supplied directly
    // via registerBroker() (e.g. the CUDA examples).
    /** @brief The single server-backed/local courtier broker, shared across all algorithms. Held here
     *  so its consumer (and any listening server) outlives the run and is torn down by RAII at Go2
     *  destruction. Null when no courtier routing was built (legacy fallback, or an MPI worker rank). */
    std::shared_ptr<Gem::Courtier::GBrokerT<gpar::GOptimizableEntity>> broker_;
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
    // the matching algorithm before it runs in the chain (Phase 8 step 4); empty by default, in which
    // case each adapting algorithm derives its config from the genome layout.
    std::map<std::string, std::shared_ptr<oa::GAdaptionConfigBase>> adaption_config_registry_;
    // The default algorithm (if any)
    std::shared_ptr<GOABase> default_algorithm_;
    // A string representation of the default algorithm
    const std::string default_algorithm_str_ = DEFAULTOPTALG; ///< This is the last fall-back
    // Holds an object capable of producing objects of the desired type
    std::shared_ptr<Gem::Common::GFactoryT<gpar::GOptimizableEntity>> content_creator_ptr_;
    // A user-defined means for information retrieval
    std::vector<std::shared_ptr<oa::GBasePluggableOM>> pluggable_monitors_cnt_;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva */
