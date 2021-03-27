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
#include <map>
#include <utility>
#include <mutex>

// Boost headers go here

// Geneva headers go here

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * This class provides access to global class objects of user-defined type.
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
class GGlobalStoreT
{
public:
	/***************************************************************************/
	// Defaulted or deleted constructors, destructor and assignment operators
	// Rule of five

    GGlobalStoreT() = default;

    GGlobalStoreT( GGlobalStoreT<T> const&) = delete;
    GGlobalStoreT( GGlobalStoreT<T> &&) = delete;

    GGlobalStoreT<T>& operator=( GGlobalStoreT<T> const&) = delete;
    GGlobalStoreT<T>& operator=( GGlobalStoreT<T> &&) = delete;

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
		std::unique_lock<std::mutex> guard(m_mutex);

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
		std::unique_lock<std::mutex> guard(m_mutex);
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
		std::unique_lock<std::mutex> guard(m_mutex);
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
		std::unique_lock<std::mutex> guard(m_mutex);
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
		std::unique_lock<std::mutex> guard(m_mutex);
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
		std::unique_lock<std::mutex> guard(m_mutex);
		return kvp_.find(key) != kvp_.end();
	}

	/************************************************************************/
	/**
	 * Allows to find out the number of registered options
	 */
	std::size_t size() const {
		std::unique_lock<std::mutex> guard(m_mutex);
		return kvp_.size();
	}

	/************************************************************************/
	/**
	 * Allows to check whether any options are present
	 */
	bool empty() const {
		std::unique_lock<std::mutex> guard(m_mutex);
		return kvp_.empty();
	}

	/************************************************************************/
	/**
	 * Retrieves a full list of all keys
	 */
	std::string getKeyDescription() const {
		std::unique_lock<std::mutex> guard(m_mutex);
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
		std::unique_lock<std::mutex> guard(m_mutex);
		typename std::map<std::string, T>::const_iterator cit;
		for (cit = kvp_.begin(); cit != kvp_.end(); ++cit) {
			keys.push_back(cit->first);
		}
	}

	/************************************************************************/
	/**
	 * Retrieves a vector of all content items
	 */
	void getContentVector(std::vector<T> & content) const {
		std::unique_lock<std::mutex> guard(m_mutex);
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
		std::unique_lock<std::mutex> guard(m_mutex);
		pos = kvp_.begin();
	}

	/************************************************************************/
	/**
	 * Switches to the next position or returns false, if this is not possible
	 */
	bool goToNextPosition() {
		std::unique_lock<std::mutex> guard(m_mutex);
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
		std::unique_lock<std::mutex> guard(m_mutex);
		return pos->second;
	}

	/************************************************************************/
	/**
	 * Retrieves the next item (thereby incrementing the position iterator)
	 * or returns false, if the end of the map has been reached. Note that it
	 * is up to you to rewind the position iterator using the rewind function.
	 */
	bool getNextItem(T &item) {
		std::unique_lock<std::mutex> guard(m_mutex);
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
	std::map<std::string, T> kvp_{};

	typename std::map<std::string, T>::iterator pos = kvp_.begin();
	mutable std::mutex m_mutex; ///< Lock get/set operations
};

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */
