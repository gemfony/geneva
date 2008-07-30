/**
 * \file
 */

/**
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

#include "GMember.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(GenEvA::GMember)

namespace GenEvA {

/*******************************************************************************************/
/**
 * The standard constructor for the GMember class. There is not much
 * work to be done here, as the GMember class serves as the base class
 * for an entire hierarchy. Please note: The initial fitness of this object will be
 * set when the first fitness calculation is performed. This will automatically be
 * done, as the dirty flag is set by default.
 */
GMember::GMember(void) throw() :
	GObject(), _parentPopGeneration(0), _myCurrentFitness(0), _dirtyFlag(true),
			_isparent(false), _evaluationPermission(ALLOWEVALUATION), // by default we do lazy evaluation
			_isRoot(true) { /* nothing */
}

/*******************************************************************************************/
/**
 * Even though this class is purely virtual, we need a copy constructor, as it might
 * be called by derived classes.
 *
 * \param cp A const reference to another GMember object.
 */
GMember::GMember(const GMember& cp) throw() :
	GObject(cp), _parentPopGeneration(cp._parentPopGeneration),
			_myCurrentFitness(cp._myCurrentFitness), _dirtyFlag(cp._dirtyFlag),
			_isparent(cp._isparent), _evaluationPermission(
					cp._evaluationPermission), _isRoot(cp._isRoot) { /* nothing */
}

/*******************************************************************************************/
/**
 * The standard destructor. We have no local, dynamically allocated data,
 * hence no work needs to be done.
 */
GMember::~GMember() { /* nothing */
}

/*******************************************************************************************/
/**
 * We want to be able to reset the object to the state it had
 * upon initialisation with the default constructor. To achieve
 * this, we first reset our internal values, followed by a reset
 * of the parent object (GObject in this case),
 *
 * <b>This is the recommended procedure for all derived classes. First
 * reset our own data, then the parent object.</b> Tear down the building
 * from the top.
 *
 * Users must overload this function in each derived class. Unfortunately
 * C++ has no mechanism to enforce this.
 */
void GMember::reset(void) {
	_myCurrentFitness = 0.;
	setDirtyFlag();
	setParentPopGeneration(0);
	setIsParent(false);
	setEvaluationPermission(ALLOWEVALUATION);
	_isRoot = true;

	GObject::reset();
}

/*******************************************************************************************/
/**
 * Like the GMember::reset() function, this function needs to be
 * overloaded by the user in each derived class.
 *
 * First, we load the data of our parent object, then we load our own
 * local data. All load() functions take a pointer to a GObject object
 * as parameter, so we need to do a cast to a GMember object
 * before loading the local data.
 *
 * The reason for passing a GObject pointer to this function is that the
 * caller will usually only see a base pointer (polymorphism).
 *
 * \param gmi A pointer to a GMember object, camouflaged as a pointer to a GObject.
 */
void GMember::load(const GObject *gmi) {
	const GMember *gm_load = dynamic_cast<const GMember *> (gmi);

	// dynamic_cast will emit a NULL pointer, if the conversion failed
	if (!gm_load) {
		gls << "In GMember::load(): Conversion error!" << endl << logLevel(
				CRITICAL);

		exit(1);
	}

	// Check that this object is not accidentally assigned to itself.
	if (gm_load == this) {
		gls << "In GMember::load(): Error!" << endl
				<< "Tried to assign an object to itself." << endl << logLevel(
				CRITICAL);

		exit(1);
	}

	// Load all necessary data
	GObject::load(gmi);

	_myCurrentFitness = gm_load->_myCurrentFitness;
	_dirtyFlag = gm_load->_dirtyFlag;
	setParentPopGeneration(gm_load->_parentPopGeneration);
	setIsParent(gm_load->_isparent);
	_evaluationPermission = gm_load->_evaluationPermission;
	_isRoot = gm_load->_isRoot;
}

