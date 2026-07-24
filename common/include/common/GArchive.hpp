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
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <list>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// Boost headers go here

// Geneva headers go here

namespace Gem::Common::archive {

/******************************************************************************/
/**
 * @file GArchive.hpp
 * @brief The structural core of the @c GArchive serialization family: the
 * @c nvp / @c base_object wrappers, the container/scalar type traits, and the
 * two CRTP dispatch bases (@ref GOArchiveT saving, @ref GIArchiveT loading)
 * that walk an object's members and hand primitives to a concrete codec.
 *
 * @par Design
 * A concrete codec (e.g. @c GBinaryOArchive, @c GJsonOArchive) derives from
 * @ref GOArchiveT / @ref GIArchiveT and implements a small, fixed set of
 * primitive and framing hooks (scalars, strings, object/sequence/element
 * delimiters, member keys). The CRTP base owns @b all type analysis -- it is
 * the single place that knows how a @c std::vector differs from a @c std::map
 * differs from a nested serializable class -- and expresses each as calls to
 * those hooks. This keeps the two codecs free of structural logic: the binary
 * codec makes the framing hooks no-ops and writes a flat byte stream; the JSON
 * codec makes them build a @c boost::json tree keyed by member name.
 *
 * @par Interface parity with Boost.Serialization
 * The reflection layer (@c GMemberReflectionT.hpp) reaches an archive through
 * exactly two spellings: @c "ar & make_nvp(name, ref)" and
 * @c "ar & make_nvp(name, base_object<Base>(derived))". This family mirrors
 * both -- @ref make_nvp and @ref base_object here have the same shape -- so
 * retargeting a class's generated @c serialize from a Boost archive to a
 * @c GArchive is a change of the archive @e type only, not of the call sites.
 *
 * @par Trees, not graphs
 * Geneva serializes trees without shared-object tracking (see the
 * Boost-replacement design doc): a cloneable member is deep-cloned on load, not
 * reconstructed as a shared node. This family therefore carries @b no
 * object-identity / de-duplication machinery. Polymorphic-pointer dispatch
 * (through @c GPolymorphicRegistry, requiring a small set of virtual serialize
 * hooks on the interface) is intentionally @b not part of this value-codec
 * layer; it arrives with the mixin integration step.
 */
/******************************************************************************/

/**
 * @brief A name/value pair: binds a member reference to the name a self-describing
 * codec (JSON) uses as its key; a nameless codec (binary) ignores the name.
 * @tparam T The (possibly const) referent type.
 */
template <typename T>
struct nvp {
    const char *name;
    T &value;
};

/** @brief Builds an @ref nvp. @param name The member name. @param value The member reference. */
template <typename T>
nvp<T> make_nvp(const char *name, T &value) {
    return nvp<T>{name, value};
}

/**
 * @brief Wraps a derived object so that only its @p Base slice is serialized in
 * place (the archive analogue of @c boost::serialization::base_object).
 * @tparam Base The base slice to serialize.
 * @tparam Derived The concrete object providing that slice.
 */
template <typename Base, typename Derived>
struct base_object_t {
    Derived &ref;
};

/** @brief Builds a @ref base_object_t. @param derived The derived object whose Base slice to serialize. */
template <typename Base, typename Derived>
base_object_t<Base, Derived> base_object(Derived &derived) {
    static_assert(std::is_base_of_v<Base, Derived>, "base_object<Base>(d): Base must be a base of decltype(d)");
    return base_object_t<Base, Derived>{derived};
}

/**
 * @brief Access shim invoking a class's (often private) @c serialize member.
 * A serializable class grants access with
 * @c "friend struct Gem::Common::archive::access;".
 */
struct access {
    /** @brief Invokes @c obj.serialize(ar, 0). @param ar The archive. @param obj The object to (de)serialize. */
    template <typename Archive, typename T>
    static void serialize(Archive &ar, T &obj) {
        obj.serialize(ar, 0u);
    }
};

/******************************************************************************/
// Type traits identifying the finite set of value shapes the codecs handle.

namespace detail {

template <typename T>
struct is_pair : std::false_type {};
template <typename A, typename B>
struct is_pair<std::pair<A, B>> : std::true_type {};

template <typename T>
struct is_tuple : std::false_type {};
template <typename... Ts>
struct is_tuple<std::tuple<Ts...>> : std::true_type {};

template <typename T>
struct is_std_array : std::false_type {};
template <typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template <typename T>
struct is_map : std::false_type {};
template <typename K, typename V, typename... R>
struct is_map<std::map<K, V, R...>> : std::true_type {};
template <typename K, typename V, typename... R>
struct is_map<std::unordered_map<K, V, R...>> : std::true_type {};

// Sequence and set-like containers: a size, a value_type, iterable, and
// (for load) clearable + able to grow one element at a time.
template <typename T>
struct is_sequence : std::false_type {};
template <typename T, typename... R>
struct is_sequence<std::vector<T, R...>> : std::true_type {};
template <typename T, typename... R>
struct is_sequence<std::deque<T, R...>> : std::true_type {};
template <typename T, typename... R>
struct is_sequence<std::list<T, R...>> : std::true_type {};
template <typename T, typename... R>
struct is_sequence<std::set<T, R...>> : std::true_type {};
template <typename T, typename... R>
struct is_sequence<std::unordered_set<T, R...>> : std::true_type {};

template <typename T>
inline constexpr bool is_string_v = std::is_same_v<T, std::string>;

template <typename T>
inline constexpr bool is_path_v = std::is_same_v<T, std::filesystem::path>;

template <typename T>
struct is_atomic : std::false_type {};
template <typename T>
struct is_atomic<std::atomic<T>> : std::true_type {};

// A "value class" is anything not covered above that exposes a serialize member
// through the access shim (checked structurally so a private serialize + friend
// access still qualifies).
template <typename Archive, typename T>
concept serializable_class = requires(Archive &ar, T &t) { access::serialize(ar, t); };

// std::set / std::unordered_set grow via insert(), std::vector/deque/list via
// push_back(); distinguish so the load base picks the right insertion.
template <typename T>
inline constexpr bool is_set_like_v = false;
template <typename T, typename... R>
inline constexpr bool is_set_like_v<std::set<T, R...>> = true;
template <typename T, typename... R>
inline constexpr bool is_set_like_v<std::unordered_set<T, R...>> = true;

} // namespace detail

/******************************************************************************/
/**
 * @brief CRTP dispatch base for a @b saving (output) archive.
 *
 * Implements @c operator& for @ref nvp, @ref base_object_t, and arbitrary
 * values, decomposing each value into calls on the concrete codec @p Derived.
 * @p Derived must provide the saving hooks:
 *   - @c put_bool(bool) / @c put_scalar(Integral) / @c put_fp(FloatingPoint)
 *   - @c put_string(std::string_view)
 *   - @c begin_object() / @c end_object() / @c member(const char*)
 *   - @c begin_seq(std::size_t) / @c end_seq() / @c begin_elem() / @c end_elem()
 *
 * @tparam Derived The concrete saving codec (CRTP).
 */
template <typename Derived>
class GOArchiveT {
public:
    static constexpr bool is_saving = true;
    static constexpr bool is_loading = false;

