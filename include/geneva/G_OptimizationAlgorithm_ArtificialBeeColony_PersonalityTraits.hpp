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
        & BOOST_SERIALIZATION_NVP(trial_);
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

    G_API_GENEVA unsigned getTrial() const;

    G_API_GENEVA void setTrial(unsigned int trial);

    /** @brief Retrieves the mnemonic of the optimization algorithm */
    G_API_GENEVA std::string getMnemonic() const override;

protected:
    G_API_GENEVA void load_(const GObject *object) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GArtificialBeeColony_PersonalityTraits>(
            GArtificialBeeColony_PersonalityTraits const &
            , GArtificialBeeColony_PersonalityTraits const &
            , Gem::Common::GToken &
    );

    G_API_GENEVA void compare_(
            const GObject &object
            , const Common::expectation &expectation
            , const double &d
    ) const override;

    G_API_GENEVA bool modify_GUnitTests_() override;

    G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests_() override;

    G_API_GENEVA void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /** @brief Emits a name for this class / object */
    G_API_GENEVA std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    G_API_GENEVA GObject *clone_() const override;

    std::uint32_t trial_ = 0;
};

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GArtificialBeeColony_PersonalityTraits)