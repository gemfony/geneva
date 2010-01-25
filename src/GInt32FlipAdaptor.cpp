/**
 * @file GInt32FlipAdaptor.cpp
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

#include "GInt32FlipAdaptor.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GInt32FlipAdaptor)

namespace Gem {
namespace GenEvA {

/*******************************************************************************************/
/**
 * The default constructor
 */
GInt32FlipAdaptor::GInt32FlipAdaptor()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GInt32FlipAdaptor object
 */
GInt32FlipAdaptor::GInt32FlipAdaptor(const GInt32FlipAdaptor& cp)
	: GIntFlipAdaptorT<boost::int32_t>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization with a mutation probability
 *
 * @param mutProb The mutation probability
 */
GInt32FlipAdaptor::GInt32FlipAdaptor(const double& mutProb)
	: GIntFlipAdaptorT<boost::int32_t>(mutProb)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GInt32FlipAdaptor::~GInt32FlipAdaptor()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GInt32FlipAdaptor object
 * @return A constant reference to this object
 */
const GInt32FlipAdaptor& GInt32FlipAdaptor::operator=(const GInt32FlipAdaptor& cp){
	GInt32FlipAdaptor::load(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GInt32FlipAdaptor::clone_() const {
	return new GInt32FlipAdaptor(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GInt32FlipAdaptor object
 *
 * @param  cp A constant reference to another GInt32FlipAdaptor object
 * @return A boolean indicating whether both objects are equal
 */
bool GInt32FlipAdaptor::operator==(const GInt32FlipAdaptor& cp) const {
	return GInt32FlipAdaptor::isEqualTo(cp, boost::logic::indeterminate);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GInt32FlipAdaptor object
 *
 * @param  cp A constant reference to another GInt32FlipAdaptor object
 * @return A boolean indicating whether both objects are inequal
 */
bool GInt32FlipAdaptor::operator!=(const GInt32FlipAdaptor& cp) const {
	return !GInt32FlipAdaptor::isEqualTo(cp, boost::logic::indeterminate);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GInt32FlipAdaptor object.  If T is an object type,
 * then it must implement operator!= .
 *
 * @param  cp A constant reference to another GInt32FlipAdaptor object
 * @return A boolean indicating whether both objects are equal
 */
bool GInt32FlipAdaptor::isEqualTo(const GObject& cp, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterT reference
	const GInt32FlipAdaptor *p_load = GObject::conversion_cast(&cp,  this);

	// Check equality of the parent class
	if(!GIntFlipAdaptorT<boost::int32_t>::isEqualTo(*p_load, expected)) return false;

	// No local data

	return true;
}

/*******************************************************************************************/
/**
 * Checks for similarity with another GInt32FlipAdaptor object.
 *
 * @param  cp A constant reference to another GInt32FlipAdaptor object
 * @param limit A double value specifying the acceptable level of differences of floating point values
 * @return A boolean indicating whether both objects are similar to each other
 */
bool GInt32FlipAdaptor::isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterT reference
	const GInt32FlipAdaptor *p_load = GObject::conversion_cast(&cp,  this);

	// Check similarity of the parent class
	if(!GIntFlipAdaptorT<boost::int32_t>::isSimilarTo(*p_load, limit, expected)) return false;

	// No local data

	return true;
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GInt32FlipAdaptor object, camouflaged as a GObject
 */
void GInt32FlipAdaptor::load(const GObject* cp){
	// Convert cp into local format (also checks for the type of cp)
	const GInt32FlipAdaptor *p_load = GObject::conversion_cast(cp, this);

	// Load our parent class'es data ...
	GIntFlipAdaptorT<boost::int32_t>::load(cp);

	// ... no local data
}

/*******************************************************************************************/
/**
 * Retrieves the id of this adaptor
 *
 * @return The id of this adaptor
 */
Gem::GenEvA::adaptorId GInt32FlipAdaptor::getAdaptorId() const {
	return Gem::GenEvA::GINT32FLIPADAPTOR;
}

/*******************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
