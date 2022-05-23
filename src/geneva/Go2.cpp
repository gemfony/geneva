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

#include "geneva/Go2.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * Set a number of parameters of the random number factory
 *
 * @param nProducerThreads The number of threads simultaneously producing random numbers
 */
void setRNFParameters(
	std::uint16_t nProducerThreads
) {
	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Set the number of threads. GRANDOMFACTORY is
	// a singleton that will be initialized by this call.
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);
}

// Regulates access to the call_once facility
std::once_flag f_go2;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A constructor that first parses the command line for relevant parameters and then
 * loads data from a configuration file. Additional configuration parameters may
 * be passed by the user. This is the only allowed constructor.
 *
 * @param argc The number of command line arguments
 * @param argv An array with the arguments
 * @param configFilename The name of a configuration file
 * @param od A vector of additional command line options (cmp. boost::program_options)
 */
Go2::Go2(
	int argc, char **argv
	, std::string const & configFilename
	, boost::program_options::options_description const & userDescriptions
)
	: G_Interface_OptimizerT<Go2>()
    , Gem::Common::GPtrVectorT<GParameterSet, GObject>()
    , m_config_filename(configFilename)
{
	//--------------------------------------------
	// Initialize Geneva as well as the known optimization algorithms

	m_gi.registerOAF<GEvolutionaryAlgorithmFactory>();
	m_gi.registerOAF<GSwarmAlgorithmFactory>();
	m_gi.registerOAF<GGradientDescentFactory>();
	m_gi.registerOAF<GSimulatedAnnealingFactory>();
	m_gi.registerOAF<GParameterScanFactory>();

	m_gi.registerConsumer<GIndividualWebsocketConsumer>();
	m_gi.registerConsumer<GIndividualAsioConsumer>();
	m_gi.registerConsumer<GIndividualThreadConsumer>();
	m_gi.registerConsumer<GIndividualSerialConsumer>();

#ifdef GENEVA_BUILD_WITH_MPI_CONSUMER
    // the mpi consumer requires to be a singleton, because it is not allowed to initialize or finalize MPI multiple times
    m_gi.registerConsumer(GMPIConsumerInstance);
#endif // GENEVA_BUILD_WITH_MPI_CONSUMER

	//--------------------------------------------
	// Parse configuration file options
	this->parseConfigFile(configFilename);

	//--------------------------------------------
	// Load configuration options from the command line
	parseCommandLine(argc, argv, userDescriptions);

	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Initialize all necessary variables
	std::call_once(f_go2, [this](){ setRNFParameters(this->m_n_producer_threads); });
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
void Go2::registerDefaultAlgorithm(std::string const & mn) {
	// Retrieve the algorithm from the global store
	std::shared_ptr<G_OptimizationAlgorithm_FactoryT<GOABase>> p;
	if (not GOAFactoryStore->get(mn, p)) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In Go2::registerDefaultAlgorithm(std::string): Error!" << std::endl
				<< "Got invalid algorithm mnemonic " << mn << std::endl
		);
	}

	this->registerDefaultAlgorithm(p->Gem::Common::GFactoryT<GOABase>::get());
}

/******************************************************************************/
/**
 * Allows to register a default algorithm to be used when no other algorithms
 * have been specified. When others have been specified, this algorithm will
 * not be used. Note that any individuals registered with the default algorithm
 * will be copied into the Go2 object.
 */
void Go2::registerDefaultAlgorithm(std::shared_ptr<GOABase> default_algorithm) {
	// Check that the pointer isn't empty
	if (not default_algorithm) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In Go2::registerDefaultAlgorithm(): Error!" << std::endl
				<< "Got empty algorithm." << std::endl
		);
	}

	// If any individuals have been storedn in the default algorithm, we assume
	// that the user wants us to use them and copy them over. Note that these are not cloned.
	if (not default_algorithm->empty()) { // Have individuals been registered ?
		for (const auto& ind_ptr: *default_algorithm) this->push_back(ind_ptr);
		// Remove the individuals from the old algorithm
		default_algorithm->clear();
	}

	// Register the algorithm
	m_default_algorithm = default_algorithm;
}

