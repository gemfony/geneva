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

#ifndef GCOMMONINTERFACET_HPP_
#define GCOMMONINTERFACET_HPP_

// Geneva header files go here
#include "common/GCommonEnums.hpp" // For the serialization mode
#include "common/GExceptions.hpp"

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
template <typename T>
class GCommonInterfaceT {
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive &ar, const unsigned int)  {
		using boost::serialization::make_nvp;

		// no local data
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The standard destructor */
	virtual ~GCommonInterfaceT() { /* nothing */ }

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
		const T *local;

		// Note: (De-)serialization must happen through a pointer to the same type.
#ifdef DEBUG
		local = dynamic_cast<const T *>(this);
		if(!local) {
			glogger
			<< "In GCommonInterfaceT<T>::toStream(): Error!" << std::endl
			<< "Conversion failed" << std::endl
			<< GEXCEPTION;
		}
#else
		local = static_cast<const T *>(this);
#endif /* DEBUG */

		switch (serMod) {
			case Gem::Common::SERIALIZATIONMODE_TEXT: {
				boost::archive::text_oarchive oa(oarchive_stream);
				oa << boost::serialization::make_nvp("classhierarchyFromT", local);
			} // note: explicit scope here is essential so the oa-destructor gets called

				break;

			case Gem::Common::SERIALIZATIONMODE_XML: {
				boost::archive::xml_oarchive oa(oarchive_stream);
				oa << boost::serialization::make_nvp("classhierarchyFromT", local);
			} // note: explicit scope here is essential so the oa-destructor gets called

				break;

			case Gem::Common::SERIALIZATIONMODE_BINARY: {
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
		T *local = nullptr;

		switch (serMod) {
			case Gem::Common::SERIALIZATIONMODE_TEXT: {
				boost::archive::text_iarchive ia(istr);
				ia >> boost::serialization::make_nvp("classhierarchyFromT", local);
			} // note: explicit scope here is essential so the ia-destructor gets called

				break;

			case Gem::Common::SERIALIZATIONMODE_XML: {
				boost::archive::xml_iarchive ia(istr);
				ia >> boost::serialization::make_nvp("classhierarchyFromT", local);
			} // note: explicit scope here is essential so the ia-destructor gets called

				break;

			case Gem::Common::SERIALIZATIONMODE_BINARY: {
				boost::archive::binary_iarchive ia(istr);
				ia >> boost::serialization::make_nvp("classhierarchyFromT", local);
			} // note: explicit scope here is essential so the ia-destructor gets called

				break;
		}

		this->load_(local);
		if(local) {
			delete local;
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
	 * Note that the string will likely describe a derivative of T, as T cannot usually be instantiated.
	 * Note also that you will have to take care yourself that serialization and de-serialization happens
	 * in the same mode.
	 *
	 * @param descr A text representation of a T-derivative
	 */
	void fromString(
		const std::string &descr, const Gem::Common::serializationMode &serMod
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
		const bf::path &p, const Gem::Common::serializationMode &serMod
	) const {
		bf::ofstream ofstr(p);

		if (!ofstr) {
			glogger
			<< "In GCommonInterfaceT::toFile():" << std::endl
			<< "Problems connecting to file " << p.string() << std::endl
			<< GEXCEPTION;
		}

		toStream(ofstr, serMod);
		ofstr.close();

#ifdef DEBUG
		if(!bf::exists(bf::path(p))) {
			glogger
			<< "In GCommonInterfaceT::toFile():" << std::endl
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

	/***************************************************************************/
	/**
	 * Loads a serial representation of this object from file. Can be used for check-pointing.
	 *
	 * @param p The name of the file the object should be loaded from
	 * @param serMod The desired serialization mode
	 */
	void fromFile(
		const bf::path &p, const Gem::Common::serializationMode &serMod
	) {
		// Check that the file exists
		if (!bf::exists(bf::path(p))) {
			glogger
			<< "In GCommonInterfaceT::fromFile(): Error!" << std::endl
			<< "Requested input file " << p.string() << std::endl
			<< "does not exist." << std::endl
			<< GEXCEPTION;
		}

		bf::ifstream ifstr(p);

		if (!ifstr) {
			glogger
			<< "In GCommonInterfaceT::fromFile():" << std::endl
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

	/***************************************************************************/
	/**
	 * Returns an XML description of the derivative it is called for
	 *
	 * @return An XML description of the GObject-derivative the function is called for
	 */
	std::string report() const {
		return toString(Gem::Common::SERIALIZATIONMODE_XML);
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
	virtual std::string name() const {
		return std::string("GCommonInterfaceT<T>");
	}

	/***************************************************************************/
	/** @brief Searches for compliance with expectations with respect to another object of the same type */
	virtual void compare(
		const T& // the other object
		, const Gem::Common::expectation& // the expectation for this object, e.g. equality
		, const double& // the limit for allowed deviations of floating point types
	) const BASE = 0;

	/** @brief Creates a clone of this object, storing it in a std::shared_ptr<T> */
	std::shared_ptr<T> clone() const {
		return std::shared_ptr<T>(clone_());
	}

	/***************************************************************************/
	/**
	 * The function creates a clone of the T pointer, converts it to a pointer to a derived class
	 * and emits it as a std::shared_ptr<> . Note that this template will only be accessible to the
	 * compiler if T is a base type of clone_type.
	 *
	 * @return A converted clone of this object, wrapped into a std::shared_ptr
	 */
	template <typename clone_type>
	std::shared_ptr<clone_type> clone(
		typename boost::enable_if<boost::is_base_of<T, clone_type>>::type* dummy = 0
	) const {
		return Gem::Common::convertSmartPointer<T, clone_type>(std::shared_ptr<T>(this->clone_()));
	}

	/* ----------------------------------------------------------------------------------
	 * cloning is tested for all objects taking part in the Geneva standard tests
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Loads the data of another T(-derivative), wrapped in a shared pointer. Note that this
	 * function is only accessible to the compiler if load_type is a derivative of T.
	 *
	 * @param cp A copy of another T-derivative, wrapped into a std::shared_ptr<>
	 */
	template <typename load_type>
	inline void load(
		const std::shared_ptr<load_type>& cp
		, typename boost::enable_if<boost::is_base_of<T, load_type>>::type* dummy = 0
	) {
		load_(cp.get());
	}

	/* ----------------------------------------------------------------------------------
	 * loading is tested for all objects taking part in the Geneva standard tests
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Loads the data of another T(-derivative), presented as a constant reference. Note that this
	 * function is only accessible to the compiler if load_type is a derivative of T.
	 *
	 * @param cp A copy of another T-derivative, wrapped into a std::shared_ptr<>
	 */
	template <typename load_type>
	inline void load(
		const load_type& cp
		, typename boost::enable_if<boost::is_base_of<T, load_type>>::type* dummy = 0
	) {
		load_(&cp);
	}

	/* ----------------------------------------------------------------------------------
	 * loading is tested for all objects taking part in the Geneva standard tests
	 * ----------------------------------------------------------------------------------
	 */

protected:
	/***************************************************************************/
	/** @brief Loads the data of another T */
	virtual G_API_COMMON void load_(const T*) BASE = 0;
	/** @brief Creates a deep clone of this object */
	virtual G_API_COMMON T* clone_() const BASE = 0;
};

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
namespace serialization {
template<typename T>
struct is_abstract<Gem::Common::GCommonInterfaceT<T>> : public boost::true_type {};
template<typename T>
struct is_abstract< const Gem::Common::GCommonInterfaceT<T>> : public boost::true_type {};
}
}

/******************************************************************************/


#endif /* GCOMMONINTERFACET_HPP_ */
