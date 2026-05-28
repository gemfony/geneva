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
#include "common/GFormulaParserT.hpp"
#include "geneva/GIndividualMultiConstraint.hpp"
#include "geneva/GMultiConstraintT.hpp"
#include "geneva/par/GParameterSet.hpp"

namespace Gem::Geneva::Parameters {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class implements constraint definitions based on GParameterSet-derivatives.
 * It is meant to be added to a constraint collection. The main purpose of this
 * class is to "translate" GParameterSet-based constraints into constraints
 * based on GParameterSets
 */
class GParameterSetConstraint // NOLINT(cppcoreguidelines-special-member-functions)
  : public GPreEvaluationValidityCheckT<GParameterSet> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar &boost::serialization::make_nvp(
            "GPreEvaluationValidityCheckT_GParameterSet",
            boost::serialization::base_object<GPreEvaluationValidityCheckT<GParameterSet>>(*this)
        );
    }
    ///////////////////////////////////////////////////////////////////////
public:
    /** @brief The default constructor */
    GParameterSetConstraint() = default;
    /** @brief The copy constructor */
    GParameterSetConstraint(const GParameterSetConstraint &) = default;
    /** @brief The destructor */
    ~GParameterSetConstraint() override = default;

protected:
    /** @brief Checks whether a given individual is valid */
    double check_(const GParameterSet *) const override = 0;

    /** @brief Adds local configuration options to a GParserBuilder object */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &) override;
    /** @brief Loads the data of another GParameterSetConstraint */
    void load_(const GPreEvaluationValidityCheckT<GParameterSet> *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GParameterSetConstraint>(
        GParameterSetConstraint const &,
        GParameterSetConstraint const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GPreEvaluationValidityCheckT<GParameterSet> & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const override;

private:
    /** @brief Creates a deep clone of this object */
    GPreEvaluationValidityCheckT<GParameterSet> *clone_() const override = 0;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class accepts a string as input, which describes a formula. It then
 * inserts parameter values into the string, parses the formula and returns the
 * value represented by the formula as the "check"-value. Note that this class
 * currently only deals with double values.
 */
class GParameterSetFormulaConstraint // NOLINT(cppcoreguidelines-special-member-functions)
  : public GParameterSetConstraint {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /** @brief Single declaration of this class'es local data members */
    auto localMembers() {
        return std::make_tuple(Gem::Common::make_member("raw_formula_", raw_formula_));
    }
    auto localMembers() const {
        return std::make_tuple(Gem::Common::make_member("raw_formula_", raw_formula_));
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSetConstraint);
        Gem::Common::serialize_members(ar, localMembers());
    }
    ///////////////////////////////////////////////////////////////////////
public:
    /** @brief The default constructor */
    explicit GParameterSetFormulaConstraint(std::string);
    /** @brief The copy constructor */
    GParameterSetFormulaConstraint(const GParameterSetFormulaConstraint &) = default;
    /** @brief The destructor */
    ~GParameterSetFormulaConstraint() override = default;

protected:
    /** @brief Checks whether a given GParameterSet object is valid */
    double check_(const GParameterSet *) const override;

    /** @brief Adds local configuration options to a GParserBuilder object */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &) override;
    /** @brief Loads the data of another GParameterSetConstraint */
    void load_(const GPreEvaluationValidityCheckT<GParameterSet> *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GParameterSetFormulaConstraint>(
        GParameterSetFormulaConstraint const &,
        GParameterSetFormulaConstraint const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GPreEvaluationValidityCheckT<GParameterSet> & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const override;

private:
    /** @brief Creates a deep clone of this object */
    GPreEvaluationValidityCheckT<GParameterSet> *clone_() const override;

    /** @brief The default constructor -- intentionally private, only needed for (de-)serialization */
    GParameterSetFormulaConstraint() = default;

    std::string raw_formula_; ///< Holds the raw formula, in which values haven't been replaced yet
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::Parameters::GParameterSetConstraint) // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Parameters::GParameterSetFormulaConstraint)       // NOLINT
