/**
 * @file GParameterBase.cpp
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

#include "GParameterBase.hpp"

namespace Gem {
namespace GenEvA {

/**********************************************************************************/
/**
 * The default constructor. mutations are switched on by default.
 */
GParameterBase::GParameterBase():
	GMutableI(),
	GObject(),
	mutationsActive_(true)
{ /* nothing */ }

/**********************************************************************************/
/**
 * The standard copy constructor.
 *
 * @param cp A copy of another GParameterBase object
 */
GParameterBase::GParameterBase(const GParameterBase& cp)
	:GMutableI(cp),
	 GObject(cp),
	 mutationsActive_(cp.mutationsActive_)
{ /* nothing */ }

/**********************************************************************************/
/**
 * The standard destructor. No local data, hence nothing to do.
 */
GParameterBase::~GParameterBase()
{ /* nothing */ }

/**********************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GParameterBase object, camouflaged as a GObject
 */
void GParameterBase::load(const GObject* cp){
	// Convert cp into local format
	const GParameterBase *gpb = conversion_cast(cp, this);

	// Load the parent class'es data
	GObject::load(cp);

	// Load local data
	mutationsActive_ = gpb->mutationsActive_;
}

/**********************************************************************************/
/**
 * Calls the function that does the actual mutation (which is in turn implemented
 * by derived classes. Will omit mutation if the mutationsActive_ parameter is set.
 */
void GParameterBase::mutate() {
	if(mutationsActive_) this->mutateImpl();
}

/**********************************************************************************/
/**
 * Switches on mutations for this object
 */
void GParameterBase::setMutationsActive() {
	mutationsActive_ = true;
}

/**********************************************************************************/
/**
 * Disables mutations for this object
 */
void GParameterBase::setMutationsInactive() {
	mutationsActive_ = false;
}

/**********************************************************************************/
/**
 * Determines whether mutations are performed for this object
 *
 * @return A boolean indicating whether mutations are performed for this object
 */
bool GParameterBase::mutationsActive() const {
	return mutationsActive_;
}

/**********************************************************************************/
/**
 * Checks for equality with another GParameterBase object
 *
 * @param  cp A constant reference to another GParameterBase object
 * @return A boolean indicating whether both objects are equal
 */
bool GParameterBase::operator==(const GParameterBase& cp) const {
	return GParameterBase::isEqualTo(cp, boost::logic::indeterminate);
}

/**********************************************************************************/
/**
 * Checks for inequality with another GParameterBase object
 *
 * @param  cp A constant reference to another GParameterBase object
 * @return A boolean indicating whether both objects are inequal
 */
bool GParameterBase::operator!=(const GParameterBase& cp) const {
	return !GParameterBase::isEqualTo(cp, boost::logic::indeterminate);
}

/**********************************************************************************/
/**
 * Checks for equality with another GParameterBase object. As we have no
 * local data, we just check for equality of the parent class.
 *
 * @param  cp A constant reference to another GParameterBase object
 * @return A boolean indicating whether both objects are equal
 */
bool GParameterBase::isEqualTo(const GObject& cp, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterBase reference
	const GParameterBase *gpb_load = GObject::conversion_cast(&cp,  this);

	// Check our parent class
	if(!GObject::isEqualTo(*gpb_load, expected)) return  false;

	// Check the local data
	if(checkForInequality("GParameterBase", mutationsActive_, gpb_load->mutationsActive_,"mutationsActive_", "gpb_load->mutationsActive_", expected)) return false;

	return true;
}

/**********************************************************************************/
/**
 * Checks for similarity with another GParameterBase object. As we have
 * no local data, we just check for similarity of the parent class.
 *
 * @param  cp A constant reference to another GParameterBase object
 * @param limit A double value specifying the acceptable level of differences of floating point values
 * @return A boolean indicating whether both objects are similar to each other
 */
bool GParameterBase::isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterBase reference
	const GParameterBase *gpb_load = GObject::conversion_cast(&cp,  this);

	// Check our parent class
	if(!GObject::isSimilarTo(*gpb_load, limit, expected)) 	return false;

	// Check the local data
	if(checkForDissimilarity("GParameterBase", mutationsActive_, gpb_load->mutationsActive_, limit, "mutationsActive_", "gpb_load->mutationsActive_", expected)) return false;


	return true;
}

/**********************************************************************************/
/**
 * Convenience function so we do not need to always cast  derived classes.
 * See GParameterBaseWithAdaptors::hasAdaptors() for the "real"
 * function.
 */
bool GParameterBase::hasAdaptor() const {
	return false;
}

/**********************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
