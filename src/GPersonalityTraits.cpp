/**
 * @file GPersonalityTraits.cpp
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

#include "GPersonalityTraits.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GPersonalityTraits)

namespace Gem {
namespace GenEvA {

/*****************************************************************************/
/**
 * The default constructor
 */
GPersonalityTraits::GPersonalityTraits():
	parentAlgIteration_(0)
{ /* nothing */ }

/*****************************************************************************/
/**
 * The copy constructor
 */
GPersonalityTraits::GPersonalityTraits(const GPersonalityTraits& cp):
	parentAlgIteration_(cp.parentAlgIteration_)
{ /* nothing */ }

/*****************************************************************************/
/**
 * The standard destructor. No local, dynamically allocated data,
 * hence it does nothing.
 */
GPersonalityTraits::~GPersonalityTraits()
{ /* nothing */ }

/*****************************************************************************/
/**
 * Checks for equality with another GPersonalityTraits object
 *
 * @param  cp A constant reference to another GPersonalityTraits object
 * @return A boolean indicating whether both objects are equal
 */
bool GPersonalityTraits::operator==(const GPersonalityTraits& cp) const {
	return GPersonalityTraits::isEqualTo(cp, boost::logic::indeterminate);
}

/*****************************************************************************/
/**
 * Checks for inequality with another GPersonalityTraits object
 *
 * @param  cp A constant reference to another GPersonalityTraits object
 * @return A boolean indicating whether both objects are inequal
 */
bool GPersonalityTraits::operator!=(const GPersonalityTraits& cp) const {
	return !GPersonalityTraits::isEqualTo(cp, boost::logic::indeterminate);
}

/*****************************************************************************/
/**
 * Checks for equality with another GPersonalityTraits object.
 *
 * @param  cp A constant reference to another GPersonalityTraits object
 * @return A boolean indicating whether both objects are equal
 */
bool GPersonalityTraits::isEqualTo(const GObject& cp, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GPersonalityTraits reference
	const GPersonalityTraits *gpt_load = GObject::conversion_cast(&cp,  this);

	// Check for equality of our parent class
	if(!GObject::isEqualTo(*gpt_load, expected)) return  false;

	// Then we take care of the local data
	if(checkForInequality("GPersonalityTraits", parentAlgIteration_, gpt_load->parentAlgIteration_,"parentAlgIteration_", "gpt_load->parentAlgIteration_", expected)) return false;

	return true;
}

/*****************************************************************************/
/**
 * Checks for similarity with another GPersonalityTraits object.
 *
 * @param  cp A constant reference to another GPersonalityTraits object
 * @param limit A double value specifying the acceptable level of differences of floating point values
 * @return A boolean indicating whether both objects are similar to each other
 */
bool GPersonalityTraits::isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GPersonalityTraits reference
	const GPersonalityTraits *gpt_load = GObject::conversion_cast(&cp,  this);

	// Check for equality of our parent class
	if(!GObject::isSimilarTo(*gpt_load, limit, expected)) return false;

	// Then we take care of the local data
	if(checkForDissimilarity("GPersonalityTraits", parentAlgIteration_, gpt_load->parentAlgIteration_, limit, "parentAlgIteration_", "gpt_load->parentAlgIteration_", expected)) return false;

	return true;
}

/*****************************************************************************/
/**
 * Loads the data of another GPersonalityTraits object
 *
 * @param cp A copy of another GPersonalityTraits object, camouflaged as a GObject
 */
void GPersonalityTraits::load(const GObject *cp) {
	const GPersonalityTraits *gpt_load = this->conversion_cast(cp, this);

	// Load the parent class'es data
	GObject::load(cp);

	// Then load our local data
	parentAlgIteration_ = gpt_load->parentAlgIteration_;
}

/*****************************************************************************/
/**
 * Allows to set the current iteration of the parent optimization algorithm.
 *
 * @param parentAlgIteration The current iteration of the optimization algorithm
 */
void GPersonalityTraits::setParentAlgIteration(const boost::uint32_t& parentAlgIteration) {
	parentAlgIteration_ = parentAlgIteration;
}

/*****************************************************************************/
/**
 * Gives access to the parent optimization algorithm's iteration
 *
 * @return The parent optimization algorithm's current iteration
 */
boost::uint32_t GPersonalityTraits::getParentAlgIteration() const {
	return parentAlgIteration_;
}

/*****************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
