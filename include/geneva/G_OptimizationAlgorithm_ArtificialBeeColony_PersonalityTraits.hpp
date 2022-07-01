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
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/GenevaHelperFunctionsT.hpp"
#include "geneva/GParameterSet.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class adds variables and functions to GPersonalityTraits that are specific
 * to artificial bee colonies.
 */

class GArtificialBeeColony_PersonalityTraits :
        public GPersonalityTraits
{

///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GPersonalityTraits)
        & BOOST_SERIALIZATION_NVP(trial_)
        & BOOST_SERIALIZATION_NVP(onlookers_)
        & BOOST_SERIALIZATION_NVP(belongs_to_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief An easy identifier for the class */
    static G_API_GENEVA const std::string nickname; // Initialized in the .cpp definition file

    /** @brief The default constructor */
    G_API_GENEVA GArtificialBeeColony_PersonalityTraits() = default;
    /** @brief The copy contructor */
    G_API_GENEVA GArtificialBeeColony_PersonalityTraits(const GArtificialBeeColony_PersonalityTraits &);
    /** @brief The standard destructor */
    G_API_GENEVA ~GArtificialBeeColony_PersonalityTraits() override = default;

    G_API_GENEVA std::uint32_t getTrial() const;
    /** @brief Increases trial by 1 */
    G_API_GENEVA void increaseTrial();
    /** @brief Resets trial to 0 */
    G_API_GENEVA void resetTrial();

    /** @brief Retrieves the mnemonic of the optimization algorithm */
    G_API_GENEVA std::string getMnemonic() const override;
    /** @brief Retrieves the amount of Onlookers the Individual Currently has */
    G_API_GENEVA std::uint32_t getOnlookers() const;
    /** @brief Retrieves the amount of Onlookers the Individual Currently has */
    G_API_GENEVA void setOnlookers(std::uint32_t);
    /** @brief increases Onlookers by 1 */
    G_API_GENEVA void increaseOnlookers();
    /** @brief decreases Onlookers by 1 */
    G_API_GENEVA void decreaseOnlookers();
    /** @brief Resets onlookers to 0 */
    G_API_GENEVA void resetOnlookers();
    /** @brief Retrieves which Individual a copied object belongs to */
    G_API_GENEVA std::size_t getBelongsTo() const;
    /** @brief Sets which Individual a copied object belongs to */
    G_API_GENEVA void setBelongsTo(std::size_t);

protected:
    /** @brief Loads the data of another GABC_PersonalityTraits object */
    G_API_GENEVA void load_(const GObject *object) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GArtificialBeeColony_PersonalityTraits>(
            GArtificialBeeColony_PersonalityTraits const &
            , GArtificialBeeColony_PersonalityTraits const &
            , Gem::Common::GToken &
    );

    /**
     * Searches for compliance with expectations with respect to another object of the same type
     * @param object The other object
     * @param expectation The expectations for this object, e.g. equality
     * @param d The limit of deviation for comparing floating point types
     */
    G_API_GENEVA void compare_(
            const GObject &object
            , const Common::expectation &expectation
            , const double &d
    ) const override;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    G_API_GENEVA bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    G_API_GENEVA void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /** @brief Emits a name for this class / object */
    G_API_GENEVA std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    G_API_GENEVA GObject *clone_() const override;

    /** @brief The amount of trial the individual has */
    std::uint32_t trial_ = 0;
    /** @brief The amount of onlookers the individual has */
    std::uint32_t onlookers_ = 0;
    /** @brief The index of the individual in the population this individual belongs to */
    std::size_t belongs_to_ = -1;

};

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GArtificialBeeColony_PersonalityTraits)