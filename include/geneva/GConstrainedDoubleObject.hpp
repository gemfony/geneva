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
#include "geneva/GConstrainedFPT.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"

namespace Gem::Geneva {

/******************************************************************************/
/**
 * The GConstrainedDoubleObject class allows to limit the value range of a double value,
 * while applying adaptions to a continuous range. This is done by means of a
 * mapping from an internal representation to an externally visible value.
 */
class GConstrainedDoubleObject // NOLINT(cppcoreguidelines-special-member-functions)
  : public GConstrainedFPT<double> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GConstrainedFPT_double",
            boost::serialization::base_object<GConstrainedFPT<double>>(*this)
        );
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GConstrainedDoubleObject() = default;
    /** @brief Initialization with boundaries only */
    GConstrainedDoubleObject(const double &, const double &);
    /** @brief Initialization with value and boundaries */
    GConstrainedDoubleObject(const double &, const double &, const double &);
    /** @brief The copy constructor */
    GConstrainedDoubleObject(const GConstrainedDoubleObject &) = default;

    /** @brief Initialization by contained value */
    explicit GConstrainedDoubleObject(const double &);

    /** @brief The destructor */
    ~GConstrainedDoubleObject() override = default;

    /** @brief An assignment operator for the contained value type */
    GConstrainedDoubleObject &operator=(const double &) override;

protected:
    /** @brief Loads the data of another GObject */
    void load_(const GObject *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GConstrainedDoubleObject>(
        GConstrainedDoubleObject const &,
        GConstrainedDoubleObject const &,
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

    /** @brief Attach our local value to the vector. */
    void
    doubleStreamline(std::vector<double> &, const activityMode &am) const override;
    /** @brief Attach boundaries of type double to the vectors */
    void doubleBoundaries(
        std::vector<double> &,
        std::vector<double> &,
        const activityMode &am
    ) const override;
    /** @brief Tell the audience that we own a double value */
    std::size_t countDoubleParameters(const activityMode &am) const override;
    /** @brief Assigns part of a value vector to the parameter */
    void assignDoubleValueVector(
        const std::vector<double> &,
        std::size_t &,
        const activityMode &am
    ) override;
    /** @brief Attach our local value to the vector. */
    void doubleStreamline(
        std::map<std::string, std::vector<double>> &,
        const activityMode &am
    ) const override;
    /** @brief Assigns part of a value map to the parameter */
    void assignDoubleValueVectors(
        const std::map<std::string, std::vector<double>> &,
        const activityMode &am
    ) override;

    /** @brief Multiplication with a random value in a given range */
    void doubleMultiplyByRandom(
        const double &min,
        const double &max,
        const activityMode &am,
        Gem::Hap::GRandomBase &
    ) override;
    /** @brief Multiplication with a random value in the range [0,1[ */
    void
    doubleMultiplyByRandom(const activityMode &am, Gem::Hap::GRandomBase &) override;
    /** @brief Multiplication with a constant value */
    void doubleMultiplyBy(const double &value, const activityMode &am) override;
    /** @brief Initialization with a constant value */
    void doubleFixedValueInit(const double &value, const activityMode &am) override;
    /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
    void doubleAdd(std::shared_ptr<GParameterBase>, const activityMode &am) override;
    /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
    void
    doubleSubtract(std::shared_ptr<GParameterBase>, const activityMode &am) override;

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

} /* namespace Gem::Geneva */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GConstrainedDoubleObject) // NOLINT
