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
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here

// Boost headers go here
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

// Geneva headers go here
#include "common/GCommonInterfaceT.hpp"

namespace Gem::Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief Base of a hierarchy of serializable function objects.
 *
 * These function objects can be registered with serializable objects and thus
 * modify their behaviour. Being serializable themselves, they can travel over
 * the network alongside the objects they operate on. Derived classes implement
 * the actual behaviour by overriding process_().
 *
 * @tparam processable_type The type of object the function object operates on
 */
template <typename processable_type>
class GSerializableFunctionObjectT
  : public GCommonInterfaceT<GSerializableFunctionObjectT<processable_type>> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize([[maybe_unused]] Archive & ar, const unsigned int) {
        using boost::serialization::make_nvp;

        /* nothing */
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    // Defaulted functions

    GSerializableFunctionObjectT() = default;
    GSerializableFunctionObjectT(GSerializableFunctionObjectT<processable_type> const &cp) =
        default;
    GSerializableFunctionObjectT(GSerializableFunctionObjectT<processable_type> &&cp) = default;
    ~GSerializableFunctionObjectT() override = default;

    GSerializableFunctionObjectT<processable_type> &
    operator=(GSerializableFunctionObjectT<processable_type> const &) = default;
    GSerializableFunctionObjectT<processable_type> &
    operator=(GSerializableFunctionObjectT<processable_type> &&) = default;

    /***************************************************************************/
    /**
	  * @brief Function call operator; applies this function object to p.
	  *
	  * @param p The object to be processed (modified in place by process_())
	  * @return The boolean result returned by the concrete process_() override
	  */
    bool operator()(processable_type &p) {
        return this->process_(p);
    }

protected:
    /***************************************************************************/
    /**
	  * @brief Loads the data of another GSerializableFunctionObjectT<processable_type> object.
	  *
	  * This class holds no local data; the conversion is invoked purely for its
	  * type-mismatch / self-load checking side effect.
	  *
	  * @param cp A pointer to another GSerializableFunctionObjectT to load from
	  */
    void load_(const GSerializableFunctionObjectT<processable_type> *cp) override {
        // Invoke g_convert_and_compare purely for its side effect (throws on
        // type mismatch / self-load). The returned pointer would be unused
        // and a previous assignment to a named local `p_load` triggered
        // -Wunused-variable.
        (void)Gem::Common::g_convert_and_compare<
            GSerializableFunctionObjectT<processable_type>,
            GSerializableFunctionObjectT<processable_type>>(cp, this);
        // ... no local data
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GSerializableFunctionObjectT<processable_type>>(
        GSerializableFunctionObjectT<processable_type> const &,
        GSerializableFunctionObjectT<processable_type> const &,
        Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
     * @brief Checks for compliance with expectations with respect to another
     * object of the same type.
     *
     * @param cp A constant reference to another GSerializableFunctionObjectT<processable_type> object
     * @param e The expected outcome of the comparison
     * @param limit The maximum acceptable deviation for similarity checks (unused; no local data)
     */
    void compare_(
        const GSerializableFunctionObjectT<processable_type> &cp,
        const Gem::Common::expectation &e,
        [[maybe_unused]] const double & limit
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GSerializableFunctionObjectT<processable_type> reference independent of this object and convert the pointer
        const GSerializableFunctionObjectT<processable_type> *p_load =
            Gem::Common::g_convert_and_compare<
                GSerializableFunctionObjectT<processable_type>,
                GSerializableFunctionObjectT<processable_type>>(cp, this);

        GToken token("GSerializableFunctionObjectT<processable_type>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<
            GCommonInterfaceT<GSerializableFunctionObjectT<processable_type>>>(
            *this,
            *p_load,
            token
        );

        // ... no local data

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/

    /**
     * @brief Overload this function to make this class operational.
     * @param p The object to be processed (typically modified in place)
     * @return A boolean status defined by the concrete implementation
     */
    virtual bool process_(processable_type &p) = 0;

    /**
     * @brief Applies modifications to this object. This is needed for testing purposes.
     * @return true if a modification was made, false otherwise (here: always false)
     */
    bool modify_GUnitTests_() override {
        return false;
    };
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override { /* nothing */ };
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override { /* nothing */ };

private:
    /***************************************************************************/
    /**
	  * @brief Returns the name of this class.
	  * @return The class name "GSerializableFunctionObjectT<processable_type>"
	  */
    std::string name_() const override {
        return std::string("GSerializableFunctionObjectT<processable_type>");
    }

    /***************************************************************************/
    /** @brief Creates a deep clone of this object */
    GSerializableFunctionObjectT<processable_type> *clone_() const override = 0;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Common */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) macro. Needed for Boost.Serialization
 */
namespace boost::serialization {
template <typename processable_type>
struct is_abstract<Gem::Common::GSerializableFunctionObjectT<processable_type>>
  : public std::true_type {};
template <typename processable_type>
struct is_abstract<const Gem::Common::GSerializableFunctionObjectT<processable_type>>
  : public std::true_type {};
} /* namespace boost::serialization */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
