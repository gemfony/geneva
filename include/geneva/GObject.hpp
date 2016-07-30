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
#include <functional>
#include <memory>
#include <tuple>
#include <limits>


// Boost header files go here
#include <boost/any.hpp>
#include <boost/archive/basic_archive.hpp>
#include <boost/cast.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/logic/tribool_io.hpp>
#include <boost/numeric/conversion/bounds.hpp> // get rid of the numeric_limits<double>::min() vs. numeric_limits<int>::min() problem
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/utility.hpp>
#include <boost/variant.hpp>
#include <boost/property_tree/ptree_serialization.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#ifdef GEM_TESTING
#include <boost/test/unit_test.hpp>
#endif /* GEM_TESTING */

#ifndef GOBJECT_HPP_
#define GOBJECT_HPP_

// Geneva header files go here
#include "common/GDefaultValueT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "common/GLogger.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GParserBuilder.hpp"
#include "common/GSerializeTupleT.hpp"
#include "common/GTupleIO.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "hap/GRandomT.hpp"
#include "hap/GRandomDistributionsT.hpp"

#ifdef GEM_TESTING
#include "common/GUnitTestFrameworkT.hpp"
#endif /* GEM_TESTING */

// aliases for ease of use
namespace pt = boost::property_tree;

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * GObject is the parent class for the majority of Geneva optimization classes.
 * Handling of optimization-related classes sometimes happens through a
 * std::shared_ptr<GObject>, hence this class has a very central role.
 */
class GObject
	:public Gem::Common::GCommonInterfaceT<GObject>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive &ar, const unsigned int)  {
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GCommonInterfaceT_GObject", boost::serialization::base_object<GCommonInterfaceT<GObject>>(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	G_API_GENEVA GObject() ;
	/** @brief The copy constructor */
	G_API_GENEVA GObject(const GObject& cp) ;
	/** @brief The destructor */
	virtual G_API_GENEVA ~GObject();

	/** @brief Allows derived classes to assign other class'es values */
	G_API_GENEVA const GObject& operator=(const GObject&);

	/** @brief Checks for equality with another GObject object */
	virtual G_API_GENEVA bool operator==(const GObject&) const;
	/** @brief Checks for inequality with another GObject object */
	virtual G_API_GENEVA bool operator!=(const GObject&) const;

	/** @brief Writes a configuration file to disk */
	G_API_GENEVA void writeConfigFile(const std::string&, const std::string&);
	/** @brief Reads a configuration file from disk */
	G_API_GENEVA void readConfigFile(const std::string&);

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual G_API_GENEVA void addConfigurationOptions(Gem::Common::GParserBuilder&);

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

	/** @brief Emits a name for this class / object */
	virtual G_API_GENEVA std::string name() const override;

	/** @brief Searches for compliance with expectations with respect to another object of the same type */
	virtual G_API_GENEVA void compare(
		const GObject& // the other object
		, const Gem::Common::expectation& // the expectation for this object, e.g. equality
		, const double& // the limit for allowed deviations of floating point types
	) const override;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual G_API_GENEVA boost::optional<std::string> checkRelationshipWith(
		const GObject&
		, const Gem::Common::expectation&
		, const double&
		, const std::string&
		, const std::string&
		, const bool&
	) const;

protected:
	/***************************************************************************/
	/** @brief Loads the data of another GObject */
	virtual G_API_GENEVA void load_(const GObject*) override;
	/** @brief Creates a deep clone of this object */
	virtual G_API_GENEVA GObject* clone_() const override = 0;

	/***************************************************************************/
	/**
	 * This function checks in DEBUG mode whether a load pointer points to the current object. Note
	 * that this template will only be accessible to the compiler if GObject is a base type of load_type.
	 */
	template <typename load_type>
	G_DEPRECATED("Use Gem::Common::ptrDifferenceCheck instead")
	void selfAssignmentCheck (
		const GObject *load_ptr
		, typename std::enable_if<std::is_base_of<Gem::Geneva::GObject, load_type>::value>::type *dummy = nullptr
	) const {
		Gem::Common::ptrDifferenceCheck(load_ptr, this);
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
	template <typename target_type>
	G_DEPRECATED("Use Gem::Common::g_convert_and_compare instead")
	const target_type* gobject_conversion (
		const GObject *convert_ptr
		, typename std::enable_if<std::is_base_of<Gem::Geneva::GObject, target_type>::value>::type *dummy = nullptr
	) const {
		// Convert the base pointer -- this call will throw, if conversion cannot be done
		const target_type * p =  Gem::Common::g_ptr_conversion<GObject, target_type>(convert_ptr);

		// Then compare the two pointers (will throw in case of equality)
		Gem::Common::ptrDifferenceCheck(convert_ptr, (GObject *)this);

		// Return the converted pointer
		return p;
	}

	/* ----------------------------------------------------------------------------------
	 * gobject_conversions are regularly performed as part of the loading process and are thus
	 * considered to be well tested.
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * This function converts a GObject std::shared_ptr to the target type, optionally checking for
	 * self-assignment along the ways in DEBUG mode (through selfAssignmentCheck() ).  Note that this
	 * template will only be accessible to the compiler if GObject is a base type of load_type.
	 *
	 * @param load_ptr A std::shared_ptr<load_type> to the item to be converted
	 * @param dummy A dummy argument needed for std::enable_if and type_traits magic
	 * @return A std::shared_ptr holding the converted object
	 */
	template <typename target_type>
	G_DEPRECATED("Use Gem::Common::g_convert_and_compare instead")
	std::shared_ptr<target_type> gobject_conversion (
		std::shared_ptr<GObject> convert_ptr
		, typename std::enable_if<std::is_base_of<Gem::Geneva::GObject, target_type>::value>::type *dummy = nullptr
	) const {
		// Convert the base pointer -- this call will throw, if conversion cannot be done
		std::shared_ptr<target_type> p =  Gem::Common::g_ptr_conversion<GObject, target_type>(convert_ptr);

		// Then compare the two pointers (will throw in case of equality)
		Gem::Common::ptrDifferenceCheck(convert_ptr.get(), (GObject *)this);

		// Return the converted pointer
		return p;
	}

	/* ----------------------------------------------------------------------------------
	 * gobject_conversions are regularly performed as part of the loading process and are thus
	 * considered to be well tested.
	 * ----------------------------------------------------------------------------------
	 */

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

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GObject)

#endif /* GOBJECT_HPP_ */
