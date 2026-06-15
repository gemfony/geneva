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
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include "geneva/oa/GGradientDescent_PersonalityTraits.hpp"
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
/** @brief Puts a Gem::Common::logType into a stream. Needed also for boost::lexical_cast<> */
std::ostream &operator<<(std::ostream &, const Gem::Geneva::Individuals::PERFOBJECTTYPE &);
/** @brief Reads a Gem::Common::logType from a stream. Needed also for boost::lexical_cast<> */
std::istream &operator>>(std::istream &, Gem::Geneva::Individuals::PERFOBJECTTYPE &);

/******************************************************************************/
/**
 * This individual serves as the basis for unit tests of the individual hierarchy. At the time
 * of writing, it was included in order to be able to set the individual's personality without
 * weakening data protection.
 */
class GTestIndividual2
  : public gpar::GFlatGenome { // NOLINT(cppcoreguidelines-special-member-functions)
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(gpar::GFlatGenome);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GTestIndividual2(const std::size_t &, const PERFOBJECTTYPE &);
    /** @brief The copy constructor */
    GTestIndividual2(const GTestIndividual2 &);

    /** @brief The standard destructor */
    ~GTestIndividual2() override;

    /** @brief The OA-owned Gauss adaption config authoring every double group of this genome (used by the
     *  self-driven StandaloneAdapter benchmark / manual-test sites). */
    std::shared_ptr<OptimizationAlgorithms::GAdaptionConfigBase> getAdaptionConfig() const;

protected:
    /** @brief Loads the data of another GTestIndividual2 */
    void load_(const gpar::GOptimizableEntity *) final;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GTestIndividual2>(
        GTestIndividual2 const &,
        GTestIndividual2 const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const gpar::GOptimizableEntity & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const final;

    /** @brief The actual fitness calculation takes place here. */
    double fitnessCalculation() final;

    /** @brief Applies modifications to this object. */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /** @brief Creates a deep clone of this object */
    gpar::GFlatGenome *clone_() const final;

    /** @brief The default constructor -- protected, as it is only needed for (de-)serialization purposes */
    GTestIndividual2();
};

/******************************************************************************/

} /* namespace Gem::Geneva::Individuals */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Individuals::GTestIndividual2) // NOLINT
