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
#include <random>

// Boost headers go here

// Geneva headers go here
#include "geneva/GParameterT.hpp"
#include "geneva/GBooleanAdaptor.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class encapsulates a single bit, represented as a bool. This might appear heavy weight,
 * and indeed for most applications this is not the recommended solution -
 * use the GBooleanCollection instead.
 */
class GBooleanObject
    :
        public GParameterT<bool>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & make_nvp(
            "GParameterT_bool"
            , boost::serialization::base_object<GParameterT<bool>>(*this));
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    G_API_GENEVA GBooleanObject() = default;
    /** @brief The copy constructor */
    G_API_GENEVA GBooleanObject(const GBooleanObject &) = default;
    /** @brief Initialization by contained value */
    explicit G_API_GENEVA GBooleanObject(const bool &);
    /** @brief Initialization with a given probability for "true" */
    explicit G_API_GENEVA GBooleanObject(const double &);
    /** @brief The destructor */
    G_API_GENEVA ~GBooleanObject() override = default;

    /** @brief An assignment operator */
    G_API_GENEVA bool operator=(const bool &) override;

    /** @brief Triggers random initialization of the parameter object */
    G_API_GENEVA bool randomInit(
        const activityMode &
        , Gem::Hap::GRandomBase &
    ) override;
    /** @brief Triggers random initialization of the parameter object, with a given likelihood structure */
    G_API_GENEVA bool randomInit(
        const double &
        , const activityMode &
        , Gem::Hap::GRandomBase &
    );

    /** @brief Flips the value of this object */
    G_API_GENEVA void flip();

protected:
    /** @brief Loads the data of another GObject */
    G_API_GENEVA void load_(const GObject *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GBooleanObject>(
        GBooleanObject const &
        , GBooleanObject const &
        , Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    G_API_GENEVA void compare_(
        const GObject & // the other object
        , const Gem::Common::expectation & // the expectation for this object, e.g. equality
        , const double & // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Triggers random initialization of the parameter object */
    G_API_GENEVA bool randomInit_(
        const activityMode &
        , Gem::Hap::GRandomBase &
    ) override;
    /** @brief Triggers random initialization of the parameter object, with a given likelihood structure */
    G_API_GENEVA bool randomInit_(
        const double &
        , const activityMode &
        , Gem::Hap::GRandomBase &
    );

    /** @brief Returns a "comparative range" for this type */
    G_API_GENEVA bool range() const override;

    /** @brief Attach our local value to the vector. */
    G_API_GENEVA void booleanStreamline(std::vector<bool> &, const activityMode &am) const override;
    /** @brief Attach boundaries of type bool to the vectors */
    G_API_GENEVA void booleanBoundaries(
        std::vector<bool> &, std::vector<bool> &, const activityMode &am
    ) const override;

    /** @brief Tell the audience that we own a std::int32_t value */
    std::size_t countBoolParameters(const activityMode &am) const override;
    /** @brief Assigns part of a value vector to the parameter */
    G_API_GENEVA void assignBooleanValueVector(
        const std::vector<bool> &, std::size_t &, const activityMode &am
    ) override;
    /** @brief Attach our local value to the map. */
    G_API_GENEVA void booleanStreamline(
        std::map<std::string, std::vector<bool>> &, const activityMode &am
    ) const override;
    /** @brief Assigns part of a value map to the parameter */
    G_API_GENEVA void assignBooleanValueVectors(
        const std::map<std::string, std::vector<bool>> &, const activityMode &am
    ) override;

private:
    /** @brief Emits a name for this class / object */
    G_API_GENEVA std::string name_() const override;
    /** @brief Creates a deep clone of this object. */
    G_API_GENEVA GObject *clone_() const override;

public:
    /** @brief Applies modifications to this object. This is needed for testing purposes */
    G_API_GENEVA bool modify_GUnitTests() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() override;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBooleanObject)

