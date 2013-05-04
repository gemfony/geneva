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

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * Set a number of parameters of the random number factory
 *
 * @param nProducerThreads The number of threads simultaneously producing random numbers
 */
void setRNFParameters(
		const boost::uint16_t& nProducerThreads
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
////////////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
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
	, offset_(GO2_DEF_OFFSET)
	, sorted_(false)
	, iterationsConsumed_(0)
{
	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Initialize all necessary variables
	boost::call_once(f_go2, boost::bind(setRNFParameters, nProducerThreads_));
}


/******************************************************************************/
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
	boost::call_once(f_go2, boost::bind(setRNFParameters, nProducerThreads_));
}

/******************************************************************************/
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
	boost::call_once(f_go2, boost::bind(setRNFParameters, nProducerThreads_));
}


/******************************************************************************/
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
	, const execMode& pm
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
	, offset_(GO2_DEF_OFFSET)
	, sorted_(false)
	, iterationsConsumed_(0)
{
	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Initialize all necessary variables
	boost::call_once(f_go2, boost::bind(setRNFParameters, nProducerThreads_));
}

/******************************************************************************/
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
	, offset_(cp.offset_)
	, sorted_(cp.sorted_)
	, iterationsConsumed_(0)
{
	// Copy the best individual over (if any)
	copyGenevaSmartPointer<GParameterSet>(cp.bestIndividual_, bestIndividual_);

	// Copy the algorithms vector over
	copyGenevaSmartPointerVector(cp.algorithms_, algorithms_);

	//--------------------------------------------
	// Random numbers are our most valuable good.
	// Initialize all necessary variables
	boost::call_once(f_go2, boost::bind(setRNFParameters, nProducerThreads_));

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
	bestIndividual_.reset(); // Get rid of the stored best individual
	algorithms_.clear(); // Get rid of the optimization algorithms
	cl_algorithms_.clear(); // Get rid of algorithms registered on the command line
}

/******************************************************************************/
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

/******************************************************************************/
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

/******************************************************************************/
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

/******************************************************************************/
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
	deviations.push_back(checkExpectation(withMessages, "Go2", offset_, p_load->offset_, "offset_", "p_load->offset_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", sorted_, p_load->sorted_, "sorted_", "p_load->sorted_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", iterationsConsumed_, p_load->iterationsConsumed_, "iterationsConsumed_", "p_load->iterationsConsumed_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", bestIndividual_, p_load->bestIndividual_, "bestIndividual_", "p_load->bestIndividual_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go2", default_algorithm_, p_load->default_algorithm_, "default_algorithm_", "p_load->default_algorithm_", e , limit));

	// TODO: Compare algorithms; cross check whether other data has been added

	return evaluateDiscrepancies("Go2", caller, deviations, e);
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
 * will be copied into the Go2 object.
 */
