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
#include <cmath>
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <vector>

// Boost headers go here

// Geneva headers go here
#include "common/GCommonHelperFunctions.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GTypeTraitsT.hpp"

namespace Gem::Common {

/******************************************************************************/
// Helper for narrow: maps an enum to its underlying integer; otherwise
// passes the type through unchanged. std::conditional_t cannot be used
// directly because it eagerly evaluates both branches and
// std::underlying_type<int> is ill-formed.
namespace detail {
template <typename T, bool = std::is_enum_v<T>>
struct enum_or_self { using type = T; };
template <typename T>
struct enum_or_self<T, true> { using type = std::underlying_type_t<T>; };
template <typename T>
using enum_or_self_t = typename enum_or_self<T>::type;
} // namespace detail

/******************************************************************************/
/**
 * Checked numeric/enum cast. Returns the converted value, or throws
 * std::overflow_error if a narrowing conversion would change it. Range
 * validation always happens *before* the value-changing static_cast, so an
 * out-of-range input never reaches a cast whose result would be undefined
 * behaviour (e.g. float->int or float->float out of range).
 *
 * @note Naming and semantics follow the C++ Core Guidelines / GSL: this is the
 * *checked* narrowing cast, i.e. the counterpart of `gsl::narrow<T>(x)` (which
 * throws when the round-tripped value differs). It is deliberately NOT named
 * `narrow_cast`: in the GSL `gsl::narrow_cast` is merely an *unchecked*
 * `static_cast` marker, so that name would advertise the opposite of what this
 * does. We keep our own implementation rather than depend on the GSL -- it
 * additionally supports enum source/target types, throws std::overflow_error
 * (instead of introducing a gsl::narrowing_error type), and avoids pulling the
 * Microsoft.GSL library in as an extra Geneva dependency. It also supersedes
 * the previously used boost::numeric_cast.
 *
 * What is checked, by target type:
 *   - integer / enum target: rejected unless the value fits (round-trip for
 *     integer<->integer, an exact boundary check for float->integer). Enum
 *     source/target types are handled via their underlying integer type.
 *     Float->integer truncates toward zero (no fractional-part check) but is
 *     rejected if outside the target integer range.
 *   - floating target from a *wider* floating type: a finite value whose
 *     magnitude exceeds the destination range is rejected (it would overflow
 *     to +/-inf). inf/NaN pass through unchanged; precision loss is inherent
 *     and is NOT treated as an error.
 *   - floating target from an integer/enum or a not-wider floating type:
 *     always in range, hence unchecked (may lose precision).
 */
template <typename To, typename From>
To narrow(From value) {
    static_assert((std::is_arithmetic_v<To> || std::is_enum_v<To>) &&
                  (std::is_arithmetic_v<From> || std::is_enum_v<From>),
                  "narrow requires arithmetic or enum types");

    // For enum source/target types fall back to their underlying integer
    // representation so the same range checks apply. Without this, casts
    // like `narrow<gColor>(some_int)` silently degenerated to plain
    // static_cast<gColor>(...) and dropped all bounds checking.
    using ToCheck   = detail::enum_or_self_t<To>;
    using FromCheck = detail::enum_or_self_t<From>;

    const auto check_value = static_cast<FromCheck>(value);

    if constexpr (std::is_integral_v<ToCheck> && std::is_integral_v<FromCheck>) {
        // Integer-to-integer (and enum-via-underlying): a round-trip on the
        // underlying integer types catches narrowing in both directions. It is
        // done purely on the integers (never forming a To-typed result first),
        // so no out-of-range enum value is ever materialised; integral
        // conversions are well-defined, so this is never UB.
        if(static_cast<FromCheck>(static_cast<ToCheck>(check_value)) != check_value) {
            throw std::overflow_error("narrow: integer overflow or underflow");
        }
        return static_cast<To>(value);
    } else if constexpr (std::is_integral_v<ToCheck> &&
                         std::is_floating_point_v<FromCheck>) {
        // Float-to-integer: validate the source range BEFORE the float->int
        // static_cast, which is undefined behaviour for out-of-range inputs.
        // The obvious `value > ToCheck::max()` check is unsafe at the int64
        // extreme: int64_t::max() == 2^63 - 1 is NOT exactly representable in
        // double; static_cast<double>(int64_max) rounds UP to 2^63, so a value
        // equal to 2^63 would pass and static_cast<int64_t>(2^63) is UB.
        //
        // Detect whether the max-as-float round-tripped exactly by checking
        // whether the float distance between adjacent integer endpoints is
        // exactly 1: it is when no rounding occurred (e.g. int32 -> double)
        // and < 1 when adjacent integers collapsed onto the same float
        // (int64 -> double). When inexact, reject `==` at the boundary too.
        constexpr ToCheck   to_max                 = std::numeric_limits<ToCheck>::max();
        constexpr ToCheck   to_min                 = std::numeric_limits<ToCheck>::min();
        constexpr FromCheck max_as_from            = static_cast<FromCheck>(to_max);
        constexpr FromCheck max_minus_one_as_from  = static_cast<FromCheck>(to_max - 1);
        constexpr FromCheck min_as_from            = static_cast<FromCheck>(to_min);
        constexpr bool      max_is_exact           =
            (max_as_from - max_minus_one_as_from) == FromCheck{1};

        if(check_value < min_as_from) {
            throw std::overflow_error("narrow: float-to-integer overflow");
        }
        if constexpr (max_is_exact) {
            // ToCheck::max() round-trips exactly, so equality is legitimate.
            if(check_value > max_as_from) {
                throw std::overflow_error("narrow: float-to-integer overflow");
            }
        } else {
            // Rounding pushed max_as_from above ToCheck::max(): equality
            // would convert to UB territory, so reject it as well.
            if(check_value >= max_as_from) {
                throw std::overflow_error("narrow: float-to-integer overflow");
            }
        }
        return static_cast<To>(value); // provably in range now
    } else if constexpr (std::is_floating_point_v<ToCheck> &&
                         std::is_floating_point_v<FromCheck>) {
        // Floating-to-floating. Only a *narrowing* conversion (destination
        // range strictly smaller than the source) can overflow. Reject a
        // finite value whose magnitude exceeds the destination range BEFORE the
        // cast, since an out-of-range floating conversion is undefined
        // behaviour. inf/NaN pass through unchanged; precision loss is inherent
        // and is NOT treated as an error.
        if constexpr (std::numeric_limits<ToCheck>::max() <
                      std::numeric_limits<FromCheck>::max()) {
            if(std::isfinite(check_value)) {
                // Widening the (smaller) destination max into FromCheck is
                // exact, so this comparison is well-defined.
                constexpr FromCheck to_max =
                    static_cast<FromCheck>(std::numeric_limits<ToCheck>::max());
                if(check_value > to_max || check_value < -to_max) {
                    throw std::overflow_error("narrow: floating-point overflow");
                }
            }
        }
        return static_cast<To>(value);
    } else {
        // Integer/enum -> floating: always in range (may lose precision, which
        // is inherent and not treated as an error).
        return static_cast<To>(value);
    }
}

/******************************************************************************/
/**
 * Generates a UUID v4 string (e.g. "550e8400-e29b-41d4-a716-446655440000").
 * Uses a thread-local Mersenne-Twister seeded from std::random_device.
 */
inline std::string generate_uuid_v4() {
    static thread_local std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<std::uint64_t> dist;
    std::uint64_t hi = dist(rng);
    std::uint64_t lo = dist(rng);
    // Set version 4 (nibble at bits 15-12 of `hi`, i.e. the 3rd group nibble)
    hi = (hi & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
    // Set variant 10xx (top 2 bits of lo's most-significant byte)
    lo = (lo & 0xBFFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;
    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(8) << static_cast<std::uint32_t>(hi >> 32) << '-'
        << std::setw(4) << static_cast<std::uint32_t>((hi >> 16) & 0xFFFFU) << '-'
        << std::setw(4) << static_cast<std::uint32_t>(hi & 0xFFFFU) << '-'
        << std::setw(4) << static_cast<std::uint32_t>(lo >> 48) << '-'
        << std::setw(12) << (lo & 0x0000FFFFFFFFFFFFULL);
    return oss.str();
}

/******************************************************************************/
/** @brief Converts a string to target_type via stream extraction (replaces boost::lexical_cast) */
template <typename T>
T from_string(const std::string &s) {
    T val;
    std::istringstream iss(s);
    iss >> val;
    return val;
}

/******************************************************************************/
/**
 * Reads a given environment variable and converts it to a target type. The
 * function requires that target_type is extractable from an istringstream.
 *
 * @param var The name of the environment variable to be read
 * @return The converted environment variable, or an empty optional
 */
template <typename target_type>
std::optional<target_type> environmentVariableAs(std::string const &var) {
    std::string result_str; // NOLINT(cppcoreguidelines-init-variables)

    {
        // std::getenv is not thread-safe; serialise access with a local mutex.
        static std::mutex read_env_mutex;
        std::scoped_lock lk(read_env_mutex);

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
        char *env_ptr = 0;
        size_t sz = 0;
        if(0 == _dupenv_s(&env_ptr, &sz, var.c_str()) && nullptr != env_ptr) {
            result_str = std::string(env_ptr);
            free(env_ptr);
        }
        else {
            return {};
        }
#else
        const char *env_ptr = std::getenv(
            var.c_str()
        ); // NOLINT(concurrency-mt-unsafe) — called under lock; Geneva never calls putenv/setenv from threads
        if(env_ptr) {
            result_str = std::string(env_ptr);
        }
        else {
            return {};
        }
#endif
    } // releases the lock

    auto ltrim = result_str.find_first_not_of(" \t\r\n");
    auto rtrim = result_str.find_last_not_of(" \t\r\n");
    if(ltrim != std::string::npos) {
        result_str = result_str.substr(ltrim, rtrim - ltrim + 1);
    }
    else {
        result_str.clear();
    }
    return {Gem::Common::from_string<target_type>(result_str)};
}

/******************************************************************************/
/**
 * Null-safe delete, then sets the pointer to nullptr.
 */
template <typename T>
void g_delete(T *&p) {
    if(p) {
        delete p;
        p = nullptr;
    }
}

/******************************************************************************/
/**
 * Null-safe array delete, then sets the pointer to nullptr.
 */
template <typename T>
void g_array_delete(T *&p) {
    if(p) {
        delete[] p;
        p = nullptr;
    }
}

/******************************************************************************/
/**
 * In debug builds, throws if two raw pointers alias the same object.
 * No-op for nullptr p1.
 */
template <typename T>
void ptrDifferenceCheck(const T *p1, const T *p2) {
#ifdef DEBUG
    if(nullptr != p1 && p1 == p2) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Gem::Common::ptrDifferenceCheck<T>(): "
            << "p1 and p2 point to the same object!" << '\n'
        );
    }
#endif
}

/******************************************************************************/
/**
 * Shared-pointer overload: in debug builds, throws if both non-null shared
 * pointers alias the same object.
 */
template <typename T>
void ptrDifferenceCheck(std::shared_ptr<T> p1, std::shared_ptr<T> p2) {
#ifdef DEBUG
    if(p1 && p1.get() == p2.get()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Gem::Common::ptrDifferenceCheck<T>(): "
            << "Smart pointers p1 and p2 point to the same object!" << '\n'
        );
    }
#endif
}

