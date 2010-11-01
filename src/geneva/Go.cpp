/**
 * @file Go.cpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "geneva/Go.hpp"

namespace Gem {
namespace Geneva {

/**************************************************************************************/
/**
 * The default constructor
 */
Go::Go()
	: GMutableSetT<GParameterSet>()
	, pers_(GO_DEF_PERSONALITY)
	, parMode_(GO_DEF_PARALLELIZATIONMODE)
	, serverMode_(GO_DEF_SERVERMODE)
	, serializationMode_(GO_DEF_SERIALIZATIONMODE)
	, ip_(GO_DEF_IP)
	, port_(GO_DEF_PORT)
	, configFilename_(GO_DEF_DEFAULTCONFIGFILE)
	, verbose_(GO_DEF_DEFAULTVERBOSE)
	, copyBestOnly_(GO_DEF_COPYBESTONLY)
	, maxStalledDataTransfers_(GO_DEF_MAXSTALLED)
	, maxConnectionAttempts_(GO_DEF_MAXCONNATT)
	, returnRegardless_(GO_DEF_RETURNREGARDLESS)
	, nProducerThreads_(GO_DEF_NPRODUCERTHREADS)
	, arraySize_(GO_DEF_ARRAYSIZE)
	, nEvaluationThreads_(GO_DEF_NEVALUATIONTHREADS)
	, waitFactor_(GO_DEF_WAITFACTOR)
	, maxIterations_(GO_DEF_MAXITERATIONS)
	, maxMinutes_(GO_DEF_MAXMINUTES)
	, reportIteration_(GO_DEF_REPORTITERATION)
	, eaPopulationSize_(GO_DEF_EAPOPULATIONSIZE)
	, eaNParents_(GO_DEF_EANPARENTS)
	, eaRecombinationScheme_(GO_DEF_EARECOMBINATIONSCHEME)
	, eaSortingScheme_(GO_DEF_EASORTINGSCHEME)
	, eaTrackParentRelations_(GO_DEF_EATRACKPARENTRELATIONS)
	, swarmNNeighborhoods_(GO_DEF_SWARMNNEIGHBORHOODS)
	, swarmNNeighborhoodMembers_(GO_DEF_SWARMNNEIGHBORHOODMEMBERS)
	, swarmRandomFillUp_(GO_DEF_SWARMRANDOMFILLUP)
	, swarmCLocal_(GO_DEF_SWARMCLOCAL)
	, swarmCGlobal_(GO_DEF_SWARMCCGLOBAL)
	, swarmCDelta_(GO_DEF_SWARMCCDELTA)
	, swarmUpdateRule_(GO_DEF_SWARMUPDATERULE)
	, gdNStartingPoints_(GO_DEF_GDNSTARTINGPOINTS)
	, gdFiniteStep_(GO_DEF_GDFINITESTEP)
	, gdStepSize_(GO_DEF_GDSTEPSIZE)
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
Go::Go(int argc, char **argv, const std::string& configFilename)
	: GMutableSetT<GParameterSet>()
	, pers_(GO_DEF_PERSONALITY)
	, parMode_(GO_DEF_PARALLELIZATIONMODE)
	, serverMode_(GO_DEF_SERVERMODE)
	, serializationMode_(GO_DEF_SERIALIZATIONMODE)
	, ip_(GO_DEF_IP)
	, port_(GO_DEF_PORT)
	, configFilename_(GO_DEF_DEFAULTCONFIGFILE)
	, verbose_(GO_DEF_DEFAULTVERBOSE)
	, copyBestOnly_(GO_DEF_COPYBESTONLY)
	, maxStalledDataTransfers_(GO_DEF_MAXSTALLED)
	, maxConnectionAttempts_(GO_DEF_MAXCONNATT)
	, returnRegardless_(GO_DEF_RETURNREGARDLESS)
	, nProducerThreads_(GO_DEF_NPRODUCERTHREADS)
	, arraySize_(GO_DEF_ARRAYSIZE)
	, nEvaluationThreads_(GO_DEF_NEVALUATIONTHREADS)
	, waitFactor_(GO_DEF_WAITFACTOR)
	, maxIterations_(GO_DEF_MAXITERATIONS)
	, maxMinutes_(GO_DEF_MAXMINUTES)
	, reportIteration_(GO_DEF_REPORTITERATION)
	, eaPopulationSize_(GO_DEF_EAPOPULATIONSIZE)
	, eaNParents_(GO_DEF_EANPARENTS)
	, eaRecombinationScheme_(GO_DEF_EARECOMBINATIONSCHEME)
	, eaSortingScheme_(GO_DEF_EASORTINGSCHEME)
	, eaTrackParentRelations_(GO_DEF_EATRACKPARENTRELATIONS)
	, swarmNNeighborhoods_(GO_DEF_SWARMNNEIGHBORHOODS)
	, swarmNNeighborhoodMembers_(GO_DEF_SWARMNNEIGHBORHOODMEMBERS)
	, swarmRandomFillUp_(GO_DEF_SWARMRANDOMFILLUP)
	, swarmCLocal_(GO_DEF_SWARMCLOCAL)
	, swarmCGlobal_(GO_DEF_SWARMCCGLOBAL)
	, swarmCDelta_(GO_DEF_SWARMCCDELTA)
	, swarmUpdateRule_(GO_DEF_SWARMUPDATERULE)
	, gdNStartingPoints_(GO_DEF_GDNSTARTINGPOINTS)
	, gdFiniteStep_(GO_DEF_GDFINITESTEP)
	, gdStepSize_(GO_DEF_GDSTEPSIZE)
{
	//--------------------------------------------
	// Load initial configuration options from the command line
	parseCommandLine(argc, argv);

	// Update the name of the configuration file, if necessary
	if(configFilename != GO_DEF_DEFAULTCONFIGFILE) {
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
Go::Go(
	const personality& pers
	, const parMode& pM
	, const bool& serverMode
	, const Gem::Common::serializationMode& sM
	, const std::string& ip
	, const unsigned short& port
	, const std::string& configFilename
	, const bool& verbose
)
	: GMutableSetT<GParameterSet>()
	, pers_(pers)
	, parMode_(pM)
	, serverMode_(serverMode)
	, serializationMode_(sM)
	, ip_(ip)
	, port_(port)
	, configFilename_(configFilename)
	, verbose_(verbose)
	, copyBestOnly_(GO_DEF_COPYBESTONLY)
	, maxStalledDataTransfers_(GO_DEF_MAXSTALLED)
	, maxConnectionAttempts_(GO_DEF_MAXCONNATT)
	, returnRegardless_(GO_DEF_RETURNREGARDLESS)
	, nProducerThreads_(GO_DEF_NPRODUCERTHREADS)
	, arraySize_(GO_DEF_ARRAYSIZE)
	, nEvaluationThreads_(GO_DEF_NEVALUATIONTHREADS)
	, waitFactor_(GO_DEF_WAITFACTOR)
	, maxIterations_(GO_DEF_MAXITERATIONS)
	, maxMinutes_(GO_DEF_MAXMINUTES)
	, reportIteration_(GO_DEF_REPORTITERATION)
	, eaPopulationSize_(GO_DEF_EAPOPULATIONSIZE)
	, eaNParents_(GO_DEF_EANPARENTS)
	, eaRecombinationScheme_(GO_DEF_EARECOMBINATIONSCHEME)
	, eaSortingScheme_(GO_DEF_EASORTINGSCHEME)
	, eaTrackParentRelations_(GO_DEF_EATRACKPARENTRELATIONS)
	, swarmNNeighborhoods_(GO_DEF_SWARMNNEIGHBORHOODS)
	, swarmNNeighborhoodMembers_(GO_DEF_SWARMNNEIGHBORHOODMEMBERS)
	, swarmRandomFillUp_(GO_DEF_SWARMRANDOMFILLUP)
	, swarmCLocal_(GO_DEF_SWARMCLOCAL)
	, swarmCGlobal_(GO_DEF_SWARMCCGLOBAL)
	, swarmCDelta_(GO_DEF_SWARMCCDELTA)
	, swarmUpdateRule_(GO_DEF_SWARMUPDATERULE)
	, gdNStartingPoints_(GO_DEF_GDNSTARTINGPOINTS)
	, gdFiniteStep_(GO_DEF_GDFINITESTEP)
	, gdStepSize_(GO_DEF_GDSTEPSIZE)
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
Go::Go(const Go& cp)
	: GMutableSetT<GParameterSet>(cp)
	, pers_(cp.pers_)
	, parMode_(cp.parMode_)
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
	, nEvaluationThreads_(cp.nEvaluationThreads_)
	, waitFactor_(cp.waitFactor_)
	, maxIterations_(cp.maxIterations_)
	, maxMinutes_(cp.maxMinutes_)
	, reportIteration_(cp.reportIteration_)
	, eaPopulationSize_(cp.eaPopulationSize_)
	, eaNParents_(cp.eaNParents_)
	, eaRecombinationScheme_(cp.eaRecombinationScheme_)
	, eaSortingScheme_(cp.eaSortingScheme_)
	, eaTrackParentRelations_(cp.eaTrackParentRelations_)
	, swarmNNeighborhoods_(cp.swarmNNeighborhoods_)
	, swarmNNeighborhoodMembers_(cp.swarmNNeighborhoodMembers_)
	, swarmRandomFillUp_(cp.swarmRandomFillUp_)
	, swarmCLocal_(cp.swarmCLocal_)
	, swarmCGlobal_(cp.swarmCGlobal_)
	, swarmCDelta_(cp.swarmCDelta_)
	, swarmUpdateRule_(cp.swarmUpdateRule_)
	, gdNStartingPoints_(cp.gdNStartingPoints_)
	, gdFiniteStep_(cp.gdFiniteStep_)
	, gdStepSize_(cp.gdStepSize_)
{
	//--------------------------------------------
	// Copy the optimization monitors over (if any)
	copyGenevaSmartPointer<GEvolutionaryAlgorithm::GEAOptimizationMonitor>(cp.ea_om_ptr_, ea_om_ptr_);
	copyGenevaSmartPointer<GSwarm::GSwarmOptimizationMonitor>(cp.swarm_om_ptr_, swarm_om_ptr_);
	copyGenevaSmartPointer<GGradientDescent::GGDOptimizationMonitor>(cp.gd_om_ptr_, gd_om_ptr_);

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
Go::~Go()
{ /* nothing */ }

/**************************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another Go object
 * @return A constant reference to this object
 */
const Go& Go::operator=(const Go& cp) {
	Go::load_(&cp);
	return *this;
}

/**************************************************************************************/
/**
 * Checks for equality with another Go object
 *
 * @param  cp A constant reference to another Go object
 * @return A boolean indicating whether both objects are equal
 */
bool Go::operator==(const Go& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"Go::operator==","cp", CE_SILENT);
}

/**************************************************************************************/
/**
 * Checks for inequality with another Go object
 *
 * @param  cp A constant reference to another Go object
 * @return A boolean indicating whether both objects are inequal
 */
bool Go::operator!=(const Go& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"Go::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> Go::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const {
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GOptimizationMonitorT reference
	const Go *p_load = GObject::conversion_cast<Go>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GMutableSetT<GParameterSet>::checkRelationshipWith(cp, e, limit, "Go", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "Go", pers_, p_load->pers_, "pers_", "p_load->pers_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", parMode_, p_load->parMode_, "parMode_", "p_load->parMode_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", serverMode_, p_load->serverMode_, "serverMode_", "p_load->serverMode_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", serializationMode_, p_load->serializationMode_, "serializationMode_", "p_load->serializationMode_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", ip_, p_load->ip_, "ip_", "p_load->ip_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", port_, p_load->port_, "port_", "p_load->port_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", configFilename_, p_load->configFilename_, "configFilename_", "p_load->configFilename_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", verbose_, p_load->verbose_, "verbose_", "p_load->verbose_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", ea_om_ptr_, p_load->ea_om_ptr_, "ea_om_ptr_", "p_load->ea_om_ptr_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", swarm_om_ptr_, p_load->swarm_om_ptr_, "swarm_om_ptr_", "p_load->swarm_om_ptr_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", gd_om_ptr_, p_load->gd_om_ptr_, "gd_om_ptr_", "p_load->gd_om_ptr_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", copyBestOnly_, p_load->copyBestOnly_, "copyBestOnly_", "p_load->copyBestOnly_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", maxStalledDataTransfers_, p_load->maxStalledDataTransfers_, "maxStalledDataTransfers_", "p_load->maxStalledDataTransfers_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", maxConnectionAttempts_, p_load->maxConnectionAttempts_, "maxConnectionAttempts_", "p_load->maxConnectionAttempts_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", returnRegardless_, p_load->returnRegardless_, "returnRegardless_", "p_load->returnRegardless_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", nProducerThreads_, p_load->nProducerThreads_, "nProducerThreads_", "p_load->nProducerThreads_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", arraySize_, p_load->arraySize_, "arraySize_", "p_load->arraySize_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", nEvaluationThreads_, p_load->nEvaluationThreads_, "nEvaluationThreads_", "p_load->nEvaluationThreads_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", waitFactor_, p_load->waitFactor_, "waitFactor_", "p_load->waitFactor_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", maxIterations_, p_load->maxIterations_, "maxIterations_", "p_load->maxIterations_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", maxMinutes_, p_load->maxMinutes_, "maxMinutes_", "p_load->maxMinutes_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", reportIteration_, p_load->reportIteration_, "reportIteration_", "p_load->reportIteration_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", eaPopulationSize_, p_load->eaPopulationSize_, "eaPopulationSize_", "p_load->eaPopulationSize_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", eaNParents_, p_load->eaNParents_, "eaNParents_", "p_load->eaNParents_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", eaRecombinationScheme_, p_load->eaRecombinationScheme_, "eaRecombinationScheme_", "p_load->eaRecombinationScheme_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", eaSortingScheme_, p_load->eaSortingScheme_, "eaSortingScheme_", "p_load->eaSortingScheme_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", eaTrackParentRelations_, p_load->eaTrackParentRelations_, "eaTrackParentRelations_", "p_load->eaTrackParentRelations_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", swarmNNeighborhoods_, p_load->swarmNNeighborhoods_, "swarmNNeighborhoods_", "p_load->swarmNNeighborhoods_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", swarmNNeighborhoodMembers_, p_load->swarmNNeighborhoodMembers_, "swarmNNeighborhoodMembers_", "p_load->swarmNNeighborhoodMembers_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", swarmRandomFillUp_, p_load->swarmRandomFillUp_, "swarmRandomFillUp_", "p_load->swarmRandomFillUp_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", swarmCLocal_, p_load->swarmCLocal_, "swarmCLocal_", "p_load->swarmCLocal_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", swarmCGlobal_, p_load->swarmCGlobal_, "swarmCGlobal_", "p_load->swarmCGlobal_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", swarmCDelta_, p_load->swarmCDelta_, "swarmCDelta_", "p_load->swarmCDelta_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", swarmUpdateRule_, p_load->swarmUpdateRule_, "swarmUpdateRule_", "p_load->swarmUpdateRule_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", gdNStartingPoints_, p_load->gdNStartingPoints_, "gdNStartingPoints_", "p_load->gdNStartingPoints_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", gdFiniteStep_, p_load->gdFiniteStep_, "gdFiniteStep_", "p_load->gdFiniteStep_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", gdStepSize_, p_load->gdStepSize_, "gdStepSize_", "p_load->gdStepSize_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "Go", bestIndividual_, p_load->bestIndividual_, "bestIndividual_", "p_load->bestIndividual_", e , limit));

	return evaluateDiscrepancies("Go", caller, deviations, e);
}

/**************************************************************************************/
/**
 * Loads the data of another Go object
 *
 * @param cp A copy of another Go object, camouflaged as a GObject
 */
void Go::load_(const GObject *cp) {
	const Go *p_load = conversion_cast<Go>(cp);

	// First load the parent class'es data ...
	GMutableSetT<GParameterSet>::load_(cp);

	// and then our local data
	pers_ = p_load->pers_;
	parMode_ = p_load->parMode_;
	serverMode_ = p_load->serverMode_;
	serializationMode_ = p_load->serializationMode_;
	ip_ = p_load->ip_;
	port_ = p_load->port_;
	configFilename_ = p_load->configFilename_;
	verbose_ = p_load->verbose_;

	copyGenevaSmartPointer<GEvolutionaryAlgorithm::GEAOptimizationMonitor>(p_load->ea_om_ptr_, ea_om_ptr_);
	copyGenevaSmartPointer<GSwarm::GSwarmOptimizationMonitor>(p_load->swarm_om_ptr_, swarm_om_ptr_);
	copyGenevaSmartPointer<GGradientDescent::GGDOptimizationMonitor>(p_load->gd_om_ptr_, gd_om_ptr_);

	copyBestOnly_ = p_load->copyBestOnly_;
	maxStalledDataTransfers_ = p_load->maxStalledDataTransfers_;
	maxConnectionAttempts_ = p_load->maxConnectionAttempts_;
	returnRegardless_ = p_load->returnRegardless_;
	nProducerThreads_ = p_load->nProducerThreads_;
	arraySize_ = p_load->arraySize_;
	nEvaluationThreads_ = p_load->nEvaluationThreads_;
	waitFactor_ = p_load->waitFactor_;
	maxIterations_ = p_load->maxIterations_;
	maxMinutes_ = p_load->maxMinutes_;
	reportIteration_ = p_load->reportIteration_;
	eaPopulationSize_ = p_load->eaPopulationSize_;
	eaNParents_ = p_load->eaNParents_;
	eaRecombinationScheme_ = p_load->eaRecombinationScheme_;
	eaSortingScheme_ = p_load->eaSortingScheme_;
	eaTrackParentRelations_ = p_load->eaTrackParentRelations_;
	swarmNNeighborhoods_ = p_load->swarmNNeighborhoods_;
	swarmNNeighborhoodMembers_ = p_load->swarmNNeighborhoodMembers_;
	swarmRandomFillUp_ = p_load->swarmRandomFillUp_;
	swarmCLocal_ = p_load->swarmCLocal_;
	swarmCGlobal_ = p_load->swarmCGlobal_;
	swarmCDelta_ = p_load->swarmCDelta_;
	swarmUpdateRule_ = p_load->swarmUpdateRule_;
	gdNStartingPoints_ = p_load->gdNStartingPoints_;
	gdFiniteStep_ = p_load->gdFiniteStep_;
	gdStepSize_ = p_load->gdStepSize_;

	copyGenevaSmartPointer<GParameterSet>(p_load->bestIndividual_, bestIndividual_);
}

/**************************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object
 */
GObject *Go::clone_() const {
	return new Go(*this);
}



/**************************************************************************************/
/**
 * Allows to specify an optimization monitor to be used with evolutionary algorithms.
 * Note that this function will take ownership of the monitor by cloning it.
 *
 * @param ea_om_ptr A pointer to an optimization monitor specific for evolutionary algorithms
 */
void Go::registerOptimizationMonitor(boost::shared_ptr<GEvolutionaryAlgorithm::GEAOptimizationMonitor> ea_om_ptr) {
	if(!ea_om_ptr) {
		std::ostringstream error;
		error << "In Go::registerOptimizationMonitor(): Error!" << std::endl
			  << "Empty optimization monitor pointer found for EA" << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	ea_om_ptr_ = ea_om_ptr->clone<GEvolutionaryAlgorithm::GEAOptimizationMonitor>();
}

/**************************************************************************************/
/**
 * Allows to specify an optimization monitor to be used with swarm algorithms.
 * Note that this function will take ownership of the monitor by cloning it.
 *
 * @param swarm_om_ptr A pointer to an optimization monitor specific for swarm algorithms
 */
void Go::registerOptimizationMonitor(boost::shared_ptr<GSwarm::GSwarmOptimizationMonitor> swarm_om_ptr) {
	if(!swarm_om_ptr) {
		std::ostringstream error;
		error << "In Go::registerOptimizationMonitor(): Error!" << std::endl
			  << "Empty optimization monitor pointer found for SWARM" << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	swarm_om_ptr_ = swarm_om_ptr->clone<GSwarm::GSwarmOptimizationMonitor>();
}

/**************************************************************************************/
/**
 * Allows to specify an optimization monitor to be used with gradient descents.
 * Note that this function will take ownership of the monitor by cloning it.
 *
 * @param gd_om_ptr A pointer to an optimization monitor specific for gradient descents
 */
void Go::registerOptimizationMonitor(boost::shared_ptr<GGradientDescent::GGDOptimizationMonitor> gd_om_ptr) {
	if(!gd_om_ptr) {
		std::ostringstream error;
		error << "In Go::registerOptimizationMonitor(): Error!" << std::endl
			  << "Empty optimization monitor pointer found for GD" << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	gd_om_ptr_ = gd_om_ptr->clone<GGradientDescent::GGDOptimizationMonitor>();
}

/**************************************************************************************/
/**
 * Triggers execution of the client loop. Note that it is up to you to terminate
 * the program after calling this function.
 */
bool Go::clientRun() {
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
	}
}

/**************************************************************************************/
/**
 * Checks whether server mode has been requested for this object
 *
 * @return A boolean which indicates whether the server mode has been set for this object
 */
bool Go::serverMode() const {
	return serverMode_;
}

/**************************************************************************************/
/**
 * Checks whether this object is running in client mode
 *
 * @return A boolean which indicates whether the client mode has been set for this object
 */
bool Go::clientMode() const {
	return !serverMode_;
}

/**************************************************************************************/
/*
 * Specifies whether only the best individuals of a population should be copied.
 *
 * @param copyBestOnly Specifies whether only the best individuals of a population are copied
 */
void Go::setCopyBestIndividualsOnly(const bool& copyBestOnly) {
	copyBestOnly_ = copyBestOnly;
}

/**************************************************************************************/
/**
 * Checks whether only the best individuals are copied
 *
 * @return A boolean indicating whether only the best individuals of a population are copied
 */
bool Go::onlyBestIndividualsAreCopied() const {
	return copyBestOnly_;
}

/**************************************************************************************/
/**
 * Allows to randomly initialize parameter members. Note that for this wrapper object
 * this function doesn't make any sense. It is made available to satisfy a requirement
 * of GIndividual.
 */
void Go::randomInit()
{ /* nothing */ }

/**************************************************************************************/
/**
 * Fitness calculation for an optimization algorithm means optimization. The fitness is
 * then determined by the best individual which, after the end of the optimization cycle.
 *
 * @return The fitness of the best individual in the population
 */
double Go::fitnessCalculation() {
	bool dirty = false;

	boost::shared_ptr<GParameterSet> p = this->optimize<GParameterSet>();

	double val = p->getCurrentFitness(dirty);
	// is this the current fitness ? We should at this stage never
	// run across an unevaluated individual.
	if(dirty) {
		std::ostringstream error;
		error << "In Go::fitnessCalculation(): Error!" << std::endl
			  << "Came across dirty individual" << std::endl;

		// throw an exception. Add some information so that if the exception
		// is caught through a base object, no information is lost.
		throw Gem::Common::gemfony_error_condition(error.str());
	}

	return val;
}

/**************************************************************************************/
/**
 * Allows to set the type of optimization algorithm to be used for the optimization
 *
 * @param pers The type of optimization algorithm to be used for the optimization
 */
void Go::setPersonality(const personality& pers) {
	pers_ = pers;
}

/**************************************************************************************/
/**
 * Allows to retrieve the type of optimization algorithm currently used for the optimization
 *
 * @return The type of optimization algorithm currently used for the optimization
 */
personality Go::getPersonality() const {
	return pers_;
}

/**************************************************************************************/
/**
 * Allows to set the parallelization mode used for the optimization
 *
 * @param parMode The parallelization mode used for the optimization
 */
void Go::setParallelizationMode(const parMode& parMode) {
	parMode_ = parMode;
}

/**************************************************************************************/
/**
 * Allows to retrieve the parallelization mode currently used for the optimization
 *
 * @return The parallelization mode currently used for the optimization
 */
parMode Go::getParallelizationMode() const {
	return parMode_;
}

/**************************************************************************************/
/**
 * Allows to mark this object as belonging to a server as opposed to a client
 *
 * @param serverMode Allows to mark this object as belonging to a server as opposed to a client
 */
void Go::setServerMode(const bool& serverMode) {
	serverMode_ = serverMode;
}

/**************************************************************************************/
/**
 * Allows to check whether this object is working in server or client mode
 *
 * @return A boolean indicating whether this object is working in server or client mode
 */
bool Go::getServerMode() const {
	return serverMode_;
}

/**************************************************************************************/
/**
 * Allows to set the serialization mode used for network transfers
 *
 * @param serializationMode The serialization mode used for network transfers
 */
void Go::setSerializationMode(const Gem::Common::serializationMode& serializationMode) {
	serializationMode_ = serializationMode;
}

/**************************************************************************************/
/**
 * Allows to retrieve the serialization mode currently used for network transfers
 */
Gem::Common::serializationMode Go::getSerializationMode() const {
	return serializationMode_;
}

/**************************************************************************************/
/**
 * Allows to set the ip of the server
 *
 * @param ip The ip of the server
 */
void Go::setServerIp(const std::string& ip) {
	ip_ = ip;
}

/**************************************************************************************/
/**
 * Allows to retrieve the ip of the server
 *
 * @return The current ip used to access the server
 */
std::string Go::getServerIp() const {
	return ip_;
}

/**************************************************************************************/
/**
 * Allows to set the port used to access the server
 *
 * @param port The port used to access the server
 */
void Go::setServerPort(const unsigned short& port) {
	port_ = port;
}

/**************************************************************************************/
/**
 * Allows to retrieve the port currently used to access the server
 *
 * @return The number of the port currently used to access the server
 */
unsigned short Go::getServerPort() const {
	return port_;
}

/**************************************************************************************/
/**
 * Allows to set the name of the configuration file from which further options will
 * be read.
 *
 * @param configFilename The name of the file from which further options will be read
 */
void Go::setConfigFileName(const std::string& configFilename) {
	configFilename_ = configFilename;
}

/**************************************************************************************/
/**
 * Allows to retrieve the name of the configuration file from which further options will
 * be read
 *
 * @return The name of the configuration file from which further options will be read
 */
std::string Go::getConfigFileName() const {
	return configFilename_;
}

/**************************************************************************************/
/**
 * Allows to specify whether further information should be emitted after parsing the
 * command line and configuration file.
 *
 * @param verbose Specifies whether information about parsed variables should be emitted in a more verbose format
 */
void Go::setVerbosity(const bool& verbose) {
	verbose_ = verbose;
}

/**************************************************************************************/
/**
 * Allows to check whether further information should be emitted after parsing the
 * command line and configuration file.
 */
bool Go::getVerbosity() const {
	return verbose_;
}

/**************************************************************************************/
/**
 * Allows to specify the number of failed data transfers before a client terminates
 * its work. Set this to 0 in order to loop indefinitely.
 *
 * @param maxStalledDataTransfers The number of failed data transfers before a client terminates its work
 */
void Go::setMaxStalledDataTransfers(const boost::uint32_t& maxStalledDataTransfers) {
	maxStalledDataTransfers_ = maxStalledDataTransfers;
}

/**************************************************************************************/
/**
 * Allows to retrieve the number of failed data transfers before a client terminates
 * its work. Set this to 0 in order to loop indefinitely.
 *
 * @return The number of failed data transfers before a client terminates its work
 */
boost::uint32_t Go::getMaxStalledDataTransfers() const {
	return maxStalledDataTransfers_;
}

/**************************************************************************************/
/**
 * Allows to specify how often a client may try to connect the server without response
 * before terminating itself.
 *
 * @param maxConnectionAttempts Specifies the number of failed connection attempts before the client terminates itself
 */
void Go::setMaxConnectionAttempts(const boost::uint32_t& maxConnectionAttempts) {
	maxConnectionAttempts_ = maxConnectionAttempts;
}

/**************************************************************************************/
/**
 * Allows to retrieve the amount of times a client may try to connect the server without response
 * before terminating itself.
 *
 * @return The amount of times a client may try to connect the server without response before terminating itself.
 */
boost::uint32_t Go::getMaxConnectionAttempts() const {
	return maxConnectionAttempts_;
}

/**************************************************************************************/
/**
 * Allows to specify whether a client should return results even though here was no
 * improvement.
 *
 * @param returnRegardless Specifies whether a client should return results even though here was no improvement
 */
void Go::setReturnRegardless(const bool& returnRegardless) {
	returnRegardless_ = returnRegardless;
}

/**************************************************************************************/
/**
 * Allows to check whether a client should return results even though here was no
 * improvement.
 *
 * @return A boolean indicating whether a client should return results even though here was no improvement
 */
bool Go::getReturnRegardless() const {
	return returnRegardless_;
}

/**************************************************************************************/
/**
 * Allows to set the number of threads that will simultaneously produce random numbers.
 *
 * @param nProducerThreads The number of threads that will simultaneously produce random numbers
 */
void Go::setNProducerThreads(const boost::uint16_t& nProducerThreads) {
	nProducerThreads_ = nProducerThreads;
}

/**************************************************************************************/
/**
 * Allows to retrieve the number of threads that will simultaneously produce random numbers.
 *
 * @return The number of threads that will simultaneously produce random numbers
 */
boost::uint16_t Go::getNProducerThreads() const {
	return nProducerThreads_;
}

/**************************************************************************************/
/**
 * Allows to set the size of the array of random numbers transferred to proxies upon request.
 *
 * @param arraySize The size of the array of random numbers transferred to proxies upon request
 */
void Go::setArraySize(const std::size_t& arraySize) {
	arraySize_ = arraySize;
}

/**************************************************************************************/
/**
 * Allows to retrieve the size of the array of random numbers transferred to proxies upon request.
 *
 * @return The size of the array of random numbers transferred to proxies upon request
 */
std::size_t Go::getArraySize() const {
	return arraySize_;
}

/**************************************************************************************/
/**
 * Allows to set the number of threads that will simultaneously evaluate individuals.
 * Set this to 0 to set this number to the amount of CPU cores in your system.
 *
 * @param nEvaluationThreads The number of threads that will simultaneously evaluate individuals
 */
void Go::setNEvaluationThreads(const boost::uint16_t& nEvaluationThreads) {
	nEvaluationThreads_ = nEvaluationThreads;
}

/**************************************************************************************/
/**
 * Allows to retrieve the number of threads that will simultaneously evaluate individuals.
 *
 * @return The number of threads that will simultaneously evaluate individuals
 */
boost::uint16_t Go::getNEvaluationThreads() const {
	return nEvaluationThreads_;
}

/**************************************************************************************/
/**
 * Allows to set the wait factor used in each iteration to wait for further arrivals.
 * This is interpreted as a multiple of the arrival times of the first individual.
 *
 * @param waitFactor The wait factor used in each iteration to wait for further arrivals
 */
void Go::setWaitFactor(const boost::uint32_t& waitFactor) {
	waitFactor_ = waitFactor;
}

/**************************************************************************************/
/**
 * Allows to retrieve the wait factor used in each iteration to wait for further arrivals.
 *
 * @return The wait factor used in each iteration to wait for further arrivals
 */
boost::uint32_t Go::getWaitFactor() const {
	return waitFactor_;
}

/**************************************************************************************/
/**
 * Allows to specify the maximum amount of iterations in an optimization run
 *
 * @param maxIterations The maximum amount of iterations in an optimization run
 */
void Go::setMaxIterations(const boost::uint32_t& maxIterations) {
	maxIterations_ = maxIterations;
}

/**************************************************************************************/
/**
 * Allows to retrieve the maximum amount of iterations in an optimization run
 *
 * @return The maximum amount of iterations in an optimization run
 */
boost::uint32_t Go::getMaxIterations() const {
	return maxIterations_;
}

/**************************************************************************************/
/**
 * Allows to specify the maximum amount of minutes an optimization may last
 *
 * @param maxMinutes The maximum amount of minutes an optimization may last
 */
void Go::setMaxMinutes(const long& maxMinutes) {
	maxMinutes_ = maxMinutes;
}

/**************************************************************************************/
/**
 * Allows to retrieve the maximum amount of minutes an optimization may last
 *
 * @return The maximum amount of minutes an optimization may last
 */
long Go::getMaxMinutes() const {
	return maxMinutes_;
}

/**************************************************************************************/
/**
 * Allows to specify in which intervals information about the optimization's progress should be emitted
 *
 * @param reportIteration The intervals in which information about the optimization's progress should be emitted
 */
void Go::setReportIteration(const boost::uint32_t& reportIteration) {
	reportIteration_ = reportIteration;
}

/**************************************************************************************/
/**
 * Allows to check in which intervals information about the optimization's progress should be emitted
 *
 * @return The intervals in which information about the optimization's progress should be emitted
 */
boost::uint32_t Go::getReportIteration() const {
	return reportIteration_;
}

/**************************************************************************************/
/**
 * Allows to set the default size of an evolutionary algorithm population
 *
 * @param eaPopulationSize The default size of an evolutionary algorithm population
 */
void Go::setEAPopulationSize(const std::size_t& eaPopulationSize) {
	eaPopulationSize_ = eaPopulationSize;
}

/**************************************************************************************/
/**
 * Allows to retrieve the default size of an evolutionary algorithm population
 *
 * @return The default size of an evolutionary algorithm population
 */
std::size_t Go::getEAPopulationSize() const {
	return eaPopulationSize_;
}

/**************************************************************************************/
/**
 * Allows to set the number of parents in an evolutionary algorithm population
 *
 * @param eaNParents The number of parents in an evolutionary algorithm population
 */
void Go::setEANParents(const std::size_t& eaNParents) {
	eaNParents_ = eaNParents;
}

/**************************************************************************************/
/**
 * Allows to retrieve the number of parents in an evolutionary algorithm population
 *
 * @return The number of parents in an evolutionary algorithm population
 */
std::size_t Go::getEANParents() const {
	return eaNParents_;
}

/**************************************************************************************/
/**
 * Allows to set the recombination scheme used in evolutionary algorithms
 *
 * @param eaRecombinationScheme The recombination scheme used in evolutionary algorithms
 */
void Go::setEARecombinationScheme(const recoScheme& eaRecombinationScheme) {
	eaRecombinationScheme_ = eaRecombinationScheme;
}

/**************************************************************************************/
/**
 * Allows to retrieve the recombination scheme used in evolutionary algorithms
 *
 * @return The recombination scheme used in evolutionary algorithms
 */
recoScheme Go::getEARecombinationScheme() const {
	return eaRecombinationScheme_;
}

/**************************************************************************************/
/**
 * Allows to set the sorting scheme used in evolutionary algorithms
 *
 * @param eaSortingScheme The sorting scheme used in evolutionary algorithms
 */
void Go::setEASortingScheme(const sortingMode& eaSortingScheme) {
	eaSortingScheme_ = eaSortingScheme;
}

/**************************************************************************************/
/**
 * Allows to retrieve the sorting scheme used in evolutionary algorithms
 *
 * @return The sorting scheme used in evolutionary algorithms
 */
sortingMode Go::getEASortingScheme() const {
	return eaSortingScheme_;
}

/**************************************************************************************/
/**
 * Allows to specify whether evolutionary algorithms should track the relationship
 * between parents and children
 *
 * @param eaTrackParentRelations Determines whether evolutionary algorithms should track the relationship between parents and children
 */
void Go::setEATrackParentRelations(const bool& eaTrackParentRelations) {
	eaTrackParentRelations_ = eaTrackParentRelations;
}

/**************************************************************************************/
/**
 * Allows to check whether evolutionary algorithms should track the relationship
 * between parents and children
 *
 * @return Whether evolutionary algorithms track the relationship between parents and children
 */
bool Go::getEATrackParentRelations() const {
	return eaTrackParentRelations_;
}

/**************************************************************************************/
/**
 * Allows to set the number of neighborhoods in a swarm algorithm
 *
 * @param swarmNNeighborhoods The number of neighborhoods in a swarm algorithm
 */
void Go::setSwarmNNeighborhoods(const std::size_t& swarmNNeighborhoods) {
	swarmNNeighborhoods_ = swarmNNeighborhoods;
}

/**************************************************************************************/
/**
 * Allows to retrieve the number of neighborhoods in a swarm algorithm
 *
 * @return The number of neighborhoods in a swarm algorithm
 */
std::size_t Go::getSwarmNNeighborhoods() const {
	return swarmNNeighborhoods_;
}

/**************************************************************************************/
/**
 * Allows to set the number of individuals in each neighborhood in a swarm algorithm
 *
 * @param swarmNNeighborhoodMembers The number of individuals in each neighborhood in a swarm algorithm
 */
void Go::setSwarmNNeighborhoodMembers(const std::size_t& swarmNNeighborhoodMembers) {
	swarmNNeighborhoodMembers_ = swarmNNeighborhoodMembers;
}

/**************************************************************************************/
/**
 * Allows to retrieve the number of individuals in each neighborhood in a swarm algorithm
 *
 * @return The number of individuals in each neighborhood in a swarm algorithm
 */
std::size_t Go::getSwarmNNeighborhoodMembers() const {
	return swarmNNeighborhoodMembers_;
}

/**************************************************************************************/
/**
 * Allows to specify whether missing individuals in a swarm algorithm should be initialized
 * randomly
 *
 * @param swarmRandomFillUp Determines whether missing individuals should be initialized randomly
 */
void Go::setSwarmRandomFillUp(const bool& swarmRandomFillUp) {
	swarmRandomFillUp_ = swarmRandomFillUp;
}

/**************************************************************************************/
/**
 * Allows to check whether missing individuals in a swarm algorithm should be initialized
 * randomly
 *
 * @return Determines whether missing individuals should be initialized randomly
 */
bool Go::getSwarmRandomFillUp() const {
	return swarmRandomFillUp_;
}

/**************************************************************************************/
/**
 * Allows to set the desired value of the swarm algorithm's CLocal parameter
 *
 * @param swarmCLocal The desired value of the swarm algorithm's CLocal parameter
 */
void Go::setSwarmCLocal(const float& swarmCLocal) {
	swarmCLocal_ = swarmCLocal;
}

/**************************************************************************************/
/**
 * Allows to retrieve the current value of the swarm algorithm's CLocal parameter
 *
 * @return The current value of the swarm algorithm's CLocal parameter
 */
float Go::getSwarmCLocal() const {
	return swarmCLocal_;
}

/**************************************************************************************/
/**
 * Allows to set the desired value of the swarm algorithm's CGlobal parameter
 *
 * @param swarmCGlobal The desired value of the swarm algorithm's CGlobal parameter
 */
void Go::setSwarmCGlobal(const float& swarmCGlobal) {
	swarmCGlobal_ = swarmCGlobal;
}

/**************************************************************************************/
/**
 * Allows to retrieve the current value of the swarm algorithm's CGlobal parameter
 *
 * @return The current value of the swarm algorithm's CGlobal parameter
 */
float Go::getSwarmCGlobal() const {
	return swarmCGlobal_;
}

/**************************************************************************************/
/**
 * Allows to set the desired value of the swarm algorithm's CDelta parameter
 *
 * @param swarmCDelta The desired valuze of the swarm algorithm's CDelta parameter
 */
void Go::setSwarmCDelta(const float& swarmCDelta) {
	swarmCDelta_ = swarmCDelta;
}

/**************************************************************************************/
/**
 * Allows to retrieve the current value of the swarm algorithm's CDelta parameter
 *
 * @return The current value of the swarm algorithm's CDelta parameter
 */
float Go::getSwarmCDelta() const {
	return swarmCDelta_;
}

/**************************************************************************************/
/**
 * Allows to set the desired update rule used in swarm algorithms
 *
 * @param swarmUpdateRule The desired update rule used in swarm algorithms
 */
void Go::setSwarmUpdateRule(const updateRule& swarmUpdateRule) {
	swarmUpdateRule_ = swarmUpdateRule;
}

/**************************************************************************************/
/**
 * Allows to retrieve the current update rule used in swarm algorithms
 *
 * @return The current update rule used in swarm algorithms
 */
updateRule Go::getSwarmUpdateRule() const {
	return swarmUpdateRule_;
}

/**************************************************************************************/
/**
 * Allows to set the number of simultaneous starting points in a gradient descent
 *
 * @param gdNStartingPoints The number of simultaneous starting points in a gradient descent
 */
void Go::setGDNStartingPoints(const std::size_t& gdNStartingPoints) {
	gdNStartingPoints_ = gdNStartingPoints;
}

/**************************************************************************************/
/**
 * Allows to retrieve the number of simultaneous starting points in a gradient descent
 *
 * @return The number of simultaneous starting points in a gradient descent
 */
std::size_t Go::getGDNStartingPoints() const {
	return gdNStartingPoints_;
}

/**************************************************************************************/
/**
 * Allows to set the finite step size used in each iteration for each parameter in gradient
 * descents.
 *
 * @param gdFiniteStep The finite step size used in each iteration for each parameter in gradient descents
 */
void Go::setGDFiniteStep(const float& gdFiniteStep) {
	gdFiniteStep_ = gdFiniteStep;
}

/**************************************************************************************/
/**
 * Returns the finite step size used in each iteration for each parameter in gradient
 * descents.
 *
 * @return The finite step size used in each iteration for each parameter in gradient descents
 */
float Go::getGDFiniteStep() const {
	return gdFiniteStep_;
}

/**************************************************************************************/
/**
 * Sets the desired step size of gradient descents
 *
 * @param gdStepSize The desired step size of gradient descents
 */
void Go::setGDStepSize(const float& gdStepSize) {
	gdStepSize_ = gdStepSize;
}

/**************************************************************************************/
/**
 * Returns the current step size of gradient descents
 *
 * @return The current step size of gradient descents
 */
float Go::getGDStepSize() const {
	return gdStepSize_;
}

/**************************************************************************************/
/**
 *
 * @param argc The number of command line arguments
 * @param argv An array with the arguments
 */
void Go::parseCommandLine(int argc, char **argv) {
    namespace po = boost::program_options;

	try {
		std::string usageString = std::string("Usage: ") + argv[0] + " [options]";
		po::options_description desc(usageString.c_str());
		desc.add_options()
				("help,h", "emit help message")
				("configFilename,c", po::value<std::string>(&configFilename_)->default_value(GO_DEF_DEFAULTCONFIGFILE),
				"The name of the file holding configuration information for optimization algorithms")
				("algorithm,a", po::value<personality>(&pers_)->default_value(GO_DEF_PERSONALITY),
				"The type of optimization algorithm: Evolutionary Algorithm (1), Gradient Descent (2), Swarm (3)")
				("parallelizationMode,p", po::value<parMode>(&parMode_)->default_value(GO_DEF_DEFAULPARALLELIZATIONMODE),
				"Whether to perform the optimization in serial mode (0), multi-threaded (1) or networked (2) mode")
				("serverMode,s",
				"Whether to run networked execution in client or server mode. This option only gets evaluated if \"--parallelizationMode=2\" is set")
				("ip", po::value<std::string>(&ip_)->default_value(GO_DEF_IP),
				"The ip of the server")
				("port", po::value<unsigned short>(&port_)->default_value(GO_DEF_PORT),
				"The port of the server")
				("serializationMode", po::value<Gem::Common::serializationMode>(&serializationMode_)->default_value(GO_DEF_SERIALIZATIONMODE),
				"Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)")
				("writeConfigFile,w",
				"Instructs the program to write out a configuration file and then to exit. If the configuration option \"configFileName\" has been specified, its value will be used as the name of the file")
				("verbose,v", po::value<bool>(&verbose_)->default_value(GO_DEF_DEFAULTVERBOSE),
				"Instructs the parsers to output information about configuration parameters")
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
			Go::writeConfigurationFile(configFilename_);
			exit(0);
		}

		if (parMode_ == 2  &&  !vm.count("serverMode")) serverMode_ = false;
		else serverMode_ = true;

		if(verbose_) {
			std::cout << std::endl
					<< "Running with the following command line options:" << std::endl
					<< "configFilename = " << configFilename_ << std::endl
					<< "algorithm = " << pers_ << std::endl
					<< "parMode = " << parMode_ << std::endl
					<< "serverMode = " << serverMode_ << std::endl
					<< "ip = " << ip_ << std::endl
					<< "port = " << port_ << std::endl
					<< "serializationMode = " << serializationMode_ << std::endl;
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
 * Loads the configuration data from a given configuration file
 */
void Go::parseConfigurationFile(const std::string& configFile) {
	namespace bf = boost::filesystem;
    namespace po = boost::program_options;

	// Check the name of the configuration file
	if (!bf::exists(configFile)) {
		std::ostringstream error;
		error << "In Go::parseConfigurationFile(): Error!" << std::endl
			  << "Invalid file name given for configuration file: \"" << configFile << "\"" << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	try {
		po::options_description config(configFile.c_str());
		config.add_options()
		("maxStalledDataTransfers", po::value<boost::uint32_t>(&maxStalledDataTransfers_)->default_value(GO_DEF_MAXSTALLED))
		("maxConnectionAttempts", po::value<boost::uint32_t>(&maxConnectionAttempts_)->default_value(GO_DEF_MAXCONNATT))
		("copyBestOnly", po::value<bool>(&copyBestOnly_)->default_value(GO_DEF_COPYBESTONLY))
		("returnRegardless", po::value<bool>(&returnRegardless_)->default_value(GO_DEF_RETURNREGARDLESS))
		("nProducerThreads", po::value<boost::uint16_t>(&nProducerThreads_)->default_value(GO_DEF_NPRODUCERTHREADS))
		("arraySize", po::value<std::size_t>(&arraySize_)->default_value(GO_DEF_ARRAYSIZE))
		("nEvaluationThreads", po::value<boost::uint16_t>(&nEvaluationThreads_)->default_value(GO_DEF_NEVALUATIONTHREADS))
		("serializationMode", po::value<Gem::Common::serializationMode>(&serializationMode_)->default_value(GO_DEF_SERIALIZATIONMODE))
		("waitFactor", po::value<boost::uint32_t>(&waitFactor_)->default_value(GO_DEF_WAITFACTOR))
		("maxIterations", po::value<boost::uint32_t>(&maxIterations_)->default_value(GO_DEF_MAXITERATIONS))
		("maxMinutes", po::value<long>(&maxMinutes_)->default_value(GO_DEF_MAXMINUTES))
		("reportIteration", po::value<boost::uint32_t>(&reportIteration_)->default_value(GO_DEF_REPORTITERATION))
		("eaPopulationSize", po::value<std::size_t>(&eaPopulationSize_)->default_value(GO_DEF_EAPOPULATIONSIZE))
		("eaNParents", po::value<std::size_t>(&eaNParents_)->default_value(GO_DEF_EANPARENTS))
		("eaRecombinationScheme", po::value<recoScheme>(&eaRecombinationScheme_)->default_value(GO_DEF_EARECOMBINATIONSCHEME))
		("eaSortingScheme", po::value<sortingMode>(&eaSortingScheme_)->default_value(GO_DEF_EASORTINGSCHEME))
		("eaTrackParentRelations", po::value<bool>(&eaTrackParentRelations_)->default_value(GO_DEF_EATRACKPARENTRELATIONS))
		("swarmNNeighborhoods", po::value<std::size_t>(&swarmNNeighborhoods_)->default_value(GO_DEF_SWARMNNEIGHBORHOODS))
		("swarmNNeighborhoodMembers", po::value<std::size_t>(&swarmNNeighborhoodMembers_)->default_value(GO_DEF_SWARMNNEIGHBORHOODMEMBERS))
		("swarmRandomFillUp", po::value<bool>(&swarmRandomFillUp_)->default_value(GO_DEF_SWARMRANDOMFILLUP))
		("swarmCLocal", po::value<float>(&swarmCLocal_)->default_value(GO_DEF_SWARMCLOCAL))
		("swarmCGlobal", po::value<float>(&swarmCGlobal_)->default_value(GO_DEF_SWARMCCGLOBAL))
		("swarmCDelta", po::value<float>(&swarmCDelta_)->default_value(GO_DEF_SWARMCCDELTA))
		("swarmUpdateRule", po::value<updateRule>(&swarmUpdateRule_)->default_value(GO_DEF_SWARMUPDATERULE))
		("gdNStartingPoints", po::value<std::size_t>(&gdNStartingPoints_)->default_value(GO_DEF_GDNSTARTINGPOINTS))
		("gdFiniteStep", po::value<float>(&gdFiniteStep_)->default_value(GO_DEF_GDFINITESTEP))
		("gdStepSize", po::value<float>(&gdStepSize_)->default_value(GO_DEF_GDSTEPSIZE))
		;

		po::variables_map vm;
		std::ifstream ifs(configFile.c_str());
		if (!ifs.good()) {
			std::cerr << "Error accessing configuration file " << configFile;
			exit(1);
		}

		po::store(po::parse_config_file(ifs, config), vm);
		po::notify(vm);

		ifs.close();

		if(verbose_) { // Let the audience know
			std::cout << "Found the following values in configuration file:" << configFile << ":" << std::endl
					  << "maxStalledDataTransfers = " << maxStalledDataTransfers_ << std::endl
					  << "maxConnectionAttempts = " << maxConnectionAttempts_ << std::endl
					  << "copyBestOnly = " << copyBestOnly_ << std::endl
					  << "returnRegardless = " << returnRegardless_ << std::endl
					  << "nProducerThreads = " << nProducerThreads_ << std::endl
					  << "arraySize = " << arraySize_ << std::endl
					  << "nEvaluationThreads = " << nEvaluationThreads_ << std::endl
					  << "serializationMode = " << serializationMode_ << std::endl
					  << "waitFactor = " << waitFactor_ << std::endl
					  << "maxIterations = " << maxIterations_ << std::endl
					  << "maxMinutes = " << maxMinutes_ << std::endl
					  << "reportIteration = " << reportIteration_ << std::endl
					  << "eaPopulationSize = " << eaPopulationSize_ << std::endl
					  << "eaNParents = " << eaNParents_ << std::endl
					  << "eaRecombinationScheme = " << eaRecombinationScheme_ << std::endl
					  << "eaSortingScheme = " << eaSortingScheme_ << std::endl
					  << "eaTrackParentRelations = " << eaTrackParentRelations_ << std::endl
					  << "swarmNNeighborhoods = " << swarmNNeighborhoods_ << std::endl
					  << "swarmNNeighborhoodMembers = " << swarmNNeighborhoodMembers_ << std::endl
					  << "swarmRandomFillUp = " << swarmRandomFillUp_ << std::endl
					  << "swarmCLocal = " << swarmCLocal_ << std::endl
					  << "swarmCGlobal = " << swarmCGlobal_ << std::endl
					  << "swarmCDelta = " << swarmCDelta_ << std::endl
					  << "swarmUpdateRule = " << swarmUpdateRule_ << std::endl
					  << "gdNStartingPoints = " << gdNStartingPoints_ << std::endl
					  << "gdFiniteStep = " << gdFiniteStep_ << std::endl
					  << "gdStepSize = " << gdStepSize_ << std::endl
			;
		}
	}
	catch(const po::error& e) {
		std::cerr << "Error parsing the configuration file " << configFile << ":" << std::endl
				  << e.what() << std::endl;
		exit(1);
	}
	catch(...) {
		std::cerr << "Unknown error while parsing the configuration file " << configFile << std::endl;
		exit(1);
	}
}

/**************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
