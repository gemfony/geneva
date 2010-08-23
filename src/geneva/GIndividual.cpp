/**
 * @file GIndividual.cpp
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
	, allowLazyEvaluation_(false)
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
	, allowLazyEvaluation_(cp.allowLazyEvaluation_)
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
	deviations.push_back(checkExpectation(withMessages, "GIndividual", allowLazyEvaluation_, p_load->allowLazyEvaluation_, "allowLazyEvaluation_", "p_load->allowLazyEvaluation_", e , limit));
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
	allowLazyEvaluation_ = p_load->allowLazyEvaluation_;
	processingCycles_ = p_load->processingCycles_;
	maximize_ = p_load->maximize_;
	parentAlgIteration_ = p_load->parentAlgIteration_;
	setPersonality(p_load->pers_);
	if(pers_ != NONE) pt_ptr_->GObject::load(p_load->pt_ptr_);
}

/************************************************************************************************************/
/**
 * The adaption interface. If lazy evaluation is not allowed (the default), this
 * function also triggers the re-calculation of the fitness.
 */
void GIndividual::adapt() {
	customAdaptions();
	GIndividual::setDirtyFlag(true);

	// In some cases we want to allow lazy evaluation. The
	// fitness calculation is then deferred to the next call
	// of the GIndividual::fitness() function
	if (!allowLazyEvaluation_) {
		currentFitness_ = fitnessCalculation();
		setDirtyFlag(false);
	}
}

/************************************************************************************************************/
/**
 * Returns the last known fitness calculation of this object. Re-calculation
 * of the fitness is triggered, if lazy evaluation is allowed.
 *
 * @return The fitness of this individual
 */
double GIndividual::fitness() {
	if (dirtyFlag_) {

#ifdef DEBUG
		// Except for iteration 0, it is an error if lazy evaluation is not allowed, but
		// the dirty flag is set. There the fitness calculation of initial individuals happens
		// before their modification.
		if (!allowLazyEvaluation_ && getParentAlgIteration() > 0) {
			std::ostringstream error;
			error << "In GIndividual::fitness(): Error!" << std::endl
				  << "The dirty flag is set while lazy evaluation is not allowed."
				  << std::endl;

			throw Gem::Common::gemfony_error_condition(error.str());
		}
#endif /* DEBUG */

		currentFitness_ = fitnessCalculation();
		setDirtyFlag(false);
	}

	return currentFitness_;
}

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

/************************************************************************************************************/
/**
 * Enforces re-calculation of the fitness.
 *
 * @return The result of the fitness calculation
 */
double GIndividual::doFitnessCalculation() {
	currentFitness_ = fitnessCalculation();
	setDirtyFlag(false);
	return currentFitness_;
}

/************************************************************************************************************/
/**
 * Indicates whether lazy evaluation is allowed
 *
 * @param allowLazyEvaluation The new value for the allowLazyEvaluation_ parameter
 * @return The previous value of the allowLazyEvaluation_ parameter
 */
bool GIndividual::setAllowLazyEvaluation(const bool& allowLazyEvaluation)  {
	bool previous = allowLazyEvaluation_;
	allowLazyEvaluation_ = allowLazyEvaluation;
	return previous;
}

/************************************************************************************************************/
/**
 * Retrieve the allowLazyEvaluation_ parameter
 *
 * @return The value of the allowLazyEvaluation_ parameter
 */
bool GIndividual::getAllowLazyEvaluation() const  {
	return allowLazyEvaluation_;
}

/************************************************************************************************************/
/**
 * Checks whether the dirty flag is set
 *
 * @return The value of the dirtyFlag_ variable
 */
bool GIndividual::isDirty() const  {
	return dirtyFlag_;
}

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

/************************************************************************************************************/
/**
 * Allows to retrieve the maximize_ parameter
 *
 * @return The current value of the maximize_ parameter
 */
bool GIndividual::getMaxMode() const {
	return maximize_;
}

/************************************************************************************************************/
/**
 * Sets the dirtyFlag_. This is a "one way" function, accessible to the external
 * user. Once the dirty flag has been set, the only way to reset it is to calculate
 * the fitness of this object.
 */
void GIndividual::setDirtyFlag()  {
	dirtyFlag_ = true;
}

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

/************************************************************************************************************/
/**
 * Sets the current personality of this individual
 *
 * @param pers The desired personality of this individual
 */
void GIndividual::setPersonality(const personality& pers) {
	if(pers_==pers && pt_ptr_)  return; // A suitable personality has already been added

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

	pers_ = pers;
}

/************************************************************************************************************/
/**
 * Resets the current personality to NONE
 */
void GIndividual::resetPersonality() {
	setPersonality(NONE);
}

/************************************************************************************************************/
/**
 * Retrieves the current personality of this individual
 *
 * @return The current personality of this object
 */
personality GIndividual::getPersonality() const {
	return pers_;
}

/************************************************************************************************************/
/**
 * This function returns the current personality traits base pointer. Note that there
 * is another version of the same command that does on-the-fly conversion of the
 * personality traits to the derived class.
 *
 * @return A shared pointer to the personality traits base class
 */
boost::shared_ptr<GPersonalityTraits> GIndividual::getPersonalityTraits() {
	return pt_ptr_;
}

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

