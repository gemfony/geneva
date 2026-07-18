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

/**
 * @file
 * @brief The one manifest a runtime-loadable Geneva module exports.
 *
 * A module (an individual, an optimization algorithm, a monitor or a consumer) exports a single C entry
 * point, @c geneva_module_manifest(), returning a @c GenevaModuleManifest. The manifest carries the
 * toolchain-compatibility fingerprint (`GenevaCompat`, validated FIRST -- see GBuildFingerprint.hpp) plus a
 * list of typed contributions. It is the ONLY module convention the loader accepts (the earlier
 * two-symbol individual convention has been retired).
 *
 * Everything crossing the module boundary here is **plain C** (fixed-width integers, `const char*`, and a
 * `void*(*)()` factory thunk): the loader must read @c compat before it can trust any C++ type, so the
 * manifest itself cannot depend on C++ layout. Each contribution's @c make_factory returns a `void*` that
 * the loader reinterprets per @c kind (for an individual, a heap-allocated content-creator factory pointer;
 * later kinds add OA / consumer factories).
 */

// Global checks, defines and includes needed for all of Geneva
#include "common/GBuildFingerprint.hpp" // GenevaCompat + GENEVA_BUILD_FINGERPRINT

// Standard headers go here
#include <cstddef>
#include <cstdint>

// Boost headers go here
#include <boost/config.hpp> // BOOST_SYMBOL_EXPORT

/******************************************************************************/
/*
 * Contribution kinds. A module may contribute several things (e.g. an OA plus its personality traits plus a
 * bundled monitor, or an individual plus its GPU marshaller). INDIVIDUAL, OA and MARSHALLER are wired by the
 * loader today; MONITOR and CONSUMER are reserved and added as their categories are modularised.
 */
#define GENEVA_CONTRIBUTION_INDIVIDUAL 1u
#define GENEVA_CONTRIBUTION_OA 2u
#define GENEVA_CONTRIBUTION_MONITOR 3u
#define GENEVA_CONTRIBUTION_CONSUMER 4u
#define GENEVA_CONTRIBUTION_MARSHALLER 5u

/* Two-level stringization, and the Geneva version as a "MAJOR.MINOR.PATCH" string literal (a module's own
 * version string when it is a Geneva-shipped module). */
#define GENEVA_MODULE_STRINGIZE_(x) #x
#define GENEVA_MODULE_STRINGIZE(x) GENEVA_MODULE_STRINGIZE_(x)
#define GENEVA_VERSION_STRING                                                                            \
    GENEVA_MODULE_STRINGIZE(GENEVA_VERSION_MAJOR)                                                         \
    "." GENEVA_MODULE_STRINGIZE(GENEVA_VERSION_MINOR) "." GENEVA_MODULE_STRINGIZE(GENEVA_VERSION_PATCH)

#ifdef __cplusplus
extern "C" {
#endif

/** @brief One thing a module provides: a typed factory keyed by name/mnemonic. Plain C. */
typedef struct GenevaContribution {
    std::uint32_t kind;             /* GENEVA_CONTRIBUTION_*                                     */
    const char *name_or_mnemonic;   /* NUL-terminated; the mnemonic/name the host resolves by    */
    void *(*make_factory)(void);    /* returns the base factory ptr for `kind` (loader casts it)  */
} GenevaContribution;

/** @brief The single manifest a module exports via geneva_module_manifest(). Plain C. */
typedef struct GenevaModuleManifest {
    GenevaCompat compat;                     /* validated FIRST, before any C++ is touched        */
    const char *module_name;                 /* diagnostics + (later) checkpoint self-description */
    const char *module_version;              /* the module's own version string                   */
    const GenevaContribution *contributions; /* array of length contributions_count               */
    std::uint32_t contributions_count;
} GenevaModuleManifest;

#ifdef __cplusplus
} /* extern "C" */
#endif

namespace Gem::Common {

/** @brief The (unmangled) name of the manifest entry point the loader resolves. */
inline constexpr const char *GENEVA_MODULE_MANIFEST_SYMBOL = "geneva_module_manifest";

/** @brief Signature of the manifest entry point (used by the loader's typed symbol lookup). */
using geneva_module_manifest_fn = const GenevaModuleManifest *();

/******************************************************************************/
/**
 * @brief A structural, compile-time fixed string usable as a non-type template parameter (C++20/23).
 *
 * Lets the manifest author helpers (individualManifest() / oaManifest()) take the config path and
 * module/contribution name as template arguments (string literals), so a loadable module's manifest is an
 * ordinary typed C++ construct rather than a preprocessor macro. Shared by every kind's helper.
 */
template <std::size_t N>
struct GFixedString {
    char value[N]{};
    // NOLINTNEXTLINE(google-explicit-constructor) -- implicit from a string literal is the whole point.
    constexpr GFixedString(const char (&str)[N]) {
        for(std::size_t i = 0; i < N; ++i) { value[i] = str[i]; }
    }
    [[nodiscard]] constexpr const char *c_str() const noexcept { return value; }
};

} // namespace Gem::Common

/******************************************************************************/
/*
 * A loadable module publishes its manifest by hand-writing the fixed entry point (no Geneva macro):
 *
 *   extern "C" BOOST_SYMBOL_EXPORT const GenevaModuleManifest *geneva_module_manifest() {
 *       return Gem::Geneva::individualManifest<Factory, "config path", "Name">();  // for an individual
 *   }
 *
 * The extern "C" wrapper is irreducible (the loader resolves the fixed, unmangled symbol via dlsym); the
 * manifest itself is built by a typed helper (individualManifest() for individuals). A pointer return keeps
 * the extern "C" free of the class-return warning.
 */

/******************************************************************************/
