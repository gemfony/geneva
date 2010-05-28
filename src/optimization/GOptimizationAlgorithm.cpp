/**
 * @file GOptimizationAlgorithm.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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

#include "GOptimizationAlgorithm.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
// #include <boost/serialization/export.hpp>
// BOOST_CLASS_EXPORT(Gem::GenEvA::GOptimizationAlgorithm)

namespace Gem
{
namespace GenEvA
{

/************************************************************************************************************/
/**
 * The default constructor
 */
GOptimizationAlgorithm::GOptimizationAlgorithm()
	:GMutableSetT<Gem::GenEvA::GIndividual>()
	 , gr(Gem::Util::DEFAULTRNRGENMODE)
	 , iteration_(0)
	 , maxIteration_(DEFAULTMAXIT)
	 , maxStallIteration_(DEFAULMAXTSTALLIT)
	 , reportIteration_(DEFAULTREPORTITER)
	 , defaultPopulationSize_(0)
	 , bestPastFitness_(0.) // will be set appropriately in the optimize() function
	 , maximize_(DEFAULTMAXMODE)
	 , stallCounter_(0)
	 , cpInterval_(DEFAULTCHECKPOINTIT)
	 , cpBaseName_(DEFAULTCPBASENAME)
	 , cpDirectory_(DEFAULTCPDIR)
	 , cpSerMode_(DEFAULTCPSERMODE)
	 , qualityThreshold_(DEFAULTQUALITYTHRESHOLD)
	 , hasQualityThreshold_(false)
	 , maxDuration_(boost::posix_time::duration_from_string(DEFAULTDURATION))
	 , emitTerminationReason_(false)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A constant reference to another GOptimizationAlgorithm object
 */
GOptimizationAlgorithm::GOptimizationAlgorithm(const GOptimizationAlgorithm& cp)
	:GMutableSetT<Gem::GenEvA::GIndividual>(cp)
	 , gr(cp.gr)
	 , iteration_(cp.iteration_)
	 , maxIteration_(cp.maxIteration_)
	 , maxStallIteration_(cp.maxStallIteration_)
	 , reportIteration_(cp.reportIteration_)
	 , defaultPopulationSize_(cp.defaultPopulationSize_)
	 , bestPastFitness_(cp.bestPastFitness_)
	 , maximize_(cp.maximize_)
	 , stallCounter_(cp.stallCounter_)
	 , cpInterval_(cp.cpInterval_)
	 , cpBaseName_(cp.cpBaseName_)
	 , cpDirectory_(cp.cpDirectory_)
	 , cpSerMode_(cp.cpSerMode_)
	 , qualityThreshold_(cp.qualityThreshold_)
	 , hasQualityThreshold_(cp.hasQualityThreshold_)
	 , maxDuration_(cp.maxDuration_)
	 , emitTerminationReason_(cp.emitTerminationReason_)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The destructor
 */
GOptimizationAlgorithm::~GOptimizationAlgorithm()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * Checks for equality with another GOptimizationAlgorithm object
 *
 * @param  cp A constant reference to another GOptimizationAlgorithm object
 * @return A boolean indicating whether both objects are equal
 */
bool GOptimizationAlgorithm::operator==(const GOptimizationAlgorithm& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GOptimizationAlgorithm::operator==","cp", CE_SILENT);
}

/************************************************************************************************************/
/**
 * Checks for inequality with another GOptimizationAlgorithm object
 *
 * @param  cp A constant reference to another GOptimizationAlgorithm object
 * @return A boolean indicating whether both objects are inequal
 */
bool GOptimizationAlgorithm::operator!=(const GOptimizationAlgorithm& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GOptimizationAlgorithm::operator!=","cp", CE_SILENT);
}


