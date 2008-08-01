/**
 * @file
 */

/* GIndividual.cpp
 *
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
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
 * Resets the class to its initial state
 */
void GIndividual::reset() {
	// First reset our local data
	currentFitness_ = 0.;
	dirtyFlag_ = true;
	allowLazyEvaluation_ = false;
	parentPopGeneration_ = 0;
	parentCounter_ = 0;

	// Then the data of our parents
	GObject::reset();
}

/**********************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GIndividual object, camouflaged as a GObject
 */
void GIndividual::load(const GObject* cp) {
	const GIndividual *gi_load = this->checkedConversion(cp, this);

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
inline bool GIndividual::setIsParent(const bool& isParent) throw() {
	bool previous = parentCounter_>0?true:false;
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
inline bool GIndividual::isParent() const throw() {
	return (parentCounter_ > 0)?true:false;
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

}
} /* namespace Gem::GenEvA */

