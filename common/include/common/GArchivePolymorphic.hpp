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
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

// Boost headers go here

// Geneva headers go here
#include "common/GArchive.hpp"
#include "common/GBinaryArchive.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GJsonArchive.hpp"
#include "common/GPolymorphicRegistry.hpp"
#include "common/GReflectiveInterfaceT.hpp" // GReflectiveInterfaceAccess::construct (private-ctor factory seam)

namespace Gem::Common::archive {

/******************************************************************************/
/**
 * @file GArchivePolymorphic.hpp
 * @brief Polymorphic-pointer dispatch for the @c GArchive family -- the codec
 * layer that lets a @c GArchive (de)serialize a @c shared_ptr / @c unique_ptr to
 * a hierarchy root, reconstructing the exact dynamic type on load.
 *
 * @par Layering (why this is a separate header)
 * The value-codec base (@c GArchive.hpp) owns value dispatch and must stay free
 * of the registry and the codecs, or an include cycle forms
 * (@c GArchive -> registry -> codecs -> @c GArchive). Pointer dispatch instead
 * lives here, one layer up, where the registry (identity) and the concrete
 * codecs are both fully visible. The base reaches this layer only through the
 * @c gem_serialize_pointer customization point, found by ADL at instantiation --
 * so a translation unit that serializes a pointer must include this header (the
 * @c GEM_REGISTER_ARCHIVABLE sites and the serialization choke points do).
 *
 * @par Mechanism (registry-thunk model)
 * Identity is the codec-agnostic @c GPolymorphicRegistry (tag <-> factory).
 * Dispatch is @ref GArchivePointerDispatch, a per-hierarchy table mapping a tag
 * to four function-pointer thunks (one per concrete archive type) that each call
 * the concrete type's @c serialize through @c archive::access. A single
 * mechanism -- the registry -- carries identity; this table carries the typed
 * entry points a template @c serialize cannot expose as a virtual. Both are
 * populated together by @ref GEM_REGISTER_ARCHIVABLE.
 */
/******************************************************************************/

/**
 * @brief Per-hierarchy table of typed serialize thunks: for each registered
 * concrete type (keyed by its wire tag) it stores one @c void(*)(Root&, Archive&)
 * per concrete archive type, each invoking that type's @c serialize.
 * @tparam Root The category root of the hierarchy.
 */
template <typename Root>
class GArchivePointerDispatch {
public:
    struct entry_t {
        void (*bin_o)(Root &, GBinaryOArchive &);
        void (*bin_i)(Root &, GBinaryIArchive &);
        void (*json_o)(Root &, GJsonOArchive &);
        void (*json_i)(Root &, GJsonIArchive &);
    };

    /**
     * @brief Installs the four serialize thunks for concrete type @p Derived
     * under wire tag @p tag. Idempotent for a repeated identical registration.
     * @tparam Derived The concrete type (must derive from @p Root and be reachable via @c archive::access).
     * @param tag The wire tag (the same one used for the identity registry).
     */
    template <typename Derived>
    static void add(std::string_view tag) {
        static_assert(std::is_base_of_v<Root, Derived>,
                      "GArchivePointerDispatch::add<Derived>: Derived must derive from Root");
        entry_t e{
            +[](Root &r, GBinaryOArchive &ar) { access::serialize(ar, static_cast<Derived &>(r)); },
            +[](Root &r, GBinaryIArchive &ar) { access::serialize(ar, static_cast<Derived &>(r)); },
            +[](Root &r, GJsonOArchive &ar) { access::serialize(ar, static_cast<Derived &>(r)); },
            +[](Root &r, GJsonIArchive &ar) { access::serialize(ar, static_cast<Derived &>(r)); },
        };
        const std::scoped_lock lock{instance().mutex_};
        instance().map_.insert_or_assign(std::string{tag}, e);
    }

    /**
     * @brief Invokes the thunk for @p tag matching the concrete archive type of
     * @p ar, serializing the @p Root slice-and-below of @p r as its dynamic type.
     * @tparam Archive The concrete archive type (selects the thunk field).
     * @param tag The wire tag identifying the dynamic type.
     * @param r The object, upcast to @p Root.
     * @param ar The archive.
     * @throws geneva_exception if @p tag has no dispatch entry.
     */
    template <typename Archive>
    static void invoke(std::string_view tag, Root &r, Archive &ar) {
        const entry_t &e = lookup(tag);
        if constexpr (std::is_same_v<Archive, GBinaryOArchive>) {
            e.bin_o(r, ar);
        } else if constexpr (std::is_same_v<Archive, GBinaryIArchive>) {
            e.bin_i(r, ar);
        } else if constexpr (std::is_same_v<Archive, GJsonOArchive>) {
            e.json_o(r, ar);
        } else if constexpr (std::is_same_v<Archive, GJsonIArchive>) {
            e.json_i(r, ar);
        } else {
            static_assert(sizeof(Archive) == 0, "GArchivePointerDispatch::invoke: unknown archive type");
        }
    }

private:
    static const entry_t &lookup(std::string_view tag) {
        const std::scoped_lock lock{instance().mutex_};
        auto it = instance().map_.find(std::string{tag});
        if (it == instance().map_.end()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GArchivePointerDispatch::lookup(): no serialize thunk is registered under tag \"" << tag
                << "\" (its type is in the identity registry but was not registered for archive dispatch -- "
                << "use GEM_REGISTER_ARCHIVABLE, not the identity-only GEM_REGISTER_TYPE)." << '\n'
            );
        }
        return it->second;
    }

