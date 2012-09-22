/**
 * @file GGlobalOptionsT.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

// Standard headers go here
#include <map>
#include <utility>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

/**
 * Check that we have support for threads. This collection of classes is useless
 * without this.
 */
#ifndef BOOST_HAS_THREADS
#error "Error: No support for Boost.threads available."
#endif

#ifndef GGLOBALOPTIONST_HPP_
#define GGLOBALOPTIONST_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "GSingletonT.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * This class provides access to global options of user-defined type.
 * Note that these are not serialized, so you need to take care yourself
 * that these are available on remote systems. An easy way is to instantiate
 * both the client and the server from the same main function and to fill
 * the global object before both are started. A command line option can
 * then steer whether the program acts as a server or client, and both
 * will have the same options.
 *
 * NOTE: THIS CLASS IS STILL ENTIRELY UNTESTED. USE WITH CARE AND REPORT ANY
 * ERROR YOU FIND.
 */
template <typename T>
class GGlobalOptionsT {
public:
	/***********************************************************************/
	/**
	 * Retrieves the value of an option from the map, storing it in
	 * an argument.
	 *
	 * @param key The name of the option that should be retrieved
	 * @param value The value that should be retrieved
	 * @return A boolean indicating whether retrieval of the option was successful
	 */
	bool getOption(const std::string& key, T& value) const {
		if(kvp_.find(key) != kvp_.end()) {
			value = kvp_[key];
			return true;
		}
		else
			return false;
	}

	/***********************************************************************/
	/**
	 * Retrieves an option from the map, returning it as the function result.
	 * Note that this function does not check for availability of the option.
	 */
	T getOption(const std::string& key) {
		return kvp_[key];
	}

	/***********************************************************************/
	/**
	 * Sets a new option or changes an existing option
	 *
	 * @param key The name of the option
	 * @param value The value of the option
	 */
	void setOption(const std::string& key, const T& value) {
		kvp_[key] = value;
	}
	/************************************************************************/
	/**
	 * Removes an option from the map, if available
	 *
	 * @param key The name of the option that should be removed
	 * @return A boolean indicating whether the option was indeed available
	 */
	bool removeOption(const std::string& key) {
		typename std::map<std::string, T>::iterator it = kvp_.end();
		if(it == kvp_.end()) return false;
		else {
			kvp_.erase(it);
			return true;
		}
	}

	/************************************************************************/
	/**
	 * Allows to check whether an option with a given name is available
	 *
	 * @param key The name of the option that should be checked for existence
	 * @return A boolean that indicates whether a given option is available
	 */
	bool hasOption(const std::string& key) const {
		if(kvp_.find(key) != kvp_.end()) return true;
		else return false;
	}

	/************************************************************************/

private:
	std::map<std::string, T> kvp_;
};

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

// Specialization for string options
typedef Gem::Common::GSingletonT<Gem::Common::GGlobalOptionsT<std::string> > GStringOptions;
#define GSTRINGOPTIONS GStringOptions::Instance(0)

#endif /* GGLOBALOPTIONST_HPP_ */
