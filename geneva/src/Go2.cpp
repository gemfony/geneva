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

#include "geneva/Go2.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GExceptions.hpp"
#include "common/GFactoryT.hpp"
#include "common/GLogger.hpp"
#include "common/GParserBuilder.hpp"
#include "common/GProviderT.hpp"
#include "courtier/GBaseClientT.hpp" // the networked client run by clientRun_ (built via the setup layer)
// courtier consumer construction, the per-mnemonic command-line spec, the consumer option surface and
// the networked client all live in the shared setup layer (buildConsumerSetup / specFromCommandLine /
// addConsumerOptions / buildConsumerClient); Go2 no longer touches the concrete consumer
// types or the consumer store at all.
#include "geneva/GConsumerSetup.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GenevaHelperFunctions.hpp"
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/oa/GFactoryStore.hpp"
#include "geneva/ind/GIndividualPluginLoader.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "hap/GRandomFactory.hpp"
#include <boost/program_options.hpp>
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

namespace Gem::Geneva {

/******************************************************************************/
/**
 * Set a number of parameters of the random number factory
 *
 * @param n_producer_threads The number of threads simultaneously producing random numbers
 */
void setRNFParameters(std::uint16_t n_producer_threads) {
    //--------------------------------------------
    // Random numbers are our most valuable good.
    // Set the number of threads. Gem::Hap::randomFactory() is
    // a singleton that will be initialized by this call.
    Gem::Hap::randomFactory()->setNProducerThreads(n_producer_threads);
}

std::once_flag fGo2; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A constructor that first parses the command line for relevant parameters and then
 * loads data from a configuration file. The user may pass additional configuration
 * parameters. This is the only allowed constructor.
 *
 * @param argc The number of command line arguments
 * @param argv An array with the arguments
 * @param config_filename The name of a configuration file
 * @param user_descriptions A vector of additional command line options (cmp. boost::program_options)
 */
Go2::Go2(
    int argc,
    char **argv,
    std::string const &config_filename,
    boost::program_options::options_description const &user_descriptions
)
  : config_filename_(config_filename) {
    //--------------------------------------------
    // The known optimization algorithms register themselves with the global factory store at
    // library-load time (see the self-registration helpers in each factory's .cpp). Consumers are
    // built on demand by the courtier setup layer. The GenevaInitializer member gi_ performs the
    // required runtime initialization (random factory) via its constructor / destructor.

    //--------------------------------------------
    // Parse configuration file options
    this->parseConfigFile(config_filename);

    //--------------------------------------------
    // Load configuration options from the command line
    parseCommandLine(argc, argv, user_descriptions);

    //--------------------------------------------
    // If a runtime individual (optimization-problem) plugin was requested -- via the config file or the
    // command line -- load it now, before any population is built or (on a client) any work item /
    // checkpoint is deserialized. The load claims the single content-creator slot; a compiled-in
    // registerContentCreator() or a second plugin then hits the one-individual-per-process guard. Server
    // and client are the same binary launched with different options, so both load it identically here.
    if(not individual_plugin_path_.empty()) {
        this->claimContentCreator_(loadIndividualPlugin(individual_plugin_path_), individualSource::LOADED);
    }

    //--------------------------------------------
    // Random numbers are our most valuable good.
    // Initialize all necessary variables
    std::call_once(fGo2, [this]() { setRNFParameters(this->n_producer_threads_); });
}

/******************************************************************************/
/**
 * @brief Destructor. Releases the process consumer this Go2 established so a networked consumer's server
 * threads are torn down by RAII at the end of the run, instead of lingering in the process-global
 * registry until process exit. Only the consumer THIS Go2 registered is cleared (a later
 * Go2 / registerConsumer may have replaced it), and only if the registry still holds it.
 */
Go2::~Go2() {
    if(consumer_) {
        auto &registry = Gem::Courtier::GConsumerRegistryT<gen::GOptimizableEntity>::instance();
        if(registry.consumer() == consumer_) {
            registry.clear();
        }
        consumer_.reset(); // drop our reference -> RAII teardown once no one else holds it
    }
}

/******************************************************************************/
/**
 * Allows to register a default algorithm to be used when no other algorithms
 * have been specified. When others have been specified, this algorithm will
 * not be used. Note that any individuals registered with the default algorithm
 * will be copied into the Go2 object. This function takes the algorithm from a
 * global algorithm factory store. The algorithm needs to be specified using a
 * small nickname, such as "ea" for "Evolutionary Algorithms". See the available
 * algorithms in the Geneva distribution for further information.
 *
 * @param mn A small mnemonic for the optimization algorithm
 */
void Go2::registerDefaultAlgorithm(std::string const &mn) {
    // Retrieve the algorithm from the global store
    std::shared_ptr<Gem::Common::GProviderT<GOABase>> p;
    if(not oaFactoryStore()->get(mn, p)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Go2::registerDefaultAlgorithm(std::string): Error!" << '\n'
            << "Got invalid algorithm mnemonic " << mn << '\n'
        );
    }

    this->registerDefaultAlgorithm(p->provide());
}

/******************************************************************************/
/**
 * @brief Allows to register a default algorithm to be used when no other algorithms have been specified.
 *
 * When others have been specified, this algorithm will not be used. Note that any individuals registered
 * with the default algorithm will be copied into the Go2 object.
 *
 * @param default_algorithm A smart pointer to the optimization algorithm to use as the default
 */
void Go2::registerDefaultAlgorithm(const std::shared_ptr<GOABase> &default_algorithm) {
    // Check that the pointer isn't empty
    if(not default_algorithm) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Go2::registerDefaultAlgorithm(): Error!" << '\n'
            << "Got empty algorithm." << '\n'
        );
    }

    // If any individuals have been storedn in the default algorithm, we assume
    // that the user wants us to use them and copy them over. Note that these are not cloned.
    if(not default_algorithm->empty()) { // Have individuals been registered ?
        for(const auto &ind_ptr : *default_algorithm) {
            this->push_back(ind_ptr->clone_unique());
        }
        // Remove the individuals from the old algorithm
        default_algorithm->clear();
    }

    // Register the algorithm
    default_algorithm_ = default_algorithm;
}

