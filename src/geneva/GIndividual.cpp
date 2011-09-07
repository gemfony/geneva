/**
 * @file GIndividual.cpp
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

#include "geneva/GIndividual.hpp"

namespace Gem {
namespace Geneva {

/************************************************************************************************************/
/**
 * The default constructor.
 */
GIndividual::GIndividual()
	: GMutableI()
	, GRateableI()
	, GObject()
	, currentFitness_(0.)
    , currentSecondaryFitness_()
	, bestPastFitness_(0.)
	, bestPastSecondaryFitness_(0)
	, nStalls_(0)
	, dirtyFlag_(true)
	, serverMode_(false)
	, processingCycles_(1)
	, maximize_(false)
	, assignedIteration_(0)
	, pers_(PERSONALITY_NONE)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The standard copy constructor.
 *
 * @param cp A copy of another GIndividual object
 */
GIndividual::GIndividual(const GIndividual& cp)
	: GMutableI(cp)
	, GRateableI(cp)
	, GObject(cp)
	, currentFitness_(cp.currentFitness_)
	, currentSecondaryFitness_(cp.currentSecondaryFitness_)
	, bestPastFitness_(cp.bestPastFitness_)
	, bestPastSecondaryFitness_(cp.bestPastSecondaryFitness_)
	, nStalls_(cp.nStalls_)
	, dirtyFlag_(cp.dirtyFlag_)
	, serverMode_(cp.serverMode_)
	, processingCycles_(cp.processingCycles_)
	, maximize_(cp.maximize_)
	, assignedIteration_(cp.assignedIteration_)
	, pers_(cp.pers_)
{
	// We need to take care of the personality pointer manually
	setPersonality(pers_); // this call will also make sure that a suitable personality object is being created
	if(pers_ != PERSONALITY_NONE) pt_ptr_->GObject::load(cp.pt_ptr_);
}

/************************************************************************************************************/
/**
 * The standard destructor.
 */
GIndividual::~GIndividual() { /* nothing */ }

/************************************************************************************************************/
/**
 * Checks for equality with another GIndividual object
 *
 * @param  cp A constant reference to another GIndividual object
 * @return A boolean indicating whether both objects are equal
 */
bool GIndividual::operator==(const GIndividual& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GIndividual::operator==","cp", CE_SILENT);
}

/************************************************************************************************************/
/**
 * Checks for inequality with another GIndividual object
 *
 * @param  cp A constant reference to another GIndividual object
 * @return A boolean indicating whether both objects are inequal
 */
