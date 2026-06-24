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
#include <cstddef>
#include <cstdint>

namespace Gem::Hap {

/******************************************************************************/
/**
 * The number of 64-bit words a QUARANTINE proxy reads per claimed span. Unlike
 * STAGED, the span is NOT copied out -- the proxy reads these words straight from
 * the shared pool (the "stable span in place" that makes QUARANTINE lower-overhead
 * than STAGED, at the price of a benign race; see GQuarantineSource.cpp).
 */
constexpr std::size_t QUARANTINE_CHUNK_WORDS = 4096;

namespace detail {

/******************************************************************************/
/**
 * @brief Claims the next QUARANTINE_CHUNK_WORDS-word span from the rotating pool set.
 *
 * Returns a pointer into one of the N bulk-filled pools; the caller reads
 * QUARANTINE_CHUNK_WORDS words from it in place (no copy). A background producer
 * keeps pools filled ahead of the claim cursor, refilling a pool only after a full
 * rotation (the "quarantine"), so an active reader never shares a pool with the
 * refiller -- only a descheduled, stale reader can, and that read is benign on
 * aligned-64-bit hardware (old-or-new, never torn). This is the library-private
 * seam used by GRandomT<randomSource::QUARANTINE>; the implementation lives in
 * GQuarantineSource.cpp.
 *
 * @return Pointer to a QUARANTINE_CHUNK_WORDS-word span inside a shared pool
 */
const std::uint64_t *quarantineClaimSpan();

/******************************************************************************/

} /* namespace detail */
} /* namespace Gem::Hap */