/************************************************************************************************************/
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
boost::optional<std::string> GOptimizationAlgorithm::checkRelationshipWith(const GObject& cp,
		const Gem::Util::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Util;
    using namespace Gem::Util::POD;

	// Check that we are indeed dealing with a GParamterBase reference
	const GOptimizationAlgorithm *p_load = GObject::conversion_cast<GOptimizationAlgorithm>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GMutableSetT<Gem::GenEvA::GIndividual>::checkRelationshipWith(cp, e, limit, "GOptimizationAlgorithm", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GOptimizationAlgorithm", iteration_, p_load->iteration_, "iteration_", "p_load->iteration_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizationAlgorithm", maxIteration_, p_load->maxIteration_, "maxIteration_", "p_load->maxIteration_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizationAlgorithm", maxStallIteration_, p_load->maxStallIteration_, "maxStallIteration_", "p_load->maxStallIteration_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizationAlgorithm", reportIteration_, p_load->reportIteration_, "reportIteration_", "p_load->reportIteration_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizationAlgorithm", defaultPopulationSize_, p_load->defaultPopulationSize_, "defaultPopulationSize_", "p_load->defaultPopulationSize_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizationAlgorithm", bestPastFitness_, p_load->bestPastFitness_, "bestPastFitness_", "p_load->bestPastFitness_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizationAlgorithm", maximize_, p_load->maximize_, "maximize_", "p_load->maximize_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizationAlgorithm", stallCounter_, p_load->stallCounter_, "stallCounter_", "p_load->stallCounter_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizationAlgorithm", cpInterval_, p_load->cpInterval_, "cpInterval_", "p_load->cpInterval_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizationAlgorithm", cpBaseName_, p_load->cpBaseName_, "cpBaseName_", "p_load->cpBaseName_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizationAlgorithm", cpDirectory_, p_load->cpDirectory_, "cpDirectory_", "p_load->cpDirectory_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizationAlgorithm", cpSerMode_, p_load->cpSerMode_, "cpSerMode_", "p_load->cpSerMode_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizationAlgorithm", qualityThreshold_, p_load->qualityThreshold_, "qualityThreshold_", "p_load->qualityThreshold_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizationAlgorithm", hasQualityThreshold_, p_load->hasQualityThreshold_, "hasQualityThreshold_", "p_load->hasQualityThreshold_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizationAlgorithm", maxDuration_, p_load->maxDuration_, "maxDuration_", "p_load->maxDuration_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GOptimizationAlgorithm", emitTerminationReason_, p_load->emitTerminationReason_, "emitTerminationReason_", "p_load->emitTerminationReason_", e , limit));

	return evaluateDiscrepancies("GOptimizationAlgorithm", caller, deviations, e);
}

/************************************************************************************************************/
/**
 * Determines whether production of random numbers should happen remotely
 * (RNRFACTORY) or locally (RNRLOCAL)
 *
 * @param rnrGenMode A parameter which indicates where random numbers should be produced
 */
void GOptimizationAlgorithm::setRnrGenerationMode(const Gem::Util::rnrGenerationMode& rnrGenMode) {
	gr.setRnrGenerationMode(rnrGenMode);
}

/************************************************************************************************************/
/**
 * Retrieves the random number generators current generation mode.
 *
 * @return The current random number generation mode of the local generator
 */
Gem::Util::rnrGenerationMode GOptimizationAlgorithm::getRnrGenerationMode() const {
	return gr.getRnrGenerationMode();
}

/************************************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp Another GOptimizationAlgorithm object, camouflaged as a GObject
 */
void GOptimizationAlgorithm::load_(const GObject* cp)
{
	const GOptimizationAlgorithm *p_load = GObject::conversion_cast<GOptimizationAlgorithm>(cp);

	// Load the parent class'es data
	GMutableSetT<Gem::GenEvA::GIndividual>::load_(cp);

	// and then our local data
	gr.load(p_load->gr);

	iteration_ = p_load->iteration_;
	maxIteration_ = p_load->maxIteration_;
	maxStallIteration_ = p_load->maxStallIteration_;
	reportIteration_ = p_load->reportIteration_;
	defaultPopulationSize_ = p_load->defaultPopulationSize_;
	bestPastFitness_ = p_load->bestPastFitness_;
	maximize_ = p_load->maximize_;
	stallCounter_ = p_load->stallCounter_;
	cpInterval_ = p_load->cpInterval_;
	cpBaseName_ = p_load->cpBaseName_;
	cpDirectory_ = p_load->cpDirectory_;
	cpSerMode_ = p_load->cpSerMode_;
	qualityThreshold_ = p_load->qualityThreshold_;
	hasQualityThreshold_ = p_load->hasQualityThreshold_;
	maxDuration_ = p_load->maxDuration_;
	emitTerminationReason_ = p_load->emitTerminationReason_;
}

/************************************************************************************************************/
/**
 * This function encapsulates some common functionality of iteration-based
 * optimization algorithms. E.g., they all need a a loop that stops if some
 * predefined criterion is reached. The function is also the main entry
 * point for all optimization algorithms.
 *
 * @param startIteration Specifies the iteration number to start with (e.g. useful when starting from a checkpoint file)
 */
void GOptimizationAlgorithm::optimize(const boost::uint32_t& startIteration) {
	// Reset the generation counter
	iteration_ = startIteration;

	// Resize the population to the desired size and do some error checks
	adjustPopulation();

	// Set the individual's personalities (some algorithm-specific information needs to be stored
	// in individuals. Optimization algorithms need to re-implement this function to add
	// the required functionality.)
	setIndividualPersonalities();

	// Let individuals know whether they are part of a maximization or minimization scheme
	setIndividualMaxMode();

	// Emit the info header, unless we do not want any info (parameter 0).
	// Note that this call needs to come after the initialization, so we have the
	// complete set of individuals available.
	if(reportIteration_) doInfo(INFOINIT);

	// We want to know if no better values were found for a longer period of time
	bestPastFitness_ = getWorstCase();
	stallCounter_ = 0;

	// Give derived classes the opportunity to perform any necessary preparatory work.
	init();

	// Initialize the start time with the current time.
	startTime_ = boost::posix_time::second_clock::local_time(); /// Hmmm - not necessarily thread-safe, if each population runs in its own thread ...

	do {
		// Let all individuals know the current iteration
		markIteration();

		// Check whether a better value was found, and do the check-pointing, if necessary.
		// Uses the output of the function that contains the actual business logic of a
		// given optimization algorithm.
		checkpoint(ifProgress(cycleLogic()));

		// Let all individuals know about the best fitness known so far
		markBestFitness();

		// Let all individuals know about the number of failed optimization attempts in a row
		markNStalls();

		// We want to provide feedback to the user in regular intervals.
		// Set the reportGeneration_ variable to 0 in order not to emit
		// any information at all.
		if(reportIteration_ && (iteration_%reportIteration_ == 0)) doInfo(INFOPROCESSING);

		// update the iteration_ counter
		iteration_++;
	}
	while(!halt(startIteration));

	// Give derived classes the opportunity to perform any remaining clean-up work
	finalize();

	// Finalize the info output
	if(reportIteration_) doInfo(INFOEND);

	// Remove information particular to evolutionary algorithms from the individuals
	resetIndividualPersonalities();
}

/************************************************************************************************************/
/**
 * Emits information specific to this class. The function can be overloaded
 * in derived classes and it indeed makes sense to emit much more information
 * than is done in this simple implementation.
 *
 * @param im The information mode (INFOINIT, INFOPROCESSING or INFOEND)
 */
void GOptimizationAlgorithm::doInfo(const infoMode& im) {
	switch(im) {
	case INFOINIT:
		std::cout << "Starting optimization cycle" << std::endl;
		break;
	case INFOPROCESSING:
		std::cout << getIteration() << ": " << getBestFitness() << std::endl;
		break;
	case INFOEND:
		std::cout << "Optimization cycle terminated" << std::endl;
		break;
	}
}

/************************************************************************************************************/
/**
 * Sets the nominal size of the population
 *
 * @param popSize The desired size of the population
 */
void GOptimizationAlgorithm::setPopulationSize(const std::size_t& popSize) {
	defaultPopulationSize_ = popSize;
}

/************************************************************************************************************/
/**
 * Retrieves the default population size
 *
 * @return The default population size
 */
std::size_t GOptimizationAlgorithm::getDefaultPopulationSize() const {
	return defaultPopulationSize_;
}

/************************************************************************************************************/
/**
 * Retrieve the current population size
 *
 * @return The current population size
 */
std::size_t GOptimizationAlgorithm::getPopulationSize() const {
	return data.size();
}

/************************************************************************************************************/
/**
 * Lets individuals know whether they are part of a maximization or minimization scheme
 */
void GOptimizationAlgorithm::setIndividualMaxMode() {
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it) (*it)->setMaxMode(maximize_);
}