/******************************************************************************/
/**
 * @brief Allows to register a pluggable optimization monitor.
 *
 * @param pluggable_om A smart pointer to the pluggable optimization monitor to register (must not be empty)
 */
void Go2::registerPluggableOM(const std::shared_ptr<oa::GBasePluggableOM> &pluggable_om) {
    if(pluggable_om) {
        pluggable_monitors_cnt_.push_back(pluggable_om);
    }
    else {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Go2::registerPluggableOM(): Tried to register empty pluggable optimization monitor\n"
        );
    }
}

/******************************************************************************/
/**
 * @brief Allows resetting the local pluggable optimization monitors.
 */
void Go2::resetPluggableOM() {
    pluggable_monitors_cnt_.clear();
}

/******************************************************************************/
/**
 * @brief Allows to check whether pluggable optimization monitors were registered.
 *
 * @return true if at least one pluggable optimization monitor has been registered, false otherwise
 */
bool Go2::hasOptimizationMonitors() const {
    return not pluggable_monitors_cnt_.empty();
}

/******************************************************************************/
/**
 * @brief Allows to set the maximum running time for a client.
 *
 * A duration of 0 results in no time limit being set.
 *
 * @param max_duration The maximum running time for a client (in seconds); 0 means no time limit
 */
void Go2::setMaxClientTime(std::chrono::duration<double> max_duration) {
    max_client_duration_ = max_duration;
}

/******************************************************************************/
/**
 * @brief Allows to retrieve the maximum running time for a client.
 *
 * @return The maximum running time for a client (in seconds); 0 means no time limit
 */
std::chrono::duration<double> Go2::getMaxClientTime() const {
    return max_client_duration_;
}

/******************************************************************************/
/**
 * @brief Triggers execution of the client loop.
 *
 * Note that it is up to you to terminate the program after calling this function.
 *
 * @return The exit status of the client loop (0 on normal completion)
 */
int Go2::clientRun() {
    return this->clientRun_();
}

/**
 * @brief Implementation of the client loop.
 *
 * On an MPI worker rank routed through courtier this serves work through the courtier worker node;
 * otherwise it builds the networked client for the chosen consumer and runs its processing loop.
 *
 * @return The exit status of the client loop (always 0 here)
 */
int Go2::clientRun_() {
    // On an MPI worker rank routed through courtier, serve work through the courtier worker node
    // (held type-erased from setupChosenConsumer) instead of a networked client.
    if(mpi_run_worker_) {
        mpi_run_worker_();
        return 0;
    }

    // Build the networked client for the chosen consumer through the courtier setup layer, from the
    // spec assembled in setupChosenConsumer(). The client is wire-compatible with the courtier socket
    // server. Go2 thus stays free of the concrete consumer/client types and the consumer store.
    std::shared_ptr<Gem::Courtier::GBaseClientT<gen::GOptimizableEntity>> p =
        Gem::Geneva::buildConsumerClient(consumer_spec_);

    if(not p) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Go2::clientRun(): Error!" << '\n'
            << "Consumer \"" << consumer_name_ << "\" does not provide a networked client." << '\n'
        );
    }

    // Set the maximum runtime of the client
    p->setMaxTime(this->max_client_duration_);

    // Start the actual processing loop. This call will not return until run() is finished.
    p->run();

    return 0;
}

/******************************************************************************/
/**
 * Checks whether this object is running in client mode
 *
 * @return A boolean which indicates whether the client mode has been set for this object
 */
bool Go2::clientMode() const {
    return client_mode_;
}

/******************************************************************************/
/**
 * @brief Specifies whether only the best individuals of a population should be copied.
 *
 * @param copy_best_individuals_only If true, only the best individuals are carried over between algorithms
 */
void Go2::setCopyBestIndividualsOnly(bool copy_best_individuals_only) {
    copy_best_individuals_only_ = copy_best_individuals_only;
}

/******************************************************************************/
/**
 * @brief Checks whether only the best individuals are copied.
 *
 * @return true if only the best individuals are carried over between algorithms, false otherwise
 */
bool Go2::onlyBestIndividualsAreCopied() const {
    return copy_best_individuals_only_;
}

/******************************************************************************/
/**
 * @brief Retrieves the currently registered number of algorithms.
 *
 * @return The number of optimization algorithms currently registered with this object
 */
std::size_t Go2::getNAlgorithms() const {
    return algorithms_cnt_.size();
}

/******************************************************************************/
/**
 * Allows to add an optimization algorithm to the chain. If any individuals have
 * been stored in these algorithms, Go2 will unload them and store them for later
 * usage.
 *
 * @param alg A base pointer to another optimization algorithm
 */
void Go2::addAlgorithm(const std::shared_ptr<GOABase> &alg) {
    // Check that the pointer is not empty
    if(not alg) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Go2::addAlgorithm(): Error!" << '\n'
            << "Tried to register an empty pointer" << '\n'
        );
    }

    // If any individuals have already been registered with alg, we assume
    // that the user wants us to add them to the optimization and copy them over.
    // Note that these are not cloned, as we will clear its vector anyway.
    if(not alg->empty()) { // Have individuals been registered?
        for(const auto &ind_ptr : *alg) {
            this->push_back(ind_ptr->clone_unique());
        }
        // Remove the individuals from the old algorithm
        alg->clear();
    }

    algorithms_cnt_.push_back(alg);
}

/******************************************************************************/
/**
 * Retrieves the algorithms that were registered with this class. Note that
 * this function gives you access to the actual objects used for the
 * optimization, so altering their configuration will alter the course of the
 * optimization.
 *
 * @return The algorithms that were registered with this class
 */
