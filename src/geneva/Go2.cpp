/**
 * @file Go2.cpp
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
	const std::uint16_t &nProducerThreads
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
 * The default constructor
 */
Go2::Go2()
	: GObject()
	, GOptimizableI()
   , Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, GObject>()
{
	//--------------------------------------------
	// Initialize Geneva as well as the known optimization algorithms and consumers

	m_gi.registerOAF<GEvolutionaryAlgorithmFactory2>();
	m_gi.registerOAF<GSwarmAlgorithmFactory2>();
	m_gi.registerOAF<GGradientDescentFactory2>();
	m_gi.registerOAF<GSimulatedAnnealingFactory2>();
	m_gi.registerOAF<GParameterScanFactory2>();

	m_gi.registerConsumer<GIndividualSerialTCPConsumer>();
	m_gi.registerConsumer<GIndividualAsyncTCPConsumer>();
	m_gi.registerConsumer<GIndividualThreadConsumer>();
	m_gi.registerConsumer<GIndividualSerialConsumer>();

	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Initialize all necessary variables
	std::call_once(f_go2, std::bind(setRNFParameters, m_n_producer_threads));
}


/******************************************************************************/
/**
 * A constructor that first parses the command line for relevant parameters and then
 * loads data from a configuration file
 *
 * @param argc The number of command line arguments
 * @param argv An array with the arguments
 * @param od A vector of additional command line options (cmp. boost::program_options)
 */
Go2::Go2(
	int argc
	, char **argv
	, const boost::program_options::options_description &userDescriptions
)
	: GObject()
	, GOptimizableI()
  	, Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, GObject>()
{
	//--------------------------------------------
	// Initialize Geneva as well as the known optimization algorithms

	m_gi.registerOAF<GEvolutionaryAlgorithmFactory2>();
	m_gi.registerOAF<GSwarmAlgorithmFactory2>();
	m_gi.registerOAF<GGradientDescentFactory2>();
	m_gi.registerOAF<GSimulatedAnnealingFactory2>();
	m_gi.registerOAF<GParameterScanFactory2>();

	m_gi.registerConsumer<GIndividualSerialTCPConsumer>();
	m_gi.registerConsumer<GIndividualAsyncTCPConsumer>();
	m_gi.registerConsumer<GIndividualThreadConsumer>();
	m_gi.registerConsumer<GIndividualSerialConsumer>();

	//--------------------------------------------
	// Load initial configuration options from the command line
	parseCommandLine(argc, argv, userDescriptions);

	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Initialize all necessary variables
	std::call_once(f_go2, std::bind(setRNFParameters, m_n_producer_threads));
}

/******************************************************************************/
/**
 * A constructor that only loads data from a configuration file
 *
 * @param configFilename The name of a configuration file
 */
Go2::Go2(const std::string &configFilename)
	: GObject()
	, GOptimizableI()
   , Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, GObject>()
   , m_config_filename(configFilename)
{
	//--------------------------------------------
	// Initialize Geneva as well as the known optimization algorithms

	m_gi.registerOAF<GEvolutionaryAlgorithmFactory2>();
	m_gi.registerOAF<GSwarmAlgorithmFactory2>();
	m_gi.registerOAF<GGradientDescentFactory2>();
	m_gi.registerOAF<GSimulatedAnnealingFactory2>();
	m_gi.registerOAF<GParameterScanFactory2>();

	m_gi.registerConsumer<GIndividualSerialTCPConsumer>();
	m_gi.registerConsumer<GIndividualAsyncTCPConsumer>();
	m_gi.registerConsumer<GIndividualThreadConsumer>();
	m_gi.registerConsumer<GIndividualSerialConsumer>();

	//--------------------------------------------
	// Parse configuration file options
	this->parseConfigFile(configFilename);

	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Initialize all necessary variables
	std::call_once(f_go2, std::bind(setRNFParameters, m_n_producer_threads));
}