/************************************************************************************************************/
/**
 * Lets individuals know about the current iteration of the optimization
 * cycle.
 */
void GOptimizationAlgorithm::markIteration() {
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it) (*it)->setParentAlgIteration(iteration_);
}

/************************************************************************************************************/
/**
 * Marks the globally best known fitness in all individuals
 */
void GOptimizationAlgorithm::markBestFitness() {
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it) (*it)->setBestKnownFitness(bestPastFitness_);
}

/************************************************************************************************************/
/**
 * Marks the number of stalled optimization attempts in all individuals
 */
void GOptimizationAlgorithm::markNStalls() {
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it) (*it)->setNStalls(stallCounter_);
}

/************************************************************************************************************/
/**
 * Set the number of iterations after which the optimization should
 * be stopped
 *
 * @param maxIteration The number of iterations after which the optimization should terminate
 */
void GOptimizationAlgorithm::setMaxIteration(const boost::uint32_t& maxIteration) {
	maxIteration_ = maxIteration;
}

/************************************************************************************************************/
/**
 * Retrieve the number of iterations after which optimization should
 * be stopped
 *
 * @return The number of iterations after which the optimization should terminate
 */
boost::uint32_t GOptimizationAlgorithm::getMaxIteration() const {
	return maxIteration_;
}

