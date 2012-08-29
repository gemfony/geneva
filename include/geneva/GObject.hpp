/**
 * @file GObject.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */


// Standard header files go here
#include <algorithm>
#include <assert.h>
#include <cmath>
#include <cfloat>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost header files go here
#include <boost/any.hpp>
#include <boost/bind.hpp>
#include <boost/cast.hpp>
#include <boost/cstdint.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/filesystem.hpp>
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

#ifdef GEM_TESTING
#include <boost/test/unit_test.hpp>
#endif /* GEM_TESTING */

/** Check that we have support for threads */
#ifndef BOOST_HAS_THREADS
#error "Error: Support for multi-threading does not seem to be available."
#endif

#ifndef GOBJECT_HPP_
#define GOBJECT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva header files go here
#include "common/GDefaultValueT.hpp"
#include "common/GExceptions.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "common/GSerializableI.hpp"
#include "common/GParserBuilder.hpp"
#include "common/GSerializeTupleT.hpp"
#include "geneva/GOptimizationEnums.hpp"

#ifdef GEM_TESTING
#include "common/GUnitTestFrameworkT.hpp"
#endif /* GEM_TESTING */

namespace Gem {
namespace Geneva {

/**************************************************************************************************/
/**
 * GObject is the parent class for the majority of Geneva classes. Essentially, GObject gives a
 * Geneva class the ability to carry a name and defines a number of interface functions.
 * The GObject::load_(const GObject *) and  GObject::clone_() member functions should be
 * re-implemented for each derived class. Unfortunately, there is no way to enforce this in C++.
 * Further common functionality of many Geneva classes will be implemented here over time.
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
	GObject() ;
	/** @brief The copy constructor */
	GObject(const GObject& cp) ;
	/** @brief The destructor */
	virtual ~GObject();

	/** @brief A standard assignment operator */
	const GObject& operator=(const GObject&);

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&
			, const Gem::Common::expectation&
			, const double&
			, const std::string&
			, const std::string&
			, const bool&
	) const;

	/** @brief Convert class to a serial representation that is then written to a stream */
	void toStream(std::ostream&, const Gem::Common::serializationMode&) const;
	/** @brief Load class from a stream */
	void fromStream(std::istream&, const Gem::Common::serializationMode&);

	/** @brief Convert class to a serial representation, using a user-specified serialization mode */
	std::string toString(const Gem::Common::serializationMode&) const;
	/** @brief Convert class to a serial representation, using a specific serialization mode */
	void fromString(const std::string&, const Gem::Common::serializationMode&);

	/** @brief Writes a serial representation of this object to a file */
	void toFile(const std::string&, const Gem::Common::serializationMode&) const;
	/** @brief Loads a serial representation of this object from file */
	void fromFile(const std::string&, const Gem::Common::serializationMode&);

	/** @brief Returns an XML description of the derivative it is called for */
	std::string report() const;

	/** @brief Writes a configuration file to disk */
	void writeConfigFile(const std::string&, const std::string&);
	/** @brief Reads a configuration file from disk */
	void readConfigFile(const std::string&);

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual void addConfigurationOptions(Gem::Common::GParserBuilder&);
	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual void addConfigurationOptions(Gem::Common::GParserBuilder&, const bool&);

	/** @brief Creates a clone of this object, storing it in a boost::shared_ptr<GObject> */
	boost::shared_ptr<GObject> clone() const;

	/**************************************************************************************************/
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

	/**************************************************************************************************/
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

	/**************************************************************************************************/
	/**
	 * The function creates a clone of the GObject pointer, converts it to a pointer to a derived class
	 * and emits it as a boost::shared_ptr<> . Note that this template will only be accessible to the
	 * compiler if GObject is a base type of clone_type.
	 *
	 * @return A converted clone of this object, wrapped into a boost::shared_ptr
	 */
	template <typename clone_type>
	inline boost::shared_ptr<clone_type> clone(
			typename boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, clone_type> >::type* dummy = 0
	) const {
#ifdef DEBUG
		boost::shared_ptr<clone_type> p = boost::dynamic_pointer_cast<clone_type>(boost::shared_ptr<GObject>(this->clone_()));
		if(p) return p;
		else {
			raiseException(
					"In boost::shared_ptr<clone_type> GObject::clone<load_type>() :" << std::endl
					<< "Invalid conversion"
			);
		}
#else
		return boost::static_pointer_cast<clone_type>(boost::shared_ptr<GObject>(this->clone_()));
#endif /* DEBUG */
	}

	/* ----------------------------------------------------------------------------------
	 * cloning is tested for all objects taking part in the Geneva standard tests
	 * ----------------------------------------------------------------------------------
	 */

protected:
	/**************************************************************************************************/
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const = 0;

	/** @brief Sets a flag indicating whether this object may be serialized */
	void setMayBeSerialized(const bool&);
	/** @brief Checks whether this object may currently be serialized */
	bool mayBeSerialized() const;

	/**************************************************************************************************/
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
			raiseException(
					"In GObject::selfAssignmentCheck<load_type>() :" << std::endl
					<< "Tried to assign an object to or compare with itself."
			);
		}
#endif
	}

	/* ----------------------------------------------------------------------------------
	 * selfAssignment checks are performed for all objects taking part in the Geneva standard tests
	 * ----------------------------------------------------------------------------------
	 */

	/**************************************************************************************************/
	/**
	 * This function converts the GObject pointer to the target type, optionally checking for self-assignment
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
			raiseException(
					"In const GObject* GObject::gobject_conversion<load_type>() :" << std::endl
					<< "Invalid conversion"
			);
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

	/**************************************************************************************************/
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
			raiseException(
					"In boost::shared_ptr<load_type> GObject::gobject_conversion<load_type>() :" << std::endl
					<< "Invalid conversion"
			);
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

	/**************************************************************************************************/

private:
	bool mayBeSerialized_; ///< Indicates whether derivatives may be serialized.

	/** @brief Checks for equality with another GObject object. Intentionally left undefined, as this class is abstract */
	bool operator==(const GObject&) const;
	/** @brief Checks inequality with another GObject object. Intentionally left undefined, as this class is abstract */
	bool operator!=(const GObject&) const;

#ifdef GEM_TESTING
public:
	/**************************************************************************************************/
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();
#endif /* GEM_TESTING */
};

/**********************************************************************************************************/
/** @brief A specialization for cases for no conversion is supposed to take place */
template <> boost::shared_ptr<GObject> GObject::clone<GObject>(
		boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, GObject> >::type*
) const;

} /* namespace Geneva */
} /* namespace Gem */

/**********************************************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GObject)

/**********************************************************************************************************/
////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**********************************************************************************************************/
// Tests for the GObject class

#endif /* GOBJECT_HPP_ */
