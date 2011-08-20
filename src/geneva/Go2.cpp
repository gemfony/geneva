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
 * The default constructor
 */
Go2::Go2()
	: GMutableSetT<GParameterSet>()
	, serverMode_(GO2_DEF_SERVERMODE)
	, serializationMode_(GO2_DEF_SERIALIZATIONMODE)
	, ip_(GO2_DEF_IP)
	, port_(GO2_DEF_PORT)
	, configFilename_(GO2_DEF_DEFAULTCONFIGFILE)
	, verbose_(GO2_DEF_DEFAULTVERBOSE)
	, copyBestOnly_(GO2_DEF_COPYBESTONLY)
	, maxStalledDataTransfers_(GO2_DEF_MAXSTALLED)
	, maxConnectionAttempts_(GO2_DEF_MAXCONNATT)
	, returnRegardless_(GO2_DEF_RETURNREGARDLESS)
	, nProducerThreads_(GO2_DEF_NPRODUCERTHREADS)
	, arraySize_(GO2_DEF_ARRAYSIZE)
	, offset_(GO2_DEF_OFFSET)
	, consumerInitialized_(GO2_DEF_CONSUMERINITIALIZED)
	, iterationsConsumed_(0)
{
	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Set the number of threads. GRANDOMFACTORY is
	// a singleton that will be initialized by this call.
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads_);
	GRANDOMFACTORY->setArraySize(arraySize_);
}


/**************************************************************************************/
/**
 * A constructor that first parses the command line for relevant parameters and then
 * loads data from a configuration file
 *
 * @param argc The number of command line arguments
 * @param argv An array with the arguments
 * @param configFileName The name of a configuration file
 */
Go2::Go2(int argc, char **argv, const std::string& configFilename)
	: GMutableSetT<GParameterSet>()
	, serverMode_(GO2_DEF_SERVERMODE)
	, serializationMode_(GO2_DEF_SERIALIZATIONMODE)
	, ip_(GO2_DEF_IP)
	, port_(GO2_DEF_PORT)
	, configFilename_(GO2_DEF_DEFAULTCONFIGFILE)
	, verbose_(GO2_DEF_DEFAULTVERBOSE)
	, copyBestOnly_(GO2_DEF_COPYBESTONLY)
	, maxStalledDataTransfers_(GO2_DEF_MAXSTALLED)
	, maxConnectionAttempts_(GO2_DEF_MAXCONNATT)
	, returnRegardless_(GO2_DEF_RETURNREGARDLESS)
	, nProducerThreads_(GO2_DEF_NPRODUCERTHREADS)
	, arraySize_(GO2_DEF_ARRAYSIZE)
	, offset_(GO2_DEF_OFFSET)
	, consumerInitialized_(GO2_DEF_CONSUMERINITIALIZED)
	, iterationsConsumed_(0)
{
	//--------------------------------------------
	// Load initial configuration options from the command line
	parseCommandLine(argc, argv);

	// Update the name of the configuration file, if necessary
	if(configFilename != GO2_DEF_DEFAULTCONFIGFILE) {
		configFilename_ = configFilename;
	}

	//--------------------------------------------
	// Load further configuration options from file
	parseConfigurationFile(configFilename_);

	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Set the number of threads. GRANDOMFACTORY is
	// a singleton that will be initialized by this call.
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads_);
	GRANDOMFACTORY->setArraySize(arraySize_);
}

/**************************************************************************************/
/**
 * A constructor that is given the usual command line parameters, then loads the
 * rest of the data from a config file.
 *
 * @param pers Specifies the optimization algorithm to be used by this object
 * @param pM Indicates the parallelization mode (serial, multi-threaded or networked)
 * @param serverMode Indicates whether this object should operate in server (or multithreaded + serial) or client mode
 * @param sM Specifies whether serialization should happen in XML, Text oder Binary mode
 * @param ip Specifies the ip under which the server can be reached
 * @param port Specifies the port under which the server can be reached
 * @param configFilename Determines the name of the configuration file from which additional options will be loaded
 * @param verbose Specifies whether additional information about parsed parameters should be emitted
 */
