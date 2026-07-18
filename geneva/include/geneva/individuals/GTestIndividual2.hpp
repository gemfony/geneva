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
#include <algorithm> // for std::sort
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <list>
#include <sstream>
#include <vector>

// Boost header files go here

// Geneva header files go here
#include "common/GCommonEnums.hpp"
#include "common/GExceptions.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GGenome.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include "geneva/oa/GSwarmAlgorithm_PersonalityTraits.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {
class GAdaptionConfigBase;
} // namespace Gem::Geneva::OptimizationAlgorithms

namespace Gem::Geneva::Individuals {

/**
 * The types of objects to be tested in this class
 */
enum class PERFOBJECTTYPE : Gem::Common::ENUMBASETYPE {
    PERFGDOUBLEOBJECT = 0,
    PERFGCONSTRDOUBLEOBJECT = 1,
    PERFGCONSTRAINEDDOUBLEOBJECTCOLLECTION = 2,
    PERFGDOUBLECOLLECTION = 3,
    PERFGCONSTRAINEDDOUBLECOLLECTION = 4
};

const PERFOBJECTTYPE POTMIN = PERFOBJECTTYPE::PERFGDOUBLEOBJECT;
const PERFOBJECTTYPE POTMAX = PERFOBJECTTYPE::PERFGCONSTRAINEDDOUBLEOBJECTCOLLECTION;
constexpr std::size_t NPERFOBJECTTYPES = 5;

/******************************************************************************/
/**
 * @brief Puts a Gem::Geneva::Individuals::PERFOBJECTTYPE into a stream. Needed for streaming / Gem::Common::fromString<>
 * @param o The output stream the value is written to
 * @param lt The PERFOBJECTTYPE value to be streamed out
 * @return A reference to the output stream, to allow chaining of stream operations
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::Individuals::PERFOBJECTTYPE &lt);
/**
 * @brief Reads a Gem::Geneva::Individuals::PERFOBJECTTYPE from a stream. Needed for streaming / Gem::Common::fromString<>
 * @param i The input stream the value is read from
 * @param lt The PERFOBJECTTYPE variable the read-in value is assigned to
 * @return A reference to the input stream, to allow chaining of stream operations
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::Individuals::PERFOBJECTTYPE &lt);

/******************************************************************************/
/**
 * This individual serves as the basis for unit tests of the individual hierarchy. At the time
 * of writing, it was included in order to be able to set the individual's personality without
 * weakening data protection.
 */
class GTestIndividual2
  : public gen::GGenome { // NOLINT(cppcoreguidelines-special-member-functions)
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(gen::GGenome);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /**
     * @brief Initialising constructor: builds a flat genome holding the requested number of double
     *  parameters of the requested representation type
     * @param n_objects The number of double parameters (or collection entries) to add to the genome
     * @param otype The representation type of the double parameters to be created (see PERFOBJECTTYPE)
     */
    GTestIndividual2(const std::size_t &n_objects, const PERFOBJECTTYPE &otype);
    /**
     * @brief The copy constructor
     * @param cp Another GTestIndividual2 object whose data is copied into this one
     */
    GTestIndividual2(const GTestIndividual2 &cp);

    /** @brief The standard destructor */
    ~GTestIndividual2() override;

    /**
     * @brief The OA-owned Gauss adaption config authoring every double group of this genome (used by the
     *  self-driven StandaloneAdapter benchmark / manual-test sites).
     * @return A shared pointer to the Gauss adaption config describing every double group of this genome
     */
    std::shared_ptr<OptimizationAlgorithms::GAdaptionConfigBase> getAdaptionConfig() const;

protected:
    /**
     * @brief Loads the data of another GTestIndividual2
     * @param cp A pointer to another GTestIndividual2 object, camouflaged as a GOptimizableEntity
     */
    void load_(const gen::GOptimizableEntity *cp) final;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GTestIndividual2>(
        GTestIndividual2 const &,
        GTestIndividual2 const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type
     * @param cp The other object to compare this one against, camouflaged as a GOptimizableEntity
     * @param e The expectation for this comparison, e.g. equality or inequality
     * @param limit The limit for allowed deviations of floating point types
     */
    void compare_(
        const gen::GOptimizableEntity &cp // the other object
        ,
        const Gem::Common::expectation &e // the expectation for this object, e.g. equality
        ,
        const double &limit // the limit for allowed deviations of floating point types
    ) const final;

    /**
     * @brief The evaluation hook: computes this individual's fitness.
     * @return The fitness as a one-element vector (single criterion)
     */
    std::vector<double> evaluate() final;

    /**
     * @brief Applies modifications to this object.
     * @return A boolean indicating whether a modification was actually carried out
     */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /**
     * @brief Creates a deep clone of this object
     * @return A deep clone of this object, returned as a pointer to its GGenome base
     */
    gen::GGenome *clone_() const final;

    /** @brief The default constructor -- protected, as it is only needed for (de-)serialization purposes */
    GTestIndividual2();
};

/******************************************************************************/

} /* namespace Gem::Geneva::Individuals */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Individuals::GTestIndividual2) // NOLINT