/******************************************************************************/
/**
 * Converts a raw base pointer to target_type*. The `requires` clause restricts
 * the template to *downcasts* (target_type must derive from base_type), so
 * this is a checked dynamic_cast in DEBUG and an unchecked static_cast in
 * release. Returns nullptr unchanged. Throws in DEBUG on a failed cast.
 * (The previous comment "upcasts only" was inverted — a derived-from base
 * cast is a downcast.)
 */
template <typename base_type, typename target_type>
    requires std::derived_from<target_type, base_type>
const target_type *g_ptr_conversion(const base_type *convert_ptr) {
#ifdef DEBUG
    const auto *p = dynamic_cast<const target_type *>(convert_ptr);
    if(nullptr == convert_ptr || p) {
        return p;
    }
    throw geneva_exception(
        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
        << "In g_ptr_conversion(): invalid conversion from " << typeid(base_type).name() << " to "
        << typeid(target_type).name() << '\n'
    );
    return nullptr;
#else
    return static_cast<const target_type *>(convert_ptr);
#endif
}

/******************************************************************************/
/**
 * Shared-pointer overload of g_ptr_conversion.
 */
template <typename base_type, typename target_type>
    requires std::derived_from<target_type, base_type>
std::shared_ptr<target_type> g_ptr_conversion(std::shared_ptr<base_type> convert_ptr) {
#ifdef DEBUG
    auto p = std::dynamic_pointer_cast<target_type>(convert_ptr);
    if(nullptr == convert_ptr.get() || p) {
        return p;
    }
    throw geneva_exception(
        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
        << "In g_ptr_conversion(): invalid conversion from " << typeid(base_type).name() << " to "
        << typeid(target_type).name() << '\n'
    );
#else
    return std::static_pointer_cast<target_type>(convert_ptr);
#endif
}