/************************************************************************************************************/
/**
 * Sets the maximum allowed processing time
 *
 * @param maxDuration The maximum allowed processing time
 */
void GOptimizationAlgorithm::setMaxTime(const boost::posix_time::time_duration& maxDuration) {
	using namespace boost::posix_time;

	// Only allow "real" values
	if(maxDuration.is_special() || maxDuration.is_negative()) {
		std::ostringstream error;
		error << "In GOptimizationAlgorithm::setMaxTime() : Error!" << std::endl
			  << "Invalid maxDuration." << std::endl;

		throw Gem::Common::gemfony_error_condition(error.str());
	}

	maxDuration_ = maxDuration;
}

/************************************************************************************************************/
/**
 * Retrieves the value of the maxDuration_ parameter.
 *
 * @return The maximum allowed processing time
 */
boost::posix_time::time_duration GOptimizationAlgorithm::getMaxTime() const {
	return maxDuration_;
}

/************************************************************************************************************/
/**
 * Retrieve the current iteration of the optimization run
 *
 * @return The current iteration of the optimization run
 */
boost::uint32_t GOptimizationAlgorithm::getIteration() const {
	return iteration_;
}

/************************************************************************************************************/
/**
 * Sets the number of iterations after which the algorithm should
 * report about its inner state.
 *
 * @param iter The number of iterations after which information should be emitted
 */
void GOptimizationAlgorithm::setReportIteration(const boost::uint32_t& iter) {
	reportIteration_ = iter;
}

