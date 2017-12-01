/**
 * @file GRandomNumberContainer.hpp
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

#ifndef GRANDOMNUMBERCONTAINER_HPP_
#define GRANDOMNUMBERCONTAINER_HPP_

// Geneva headers go here
#include "common/GSerializeTupleT.hpp"
#include "courtier/GProcessingContainerT.hpp"
#include "hap/GRandomT.hpp"

namespace Gem {
namespace Courtier {
namespace Tests {

/**********************************************************************************************/
/**
 * This class implements a container of random objects, used for tests of the courtier lib.
 */
class GRandomNumberContainer
	:public Gem::Courtier::GProcessingContainerT<GRandomNumberContainer, bool>
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
	 GRandomNumberContainer(const std::size_t&);
	 /** @brief The copy constructor */
	 GRandomNumberContainer(const GRandomNumberContainer&) = default;
	 /** @brief The destructor */
	 virtual ~GRandomNumberContainer() = default;

	 /** @brief Prints out this objects random number container */
	 void print();

private:
	 /** @brief The default constructor -- only needed for de-serialization purposes */
	 GRandomNumberContainer() = default;

	 /** @brief Allows to specify the tasks to be performed for this object */
	 virtual void process_() override;

	 /** @brief Allows to give an indication of the processing result; may not throw. */
	 virtual bool get_processing_result() const noexcept override;

	 std::vector<double> randomNumbers_; ///< Holds the pay-load of this object
};

/**********************************************************************************************/

} /* namespace Tests */
} /* namespace Courtier */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Courtier::Tests::GRandomNumberContainer)

#endif /* GRANDOMNUMBERCONTAINER_HPP_ */