/******************************************************************************/
/**
 * Converts convert_ptr to target_type and checks it does not alias
 * compare_ptr. Only accessible when base_type is a base of target_type.
 */
template <typename base_type, typename target_type>
    requires std::derived_from<target_type, base_type>
std::shared_ptr<target_type> g_convert_and_compare(
    std::shared_ptr<base_type> convert_ptr,
    std::shared_ptr<target_type> compare_ptr
) {
    auto p = g_ptr_conversion<base_type, target_type>(convert_ptr);
    ptrDifferenceCheck(p, compare_ptr);
    return p;
}

/******************************************************************************/
/**
 * Raw-pointer overload of g_convert_and_compare.
 */
template <typename base_type, typename target_type>
    requires std::derived_from<target_type, base_type>
const target_type *g_convert_and_compare(
    const base_type *convert_ptr,
    const target_type *compare_ptr
) {
    const target_type *p = g_ptr_conversion<base_type, target_type>(convert_ptr);
    ptrDifferenceCheck(p, compare_ptr);
    return p;
}

/******************************************************************************/
/**
 * Reference overload of g_convert_and_compare.
 */
template <typename base_type, typename target_type>
    requires std::derived_from<target_type, base_type>
const target_type *g_convert_and_compare(
    const base_type &convert_ref,
    const target_type *compare_ptr
) {
    const auto *p = g_ptr_conversion<base_type, target_type>(&convert_ref);
    ptrDifferenceCheck(p, compare_ptr);
    return p;
}

