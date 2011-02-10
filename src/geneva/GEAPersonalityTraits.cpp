/**
 * @file GEAPersonalityTraits.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt) and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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

#include "geneva/GEAPersonalityTraits.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GEAPersonalityTraits)

namespace Gem {
namespace Geneva {

/*****************************************************************************/
/**
 * The default constructor
 */
GEAPersonalityTraits::GEAPersonalityTraits()
	: GPersonalityTraits()
	, parentCounter_(0)
	, popPos_(0)
	, command_("")
	, parentId_(-1) // means "unset"
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
	, parentId_(cp.parentId_)
{ /* nothing */ }

/*****************************************************************************/
/**
 * The standard destructor
 */
GEAPersonalityTraits::~GEAPersonalityTraits()
{ /* nothing */ }

/*****************************************************************************/
/**
 * A standard assignment operator for GEAPersonalityTraits objects.
 *
 * @param cp Reference to another GEAPersonalityTraits object
 * @return A constant reference to this object
 */
const GEAPersonalityTraits& GEAPersonalityTraits::operator=(const GEAPersonalityTraits& cp) {
	GEAPersonalityTraits::load_(&cp);
	return *this;
}

/*****************************************************************************/
/**
 * Checks for equality with another GEAPersonalityTraits object
 *
 * @param  cp A constant reference to another GEAPersonalityTraits object
 * @return A boolean indicating whether both objects are equal
 */