/************************************************************************************************************/
/**
 * Returns the number of iterations after which the algorithm should
 * report about its inner state.
 *
 * @return The number of iterations after which information is emitted
 */
boost::uint32_t GOptimizationAlgorithm::getReportIteration() const {
	return reportIteration_;
}

/************************************************************************************************************/
/**
 * Retrieves the current number of failed optimization attempts
 *
 * @return The current number of failed optimization attempts
 */
boost::uint32_t GOptimizationAlgorithm::getStallCounter() const {
	return stallCounter_;
}

/************************************************************************************************************/
/**
 * Retrieve the current best value found
 *
 * @return The best fitness found so far
 */
double GOptimizationAlgorithm::getBestFitness() const {
	return bestPastFitness_;
}

/************************************************************************************************************/
/**
 * Specify whether we want to work in maximization or minimization mode
 *
 * @param maximize A boolean which indicates whether we should work in maximization or minimization mode
 */
void GOptimizationAlgorithm::setMaximize(const bool& maximize) {
	maximize_ = maximize;
}

/************************************************************************************************************/
/**
 * Find out whether we work in maximization or minimization mode
 *
 * @return A boolean which indicates whether we are working in maximization or minimization mode
 */
bool GOptimizationAlgorithm::getMaximize() const {
	return maximize_;
}

/************************************************************************************************************/
/**
 * Checks whether a better solution was found and updates the stallCounter_ variable
 * as necessary.
 *
 * @param bestEval The best known evaluation of the current iteration
 * @return A boolean indicating whether a better solution was found
 */
bool GOptimizationAlgorithm::ifProgress(const double& bestEval) {
	// Check whether an improvement has been achieved
	bool better = isBetter(bestEval, bestPastFitness_);
	if(better) {
		bestPastFitness_ = bestEval;
		stallCounter_ = 0;
	}
	else {
		stallCounter_++;
	}

	return better;
}

/************************************************************************************************************/
/**
 * A helper function that helps to determine whether a given value is better than
 * a given older one. As "better" means something different for maximization and minimization,
 * this function helps to make the code easier to understand.
 *
 * @param newValue The new value
 * @param oldValue The old value
 * @return true of newValue is better than oldValue, otherwise false.
 */
bool GOptimizationAlgorithm::isBetter(double newValue, const double& oldValue) const {
	if(maximize_) {
		if(newValue > oldValue) return true;
		else return false;
	}
	else { // minimization
		if(newValue < oldValue) return true;
		else return false;
	}
}

/************************************************************************************************************/
/**
 * Helper function that emits the worst case value depending on whether maximization or minimization is performed.
 *
 * @return The worst case value, depending on maximization or minimization
 */
double GOptimizationAlgorithm::getWorstCase() const {
	return (maximize_?-DBL_MAX:DBL_MAX);
}

/************************************************************************************************************/
/**
 * Performs the necessary administratory work of doing check-pointing
 *
 * @param better A boolean which indicates whether a better result was found
 */
void GOptimizationAlgorithm::checkpoint(const bool& better) const {
	// Save checkpoints if required by the user
	if(cpInterval_ == -1 && better) saveCheckpoint();
	else if(cpInterval_ && iteration_%cpInterval_ == 0) saveCheckpoint();
}

/************************************************************************************************************/
/**
 * Allows to set the number of generations after which a checkpoint should be written
 *
 * @param cpInterval The number of generations after which a checkpoint should be written
 */
void GOptimizationAlgorithm::setCheckpointInterval(const boost::int32_t& cpInterval) {
	if(cpInterval < -1) {
		std::ostringstream error;
		error << "In GOptimizationAlgorithm::setCheckpointInterval():" << std::endl
			  << "Error: received bad checkpoint interval: " << cpInterval << std::endl;
		throw Gem::Common::gemfony_error_condition(error.str());
	}

	cpInterval_ = cpInterval;
}