/******************************************************************************/
/**
 * Returns a space-separated string representation of a std::vector.
 * T must be streamable.
 */
template <typename T>
std::string vecToString(const std::vector<T> &vec) {
    std::ostringstream result; // NOLINT(cppcoreguidelines-init-variables)
    for(const auto &item : vec) {
        result << item << " ";
    }
    return result.str();
}

/******************************************************************************/
/**
 * Deep-copies a shared_ptr to a cloneable/loadable object using clone()/load().
 */
template <typename T>
    requires Gem::Common::gemfony_common_interface<T>
void copyCloneableSmartPointer(const std::shared_ptr<T> &from, std::shared_ptr<T> &to) {
    if(not from) {
        to.reset();
    }
    else if(not to or typeid(*to) != typeid(*from)) {
        // Either nothing to load into, or the existing target has a different
        // dynamic type than the source (e.g. polymorphic container slots whose
        // element types differ after a structural change). load() assumes a
        // matching concrete type, so in both cases deep-clone the source instead
        // of loading into a type-incompatible target.
        to = from->T::template clone<T>();
    }
    else {
        to->T::load(from);
    }
}

/******************************************************************************/
/**
 * unique_ptr counterpart of copyCloneableSmartPointer(): deep-copies a single cloneable object held
 * in a unique_ptr, loading in place when the dynamic types match (no allocation) and deep-cloning
 * otherwise. No atomic reference counting is involved.
 */
