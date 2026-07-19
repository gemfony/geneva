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

#include "common/GGlobalDefines.hpp"

#include <cstddef>
#include <map>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>

namespace Gem::Common::Concurrency {

/******************************************************************************/
/**
 * The process-wide compute-device registry: tracks which sources currently hold
 * which device, by plain integer device id (no CUDA types -- both the courtier
 * GPU consumer and Hap's RNG fill backend depend only on common). Threads are
 * not the only resource that oversubscribes: on a GPU run the RNG refill
 * (cuRAND) and the evaluation kernel would double-book the same device. This
 * registry makes that contention observable and lets the RNG fill yield the
 * device to the evaluation kernel (the policy lives at the call sites; the
 * registry only counts).
 *
 * Like the thread budget it does accounting, not scheduling: acquire() never
 * blocks, never throws and never refuses. Thread-safe; short mutex-protected
 * critical sections, and the read side (users()) is cheap enough to consult
 * once per bulk refill.
 */
class GDeviceRegistry {
public:
    /***************************************************************************/
    /**
     * An RAII handle for one device use: registers the holder for its lifetime
     * and deregisters on destruction (or an early release()). Move-only; a
     * default-constructed handle is empty.
     */
    class DeviceUse {
        friend class GDeviceRegistry;

    public:
        /** @brief Default constructor: an empty (no-op) handle */
        DeviceUse() = default;

        /** @brief Move constructor: takes over the other handle's registration
         *  @param other The handle to take the registration from (left empty) */
        DeviceUse(DeviceUse &&other) noexcept
          : registry_(std::exchange(other.registry_, nullptr))
          , key_(std::exchange(other.key_, 0)) { /* nothing */ }

        /** @brief Move assignment: releases any held registration, then takes over the other's
         *  @param other The handle to take the registration from (left empty)
         *  @return A reference to this handle */
        DeviceUse &operator=(DeviceUse &&other) noexcept {
            if(this != &other) {
                release();
                registry_ = std::exchange(other.registry_, nullptr);
                key_ = std::exchange(other.key_, 0);
            }
            return *this;
        }

        /** @brief The destructor deregisters the device use */
        ~DeviceUse() { release(); }

        DeviceUse(const DeviceUse &) = delete;
        DeviceUse &operator=(const DeviceUse &) = delete;

        /** @brief Deregisters the device use early; idempotent */
        void release() noexcept {
            if(registry_ != nullptr) {
                registry_->release_(key_);
                registry_ = nullptr;
            }
        }

    private:
        /** @brief Constructs a live handle (used by GDeviceRegistry::acquire())
         *  @param registry The owning registry
         *  @param key The registry key of this device use */
        DeviceUse(GDeviceRegistry *registry, std::size_t key)
          : registry_(registry)
          , key_(key) { /* nothing */ }

        GDeviceRegistry *registry_ = nullptr; ///< The owning registry (nullptr == empty handle)
        std::size_t key_ = 0;                 ///< This use's registry key
    };

    /***************************************************************************/
    /** @brief Registers a named source as a holder of the given device for the handle's lifetime.
     *  Advisory: never blocks, never throws, never refuses.
     *
     *  @param source A short name identifying the holder (e.g. "consumer:gpu")
     *  @param device_id The device being used (a plain integer id; 0 is the default device)
     *  @return An RAII handle deregistering the use on destruction */
    [[nodiscard]] DeviceUse acquire(std::string_view source, int device_id = 0) {
        std::scoped_lock const lk(mutex_);
        const std::size_t key = next_key_++;
        uses_.emplace(key, use{std::string(source), device_id});
        ++users_[device_id];
        return DeviceUse{this, key};
    }

    /** @brief How many sources currently hold the given device
     *  @param device_id The device queried (0 is the default device)
     *  @return The number of live registrations for the device */
    [[nodiscard]] unsigned int users(int device_id = 0) const noexcept {
        std::scoped_lock const lk(mutex_);
        const auto it = users_.find(device_id);
        return it != users_.end() ? it->second : 0u;
    }

    /** @brief Whether more than one source holds the given device
     *  @param device_id The device queried (0 is the default device)
     *  @return true if at least two sources are registered for the device */
    [[nodiscard]] bool contended(int device_id = 0) const noexcept { return users(device_id) > 1; }

private:
    /***************************************************************************/
    /** @brief One live device use's bookkeeping record */
    struct use {
        std::string source; ///< The holder's name
        int device_id = 0;  ///< The device held
    };

    /** @brief Deregisters a device use (called by DeviceUse only)
     *  @param key The registry key handed out by acquire() */
    void release_(std::size_t key) noexcept {
        std::scoped_lock const lk(mutex_);
        if(const auto it = uses_.find(key); it != uses_.end()) {
            if(const auto uit = users_.find(it->second.device_id); uit != users_.end()) {
                if(--uit->second == 0) {
                    users_.erase(uit);
                }
            }
            uses_.erase(it);
        }
    }

    /***************************************************************************/
    mutable std::mutex mutex_;             ///< Guards all bookkeeping below
    std::map<std::size_t, use> uses_;      ///< Live device uses by key
    std::map<int, unsigned int> users_;    ///< Live holder count per device id
    std::size_t next_key_ = 1;             ///< The next registry key to hand out
};

/******************************************************************************/
/**
 * @brief The process-global device registry
 *
 * Never destroyed (reclaimed only by the OS at process exit), for the same
 * static-destruction-order reasoning as the thread budget: a handle held by a
 * static-lifetime object must find a live registry whenever it releases.
 *
 * @return A reference to the one process-wide device registry
 */
inline GDeviceRegistry &deviceRegistry() {
    static GDeviceRegistry *registry = new GDeviceRegistry(); // NOLINT(cppcoreguidelines-owning-memory) -- deliberate never-destroy
    return *registry;
}

/******************************************************************************/

} /* namespace Gem::Common::Concurrency */
