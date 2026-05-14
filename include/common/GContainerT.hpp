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
#include <algorithm>
#include <concepts>
#include <deque>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <ranges>
#include <sstream>
#include <type_traits>
#include <vector>

// Boost header files go here
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

// Geneva headers go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "common/GTypeTraitsT.hpp"

namespace Gem::Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/**
 * @brief Concept: the container provides contiguous storage accessible via data().
 *
 * Satisfied by std::vector and std::array but not by std::deque or std::list.
 *
 * @tparam C The container type to check.
 */
template <typename C>
concept HasContiguousStorage = requires(C c) { c.data(); };

/******************************************************************************/
/**
 * @brief Concept: the container supports capacity management.
 *
 * Satisfied by std::vector but not by std::deque or std::list.
 *
 * @tparam C The container type to check.
 */
template <typename C>
concept HasCapacity = requires(C c, typename C::size_type n) {
    c.reserve(n);
    c.capacity();
    c.shrink_to_fit();
};

/******************************************************************************/
/**
 * @brief Concept: the container's iterators satisfy the random-access requirement.
 *
 * Satisfied by std::vector and std::deque but not by std::list or
 * std::forward_list.
 *
 * @tparam C The container type to check.
 */
template <typename C>
concept HasRandomAccess = std::random_access_iterator<typename C::iterator>;

/******************************************************************************/
/**
 * @brief Concept: the container supports O(1) front insertion and removal.
 *
 * Satisfied by std::deque and std::list but not by std::vector.
 *
 * @tparam C The container type to check.
 */
