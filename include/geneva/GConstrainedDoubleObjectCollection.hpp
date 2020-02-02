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

// Standard header files go here

// Boost header files go here

// Geneva header files go here
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GParameterTCollectionT.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A collection of GConstrainedDoubleObject objects, ready for use in a
 * GParameterSet derivative.
 */
class GConstrainedDoubleObjectCollection
    :
        public GParameterTCollectionT<GConstrainedDoubleObject>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & make_nvp(
            "GParameterTCollectionT_gbd"
            , boost::serialization::base_object<GParameterTCollectionT<GConstrainedDoubleObject>>(*this));
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    G_API_GENEVA GConstrainedDoubleObjectCollection() = default;
    /** @brief Initialization with a number of GConstrainedDoubleObject objects */
    G_API_GENEVA GConstrainedDoubleObjectCollection(const std::size_t &, std::shared_ptr<GConstrainedDoubleObject>);
    /** @brief The copy constructor */
    G_API_GENEVA GConstrainedDoubleObjectCollection(const GConstrainedDoubleObjectCollection &) = default;
    /** @brief The destructor */
    G_API_GENEVA ~GConstrainedDoubleObjectCollection() override = default;

protected:
    /** @brief Loads the data of another GObject */
    G_API_GENEVA void load_(const GObject *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GConstrainedDoubleObjectCollection>(
        GConstrainedDoubleObjectCollection const &
        , GConstrainedDoubleObjectCollection const &
        , Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    G_API_GENEVA void compare_(
        const GObject & // the other object
        , const Gem::Common::expectation & // the expectation for this object, e.g. equality
        , const double & // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    G_API_GENEVA bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    G_API_GENEVA void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /** @brief Emits a name for this class / object */
    G_API_GENEVA std::string name_() const override;
    /** @brief Creates a deep clone of this object. */
    G_API_GENEVA GObject *clone_() const override;

    /** @brief Fills the collection with GConstrainedDoubleObject objects */
    G_API_GENEVA void fillWithObjects_(const std::size_t &);
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GConstrainedDoubleObjectCollection)

