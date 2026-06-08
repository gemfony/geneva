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
#include <random>

// Boost headers go here

// Geneva headers go here
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "geneva/par/GBooleanAdaptor.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/par/GParameterCollectionT.hpp"

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * This class represents collections of bits. They are usually adapted by
 * the GBooleanAdaptor, which has a mutable flip probability. One adaptor
 * is applied to all bits. If you want individual flip probabilities for
 * all bits, use GBool objects instead.
 */
class GBooleanCollection // NOLINT(cppcoreguidelines-special-member-functions)
  : public GParameterCollectionT<bool> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GParameterCollectionT_bool",
            boost::serialization::base_object<GParameterCollectionT<bool>>(*this)
        );
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GBooleanCollection() = default;
    /** @brief Random initialization with a given number of values */
    explicit GBooleanCollection(const std::size_t &);
    /** @brief Initialization with a given number of items of defined value */
    GBooleanCollection(const std::size_t &, const bool &);
    /** @brief Random initialization with a given number of values of
	  * a certain probability structure */
    GBooleanCollection(const std::size_t &, const double &);
    /** @brief A standard copy constructor */
    GBooleanCollection(const GBooleanCollection &) = default;
    /** @brief The standard destructor */
    ~GBooleanCollection() override = default;

    /** @brief FLips the value at a given position */
    void flip(const std::size_t &);

    /** @brief Random initialization */
    bool randomInit(const activityMode &, Gem::Hap::GRandomBase &) override;
    /** @brief Random initialization with a given probability structure */
    bool randomInit(const double &, const activityMode &, Gem::Hap::GRandomBase &);

protected:
    /** @brief Loads the data of another GBooleanCollection class */
    void load_(const GParameterBase *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GBooleanCollection>(
        GBooleanCollection const &,
        GBooleanCollection const &,
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

    /** @brief Triggers random initialization of the parameter collection */
    bool randomInit_(const activityMode &, Gem::Hap::GRandomBase &) override;
    /** @brief Triggers random initialization of the parameter collection, with a given likelihood structure */
    bool randomInit_(const double &, const activityMode &, Gem::Hap::GRandomBase &);

    /** @brief Returns a "comparative range" for this type */
    bool range() const override;

    /** @brief Tell the audience that we own a number of boolean values */
    std::size_t countBoolParameters(const activityMode &am) const override;
    /** @brief Attach boundaries of type bool to the vectors */
    void booleanBoundaries(
        std::vector<bool> &,
        std::vector<bool> &,
        const activityMode &am
    ) const override;
    /** @brief Attach our local values to the vector. */
    void booleanStreamline(std::vector<bool> &, const activityMode &am) const override;
    /** @brief Assigns part of a value vector to the parameter */
    void assignBooleanValueVector(
        const std::vector<bool> &,
        std::size_t &,
        const activityMode &am
    ) override;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /** @brief Emits a name for this class / object */
    std::string name_() const override;
    /** @brief Creates a deep copy of this object */
    GParameterBase *clone_() const override;
};

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Parameters::GBooleanCollection) // NOLINT
