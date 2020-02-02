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

// Standard headers go here

// Boost headers go here

// Geneva headers go here
#include "geneva/G_OptimizationAlgorithm_ParChildT_PersonalityTraits.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class adds variables and functions to GPersonalityTraits that are specific
 * to evolutionary algorithms.
 */
class GEvolutionaryAlgorithm_PersonalityTraits
    :
        public GBaseParChildPersonalityTraits
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBaseParChildPersonalityTraits)
        & BOOST_SERIALIZATION_NVP(m_isOnParetoFront);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief An easy identifier for the class */
    static G_API_GENEVA const std::string nickname; // Initialized in the .cpp definition file

    /** @brief The default constructor */
    G_API_GENEVA GEvolutionaryAlgorithm_PersonalityTraits() = default;
    /** @brief The copy contructor */
    G_API_GENEVA GEvolutionaryAlgorithm_PersonalityTraits(const GEvolutionaryAlgorithm_PersonalityTraits &) = default;
    /** @brief The standard destructor */
    G_API_GENEVA ~GEvolutionaryAlgorithm_PersonalityTraits() override = default;

    /** @brief Allows to check whether this individual lies on the pareto front (only yields useful results after pareto-sorting in EA) */
    G_API_GENEVA bool isOnParetoFront() const;
    /** @brief Allows to reset the pareto tag to "true" */
    G_API_GENEVA void resetParetoTag();
    /** @brief Allows to specify that this individual does not lie on the pareto front of the current iteration */
    G_API_GENEVA void setIsNotOnParetoFront();

    /** @brief Retrieves the mnemonic of the optimization algorithm */
    G_API_GENEVA std::string getMnemonic() const override;

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /** @brief Loads the data of another GEAPersonalityTraits object */
    G_API_GENEVA void load_(const GObject *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GEvolutionaryAlgorithm_PersonalityTraits>(
        GEvolutionaryAlgorithm_PersonalityTraits const &
        , GEvolutionaryAlgorithm_PersonalityTraits const &
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

    /***************************************************************************/

private:
    /** @brief Emits a name for this class / object */
    G_API_GENEVA std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    G_API_GENEVA GObject *clone_() const override;

    /** @brief Determines whether the individual lies on the pareto front */
    bool m_isOnParetoFront = true;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GEvolutionaryAlgorithm_PersonalityTraits)