template <typename T>
    requires Gem::Common::gemfony_common_interface<T>
void copyCloneableSmartPointer(const std::unique_ptr<T> &from, std::unique_ptr<T> &to) {
    if(not from) {
        to.reset();
    }
    else if(not to or typeid(*to) != typeid(*from)) {
        to = from->template clone_unique<T>();
    }
    else {
        to->T::load(*from);
    }
}

/******************************************************************************/
/**
 * Wraps a uniquely-owned object in a NON-OWNING std::shared_ptr (a no-op deleter), for APIs that still
 * take a const std::shared_ptr<T>& but only read through it. The unique_ptr retains sole ownership; the
 * returned shared_ptr must not outlive it. Lets unique_ptr-owned objects be passed to such read-only
 * shared_ptr APIs without changing those APIs or co-owning the object.
 */
template <typename T>
std::shared_ptr<T> nonOwningShared(const std::unique_ptr<T> &p) {
    return std::shared_ptr<T>(p.get(), [](T *) { /* non-owning: do not delete */ });
}

/******************************************************************************/
/**
 * Deep-copies a container of shared_ptrs to cloneable objects using
 * clone()/load(). Resizes the target container as needed.
 */
template <typename T, template <typename, typename> class c_type>
    requires Gem::Common::gemfony_common_interface<T>
void copyCloneableSmartPointerContainer(
    const c_type<std::shared_ptr<T>, std::allocator<std::shared_ptr<T>>> &from,
    c_type<std::shared_ptr<T>, std::allocator<std::shared_ptr<T>>> &to
) {
    using iter_t =
        typename c_type<std::shared_ptr<T>, std::allocator<std::shared_ptr<T>>>::iterator;
    using const_iter_t =
        typename c_type<std::shared_ptr<T>, std::allocator<std::shared_ptr<T>>>::const_iterator;

    const std::size_t size_from = from.size();
    const std::size_t size_to = to.size();

    if(size_from == size_to) {
        const_iter_t it_from = from.begin();
        for(iter_t it_to = to.begin(); it_to != to.end(); ++it_from, ++it_to) {
            copyCloneableSmartPointer(*it_from, *it_to);
        }
    }
    else if(size_from > size_to) {
        const_iter_t it_from = from.begin();
        for(iter_t it_to = to.begin(); it_to != to.end(); ++it_from, ++it_to) {
            copyCloneableSmartPointer(*it_from, *it_to);
        }
        for(const_iter_t it = from.begin() + size_to; it != from.end(); ++it) {
            to.push_back((*it)->T::template clone<T>());
        }
    }
    else { // size_from < size_to
        const_iter_t it_from = from.begin();
        for(iter_t it_to = to.begin(); it_from != from.end(); ++it_from, ++it_to) {
            copyCloneableSmartPointer(*it_from, *it_to);
        }
        to.resize(size_from);
    }
}

/******************************************************************************/
/**
 * unique_ptr counterpart of copyCloneableSmartPointerContainer(): deep-copies a container of
 * unique_ptrs to cloneable objects via clone_unique()/load(), reusing existing slots when sizes
 * match (load in place) and resizing the target as needed. No atomic reference counting.
 */
template <typename T, template <typename, typename> class c_type>
    requires Gem::Common::gemfony_common_interface<T>
