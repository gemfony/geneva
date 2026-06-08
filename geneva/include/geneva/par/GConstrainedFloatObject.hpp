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
#include "geneva/par/GConstrainedFPT.hpp"
#include "geneva/par/GFloatGaussAdaptor.hpp"

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * The GConstrainedFloatObject class allows to limit the value range of a float value,
 * while applying adaptions to a continuous range. This is done by means of a
 * mapping from an internal representation to an externally visible value. It is the
 * single-precision sibling of GConstrainedDoubleObject.
 */
class GConstrainedFloatObject // NOLINT(cppcoreguidelines-special-member-functions)
  : public GConstrainedFPT<float> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GConstrainedFPT_float",
            boost::serialization::base_object<GConstrainedFPT<float>>(*this)
        );
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GConstrainedFloatObject() = default;
    /** @brief Initialization with boundaries only */
    GConstrainedFloatObject(const float &, const float &);
    /** @brief Initialization with value and boundaries */
    GConstrainedFloatObject(const float &, const float &, const float &);
    /** @brief The copy constructor */
    GConstrainedFloatObject(const GConstrainedFloatObject &) = default;

    /** @brief Initialization by contained value */
    explicit GConstrainedFloatObject(const float &);

    /** @brief The destructor */
    ~GConstrainedFloatObject() override = default;

    /** @brief An assignment operator for the contained value type */
    GConstrainedFloatObject &operator=(const float &) override;

protected:
    /** @brief Loads the data of another GParameterBase */
    void load_(const GParameterBase *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GConstrainedFloatObject>(
        GConstrainedFloatObject const &,
        GConstrainedFloatObject const &,
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

    /** @brief Attach our local value to the vector. */
    void
    floatStreamline(std::vector<float> &, const activityMode &am) const override;
    /** @brief Attach boundaries of type float to the vectors */
    void floatBoundaries(
        std::vector<float> &,
        std::vector<float> &,
        const activityMode &am
    ) const override;
    /** @brief Tell the audience that we own a float value */
    std::size_t countFloatParameters(const activityMode &am) const override;
    /** @brief Assigns part of a value vector to the parameter */
    void assignFloatValueVector(
        const std::vector<float> &,
        std::size_t &,
        const activityMode &am
    ) override;

    /** @brief Multiplication with a random value in a given range */
    void floatMultiplyByRandom(
        const float &min,
        const float &max,
        const activityMode &am,
        Gem::Hap::GRandomBase &
    ) override;
    /** @brief Multiplication with a random value in the range [0,1[ */
    void
    floatMultiplyByRandom(const activityMode &am, Gem::Hap::GRandomBase &) override;
    /** @brief Multiplication with a constant value */
    void floatMultiplyBy(const float &value, const activityMode &am) override;
    /** @brief Initialization with a constant value */
    void floatFixedValueInit(const float &value, const activityMode &am) override;
    /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
    void floatAdd(std::shared_ptr<GParameterBase>, const activityMode &am) override;
    /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
    void
    floatSubtract(std::shared_ptr<GParameterBase>, const activityMode &am) override;

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
};

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Parameters::GConstrainedFloatObject) // NOLINT