    /** @brief Serializes a named member. @param n The name/value pair. */
    template <typename T>
    Derived &operator&(const nvp<T> &n) {
        d().member(n.name);
        return process(n.value);
    }

    /** @brief Serializes only the @p Base slice of a derived object. @param b The base-object wrapper. */
    template <typename Base, typename Der>
    Derived &operator&(const base_object_t<Base, Der> &b) {
        access::serialize(d(), static_cast<Base &>(b.ref));
        return d();
    }

    /** @brief Serializes an unnamed value. @param v The value to save. */
    template <typename T>
    Derived &operator&(const T &v) {
        return process(const_cast<T &>(v)); // save never mutates; const_cast lets one serialize() serve both directions
    }

    /** @brief Boost-style save alias. @param v The value to save. */
    template <typename T>
    Derived &operator<<(const T &v) {
        return *this & v;
    }

private:
    Derived &d() { return static_cast<Derived &>(*this); }

    template <typename T>
    Derived &process(T &v) {
        using U = std::remove_cv_t<T>;
        if constexpr (std::is_same_v<U, bool>) {
            d().put_bool(v);
        } else if constexpr (std::is_enum_v<U>) {
            auto raw = static_cast<std::underlying_type_t<U>>(v);
            d().put_scalar(raw);
        } else if constexpr (std::is_integral_v<U>) {
            d().put_scalar(v);
        } else if constexpr (std::is_floating_point_v<U>) {
            d().put_fp(v);
        } else if constexpr (detail::is_string_v<U>) {
            d().put_string(std::string_view{v});
        } else if constexpr (detail::is_path_v<U>) {
            d().put_string(v.string());
        } else if constexpr (detail::is_atomic<U>::value) {
            typename U::value_type held = v.load();
            process(held);
        } else if constexpr (detail::is_pair<U>::value) {
            d().begin_object();
            d().member("first");
            process(v.first);
            d().member("second");
            process(v.second);
            d().end_object();
        } else if constexpr (detail::is_tuple<U>::value) {
            d().begin_seq(std::tuple_size_v<U>);
            std::apply(
                [&](auto &...elems) {
                    ((d().begin_elem(), process(elems), d().end_elem()), ...);
                },
                v);
            d().end_seq();
        } else if constexpr (detail::is_std_array<U>::value) {
            d().begin_seq(v.size());
            for (auto &e : v) {
                d().begin_elem();
                process(e);
                d().end_elem();
            }
            d().end_seq();
        } else if constexpr (detail::is_map<U>::value) {
            d().begin_seq(v.size());
            for (auto &kv : v) {
                d().begin_elem();
                d().begin_object();
                d().member("key");
                process(kv.first);
                d().member("value");
                process(kv.second);
                d().end_object();
                d().end_elem();
            }
            d().end_seq();
        } else if constexpr (detail::is_sequence<U>::value) {
            d().begin_seq(v.size());
            for (auto &e : v) {
                d().begin_elem();
                process(e);
                d().end_elem();
            }
            d().end_seq();
        } else {
            static_assert(detail::serializable_class<Derived, U>,
                          "GOArchiveT: type is neither a supported primitive/container nor a serializable class "
                          "(needs a serialize(Archive&, unsigned) reachable via archive::access)");
            d().begin_object();
            access::serialize(d(), v);
            d().end_object();
        }
        return d();
    }
};

/******************************************************************************/
/**
 * @brief CRTP dispatch base for a @b loading (input) archive.
 *
 * The mirror of @ref GOArchiveT. @p Derived must provide the loading hooks:
 *   - @c get_bool(bool&) / @c get_scalar(Integral&) / @c get_fp(FloatingPoint&)
 *   - @c get_string(std::string&)
 *   - @c enter_object() / @c leave_object() / @c member(const char*)
 *   - @c begin_seq() -> std::size_t / @c end_seq() / @c begin_elem() / @c end_elem()
 *
 * @tparam Derived The concrete loading codec (CRTP).
 */
template <typename Derived>
class GIArchiveT {
public:
    static constexpr bool is_saving = false;
    static constexpr bool is_loading = true;

