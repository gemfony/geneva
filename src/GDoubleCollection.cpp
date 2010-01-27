/**
 * @file GDoubleCollection.cpp
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

#include "GDoubleCollection.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GDoubleCollection)


namespace Gem {
namespace GenEvA {

/*******************************************************************************************/
/**
 * The default constructor
 */
GDoubleCollection::GDoubleCollection()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization with a number of random values in a given range
 *
 * @param nval The amount of random values
 * @param min The minimum random value
 * @param max The maximum random value
 */
GDoubleCollection::GDoubleCollection(const std::size_t& nval, const double& min, const double& max)
	: GNumCollectionT<double>(nval, min, max)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GDoubleCollection object
 */
GDoubleCollection::GDoubleCollection(const GDoubleCollection& cp)
	: GNumCollectionT<double>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GDoubleCollection::~GDoubleCollection()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GDoubleCollection object
 * @return A constant reference to this object
 */
const GDoubleCollection& GDoubleCollection::operator=(const GDoubleCollection& cp){
	GDoubleCollection::load(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GDoubleCollection::clone_() const {
	return new GDoubleCollection(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GDoubleCollection object
 *
 * @param  cp A constant reference to another GDoubleCollection object
 * @return A boolean indicating whether both objects are equal
 */
bool GDoubleCollection::operator==(const GDoubleCollection& cp) const {
	return GDoubleCollection::isEqualTo(cp, boost::logic::indeterminate);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GDoubleCollection object
 *
 * @param  cp A constant reference to another GDoubleCollection object
 * @return A boolean indicating whether both objects are inequal
 */
bool GDoubleCollection::operator!=(const GDoubleCollection& cp) const {
	return !GDoubleCollection::isEqualTo(cp, boost::logic::indeterminate);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GDoubleCollection object.  If T is an object type,
 * then it must implement operator!= .
 *
 * @param  cp A constant reference to another GDoubleCollection object
 * @return A boolean indicating whether both objects are equal
 */
bool GDoubleCollection::isEqualTo(const GObject& cp, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterT reference
	const GDoubleCollection *p_load = GObject::conversion_cast(&cp,  this);

	// Check equality of the parent class
	if(!GNumCollectionT<double>::isEqualTo(*p_load, expected)) return false;

	// No local data

	return true;
}

/*******************************************************************************************/
/**
 * Checks for similarity with another GDoubleCollection object.
 *
 * @param  cp A constant reference to another GDoubleCollection object
 * @param limit A double value specifying the acceptable level of differences of floating point values
 * @return A boolean indicating whether both objects are similar to each other
 */
bool GDoubleCollection::isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterT reference
	const GDoubleCollection *p_load = GObject::conversion_cast(&cp,  this);

	// Check similarity of the parent class
	if(!GNumCollectionT<double>::isSimilarTo(*p_load, limit, expected)) return false;

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
boost::optional<std::string> GDoubleCollection::checkRelationshipWith(const GObject& cp,
		const Gem::Util::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Util;
    using namespace Gem::Util::POD;

	// Check that we are indeed dealing with a GParamterBase reference
	const GDoubleCollection *p_load = GObject::conversion_cast(&cp,  this);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GParameterCollectionT<double>::checkRelationshipWith(cp, e, limit, "GDoubleCollection", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GDoubleCollection", caller, deviations, e);
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GDoubleCollection object, camouflaged as a GObject
 */
void GDoubleCollection::load(const GObject* cp){
	// Convert cp into local format (also checks for the type of cp)
	const GDoubleCollection *p_load = GObject::conversion_cast(cp, this);

	// Load our parent class'es data ...
	GNumCollectionT<double>::load(cp);

	// ... no local data
}

/*******************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