void copyCloneableSmartPointerContainer(
    const c_type<std::unique_ptr<T>, std::allocator<std::unique_ptr<T>>> &from,
    c_type<std::unique_ptr<T>, std::allocator<std::unique_ptr<T>>> &to
) {
    using iter_t =
        typename c_type<std::unique_ptr<T>, std::allocator<std::unique_ptr<T>>>::iterator;
    using const_iter_t =
        typename c_type<std::unique_ptr<T>, std::allocator<std::unique_ptr<T>>>::const_iterator;

    const std::size_t size_from = from.size();
    const std::size_t size_to = to.size();

    if(size_from == size_to) {
        const_iter_t it_from = from.begin();
        for(iter_t it_to = to.begin(); it_to != to.end(); ++it_from, ++it_to) {
            copyCloneableSmartPointer(*it_from, *it_to);
        }
    }
    else if(size_from > size_to) {
        const_iter_t it_from = from.begin();
        for(iter_t it_to = to.begin(); it_to != to.end(); ++it_from, ++it_to) {
            copyCloneableSmartPointer(*it_from, *it_to);
        }
        for(const_iter_t it = from.begin() + size_to; it != from.end(); ++it) {
            to.push_back((*it)->template clone_unique<T>());
        }
    }
    else { // size_from < size_to
        const_iter_t it_from = from.begin();
        for(iter_t it_to = to.begin(); it_from != from.end(); ++it_from, ++it_to) {
            copyCloneableSmartPointer(*it_from, *it_to);
        }
        to.resize(size_from);
    }
}

/******************************************************************************/
/**
 * Deep-copies a container of cloneable objects using load(). Resizes the
 * target container as needed.
 */
template <typename T, template <typename, typename> class c_type>
    requires Gem::Common::gemfony_common_interface<T>
void copyCloneableObjectsContainer(
    const c_type<T, std::allocator<T>> &from,
    c_type<T, std::allocator<T>> &to
) {
    using iter_t = typename c_type<T, std::allocator<T>>::iterator;
    using const_iter_t = typename c_type<T, std::allocator<T>>::const_iterator;

    const std::size_t size_from = from.size();
    const std::size_t size_to = to.size();

    if(size_from == size_to) {
        const_iter_t it_from = from.begin();
        for(iter_t it_to = to.begin(); it_from != from.end(); ++it_from, ++it_to) {
            it_to->T::load(*it_from);
        }
    }
    else if(size_from > size_to) {
        const_iter_t it_from = from.begin();
        for(iter_t it_to = to.begin(); it_to != to.end(); ++it_from, ++it_to) {
            it_to->T::load(*it_from);
        }
        for(const_iter_t it = from.begin() + size_to; it != from.end(); ++it) {
            to.push_back(T(*it));
        }
    }
    else { // size_from < size_to
        const_iter_t it_from = from.begin();
        for(iter_t it_to = to.begin(); it_from != from.end(); ++it_from, ++it_to) {
            it_to->T::load(*it_from);
        }
        to.resize(size_from);
    }
}

/******************************************************************************/
/**
 * Copies a raw array into another raw array, allocating or reallocating the
 * destination as needed. Both size parameters are kept consistent.
 */
template <typename T>
void copyArrays(T const *const from, T *&to, const std::size_t &n_from, std::size_t &n_to) {
    if(nullptr == from && 0 != n_from) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In copyArrays(): from is null but n_from=" << n_from << '\n'
        );
    }
    if(nullptr != from && 0 == n_from) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In copyArrays(): from is non-null but n_from=0" << '\n'
        );
    }
    if(nullptr == to && 0 != n_to) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In copyArrays(): to is null but nTo=" << n_to << '\n'
        );
    }
    if(nullptr != to && 0 == n_to) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In copyArrays(): to is non-null but nTo=0" << '\n'
        );
    }

    if(nullptr == from) {
        n_to = 0;
        if(to) {
            g_array_delete(to);
        }
        return;
    }

    if(n_from != n_to) {
        if(to) {
            g_array_delete(to);
        }
        to = new T[n_from];
        n_to = n_from;
    }

    for(std::size_t i = 0; i < n_from; i++) {
        to[i] = from[i];
    }
}

