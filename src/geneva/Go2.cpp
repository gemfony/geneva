/**
 * @file Go2.cpp
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

#include "geneva/Go2.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Go2)

namespace Gem {
namespace Geneva {

/**************************************************************************************/
/**
 * Set a number of parameters of the random number factory
 *
 * @param nProducerThreads The number of threads simultaneously producing random numbers
 * @arraySize The size of individual random number packages
 */
void setRNFParameters(
		const boost::uint16_t& nProducerThreads
		, const std::size_t& arraySize
) {
	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Set the number of threads. GRANDOMFACTORY is
	// a singleton that will be initialized by this call.
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads);
	GRANDOMFACTORY->setArraySize(arraySize);
}

/** @brief Regulates access to the call_once facility*/
boost::once_flag f_go2 = BOOST_ONCE_INIT;

/**************************************************************************************/
////////////////////////////////////////////////////////////////////////////////////////
/**************************************************************************************/
/**
 * The default constructor
 */
Go2::Go2()
	: GMutableSetT<GParameterSet>()
	, clientMode_(GO2_DEF_CLIENTMODE)
	, serializationMode_(GO2_DEF_SERIALIZATIONMODE)
	, ip_(GO2_DEF_IP)
	, port_(GO2_DEF_PORT)
	, configFilename_(GO2_DEF_DEFAULTCONFIGFILE)
	, parMode_(GO2_DEF_DEFAULPARALLELIZATIONMODE)
	, verbose_(GO2_DEF_DEFAULTVERBOSE)
	, maxStalledDataTransfers_(GO2_DEF_MAXSTALLED)
	, maxConnectionAttempts_(GO2_DEF_MAXCONNATT)
	, returnRegardless_(GO2_DEF_RETURNREGARDLESS)
	, nProducerThreads_(GO2_DEF_NPRODUCERTHREADS)
	, arraySize_(GO2_DEF_ARRAYSIZE)
	, offset_(GO2_DEF_OFFSET)
	, sorted_(false)
	, iterationsConsumed_(0)
{
	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Initialize all necessary variables
	boost::call_once(f_go2, boost::bind(setRNFParameters, nProducerThreads_, arraySize_));

	//--------------------------------------------
	// Store a local clone of this object so we can
	// restore the original settings later
	tmpl_ptr = boost::shared_ptr<Go2>(new Go2(*this, NOCLONE));
}


/**************************************************************************************/
/**
 * A constructor that first parses the command line for relevant parameters and then
 * loads data from a configuration file
 *
 * @param argc The number of command line arguments
 * @param argv An array with the arguments
 */
Go2::Go2(int argc, char **argv)
	: GMutableSetT<GParameterSet>()
	, clientMode_(GO2_DEF_CLIENTMODE)
	, serializationMode_(GO2_DEF_SERIALIZATIONMODE)
	, ip_(GO2_DEF_IP)
	, port_(GO2_DEF_PORT)
	, configFilename_(GO2_DEF_DEFAULTCONFIGFILE)
	, parMode_(GO2_DEF_DEFAULPARALLELIZATIONMODE)
	, verbose_(GO2_DEF_DEFAULTVERBOSE)
	, maxStalledDataTransfers_(GO2_DEF_MAXSTALLED)
	, maxConnectionAttempts_(GO2_DEF_MAXCONNATT)
	, returnRegardless_(GO2_DEF_RETURNREGARDLESS)
	, nProducerThreads_(GO2_DEF_NPRODUCERTHREADS)
	, arraySize_(GO2_DEF_ARRAYSIZE)
	, offset_(GO2_DEF_OFFSET)
	, sorted_(false)
	, iterationsConsumed_(0)
{
	//--------------------------------------------
	// Load initial configuration options from the command line
	parseCommandLine(argc, argv);

	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Initialize all necessary variables
	boost::call_once(f_go2, boost::bind(setRNFParameters, nProducerThreads_, arraySize_));

	//--------------------------------------------
	// Store a local clone of this object so we can
	// restore the original settings later
	tmpl_ptr = boost::shared_ptr<Go2>(new Go2(*this, NOCLONE));
}

/**************************************************************************************/
/**
 * A constructor that first parses the command line for relevant parameters and then
 * loads data from a configuration file
 *
 * @param argc The number of command line arguments
 * @param argv An array with the arguments
 * @param configFilename The name of a configuration file
 */
Go2::Go2(int argc, char **argv, const std::string& configFilename)
	: GMutableSetT<GParameterSet>()
	, clientMode_(GO2_DEF_CLIENTMODE)
	, serializationMode_(GO2_DEF_SERIALIZATIONMODE)
	, ip_(GO2_DEF_IP)
	, port_(GO2_DEF_PORT)
	, configFilename_(configFilename)
	, parMode_(GO2_DEF_DEFAULPARALLELIZATIONMODE)
	, verbose_(GO2_DEF_DEFAULTVERBOSE)
	, maxStalledDataTransfers_(GO2_DEF_MAXSTALLED)
	, maxConnectionAttempts_(GO2_DEF_MAXCONNATT)
	, returnRegardless_(GO2_DEF_RETURNREGARDLESS)
	, nProducerThreads_(GO2_DEF_NPRODUCERTHREADS)
	, arraySize_(GO2_DEF_ARRAYSIZE)
	, offset_(GO2_DEF_OFFSET)
	, sorted_(false)
	, iterationsConsumed_(0)
{
	//--------------------------------------------
	// Load initial configuration options from the command line
	parseCommandLine(argc, argv);

	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Initialize all necessary variables
	boost::call_once(f_go2, boost::bind(setRNFParameters, nProducerThreads_, arraySize_));

	//--------------------------------------------
	// Store a local clone of this object so we can
	// restore the original settings later
	tmpl_ptr = boost::shared_ptr<Go2>(new Go2(*this, NOCLONE));
}


