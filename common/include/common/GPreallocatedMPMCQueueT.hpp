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
#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <new>
#include <optional>
#include <type_traits>
#include <utility>

// Geneva headers go here
#include "common/GQueueCommon.hpp" // for the MPMCQueue concept (documentation/checking)

namespace Gem::Common {

/******************************************************************************/
/**
 * A thread-safe, multi-producer / multi-consumer (MPMC) FIFO blocking queue backed by a
 * **preallocated ring buffer**. It is interface-identical to GBlockingMPMCQueueT (it satisfies the
 * same MPMCQueue concept) and is meant as a drop-in, optionally selectable alternative via the
 * GMPMCQueueT facade. Compared with the deque-backed queue it never allocates per item (the ring is
 * allocated once) and stores elements contiguously, which is friendlier to the cache.
 *
 * Storage. The ring holds `Cap` raw slots allocated ONCE on the heap (not an inline std::array, so a
 * large `Cap` does not bloat the queue object or the stack). Each slot manages the lifetime of its
 * `T` by hand (placement-new on push, explicit destruction on pop / teardown), so the element type
 * needs no default constructor and move-only types such as std::unique_ptr are fully supported. Only
 * the `[head, head+count)` slots are ever live; the destructor destroys exactly those.
 *
 * Synchronisation. A single mutex plus two condition variables, with predicate-based waits and
 * notification outside the lock -- the same proven core as GBlockingMPMCQueueT. NOTE: the original
 * standalone implementation this is derived from used a finer-grained TWO-mutex design (separate
 * head/tail locks with a lock-free atomic count) for concurrent push/pop. That design, as written,
 * has a latent lost-wakeup: a consumer that frees a slot signals `not_full` without holding the
 * producer's lock, so a producer in the unlock-and-block window of its wait can miss the
 * notification and park even though space is available. A correct bounded two-lock queue needs
 * semaphore-style accounting (edge-triggered notification under the peer lock does not wake the
 * right number of waiters). Rather than import that subtlety, this Geneva version keeps the ring
 * storage but uses the single-mutex core, which is unconditionally wake-up-safe. Reintroducing a
 * correct two-lock fast path is possible future work; the public interface would not change.
 *
 * `Cap` is the fixed capacity and must be > 0 (this queue is bounded by construction; use the deque
 * backend for the unbounded case). Items are pushed at the tail and popped from the head (FIFO).
 */
template <typename T, std::size_t Cap>
class GPreallocatedMPMCQueueT final {
    static_assert(Cap > 0, "GPreallocatedMPMCQueueT<T, Cap>: Cap must be > 0 (this queue is bounded)");
    static_assert(
        std::is_move_constructible_v<T>,
        "GPreallocatedMPMCQueueT<T>: T must be move-constructible"
    );

    /*************************************************************************/
    /** @brief One ring slot. The contained T's lifetime is managed by hand (the union member is left
     *  uninitialised by the slot's constructor and only ever constructed/destroyed explicitly). */
    union Slot {
        T value_;
        Slot() noexcept {}  // leave the storage uninitialised
        ~Slot() noexcept {} // the owning queue destroys live elements explicitly
    };

public:
    using value_type = T;

    /*************************************************************************/
    GPreallocatedMPMCQueueT()
        : ring_(std::make_unique<Slot[]>(Cap))
    { /* nothing */ }

    // Owns a mutex, condition variables and the ring: neither copyable nor movable.
    GPreallocatedMPMCQueueT(GPreallocatedMPMCQueueT const &) = delete;
    GPreallocatedMPMCQueueT &operator=(GPreallocatedMPMCQueueT const &) = delete;
    GPreallocatedMPMCQueueT(GPreallocatedMPMCQueueT &&) = delete;
    GPreallocatedMPMCQueueT &operator=(GPreallocatedMPMCQueueT &&) = delete;

