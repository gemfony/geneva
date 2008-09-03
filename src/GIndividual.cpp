/**
 * @file GIndividual.cpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
 */

#include "GIndividual.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GIndividual)

namespace Gem {
namespace GenEvA {

/**********************************************************************************/
/**
 * The default constructor.
 */
GIndividual::GIndividual() :
	GMutableI(),
	GRateableI(),
	GObject(),
	currentFitness_(0.),
	dirtyFlag_(true),
	allowLazyEvaluation_(false),
	parentPopGeneration_(0),
	parentCounter_(0)
{ /* nothing */ }

/**********************************************************************************/
/**
 * The standard copy constructor.
 *
 * @param cp A copy of another GIndividual object
 */
GIndividual::GIndividual(const GIndividual& cp) :
	GMutableI(cp),
	GRateableI(cp),
	GObject(cp),
	currentFitness_(cp.currentFitness_),
	dirtyFlag_(cp.dirtyFlag_),
	allowLazyEvaluation_(cp.allowLazyEvaluation_),
	parentPopGeneration_(cp.parentPopGeneration_),
	parentCounter_(cp.parentCounter_)
{ /* nothing */ }

/**********************************************************************************/
/**
 * The standard destructor.
 */
GIndividual::~GIndividual() { /* nothing */ }

/**********************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GIndividual object, camouflaged as a GObject
 */
void GIndividual::load(const GObject* cp) {
	const GIndividual *gi_load = this->conversion_cast(cp, this);

	// Load the parent class'es data
	GObject::load(cp);

	// Then load our local data
	currentFitness_ = gi_load->currentFitness_;
	dirtyFlag_ = gi_load->dirtyFlag_;
	allowLazyEvaluation_ = gi_load->allowLazyEvaluation_;
	parentPopGeneration_ = gi_load->parentPopGeneration_;
	parentCounter_ = gi_load->parentCounter_;
}

/**********************************************************************************/
/**
 * The actual fitness calculation is triggered by this function in derived
 * classes. The function is not intended to be called directly on a GIndividual.
 * However, in order to allow (de-)serialization, GIndividual must be instantiable.
 * Hence we cannot make the class abstract.
 *
 * @return A 0 value - this function is not intended to be called
 */
double GIndividual::fitnessCalculation(){
	std::ostringstream error;
	error << "In GIndividual::fitnessCalculation(): Error" << std::endl
		  << "This function should never have been called directly!" << std::endl;

	LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

	throw geneva_function_called_erroneously() << error_string(error.str());

	return 0; // Make the compiler happy
}

/**********************************************************************************/
/**
 * The actual mutation operation takes place in derived classes. The function is
 * not intended to be called directly on a GIndividual. However, in order to allow
 * (de-)serialization, GIndividual must be instantiable. Hence we cannot make the
 * class abstract.
 */
void GIndividual::customMutations(){
	std::ostringstream error;
	error << "In GIndividual::customMutations(): Error" << std::endl
		  << "This function should never have been called directly!" << std::endl;

	LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

	throw geneva_function_called_erroneously() << error_string(error.str());
}

/**********************************************************************************/
/**
 * The mutation interface. If lazy evaluation is not allowed (the default), this
 * function also triggers the re-calculation of the fitness.
 */
void GIndividual::mutate() {
	customMutations();
	GIndividual::setDirtyFlag(true);

	// In some cases we want to allow lazy evaluation. The
	// fitness calculation is then deferred to the next call
	// of the GIndividual::fitness() function
	if (!allowLazyEvaluation_) {
		currentFitness_ = fitnessCalculation();
		setDirtyFlag(false);
	}
}

/**********************************************************************************/
/**
 * Returns the last known fitness calculation of this object. Re-calculation
 * of the fitness is triggered, if lazy evaluation is allowed.
 *
 * @return The fitness of this individual
 */
double GIndividual::fitness() {
	if (dirtyFlag_) {
		// Except for generation 0, it is an error if lazy evaluation is not allowed, but
		// the dirty flag is set. There the fitness calculation of parents happens before
		// the mutations
		if (!allowLazyEvaluation_ && parentPopGeneration_>0) {
			std::ostringstream error;
			error << "In GIndividual::fitness(): Error!" << std::endl
					<< "The dirty flag is set while lazy evaluation is not allowed."
					<< std::endl;

			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

			throw geneva_dirtyflag_set_lazyevaluation_not() << error_string(
					error.str());
		}

		currentFitness_ = fitnessCalculation();
		setDirtyFlag(false);
	}

	return currentFitness_;
}

/**********************************************************************************/
/**
 * Retrieves the current (not necessarily up-to-date) fitness
 *
 * @param dirtyFlag The value of the dirtyFlag_ variable
 * @return The current fitness value (not necessarily up-to-date)
 */
double GIndividual::getCurrentFitness(bool& dirtyFlag) const throw() {
	dirtyFlag = dirtyFlag_;
	return currentFitness_;
}

/**********************************************************************************/
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

/**********************************************************************************/
/**
 * Indicates whether lazy evaluation is allowed
 *
 * @param allowLazyEvaluation The new value for the allowLazyEvaluation_ parameter
 * @return The previous value of the allowLazyEvaluation_ parameter
 */
bool GIndividual::setAllowLazyEvaluation(const bool& allowLazyEvaluation) throw() {
	bool previous = allowLazyEvaluation_;
	allowLazyEvaluation_ = allowLazyEvaluation;
	return previous;
}

/**********************************************************************************/
/**
 * Retrieve the allowLazyEvaluation_ parameter
 *
 * @return The value of the allowLazyEvaluation_ parameter
 */
bool GIndividual::getAllowLazyEvaluation(void) const throw() {
	return allowLazyEvaluation_;
}

/**********************************************************************************/
/**
 * Sets the generation of the population this individual is in
 *
 * @param parenPopGeneration The generation of the parent population
 */
void GIndividual::setParentPopGeneration(const boost::uint32_t& parentPopGeneration) throw() {
	parentPopGeneration_ = parentPopGeneration;
}

/**********************************************************************************/
/**
 * Retrieves the parentPopGeneration_ parameter
 *
 * @return The current generation of the parent population
 */
boost::uint32_t GIndividual::getParentPopGeneration() const throw() {
	return parentPopGeneration_;
}

/**********************************************************************************/
/**
 * Sets the parentCounter_ parameter. This is a counter which lets us find out
 * how often this object has become a parent in one go. Become parent frequently
 * could indicate that the mutations need to cover a wider area, as the
 * evolution has stalled.
 *
 * @param isParent Indicates whether or not this is a parent individual
 * @return A boolean indicating the previous situation
 */
bool GIndividual::setIsParent(const bool& isParent) throw() {
	bool previous=(parentCounter_>0)?true:false;

	if(isParent) parentCounter_++;
	else parentCounter_ = 0;

	return previous;
}

/**********************************************************************************/
/**
 * Marks an individual as a parent
 *
 * @return A boolean indicating the previous situation before calling this function
 */
bool GIndividual::setIsParent(void){
	return setIsParent(true);
}

/**********************************************************************************/
/**
 * Marks an individual as a child.
 *
 * @return A boolean indicating the previous situation before calling this function
 */
bool GIndividual::setIsChild(void){
	return setIsParent(false);
}

/**********************************************************************************/
/**
 * Checks whether this is a parent individual
 *
 * @return A boolean indicating whether this object is a parent at this time
 */
bool GIndividual::isParent() const throw() {
	return (parentCounter_>0)?true:false;
}

/**********************************************************************************/
/**
 * Checks whether the dirty flag is set
 *
 * @return The value of the dirtyFlag_ variable
 */
bool GIndividual::isDirty() const throw() {
	return dirtyFlag_;
}

/**********************************************************************************/
/**
 * Sets the dirtyFlag_
 */
void GIndividual::setDirtyFlag() throw() {
	dirtyFlag_ = true;
}

/**********************************************************************************/
/**
 * Sets the dirtyFlag_ to any desired value
 *
 * @param dirtyFlag The new value for the dirtyFlag_ variable
 * @return The previous value of the dirtyFlag_ variable
 */
bool GIndividual::setDirtyFlag(const bool& dirtyFlag) throw() {
	bool previous = dirtyFlag_;
	dirtyFlag_ = dirtyFlag;
	return previous;
}

/**********************************************************************************/
/**
 * Retrieves the parentCounter_ variable
 *
 * @return The current value of the parentCounter_ variable
 */
boost::uint32_t GIndividual::getParentCounter() const throw(){
	return parentCounter_;
}

/**********************************************************************************/
/**
 * Adds or sets an attribute in the object. Note that, if the attribute wasn't
 * available so far, std::map() will create a corresponding entry. If the entry
 * was available, the old setting will be returned by the function.
 *
 * @param key A key to be assigned to the attribute
 * @param attribute The actual attribute
 * @return The previous entry for this key, if any
 */
std::string GIndividual::setAttribute(const std::string& key, const std::string& attribute){
	std::string previous;

	if(attributeTable_.find(key) != attributeTable_.end())	previous = attributeTable_[key];
	attributeTable_[key]=attribute;

	return previous;
}

/**********************************************************************************/
/**
 * Retrieves an attribute from the object. An empty string will be returned if no
 * attribute with this key is available.
 *
 * @param key The key for the attribute.
 * @return The attribute associated with key (or an empty string, if this key doesn't exist)
 */
std::string GIndividual::getAttribute(const std::string& key){
	std::string result;
	if(attributeTable_.find(key) != attributeTable_.end()) result = attributeTable_[key];
    return result;
}

/**********************************************************************************/
/**
 * Removes an attribute from the object, if the corresponding key exists in the map. An
 * empty string will be returned, if this key doesn't exist-
 *
 * @param key The key of the attribute to be removed
 * @return The value of the attribute that has been removed (or an empty string, if this key doesn't exist)
 */
std::string GIndividual::delAttribute(const std::string& key){
	std::string previous;

	if(attributeTable_.find(key) != attributeTable_.end()) {
		previous = attributeTable_[key];
		attributeTable_.erase(key);
	}

	return previous;
}

/**********************************************************************************/
/**
 * Clears the attribute table.
 */
void GIndividual::clearAttributes() {
	attributeTable_.clear();
}

/**********************************************************************************/
/**
 * A version of the fitness framework that also checks for
 * exceptions. To be used when fitness() is to become the main
 * function to be called by a thread.
 */
double GIndividual::checkedFitness(){
	try{
		return this->fitness();
	}
	catch(std::exception& e){
		std::ostringstream error;
		error << "In GIndividual::checkedFitness(): Caught std::exception with message" << std::endl
		      << e.what() << std::endl;
		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

		std::terminate();
	}
	catch(boost::exception& e){
		std::ostringstream error;
		error << "In GIndividual::checkedFitness(): Caught boost::exception with message" << std::endl
		      << e.diagnostic_information() << std::endl;
		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

		std::terminate();
	}
	catch(...){
		std::ostringstream error;
		error << "In GIndividual::checkedFitness(): Caught unknown exception" << std::endl;
		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

		std::terminate();
	}
}

/**********************************************************************************/
/**
 * Performs all necessary processing steps for this object. Not meant to be
 * called from threads, as no exceptions are caught. Use checkedProcess() instead.
 */
void GIndividual::process(){
	bool previous=this->setAllowLazyEvaluation(false);
	if(this->getAttribute("command") == "mutate") this->mutate();
	else if(this->getAttribute("command") == "evaluate") this->fitness();
	else {
		std::ostringstream error;
		error << "In GIndividual::process(): Unknown command:\""
			  << this->getAttribute("command") << "\"" << std::endl;

		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

		throw geneva_unknown_command() << error_string(error.str());
	}
	this->setAllowLazyEvaluation(previous);
}

/**********************************************************************************/
/**
 * Performs all necessary processing steps for this object and catches all exceptions.
 * Meant to be called by threads.
 */
void GIndividual::checkedProcess(){
	try{
		return this->process();
	}
	catch(std::exception& e){
		std::ostringstream error;
		error << "In GIndividual::checkedProcess(): Caught std::exception with message" << std::endl
		      << e.what() << std::endl;
		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

		std::terminate();
	}
	catch(boost::exception& e){
		std::ostringstream error;
		error << "In GIndividual::checkedProcess(): Caught boost::exception with message" << std::endl
		      << e.diagnostic_information() << std::endl;
		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

		std::terminate();
	}
	catch(...){
		std::ostringstream error;
		error << "In GIndividual::checkedProcess(): Caught unknown exception" << std::endl;
		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

		std::terminate();
	}
}

/**********************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