/**************************************************************************************/
/**
 * A constructor that is given the usual command line parameters, then loads the
 * rest of the data from a config file.
 *
 * @param clientMode Indicates whether this object should operate in (networked) client mode
 * @param sM Specifies whether serialization should happen in XML, Text oder Binary mode
 * @param ip Specifies the ip under which the server can be reached
 * @param port Specifies the port under which the server can be reached
 * @param configFilename Determines the name of the configuration file from which additional options will be loaded
 * @param verbose Specifies whether additional information about parsed parameters should be emitted
 */
Go2::Go2(
	const bool& clientMode
	, const Gem::Common::serializationMode& sM
	, const std::string& ip
	, const unsigned short& port
	, const std::string& configFilename
	, const parMode& pm
	, const bool& verbose
)
	: GMutableSetT<GParameterSet>()
	, clientMode_(clientMode)
	, serializationMode_(sM)
	, ip_(ip)
	, port_(port)
	, configFilename_(configFilename)
	, parMode_(pm)
	, verbose_(verbose)
	, maxStalledDataTransfers_(GO2_DEF_MAXSTALLED)
	, maxConnectionAttempts_(GO2_DEF_MAXCONNATT)
	, returnRegardless_(GO2_DEF_RETURNREGARDLESS)
	, nProducerThreads_(GO2_DEF_NPRODUCERTHREADS)
	, arraySize_(GO2_DEF_ARRAYSIZE)
	, offset_(GO2_DEF_OFFSET)
	, sorted_(false)
	, iterationsConsumed_(0)
{
	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Initialize all necessary variables
	boost::call_once(f_go2, boost::bind(setRNFParameters, nProducerThreads_, arraySize_));

	//--------------------------------------------
	// Store a local clone of this object so we can
	// restore the original settings later
	tmpl_ptr = boost::shared_ptr<Go2>(new Go2(*this, NOCLONE));
}

/**************************************************************************************/
/**
 * The copy constructor
 */
Go2::Go2(const Go2& cp)
	: GMutableSetT<GParameterSet>(cp)
	, clientMode_(cp.clientMode_)
	, serializationMode_(cp.serializationMode_)
	, ip_(cp.ip_)
	, port_(cp.port_)
	, configFilename_(cp.configFilename_)
	, parMode_(cp.parMode_)
	, verbose_(cp.verbose_)
	, maxStalledDataTransfers_(cp.maxStalledDataTransfers_)
	, maxConnectionAttempts_(cp.maxConnectionAttempts_)
	, returnRegardless_(cp.returnRegardless_)
	, nProducerThreads_(cp.nProducerThreads_)
	, arraySize_(cp.arraySize_)
	, offset_(cp.offset_)
	, sorted_(cp.sorted_)
	, iterationsConsumed_(0)
{
	// Copy the best individual over (if any)
	copyGenevaSmartPointer<GParameterSet>(cp.bestIndividual_, bestIndividual_);

	// Copy the algorithms vector over
	copyAlgorithmsVector(cp.algorithms_, algorithms_);

	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Initialize all necessary variables
	boost::call_once(f_go2, boost::bind(setRNFParameters, nProducerThreads_, arraySize_));

	//--------------------------------------------
	// Store a local clone of this object so we can
	// restore the original settings later
	tmpl_ptr = boost::shared_ptr<Go2>(new Go2(*this, NOCLONE));
}

/**************************************************************************************/
/**
 * Copy construction without cloning
 */
Go2::Go2(const Go2& cp, bool noClone)
	: GMutableSetT<GParameterSet>(cp)
	, clientMode_(cp.clientMode_)
	, serializationMode_(cp.serializationMode_)
	, ip_(cp.ip_)
	, port_(cp.port_)
	, configFilename_(cp.configFilename_)
	, parMode_(cp.parMode_)
	, verbose_(cp.verbose_)
	, maxStalledDataTransfers_(cp.maxStalledDataTransfers_)
	, maxConnectionAttempts_(cp.maxConnectionAttempts_)
	, returnRegardless_(cp.returnRegardless_)
	, nProducerThreads_(cp.nProducerThreads_)
	, arraySize_(cp.arraySize_)
	, offset_(cp.offset_)
	, sorted_(cp.sorted_)
	, iterationsConsumed_(0)
{
	// Copy the best individual over (if any)
	copyGenevaSmartPointer<GParameterSet>(cp.bestIndividual_, bestIndividual_);

	// Copy the algorithms vector over
	copyAlgorithmsVector(cp.algorithms_, algorithms_);
}

/**************************************************************************************/
/**
 * The destructor
 */
Go2::~Go2() {
	this->clear(); // Get rid of the local individuals
	bestIndividual_.reset(); // Get rid of the stored best individual
	algorithms_.clear(); // Get rid of the optimization algorithms
}

/**************************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another Go2 object
 * @return A constant reference to this object
 */
const Go2& Go2::operator=(const Go2& cp) {
	Go2::load_(&cp);
	return *this;
}