/******************************************************************************/
/**
 * Allows to register a pluggable optimization monitor
 */
void Go2::registerPluggableOM(
	std::shared_ptr<GBasePluggableOM> pluggableOM
) {
	if (pluggableOM) {
		m_pluggable_monitors_cnt.push_back(pluggableOM);
	} else {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In Go2::registerPluggableOM(): Tried to register empty pluggable optimization monitor" << std::endl
		);
	}
}

/******************************************************************************/
/**
 * Allows to reset the local pluggable optimization monitor
 */
void Go2::resetPluggableOM() {
	m_pluggable_monitors_cnt.clear();
}

/******************************************************************************/
/**
 * Allows to check whether pluggable optimization monitors were registered
 */
bool Go2::hasOptimizationMonitors() const {
	return not m_pluggable_monitors_cnt.empty();
}

/******************************************************************************/
/**
 * Allows to set the maximum running time for a client. A duration of 0 results
 * in no time limit being set.
 */
void Go2::setMaxClientTime(std::chrono::duration<double> maxDuration) {
	m_max_client_duration = maxDuration;
}

/******************************************************************************/
/**
 * Allows to retrieve the maximum running time for a client
 */
std::chrono::duration<double> Go2::getMaxClientTime() const {
	return m_max_client_duration;
}

/******************************************************************************/
/**
 * Triggers execution of the client loop. Note that it is up to you to terminate
 * the program after calling this function.
 */
int Go2::clientRun() {
	// Check that we have indeed been given a valid name
	if (
		GO2_DEF_NOCONSUMER == m_consumer_name
		|| not GConsumerStore->exists(m_consumer_name)
		) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In Go2::clientRun(): Error!" << std::endl
				<< "Received invalid consumer name: " << m_consumer_name << std::endl
		);
	}

	// Retrieve the client worker from the consumer
	std::shared_ptr<Gem::Courtier::GBaseClientT<Gem::Geneva::GParameterSet>> p;

	if (GConsumerStore->get(m_consumer_name)->needsClient()) {
		p = GConsumerStore->get(m_consumer_name)->getClient();
	} else {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In Go2::clientRun(): Error!" << std::endl
				<< "Trying to execute clientRun() on consumer " << m_consumer_name << std::endl
				<< "which does not require a client" << std::endl
		);
	}

	// Check for errors
	if (not p) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In Go2::clientRun(): Error!" << std::endl
				<< "Received empty client from consumer " << m_consumer_name << std::endl
		);
	}

	// Set the maximum runtime of the client
	p->setMaxTime(this->m_max_client_duration);

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
	return m_client_mode;
}

/******************************************************************************/
/**
 * Specifies whether only the best individuals of a population should be copied
 */
void Go2::setCopyBestIndividualsOnly(bool copyBestIndividualsOnly) {
	m_copyBestIndividualsOnly = copyBestIndividualsOnly;
}

/******************************************************************************/
/**
 * Checks whether only the best individuals are copied
 */
bool Go2::onlyBestIndividualsAreCopied() const {
	return m_copyBestIndividualsOnly;
}

/******************************************************************************/
/**
 * Retrieves the currently registered number of algorithms
 */
std::size_t Go2::getNAlgorithms() const {
	return m_algorithms_cnt.size();
}

/******************************************************************************/
/**
 * Allows to add an optimization algorithm to the chain. If any individuals have
 * been stored in these algorithms, Go2 will unload them and store them for later
 * usage.
 *
 * @param alg A base pointer to another optimization algorithm
 */
void Go2::addAlgorithm(std::shared_ptr<GOABase> alg) {
	// Check that the pointer is not empty
	if (not alg) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In Go2::addAlgorithm(): Error!" << std::endl
				<< "Tried to register an empty pointer" << std::endl
		);
	}

	// If any individuals have been registered with alg, we assume
	// that the user wants us to add them to the optimization and copy them over.
	// Note that these are not cloned.
	if (not alg->empty()) { // Have individuals been registered ?
		for (const auto& ind_ptr: *alg) this->push_back(ind_ptr);
		// Remove the individuals from the old algorithm
		alg->clear();
	}

	m_algorithms_cnt.push_back(alg);
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
	return m_algorithms_cnt;
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
Go2 &Go2::operator&(std::shared_ptr<GOABase> alg) {
	this->addAlgorithm(alg); // NOLINT
	return *this;
}

/***************************************************************************/
/**
 * Allows to add an optimization algorithm through its mnemonic
 */
void Go2::addAlgorithm(std::string const & mn) {
	// Retrieve the algorithm from the global store
	std::shared_ptr<G_OptimizationAlgorithm_FactoryT<GOABase>> p;
	if (not GOAFactoryStore->get(mn, p)) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In Go2::addAlgorithm(std::string): Error!" << std::endl
				<< "Got invalid algorithm mnemonic " << mn << std::endl
		);
	}

	this->addAlgorithm(p->Gem::Common::GFactoryT<GOABase>::get()); // The factory might add a monitor to the object
}

/***************************************************************************/
/**
 * Makes it easier to add algorithms through their mnemonics
 */
Go2 &Go2::operator&(std::string const & mn) {
	this->addAlgorithm(mn);
	return *this;
}

/***************************************************************************/
/**
 * Allows to register a content creator
 */
void Go2::registerContentCreator(
	std::shared_ptr<Gem::Common::GFactoryT<GParameterSet>> cc_ptr
) {
	if (not cc_ptr) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In Go2::registerContentCreator(): Error!" << std::endl
				<< "Tried to register an empty pointer" << std::endl
		);
	}

	m_content_creator_ptr = cc_ptr;
}

/******************************************************************************/
/**
 * Perform the actual optimization cycle. Note that we assume that individuals
 * have either been registered with the Go2 object or with the first algorithm
 * which has been added to the object. When no algorithm was added to the Go2
 * object (either on the command line or by passing a mnemonic or smart pointer),
 * a default algorithm will be used. Check the #define DEFAULTOPTALG for information
 * on the type of algorithm being used. The default algorithm may also be altered
 * by the user.
 *
 * @param offset An offset at which the first algorithm should start
 */