void Go2::registerDefaultAlgorithm(boost::shared_ptr<GOABase> default_algorithm) {
   // Check that the pointer isn't empty
   if(!default_algorithm) {
      glogger
      << "In Go2::registerDefaultAlgorithm(): Error!" << std::endl
      << "Got empty algorithm." << std::endl
      << GEXCEPTION;
   }

   if(!default_algorithm->empty()) { // Have individuals been registered ?
      GOABase::iterator it;
      for(it=default_algorithm->begin(); it!=default_algorithm->end(); ++it) {
         this->push_back(*it);
      }

      default_algorithm->clear();
   }

   // Register the algorithm
   default_algorithm_ = default_algorithm;
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
 * @param mn A small mnemomic for the optimization algorithm
 */
void Go2::registerDefaultAlgorithm(const std::string& mn) {
   // Retrieve the algorithm from the global store
   boost::shared_ptr<GOptimizationAlgorithmFactoryT<GOABase> > p;
   if(!GOAFactoryStore->get(mn, p)) {
      glogger
      << "In Go2::registerDefaultAlgorithm(std::string): Error!" << std::endl
      << "Got invalid algorithm mnemomic " << mn << std::endl
      << GEXCEPTION;
   }

   this->registerDefaultAlgorithm(p->get(parMode_));
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
	offset_ = p_load->offset_;
	sorted_ = p_load->sorted_;
	iterationsConsumed_ = p_load->iterationsConsumed_;

	copyGenevaSmartPointer<GParameterSet>(p_load->bestIndividual_, bestIndividual_);
	copyGenevaSmartPointer<GOABase>(p_load->default_algorithm_, default_algorithm_);

	// Copy the algorithms vector over
	copyGenevaSmartPointerVector(p_load->algorithms_, algorithms_);

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
	// Instantiate the client worker
	boost::shared_ptr<Gem::Courtier::GAsioTCPClientT<Gem::Geneva::GIndividual> >
		p(new Gem::Courtier::GAsioTCPClientT<Gem::Geneva::GIndividual>(ip_, boost::lexical_cast<std::string>(port_)));

	p->setMaxStalls(maxStalledDataTransfers_); // Set to 0 to allow an infinite number of stalls
	p->setMaxConnectionAttempts(maxConnectionAttempts_); // Set to 0 to allow an infinite number of failed connection attempts
	p->returnResultIfUnsuccessful(returnRegardless_);  // Prevent return of unsuccessful adaption attempts to the server

	// Start the actual processing loop
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
void Go2::setParallelizationMode(const execMode& parMode) {
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
 * of GIndividual.
 */
void Go2::randomInit()
{ /* nothing */ }

/******************************************************************************/
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
	   glogger
	   << "In Go2::fitnessCalculation():" << std::endl
      << "Came across dirty individual" << std::endl
      << GEXCEPTION;
	}

	return val;
}

/******************************************************************************/
/**
 * Allows to add an optimization algorithm to the chain. If any individuals have
 * been registered, the algorithm will unload them.
 *
 * @param alg A base pointer to another optimization algorithm
 */
void Go2::addAlgorithm(boost::shared_ptr<GOABase> alg) {
	// Check that the pointer is not empty
	if(!alg) {
	   glogger
	   << "In Go2::addAlgorithm(): Error!" << std::endl
      << "Tried to register an empty pointer" << std::endl
      << GEXCEPTION;
	}

	if(!alg->empty()) { // Have individuals been registered ?
	   GOABase::iterator it;
	   for(it=alg->begin(); it!=alg->end(); ++it) {
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
Go2& Go2::operator&(boost::shared_ptr<GOABase> alg) {
	this->addAlgorithm(alg);
	return *this;
}

/***************************************************************************/
/**
 * Allows to add an optimization algorithm through its mnemomic
 */
void Go2::addAlgorithm(const std::string& mn) {
   // Retrieve the algorithm from the global store
   boost::shared_ptr<GOptimizationAlgorithmFactoryT<GOABase> > p;
   if(!GOAFactoryStore->get(mn, p)) {
      glogger
      << "In Go2::addAlgorithm(std::string): Error!" << std::endl
      << "Got invalid algorithm mnemomic " << mn << std::endl
      << GEXCEPTION;
   }

   this->addAlgorithm(p->get(parMode_)); // The factory might add a monitor to the object
}

/***************************************************************************/
/**
 * Makes it easier to add algorithms through their mnemomics
 */
Go2& Go2::operator&(const std::string& mn) {
   this->addAlgorithm(mn);
   return *this;
}

/***************************************************************************/
/**
 * Allows to register a content creator
 */
void Go2::registerContentCreator (
      boost::shared_ptr<Gem::Common::GFactoryT<GParameterSet> > cc_ptr
) {
   if(!cc_ptr) {
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
void Go2::optimize(const boost::uint32_t& offset) {
   // Algorithms specified manually in main() take precedence
   // before those specified on the command line
   if(!cl_algorithms_.empty()) {
      // Add algorithms that have been specified on the command line
      std::vector<boost::shared_ptr<GOABase> >::iterator pers_it;
      for(pers_it=cl_algorithms_.begin(); pers_it!=cl_algorithms_.end(); ++pers_it) {
         this->addAlgorithm(*pers_it);
      }
      cl_algorithms_.clear();
   }

	// Check that algorithms have indeed been registered. If not, try to add a default algorithm (if available)
	if(algorithms_.empty()) {
	   if(default_algorithm_) { // Fill with our default algorithm
	      algorithms_.push_back(default_algorithm_->clone<GOABase>());
	   } else {
         glogger
         << "No algorithms have been registered." << std::endl
         << "No way to continue." << std::endl
         << GEXCEPTION;
	   }
	}

	// Check that individuals have been registered
	if(this->empty()) {
	   if(contentCreatorPtr_) {
         for(std::size_t ind=0; ind<algorithms_.at(0)->getDefaultPopulationSize(); ind++) {
            boost::shared_ptr<GParameterSet> p_ind = (*contentCreatorPtr_)();
            if(p_ind) {
               this->push_back(p_ind);
            } else { // No valid item received, the factory has run empty
               if(this->empty()) { // Still empty ?
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

	// Retrieve the minimization/maximization mode of the first individual
	bool maxmode = this->front()->getMaxMode();

	// Check that all individuals have the same mode
	GOABase::iterator ind_it;
	for(ind_it=this->begin()+1; ind_it!=this->end(); ++ind_it) {
		if((*ind_it)->getMaxMode() != maxmode) {
		   glogger
		   << "In Go2::optimize(): Error!" << std::endl
         << "Found inconsistent min/max modes" << std::endl
         << GEXCEPTION;
		}
	}

	// Loop over all algorithms
	iterationsConsumed_ = offset_;
	sorted_ = false;
	std::vector<boost::shared_ptr<GOABase> >::iterator alg_it;
	for(alg_it=algorithms_.begin(); alg_it!=algorithms_.end(); ++alg_it) {
		boost::shared_ptr<GOABase> p_base = (*alg_it);

		// If this is a broker-based population, check whether we need to enrol a consumer
		if(p_base->usesBroker() && !GBROKER(Gem::Geneva::GIndividual)->hasConsumers()) {
			// Create a network consumer and enrol it with the broker
			boost::shared_ptr<Gem::Courtier::GAsioTCPConsumerT<Gem::Geneva::GIndividual> > gatc(new Gem::Courtier::GAsioTCPConsumerT<Gem::Geneva::GIndividual>(
					port_
					, 0 // Try to automatically determine the number of listener threads
					, serializationMode_
					)
			);
			GBROKER(Gem::Geneva::GIndividual)->enrol(gatc);
		}

      // Add the individuals to the algorithm.
      for(ind_it=this->begin(); ind_it!=this->end(); ++ind_it) {
         p_base->push_back(*ind_it);
      }

      // Remove our local copies
      this->clear();

      // Do the actual optimization
      bestIndividual_ = p_base->GOptimizableI::optimize<GParameterSet>(iterationsConsumed_);

      // Make sure we start with the correct iteration in the next algorithm
      iterationsConsumed_ = p_base->getIteration();

      // Unload the individuals from the last algorithm and store them again in this object
      std::vector<boost::shared_ptr<GParameterSet> > bestIndividuals = p_base->getBestIndividuals<GParameterSet>();
      std::vector<boost::shared_ptr<GParameterSet> >::iterator best_it;
      for(best_it=bestIndividuals.begin(); best_it != bestIndividuals.end(); ++best_it) {
         this->push_back(*best_it);
      }
      bestIndividuals.clear();
      p_base->clear();
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

/******************************************************************************/
/**
 * Retrieves the best individual found. This function returns a base pointer.
 * Conversion is done through a function stored in GOptimizableI.
 *
 * @return The best individual found
 */
boost::shared_ptr<Gem::Geneva::GIndividual> Go2::getBestIndividual() {
	Go2::iterator it;

	// Do some error checking
	if(this->empty()) {
	   glogger
	   << "In Go2::getBestIndividual(): Error!" << std::endl
      << "No individuals found" << std::endl
      << GEXCEPTION;

		for(it=this->begin(); it!=this->end(); ++it) {
			if((*it)->isDirty()) {
			   glogger
			   << "In Go2::getBestIndividual(): Error!" << std::endl
            << "Found individual in position " << std::distance(this->begin(),it) << " whose dirty flag is set" << std::endl
            << GEXCEPTION;
			}
		}

		if(!sorted_) {
		   glogger
		   << "In Go2::getBestIndividual(): Error!" << std::endl
         << "Tried to retrieve best individual" << std::endl
         << "from an unsorted population." << std::endl
         << GEXCEPTION;
		}
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
std::vector<boost::shared_ptr<Gem::Geneva::GIndividual> > Go2::getBestIndividuals() {
	Go2::iterator it;

	// Do some error checking
	if(this->empty()) {
	   glogger
	   <<"In Go2::getBestIndividuals(): Error!" << std::endl
      << "No individuals found" << std::endl
      << GEXCEPTION;

		for(it=this->begin(); it!=this->end(); ++it) {
			if((*it)->isDirty()) {
			   glogger
			   << "In Go2::getBestIndividuals(): Error!" << std::endl
            << "Found individual in position " << std::distance(this->begin(),it) << " whose dirty flag is set" << std::endl
            << GEXCEPTION;
			}
		}
	}

	std::vector<boost::shared_ptr<Gem::Geneva::GIndividual> > bestIndividuals;
	for(it=this->begin(); it!=this->end(); ++it) {
		// This will result in an implicit downcast
		bestIndividuals.push_back(*it);
	}

	return bestIndividuals;
}

/******************************************************************************/
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
 * Allows to set the serialization mode used for network transfers
 *
 * @param serializationMode The serialization mode used for network transfers
 */
void Go2::setSerializationMode(const Gem::Common::serializationMode& serializationMode) {
	serializationMode_ = serializationMode;
}

/******************************************************************************/
/**
 * Allows to retrieve the serialization mode currently used for network transfers
 */
Gem::Common::serializationMode Go2::getSerializationMode() const {
	return serializationMode_;
}

/******************************************************************************/
/**
 * Allows to set the ip of the server
 *
 * @param ip The ip of the server
 */
void Go2::setServerIp(const std::string& ip) {
	ip_ = ip;
}

/******************************************************************************/
/**
 * Allows to retrieve the ip of the server
 *
 * @return The current ip used to access the server
 */
std::string Go2::getServerIp() const {
	return ip_;
}

/******************************************************************************/
/**
 * Allows to set the port used to access the server
 *
 * @param port The port used to access the server
 */
void Go2::setServerPort(const unsigned short& port) {
	port_ = port;
}

/******************************************************************************/
/**
 * Allows to retrieve the port currently used to access the server
 *
 * @return The number of the port currently used to access the server
 */
unsigned short Go2::getServerPort() const {
	return port_;
}

/******************************************************************************/
/**
 * Allows to specify whether further information should be emitted after parsing the
 * command line and configuration file.
 *
 * @param verbose Specifies whether information about parsed variables should be emitted in a more verbose format
 */
void Go2::setVerbosity(const bool& verbose) {
	verbose_ = verbose;
}

/******************************************************************************/
/**
 * Allows to check whether further information should be emitted after parsing the
 * command line and configuration file.
 */
bool Go2::getVerbosity() const {
	return verbose_;
}

/******************************************************************************/
/**
 * Allows to specify the number of failed data transfers before a client terminates
 * its work. Set this to 0 in order to loop indefinitely.
 *
 * @param maxStalledDataTransfers The number of failed data transfers before a client terminates its work
 */
void Go2::setMaxStalledDataTransfers(const boost::uint32_t& maxStalledDataTransfers) {
	maxStalledDataTransfers_ = maxStalledDataTransfers;
}

/******************************************************************************/
/**
 * Allows to retrieve the number of failed data transfers before a client terminates
 * its work. Set this to 0 in order to loop indefinitely.
 *
 * @return The number of failed data transfers before a client terminates its work
 */
boost::uint32_t Go2::getMaxStalledDataTransfers() const {
	return maxStalledDataTransfers_;
}

/******************************************************************************/
/**
 * Allows to specify how often a client may try to connect the server without response
 * before terminating itself.
 *
 * @param maxConnectionAttempts Specifies the number of failed connection attempts before the client terminates itself
 */
void Go2::setMaxConnectionAttempts(const boost::uint32_t& maxConnectionAttempts) {
	maxConnectionAttempts_ = maxConnectionAttempts;
}

/******************************************************************************/
/**
 * Allows to retrieve the amount of times a client may try to connect the server without response
 * before terminating itself.
 *
 * @return The amount of times a client may try to connect the server without response before terminating itself.
 */
boost::uint32_t Go2::getMaxConnectionAttempts() const {
	return maxConnectionAttempts_;
}

/******************************************************************************/
/**
 * Allows to specify whether a client should return results even though here was no
 * improvement.
 *
 * @param returnRegardless Specifies whether a client should return results even though here was no improvement
 */
void Go2::setReturnRegardless(const bool& returnRegardless) {
	returnRegardless_ = returnRegardless;
}

/******************************************************************************/
/**
 * Allows to check whether a client should return results even though here was no
 * improvement.
 *
 * @return A boolean indicating whether a client should return results even though here was no improvement
 */
bool Go2::getReturnRegardless() const {
	return returnRegardless_;
}

/******************************************************************************/
/**
 * Allows to set the number of threads that will simultaneously produce random numbers.
 *
 * @param nProducerThreads The number of threads that will simultaneously produce random numbers
 */
void Go2::setNProducerThreads(const boost::uint16_t& nProducerThreads) {
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
void Go2::setOffset(const boost::uint32_t& offset) {
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
 */
void Go2::parseCommandLine(int argc, char **argv) {
   namespace po = boost::program_options;

   std::string optimization_algorithms;
   std::ostringstream oa_help;

   oa_help
   << "A comma-separated list of optimization algorithms, e.g. \"arg1,arg2\". "
   << GOAFactoryStore->size() << " algorithms have been registered: "
   << GOAFactoryStore->getKeyVector();

	try {
		std::string configFilename = std::string("");
		std::string usageString = std::string("Usage: ") + argv[0] + " [options]";
		po::options_description desc(usageString.c_str());
		desc.add_options()
				("help,h", "emit help message")
				("configFilename,f", po::value<std::string>(&configFilename)->default_value(GO2_DEF_DEFAULTCONFIGFILE),
				"The name of the file holding configuration information for optimization algorithms")
				("executionMode,e", po::value<execMode>(&parMode_)->default_value(GO2_DEF_DEFAULPARALLELIZATIONMODE),
				"The default parallelization mode: (0) means serial execution; "
				"(1) means multi-threaded execution and (2) means execution through the broker")
				("optimizationAlgorithms,a", po::value<std::string>(&optimization_algorithms)->default_value(GO2_DEF_OPTALGS),
             oa_help.str().c_str())
				("clientMode,c", "Makes this program behave as a networked client")
				("ip,i", po::value<std::string>(&ip_)->default_value(GO2_DEF_IP), "The ip of the server")
				("port,p", po::value<unsigned short>(&port_)->default_value(GO2_DEF_PORT),	"The port of the server")
				("serializationMode,s", po::value<Gem::Common::serializationMode>(&serializationMode_)->default_value(GO2_DEF_SERIALIZATIONMODE),
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
			GObject::readConfigFile(configFilename_);
		}

		// Parse the list of optimization algorithms
		if(vm.count("optimizationAlgorithms")) {
			typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
			boost::char_separator<char> comma_sep(",");
			tokenizer oaTokenizer(optimization_algorithms, comma_sep);
			for(tokenizer::iterator oa=oaTokenizer.begin(); oa!=oaTokenizer.end(); ++oa) {
				std::string alg = *oa;
				boost::trim(alg); // Remove any leading or trailing white spaces

				if(!alg.empty()) {
				   // Retrieve the algorithm from the global store
				   boost::shared_ptr<GOptimizationAlgorithmFactoryT<GOABase> > p;
				   if(!GOAFactoryStore->get(alg, p)) {
				      glogger
				      << "In Go2::parseCommandLine(int, char**): Error!" << std::endl
				      << "Got invalid algorithm mnemomic \"" << alg << "\"." << std::endl
				      << "No algorithm found for this string." << std::endl
				      << GEXCEPTION;
				   }

					cl_algorithms_.push_back(p->get(parMode_));
				}
			}
		}

		if (vm.count("clientMode")) clientMode_ = true;

		if(vm.count("verbose")) {
			verbose_ = true;
			std::cout << std::endl
					<< "Running with the following command line options:" << std::endl
					<< "configFilename = " << configFilename_ << std::endl
					<< "executionMode = " << parMode_ << std::endl
					<< "optimizationAlgorithms = " << optimization_algorithms << std::endl
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

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GenevaInitializer::GenevaInitializer()
   : grf_(GRANDOMFACTORY)
   , gbr_(GBROKER(Gem::Geneva::GIndividual))
{
   grf_->init();
   gbr_->init();
}

/******************************************************************************/
/**
 * The destructor
 */
GenevaInitializer::~GenevaInitializer() {
   gbr_->finalize();
   grf_->finalize();

#ifdef GEM_INT_FORCE_TERMINATION // Defined in GGlobalDefines.hpp.in
   std::set_terminate(Go2::GTerminateImproperBoostTermination);
   std::terminate();
#endif /* GEM_INT_FORCE_TERMINATION */
}

/******************************************************************************/
// Create an instance of this class
GenevaInitializer gi;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
