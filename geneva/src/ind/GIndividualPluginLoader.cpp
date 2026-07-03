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

#include "geneva/ind/GIndividualPluginLoader.hpp"

// Standard headers go here
#include <cstdint>
#include <mutex>
#include <vector>

// Boost headers go here
#include <boost/dll/shared_library.hpp>

// Geneva headers go here
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"

namespace Gem::Geneva {

namespace {

/******************************************************************************/
/**
 * A process-lifetime store of loaded plugin handles. A loaded .so backs live individuals AND their
 * Boost.Serialization type registrations, so it must never be unloaded while the program runs; the store
 * is therefore intentionally never cleared. Guarded by a mutex so concurrent loads are safe (loads are
 * rare -- once at startup -- so a plain mutex is ample).
 */
std::mutex g_plugin_mutex;

std::vector<boost::dll::shared_library> &keptPlugins_() {
    static std::vector<boost::dll::shared_library> libs;
    return libs;
}

} // namespace

/******************************************************************************/

GIndividualFactoryPtr loadIndividualPlugin(const std::filesystem::path &plugin_path) {
    namespace dll = boost::dll;
    const std::string path_str = plugin_path.string();

    std::scoped_lock lock(g_plugin_mutex);

    // Load the shared object. RTLD_GLOBAL keeps ONE symbol namespace (single Boost serialization registry
    // + single Geneva singletons); RTLD_NOW resolves eagerly so a broken plugin fails here, not later.
    dll::shared_library lib;
    try {
        // boost::dll uses its own filesystem path type (boost::dll::fs::path, which may be
        // boost::filesystem::path), so convert from std::filesystem::path via the native string.
        lib.load(dll::fs::path(path_str), dll::load_mode::rtld_global | dll::load_mode::rtld_now);
    }
    catch(const std::exception &e) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Gem::Geneva::loadIndividualPlugin(): Error!" << '\n'
            << "Could not load the individual plugin '" << path_str << "':" << '\n'
            << e.what() << '\n'
        );
    }

    // ABI gate. Boost.DLL performs no compatibility check itself; its only free failure mode is a missing
    // symbol. So we require the ABI marker and compare it explicitly, BEFORE constructing any individual.
    if(not lib.has(GENEVA_INDIVIDUAL_ABI_SYMBOL)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Gem::Geneva::loadIndividualPlugin(): Error!" << '\n'
            << "'" << path_str << "' is not a Geneva individual plugin: it exports no '"
            << GENEVA_INDIVIDUAL_ABI_SYMBOL << "' marker" << '\n'
            << "(it may predate the plugin ABI scheme, or was built without GENEVA_INDIVIDUAL_PLUGIN())."
            << '\n'
        );
    }
    const auto plugin_abi =
        lib.get<geneva_individual_abi_version_fn>(GENEVA_INDIVIDUAL_ABI_SYMBOL)();
    if(plugin_abi != static_cast<std::uint32_t>(GENEVA_VERSION)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Gem::Geneva::loadIndividualPlugin(): Error!" << '\n'
            << "The individual plugin '" << path_str << "' was built against Geneva version "
            << plugin_abi << ',' << '\n'
            << "but this Geneva is version " << static_cast<std::uint32_t>(GENEVA_VERSION) << '.' << '\n'
            << "The two are not guaranteed compatible; rebuild the plugin against this Geneva." << '\n'
        );
    }

    // Fetch the factory entry point and construct the content creator.
    if(not lib.has(GENEVA_INDIVIDUAL_FACTORY_SYMBOL)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Gem::Geneva::loadIndividualPlugin(): Error!" << '\n'
            << "The individual plugin '" << path_str << "' exports no '"
            << GENEVA_INDIVIDUAL_FACTORY_SYMBOL << "' factory entry point." << '\n'
        );
    }
    GIndividualFactoryPtr factory =
        lib.get<geneva_individual_factory_fn>(GENEVA_INDIVIDUAL_FACTORY_SYMBOL)();
    if(not factory) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In Gem::Geneva::loadIndividualPlugin(): Error!" << '\n'
            << "The individual plugin '" << path_str << "' returned an empty factory." << '\n'
        );
    }

    // Keep the library resident for the process lifetime (its code + type registrations back live objects).
    keptPlugins_().push_back(std::move(lib));
    return factory;
}

/******************************************************************************/

} /* namespace Gem::Geneva */