Go2 const * const Go2::optimize_(std::uint32_t) {
	// Check that algorithms have indeed been registered. If not, try to add a default algorithm
	if (m_algorithms_cnt.empty()) {
		if (not m_default_algorithm) {
			// No algorithms given, no default algorithm specified by the user:
			// Simply add the Geneva-side default algorithm
			this->registerDefaultAlgorithm(m_default_algorithm_str);

			glogger
				<< "In Go2::optimize(): INFORMATION:" << std::endl
				<< "No user-defined optimization algorithm available." << std::endl
				<< "Using default algorithm \"" << m_default_algorithm_str << "\" instead." << std::endl
				<< GLOGGING;
		}

		m_algorithms_cnt.push_back(m_default_algorithm->clone<GOABase>());
	}

	// Check whether a possible checkpoint file fits the first algorithm in the chain
	if(m_cp_file != "empty" && not m_algorithms_cnt[0]->cp_personality_fits(std::filesystem::path(m_cp_file))) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In Go2::optimize(): Error!" << std::endl
				<< "Checkpoint file " << m_cp_file << " does not" << std::endl
				<< "fit requirements of first algorithm " << m_algorithms_cnt[0]->getAlgorithmPersonalityType() << std::endl
		);
	}

	// Load the checkpoint file or create individuals from the content creator
	if(m_cp_file != "empty") {
		// Load the external data
		m_algorithms_cnt[0]->loadCheckpoint(std::filesystem::path(m_cp_file));

		// Make sure the first algorithm starts right after the iteration where the checkpoint file ended
		m_iterations_consumed = m_algorithms_cnt[0]->getIteration() + 1;
	} else {
		// Check that individuals have been registered
		if (this->empty()) {
			if (m_content_creator_ptr) {
				for (std::size_t ind = 0; ind < m_algorithms_cnt.at(0)->getDefaultPopulationSize(); ind++) {
					std::shared_ptr<GParameterSet> p_ind = (*m_content_creator_ptr)();
					if (p_ind) {
						this->push_back(p_ind);
					} else { // No valid item received, the factory has run empty
						if (this->empty()) { // Still empty ?
							throw gemfony_exception(
								g_error_streamer(DO_LOG,  time_and_place)
									<< "In Go2::optimize(): Error!" << std::endl
									<< "The content creator did not deliver any individuals" << std::endl
									<< "and none have been registered so far." << std::endl
									<< "No way to continue." << std::endl
							);
						}
						break;
					}
				}
			} else {
				throw gemfony_exception(
					g_error_streamer(DO_LOG,  time_and_place)
						<< "In Go2::optimize(): Error!" << std::endl
						<< "Neither a content creator nor individuals have been registered." << std::endl
						<< "No way to continue." << std::endl
				);
			}
		}

		// We start with a predefined iteration offset
		m_iterations_consumed = m_offset;
	}

	// Loop over all algorithms
	m_sorted = false;
	for (const auto& alg_ptr: m_algorithms_cnt) {
		// Add the pluggable optimization monitors to the algorithm
		for(auto const & pm_ptr: m_pluggable_monitors_cnt) {
			alg_ptr->registerPluggableOM(pm_ptr);
		}

		// Add the individuals to the algorithm
		for (const auto& ind_ptr: *this) {
			alg_ptr->push_back(ind_ptr);
		}

		// Remove our local copies
		this->clear();

		// Do the actual optimization
		alg_ptr->optimize(m_iterations_consumed);

		// Make sure we start with the correct iteration in the next algorithm
		m_iterations_consumed = alg_ptr->getIteration();

		// Unload the individuals from the last algorithm and store them again in this object
		if(m_copyBestIndividualsOnly) {
			for (const auto &best_ind_ptr: alg_ptr->getBestGlobalIndividuals<GParameterSet>()) {
				this->push_back(best_ind_ptr);
			}
		} else { // copy all individuals
			for (const auto &ind_ptr: *alg_ptr) {
				this->push_back(ind_ptr);
			}
		}

		alg_ptr->clear(); // Get rid of local individuals in the algorithm
		alg_ptr->resetPluggableOM(); // Get rid of the algorithm's pluggable optimization monitors
	}

	// Sort the individuals according to their primary fitness so we have it easier later on
	// to extract the best individuals found.
	std::sort(
		this->begin()
		, this->end()
		, [](std::shared_ptr<GParameterSet> x_ptr, std::shared_ptr<GParameterSet> y_ptr) -> bool {
			return minOnly_transformed_fitness(x_ptr) < minOnly_transformed_fitness(y_ptr);
		}
	);

	m_sorted = true;

	return this;
}

/******************************************************************************/
/**
 * Retrieves the best individual found. This function returns a base pointer.
 * Conversion is done through a function stored in GOptimizableI.
 *
 * @return The best individual found
 */
std::shared_ptr<Gem::Geneva::GParameterSet> Go2::getBestGlobalIndividual_() const {
	// Do some error checking
	if (this->empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In Go2::getBestGlobalIndividual_(): Error!" << std::endl
				<< "No individuals found" << std::endl
		);
	}

	if (not m_sorted) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In Go2::getBestGlobalIndividual_(): Error!" << std::endl
				<< "Tried to retrieve best individual" << std::endl
				<< "from an unsorted population." << std::endl
		);
	}

	// Check if the best individual is processed
	if(not this->front()->is_processed() && not this->front()->is_ignored()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In Go2::getBestGlobalIndividual_(): Error!" << std::endl
				<< "Best individual is unprocessed or has errors" << std::endl
		);
	}

	// Simply return the best individual. This will result in an implicit downcast
	return this->front()->clone<Gem::Geneva::GParameterSet>();
}

