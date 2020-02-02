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

#include <iostream>
#include <sstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <atomic>

// Boost headers go here

#include <boost/utility.hpp>
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
#include <boost/lexical_cast.hpp>
#include <boost/cast.hpp>
#include <boost/logic/tribool.hpp>

// Geneva headers go here
#include "common/GCommonEnums.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GLogger.hpp"
#include "common/GExceptions.hpp"
#include "common/GErrorStreamer.hpp"

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
template<typename T>
std::string sharedPtrToString(std::shared_ptr<T> gt_ptr, const Gem::Common::serializationMode &serMod) {
	std::ostringstream oarchive_stream;

	switch (serMod) {
		case Gem::Common::serializationMode::TEXT: {
			boost::archive::text_oarchive oa(oarchive_stream);
			oa << boost::serialization::make_nvp("classHierarchyFromT_ptr", gt_ptr);
		} // note: explicit scope here is essential so the oa-destructor gets called

			break;

		case Gem::Common::serializationMode::XML: {
			boost::archive::xml_oarchive oa(oarchive_stream);
			oa << boost::serialization::make_nvp("classHierarchyFromT_ptr", gt_ptr);
		}
			break;

		case Gem::Common::serializationMode::BINARY: {
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
template<typename T>
std::shared_ptr<T> sharedPtrFromString(const std::string &gt_string, const Gem::Common::serializationMode &serMod) {
	std::istringstream istr(gt_string);
	std::shared_ptr<T> gt_ptr;

	try {
		switch (serMod) {
			case Gem::Common::serializationMode::TEXT: {
				boost::archive::text_iarchive ia(istr);
				ia >> boost::serialization::make_nvp("classHierarchyFromT_ptr", gt_ptr);
			} // note: explicit scope here is essential so the ia-destructor gets called

				break;

			case Gem::Common::serializationMode::XML: {
				boost::archive::xml_iarchive ia(istr);
				ia >> boost::serialization::make_nvp("classHierarchyFromT_ptr", gt_ptr);
			}

				break;

			case Gem::Common::serializationMode::BINARY: {
				boost::archive::binary_iarchive ia(istr);
				ia >> boost::serialization::make_nvp("classHierarchyFromT_ptr", gt_ptr);
			}
				break;
		}
	} catch (boost::archive::archive_exception &e) {
		glogger
		<< "In sharedPtrFromString(): Error!" << std::endl
		<< "Caught boost::archive::archive_exception" << std::endl
		<< "with message" << std::endl
		<< e.what() << std::endl
		<< "We will return an empty pointer." << std::endl
		<< GWARNING;

		return std::shared_ptr<T>();
	} catch (std::exception& e) {
		glogger
			<< "In sharedPtrFromString(): Error!" << std::endl
			<< "Caught std::exception with message" << std::endl
			<< e.what() << std::endl
			<< "We will return an empty pointer." << std::endl
			<< GWARNING;

		return std::shared_ptr<T>();
	} catch (...) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In sharedPtrFromString(): Error!" << std::endl
				<< "Caught unknown exception" << std::endl
		);
	}

	return gt_ptr;
}

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

namespace boost {
namespace serialization {

/******************************************************************************/
/**
 * Saves a tribool variable to an archive
 */
template<typename Archive>
void save(
	Archive &ar
	, const boost::logic::tribool &val
	, unsigned int version
) {
	Gem::Common::triboolStates tbs = Gem::Common::triboolStates::TBS_FALSE;
	if (val == true)
		tbs = Gem::Common::triboolStates::TBS_TRUE;
	else if (boost::logic::indeterminate(val))
		tbs = Gem::Common::triboolStates::TBS_INDETERMINATE;

	ar &make_nvp("tbs", tbs);
}

/******************************************************************************/
/**
 * Loads a tribool variable from an archive
 */
template<typename Archive>
void load(
	Archive &ar
	, boost::logic::tribool &val
	, unsigned int version
) {
	Gem::Common::triboolStates tbs = Gem::Common::triboolStates::TBS_FALSE;
	ar &make_nvp("tbs", tbs);

	switch (tbs) {
		case Gem::Common::triboolStates::TBS_FALSE:
			val = false;
			break;

		case Gem::Common::triboolStates::TBS_TRUE:
			val = true;
			break;

		case Gem::Common::triboolStates::TBS_INDETERMINATE:
			val = boost::logic::indeterminate;
			break;
	};
}

/******************************************************************************/
/**
 * Saves a std::chrono::duration<double> variable to an archive
 */
template<typename Archive>
void save(
	Archive &ar
	, const std::chrono::duration<double> &val
	, unsigned int version
) {
	typename std::chrono::duration<double>::rep chrono_duration = val.count();
	ar & make_nvp("chrono_duration", chrono_duration);
}

/******************************************************************************/
/**
 * Loads a tribool variable from an archive
 */
template<typename Archive>
void load(
	Archive &ar
	, std::chrono::duration<double> &val
	, unsigned int version
) {
	typename std::chrono::duration<double>::rep chrono_duration;
	ar & make_nvp("chrono_duration", chrono_duration);
	val = std::chrono::duration<double>(chrono_duration);
}

/******************************************************************************/
/**
 * Loads a time point from an archive
 */
template<typename Archive>
void save(
	Archive& ar
	, std::chrono::high_resolution_clock::time_point const& val
	, unsigned int version
) {
	std::chrono::milliseconds::rep representation = Gem::Common::time_point_to_milliseconds(val);
	ar & make_nvp("timpoint_milliseconds", representation);;
}

/******************************************************************************/
/**
 * Saves a time point to an archive
 */
template<typename Archive>
void load(
	Archive& ar
	, std::chrono::high_resolution_clock::time_point& val
	, unsigned int version
) {
	std::chrono::milliseconds::rep representation;
	ar & make_nvp("timpoint_milliseconds", representation);
	val = Gem::Common::milliseconds_to_time_point(representation);
}

/******************************************************************************/
/**
 * Serialization of std::atomic<bool>
 */
template<typename Archive>
void save(
	Archive & ar
	, const std::atomic<bool>& b
	, unsigned int version
){
	bool value = b.load();
	ar & make_nvp("bool_val", value);
}

/******************************************************************************/
/**
 * Deserialization of std::atomic<bool>
 */
template<typename Archive>
void load(
	Archive & ar
	, std::atomic<bool>& b
	, unsigned int version
){
	bool value = false;
	ar & make_nvp("bool_val", value);
	b.store(value);
}

/******************************************************************************/

} /* namespace serialization */
} /* namespace boost */

/******************************************************************************/
/**
 * Needed so Boost.Serialization does not search for a serialize function
 */
BOOST_SERIALIZATION_SPLIT_FREE(boost::logic::tribool)
BOOST_SERIALIZATION_SPLIT_FREE(std::chrono::duration<double>)
BOOST_SERIALIZATION_SPLIT_FREE(std::chrono::high_resolution_clock::time_point)
BOOST_SERIALIZATION_SPLIT_FREE(std::atomic<bool>)

/*
template<typename Archive, typename clock_type>
inline void serialize(
	Archive & ar
	, std::chrono::time_point<clock_type>& time_point
	, unsigned int version
) {
	boost::serialization::split_free(ar, time_point, version);
}
 */

/******************************************************************************/