    GArchivePointerDispatch() = default;

    static GArchivePointerDispatch &instance() {
        static GArchivePointerDispatch *const inst = new GArchivePointerDispatch(); // leaked by design (see GPolymorphicRegistry)
        return *inst;
    }

    std::mutex mutex_;
    std::unordered_map<std::string, entry_t> map_;
};

/******************************************************************************/
/**
 * @brief The pointer-dispatch customization point the value-codec base calls by
 * ADL for a @c shared_ptr / @c unique_ptr member.
 *
 * On save: writes a present flag; if non-null, writes the dynamic type's wire
 * tag then serializes its members (as a nested object). On load: reads the
 * flag; if present, reads the tag, constructs the dynamic type from the identity
 * registry, loads its members, and adopts it into @p p. A null pointer is a
 * single false flag. There is no object tracking -- Geneva serializes trees.
 *
 * @tparam Archive The concrete archive type (its @c is_saving fixes the direction).
 * @tparam SmartPtr A @c shared_ptr or @c unique_ptr whose element type exposes @c gemfony_common_root_t.
 * @param ar The archive.
 * @param p The pointer to serialize or fill.
 */
template <typename Archive, typename SmartPtr>
void gem_serialize_pointer(Archive &ar, SmartPtr &p) {
    using T = typename SmartPtr::element_type;
    static_assert(requires { typename T::gemfony_common_root_t; },
                  "gem_serialize_pointer: the pointer's element type must be a GCommonInterfaceT hierarchy type "
                  "(exposing gemfony_common_root_t) to be dispatched polymorphically");
    using Root = typename T::gemfony_common_root_t;

    // The pointer serializes as ONE nested object {present[, tag, data]} so that,
    // in a self-describing codec, the several fields occupy a single member slot
    // rather than overwriting each other. In the binary codec the object/member
    // framing is free (no-ops) and this is just: flag [tag data].
    if constexpr (Archive::is_saving) {
        ar.begin_object();
        bool present = static_cast<bool>(p);
        ar.member("present");
        ar &present;
        if (present) {
            std::string tag = GPolymorphicRegistry<Root>::tagOf(static_cast<const Root &>(*p));
            ar.member("tag");
            ar &tag;
            ar.member("data");
            ar.begin_object();
            GArchivePointerDispatch<Root>::invoke(tag, static_cast<Root &>(*p), ar);
            ar.end_object();
        }
        ar.end_object();
    } else {
        ar.enter_object();
        bool present = false;
        ar.member("present");
        ar &present;
        if (!present) {
            p.reset();
            ar.leave_object();
            return;
        }
        std::string tag;
        ar.member("tag");
        ar &tag;
        std::unique_ptr<Root> obj = GPolymorphicRegistry<Root>::create(tag);
        ar.member("data");
        ar.enter_object();
        GArchivePointerDispatch<Root>::invoke(tag, *obj, ar);
        ar.leave_object();
        ar.leave_object();
        // The constructed object is dynamically a T-or-derived; downcast Root* -> T*
        // and adopt it into the requested smart-pointer flavour.
        T *raw = static_cast<T *>(obj.release());
        p = SmartPtr(raw);
    }
}

/******************************************************************************/
/**
 * @brief Registers concrete type @p T for @b both identity (the tag<->factory
 * @c GPolymorphicRegistry) and @c GArchive dispatch (the serialize thunks), the
 * full-registration counterpart of a @c BOOST_CLASS_EXPORT_IMPLEMENT. Prefer this
 * over the identity-only @c GEM_REGISTER_TYPE for any type that travels through a
 * @c GArchive as a polymorphic pointer.
 * @tparam T The concrete type to register.
 * @param tag The wire tag (fully-qualified stringized type).
 * @return @c true (a value to seed a namespace-scope registration variable).
 */
template <typename T>
inline bool register_archivable(std::string_view tag) {
    using Root = typename T::gemfony_common_root_t;
    if constexpr (std::is_default_constructible_v<T>) {
        // Public default constructor: the registry's own std::make_unique factory reaches it.
        GPolymorphicRegistry<Root>::template reg<T>(tag);
    } else {
        // Private default constructor (constructed only on load, through a friend, exactly as
        // Boost does): route the factory through GReflectiveInterfaceAccess -- the shim every
        // mixin-managed class befriends -- so the reconstruction has ctor access the (non-friend)
        // registration site lacks.
        GPolymorphicRegistry<Root>::reg(
            tag, std::type_index(typeid(T)),
            +[]() -> std::unique_ptr<Root> { return GReflectiveInterfaceAccess::template construct<T>(); });
    }
    GArchivePointerDispatch<Root>::template add<T>(tag);
    return true;
}

} // namespace Gem::Common::archive

/**
 * @brief Registers a concrete polymorphic type for identity + @c GArchive
 * dispatch (see @ref Gem::Common::archive::register_archivable). Place exactly
 * one at namespace scope in a single translation unit per type. Variadic so a
 * template instantiation with comma-separated arguments passes through intact.
 * @param ... The fully-qualified concrete type.
 */
#define GEM_REGISTER_ARCHIVABLE(...)                                                       \
    namespace {                                                                            \
    const bool GEM_REGISTRY_CAT(gem_archivable_tag_, __COUNTER__) =                        \
        ::Gem::Common::archive::register_archivable<__VA_ARGS__>(#__VA_ARGS__);            \
    }
