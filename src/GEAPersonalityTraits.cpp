/**
 * @file GEAPersonalityTraits.cpp
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

#include "GEAPersonalityTraits.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GEAPersonalityTraits)

namespace Gem {
namespace GenEvA {

/*****************************************************************************/
/**
 * The default constructor
 */
GEAPersonalityTraits::GEAPersonalityTraits()
	:GPersonalityTraits(),
	 parentCounter_(0),
	 popPos_(0),
	 command_("")
{ /* nothing */ }

/*****************************************************************************/
/**
 * The copy contructor
 *
 * @param cp A copy of another GEAPersonalityTraits object
 */
GEAPersonalityTraits::GEAPersonalityTraits(const GEAPersonalityTraits& cp)
	:GPersonalityTraits(cp),
	 parentCounter_(cp.parentCounter_),
	 popPos_(cp.popPos_),
	 command_(cp.command_)
{ /* nothing */ }

/*****************************************************************************/
/**
 * The standard destructor
 */
GEAPersonalityTraits::~GEAPersonalityTraits()
{ /* nothing */ }

/*****************************************************************************/
/**
 * Checks for equality with another GEAPersonalityTraits object
 *
 * @param  cp A constant reference to another GEAPersonalityTraits object
 * @return A boolean indicating whether both objects are equal
 */
bool GEAPersonalityTraits::operator==(const GEAPersonalityTraits& cp) const {
	return GEAPersonalityTraits::isEqualTo(cp, boost::logic::indeterminate);
}

/*****************************************************************************/
/**
 * Checks for inequality with another GEAPersonalityTraits object
 *
 * @param  cp A constant reference to another GEAPersonalityTraits object
 * @return A boolean indicating whether both objects are inequal
 */
bool GEAPersonalityTraits::operator!=(const GEAPersonalityTraits& cp) const {
	return !GEAPersonalityTraits::isEqualTo(cp, boost::logic::indeterminate);
}

/*****************************************************************************/
/**
 * Checks for equality with another GEAPersonalityTraits object
 *
 * @param  cp A constant reference to another GPersonalityTraits object
 * @return A boolean indicating whether both objects are equal
 */
bool GEAPersonalityTraits::isEqualTo(const GObject& cp, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GPersonalityTraits reference
	const GEAPersonalityTraits *geapt_load = GObject::conversion_cast(&cp,  this);

	// Check for equality of our parent class
	if(!GObject::isEqualTo(*geapt_load, expected)) return  false;

	// Then we take care of the local data
	if(checkForInequality("GEAPersonalityTraits", parentCounter_, geapt_load->parentCounter_,"parentCounter_", "geapt_load->parentCounter_", expected)) return false;
	if(checkForInequality("GEAPersonalityTraits", popPos_, geapt_load->popPos_,"popPos_", "geapt_load->popPos_", expected)) return false;
	if(checkForInequality("GEAPersonalityTraits", command_, geapt_load->command_,"command_", "geapt_load->command_", expected)) return false;

	return true;
}

/*****************************************************************************/
/**
 * Checks for similarity with another GEAPersonalityTraits object
 *
 * @param  cp A constant reference to another GEAPersonalityTraits object
 * @param limit A double value specifying the acceptable level of differences of floating point values
 * @return A boolean indicating whether both objects are similar to each other
 */
bool GEAPersonalityTraits::isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GPersonalityTraits reference
	const GEAPersonalityTraits *geapt_load = GObject::conversion_cast(&cp,  this);

	// Check for equality of our parent class
	if(!GObject::isSimilarTo(*geapt_load, limit, expected)) return false;

	// Then we take care of the local data
	if(checkForDissimilarity("GEAPersonalityTraits", parentCounter_, geapt_load->parentCounter_, limit, "parentCounter_", "geapt_load->parentCounter_", expected)) return false;
	if(checkForDissimilarity("GEAPersonalityTraits", popPos_, geapt_load->popPos_, limit, "popPos_", "geapt_load->popPos_", expected)) return false;
	if(checkForDissimilarity("GEAPersonalityTraits", command_, geapt_load->command_, limit, "command_", "geapt_load->command_", expected)) return false;

	return true;
}

/*****************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A clone of this object, camouflaged as a GObject
 */
GObject* GEAPersonalityTraits::clone() const {
	return new GEAPersonalityTraits(*this);
}

/*****************************************************************************/
/**
 * Loads the data of another GEAPersonalityTraits object
 *
 * @param cp A copy of another GEAPersonalityTraits object, camouflaged as a GObject
 */
void GEAPersonalityTraits::load(const GObject* cp) {
	const GEAPersonalityTraits *geapt_load = this->conversion_cast(cp, this);

	// Load the parent class'es data
	GObject::load(cp);

	// Then load our local data
	parentCounter_ = geapt_load->parentCounter_;
	popPos_ = geapt_load->popPos_;
	command_ = geapt_load->command_;
}

/*****************************************************************************/
/**
 * Checks whether this is a parent individual
 *
 * @return A boolean indicating whether this object is a parent at this time
 */
bool GEAPersonalityTraits::isParent() const {
	return (parentCounter_>0)?true:false;
}

/*****************************************************************************/
/**
 * Retrieves the current value of the parentCounter_ variable
 *
 * @return The current value of the parentCounter_ variable
 */
boost::uint32_t GEAPersonalityTraits::getParentCounter() const {
	return parentCounter_;
}

/*****************************************************************************/
/**
 * Marks an individual as a parent
 *
 * @return A boolean indicating whether this individual was previously a parent (true) or a child (false)
 */
bool GEAPersonalityTraits::setIsParent() {
	bool previous=(parentCounter_>0)?true:false;
	parentCounter_++;
	return previous;
}

/*****************************************************************************/
/**
 * Marks an individual as a child
 *
 * @return A boolean indicating whether this individual was previously a parent (true) or a child (false)
 */
bool GEAPersonalityTraits::setIsChild() {
	bool previous=(parentCounter_>0)?true:false;
	parentCounter_=0;
	return previous;
}

/*****************************************************************************/
/**
 * Sets the position of the individual in the population
 *
 * @param popPos The new position of this individual in the population
 */
void GEAPersonalityTraits::setPopulationPosition(std::size_t popPos) {
	popPos_ = popPos;
}

/*****************************************************************************/
/**
 * Retrieves the position of the individual in the population
 *
 * @return The current position of this individual in the population
 */
std::size_t GEAPersonalityTraits::getPopulationPosition(void) const {
	return popPos_;
}

/*****************************************************************************/
/**
 * Sets a command to be performed by a remote client.
 *
 * @param command The command to be performed by a remote client
 */
void GEAPersonalityTraits::setCommand(const std::string& command) {
	if(command != "evaluate" && command != "mutate") { // The allowed "grammar"
		std::ostringstream error;
		error << "In GEAPersonalityTraits::setCommand(): Got invalid command " << command << std::endl;
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
std::string GEAPersonalityTraits::getCommand() const {
	return command_;
}

/*****************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