template <typename C>
concept HasFrontInsertion = requires(C c, typename C::value_type v) {
    c.push_front(v);
    c.pop_front();
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/**
 * @brief Storage policy for POD (plain-old-data) types.
 *
 * Wraps a plain sequence container whose element type is @p T. A simple
 * value-copy is sufficient for deep-copy semantics because POD objects have
 * no ownership semantics.
 *
 * @tparam T         The element type. Must satisfy std::is_trivial and
 *                   std::is_standard_layout (i.e., be a POD type).
 * @tparam Container The underlying sequence container. Defaults to
 *                   std::vector<T>. Any container with a compatible STL
 *                   sequence interface may be used (e.g., std::deque<T>).
 */
template <typename T, typename Container = std::vector<T>>
    requires std::is_trivial_v<T> && std::is_standard_layout_v<T>
struct PodStorage {
    /** @brief The logical element type exposed by the container interface. */
    using ValueType = T;
    /** @brief The type actually stored in the container (identical to T for POD). */
    using StoredType = T;
    /** @brief The underlying sequence container type. */
    using ContainerType = Container;

    /**
     * @brief Performs a deep copy of the container.
     *
     * For POD types, a plain assignment is sufficient.
     *
     * @param src The source container.
     * @param dst The destination container; its previous contents are replaced.
     */
    static void deepCopy(const ContainerType &src, ContainerType &dst) {
        dst = src;
    }
};

/******************************************************************************/
/**
 * @brief Storage policy for polymorphic Geneva objects held via shared_ptr.
 *
 * Wraps a sequence container of std::shared_ptr<T>. Deep copies are performed
 * via the clone()/load() protocol required by the Gemfony common interface.
 *
 * @tparam T         The element type. Must expose the Gemfony common interface
 *                   (detected via has_gemfony_common_interface<T>::value).
 * @tparam Container The underlying sequence container. Defaults to
 *                   std::vector<std::shared_ptr<T>>.
 */
template <typename T, typename Container = std::vector<std::shared_ptr<T>>>
    requires Gem::Common::has_gemfony_common_interface<T>::value
struct SharedPtrStorage {
    /** @brief The logical element type exposed by the container interface. */
    using ValueType = T;
    /** @brief The type actually stored in the container (a shared_ptr to T). */
    using StoredType = std::shared_ptr<T>;
    /** @brief The underlying sequence container type. */
    using ContainerType = Container;

    /**
     * @brief Performs a deep copy of the container using the clone()/load() protocol.
     *
     * Each element in @p src is cloned; pre-existing elements in @p dst are
     * reused via load() when sizes match to minimise allocations.
     *
     * @param src The source container.
     * @param dst The destination container; its previous contents are replaced.
     */
    static void deepCopy(const ContainerType &src, ContainerType &dst) {
        Gem::Common::copyCloneableSmartPointerContainer(src, dst);
    }
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/**
 * @brief A unified generic container base class combining POD and smart-pointer
 *        vector semantics under a single, policy-based interface.
 *
 * GContainerT is an abstract base class (pure-virtual destructor) that wraps
 * a configurable sequence container and provides the full STL sequence-container
 * API together with Geneva-specific extensions such as crossOver(), compareBase(),
 * clone-aware push/insert/resize operations (for SharedPtrStorage), and a
 * type-filtered range view.
 *
 * The underlying storage strategy is chosen at compile time via the
 * @p StoragePolicy parameter:
 *  - PodStorage<T>         — for POD element types (equivalent to GPODVectorT)
 *  - SharedPtrStorage<T>   — for polymorphic Geneva objects (equivalent to
 *                             GPtrVectorT)
 *
 * Container-capability concepts (HasContiguousStorage, HasCapacity,
 * HasRandomAccess, HasFrontInsertion) gate methods that are not universally
 * available, providing clear compile-time diagnostics when an unsupported
 * operation is attempted.
 *
 * @tparam T             The logical element type.
 * @tparam StoragePolicy The storage policy. Must be either PodStorage<T,...>
 *                       or SharedPtrStorage<T,...>.
 */
template <typename T, typename StoragePolicy = PodStorage<T>>
class GContainerT {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar &BOOST_SERIALIZATION_NVP(dataCnt_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    // ------------------------------------------------------------------
    // Type aliases
    // ------------------------------------------------------------------

    /** @brief The logical element type. */
    using ValueType = typename StoragePolicy::ValueType;
    /** @brief The type physically stored in the container. */
    using StoredType = typename StoragePolicy::StoredType;
    /** @brief The underlying sequence container type. */
    using ContainerType = typename StoragePolicy::ContainerType;

    /** @brief Standard STL alias: value_type */
    using value_type = typename ContainerType::value_type;
    /** @brief Standard STL alias: reference */
    using reference = typename ContainerType::reference;
    /** @brief Standard STL alias: const_reference */
    using const_reference = typename ContainerType::const_reference;
    /** @brief Standard STL alias: iterator */
    using iterator = typename ContainerType::iterator;
    /** @brief Standard STL alias: const_iterator */
    using const_iterator = typename ContainerType::const_iterator;
    /** @brief Standard STL alias: reverse_iterator */
    using reverse_iterator = typename ContainerType::reverse_iterator;
    /** @brief Standard STL alias: const_reverse_iterator */
    using const_reverse_iterator = typename ContainerType::const_reverse_iterator;
    /** @brief Standard STL alias: size_type */
    using size_type = typename ContainerType::size_type;
    /** @brief Standard STL alias: difference_type */
    using difference_type = typename ContainerType::difference_type;

    // ------------------------------------------------------------------
    // Constructors, destructor, assignment
    // ------------------------------------------------------------------

    /** @brief Default constructor — creates an empty container. */
    GContainerT() = default;

    /**
     * @brief Constructs the container with @p nVal copies of @p val.
     *
     * Only participates in overload resolution when StoragePolicy is
     * PodStorage (i.e., StoredType == ValueType).
     *
     * @param nVal The number of elements to create.
     * @param val  The value assigned to every element.
     */
    explicit GContainerT(size_type nVal, const StoredType &val)
        requires std::same_as<StoredType, ValueType>
      : dataCnt_(nVal, val) {}

    /**
     * @brief Copy constructor — performs a deep copy via the storage policy.
     *
     * For PodStorage a plain container copy suffices. For SharedPtrStorage
     * every element is cloned via the Gemfony clone()/load() protocol.
     *
     * @param cp The source object.
     */
    GContainerT(const GContainerT &cp) {
        StoragePolicy::deepCopy(cp.dataCnt_, dataCnt_);
    }

    /** @brief Move constructor. Leaves the source in a valid but unspecified state. */
    GContainerT(GContainerT &&) noexcept = default;

    /**
     * @brief Pure-virtual destructor — makes GContainerT an abstract base class.
     *
     * @note noexcept: the destructor itself never throws; derived destructors
     *       must uphold the same guarantee.
     */
    virtual ~GContainerT() = 0;

    /**
     * @brief Copy-assignment operator — performs a deep copy.
     *
     * Self-assignment is handled safely.
     *
     * @param cp The source object.
     * @return A reference to this object.
     */
    GContainerT &operator=(const GContainerT &cp) {
        if(this == &cp) {
            return *this;
        }
        StoragePolicy::deepCopy(cp.dataCnt_, dataCnt_);
        return *this;
    }

    /** @brief Move-assignment operator. */
    GContainerT &operator=(GContainerT &&) noexcept = default;

    // Deleted comparison operators (use compareBase() instead).
    bool operator==(const GContainerT &) const = delete;
    bool operator!=(const GContainerT &) const = delete;

    // ------------------------------------------------------------------
    // Capacity
    // ------------------------------------------------------------------

    /**
     * @brief Returns the number of elements in the container.
     *
     * @return The element count.
     * @note noexcept: this function never throws.
     */
    [[nodiscard]] size_type size() const noexcept {
        return dataCnt_.size();
    }

    /**
     * @brief Returns true if the container contains no elements.
     *
     * @return true when empty, false otherwise.
     * @note noexcept: this function never throws.
     */
    [[nodiscard]] bool empty() const noexcept {
        return dataCnt_.empty();
    }

    /**
     * @brief Returns the maximum number of elements the container can hold.
     *
     * @return The theoretical maximum size.
     */
    [[nodiscard]] size_type maxSize() const {
        return dataCnt_.max_size();
    }

    /**
     * @brief Returns the number of elements the container has allocated space for.
     *
     * @note Only available when ContainerType satisfies HasCapacity
     *       (e.g., std::vector, but not std::deque).
     * @return The capacity.
     */
    [[nodiscard]] size_type capacity() const
        requires HasCapacity<ContainerType>
    {
        return dataCnt_.capacity();
    }

    /**
     * @brief Requests that the container increase its capacity to at least @p amount.
     *
     * @note Only available when ContainerType satisfies HasCapacity.
     * @param amount The minimum desired capacity.
     */
    void reserve(size_type amount)
        requires HasCapacity<ContainerType>
    {
        dataCnt_.reserve(amount);
    }

    /**
     * @brief Requests that unused capacity be released to the system.
     *
     * @note Only available when ContainerType satisfies HasCapacity.
     */
    void shrinkToFit()
        requires HasCapacity<ContainerType>
    {
        dataCnt_.shrink_to_fit();
    }

    // ------------------------------------------------------------------
    // Element access
    // ------------------------------------------------------------------

    /**
     * @brief Returns a reference to the element at position @p pos.
     *
     * No bounds checking is performed.
     *
     * @param pos The zero-based position.
     * @return A reference to the element.
     */
    reference operator[](size_type pos) {
        return dataCnt_[pos];
    }

    /**
     * @brief Returns a const reference to the element at position @p pos.
     *
     * No bounds checking is performed.
     *
     * @param pos The zero-based position.
     * @return A const reference to the element.
     */
    [[nodiscard]] const_reference operator[](size_type pos) const {
        return dataCnt_[pos];
    }

    /**
     * @brief Returns a reference to the element at position @p pos.
     *
     * @param pos The zero-based position.
     * @return A reference to the element.
     * @throws std::out_of_range when @p pos >= size().
     */
    reference at(size_type pos) {
        return dataCnt_.at(pos);
    }

    /**
     * @brief Returns a const reference to the element at position @p pos.
     *
     * @param pos The zero-based position.
     * @return A const reference to the element.
     * @throws std::out_of_range when @p pos >= size().
     */
    [[nodiscard]] const_reference at(size_type pos) const {
        return dataCnt_.at(pos);
    }

    /**
     * @brief Returns a reference to the first element.
     *
     * @return A reference to the first element.
     * @note Calling this on an empty container is undefined behaviour.
     */
    reference front() {
        return dataCnt_.front();
    }

    /**
     * @brief Returns a const reference to the first element.
     *
     * @return A const reference to the first element.
     * @note Calling this on an empty container is undefined behaviour.
     */
    [[nodiscard]] const_reference front() const {
        return dataCnt_.front();
    }

    /**
     * @brief Returns a reference to the last element.
     *
     * @return A reference to the last element.
     * @note Calling this on an empty container is undefined behaviour.
     */
    reference back() {
        return dataCnt_.back();
    }

    /**
     * @brief Returns a const reference to the last element.
     *
     * @return A const reference to the last element.
     * @note Calling this on an empty container is undefined behaviour.
     */
    [[nodiscard]] const_reference back() const {
        return dataCnt_.back();
    }

    /**
     * @brief Returns a raw pointer to the underlying contiguous element array.
     *
     * @note Only available when ContainerType satisfies HasContiguousStorage
     *       (e.g., std::vector, but not std::deque).
     * @return A pointer to the first element.
     */
    [[nodiscard]] StoredType *data() noexcept
        requires HasContiguousStorage<ContainerType>
    {
        return dataCnt_.data();
    }

    /**
     * @brief Returns a const raw pointer to the underlying contiguous element array.
     *
     * @note Only available when ContainerType satisfies HasContiguousStorage.
     * @return A const pointer to the first element.
     */
    [[nodiscard]] const StoredType *data() const noexcept
        requires HasContiguousStorage<ContainerType>
    {
        return dataCnt_.data();
    }

    // ------------------------------------------------------------------
    // Iterators
    // ------------------------------------------------------------------

    /**
     * @brief Returns an iterator to the first element.
     * @note noexcept: iterator construction never throws.
     */
    iterator begin() noexcept {
        return dataCnt_.begin();
    }

    /**
     * @brief Returns a const iterator to the first element.
     * @note noexcept: iterator construction never throws.
     */
    [[nodiscard]] const_iterator begin() const noexcept {
        return dataCnt_.begin();
    }

    /**
     * @brief Returns a past-the-end iterator.
     * @note noexcept: iterator construction never throws.
     */
    iterator end() noexcept {
        return dataCnt_.end();
    }

    /**
     * @brief Returns a const past-the-end iterator.
     * @note noexcept: iterator construction never throws.
     */
    [[nodiscard]] const_iterator end() const noexcept {
        return dataCnt_.end();
    }

    /**
     * @brief Returns a const iterator to the first element.
     * @note noexcept: iterator construction never throws.
     */
    [[nodiscard]] const_iterator cbegin() const noexcept {
        return dataCnt_.cbegin();
    }

    /**
     * @brief Returns a const past-the-end iterator.
     * @note noexcept: iterator construction never throws.
     */
    [[nodiscard]] const_iterator cend() const noexcept {
        return dataCnt_.cend();
    }

    /**
     * @brief Returns a reverse iterator to the last element.
     * @note noexcept: iterator construction never throws.
     */
    reverse_iterator rbegin() noexcept {
        return dataCnt_.rbegin();
    }

    /**
     * @brief Returns a const reverse iterator to the last element.
     * @note noexcept: iterator construction never throws.
     */
    [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
        return dataCnt_.rbegin();
    }

    /**
     * @brief Returns a reverse past-the-end iterator.
     * @note noexcept: iterator construction never throws.
     */
    reverse_iterator rend() noexcept {
        return dataCnt_.rend();
    }

    /**
     * @brief Returns a const reverse past-the-end iterator.
     * @note noexcept: iterator construction never throws.
     */
    [[nodiscard]] const_reverse_iterator rend() const noexcept {
        return dataCnt_.rend();
    }

    /**
     * @brief Returns a const reverse iterator to the last element.
     * @note noexcept: iterator construction never throws.
     */
    [[nodiscard]] const_reverse_iterator crbegin() const noexcept {
        return dataCnt_.crbegin();
    }

    /**
     * @brief Returns a const reverse past-the-end iterator.
     * @note noexcept: iterator construction never throws.
     */
    [[nodiscard]] const_reverse_iterator crend() const noexcept {
        return dataCnt_.crend();
    }

    // ------------------------------------------------------------------
    // Modifiers — clear / resize
    // ------------------------------------------------------------------

    /**
     * @brief Removes all elements from the container.
     *
     * The capacity (if applicable) is left unchanged.
     */
    void clear() {
        dataCnt_.clear();
    }

    /**
     * @brief Resizes the container to @p amount elements.
     *
     * If the container grows, new elements are value-initialised (POD: zero,
     * shared_ptr: nullptr).
     *
     * @param amount The desired number of elements.
     */
    void resize(size_type amount) {
        dataCnt_.resize(amount);
    }

    /**
     * @brief Resizes the container to @p amount elements, initialising new ones
     *        with copies of @p item.
     *
     * @param amount The desired number of elements.
     * @param item   The value used for new elements.
     */
    void resize(size_type amount, const StoredType &item) {
        dataCnt_.resize(amount, item);
    }

    // ------------------------------------------------------------------
    // Modifiers — assign
    // ------------------------------------------------------------------

    /**
     * @brief Replaces the contents with @p count copies of @p value.
     *
     * @param count The number of copies.
     * @param value The value to assign.
     */
    void assign(size_type count, const StoredType &value) {
        dataCnt_.assign(count, value);
    }

    /**
     * @brief Replaces the contents with elements from the range [@p first, @p last).
     *
     * @tparam InputIt An input iterator type.
     * @param first The beginning of the source range.
     * @param last  The past-the-end of the source range.
     */
    template <std::input_iterator InputIt>
    void assign(InputIt first, InputIt last) {
        dataCnt_.assign(first, last);
    }

    /**
     * @brief Replaces the contents with elements from an initializer list.
     *
     * @param il The initializer list.
     */
    void assign(std::initializer_list<StoredType> il) {
        dataCnt_.assign(il);
    }

    // ------------------------------------------------------------------
    // Modifiers — push / emplace / pop (back)
    // ------------------------------------------------------------------

    /**
     * @brief Appends a copy of @p item to the back of the container.
     *
     * For SharedPtrStorage: performs a no-clone insert (shared ownership).
     * Use pushBackClone() to insert an independent copy.
     *
     * @param item The item to append.
     */
    void pushBack(const StoredType &item) {
        dataCnt_.push_back(item);
    }

    /**
     * @brief Appends a moved item to the back of the container.
     *
     * @param item The item to move-append.
     */
    void pushBack(StoredType &&item) {
        dataCnt_.push_back(std::move(item));
    }

    /**
     * @brief Constructs an element in-place at the back of the container.
     *
     * Only available for PodStorage (emplace_back on shared_ptr would require
     * passing constructor arguments for the pointee, which is not meaningful
     * in this context).
     *
     * @tparam Args Argument types forwarded to the element constructor.
     * @param args Arguments forwarded to the element constructor.
     * @return A reference to the newly constructed element.
     */
    template <typename... Args>
    reference emplaceBack(Args &&...args)
        requires std::same_as<StoredType, ValueType>
    {
        return dataCnt_.emplace_back(std::forward<Args>(args)...);
    }

    /**
     * @brief Removes the last element from the container.
     *
     * Calling this on an empty container is undefined behaviour.
     */
    void popBack() {
        dataCnt_.pop_back();
    }

    // ------------------------------------------------------------------
    // Modifiers — push / emplace / pop (front) — conditional
    // ------------------------------------------------------------------

    /**
     * @brief Prepends a copy of @p item to the front of the container.
     *
     * @note Only available when ContainerType satisfies HasFrontInsertion
     *       (e.g., std::deque, but not std::vector).
     * @param item The item to prepend.
     */
    void pushFront(const StoredType &item)
        requires HasFrontInsertion<ContainerType>
    {
        dataCnt_.push_front(item);
    }

    /**
     * @brief Prepends a moved item to the front of the container.
     *
     * @note Only available when ContainerType satisfies HasFrontInsertion.
     * @param item The item to move-prepend.
     */
    void pushFront(StoredType &&item)
        requires HasFrontInsertion<ContainerType>
    {
        dataCnt_.push_front(std::move(item));
    }

    /**
     * @brief Constructs an element in-place at the front of the container.
     *
     * @note Only available when ContainerType satisfies HasFrontInsertion
     *       and StoredType == ValueType (PodStorage).
     * @tparam Args Argument types forwarded to the element constructor.
     * @param args Arguments forwarded to the element constructor.
     */
    template <typename... Args>
    void emplaceFront(Args &&...args)
        requires HasFrontInsertion<ContainerType> && std::same_as<StoredType, ValueType>
    {
        dataCnt_.emplace_front(std::forward<Args>(args)...);
    }

    /**
     * @brief Removes the first element from the container.
     *
     * @note Only available when ContainerType satisfies HasFrontInsertion.
     *       Calling this on an empty container is undefined behaviour.
     */
    void popFront()
        requires HasFrontInsertion<ContainerType>
    {
        dataCnt_.pop_front();
    }

    // ------------------------------------------------------------------
    // Modifiers — insert / emplace
    // ------------------------------------------------------------------

    /**
     * @brief Inserts @p item before the element at @p pos.
     *
     * @param pos  An iterator pointing to the insertion position.
     * @param item The item to insert.
     * @return An iterator to the inserted element.
     */
    iterator insert(const_iterator pos, const StoredType &item) {
        return dataCnt_.insert(pos, item);
    }

    /**
     * @brief Inserts a moved @p item before the element at @p pos.
     *
     * @param pos  An iterator pointing to the insertion position.
     * @param item The item to move-insert.
     * @return An iterator to the inserted element.
     */
    iterator insert(const_iterator pos, StoredType &&item) {
        return dataCnt_.insert(pos, std::move(item));
    }

    /**
     * @brief Inserts @p count copies of @p item before @p pos.
     *
     * @param pos   An iterator pointing to the insertion position.
     * @param count The number of copies to insert.
     * @param item  The value to insert.
     * @return An iterator to the first inserted element.
     */
    iterator insert(const_iterator pos, size_type count, const StoredType &item) {
        return dataCnt_.insert(pos, count, item);
    }

    /**
     * @brief Inserts elements from the range [@p first, @p last) before @p pos.
     *
     * @tparam InputIt An input iterator type.
     * @param pos   An iterator pointing to the insertion position.
     * @param first The beginning of the source range.
     * @param last  The past-the-end of the source range.
     * @return An iterator to the first inserted element.
     */
    template <std::input_iterator InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last) {
        return dataCnt_.insert(pos, first, last);
    }

    /**
     * @brief Inserts elements from an initializer list before @p pos.
     *
     * @param pos An iterator pointing to the insertion position.
     * @param il  The initializer list.
     * @return An iterator to the first inserted element.
     */
    iterator insert(const_iterator pos, std::initializer_list<StoredType> il) {
        return dataCnt_.insert(pos, il);
    }

    /**
     * @brief Constructs an element in-place before @p pos.
     *
     * Only available for PodStorage (same rationale as emplaceBack()).
     *
     * @tparam Args Argument types forwarded to the element constructor.
     * @param pos  An iterator pointing to the insertion position.
     * @param args Arguments forwarded to the element constructor.
     * @return An iterator to the newly constructed element.
     */
    template <typename... Args>
    iterator emplace(const_iterator pos, Args &&...args)
        requires std::same_as<StoredType, ValueType>
    {
        return dataCnt_.emplace(pos, std::forward<Args>(args)...);
    }

    // ------------------------------------------------------------------
    // Modifiers — erase
    // ------------------------------------------------------------------

    /**
     * @brief Erases the element at @p pos.
     *
     * @param pos An iterator to the element to remove.
     * @return An iterator following the removed element.
     */
    iterator erase(const_iterator pos) {
        return dataCnt_.erase(pos);
    }

    /**
     * @brief Erases elements in the range [@p first, @p last).
     *
     * @param first An iterator to the first element to remove.
     * @param last  A past-the-end iterator of the range to remove.
     * @return An iterator following the last removed element.
     */
    iterator erase(const_iterator first, const_iterator last) {
        return dataCnt_.erase(first, last);
    }

    // ------------------------------------------------------------------
    // Modifiers — swap
    // ------------------------------------------------------------------

    /**
     * @brief Swaps the contents of this container with a raw ContainerType.
     *
     * @param cont The external container to swap with.
     */
    void swap(ContainerType &cont) {
        dataCnt_.swap(cont);
    }

    /**
     * @brief Swaps the contents of two GContainerT objects.
     *
     * @param other The other GContainerT object to swap with.
     */
    void swap(GContainerT &other) {
        dataCnt_.swap(other.dataCnt_);
    }

    // ------------------------------------------------------------------
    // Search
    // ------------------------------------------------------------------

    /**
     * @brief Counts the number of elements equal to @p item.
     *
     * For PodStorage, equality is checked by value. For SharedPtrStorage the
     * function is overloaded below with the appropriate null-pointer guard.
     *
     * @param item The value to search for.
     * @return The number of matching elements.
     */
    [[nodiscard]] size_type count(const StoredType &item) const
        requires std::same_as<StoredType, ValueType>
    {
        return Gem::Common::narrow_cast<size_type>(
            std::ranges::count(dataCnt_, item)
        );
    }

    /**
     * @brief Returns an iterator to the first element equal to @p item.
     *
     * For PodStorage only; SharedPtrStorage uses its own overload below.
     *
     * @param item The value to search for.
     * @return An iterator to the found element, or end() if not found.
     */
    [[nodiscard]] const_iterator find(const StoredType &item) const
        requires std::same_as<StoredType, ValueType>
    {
        return std::ranges::find(dataCnt_, item);
    }

    // ------------------------------------------------------------------
    // SharedPtrStorage-specific: count / find
    // ------------------------------------------------------------------

    /**
     * @brief Counts elements whose dereferenced value equals @p item.
     *
     * Compares via operator== on the pointed-to objects, not on pointer identity.
     * A null pointer in @p item is not accepted.
     *
     * @tparam ItemType A type derived from or equal to ValueType.
     * @param item A shared_ptr to the reference value.
     * @return The number of matching elements.
     * @throws geneva_exception when @p item is null.
     * @note Only available for SharedPtrStorage.
     */
    template <typename ItemType>
        requires(!std::same_as<StoredType, ValueType> && std::derived_from<ItemType, ValueType>)
    [[nodiscard]] size_type count(const std::shared_ptr<ItemType> &item) const {
        if(not item) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GContainerT::count(): "
                << "Tried to count with an empty smart pointer." << std::endl
            );
        }
        return Gem::Common::narrow_cast<size_type>(std::count_if(
            dataCnt_.begin(),
            dataCnt_.end(),
            [&item](const StoredType &contItem) -> bool {
                auto cast = std::dynamic_pointer_cast<ItemType>(contItem);
                return cast && (*item == *cast);
            }
        ));
    }

    /**
     * @brief Returns an iterator to the first element whose dereferenced value
     *        equals @p item.
     *
     * @tparam ItemType A type derived from or equal to ValueType.
     * @param item A shared_ptr to the reference value.
     * @return A const_iterator to the found element, or end() if not found.
     * @throws geneva_exception when @p item is null.
     * @note Only available for SharedPtrStorage.
     */
    template <typename ItemType>
        requires(!std::same_as<StoredType, ValueType> && std::derived_from<ItemType, ValueType>)
    [[nodiscard]] const_iterator find(const std::shared_ptr<ItemType> &item) const {
        if(not item) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GContainerT::find(): "
                << "Tried to find an empty smart pointer." << std::endl
            );
        }
        return std::find_if(
            dataCnt_.begin(),
            dataCnt_.end(),
            [&item](const StoredType &contItem) -> bool {
                auto cast = std::dynamic_pointer_cast<ItemType>(contItem);
                return cast && (*item == *cast);
            }
        );
    }

    // ------------------------------------------------------------------
    // SharedPtrStorage-specific: push_back variants
    // ------------------------------------------------------------------

    /**
     * @brief Appends a cloned copy of @p itemPtr to the container.
     *
     * Changes to the original object after this call do not affect the stored copy.
     *
     * @param itemPtr A shared_ptr to the object to clone and append.
     * @throws geneva_exception when @p itemPtr is null.
     * @note Only available for SharedPtrStorage.
     */
    void pushBackClone(const StoredType &itemPtr)
        requires(!std::same_as<StoredType, ValueType>)
    {
        if(not itemPtr) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GContainerT::pushBackClone(): "
                << "Tried to clone an empty smart pointer." << std::endl
            );
        }
        dataCnt_.push_back(itemPtr->ValueType::template clone<ValueType>());
    }

    /**
     * @brief Appends @p itemPtr itself (shared ownership) to the container.
     *
     * Changes to the original object after this call will also be visible
     * through the stored pointer.
     *
     * @param itemPtr A shared_ptr to the object to append.
     * @throws geneva_exception when @p itemPtr is null.
     * @note Only available for SharedPtrStorage.
     */
    void pushBackNoclone(StoredType itemPtr)
        requires(!std::same_as<StoredType, ValueType>)
    {
        if(not itemPtr) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GContainerT::pushBackNoclone(): "
                << "Tried to insert an empty smart pointer." << std::endl
            );
        }
        dataCnt_.push_back(std::move(itemPtr));
    }

    // ------------------------------------------------------------------
    // SharedPtrStorage-specific: insert variants
    // ------------------------------------------------------------------

    /**
     * @brief Inserts a cloned copy of @p itemPtr before @p pos.
     *
     * @param pos     An iterator pointing to the insertion position.
     * @param itemPtr The object to clone and insert.
     * @return An iterator to the inserted element.
     * @throws geneva_exception when @p itemPtr is null.
     * @note Only available for SharedPtrStorage.
     */
    iterator insertClone(const_iterator pos, const StoredType &itemPtr)
        requires(!std::same_as<StoredType, ValueType>)
    {
        if(not itemPtr) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GContainerT::insertClone(): "
                << "Tried to clone an empty smart pointer." << std::endl
            );
        }
        return dataCnt_.insert(pos, itemPtr->ValueType::template clone<ValueType>());
    }

    /**
     * @brief Inserts @p count cloned copies of @p itemPtr before @p pos.
     *
     * @param pos     An iterator pointing to the insertion position.
     * @param count   The number of clones to insert.
     * @param itemPtr The object to clone and insert.
     * @throws geneva_exception when @p itemPtr is null.
     * @note Only available for SharedPtrStorage.
     */
    void insertClone(const_iterator pos, size_type count, const StoredType &itemPtr)
        requires(!std::same_as<StoredType, ValueType>)
    {
        if(not itemPtr) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GContainerT::insertClone(count): "
                << "Tried to clone an empty smart pointer." << std::endl
            );
        }
        std::size_t iterPos = static_cast<std::size_t>(pos - dataCnt_.begin());
        for(std::size_t i = 0; i < count; ++i) {
            dataCnt_.insert(
                dataCnt_.begin() + static_cast<difference_type>(iterPos),
                itemPtr->ValueType::template clone<ValueType>()
            );
        }
    }

    /**
     * @brief Inserts @p itemPtr itself (shared ownership) before @p pos.
     *
     * @param pos     An iterator pointing to the insertion position.
     * @param itemPtr The object to insert (not cloned).
     * @return An iterator to the inserted element.
     * @throws geneva_exception when @p itemPtr is null.
     * @note Only available for SharedPtrStorage.
     */
    iterator insertNoclone(const_iterator pos, StoredType itemPtr)
        requires(!std::same_as<StoredType, ValueType>)
    {
        if(not itemPtr) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GContainerT::insertNoclone(): "
                << "Tried to insert an empty smart pointer." << std::endl
            );
        }
        return dataCnt_.insert(pos, std::move(itemPtr));
    }

    /**
     * @brief Inserts @p count references to @p itemPtr (clones all but the last)
     *        before @p pos.
     *
     * Inserts (count-1) clones followed by @p itemPtr itself. This matches the
     * semantics of GPtrVectorT::insert_noclone(pos, amount, item).
     *
     * @param pos     An iterator pointing to the insertion position.
     * @param count   The number of elements to insert.
     * @param itemPtr The object to insert.
     * @throws geneva_exception when @p itemPtr is null.
     * @note Only available for SharedPtrStorage.
     */
    void insertNoclone(const_iterator pos, size_type count, StoredType itemPtr)
        requires(!std::same_as<StoredType, ValueType>)
    {
        if(not itemPtr) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GContainerT::insertNoclone(count): "
                << "Tried to insert an empty smart pointer." << std::endl
            );
        }
        std::size_t iterPos = static_cast<std::size_t>(pos - dataCnt_.begin());
        // Insert (count-1) clones
        for(std::size_t i = 0; i < count - 1; ++i) {
            dataCnt_.insert(
                dataCnt_.begin() + static_cast<difference_type>(iterPos),
                itemPtr->ValueType::template clone<ValueType>()
            );
        }
        // Insert the original
        dataCnt_.insert(
            dataCnt_.begin() + static_cast<difference_type>(iterPos),
            std::move(itemPtr)
        );
    }

    // ------------------------------------------------------------------
    // SharedPtrStorage-specific: resize variants
    // ------------------------------------------------------------------

    /**
     * @brief Resizes the container, filling new slots with clones of @p itemPtr.
     *
     * If the container shrinks, excess elements are removed. If it grows,
     * clones of @p itemPtr are appended.
     *
     * @param amount  The desired number of elements.
     * @param itemPtr The prototype object used to fill new slots.
     * @throws geneva_exception when growing and @p itemPtr is null.
     * @note Only available for SharedPtrStorage.
     */
    void resizeClone(size_type amount, const StoredType &itemPtr)
        requires(!std::same_as<StoredType, ValueType>)
    {
        std::size_t dataSize = dataCnt_.size();
        if(amount < dataSize) {
            dataCnt_.resize(amount);
        }
        else if(amount > dataSize) {
            if(not itemPtr) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, time_and_place)
                    << "In GContainerT::resizeClone(): "
                    << "Tried to clone an empty smart pointer." << std::endl
                );
            }
            for(std::size_t i = dataSize; i < amount; ++i) {
                dataCnt_.push_back(itemPtr->ValueType::template clone<ValueType>());
            }
        }
    }

    /**
     * @brief Resizes the container; the last new slot gets @p itemPtr itself,
     *        all others get clones.
     *
     * @param amount  The desired number of elements.
     * @param itemPtr The prototype object used to fill new slots.
     * @throws geneva_exception when growing and @p itemPtr is null.
     * @note Only available for SharedPtrStorage.
     */
    void resizeNoclone(size_type amount, StoredType itemPtr)
        requires(!std::same_as<StoredType, ValueType>)
    {
        std::size_t dataSize = dataCnt_.size();
        if(amount < dataSize) {
            dataCnt_.resize(amount);
        }
        else if(amount > dataSize) {
            if(not itemPtr) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, time_and_place)
                    << "In GContainerT::resizeNoclone(): "
                    << "Tried to insert an empty smart pointer." << std::endl
                );
            }
            for(std::size_t i = dataSize; i < amount - 1; ++i) {
                dataCnt_.push_back(itemPtr->ValueType::template clone<ValueType>());
            }
            dataCnt_.push_back(std::move(itemPtr));
        }
    }

    /**
     * @brief Resizes the container, filling new slots with empty (null) shared_ptrs.
     *
     * Intended for pre-allocation followed by manual assignment to each slot.
     *
     * @param amount The desired number of elements.
     * @note Only available for SharedPtrStorage.
     */
    void resizeEmpty(size_type amount)
        requires(!std::same_as<StoredType, ValueType>)
    {
        std::size_t dataSize = dataCnt_.size();
        if(amount < dataSize) {
            dataCnt_.resize(amount);
        }
        else {
            for(std::size_t i = dataSize; i < amount; ++i) {
                dataCnt_.push_back(StoredType{});
            }
        }
    }

    // ------------------------------------------------------------------
    // SharedPtrStorage-specific: cloneAt / attachViewTo / filteredView
    // ------------------------------------------------------------------

    /**
     * @brief Clones the element at index @p pos and returns it as a shared_ptr
     *        to @p TargetType.
     *
     * @tparam TargetType The desired result type (must be derived from ValueType).
     * @param pos The zero-based index.
     * @return A shared_ptr<TargetType> owning an independent clone.
     * @throws std::out_of_range when @p pos >= size().
     * @note Only available for SharedPtrStorage.
     */
    template <typename TargetType = ValueType>
    [[nodiscard]] std::shared_ptr<TargetType> cloneAt(std::size_t pos) const
        requires(!std::same_as<StoredType, ValueType>)
    {
        return dataCnt_.at(pos)->ValueType::template clone<TargetType>();
    }

    /**
     * @brief Appends shared_ptrs to all elements that are dynamically castable
     *        to @p DerivedType to the provided target vector.
     *
     * Elements that cannot be cast are silently skipped.
     *
     * @tparam DerivedType The derived type to filter for.
     * @param target A vector that receives the filtered pointers.
     * @note Only available for SharedPtrStorage.
     */
    template <typename DerivedType>
    void attachViewTo(std::vector<std::shared_ptr<DerivedType>> &target)
        requires(!std::same_as<StoredType, ValueType>)
    {
        for(auto &itemPtr : dataCnt_) {
            std::shared_ptr<DerivedType> cast = std::dynamic_pointer_cast<DerivedType>(itemPtr);
            if(cast) {
                target.push_back(std::move(cast));
            }
        }
    }

    /**
     * @brief Returns a lazy C++20 range view that yields only elements
     *        dynamically castable to @p DerivedType.
     *
     * The returned view is a composition of std::views::transform (attempting
     * the dynamic_cast) and std::views::filter (dropping null results).
     * A stateful iterator class was considered but the range-view approach is
     * preferred here because it composes naturally with other std::views
     * adaptors and avoids manual iterator bookkeeping.
     *
     * @tparam DerivedType The derived type to filter for.
     * @return A range of std::shared_ptr<DerivedType> (nulls excluded).
     * @note Only available for SharedPtrStorage.
     */
    template <typename DerivedType>
    [[nodiscard]] auto filteredView()
        requires(!std::same_as<StoredType, ValueType>)
    {
        return dataCnt_ | std::views::transform([](const StoredType &ptr) {
            return std::dynamic_pointer_cast<DerivedType>(ptr);
        }) | std::views::filter([](const std::shared_ptr<DerivedType> &ptr) {
            return static_cast<bool>(ptr);
        });
    }

    /**
     * @brief Returns a lazy const C++20 range view that yields only elements
     *        dynamically castable to @p DerivedType.
     *
     * @tparam DerivedType The derived type to filter for.
     * @return A const range of std::shared_ptr<DerivedType> (nulls excluded).
     * @note Only available for SharedPtrStorage.
     */
    template <typename DerivedType>
    [[nodiscard]] auto filteredView() const
        requires(!std::same_as<StoredType, ValueType>)
    {
        return dataCnt_ | std::views::transform([](const StoredType &ptr) {
            return std::dynamic_pointer_cast<DerivedType>(ptr);
        }) | std::views::filter([](const std::shared_ptr<DerivedType> &ptr) {
            return static_cast<bool>(ptr);
        });
    }

    // ------------------------------------------------------------------
    // SharedPtrStorage-specific: getDataCopy (deep clone)
    // ------------------------------------------------------------------

    /**
     * @brief Creates a deep copy of the internal data container into @p cp.
     *
     * For PodStorage, a plain assignment suffices.
     * For SharedPtrStorage, every element is cloned independently.
     *
     * @param cp The destination container. Its previous content is discarded.
     */
    void getDataCopy(ContainerType &cp) const
        requires std::same_as<StoredType, ValueType>
    {
        cp = dataCnt_;
    }

    /**
     * @brief Creates a deep copy of the internal data container into @p cp.
     *
     * Every element is individually cloned so that @p cp has no shared
     * ownership with this container.
     *
     * @param cp The destination container. Its previous content is discarded.
     * @note Only available for SharedPtrStorage.
     */
    void getDataCopy(ContainerType &cp) const
        requires(!std::same_as<StoredType, ValueType>)
    {
        cp.clear();
        for(const auto &item : dataCnt_) {
            cp.push_back(item->ValueType::template clone<ValueType>());
        }
    }

    // ------------------------------------------------------------------
    // Geneva-specific operations
    // ------------------------------------------------------------------

    /**
     * @brief Performs a genetic cross-over operation at index @p pos.
     *
     * Elements from index @p pos up to min(size(), cp.size())-1 are swapped
     * between the two containers. If the containers differ in length, the
     * excess elements of the longer container are moved to the shorter one,
     * making both containers change their sizes to reflect the exchange.
     *
     * @param cp  The partner container.
     * @param pos The starting position of the cross-over.
     * @note Only available when ContainerType satisfies HasRandomAccess.
     * @note In debug builds: throws when @p pos >= min(size(), cp.size()).
     */
    void crossOver(GContainerT &cp, const std::size_t &pos)
        requires HasRandomAccess<ContainerType>
    {
        std::size_t minSize = std::min(this->size(), cp.size());

#ifdef DEBUG
        if(pos >= minSize) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GContainerT::crossOver(): Error!" << std::endl
                << "Invalid position " << pos << " / " << this->size() << " / " << cp.size()
                << std::endl
            );
        }