/**************************************************************************************/
/**
 * Checks for equality with another Go2 object
 *
 * @param  cp A constant reference to another Go2 object
 * @return A boolean indicating whether both objects are equal
 */
bool Go2::operator==(const Go2& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"Go2::operator==","cp", CE_SILENT);
}

/**************************************************************************************/
/**
 * Checks for inequality with another Go2 object
 *
 * @param  cp A constant reference to another Go2 object
 * @return A boolean indicating whether both objects are inequal
 */
bool Go2::operator!=(const Go2& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"Go2::operator!=","cp", CE_SILENT);
}

/**************************************************************************************/
/**
 * Checks whether a given expectation for the relationship between this object and another object
 * is fulfilled.
 *
 * @param cp A constant reference to another object, camouflaged as a GObject
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 * @param caller An identifier for the calling entity
 * @param y_name An identifier for the object that should be compared to this one
 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
 */
boost::optional<std::string> Go2::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const {
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GOptimizationMonitorT reference
	const Go2 *p_load = GObject::gobject_conversion<Go2>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GMutableSetT<GParameterSet>::checkRelationshipWith(cp, e, limit, "Go2", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "Go2", clientMode_, p_load->clientMode_, "clientMode_", "p_load->clientMode_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", serializationMode_, p_load->serializationMode_, "serializationMode_", "p_load->serializationMode_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", ip_, p_load->ip_, "ip_", "p_load->ip_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", port_, p_load->port_, "port_", "p_load->port_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", configFilename_, p_load->configFilename_, "configFilename_", "p_load->configFilename_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", parMode_, p_load->parMode_, "parMode_", "p_load->parMode_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", verbose_, p_load->verbose_, "verbose_", "p_load->verbose_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", maxStalledDataTransfers_, p_load->maxStalledDataTransfers_, "maxStalledDataTransfers_", "p_load->maxStalledDataTransfers_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", maxConnectionAttempts_, p_load->maxConnectionAttempts_, "maxConnectionAttempts_", "p_load->maxConnectionAttempts_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", returnRegardless_, p_load->returnRegardless_, "returnRegardless_", "p_load->returnRegardless_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", nProducerThreads_, p_load->nProducerThreads_, "nProducerThreads_", "p_load->nProducerThreads_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", arraySize_, p_load->arraySize_, "arraySize_", "p_load->arraySize_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", offset_, p_load->offset_, "offset_", "p_load->offset_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", sorted_, p_load->sorted_, "sorted_", "p_load->sorted_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", iterationsConsumed_, p_load->iterationsConsumed_, "iterationsConsumed_", "p_load->iterationsConsumed_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", bestIndividual_, p_load->bestIndividual_, "bestIndividual_", "p_load->bestIndividual_", e , limit));

	// TODO: Compare algorithms; cross check other data has been added

	return evaluateDiscrepancies("Go2", caller, deviations, e);
}

/**************************************************************************************/
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
	serializationMode_ = p_load->serializationMode_;
	ip_ = p_load->ip_;
	port_ = p_load->port_;
	configFilename_ = p_load->configFilename_;
	parMode_ = p_load->parMode_;
	verbose_ = p_load->verbose_;
	maxStalledDataTransfers_ = p_load->maxStalledDataTransfers_;
	maxConnectionAttempts_ = p_load->maxConnectionAttempts_;
	returnRegardless_ = p_load->returnRegardless_;
	nProducerThreads_ = p_load->nProducerThreads_;
	arraySize_ = p_load->arraySize_;
	offset_ = p_load->offset_;
	sorted_ = p_load->sorted_;
	iterationsConsumed_ = p_load->iterationsConsumed_;

	copyGenevaSmartPointer<GParameterSet>(p_load->bestIndividual_, bestIndividual_);

	// Copy the algorithms vector over
	copyAlgorithmsVector(p_load->algorithms_, algorithms_);

	// Cross check other data has been added
}

/**************************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object
 */
GObject *Go2::clone_() const {
	return new Go2(*this);
}

/**************************************************************************************/
/**
 * Triggers execution of the client loop. Note that it is up to you to terminate
 * the program after calling this function.
 */
int Go2::clientRun() {
	// Instantiate the client worker
	boost::shared_ptr<Gem::Courtier::GAsioTCPClientT<GIndividual> > p(new Gem::Courtier::GAsioTCPClientT<GIndividual>(ip_, boost::lexical_cast<std::string>(port_)));

	p->setMaxStalls(maxStalledDataTransfers_); // Set to 0 to allow an infinite number of stalls
	p->setMaxConnectionAttempts(maxConnectionAttempts_); // Set to 0 to allow an infinite number of failed connection attempts
	p->returnResultIfUnsuccessful(returnRegardless_);  // Prevent return of unsuccessful adaption attempts to the server

	// Start the actual processing loop
	p->run();

	return 0;
}

/**************************************************************************************/
/**
 * Checks whether this object is running in client mode
 *
 * @return A boolean which indicates whether the client mode has been set for this object
 */
bool Go2::clientMode() const {
	return clientMode_;
}

/**************************************************************************************/
/**
 * Allows to set the parallelization mode used for the optimization. Note that
 * this setting will only have an effect on algorithms that have not been explicitly
 * added to Go2 and only to thos algorithms that have been added after the parMode_
 * has been set.
 *
 * @param parMode The parallelization mode used for the optimization
 */