/******************************************************************************/
/**
 * Retrieves a list of the best individuals found. This function returns  base pointers.
 * Conversion is done through a function stored in GOptimizableI.
 *
 * @return The best individual found
 */
std::vector<std::shared_ptr<Gem::Geneva::GParameterSet>> Go2::getBestGlobalIndividuals_() const {
	// Do some error checking
	if (this->empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In Go2::getBestGlobalIndividuals_(): Error!" << std::endl
				<< "No individuals found" << std::endl
		);
	}

	std::size_t pos = 0;
	std::vector<std::shared_ptr<Gem::Geneva::GParameterSet>> bestIndividuals;
	for (const auto& ind_ptr: *this) {
		if (ind_ptr->is_due_for_processing() || ind_ptr->has_errors()) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In Go2::getBestGlobalIndividuals_(): Error!" << std::endl
					<< "Found individual in position " << pos << " which is unprocessed or which has errors" <<
					std::endl
			);
		}

		// This will result in an implicit downcast
		bestIndividuals.push_back(ind_ptr->clone<Gem::Geneva::GParameterSet>());

		pos++;
	}

	return bestIndividuals;
}

/******************************************************************************/
/**
 * Retrieves the best individual found. This function returns a base pointer.
 * Conversion is done through a function stored in GOptimizableI.
 *
 * @return The best individual found
 */
std::shared_ptr<Gem::Geneva::GParameterSet> Go2::getBestIterationIndividual_() const {
	throw gemfony_exception(
		g_error_streamer(DO_LOG,  time_and_place)
			<< "In Go2::getBestIterationIndividual_(): Error!" << std::endl
			<< "This function should not be called" << std::endl
	);
}

/******************************************************************************/
/**
 * Retrieves a list of the best individuals found. This function returns  base pointers.
 * Conversion is done through a function stored in GOptimizableI.
 *
 * @return The best individual found
 */
std::vector<std::shared_ptr<Gem::Geneva::GParameterSet>> Go2::getBestIterationIndividuals_() const {
	throw gemfony_exception(
		g_error_streamer(DO_LOG,  time_and_place)
			<< "In Go2::getBestIterationIndividuals_(): Error!" << std::endl
			<< "This function should not be called" << std::endl
	);
}

/******************************************************************************/
/**
 * Satisfies a requirement of G_Interface_Optimizer
 */
void Go2::runFitnessCalculation_() { /* nothing */ }

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void Go2::addConfigurationOptions(
	Gem::Common::GParserBuilder &gpb
) {
	using namespace Gem::Common;

	// Add local data only -- no options from parent classes
	gpb.registerFileParameter<std::uint16_t>(
		"nProducerThreads"
		, GO2_DEF_NPRODUCERTHREADS
		, [this](std::uint16_t npt) { this->setNProducerThreads(npt); }
	)
		<< "The number of threads simultaneously producing random numbers";

	gpb.registerFileParameter<bool>(
		"copyBestIndividualsOnly"
		, GO2_DEF_COPYBESTINDIVIDUALSONLY
		, [this](bool copyBestIndividualsOnly) { this->setCopyBestIndividualsOnly(copyBestIndividualsOnly); }
	)
		<< "Indicates whether only the best individuals should be copied when" << std::endl
		<< "switching from one optimization algorithm to the next";
}

/******************************************************************************/
/**
 * Allows to mark this object as belonging to a client as opposed to a server
 *
 * @param serverMode Allows to mark this object as belonging to a client as opposed to a server
 */
void Go2::setClientMode(bool clientMode) {
#ifdef GENEVA_BUILD_WITH_MPI_CONSUMER
    if (m_consumer_name == "GMPIConsumerT" || m_consumer_name == "mpi") {
        throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
					<< "In Go2::setClientMode(): Error!" << std::endl
					<< "If running MPI then the mode can not be changed between client and server mode after the process has been launched"
			);
    }
