/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "geneva/GObject.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
// Needed to allow catching of a SIGHUP or CTRL_CLOSE_EVENT event
volatile G_API_GENEVA std::sig_atomic_t GObject::GenevaSigHupSent = 0;

/******************************************************************************/
/**
 * Checks for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GObject::compare_(
	const GObject &cp
	, const Gem::Common::expectation &e
	, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GObject reference independent of this object and convert the pointer
	const GObject *p_load = Gem::Common::g_convert_and_compare<GObject, GObject>(cp, this);

	GToken token("GObject", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GCommonInterfaceT<GObject>>(*this, *p_load, token);

	// ... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object. This is a protected, virtual version
 * of this function that is overloaded in derived classes.
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GObject::addConfigurationOptions_(
	Gem::Common::GParserBuilder &gpb
) {
	// Call the parent classes function
	Gem::Common::GCommonInterfaceT<GObject>::addConfigurationOptions_(gpb);

   // No local data, no relevant parent classes, hence nothing to do
}


/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GObject::name_() const {
	return std::string("GObject");
}

/* ----------------------------------------------------------------------------------
 * Tested in GObject::specificTestsNoFailureExpected_GUnitTests()
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
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GObject::modify_GUnitTests_() {
#ifdef GEM_TESTING
	// There is no modifiable parent class and no local data,
	// so there is nothing we can do here in this function.

	return false;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GObject::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GObject::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// no parent class

	// --------------------------------------------------------------------------

	/*
	{ // Tests reading and writing of configuration files
		std::shared_ptr <GObject> p_test = this->clone<GObject>();

		std::string configFileName = "./someTestConfig.json";
		std::string header = "A not so complicated header";

		// First make sure the configuration file does not exist
		boost::filesystem::remove(boost::filesystem::path(configFileName));

		// Write and read the config file
		BOOST_CHECK_NO_THROW(p_test->writeConfigFile(configFileName, header));
		BOOST_CHECK_NO_THROW(p_test->readConfigFile(configFileName));

		// Check that a config file exists
		BOOST_CHECK_MESSAGE(
			boost::filesystem::exists(boost::filesystem::path(configFileName))
			, "Error: file " << configFileName << " was not found"
		);
	}
	 */

	// --------------------------------------------------------------------------

	{ // Check cloning to GObject format
		std::shared_ptr <GObject> p_test = this->clone<GObject>();

		// Check that the pointer actually points somewhere
		BOOST_CHECK(p_test);
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
		BOOST_CHECK(not (p_test->report()).empty());
	}


	{ // Check (de-)serialization from/to a stream in three modes
		std::shared_ptr <GObject> p_test = this->clone();

		{ // Text mode
			std::ostringstream ostr;
			BOOST_CHECK_NO_THROW(p_test->toStream(ostr, Gem::Common::serializationMode::TEXT));
			std::istringstream istr(ostr.str());
			BOOST_CHECK_NO_THROW(p_test->fromStream(istr, Gem::Common::serializationMode::TEXT));
		}

		{ // XML mode
			std::ostringstream ostr;
			BOOST_CHECK_NO_THROW(p_test->toStream(ostr, Gem::Common::serializationMode::XML));
			std::istringstream istr(ostr.str());
			BOOST_CHECK_NO_THROW(p_test->fromStream(istr, Gem::Common::serializationMode::XML));
		}

		{ // Binary mode
			std::ostringstream ostr;
			BOOST_CHECK_NO_THROW(p_test->toStream(ostr, Gem::Common::serializationMode::BINARY));
			std::istringstream istr(ostr.str());
			BOOST_CHECK_NO_THROW(p_test->fromStream(istr, Gem::Common::serializationMode::BINARY));
		}
	}

	// --------------------------------------------------------------------------

	{ // Check (de-)serialization from/to strings in three modes
		std::shared_ptr <GObject> p_test = this->clone();

		BOOST_CHECK_NO_THROW(p_test->fromString(p_test->toString(Gem::Common::serializationMode::TEXT),
															 Gem::Common::serializationMode::TEXT));
		BOOST_CHECK_NO_THROW(
			p_test->fromString(p_test->toString(Gem::Common::serializationMode::XML), Gem::Common::serializationMode::XML));
		BOOST_CHECK_NO_THROW(p_test->fromString(p_test->toString(Gem::Common::serializationMode::BINARY),
															 Gem::Common::serializationMode::BINARY));
	}

	// --------------------------------------------------------------------------

	{ // Check (de-)serialization from/to a file in three different modes
		std::shared_ptr <GObject> p_test = this->clone();

		{ // Text mode
			BOOST_CHECK_NO_THROW(p_test->toFile(bf::path("123test.txt"), Gem::Common::serializationMode::TEXT));
			BOOST_CHECK_NO_THROW(p_test->fromFile(bf::path("123test.txt"), Gem::Common::serializationMode::TEXT));

			// Get rid of the file
			remove(bf::path("./123test.txt"));
		}

		{ // XML mode
			BOOST_CHECK_NO_THROW(p_test->toFile(bf::path("123test.xml"), Gem::Common::serializationMode::XML));
			BOOST_CHECK_NO_THROW(p_test->fromFile(bf::path("123test.xml"), Gem::Common::serializationMode::XML));

			// Get rid of the file
			remove(bf::path("./123test.xml"));
		}

		{ // Binary mode
			BOOST_CHECK_NO_THROW(p_test->toFile(bf::path("123test.bin"), Gem::Common::serializationMode::BINARY));
			BOOST_CHECK_NO_THROW(p_test->fromFile(bf::path("123test.bin"), Gem::Common::serializationMode::BINARY));

			// Get rid of the file
			remove(bf::path("./123test.bin"));
		}
	}

	// --------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GObject::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GObject::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
	// no parent class

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GObject::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
