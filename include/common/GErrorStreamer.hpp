/**
 * @file GErrorStreamer.hpp
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

// Standard header files go here
#include <string>
#include <sstream>

// Boost header files go here

#ifndef GERRORSTREAMER_HPP_
#define GERRORSTREAMER_HPP_

// Geneva header files go here
#include "common/GLogger.hpp"

/******************************************************************************/
// Syntactic sugar
const bool DO_LOG=true;
const bool NO_LOG=false;

/******************************************************************************/
#define time_and_place \
	std::string(std::string("Recorded on ") + __DATE__ + " at " + __TIME__  + "\n" \
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
	 g_error_streamer(
		 bool do_log = NO_LOG
		 , const std::string& where_and_when = std::string()
	 )
		 : m_do_log(do_log)
	 	 , m_where_and_when(where_and_when)
	 { /* nothing */ }

	 /**************************************************************************/
	 /**
	  * The standard destructor
	  */
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
	 operator std::string() const {
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
	 bool m_do_log = false;
	 const std::string exception_file = "./GENEVA-EXCEPTION.log";
	 std::string m_where_and_when;

	 /**************************************************************************/
	 // Prevent copying and assignment
	 g_error_streamer(const g_error_streamer&) = delete;
	 g_error_streamer& operator=(g_error_streamer&) = delete;
	 g_error_streamer(const g_error_streamer&&) = delete;
	 g_error_streamer& operator=(g_error_streamer &&) = delete;

	 /**************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

#endif /* GERRORSTREAMER_HPP_ */