std::vector<std::shared_ptr<GOABase>> Go2::getRegisteredAlgorithms() {
    return algorithms_cnt_;
}

/**
 * @brief Returns the name of the consumer currently in use.
 *
 * The consumer is set using the command line argument `--consumer`.
 *
 * @return The used consumer's name
 */
std::string Go2::getConsumerName() {
    return consumer_name_;
}

/******************************************************************************/
/**
 * Makes it easier to add algorithms. The idea is to call this function like this:
 *
 * Go2 go2;
 * go2 & alg1 & alg2 & alg3;
 * go2.optimize();
 *
 * @param alg A base pointer to another optimization algorithm
 * @return A reference to this object
 */
Go2 &Go2::operator&(const std::shared_ptr<GOABase> &alg) {
    this->addAlgorithm(alg); // NOLINT
    return *this;
}

/***************************************************************************/
/**
 * @brief Allows to add an optimization algorithm through its mnemonic.
 *
 * @param mn A small mnemonic identifying the optimization algorithm in the global factory store (e.g. "ea")
 */
void Go2::addAlgorithm(std::string const &mn) {
    // Retrieve the algorithm from the global store
    std::shared_ptr<Gem::Common::GProviderT<GOABase>> p;
    if(not oaFactoryStore()->get(mn, p)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Go2::addAlgorithm(std::string): Error!" << '\n'
            << "Got invalid algorithm mnemonic " << mn << '\n'
        );
    }

    this->addAlgorithm(
        p->provide()
    ); // The provider's factory might add a monitor to the object
}

/***************************************************************************/
/**
 * @brief Makes it easier to add algorithms through their mnemonics.
 *
 * @param mn A small mnemonic identifying the optimization algorithm in the global factory store (e.g. "ea")
 * @return A reference to this object
 */
Go2 &Go2::operator&(std::string const &mn) {
    this->addAlgorithm(mn);
    return *this;
}

/***************************************************************************/
/**
 * @brief Allows to register a content creator.
 *
 * A content creator creates individuals to be added to the population.
 *
 * @param cc_ptr A smart pointer to a factory that produces optimizable entities (must not be empty)
 */
void Go2::registerContentCreator(const std::shared_ptr<Gem::Common::GFactoryT<gen::GOptimizableEntity>> &cc_ptr) {
    // A user-compiled-in individual: claim the single slot, so a later plugin load (or a second
    // registration) is refused by the one-individual-per-process guard.
    this->claimContentCreator_(cc_ptr, individualSource::COMPILED_IN);
}

/******************************************************************************/
/**
 * Claims the single content-creator slot, enforcing that exactly one optimization problem (individual)
 * exists per process: whichever source registers first wins, and any second claim -- a compiled-in
 * registration when a plugin was already loaded, or a second plugin -- throws, naming both sources.
 */
void Go2::claimContentCreator_(
    const std::shared_ptr<Gem::Common::GFactoryT<gen::GOptimizableEntity>> &cc_ptr,
    individualSource source
) {
    auto sourceStr = [](individualSource s) -> const char * {
        switch(s) {
            case individualSource::COMPILED_IN: return "compiled in (registerContentCreator)";
            case individualSource::LOADED: return "loaded from a plugin (--individual)";
            default: return "none";
        }
    };

    if(not cc_ptr) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Go2::claimContentCreator_(): Error!" << '\n'
            << "Tried to register an empty content creator (source: " << sourceStr(source) << ")" << '\n'
        );
    }

    if(content_creator_source_ != individualSource::NONE) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Go2::claimContentCreator_(): Error!" << '\n'
            << "An optimization problem (individual) is already provided (" << sourceStr(content_creator_source_)
            << ");" << '\n'
            << "refusing to add another (" << sourceStr(source) << ")." << '\n'
            << "Exactly one individual may exist per process -- compile one in OR load one, not both, and"
            << '\n'
            << "never two. (If you compiled an individual in, do not also pass --individual.)" << '\n'
        );
    }

    content_creator_ptr_ = cc_ptr;
    content_creator_source_ = source;
}

/******************************************************************************/
/**
 * Perform the actual optimization cycle. Note that we assume that individuals
 * have either been registered with the Go2 object or with the first algorithm
 * which has been added to the object. When no algorithm was added to the Go2
 * object (either on the command line or by passing a mnemonic or smart pointer),
 * a default algorithm will be used. Check the @c \#define @c DEFAULTOPTALG for information
 * on the type of algorithm being used. The default algorithm may also be altered
 * by the user.
 *
 * @param offset An iteration offset at which the first algorithm should start
 *               (e.g. for checkpoint resume); subsequent algorithms in the chain
 *               always start at 0. Defaults to 0 through the GOptimizerIT interface.
 * @return A pointer to this object (after the optimization has run)
 */
Go2 const *Go2::optimize_(std::uint32_t offset) {
    this->ensureGPUConsumerBuilt(); // build the gpu consumer (if selected) now the marshaller is available
    this->ensureAlgorithmPresent();
    std::uint32_t const first_algorithm_offset = this->prepareInitialPopulation(offset);
    this->runAlgorithmChain(first_algorithm_offset);
    this->sortIndividualsByFitness();
    return this;
}

/******************************************************************************/
/**
 * @brief Adds the Geneva default algorithm if the user has registered none.
 */
void Go2::ensureAlgorithmPresent() {
    if(algorithms_cnt_.empty()) {
        if(not default_algorithm_) {
            // No algorithms given, no default algorithm specified by the user:
            // Simply add the Geneva-side default algorithm
            this->registerDefaultAlgorithm(default_algorithm_str_);

            glogger << "In Go2::optimize(): INFORMATION:" << '\n'
                    << "No user-defined optimization algorithm available." << '\n'
                    << "Using default algorithm \"" << default_algorithm_str_ << "\" instead."
                    << '\n'
                    << GLOGGING;
        }

        algorithms_cnt_.push_back(default_algorithm_->clone<GOABase>());
    }
}

