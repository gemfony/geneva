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
#include <tuple>

// Boost header files go here

// Geneva header files go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "geneva/GIndividualMultiConstraint.hpp"
#include "geneva/GMultiConstraintT.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"

namespace Gem::Geneva::Genome {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class implements constraint definitions based on GOptimizableEntity-derivatives.
 * It is meant to be added to a constraint collection. The main purpose of this
 * class is to "translate" GOptimizableEntity-based constraints into constraints
 * based on GOptimizableEntity
 */
class GOptimizableEntityConstraint // NOLINT(cppcoreguidelines-special-member-functions)
  : public GPreEvaluationValidityCheckT<GOptimizableEntity> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;
        ar &boost::serialization::make_nvp(
            "GPreEvaluationValidityCheckT_GOptimizableEntity",
            boost::serialization::base_object<GPreEvaluationValidityCheckT<GOptimizableEntity>>(*this)
        );
    }
    ///////////////////////////////////////////////////////////////////////
public:
    /** @brief The default constructor */
    GOptimizableEntityConstraint() = default;
    /**
     * @brief The copy constructor
     *
     * @param cp A constant reference to another GOptimizableEntityConstraint object to be copied
     */
    GOptimizableEntityConstraint(const GOptimizableEntityConstraint & cp) = default;
    /** @brief The destructor */
    ~GOptimizableEntityConstraint() override = default;

protected:
    /**
     * @brief Checks whether a given individual is valid
     *
     * @param individual A pointer to the GOptimizableEntity to be checked for constraint compliance
     * @return A measure of the constraint violation (pure virtual; defined by derived classes)
     */
    double check_(const GOptimizableEntity * individual) const override = 0;

    /**
     * @brief Adds local configuration options to a GParserBuilder object
     *
     * @param gpb A reference to the GParserBuilder to which this object's configuration options are added
     */
    void addConfigurationOptions_(Gem::Common::GParserBuilder & gpb) override;
    /**
     * @brief Loads the data of another GOptimizableEntityConstraint
     *
     * @param cp A pointer to another GPreEvaluationValidityCheckT (expected to be a GOptimizableEntityConstraint) whose data is to be loaded
     */
    void load_(const GPreEvaluationValidityCheckT<GOptimizableEntity> * cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GOptimizableEntityConstraint>(
        GOptimizableEntityConstraint const &,
        GOptimizableEntityConstraint const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type
     *
     * @param cp A constant reference to another GPreEvaluationValidityCheckT object (the object to be compared against)
     * @param e The expectation for this comparison, e.g. equality or inequality
     * @param limit The maximum allowed deviation of floating point types still considered equal
     */
    void compare_(
        const GPreEvaluationValidityCheckT<GOptimizableEntity> & cp // the other object
        ,
        const Gem::Common::expectation & e // the expectation for this object, e.g. equality
        ,
        const double & limit // the limit for allowed deviations of floating point types
    ) const override;

private:
    /**
     * @brief Creates a deep clone of this object
     *
     * @return A deep clone of this object (pure virtual; defined by derived classes)
     */
    GPreEvaluationValidityCheckT<GOptimizableEntity> *clone_() const override = 0;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Genome */

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::Genome::GOptimizableEntityConstraint) // NOLINT