#endif /* DEBUG */

        for(std::size_t i = pos; i < minSize; ++i) {
            std::swap(dataCnt_[i], cp.dataCnt_[i]);
        }

        if(this->size() > cp.size()) {
            for(std::size_t i = cp.size(); i < this->size(); ++i) {
                cp.dataCnt_.push_back(std::move(dataCnt_[i]));
            }
            dataCnt_.erase(dataCnt_.begin() + static_cast<difference_type>(minSize), dataCnt_.end());
        }
        else if(cp.size() > this->size()) {
            for(std::size_t i = this->size(); i < cp.size(); ++i) {
                dataCnt_.push_back(std::move(cp.dataCnt_[i]));
            }
            cp.dataCnt_.erase(
                cp.dataCnt_.begin() + static_cast<difference_type>(minSize), cp.dataCnt_.end()
            );
        }
    }

    /**
     * @brief Compares this container against @p cp according to expectation @p e.
     *
     * Uses the Geneva GToken / compare_t infrastructure so that results
     * participate in the standard reporting chain.
     *
     * @param cp    The container to compare against.
     * @param e     The expected outcome (EQUALITY, FP_SIMILARITY, INEQUALITY).
     * @param limit The tolerance used for floating-point similarity checks.
     * @throws g_expectation_violation when the comparison result violates @p e.
     */
    virtual void compareBase(
        const GContainerT &cp,
        Gem::Common::expectation e,
        double limit
    ) const {
        Gem::Common::GToken token("GContainerT", e);
        Gem::Common::compare_t(IDENTITY(this->dataCnt_, cp.dataCnt_), token);
        token.evaluate();
    }

    // ------------------------------------------------------------------
    // Comparison operator<=> (conditional)
    // ------------------------------------------------------------------

    /**
     * @brief Three-way lexicographic comparison.
     *
     * @note Only available when ContainerType satisfies HasRandomAccess and
     *       StoredType is three-way comparable.
     * @param other The container to compare against.
     * @return The result of std::lexicographical_compare_three_way.
     */
    [[nodiscard]] auto operator<=>(const GContainerT &other) const
        requires HasRandomAccess<ContainerType>
              && std::three_way_comparable<StoredType>
    {
        return std::lexicographical_compare_three_way(
            dataCnt_.begin(),
            dataCnt_.end(),
            other.dataCnt_.begin(),
            other.dataCnt_.end()
        );
    }

