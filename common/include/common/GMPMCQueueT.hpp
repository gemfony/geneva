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
#include <chrono>
#include <concepts>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>

// Geneva headers go here
#include "common/GBlockingMPMCQueueT.hpp"
#include "common/GCommonEnums.hpp" // for DEFAULTBUFFERSIZE
#include "common/GPreallocatedMPMCQueueT.hpp"
#include "common/GQueueCommon.hpp"

namespace Gem::Common {

/******************************************************************************/
/**
 * A thin facade over the interchangeable MPMC-queue backends. It exposes exactly the
 * GBlockingMPMCQueueT interface (the MPMCQueue concept) and forwards every call to the backend
 * chosen at compile time by the @p Backend template argument:
 *
 *   - QueueBackend::Deque        -> GBlockingMPMCQueueT<T, Cap>        (the default; supports the
 *                                   unbounded case Cap == 0). Default keeps existing behaviour.
 *   - QueueBackend::Preallocated -> GPreallocatedMPMCQueueT<T, Cap>    (preallocated ring; Cap > 0).
 *
 * The facade COMPOSES its backend rather than inheriting it, because GBlockingMPMCQueueT is `final`
 * and must not be modified. The forwarders are trivial and fully inlined, so there is no runtime
 * overhead. A static_assert guarantees the selected backend really satisfies the shared contract, so
 * the two backends can never drift apart.
 *
 * Use this type wherever a queue might benefit from switching backends at build time (e.g. a hot,
 * bounded buffer) -- callers depend only on the stable facade, not on a concrete backend.
 */
template <typename T, std::size_t Cap = DEFAULTBUFFERSIZE, QueueBackend Backend = QueueBackend::Deque>
class GMPMCQueueT final {
    static_assert(
        Backend != QueueBackend::Preallocated || Cap > 0,
        "GMPMCQueueT: the preallocated backend cannot be unbounded (Cap must be > 0); "
        "use the Deque backend for an unbounded queue"
    );

    using impl_type = std::conditional_t<
        Backend == QueueBackend::Deque,
        GBlockingMPMCQueueT<T, Cap>,
        GPreallocatedMPMCQueueT<T, Cap>
    >;

    static_assert(
        MPMCQueue<impl_type, T>,
        "GMPMCQueueT: the selected backend does not satisfy the MPMCQueue interface contract"
    );

public:
    using value_type = T;

    /*************************************************************************/
    GMPMCQueueT() = default;

    // Owns the backend (a mutex, condition variables, storage): neither copyable nor movable.
    GMPMCQueueT(GMPMCQueueT const &) = delete;
    GMPMCQueueT &operator=(GMPMCQueueT const &) = delete;
    GMPMCQueueT(GMPMCQueueT &&) = delete;
    GMPMCQueueT &operator=(GMPMCQueueT &&) = delete;

    /*************************************************************************/
    /** @brief Which backend this instance uses. Compile-time. */
    [[nodiscard]] static constexpr QueueBackend backend() noexcept { return Backend; }

    /*************************************************************************/
    // --- producer side (forwarded verbatim) ---

    template <typename U>
        requires std::constructible_from<T, U &&>
    [[nodiscard]] bool try_push(U &&item) { return impl_.try_push(std::forward<U>(item)); }

    template <typename U>
        requires std::constructible_from<T, U &&>
    [[nodiscard]] bool push(U &&item) { return impl_.push(std::forward<U>(item)); }

    template <typename U, typename Rep, typename Period>
        requires std::constructible_from<T, U &&>
    [[nodiscard]] bool push_wait(U &&item, std::chrono::duration<Rep, Period> const &timeout) {
        return impl_.push_wait(std::forward<U>(item), timeout);
    }

    /*************************************************************************/
    // --- consumer side (forwarded verbatim) ---

    [[nodiscard]] std::optional<T> try_pop() { return impl_.try_pop(); }
    [[nodiscard]] std::optional<T> pop() { return impl_.pop(); }

    template <typename Rep, typename Period>
    [[nodiscard]] std::optional<T> pop_wait(std::chrono::duration<Rep, Period> const &timeout) {
        return impl_.pop_wait(timeout);
    }

    /*************************************************************************/
    // --- lifecycle and observers (forwarded verbatim) ---

    void close() { impl_.close(); }
    [[nodiscard]] bool is_closed() const { return impl_.is_closed(); }
    [[nodiscard]] static constexpr std::size_t capacity() noexcept { return impl_type::capacity(); }
    [[nodiscard]] static constexpr bool bounded() noexcept { return impl_type::bounded(); }
    [[nodiscard]] std::size_t size() const { return impl_.size(); }
    [[nodiscard]] std::size_t remaining_space() const { return impl_.remaining_space(); }
    [[nodiscard]] bool empty() const { return impl_.empty(); }

private:
    impl_type impl_;
};

/******************************************************************************/

} /* namespace Gem::Common */
