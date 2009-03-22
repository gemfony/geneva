/**
 * @file GDataExchange.hpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License,
 * or, at your option, under the terms of version 2 of the GNU General Public
 * License, as published by the Free Software Foundation.
 *
 * NOTE THAT THIS FORM OF DUAL-LICENSING DOES NOT APPLY TO ANY OTHER FILES
 * OF THE GENEVA LIBRARY, UNLESS THIS IS EXPLICITLY STATED IN THE CORRSPONDING FILE.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of version 2 of the GNU General Public License
 * and of version 3 of the GNU Affero General Public License along with the Geneva
 * library. If not, see <http://www.gnu.org/licenses/>.
 */

// Standard headers go here
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// Boost headers go here
#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GDATAEXCHANGE_HPP_
#define GDATAEXCHANGE_HPP_

#include "GBoolParameter.hpp"
#include "GCharParameter.hpp"
#include "GDoubleParameter.hpp"
#include "GLongParameter.hpp"

namespace Gem
{
namespace Util
{

/*******************************************************************************/
/**
 * This class serves as an exchange vehicle between external programs and
 * the Geneva library. It allows to store and load parameters particular to a given
 * individual. Particular storage formats can be re-defined in derived classes
 * in order to accommodate "foreign" exchange formats. The class itself only
 * implements a very simple format, where all data is stored as ASCII data
 * consecutively in a file.
 */
class GDataExchange {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;

      ar & make_nvp("dArray_", dArray_);
      ar & make_nvp("lArray_", lArray_);
      ar & make_nvp("bArray_", bArray_);
      ar & make_nvp("cArray_", cArray_);
    }
    ///////////////////////////////////////////////////////////////////////

public:

private:
	/**************************************************************************/
	std::vector<double> dArray_; ///< Array holding double values
	std::vector<boost::int32_t> lArray_; ///< Array holding long values
	std::vector<bool> bArray_; ///< Array holding boolean values
	std::vector<char> cArray_; ///< Array holding character values
};

/*******************************************************************************/

} /* namespace Util */
} /* namespace Gem */

#endif /* GDATAEXCHANGE_HPP_ */

