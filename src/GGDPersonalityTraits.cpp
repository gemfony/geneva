/**
 * @file GGDPersonalityTraits.cpp
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

#include "GGDPersonalityTraits.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GGDPersonalityTraits)

namespace Gem {
namespace GenEvA {

/*****************************************************************************/
/**
 * The default constructor
 */
GGDPersonalityTraits::GGDPersonalityTraits()
	:GPersonalityTraits(),
	 command_("")
{ /* nothing */ }

/*****************************************************************************/
/**
 * The copy contructor
 *
 * @param cp A copy of another GGDPersonalityTraits object
 */
GGDPersonalityTraits::GGDPersonalityTraits(const GGDPersonalityTraits& cp)
	:GPersonalityTraits(cp),
	 command_(cp.command_)
{ /* nothing */ }

/*****************************************************************************/
/**
 * The standard destructor
 */
GGDPersonalityTraits::~GGDPersonalityTraits()
{ /* nothing */ }

/*****************************************************************************/
/**
 * Checks for equality with another GGDPersonalityTraits object
 *
 * @param  cp A constant reference to another GGDPersonalityTraits object
 * @return A boolean indicating whether both objects are equal
 */
bool GGDPersonalityTraits::operator==(const GGDPersonalityTraits& cp) const {
	return GGDPersonalityTraits::isEqualTo(cp, boost::logic::indeterminate);
}

/*****************************************************************************/
/**
 * Checks for inequality with another GGDPersonalityTraits object
 *
 * @param  cp A constant reference to another GGDPersonalityTraits object
 * @return A boolean indicating whether both objects are inequal
 */
bool GGDPersonalityTraits::operator!=(const GGDPersonalityTraits& cp) const {
	return !GGDPersonalityTraits::isEqualTo(cp, boost::logic::indeterminate);
}

/*****************************************************************************/
/**
 * Checks for equality with another GGDPersonalityTraits object
 *
 * @param  cp A constant reference to another GPersonalityTraits object
 * @return A boolean indicating whether both objects are equal
 */
bool GGDPersonalityTraits::isEqualTo(const GObject& cp, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GPersonalityTraits reference
	const GGDPersonalityTraits *geapt_load = GObject::conversion_cast(&cp,  this);

	// Check for equality of our parent class
	if(!GObject::isEqualTo(*geapt_load, expected)) return  false;

	// Then we take care of the local data
	if(checkForInequality("GGDPersonalityTraits", command_, geapt_load->command_,"command_", "geapt_load->command_", expected)) return false;

	return true;
}

/*****************************************************************************/
/**
 * Checks for similarity with another GGDPersonalityTraits object
 *
 * @param  cp A constant reference to another GGDPersonalityTraits object
 * @param limit A double value specifying the acceptable level of differences of floating point values
 * @return A boolean indicating whether both objects are similar to each other
 */
bool GGDPersonalityTraits::isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GPersonalityTraits reference
	const GGDPersonalityTraits *geapt_load = GObject::conversion_cast(&cp,  this);

	// Check for equality of our parent class
	if(!GObject::isSimilarTo(*geapt_load, limit, expected)) return false;

	// Then we take care of the local data
	if(checkForDissimilarity("GGDPersonalityTraits", command_, geapt_load->command_, limit, "command_", "geapt_load->command_", expected)) return false;

	return true;
}

/*****************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A clone of this object, camouflaged as a GObject
 */
GObject* GGDPersonalityTraits::clone() const {
	return new GGDPersonalityTraits(*this);
}

/*****************************************************************************/
/**
 * Loads the data of another GGDPersonalityTraits object
 *
 * @param cp A copy of another GGDPersonalityTraits object, camouflaged as a GObject
 */
void GGDPersonalityTraits::load(const GObject* cp) {
	const GGDPersonalityTraits *geapt_load = this->conversion_cast(cp, this);

	// Load the parent class'es data
	GObject::load(cp);

	// and then the local data
	command_ = geapt_load->command_;
}

/*****************************************************************************/
/**
 * Sets a command to be performed by a remote client.
 *
 * @param command The command to be performed by a remote client
 */
void GGDPersonalityTraits::setCommand(const std::string& command) {
	if(command != "evaluate") { // The allowed "grammar"
		std::ostringstream error;
		error << "In GGDPersonalityTraits::setCommand(): Got invalid command " << command << std::endl;
		throw(Gem::GenEvA::geneva_error_condition(error.str()));
	}

	command_ = command;
}

/*****************************************************************************/
/**
 * Retrieves the command to be performed by a remote client.
 *
 * @return The command to be performed by a remote client.
 */
std::string GGDPersonalityTraits::getCommand() const {
	return command_;
}

/*****************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
