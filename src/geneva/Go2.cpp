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
	const boost::uint16_t &nProducerThreads
) {
	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Set the number of threads. GRANDOMFACTORY is
	// a singleton that will be initialized by this call.
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);
}

// Regulates access to the call_once facility
boost::once_flag f_go2 = BOOST_ONCE_INIT;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
Go2::Go2()
	: GMutableSetT<GParameterSet>(), clientMode_(GO2_DEF_CLIENTMODE), configFilename_(GO2_DEF_DEFAULTCONFIGFILE),
	  parMode_(GO2_DEF_DEFAULPARALLELIZATIONMODE), consumerName_(GO2_DEF_NOCONSUMER),
	  nProducerThreads_(GO2_DEF_NPRODUCERTHREADS), offset_(GO2_DEF_OFFSET), sorted_(false), iterationsConsumed_(0) {
	//--------------------------------------------
	// Initialize Geneva as well as the known optimization algorithms and consumers

	gi_.registerOAF<GEvolutionaryAlgorithmFactory>();
	gi_.registerOAF<GSwarmAlgorithmFactory>();
	gi_.registerOAF<GGradientDescentFactory>();
	gi_.registerOAF<GSimulatedAnnealingFactory>();
	gi_.registerOAF<GParameterScanFactory>();

	gi_.registerConsumer<GIndividualTCPConsumer>();
	gi_.registerConsumer<GIndividualThreadConsumer>();
	gi_.registerConsumer<GIndividualSerialConsumer>();

	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Initialize all necessary variables
	boost::call_once(f_go2, std::bind(setRNFParameters, nProducerThreads_));
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
	int argc, char **argv, const boost::program_options::options_description &userDescriptions
)
	: GMutableSetT<GParameterSet>(), clientMode_(GO2_DEF_CLIENTMODE), configFilename_(GO2_DEF_DEFAULTCONFIGFILE),
	  parMode_(GO2_DEF_DEFAULPARALLELIZATIONMODE), consumerName_(GO2_DEF_NOCONSUMER),
	  nProducerThreads_(GO2_DEF_NPRODUCERTHREADS), offset_(GO2_DEF_OFFSET), sorted_(false), iterationsConsumed_(0) {
	//--------------------------------------------
	// Initialize Geneva as well as the known optimization algorithms

	gi_.registerOAF<GEvolutionaryAlgorithmFactory>();
	gi_.registerOAF<GSwarmAlgorithmFactory>();
	gi_.registerOAF<GGradientDescentFactory>();
	gi_.registerOAF<GSimulatedAnnealingFactory>();
	gi_.registerOAF<GParameterScanFactory>();

	gi_.registerConsumer<GIndividualTCPConsumer>();
	gi_.registerConsumer<GIndividualThreadConsumer>();
	gi_.registerConsumer<GIndividualSerialConsumer>();

	//--------------------------------------------
	// Load initial configuration options from the command line
	parseCommandLine(argc, argv, userDescriptions);

	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Initialize all necessary variables
	boost::call_once(f_go2, std::bind(setRNFParameters, nProducerThreads_));
}

/******************************************************************************/
/**
 * A constructor that only loads data from a configuration file
 *
 * @param configFilename The name of a configuration file
 */
