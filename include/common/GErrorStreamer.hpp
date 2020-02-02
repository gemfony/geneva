/**
 * @file
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

// Standard header files go here
#include <string>
#include <sstream>

// Boost header files go here

// Geneva header files go here
#include "common/GLogger.hpp"
#include "common/GCommonHelperFunctions.hpp"

/******************************************************************************/
// Syntactic sugar
const bool DO_LOG=true;
const bool NO_LOG=false;

/******************************************************************************/

#define time_and_place \
	std::string(std::string("Recorded on ") + Gem::Common::currentTimeAsString()  + "\n" \
	+ "in File " + __FILE__ + " at line " + std::to_string(__LINE__) + " :\n")

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A simple wrapper for a string_error_streamer, so we can more easily send data to a string
 * when throwing an exception. The class may optionally duplicate data and send it
 * to the global logger. This will happen during string conversion, so we may simply
 * construct a g_error_streamer object inside of a throw()-call.
 */
class g_error_streamer {
public:
	 /**************************************************************************/
	 /**
	  * The default constructor. We may optionally instruct the class to
	  * also log to the global logger during string conversion.
	  *
	  * @param do_log Instructs the object to also send data to the logger
	  */
	 explicit g_error_streamer(
		 bool do_log
		 , std::string where_and_when
	 )
		 : m_do_log(do_log)
	 	 , m_where_and_when(std::move(where_and_when))
	 { /* nothing */ }

	/*************************************************************************/
	// Defaulted or deleted constructors, destructor and assignment operators

	g_error_streamer() = default;

	g_error_streamer(const g_error_streamer&) = delete;
	g_error_streamer& operator=(g_error_streamer&) = delete;
	g_error_streamer(const g_error_streamer&&) = delete;
	g_error_streamer& operator=(g_error_streamer &&) = delete;

	~g_error_streamer() = default;

	 /**************************************************************************/
	 /**
	  * This function allows us to stream virtually any type of streamable data
	  * to this class.
	  *
	  * @tparam value_type The parameter type of a value streamed into the class
	  * @param value The value streamed into this class
	  * @return A pointer to this object
	  */
	 template <typename value_type>
	 g_error_streamer& operator<<(const value_type& val) {
		 m_ostream << val;
		 return *this;
	 }

	 /******************************************************************************/
	 /**
	  * Needed for stringstream
	  */
	 g_error_streamer& operator<<(std::ostream &( *val )(std::ostream &)) {
		 m_ostream << val;
		 return *this;
	 }

	 /******************************************************************************/
	 /**
	  * Needed for stringstream
	  */
	 g_error_streamer& operator<<(std::ios &( *val )(std::ios &)) {
		 m_ostream << val;
		 return *this;
	 }

	 /******************************************************************************/
	 /**
	  *  Needed for stringstream
	  */
	 g_error_streamer& operator<<(std::ios_base &( *val )(std::ios_base &)) {
		 m_ostream << val;
		 return *this;
	 }

	 /**************************************************************************/
	 /**
	  * Automatic conversion to a string. The function will optionally send the
	  * output to the global logger.
	  *
	  * @return A string with the content of the wrapped string_error_streamer object.
	  */
	 operator std::string() const { // NOLINT
		 using namespace Gem::Common;
		 if(m_do_log) {
			 glogger(boost::filesystem::path(exception_file))
				 << "========================================================" << std::endl
				 << "Error!" << std::endl
				 << std::endl
				 << m_where_and_when
				 << std::endl
				 << m_ostream.str() << std::endl
				 << std::endl
				 << "If you suspect that there is an underlying problem with the" << std::endl
				 << "Gemfony library collection, then please consider filing a bug via" << std::endl
				 << "http://www.gemfony.eu (link \"Bug Reports\") or" << std::endl
				 << "through http://www.launchpad.net/geneva" << std::endl
				 << std::endl
				 << "We appreciate your help!" << std::endl
				 << "The Geneva team" << std::endl
				 << std::endl
				 << "========================================================" << std::endl
				 << GFILE;
		 }
		 return m_ostream.str();
	 }

private:
	 /**************************************************************************/
	 // Data
	 std::ostringstream m_ostream;
	 bool m_do_log = NO_LOG;
	 const std::string exception_file = "./GENEVA-EXCEPTION.log";
	 std::string m_where_and_when{};

	 /**************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

