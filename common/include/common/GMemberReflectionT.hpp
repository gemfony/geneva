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
#include <atomic>
#include <tuple>
#include <utility>
#include <type_traits>

// Boost headers go here
#include <boost/serialization/nvp.hpp>

// Gemfony headers go here
#include "common/GCommonHelperFunctionsT.hpp"

namespace Gem::Common {

/******************************************************************************/
/**
 * Single-source-of-truth machinery for a GCommonInterfaceT subclass's local data members.
 *
 * A class declares its local members exactly once through a localMembers() pair
 * (const + non-const) that returns a tuple of member_t entries (a name plus a
 * reference to the member). load_() and compare_() then derive their behaviour
 * from that single declaration via g_load_members() / g_compare_members(),
 * instead of each function enumerating the members separately. This removes the
 * "added a member but forgot to update load_()/compare_()" class of bugs.
 *
 * This header holds the member-reflection factories (make_member etc.) plus the
 * load and serialize derivations. The matching compare derivation lives in
 * GExpectationChecksT.hpp (it depends on the comparison DSL), which includes
 * this header.
 *
 * serialize() is intentionally NOT derived from this for the wire format
 * by default -- see prompts/2026-05-25-serialize-single-source-followup.md.
 *
 * @tparam T The type of the referenced member
 */
template <typename T>
struct member_t {
    const char* name;
    T &ref; // T& in a non-const context, T const& in a const context
};

/**
 * @brief Builds one named member reference for a localMembers() tuple.
 * @tparam T The (deduced) type of the referenced member
 * @param name The member's name, used as the serialization NVP tag (a static string literal)
 * @param ref A reference to the data member being described
 * @return A member_t pairing @p name with @p ref
 */
template <typename T>
member_t<T> make_member(const char* name, T &ref) {
    return member_t<T>{name, ref};
}

/******************************************************************************/
/**
 * Tagged member variants for members whose in-memory copy is a deep clone rather
 * than a plain assignment. They carry the same { name, ref } shape as member_t -- so
 * serialize_members() and g_compare_members() treat them identically (a shared_ptr
 * and a container of shared_ptr each have their own serialize / compare support) --
 * but g_load_members() recognises the tag and performs a deep clone instead of a
 * shallow pointer assignment (which would alias shared state).
 *
 *  - cloneable_member_t           : a single std::shared_ptr<Cloneable> member,
 *                                   deep-copied via copyCloneableSmartPointer().
 *  - cloneable_container_member_t : a container (e.g. std::vector) of
 *                                   std::shared_ptr<Cloneable>, deep-copied via
 *                                   copyCloneableSmartPointerContainer().
 *
 * This lets a class list ALL of its data members -- plain and deep-cloned alike -- in
 * one localMembers() declaration, so serialize()/load_()/compare_() all derive from
 * the same single source with no hand-written tail.
 *
 * @tparam T The type of the referenced std::shared_ptr<Cloneable> member
 */
template <typename T>
struct cloneable_member_t {
    const char* name;
    T &ref; ///< a std::shared_ptr<Cloneable> (const& in a const context)
};

/**
 * @brief A tagged member variant for a container of std::shared_ptr<Cloneable>, deep-cloned on load.
 * @tparam T The type of the referenced container of std::shared_ptr<Cloneable>
 */
template <typename T>
struct cloneable_container_member_t {
    const char* name;
    T &ref; ///< a container of std::shared_ptr<Cloneable> (const& in a const context)
};

/**
 * An atomic member (e.g. std::atomic<bool>). Boost already serialises std::atomic<bool>
 * via a free serialization, so serialize_members() handles it through the common .ref
 * path; but an atomic is neither copy-assignable nor directly comparable through the
 * generic value path, so g_load_members() loads it via .store(.load()) and
 * g_compare_members() compares its loaded value.
 *
 * @tparam T The type of the referenced std::atomic<...> member
 */
template <typename T>
struct atomic_member_t {
    const char* name;
    T &ref; ///< a std::atomic<...> (const& in a const context)
};

/**
 * @brief Builds one named, deep-cloned single-pointer member for a localMembers() tuple.
 * @tparam T The (deduced) type of the referenced std::shared_ptr<Cloneable> member
 * @param name The member's name, used as the serialization NVP tag (a static string literal)
 * @param ref A reference to the std::shared_ptr<Cloneable> data member
 * @return A cloneable_member_t pairing @p name with @p ref
 */
template <typename T>
cloneable_member_t<T> make_cloneable_member(const char* name, T &ref) {
    return cloneable_member_t<T>{name, ref};
}

/**
 * @brief Builds one named, deep-cloned pointer-container member for a localMembers() tuple.
 * @tparam T The (deduced) type of the referenced pointer-container member
 * @param name The member's name, used as the serialization NVP tag (a static string literal)
 * @param ref A reference to the container of std::shared_ptr<Cloneable>
 * @return A cloneable_container_member_t pairing @p name with @p ref
 */
template <typename T>
cloneable_container_member_t<T> make_cloneable_container_member(const char* name, T &ref) {
    return cloneable_container_member_t<T>{name, ref};
}

/**
 * @brief Builds one named atomic member for a localMembers() tuple.
 * @tparam T The (deduced) type of the referenced std::atomic<...> member
 * @param name The member's name, used as the serialization NVP tag (a static string literal)
 * @param ref A reference to the std::atomic<...> data member
 * @return An atomic_member_t pairing @p name with @p ref
 */
template <typename T>
atomic_member_t<T> make_atomic_member(const char* name, T &ref) {
    return atomic_member_t<T>{name, ref};
}

/******************************************************************************/
/**
 * @brief Loads a single plain member by assignment.
 * @tparam Dst The destination member type
 * @tparam Src The source member type
 * @param dst The destination member descriptor (written to)
 * @param src The source member descriptor (read from)
 */
template <typename Dst, typename Src>
void g_load_one(member_t<Dst> &dst, const member_t<Src> &src) {
    dst.ref = src.ref; // plain assignment
}
/**
 * @brief Loads a single std::shared_ptr<Cloneable> member by deep clone (no pointer aliasing).
 * @tparam Dst The destination member type
 * @tparam Src The source member type
 * @param dst The destination member descriptor (written to)
 * @param src The source member descriptor (read from)
 */
template <typename Dst, typename Src>
void g_load_one(cloneable_member_t<Dst> &dst, const cloneable_member_t<Src> &src) {
    Gem::Common::copyCloneableSmartPointer(src.ref, dst.ref); // deep clone
}
/**
 * @brief Loads a container-of-pointers member by deep-cloning each element.
 * @tparam Dst The destination member type
 * @tparam Src The source member type
 * @param dst The destination member descriptor (written to)
 * @param src The source member descriptor (read from)
 */
template <typename Dst, typename Src>
void g_load_one(cloneable_container_member_t<Dst> &dst, const cloneable_container_member_t<Src> &src) {
    Gem::Common::copyCloneableSmartPointerContainer(src.ref, dst.ref); // deep clone of each element
}
/**
 * @brief Loads a single atomic member via load/store (atomics are not copy-assignable).
 * @tparam Dst The destination member type
 * @tparam Src The source member type
 * @param dst The destination member descriptor (written to)
 * @param src The source member descriptor (read from)
 */
template <typename Dst, typename Src>
void g_load_one(atomic_member_t<Dst> &dst, const atomic_member_t<Src> &src) {
    dst.ref.store(src.ref.load()); // atomic load/store (atomics are not copy-assignable)
}

/**
 * @brief Loads each member of the two tuples pairwise (implementation helper).
 * @tparam DstTuple The destination localMembers() tuple type
 * @tparam SrcTuple The source localMembers() tuple type
 * @tparam I The compile-time member indices expanded by the fold
 * @param dst The destination member tuple (written to)
 * @param src The source member tuple (read from)
 * @param  Index sequence used to expand the pack; its value is unused
 */
template <typename DstTuple, typename SrcTuple, std::size_t... I>
void g_load_members_impl(DstTuple &dst, const SrcTuple &src, std::index_sequence<I...>) {
    (g_load_one(std::get<I>(dst), std::get<I>(src)), ...);
}

/**
 * @brief Copies each local member from src to dst, member by member.
 * @tparam DstTuple The destination localMembers() tuple type
 * @tparam SrcTuple The source localMembers() tuple type
 * @param dst The destination member tuple (taken by value; its references write through)
 * @param src The source member tuple (taken by value; its references read through)
 */
template <typename DstTuple, typename SrcTuple>
void g_load_members(DstTuple dst, SrcTuple src) {
    static_assert(
        std::tuple_size_v<DstTuple> == std::tuple_size_v<SrcTuple>,
        "g_load_members: localMembers() arity mismatch"
    );
    g_load_members_impl(dst, src, std::make_index_sequence<std::tuple_size_v<DstTuple>>{});
}

/******************************************************************************/
/**
 * Serializes each local member through the Boost archive, using the member's
 * name (from its member_t) as the NVP tag. This lets a class's serialize()
 * derive its member list from the same single localMembers() declaration that
 * load_() and compare_() already use, keeping the member list in one place.
 *
 * The member names are compile-time string literals (const char*) with static
 * storage duration, so the pointers handed to make_nvp are always valid.
 *
 * @tparam Archive The Boost.Serialization archive type
 * @tparam Tuple The localMembers() tuple type
 * @tparam I The compile-time member indices expanded by the fold
 * @param ar The archive to serialize through
 * @param members The localMembers() tuple whose entries are serialized
 * @param  Index sequence used to expand the pack; its value is unused
 */
template <typename Archive, typename Tuple, std::size_t... I>
void serialize_members_impl(Archive& ar, Tuple& members, std::index_sequence<I...>) {
    ((ar & boost::serialization::make_nvp(std::get<I>(members).name, std::get<I>(members).ref)), ...);
}
/**
 * @brief Serializes every entry of a localMembers() tuple through the archive, name by name.
 * @tparam Archive The Boost.Serialization archive type
 * @tparam Tuple The localMembers() tuple type
 * @param ar The archive to serialize through
 * @param members The localMembers() tuple whose entries are serialized (taken by value)
 */
template <typename Archive, typename Tuple>
void serialize_members(Archive& ar, Tuple members) {
    serialize_members_impl(ar, members, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
}

/******************************************************************************/

} /* namespace Gem::Common */
