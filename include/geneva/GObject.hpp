/**
 * @file GObject.hpp
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
#include <algorithm>
#include <assert.h>
#include <cmath>
#include <cfloat>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>

// Boost header files go here
#include <boost/any.hpp>
#include <boost/bind.hpp>
#include <boost/cast.hpp>
#include <boost/cstdint.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/limits.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/logic/tribool_io.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/numeric/conversion/bounds.hpp> // get rid of the numeric_limits<double>::min() vs. numeric_limits<int>::min() problem
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_io.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/variant.hpp>

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
#include <boost/property_tree/ptree_serialization.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#ifdef GEM_TESTING
#include <boost/test/unit_test.hpp>
#endif /* GEM_TESTING */

/** Check that we have support for threads */
#ifndef BOOST_HAS_THREADS
#error "Error: Support for multi-threading does not seem to be available."
#endif

#ifndef GOBJECT_HPP_
#define GOBJECT_HPP_

// Geneva header files go here
#include "common/GDefaultValueT.hpp"
#include "common/GExceptions.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "common/GLogger.hpp"
#include "common/GSerializableI.hpp"
#include "common/GParserBuilder.hpp"
#include "common/GSerializeTupleT.hpp"
#include "geneva/GOptimizationEnums.hpp"

#ifdef GEM_TESTING
#include "common/GUnitTestFrameworkT.hpp"
#endif /* GEM_TESTING */

// aliases for ease of use
namespace bf = boost::filesystem;
namespace pt = boost::property_tree;

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * GObject is the parent class for the majority of Geneva optimization classes. Essentially, GObject
 * defines a number of interface functions and access patterns commonly needed throughout derived classes.
 * As one example, (de-)serialization is simplified by some of the functions in this class, as is the
 * task of conversion to the derived types. Handly of optimization-related classes often happens through
 * a boost::shared_ptr<GObject>, hence this class has a very central role. The GObject::load_(const GObject *)
 * and  GObject::clone_() member functions must be re-implemented for each derived class. Further common
 * functionality of many Geneva classes will be implemented here over time.
 */
class GObject
	:public Gem::Common::GSerializableI
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    G_API_GENEVA void serialize(Archive &, const unsigned int)  {
      using boost::serialization::make_nvp;

      // No local data
    }
    ///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
   G_API_GENEVA GObject() ;
	/** @brief The copy constructor */
   G_API_GENEVA GObject(const GObject& cp) ;
	/** @brief The destructor */
	virtual G_API_GENEVA ~GObject();

	/** @brief The assignment operator */
	G_API_GENEVA const GObject& operator=(const GObject&);

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual G_API_GENEVA boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
	) const;

	/** @brief Convert class to a serial representation that is then written to a stream */
	G_API_GENEVA void toStream(std::ostream&, const Gem::Common::serializationMode&) const;
	/** @brief Load class from a stream */
	G_API_GENEVA void fromStream(std::istream&, const Gem::Common::serializationMode&);

	/** @brief Convert class to a serial representation, using a user-specified serialization mode */
	virtual G_API_GENEVA std::string toString(const Gem::Common::serializationMode&) const OVERRIDE;
	/** @brief Convert class to a serial representation, using a specific serialization mode */
	virtual G_API_GENEVA void fromString(const std::string&, const Gem::Common::serializationMode&) OVERRIDE;

	/** @brief Writes a serial representation of this object to a file */
	G_API_GENEVA void toFile(const bf::path&, const Gem::Common::serializationMode&) const;
	/** @brief Loads a serial representation of this object from file */
	G_API_GENEVA void fromFile(const bf::path&, const Gem::Common::serializationMode&);

	/** @brief Returns an XML description of the derivative it is called for */
	G_API_GENEVA std::string report() const;

   /** @brief Emits a name for this class / object */
   virtual G_API_GENEVA std::string name() const;

	/** @brief Writes a configuration file to disk */
   G_API_GENEVA void writeConfigFile(const std::string&, const std::string&);
	/** @brief Reads a configuration file from disk */
   G_API_GENEVA void readConfigFile(const std::string&);

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual G_API_GENEVA void addConfigurationOptions(Gem::Common::GParserBuilder&);

	/** @brief Creates a clone of this object, storing it in a boost::shared_ptr<GObject> */
	G_API_GENEVA boost::shared_ptr<GObject> clone() const;

   /***************************************************************************/
   /**
    * The function creates a clone of the GObject pointer, converts it to a pointer to a derived class
    * and emits it as a boost::shared_ptr<> . Note that this template will only be accessible to the
    * compiler if GObject is a base type of clone_type.
    *
    * @return A converted clone of this object, wrapped into a boost::shared_ptr
    */
   template <typename clone_type>
   G_API_GENEVA boost::shared_ptr<clone_type> clone(
      typename boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, clone_type> >::type* dummy = 0
   ) const {
      return Gem::Common::convertSmartPointer<GObject, clone_type>(boost::shared_ptr<GObject>(this->clone_()));
   }

   /* ----------------------------------------------------------------------------------
    * cloning is tested for all objects taking part in the Geneva standard tests
    * ----------------------------------------------------------------------------------
    */

	/***************************************************************************/
	/**
	 * Loads the data of another GObject(-derivative), wrapped in a shared pointer. Note that this
	 * function is only accessible to the compiler if load_type is a derivative of GObject.
	 *
	 * @param cp A copy of another GObject-derivative, wrapped into a boost::shared_ptr<>
	 */
	template <typename load_type>
	inline G_API_GENEVA void load(
      const boost::shared_ptr<load_type>& cp
      , typename boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, load_type> >::type* dummy = 0
	) {
		load_(cp.get());
	}

	/* ----------------------------------------------------------------------------------
	 * loading is tested for all objects taking part in the Geneva standard tests
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Loads the data of another GObject(-derivative), presented as a constant reference. Note that this
	 * function is only accessible to the compiler if load_type is a derivative of GObject.
	 *
	 * @param cp A copy of another GObject-derivative, wrapped into a boost::shared_ptr<>
	 */
	template <typename load_type>
	inline G_API_GENEVA void load(
      const load_type& cp
      , typename boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, load_type> >::type* dummy = 0
	) {
		load_(&cp);
	}

	/* ----------------------------------------------------------------------------------
	 * loading is tested for all objects taking part in the Geneva standard tests
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Checks whether a SIGHUP or CTRL_CLOSE_EVENT signal has been sent
	 */
	static G_API_GENEVA bool G_SIGHUP_SENT() {
	   return (1==GObject::GenevaSigHupSent);
	}

	/***************************************************************************/
	/**
	 * A handler for SIGHUP or CTRL_CLOSE_EVENT signals. This function should work
	 * both for Windows and Unix-Systems.
	 */
	static G_API_GENEVA void sigHupHandler(int signum) {
	   if(G_SIGHUP == signum) {
	      GObject::GenevaSigHupSent = 1;
	   }
	}

protected:
	/***************************************************************************/
	/** @brief Loads the data of another GObject */
	virtual G_API_GENEVA void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual G_API_GENEVA GObject* clone_() const = 0;

	/***************************************************************************/
	/**
	 * This function checks in DEBUG mode whether a load pointer points to the current object. Note
	 * that this template will only be accessible to the compiler if GObject is a base type of load_type.
	 */
	template <typename load_type>
	inline G_API_GENEVA void selfAssignmentCheck (
			const GObject *load_ptr
		  , typename boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, load_type> >::type* dummy = 0
	) const {
#ifdef DEBUG
		// Check that this object is not accidentally assigned to itself.
		if (load_ptr == this) {
			glogger
			<< "In GObject::selfAssignmentCheck<load_type>() :" << std::endl
			<< "Tried to assign an object to or compare with itself." << std::endl
			<< GEXCEPTION;
		}
#endif
	}

	/* ----------------------------------------------------------------------------------
	 * selfAssignment checks are performed for all objects taking part in the Geneva standard tests.
	 * There is also an explicit test in the standard tests suite.
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * This function converts the GObject pointer to the target type, checking for self-assignment
	 * along the ways in DEBUG mode (through selfAssignmentCheck() ).  Note that this template will
	 * only be accessible to the compiler if GObject is a base type of load_type.
	 */
	template <typename load_type>
	inline G_API_GENEVA const load_type* gobject_conversion (
     const GObject *load_ptr
     , typename boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, load_type> >::type* dummy = 0
	) const {
		selfAssignmentCheck<load_type>(load_ptr);

#ifdef DEBUG
		const load_type *p = dynamic_cast<const load_type *>(load_ptr);
		if(p) return p;
		else {
		   glogger
		   << "In const GObject* GObject::gobject_conversion<load_type>() :" << std::endl
		   << "Invalid conversion to type with type name " << typeid(load_type).name() << std::endl
		   << GEXCEPTION;

		   // Make the compiler happy
		   return (load_type *)NULL;
		}
#else
		return static_cast<const load_type *>(load_ptr);
#endif
	}

	/* ----------------------------------------------------------------------------------
	 * gobject_conversions are regularly performed as part of the loading process and are thus
	 * considered to be well tested.
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * This function converts a GObject boost::shared_ptr to the target type, optionally checking for
	 * self-assignment along the ways in DEBUG mode (through selfAssignmentCheck() ).  Note that this
	 * template will only be accessible to the compiler if GObject is a base type of load_type.
	 *
	 * @param load_ptr A boost::shared_ptr<load_type> to the item to be converted
	 * @param dummy A dummy argument needed for boost's enable_if and type traits magic
	 * @return A boost::shared_ptr holding the converted object
	 */
	template <typename load_type>
	inline G_API_GENEVA boost::shared_ptr<load_type> gobject_conversion (
     boost::shared_ptr<GObject> load_ptr
     , typename boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, load_type> >::type* dummy = 0
	) const {
		selfAssignmentCheck<load_type>(load_ptr.get());

#ifdef DEBUG
		boost::shared_ptr<load_type> p = boost::dynamic_pointer_cast<load_type>(load_ptr);
		if(p) return p;
		else {
		   glogger
		   << "In boost::shared_ptr<load_type> GObject::gobject_conversion<load_type>() :" << std::endl
		   << "Invalid conversion" << std::endl
		   << GEXCEPTION;
		}
#else
		return boost::static_pointer_cast<load_type>(load_ptr);
#endif
	}

	/* ----------------------------------------------------------------------------------
	 * gobject_conversions are regularly performed as part of the loading process and are thus
	 * considered to be well tested.
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/

private:
	/** @brief Checks for equality with another GObject object. Intentionally left undefined, as this class is abstract */
	bool operator==(const GObject&) const;
	/** @brief Checks inequality with another GObject object. Intentionally left undefined, as this class is abstract */
	bool operator!=(const GObject&) const;

	// Needed to allow interruption of the optimization run without loss of data
	// Npte that "volatile" is needed in order for the signal handler to work
	static volatile G_API_GENEVA std::sig_atomic_t GenevaSigHupSent;  // Initialized in GObject.cpp

public:
	/***************************************************************************/
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual G_API_GENEVA bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests();
};

/******************************************************************************/
/**
 * A specialization of the general clone for cases where no conversion takes place at all
 *
 * @return A boost::shared_ptr<GObject> to a clone of the derived object
 */
template <>
inline G_API_GENEVA boost::shared_ptr<GObject> GObject::clone<GObject> (
   boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, GObject> >::type* dummy
) const {
   return boost::shared_ptr<GObject>(clone_());
}

/* ----------------------------------------------------------------------------------
 * Tested in GObject::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GObject)

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
// Tests for the GObject class

#endif /* GOBJECT_HPP_ */