/************************************************************************************************************/
/**
 * Allows to retrieve the number of generations after which a checkpoint should be written
 *
 * @return The number of generations after which a checkpoint should be written
 */
boost::uint32_t GOptimizationAlgorithm::getCheckpointInterval() const {
	return cpInterval_;
}

/************************************************************************************************************/
/**
 * Allows to set the base name of the checkpoint file and the directory where it
 * should be stored.
 *
 * @param cpDirectory The directory where checkpoint files should be stored
 * @param cpBaseName The base name used for the checkpoint files
 */
void GOptimizationAlgorithm::setCheckpointBaseName(const std::string& cpDirectory, const std::string& cpBaseName) {
	// Do some basic checks
	if(cpBaseName == "empty" || cpBaseName.empty()) {
		std::ostringstream error;
		error << "In GOptimizationAlgorithm::setCheckpointBaseName(const std::string&, const std::string&):" << std::endl
			  << "Error: Invalid cpBaseName: " << cpBaseName << std::endl;
		throw Gem::Common::gemfony_error_condition(error.str());
	}

	if(cpDirectory == "empty" || cpDirectory.empty()) {
		std::ostringstream error;
		error << "In GOptimizationAlgorithm::setCheckpointBaseName(const std::string&, const std::string&):" << std::endl
			  << "Error: Invalid cpDirectory: " << cpDirectory << std::endl;
		throw Gem::Common::gemfony_error_condition(error.str());
	}

	cpBaseName_ = cpBaseName;

	// Check that the provided directory exists
	if(!boost::filesystem::exists(cpDirectory) || !boost::filesystem::is_directory(cpDirectory)) {
		std::ostringstream error;
		error << "In GOptimizationAlgorithm::setCheckpointBaseName(const std::string&, const std::string&):" << std::endl
			  << "Error: directory does not exist: " << cpDirectory << std::endl;
		throw Gem::Common::gemfony_error_condition(error.str());
	}

	// Add a trailing slash to the directory name, if necessary
	// TODO: THIS IS NOT PORTABLE TO WINDOWS!
    if(cpDirectory[cpDirectory.size() - 1] != '/') cpDirectory_ = cpDirectory + '/';
    else cpDirectory_ = cpDirectory;
}

/************************************************************************************************************/
/**
 * Allows to retrieve the base name of the checkpoint file.
 *
 * @return The base name used for checkpoint files
 */
std::string GOptimizationAlgorithm::getCheckpointBaseName() const {
	return cpBaseName_;
}

/************************************************************************************************************/
/**
 * Allows to retrieve the directory where checkpoint files should be stored
 *
 * @return The base name used for checkpoint files
 */
std::string GOptimizationAlgorithm::getCheckpointDirectory() const {
	return cpDirectory_;
}

/************************************************************************************************************/
/**
 * Determines whether checkpointing should be done in Text-, XML- or Binary-mode
 *
 * @param cpSerMode The desired new checkpointing serialization mode
 */
void GOptimizationAlgorithm::setCheckpointSerializationMode(const serializationMode& cpSerMode) {
	cpSerMode_ = cpSerMode;
}

/************************************************************************************************************/
/**
 * Retrieves the current checkpointing serialization mode
 *
 * @return The current checkpointing serialization mode
 */
serializationMode GOptimizationAlgorithm::getCheckpointSerializationMode() const {
	return cpSerMode_;
}

/************************************************************************************************************/
/**
 * Sets the maximum number of generations allowed without improvement of the best
 * individual. Set to 0 in order for this stop criterion to be disabled.
 *
 * @param The maximum number of allowed generations
 */
void GOptimizationAlgorithm::setMaxStallIteration(const boost::uint32_t& maxStallIteration) {
	maxStallIteration_ = maxStallIteration;
}

/************************************************************************************************************/
/**
 * Retrieves the maximum number of generations allowed in an optimization run without
 * improvement of the best individual.
 *
 * @return The maximum number of generations
 */
