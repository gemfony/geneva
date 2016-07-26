/**
 * @file GGlobalOptionsT.hpp
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
#include <map>
#include <utility>
#include <mutex>

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

// Geneva headers go here

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
 * will have the same options. NOTE: This class uses locking internally
 * to make it thread-safe. It thus assumes occasional accesses and is not
 * suited well for frequent querying.
 */
template<typename T>
class GGlobalOptionsT {
public:
	/***************************************************************************/
	/**
	 * The default constructor
	 */
	GGlobalOptionsT()
		: kvp_(), pos(kvp_.begin()) { /* nothing */ }

	/***************************************************************************/
	/**
	 * Retrieves the value of an option from the map, storing it in
	 * an argument.
	 *
	 * @param key The name of the option that should be retrieved
	 * @param value The value that should be retrieved
	 * @return A boolean indicating whether retrieval of the option was successful
	 */
	bool get(const std::string &key, T &value) {
		std::unique_lock<std::mutex> guard(m_);

		if (kvp_.find(key) != kvp_.end()) {
			value = kvp_[key];
			return true;
		} else {
			return false;
		}
	}

	/***************************************************************************/
	/**
	 * Retrieves an option from the map, returning it as the function result.
	 * Note that this function does not check for availability of the option.
	 */
	T get(const std::string &key) {
		std::unique_lock<std::mutex> guard(m_);
		return kvp_[key];
	}

	/***************************************************************************/
	/**
	 * Sets a new option or changes an existing option
	 *
	 * @param key The name of the option
	 * @param value The value of the option
	 */
	void set(const std::string &key, T value) {
		std::unique_lock<std::mutex> guard(m_);
		kvp_[key] = value;
	}

	/***************************************************************************/
	/**
	 * Sets a new option once or returns false, if the option already exists
	 *
	 * @param key The name of the option
    * @param value The value of the option
	 * @return A boolean indicating whether creation of the new option was successful
	 */
	bool setOnce(const std::string &key, T value) {
		std::unique_lock<std::mutex> guard(m_);
		if (kvp_.find(key) != kvp_.end()) {
			return false;
		}
		kvp_[key] = value;
		return true;
	}

	/***************************************************************************/
	/**
	 * Removes an option from the map, if available
	 *
	 * @param key The name of the option that should be removed
	 * @return A boolean indicating whether the option was indeed available
	 */
	bool remove(const std::string &key) {
		std::unique_lock<std::mutex> guard(m_);
		typename std::map<std::string, T>::iterator it = kvp_.end();
		if (it == kvp_.end()) {
			return false;
		} else {
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
	bool exists(const std::string &key) const {
		std::unique_lock<std::mutex> guard(m_);
		return (kvp_.find(key) != kvp_.end() ? true : false);
	}

	/************************************************************************/
	/**
	 * Allows to find out the number of registered options
	 */
	std::size_t size() const {
		std::unique_lock<std::mutex> guard(m_);
		return kvp_.size();
	}

	/************************************************************************/
	/**
	 * Allows to check whether any options are present
	 */
	bool empty() const {
		std::unique_lock<std::mutex> guard(m_);
		return kvp_.empty();
	}

	/************************************************************************/
	/**
	 * Retrieves a full list of all keys
	 */
	std::string getKeyDescription() const {
		std::unique_lock<std::mutex> guard(m_);
		std::string result;
		typename std::map<std::string, T>::const_iterator cit;
		std::size_t pos = 0;
		for (cit = kvp_.begin(); cit != kvp_.end(); ++cit) {
			result += cit->first;
			if (++pos != kvp_.size()) {
				result += ", ";
			}
		}
		return result;
	}

	/************************************************************************/
	/**
	 * Retrieves a vector of all keys
	 */
	void getKeyVector(std::vector<std::string> &keys) const {
		keys.clear(); // Make sure the vector is empty
		std::unique_lock<std::mutex> guard(m_);
		typename std::map<std::string, T>::const_iterator cit;
		for (cit = kvp_.begin(); cit != kvp_.end(); ++cit) {
			keys.push_back(cit->first);
		}
	}

	/************************************************************************/
	/**
	 * Retrieves a vector of all content items
	 */
	void getContentVector(std::vector<T> content) const {
		std::unique_lock<std::mutex> guard(m_);
		content.clear();
		typename std::map<std::string, T>::const_iterator cit;
		for (cit = kvp_.begin(); cit != kvp_.end(); ++cit) {
			content.push_back(cit->second);
		}
	}

	/************************************************************************/
	/**
	 * Positions an internal iterator at the beginning of the map
	 */
	void rewind() {
		std::unique_lock<std::mutex> guard(m_);
		pos = kvp_.begin();
	}

	/************************************************************************/
	/**
	 * Switches to the next position or returns false, if this is not possible
	 */
	bool goToNextPosition() {
		std::unique_lock<std::mutex> guard(m_);
		if (++pos != kvp_.end()) {
			return true;
		} else {
			return false;
		}
	}

	/************************************************************************/
	/**
	 * Retrieves the item at the current position
	 */
	T getCurrentItem() {
		std::unique_lock<std::mutex> guard(m_);
		return pos->second;
	}

	/************************************************************************/
	/**
	 * Retrieves the next item (thereby incrementing the position iterator)
	 * or returns false, if the end of the map has been reached. Note that it
	 * is up to you to rewind the position iterator using the rewind function.
	 */
	bool getNextItem(T &item) {
		std::unique_lock<std::mutex> guard(m_);
		if (++pos != kvp_.end()) {
			item = pos->second;
			return true;
		} else {
			return false;
		}
	}

private:
	/************************************************************************/
	// Holds the actual data
	std::map<std::string, T> kvp_;

	typename std::map<std::string, T>::iterator pos;
	mutable std::mutex m_; ///< Lock get/set operations
};

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GGLOBALOPTIONST_HPP_ */