Go2::Go2(const std::string &configFilename)
	: GMutableSetT<GParameterSet>(), clientMode_(GO2_DEF_CLIENTMODE), configFilename_(configFilename),
	  parMode_(GO2_DEF_DEFAULPARALLELIZATIONMODE), consumerName_(GO2_DEF_NOCONSUMER),
	  nProducerThreads_(GO2_DEF_NPRODUCERTHREADS), offset_(GO2_DEF_OFFSET), sorted_(false), iterationsConsumed_(0),
	  default_algorithm_str_(DEFAULTOPTALG) {
	//--------------------------------------------
	// Initialize Geneva as well as the known optimization algorithms

	gi_.registerOAF<GEvolutionaryAlgorithmFactory>();
	gi_.registerOAF<GSwarmAlgorithmFactory>();
	gi_.registerOAF<GGradientDescentFactory>();
	gi_.registerOAF<GSimulatedAnnealingFactory>();
	gi_.registerOAF<GParameterScanFactory>();

	gi_.registerConsumer<GIndividualTCPConsumer>();
	gi_.registerConsumer<GIndividualThreadConsumer>();
	gi_.registerConsumer<GIndividualSerialConsumer>();

	//--------------------------------------------
	// Parse configuration file options
	this->parseConfigFile(configFilename);

	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Initialize all necessary variables
	boost::call_once(f_go2, std::bind(setRNFParameters, nProducerThreads_));
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
	int argc, char **argv, const std::string &configFilename,
	const boost::program_options::options_description &userDescriptions
)
	: GMutableSetT<GParameterSet>(), clientMode_(GO2_DEF_CLIENTMODE), configFilename_(configFilename),
	  parMode_(GO2_DEF_DEFAULPARALLELIZATIONMODE), consumerName_(GO2_DEF_NOCONSUMER),
	  nProducerThreads_(GO2_DEF_NPRODUCERTHREADS), offset_(GO2_DEF_OFFSET), sorted_(false), iterationsConsumed_(0),
	  default_algorithm_str_(DEFAULTOPTALG) {
	//--------------------------------------------
	// Initialize Geneva as well as the known optimization algorithms

	gi_.registerOAF<GEvolutionaryAlgorithmFactory>();
	gi_.registerOAF<GSwarmAlgorithmFactory>();
	gi_.registerOAF<GGradientDescentFactory>();
	gi_.registerOAF<GSimulatedAnnealingFactory>();
	gi_.registerOAF<GParameterScanFactory>();

	gi_.registerConsumer<GIndividualTCPConsumer>();
	gi_.registerConsumer<GIndividualThreadConsumer>();
	gi_.registerConsumer<GIndividualSerialConsumer>();

	//--------------------------------------------
	// Parse configuration file options
	this->parseConfigFile(configFilename);

	//--------------------------------------------
	// Load configuration options from the command line
	parseCommandLine(argc, argv, userDescriptions);

	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Initialize all necessary variables
	boost::call_once(f_go2, std::bind(setRNFParameters, nProducerThreads_));
}

/******************************************************************************/
/**
 * The copy constructor
 */
Go2::Go2(const Go2 &cp)
	: GMutableSetT<GParameterSet>(cp), clientMode_(cp.clientMode_), configFilename_(cp.configFilename_),
	  parMode_(cp.parMode_), consumerName_(cp.consumerName_), nProducerThreads_(cp.nProducerThreads_),
	  offset_(cp.offset_), sorted_(cp.sorted_), iterationsConsumed_(0), default_algorithm_str_(DEFAULTOPTALG) {
	//--------------------------------------------
	// Initialize Geneva as well as the known optimization algorithms

	gi_.registerOAF<GEvolutionaryAlgorithmFactory>();
	gi_.registerOAF<GSwarmAlgorithmFactory>();
	gi_.registerOAF<GGradientDescentFactory>();
	gi_.registerOAF<GSimulatedAnnealingFactory>();
	gi_.registerOAF<GParameterScanFactory>();

	gi_.registerConsumer<GIndividualTCPConsumer>();
	gi_.registerConsumer<GIndividualThreadConsumer>();
	gi_.registerConsumer<GIndividualSerialConsumer>();

	//--------------------------------------------
	// Copy the algorithms vectors over
	copyGenevaSmartPointerVector(cp.cl_algorithms_, cl_algorithms_);
	copyGenevaSmartPointerVector(cp.algorithms_, algorithms_);

	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Initialize all necessary variables
	boost::call_once(f_go2, std::bind(setRNFParameters, nProducerThreads_));

	//--------------------------------------------
	// Copy the default algorithm over, if any
	copyGenevaSmartPointer<GOABase>(cp.default_algorithm_, default_algorithm_);
}

/******************************************************************************/
/**
 * The destructor
 */
