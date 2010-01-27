/**
 * @file GBooleanAdaptor.cpp
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

#include "GBooleanAdaptor.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GBooleanAdaptor)

namespace Gem {
namespace GenEvA {

/*******************************************************************************************/
/**
 * The default constructor
 */
GBooleanAdaptor::GBooleanAdaptor()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GBooleanAdaptor object
 */
GBooleanAdaptor::GBooleanAdaptor(const GBooleanAdaptor& cp)
	: GIntFlipAdaptorT<bool>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization with a mutation probability
 *
 * @param mutProb The mutation probability
 */
GBooleanAdaptor::GBooleanAdaptor(const double& mutProb)
	: GIntFlipAdaptorT<bool>(mutProb)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GBooleanAdaptor::~GBooleanAdaptor()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GBooleanAdaptor object
 * @return A constant reference to this object
 */
const GBooleanAdaptor& GBooleanAdaptor::operator=(const GBooleanAdaptor& cp){
	GBooleanAdaptor::load(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GBooleanAdaptor::clone_() const {
	return new GBooleanAdaptor(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GBooleanAdaptor object
 *
 * @param  cp A constant reference to another GBooleanAdaptor object
 * @return A boolean indicating whether both objects are equal
 */
bool GBooleanAdaptor::operator==(const GBooleanAdaptor& cp) const {
	return GBooleanAdaptor::isEqualTo(cp, boost::logic::indeterminate);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GBooleanAdaptor object
 *
 * @param  cp A constant reference to another GBooleanAdaptor object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBooleanAdaptor::operator!=(const GBooleanAdaptor& cp) const {
	return !GBooleanAdaptor::isEqualTo(cp, boost::logic::indeterminate);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GBooleanAdaptor object.  If T is an object type,
 * then it must implement operator!= .
 *
 * @param  cp A constant reference to another GBooleanAdaptor object
 * @return A boolean indicating whether both objects are equal
 */
bool GBooleanAdaptor::isEqualTo(const GObject& cp, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterT reference
	const GBooleanAdaptor *p_load = GObject::conversion_cast(&cp,  this);

	// Check equality of the parent class
	if(!GIntFlipAdaptorT<bool>::isEqualTo(*p_load, expected)) return false;

	// No local data

	return true;
}

/*******************************************************************************************/
/**
 * Checks for similarity with another GBooleanAdaptor object.
 *
 * @param  cp A constant reference to another GBooleanAdaptor object
 * @param limit A double value specifying the acceptable level of differences of floating point values
 * @return A boolean indicating whether both objects are similar to each other
 */
bool GBooleanAdaptor::isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterT reference
	const GBooleanAdaptor *p_load = GObject::conversion_cast(&cp,  this);

	// Check similarity of the parent class
	if(!GIntFlipAdaptorT<bool>::isSimilarTo(*p_load, limit, expected)) return false;

	// No local data

	return true;
}

/*******************************************************************************************/
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
boost::optional<std::string> GBooleanAdaptor::checkRelationshipWith(const GObject& cp,
		const Gem::Util::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Util;
    using namespace Gem::Util::POD;

	// Check that we are indeed dealing with a GParamterBase reference
	const GBooleanAdaptor  *p_load = GObject::conversion_cast(&cp,  this);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GIntFlipAdaptorT<bool>::checkRelationshipWith(cp, e, limit, "GBooleanAdaptor", y_name, withMessages));

	// no local data ...

	return POD::evaluateDiscrepancies("GBooleanAdaptor", caller, deviations, e);
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GBooleanAdaptor object, camouflaged as a GObject
 */
void GBooleanAdaptor::load(const GObject* cp){
	// Convert cp into local format (also checks for the type of cp)
	const GBooleanAdaptor *p_load = GObject::conversion_cast(cp, this);

	// Load our parent class'es data ...
	GIntFlipAdaptorT<bool>::load(cp);

	// ... no local data
}

/*******************************************************************************************/
/**
 * Retrieves the id of this adaptor
 *
 * @return The id of this adaptor
 */
Gem::GenEvA::adaptorId GBooleanAdaptor::getAdaptorId() const {
	return Gem::GenEvA::GBOOLEANADAPTOR;
}

/*******************************************************************************************/
/**
 * The actual mutation logic
 *
 * @param value The parameter to be mutated
 */
void GBooleanAdaptor::customMutations(bool& value) {
	value==true?value=false:value=true;
}

/*******************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
