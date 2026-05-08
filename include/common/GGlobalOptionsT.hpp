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
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <map>
#include <mutex>
#include <string>
#include <vector>

// Boost headers go here

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"

namespace Gem::Common {

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
template <typename T>
class GGlobalOptionsT { // NOLINT(cppcoreguidelines-special-member-functions)
public:
    /***************************************************************************/
    // Defaulted or deleted constructors, destructor and assignment operators
    // Rule of five

    GGlobalOptionsT() = default;

    GGlobalOptionsT(GGlobalOptionsT<T> const &) = delete;
    GGlobalOptionsT(GGlobalOptionsT<T> &&) = delete;

    GGlobalOptionsT<T> &operator=(GGlobalOptionsT<T> const &) = delete;
    GGlobalOptionsT<T> &operator=(GGlobalOptionsT<T> &&) = delete;

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
        std::scoped_lock guard(mutex_);
        if(auto it = kvp_.find(key); it != kvp_.end()) {
            value = it->second;
            return true;
        }
        return false;
    }

    /***************************************************************************/
    /**
	 * Retrieves an option from the map, returning it as the function result.
	 * Note that this function does not check for availability of the option.
	 */
    T get(const std::string &key) {
        std::scoped_lock guard(mutex_);
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
        std::scoped_lock guard(mutex_);
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
        std::scoped_lock guard(mutex_);
        if(kvp_.contains(key)) {
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
        std::scoped_lock guard(mutex_);
        if(auto it = kvp_.find(key); it != kvp_.end()) {
            kvp_.erase(it);
            return true;
        }
        return false;
    }

    /************************************************************************/
    /**
	 * Allows to check whether an option with a given name is available
	 *
	 * @param key The name of the option that should be checked for existence
	 * @return A boolean that indicates whether a given option is available
	 */
    bool exists(const std::string &key) const {
        std::scoped_lock guard(mutex_);
        return kvp_.contains(key);
    }

    /************************************************************************/
    /**
	 * Allows to find out the number of registered options
	 */
    std::size_t size() const {
        std::scoped_lock guard(mutex_);
        return kvp_.size();
    }

    /************************************************************************/
    /**
	 * Allows to check whether any options are present
	 */
    bool empty() const {
        std::scoped_lock guard(mutex_);
        return kvp_.empty();
    }

    /************************************************************************/
    /**
	 * Retrieves a full list of all keys
	 */
    std::string getKeyDescription() const {
        std::scoped_lock guard(mutex_);
        std::string result; // NOLINT(cppcoreguidelines-init-variables)
        typename std::map<std::string, T>::const_iterator cit;
        std::size_t count = 0;
        for(cit = kvp_.begin(); cit != kvp_.end(); ++cit) {
            result += cit->first;
            if(++count != kvp_.size()) {
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
        std::scoped_lock guard(mutex_);
        typename std::map<std::string, T>::const_iterator cit;
        for(cit = kvp_.begin(); cit != kvp_.end(); ++cit) {
            keys.push_back(cit->first);
        }
    }

    /************************************************************************/
    /**
	 * Retrieves a vector of all content items
	 */
    void getContentVector(std::vector<T> &content) const {
        std::scoped_lock guard(mutex_);
        content.clear();
        typename std::map<std::string, T>::const_iterator cit;
        for(cit = kvp_.begin(); cit != kvp_.end(); ++cit) {
            content.push_back(cit->second);
        }
    }

    /************************************************************************/
    /**
	 * Positions an internal iterator at the beginning of the map
	 */
    void rewind() {
        std::scoped_lock guard(mutex_);
        pos_ = kvp_.begin();
    }

    /************************************************************************/
    /**
	 * Switches to the next position or returns false, if this is not possible
	 */
    bool goToNextPosition() {
        std::scoped_lock guard(mutex_);
        if(pos_ == kvp_.end()) {
            return false;
        }
        ++pos_;
        return pos_ != kvp_.end();
    }

    /************************************************************************/
    /**
	 * Retrieves the item at the current position
	 */
    T getCurrentItem() {
        std::scoped_lock guard(mutex_);
        if(pos_ == kvp_.end()) {
            glogger << "In GGlobalOptionsT<T>::getCurrentItem(): Warning!\n"
                    << "Iterator is at end of map. Returning default-constructed value.\n"
                    << GWARNING;
            return T{};
        }
        return pos_->second;
    }

    /************************************************************************/
    /**
	 * Retrieves the next item (thereby incrementing the position iterator)
	 * or returns false, if the end of the map has been reached. Note that it
	 * is up to you to rewind the position iterator using the rewind function.
	 */
    bool getNextItem(T &item) {
        std::scoped_lock guard(mutex_);
        if(pos_ == kvp_.end()) {
            return false;
        }
        ++pos_;
        if(pos_ != kvp_.end()) {
            item = pos_->second;
            return true;
        }
        return false;
    }

private:
    /************************************************************************/
    // Holds the actual data
    std::map<std::string, T> kvp_{};

    typename std::map<std::string, T>::iterator pos_ = kvp_.begin();
    mutable std::mutex mutex_; ///< Lock get/set operations
};

/******************************************************************************/

} /* namespace Gem::Common */
