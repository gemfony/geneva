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

// Boost header files go here

// Geneva headers go here
#include "common/GGlobalOptionsT.hpp"
#include "common/GProviderT.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/oa/GOAFactoryT.hpp"

// A global store for optimization-algorithm providers. Each provider wraps a
// config-file-driven factory (see GOAFactoryProviderT in GInitializerT.hpp) and
// produces a freshly configured algorithm on every provide() call.
using GOAStore =
    Gem::Common::GSingletonT<Gem::Common::GGlobalOptionsT<
        std::shared_ptr<Gem::Common::GProviderT<oa::GOptimizationAlgorithmBase>>>>;
/**
 * @brief Returns the global optimization-algorithm-provider store singleton.
 *
 * Drop-in replacement for the former GOAFactoryStore macro.
 *
 * @return A shared pointer to the singleton store mapping keys to
 *         optimization-algorithm providers (never nullptr)
 */
[[nodiscard]] inline std::shared_ptr<GOAStore::STYPE> oaFactoryStore() {
    return GOAStore::instance();
}
