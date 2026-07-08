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
#include <tuple>
#include <vector>

// Boost header files go here

// Geneva header files go here
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GParserBuilder.hpp"
#include "common/GSingletonT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GGenome.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"

namespace Gem::Geneva::Individuals {

/******************************************************************************/
/**
 * This individual tests different access methods for parameter objects inside
 * of the individual.
 */
class GTestIndividual3 // NOLINT(cppcoreguidelines-special-member-functions)
  : public gen::GGenome {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;
        using namespace Gem::Geneva;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(gen::GGenome);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GTestIndividual3();
    /**
     * @brief The copy constructor
     * @param cp Another GTestIndividual3 object whose data is copied into this one
     */
    GTestIndividual3(const GTestIndividual3 &cp);

    /** @brief The destructor */
    ~GTestIndividual3() override;

    /**
     * @brief Get all data members of this class as a plain array
     * @return A shared pointer to a freshly allocated float array (with array deleter) holding all of this
     *  individual's double parameters, narrowed to float in flat streamline order
     */
    std::shared_ptr<float> getPlainData() const;

protected:
    /**
     * @brief Loads the data of another GTestIndividual3
     * @param cp A pointer to another GTestIndividual3 object, camouflaged as a GOptimizableEntity
     */
    void load_(const gen::GOptimizableEntity *cp) final;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GTestIndividual3>(
        GTestIndividual3 const &,
        GTestIndividual3 const &,
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
        [[maybe_unused]] const double &limit // the limit for allowed deviations of floating point types
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
};

/******************************************************************************/

} /* namespace Gem::Geneva::Individuals */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Individuals::GTestIndividual3) // NOLINT