boost::uint32_t GOptimizationAlgorithm::getMaxStallIteration() const {
	return maxStallIteration_;
}

/************************************************************************************************************/
/**
 *  Sets a quality threshold beyond which optimization is expected to stop
 *
 *  @param qualityThreshold A threshold beyond which optimization should stop
 */
void GOptimizationAlgorithm::setQualityThreshold(const double& qualityThreshold) {
	qualityThreshold_ = qualityThreshold;
	hasQualityThreshold_=true;
}

/************************************************************************************************************/
/**
 * Retrieves the current value of the quality threshold and also indicates whether
 * the threshold is active
 *
 * @param hasQualityThreshold A boolean indicating whether a quality threshold has been set
 * @return The current value of the quality threshold
 */
double GOptimizationAlgorithm::getQualityThreshold(bool& hasQualityThreshold) const {
	hasQualityThreshold = hasQualityThreshold_;
	return qualityThreshold_;
}


/************************************************************************************************************/
/**
 * Removes the quality threshold
 */
void GOptimizationAlgorithm::unsetQualityThreshold() {
	hasQualityThreshold_ = false;
}

/************************************************************************************************************/
/**
 * Checks whether a quality threshold has been set
 *
 * @return A boolean indicating whether a quality threshold has been set
 */
bool GOptimizationAlgorithm::hasQualityThreshold() const {
	return hasQualityThreshold_;
}

/************************************************************************************************************/
/**
 * It is possible for derived classes to specify in overloaded versions of this
 * function under which conditions the optimization should be stopped. The
 * function is called from GOptimizationAlgorithm::halt .
 *
 * @return boolean indicating that a stop condition was reached
 */
bool GOptimizationAlgorithm::customHalt() const {
	/* nothing - specify your own criteria in derived classes. Make sure
	 * to emit a suitable mesage if execution was halted due to a
	 * custom criterium */
	return false;
}

/************************************************************************************************************/
/**
 * This function returns true once a given time (set with GOptimizationAlgorithm::setMaxTime())
 * has passed. It is used in the GOptimizationAlgorithm::halt() function.
 *
 * @return A boolean indicating whether a given amount of time has passed
 */
bool GOptimizationAlgorithm::timedHalt() const {
	using namespace boost::posix_time;
	ptime currentTime = microsec_clock::local_time();
	if((currentTime - startTime_) >= maxDuration_) {
		if(emitTerminationReason_)
			std::cerr << "Terminating optimization run because maximum time frame has been exceeded" << std::endl;
		return true;
	}
	return false;
}

/************************************************************************************************************/
/**
 * This function returns true once the quality is below or above a given threshold
 * (depending on whether we maximize or minimize).
 *
 * @return A boolean indicating whether the quality is above or below a given threshold
 */
bool GOptimizationAlgorithm::qualityHalt() const {
	if(isBetter(bestPastFitness_, qualityThreshold_)) {
		if(emitTerminationReason_)
			std::cerr << "Terminating optimization run because quality threshold has been reached" << std::endl;
		return true;
	}
	else return false;
}

/************************************************************************************************************/
/**
 * This function checks whether a halt criterion has been reached. The most
 * common criterion is the maximum number of iterations. Set the maxIteration_
 * counter to 0 if you want to disable this criterion. If the optimization is
 * supposed to start with a higher value of the iteration counter, e.g. because
 * a checkpoint file has been loaded, then an offset can be added to the
 * iteration counter.
 *
 * @param iterationOffset An offset to be added to the maximum iteration
 * @return A boolean indicating whether a halt criterion has been reached
 */