/******************************************************************************/
/**
 * A constructor that first parses the command line for relevant parameters and then
 * loads data from a configuration file
 *
 * @param argc The number of command line arguments
 * @param argv An array with the arguments
 * @param configFilename The name of a configuration file
 * @param od A vector of additional command line options (cmp. boost::program_options)
 */
Go2::Go2(
	int argc, char **argv
	, const std::string &configFilename
	, const boost::program_options::options_description &userDescriptions
)
	: GObject()
	, GOptimizableI()
   , Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, GObject>()
   , m_config_filename(configFilename)
{
	//--------------------------------------------
	// Initialize Geneva as well as the known optimization algorithms

	m_gi.registerOAF<GEvolutionaryAlgorithmFactory2>();
	m_gi.registerOAF<GSwarmAlgorithmFactory2>();
	m_gi.registerOAF<GGradientDescentFactory2>();
	m_gi.registerOAF<GSimulatedAnnealingFactory2>();
	m_gi.registerOAF<GParameterScanFactory2>();

	m_gi.registerConsumer<GIndividualSerialTCPConsumer>();
	m_gi.registerConsumer<GIndividualAsyncTCPConsumer>();
	m_gi.registerConsumer<GIndividualThreadConsumer>();
	m_gi.registerConsumer<GIndividualSerialConsumer>();

	//--------------------------------------------
	// Parse configuration file options
	this->parseConfigFile(configFilename);

	//--------------------------------------------
	// Load configuration options from the command line
	parseCommandLine(argc, argv, userDescriptions);

	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Initialize all necessary variables
	std::call_once(f_go2, std::bind(setRNFParameters, m_n_producer_threads));
}

/******************************************************************************/
/**
 * The copy constructor
 *
 * TODO: Should be non-copyable
 */
Go2::Go2(const Go2 &cp)
	: GObject(cp)
   , Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, GObject>(cp)
   , m_client_mode(cp.m_client_mode)
   , m_config_filename(cp.m_config_filename)
   , m_consumer_name(cp.m_consumer_name)
   , m_n_producer_threads(cp.m_n_producer_threads)
   , m_offset(cp.m_offset)
   , m_sorted(cp.m_sorted)
{
	//--------------------------------------------
	// Initialize Geneva as well as the known optimization algorithms

	m_gi.registerOAF<GEvolutionaryAlgorithmFactory2>();
	m_gi.registerOAF<GSwarmAlgorithmFactory2>();
	m_gi.registerOAF<GGradientDescentFactory2>();
	m_gi.registerOAF<GSimulatedAnnealingFactory2>();
	m_gi.registerOAF<GParameterScanFactory2>();

	m_gi.registerConsumer<GIndividualSerialTCPConsumer>();
	m_gi.registerConsumer<GIndividualAsyncTCPConsumer>();
	m_gi.registerConsumer<GIndividualThreadConsumer>();
	m_gi.registerConsumer<GIndividualSerialConsumer>();

	//--------------------------------------------
	// Copy the algorithms vectors over
	Gem::Common::copyCloneableSmartPointerContainer(cp.m_algorithms_vec, m_algorithms_vec);

	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Initialize all necessary variables
	std::call_once(f_go2, std::bind(setRNFParameters, m_n_producer_threads));

	//--------------------------------------------
	// Copy the default algorithm over, if any
	Gem::Common::copyCloneableSmartPointer<GOABase>(cp.m_default_algorithm, m_default_algorithm);
}

/******************************************************************************/
/**
 * The destructor
 */
Go2::~Go2() {
	this->clear(); // Get rid of the local individuals
	m_algorithms_vec.clear(); // Get rid of the optimization algorithms
}

/***************************************************************************/
/**
 * The standard assignment operator
 */
const Go2 &Go2::operator=(const Go2 &cp) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another Go2 object
 *
 * @param  cp A constant reference to another Go2 object
 * @return A boolean indicating whether both objects are equal
 */
