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
	: GPersonalityTraits()
	, parentCounter_(0)
	, popPos_(0)
	, command_("")
{ /* nothing */ }

/*****************************************************************************/
/**
 * The copy contructor
 *
 * @param cp A copy of another GEAPersonalityTraits object
 */
GEAPersonalityTraits::GEAPersonalityTraits(const GEAPersonalityTraits& cp)
	: GPersonalityTraits(cp)
	, parentCounter_(cp.parentCounter_)
	, popPos_(cp.popPos_)
	, command_(cp.command_)
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
	using namespace Gem::Util;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GEAPersonalityTraits::operator==","cp", CE_SILENT);
}

/*****************************************************************************/
/**
 * Checks for inequality with another GEAPersonalityTraits object
 *
 * @param  cp A constant reference to another GEAPersonalityTraits object
 * @return A boolean indicating whether both objects are inequal
 */
bool GEAPersonalityTraits::operator!=(const GEAPersonalityTraits& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GEAPersonalityTraits::operator!=","cp", CE_SILENT);
}

/*****************************************************************************/
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
boost::optional<std::string> GEAPersonalityTraits::checkRelationshipWith(const GObject& cp,
		const Gem::Util::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Util;
    using namespace Gem::Util::POD;

	// Check that we are indeed dealing with a GParamterBase reference
	const GEAPersonalityTraits *p_load = GObject::conversion_cast(&cp,  this);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GPersonalityTraits::checkRelationshipWith(cp, e, limit, "GEAPersonalityTraits", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GEAPersonalityTraits", parentCounter_, p_load->parentCounter_, "parentCounter_", "p_load->parentCounter_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GEAPersonalityTraits", popPos_, p_load->popPos_, "popPos_", "p_load->popPos_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GEAPersonalityTraits", command_, p_load->command_, "command_", "p_load->command_", e , limit));

	return evaluateDiscrepancies("GEAPersonalityTraits", caller, deviations, e);
}

/*****************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A clone of this object, camouflaged as a GObject
 */
GObject* GEAPersonalityTraits::clone_() const {
	return new GEAPersonalityTraits(*this);
}

/*****************************************************************************/
/**
 * Loads the data of another GEAPersonalityTraits object
 *
 * @param cp A copy of another GEAPersonalityTraits object, camouflaged as a GObject
 */
void GEAPersonalityTraits::load(const GObject* cp) {
	const GEAPersonalityTraits *p_load = this->conversion_cast(cp, this);

	// Load the parent class'es data
	GObject::load(cp);

	// Then load our local data
	parentCounter_ = p_load->parentCounter_;
	popPos_ = p_load->popPos_;
	command_ = p_load->command_;
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
