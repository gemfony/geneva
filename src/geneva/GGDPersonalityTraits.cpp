/**
 * @file GGDPersonalityTraits.cpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
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
#include "geneva/GGDPersonalityTraits.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GGDPersonalityTraits)

namespace Gem {
namespace Geneva {

/*****************************************************************************/
/**
 * The default constructor
 */
GGDPersonalityTraits::GGDPersonalityTraits()
	: GPersonalityTraits()
	, command_("")
{ /* nothing */ }

/*****************************************************************************/
/**
 * The copy contructor
 *
 * @param cp A copy of another GGDPersonalityTraits object
 */
GGDPersonalityTraits::GGDPersonalityTraits(const GGDPersonalityTraits& cp)
	: GPersonalityTraits(cp)
	, command_(cp.command_)
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
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GGDPersonalityTraits::operator==","cp", CE_SILENT);
}

/*****************************************************************************/
/**
 * Checks for inequality with another GGDPersonalityTraits object
 *
 * @param  cp A constant reference to another GGDPersonalityTraits object
 * @return A boolean indicating whether both objects are inequal
 */
bool GGDPersonalityTraits::operator!=(const GGDPersonalityTraits& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GGDPersonalityTraits::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GGDPersonalityTraits::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GGDPersonalityTraits *p_load = GObject::conversion_cast<GGDPersonalityTraits>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GPersonalityTraits::checkRelationshipWith(cp, e, limit, "GGDPersonalityTraits", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GGDPersonalityTraits", command_, p_load->command_, "command_", "p_load->command_", e , limit));

	return evaluateDiscrepancies("GGDPersonalityTraits", caller, deviations, e);
}


/*****************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A clone of this object, camouflaged as a GObject
 */
GObject* GGDPersonalityTraits::clone_() const {
	return new GGDPersonalityTraits(*this);
}

/*****************************************************************************/
/**
 * Loads the data of another GGDPersonalityTraits object
 *
 * @param cp A copy of another GGDPersonalityTraits object, camouflaged as a GObject
 */
void GGDPersonalityTraits::load_(const GObject* cp) {
	const GGDPersonalityTraits *p_load = conversion_cast<GGDPersonalityTraits>(cp);

	// Load the parent class'es data
	GObject::load_(cp);

	// and then the local data
	command_ = p_load->command_;
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
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	command_ = command;
}

/* ----------------------------------------------------------------------------------
 * Tested in GGDPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * Tested in GGDPersonalityTraits::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/*****************************************************************************/
/**
 * Retrieves the command to be performed by a remote client.
 *
 * @return The command to be performed by a remote client.
 */
std::string GGDPersonalityTraits::getCommand() const {
#ifdef DEBUG
	// Some error checking
	if(command_.empty() || command_=="" || command_=="empty") {
		std::ostringstream error;
		error << "In GGDPersonalityTraits::getCommand(): Error " << std::endl
			  << "Tried to retrieve a command while a command hasn't been set" << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}
#endif /* DEBUG */

	return command_;
}

/* ----------------------------------------------------------------------------------
 * Tested in GGDPersonalityTraits::specificTestsNoFailuresExpected_GUnitTests()
 * Tested in GGDPersonalityTraits::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/*****************************************************************************/
/**
 * Resets the command string
 */
void GGDPersonalityTraits::resetCommand() {
	command_ = "";
}

/* ----------------------------------------------------------------------------------
 * Used in GGDPersonalityTraits::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

#ifdef GENEVATESTING
/*****************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GGDPersonalityTraits::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GPersonalityTraits::modify_GUnitTests()) result = true;

	return result;
}

/*****************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GGDPersonalityTraits::specificTestsNoFailureExpected_GUnitTests() {
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GPersonalityTraits::specificTestsNoFailureExpected_GUnitTests();

	//------------------------------------------------------------------------------

	{ // Test setting and retrieval of the allowed commands
		boost::shared_ptr<GGDPersonalityTraits> p_test = this->clone<GGDPersonalityTraits>();

		BOOST_CHECK_NO_THROW(p_test->setCommand("evaluate"));
		BOOST_CHECK(p_test->getCommand() == "evaluate");
	}

	//------------------------------------------------------------------------------



	//------------------------------------------------------------------------------
}

/*****************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GGDPersonalityTraits::specificTestsFailuresExpected_GUnitTests() {
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent class'es function
	GPersonalityTraits::specificTestsFailuresExpected_GUnitTests();

	//------------------------------------------------------------------------------

#ifdef DEBUG
	{ // Test that retrieval of an unset command throws in DEBUG mode
		boost::shared_ptr<GGDPersonalityTraits> p_test = this->clone<GGDPersonalityTraits>();

		// Reset the command string
		BOOST_CHECK_NO_THROW(p_test->resetCommand());

		// Try to retrieve the command string
		BOOST_CHECK_THROW(p_test->getCommand(), Gem::Common::gemfony_error_condition);
	}
#endif /* DEBUG */

	//------------------------------------------------------------------------------

	{ // Check that setting any other command than "evaluate" or "adapt" throws
		boost::shared_ptr<GGDPersonalityTraits> p_test = this->clone<GGDPersonalityTraits>();

		// Try to set an unknown command
		BOOST_CHECK_THROW(p_test->setCommand("abc"), Gem::Common::gemfony_error_condition);
	}

	//------------------------------------------------------------------------------
}

/*****************************************************************************/
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
