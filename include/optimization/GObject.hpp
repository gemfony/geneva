/**
 * @file GObject.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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

// Standard header files go here

#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <typeinfo>
#include <map>
#include <vector>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost header files go here

#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/optional.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>

#ifdef GENEVATESTING
#include <boost/test/unit_test.hpp>
#endif /* GENEVATESTING */

#ifndef GOBJECT_HPP_
#define GOBJECT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva header files go here
#include "GSerializableI.hpp"
#include "GExceptions.hpp"
#include "GOptimizationEnums.hpp"
#include "GHelperFunctionsT.hpp"

#ifdef GENEVATESTING
#include "GUnitTestFrameworkT.hpp"
#endif /* GENEVATESTING */

namespace Gem {
namespace GenEvA {

/**************************************************************************************************/
/**
 * GObject is the parent class for the majority of GenEvA classes. Essentially, GObject gives a
 * GenEvA class the ability to carry a name and defines a number of interface functions.
 * The GObject::load_(const GObject *) and  GObject::clone_() member functions should be
 * re-implemented for each derived class. Unfortunately, there is no way to enforce this in C++.
 * Further common functionality of many GenEvA classes will be implemented here over time.
 */
class GObject
	:public GSerializableI
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int)  {
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
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Util::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief Convert class to a serial representation that is then written to a stream */
	void toStream(std::ostream&, const serializationMode&);
	/** @brief Load class from a stream */
	void fromStream(std::istream&, const serializationMode&);

	/** @brief Convert class to a serial representation, using a user-specified serialization mode */
	std::string toString(const serializationMode&);
	/** @brief Convert class to a serial representation, using a specific serialization mode */
	void fromString(const std::string&, const serializationMode&);

	/** @brief Writes a serial representation of this object to a file */
	void toFile(const std::string&, const serializationMode&);
	/** @brief Loads a serial representation of this object from file */
	void fromFile(const std::string&, const serializationMode&);

	/** @brief Returns an XML description of the derivative it is called for */
	std::string report();

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
			, typename boost::enable_if<boost::is_base_of<Gem::GenEvA::GObject, load_type> >::type* dummy = 0
	) {
		load_(cp.get());
	}

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
			, typename boost::enable_if<boost::is_base_of<Gem::GenEvA::GObject, load_type> >::type* dummy = 0
	) {
		load_(&cp);
	}

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
			typename boost::enable_if<boost::is_base_of<Gem::GenEvA::GObject, clone_type> >::type* dummy = 0
	) const {
#ifdef DEBUG
		boost::shared_ptr<clone_type> p = boost::dynamic_pointer_cast<clone_type>(boost::shared_ptr<GObject>(this->clone_()));
		if(p) return p;
		else {
			std::ostringstream error;
			error << "In boost::shared_ptr<clone_type> GObject::clone<load_type>() : Error!" << std::endl
				  << "Invalid conversion" << std::endl;
			throw geneva_error_condition(error.str());
		}
#else
		return boost::static_pointer_cast<clone_type>(boost::shared_ptr<GObject>(this->clone_()));
#endif /* DEBUG */
	}

#ifdef GENEVATESTING
	/**************************************************************************************************/
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests() { return false; }
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests() { /* nothing - no local data */ }
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests() { /* nothing - no local data */ }
#endif /* GENEVATESTING */

protected:
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const = 0;

	/**************************************************************************************************/
	/**
	 * This function checks in DEBUG mode whether a load pointer points to the current object. Note
	 * that this template will only be accessible to the compiler if GObject is a base type of load_type.
	 */
	template <typename load_type>
	inline void selfAssignmentCheck (
			const GObject *load_ptr
		  , typename boost::enable_if<boost::is_base_of<Gem::GenEvA::GObject, load_type> >::type* dummy = 0
	) const {
#ifdef DEBUG
		// Check that this object is not accidentally assigned to itself.
		if (load_ptr == this) {
			std::ostringstream error;
			error << "In GObject::selfAssignmentCheck<load_type>() : Error!" << std::endl
				  << "Tried to assign an object to or compare with itself." << std::endl;
			throw geneva_error_condition(error.str());
		}
#endif
	}

	/**************************************************************************************************/
	/**
	 * This function converts the GObject pointer to the target, optionally checking for self-assignment
	 * along the ways in DEBUG mode (through selfAssignmentCheck() ).  Note that this template will
	 * only be accessible to the compiler if GObject is a base type of load_type.
	 */
	template <typename load_type>
	inline const load_type* conversion_cast(
			const GObject *load_ptr
		  , typename boost::enable_if<boost::is_base_of<Gem::GenEvA::GObject, load_type> >::type* dummy = 0
	) const {
		selfAssignmentCheck<load_type>(load_ptr);

#ifdef DEBUG
		const load_type *p = dynamic_cast<const load_type *>(load_ptr);
		if(p) return p;
		else {
			std::ostringstream error;
			error << "In const GObject* GObject::conversion_cast<load_type>() : Error!" << std::endl
				  << "Invalid conversion" << std::endl;
			throw geneva_error_condition(error.str());
		}
#else
		return static_cast<const load_type *>(load_ptr);
#endif
	}

	/**************************************************************************************************/

private:
	/** @brief Checks for equality with another GObject object. Intentionally left undefined, as this class is abstract */
	bool operator==(const GObject&) const;
	/** @brief Checks inequality with another GObject object. Intentionally left undefined, as this class is abstract */
	bool operator!=(const GObject&) const;
};

/** @brief A specialization for cases for no conversion is supposed to take place */
template <> boost::shared_ptr<GObject> GObject::clone<GObject>(
		boost::enable_if<boost::is_base_of<Gem::GenEvA::GObject, GObject> >::type*
) const;

} /* namespace GenEvA */
} /* namespace Gem */

/**********************************************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::GenEvA::GObject)

/**********************************************************************************************************/
////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**********************************************************************************************************/
// Tests for the GObject class




#endif /* GOBJECT_HPP_ */