bool GIndividual::operator!=(const GIndividual& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GIndividual::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GIndividual::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const {
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GIndividual *p_load = GObject::gobject_conversion<GIndividual>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GObject::checkRelationshipWith(cp, e, limit, "GIndividual", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GIndividual", currentFitness_, p_load->currentFitness_, "currentFitness_", "p_load->currentFitness_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GIndividual", currentSecondaryFitness_, p_load->currentSecondaryFitness_, "currentSecondaryFitness_", "p_load->currentSecondaryFitness_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GIndividual", bestPastFitness_, p_load->bestPastFitness_, "bestPastFitness_", "p_load->bestPastFitness_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GIndividual", bestPastSecondaryFitness_, p_load->bestPastSecondaryFitness_, "bestPastSecondaryFitness_", "p_load->bestPastSecondaryFitness_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GIndividual", nStalls_, p_load->nStalls_, "nStalls_", "p_load->nStalls_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GIndividual", dirtyFlag_, p_load->dirtyFlag_, "dirtyFlag_", "p_load->dirtyFlag_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GIndividual", serverMode_, p_load->serverMode_, "serverMode_", "p_load->serverMode_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GIndividual", processingCycles_, p_load->processingCycles_, "processingCycles_", "p_load->processingCycles_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GIndividual", maximize_, p_load->maximize_, "maximize_", "p_load->maximize_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GIndividual", assignedIteration_, p_load->assignedIteration_, "assignedIteration_", "p_load->assignedIteration_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GIndividual", pers_, p_load->pers_, "pers_", "p_load->pers_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GIndividual", pt_ptr_, p_load->pt_ptr_, "pt_ptr_", "p_load->pt_ptr_", e , limit));

	return evaluateDiscrepancies("GIndividual", caller, deviations, e);
}

/************************************************************************************************************/
/**
 * Loads the data of another GIndividual, camouflaged as a GObject
 *
 * @param cp A copy of another GIndividual object, camouflaged as a GObject
 */
void GIndividual::load_(const GObject* cp) {
	const GIndividual *p_load = gobject_conversion<GIndividual>(cp);

	// Load the parent class'es data
	GObject::load_(cp);

	// Then load our local data
	currentFitness_ = p_load->currentFitness_;
	currentSecondaryFitness_ = p_load->currentSecondaryFitness_;
	bestPastFitness_ = p_load->bestPastFitness_;
	bestPastSecondaryFitness_ = p_load->bestPastSecondaryFitness_;
	nStalls_ = p_load->nStalls_;
	dirtyFlag_ = p_load->dirtyFlag_;
	serverMode_ = p_load->serverMode_;
	processingCycles_ = p_load->processingCycles_;
	maximize_ = p_load->maximize_;
	assignedIteration_ = p_load->assignedIteration_;
	setPersonality(p_load->pers_);
	if(pers_ != PERSONALITY_NONE) pt_ptr_->GObject::load(p_load->pt_ptr_);
}

/************************************************************************************************************/
/**
 * The adaption interface. Triggers adaption of the individual, using each parameter object's adaptor.
 * Sets the dirty flag, as the parameters have been changed.
 *
 * TODO: Check why immediate evaluation is done here (is this still the case ???)
 */
void GIndividual::adapt() {
	customAdaptions(); // The actual mutation and adaption process
	GIndividual::setDirtyFlag(true);
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Returns the last known fitness calculations of this object. Re-calculation
 * of the fitness is triggered, unless this is the server mode. By means of supplying
 * an id it is possible to distinguish between different target functions. 0 denotes
 * the main fitness criterion.
 *
 * @param id The id of the fitness criterion
 * @return The fitness of this individual
 */
double GIndividual::fitness(const std::size_t& id) {
	// Check whether we need to recalculate the fitness
	if (dirtyFlag_) {
		// Re-evaluation is not allowed on the server
		if (serverMode_) {
			raiseException(
					"In GIndividual::fitness():" << std::endl
					<< "Tried to perform re-evaluation in server-mode"
			);
		}

		// Make sure the secondary fitness vector is empty
		currentSecondaryFitness_.clear();

		// Trigger fitness calculation. This will also
		// register secondary fitness values used in multi-criterion
		// optimization.
		currentFitness_ = fitnessCalculation();

		// Clear the dirty flag
		setDirtyFlag(false);
	}

	// Return the desired result
	bool tmpDirtyFlag = false;
	return getCachedFitness(tmpDirtyFlag, id);
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * Test for throw in serverMode tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Returns the last known fitness calculation of this object, using the fitness function
 * with id 0.
 *
 * @return The fitness of this individual, according to the fitness function with id 0
 */
double GIndividual::fitness() {
	return fitness(0);
}

/************************************************************************************************************/
/**
 * Adapts and evaluates the individual in one go
 *
 * @return The main fitness result
 */
double GIndividual::adaptAndEvaluate() {
	adapt();
	return doFitnessCalculation();
}

/************************************************************************************************************/
/**
 * Retrieves the cached (not necessarily up-to-date) fitness
 *
 * @param dirtyFlag The value of the dirtyFlag_ variable
 * @param id The id of the primary or secondary fitness value
 * @return The cached fitness value (not necessarily up-to-date) with id id
 */
double GIndividual::getCachedFitness(bool& dirtyFlag, const std::size_t& id) const  {
	dirtyFlag = dirtyFlag_;

	if(0 == id) {
		return currentFitness_;
	} else {
#ifdef DEBUG
		if(currentSecondaryFitness_.size() < id) {
			raiseException(
					"In GIndividual::getCachedFitness(bool&, const std::size_t& id): Error!" << std::endl
					<< "Got invalid result id: " << id << std::endl
					<< "where maximum allowed id would be " << currentSecondaryFitness_.size()-1 << std::endl
			);
		}
#endif /* DEBUG */

		return currentSecondaryFitness_.at(id - 1);
	}

	// Make the compiler happy
	return currentFitness_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Enforces re-calculation of the fitness.
 *
 * @return The main result of the fitness calculation
 */
double GIndividual::doFitnessCalculation() {
	// Make sure the secondary fitness vector is empty
	currentSecondaryFitness_.clear();
	// Do the actual calculation
	currentFitness_ = fitnessCalculation();
	// Clear the dirty flag
	setDirtyFlag(false);
	// Return the main fitness value
	return currentFitness_;
}

/* ----------------------------------------------------------------------------------
 * untested
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Registers a new, secondary result value of the custom fitness calculation. This is used in multi-criterion
 * optimization. fitnessCalculation() returns the main fitness value, but may also add further, secondary
 * results. Note that, whether these are used, depends on the optimization algorithm being used.
 *
 * @param secondaryValue The secondary fitness value to be registered
 */
void GIndividual::registerSecondaryResult(const double& secondaryValue) {
	currentSecondaryFitness_.push_back(secondaryValue);
}

/************************************************************************************************************/
/**
 * Determines the number of fitness criteria present for individual. Secondary fitness values are stored in
 * a std::vector, so we determine its size and add 1 for the main fitness value (which always needs to be present).
 *
 * @return The number of fitness criteria registered with this individual
 */
std::size_t GIndividual::getNumberOfFitnessCriteria() const {
	return currentSecondaryFitness_.size() + 1;
}

/************************************************************************************************************/
/**
 * Determines whether more than one fitness criterion is present for this individual
 *
 * @return A boolean indicating whether more than one target function is present
 */
bool GIndividual::hasMultipleFitnessCriteria() const {
	return (getNumberOfFitnessCriteria()>1?true:false);
}

/************************************************************************************************************/
/**
 * (De-)activates the server mode.
 *
 * @param sM The desired new value of the serverMode_ variable
 * @return The previous value of the serverMode_ variable
 */
bool GIndividual::setServerMode(const bool& sM) {
	bool previous = serverMode_;
	serverMode_ = sM;
	return previous;
}

/* ----------------------------------------------------------------------------------
 * Setting is tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
 * Test for throw of fitness() function in serverMode tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Checks whether the server mode is set
 *
 * @return The current value of the serverMode_ variable
 */
bool GIndividual::serverMode() const {
	return serverMode_;
}

/* ----------------------------------------------------------------------------------
 * Retrieval is tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Checks whether the dirty flag is set
 *
 * @return The value of the dirtyFlag_ variable
 */
bool GIndividual::isDirty() const  {
	return dirtyFlag_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Specify whether we want to work in maximization (true) or minimization
 * (false) mode. This function is protected. The idea is that GParameterSet provides a public
 * wrapper for this function, so that a user can specify whether he wants to maximize or
 * minimize a given evaluation function. Optimization algorithms, in turn, only check the
 * maximization-mode of the individuals stored in them and set their own maximization mode
 * internally accordingly, using the protected, overloaded function.
 *
 * @param mode A boolean which indicates whether we want to work in maximization or minimization mode
 */
void GIndividual::setMaxMode_(const bool& mode) {
	maximize_ = mode;
}

/* ----------------------------------------------------------------------------------
 * Setting is tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Allows to retrieve the maximize_ parameter
 *
 * @return The current value of the maximize_ parameter
 */
bool GIndividual::getMaxMode() const {
	return maximize_;
}

/* ----------------------------------------------------------------------------------
 * Retrieval is tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Sets the dirtyFlag_. This is a "one way" function, accessible to derived classes. Once the dirty flag
 * has been set, the only way to reset it is to calculate the fitness of this object.
 */
void GIndividual::setDirtyFlag()  {
	dirtyFlag_ = true;
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * Tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Sets the dirtyFlag_ to any desired value
 *
 * @param dirtyFlag The new value for the dirtyFlag_ variable
 * @return The previous value of the dirtyFlag_ variable
 */
bool GIndividual::setDirtyFlag(const bool& dirtyFlag)  {
	bool previous = dirtyFlag_;
	dirtyFlag_ = dirtyFlag;
	return previous;
}

/* ----------------------------------------------------------------------------------
 * Tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Combines secondary evaluation results by adding the individual results
 *
 *  @return The result of the combination
 */
double GIndividual::sumCombiner() const {
	double result = 0.;
	std::vector<double>::const_iterator cit;
	for(cit=currentSecondaryFitness_.begin(); cit!=currentSecondaryFitness_.end(); ++cit) {
		result += *cit;
	}
	return result;
}

/************************************************************************************************************/
/**
 * Combines secondary evaluation results by adding the absolute values of individual results
 *
 *  @return The result of the combination
 */
double GIndividual::fabsSumCombiner() const {
	double result = 0.;
	std::vector<double>::const_iterator cit;
	for(cit=currentSecondaryFitness_.begin(); cit!=currentSecondaryFitness_.end(); ++cit) {
		result += fabs(*cit);
	}
	return result;
}

/************************************************************************************************************/
/**
 * Combines secondary evaluation results by calculation the square root of the squared sum. Note that we
 * only evaluate the secondary results here. It is assumed that the result of this function is returned as
 * the main result of the fitnessCalculation() function.
 *
 * @return The result of the combination
 */
double GIndividual::squaredSumCombiner() const {
	double result = 0.;
	std::vector<double>::const_iterator cit;
	for(cit=currentSecondaryFitness_.begin(); cit!=currentSecondaryFitness_.end(); ++cit) {
		result += GSQUARED(*cit);
	}
	return sqrt(result);
}

/************************************************************************************************************/
/**
 * Combines secondary evaluation results by calculation the square root of the weighed squared sum. Note that we
 * only evaluate the secondary results here. It is assumed that the result of this function is returned as
 * the main result of the fitnessCalculation() function.
 *
 * @param weights The weights to be multiplied with the cached results
 * @return The result of the combination
 */
double GIndividual::weighedSquaredSumCombiner(const std::vector<double>& weights) const {
	double result = 0.;
	std::vector<double>::const_iterator cit_eval, cit_weights;

	if(currentSecondaryFitness_.size() != weights.size()) {
		raiseException(
				"In GIndividual::weighedSquaredSumCombine(): Error!" << std::endl
				<< "Sizes of currentSecondaryFitness_ and the weights vector don't match: " << currentSecondaryFitness_.size() << " / " << weights.size() << std::endl
		);
	}

	for(cit_eval=currentSecondaryFitness_.begin(), cit_weights=weights.begin();
		cit_eval!=currentSecondaryFitness_.end();
		++cit_eval, ++cit_weights
	) {
		result += GSQUARED((*cit_weights)*(*cit_eval));
	}

	return sqrt(result);
}

/************************************************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void GIndividual::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
	, const bool& showOrigin
) {
	// Add local data
	// maximize_ will be set in GParameterSet, as it has a different
	// meaning for optimization algorithms that also derive indirectly
	// from this class.

	// Call our parent class'es function
	GObject::addConfigurationOptions(gpb, showOrigin);
}

/************************************************************************************************************/
/**
 * Sets the current personality of this individual
 *
 * TODO: Remove this dependency on GXXPersonalityTraits
 *
 * @param pers The desired personality of this individual
 * @return The previous personality of this individual
 */
personality_oa GIndividual::setPersonality(const personality_oa& pers) {
	// Make a note of the current (soon to be previous) personality_oa
	personality_oa previous = pers_;

	// Do nothing if this particular personality type has already been set
	if(pers_==pers && pt_ptr_)  return pers_; // A suitable personality has already been added

	// Create suitable personality objects
	switch(pers) {
	case PERSONALITY_NONE:
		pt_ptr_.reset();
		break;

	case PERSONALITY_EA:
		pt_ptr_ = boost::shared_ptr<GEAPersonalityTraits>(new GEAPersonalityTraits());
		break;

	case PERSONALITY_GD:
		pt_ptr_ = boost::shared_ptr<GGDPersonalityTraits>(new GGDPersonalityTraits());
		break;

	case PERSONALITY_SWARM:
		pt_ptr_ = boost::shared_ptr<GSwarmPersonalityTraits>(new GSwarmPersonalityTraits());
		break;
	}

	// Update our local personality
	pers_ = pers;

	// Let the audience know the previous personality type
	return previous;
}


/* ----------------------------------------------------------------------------------
 * Tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Resets the current personality to PERSONALITY_NONE
 */
void GIndividual::resetPersonality() {
	setPersonality(PERSONALITY_NONE);
}

/* ----------------------------------------------------------------------------------
 * Tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Retrieves the current personality of this individual
 *
 * @return The current personality of this object
 */
personality_oa GIndividual::getPersonality() const {
	return pers_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * This function returns the current personality traits base pointer. Note that there
 * is another version of the same command that does on-the-fly conversion of the
 * personality traits to the derived class.
 *
 * @return A shared pointer to the personality traits base class
 */
boost::shared_ptr<GPersonalityTraits> GIndividual::getPersonalityTraits() {
#ifdef DEBUG
	// Do some error checking
	if(!pt_ptr_) {
		raiseException(
				"In GIndividual::getPersonalityTraits():" << std::endl
				<< "Pointer to personality traits object is empty."
		);
	}
#endif

	return pt_ptr_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
 * Tested in GIndividual::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * A wrapper for GIndividual::customUpdateOnStall() (or the corresponding overloaded
 * functions in derived classes) that does error-checking and sets the dirty flag.
 *
 * @return A boolean indicating whether an update was performed and the individual has changed
 */
bool GIndividual::updateOnStall() {
	// Do the actual update of the individual's structure
	bool updatePerformed = customUpdateOnStall();
	if(updatePerformed) {
		setDirtyFlag();
	}

	return updatePerformed;
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Updates the object's structure and/or parameters, if the optimization has
 * stalled. The quality of the object is likely to get worse. Hence it will
 * enter a micro-training environment to improve its quality. The function can
 * inform the caller that no action was taken, by returning false. Otherwise, if
 * an update was made that requires micro-training, true should be returned.
 * The actions to be taken for this update depend on the actual structure of the
 * individual and need to be implemented for each particular case individually.
 * Note that, as soon as the structure of this object changes, it should return
 * true, as otherwise no value calculation takes place.
 *
 * @return A boolean indicating whether an update was performed and the object has changed
 */
bool GIndividual::customUpdateOnStall() {
	return false;
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Actions to be performed when adapting this object. This function will be overloaded particularly
 * for the GParameterSet class.
 */
void GIndividual::customAdaptions()
{ /* nothing */}

/************************************************************************************************************/
/**
 * Performs all necessary processing steps for this object. Not meant to be
 * called directly from threads, as no exceptions are caught. Use checkedProcess() instead.
 * If the processingCycles_ variable is set to a value of 0 or higher than 1, multiple
 * adapt() calls will be performed in EA mode, until the maximum number of calls is reached or
 * a better solution is found. If processingCycles_ has a value of 0, this routine
 * will loop forever, unless a better solution is found (DANGEROUS: USE WITH CARE!!!).
 *
 * @return A boolean which indicates whether processing has led to a useful result
 */
bool GIndividual::process(){
	bool gotUsefulResult = false;

	// Make sure GParameterBase objects are updated with our local random number generator
	this->updateRNGs();

	// Record the previous setting of the serverMode_ flag and make
	// sure that re-evaluation is possible
	bool previousServerMode=setServerMode(false);

	switch(pers_) {
	//-------------------------------------------------------------------------------------------------------
	case PERSONALITY_EA: // Evolutionary Algorithm
		{
			if(getPersonalityTraits()->getCommand() == "adapt") {
				if(processingCycles_ == 1 || getAssignedIteration() == 0) {
					adaptAndEvaluate();
					gotUsefulResult = true;
				}
				else{
					// Retrieve this object's current fitness.
					bool isDirty=false;
					double originalFitness = getCachedFitness(isDirty);

#ifdef DEBUG
					// Individuals that arrive here for adaption should be "clean"
					if(isDirty) {
						raiseException(
								"In GIndividual::process(): Dirty flag set when it shouldn't be!"
						);
					}
#endif /* DEBUG */

					// Record the number of processing cycles
					boost::uint32_t nCycles=0;

					// Will hold a copy of this object
					boost::shared_ptr<GIndividual> p;

					// Indicates whether a better solution was found
					bool success = false;

					// Loop until a better solution was found or the maximum number of attempts was reached
					while(true) {
						// Create a copy of this object
						p = clone<GIndividual>();

						// Adapt and check fitness. Leave if a better solution was found
						p->adapt();
						p->doFitnessCalculation();
						if((!maximize_ && p->fitness(0) < originalFitness) || (maximize_ && p->fitness(0) > originalFitness))	{
							success = true;
							break;
						}

						// Leave if the maximum number of cycles was reached. Will continue
						// to loop if processingCycles_ is 0 (dangerous!)
						if(processingCycles_ && nCycles++ >= processingCycles_) break;
					}

					// Load the last tested solution into this object
					GObject::load(p);

					// If a better solution was found, let the audience know
					if(success) gotUsefulResult = true;
				}
			}
			else if(getPersonalityTraits()->getCommand() == "evaluate") {
				doFitnessCalculation();
				gotUsefulResult = true;
			}
			else {
				raiseException(
						"In GIndividual::process(//EA//): Unknown command: \""
						<< getPersonalityTraits()->getCommand() << "\""
				);
			}
		}
		break;

	//-------------------------------------------------------------------------------------------------------
	case PERSONALITY_SWARM:
		{
			if(getPersonalityTraits()->getCommand() == "evaluate") {
				// Trigger fitness calculation
				doFitnessCalculation();
			}
			else {
				raiseException(
						"In GIndividual::process(//SWARM//): Unknown command: \""
						<< getPersonalityTraits()->getCommand() << "\""
				);
			}

			// Processing in swarms will always yield useful results, regardless of
			// whether a better solution was found than previously known.
			gotUsefulResult = true;
		}
		break;

	//-------------------------------------------------------------------------------------------------------
	case PERSONALITY_GD:
		{
			if(getPersonalityTraits()->getCommand() == "evaluate") {
				// Trigger fitness calculation
				doFitnessCalculation();
			}
			else {
				raiseException(
						"In GIndividual::process(//GD//): Unknown command: \""
						<< getPersonalityTraits()->getCommand() << "\""
				);
			}

			// Processing in gradient descents will always yield useful results, regardless of
			// whether a better solution was found than previously known.
			gotUsefulResult = true;
		}
		break;

	//-------------------------------------------------------------------------------------------------------
	default:
		{
			raiseException(
					"In GIndividual::process(): Error" << std::endl
					<< "Processing for invalid algorithm requested"
			);
		}
		break;
	}

	// Restore the serverMode_ flag
	setServerMode(previousServerMode);

	// Restore the local random number generators in the individuals
	this->restoreRNGs();

	// Let the audience know
	return gotUsefulResult;
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * Tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Allows to instruct this individual to perform multiple process operations in one go.
 * This is useful in order to minimize communication between client and server. See the
 * description of the process() function for further information.
 *
 * @param processingCycles The desired number of maximum processing cycles
 */
void GIndividual::setProcessingCycles(const boost::uint32_t& processingCycles) {
	processingCycles_= processingCycles;
}

/* ----------------------------------------------------------------------------------
 * Tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/** @brief Retrieves the number of allowed processing cycles */
boost::uint32_t GIndividual::getProcessingCycles() const {
	return processingCycles_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Allows to set the current iteration of the parent optimization algorithm.
 *
 * @param parentAlgIteration The current iteration of the optimization algorithm
 */
void GIndividual::setAssignedIteration(const boost::uint32_t& parentAlgIteration) {
	assignedIteration_ = parentAlgIteration;
}

/* ----------------------------------------------------------------------------------
 * Tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Gives access to the parent optimization algorithm's iteration
 *
 * @return The parent optimization algorithm's current iteration
 */
boost::uint32_t GIndividual::getAssignedIteration() const {
	return assignedIteration_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Allows to set the globally best known fitness
 *
 * @param bnf The best known fitness so far
 */
void GIndividual::setBestKnownFitness(const double& bnf) {
	bestPastFitness_ = bnf;
}

/* ----------------------------------------------------------------------------------
 * Tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Retrieves the value of the globally best known fitness
 *
 * @return The best known fitness so far
 */
double GIndividual::getBestKnownFitness() const {
	return bestPastFitness_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Allows to specify the number of optimization cycles without improvement
 *
 * @param nStalls The number of optimization cycles without improvement in the parent algorithm
 */
void GIndividual::setNStalls(const boost::uint32_t& nStalls) {
	nStalls_ = nStalls;
}

/* ----------------------------------------------------------------------------------
 * Tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Allows to retrieve the number of optimization cycles without improvement
 *
 * @return The number of optimization cycles without improvement in the parent algorithm
 */
boost::uint32_t GIndividual::getNStalls() const {
	return nStalls_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Updates the random number generators contained in this object's GParameterBase-derivatives. This function
 * is filled with meaning in GParameterSet, but is empty for other GIndividual-derivatives.
 */
void GIndividual::updateRNGs()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * Restores the random number generators contained in this object's GParameterBase-derivatives. This function
 * is filled with meaning in GParameterSet, but is empty for other GIndividual-derivatives.
 */
void GIndividual::restoreRNGs()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * Checks whether all GParameterBase derivatives use local random number generators. This function
 * is filled with meaning in GParameterSet, but is empty for other GIndividual-derivatives. In this
 * dummy version the result will always be "true".
 *
 * @return A boolean indicating whether only local random number generators are used in the GParameterBase derivatives (return value will always be true)
 */
bool GIndividual::localRNGsUsed() const {
	return true;
}

#ifdef GENEVATESTING

/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GIndividual::modify_GUnitTests() {
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent class'es function
	if(GObject::modify_GUnitTests()) result = true;

	// Adaption of parameters through the adapt() call is done in GTestIndividual1.
	// We do not know what is stored in this individual, hence we cannot modify
	// parameters directly here in this class.

	// A relatively harmless change
	this->setProcessingCycles(this->getProcessingCycles() + 1);

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GIndividual::specificTestsNoFailureExpected_GUnitTests() {
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GObject::specificTestsNoFailureExpected_GUnitTests();

	//------------------------------------------------------------------------------

	{ // Test setting and retrieval of the server mode flag
		boost::shared_ptr<GIndividual> p_test = this->clone<GIndividual>();

		BOOST_CHECK_NO_THROW(p_test->setServerMode(true));
		BOOST_CHECK(p_test->serverMode() == true);
		BOOST_CHECK_NO_THROW(p_test->setServerMode(false));
		BOOST_CHECK(p_test->serverMode() == false);
	}

	//------------------------------------------------------------------------------

	{ // Test setting and retrieval of the maximization mode flag
		boost::shared_ptr<GIndividual> p_test = this->clone<GIndividual>();

		BOOST_CHECK_NO_THROW(p_test->setMaxMode_(true));
		BOOST_CHECK(p_test->getMaxMode() == true);
		BOOST_CHECK_NO_THROW(p_test->setMaxMode_(false));
		BOOST_CHECK(p_test->getMaxMode() == false);
	}

	//------------------------------------------------------------------------------

	{ // Check setting of the dirty flag
		boost::shared_ptr<GIndividual> p_test = this->clone<GIndividual>();

		BOOST_CHECK_NO_THROW(p_test->setDirtyFlag(true));
		BOOST_CHECK(p_test->isDirty() == true);
		BOOST_CHECK_NO_THROW(p_test->setDirtyFlag(false));
		BOOST_CHECK(p_test->isDirty() == false);
		BOOST_CHECK_NO_THROW(p_test->setDirtyFlag());
		BOOST_CHECK(p_test->isDirty() == true); // Note the missing argument -- this is a different function
		BOOST_CHECK_NO_THROW(p_test->setDirtyFlag(false));
		BOOST_CHECK(p_test->isDirty() == false);
	}

	//------------------------------------------------------------------------------

	{ // Test setting and retrieval of processing cycles
		boost::shared_ptr<GIndividual> p_test = this->clone<GIndividual>();

		for(boost::uint32_t i=1; i<10; i++) {
			BOOST_CHECK_NO_THROW(p_test->setProcessingCycles(i));
			BOOST_CHECK_MESSAGE(
					p_test->getProcessingCycles() == i
					,  "\n"
					<< "p_test->getProcessingCycles() = " << p_test->getProcessingCycles() << "\n"
					<< "i = " << i << "\n"
			);
		}
	}

	//------------------------------------------------------------------------------

	{ // Test setting and retrieval of the surrounding optimization algorithm's current iteration
		boost::shared_ptr<GIndividual> p_test = this->clone<GIndividual>();

		for(boost::uint32_t i=1; i<10; i++) {
			BOOST_CHECK_NO_THROW(p_test->setAssignedIteration(i));
			BOOST_CHECK_MESSAGE(
					p_test->getAssignedIteration() == i
					,  "\n"
					<< "p_test->getAssignedIteration() = " << p_test->getAssignedIteration() << "\n"
					<< "i = " << i << "\n"
			);
		}
	}

	//------------------------------------------------------------------------------

	{ // Test setting and retrieval of the best known fitness so far
		boost::shared_ptr<GIndividual> p_test = this->clone<GIndividual>();

		for(double d=0.; d<1.; d+=0.1) {
			BOOST_CHECK_NO_THROW(p_test->setBestKnownFitness(d));
			BOOST_CHECK_MESSAGE(
					p_test->getBestKnownFitness() == d
					,  "\n"
					<< "p_test->getBestKnownFitness() = " << p_test->getBestKnownFitness() << "\n"
					<< "d = " << d << "\n"
			);
		}
	}

	//------------------------------------------------------------------------------

	{ // Test setting and retrieval of the number of consecutive stalls
		boost::shared_ptr<GIndividual> p_test = this->clone<GIndividual>();

		for(boost::uint32_t i=1; i<10; i++) {
			BOOST_CHECK_NO_THROW(p_test->setNStalls(i));
			BOOST_CHECK_MESSAGE(
					p_test->getNStalls() == i
					,  "\n"
					<< "p_test->getNStalls() = " << p_test->getNStalls() << "\n"
					<< "i = " << i << "\n"
			);
		}
	}

	//------------------------------------------------------------------------------

	{ // Check setting and retrieval of the current personality status and whether the personalities themselves can be accessed
		boost::shared_ptr<GIndividual> p_test = this->clone<GIndividual>();
		boost::shared_ptr<GPersonalityTraits> p_pt;

		// Reset the personality type
		BOOST_CHECK_NO_THROW(p_test->resetPersonality());
		BOOST_CHECK_MESSAGE(
				p_test->getPersonality() == PERSONALITY_NONE
				,  "\n"
				<< "p_test->getPersonality() = " << p_test->getPersonality() << "\n"
				<< "expected PERSONALITY_NONE\n"
		);

		// Set the personality type to EA
		personality_oa previous;
		BOOST_CHECK_NO_THROW(previous = p_test->setPersonality(PERSONALITY_EA));
		BOOST_CHECK_MESSAGE(
				previous == PERSONALITY_NONE
				,  "\n"
				<< "previous = " << previous << "\n"
				<< "expected PERSONALITY_NONE"
		);
		BOOST_CHECK_MESSAGE(
				p_test->getPersonality() == PERSONALITY_EA
				,  "\n"
				<< "p_test->getPersonality() = " << p_test->getPersonality() << "\n"
				<< "expected EA\n"
		);

		// Try to retrieve a GEAPersonalityTraits object and check that the smart pointer actually points somewhere
		boost::shared_ptr<GEAPersonalityTraits> p_pt_ea;
		BOOST_CHECK_NO_THROW(p_pt_ea = p_test->getPersonalityTraits<GEAPersonalityTraits>());
		BOOST_CHECK(p_pt_ea);
		p_pt_ea.reset();

		// Retrieve a base pointer to the EA object and check that it points somewhere
		BOOST_CHECK_NO_THROW(p_pt = p_test->getPersonalityTraits());
		BOOST_CHECK(p_pt);
		p_pt.reset();

		// Set the personality type to GD
		BOOST_CHECK_NO_THROW(previous = p_test->setPersonality(PERSONALITY_GD));
		BOOST_CHECK_MESSAGE(
				previous == PERSONALITY_EA
				,  "\n"
				<< "previous = " << previous << "\n"
				<< "expected EA"
		);
		BOOST_CHECK_MESSAGE(
				p_test->getPersonality() == PERSONALITY_GD
				,  "\n"
				<< "p_test->getPersonality() = " << p_test->getPersonality() << "\n"
				<< "expected GD\n"
		);

		// Try to retrieve a GGDPersonalityTraits object and check that the smart pointer actually points somewhere
		boost::shared_ptr<GGDPersonalityTraits> p_pt_gd;
		BOOST_CHECK_NO_THROW(p_pt_gd = p_test->getPersonalityTraits<GGDPersonalityTraits>());
		BOOST_CHECK(p_pt_gd);
		p_pt_gd.reset();

		// Retrieve a base pointer to the GD object and check that it points somewhere
		BOOST_CHECK_NO_THROW(p_pt = p_test->getPersonalityTraits());
		BOOST_CHECK(p_pt);
		p_pt.reset();

		// Set the personality type to SWARM
		BOOST_CHECK_NO_THROW(previous = p_test->setPersonality(PERSONALITY_SWARM));
		BOOST_CHECK_MESSAGE(
				previous == PERSONALITY_GD
				,  "\n"
				<< "previous = " << previous << "\n"
				<< "expected GD"
		);
		BOOST_CHECK_MESSAGE(
				p_test->getPersonality() == PERSONALITY_SWARM
				,  "\n"
				<< "p_test->getPersonality() = " << p_test->getPersonality() << "\n"
				<< "expected SWARM\n"
		);

		// Try to retrieve a GSwarmPersonalityTraits object and check that the smart pointer actually points somewhere
		boost::shared_ptr<GSwarmPersonalityTraits> p_pt_swarm;
		BOOST_CHECK_NO_THROW(p_pt_swarm = p_test->getPersonalityTraits<GSwarmPersonalityTraits>());
		BOOST_CHECK(p_pt_swarm);
		p_pt_swarm.reset();

		// Retrieve a base pointer to the SWARM object and check that it points somewhere
		BOOST_CHECK_NO_THROW(p_pt = p_test->getPersonalityTraits());
		BOOST_CHECK(p_pt);
		p_pt.reset();

		// Set the personality type to PERSONALITY_NONE
		BOOST_CHECK_NO_THROW(previous = p_test->setPersonality(PERSONALITY_NONE));
		BOOST_CHECK_MESSAGE(
				previous == PERSONALITY_SWARM
				,  "\n"
				<< "previous = " << previous << "\n"
				<< "expected SWARM"
		);
		BOOST_CHECK_MESSAGE(
				p_test->getPersonality() == PERSONALITY_NONE
				,  "\n"
				<< "p_test->getPersonality() = " << p_test->getPersonality() << "\n"
				<< "expected PERSONALITY_NONE\n"
		);
	}

	//------------------------------------------------------------------------------
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GIndividual::specificTestsFailuresExpected_GUnitTests() {
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GObject::specificTestsFailuresExpected_GUnitTests();

	//------------------------------------------------------------------------------

#ifdef DEBUG
	{ // Test that retrieval of an EA personality traits object from an uninitialized pointer throws in DEBUG mode
		boost::shared_ptr<GIndividual> p_test = this->clone<GIndividual>();

		// Make sure the personality type is set to PERSONALITY_NONE
		BOOST_CHECK_NO_THROW(p_test->resetPersonality());

		// Trying to retrieve an EA personality object should throw
		boost::shared_ptr<GEAPersonalityTraits> p_pt_ea;
		BOOST_CHECK_THROW(p_pt_ea = p_test->getPersonalityTraits<GEAPersonalityTraits>(), Gem::Common::gemfony_error_condition);
	}
#endif /* DEBUG */

	//------------------------------------------------------------------------------

#ifdef DEBUG
	{ // Test that retrieval of an EA personality traits object from an individual with SWARM personality throws
		boost::shared_ptr<GIndividual> p_test = this->clone<GIndividual>();

		// Make sure the personality type is set to SWARM
		BOOST_CHECK_NO_THROW(p_test->setPersonality(PERSONALITY_SWARM));

		// Trying to retrieve an EA personality object should throw
		BOOST_CHECK_THROW(p_test->getPersonalityTraits<GEAPersonalityTraits>(), Gem::Common::gemfony_error_condition);
	}
#endif /* DEBUG */

	//------------------------------------------------------------------------------

#ifdef DEBUG
	{ // Test that retrieval of a personality traits base object from an individual without personality throws
		boost::shared_ptr<GIndividual> p_test = this->clone<GIndividual>();

		// Make sure the personality type is set to PERSONALITY_NONE
		BOOST_CHECK_NO_THROW(p_test->resetPersonality());

		// Trying to retrieve an EA personality object should throw
		boost::shared_ptr<GPersonalityTraits> p_pt;
		BOOST_CHECK_THROW(p_pt = p_test->getPersonalityTraits(), Gem::Common::gemfony_error_condition);
	}
#endif /* DEBUG */

	//------------------------------------------------------------------------------
}

/************************************************************************************************************/

#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