bool Go2::operator==(const Go2 &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Checks for inequality with another Go2 object
 *
 * @param  cp A constant reference to another Go2 object
 * @return A boolean indicating whether both objects are inequal
 */
bool Go2::operator!=(const Go2 &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void Go2::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a Go2 reference independent of this object and convert the pointer
	const Go2 *p_load = Gem::Common::g_convert_and_compare<GObject, Go2>(cp, this);

	GToken token("Go2", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GObject>(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	compare_t(IDENTITY(this->data,  p_load->data), token); // Actually data is contained in a parent class
	compare_t(IDENTITY(m_client_mode, p_load->m_client_mode), token);
	compare_t(IDENTITY(m_config_filename, p_load->m_config_filename), token);
	compare_t(IDENTITY(m_consumer_name, p_load->m_consumer_name), token);
	compare_t(IDENTITY(m_n_producer_threads, p_load->m_n_producer_threads), token);
	compare_t(IDENTITY(m_max_client_duration.count(), p_load->m_max_client_duration.count()), token);
	compare_t(IDENTITY(m_offset, p_load->m_offset), token);
	compare_t(IDENTITY(m_sorted, p_load->m_sorted), token);
	compare_t(IDENTITY(m_iterations_consumed, p_load->m_iterations_consumed), token);
	compare_t(IDENTITY(m_default_algorithm, p_load->m_default_algorithm), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string Go2::name() const {
	return std::string("Go2");
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
void Go2::registerDefaultAlgorithm(const std::string &mn) {
	// Retrieve the algorithm from the global store
	std::shared_ptr<GOptimizationAlgorithmFactoryT<GOABase>> p;
	if (!GOAFactoryStore->get(mn, p)) {
		glogger
		<< "In Go2::registerDefaultAlgorithm(std::string): Error!" << std::endl
		<< "Got invalid algorithm mnemonic " << mn << std::endl
		<< GEXCEPTION;
	}

	this->registerDefaultAlgorithm(p->get());
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
	if (!default_algorithm) {
		glogger
		<< "In Go2::registerDefaultAlgorithm(): Error!" << std::endl
		<< "Got empty algorithm." << std::endl
		<< GEXCEPTION;
	}

	if (!default_algorithm->empty()) { // Have individuals been registered ?
		GOABase::iterator it;
		for (it = default_algorithm->begin(); it != default_algorithm->end(); ++it) {
			this->push_back(*it);
		}

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
	std::shared_ptr<GBasePluggableOMT<GOABase>> pluggableOM
) {
	if (pluggableOM) {
		m_pluggable_monitors_vec.push_back(pluggableOM);
	} else {
		glogger
		<< "In Go2::registerPluggableOM(): Tried to register empty pluggable optimization monitor" << std::endl
		<< GEXCEPTION;
	}
}

/******************************************************************************/
/**
 * Allows to reset the local pluggable optimization monitor
 */
void Go2::resetPluggableOM() {
	m_pluggable_monitors_vec.clear();
}

/******************************************************************************/
/**
 * Allows to check whether pluggable optimization monitors were registered
 */
bool Go2::hasOptimizationMonitors() const {
	return !m_pluggable_monitors_vec.empty();
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
 * Loads the data of another Go2 object
 *
 * @param cp A copy of another Go2 object, camouflaged as a GObject
 */
void Go2::load_(const GObject *cp) {
	// Check that we are dealing with a Go2 reference independent of this object and convert the pointer
	const Go2 *p_load = Gem::Common::g_convert_and_compare<GObject, Go2>(cp, this);

	// First load the parent class'es data ...
	GObject::load_(cp);
	Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, GObject>::operator=(*p_load);

	// and then our local data
	m_client_mode = p_load->m_client_mode;
	m_config_filename = p_load->m_config_filename;
	m_consumer_name = p_load->m_consumer_name;
	m_n_producer_threads = p_load->m_n_producer_threads;
	m_max_client_duration = p_load->m_max_client_duration;
	m_offset = p_load->m_offset;
	m_sorted = p_load->m_sorted;
	m_iterations_consumed = p_load->m_iterations_consumed;
	m_cp_file = p_load->m_cp_file;

	Gem::Common::copyCloneableSmartPointer<GOABase>(p_load->m_default_algorithm, m_default_algorithm);

	// Copy the algorithm vectors over
	Gem::Common::copyCloneableSmartPointerContainer(p_load->m_algorithms_vec, m_algorithms_vec);
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object
 */
GObject *Go2::clone_() const {
	return new Go2(*this);
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
		|| !GConsumerStore->exists(m_consumer_name)
	) {
		glogger
		<< "In Go2::clientRun(): Error!" << std::endl
		<< "Received invalid consumer name: " << m_consumer_name << std::endl
		<< GEXCEPTION;
	}

	// Retrieve the client worker from the consumer
	std::shared_ptr<Gem::Courtier::GBaseClientT<Gem::Geneva::GParameterSet>> p;

	if (GConsumerStore->get(m_consumer_name)->needsClient()) {
		p = GConsumerStore->get(m_consumer_name)->getClient();
	} else {
		glogger
		<< "In Go2::clientRun(): Error!" << std::endl
		<< "Trying to execute clientRun() on consumer " << m_consumer_name << std::endl
		<< "which does not require a client" << std::endl
		<< GEXCEPTION;
	}

	// Check for errors
	if (!p) {
		glogger
		<< "In Go2::clientRun(): Error!" << std::endl
		<< "Received empty client from consumer " << m_consumer_name << std::endl
		<< GEXCEPTION;
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
 * Retrieves the currently registered number of algorithms
 */
std::size_t Go2::getNAlgorithms() const {
	return m_algorithms_vec.size();
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
	if (!alg) {
		glogger
		<< "In Go2::addAlgorithm(): Error!" << std::endl
		<< "Tried to register an empty pointer" << std::endl
		<< GEXCEPTION;
	}

	if (!alg->empty()) { // Have individuals been registered ?
		GOABase::iterator it;
		for (it = alg->begin(); it != alg->end(); ++it) {
			this->push_back(*it);
		}

		alg->clear();
	}

	m_algorithms_vec.push_back(alg);
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
	return m_algorithms_vec;
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
Go2 &Go2::operator&(std::shared_ptr<GOABase > alg) {
	this->addAlgorithm(alg);
	return *this;
}

/***************************************************************************/
/**
 * Allows to add an optimization algorithm through its mnemonic
 */
void Go2::addAlgorithm(const std::string &mn) {
	// Retrieve the algorithm from the global store
	std::shared_ptr<GOptimizationAlgorithmFactoryT<GOABase>> p;
	if (!GOAFactoryStore->get(mn, p)) {
		glogger
		<< "In Go2::addAlgorithm(std::string): Error!" << std::endl
		<< "Got invalid algorithm mnemonic " << mn << std::endl
		<< GEXCEPTION;
	}

	this->addAlgorithm(p->get()); // The factory might add a monitor to the object
}

/***************************************************************************/
/**
 * Makes it easier to add algorithms through their mnemonics
 */
Go2 &Go2::operator&(const std::string &mn) {
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
	if (!cc_ptr) {
		glogger
		<< "In Go2::registerContentCreator(): Error!" << std::endl
		<< "Tried to register an empty pointer" << std::endl
		<< GEXCEPTION;
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
void Go2::optimize(const std::uint32_t &offset) {
	// Check that algorithms have indeed been registered. If not, try to add a default algorithm
	if (m_algorithms_vec.empty()) {
		if (!m_default_algorithm) {
			// No algorithms given, no default algorithm specified by the user:
			// Simply add the Geneva-side default algorithm
			this->registerDefaultAlgorithm(m_default_algorithm_str);

			glogger
			<< "In Go2::optimize(): INFORMATION:" << std::endl
			<< "No user-defined optimization algorithm available." << std::endl
			<< "Using default algorithm \"" << m_default_algorithm_str << "\" instead." << std::endl
			<< GLOGGING;
		}

		m_algorithms_vec.push_back(m_default_algorithm->clone<GOABase>());
	}

	// Check whether a possible checkpoint file fits the first algorithm in the chain
	if(m_cp_file != "empty" && !m_algorithms_vec[0]->cp_personality_fits(boost::filesystem::path(m_cp_file))) {
		glogger
		<< "In Go2::optimize(): Error!" << std::endl
	   << "Checkpoint file " << m_cp_file << " does not" << std::endl
	   << "fit requirements of first algorithm " << m_algorithms_vec[0]->getOptimizationAlgorithm() << std::endl
		<< GEXCEPTION;
	}

	// Load the checkpoint file or create individuals from the content creator
	if(m_cp_file != "empty") {
		// Load the external data
		m_algorithms_vec[0]->loadCheckpoint(boost::filesystem::path(m_cp_file));

		// Make sure the first algorithm starts right after the iteration where the checkpoint file ended
		m_iterations_consumed = m_algorithms_vec[0]->getIteration() + 1;
	} else {
		// Check that individuals have been registered
		if (this->empty()) {
			if (m_content_creator_ptr) {
				for (std::size_t ind = 0; ind < m_algorithms_vec.at(0)->getDefaultPopulationSize(); ind++) {
					std::shared_ptr<GParameterSet> p_ind = (*m_content_creator_ptr)();
					if (p_ind) {
						this->push_back(p_ind);
					} else { // No valid item received, the factory has run empty
						if (this->empty()) { // Still empty ?
							glogger
								<< "In Go2::optimize(): Error!" << std::endl
								<< "The content creator did not deliver any individuals" << std::endl
								<< "and none have been registered so far." << std::endl
								<< "No way to continue." << std::endl
								<< GEXCEPTION;
						}
						break;
					}
				}
			} else {
				glogger
					<< "In Go2::optimize(): Error!" << std::endl
					<< "Neither a content creator nor individuals have been registered." << std::endl
					<< "No way to continue." << std::endl
					<< GEXCEPTION;
			}
		}

		// We start with a predefined iteration offset
		m_iterations_consumed = m_offset;
	}

	// Loop over all algorithms
	m_sorted = false;
	GOABase::iterator ind_it;
	std::vector<std::shared_ptr<GOABase>>::iterator alg_it;
	for (alg_it = m_algorithms_vec.begin(); alg_it != m_algorithms_vec.end(); ++alg_it) {
		std::shared_ptr<GOABase> p_base = (*alg_it);

		// Add the pluggable optimization monitors to the algorithm
		for(auto pm_ptr: m_pluggable_monitors_vec) { // std::shared_ptr may be copied
			p_base->registerPluggableOM(pm_ptr);
		}

		// Add the individuals to the algorithm.
		for (ind_it = this->begin(); ind_it != this->end(); ++ind_it) {
			p_base->push_back(*ind_it);
		}

		// Remove our local copies
		this->clear();

		// Do the actual optimization
		p_base->GOptimizableI::optimize<GParameterSet>(m_iterations_consumed);

		// Make sure we start with the correct iteration in the next algorithm
		m_iterations_consumed = p_base->getIteration();

		// Unload the individuals from the last algorithm and store them again in this object
		std::vector<std::shared_ptr<GParameterSet>> bestIndividuals = p_base->GOptimizableI::getBestGlobalIndividuals < GParameterSet > ();
		std::vector<std::shared_ptr<GParameterSet>>::iterator best_it;
		for (best_it = bestIndividuals.begin(); best_it != bestIndividuals.end(); ++best_it) {
			this->push_back(*best_it);
		}

		bestIndividuals.clear();
		p_base->clear(); // Get rid of local individuals in the algorithm
		p_base->resetPluggableOM(); // Get rid of the algorithm's pluggable optimization monitors
	}

	// Sort the individuals according to their primary fitness so we have it easier later on
	// to extract the best individuals found.
	std::sort(
		this->begin(), this->end(), [](std::shared_ptr<GParameterSet> x, std::shared_ptr<GParameterSet> y) -> bool {
			return x->minOnly_fitness() < y->minOnly_fitness();
		}
	);

	m_sorted = true;
}

/******************************************************************************/
/**
 * Retrieves the best individual found. This function returns a base pointer.
 * Conversion is done through a function stored in GOptimizableI.
 *
 * @return The best individual found
 */
std::shared_ptr<Gem::Geneva::GParameterSet> Go2::customGetBestGlobalIndividual() {
	Go2::iterator it;

	// Do some error checking
	if (this->empty()) {
		glogger
		<< "In Go2::customGetBestGlobalIndividual(): Error!" << std::endl
		<< "No individuals found" << std::endl
		<< GEXCEPTION;
	}

	for (it = this->begin(); it != this->end(); ++it) {
		if ((*it)->isDirty()) {
			glogger
			<< "In Go2::customGetBestGlobalIndividual(): Error!" << std::endl
			<< "Found individual in position " << std::distance(this->begin(), it) << " whose dirty flag is set" <<
			std::endl
			<< GEXCEPTION;
		}
	}

	if (!m_sorted) {
		glogger
		<< "In Go2::customGetBestGlobalIndividual(): Error!" << std::endl
		<< "Tried to retrieve best individual" << std::endl
		<< "from an unsorted population." << std::endl
		<< GEXCEPTION;
	}

	// Simply return the best individual. This will result in an implicit downcast
	return this->front();
}

/******************************************************************************/
/**
 * Retrieves a list of the best individuals found. This function returns  base pointers.
 * Conversion is done through a function stored in GOptimizableI.
 *
 * @return The best individual found
 */
std::vector<std::shared_ptr<Gem::Geneva::GParameterSet>> Go2::customGetBestGlobalIndividuals() {
	Go2::iterator it;

	// Do some error checking
	if (this->empty()) {
		glogger
		<< "In Go2::customGetBestGlobalIndividuals(): Error!" << std::endl
		<< "No individuals found" << std::endl
		<< GEXCEPTION;
	}

	for (it = this->begin(); it != this->end(); ++it) {
		if ((*it)->isDirty()) {
			glogger
			<< "In Go2::customGetBestGlobalIndividuals(): Error!" << std::endl
			<< "Found individual in position " << std::distance(this->begin(), it) << " whose dirty flag is set" <<
			std::endl
			<< GEXCEPTION;
		}
	}

	std::vector<std::shared_ptr<Gem::Geneva::GParameterSet>> bestIndividuals;
	for (it = this->begin(); it != this->end(); ++it) {
		// This will result in an implicit downcast
		bestIndividuals.push_back(*it);
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
std::shared_ptr<Gem::Geneva::GParameterSet> Go2::customGetBestIterationIndividual() {
	glogger
	<< "In Go2::customGetBestIterationIndividual(): Error!" << std::endl
	<< "This function should not be called" << std::endl
	<< GEXCEPTION;

	// Make the compiler happy
	return std::shared_ptr<Gem::Geneva::GParameterSet>();
}

/******************************************************************************/
/**
 * Retrieves a list of the best individuals found. This function returns  base pointers.
 * Conversion is done through a function stored in GOptimizableI.
 *
 * @return The best individual found
 */
std::vector<std::shared_ptr<Gem::Geneva::GParameterSet>> Go2::customGetBestIterationIndividuals() {
	glogger
	<< "In Go2::customGetBestIterationIndividuals(): Error!" << std::endl
	<< "This function should not be called" << std::endl
	<< GEXCEPTION;

	// Make the compiler happy
	return std::vector<std::shared_ptr<Gem::Geneva::GParameterSet>>();
}

/******************************************************************************/
/**
 * Satisfies a requirement of GOptimizableI
 */
void Go2::runFitnessCalculation() { /* nothing */ }

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

	// Call our parent class'es function
	GObject::addConfigurationOptions(gpb);

	// Add local data
	gpb.registerFileParameter<std::uint16_t>(
		"nProducerThreads" // The name of the first variable
		, GO2_DEF_NPRODUCERTHREADS
		, [this](std::uint16_t npt) { this->setNProducerThreads(npt); }
	)
	<< "The number of threads simultaneously producing random numbers";
}

/******************************************************************************/
/**
 * Allows to mark this object as belonging to a client as opposed to a server
 *
 * @param serverMode Allows to mark this object as belonging to a client as opposed to a server
 */
void Go2::setClientMode(bool clientMode) {
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
void Go2::setNProducerThreads(const std::uint16_t &nProducerThreads) {
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
void Go2::setOffset(const std::uint32_t &offset) {
	m_offset = offset;
}

/******************************************************************************/
/**
 * Retrieval of the current iteration
 */
uint32_t Go2::getIteration() const {
	return m_iterations_consumed;
}

/******************************************************************************/
/**
 * Returns the name of this optimization algorithm
 *
 * @return The name assigned to this optimization algorithm
 */
std::string Go2::getAlgorithmName() const {
	return std::string("Algorithm Combiner");
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
	, const boost::program_options::options_description &userOptions
) {
	namespace po = boost::program_options;

	try {
		std::string maxClientDuration = EMPTYDURATION; // 00:00:00

		std::string optimization_algorithms;
		std::string checkpointFile = "empty";

		// Extract a list of algorithm mnemonics and clear-text descriptions
		std::string algorithm_description;
		std::vector<std::string> keys;
		GOAFactoryStore->getKeyVector(keys);
		std::vector<std::string>::iterator k_it;
		for (k_it = keys.begin(); k_it != keys.end(); ++k_it) {
			algorithm_description += (*k_it + ":  " + GOAFactoryStore->get(*k_it)->getAlgorithmName() + "\n");
		}

		std::ostringstream oa_help;
		oa_help
		<< "A comma-separated list of optimization algorithms, e.g. \"arg1,arg2\". "
		<< GOAFactoryStore->size() << " algorithms have been registered: " << std::endl
		<< algorithm_description;

		// Extract a list of consumer mnemonics and clear-text descriptions
		std::string consumer_description;
		GConsumerStore->getKeyVector(keys);
		for (k_it = keys.begin(); k_it != keys.end(); ++k_it) {
			consumer_description += (*k_it + ":  " + GConsumerStore->get(*k_it)->getConsumerName() + "\n");
		}

		std::ostringstream consumer_help;
		consumer_help
			<< "The name of a consumer for brokered execution (an error will be flagged if called with any other execution mode than (2) ). "
			<< GConsumerStore->size() << " consumers have been registered: " << std::endl
			<< consumer_description;

		std::string usageString = std::string("Usage: ") + argv[0] + " [options]";

		boost::program_options::options_description general(usageString.c_str());
		boost::program_options::options_description basic("Basic options");

		// First add local options
		basic.add_options()
			("help,h", "Emit help message")
			("showAll", "Show all available options")
			("optimizationAlgorithms,a", po::value<std::string>(&optimization_algorithms), oa_help.str().c_str())
			("cp_file,f", po::value<std::string>(&checkpointFile)->default_value("empty"),
			 "A file (including its path) holding a checkpoint for a given optimization algorithm")
			("client", "Indicates that this program should run as a client or in server mode. Note that this setting will trigger an error unless called in conjunction with a consumer capable of dealing with clients")
			("maxClientDuration", po::value<std::string>(&maxClientDuration)->default_value(EMPTYDURATION), "The maximum runtime for a client in the form \"hh:mm:ss\". Note that a client may run longer as this time-frame if its work load still runs. The default value \"00:00:00\" means: \"no time limit\"")
			("consumer,c", po::value<std::string>(&m_consumer_name)->default_value("btc"), consumer_help.str().c_str());

		// Add additional options coming from the algorithms and consumers
		boost::program_options::options_description visible("Global algorithm- and consumer-options");
		boost::program_options::options_description hidden("Hidden algorithm- and consumer-options");

		// Retrieve available command-line options from registered consumers, if any
		if (!GConsumerStore->empty()) {
			GConsumerStore->rewind();
			do {
				GConsumerStore->getCurrentItem()->addCLOptions(visible, hidden);
			} while (GConsumerStore->goToNextPosition());
		}

		// Retrieve available command-line options from registered optimization algorithm factories, if any
		if (!GOAFactoryStore->empty()) {
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
		po::store(po::parse_command_line(argc, (const char *const *) argv, general), vm);

		// Emit a help message, if necessary
		if (vm.count("help") || vm.count("showAll")) { // Allow syntax "programm --help --showAll" and "program --showAll"
			if (vm.count("showAll")) { // Show all options
				std::cout << general << std::endl;
			} else { // Just show a selection
				boost::program_options::options_description selected(usageString.c_str());
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
			glogger
			<< "In Go2::parseCommandLine(): Error!" << std::endl
			<< "You need to specify exactly one consumer for brokered execution," << std::endl
			<< "on the command line. Found " << vm.count("consumer") << "." << std::endl
			<< GEXCEPTION;
		}

		// Check that the requested consumer actually exists
		if (vm.count("consumer") && !GConsumerStore->exists(m_consumer_name)) {
			glogger
			<< "In Go2::parseCommandLine(): Error!" << std::endl
			<< "You have requested a consumer with name " << m_consumer_name << std::endl
			<< "which could not be found in the consumer store." << std::endl
			<< GEXCEPTION;
		}

		if (m_client_mode && !GConsumerStore->get(m_consumer_name)->needsClient()) {
			glogger
			<< "In Go2::parseCommandLine(): Error!" << std::endl
			<< "Requested client mode even though consumer " << m_consumer_name << " does not require a client" <<
			std::endl
			<< GEXCEPTION;
		}

		std::cout << "Using consumer " << m_consumer_name << std::endl;

		// Finally give the consumer the chance to act on the command line options
		// TODO: clone the consumer, then let the clone act on CL options and add the clone to the broker
		GConsumerStore->get(m_consumer_name)->actOnCLOptions(vm);

		// At this point the consumer should be fully configured

		// Register the consumer with the broker, unless other consumers have already been registered or we are running in client mode
		if (!m_client_mode) {
			if (!GBROKER(Gem::Geneva::GParameterSet)->hasConsumers()) {
				GBROKER(Gem::Geneva::GParameterSet)->enrol(GConsumerStore->get(m_consumer_name));
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

			std::vector<std::string>::iterator it;
			for (it = algs.begin(); it != algs.end(); ++it) {
				// Retrieve the algorithm factory from the global store
				std::shared_ptr<GOptimizationAlgorithmFactoryT<GOABase>> p;
				if (!GOAFactoryStore->get(*it, p)) {
					glogger
					<< "In Go2::parseCommandLine(int, char**): Error!" << std::endl
					<< "Got invalid algorithm mnemonic \"" << *it << "\"." << std::endl
					<< "No algorithm found for this string." << std::endl
					<< GEXCEPTION;
				}

				// Retrieve an algorithm from the factory and add it to the list
				m_algorithms_vec.push_back(p->get());
			}
		}

		// Set the name of a checkpoint file (if any)
		m_cp_file = checkpointFile;

		// Set the maximum running time for the client (if any)
		m_max_client_duration = Gem::Common::duration_from_string(maxClientDuration);
	}
	catch (const po::error &e) {
		glogger
		<< "Error parsing the command line:" << std::endl
		<< e.what() << std::endl
		<< GEXCEPTION;
	}
}

/******************************************************************************/
/**
 * Parses a configuration file for configuration options
 *
 * @param configFilename The name of a configuration file to be parsed
 */
void Go2::parseConfigFile(const std::string &configFilename) {
	// Create a parser builder object. It will be destroyed at
	// the end of this scope and thus cannot cause trouble
	// due to registered call-backs and references
	Gem::Common::GParserBuilder gpb;

	// Add local configuration options
	this->addConfigurationOptions(gpb);

	// Do the actual parsing
	if (!gpb.parseConfigFile(configFilename)) {
		glogger
		<< "In Go2::parseConfigFile: Error!" << std::endl
		<< "Could not parse configuration file " << configFilename << std::endl
		<< GTERMINATION;
	}
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
