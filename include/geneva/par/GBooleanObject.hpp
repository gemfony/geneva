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
#include "geneva/par/GBooleanAdaptor.hpp"
#include "geneva/par/GParameterT.hpp"

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * This class encapsulates a single bit, represented as a bool. This might appear heavy weight,
 * and indeed for most applications this is not the recommended solution -
 * use the GBooleanCollection instead.
 */
class GBooleanObject // NOLINT(cppcoreguidelines-special-member-functions)
  : public GParameterT<bool> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GParameterT_bool",
            boost::serialization::base_object<GParameterT<bool>>(*this)
        );
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GBooleanObject() = default;
    /** @brief The copy constructor */
    GBooleanObject(const GBooleanObject &) = default;
    /** @brief Initialization by contained value */
    explicit GBooleanObject(const bool &);
    /** @brief Initialization with a given probability for "true" */
    explicit GBooleanObject(const double &);
    /** @brief The destructor */
    ~GBooleanObject() override = default;

    /** @brief An assignment operator */
    GBooleanObject &operator=(const bool &) override;

    /** @brief Triggers random initialization of the parameter object */
    bool randomInit(const activityMode &, Gem::Hap::GRandomBase &) override;
    /** @brief Triggers random initialization of the parameter object, with a given likelihood structure */
    bool randomInit(const double &, const activityMode &, Gem::Hap::GRandomBase &);

    /** @brief Flips the value of this object */
    void flip();

protected:
    /** @brief Loads the data of another GObject */
    void load_(const GObject *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GBooleanObject>(
        GBooleanObject const &,
        GBooleanObject const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GObject & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Triggers random initialization of the parameter object */
    bool randomInit_(const activityMode &, Gem::Hap::GRandomBase &) override;
    /** @brief Triggers random initialization of the parameter object, with a given likelihood structure */
    bool randomInit_(const double &, const activityMode &, Gem::Hap::GRandomBase &);

    /** @brief Returns a "comparative range" for this type */
    bool range() const override;

    /** @brief Attach our local value to the vector. */
    void booleanStreamline(std::vector<bool> &, const activityMode &am) const override;
    /** @brief Attach boundaries of type bool to the vectors */
    void booleanBoundaries(
        std::vector<bool> &,
        std::vector<bool> &,
        const activityMode &am
    ) const override;

    /** @brief Tell the audience that we own a std::int32_t value */
    std::size_t countBoolParameters(const activityMode &am) const override;
    /** @brief Assigns part of a value vector to the parameter */
    void assignBooleanValueVector(
        const std::vector<bool> &,
        std::size_t &,
        const activityMode &am
    ) override;
    /** @brief Attach our local value to the map. */
    void booleanStreamline(
        std::map<std::string, std::vector<bool>> &,
        const activityMode &am
    ) const override;
    /** @brief Assigns part of a value map to the parameter */
    void assignBooleanValueVectors(
        const std::map<std::string, std::vector<bool>> &,
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
    /** @brief Creates a deep clone of this object. */
    GObject *clone_() const override;
};

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Parameters::GBooleanObject) // NOLINT
