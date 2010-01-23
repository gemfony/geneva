/**
 * @file GCharFlipAdaptor.cpp
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

#include "GCharFlipAdaptor.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GCharFlipAdaptor)

namespace Gem {
namespace GenEvA {

/*******************************************************************************************/
/**
 * The default constructor
 */
GCharFlipAdaptor::GCharFlipAdaptor()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GCharFlipAdaptor object
 */
GCharFlipAdaptor::GCharFlipAdaptor(const GCharFlipAdaptor& cp)
	: GIntFlipAdaptorT<char>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization with a mutation probability
 *
 * @param mutProb The mutation probability
 */
GCharFlipAdaptor::GCharFlipAdaptor(const double& mutProb)
	: GIntFlipAdaptorT<char>(mutProb)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GCharFlipAdaptor::~GCharFlipAdaptor()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GCharFlipAdaptor object
 * @return A constant reference to this object
 */
const GCharFlipAdaptor& GCharFlipAdaptor::operator=(const GCharFlipAdaptor& cp){
	GCharFlipAdaptor::load(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GCharFlipAdaptor::clone() const {
	return new GCharFlipAdaptor(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GCharFlipAdaptor object
 *
 * @param  cp A constant reference to another GCharFlipAdaptor object
 * @return A boolean indicating whether both objects are equal
 */
bool GCharFlipAdaptor::operator==(const GCharFlipAdaptor& cp) const {
	return GCharFlipAdaptor::isEqualTo(cp, boost::logic::indeterminate);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GCharFlipAdaptor object
 *
 * @param  cp A constant reference to another GCharFlipAdaptor object
 * @return A boolean indicating whether both objects are inequal
 */
bool GCharFlipAdaptor::operator!=(const GCharFlipAdaptor& cp) const {
	return !GCharFlipAdaptor::isEqualTo(cp, boost::logic::indeterminate);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GCharFlipAdaptor object.  If T is an object type,
 * then it must implement operator!= .
 *
 * @param  cp A constant reference to another GCharFlipAdaptor object
 * @return A boolean indicating whether both objects are equal
 */
bool GCharFlipAdaptor::isEqualTo(const GObject& cp, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterT reference
	const GCharFlipAdaptor *p_load = GObject::conversion_cast(&cp,  this);

	// Check equality of the parent class
	if(!GIntFlipAdaptorT<char>::isEqualTo(*p_load, expected)) return false;

	// No local data

	return true;
}

/*******************************************************************************************/
/**
 * Checks for similarity with another GCharFlipAdaptor object.
 *
 * @param  cp A constant reference to another GCharFlipAdaptor object
 * @param limit A double value specifying the acceptable level of differences of floating point values
 * @return A boolean indicating whether both objects are similar to each other
 */
bool GCharFlipAdaptor::isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterT reference
	const GCharFlipAdaptor *p_load = GObject::conversion_cast(&cp,  this);

	// Check similarity of the parent class
	if(!GIntFlipAdaptorT<char>::isSimilarTo(*p_load, limit, expected)) return false;

	// No local data

	return true;
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GCharFlipAdaptor object, camouflaged as a GObject
 */
void GCharFlipAdaptor::load(const GObject* cp){
	// Convert cp into local format (also checks for the type of cp)
	const GCharFlipAdaptor *p_load = GObject::conversion_cast(cp, this);

	// Load our parent class'es data ...
	GIntFlipAdaptorT<char>::load(cp);

	// ... no local data
}

/*******************************************************************************************/
/**
 * Retrieves the id of this adaptor
 *
 * @return The id of this adaptor
 */
Gem::GenEvA::adaptorId GCharFlipAdaptor::getAdaptorId() const {
	return Gem::GenEvA::GCHARFLIPADAPTOR;
}

/*******************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