void Go2::setParallelizationMode(const parMode& parMode) {
	parMode_ = parMode;
}

/**************************************************************************************/
/**
 * Allows to retrieve the parallelization mode currently used for the optimization
 *
 * @return The parallelization mode currently used for the optimization
 */
parMode Go2::getParallelizationMode() const {
	return parMode_;
}

/**************************************************************************************/
/**
 * Allows to randomly initialize parameter members. Note that for this wrapper object
 * this function doesn't make any sense. It is made available to satisfy a requirement
 * of GIndividual.
 */
void Go2::randomInit()
{ /* nothing */ }

/**************************************************************************************/
/**
 * Fitness calculation for an optimization algorithm means optimization. The fitness is
 * then determined by the best individual which, after the end of the optimization cycle.
 *
 * @param id The id of the target function (ignored here)
 * @return The fitness of the best individual in the population
 */
double Go2::fitnessCalculation() {
	bool dirty = false;

	boost::shared_ptr<GParameterSet> p = this->GOptimizableI::optimize<GParameterSet>(offset_ + iterationsConsumed_);

	double val = p->getCachedFitness(dirty);
	// is this the current fitness ? We should at this stage never
	// run across an unevaluated individual.
	if(dirty) {
		raiseException(
				"In Go2::fitnessCalculation():" << std::endl
				<< "Came across dirty individual"
		);
	}

	return val;
}

/**************************************************************************************/
/**
 * Allows to add an optimization algorithm to the chain
 *
 * @param alg A base pointer to another optimization algorithm
 */
void Go2::addAlgorithm(boost::shared_ptr<GOptimizableI> alg) {
	// Check that the pointer is not empty
	if(!alg) {
		raiseException(
			"In Go2::addAlgorithm(): Error!" << std::endl
			<< "Tried to register an empty pointer" << std::endl
		);
	}

	algorithms_.push_back(alg);
}

/**************************************************************************************/
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
Go2& Go2::operator&(boost::shared_ptr<GOptimizableI> alg) {
	this->addAlgorithm(alg);
	return *this;
}

/**************************************************************************************/
/**
 * Allows to add an algorithm with unspecified parallelization mode to the chain. The
 * mode is then set internally accordoing to the value of the parMode_ variable. This
 * allows to dynamically change the parallelization mode e.g. with a command-line
 * parameter.
 *
 * @param per The desired optimization algorithm
 */
void Go2::addAlgorithm(personality_oa pers) {
	switch(pers) {
	case PERSONALITY_NONE:
		{
			raiseException(
				"In Go2::addAlgorithm(personality_oa): Error!" << std::endl
				<< "Got PERSONALITY_NONE" << std::endl
			);
		}
		break;

	case PERSONALITY_EA:
		{
			GEvolutionaryAlgorithmFactory geaf("config/GEvolutionaryAlgorithm.json", parMode_);
			this->addAlgorithm(geaf());
		}
		break;

	case PERSONALITY_GD:
		{
			GGradientDescentFactory ggdf("config/GGradientDescent.json", parMode_);
			this->addAlgorithm(ggdf());
		}
		break;

	case PERSONALITY_SWARM:
		{
			GSwarmAlgorithmFactory gsaf("config/GSwarmAlgorithm.json", parMode_);
			this->addAlgorithm(gsaf());
		}
		break;
	}
}

/**************************************************************************************/
/**
 * Facilitates adding of algorithms with unspecified parallelization mode
 *
 * @param per The desired optimization algorithm
 */
Go2& Go2::operator&(personality_oa pers) {
	this->addAlgorithm(pers);
	return *this;
}

/**************************************************************************************/
/**
 * Perform the actual optimization cycle
 *
 * @param offset An offset at which the first algorithm should start
 */
