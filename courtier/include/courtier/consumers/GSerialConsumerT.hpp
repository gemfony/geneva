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

#include "common/GGlobalDefines.hpp"

// Standard headers
#include <memory>
#include <span>
#include <vector>

// Geneva headers
#include "courtier/GBaseConsumerT.hpp"

namespace Gem::Courtier {

/******************************************************************************/
/**
 * The simplest consumer: it evaluates each item inline on the calling thread. As with the local
 * thread-pool consumer a local evaluation never goes MISSING -- it succeeds (PROCESSED) or its
 * thrown processing exception is funnelled into EXCEPTION_CAUGHT (and caught here so it does not
 * escape). Mostly useful for debugging and as the reference implementation of dispatch_().
 *
 * @tparam processable_type The concrete work-item type evaluated inline by this consumer
 */
template <typename processable_type>
class GSerialConsumerT final : public GBaseConsumerT<processable_type> {
public:
    using item_ptr = typename GBaseConsumerT<processable_type>::item_ptr;

    GSerialConsumerT() = default;
    ~GSerialConsumerT() override = default;

protected:
    /***************************************************************************/
    /** @brief Evaluates each DO_PROCESS slot of the batch span inline on the calling thread, funnelling
     *  any thrown processing exception into the item's status so it does not escape.
     *  @param items The batch span; only slots flagged DO_PROCESS are evaluated, in place */
    void dispatch_(std::span<item_ptr> items) override {
        for(auto &it : items) {
            if(not it || it->getProcessingStatus() != Gem::Courtier::processingStatus::DO_PROCESS) {
                continue;
            }
            try {
                it->process();
            }
            catch(...) {
                // The item's status already reflects the failure (EXCEPTION_CAUGHT); reconciliation
                // reads the status, not an exception.
            }
        }
    }
};

/******************************************************************************/

} /* namespace Gem::Courtier */