Go2::Go2(
	const bool& serverMode
	, const Gem::Common::serializationMode& sM
	, const std::string& ip
	, const unsigned short& port
	, const std::string& configFilename
	, const bool& verbose
)
	: GMutableSetT<GParameterSet>()
	, serverMode_(serverMode)
	, serializationMode_(sM)
	, ip_(ip)
	, port_(port)
	, configFilename_(configFilename)
	, verbose_(verbose)
	, copyBestOnly_(GO2_DEF_COPYBESTONLY)
	, maxStalledDataTransfers_(GO2_DEF_MAXSTALLED)
	, maxConnectionAttempts_(GO2_DEF_MAXCONNATT)
	, returnRegardless_(GO2_DEF_RETURNREGARDLESS)
	, nProducerThreads_(GO2_DEF_NPRODUCERTHREADS)
	, arraySize_(GO2_DEF_ARRAYSIZE)
	, offset_(GO2_DEF_OFFSET)
	, iterationsConsumed_(0)
	, consumerInitialized_(GO2_DEF_CONSUMERINITIALIZED)
{
	//--------------------------------------------
	// Load further configuration options from file
	parseConfigurationFile(configFilename_);

	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Set the number of threads. GRANDOMFACTORY is
	// a singleton that will be initialized by this call.
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads_);
	GRANDOMFACTORY->setArraySize(arraySize_);
}

/**************************************************************************************/
/**
 * The copy constructor
 */
Go2::Go2(const Go2& cp)
	: GMutableSetT<GParameterSet>(cp)
	, serverMode_(cp.serverMode_)
	, serializationMode_(cp.serializationMode_)
	, ip_(cp.ip_)
	, port_(cp.port_)
	, configFilename_(cp.configFilename_)
	, verbose_(cp.verbose_)
	, copyBestOnly_(cp.copyBestOnly_)
	, maxStalledDataTransfers_(cp.maxStalledDataTransfers_)
	, maxConnectionAttempts_(cp.maxConnectionAttempts_)
	, returnRegardless_(cp.returnRegardless_)
	, nProducerThreads_(cp.nProducerThreads_)
	, arraySize_(cp.arraySize_)
	, offset_(cp.offset_)
	, iterationsConsumed_(0)
	, consumerInitialized_(cp.consumerInitialized_)
{
	// Copy the best individual over (if any)
	copyGenevaSmartPointer<GParameterSet>(cp.bestIndividual_, bestIndividual_);

	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Set the number of threads. GRANDOMFACTORY is
	// a singleton that will be initialized by this call.
	GRANDOMFACTORY->setNProducerThreads(nProducerThreads_);
	GRANDOMFACTORY->setArraySize(arraySize_);
}

/**************************************************************************************/
/**
 * The destructor
 */
Go2::~Go2()
{ /* nothing */ }

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
	const Go2 *p_load = GObject::conversion_cast<Go2>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GMutableSetT<GParameterSet>::checkRelationshipWith(cp, e, limit, "Go2", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "Go2", serverMode_, p_load->serverMode_, "serverMode_", "p_load->serverMode_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", serializationMode_, p_load->serializationMode_, "serializationMode_", "p_load->serializationMode_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", ip_, p_load->ip_, "ip_", "p_load->ip_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", port_, p_load->port_, "port_", "p_load->port_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", configFilename_, p_load->configFilename_, "configFilename_", "p_load->configFilename_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", verbose_, p_load->verbose_, "verbose_", "p_load->verbose_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", copyBestOnly_, p_load->copyBestOnly_, "copyBestOnly_", "p_load->copyBestOnly_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", maxStalledDataTransfers_, p_load->maxStalledDataTransfers_, "maxStalledDataTransfers_", "p_load->maxStalledDataTransfers_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", maxConnectionAttempts_, p_load->maxConnectionAttempts_, "maxConnectionAttempts_", "p_load->maxConnectionAttempts_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", returnRegardless_, p_load->returnRegardless_, "returnRegardless_", "p_load->returnRegardless_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", nProducerThreads_, p_load->nProducerThreads_, "nProducerThreads_", "p_load->nProducerThreads_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", arraySize_, p_load->arraySize_, "arraySize_", "p_load->arraySize_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", offset_, p_load->offset_, "offset_", "p_load->offset_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", iterationsConsumed_, p_load->iterationsConsumed_, "iterationsConsumed_", "p_load->iterationsConsumed_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", consumerInitialized_, p_load->consumerInitialized_, "consumerInitialized_", "p_load->consumerInitialized_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", bestIndividual_, p_load->bestIndividual_, "bestIndividual_", "p_load->bestIndividual_", e , limit));

	return evaluateDiscrepancies("Go2", caller, deviations, e);
}