void Go2::optimize(const boost::uint32_t& offset) {
	// Add algorithms that have been specified on the command line
	std::vector<personality_oa>::iterator pers_it;
	for(pers_it=optimization_algorithms_.begin(); pers_it!=optimization_algorithms_.end(); ++pers_it) {
		this->addAlgorithm(*pers_it);
	}


	// Check that algorithms have indeed been registered
	if(algorithms_.empty()) {
		std::cerr << "No algorithms have been registered." << std::endl
				  << "Adding evolutionary algorithm with" << std::endl
				  << "default settings" << std::endl;
		this->addAlgorithm(PERSONALITY_EA);
	}

	// Check that individuals have been registered
	if(this->empty()) {
		raiseException(
			"In Go2::optimize(): Error!" << std::endl
			<< "No individuals have been registered." << std::endl
			<< "No way to continue." << std::endl
		)
	}

	// Retrieve the minimization/maximization mode of the first individual
	bool maxmode = this->front()->getMaxMode();

	// Check that all individuals have the same mode
	GMutableSetT<GParameterSet>::iterator ind_it;
	for(ind_it=this->begin()+1; ind_it!=this->end(); ++ind_it) {
		if((*ind_it)->getMaxMode() != maxmode) {
			raiseException(
				"In Go2::optimize(): Error!" << std::endl
				<< "Found inconsistent min/max modes" << std::endl
			);
		}
	}

	// Loop over all algorithms
	iterationsConsumed_ = offset_;
	sorted_ = false;
	std::vector<boost::shared_ptr<GOptimizableI> >::iterator alg_it;
	for(alg_it=algorithms_.begin(); alg_it!=algorithms_.end(); ++alg_it) {
		boost::shared_ptr<GOptimizableI> p_base = (*alg_it);

		// If this is a broker-based population, check whether we need to enrol a consumer
		if(p_base->usesBroker() && !GBROKER(Gem::Geneva::GIndividual)->hasConumers()) {
			// Create a network consumer and enrol it with the broker
			boost::shared_ptr<Gem::Courtier::GAsioTCPConsumerT<GIndividual> > gatc(new Gem::Courtier::GAsioTCPConsumerT<GIndividual>(
					port_
					, 0 // Try to automatically determine the number of listener threads
					, serializationMode_
					)
			);
			GBROKER(Gem::Geneva::GIndividual)->enrol(gatc);
		}

		switch(p_base->getOptimizationAlgorithm()) {
		case PERSONALITY_EA:
		{
			// Add the individuals to the EA. Note that it needs to be converted for this purpose
			boost::shared_ptr<GBaseEA> p_derived = boost::dynamic_pointer_cast<GBaseEA>(p_base);
			for(ind_it=this->begin(); ind_it!=this->end(); ++ind_it) {
				p_derived->push_back(*ind_it);
			}

			// Remove our local copies
			this->clear();
			assert(this->empty());

			// Do the actual optimization
			bestIndividual_ = p_base->optimize<GParameterSet>(iterationsConsumed_);

			// Make sure we start with the correct iteration in the next algorithm
			iterationsConsumed_ = p_base->getIteration();

			// Unload the individuals from the last algorithm and store them again in this object
			std::vector<boost::shared_ptr<GParameterSet> > bestIndividuals = p_derived->GOptimizableI::getBestIndividuals<GParameterSet>();
			std::vector<boost::shared_ptr<GParameterSet> >::iterator best_it;
			for(best_it=bestIndividuals.begin(); best_it != bestIndividuals.end(); ++best_it) {
				this->push_back(*best_it);
			}
			bestIndividuals.clear();
		}
			break;

		case PERSONALITY_SWARM:
		{
			// Add the individuals to the Swarm. Note that it needs to be converted for this purpose
			boost::shared_ptr<GBaseSwarm> p_derived = boost::dynamic_pointer_cast<GBaseSwarm>(p_base);
			for(ind_it=this->begin(); ind_it!=this->end(); ++ind_it) {
				p_derived->push_back(*ind_it);
			}

			// Remove our local copies
			this->clear();
			assert(this->empty());

			// Do the actual optimization
			bestIndividual_ = p_base->optimize<GParameterSet>(iterationsConsumed_);

			// Make sure we start with the correct iteration in the next algorithm
			iterationsConsumed_ = p_base->getIteration();

			// Unload the individuals from the last algorithm and store them again in this object
			std::vector<boost::shared_ptr<GParameterSet> > bestIndividuals = p_derived->GOptimizableI::getBestIndividuals<GParameterSet>();
			std::vector<boost::shared_ptr<GParameterSet> >::iterator best_it;
			for(best_it=bestIndividuals.begin(); best_it != bestIndividuals.end(); ++best_it) {
				this->push_back(*best_it);
			}
			bestIndividuals.clear();
		}
			break;

		case PERSONALITY_GD:
		{
			// Add the individuals to the GD. Note that it needs to be converted for this purpose
			boost::shared_ptr<GBaseGD> p_derived = boost::dynamic_pointer_cast<GBaseGD>(p_base);
			for(ind_it=this->begin(); ind_it!=this->end(); ++ind_it) {
				p_derived->push_back(*ind_it);
			}

			// Remove our local copies
			this->clear();
			assert(this->empty());

			// Do the actual optimization
			bestIndividual_ = p_base->optimize<GParameterSet>(iterationsConsumed_);

			// Make sure we start with the correct iteration in the next algorithm
			iterationsConsumed_ = p_base->getIteration();

			// Unload the individuals from the last algorithm and store them again in this object
			std::vector<boost::shared_ptr<GParameterSet> > bestIndividuals = p_derived->GOptimizableI::getBestIndividuals<GParameterSet>();
			std::vector<boost::shared_ptr<GParameterSet> >::iterator best_it;
			for(best_it=bestIndividuals.begin(); best_it != bestIndividuals.end(); ++best_it) {
				this->push_back(*best_it);
			}
			bestIndividuals.clear();
		}
			break;

		default:
		{
			raiseException(
				"In Go2::optimize(): Error!" << std::endl
				<< "Came across invalid algorithm" << std::endl
			);
		}
			break;

		}
	}

	// Sort the individuals according to their fitness so we have it easier later on
	// to extract the best individuals found.
	if(maxmode){
		std::sort(this->begin(), this->end(),
				boost::bind(&GParameterSet::fitness, _1, 0) > boost::bind(&GParameterSet::fitness, _2, 0));
	}
	else{ // Minimization
		std::sort(this->begin(), this->end(),
				boost::bind(&GParameterSet::fitness, _1, 0) < boost::bind(&GParameterSet::fitness, _2, 0));
	}

	sorted_ = true;
}

/**************************************************************************************/
/**
 * Resets the object to its start position
 */
void Go2::reset() {
	/* this->clear(); // Get rid of the local individuals
	bestIndividual_.reset(); // Get rid of the stored best individual
	algorithms_.clear(); // Get rid of the optimization algorithms */

	this->load(tmpl_ptr);
}

