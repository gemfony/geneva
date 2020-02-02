/**
 * @file
 */

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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here

// Boost headers go here
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>

// Geneva headers go here
#include "common/GCommonInterfaceT.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class forms the basis of a hierarchy of function objects that are
 * required to be serializable, so they may be registered with serializable
 * objects and thus modify their behaviour.
 */
template <typename processable_type>
class GSerializableFunctionObjectT
	: public GCommonInterfaceT<GSerializableFunctionObjectT<processable_type>>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 /* nothing */
	 }

	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
	 // Defaulted functions

	 GSerializableFunctionObjectT() = default;
	 GSerializableFunctionObjectT(GSerializableFunctionObjectT<processable_type> const& cp) = default;
	 GSerializableFunctionObjectT(GSerializableFunctionObjectT<processable_type> && cp) = default;
	 virtual ~GSerializableFunctionObjectT() BASE = default;

	 GSerializableFunctionObjectT<processable_type>& operator=(GSerializableFunctionObjectT<processable_type> const&) = default;
	 GSerializableFunctionObjectT<processable_type>& operator=(GSerializableFunctionObjectT<processable_type> &&) = default;

	 /***************************************************************************/
	 /**
	  * Function call operator
  	  */
	 bool operator()(processable_type& p) {
		 return this->process_(p);
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads the data of another GSerializableFunctionObjectT<processable_type> object
	  */
	 void load_(const GSerializableFunctionObjectT<processable_type> *cp) override {
		 // Check that we are dealing with a GSerializableFunctionObjectT<processable_type> reference independent of this object and convert the pointer
		 const GSerializableFunctionObjectT<processable_type> *p_load = Gem::Common::g_convert_and_compare<GSerializableFunctionObjectT<processable_type>, GSerializableFunctionObjectT<processable_type>>(cp, this);

		 // ... no local data
	 }

	/***************************************************************************/
	/** @brief Allow access to this classes compare_ function */
	friend void Gem::Common::compare_base_t<GSerializableFunctionObjectT<processable_type>>(
		GSerializableFunctionObjectT<processable_type> const &
		, GSerializableFunctionObjectT<processable_type> const &
		, Gem::Common::GToken &
	);

	/***************************************************************************/
	/**
     * Checks for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GSerializableFunctionObjectT<processable_type> object
     * @param e The expected outcome of the comparison
     * @param limit The maximum deviation for floating point values (important for similarity checks)
     */
	void compare_(
		const GSerializableFunctionObjectT<processable_type> &cp
		, const Gem::Common::expectation &e
		, const double &limit
	) const override {
		using namespace Gem::Common;

		// Check that we are dealing with a GSerializableFunctionObjectT<processable_type> reference independent of this object and convert the pointer
		const GSerializableFunctionObjectT<processable_type> *p_load
			= Gem::Common::g_convert_and_compare<GSerializableFunctionObjectT<processable_type>, GSerializableFunctionObjectT<processable_type>>(cp, this);

		GToken token("GSerializableFunctionObjectT<processable_type>", e);

		// Compare our parent data ...
		Gem::Common::compare_base_t<GCommonInterfaceT<GSerializableFunctionObjectT<processable_type>>>(*this, *p_load, token);

		// ... no local data

		// React on deviations from the expectation
		token.evaluate();
	}

	 /***************************************************************************/

	 /** @brief overload this function to make this class operational */
	 virtual G_API_COMMON bool process_(processable_type& p) BASE = 0;

	 /** @brief Applies modifications to this object. This is needed for testing purposes */
	 bool modify_GUnitTests_() override { return false; };
	 /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	 void specificTestsNoFailureExpected_GUnitTests_() override { /* nothing */ };
	 /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	 void specificTestsFailuresExpected_GUnitTests_() override { /* nothing */ };

private:
	 /***************************************************************************/
	 /**
	  * Returns the name of this class
	  */
	 std::string name_() const override {
		 return std::string("GSerializableFunctionObjectT<processable_type>");
	 }

	 /***************************************************************************/
	 /** @brief Creates a deep clone of this object */
	 G_API_COMMON GSerializableFunctionObjectT<processable_type> * clone_() const override = 0;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) macro. Needed for Boost.Serialization
 */
namespace boost {
namespace serialization {
template<typename processable_type>
struct is_abstract<Gem::Common::GSerializableFunctionObjectT<processable_type>> : public boost::true_type {};
template<typename processable_type>
struct is_abstract< const Gem::Common::GSerializableFunctionObjectT<processable_type>> : public boost::true_type {};
} /* namespace serialization */
} /* namespace boost */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
