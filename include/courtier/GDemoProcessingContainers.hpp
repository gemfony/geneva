/**
 * @file GDemoProcessingContainers.hpp
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
#include <vector>
#include <iostream>

// Boost headers go here
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>

// Geneva headers go here
#include "common/GSerializeTupleT.hpp"
#include "courtier/GProcessingContainerT.hpp"
#include "hap/GRandomT.hpp"

namespace Gem::Courtier {

/**********************************************************************************************/
/**
 * This class implements the simplest-possible procesiing container object, used for tests of
 * the courtier lib.
 */
class GSimpleContainer
        : public Gem::Courtier::GProcessingContainerT<GSimpleContainer, bool>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int){
        using boost::serialization::make_nvp;

        ar
        & make_nvp("GProcessingContainerT_GSimpleContainer", boost::serialization::base_object<Gem::Courtier::GProcessingContainerT<GSimpleContainer, bool>>(*this))
        & BOOST_SERIALIZATION_NVP(m_stored_number);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The standard constructor -- Initialization with an amount of random numbers */
    G_API_COURTIER explicit GSimpleContainer(const std::size_t&);
    /** @brief The copy constructor */
    G_API_COURTIER GSimpleContainer(const GSimpleContainer&) = default;
    /** @brief The destructor */
    G_API_COURTIER ~GSimpleContainer() override = default;

    /** @brief Prints out this objects random number container */
    G_API_COURTIER void print();

private:
    /** @brief The default constructor -- only needed for de-serialization purposes */
    G_API_COURTIER GSimpleContainer() = default;
    /** @brief Allows to specify the tasks to be performed for this object */
    G_API_COURTIER void process_() override;

    std::size_t m_stored_number = 0; ///< Holds the pay-load of this object
};

/**********************************************************************************************/
/**
 * This class implements a container of random objects, used for tests of the courtier lib.
 */
class GRandomNumberContainer
        : public Gem::Courtier::GProcessingContainerT<GRandomNumberContainer, bool>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int){
        using boost::serialization::make_nvp;

        ar & make_nvp("GProcessingContainerT_GRandomNumberContainer", boost::serialization::base_object<Gem::Courtier::GProcessingContainerT<GRandomNumberContainer, bool>>(*this))
        & BOOST_SERIALIZATION_NVP(randomNumbers_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The standard constructor -- Initialization with an amount of random numbers */
    explicit G_API_COURTIER GRandomNumberContainer(const std::size_t&);

    /******************************************************************************************/
    // Defaulted constructors, destructor and assignment operators

    // Default constructor is in the private section (only needed for de-serialization)

    G_API_COURTIER GRandomNumberContainer(const GRandomNumberContainer&) = default;
    G_API_COURTIER ~GRandomNumberContainer() override = default;

    GRandomNumberContainer& operator=(GRandomNumberContainer const&) = default;

    // TODO: Make class movable --> see base class
    G_API_COURTIER GRandomNumberContainer(GRandomNumberContainer&&) = delete;
    G_API_COURTIER GRandomNumberContainer& operator=(GRandomNumberContainer &&) = delete;

    /******************************************************************************************/

    /** @brief Prints out this objects random number container */
    G_API_COURTIER void print();

private:
    /** @brief The default constructor -- only needed for de-serialization purposes */
    G_API_COURTIER GRandomNumberContainer() = default;
    /** @brief Allows to specify the tasks to be performed for this object */
    G_API_COURTIER void process_() override;

    std::vector<double> randomNumbers_; ///< Holds the pay-load of this object
};

/**********************************************************************************************/

} // namespace Gem::Courtier

BOOST_CLASS_EXPORT_KEY(Gem::Courtier::GSimpleContainer)
BOOST_CLASS_EXPORT_KEY(Gem::Courtier::GRandomNumberContainer)