#endif // GENEVA_BUILD_WITH_MPI_CONSUMER
	m_client_mode = clientMode;
}

/******************************************************************************/
/**
 * Allows to check whether this object is working in server or client mode
 *
 * @return A boolean indicating whether this object is working in server or client mode
 */
bool Go2::getClientMode() const {
	return m_client_mode;
}

/******************************************************************************/
/**
 * Allows to set the number of threads that will simultaneously produce random numbers.
 *
 * @param nProducerThreads The number of threads that will simultaneously produce random numbers
 */
void Go2::setNProducerThreads(std::uint16_t nProducerThreads) {
	m_n_producer_threads = nProducerThreads;
}

/******************************************************************************/
/**
 * Allows to retrieve the number of threads that will simultaneously produce random numbers.
 *
 * @return The number of threads that will simultaneously produce random numbers
 */
std::uint16_t Go2::getNProducerThreads() const {
	return m_n_producer_threads;
}

/******************************************************************************/
/**
 * Allows to specify the offset with which the iteration counter should start. This is
 * important when using more than one optimization algorithm with different Go2 objects.
 *
 * @param offset The offset with which the iteration counter should start
 */
void Go2::setOffset(std::uint32_t offset) {
	m_offset = offset;
}

/******************************************************************************/
/**
 * Retrieval of the current iteration
 */
uint32_t Go2::getIteration_() const {
	return m_iterations_consumed;
}

/******************************************************************************/
/**
 * Returns the name of this optimization algorithm
 *
 * @return The name assigned to this optimization algorithm
 */
std::string Go2::getAlgorithmName_() const {
	return std::string("Algorithm Combiner");
}

/******************************************************************************/
/** @brief Returns one-word information about the type of optimization algorithm. */
std::string Go2::getAlgorithmPersonalityType_() const {
	return std::string("PERSONALITY_NONE");
}

/******************************************************************************/
/**
 * Allows to retrieve the current offset with which the iteration counter will start
 *
 * @return The current offset with which the iteration counter will start
 */
std::uint32_t Go2::getIterationOffset() const {
	return m_offset;
}

/******************************************************************************/
/**
 *
 * @param argc The number of command line arguments
 * @param argv An array with the arguments
 * @param od A program_options object for user-defined command line options
 */
