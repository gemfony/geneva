/**
 * @file GIndividual.cpp
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

#include "geneva/GIndividual.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GIndividual)

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
	, bestPastFitness_(0.)
	, nStalls_(0)
	, dirtyFlag_(true)
	, serverMode_(false)
	, processingCycles_(1)
	, maximize_(false)
	, parentAlgIteration_(0)
	, pers_(NONE)
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
	, bestPastFitness_(cp.bestPastFitness_)
	, nStalls_(cp.nStalls_)
	, dirtyFlag_(cp.dirtyFlag_)
	, serverMode_(cp.serverMode_)
	, processingCycles_(cp.processingCycles_)
	, maximize_(cp.maximize_)
	, parentAlgIteration_(cp.parentAlgIteration_)
	, pers_(cp.pers_)
{
	// We need to take care of the personality pointer manually
	setPersonality(pers_); // this call will also make sure that a suitable personality object is being created
	if(pers_ != NONE) pt_ptr_->GObject::load(cp.pt_ptr_);
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
boost::optional<std::string> GIndividual::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GIndividual *p_load = GObject::conversion_cast<GIndividual>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GObject::checkRelationshipWith(cp, e, limit, "GIndividual", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GIndividual", currentFitness_, p_load->currentFitness_, "currentFitness_", "p_load->currentFitness_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GIndividual", bestPastFitness_, p_load->bestPastFitness_, "bestPastFitness_", "p_load->bestPastFitness_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GIndividual", nStalls_, p_load->nStalls_, "nStalls_", "p_load->nStalls_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GIndividual", dirtyFlag_, p_load->dirtyFlag_, "dirtyFlag_", "p_load->dirtyFlag_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GIndividual", serverMode_, p_load->serverMode_, "serverMode_", "p_load->serverMode_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GIndividual", processingCycles_, p_load->processingCycles_, "processingCycles_", "p_load->processingCycles_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GIndividual", maximize_, p_load->maximize_, "maximize_", "p_load->maximize_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GIndividual", parentAlgIteration_, p_load->parentAlgIteration_, "parentAlgIteration_", "p_load->parentAlgIteration_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GIndividual", pers_, p_load->pers_, "pers_", "p_load->pers_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GIndividual", pt_ptr_, p_load->pt_ptr_, "pt_ptr_", "p_load->pt_ptr_", e , limit));

	return evaluateDiscrepancies("GIndividual", caller, deviations, e);
}

/************************************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GIndividual object, camouflaged as a GObject
 */
void GIndividual::load_(const GObject* cp) {
	const GIndividual *p_load = conversion_cast<GIndividual>(cp);

	// Load the parent class'es data
	GObject::load_(cp);

	// Then load our local data
	currentFitness_ = p_load->currentFitness_;
	bestPastFitness_ = p_load->bestPastFitness_;
	nStalls_ = p_load->nStalls_;
	dirtyFlag_ = p_load->dirtyFlag_;
	serverMode_ = p_load->serverMode_;
	processingCycles_ = p_load->processingCycles_;
	maximize_ = p_load->maximize_;
	parentAlgIteration_ = p_load->parentAlgIteration_;
	setPersonality(p_load->pers_);
	if(pers_ != NONE) pt_ptr_->GObject::load(p_load->pt_ptr_);
}

/************************************************************************************************************/
/**
 * The adaption interface. This function also triggers re-evaluation of the fitness.
 */
