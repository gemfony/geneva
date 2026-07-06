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

// Standard header files go here
#include <atomic>
#include <mutex>
#include <utility>

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"

namespace Gem::Geneva::Genome {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief A load-once, thereafter-immutable store for a problem's hardware-independent constant data.
 *
 * This is the metadata-boundary facility ((c) in the evaluator-unification plan): a runtime-loadable
 * individual module holds its hardware-independent problem constants -- a neural-network training set, a
 * Mona-Lisa target image, a list of parabola minima -- in one static instance of this store. The store is
 * filled EXACTLY ONCE from the factory's @c init_() hook (which @c Gem::Common::GFactoryT::globalInit()
 * already runs under its @c init_mutex_, so the fill is serialised process-wide), and is thereafter
 * immutable. Immutability is what lets both readers reach it with no locking: the free evaluator reads it in
 * its body, and -- when a device runs -- the consumer's problem-specific plug packs it for the device, from
 * the same single source (retiring today's duplicate load between the CPU path and the GPU marshaller).
 *
 * The single writer / many concurrent readers pattern is exactly the one the shared @c GGenomeLayout already
 * uses (built once, then read lock-free by the whole population). @c ensureLoaded() is additionally guarded
 * by a @c std::once_flag so the facility is self-contained and safe even if a module ever fills it outside
 * the factory's serialised @c init_() path; reads after loading touch only the immutable payload and an
 * acquire-load of the ready flag, so they never contend.
 *
 * @tparam DataType The (copyable or movable) payload type holding the problem constants
 */
template <typename DataType>
class GProblemStoreT {
public:
    /***************************************************************************/
    /** @brief The default constructor: an empty, not-yet-loaded store. */
    GProblemStoreT() = default;

    /**
     * @brief Fills the store exactly once, on the first call, by invoking @p fill; later calls are no-ops.
     *
     * The producing function is called at most once for the process, under a @c std::once_flag, so
     * concurrent first calls are serialised and only one wins. Intended to be invoked from the individual
     * factory's @c init_() hook (already serialised by @c globalInit()), but safe to call from anywhere.
     *
     * @tparam FillFn A callable returning a @c DataType (the constructed problem constants)
     * @param fill The producer invoked (once) to build the payload, e.g. a lambda that reads a data file
     */
    template <typename FillFn>
    void ensureLoaded(FillFn &&fill) {
        std::call_once(once_, [&]() {
            data_ = std::forward<FillFn>(fill)();
            loaded_.store(true, std::memory_order_release);
        });
    }

    /***************************************************************************/
    /** @brief @return true once the store has been filled (acquire-load of the ready flag) */
    [[nodiscard]] bool loaded() const noexcept { return loaded_.load(std::memory_order_acquire); }

    /***************************************************************************/
    /**
     * @brief Reads the immutable payload (lock-free once loaded).
     *
     * @return A const reference to the stored problem constants
     * @throws geneva_exception if the store has not been loaded yet (a programming error: the factory's
     *         @c init_() must fill the store before any evaluation reads it)
     */
    [[nodiscard]] const DataType &get() const {
        if(not this->loaded()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GProblemStoreT::get(): Error!" << '\n'
                << "The problem store was read before it was loaded. Fill it once from the individual" << '\n'
                << "factory's init_() hook (via ensureLoaded()) before any evaluation runs." << '\n'
            );
        }
        return data_;
    }

private:
    /***************************************************************************/
    // Data

    DataType data_{};                  ///< The immutable-after-load problem constants
    std::once_flag once_;              ///< Serialises the single fill (self-contained safety)
    std::atomic<bool> loaded_{false};  ///< Ready flag; readers acquire-load it before touching data_
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Genome */