void Go2::parseCommandLine(
	int argc
	, char **argv
	, boost::program_options::options_description const & userOptions
) {
	namespace po = boost::program_options;

	try {
		std::string maxClientDuration = EMPTYDURATION; // 00:00:00

		std::string optimization_algorithms;
		std::string checkpointFile = "empty";

		// Extract a list of algorithm mnemonics and clear-text descriptions
		std::string algorithm_description;
		std::vector<std::string> keys;
		GOAFactoryStore->getKeyVector(keys); // will clear "keys"
		for (const auto& key: keys) {
			algorithm_description += (key + ":  " + GOAFactoryStore->get(key)->getAlgorithmName() + "\n");
		}

		std::ostringstream oa_help;
		oa_help
			<< "A comma-separated list of optimization algorithms, e.g. \"arg1,arg2\". "
			<< GOAFactoryStore->size() << " algorithms have been registered: " << std::endl
			<< algorithm_description;

		// Extract a list of consumer mnemonics and clear-text descriptions
		std::string consumer_description;
		GConsumerStore->getKeyVector(keys);
		for(const auto& key: keys) {
			consumer_description += (key + ":  " + GConsumerStore->get(key)->getConsumerName() + "\n");
		}

		std::ostringstream consumer_help;
		consumer_help
			<< "The name of a consumer for brokered execution (an error will be flagged if called with any other execution mode than (2) ). "
			<< GConsumerStore->size() << " consumers have been registered: " << std::endl
			<< consumer_description;

		auto usageString = std::string("Usage: ") + argv[0] + " [options]";

		boost::program_options::options_description general(usageString);
		boost::program_options::options_description basic("Basic options");

		// First add local options
		basic.add_options()
			("help,h", "Emit help message")
			("showAll", "Show all available options")
			("optimizationAlgorithms,a", po::value<std::string>(&optimization_algorithms), oa_help.str().c_str())
			("cp_file,f", po::value<std::string>(&checkpointFile)->default_value("empty"),
				"A file (including its path) holding a checkpoint for a given optimization algorithm")
			("client", "Indicates that this program should run as a client or in server mode. Note that this setting will trigger an error unless called in conjunction with a consumer capable of dealing with clients. This option is ignored when working with the mpi consumer, because the mpi consumer will configure itself to be a client or server depending on its rank.")
			("maxClientDuration", po::value<std::string>(&maxClientDuration)->default_value(EMPTYDURATION),
				R"(The maximum runtime for a client in the form "hh:mm:ss". Note that a client may run longer as this time-frame if its work load still runs. The default value "00:00:00" means: "no time limit")")
			("consumer,c", po::value<std::string>(&m_consumer_name)->default_value("stc"), consumer_help.str().c_str());

		// Add additional options coming from the algorithms and consumers
		boost::program_options::options_description visible("Global algorithm- and consumer-options");
		boost::program_options::options_description hidden("Hidden algorithm- and consumer-options");

		// Retrieve available command-line options from registered consumers, if any
		if (not GConsumerStore->empty()) {
			GConsumerStore->rewind();
			do {
				GConsumerStore->getCurrentItem()->addCLOptions(visible, hidden);
			} while (GConsumerStore->goToNextPosition());
		}

		// Retrieve available command-line options from registered optimization algorithm factories, if any
		if (not GOAFactoryStore->empty()) {
			GOAFactoryStore->rewind();
			do {
				GOAFactoryStore->getCurrentItem()->addCLOptions(visible, hidden);
			} while (GOAFactoryStore->goToNextPosition());
		}

		// Add the other options to "general"
		if (userOptions.options().empty()) {
			general.add(basic).add(visible).add(hidden);
		} else {
			general.add(basic).add(userOptions).add(visible).add(hidden);
		}

		// Do the actual parsing of the command line
		po::variables_map vm;
		po::store(
			po::parse_command_line<char>(argc, static_cast<const char* const*>(argv), general)
			, vm
		);

		// Emit a help message, if necessary
		if (vm.count("help") || vm.count("showAll")) { // Allow syntax "program --help --showAll" and "program --showAll"
			if (vm.count("showAll")) { // Show all options
				std::cout << general << std::endl;
			} else { // Just show a selection
				boost::program_options::options_description selected(usageString);
				if (userOptions.options().empty()) {
					selected.add(basic).add(visible);
				} else {
					selected.add(basic).add(userOptions).add(visible);
				}
				std::cout << selected << std::endl;
			}
			exit(0);
		}

		po::notify(vm);

		if (vm.count("client")) {
			m_client_mode = true;
		}

		// No consumer specified, although brokered execution was requested
		if (vm.count("consumer") != 1) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In Go2::parseCommandLine(): Error!" << std::endl
					<< "You need to specify exactly one consumer for brokered execution," << std::endl
					<< "on the command line. Found " << vm.count("consumer") << "." << std::endl
			);
		}

		// Check that the requested consumer actually exists
		if (vm.count("consumer") && not GConsumerStore->exists(m_consumer_name)) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In Go2::parseCommandLine(): Error!" << std::endl
					<< "You have requested a consumer with name " << m_consumer_name << std::endl
					<< "which could not be found in the consumer store." << std::endl
			);
		}

		if (m_client_mode && not GConsumerStore->get(m_consumer_name)->needsClient()) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In Go2::parseCommandLine(): Error!" << std::endl
					<< "Requested client mode even though consumer " << m_consumer_name << " does not require a client" <<
					std::endl
			);
		}

		std::cout << "Using consumer " << m_consumer_name << std::endl;

        // allow the consumer to perform necessary initialization before startup
        GConsumerStore->get(m_consumer_name)->init();

        // TODO: Consider removing this #ifdefs
        //  However, an issue is that this would require to create public functions like GBaseConsumerT::clientMode()
        //  and GBaseConsumerT::setsClientModeItself() that are only implemented by GMPIConsumerT.
        //  While this sounds good at first this would mean that GMPIConsumerT which is typically a server requires
        //  knowledge about server AND client side. Both implementations (public functions to override or #ifdefs)
        //  are not very clean. Maybe another solution can be found. But we stick with the #ifdef for now

        // reset the client mode to the information that the consumer has in case we are dealing with the GMPIConsumerT.
        // That is because the MPI consumer should not depend on the --client command-line parameter but decide itself
        // whether it is a server or client depending on the process's MPI rank