/**************************************************************************************/
/**
 * Retrieves the best individual found. This function returns a base pointer.
 * Conversion is done through a function stored in GOptimizableI.
 *
 * @return The best individual found
 */
boost::shared_ptr<GIndividual> Go2::getBestIndividual() {
	Go2::iterator it;

	// Do some error checking
	if(this->empty()) {
		raiseException(
			"In Go2::getBestIndividual(): Error!" << std::endl
			<< "No individuals found"
		);

		for(it=this->begin(); it!=this->end(); ++it) {
			if((*it)->isDirty()) {
				raiseException(
					"In Go2::getBestIndividual(): Error!" << std::endl
					<< "Found individual in position " << std::distance(it,this->begin()) << " whose dirty flag is set" << std::endl
				);
			}
		}

		if(!sorted_) {
			raiseException(
				"In Go2::getBestIndividual(): Error!" << std::endl
				<< "Tried to retrieve best individual" << std::endl
				<< "from an unsorted population."
			);
		}
	}

	// Simply return the best individual. This will result in an implicit downcast
	return this->front();
}

/**************************************************************************************/
/**
 * Retrieves a list of the best individuals found. This function returns  base pointers.
 * Conversion is done through a function stored in GOptimizableI.
 *
 * @return The best individual found
 */
std::vector<boost::shared_ptr<GIndividual> > Go2::getBestIndividuals() {
	Go2::iterator it;

	// Do some error checking
	if(this->empty()) {
		raiseException(
			"In Go2::getBestIndividuals(): Error!" << std::endl
			<< "No individuals found"
		);

		for(it=this->begin(); it!=this->end(); ++it) {
			if((*it)->isDirty()) {
				raiseException(
					"In Go2::getBestIndividuals(): Error!" << std::endl
					<< "Found individual in position " << std::distance(it,this->begin()) << " whose dirty flag is set" << std::endl
				);
			}
		}
	}

	std::vector<boost::shared_ptr<GIndividual> > bestIndividuals;
	for(it=this->begin(); it!=this->end(); ++it) {
		// This will result in an implicit downcast
		bestIndividuals.push_back(*it);
	}

	return bestIndividuals;
}

/**************************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void Go2::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
	, const bool& showOrigin
) {
	using namespace Gem::Common;

	// Call our parent class'es function
	GMutableSetT<GParameterSet>::addConfigurationOptions(gpb, showOrigin);

	// Add local data
	gpb.registerFileParameter<boost::uint32_t> (
		"maxStalledDataTransfers"
		, maxStalledDataTransfers_
		, GO2_DEF_MAXSTALLED
		, VAR_IS_ESSENTIAL
		, "Specifies how often a client may try to unsuccessfully retrieve data from the server (0 means endless)"
	);

	gpb.registerFileParameter<boost::uint32_t> (
		"maxConnectionAttempts"
		, maxConnectionAttempts_
		, GO2_DEF_MAXCONNATT
		, VAR_IS_ESSENTIAL
		, "Specifies how often a client may try to connect unsuccessfully to the server (0 means endless)"
	);

	gpb.registerFileParameter<bool> (
		"returnRegardless"
		, returnRegardless_
		, GO2_DEF_RETURNREGARDLESS
		, VAR_IS_ESSENTIAL
		, "Specifies whether unsuccessful processing attempts should be returned to the server"
	);
}

/**************************************************************************************/
/**
 * Allows to mark this object as belonging to a client as opposed to a server
 *
 * @param serverMode Allows to mark this object as belonging to a client as opposed to a server
 */
void Go2::setClientMode(bool clientMode) {
	clientMode_ = clientMode;
}

/**************************************************************************************/
/**
 * Allows to check whether this object is working in server or client mode
 *
 * @return A boolean indicating whether this object is working in server or client mode
 */
bool Go2::getClientMode() const {
	return clientMode_;
}

/**************************************************************************************/
/**
 * Allows to set the serialization mode used for network transfers
 *
 * @param serializationMode The serialization mode used for network transfers
 */
void Go2::setSerializationMode(const Gem::Common::serializationMode& serializationMode) {
	serializationMode_ = serializationMode;
}

/**************************************************************************************/
/**
 * Allows to retrieve the serialization mode currently used for network transfers
 */
Gem::Common::serializationMode Go2::getSerializationMode() const {
	return serializationMode_;
}

/**************************************************************************************/
/**
 * Allows to set the ip of the server
 *
 * @param ip The ip of the server
 */
void Go2::setServerIp(const std::string& ip) {
	ip_ = ip;
}

/**************************************************************************************/
/**
 * Allows to retrieve the ip of the server
 *
 * @return The current ip used to access the server
 */
std::string Go2::getServerIp() const {
	return ip_;
}

/**************************************************************************************/
/**
 * Allows to set the port used to access the server
 *
 * @param port The port used to access the server
 */
void Go2::setServerPort(const unsigned short& port) {
	port_ = port;
}

/**************************************************************************************/
/**
 * Allows to retrieve the port currently used to access the server
 *
 * @return The number of the port currently used to access the server
 */
unsigned short Go2::getServerPort() const {
	return port_;
}

/**************************************************************************************/
/**
 * Allows to specify whether further information should be emitted after parsing the
 * command line and configuration file.
 *
 * @param verbose Specifies whether information about parsed variables should be emitted in a more verbose format
 */
void Go2::setVerbosity(const bool& verbose) {
	verbose_ = verbose;
}

