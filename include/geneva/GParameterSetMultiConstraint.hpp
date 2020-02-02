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
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GFormulaParserT.hpp"
#include "geneva/GMultiConstraintT.hpp"
#include "geneva/GIndividualMultiConstraint.hpp"
#include "geneva/GParameterSet.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class implements constraint definitions based on GParameterSet-derivatives.
 * It is meant to be added to a constraint collection. The main purpose of this
 * class is to "translate" GParameterSet-based constraints into constraints
 * based on GParameterSets
 */
class GParameterSetConstraint :
    public GPreEvaluationValidityCheckT<GParameterSet>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GPreEvaluationValidityCheckT<GParameterSet>);
    }
    ///////////////////////////////////////////////////////////////////////
public:

    /** @brief The default constructor */
    G_API_GENEVA GParameterSetConstraint() = default;
    /** @brief The copy constructor */
    G_API_GENEVA GParameterSetConstraint(const GParameterSetConstraint &) = default;
    /** @brief The destructor */
    G_API_GENEVA ~GParameterSetConstraint() override = default;

protected:
    /** @brief Checks whether a given individual is valid */
    G_API_GENEVA double check_(const GParameterSet *) const override = 0;

    /** @brief Adds local configuration options to a GParserBuilder object */
    G_API_GENEVA void addConfigurationOptions_(Gem::Common::GParserBuilder &) override;
    /** @brief Loads the data of another GParameterSetConstraint */
    G_API_GENEVA void load_(const GObject *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GParameterSetConstraint>(
        GParameterSetConstraint const &
        , GParameterSetConstraint const &
        , Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    G_API_GENEVA void compare_(
        const GObject & // the other object
        , const Gem::Common::expectation & // the expectation for this object, e.g. equality
        , const double & // the limit for allowed deviations of floating point types
    ) const override;

private:
    /** @brief Creates a deep clone of this object */
    G_API_GENEVA GObject *clone_() const override = 0;
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
class GParameterSetFormulaConstraint :
    public GParameterSetConstraint
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSetConstraint)
        & BOOST_SERIALIZATION_NVP(rawFormula_);
    }
    ///////////////////////////////////////////////////////////////////////
public:
    /** @brief The default constructor */
    explicit G_API_GENEVA GParameterSetFormulaConstraint(std::string);
    /** @brief The copy constructor */
    G_API_GENEVA GParameterSetFormulaConstraint(const GParameterSetFormulaConstraint &) = default;
    /** @brief The destructor */
    G_API_GENEVA ~GParameterSetFormulaConstraint() override = default;

protected:
    /** @brief Checks whether a given GParameterSet object is valid */
    G_API_GENEVA double check_(const GParameterSet *) const override;

    /** @brief Adds local configuration options to a GParserBuilder object */
    G_API_GENEVA void addConfigurationOptions_(Gem::Common::GParserBuilder &) override;
    /** @brief Loads the data of another GParameterSetConstraint */
    G_API_GENEVA void load_(const GObject *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GParameterSetFormulaConstraint>(
        GParameterSetFormulaConstraint const &
        , GParameterSetFormulaConstraint const &
        , Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    G_API_GENEVA void compare_(
        const GObject & // the other object
        , const Gem::Common::expectation & // the expectation for this object, e.g. equality
        , const double & // the limit for allowed deviations of floating point types
    ) const override;

private:
    /** @brief Creates a deep clone of this object */
    G_API_GENEVA GObject *clone_() const override;

    /** @brief The default constructor -- intentionally private, only needed for (de-)serialization */
    G_API_GENEVA GParameterSetFormulaConstraint() = default;

    std::string rawFormula_; ///< Holds the raw formula, in which values haven't been replaced yet
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GParameterSetConstraint)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GParameterSetFormulaConstraint)