/******************************************************************************/
/**
 * @brief Loads a checkpoint into the first algorithm, or fills the population from the content creator.
 *
 * Returns the iteration offset for the FIRST algorithm only (a checkpoint resume overrides the
 * passed-in offset); every subsequent algorithm in the chain starts at iteration 0 so it gets its
 * full iteration budget -- otherwise a chained algorithm would inherit the previous one's end
 * iteration and, with an absolute max-iteration halt criterion, stop immediately.
 *
 * @param offset The iteration offset requested for the first algorithm
 * @return The iteration offset the first algorithm should actually start at (offset, or one past the
 *         checkpoint's last iteration when resuming from a checkpoint)
 */
std::uint32_t Go2::prepareInitialPopulation(std::uint32_t offset) {
    // Check whether a possible checkpoint file fits the first algorithm in the chain
    if(cp_file_ != "empty" &&
       not algorithms_cnt_[0]->cp_personality_fits(std::filesystem::path(cp_file_))) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Go2::optimize(): Error!" << '\n'
            << "Checkpoint file " << cp_file_ << " does not" << '\n'
            << "fit requirements of first algorithm "
            << algorithms_cnt_[0]->getAlgorithmPersonalityType() << '\n'
        );
    }

    std::uint32_t first_algorithm_offset = offset;

    // Load the checkpoint file or create individuals from the content creator
    if(cp_file_ != "empty") {
        // Load the external data
        algorithms_cnt_[0]->loadCheckpoint(std::filesystem::path(cp_file_));

        // Make sure the first algorithm starts right after the iteration where the checkpoint file ended
        first_algorithm_offset = algorithms_cnt_[0]->getIteration() + 1;
    }
    else {
        // Check that individuals have been registered
        if(this->empty()) {
            if(content_creator_ptr_) {
                for(std::size_t ind = 0; ind < algorithms_cnt_.at(0)->getDefaultPopulationSize();
                    ind++) {
                    std::shared_ptr<gen::GOptimizableEntity> p_ind = (*content_creator_ptr_)();
                    if(p_ind) {
                        this->push_back(p_ind);
                    }
                    else {                  // No valid item received, the factory has run empty
                        if(this->empty()) { // Still empty?
                            throw geneva_exception(
                                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                                << "In Go2::optimize(): Error!" << '\n'
                                << "The content creator did not deliver any individuals"
                                << '\n'
                                << "and none have been registered so far." << '\n'
                                << "No way to continue." << '\n'
                            );
                        }
                        break;
                    }
                }
            }
            else {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In Go2::optimize(): Error!" << '\n'
                    << "No optimization problem (individual) is available: no content creator and no"
                    << '\n'
                    << "individuals have been registered. Provide exactly one of:" << '\n'
                    << "  (1) compile an individual in and call registerContentCreator();" << '\n'
                    << "  (2) add individuals directly via push_back();" << '\n'
                    << "  (3) load one at runtime with --individual <path>.so (or the" << '\n'
                    << "      individual_plugin_path config-file setting)." << '\n'
                );
            }
        }
    }

    return first_algorithm_offset;
}

/******************************************************************************/
/**
 * @brief Runs the registered algorithms in sequence, threading the individuals from one algorithm to the next.
 *
 * Only the first algorithm honours the offset (checkpoint resume / user offset); subsequent algorithms
 * start at 0 (full iteration budget).
 *
 * @param first_algorithm_offset The iteration offset at which the first algorithm in the chain should start
 */
void Go2::runAlgorithmChain(std::uint32_t first_algorithm_offset) {
    total_iterations_ = 0;
    sorted_           = false;
    bool is_first_algorithm = true;
    for(const auto &alg_ptr : algorithms_cnt_) {
        // No per-algorithm broker injection: every algorithm submits through the one process consumer
        // (registered in GConsumerRegistry by setupChosenConsumer / registerConsumer). A standalone
        // algorithm with none set builds a default local consumer on first use.

        // Add the pluggable optimization monitors to the algorithm
        for(auto const &pm_ptr : pluggable_monitors_cnt_) {
            alg_ptr->registerPluggableOM(pm_ptr);
        }

        // Add the individuals to the algorithm
        for(const auto &ind_ptr : *this) {
            alg_ptr->push_back(ind_ptr->clone_unique());
        }

        // Remove our local copies
        this->clear();

        // If an OA-owned adaption config was registered for this algorithm's type, hand
        // it over now (before it runs). The algorithm adopts it -- validating it against the population's
        // genome -- in place of deriving a default from the genome layout.
        if(const auto it = adaption_config_registry_.find(alg_ptr->getAlgorithmPersonalityType());
           it != adaption_config_registry_.end()) {
            alg_ptr->setAdaptionConfig(it->second);
        }

        // Do the actual optimization (see first_algorithm_offset above)
        if(is_first_algorithm) {
            alg_ptr->optimize(first_algorithm_offset);
            is_first_algorithm = false;
        }
        else {
            alg_ptr->optimize();
        }

        // Accumulate a continuous iteration count for reporting (getIteration_) only,
        // WITHOUT feeding it back as the next algorithm's start offset.
        total_iterations_ += alg_ptr->getIteration();

        // Unload the individuals from the last algorithm and store them again in this object
        if(copy_best_individuals_only_) {
            for(const auto &best_ind_ptr : alg_ptr->getBestGlobalIndividuals<gen::GOptimizableEntity>()) {
                this->push_back(best_ind_ptr);
            }
        }
        else { // copy all individuals
            for(const auto &ind_ptr : *alg_ptr) {
                this->push_back(ind_ptr->clone_unique());
            }
        }

        alg_ptr->clear();            // Get rid of local individuals in the algorithm
        alg_ptr->resetPluggableOM(); // Get rid of the algorithm's pluggable optimization monitors
    }
}

/******************************************************************************/
/**
 * @brief Registers an OA-owned adaption configuration for an algorithm type.
 *
 * Stored keyed by the algorithm's personality type and handed to the matching algorithm in
 * runAlgorithmChain() before it runs. A null config removes any existing entry.
 *
 * @param oa_personality_type The algorithm personality type the configuration applies to (registry key)
 * @param config A smart pointer to the adaption configuration to register; a null pointer removes any existing entry
 */
void Go2::registerAdaptionConfig(
    const std::string &oa_personality_type,
    std::shared_ptr<oa::GAdaptionConfigBase> config
) {
    if(config) {
        adaption_config_registry_[oa_personality_type] = std::move(config);
    }
    else {
        adaption_config_registry_.erase(oa_personality_type);
    }
}

/******************************************************************************/
/**
 * @brief Sorts the collected individuals by their (min-only transformed) fitness.
 *
 * Sorting makes the best individuals easy to extract afterwards.
 */
void Go2::sortIndividualsByFitness() {
    std::ranges::sort(
        this->begin(),
        this->end(),
        [](const auto &x_ptr, const auto &y_ptr) -> bool {
            return minOnly_transformed_fitness(*x_ptr) < minOnly_transformed_fitness(*y_ptr);
        }
    );

    sorted_ = true;
}

/******************************************************************************/
/**
 * @brief Retrieves the best individual found.
 *
 * This function returns a base pointer. Conversion is done through the clone<>() template in GCommonInterfaceT.
 *
 * @return The best individual found
 */
std::shared_ptr<gen::GOptimizableEntity> Go2::getBestGlobalIndividual_() const {
    // Do some error checking
    if(this->empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Go2::getBestGlobalIndividual_(): Error!" << '\n'
            << "No individuals found" << '\n'
        );
    }

    if(not sorted_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Go2::getBestGlobalIndividual_(): Error!" << '\n'
            << "Tried to retrieve best individual" << '\n'
            << "from an unsorted population." << '\n'
        );
    }

    // Check if the best individual is processed
    if(not this->front()->is_processed() && not this->front()->is_unprocessed()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Go2::getBestGlobalIndividual_(): Error!" << '\n'
            << "Best individual is unprocessed or has errors" << '\n'
        );
    }

    // Simply return the best individual. This will result in an implicit downcast
    return this->front()->clone<gen::GOptimizableEntity>();
}

/******************************************************************************/
/**
 * @brief Retrieves a list of the best individuals found.
 *
 * This function returns base pointers. Conversion is done through the clone<>() template in GCommonInterfaceT.
 *
 * @return A vector holding the best individuals found
 */
std::vector<std::shared_ptr<gen::GOptimizableEntity>> Go2::getBestGlobalIndividuals_() const {
    // Do some error checking
    if(this->empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Go2::getBestGlobalIndividuals_(): Error!" << '\n'
            << "No individuals found" << '\n'
        );
    }

    std::size_t pos = 0;
    std::vector<std::shared_ptr<gen::GOptimizableEntity>> best_individuals;
    for(const auto &ind_ptr : *this) {
        if(ind_ptr->is_due_for_processing() || ind_ptr->has_errors()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In Go2::getBestGlobalIndividuals_(): Error!" << '\n'
                << "Found individual in position " << pos
                << " which is unprocessed or which has errors" << '\n'
            );
        }

        // This will result in an implicit downcast
        best_individuals.push_back(ind_ptr->clone<gen::GOptimizableEntity>());

        pos++;
    }

    return best_individuals;
}

/******************************************************************************/
/**
 * @brief Retrieves the best individual of the current iteration.
 *
 * This function is not meaningful for the algorithm-combiner Go2 and always throws when called.
 *
 * @return Never returns normally; always throws a geneva_exception
 */
std::shared_ptr<gen::GOptimizableEntity> Go2::getBestIterationIndividual_() const {
    throw geneva_exception(
        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
        << "In Go2::getBestIterationIndividual_(): Error!" << '\n'
        << "This function should not be called" << '\n'
    );
}

/******************************************************************************/
/**
 * @brief Retrieves a list of the best individuals of the current iteration.
 *
 * This function is not meaningful for the algorithm-combiner Go2 and always throws when called.
 *
 * @return Never returns normally; always throws a geneva_exception
 */
std::vector<std::shared_ptr<gen::GOptimizableEntity>> Go2::getBestIterationIndividuals_() const {
    throw geneva_exception(
        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
        << "In Go2::getBestIterationIndividuals_(): Error!" << '\n'
        << "This function should not be called" << '\n'
    );
}

/******************************************************************************/
/**
 * @brief Satisfies a requirement of GOptimizerIT (no-op for the algorithm combiner).
 */