bool GEAPersonalityTraits::operator==(const GEAPersonalityTraits& cp) const {
	using namespace Gem::Common;
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
	using namespace Gem::Common;
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
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GEAPersonalityTraits *p_load = GObject::conversion_cast<GEAPersonalityTraits>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GPersonalityTraits::checkRelationshipWith(cp, e, limit, "GEAPersonalityTraits", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GEAPersonalityTraits", parentCounter_, p_load->parentCounter_, "parentCounter_", "p_load->parentCounter_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GEAPersonalityTraits", popPos_, p_load->popPos_, "popPos_", "p_load->popPos_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GEAPersonalityTraits", command_, p_load->command_, "command_", "p_load->command_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GEAPersonalityTraits", parentId_, p_load->parentId_, "parentId_", "p_load->parentId_", e , limit));

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
void GEAPersonalityTraits::load_(const GObject* cp) {
	const GEAPersonalityTraits *p_load = conversion_cast<GEAPersonalityTraits>(cp);

	// Load the parent class'es data
	GObject::load_(cp);

	// Then load our local data
	parentCounter_ = p_load->parentCounter_;
	popPos_ = p_load->popPos_;
	command_ = p_load->command_;
	parentId_ = p_load->parentId_;
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

/* ----------------------------------------------------------------------------------
 * Tested in GEAPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/*****************************************************************************/
/**
 * Retrieves the current value of the parentCounter_ variable
 *
 * @return The current value of the parentCounter_ variable
 */
boost::uint32_t GEAPersonalityTraits::getParentCounter() const {
	return parentCounter_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GEAPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

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

/* ----------------------------------------------------------------------------------
 * Tested in GEAPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

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

/* ----------------------------------------------------------------------------------
 * Tested in GEAPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/*****************************************************************************/
/**
 * Sets the position of the individual in the population
 *
 * @param popPos The new position of this individual in the population
 */
void GEAPersonalityTraits::setPopulationPosition(std::size_t popPos) {
	popPos_ = popPos;
}

/* ----------------------------------------------------------------------------------
 * Tested in GEAPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/*****************************************************************************/
/**
 * Retrieves the position of the individual in the population
 *
 * @return The current position of this individual in the population
 */
std::size_t GEAPersonalityTraits::getPopulationPosition(void) const {
	return popPos_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GEAPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/*****************************************************************************/
/**
 * Sets a command to be performed by a remote client.
 *
 * @param command The command to be performed by a remote client
 */
void GEAPersonalityTraits::setCommand(const std::string& command) {
	if(command != "evaluate" && command != "adapt") { // The allowed "grammar"
		raiseException(
				"In GEAPersonalityTraits::setCommand(): Got invalid command " << command
		);
	}

	command_ = command;
}

/* ----------------------------------------------------------------------------------
 * Tested in GEAPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * Tested in GEAPersonalityTraits::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/*****************************************************************************/
/**
 * Retrieves the command to be performed by a remote client.
 *
 * @return The command to be performed by a remote client.
 */
std::string GEAPersonalityTraits::getCommand() const {
#ifdef DEBUG
	// Some error checking
	if(command_.empty() || command_=="" || command_=="empty") {
		raiseException(
				"In GEAPersonalityTraits::getCommand(): Error " << std::endl
				<< "Tried to retrieve a command while a command hasn't been set"
		);
	}
#endif /* DEBUG */

	return command_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GEAPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * Tested in GEAPersonalityTraits::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/*****************************************************************************/
/**
 * Resets the command string
 */
void GEAPersonalityTraits::resetCommand() {
	command_ = "";
}

/* ----------------------------------------------------------------------------------
 * Used in GEAPersonalityTraits::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/*****************************************************************************/
/**
 * Stores the parent's id with this object.
 *
 * @param parentId The id of the individual's parent
 */
void GEAPersonalityTraits::setParentId(const std::size_t& parentId) {
	parentId_ = (boost::int16_t) parentId;
}

/* ----------------------------------------------------------------------------------
 * Tested in GEAPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/*****************************************************************************/
/**
 * Retrieves the parent id's value. Note that this function will throw if
 * no parent id has been set.
 *
 * @return The parent's id
 */
std::size_t GEAPersonalityTraits::getParentId() const {
	if(parentId_ >= 0) return parentId_;
	else {
		raiseException(
				"In GEAPersonalityTraits::getParentId():" << std::endl
				<< "parentId_ is unset"
		);
	}
}

/* ----------------------------------------------------------------------------------
 * Tested in GEAPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * Tested in GEAPersonalityTraits::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/*****************************************************************************/
/**
 * Checks whether a parent id has been set
 *
 * @return A boolean which indicates whether the parent id has been set
 */
bool GEAPersonalityTraits::parentIdSet() const {
	if(parentId_ >= 0) return true;
	else return false;
}

/* ----------------------------------------------------------------------------------
 * Tested in GEAPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/*****************************************************************************/
/**
 * Marks the parent id as unset
 */
void GEAPersonalityTraits::unsetParentId() {
	parentId_ = -1;
}

/* ----------------------------------------------------------------------------------
 * Tested in GEAPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * Tested in GEAPersonalityTraits::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

#ifdef GENEVATESTING
/*****************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GEAPersonalityTraits::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GPersonalityTraits::modify_GUnitTests()) result = true;

	// A relatively harmless modification is a change of the parentCounter variable
	parentCounter_++;

	return result;
}

/*****************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GEAPersonalityTraits::specificTestsNoFailureExpected_GUnitTests() {
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GPersonalityTraits::specificTestsNoFailureExpected_GUnitTests();

	//------------------------------------------------------------------------------

	{ // Check that it is possible to mark this as a parent or child-entity
		boost::shared_ptr<GEAPersonalityTraits> p_test = this->clone<GEAPersonalityTraits>();

		// Mark this object as belonging to a parent and check the correct setting
		BOOST_CHECK_NO_THROW(p_test->setIsParent());
		BOOST_CHECK(p_test->isParent() == true);

		// Mark this object as belonging to a child and check the correct setting
		BOOST_CHECK_NO_THROW(p_test->setIsChild());
		BOOST_CHECK(p_test->isParent() == false);
	}

	//------------------------------------------------------------------------------


	{ // Check that the parent counter is incremented or reset correctly
		boost::shared_ptr<GEAPersonalityTraits> p_test = this->clone<GEAPersonalityTraits>();

		// Mark this object as belonging to a child and check the correct setting
		BOOST_CHECK_NO_THROW(p_test->setIsChild());
		BOOST_CHECK(p_test->isParent() == false);

		// Check that the parent counter is now 0
		BOOST_CHECK(p_test->getParentCounter() == 0);

		// Mark the individual as a parent a number of times and check the parent counter
		for(boost::uint32_t i=1; i<=10; i++) {
			BOOST_CHECK_NO_THROW(p_test->setIsParent());
			BOOST_CHECK(p_test->getParentCounter() == i);
		}

		// Mark the individual as a child and check the parent counter again
		BOOST_CHECK_NO_THROW(p_test->setIsChild());
		BOOST_CHECK(p_test->isParent() == false);

		// Check that the parent counter is now 0
		BOOST_CHECK(p_test->getParentCounter() == 0);
	}

	//------------------------------------------------------------------------------

	{ // Check setting and retrieval of the individual's position in the population
		boost::shared_ptr<GEAPersonalityTraits> p_test = this->clone<GEAPersonalityTraits>();

		for(std::size_t i=0; i<10; i++) {
			BOOST_CHECK_NO_THROW(p_test->setPopulationPosition(i));
			BOOST_CHECK(p_test->getPopulationPosition() == i);
		}
	}

	//------------------------------------------------------------------------------

	{ // Test setting and retrieval of the allowed commands
		boost::shared_ptr<GEAPersonalityTraits> p_test = this->clone<GEAPersonalityTraits>();

		BOOST_CHECK_NO_THROW(p_test->setCommand("evaluate"));
		BOOST_CHECK(p_test->getCommand() == "evaluate");

		BOOST_CHECK_NO_THROW(p_test->setCommand("adapt"));
		BOOST_CHECK(p_test->getCommand() == "adapt");
	}

	//------------------------------------------------------------------------------

	{ // Test setting and retrieval of valid parent ids
		boost::shared_ptr<GEAPersonalityTraits> p_test = this->clone<GEAPersonalityTraits>();

		for(std::size_t i=0; i<10; i++) {
			BOOST_CHECK_NO_THROW(p_test->setParentId(i));
			BOOST_CHECK(p_test->getParentId() == i);
			BOOST_CHECK(p_test->parentIdSet() == true);
			BOOST_CHECK_NO_THROW(p_test->unsetParentId());
			BOOST_CHECK(p_test->parentIdSet() == false);
		}
	}

	//------------------------------------------------------------------------------
}

/*****************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GEAPersonalityTraits::specificTestsFailuresExpected_GUnitTests() {
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GPersonalityTraits::specificTestsFailuresExpected_GUnitTests();

	//------------------------------------------------------------------------------

#ifdef DEBUG
	{ // Test that retrieval of an unset command throws in DEBUG mode
		boost::shared_ptr<GEAPersonalityTraits> p_test = this->clone<GEAPersonalityTraits>();

		// Reset the command string
		BOOST_CHECK_NO_THROW(p_test->resetCommand());

		// Try to retrieve the command string
		BOOST_CHECK_THROW(p_test->getCommand(), Gem::Common::gemfony_error_condition);
	}
#endif /* DEBUG */

	//------------------------------------------------------------------------------

	{ // Check that setting any other command than "evaluate" or "adapt" throws
		boost::shared_ptr<GEAPersonalityTraits> p_test = this->clone<GEAPersonalityTraits>();

		// Try to set an unknown command
		BOOST_CHECK_THROW(p_test->setCommand("abc"), Gem::Common::gemfony_error_condition);
	}

	//------------------------------------------------------------------------------

	{ // Test that retrieval of the parent id throws, if the id isn't set
		boost::shared_ptr<GEAPersonalityTraits> p_test = this->clone<GEAPersonalityTraits>();

		BOOST_CHECK_NO_THROW(p_test->unsetParentId());
		BOOST_CHECK_THROW(p_test->getParentId(), Gem::Common::gemfony_error_condition);
	}

	//------------------------------------------------------------------------------
}

/*****************************************************************************/
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