    /*************************************************************************/
    /**
     * The destructor closes the queue (waking any stragglers) and destroys every element still in
     * the ring. A destructor must not throw, so any escaping exception is swallowed.
     */
    ~GPreallocatedMPMCQueueT() {
        try {
            close();
            std::scoped_lock lock(mutex_);
            destroy_all_unlocked();
        } catch(...) { // NOLINT(bugprone-empty-catch) — a destructor must not throw
        }
    }

    /*************************************************************************/
    /**
     * Tries to add an item without blocking. Returns false immediately if the queue is full or
     * closed. Covers both copy and move via perfect forwarding.
     */
    template <typename U>
        requires std::constructible_from<T, U &&>
    [[nodiscard]] bool try_push(U &&item) {
        {
            std::scoped_lock lock(mutex_);
            if(closed_ || count_ >= Cap) {
                return false;
            }
            emplace_at_tail_unlocked(std::forward<U>(item));
        }
        not_empty_.notify_one();
        return true;
    }

    /*************************************************************************/
    /**
     * Adds an item, blocking until space is available. Returns false (without adding) if the queue
     * is closed while waiting or already closed.
     */
    template <typename U>
        requires std::constructible_from<T, U &&>
    [[nodiscard]] bool push(U &&item) {
        {
            std::unique_lock lock(mutex_);
            not_full_.wait(lock, [this]() -> bool { return closed_ || count_ < Cap; });
            if(closed_) {
                return false;
            }
            emplace_at_tail_unlocked(std::forward<U>(item));
        }
        not_empty_.notify_one();
        return true;
    }

    /*************************************************************************/
    /**
     * Adds an item, blocking until space is available or the timeout elapses. Returns false if it
     * timed out or the queue was closed.
     */
    template <typename U, typename Rep, typename Period>
        requires std::constructible_from<T, U &&>
    [[nodiscard]] bool push_wait(U &&item, std::chrono::duration<Rep, Period> const &timeout) {
        {
            std::unique_lock lock(mutex_);
            if(not not_full_.wait_for(
                   lock, timeout, [this]() -> bool { return closed_ || count_ < Cap; }
               )) {
                return false; // timed out with no space
            }
            if(closed_) {
                return false;
            }
            emplace_at_tail_unlocked(std::forward<U>(item));
        }
        not_empty_.notify_one();
        return true;
    }

    /*************************************************************************/
    /** @brief Tries to remove an item without blocking. Returns std::nullopt if the queue is empty. */
    [[nodiscard]] std::optional<T> try_pop() {
        std::optional<T> result;
        {
            std::scoped_lock lock(mutex_);
            if(count_ > 0) {
                result.emplace(extract_at_head_unlocked());
            }
        }
        if(result.has_value()) {
            not_full_.notify_one();
        }
        return result;
    }

    /*************************************************************************/
    /**
     * Removes an item, blocking until one is available. If the queue is closed and drained, returns
     * std::nullopt -- so the canonical consumer loop is `while (auto item = q.pop()) { ... }`.
     */
    [[nodiscard]] std::optional<T> pop() {
        std::optional<T> result;
        {
            std::unique_lock lock(mutex_);
            not_empty_.wait(lock, [this]() -> bool { return closed_ || count_ > 0; });
            // Drain remaining items even after close; only report end-of-stream once empty.
            if(count_ > 0) {
                result.emplace(extract_at_head_unlocked());
            }
        }
        if(result.has_value()) {
            not_full_.notify_one();
        }
        return result;
    }

    /*************************************************************************/
    /**
     * Removes an item, blocking until one is available or the timeout elapses. Returns std::nullopt
     * on timeout, or if the queue is closed and empty.
     */
    template <typename Rep, typename Period>
    [[nodiscard]] std::optional<T> pop_wait(std::chrono::duration<Rep, Period> const &timeout) {
        std::optional<T> result;
        {
            std::unique_lock lock(mutex_);
            if(not not_empty_.wait_for(
                   lock, timeout, [this]() -> bool { return closed_ || count_ > 0; }
               )) {
                return std::nullopt; // timed out with no item
            }
            if(count_ > 0) {
                result.emplace(extract_at_head_unlocked());
            }
        }
        if(result.has_value()) {
            not_full_.notify_one();
        }
        return result;
    }

