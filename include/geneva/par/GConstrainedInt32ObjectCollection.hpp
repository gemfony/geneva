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

// Standard header files go here

// Boost header files go here

// Geneva header files go here
#include "geneva/par/GConstrainedInt32Object.hpp"
#include "geneva/par/GInt32GaussAdaptor.hpp"
#include "geneva/par/GParameterTCollectionT.hpp"

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * A collection of GConstrainedInt32Object objects, ready for use in a
 * GParameterSet derivative.
 */
class GConstrainedInt32ObjectCollection // NOLINT(cppcoreguidelines-special-member-functions)
  : public GParameterTCollectionT<GConstrainedInt32Object> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GParameterTCollectionT_gbi",
            boost::serialization::base_object<GParameterTCollectionT<GConstrainedInt32Object>>(
                *this
            )
        );
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GConstrainedInt32ObjectCollection() = default;
    /** @brief Initialization with a number of GConstrainedInt32Object objects */
    GConstrainedInt32ObjectCollection(
        const std::size_t &,
        std::shared_ptr<GConstrainedInt32Object>
    );
    /** @brief The copy constructor */
    
    GConstrainedInt32ObjectCollection(const GConstrainedInt32ObjectCollection &) = default;

    /** @brief The destructor */
    ~GConstrainedInt32ObjectCollection() override = default;

protected:
    /** @brief Loads the data of another GParameterBase */
    void load_(const GParameterBase *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GConstrainedInt32ObjectCollection>(
        GConstrainedInt32ObjectCollection const &,
        GConstrainedInt32ObjectCollection const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GParameterBase & // the other object
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
    /** @brief Creates a deep clone of this object. */
    GParameterBase *clone_() const override;

    /** @brief Fills the collection with GConstrainedInt32Object objects */
    void fillWithObjects_(const std::size_t &);
};

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Parameters::GConstrainedInt32ObjectCollection) // NOLINT
