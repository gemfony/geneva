/**
 * @file GObject.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

#include "geneva/GObject.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
// Needed to allow catching of a SIGHUP or CTRL_CLOSE_EVENT event
volatile G_API_GENEVA std::sig_atomic_t GObject::GenevaSigHupSent = 0;

/******************************************************************************/
/**
 * The default constructor initializes the internal values of this class.
 * In particular, it sets the name of the Geneva object to "GObject"
 */
GObject::GObject() { /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GObject object
 */
GObject::GObject(const GObject &cp) { /* nothing */ }

/******************************************************************************/
/**
 * As no memory is dynamically allocated in GObject, no work has to
 * be done by this destructor.
 */
GObject::~GObject() { /* nothing */ }

/******************************************************************************/
/**
 * Checks for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GObject::compare(
	const GObject &cp
	, const Gem::Common::expectation &e
	, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GObject reference independent of this object and convert the pointer
	const GObject *p_load = Gem::Common::g_convert_and_compare<GObject, GObject>(cp, this);

	GToken token("GObject", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GCommonInterfaceT<GObject>>(IDENTITY(*this, *p_load), token);

	// ... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Checks whether a given expectation for the relationship between this object and another object
 * is fulfilled. This function was added to GObject to ensure backward compatibility. All
 * new code should use the compare-Infrastructure.
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 * @param caller An identifier for the calling entity
 * @param y_name An identifier for the object that should be compared to this one
 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
 */
boost::optional<std::string> GObject::checkRelationshipWith(
	const GObject &cp
	, const Gem::Common::expectation &e
	, const double &limit
	, const std::string &caller
	, const std::string &y_name
	, const bool &withMessages
) const {
	using namespace Gem::Common;

	// Check that cp isn't the same object as this one
	Gem::Common::ptrDifferenceCheck(&cp, this);

	GToken token("GObject", e);

	// Do the actual comparison
	Gem::Common::compare_t(IDENTITY(*this, cp), token);

	// Perform a standard compare check
	try {
		token.evaluate();
	} catch (g_expectation_violation &g) {
		return boost::optional<std::string>(std::string(g.what()));
	}

	return boost::optional<std::string>();
}

/* ----------------------------------------------------------------------------------
 * UNCHECKED
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GObject::name() const {
	return std::string("GObject");
}

/* ----------------------------------------------------------------------------------
 * Tested in GObject::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Writes a configuration file to disk
 *
 * @param configFile The name of the configuration file to be written
 * @param header A header to be prepended to the configuration file
 */
void GObject::writeConfigFile(
	const std::string &configFile, const std::string &header
) {
	// This class will handle the interaction with configuration files
	Gem::Common::GParserBuilder gpb;

	// Recursively add configuration options to gpb,
	// starting with the most derived class
	addConfigurationOptions(gpb);

	// Write out the configuration file
	gpb.writeConfigFile(configFile, header, true);
}

/* ----------------------------------------------------------------------------------
 * UNTESTED
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Reads a configuration file from disk
 *
 * @param configFile The name of the configuration file to be parsed
 */
void GObject::readConfigFile(const std::string &configFile) {
	// This class will handle the interaction with configuration files
	Gem::Common::GParserBuilder gpb;

	// Recursively add configuration options to gpb,
	// starting with the most derived class
	addConfigurationOptions(gpb);

	// Read in the configuration file
	gpb.parseConfigFile(configFile);
}

/* ----------------------------------------------------------------------------------
 * UNTESTED
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A pointer to another GObject object
 */
void GObject::load_(const GObject *cp) {
	// Check that cp isn't the same object as this one
	Gem::Common::ptrDifferenceCheck(cp, this);

	// No local data
}

/* ----------------------------------------------------------------------------------
 * Loading is checked as part of the Geneva standard test suite
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Allows derived classes to assign other class'es values
 */
const GObject &GObject::operator=(const GObject &cp) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GObject object
 *
 * @param  cp A constant reference to another GObject object
 * @return A boolean indicating whether both objects are equal
 */
bool GObject::operator==(const GObject &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Checks for inequality with another GObject object
 *
 * @param  cp A constant reference to another GObject object
 * @return A boolean indicating whether both objects are inequal
 */
bool GObject::operator!=(const GObject &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Creates a deep clone of this object, storing it in a std::shared_ptr<GObject>
 *
 * @return A std::shared_ptr<GObject> to a clone of the derived object
 */
// std::shared_ptr <GObject> GObject::clone() const {
// 	return std::shared_ptr<GObject>(clone_());
// }

/* ----------------------------------------------------------------------------------
 * Tested in GObject::specificTestsNoFailureExpected_GUnitTests() as well as in
 * the Geneva standard test suite.
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object. This is a protected, virtual version
 * of this function that is overloaded in derived classes.
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GObject::addConfigurationOptions(
	Gem::Common::GParserBuilder &gpb
) {
	// No local data, no relevant parent classes, hence nothing to do
}

/* ----------------------------------------------------------------------------------
 * UNTESTED
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GObject::modify_GUnitTests() {
#ifdef GEM_TESTING
	// There is no modifiable parent class and no local data,
	// so there is nothing we can do here in this function.

	return false;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GObject::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GObject::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// no parent class

	// --------------------------------------------------------------------------

	{ // Check cloning to GObject format
		std::shared_ptr <GObject> p_test = this->clone<GObject>();

		// Check that the pointer actually points somewhere
		BOOST_CHECK(p_test);
	}

	// --------------------------------------------------------------------------

	{ // Check the name of the object
		std::shared_ptr <GObject> p_test = this->clone<GObject>();

		// Check that the pointer actually points somewhere
		BOOST_CHECK_MESSAGE(
			p_test->GObject::name() == "GObject", "Name is " << p_test->GObject::name()
		);
	}

	// --------------------------------------------------------------------------

	{ // Check cloning to GObject format, using a different method
		std::shared_ptr <GObject> p_test = this->clone();

		// Check that the pointer actually points somewhere
		BOOST_CHECK(p_test);
	}

	// --------------------------------------------------------------------------

	{ // Check that the report function returns a non-empty description
		std::shared_ptr <GObject> p_test = this->clone();

		// Check that the pointer actually points somewhere
		BOOST_CHECK(!(p_test->report()).empty());
	}


	{ // Check (de-)serialization from/to a stream in three modes
		std::shared_ptr <GObject> p_test = this->clone();

		{ // Text mode
			std::ostringstream ostr;
			BOOST_CHECK_NO_THROW(p_test->toStream(ostr, Gem::Common::serializationMode::SERIALIZATIONMODE_TEXT));
			std::istringstream istr(ostr.str());
			BOOST_CHECK_NO_THROW(p_test->fromStream(istr, Gem::Common::serializationMode::SERIALIZATIONMODE_TEXT));
		}

		{ // XML mode
			std::ostringstream ostr;
			BOOST_CHECK_NO_THROW(p_test->toStream(ostr, Gem::Common::serializationMode::SERIALIZATIONMODE_XML));
			std::istringstream istr(ostr.str());
			BOOST_CHECK_NO_THROW(p_test->fromStream(istr, Gem::Common::serializationMode::SERIALIZATIONMODE_XML));
		}

		{ // Binary mode
			std::ostringstream ostr;
			BOOST_CHECK_NO_THROW(p_test->toStream(ostr, Gem::Common::serializationMode::SERIALIZATIONMODE_BINARY));
			std::istringstream istr(ostr.str());
			BOOST_CHECK_NO_THROW(p_test->fromStream(istr, Gem::Common::serializationMode::SERIALIZATIONMODE_BINARY));
		}
	}

	// --------------------------------------------------------------------------

	{ // Check (de-)serialization from/to strings in three modes
		std::shared_ptr <GObject> p_test = this->clone();

		BOOST_CHECK_NO_THROW(p_test->fromString(p_test->toString(Gem::Common::serializationMode::SERIALIZATIONMODE_TEXT),
															 Gem::Common::serializationMode::SERIALIZATIONMODE_TEXT));
		BOOST_CHECK_NO_THROW(
			p_test->fromString(p_test->toString(Gem::Common::serializationMode::SERIALIZATIONMODE_XML), Gem::Common::serializationMode::SERIALIZATIONMODE_XML));
		BOOST_CHECK_NO_THROW(p_test->fromString(p_test->toString(Gem::Common::serializationMode::SERIALIZATIONMODE_BINARY),
															 Gem::Common::serializationMode::SERIALIZATIONMODE_BINARY));
	}

	// --------------------------------------------------------------------------

	{ // Check (de-)serialization from/to a file in three different modes
		std::shared_ptr <GObject> p_test = this->clone();

		{ // Text mode
			BOOST_CHECK_NO_THROW(p_test->toFile(bf::path("123test.txt"), Gem::Common::serializationMode::SERIALIZATIONMODE_TEXT));
			BOOST_CHECK_NO_THROW(p_test->fromFile(bf::path("123test.txt"), Gem::Common::serializationMode::SERIALIZATIONMODE_TEXT));

			// Get rid of the file
			remove(bf::path("./123test.txt"));
		}

		{ // XML mode
			BOOST_CHECK_NO_THROW(p_test->toFile(bf::path("123test.xml"), Gem::Common::serializationMode::SERIALIZATIONMODE_XML));
			BOOST_CHECK_NO_THROW(p_test->fromFile(bf::path("123test.xml"), Gem::Common::serializationMode::SERIALIZATIONMODE_XML));

			// Get rid of the file
			remove(bf::path("./123test.xml"));
		}

		{ // Binary mode
			BOOST_CHECK_NO_THROW(p_test->toFile(bf::path("123test.bin"), Gem::Common::serializationMode::SERIALIZATIONMODE_BINARY));
			BOOST_CHECK_NO_THROW(p_test->fromFile(bf::path("123test.bin"), Gem::Common::serializationMode::SERIALIZATIONMODE_BINARY));

			// Get rid of the file
			remove(bf::path("./123test.bin"));
		}
	}

	// --------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GObject::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GObject::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// no parent class

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GObject::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