/************************************************************************************************************/
/**
 * A version of the fitness framework that also checks for
 * exceptions. To be used when fitness() is to become the main
 * function to be called by a thread.
 */
double GIndividual::checkedFitness(){
	try{
		return fitness();
	}
	catch(std::exception& e){
		std::ostringstream error;
		error << "In GIndividual::checkedFitness(): Caught std::exception with message" << std::endl
		      << e.what() << std::endl;
		std::cerr << error.str();
		std::terminate();
	}
	catch(boost::exception& e){
		std::ostringstream error;
		error << "In GIndividual::checkedFitness(): Caught boost::exception with message" << std::endl;
		std::cerr << error.str();
		std::terminate();
	}
	catch(...){
		std::ostringstream error;
		error << "In GIndividual::checkedFitness(): Caught unknown exception" << std::endl;
		std::cerr << error.str();
		std::terminate();
	}
}

/************************************************************************************************************/
/**
 * Performs all necessary processing steps for this object. Not meant to be
 * called from threads, as no exceptions are caught. Use checkedProcess() instead.
 * If the processingCycles_ variable is set to a value of 0 or higher than 1, multiple
 * adapt() calls will be performed, until the maximum number of calls is reached or
 * a better solution is found. If processingCycles_ has a value of 0, this routine
 * will loop forever, unless a better solution is found (DANGEROUS: USE WITH CARE!!!).
 *
 * @return A boolean which indicates whether processing has led to a useful result
 */
bool GIndividual::process(){
	switch(pers_) {
	//-------------------------------------------------------------------------------------------------------
	case EA: // Evolutionary Algorithm
		{
			bool gotUsefulResult = false;
			bool previous=setAllowLazyEvaluation(false);
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
			setAllowLazyEvaluation(previous);

			return gotUsefulResult;
		}
		break;

	case SWARM:
		{
			// Individuals may arrive with the dirty flag set in swarm algorithms
			bool previous=setAllowLazyEvaluation(true);
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
			setAllowLazyEvaluation(previous);

			// Processing in swarms will always yield useful results, regardless of
			// whether a better solution was found than previously known.
			return true;
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
}

/************************************************************************************************************/
/**
 * Performs all necessary processing steps for this object and catches all exceptions.
 * Meant to be called by threads.
 *
 * @return A boolean which indicates whether processing has led to a useful result
 */
bool GIndividual::checkedProcess(){
	try{
		return process();
	}
	catch(std::exception& e){
		std::ostringstream error;
		error << "In GIndividual::checkedProcess(): Caught std::exception with message" << std::endl
		      << e.what() << std::endl;
		std::cerr << error.str();
		std::terminate();
	}
	catch(boost::exception& e){
		std::ostringstream error;
		error << "In GIndividual::checkedProcess(): Caught boost::exception" << std::endl;
		std::cerr << error.str();
		std::terminate();
	}
	catch(...){
		std::ostringstream error;
		error << "In GIndividual::checkedProcess(): Caught unknown exception" << std::endl;
		std::cerr << error.str();
		std::terminate();
	}
}

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

/************************************************************************************************************/
/** @brief Retrieves the number of allowed processing cycles */
boost::uint32_t GIndividual::getProcessingCycles() const {
	return processingCycles_;
}

/************************************************************************************************************/
/**
 * Allows to set the current iteration of the parent optimization algorithm.
 *
 * @param parentAlgIteration The current iteration of the optimization algorithm
 */
void GIndividual::setParentAlgIteration(const boost::uint32_t& parentAlgIteration) {
	parentAlgIteration_ = parentAlgIteration;
}

/************************************************************************************************************/
/**
 * Gives access to the parent optimization algorithm's iteration
 *
 * @return The parent optimization algorithm's current iteration
 */
boost::uint32_t GIndividual::getParentAlgIteration() const {
	return parentAlgIteration_;
}

/************************************************************************************************************/
/**
 * Allows to set the globally best known fitness
 *
 * @param bnf The best known fitness so far
 */
void GIndividual::setBestKnownFitness(const double& bnf) {
	bestPastFitness_ = bnf;
}

/************************************************************************************************************/
/**
 * Retrieves the value of the globally best known fitness
 *
 * @return The best known fitness so far
 */
double GIndividual::getBestKnownFitness() const {
	return bestPastFitness_;
}

/************************************************************************************************************/
/**
 * Allows to specify the number of optimization cycles without improvement
 *
 * @param nStalls The number of optimization cycles without improvement in the parent algorithm
 */
void GIndividual::setNStalls(const boost::uint32_t& nStalls) {
	nStalls_ = nStalls;
}

/************************************************************************************************************/
/**
 * Allows to retrieve the number of optimization cycles without improvement
 *
 * @return The number of optimization cycles without improvement in the parent algorithm
 */
boost::uint32_t GIndividual::getNStalls() const {
	return nStalls_;
}

#ifdef GENEVATESTING

/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GIndividual::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GObject::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GIndividual::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GObject::specificTestsNoFailureExpected_GUnitTests();
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GIndividual::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GObject::specificTestsFailuresExpected_GUnitTests();
}

/************************************************************************************************************/

#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */

