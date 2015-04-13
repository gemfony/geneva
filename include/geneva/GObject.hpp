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
#include <deque>
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
#include <boost/mpl/has_xxx.hpp>
#include <boost/numeric/conversion/bounds.hpp> // get rid of the numeric_limits<double>::min() vs. numeric_limits<int>::min() problem
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/thread/tss.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_io.hpp>
#include <boost/type_traits.hpp>
#include <boost/type_traits/is_abstract.hpp>
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
#include "common/GExpectationChecksT.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "common/GLogger.hpp"
#include "common/GSerializableI.hpp"
#include "common/GParserBuilder.hpp"
#include "common/GSerializeTupleT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "hap/GRandomT.hpp"

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
 * task of conversion to the derived types. Handling of optimization-related classes sometimes happens through
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
    void serialize(Archive &, const unsigned int)  {
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

	/** @brief Searches for compliance with expectations with respect to another object of the same type */
	virtual G_API_GENEVA void compare(
      const GObject& // the other object
      , const Gem::Common::expectation& // the expectation for this object, e.g. equality
      , const double& // the limit for allowed deviations of floating point types
	) const BASE;

	/** @brief Allows derived classes to assign other class'es values */
	G_API_GENEVA const GObject& operator=(const GObject&);

   /** @brief Checks for equality with another GObject object */
   virtual G_API_GENEVA bool operator==(const GObject&) const;
   /** @brief Checks for inequality with another GObject object */
   virtual G_API_GENEVA bool operator!=(const GObject&) const;

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
   boost::shared_ptr<clone_type> clone(
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
	inline void load(
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
	inline void load(
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

   /***************************************************************************/
	/**
	 * Central access to a random number generator through thread-local storage
	 */
	inline boost::thread_specific_ptr<Gem::Hap::GRandom>& gr_ptr() {
	   static boost::thread_specific_ptr<Gem::Hap::GRandom> instance;
	   if(!instance.get()) {
	      instance.reset(new Gem::Hap::GRandom());
	   }
	   return instance;
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
	inline void selfAssignmentCheck (
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
	inline const load_type* gobject_conversion (
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
	inline boost::shared_ptr<load_type> gobject_conversion (
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
inline boost::shared_ptr<GObject> GObject::clone<GObject> (
   boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, GObject> >::type* dummy
) const {
   return boost::shared_ptr<GObject>(clone_());
}

/* ----------------------------------------------------------------------------------
 * Tested in GObject::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/******************************************************************************/
// A define for the compare-infrastructure (used e.g. in operator== and in unit tests)
#define COMPARE_PARENT(p,c,e,l) \
g_n_tests++; \
switch(e){ \
case Gem::Common::CE_FP_SIMILARITY: \
case Gem::Common::CE_EQUALITY: \
{ \
    p::compare((c),(e),(l));\
} \
break; \
\
case Gem::Common::CE_INEQUALITY: \
{ \
   try{ \
      p::compare((c),(e),(l)); \
   } catch(g_expectation_violation&) { \
       g_n_violations++; \
   } \
} \
break; \
\
default: \
{ \
   glogger \
   << "Got invalid expectation " << e << std::endl \
   << GEXCEPTION; \
} \
break; \
}; \

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

namespace Gem {
namespace Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This function checks whether two complex types meet a given expectation. It is assumed that
 * these types have the standard Geneva interface with corresponding "compare" functions.
 *
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 */
template <typename geneva_type>
void compare (
   const geneva_type& x
   , const geneva_type& y
   , const std::string& x_name
   , const std::string& y_name
   , const Gem::Common::expectation& e
   , const double& limit = Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
   , typename boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, geneva_type> >::type* dummy = 0
) {
   bool expectationMet = false;
   std::string expectation_str;
   std::ostringstream error;

   switch(e) {
   case Gem::Common::CE_FP_SIMILARITY:
   case Gem::Common::CE_EQUALITY:
   {
      expectation_str = "CE_FP_SIMILARITY / CE_EQUALITY";

      // If we reach this line, then both pointers have content

      { // Check whether the content differs
         try {
            x.compare(y,e,limit);
         } catch(g_expectation_violation& g) {
            error
            << "Content of " << x_name << " and " << y_name << " differ." << std::endl
            << "Thus the expectation of " << expectation_str << " was violated:" << std::endl
            << g.what() << std::endl;
            break; // Terminate the switch statement
         }

         // If we reach this line, the expectation was met
         expectationMet = true;
      }
   }
   break;

   case Gem::Common::CE_INEQUALITY:
   {
      expectation_str = "CE_INEQUALITY";

      // Check whether the content differs
      try {
         x.compare(y,e,limit);
      } catch(g_expectation_violation& g) {
         // If we catch an expectation violation for expectation "inequality",
         // we simply break the switch statement so that expectationMet remains to be false
         error
         << "Content of " << x_name << " and " << y_name << " are equal/similar." << std::endl
         << "Thus the expectation of " << expectation_str << " was violated:" << std::endl
         << g.what() << std::endl;
         break;
      }
      expectationMet = true;
   }
   break;

   default:
   {
      glogger
      << "In compare(/* 6 */): Got invalid expectation " << e << std::endl
      << GEXCEPTION;
   }
   break;
   };

   if(!expectationMet) {
      throw g_expectation_violation(error.str());
   }
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This function checks whether two smart pointers to complex types meet a given expectation.
 * It is assumed that these types have the standard Geneva interface with corresponding "compare"
 * functions.
 *
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 */
template <typename geneva_type>
void compare (
   const boost::shared_ptr<geneva_type>& x
   , const boost::shared_ptr<geneva_type>& y
   , const std::string& x_name
   , const std::string& y_name
   , const Gem::Common::expectation& e
   , const double& limit = Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
   , typename boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, geneva_type> >::type* dummy = 0
) {
   bool expectationMet = false;
   std::string expectation_str;
   std::ostringstream error;

   switch(e) {
   case Gem::Common::CE_FP_SIMILARITY:
   case Gem::Common::CE_EQUALITY:
   {
      expectation_str = "CE_FP_SIMILARITY / CE_EQUALITY";

      // Check whether the pointers hold content
      if(x && !y) {
         error
         << "Smart pointer " << x_name << " holds content while " << y_name << " does not." << std::endl
         << "Thus the expectation of " << expectation_str << " was violated" << std::endl;
         break; //
      } else if(!x && y) {
         error
         << "Smart pointer " << x_name << " doesn't hold content while " << y_name << " does." << std::endl
         << "Thus the expectation of " << expectation_str << " was violated" << std::endl;
         break;  // The expectation was clearly not met
      } else if(!x && !y) { // No content to check. Both smart pointers can be considered equal
         expectationMet = true;
         break;
      }

      // If we reach this line, then both pointers have content

      { // Check whether the content differs
         try {
            x->compare(*y,e,limit);
         } catch(g_expectation_violation& g) {
            error
            << "Content of " << x_name << " and " << y_name << " differ." << std::endl
            << "Thus the expectation of " << expectation_str << " was violated:" << std::endl
            << g.what() << std::endl;
            break; // Terminate the switch statement
         }

         // If we reach this line, the expectation was met
         expectationMet = true;
      }
   }
   break;

   case Gem::Common::CE_INEQUALITY:
   {
      expectation_str = "CE_INEQUALITY";

      // Check whether the pointers hold content
      if((x && !y) || (!x && y)) {
         expectationMet = true;
         break;
      } else if(!x && !y) { // No content to check. Both smart pointers can be considered equal
         error
         << "Both smart pointers are empty and are thus considered equal." << std::endl
         << "Thus the expectation of " << expectation_str << " was violated:" << std::endl;
         break; // The expectation was not met
      }

      // Check whether the content differs
      try {
         x->compare(*y,e,limit);
      } catch(g_expectation_violation& g) {
         // If we catch an expectation violation for expectation "inequality",
         // we simply break the switch statement so that expectationMet remains to be false
         error
         << "Content of " << x_name << " and " << y_name << " are equal/similar." << std::endl
         << "Thus the expectation of " << expectation_str << " was violated:" << std::endl
         << g.what() << std::endl;
         break;
      }
      expectationMet = true;
   }
   break;

   default:
   {
      glogger
      << "In compare(/* 7 */): Got invalid expectation " << e << std::endl
      << GEXCEPTION;
   }
   break;
   };

   if(!expectationMet) {
      throw g_expectation_violation(error.str());
   }
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This function checks whether two vectors of smart pointers to complex types meet a given expectation.
 * It is assumed that these types have the standard Geneva interface with corresponding "compare"
 * functions.
 *
 * @param x The first vector to be compared
 * @param y The second vector to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @param e The expectation both parameters need to fulfill
 * @param limit The maximum allowed deviation of two floating point values
 * @param dummy Boost::enable_if magic to steer overloaded resolution by the compiler
 */
template <typename geneva_type>
void compare (
   const std::vector<boost::shared_ptr<geneva_type> >& x
   , const std::vector<boost::shared_ptr<geneva_type> >& y
   , const std::string& x_name
   , const std::string& y_name
   , const Gem::Common::expectation& e
   , const double& limit = Gem::Common::CE_DEF_SIMILARITY_DIFFERENCE
   , typename boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, geneva_type> >::type* dummy = 0
) {
   bool expectationMet = false;
   std::string expectation_str;
   std::ostringstream error;

   switch(e) {
   case Gem::Common::CE_FP_SIMILARITY:
   case Gem::Common::CE_EQUALITY:
   {
      expectation_str = "CE_FP_SIMILARITY / CE_EQUALITY";

      // First check sizes
      if(x.size() != y.size()) {
         error
         << "Vectors " << x_name << " and " << y_name << " have different sizes " << x.size() << " / " << y.size() << std::endl
         << "Thus the expectation of " << expectation_str << " was violated:" << std::endl;
         // Terminate the switch statement. expectationMet will be false then
         break;
      }

      // Now loop over all members of the vectors
      bool foundDeviation = false;
      typename std::vector<boost::shared_ptr<geneva_type> >::const_iterator x_it, y_it;
      std::size_t index = 0;
      for(x_it=x.begin(), y_it=y.begin(); x_it!=x.end(); ++x_it, ++y_it, ++index) {
         // First check that both pointers have content
         // Check whether the pointers hold content
         if(*x_it && !*y_it) {
            error
            << "Smart pointer " << x_name << "[" << index << "] holds content while " << y_name << "[" << index << "]  does not." << std::endl
            << "Thus the expectation of " << expectation_str << " was violated" << std::endl;
            foundDeviation = true;
            break; // terminate the loop
         } else if(!*x_it && *y_it) {
            error
            << "Smart pointer " << x_name << "[" << index << "] doesn't hold content while " << y_name << "[" << index << "]  does." << std::endl
            << "Thus the expectation of " << expectation_str << " was violated" << std::endl;
            foundDeviation = true;
            break;  // terminate the loop
         } else if(!*x_it && !*y_it) { // No content to check. Both smart pointers can be considered equal
            continue; // Go on with next iteration in the loop
         }

         // At this point we know that both pointers have content. We can now check the content
         // which is assumed to have the compare() function
         try {
          (*x_it)->compare(**y_it,e,limit);
         } catch(g_expectation_violation& g) {
            error
            << "Content of " << x_name << "[" << index << "] and " << y_name << "[" << index << "] differs." << std::endl
            << "Thus the expectation of " << expectation_str << " was violated:" << std::endl
            << g.what() << std::endl;
            foundDeviation = true;
            break; // Terminate the loop
         }
      }

      if(!foundDeviation) {
         expectationMet = true;
      }
   }
   break;

   case Gem::Common::CE_INEQUALITY:
   {
      expectation_str = "CE_INEQUALITY";

      // First check sizes. The expectation of inequality will be met if they differ
      if(x.size() != y.size()) {
         expectationMet = true;
         break; // Terminate the switch statement
      }

      // Now loop over all members of the vectors
      bool foundInequality = false;
      typename std::vector<boost::shared_ptr<geneva_type> >::const_iterator x_it, y_it;
      for(x_it=x.begin(), y_it=y.begin(); x_it!=x.end(); ++x_it, ++y_it) {
         // First check that both pointers have content
         // Check whether the pointers hold content
         if((*x_it && !*y_it) || (!*x_it && *y_it)) {
            foundInequality = true;
            break; // terminate the loop
         } else if(!*x_it && !*y_it) { // No content to check. Both smart pointers can be considered equal
            continue; // Go on with next iteration in the loop - there is nothing to check here
         }

         // At this point we know that both pointers have content. We can now check this content
         // which is assumed to have the compare() function
         try {
          (*x_it)->compare(**y_it,e,limit);
          foundInequality = true;
          break; // terminate the loop
         } catch(g_expectation_violation&) {
            // Go on with the next item in the vector -- the content is equal or similar
            continue;
         }
      }

      if(foundInequality) {
         expectationMet = true;
      } else {
         error
         << "The two vectors " << x_name << " and " << y_name << " are equal." << std::endl
         << "Thus the expectation of " << expectation_str << " was violated:" << std::endl;
      }
   }
   break;

   default:
   {
      glogger
      << "In compare(/* 8 */): Got invalid expectation " << e << std::endl
      << GEXCEPTION;
   }
   break;
   };

   if(!expectationMet) {
      throw g_expectation_violation(error.str());
   }
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GOBJECT_HPP_ */