void Go2::runFitnessCalculation_() { /* nothing */
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void Go2::addConfigurationOptions(Gem::Common::GParserBuilder &gpb) {
    this->addConfigurationOptions_(gpb);
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void Go2::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
    using namespace Gem::Common;

    // Add local data only -- no options from parent classes
    gpb.registerFileParameter<std::uint16_t>(
        "n_producer_threads",
        GO2_DEF_NPRODUCERTHREADS,
        [this](std::uint16_t npt) { this->setNProducerThreads(npt); }
    ) << "The number of threads simultaneously producing random numbers";

    gpb.registerFileParameter<bool>(
        "copy_best_individuals_only",
        GO2_DEF_COPYBESTINDIVIDUALSONLY,
        [this](bool copy_best_individuals_only) {
            this->setCopyBestIndividualsOnly(copy_best_individuals_only);
        }
    ) << "Indicates whether only the best individuals should be copied when"
      << '\n'
      << "switching from one optimization algorithm to the next";

    gpb.registerFileParameter<std::string>(
        "individual_plugin_path",
        std::string(),
        [this](std::string const &p) { individual_plugin_path_ = p; }
    ) << "Filesystem path to a runtime individual (optimization-problem) plugin (.so) to load at startup."
      << '\n'
      << "Empty (the default) means the individual is compiled into this binary. The --individual"
      << '\n'
      << "command-line option overrides this setting.";
}

/******************************************************************************/
/**
 * Allows marking this object as belonging to a client as opposed to a server
 *
 * @param client_mode Allows marking this object as belonging to a client as opposed to a server
 */
void Go2::setClientMode(bool client_mode) {
    // Note: a consumer that determines its client/server role autonomously (the MPI consumer, from its
    // process rank) overrides this request during command-line parsing -- see setupChosenConsumer(),
    // which derives client_mode_ from the courtier setup result for mpi.
    client_mode_ = client_mode;
}

/******************************************************************************/
/**
 * Allows to set the number of threads that will simultaneously produce random numbers.
 *
 * @param n_producer_threads The number of threads that will simultaneously produce random numbers
 */
void Go2::setNProducerThreads(std::uint16_t n_producer_threads) {
    n_producer_threads_ = n_producer_threads;
}

/******************************************************************************/
/**
 * Allows to retrieve the number of threads that will simultaneously produce random numbers.
 *
 * @return The number of threads that will simultaneously produce random numbers
 */
std::uint16_t Go2::getNProducerThreads() const {
    return n_producer_threads_;
}

/******************************************************************************/
/**
 * @brief Retrieval of the current iteration.
 *
 * @return The accumulated iteration count across all algorithms run so far (for reporting only)
 */
uint32_t Go2::getIteration_() const {
    return total_iterations_;
}

/******************************************************************************/
/**
 * Returns the name of this optimization algorithm
 *
 * @return The name assigned to this optimization algorithm
 */
std::string Go2::getAlgorithmName_() const {
    return {"Algorithm Combiner"};
}

/******************************************************************************/
/** @brief Returns one-word information about the type of optimization algorithm. */
std::string Go2::getAlgorithmPersonalityType_() const {
    return {"PERSONALITY_NONE"};
}

/******************************************************************************/
/******************************************************************************/
namespace {

/**
 * @brief Builds a "mnemonic:  human-readable-name" listing of all entries in a store.
 *
 * @tparam StorePtr The (pointer-like) type of the store being iterated
 * @param store The store whose registered keys and human-readable names should be listed
 * @return A newline-separated listing of "mnemonic:  name" for every entry in the store
 */
template <typename StorePtr>
std::string listMnemonics(StorePtr store) {
    std::vector<std::string> keys;
    store->getKeyVector(keys);
    std::string result;
    for(auto const &key : keys) {
        result += key + ":  " + store->get(key)->getName() + "\n";
    }
    return result;
}

} // anonymous namespace

/******************************************************************************/
/**
 * @brief Parses the command line for the algorithms, consumer, checkpoint file and client options.
 *
 * @param argc The number of command line arguments
 * @param argv An array with the arguments
 * @param user_options A program_options object for user-defined command line options
 */
void Go2::parseCommandLine(
    int argc,
    char **argv,
    boost::program_options::options_description const &user_options
) {
    namespace po = boost::program_options;

    try {
        std::string max_client_duration = EMPTYDURATION; // 00:00:00

        std::string optimization_algorithms; // NOLINT(cppcoreguidelines-init-variables)
        std::string checkpoint_file = "empty";

        // Help texts listing the registered algorithms / consumers
        std::ostringstream oa_help; // NOLINT(cppcoreguidelines-init-variables)
        oa_help << "A comma-separated list of optimization algorithms, e.g. \"arg1,arg2\". "
                << oaFactoryStore()->size() << " algorithms have been registered: " << '\n'
                << listMnemonics(oaFactoryStore());

        std::ostringstream consumer_help; // NOLINT(cppcoreguidelines-init-variables)
        consumer_help << "The name of a consumer for brokered execution (an error will be flagged "
                         "if called with any other execution mode than (2) ). "
                      << Gem::Geneva::consumerCount() << " consumers are available: " << '\n'
                      << Gem::Geneva::consumerListing();

        auto usage_string = std::string("Usage: ") + argv[0] + " [options]";

        boost::program_options::options_description general(usage_string);
        boost::program_options::options_description basic("Basic options");

        // First add local options
        basic.add_options()
				("help,h", "Emit help message")
				("showAll", "Show all available options")
				("optimizationAlgorithms,a", po::value<std::string>(&optimization_algorithms), oa_help.str().c_str())
				("cp_file,f", po::value<std::string>(&checkpoint_file)->default_value("empty"),
				 "A file (including its path) holding a checkpoint for a given optimization algorithm")
				("client", "Indicates that this program should run as a client or in server mode. Note that this setting will trigger an error unless called in conjunction with a consumer capable of dealing with clients. This option is ignored when working with the mpi consumer, because the mpi consumer will configure itself to be a client or server depending on its rank.")
				("max_client_duration", po::value<std::string>(&max_client_duration)->default_value(EMPTYDURATION),
				 R"(The maximum runtime for a client in the form "hh:mm:ss". Note that a client may run longer as this time-frame if its work load still runs. The default value "00:00:00" means: "no time limit")")
				("consumer,c", po::value<std::string>(&consumer_name_)->default_value("stc"), consumer_help.str().c_str())
				("individual,i", po::value<std::string>(&individual_plugin_path_),
				 "Filesystem path to a runtime individual (optimization-problem) plugin (.so) to load at "
				 "startup. Overrides the individual_plugin_path config-file setting. Omit it to use an "
				 "individual compiled into this binary.");

        // Add additional options coming from the algorithms and consumers
        boost::program_options::options_description visible(
            "Global algorithm- and consumer-options"
        );
        boost::program_options::options_description hidden(
            "Hidden algorithm- and consumer-options"
        );

        // Register the consumer command-line options through the courtier setup layer (the single
        // owner of the consumer option surface) -- no consumer store iteration.
        Gem::Geneva::addConsumerOptions(visible, hidden);

        // Retrieve available command-line options from registered optimization
        // algorithm factories, if any (same snapshot pattern).
        if(not oaFactoryStore()->empty()) {
            for(auto const &factory : oaFactoryStore()->getContentSnapshot()) {
                factory->addCLOptions(visible, hidden);
            }
        }

        // Add the other options to "general"
        if(user_options.options().empty()) {
            general.add(basic).add(visible).add(hidden);
        }
        else {
            general.add(basic).add(user_options).add(visible).add(hidden);
        }

        // Do the actual parsing of the command line
        po::variables_map vm;
        po::store(
            po::parse_command_line<char>(argc, static_cast<const char *const *>(argv), general),
            vm
        );

        // Emit a help message and exit the process, if requested
        this->emitHelpIfRequested(vm, general, basic, visible, user_options, usage_string);

        po::notify(vm);

        if(vm.contains("client")) {
            client_mode_ = true;
        }

        // Validate, configure and enrol the consumer chosen on the command line
        this->setupChosenConsumer(vm);

        // Turn the requested algorithm mnemonics into algorithm objects
        this->parseRequestedAlgorithms(vm, optimization_algorithms);

        // Set the name of a checkpoint file (if any)
        cp_file_ = checkpoint_file;

        // Set the maximum running time for the client (if any)
        max_client_duration_ = Gem::Common::duration_from_string(max_client_duration);
    }
    catch(const po::error &e) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "Error parsing the command line:" << '\n'
            << e.what() << '\n'
        );
    }
}