#ifdef GENEVA_BUILD_WITH_MPI_CONSUMER
        if(GIndividualMPIConsumer* mpiConsumerPtr = dynamic_cast<GIndividualMPIConsumer*>(
                GConsumerStore->get(m_consumer_name).get())) {
            m_client_mode = mpiConsumerPtr->isWorkerNode();
        }
#endif // GENEVA_BUILD_WITH_MPI_CONSUMER

		// Finally give the consumer the chance to act on the command line options
		// TODO: clone the consumer, then let the clone act on CL options and add the clone to the broker
		GConsumerStore->get(m_consumer_name)->actOnCLOptions(vm);

		// At this point the consumer should be fully configured

		// Register the consumer with the broker, unless other consumers have already been registered or we are running in client mode
		if (not m_client_mode) {
			if (not GBROKER(Gem::Geneva::GParameterSet)->hasConsumers()) {
				GBROKER(Gem::Geneva::GParameterSet)->enrol_consumer(GConsumerStore->get(m_consumer_name));
			} else {
				glogger
					<< "In Go2::parseCommandLine(): Note!" << std::endl
					<< "Could not register requested consumer," << std::endl
					<< "as a consumer was already registered with the broker" << std::endl
					<< GLOGGING;
			}
		}

		// Parse the list of optimization algorithms
		if (vm.count("optimizationAlgorithms")) {
			std::vector<std::string> algs = Gem::Common::splitString(optimization_algorithms, ",");

			for (const auto& alg_str: algs) {
				// Retrieve the algorithm factory from the global store
				std::shared_ptr<G_OptimizationAlgorithm_FactoryT<GOABase>> p;
				if (not GOAFactoryStore->get(alg_str, p)) {
					throw gemfony_exception(
						g_error_streamer(DO_LOG,  time_and_place)
							<< "In Go2::parseCommandLine(int, char**): Error!" << std::endl
							<< "Got invalid algorithm mnemonic \"" << alg_str << "\"." << std::endl
							<< "No algorithm found for this string." << std::endl
					);
				}

				// Retrieve an algorithm from the factory and add it to the list
				m_algorithms_cnt.push_back(p->Gem::Common::GFactoryT<GOABase>::get());
			}
		}

		// Set the name of a checkpoint file (if any)
		m_cp_file = checkpointFile;

		// Set the maximum running time for the client (if any)
		m_max_client_duration = Gem::Common::duration_from_string(maxClientDuration);
	}
	catch (const po::error &e) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "Error parsing the command line:" << std::endl
				<< e.what() << std::endl
		);
	}
}

/******************************************************************************/
/**
 * Parses a configuration file for configuration options
 *
 * @param configFilename The name of a configuration file to be parsed
 */
void Go2::parseConfigFile(std::filesystem::path const & configFilename) {
	// Create a parser builder object. It will be destroyed at
	// the end of this scope and thus cannot cause trouble
	// due to registered call-backs and references
	Gem::Common::GParserBuilder gpb;

	// Add local configuration options
	this->addConfigurationOptions(gpb);

	// Do the actual parsing
	if (not gpb.parseConfigFile(configFilename)) {
		glogger
			<< "In Go2::parseConfigFile: Error!" << std::endl
			<< "Could not parse configuration file " << configFilename.string() << std::endl
			<< GTERMINATION;
	}
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