    /*************************************************************************/
    /**
     * Closes the queue. Wakes every blocked producer and consumer: blocked/subsequent producers
     * return false, consumers drain the remaining items and then see std::nullopt. Terminal and
     * idempotent.
     */
    void close() {
        {
            std::scoped_lock lock(mutex_);
            closed_ = true;
        }
        not_empty_.notify_all();
        not_full_.notify_all();
    }

    /*************************************************************************/
    /** @brief Returns whether the queue has been closed. */
    [[nodiscard]] bool is_closed() const {
        std::scoped_lock lock(mutex_);
        return closed_;
    }

    /*************************************************************************/
    /** @brief The maximum number of items. Compile-time. */
    [[nodiscard]] static constexpr std::size_t capacity() noexcept {
        return Cap;
    }

    /*************************************************************************/
    /** @brief Whether this is a bounded queue. Always true for the preallocated backend. */
    [[nodiscard]] static constexpr bool bounded() noexcept {
        return true;
    }

    /*************************************************************************/
    /** @brief The current number of items. Only an indication in concurrent use. */
    [[nodiscard]] std::size_t size() const {
        std::scoped_lock lock(mutex_);
        return count_;
    }

    /*************************************************************************/
    /** @brief The remaining space. Only an indication in concurrent use. */
    [[nodiscard]] std::size_t remaining_space() const {
        std::scoped_lock lock(mutex_);
        return Cap - count_;
    }

    /*************************************************************************/
    /** @brief Whether the queue is currently empty. Only an indication. */
    [[nodiscard]] bool empty() const {
        std::scoped_lock lock(mutex_);
        return count_ == 0;
    }

private:
    /*************************************************************************/
    /** @brief The next ring index after @p i. */
    [[nodiscard]] static constexpr std::size_t next_index(std::size_t i) noexcept {
        return (i + 1) % Cap;
    }

    /*************************************************************************/
    /** @brief Constructs an item into the tail slot and advances the tail. Caller holds the lock and
     *  has verified there is space. */
    template <typename U>
    void emplace_at_tail_unlocked(U &&item) {
        ::new(static_cast<void *>(std::addressof(ring_[tail_].value_)))
            T(std::forward<U>(item));
        tail_ = next_index(tail_);
        ++count_;
    }

    /*************************************************************************/
    /** @brief Moves the head item out, destroys the slot, advances the head. Caller holds the lock
     *  and has verified the queue is non-empty. */
    [[nodiscard]] T extract_at_head_unlocked() {
        T &slot = ring_[head_].value_;
        T result = std::move(slot);
        slot.~T();
        head_ = next_index(head_);
        --count_;
        return result;
    }

    /*************************************************************************/
    /** @brief Destroys every live element (used by the destructor). Caller holds the lock. */
    void destroy_all_unlocked() noexcept {
        std::size_t idx = head_;
        for(std::size_t n = 0; n < count_; ++n) {
            ring_[idx].value_.~T();
            idx = next_index(idx);
        }
        count_ = 0;
    }

    /*************************************************************************/
    mutable std::mutex mutex_;          ///< Guards the ring, the indices, count_ and closed_
    std::condition_variable not_empty_; ///< Signalled when an item becomes available
    std::condition_variable not_full_;  ///< Signalled when space becomes available

    std::unique_ptr<Slot[]> ring_;      ///< The preallocated ring storage (Cap slots)
    std::size_t head_ = 0;              ///< Index of the next item to pop
    std::size_t tail_ = 0;             ///< Index of the next free slot to push into
    std::size_t count_ = 0;             ///< Number of live items currently in the ring
    bool closed_ = false;               ///< Set by close(); terminal
};

/******************************************************************************/

} /* namespace Gem::Common */
