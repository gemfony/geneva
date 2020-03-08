/**
 * @file GSimpleContainer.hpp
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
#include <boost/serialization/variant.hpp>
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

namespace Gem {
namespace Courtier {
namespace Tests {

/**********************************************************************************************/
/**
 * This class implements the simplest-possible container object, used for tests of the courtier lib.
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
	 GSimpleContainer(const std::size_t&);
	 /** @brief The copy constructor */
	 GSimpleContainer(const GSimpleContainer&) = default;
	 /** @brief The destructor */
	 virtual ~GSimpleContainer() = default;

	 /** @brief Prints out this objects random number container */
	 void print();

private:
	 /** @brief The default constructor -- only needed for de-serialization purposes */
	 GSimpleContainer() = default;
	 /** @brief Allows to specify the tasks to be performed for this object */
	 virtual void process_() override;

	 std::size_t m_stored_number = 0; ///< Holds the pay-load of this object
};

/**********************************************************************************************/

} /* namespace Tests */
} /* namespace Courtier */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Courtier::Tests::GSimpleContainer)