bool GOptimizationAlgorithm::halt(const boost::uint32_t& iterationOffset) const
{
	// Have we exceeded the maximum number of iterations and
	// do we indeed intend to stop in this case ?
	if(maxIteration_ && (iteration_ > (maxIteration_ + iterationOffset))) {
		if(emitTerminationReason_)
			std::cout << "Terminating optimization run because iteration threshold has been reached" << std::endl;
		return true;
	}

	// Has the optimization stalled too often ?
	if(maxStallIteration_ && stallCounter_ > maxStallIteration_) {
		if(emitTerminationReason_)
			std::cout << "Terminating optimization run because maximum number of stalls has been exceeded" << std::endl;
		return true;
	}

	// Do we have a scheduled halt time ? The comparatively expensive
	// timedHalt() calculation is only called if maxDuration_
	// is at least one microsecond.
	if(maxDuration_.total_microseconds() && timedHalt()) return true;

	// Are we supposed to stop when the quality has exceeded a threshold ?
	if(hasQualityThreshold_ && qualityHalt()) return true;

	// Has the user specified an additional stop criterion ?
	if(customHalt()) return true;

	// Fine, we can continue.
	return false;
}

/************************************************************************************************************/
/**
 * Resets the individual's personality types
 */
void GOptimizationAlgorithm::resetIndividualPersonalities() {
	GOptimizationAlgorithm::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) (*it)->resetPersonality();
}

/************************************************************************************************************/
/**
 * Possible adaptions of a population could involve shifting of individuals.
 * By default, no adaptions are defined.
 */
void GOptimizationAlgorithm::customAdaptions()
{ /* nothing */}

/************************************************************************************************************/
/**
 * Fitness calculation for a population means optimization. The fitness is then determined
 * by the best individual which, after the end of the optimization cycle, can be found in
 * the first position of the array. This is true both for all sorting modes.
 *
 * @return The fitness of the best individual in the population
 */
double GOptimizationAlgorithm::fitnessCalculation() {
	bool dirty = false;

	this->optimize();

	double val = data.at(0)->getCurrentFitness(dirty);
	// is this the current fitness ? We should at this stage never
	// run across an unevaluated individual.
	if(dirty) {
		std::ostringstream error;
		error << "In GOptimizationAlgorithm::fitnessCalculation(): Error!" << std::endl
			  << "Came across dirty individual" << std::endl;

		// throw an exception. Add some information so that if the exception
		// is caught through a base object, no information is lost.
		throw Gem::Common::gemfony_error_condition(error.str());
	}
	return val;
}

/************************************************************************************************************/
/**
 * Allows derived classes to reset the stall counter. This is e.g. used in the
 * evolutionary algorithm's micro training environment.
 */
void GOptimizationAlgorithm::resetStallCounter() {
	stallCounter_ = 0;
}

/************************************************************************************************************/
/**
 * Allows derived classes to perform initialization work before the optimization
 * cycle starts
 */
void GOptimizationAlgorithm::init()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * Allows derived classes to perform any remaining work after the
 * optimization cycle has finished
 */
void GOptimizationAlgorithm::finalize()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * Specifies whether information about termination reasons should be emitted
 *
 * @param etr A boolean which specifies whether reasons for the termination of the optimization run should be emitted
 */
void GOptimizationAlgorithm::setEmitTerminationReason(const bool& emitTerminatioReason) {
	emitTerminationReason_ = emitTerminatioReason;
}

/************************************************************************************************************/
/**
 * Retrieves information on whether information about termination reasons should be emitted
 *
 * @return A boolean which specifies whether reasons for the termination of the optimization run will be emitted
 */
bool GOptimizationAlgorithm::getEmitTerminationReason() const {
	return emitTerminationReason_;
}

/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GOptimizationAlgorithm::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GMutableSetT<Gem::GenEvA::GIndividual>::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GOptimizationAlgorithm::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GMutableSetT<Gem::GenEvA::GIndividual>::specificTestsNoFailureExpected_GUnitTests();
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GOptimizationAlgorithm::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GMutableSetT<Gem::GenEvA::GIndividual>::specificTestsFailuresExpected_GUnitTests();
}

/************************************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
