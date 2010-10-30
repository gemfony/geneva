/**
 * @file GOptimizer.cpp
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

#include "geneva/GOptimizer.hpp"

namespace Gem {
namespace Geneva {

/**************************************************************************************/
/**
 * The default constructor
 */
GOptimizer::GOptimizer()
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
 */
GOptimizer::GOptimizer(int argc, char **argv)
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
GOptimizer::GOptimizer(const GOptimizer& cp)
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
GOptimizer::~GOptimizer()
{ /* nothing */ }

/**************************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another GOptimizer object
 * @return A constant reference to this object
 */
const GOptimizer& GOptimizer::operator=(const GOptimizer& cp) {
	GOptimizer::load_(&cp);
	return *this;
}

/**************************************************************************************/
/**
 * Checks for equality with another GOptimizer object
 *
 * @param  cp A constant reference to another GOptimizer object
 * @return A boolean indicating whether both objects are equal
 */
bool GOptimizer::operator==(const GOptimizer& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GOptimizer::operator==","cp", CE_SILENT);
}

/**************************************************************************************/
/**
 * Checks for inequality with another GOptimizer object
 *
 * @param  cp A constant reference to another GOptimizer object
 * @return A boolean indicating whether both objects are inequal
 */
bool GOptimizer::operator!=(const GOptimizer& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GOptimizer::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GOptimizer::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const {
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GOptimizationMonitorT reference
	const GOptimizer *p_load = GObject::conversion_cast<GOptimizer>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GMutableSetT<GParameterSet>::checkRelationshipWith(cp, e, limit, "GOptimizer", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", pers_, p_load->pers_, "pers_", "p_load->pers_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", parMode_, p_load->parMode_, "parMode_", "p_load->parMode_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", serverMode_, p_load->serverMode_, "serverMode_", "p_load->serverMode_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", serializationMode_, p_load->serializationMode_, "serializationMode_", "p_load->serializationMode_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", ip_, p_load->ip_, "ip_", "p_load->ip_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", port_, p_load->port_, "port_", "p_load->port_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", configFilename_, p_load->configFilename_, "configFilename_", "p_load->configFilename_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", verbose_, p_load->verbose_, "verbose_", "p_load->verbose_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", ea_om_ptr_, p_load->ea_om_ptr_, "ea_om_ptr_", "p_load->ea_om_ptr_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", swarm_om_ptr_, p_load->swarm_om_ptr_, "swarm_om_ptr_", "p_load->swarm_om_ptr_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", gd_om_ptr_, p_load->gd_om_ptr_, "gd_om_ptr_", "p_load->gd_om_ptr_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", copyBestOnly_, p_load->copyBestOnly_, "copyBestOnly_", "p_load->copyBestOnly_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", maxStalledDataTransfers_, p_load->maxStalledDataTransfers_, "maxStalledDataTransfers_", "p_load->maxStalledDataTransfers_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", maxConnectionAttempts_, p_load->maxConnectionAttempts_, "maxConnectionAttempts_", "p_load->maxConnectionAttempts_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", returnRegardless_, p_load->returnRegardless_, "returnRegardless_", "p_load->returnRegardless_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", nProducerThreads_, p_load->nProducerThreads_, "nProducerThreads_", "p_load->nProducerThreads_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", arraySize_, p_load->arraySize_, "arraySize_", "p_load->arraySize_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", nEvaluationThreads_, p_load->nEvaluationThreads_, "nEvaluationThreads_", "p_load->nEvaluationThreads_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", waitFactor_, p_load->waitFactor_, "waitFactor_", "p_load->waitFactor_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", maxIterations_, p_load->maxIterations_, "maxIterations_", "p_load->maxIterations_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", maxMinutes_, p_load->maxMinutes_, "maxMinutes_", "p_load->maxMinutes_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", reportIteration_, p_load->reportIteration_, "reportIteration_", "p_load->reportIteration_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", eaPopulationSize_, p_load->eaPopulationSize_, "eaPopulationSize_", "p_load->eaPopulationSize_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", eaNParents_, p_load->eaNParents_, "eaNParents_", "p_load->eaNParents_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", eaRecombinationScheme_, p_load->eaRecombinationScheme_, "eaRecombinationScheme_", "p_load->eaRecombinationScheme_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", eaSortingScheme_, p_load->eaSortingScheme_, "eaSortingScheme_", "p_load->eaSortingScheme_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", eaTrackParentRelations_, p_load->eaTrackParentRelations_, "eaTrackParentRelations_", "p_load->eaTrackParentRelations_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", swarmNNeighborhoods_, p_load->swarmNNeighborhoods_, "swarmNNeighborhoods_", "p_load->swarmNNeighborhoods_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", swarmNNeighborhoodMembers_, p_load->swarmNNeighborhoodMembers_, "swarmNNeighborhoodMembers_", "p_load->swarmNNeighborhoodMembers_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", swarmRandomFillUp_, p_load->swarmRandomFillUp_, "swarmRandomFillUp_", "p_load->swarmRandomFillUp_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", swarmCLocal_, p_load->swarmCLocal_, "swarmCLocal_", "p_load->swarmCLocal_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", swarmCGlobal_, p_load->swarmCGlobal_, "swarmCGlobal_", "p_load->swarmCGlobal_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", swarmCDelta_, p_load->swarmCDelta_, "swarmCDelta_", "p_load->swarmCDelta_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", swarmUpdateRule_, p_load->swarmUpdateRule_, "swarmUpdateRule_", "p_load->swarmUpdateRule_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", gdNStartingPoints_, p_load->gdNStartingPoints_, "gdNStartingPoints_", "p_load->gdNStartingPoints_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", gdFiniteStep_, p_load->gdFiniteStep_, "gdFiniteStep_", "p_load->gdFiniteStep_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", gdStepSize_, p_load->gdStepSize_, "gdStepSize_", "p_load->gdStepSize_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizer", bestIndividual_, p_load->bestIndividual_, "bestIndividual_", "p_load->bestIndividual_", e , limit));

	return evaluateDiscrepancies("GOptimizationMonitorT", caller, deviations, e);
}

/**************************************************************************************/
/**
 * Loads the data of another GOptimzer object
 *
 * @param cp A copy of another GOptimizer object, camouflaged as a GObject
 */
void GOptimizer::load_(const GObject *cp) {
	const GOptimizer *p_load = conversion_cast<GOptimizer>(cp);

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
GObject *GOptimizer::clone_() const {
	return new GOptimizer(*this);
}



/**************************************************************************************/
/**
 * Allows to specify an optimization monitor to be used with evolutionary algorithms.
 * Note that this function will take ownership of the monitor by cloning it.
 *
 * @param ea_om_ptr A pointer to an optimization monitor specific for evolutionary algorithms
 */
void GOptimizer::registerOptimizationMonitor(boost::shared_ptr<GEvolutionaryAlgorithm::GEAOptimizationMonitor> ea_om_ptr) {
	if(!ea_om_ptr) {
		std::ostringstream error;
		error << "In GOptimizer::registerOptimizationMonitor(): Error!" << std::endl
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
void GOptimizer::registerOptimizationMonitor(boost::shared_ptr<GSwarm::GSwarmOptimizationMonitor> swarm_om_ptr) {
	if(!swarm_om_ptr) {
		std::ostringstream error;
		error << "In GOptimizer::registerOptimizationMonitor(): Error!" << std::endl
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
void GOptimizer::registerOptimizationMonitor(boost::shared_ptr<GGradientDescent::GGDOptimizationMonitor> gd_om_ptr) {
	if(!gd_om_ptr) {
		std::ostringstream error;
		error << "In GOptimizer::registerOptimizationMonitor(): Error!" << std::endl
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
bool GOptimizer::clientRun() {
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
bool GOptimizer::serverMode() const {
	return serverMode_;
}

/**************************************************************************************/
/**
 * Checks whether this object is running in client mode
 *
 * @return A boolean which indicates whether the client mode has been set for this object
 */
bool GOptimizer::clientMode() const {
	return !serverMode_;
}

/**************************************************************************************/
/*
 * Specifies whether only the best individuals of a population should be copied.
 *
 * @param copyBestOnly Specifies whether only the best individuals of a population are copied
 */
void GOptimizer::setCopyBestIndividualsOnly(const bool& copyBestOnly) {
	copyBestOnly_ = copyBestOnly;
}

/**************************************************************************************/
/**
 * Checks whether only the best individuals are copied
 *
 * @return A boolean indicating whether only the best individuals of a population are copied
 */
bool GOptimizer::onlyBestIndividualsAreCopied() const {
	return copyBestOnly_;
}

/**************************************************************************************/
/**
 * Allows to randomly initialize parameter members. Note that for this wrapper object
 * this function doesn't make any sense. It is made available to satisfy a requirement
 * of GIndividual.
 */
void GOptimizer::randomInit()
{ /* nothing */ }

/**************************************************************************************/
/**
 * Fitness calculation for an optimization algorithm means optimization. The fitness is
 * then determined by the best individual which, after the end of the optimization cycle.
 *
 * @return The fitness of the best individual in the population
 */
double GOptimizer::fitnessCalculation() {
	bool dirty = false;

	boost::shared_ptr<GParameterSet> p = this->optimize<GParameterSet>();

	double val = p->getCurrentFitness(dirty);
	// is this the current fitness ? We should at this stage never
	// run across an unevaluated individual.
	if(dirty) {
		std::ostringstream error;
		error << "In GOptimizer::fitnessCalculation(): Error!" << std::endl
			  << "Came across dirty individual" << std::endl;

		// throw an exception. Add some information so that if the exception
		// is caught through a base object, no information is lost.
		throw Gem::Common::gemfony_error_condition(error.str());
	}

	return val;
}


/**************************************************************************************/
/**
 * Loads some configuration data from arguments passed on the command line
 *
 * @param argc The number of command line arguments
 * @param argv An array with the arguments
 */
void GOptimizer::parseCommandLine(int argc, char **argv) {
    namespace po = boost::program_options;

	try {
		std::string usageString = std::string("Usage: ") + argv[0] + " [options]";
		po::options_description desc(usageString.c_str());
		desc.add_options()
				("help,h", "emit help message")
				("configFilename,c", po::value<std::string>(&configFilename_)->default_value(GO_DEF_DEFAULTCONFIGFILE),
				"The name of the file holding configuration information for optimization algorithms")
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

		if(vm.count("writeConfigFile")) {
			GOptimizer::writeConfigurationFile(configFilename_);
			exit(0);
		}

		if (parMode_ == 2  &&  vm.count("serverMode")) serverMode_ = true;
		else serverMode_ = false;

		if(verbose_) {
			std::cout << std::endl
					<< "Running with the following command line options:" << std::endl
					<< "configFilename = " << configFilename_ << std::endl
					<< "parMode_ = " << parMode_ << std::endl
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
void GOptimizer::parseConfigurationFile(const std::string& configFile) {
	namespace bf = boost::filesystem;
    namespace po = boost::program_options;

	// Check the name of the configuration file
	if (!bf::exists(configFile)) {
		std::ostringstream error;
		error << "In GOptimizer::parseConfigurationFile(): Error!" << std::endl
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

		store(po::parse_config_file(ifs, config), vm);
		notify(vm);

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
