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
#include "geneva/GObjectExpectationChecksT.hpp"

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
GObject::GObject()
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GObject object
 */
GObject::GObject(const GObject& cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * As no memory is dynamically allocated in GObject, no work has to
 * be done by this destructor.
 */
GObject::~GObject()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another GObject(-derivative)
 * @return A constant reference to this object
 */
const GObject& GObject::operator=(const GObject& cp){
	load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks whether a given expectation for the relationship between this object and another object
 * is fulfilled.
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
   const GObject& cp
   , const Gem::Common::expectation& e
   , const double& limit
   , const std::string& caller
   , const std::string& y_name
   , const bool& withMessages
) const {
    using namespace Gem::Common;

    // Check that cp isn't the same object as this one
    this->selfAssignmentCheck<GObject>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// No parent classes to check...

	// ... and no local data

	return evaluateDiscrepancies("GMutableSetT<T>", caller, deviations, e);
}

/* ----------------------------------------------------------------------------------
 * UNCHECKED
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Returns an XML description of the derivative it is called for
 *
 * @return An XML description of the GObject-derivative the function is called for
 */
std::string GObject::report() const {
	return toString(Gem::Common::SERIALIZATIONMODE_XML);
}

/* ----------------------------------------------------------------------------------
 * Tested in GObject::specificTestsNoFailureExpected_GUnitTests() // Check that
 * the function does return a non-empty description. Content is not checked
 * automatically.
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
 * Converts class to a serial representation that is then written to a stream.
 *
 * @param oarchive_stream The output stream the object should be written to
 * @param serMod The desired serialization mode
 */
void GObject::toStream(
   std::ostream& oarchive_stream
   , const Gem::Common::serializationMode& serMod
) const {
   const GObject *local = this; // Serialization should happen through a base pointer

   switch(serMod)
   {
      case Gem::Common::SERIALIZATIONMODE_TEXT:
      {
         boost::archive::text_oarchive oa(oarchive_stream);
         oa << boost::serialization::make_nvp("classhierarchyFromGObject", local);
      } // note: explicit scope here is essential so the oa-destructor gets called

      break;

      case Gem::Common::SERIALIZATIONMODE_XML:
      {
         boost::archive::xml_oarchive oa(oarchive_stream);
         oa << boost::serialization::make_nvp("classhierarchyFromGObject", local);
      } // note: explicit scope here is essential so the oa-destructor gets called

      break;

      case Gem::Common::SERIALIZATIONMODE_BINARY:
      {
         boost::archive::binary_oarchive oa(oarchive_stream);
         oa << boost::serialization::make_nvp("classhierarchyFromGObject", local);
      } // note: explicit scope here is essential so the oa-destructor gets called

      break;
   }
}

/* ----------------------------------------------------------------------------------
 * Tested in GObject::specificTestsNoFailureExpected_GUnitTests()
 * Tested indirectly through standard tests of toString
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Loads the object from a stream.
 *
 * @param istr The stream from which the object should be loaded
 * @param serMod The desired serialization mode
 *
 */
void GObject::fromStream(
   std::istream& istr
   , const Gem::Common::serializationMode& serMod
) {
   // De-serialization and serialization should happen through a pointer to the same type.
   GObject *local = (GObject *)NULL;

   switch(serMod)
   {
      case Gem::Common::SERIALIZATIONMODE_TEXT:
      {
         boost::archive::text_iarchive ia(istr);
         ia >> boost::serialization::make_nvp("classhierarchyFromGObject", local);
      } // note: explicit scope here is essential so the ia-destructor gets called

      break;

      case Gem::Common::SERIALIZATIONMODE_XML:
      {
         boost::archive::xml_iarchive ia(istr);
         ia >> boost::serialization::make_nvp("classhierarchyFromGObject", local);
      } // note: explicit scope here is essential so the ia-destructor gets called

      break;

      case Gem::Common::SERIALIZATIONMODE_BINARY:
      {
         boost::archive::binary_iarchive ia(istr);
         ia >> boost::serialization::make_nvp("classhierarchyFromGObject", local);
      } // note: explicit scope here is essential so the ia-destructor gets called

      break;
   }

   load_(local);
   if(local) delete local;
}

/* ----------------------------------------------------------------------------------
 * Tested in GObject::specificTestsNoFailureExpected_GUnitTests()
 * Tested indirectly through standard tests of fromString
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Converts the class to a text representation, using the currently set serialization mode for this
 * class. Note that you will have to take care yourself that serialization and de-serialization
 * happens in the same mode.
 *
 * @param serMod The desired serialization mode
 * @return A text-representation of this class (or its derivative)
 */
std::string GObject::toString(const Gem::Common::serializationMode& serMod) const {
	std::ostringstream oarchive_stream;
	toStream(oarchive_stream, serMod);
    return oarchive_stream.str();
}

/* ----------------------------------------------------------------------------------
 * Tested in GObject::specificTestsNoFailureExpected_GUnitTests()
 * Tested as part of standard serialization tests in Geneva standard test suite
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Initializes the object from its string representation, using the currently set serialization mode.
 * Note that the string will likely describe a derivative of GObject, as GObject cannot be instantiated.
 * Note also that you will have to take care yourself that serialization and de-serialization happens
 * in the same mode.
 *
 * @param descr A text representation of a GObject-derivative

 */
void GObject::fromString(
   const std::string& descr
   , const Gem::Common::serializationMode& serMod
) {
   std::istringstream istr(descr);
   fromStream(istr, serMod);
}

/* ----------------------------------------------------------------------------------
 * Tested in GObject::specificTestsNoFailureExpected_GUnitTests()
 * Tested as part of standard serialization tests in Geneva standard test suite
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Writes a serial representation of this object to a file. Can be used for check-pointing.
 *
 * @param p The name of the file the object should be saved to.
 * @param serMod The desired serialization mode
 */
void GObject::toFile(
     const bf::path& p
   , const Gem::Common::serializationMode& serMod
) const {
   bf::ofstream ofstr(p);

   if(!ofstr) {
      glogger
      << "In GObject::toFile():" << std::endl
      << "Problems connecting to file " << p.string() << std::endl
      << GEXCEPTION;
   }

   toStream(ofstr, serMod);
   ofstr.close();

#ifdef DEBUG
   if(!bf::exists(bf::path(p))) {
      glogger
      << "In GObject::toFile():" << std::endl
      << "Data was written to " << p.string() << std::endl
      << "but file does not seem to exist." << std::endl
      << GEXCEPTION;
   }
#endif
}

/* ----------------------------------------------------------------------------------
 * Tested in GObject::specificTestsNoFailureExpected_GUnitTests()
 * Part of the regular Geneva standard tests for every tested object
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Loads a serial representation of this object from file. Can be used for check-pointing.
 *
 * @param p The name of the file the object should be loaded from
 * @param serMod The desired serialization mode
 */
void GObject::fromFile(
     const bf::path& p
   , const Gem::Common::serializationMode& serMod
) {
   // Check that the file exists
   if(!bf::exists(bf::path(p))) {
      glogger
      << "In GObject::fromFile(): Error!" << std::endl
      << "Requested input file " << p.string() << std::endl
      << "does not exist." << std::endl
      << GEXCEPTION;
   }

   bf::ifstream ifstr(p);

	if(!ifstr) {
	   glogger
	   << "In GObject::fromFile():" << std::endl
	   << "Problem connecting to file " << p.string() << std::endl
	   << GEXCEPTION;
	}

	fromStream(ifstr, serMod);
	ifstr.close();
}

/* ----------------------------------------------------------------------------------
 * Tested in GObject::specificTestsNoFailureExpected_GUnitTests()
 * Part of the regular Geneva standard tests for every tested object
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
   const std::string& configFile
   , const std::string& header
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
void GObject::readConfigFile(const std::string& configFile) {
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
	// Checks whether we are accidently assigning the object to itself
	selfAssignmentCheck<GObject>(cp);

	// No local data
}

/* ----------------------------------------------------------------------------------
 * Loading is checked as part of the Geneva standard test suite
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
/**
 * Creates a deep clone of this object, storing it in a boost::shared_ptr<GObject>
 *
 * @return A boost::shared_ptr<GObject> to a clone of the derived object
 */
boost::shared_ptr<GObject> GObject::clone() const {
	return boost::shared_ptr<GObject>(clone_());
}

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
	Gem::Common::GParserBuilder& gpb
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
		boost::shared_ptr<GObject> p_test = this->clone<GObject>();

		// Check that the pointer actually points somewhere
		BOOST_CHECK(p_test);
	}

   // --------------------------------------------------------------------------

   { // Check the name of the object
      boost::shared_ptr<GObject> p_test = this->clone<GObject>();

      // Check that the pointer actually points somewhere
      BOOST_CHECK_MESSAGE(
            p_test->GObject::name() == "GObject"
            , "Name is " << p_test->GObject::name()
      );
   }

	// --------------------------------------------------------------------------

	{ // Check cloning to GObject format, using a different method
		boost::shared_ptr<GObject> p_test = this->clone();

		// Check that the pointer actually points somewhere
		BOOST_CHECK(p_test);
	}

	// --------------------------------------------------------------------------

	{ // Check that the report function returns a non-empty description
		boost::shared_ptr<GObject> p_test = this->clone();

		// Check that the pointer actually points somewhere
		BOOST_CHECK(!(p_test->report()).empty());
	}


	{ // Check (de-)serialization from/to a stream in three modes
		boost::shared_ptr<GObject> p_test = this->clone();

		{ // Text mode
			std::ostringstream ostr;
			BOOST_CHECK_NO_THROW(p_test->toStream(ostr, Gem::Common::SERIALIZATIONMODE_TEXT));
			std::istringstream istr(ostr.str());
			BOOST_CHECK_NO_THROW(p_test->fromStream(istr, Gem::Common::SERIALIZATIONMODE_TEXT));
		}

		{ // XML mode
			std::ostringstream ostr;
			BOOST_CHECK_NO_THROW(p_test->toStream(ostr, Gem::Common::SERIALIZATIONMODE_XML));
			std::istringstream istr(ostr.str());
			BOOST_CHECK_NO_THROW(p_test->fromStream(istr, Gem::Common::SERIALIZATIONMODE_XML));
		}

		{ // Binary mode
			std::ostringstream ostr;
			BOOST_CHECK_NO_THROW(p_test->toStream(ostr, Gem::Common::SERIALIZATIONMODE_BINARY));
			std::istringstream istr(ostr.str());
			BOOST_CHECK_NO_THROW(p_test->fromStream(istr, Gem::Common::SERIALIZATIONMODE_BINARY));
		}
	}

	// --------------------------------------------------------------------------

	{ // Check (de-)serialization from/to strings in three modes
		boost::shared_ptr<GObject> p_test = this->clone();

		BOOST_CHECK_NO_THROW(p_test->fromString(p_test->toString(Gem::Common::SERIALIZATIONMODE_TEXT), Gem::Common::SERIALIZATIONMODE_TEXT));
		BOOST_CHECK_NO_THROW(p_test->fromString(p_test->toString(Gem::Common::SERIALIZATIONMODE_XML), Gem::Common::SERIALIZATIONMODE_XML));
		BOOST_CHECK_NO_THROW(p_test->fromString(p_test->toString(Gem::Common::SERIALIZATIONMODE_BINARY), Gem::Common::SERIALIZATIONMODE_BINARY));
	}

	// --------------------------------------------------------------------------

	{ // Check (de-)serialization from/to a file in three different modes
		boost::shared_ptr<GObject> p_test = this->clone();

		{ // Text mode
			BOOST_CHECK_NO_THROW(p_test->toFile(bf::path("123test.txt"), Gem::Common::SERIALIZATIONMODE_TEXT));
			BOOST_CHECK_NO_THROW(p_test->fromFile(bf::path("123test.txt"), Gem::Common::SERIALIZATIONMODE_TEXT));

			// Get rid of the file
			remove(bf::path("./123test.txt"));
		}

		{ // XML mode
			BOOST_CHECK_NO_THROW(p_test->toFile(bf::path("123test.xml"), Gem::Common::SERIALIZATIONMODE_XML));
			BOOST_CHECK_NO_THROW(p_test->fromFile(bf::path("123test.xml"), Gem::Common::SERIALIZATIONMODE_XML));

			// Get rid of the file
			remove(bf::path("./123test.xml"));
		}

		{ // Binary mode
			BOOST_CHECK_NO_THROW(p_test->toFile(bf::path("123test.bin"), Gem::Common::SERIALIZATIONMODE_BINARY));
			BOOST_CHECK_NO_THROW(p_test->fromFile(bf::path("123test.bin"), Gem::Common::SERIALIZATIONMODE_BINARY));

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
