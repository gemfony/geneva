/**
 * @file GSerializationHelperFunctionsT.hpp
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

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Standard headers go here

#include <iostream>
#include <sstream>
#include <string>
#include <sstream>
#include <vector>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/utility.hpp>
#include <boost/bind.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/date_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cast.hpp>

#ifndef GSERIALIZATIONHELPERFUNCTIONST_HPP_
#define GSERIALIZATIONHELPERFUNCTIONST_HPP_

// Geneva headers go here
#include "common/GCommonEnums.hpp"
#include "common/GLogger.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * Converts a shared_ptr<T> into its string representation. This template function thus assumes that
 * T is serializable using the Boost.Serialization framework.
 *
 * @param gt_ptr A shared_ptr to the object to be serialized
 * @param sm The corresponding serialization mode
 * @return A string representation of gt_ptr
 */
template <typename T>
std::string sharedPtrToString(boost::shared_ptr<T> gt_ptr, const Gem::Common::serializationMode& serMod){
	std::ostringstream oarchive_stream;

	switch(serMod){
	case Gem::Common::SERIALIZATIONMODE_TEXT:
		{
			boost::archive::text_oarchive oa(oarchive_stream);
			oa << boost::serialization::make_nvp("classHierarchyFromT_ptr", gt_ptr);
		} // note: explicit scope here is essential so the oa-destructor gets called

		break;

	case Gem::Common::SERIALIZATIONMODE_XML:
		{
			boost::archive::xml_oarchive oa(oarchive_stream);
			oa << boost::serialization::make_nvp("classHierarchyFromT_ptr", gt_ptr);
		}
		break;

	case Gem::Common::SERIALIZATIONMODE_BINARY:
		{
			boost::archive::binary_oarchive oa(oarchive_stream);
			oa << boost::serialization::make_nvp("classHierarchyFromT_ptr", gt_ptr);
		}

		break;
	}

	return oarchive_stream.str();
}

/******************************************************************************/
/**
 * Loads a shared_ptr<T> from its string representation. This template function thus assumes that
 * T is de-serializable using the Boost.Serialization framework.
 *
 * @param gt_string A string representation of the object to be restored
 * @param sm The corresponding serialization mode
 * @return A shared_ptr to the restored object
 */
template <typename T>
boost::shared_ptr<T> sharedPtrFromString(const std::string& gt_string, const Gem::Common::serializationMode& serMod){
	std::istringstream istr(gt_string);
	boost::shared_ptr<T> gt_ptr;

	try {
	switch(serMod){
	case Gem::Common::SERIALIZATIONMODE_TEXT:
		{
			boost::archive::text_iarchive ia(istr);
			ia >> boost::serialization::make_nvp("classHierarchyFromT_ptr", gt_ptr);
		} // note: explicit scope here is essential so the ia-destructor gets called

		break;

	case Gem::Common::SERIALIZATIONMODE_XML:
		{
			boost::archive::xml_iarchive ia(istr);
			ia >> boost::serialization::make_nvp("classHierarchyFromT_ptr", gt_ptr);
		}

		break;

	case Gem::Common::SERIALIZATIONMODE_BINARY:
		{
			boost::archive::binary_iarchive ia(istr);
			ia >> boost::serialization::make_nvp("classHierarchyFromT_ptr", gt_ptr);
		}
		break;
	}
	} catch (boost::archive::archive_exception& e) {
	   glogger
	   << "In sharedPtrFromString(): Error!" << std::endl
	   << "Caught boost::archive::archive_exception" << std::endl
	   << "with message" << std::endl
	   << e.what() << std::endl
	   << GEXCEPTION;
	} catch (...) {
      glogger
      << "In sharedPtrFromString(): Error!" << std::endl
      << "Caught unknown exception" << std::endl
      << GEXCEPTION;
	}

	return gt_ptr;
}

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GSERIALIZATIONHELPERFUNCTIONST_HPP_ */
