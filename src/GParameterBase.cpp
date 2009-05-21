/**
 * @file GParameterBase.cpp
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

#include "GParameterBase.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GParameterBase)

namespace Gem {
namespace GenEvA {

/**********************************************************************************/
/**
 * The default constructor. No local data, hence nothing to do.
 */
GParameterBase::GParameterBase()  :GMutableI(), GObject()
{ /* nothing */ }

/**********************************************************************************/
/**
 * The standard copy constructor. No local data, hence nothing to do.
 *
 * @param cp A copy of another GParameterBase object
 */
GParameterBase::GParameterBase(const GParameterBase& cp)
	:GMutableI(cp),
	 GObject(cp)
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
	// Convert to local format. No local data - we can thus use the faster static_cast
	const GParameterBase *gpb_load = static_cast<const GParameterBase *> (cp);

	// Check that this object is not accidentally assigned to itself.
	if (gpb_load == this) {
		std::ostringstream error;
		error << "In GParameterBase::load(): Error!" << std::endl
			  << "Tried to assign an object to itself." << std::endl;

		throw geneva_error_condition(error.str());
	}

	// Load the parent class'es data
	GObject::load(cp);
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

	// No local data

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

	// No local data

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
