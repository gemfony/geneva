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

// Geneva headers go here
#include "common/GCommonInterfaceT.hpp"
#include "common/GExceptions.hpp"

namespace Gem::Geneva {

/******************************************************************************/
/**
 * This is the base class -- and the CRTP category root -- for a small hierarchy
 * that encapsulates information relevant to particular optimization algorithms.
 * The information is stored in individuals (i.e. the parameter sets which are
 * subject to a given optimization problem). In this sense, individuals can take
 * on more than one role or personality. Note that this class is purely virtual.
 * It can only be used in conjunction with a derived personality.
 *
 * As part of the GObject-decomposition effort, the personality-traits hierarchy
 * is its own category root: it derives directly from
 * Gem::Common::GCommonInterfaceT<GPersonalityTraits> instead of from GObject, so
 * a GPersonalityTraits pointer is an unrelated type to a GObject pointer. The
 * common infrastructure (clone/load/compare/name/IO/serialize) is supplied by
 * the CRTP base, instantiated for this root.
 */
class GPersonalityTraits // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Common::GCommonInterfaceT<GPersonalityTraits> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize([[maybe_unused]] Archive & ar, const unsigned int) {
        using boost::serialization::make_nvp;

        // This is the CRTP category root. Its CRTP base
        // (Gem::Common::GCommonInterfaceT<GPersonalityTraits>) carries no state
        // and is therefore not serialized as a base_object -- mirroring GObject,
        // whose serialize() is likewise empty. The polymorphic base_object chain
        // simply bottoms out here.
        /* nothing */
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GPersonalityTraits() = default;
    /** @brief The copy constructor */
    GPersonalityTraits(const GPersonalityTraits &) = default;
    /** @brief The standard destructor */
    ~GPersonalityTraits() override = default;

    /** @brief Retrieves the mnemonic of the optimization algorithm */
    virtual std::string getMnemonic() const = 0;

protected:
    /** @brief Loads the data of another GPersonalityTraits object */
    void load_(const GPersonalityTraits *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GPersonalityTraits>(
        GPersonalityTraits const &,
        GPersonalityTraits const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GPersonalityTraits & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /** @brief Emits a name for this class / object */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    GPersonalityTraits *clone_() const override = 0;
};

/******************************************************************************/

} /* namespace Gem::Geneva */

/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(
    Gem::Geneva::GPersonalityTraits
) // NOLINT/******************************************************************************/