/**************************************************************************************/
/**
 * Loads the data of another Go2 object
 *
 * @param cp A copy of another Go2 object, camouflaged as a GObject
 */
void Go2::load_(const GObject *cp) {
	const Go2 *p_load = conversion_cast<Go2>(cp);

	// First load the parent class'es data ...
	GMutableSetT<GParameterSet>::load_(cp);

	// and then our local data
	serverMode_ = p_load->serverMode_;
	serializationMode_ = p_load->serializationMode_;
	ip_ = p_load->ip_;
	port_ = p_load->port_;
	configFilename_ = p_load->configFilename_;
	verbose_ = p_load->verbose_;
	copyBestOnly_ = p_load->copyBestOnly_;
	maxStalledDataTransfers_ = p_load->maxStalledDataTransfers_;
	maxConnectionAttempts_ = p_load->maxConnectionAttempts_;
	returnRegardless_ = p_load->returnRegardless_;
	nProducerThreads_ = p_load->nProducerThreads_;
	arraySize_ = p_load->arraySize_;
	offset_ = p_load->offset_;
	iterationsConsumed_ = p_load->iterationsConsumed_;
	consumerInitialized_ = p_load->consumerInitialized_;

	copyGenevaSmartPointer<GParameterSet>(p_load->bestIndividual_, bestIndividual_);
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
bool Go2::clientRun() {
	if(serverMode()) {
		return false;
	}
	else {
		// Instantiate the client worker
		boost::shared_ptr<Gem::Courtier::GAsioTCPClientT<GIndividual> > p(new Gem::Courtier::GAsioTCPClientT<GIndividual>(ip_, boost::lexical_cast<std::string>(port_)));

		p->setMaxStalls(maxStalledDataTransfers_); // Set to 0 to allow an infinite number of stalls
		p->setMaxConnectionAttempts(maxConnectionAttempts_); // Set to 0 to allow an infinite number of failed connection attempts
		p->returnResultIfUnsuccessful(returnRegardless_);  // Prevent return of unsuccessful adaption attempts to the server

		// Start the actual processing loop
		p->run();

		return true;
	}
}

/**************************************************************************************/
/**
 * Checks whether server mode has been requested for this object
 *
 * @return A boolean which indicates whether the server mode has been set for this object
 */
bool Go2::serverMode() const {
	return serverMode_;
}

/**************************************************************************************/
/**
 * Checks whether this object is running in client mode
 *
 * @return A boolean which indicates whether the client mode has been set for this object
 */
bool Go2::clientMode() const {
	return !serverMode_;
}

/**************************************************************************************/
/*
 * Specifies whether only the best individuals of a population should be copied.
 *
 * @param copyBestOnly Specifies whether only the best individuals of a population are copied
 */
void Go2::setCopyBestIndividualsOnly(const bool& copyBestOnly) {
	copyBestOnly_ = copyBestOnly;
}

/**************************************************************************************/
/**
 * Checks whether only the best individuals are copied
 *
 * @return A boolean indicating whether only the best individuals of a population are copied
 */
bool Go2::onlyBestIndividualsAreCopied() const {
	return copyBestOnly_;
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

	boost::shared_ptr<GParameterSet> p = this->optimize<GParameterSet>(offset_ + iterationsConsumed_);

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
 * go2 += alg1
 *     += alg2
 *     += alg3;
 * go2.optimize();
 *
 * @param alg A base pointer to another optimization algorithm
 * @return A reference to this object
 */
Go2& Go2::operator+=(boost::shared_ptr<GOptimizableI> alg) {
	this->addAlgorithm(alg);
	return *this;
}

/**************************************************************************************/
/**
 * Perform the actual optimization cycle
 *
 * @param offset An offset at which the first algorithm should start
 */
void Go2::optimize(const boost::uint32_t& offset) {
	// Check that algorithms have indeed been registered
	if(algithms_.empty()) {
		raiseException(
			"In Go2::optimize(): Error!" << std::endl
			<< "No algorithms have been registered." << std::endl
		);
	}

	// Check that individuals have been registered
	if(this->empty()) {
		raiseException(
			"In Go2::optimize(): Error!" << std::endl
			<< "No individuals have been registered." << std::endl
		)
	}

	// Add the individuals to the first algorithm
	boost::shared_ptr<GOptimizableI> p_front_base = algorithms_.front();
	GMutableSetT<GParameterSet>::iterator it;
	switch(algorithms_.front()->getOptimizationAlgorithm()) {
	case EA:
	{
		boost::shared_ptr<GBaseEA> p_front_derived = boost::dynamic_pointer_cast<GBaseEA>(p_front_base);
		for(it=this->begin(); it!=this->end(); ++it) {
			p_front_derived->push_back(*it);
		}
	}
		break;

	case SWARM:
	{
		boost::shared_ptr<GBaseSwarm> p_front_derived = boost::dynamic_pointer_cast<GBaseSwarm>(p_front_base);
		for(it=this->begin(); it!=this->end(); ++it) {
			p_front_derived->push_back(*it);
		}
	}
		break;

	case GD:
	{
		boost::shared_ptr<GBaseGD> p_front_derived = boost::dynamic_pointer_cast<GBaseGD>(p_front_base);
		for(it=this->begin(); it!=this->end(); ++it) {
			p_front_derived->push_back(*it);
		}
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

	// Remove our local copies

	// Loop over all algorithms

	// Unload the individuals from the last algorithm and
	// store them again in this object
}

/**************************************************************************************/
/**
 * Retrieves the best individual found. This function returns a base pointer.
 * Conversion is done through a function stored in GOptimizableI.
 *
 * @return The best individual found
 */
boost::shared_ptr<GIndividual> Go2::getBestIndividual() {

}

/**************************************************************************************/
/**
 * Allows to mark this object as belonging to a server as opposed to a client
 *
 * @param serverMode Allows to mark this object as belonging to a server as opposed to a client
 */
void Go2::setServerMode(const bool& serverMode) {
	serverMode_ = serverMode;
}

/**************************************************************************************/
/**
 * Allows to check whether this object is working in server or client mode
 *
 * @return A boolean indicating whether this object is working in server or client mode
 */
bool Go2::getServerMode() const {
	return serverMode_;
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
 * Allows to set the name of the configuration file from which further options will
 * be read.
 *
 * @param configFilename The name of the file from which further options will be read
 */
void Go2::setConfigFileName(const std::string& configFilename) {
	configFilename_ = configFilename;
}

/**************************************************************************************/
/**
 * Allows to retrieve the name of the configuration file from which further options will
 * be read
 *
 * @return The name of the configuration file from which further options will be read
 */
std::string Go2::getConfigFileName() const {
	return configFilename_;
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
 * Allows to retrieve the current offset with which the iteration counter will start
 *
 * @return The current offset with which the iteration counter will start
 */
boost::uint32_t Go2::getOffset() const {
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

	try {
		std::string usageString = std::string("Usage: ") + argv[0] + " [options]";
		po::options_description desc(usageString.c_str());
		desc.add_options()
				("help,h", "emit help message")
				("configFilename,c", po::value<std::string>(&configFilename_)->default_value(GO2_DEF_DEFAULTCONFIGFILE),
				"The name of the file holding configuration information for optimization algorithms")
				("serverMode,s",
				"Whether to run networked execution in client or server mode. This option only gets evaluated if \"--parallelizationMode=2\" is set")
				("ip", po::value<std::string>(&ip_)->default_value(GO2_DEF_IP),
				"The ip of the server")
				("port", po::value<unsigned short>(&port_)->default_value(GO2_DEF_PORT),
				"The port of the server")
				("serializationMode", po::value<Gem::Common::serializationMode>(&serializationMode_)->default_value(GO2_DEF_SERIALIZATIONMODE),
				"Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)")
				("writeConfigFile,w",
				"Instructs the program to write out a configuration file and then to exit. If the configuration option \"configFileName\" has been specified, its value will be used as the name of the file")
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

		// Output the configuration file, if required
		if(vm.count("writeConfigFile")) {
			Go2::writeConfigurationFile(configFilename_);
			exit(0);
		}

		if (parMode_ == 2  &&  !vm.count("serverMode")) serverMode_ = false;
		else serverMode_ = true;

		if(vm.count("verbose")) {
			verbose_ = true;
			std::cout << std::endl
					<< "Running with the following command line options:" << std::endl
					<< "configFilename = " << configFilename_ << std::endl
					<< "serverMode = " << serverMode_ << std::endl
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

} /* namespace Geneva */
} /* namespace Gem */
