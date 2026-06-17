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
#include "common/GSingletonT.hpp" // for the gsingleton_never_destroy opt-in below

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
 *
 * @tparam T The type of the option values stored in this option store
 */
template <typename T>
class GGlobalOptionsT { // NOLINT(cppcoreguidelines-special-member-functions)
public:
    /***************************************************************************/
    // Public value-type alias (API hygiene; lets callers spell out the stored
    // type without restating the template argument).
    using value_type = T;

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
	 * @brief Retrieves the value of an option from the map, storing it in
	 * an argument.
	 *
	 * @param key The name of the option that should be retrieved
	 * @param value An output parameter that receives the option's value if the key exists
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
	 * @brief Retrieves an option from the map, returning it as the function result.
	 *
	 * Throws @c geneva_exception when the key does not exist (previously, the
	 * function used @c map::operator[] which silently inserted a default-
	 * constructed value — surprising for a read-only accessor and a source of
	 * silent map growth / null-deref bugs in callers that forgot to call
	 * @c exists() first).
	 *
	 * @param key The name of the option that should be retrieved
	 * @return The value associated with the given key
	 * @throw geneva_exception if the key is not present in the option map
	 */
    T get(const std::string &key) {
        std::scoped_lock guard(mutex_);
        if(auto it = kvp_.find(key); it != kvp_.end()) {
            return it->second;
        }
        raiseException(
            "In GGlobalOptionsT::get(\"" << key << "\"): Error!" << '\n'
                << "Key is not present in the global options map." << '\n'
                << "Use exists(key) to check before calling, or get(key, value) "
                   "which signals absence via its return value." << '\n'
        );
    }

    /***************************************************************************/
    /**
	 * @brief Sets a new option or changes an existing option
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
	 * @brief Sets a new option once or returns false, if the option already exists
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
	 * @brief Removes an option from the map, if available
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
	 * @brief Allows to check whether an option with a given name is available
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
	 * @brief Allows to find out the number of registered options
	 *
	 * @return The number of options currently held in the option store
	 */
    std::size_t size() const {
        std::scoped_lock guard(mutex_);
        return kvp_.size();
    }

    /************************************************************************/
    /**
	 * @brief Allows to check whether any options are present
	 *
	 * @return A boolean indicating whether the option store holds no options
	 */
    bool empty() const {
        std::scoped_lock guard(mutex_);
        return kvp_.empty();
    }

    /************************************************************************/
    /**
	 * @brief Retrieves a full list of all keys
	 *
	 * @return A single string holding all option keys, separated by ", "
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
	 * @brief Retrieves a vector of all keys.
	 *
	 * The caller's vector is cleared and
	 * repopulated under a single lock acquisition (the previous version
	 * cleared the caller's vector *before* taking the lock, which is a data
	 * race if the caller shares the vector across threads).
	 *
	 * @param keys An output vector that is cleared and then filled with all option keys
	 */
    void getKeyVector(std::vector<std::string> &keys) const {
        std::scoped_lock guard(mutex_);
        keys.clear();
        keys.reserve(kvp_.size());
        for(auto const &[key, _] : kvp_) {
            keys.push_back(key);
        }
    }

    /************************************************************************/
    /**
	 * @brief Retrieves a vector of all content items
	 *
	 * @param content An output vector that is cleared and then filled with all stored option values
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
	 * @brief Returns a fresh vector containing all stored values, captured under a
	 * single mutex acquisition.
	 *
	 * Prefer this over keys-then-get loops at the
	 * call site: it is both atomic (no race window between key snapshot and
	 * value lookup) and cheaper (one lock + one traversal instead of N+1
	 * locks + N find()s).
	 *
	 * @return A vector holding a snapshot of all stored option values
	 */
    [[nodiscard]] std::vector<T> getContentSnapshot() const {
        std::scoped_lock guard(mutex_);
        std::vector<T> result;
        result.reserve(kvp_.size());
        for(auto const &[_, v] : kvp_) {
            result.push_back(v);
        }
        return result;
    }

    // ----------------------------------------------------------------------
    // Removed stateful-iterator API (rewind / goToNextPosition /
    // getCurrentItem / getNextItem): the internal iterator `pos_` was
    // invalidated by remove() and leaked across calls, producing a latent
    // UB / silent-map-growth hazard whenever traversal was interleaved with
    // mutation. Callers should now take a snapshot via getKeyVector() or
    // getContentVector() and iterate that.

private:
    /************************************************************************/
    // Holds the actual data
    std::map<std::string, T> kvp_{};
    mutable std::mutex mutex_; ///< Lock get/set operations
};

/******************************************************************************/
/**
 * The global option stores hold plain configuration data (a map guarded by a
 * mutex) with no destructor side effects, yet they are queried from arbitrary
 * places — including during shutdown. Opt every GGlobalOptionsT<T> singleton
 * into never-destroy semantics so it outlives other statics and avoids any
 * destruction-order hazard. Reclaimed by the OS at process exit.
 *
 * @tparam T The option value type carried by the GGlobalOptionsT singleton
 */
template <typename T>
struct gsingleton_never_destroy<GGlobalOptionsT<T>> : std::true_type {};

/******************************************************************************/

} /* namespace Gem::Common */