protected:
    // ------------------------------------------------------------------
    // Test hooks (protected virtual)
    // ------------------------------------------------------------------

    /**
     * @brief Applies modifications for unit-test purposes.
     *
     * Subclasses should override to introduce changes that make subsequent
     * equality checks fail.
     *
     * @return true if any modification was made, false otherwise.
     */
    virtual bool modifyGUnitTests_() {
        return false;
    }

    /**
     * @brief Runs self-tests that are expected to pass without throwing.
     *
     * Override in subclasses to add test logic that must succeed.
     */
    virtual void specificTestsNoFailureExpectedGUnitTests_() {}

    /**
     * @brief Runs self-tests that are expected to throw or trigger assertions.
     *
     * Override in subclasses to add test logic that must fail gracefully.
     */
    virtual void specificTestsFailuresExpectedGUnitTests_() {}

    ContainerType dataCnt_; ///< The underlying sequence container holding the data.
};

/******************************************************************************/
/**
 * @brief Out-of-line definition of the pure-virtual destructor.
 *
 * Clears the data container so that derived-class destructors run cleanly
 * even if the base destructor is invoked directly.
 */
template <typename T, typename StoragePolicy>
inline GContainerT<T, StoragePolicy>::~GContainerT() {
    dataCnt_.clear();
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/**
 * @brief Convenience alias: a GContainerT for POD types backed by @p Container.
 *
 * Equivalent to the old GPODVectorT but with an exchangeable backend.
 *
 * @tparam T         The POD element type.
 * @tparam Container The underlying sequence container. Defaults to std::vector<T>.
 */
template <typename T, typename Container = std::vector<T>>
using GPodContainer = GContainerT<T, PodStorage<T, Container>>;

/**
 * @brief Convenience alias: a GContainerT for polymorphic Geneva objects
 *        held via shared_ptr, backed by @p Container.
 *
 * Equivalent to the old GPtrVectorT but with an exchangeable backend.
 *
 * @tparam T         The base element type (must expose the Gemfony common interface).
 * @tparam Container The underlying sequence container.
 *                   Defaults to std::vector<std::shared_ptr<T>>.
 */
template <typename T, typename Container = std::vector<std::shared_ptr<T>>>
using GPtrContainer = GContainerT<T, SharedPtrStorage<T, Container>>;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Common */

/******************************************************************************/
/**
 * @brief Boost.Serialization abstract-type declarations for GContainerT.
 *
 * Required so that Boost.Serialization correctly handles the abstract base
 * class during serialisation of derived types.
 */
namespace boost::serialization {

template <typename T, typename StoragePolicy>
struct is_abstract<Gem::Common::GContainerT<T, StoragePolicy>> : public boost::true_type {};

template <typename T, typename StoragePolicy>
struct is_abstract<const Gem::Common::GContainerT<T, StoragePolicy>> : public boost::true_type {};

} /* namespace boost::serialization */

/******************************************************************************/