void GIndividual::adapt() {
	customAdaptions(); // The actual mutation and adaption process
	GIndividual::setDirtyFlag(true);
	GIndividual::fitness(); // Trigger re-evaluation
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Returns the last known fitness calculation of this object. Re-calculation
 * of the fitness is triggered, unless this is the server mode.
 *
 * @return The fitness of this individual
 */
double GIndividual::fitness() {
	if (dirtyFlag_) {
		// Re-evaluation is not allowed on the server
		if (serverMode_) {
			std::ostringstream error;
			error << "In GIndividual::fitness(): Error!" << std::endl
				  << "Tried to perform re-evaluation in server-mode" << std::endl;
			throw Gem::Common::gemfony_error_condition(error.str());
		}

		currentFitness_ = fitnessCalculation();
		setDirtyFlag(false);
	}

	return currentFitness_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * Test for throw in serverMode tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Retrieves the current (not necessarily up-to-date) fitness
 *
 * @param dirtyFlag The value of the dirtyFlag_ variable
 * @return The current fitness value (not necessarily up-to-date)
 */
double GIndividual::getCurrentFitness(bool& dirtyFlag) const  {
	dirtyFlag = dirtyFlag_;
	return currentFitness_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Enforces re-calculation of the fitness. Mainly needed for testing purposes.
 *
 * @return The result of the fitness calculation
 */
double GIndividual::doFitnessCalculation() {
	currentFitness_ = fitnessCalculation();
	setDirtyFlag(false);
	return currentFitness_;
}

/* ----------------------------------------------------------------------------------
 * untested
 * ----------------------------------------------------------------------------------
 */

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
 * (false) mode
 *
 * @param mode A boolean which indicates whether we want to work in maximization or minimization mode
 */
void GIndividual::setMaxMode(const bool& mode) {
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
 * Sets the dirtyFlag_. This is a "one way" function, accessible to the external
 * user. Once the dirty flag has been set, the only way to reset it is to calculate
 * the fitness of this object.
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
 * Sets the current personality of this individual
 *
 * @param pers The desired personality of this individual
 * @return The previous personality of this individual
 */
personality GIndividual::setPersonality(const personality& pers) {
	// Make a note of the current (soon to be previous) personality
	personality previous = pers_;

	// Do nothing if this particular personality type has already been set
	if(pers_==pers && pt_ptr_)  return pers_; // A suitable personality has already been added

	// Create suitable personality objects
	switch(pers) {
	case NONE:
		pt_ptr_.reset();
		break;

	case EA:
		pt_ptr_ = boost::shared_ptr<GEAPersonalityTraits>(new GEAPersonalityTraits());
		break;

	case GD:
		pt_ptr_ = boost::shared_ptr<GGDPersonalityTraits>(new GGDPersonalityTraits());
		break;

	case SWARM:
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
 * Resets the current personality to NONE
 */
void GIndividual::resetPersonality() {
	setPersonality(NONE);
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
personality GIndividual::getPersonality() const {
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
		std::ostringstream error;
		error << "In GIndividual::getPersonalityTraits(): Error!" << std::endl
			  << "Pointer to personality traits object is empty." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}
#endif

	return pt_ptr_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
 * Tested in GIndividual::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/**************************************************************************************************/
/**
 * Convenience function to make the code more readable. Gives access to the evolutionary algorithm
 * personality. Will throw if another personality is active.
 *
 * @return A shared_ptr to the evolutionary algorithms personality traits
 */
boost::shared_ptr<GEAPersonalityTraits> GIndividual::getEAPersonalityTraits() {
	return this->getPersonalityTraits<GEAPersonalityTraits>();
}

/* ----------------------------------------------------------------------------------
 * Tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
 * Tested in GIndividual::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/**************************************************************************************************/
/**
 * Convenience function to make the code more readable. Gives access to the gradient descent
 * personality. Will throw if another personality is active.
 *
 * @return A shared_ptr to the gradient descent personality traits
 */
boost::shared_ptr<GGDPersonalityTraits> GIndividual::getGDPersonalityTraits() {
	return this->getPersonalityTraits<GGDPersonalityTraits>();
}

/* ----------------------------------------------------------------------------------
 * Tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/**************************************************************************************************/
/**
 * Convenience function to make the code more readable. Gives access to the swarm algorithm
 * personality. Will throw if another personality is active.
 *
 * @return A shared_ptr to the swarm algorithms personality traits
 */
boost::shared_ptr<GSwarmPersonalityTraits> GIndividual::getSwarmPersonalityTraits() {
	return this->getPersonalityTraits<GSwarmPersonalityTraits>();
}

/* ----------------------------------------------------------------------------------
 * Tested in GIndividual::specificTestsNoFailureExpected_GUnitTests()
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
	case EA: // Evolutionary Algorithm
		{
			if(getPersonalityTraits()->getCommand() == "adapt") {
				if(processingCycles_ == 1 || getParentAlgIteration() == 0) {
					adapt();
					gotUsefulResult = true;
				}
				else{
					// Retrieve this object's current fitness.
					bool isDirty=false;
					double originalFitness = getCurrentFitness(isDirty);

#ifdef DEBUG
					// Individuals that arrive here for adaption should be "clean"
					if(isDirty) {
						std::ostringstream error;
						error << "In GIndividual::process(): Dirty flag set when it shouldn't be!" << std::endl;
						throw Gem::Common::gemfony_error_condition(error.str());
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
						if((!maximize_ && p->fitness() < originalFitness) || (maximize_ && p->fitness() > originalFitness))	{
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
				fitness();
				gotUsefulResult = true;
			}
			else {
				std::ostringstream error;
				error << "In GIndividual::process(//EA//): Unknown command: \""
						<< getPersonalityTraits()->getCommand() << "\"" << std::endl;
				throw Gem::Common::gemfony_error_condition(error.str());
			}
		}
		break;

	case SWARM:
		{
			if(getPersonalityTraits()->getCommand() == "evaluate") {
				// Trigger fitness calculation
				fitness();
			}
			else {
				std::ostringstream error;
				error << "In GIndividual::process(//SWARM//): Unknown command: \""
					  << getPersonalityTraits()->getCommand() << "\"" << std::endl;
				throw Gem::Common::gemfony_error_condition(error.str());
			}

			// Processing in swarms will always yield useful results, regardless of
			// whether a better solution was found than previously known.
			gotUsefulResult = true;
		}
		break;

	//-------------------------------------------------------------------------------------------------------
	default:
		{
			std::ostringstream error;
			error << "In GIndividual::process(): Error" << std::endl
				  << "Processing for invalid algorithm requested" << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
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
void GIndividual::setParentAlgIteration(const boost::uint32_t& parentAlgIteration) {
	parentAlgIteration_ = parentAlgIteration;
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
boost::uint32_t GIndividual::getParentAlgIteration() const {
	return parentAlgIteration_;
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

		BOOST_CHECK_NO_THROW(p_test->setMaxMode(true));
		BOOST_CHECK(p_test->getMaxMode() == true);
		BOOST_CHECK_NO_THROW(p_test->setMaxMode(false));
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
			BOOST_CHECK_NO_THROW(p_test->setParentAlgIteration(i));
			BOOST_CHECK_MESSAGE(
					p_test->getParentAlgIteration() == i
					,  "\n"
					<< "p_test->getParentAlgIteration() = " << p_test->getParentAlgIteration() << "\n"
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
				p_test->getPersonality() == NONE
				,  "\n"
				<< "p_test->getPersonality() = " << p_test->getPersonality() << "\n"
				<< "expected NONE\n"
		);

		// Set the personality type to EA
		personality previous;
		BOOST_CHECK_NO_THROW(previous = p_test->setPersonality(EA));
		BOOST_CHECK_MESSAGE(
				previous == NONE
				,  "\n"
				<< "previous = " << previous << "\n"
				<< "expected NONE"
		);
		BOOST_CHECK_MESSAGE(
				p_test->getPersonality() == EA
				,  "\n"
				<< "p_test->getPersonality() = " << p_test->getPersonality() << "\n"
				<< "expected EA\n"
		);

		// Try to retrieve a GEAPersonalityTraits object and check that the smart pointer actually points somewhere
		boost::shared_ptr<GEAPersonalityTraits> p_pt_ea;
		BOOST_CHECK_NO_THROW(p_pt_ea = p_test->getEAPersonalityTraits());
		BOOST_CHECK(p_pt_ea);

		// Retrieve the same object through a different method
		p_pt_ea.reset();
		BOOST_CHECK_NO_THROW(p_pt_ea = p_test->getPersonalityTraits<GEAPersonalityTraits>());
		BOOST_CHECK(p_pt_ea);
		p_pt_ea.reset();

		// Retrieve a base pointer to the EA object and check that it points somewhere
		BOOST_CHECK_NO_THROW(p_pt = p_test->getPersonalityTraits());
		BOOST_CHECK(p_pt);
		p_pt.reset();

		// Set the personality type to GD
		BOOST_CHECK_NO_THROW(previous = p_test->setPersonality(GD));
		BOOST_CHECK_MESSAGE(
				previous == EA
				,  "\n"
				<< "previous = " << previous << "\n"
				<< "expected EA"
		);
		BOOST_CHECK_MESSAGE(
				p_test->getPersonality() == GD
				,  "\n"
				<< "p_test->getPersonality() = " << p_test->getPersonality() << "\n"
				<< "expected GD\n"
		);

		// Try to retrieve a GGDPersonalityTraits object and check that the smart pointer actually points somewhere
		boost::shared_ptr<GGDPersonalityTraits> p_pt_gd;
		BOOST_CHECK_NO_THROW(p_pt_gd = p_test->getGDPersonalityTraits());
		BOOST_CHECK(p_pt_gd);

		// Retrieve the same object through a different method
		p_pt_gd.reset();
		BOOST_CHECK_NO_THROW(p_pt_gd = p_test->getPersonalityTraits<GGDPersonalityTraits>());
		BOOST_CHECK(p_pt_gd);
		p_pt_gd.reset();

		// Retrieve a base pointer to the GD object and check that it points somewhere
		BOOST_CHECK_NO_THROW(p_pt = p_test->getPersonalityTraits());
		BOOST_CHECK(p_pt);
		p_pt.reset();

		// Set the personality type to SwARM
		BOOST_CHECK_NO_THROW(previous = p_test->setPersonality(SWARM));
		BOOST_CHECK_MESSAGE(
				previous == GD
				,  "\n"
				<< "previous = " << previous << "\n"
				<< "expected GD"
		);
		BOOST_CHECK_MESSAGE(
				p_test->getPersonality() == SWARM
				,  "\n"
				<< "p_test->getPersonality() = " << p_test->getPersonality() << "\n"
				<< "expected SWARM\n"
		);

		// Try to retrieve a GSwarmPersonalityTraits object and check that the smart pointer actually points somewhere
		boost::shared_ptr<GSwarmPersonalityTraits> p_pt_swarm;
		BOOST_CHECK_NO_THROW(p_pt_swarm = p_test->getSwarmPersonalityTraits());
		BOOST_CHECK(p_pt_swarm);

		// Retrieve the same object through a different method
		p_pt_swarm.reset();
		BOOST_CHECK_NO_THROW(p_pt_swarm = p_test->getPersonalityTraits<GSwarmPersonalityTraits>());
		BOOST_CHECK(p_pt_swarm);
		p_pt_swarm.reset();

		// Retrieve a base pointer to the SWARM object and check that it points somewhere
		BOOST_CHECK_NO_THROW(p_pt = p_test->getPersonalityTraits());
		BOOST_CHECK(p_pt);
		p_pt.reset();

		// Set the personality type to NONE
		BOOST_CHECK_NO_THROW(previous = p_test->setPersonality(NONE));
		BOOST_CHECK_MESSAGE(
				previous == SWARM
				,  "\n"
				<< "previous = " << previous << "\n"
				<< "expected SWARM"
		);
		BOOST_CHECK_MESSAGE(
				p_test->getPersonality() == NONE
				,  "\n"
				<< "p_test->getPersonality() = " << p_test->getPersonality() << "\n"
				<< "expected NONE\n"
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

		// Make sure the personality type is set to NONE
		BOOST_CHECK_NO_THROW(p_test->resetPersonality());

		// Trying to retrieve an EA personality object should throw
		boost::shared_ptr<GEAPersonalityTraits> p_pt_ea;
		BOOST_CHECK_THROW(p_pt_ea = p_test->getEAPersonalityTraits(), Gem::Common::gemfony_error_condition);
	}
#endif /* DEBUG */

	//------------------------------------------------------------------------------

#ifdef DEBUG
	{ // Test that retrieval of an EA personality traits object from an individual with SWARM personality throws
		boost::shared_ptr<GIndividual> p_test = this->clone<GIndividual>();

		// Make sure the personality type is set to SWARM
		BOOST_CHECK_NO_THROW(p_test->setPersonality(SWARM));

		// Trying to retrieve an EA personality object should throw
		BOOST_CHECK_THROW(p_test->getEAPersonalityTraits(), Gem::Common::gemfony_error_condition);
	}
#endif /* DEBUG */

	//------------------------------------------------------------------------------

#ifdef DEBUG
	{ // Test that retrieval of a personality traits base object from an individual without personality throws
		boost::shared_ptr<GIndividual> p_test = this->clone<GIndividual>();

		// Make sure the personality type is set to NONE
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