/******************************************************************************/
/**
 * Deep-copies a raw array of shared_ptrs into another, allocating or
 * reallocating the destination as needed.
 */
template <typename T>
void copySmartPointerArrays(
    std::shared_ptr<T> const *const from,
    std::shared_ptr<T> *&to,
    const std::size_t &size_from,
    std::size_t &size_to
) {
    if(nullptr == from && 0 != size_from) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In copySmartPointerArrays(): from is null but size_from=" << size_from << '\n'
        );
    }
    if(nullptr != from && 0 == size_from) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In copySmartPointerArrays(): from is non-null but size_from=0" << '\n'
        );
    }
    if(nullptr == to && 0 != size_to) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In copySmartPointerArrays(): to is null but size_to=" << size_to << '\n'
        );
    }
    if(nullptr != to && 0 == size_to) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In copySmartPointerArrays(): to is non-null but size_to=0" << '\n'
        );
    }

    if(size_from != size_to) {
        for(std::size_t i = 0; i < size_to; i++) {
            to[i].reset();
        }
        g_array_delete(to);
        to = new std::shared_ptr<T>[size_from];
        size_to = size_from;
    }

    for(std::size_t i = 0; i < size_to; i++) {
        to[i] = std::make_shared<T>(*(from[i]));
    }
}

/******************************************************************************/
/**
 * Converts a shared_ptr to target_type. In debug builds uses dynamic_pointer_cast
 * and throws on failure or null input; in release builds uses static_pointer_cast.
 */
template <typename source_type, typename target_type>
std::shared_ptr<target_type> convertSmartPointer(std::shared_ptr<source_type> p_raw) {
#ifdef DEBUG
    if(not p_raw) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In convertSmartPointer(): pointer is empty." << '\n'
        );
    }
    auto p = std::dynamic_pointer_cast<target_type>(p_raw);
    if(p) {
        return p;
    }
    throw geneva_exception(
        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
        << "In convertSmartPointer(): invalid conversion to " << typeid(target_type).name()
        << '\n'
    );
#else
    return std::static_pointer_cast<target_type>(p_raw);
#endif
}

/******************************************************************************/
/**
 * Splits a string into a vector of target_type values using a single separator.
 */
template <typename split_type>
std::vector<split_type> splitStringT(const std::string &raw, const char *sep) {
    std::vector<split_type> result;
    for(const auto &fragment : Gem::Common::splitString(raw, sep)) {
        result.push_back(Gem::Common::from_string<split_type>(fragment));
    }
    return result;
}

/******************************************************************************/
/**
 * Splits a string into a vector of (split_type1, split_type2) pairs using
 * two different separators. A possible usage: "0/0 0/1 1/0" → tuples of ints.
 */
template <typename split_type1, typename split_type2>
std::vector<std::tuple<split_type1, split_type2>>
splitStringT(const std::string &raw, const char *sep1, const char *sep2) {
    if(std::string(sep1) == std::string(sep2)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In splitStringT(): sep1 and sep2 are identical: \"" << sep1 << "\" / \"" << sep2
            << "\"" << '\n'
        );
    }

    std::vector<std::tuple<split_type1, split_type2>> result;
    for(const auto &fragment : Gem::Common::splitString(raw, sep1)) {
        const auto sub = Gem::Common::splitString(fragment, sep2);
#ifdef DEBUG
        if(2 != sub.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In splitStringT(): expected 2 sub-fragments, got " << sub.size() << '\n'
            );
        }
#endif
        result.emplace_back(
            Gem::Common::from_string<split_type1>(sub[0]),
            Gem::Common::from_string<split_type2>(sub[1])
        );
    }
    return result;
}