Go2::~Go2() {
	this->clear(); // Get rid of the local individuals
	algorithms_.clear(); // Get rid of the optimization algorithms
	cl_algorithms_.clear(); // Get rid of algorithms registered on the command line
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
		this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
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
		this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
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

	// Check that we are indeed dealing with a GBaseEA reference
	const Go2 *p_load = GObject::gobject_conversion<Go2>(&cp);

	GToken token("Go2", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GMutableSetT<GParameterSet> >(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	compare_t(IDENTITY(clientMode_, p_load->clientMode_), token);
	compare_t(IDENTITY(configFilename_, p_load->configFilename_), token);
	compare_t(IDENTITY(parMode_, p_load->parMode_), token);
	compare_t(IDENTITY(consumerName_, p_load->consumerName_), token);
	compare_t(IDENTITY(nProducerThreads_, p_load->nProducerThreads_), token);
	compare_t(IDENTITY(offset_, p_load->offset_), token);
	compare_t(IDENTITY(sorted_, p_load->sorted_), token);
	compare_t(IDENTITY(iterationsConsumed_, p_load->iterationsConsumed_), token);
	compare_t(IDENTITY(default_algorithm_, p_load->default_algorithm_), token);

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
	std::shared_ptr <GOptimizationAlgorithmFactoryT<GOABase>> p;
	if (!GOAFactoryStore->get(mn, p)) {
		glogger
		<< "In Go2::registerDefaultAlgorithm(std::string): Error!" << std::endl
		<< "Got invalid algorithm mnemonic " << mn << std::endl
		<< GEXCEPTION;
	}

	this->registerDefaultAlgorithm(p->get(parMode_));
}

/******************************************************************************/
/**
 * Allows to register a default algorithm to be used when no other algorithms
 * have been specified. When others have been specified, this algorithm will
 * not be used. Note that any individuals registered with the default algorithm
 * will be copied into the Go2 object.
 */
void Go2::registerDefaultAlgorithm(std::shared_ptr < GOABase > default_algorithm) {
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
	default_algorithm_ = default_algorithm;
}

/******************************************************************************/
/**
 * Retrieves a parameter of a given type at the specified position
 */
boost::any Go2::getVarVal(
	const std::string &descr, const boost::tuple<std::size_t, std::string, std::size_t> &target
) {
	return this->GOptimizableI::getBestIndividual<GParameterSet>()->getVarVal(descr, target);
}

/******************************************************************************/
/**
 * Allows to register a pluggable optimization monitor
 */
void Go2::registerPluggableOM(
	std::shared_ptr<GBasePluggableOMT<GOABase> > pluggableOM
) {
	if (pluggableOM) {
		pluggableOM_ = pluggableOM;
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
	pluggableOM_.reset();
}

/******************************************************************************/
/**
 * Loads the data of another Go2 object
 *
 * @param cp A copy of another Go2 object, camouflaged as a GObject
 */
void Go2::load_(const GObject *cp) {
	const Go2 *p_load = gobject_conversion<Go2>(cp);

	// First load the parent class'es data ...
	GMutableSetT<GParameterSet>::load_(cp);

	// and then our local data
	clientMode_ = p_load->clientMode_;
	configFilename_ = p_load->configFilename_;
	parMode_ = p_load->parMode_;
	consumerName_ = p_load->consumerName_;
	nProducerThreads_ = p_load->nProducerThreads_;
	offset_ = p_load->offset_;
	sorted_ = p_load->sorted_;
	iterationsConsumed_ = p_load->iterationsConsumed_;

	copyGenevaSmartPointer<GOABase>(p_load->default_algorithm_, default_algorithm_);

	// Copy the algorithm vectors over
	copyGenevaSmartPointerVector(p_load->algorithms_, algorithms_);
	copyGenevaSmartPointerVector(p_load->cl_algorithms_, cl_algorithms_);

	// Cross check other data has been added
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
		GO2_DEF_NOCONSUMER == consumerName_
		|| !GConsumerStore->exists(consumerName_)
		) {
		glogger
		<< "In Go2::clientRun(): Error!" << std::endl
		<< "Received invalid consumer name: " << consumerName_ << std::endl
		<< GEXCEPTION;
	}

	// Retrieve the client worker from the consumer
	std::shared_ptr <Gem::Courtier::GBaseClientT<Gem::Geneva::GParameterSet>> p;

	if (GConsumerStore->get(consumerName_)->needsClient()) {
		p = GConsumerStore->get(consumerName_)->getClient();
	} else {
		glogger
		<< "In Go2::clientRun(): Error!" << std::endl
		<< "Trying to execute clientRun() on consumer " << consumerName_ << std::endl
		<< "which does not require a client" << std::endl
		<< GEXCEPTION;
	}

	// Check for errors
	if (!p) {
		glogger
		<< "In Go2::clientRun(): Error!" << std::endl
		<< "Received empty client from consumer " << consumerName_ << std::endl
		<< GEXCEPTION;
	}

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
	return clientMode_;
}

/******************************************************************************/
/**
 * Allows to set the parallelization mode used for the optimization. Note that
 * this setting will only have an effect on algorithms that have not been explicitly
 * added to Go2 and only to those algorithms that have been added after the parMode_
 * has been set.
 *
 * @param execMode The parallelization mode used for the optimization
 */
void Go2::setParallelizationMode(const execMode &parMode) {
	parMode_ = parMode;
}

/******************************************************************************/
/**
 * Allows to retrieve the parallelization mode currently used for the optimization
 *
 * @return The parallelization mode currently used for the optimization
 */
execMode Go2::getParallelizationMode() const {
	return parMode_;
}

/******************************************************************************/
/**
 * Allows to randomly initialize parameter members. Note that for this wrapper object
 * this function doesn't make any sense. It is made available to satisfy a requirement
 * of GOptimizableEntity.
 */
bool Go2::randomInit(const activityMode &) {
	return false;
}

/******************************************************************************/
/**
 * (Primary) fitness calculation for an optimization algorithm means optimization. The fitness
 * is then determined by the best individual which, after the end of the optimization cycle.
 *
 * @param id The id of the target function (ignored here)
 * @return The fitness of the best individual in the population
 */
double Go2::fitnessCalculation() {
	// Make sure all optimization work has been carried out
	std::shared_ptr <GParameterSet> p = this->GOptimizableI::optimize<GParameterSet>(offset_ + iterationsConsumed_);

	// We use the raw fitness rather than the transformed fitness,
	// as this is custom also for "normal" individuals. Re-evaluation
	// should never happen at this point.
	return p->fitness(0, Gem::Geneva::PREVENTREEVALUATION, Gem::Geneva::USERAWFITNESS);
}

/******************************************************************************/
/**
 * Retrieves the currently registered number of algorithms
 */
std::size_t Go2::getNAlgorithms() const {
	return algorithms_.size();
}

/******************************************************************************/
/**
 * Retrieves the currently registered number of command line algorithms
 */
std::size_t Go2::getNCLAlgorithms() const {
	return cl_algorithms_.size();
}

/******************************************************************************/
/**
 * Allows to add an optimization algorithm to the chain. If any individuals have
 * been registered, the algorithm will unload them.
 *
 * @param alg A base pointer to another optimization algorithm
 */
void Go2::addAlgorithm(std::shared_ptr < GOABase > alg) {
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

	algorithms_.push_back(alg);
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
Go2 &Go2::operator&(std::shared_ptr < GOABase > alg) {
	this->addAlgorithm(alg);
	return *this;
}

/***************************************************************************/
/**
 * Allows to add an optimization algorithm through its mnemonic
 */
void Go2::addAlgorithm(const std::string &mn) {
	// Retrieve the algorithm from the global store
	std::shared_ptr <GOptimizationAlgorithmFactoryT<GOABase>> p;
	if (!GOAFactoryStore->get(mn, p)) {
		glogger
		<< "In Go2::addAlgorithm(std::string): Error!" << std::endl
		<< "Got invalid algorithm mnemonic " << mn << std::endl
		<< GEXCEPTION;
	}

	this->addAlgorithm(p->get(parMode_)); // The factory might add a monitor to the object
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
	std::shared_ptr < Gem::Common::GFactoryT<GParameterSet> > cc_ptr
) {
	if (!cc_ptr) {
		glogger
		<< "In Go2::registerContentCreator(): Error!" << std::endl
		<< "Tried to register an empty pointer" << std::endl
		<< GEXCEPTION;
	}

	contentCreatorPtr_ = cc_ptr;
}

/******************************************************************************/
/**
 * Perform the actual optimization cycle. Note that we assume that individuals
 * have either been registered with the Go2 object or with the first algorithm
 * which has been added to the object.
 *
 * @param offset An offset at which the first algorithm should start
 */
void Go2::optimize(const boost::uint32_t &offset) {
	// Algorithms specified manually in main() take precedence
	// before those specified on the command line. E.g., a line
	// "go & ea_ptr;" (where ea_ptr pointed to an evolutionary
	// algorithm) will add this algorithm as the first entry to
	// the algorithms_ vector.
	if (!cl_algorithms_.empty()) {
		// Add algorithms that have been specified on the command line
		std::vector<std::shared_ptr < GOABase> > ::iterator
		pers_it;
		for (pers_it = cl_algorithms_.begin(); pers_it != cl_algorithms_.end(); ++pers_it) {
			this->addAlgorithm(*pers_it);
		}
		cl_algorithms_.clear();
	}

	// Check that algorithms have indeed been registered. If not, try to add a default algorithm
	if (algorithms_.empty()) {
		if (!default_algorithm_) {
			// No algorithms given, no default algorithm specified by the user:
			// Simply add the Geneva-side default algorithm
			this->registerDefaultAlgorithm(default_algorithm_str_);

			glogger
			<< "In Go2::optimize(): INFORMATION:" << std::endl
			<< "No user-defined optimization algorithm available." << std::endl
			<< "Using default algorithm \"" << default_algorithm_str_ << "\" instead." << std::endl
			<< GLOGGING;
		}

		algorithms_.push_back(default_algorithm_->clone<GOABase>());
	}

	// Check that individuals have been registered
	if (this->empty()) {
		if (contentCreatorPtr_) {
			for (std::size_t ind = 0; ind < algorithms_.at(0)->getDefaultPopulationSize(); ind++) {
				std::shared_ptr <GParameterSet> p_ind = (*contentCreatorPtr_)();
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

	// Loop over all algorithms
	iterationsConsumed_ = offset_;
	sorted_ = false;
	GOABase::iterator ind_it;
	std::vector<std::shared_ptr < GOABase> > ::iterator
	alg_it;
	for (alg_it = algorithms_.begin(); alg_it != algorithms_.end(); ++alg_it) {
		std::shared_ptr <GOABase> p_base = (*alg_it);

		// Add the pluggable optimization monitor to the algorithm, if it is available
		if (pluggableOM_) {
			p_base->getOptimizationMonitor()->registerPluggableOM(pluggableOM_);
		}

		// Add the individuals to the algorithm.
		for (ind_it = this->begin(); ind_it != this->end(); ++ind_it) {
			p_base->push_back(*ind_it);
		}

		// Remove our local copies
		this->clear();

#ifdef DEBUG
      if(!this->empty()) {
         glogger
         << "Go2 collection still contains " << this->size() << " individuals after clear()" << std::endl
         << GEXCEPTION;
      }
#endif /* DEBUG */

		// Do the actual optimization
		p_base->GOptimizableI::optimize<GParameterSet>(iterationsConsumed_);

		// Make sure we start with the correct iteration in the next algorithm
		iterationsConsumed_ = p_base->getIteration();

		// Unload the individuals from the last algorithm and store them again in this object
		std::vector<std::shared_ptr < GParameterSet> > bestIndividuals =
			p_base->GOptimizableI::getBestIndividuals < GParameterSet > ();
		std::vector<std::shared_ptr < GParameterSet> > ::iterator
		best_it;
		for (best_it = bestIndividuals.begin(); best_it != bestIndividuals.end(); ++best_it) {
			this->push_back(*best_it);
		}

		bestIndividuals.clear();
		p_base->clear();
	}

	// Sort the individuals according to their primary fitness so we have it easier later on
	// to extract the best individuals found.
	std::sort(
		this->begin(), this->end(), [](std::shared_ptr <GParameterSet> x, std::shared_ptr <GParameterSet> y) -> bool {
			return x->minOnly_fitness() < y->minOnly_fitness();
		}
	);

	sorted_ = true;
}

/******************************************************************************/
/**
 * Retrieves the best individual found. This function returns a base pointer.
 * Conversion is done through a function stored in GOptimizableI.
 *
 * @return The best individual found
 */
std::shared_ptr <Gem::Geneva::GParameterSet> Go2::customGetBestIndividual() {
	Go2::iterator it;

	// Do some error checking
	if (this->empty()) {
		glogger
		<< "In Go2::customGetBestIndividual(): Error!" << std::endl
		<< "No individuals found" << std::endl
		<< GEXCEPTION;
	}

	for (it = this->begin(); it != this->end(); ++it) {
		if ((*it)->isDirty()) {
			glogger
			<< "In Go2::customGetBestIndividual(): Error!" << std::endl
			<< "Found individual in position " << std::distance(this->begin(), it) << " whose dirty flag is set" <<
			std::endl
			<< GEXCEPTION;
		}
	}

	if (!sorted_) {
		glogger
		<< "In Go2::customGetBestIndividual(): Error!" << std::endl
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
std::vector<std::shared_ptr < Gem::Geneva::GParameterSet> >

Go2::customGetBestIndividuals() {
	Go2::iterator it;

	// Do some error checking
	if (this->empty()) {
		glogger
		<< "In Go2::customGetBestIndividuals(): Error!" << std::endl
		<< "No individuals found" << std::endl
		<< GEXCEPTION;
	}

	for (it = this->begin(); it != this->end(); ++it) {
		if ((*it)->isDirty()) {
			glogger
			<< "In Go2::customGetBestIndividuals(): Error!" << std::endl
			<< "Found individual in position " << std::distance(this->begin(), it) << " whose dirty flag is set" <<
			std::endl
			<< GEXCEPTION;
		}
	}

	std::vector<std::shared_ptr < Gem::Geneva::GParameterSet> > bestIndividuals;
	for (it = this->begin(); it != this->end(); ++it) {
		// This will result in an implicit downcast
		bestIndividuals.push_back(*it);
	}

	return bestIndividuals;
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
	GMutableSetT<GParameterSet>::addConfigurationOptions(gpb);

	// Add local data
	gpb.registerFileParameter<boost::uint16_t>(
		"nProducerThreads" // The name of the first variable
		, GO2_DEF_NPRODUCERTHREADS, [this](boost::uint16_t npt) { this->setNProducerThreads(npt); }
	)
	<< "The number of threads simultaneously producing random numbers";
}

/******************************************************************************/
/**
 * Allows to assign a name to the role of this individual(-derivative). This is mostly important for the
 * GBrokerEA class which should prevent objects of its type from being stored as an individual in its population.
 * All other objects do not need to re-implement this function (unless they rely on the name for some reason).
 */
std::string Go2::getIndividualCharacteristic() const {
	return std::string("GENEVA_GO2WRAPPER");
}

/******************************************************************************/
/**
 * Allows to mark this object as belonging to a client as opposed to a server
 *
 * @param serverMode Allows to mark this object as belonging to a client as opposed to a server
 */
void Go2::setClientMode(bool clientMode) {
	clientMode_ = clientMode;
}

/******************************************************************************/
/**
 * Allows to check whether this object is working in server or client mode
 *
 * @return A boolean indicating whether this object is working in server or client mode
 */
bool Go2::getClientMode() const {
	return clientMode_;
}

/******************************************************************************/
/**
 * Allows to set the number of threads that will simultaneously produce random numbers.
 *
 * @param nProducerThreads The number of threads that will simultaneously produce random numbers
 */
void Go2::setNProducerThreads(const boost::uint16_t &nProducerThreads) {
	nProducerThreads_ = nProducerThreads;
}

/******************************************************************************/
/**
 * Allows to retrieve the number of threads that will simultaneously produce random numbers.
 *
 * @return The number of threads that will simultaneously produce random numbers
 */
boost::uint16_t Go2::getNProducerThreads() const {
	return nProducerThreads_;
}

/******************************************************************************/
/**
 * Allows to specify the offset with which the iteration counter should start. This is
 * important when using more than one optimization algorithm with different Go2 objects.
 *
 * @param offset The offset with which the iteration counter should start
 */
void Go2::setOffset(const boost::uint32_t &offset) {
	offset_ = offset;
}

/******************************************************************************/
/**
 * Retrieval of the current iteration
 */
uint32_t Go2::getIteration() const {
	return iterationsConsumed_;
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
boost::uint32_t Go2::getIterationOffset() const {
	return offset_;
}

/******************************************************************************/
/**
 *
 * @param argc The number of command line arguments
 * @param argv An array with the arguments
 * @param od A program_options object for user-defined command line options
 */
void Go2::parseCommandLine(
	int argc, char **argv, const boost::program_options::options_description &userOptions
) {
	namespace po = boost::program_options;

	try {
		std::string optimization_algorithms;

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
		<<
		"The name of a consumer for brokered execution (an error will be flagged if called with any other execution mode than (2). "
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
			("executionMode,e", po::value<execMode>(&parMode_)->default_value(GO2_DEF_DEFAULPARALLELIZATIONMODE),
			 "The execution mode: (0) means serial execution (1) means multi-threaded execution and (2) means execution through the broker. Note that you need to specifiy a consumer")
			("client",
			 "Indicates that this program should run as a client or in server mode. Note that this setting will trigger an error unless called in conjunction with a consumer capable of dealing with clients")
			("consumer,c", po::value<std::string>(&consumerName_), consumer_help.str().c_str());

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
		po::store(po::parse_command_line(argc, argv, general), vm);

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
			clientMode_ = true;
		}

		// If the user has requested brokered execution, do corresponding error checks
		// and prepare the environment as required
		if (EXECMODE_BROKERAGE == parMode_) {
			// No consumer specified, although brokered execution was requested
			if (vm.count("consumer") != 1) {
				glogger
				<< "In Go2::parseCommandLine(): Error!" << std::endl
				<< "You need to specify exactly one consumer for brokered execution," << std::endl
				<< "on the command line. Found " << vm.count("consumer") << "." << std::endl
				<< GEXCEPTION;
			}

			// Check that the requested consumer actually exists
			if (vm.count("consumer") && !GConsumerStore->exists(consumerName_)) {
				glogger
				<< "In Go2::parseCommandLine(): Error!" << std::endl
				<< "You have requested a consumer with name " << consumerName_ << std::endl
				<< "which could not be found in the consumer store." << std::endl
				<< GEXCEPTION;
			}

			if (clientMode_ && !GConsumerStore->get(consumerName_)->needsClient()) {
				glogger
				<< "In Go2::parseCommandLine(): Error!" << std::endl
				<< "Requested client mode even though consumer " << consumerName_ << " does not require a client" <<
				std::endl
				<< GEXCEPTION;
			}

			std::cout << "Using consumer " << consumerName_ << std::endl;

			// Finally give the consumer the chance to act on the command line options
			GConsumerStore->get(consumerName_)->actOnCLOptions(vm);

			// At this point the consumer should be fully configured

			// Register the consumer with the broker, unless other consumers have already been registered or we are running in client mode
			if (!clientMode_) {
				if (!GBROKER(Gem::Geneva::GParameterSet)->hasConsumers()) {
					GBROKER(Gem::Geneva::GParameterSet)->enrol(GConsumerStore->get(consumerName_));
				} else {
					glogger
					<< "In Go2::parseCommandLine(): Note!" << std::endl
					<< "Could not register requested consumer," << std::endl
					<< "as a consumer has already registered with the broker" << std::endl
					<< GLOGGING;
				}
			}
		} else { // not in brokered mode. No consumers to be taken into account
			// Complain if a consumer was specified, but we are not dealing with brokered execution
			if (vm.count("consumer")) {
				glogger
				<< "In Go2::parseCommandLine(): Error!" << std::endl
				<< "You have specified a consumer but have requested " << std::endl
				<< "an execution mode " << parMode_ << " where " << EXECMODE_BROKERAGE << " was expected" << std::endl
				<< GEXCEPTION;
			}

			if (clientMode_) {
				glogger
				<< "Requested client mode even though we are not running in brokered mode" << std::endl
				<< GEXCEPTION;
			}
		}

		// Parse the list of optimization algorithms
		if (vm.count("optimizationAlgorithms")) {
			std::vector<std::string> algs = Gem::Common::splitString(optimization_algorithms, ",");

			std::vector<std::string>::iterator it;
			for (it = algs.begin(); it != algs.end(); ++it) {
				// Retrieve the algorithm factory from the global store
				std::shared_ptr <GOptimizationAlgorithmFactoryT<GOABase>> p;
				if (!GOAFactoryStore->get(*it, p)) {
					glogger
					<< "In Go2::parseCommandLine(int, char**): Error!" << std::endl
					<< "Got invalid algorithm mnemonic \"" << *it << "\"." << std::endl
					<< "No algorithm found for this string." << std::endl
					<< GEXCEPTION;
				}

				// Retrieve an algorithm from the factory and add it to the list
				cl_algorithms_.push_back(p->get(parMode_));
			}
		}
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