/*******************************************************************************************/
/**
 * A standard assignment operator for GMember objects
 *
 * \param cp A copy of another GMember object
 * \return A constant reference to this object
 */
const GMember& GMember::operator=(const GMember& cp) {
	GMember::load(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * A standard assignment operator for GMember objects, camouflaged as a GObject
 *
 * \param cp A copy of another GMember object, camouflaged as a GObject
 * \return A constant reference to this object, camouflaged as a GObject
 */
const GObject& GMember::operator=(const GObject& cp) {
	GMember::load(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * The GMember::fitness() function is a core part of the GenEvA library. The entire
 * optimization process depends on the evaluation of GMember-derivatives.
 *
 * The calculation of the fitness of a GenEvA individual will
 * usually be very costly, as the library is particularly designed
 * for very large optimization problems. The calculation of an
 * individual's fitness could potentially take hours or even days. We
 * thus have to make sure that a re-calculation of the fitness
 * <em>only</em> takes place, if the object has been changed
 * (i.e. "mutated"). This is done by setting a flag whenever changes
 * to the object's core data have occurred. The actual fitness
 * calculation is done in a user-supplied function customFitness() .
 * This function is only called, if a <em>dirty flag</em> has been
 * set.  Otherwise a previously stored fitness is returned. This
 * behaviour is the same for all objects derived from the GMember
 * class.
 *
 * In some cases it is dangerous to call customFitness() from within this
 * function. This can be the case when we run in a networked environment
 * where the server shall only do sorting and communicating.
 *
 * As a safeguard against accidental calls to customFitness(), this function
 * will emit an error and exit if fitness-recalculation is not permitted.
 *
 * \return The fitness of an object derived from this class
 */
double GMember::fitness(void) throw()
{
	if (isDirty()) {
		// It is an error to attempt evaluation in some environments
		if(_evaluationPermission == PREVENTEVALUATION) {
			gls << "In GMember::fitness() : Error! Evaluation" << endl
			<< "attempted while evaluation is not allowed" << endl
			<< logLevel(CRITICAL);

			exit(1);
		}
		else if(_evaluationPermission == ALLOWEVALUATION ||
				_evaluationPermission == ENFORCEEVALUATION) {
			try {
				_myCurrentFitness = customFitness();
				setDirtyFlag(false);
			}
			catch(std::exception& e) {
				gls << "In GMember::fitness(): Error! Caught exception " << endl
				<< "from user-supplied customFitness() function with message" << endl
				<< e.what() << endl
				<< logLevel(CRITICAL);

				exit(1);
			}
			catch(...) {
				gls << "In GMember::fitness(): Error! Caught unknown exception " << endl
				<< "from user-supplied customFitness() function." << endl
				<< logLevel(CRITICAL);

				exit(1);
			}
		}
		else {
			gls << "In GMember::fitness(): Error! Invalid value of" << endl
			<< "_evaluationPermission variable: " << _evaluationPermission << endl
			<< logLevel(CRITICAL);

			exit(1);
		}
	}

	return _myCurrentFitness;
}

/*******************************************************************************************/
/**
 * Like GMember::fitness(), GMember::mutate() is one of the most central functions
 * of the GenEvA library, and the designs of both functions are similar. The user can specify a
 * function customMutate() by deriving a class directly or indirectly from GMember and overloading
 * the existing customMutate() function. Note that, while this is a common method of specifying
 * the mutation scheme, predefined functions could be used that call a program on disk or execute
 * a script to determine the fitness of an individual.
 *
 * When GMember::mutate() is called, customMutate() is executed first. As the underlying object
 * has been modified, the dirty flag is set. When the GMember::fitness() function is called the next
 * time, it detects that the GMember object is "dirty" and initiates a re-calculation of the fitness.
 *
 * In some cases this "lazy evaluation" is not desirable. E.g., it might be necessary to protect
 * server in a network from heavy work loads, so it can still serve other clients. In these cases
 * it is possible to set the GMember::_evaluationPermission to ENFORCEEVALUATION, and the calculation
 * of the object's fitness will happen immediately after the mutation.
 */
void GMember::mutate(void) throw()
{
	try {
		customMutate();
		GMember::setDirtyFlag();
	}
	catch(std::exception& e) {
		gls << "In GMember::mutate(): Error! Caught exception " << endl
		<< "from user-supplied customMutate() function with message" << endl
		<< e.what() << endl
		<< logLevel(CRITICAL);

		exit(1);
	}
	catch(...) {
		gls << "In GMember::mutate(): Error! Caught unknown exception " << endl
		<< "from user-supplied customMutate() function." << endl
		<< logLevel(CRITICAL);

		exit(1);
	}

	// In some cases we want to enforce immediate fitness recalculation
	// fitness() also sets the dirtyFlag to false
	if(_evaluationPermission == ENFORCEEVALUATION) _myCurrentFitness = fitness();
}

/*******************************************************************************************/
/**
 * This function assembles a report about the classes internal state and then calls the
 * parent classes assembleReport() function.
 *
 * \param indention The number of white spaces in front of each output line
 * \return A string containing a report about the inner state of the object
 */
string GMember::assembleReport(uint16_t indention) const
{
	ostringstream oss;
	oss << ws(indention) << "GMember: " << this << endl
	<< ws(indention) << "is a " << (isParent()?"parent":"child") << endl
	<< ws(indention) << "in parent generation " << getParentPopGeneration() << "." << endl
	<< ws(indention) << "_evaluationPermission has value " << _evaluationPermission << endl
	<< ws(indention) << "Last known fitness is " << getMyCurrentFitness() << (isDirty()?", but object is dirty!":", and object is clean.") << endl
	<< ws(indention) << "This GMember object is " << (_isRoot?"at the top of the hierarchy":"part of the hierarchy") << endl
	<< ws(indention) << "-----> Report from parent class GObject : " << endl
	<< GObject::assembleReport(indention+NINDENTION) << endl;

	return oss.str();
}

/*******************************************************************************************/
/**
 * See the GMember::getDirtyFlag() function for information.
 *
 * \return A boolean indicating whether the object is dirty or not
 */
bool GMember::isDirty(void) const throw() {
	return getDirtyFlag();
}

/*******************************************************************************************/
/**
 * With this function you can find out whether your class has been modified (i.e. is "dirty")
 * or not. If so, its fitness will be recalculated when the GMember::fitness() function is called
 * next time. Note that this can potentially be costly. A return value of "true" means "dirty",
 * "false" means "clean".
 *
 * \return The value of the <em>dirty flag</em>.
 */
bool GMember::getDirtyFlag(void) const throw() {
	return _dirtyFlag;
}

/*******************************************************************************************/
/**
 * This public function lets you set the dirty flag to "true". Users can not set it
 * to "false".
 */
void GMember::setDirtyFlag(void) {
	// This is a private function, only accessible to the GMember class.
	setDirtyFlag(true);
}

/*******************************************************************************************/
/**
 * Intended to recursively set the dirty flags of all members stored in this object,
 * if any. This function is used for populations and containers of members, which are
 * derived from the GMember class. The function needs to be overloaded for these classes.
 * The default behaviour is the same as for the GMember::setDirtyFlag() function.
 */
void GMember::setDirtyFlagAll(void) {
	GMember::setDirtyFlag();
}

/*******************************************************************************************/
/**
 * This private function simply sets the dirty flag to any desired value.
 *
 * \param  The new value of the <em>dirty flag</em>.
 */
void GMember::setDirtyFlag(bool dirtyFlag) throw()
{
	_dirtyFlag = dirtyFlag;
}

/*******************************************************************************************/
/**
 * This function gives access to our current internal fitness. Please note that this
 * is not necessarily the correct value, as the object could be "dirty" (i.e. it
 * has been modified). In this case you would only get the correct value after the
 * next fitness recalculation. Use GMember::fitness() to always get access to the "real"
 * fitness of an object. But please note that this might trigger potentially costly
 * calculations!
 *
 * \return The current internal fitness of this object.
 */
double GMember::getMyCurrentFitness() const throw() {
	return _myCurrentFitness;
}

/*******************************************************************************************/
/**
 * It can be useful for a class derived from GMember to know what the current generation
 * of its parent population (i.e. the population it is stored in) is. This function
 * allows that population to set that value. As an example, information about the current
 * generation could be used to let an object emit information every 20 generations.
 *
 * \param The current generation of the parent population
 */
void GMember::setParentPopGeneration(uint32_t parentPopGeneration) throw()
{
	_parentPopGeneration = parentPopGeneration;
}

/*******************************************************************************************/
/**
 * If a parent population (i.e. the population this GMember is stored in) has set the currently
 * active generation in this object, its value can be retrieved with this function. Otherwise
 * the function will only return the default value of this variable.
 *
 * \return The currently active generation of the parent population
 */
uint32_t GMember::getParentPopGeneration(void) const throw()
{
	return _parentPopGeneration;
}

/*******************************************************************************************/
/**
 * This function allows to check whether this GMember object (or an object derived from it)
 * acts in a parent capacity, i.e. is a parent of the current population. This requires that
 * the parent population has set the corresponding value with GMember::setIsParent(bool) .
 *
 * \return A boolean value indicating whether this object currently acts in a parent capacity
 */
bool GMember::isParent(void) const throw()
{
	return _isparent;
}

/*******************************************************************************************/
/**
 * This function allows a parent population (i.e. the population this object is stored in)
 * to specify whether this object acts in a parent capacity. See also GMember::isParent(void) .
 *
 * \param isparent Specifies whether this object acts in a parent capacity or not
 */
void GMember::setIsParent(bool isparent) throw()
{
	_isparent = isparent;
}

/*******************************************************************************************/
/**
 * Retrieves the _evaluationPermission parameter.
 *
 * \return The value of the _evaluationPermission parameter
 */
bool GMember::getEvaluationPermission(void) const throw()
{
	return _evaluationPermission;
}

/*******************************************************************************************/
/**
 * Sets the _evaluationPermission parameter to a given value and returns the original value
 *
 * \param ep The new value for the _evaluationPermission parameter
 * \return The original value of the _evaluationPermission parameter
 */
uint8_t GMember::setEvaluationPermission(uint8_t ep) {
	if(ep> 2) { // We only have three levels 0..2
		gls << "In GMember::setEvaluationPermission(): Error!" << endl
		<< "Tried to pass invald parameter ep=" << ep << " !" << endl
		<< " Must be 0,1 or 2" << endl
		<< logLevel(CRITICAL);

		exit(1);
	}

	uint8_t originalValue = _evaluationPermission;
	_evaluationPermission = ep;
	return originalValue;
}

/*******************************************************************************************/
/**
 * Specifies that this GMember is not at the root of the hierarchy. The _isRoot variable is set to true
 * by default for all GMember objects. A population sets the parameter to false for all items contained in it.
 * If these are containers for GMember objects themselves, they recursively set the parameter to false
 * for the items contained in them. This function is thus overloaded in GManagedMemberCollection. In the
 * end there is only a single GMember object for which _isRoot is set to true - the population at the
 * root of the hierarchy.
 */
void GMember::setIsNotRoot(void) {
	_isRoot = false;
}

/*******************************************************************************************/
/**
 * Retrieves the _isRoot parameter. It specifies, whether this GMember object is at the root
 * of the hierarchy.
 *
 * \return The _isRoot parameter
 */
bool GMember::isRoot(void) const throw()
{
	return _isRoot;
}

/*******************************************************************************************/

} /* namespace GenEvA */