/******************************************************************************/
/**
 * Returns a reference to the value at key in m; throws if the map is empty
 * or the key is absent.
 */
template <typename item_type>
item_type &getMapItem(std::map<std::string, item_type> &m, const std::string &key) {
    if(m.empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace()) << "In getMapItem(): map is empty" << '\n'
        );
    }
    auto it = m.find(key);
    if(it != m.end()) {
        return it->second;
    }
    throw geneva_exception(
        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
        << "In getMapItem(): key \"" << key << "\" not found" << '\n'
    );
}

/******************************************************************************/
/**
 * Const overload of getMapItem.
 */
template <typename item_type>
const item_type &getMapItem(const std::map<std::string, item_type> &m, const std::string &key) {
    if(m.empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace()) << "In getMapItem(): map is empty" << '\n'
        );
    }
    auto cit = m.find(key);
    if(cit != m.end()) {
        return cit->second;
    }
    throw geneva_exception(
        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
        << "In getMapItem(): key \"" << key << "\" not found" << '\n'
    );
}

/******************************************************************************/
/**
 * Adds an operator== to every object with a Gemfony-common interface
 */
template <class gemfony_common_type>
    requires Gem::Common::gemfony_common_interface<gemfony_common_type>
bool operator==(const gemfony_common_type &x, const gemfony_common_type &y) {
    try {
        x.compare(y, Gem::Common::expectation::EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
        return true;
    }
    catch(g_expectation_violation &) {
        return false;
    }
}

/******************************************************************************/
/**
 * Adds an operator!= to every object with a Gemfony-common interface
 */
template <class gemfony_common_type>
    requires Gem::Common::gemfony_common_interface<gemfony_common_type>
bool operator!=(const gemfony_common_type &x, const gemfony_common_type &y) {
    try {
        x.compare(y, Gem::Common::expectation::INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
        return true;
    }
    catch(g_expectation_violation &) {
        return false;
    }
}

/******************************************************************************/
/**
 * Converts integral types (except scoped enums) to std::string.
 */
template <typename integral_type>
    requires (std::is_integral_v<integral_type> ||
              (std::is_enum_v<integral_type> && std::is_convertible_v<integral_type, int>))
std::string to_string(integral_type val) {
    return std::to_string(val);
}

/******************************************************************************/
/**
 * Converts floating-point values to std::string with full precision.
 */
template <std::floating_point fp_type>
std::string to_string(fp_type val) {
    std::ostringstream oss;
    oss << std::setprecision(std::numeric_limits<fp_type>::max_digits10) << val;
    return oss.str();
}

/******************************************************************************/
/**
 * Converts a scoped enum (enum class) to std::string via uint32_t cast.
 */
template <typename enum_type>
    requires (std::is_enum_v<enum_type> && !std::is_convertible_v<enum_type, int>)
std::string to_string(enum_type val) {
    return std::to_string(static_cast<std::uint32_t>(val));
}

/******************************************************************************/
/**
 * Converts any remaining streamable type to std::string via ostringstream.
 */
template <typename default_type>
    requires (!std::is_enum_v<default_type> && !std::is_arithmetic_v<default_type>)
std::string to_string(default_type val) {
    std::ostringstream oss;
    oss << val;
    return oss.str();
}

/******************************************************************************/
/**
 * Erases elements from a standard container matching a predicate. Equivalent
 * to C++20 std::erase_if, kept here for CUDA nvcc compatibility (nvcc does not
 * expose the C++20 standard-library additions). Returns the number of erased
 * elements.
 */
template <typename container_type, typename predicate_type>
std::size_t erase_if(container_type &container, const predicate_type &predicate) {
    std::size_t n_erased = 0;
    for(auto it = container.begin(); it != container.end();) {
        if(predicate(*it)) {
            it = container.erase(it);
            ++n_erased;
        }
        else {
            ++it;
        }
    }
    return n_erased;
}

/******************************************************************************/

} /* namespace Gem::Common */