/**************************************************************************************/
/**
 * Allows to check whether further information should be emitted after parsing the
 * command line and configuration file.
 */
bool Go2::getVerbosity() const {
	return verbose_;
}

/**************************************************************************************/
/**
 * Allows to specify the number of failed data transfers before a client terminates
 * its work. Set this to 0 in order to loop indefinitely.
 *
 * @param maxStalledDataTransfers The number of failed data transfers before a client terminates its work
 */
void Go2::setMaxStalledDataTransfers(const boost::uint32_t& maxStalledDataTransfers) {
	maxStalledDataTransfers_ = maxStalledDataTransfers;
}

/**************************************************************************************/
/**
 * Allows to retrieve the number of failed data transfers before a client terminates
 * its work. Set this to 0 in order to loop indefinitely.
 *
 * @return The number of failed data transfers before a client terminates its work
 */
boost::uint32_t Go2::getMaxStalledDataTransfers() const {
	return maxStalledDataTransfers_;
}

/**************************************************************************************/
/**
 * Allows to specify how often a client may try to connect the server without response
 * before terminating itself.
 *
 * @param maxConnectionAttempts Specifies the number of failed connection attempts before the client terminates itself
 */
void Go2::setMaxConnectionAttempts(const boost::uint32_t& maxConnectionAttempts) {
	maxConnectionAttempts_ = maxConnectionAttempts;
}

/**************************************************************************************/
/**
 * Allows to retrieve the amount of times a client may try to connect the server without response
 * before terminating itself.
 *
 * @return The amount of times a client may try to connect the server without response before terminating itself.
 */
boost::uint32_t Go2::getMaxConnectionAttempts() const {
	return maxConnectionAttempts_;
}

/**************************************************************************************/
/**
 * Allows to specify whether a client should return results even though here was no
 * improvement.
 *
 * @param returnRegardless Specifies whether a client should return results even though here was no improvement
 */
void Go2::setReturnRegardless(const bool& returnRegardless) {
	returnRegardless_ = returnRegardless;
}

/**************************************************************************************/
/**
 * Allows to check whether a client should return results even though here was no
 * improvement.
 *
 * @return A boolean indicating whether a client should return results even though here was no improvement
 */
bool Go2::getReturnRegardless() const {
	return returnRegardless_;
}

/**************************************************************************************/
/**
 * Allows to set the number of threads that will simultaneously produce random numbers.
 *
 * @param nProducerThreads The number of threads that will simultaneously produce random numbers
 */
void Go2::setNProducerThreads(const boost::uint16_t& nProducerThreads) {
	nProducerThreads_ = nProducerThreads;
}

/**************************************************************************************/
/**
 * Allows to retrieve the number of threads that will simultaneously produce random numbers.
 *
 * @return The number of threads that will simultaneously produce random numbers
 */
boost::uint16_t Go2::getNProducerThreads() const {
	return nProducerThreads_;
}

/**************************************************************************************/
/**
 * Allows to set the size of the array of random numbers transferred to proxies upon request.
 *
 * @param arraySize The size of the array of random numbers transferred to proxies upon request
 */
void Go2::setArraySize(const std::size_t& arraySize) {
	arraySize_ = arraySize;
}

/**************************************************************************************/
/**
 * Allows to retrieve the size of the array of random numbers transferred to proxies upon request.
 *
 * @return The size of the array of random numbers transferred to proxies upon request
 */
std::size_t Go2::getArraySize() const {
	return arraySize_;
}

/**************************************************************************************/
/**
 * Allows to specify the offset with which the iteration counter should start. This is
 * important when using more than one optimization algorithm with different Go2 objects.
 *
 * @param offset The offset with which the iteration counter should start
 */
void Go2::setOffset(const boost::uint32_t& offset) {
	offset_ = offset;
}

/**************************************************************************************/
/**
 * Retrieval of the current iteration
 */
uint32_t Go2::getIteration() const {
	return iterationsConsumed_;
}

/**************************************************************************************/
/**
 * Returns the name of this optimization algorithm
 *
 * @return The name assigned to this optimization algorithm
 */
std::string Go2::getAlgorithmName() const {
	return std::string("Algorithm Combiner");
}

/**************************************************************************************/
/**
 * Allows to retrieve the current offset with which the iteration counter will start
 *
 * @return The current offset with which the iteration counter will start
 */
boost::uint32_t Go2::getIterationOffset() const {
	return offset_;
}

/**************************************************************************************/
/**
 *
 * @param argc The number of command line arguments
 * @param argv An array with the arguments
 */
void Go2::parseCommandLine(int argc, char **argv) {
    namespace po = boost::program_options;

    std::string optimization_algorithms;

	try {
		std::string configFilename = std::string("");
		std::string usageString = std::string("Usage: ") + argv[0] + " [options]";
		po::options_description desc(usageString.c_str());
		desc.add_options()
				("help,h", "emit help message")
				("configFilename,f", po::value<std::string>(&configFilename)->default_value(GO2_DEF_DEFAULTCONFIGFILE),
				"The name of the file holding configuration information for optimization algorithms")
				("parallelizationMode,p", po::value<parMode>(&parMode_)->default_value(GO2_DEF_DEFAULPARALLELIZATIONMODE),
				"The desired parallelization mode (this will only affect algorithms with generic parallelization mode")
				("optimizationAlgorithms,a", po::value<std::string>(&optimization_algorithms)->default_value(GO2_DEF_OPTALGS),
				"A comma-separated list of optimization algorithms. E.g. \"1,2,3\". (1) means \"Evolutionary Algorithms\", (2) means \"Gradient Descents\", (3) means \"Swarm Algorithms\"")
				("clientMode,c", "Makes this program behave as a networked client")
				("ip", po::value<std::string>(&ip_)->default_value(GO2_DEF_IP),
				"The ip of the server")
				("port", po::value<unsigned short>(&port_)->default_value(GO2_DEF_PORT),
				"The port of the server")
				("serializationMode,m", po::value<Gem::Common::serializationMode>(&serializationMode_)->default_value(GO2_DEF_SERIALIZATIONMODE),
				"Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)")
				("verbose,v", "Instructs the parsers to output information about configuration parameters")
		;

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		// Emit a help message, if necessary
		if (vm.count("help")) {
			std::cout << desc << std::endl;
			exit(0);
		}

		// Read in the configuration file. A file with default values
		// will be created for you if it does not yet exist. Note that
		// the target directory needs to exist, though.
		if(vm.count("configFilename")) {
			configFilename_ = configFilename;
			readConfigFile(configFilename_);
		}

		// Parse the list of optimization algorithms
		if(vm.count("optimizationAlgorithms")) {
			typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
			boost::char_separator<char> comma_sep(",");
			tokenizer oaTokenizer(optimization_algorithms, comma_sep);
			for(tokenizer::iterator oa=oaTokenizer.begin(); oa!=oaTokenizer.end(); ++oa) {
				std::string alg = *oa;
				boost::trim(alg);
				personality_oa num_alg = boost::lexical_cast<personality_oa>(alg);

				if(num_alg != PERSONALITY_EA && num_alg != PERSONALITY_GD && num_alg != PERSONALITY_SWARM) {
					raiseException(
						"In Go2::parseCommandLine(): Error!" << std::endl
						<< "Received invalid personality " << num_alg << std::endl
					);
				}

				if(!alg.empty()) {
					optimization_algorithms_.push_back(num_alg);
				}
			}
		}

		if (vm.count("clientMode")) clientMode_ = true;

		if(vm.count("verbose")) {
			verbose_ = true;
			std::cout << std::endl
					<< "Running with the following command line options:" << std::endl
					<< "configFilename = " << configFilename_ << std::endl
					<< "clientMode = " << clientMode_ << std::endl
					<< "ip = " << ip_ << std::endl
					<< "port = " << port_ << std::endl
					<< "serializationMode = " << serializationMode_ << std::endl;
		} else {
			verbose_ = false;
		}
	}
	catch(const po::error& e) {
		std::cerr << "Error parsing the command line:" << std::endl
				  << e.what() << std::endl;
		exit(1);
	}
	catch(...) {
		std::cerr << "Unknown error while parsing the command line" << std::endl;
		exit(1);
	}
}

/**************************************************************************************/
/**
 * Copying of the algorithms_ vector. This is a modified version of the
 * copyGenevaSmartPointerVector function, adapted to the needs of a GOptimizableI
 * derivative. Note that this must be considered a hack.
 */
void Go2::copyAlgorithmsVector(
	const std::vector<boost::shared_ptr<GOptimizableI> >& from
	, std::vector<boost::shared_ptr<GOptimizableI> >& to
) {
	std::vector<boost::shared_ptr<GOptimizableI> >::const_iterator it_from;
	std::vector<boost::shared_ptr<GOptimizableI> >::iterator it_to;

	std::size_t size_from = from.size();
	std::size_t size_to = to.size();

	if(size_from==size_to) { // The most likely case
		for(it_from=from.begin(), it_to=to.begin(); it_from!=from.end(), it_to!=to.end(); ++it_from, ++it_to) {
			const boost::shared_ptr<GObject> go_from = boost::dynamic_pointer_cast<GObject>(*it_from);
			boost::shared_ptr<GObject>       go_to   = boost::dynamic_pointer_cast<GObject>(*it_to);

			copyGenevaSmartPointer(go_from, go_to);
		}
	}
	else if(size_from > size_to) {
		// First copy the data of the first size_to items
		for(it_from=from.begin(), it_to=to.begin(); it_to!=to.end(); ++it_from, ++it_to) {
			const boost::shared_ptr<GObject> go_from = boost::dynamic_pointer_cast<GObject>(*it_from);
			boost::shared_ptr<GObject>       go_to   = boost::dynamic_pointer_cast<GObject>(*it_to);

			copyGenevaSmartPointer(go_from, go_to);
		}

		// Then attach copies of the remaining items
		for(it_from=from.begin()+size_to; it_from!=from.end(); ++it_from) {
			to.push_back(
				boost::dynamic_pointer_cast<GOptimizableI>((boost::dynamic_pointer_cast<GObject>(*it_from))->GObject::clone())
			);
		}
	}
	else if(size_from < size_to) {
		// First copy the initial size_foreight items over
		for(it_from=from.begin(), it_to=to.begin(); it_from!=from.end(); ++it_from, ++it_to) {
			const boost::shared_ptr<GObject> go_from = boost::dynamic_pointer_cast<GObject>(*it_from);
			boost::shared_ptr<GObject>       go_to   = boost::dynamic_pointer_cast<GObject>(*it_to);

			copyGenevaSmartPointer(go_from, go_to);
		}

		// Then resize the local vector. Surplus items will vanish
		to.resize(size_from);
	}
}

/**************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