    /** @brief Loads a named member. @param n The name/value pair to fill. */
    template <typename T>
    Derived &operator&(const nvp<T> &n) {
        d().member(n.name);
        return process(n.value);
    }

    /** @brief Loads only the @p Base slice of a derived object. @param b The base-object wrapper. */
    template <typename Base, typename Der>
    Derived &operator&(const base_object_t<Base, Der> &b) {
        access::serialize(d(), static_cast<Base &>(b.ref));
        return d();
    }

    /** @brief Loads an unnamed value. @param v The value to fill. */
    template <typename T>
    Derived &operator&(T &v) {
        return process(v);
    }

    /** @brief Boost-style load alias. @param v The value to fill. */
    template <typename T>
    Derived &operator>>(T &v) {
        return *this & v;
    }

private:
    Derived &d() { return static_cast<Derived &>(*this); }

    template <typename T>
    Derived &process(T &v) {
        using U = std::remove_cv_t<T>;
        if constexpr (std::is_same_v<U, bool>) {
            d().get_bool(v);
        } else if constexpr (std::is_enum_v<U>) {
            std::underlying_type_t<U> raw{};
            d().get_scalar(raw);
            v = static_cast<U>(raw);
        } else if constexpr (std::is_integral_v<U>) {
            d().get_scalar(v);
        } else if constexpr (std::is_floating_point_v<U>) {
            d().get_fp(v);
        } else if constexpr (detail::is_string_v<U>) {
            d().get_string(v);
        } else if constexpr (detail::is_path_v<U>) {
            std::string s;
            d().get_string(s);
            v = std::filesystem::path{s};
        } else if constexpr (detail::is_atomic<U>::value) {
            typename U::value_type held{};
            process(held);
            v.store(held);
        } else if constexpr (detail::is_pair<U>::value) {
            d().enter_object();
            d().member("first");
            process(const_cast<std::remove_const_t<typename U::first_type> &>(v.first));
            d().member("second");
            process(v.second);
            d().leave_object();
        } else if constexpr (detail::is_tuple<U>::value) {
            d().begin_seq();
            std::apply(
                [&](auto &...elems) {
                    ((d().begin_elem(), process(elems), d().end_elem()), ...);
                },
                v);
            d().end_seq();
        } else if constexpr (detail::is_std_array<U>::value) {
            d().begin_seq();
            for (auto &e : v) {
                d().begin_elem();
                process(e);
                d().end_elem();
            }
            d().end_seq();
        } else if constexpr (detail::is_map<U>::value) {
            std::size_t n = d().begin_seq();
            v.clear();
            for (std::size_t i = 0; i < n; ++i) {
                d().begin_elem();
                d().enter_object();
                typename U::key_type key{};
                typename U::mapped_type mapped{};
                d().member("key");
                process(key);
                d().member("value");
                process(mapped);
                d().leave_object();
                d().end_elem();
                v.emplace(std::move(key), std::move(mapped));
            }
            d().end_seq();
        } else if constexpr (detail::is_sequence<U>::value) {
            std::size_t n = d().begin_seq();
            v.clear();
            for (std::size_t i = 0; i < n; ++i) {
                d().begin_elem();
                typename U::value_type elem{};
                process(elem);
                d().end_elem();
                if constexpr (detail::is_set_like_v<U>) {
                    v.insert(std::move(elem));
                } else {
                    v.push_back(std::move(elem));
                }
            }
            d().end_seq();
        } else {
            static_assert(detail::serializable_class<Derived, U>,
                          "GIArchiveT: type is neither a supported primitive/container nor a serializable class "
                          "(needs a serialize(Archive&, unsigned) reachable via archive::access)");
            d().enter_object();
            access::serialize(d(), v);
            d().leave_object();
        }
        return d();
    }
};

/******************************************************************************/

} // namespace Gem::Common::archive

/**
 * @brief Serializes a named member, deducing the name from the identifier.
 * The @c GArchive analogue of @c "ar & BOOST_SERIALIZATION_NVP(x)".
 */
#define GEM_NVP(member) ::Gem::Common::archive::make_nvp(#member, member)
