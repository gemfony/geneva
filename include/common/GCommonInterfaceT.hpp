/**
 * @file GCommonInterfaceT.hpp
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

#ifndef GCOMMONINTERFACET_HPP_
#define GCOMMONINTERFACET_HPP_

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <string>
#include <fstream>
#include <memory>
#include <type_traits>

// Boost header files go here
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

// Geneva header files go here
#include "common/GCommonEnums.hpp" // For the serialization mode
#include "common/GExceptions.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GCommonHelperFunctionsT.hpp"

// aliases for ease of use
namespace bf = boost::filesystem;

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * This is an interface class that specifies common operations that must be
 * available for the majority of classes in the Gemfony scientific library.
 * As one example, (de-)serialization is simplified by some of the functions
 * in this class, as is the task of conversion to the derived types.
 */
template <typename g_class_type>
class GCommonInterfaceT
	 // This simplifies detection of classes that implement the Gemfony interface -- see GTypeTraits.hpp
	 // The problem here is that GCommonInterfaceT<g_class_type> is usually the base class of g_class_type and thus an incomplete
	 // type at the time type traits are applied. Hence we use another (trivial) base class that simplifies
	 // detection.
	: private gemfony_common_interface_indicator
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int)  {
		 using boost::serialization::make_nvp;

		 // no local data
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
    /** @brief The default constructor */
	 GCommonInterfaceT() = default;

	 /***************************************************************************/
	 /**
	  * The standard destructor. Making this destructor protected follows this
	  * discussion: http://www.gotw.ca/publications/mill18.htm
	  */
	 virtual ~GCommonInterfaceT() = default;

	 /** @brief The copy constructor -- no data, hence empty*/
	 GCommonInterfaceT(const GCommonInterfaceT<g_class_type>& cp) { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Converts the class(-hierarchy) to a serial representation that is
	  * then written to a stream.
	  *
	  * @param oarchive_stream The output stream the object should be written to
	  * @param serMod The desired serialization mode
	  */
	 void toStream(
		 std::ostream &oarchive_stream
		 , const Gem::Common::serializationMode &serMod
	 ) const {
		 const g_class_type *local;

		 // Note: (De-)serialization must happen through a pointer to the same type.
#ifdef DEBUG
		 local = dynamic_cast<const g_class_type *>(this);
		 if(!local) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GCommonInterfaceT<g_class_type>::toStream(): Error!" << std::endl
					 << "Conversion failed" << std::endl
			 );
		 }
#else
		 local = static_cast<const g_class_type *>(this);
#endif /* DEBUG */

		 switch (serMod) {
			 case Gem::Common::serializationMode::TEXT: {
				 boost::archive::text_oarchive oa(oarchive_stream);
				 oa << boost::serialization::make_nvp("classhierarchyFromT", local);
			 } // note: explicit scope here is essential so the oa-destructor gets called

				 break;

			 case Gem::Common::serializationMode::XML: {
				 boost::archive::xml_oarchive oa(oarchive_stream);
				 oa << boost::serialization::make_nvp("classhierarchyFromT", local);
			 } // note: explicit scope here is essential so the oa-destructor gets called

				 break;

			 case Gem::Common::serializationMode::BINARY: {
				 boost::archive::binary_oarchive oa(oarchive_stream);
				 oa << boost::serialization::make_nvp("classhierarchyFromT", local);
			 } // note: explicit scope here is essential so the oa-destructor gets called

				 break;
		 }
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested in GObject::specificTestsNoFailureExpected_GUnitTests()
	  * Tested indirectly through standard tests of toString
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Loads the object from a stream.
	  *
	  * @param istr The stream from which the object should be loaded
	  * @param serMod The desired serialization mode
	  *
	  */
	 void fromStream(
		 std::istream &istr
		 , const Gem::Common::serializationMode &serMod
	 ) {
		 g_class_type *local = nullptr;

		 switch (serMod) {
			 case Gem::Common::serializationMode::TEXT: {
				 boost::archive::text_iarchive ia(istr);
				 ia >> boost::serialization::make_nvp("classhierarchyFromT", local);
			 } // note: explicit scope here is essential so the ia-destructor gets called

				 break;

			 case Gem::Common::serializationMode::XML: {
				 boost::archive::xml_iarchive ia(istr);
				 ia >> boost::serialization::make_nvp("classhierarchyFromT", local);
			 } // note: explicit scope here is essential so the ia-destructor gets called

				 break;

			 case Gem::Common::serializationMode::BINARY: {
				 boost::archive::binary_iarchive ia(istr);
				 ia >> boost::serialization::make_nvp("classhierarchyFromT", local);
			 } // note: explicit scope here is essential so the ia-destructor gets called

				 break;
		 }

		 this->load_(local);
		 if(local) {
			 g_delete(local);
		 }
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested in GObject::specificTestsNoFailureExpected_GUnitTests()
	  * Tested indirectly through standard tests of fromString
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Converts the class to a text representation, using the currently set serialization mode for this
	  * class. Note that you will have to take care yourself that serialization and de-serialization
	  * happens in the same mode.
	  *
	  * @param serMod The desired serialization mode
	  * @return A text-representation of this class (or its derivative)
	  */
	 std::string toString(const Gem::Common::serializationMode &serMod) const {
		 std::ostringstream oarchive_stream;
		 toStream(oarchive_stream, serMod);
		 return oarchive_stream.str();
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested in GObject::specificTestsNoFailureExpected_GUnitTests()
	  * Tested as part of standard serialization tests in Geneva standard test suite
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Initializes the object from its string representation, using the currently set serialization mode.
	  * Note that the string will likely describe a derivative of g_class_type, as g_class_type cannot usually be instantiated.
	  * Note also that you will have to take care yourself that serialization and de-serialization happens
	  * in the same mode.
	  *
	  * @param descr A text representation of a g_class_type-derivative
	  */
	 void fromString(
		 const std::string &descr
		 , const Gem::Common::serializationMode &serMod
	 ) {
		 std::istringstream istr(descr);
		 fromStream(istr, serMod);
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested in GObject::specificTestsNoFailureExpected_GUnitTests()
	  * Tested as part of standard serialization tests in Geneva standard test suite
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Writes a serial representation of this object to a file. Can be used for check-pointing.
	  *
	  * @param p The name of the file the object should be saved to.
	  * @param serMod The desired serialization mode
	  */
	 void toFile(
		 const bf::path &p
		 , const Gem::Common::serializationMode &serMod
	 ) const {
		 bf::ofstream ofstr(p, std::ofstream::trunc); // Note: will overwrite existing files

		 if (!ofstr) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GCommonInterfaceT::toFile():" << std::endl
					 << "Problems connecting to file " << p.string() << std::endl
			 );
		 }

		 toStream(ofstr, serMod);
		 ofstr.close();

#ifdef DEBUG
		 if(!bf::exists(bf::path(p))) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GCommonInterfaceT::toFile():" << std::endl
					 << "Data was written to " << p.string() << std::endl
					 << "but file does not seem to exist." << std::endl
			 );
		 }
#endif
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested in GObject::specificTestsNoFailureExpected_GUnitTests()
	  * Part of the regular Geneva standard tests for every tested object
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Loads a serial representation of this object from file. Can be used for check-pointing.
	  *
	  * @param p The name of the file the object should be loaded from
	  * @param serMod The desired serialization mode
	  */
	 void fromFile(
		 const bf::path &p
		 , const Gem::Common::serializationMode &serMod
	 ) {
		 // Check that the file exists
		 if (!bf::exists(bf::path(p))) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GCommonInterfaceT::fromFile(): Error!" << std::endl
					 << "Requested input file " << p.string() << std::endl
					 << "does not exist." << std::endl
			 );
		 }

		 bf::ifstream ifstr(p);

		 if (!ifstr) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GCommonInterfaceT::fromFile():" << std::endl
					 << "Problem connecting to file " << p.string() << std::endl
			 );
		 }

		 fromStream(ifstr, serMod);
		 ifstr.close();
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested in GObject::specificTestsNoFailureExpected_GUnitTests()
	  * Part of the regular Geneva standard tests for every tested object
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Returns an XML description of the derivative it is called for
	  *
	  * @return An XML description of the GObject-derivative the function is called for
	  */
	 std::string report() const {
		 return toString(Gem::Common::serializationMode::XML);
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested in GObject::specificTestsNoFailureExpected_GUnitTests() // Check that
	  * the function does return a non-empty description. Content is not checked
	  * automatically.
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Emits a name for this class / object
	  */
	 virtual std::string name() const BASE {
		 return std::string("GCommonInterfaceT<g_class_type>");
	 }

	 /***************************************************************************/
	 /**
	  * Checks for compliance with expectations with respect to another object
	  * of type g_class_type. This purely virtual function ensures the well-formedness of the
	  * compare hierarchy in derived classes.
	  *
	  * @param cp A constant reference to another object of the same type, camouflaged as a base object
	  * @param e The expected outcome of the comparison
	  * @param limit The maximum deviation for floating point values (important for similarity checks)
	  */
	 virtual void compare(
		 const g_class_type& cp // the other object
		 , const Gem::Common::expectation& e // the expectation for this object, e.g. equality
		 , const double& limit // the limit for allowed deviations of floating point types
	 ) const BASE = 0;

	 /***************************************************************************/
	 /**
	  * Checks for compliance with expectations with respect to another object
	  * of the same type. This function does the real check. Without it we would get
	  * an error about "no known conversion from GCommonInterfaceT<g_class_type> to g_class_type.
	  *
	  * @param cp A constant reference to another object of the same type, camouflaged as a base object
	  * @param e The expected outcome of the comparison
	  * @param limit The maximum deviation for floating point values (important for similarity checks)
	  */
	 void compare(
		 const GCommonInterfaceT<g_class_type>& cp // the other object
		 , const Gem::Common::expectation& e // the expectation for this object, e.g. equality
		 , const double& limit // the limit for allowed deviations of floating point types
	 ) const {
		 using namespace Gem::Common;

		 // Check that cp isn't the same object as this one
		 Gem::Common::ptrDifferenceCheck(&cp, this);

		 // No parent classes to check...

		 // ... and no local data

		 // We consider two instances of this class to be always equal, as they
		 // do not have any local data and this is the base class. Hence
		 // we throw an expectation violation for the expectation CE_INEQUALITY.
		 if (Gem::Common::expectation::CE_INEQUALITY == e) {
			 throw g_expectation_violation(
				 "In GCommonInterfaceT<g_class_type>: instance is empty and a base class, hence the expectation of inequality is always violated."
			 );
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Creates a clone of this object, storing it in a std::shared_ptr<g_class_type>
	  */
	 std::shared_ptr<g_class_type> clone() const {
		 return std::shared_ptr<g_class_type>(clone_());
	 }

	 /***************************************************************************/
	 /**
	  * The function creates a clone of the g_class_type pointer, converts it to a pointer to a derived class
	  * and emits it as a std::shared_ptr<> . Note that this template will only be accessible to the
	  * compiler if g_class_type is a base type of clone_type.
	  *
	  * @return A converted clone of this object, wrapped into a std::shared_ptr
	  */
	 template <typename clone_type>
	 std::shared_ptr<clone_type> clone(
		 typename std::enable_if<std::is_base_of<g_class_type, clone_type>::value>::type *dummy = nullptr
	 ) const {
		 return Gem::Common::convertSmartPointer<g_class_type, clone_type>(std::shared_ptr<g_class_type>(this->clone_()));
	 }

	 /* ----------------------------------------------------------------------------------
	  * cloning is tested for all objects taking part in the Geneva standard tests
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Loads the data of another g_class_type(-derivative), wrapped in a shared pointer. Note that this
	  * function is only accessible to the compiler if load_type is a derivative of g_class_type.
	  *
	  * @param cp A copy of another g_class_type-derivative, wrapped into a std::shared_ptr<>
	  */
	 template <typename load_type>
	 inline void load(
		 const std::shared_ptr<load_type>& cp
		 , typename std::enable_if<std::is_base_of<g_class_type, load_type>::value>::type *dummy = nullptr
	 ) {
		 load_(cp.get());
	 }

	 /* ----------------------------------------------------------------------------------
	  * loading is tested for all objects taking part in the Geneva standard tests
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /**
	  * Loads the data of another g_class_type(-derivative), presented as a constant reference. Note that this
	  * function is only accessible to the compiler if load_type is a derivative of g_class_type.
	  *
	  * @param cp A copy of another g_class_type-derivative, wrapped into a std::shared_ptr<>
	  */
	 template <typename load_type>
	 inline void load(
		 const load_type& cp
		 , typename std::enable_if<std::is_base_of<g_class_type, load_type>::value>::type *dummy = nullptr
	 ) {
		 load_(&cp);
	 }

	 /* ----------------------------------------------------------------------------------
	  * loading is tested for all objects taking part in the Geneva standard tests
	  * ----------------------------------------------------------------------------------
	  */

protected:
	 /***************************************************************************/
	 /** @brief Loads the data of another g_class_type */
	 virtual G_API_COMMON void load_(const g_class_type*) BASE = 0;

private:
	 /***************************************************************************/
	 /** @brief Creates a deep clone of this object */
	 virtual G_API_COMMON g_class_type* clone_() const BASE = 0;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(GCommonInterfaceT<g_class_type>)

namespace boost {
namespace serialization {
template<typename g_class_type>
struct is_abstract<Gem::Common::GCommonInterfaceT<g_class_type>> : public boost::true_type {};
template<typename g_class_type>
struct is_abstract< const Gem::Common::GCommonInterfaceT<g_class_type>> : public boost::true_type {};
}
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

#endif /* GCOMMONINTERFACET_HPP_ */