/******************************************************************************/
/**
 * @brief Emits the help message and exits the process, if --help / --showAll was given.
 *
 * @param vm The parsed program_options variables map (checked for the "help" / "showAll" flags)
 * @param general The full options description (printed for --showAll)
 * @param basic The basic Go2 options (included in the selected --help output)
 * @param visible The visible algorithm- and consumer-options (included in the selected --help output)
 * @param user_options The user-defined command line options (included if non-empty)
 * @param usage_string The usage banner string used as the heading of the selected options listing
 */
void Go2::emitHelpIfRequested(
    boost::program_options::variables_map const &vm,
    boost::program_options::options_description const &general,
    boost::program_options::options_description const &basic,
    boost::program_options::options_description const &visible,
    boost::program_options::options_description const &user_options,
    std::string const &usage_string
) {
    if(not vm.contains("help") && not vm.contains("showAll")) {
        return;
    }
    if(vm.contains("showAll")) { // Show all options
        std::cout << general << '\n';
    }
    else { // Just show a selection
        boost::program_options::options_description selected(usage_string);
        if(user_options.options().empty()) {
            selected.add(basic).add(visible);
        }
        else {
            selected.add(basic).add(user_options).add(visible);
        }
        std::cout << selected << '\n';
    }
    exit(0); // NOLINT(concurrency-mt-unsafe) — --help path; no worker threads have started yet
}

/******************************************************************************/
/**
 * @brief Registers a ready-built custom consumer as the process's single consumer, overriding the
 * mnemonic-based default that setupChosenConsumer() established during construction.
 *
 * setupChosenConsumer() already announced and registered the mnemonic-based default (e.g. "stc"); this
 * replaces it in the GConsumerRegistry. We log the replacement so the earlier "Using consumer <name>"
 * line is not mistaken for the consumer that actually runs the work (e.g. a GPU consumer).
 *
 * @param consumer The ready-to-use consumer to register as the process consumer (ownership is moved in)
 */
void Go2::registerConsumer(
    std::shared_ptr<Gem::Courtier::GBaseConsumerT<gen::GOptimizableEntity>> consumer) {
    consumer_ = consumer;
    Gem::Courtier::GConsumerRegistryT<gen::GOptimizableEntity>::instance().setConsumer(
        std::move(consumer));
    std::cout << "Using a custom registered consumer; it replaces the default \"" << consumer_name_
              << "\" as the process consumer\n";
}

/******************************************************************************/
/**
 * @brief Registers the problem's GPU consumer builder; see the header for the rationale. The closure is
 * stored and invoked lazily by ensureGPUConsumerBuilt() at the start of optimize_(), only when the gpu
 * mnemonic is selected.
 *
 * @param builder A closure returning the ready-to-use GPU consumer (as the courtier base pointer).
 */
void Go2::registerGPUConsumerBuilder(
    std::move_only_function<std::shared_ptr<Gem::Courtier::GBaseConsumerT<gen::GOptimizableEntity>>()> builder) {
    gpu_consumer_builder_ = std::move(builder);
}

/******************************************************************************/
/**
 * @brief Builds + registers the GPU consumer from the registered builder when the gpu mnemonic is
 * selected. A no-op for every other consumer. Deferred to optimize_() (rather than the construction-time
 * setupChosenConsumer()) so the problem's marshaller / data are already available. The GPU consumer is
 * local (no client), so this runs on the single optimization process.
 */
void Go2::ensureGPUConsumerBuilt() {
    if(consumer_name_ != "gpu" || consumer_) {
        return; // not a GPU run, or already built
    }
    if(not gpu_consumer_builder_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Go2::ensureGPUConsumerBuilt(): Error!" << '\n'
            << "Consumer \"gpu\" was selected, but no GPU consumer builder was registered." << '\n'
            << "Call go.registerGPUConsumerBuilder(...) (with the problem's device marshaller) after" << '\n'
            << "constructing Go2 and before optimize()." << '\n'
        );
    }
    consumer_ = gpu_consumer_builder_();
    if(not consumer_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Go2::ensureGPUConsumerBuilt(): Error!" << '\n'
            << "The registered GPU consumer builder returned a null consumer." << '\n'
        );
    }
    Gem::Courtier::GConsumerRegistryT<gen::GOptimizableEntity>::instance().setConsumer(consumer_);
    std::cout << "Routing consumer \"gpu\" through courtier (problem-registered builder)\n";
}

