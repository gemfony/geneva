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
#include "geneva/GParameterSet.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/GenevaHelperFunctionsT.hpp"

namespace Gem::Geneva {

/******************************************************************************/
/**
 * This class adds variables and functions to GPersonalityTraits that are specific
 * to swarm optimization.
 */
class GSwarmAlgorithm_PersonalityTraits // NOLINT(cppcoreguidelines-special-member-functions)
  : public GPersonalityTraits {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GPersonalityTraits) &
            BOOST_SERIALIZATION_NVP(neighborhood_) & BOOST_SERIALIZATION_NVP(noPositionUpdate_) &
            BOOST_SERIALIZATION_NVP(personal_best_) &
            BOOST_SERIALIZATION_NVP(personal_best_quality_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief An easy identifier for the class */
    static const std::string nickname; // Initialized in the .cpp definition file

    /** @brief The default constructor */
    GSwarmAlgorithm_PersonalityTraits() = default;
    /** @brief The copy contructor */
    GSwarmAlgorithm_PersonalityTraits(const GSwarmAlgorithm_PersonalityTraits &);
    /** @brief The standard destructor */
    ~GSwarmAlgorithm_PersonalityTraits() override = default;

    /** @brief Specifies in which neighborhood the individual is at present */
    void setNeighborhood(const std::size_t &);
    /** @brief Retrieves the id of the neighborhood the individual is in at present */
    std::size_t getNeighborhood() const;

    /** @brief Sets the noPositionUpdate_ flag */
    void setNoPositionUpdate();
    /** @brief Retrieves the current value of the noPositionUpdate_ flag */
    bool noPositionUpdate() const;
    /** @brief Retrieves and resets the current value of the noPositionUpdate_ flag */
    bool checkNoPositionUpdateAndReset();

    /** @brief Allows to add a new personal best to the individual */
    void registerPersonalBest(std::shared_ptr<GParameterSet>);
    /** @brief Allows to retrieve the personal best individual */
    std::shared_ptr<GParameterSet> getPersonalBest() const;
    /** @brief Resets the personal best individual */
    void resetPersonalBest();
    /** @brief Retrieve quality of personally best individual */
    std::tuple<double, double> getPersonalBestQuality() const;

    /** @brief Retrieves the mnemonic of the optimization algorithm */
    std::string getMnemonic() const override;

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /** @brief Loads the data of another GSwarmPersonalityTraits object */
    void load_(const GObject *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GSwarmAlgorithm_PersonalityTraits>(
        GSwarmAlgorithm_PersonalityTraits const &,
        GSwarmAlgorithm_PersonalityTraits const &,
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

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

    /***************************************************************************/

private:
    /** @brief Emits a name for this class / object */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    GObject *clone_() const override;

    /** @brief Stores the current position in the population */
    std::size_t neighborhood_ = 0;

    /** @brief Determines whether the individual has been randomly initialized */
    bool noPositionUpdate_ = false;

    /** @brief Holds the personally best GParameterSet */
    std::shared_ptr<GParameterSet> personal_best_;
    /** @brief The quality of the personally best individual */
    std::tuple<double, double> personal_best_quality_{0., 0.};
};

/******************************************************************************/

} /* namespace Gem::Geneva */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GSwarmAlgorithm_PersonalityTraits) // NOLINT
