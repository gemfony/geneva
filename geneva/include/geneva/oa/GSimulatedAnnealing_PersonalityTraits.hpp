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
#include "geneva/oa/GBaseParChildPersonalityTraits.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to add variables and functions to GPersonalityTraits that
 * are specific to simulated annealing. Note that at the current time this class
 * adds no additional data. Since each optimization algorithm needs its own
 * personality type, though, we provide this default implementation. The base
 * class is the same as for evolutionary algorithms, as in Geneva Simulated Annealing
 * uses the same framework.
 */
class GSimulatedAnnealing_PersonalityTraits // NOLINT(cppcoreguidelines-special-member-functions)
  : public GBaseParChildPersonalityTraits {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBaseParChildPersonalityTraits);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief An easy identifier for the class */
    static const std::string nickname; // Initialized in the .cpp definition file

    /** @brief The default constructor */
    GSimulatedAnnealing_PersonalityTraits() = default;
    /** @brief The copy contructor */
    
    GSimulatedAnnealing_PersonalityTraits(const GSimulatedAnnealing_PersonalityTraits &) = default;

    /** @brief The standard destructor */
    ~GSimulatedAnnealing_PersonalityTraits() override = default;

    /**
     * @brief Retrieves the mnemonic of the optimization algorithm.
     * @return The short mnemonic string identifying the simulated annealing algorithm
     */
    std::string getMnemonic() const override;

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /**
     * @brief Loads the data of another GSimulatedAnnealing_PersonalityTraits object into this one.
     *
     * The (unnamed) argument is a pointer to another GSimulatedAnnealing_PersonalityTraits object,
     * camouflaged as a GPersonalityTraits, whose data is copied into this object.
     */
    void load_(const GPersonalityTraits *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GSimulatedAnnealing_PersonalityTraits>(
        GSimulatedAnnealing_PersonalityTraits const &,
        GSimulatedAnnealing_PersonalityTraits const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type.
     *
     * The three (unnamed) arguments are, in order: the other object to compare against (camouflaged as a
     * GPersonalityTraits), the expectation for this object (e.g. equality), and the limit for allowed
     * deviations of floating point types.
     */
    void compare_(
        const GPersonalityTraits &cp // the other object
        ,
        const Gem::Common::expectation &e // the expectation for this object, e.g. equality
        ,
        const double &limit // the limit for allowed deviations of floating point types
    ) const override;

    /**
     * @brief Applies modifications to this object. This is needed for testing purposes.
     * @return A boolean indicating whether modifications were actually carried out
     */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

    /***************************************************************************/

private:
    /**
     * @brief Emits a name for this class / object.
     * @return A string holding the name of this class
     */
    std::string name_() const override;
    /**
     * @brief Creates a deep clone of this object.
     * @return A deep clone of this object, allocated on the heap (caller takes ownership)
     */
    GPersonalityTraits *clone_() const override;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GSimulatedAnnealing_PersonalityTraits) // NOLINT