/******************************************************************************/
/**
 * @brief Validates, initialises, configures and enrols the consumer chosen on the command line.
 *
 * Operates on the consumer_name_ member; assembles the transport-agnostic consumer spec, builds the
 * courtier consumer/broker (except on an MPI worker rank) and derives client_mode_ for the MPI consumer.
 *
 * @param vm The parsed program_options variables map (used to assemble the consumer spec)
 */
void Go2::setupChosenConsumer(boost::program_options::variables_map const &vm) {
    // No consumer specified, although brokered execution was requested
    if(vm.count("consumer") != 1) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Go2::setupChosenConsumer(): Error!" << '\n'
            << "You need to specify exactly one consumer for brokered execution," << '\n'
            << "on the command line. Found " << vm.count("consumer") << "." << '\n'
        );
    }

    // Check that the requested consumer is one the courtier setup layer can build.
    if(not Gem::Geneva::isKnownConsumer(consumer_name_)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Go2::setupChosenConsumer(): Error!" << '\n'
            << "You have requested an unknown consumer \"" << consumer_name_ << "\"." << '\n'
            << "Available consumers are:" << '\n'
            << Gem::Geneva::consumerListing()
        );
    }

    // Client mode requires a consumer with a networked client form (asio/beast/mpi). For mpi the role
    // is fixed by the rank rather than --client, so it is exempt from this up-front check.
    if(client_mode_ && consumer_name_ != "mpi"
       && not Gem::Geneva::consumerNeedsClient(consumer_name_)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Go2::setupChosenConsumer(): Error!" << '\n'
            << "Requested client mode even though consumer " << consumer_name_
            << " does not require a client" << '\n'
        );
    }

    std::cout << "Using consumer " << consumer_name_ << '\n';

    // The GPU consumer is built from a problem-registered builder closure (it owns the device marshaller,
    // the scalar type and the kernel/backend config -- pieces Go2 cannot supply). Those are only available
    // after construction, whereas this runs during construction, so defer the build to optimize() via
    // ensureGPUConsumerBuilt(). Nothing to assemble here (gpu is a local consumer, no networked client).
    if(consumer_name_ == "gpu") {
        return;
    }

    // courtier is the submission path. Assemble the transport-agnostic spec from the command line and
    // remember it, so clientRun_() can build the matching networked client without a second pass.
    consumer_spec_ = Gem::Geneva::specFromCommandLine(consumer_name_, vm);

    // Build the courtier consumer through the shared factory -- the single place that knows the
    // concrete consumer types -- which registers the resulting consumer as the process's single consumer
    // (GConsumerRegistry); every algorithm reads it from there. MPI builds on EVERY rank (the consumer
    // self-determines master/worker from its process rank: master -> consumer, worker -> run_worker); the
    // socket and local consumers build a server only when this process is not a client.
    if(consumer_name_ == "mpi" || not client_mode_) {
        auto setup = Gem::Geneva::buildConsumerSetup(consumer_spec_);
        consumer_       = setup.consumer;   // also registered as the process consumer (null on an MPI worker)
        mpi_run_worker_ = std::move(setup.run_worker); // MPI worker rank: clientRun_ serves through it

        // MPI fixes the client/server role by rank: a worker rank yields a run_worker loop (and a null
        // consumer). Reflect that in client_mode_ so the caller dispatches to clientRun_().
        if(consumer_name_ == "mpi") {
            client_mode_ = static_cast<bool>(mpi_run_worker_);
        }

        if(consumer_) {
            std::cout << "Routing consumer \"" << consumer_name_ << "\" through courtier\n";
        }
    }
}

/******************************************************************************/
/**
 * @brief Turns the comma-separated --optimizationAlgorithms list into algorithm objects.
 *
 * @param vm The parsed program_options variables map (checked for the "optimizationAlgorithms" flag)
 * @param optimization_algorithms The comma-separated list of algorithm mnemonics to instantiate
 */
void Go2::parseRequestedAlgorithms(
    boost::program_options::variables_map const &vm,
    std::string const &optimization_algorithms
) {
    if(not vm.contains("optimizationAlgorithms")) {
        return;
    }

    std::vector<std::string> const algs = Gem::Common::splitString(optimization_algorithms, ",");
    for(const auto &alg_str : algs) {
        // Retrieve the algorithm provider from the global store
        std::shared_ptr<Gem::Common::GProviderT<GOABase>> p;
        if(not oaFactoryStore()->get(alg_str, p)) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In Go2::parseRequestedAlgorithms(): Error!" << '\n'
                << "Got invalid algorithm mnemonic \"" << alg_str << "\"." << '\n'
                << "No algorithm found for this string." << '\n'
            );
        }
        // Retrieve an algorithm from the provider and add it to the list
        algorithms_cnt_.push_back(p->provide());
    }
}

/******************************************************************************/
/**
 * Parses a configuration file for configuration options
 *
 * @param config_filename The name of a configuration file to be parsed
 */
void Go2::parseConfigFile(std::filesystem::path const &config_filename) {
    // Create a parser builder object. It will be destroyed at
    // the end of this scope and thus cannot cause trouble
    // due to registered call-backs and references
    Gem::Common::GParserBuilder gpb;

    // Add local configuration options
    this->addConfigurationOptions(gpb);

    // Do the actual parsing
    if(not gpb.parseConfigFile(config_filename)) {
        glogger << "In Go2::parseConfigFile: Error!" << '\n'
                << "Could not parse configuration file " << config_filename.string() << '\n'
                << LOGEXIT(EXIT_FAILURE);
    }
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva */
