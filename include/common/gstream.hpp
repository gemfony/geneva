/**
 * @file GStream.hpp
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

#ifndef GENEVA_LIBRARY_COLLECTION_GSTREAMER_HPP
#define GENEVA_LIBRARY_COLLECTION_GSTREAMER_HPP

// Geneva header files go here
#include "common/GLogger.hpp"

/******************************************************************************/
// Syntactic sugar
const bool DO_LOG=true;
const bool NO_LOG=false;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A simple wrapper for a stringstream, so we can more easily send data to a string.
 * The class may optionally duplicate data and send it to the global logger. This
 * will happen during string conversion, as it is targetted at the throwing of
 * exceptions.
 */
class gstream {
public:
	 /**************************************************************************/
	 /**
	  * The default constructor. We may optionally instruct the class to
	  * also log to the global logger during string conversion.
	  *
	  * @param do_log
	  */
	 explicit gstream(bool do_log = NO_LOG) : m_do_log(do_log)
	 { /* nothing */ }

	 /**************************************************************************/
	 /**
	  * The standard destructor
	  */
	 ~gstream() = default;

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
	 gstream& operator<<(const value_type& value) {
		 m_stream << value;
		 return *this;
	 }

	 /**************************************************************************/
	 /**
	  * Automatic conversion to a string. The function will optionally send the
	  * output to the global logger.
	  *
	  * @return A string with the content of the wrapped stringstream object.
	  */
	 operator std::string() const {
		 if(m_do_log) {
			 glogger
				 << "========================================================" << std::endl
				 << "In file " << __FILE__ << "near line " << __LINE__ << " :" << std::endl
				 << m_stream.str()
				 << "========================================================" << std::endl
				 << GLOGGING;
		 }
		 return m_stream.str();
	 }

	 /**************************************************************************/
	 /**
	  * Comply with the stringstream conventions
	  *
	  * @return The string content of the wrapped stream
	  */
	 std::string str() const {
		 return m_stream.str();
	 }

private:
	 /**************************************************************************/
	 // Data
	 std::stringstream m_stream;
	 bool m_do_log = false;

	 /**************************************************************************/
	 // Prevent copying and assignment
	 gstream(const gstream&) = delete;
	 gstream& operator=(gstream&) = delete;
	 gstream(const gstream&&) = delete;
	 gstream& operator=(gstream &&) = delete;

	 /**************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

#endif /* GENEVA_LIBRARY_COLLECTION_GSTREAMER_HPP */

