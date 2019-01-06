/**
 * @file
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here

// Boost headers go here

// Geneva headers go here
#include "geneva/GPersonalityTraits.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class adds variables and functions to GPersonalityTraits that are specific
 * to gradient descents.
 */
class GParameterScan_PersonalityTraits :
    public GPersonalityTraits
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GPersonalityTraits)
        & BOOST_SERIALIZATION_NVP(m_popPos);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief An easy identifier for the class */
    static G_API_GENEVA const std::string nickname; // Initialized in the .cpp definition file

    /** @brief The default constructor */
    G_API_GENEVA GParameterScan_PersonalityTraits() = default;
    /** @brief The copy contructor */
    G_API_GENEVA GParameterScan_PersonalityTraits(const GParameterScan_PersonalityTraits &) = default;

    /** @brief The standard destructor */
    G_API_GENEVA ~GParameterScan_PersonalityTraits() override = default;

    /** @brief Sets the position of the individual in the population */
    G_API_GENEVA void setPopulationPosition(const std::size_t &);
    /** @brief Retrieves the position of the individual in the population */
    G_API_GENEVA std::size_t getPopulationPosition() const;

    /** @brief Retrieves the mnemonic of the optimization algorithm */
    G_API_GENEVA std::string getMnemonic() const override;

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /** @brief Loads the data of another GPSPersonalityTraits object */
    G_API_GENEVA void load_(const GObject *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GParameterScan_PersonalityTraits>(
        GParameterScan_PersonalityTraits const &
        , GParameterScan_PersonalityTraits const &
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

    /** @brief Stores the current position in the population */
    std::size_t m_popPos = 0;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GParameterScan_PersonalityTraits)

