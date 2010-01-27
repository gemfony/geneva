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
GObject* GCharFlipAdaptor::clone_() const {
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
boost::optional<std::string> GCharFlipAdaptor::checkRelationshipWith(const GObject& cp,
		const Gem::Util::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Util;
    using namespace Gem::Util::POD;

	// Check that we are indeed dealing with a GParamterBase reference
	const GCharFlipAdaptor  *p_load = GObject::conversion_cast(&cp,  this);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GIntFlipAdaptorT<char>::checkRelationshipWith(cp, e, limit, "GCharFlipAdaptor", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GCharFlipAdaptor", caller, deviations, e);
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
