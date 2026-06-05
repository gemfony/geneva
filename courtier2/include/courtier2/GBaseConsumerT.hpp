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
#include <cstddef>
#include <memory>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

// Geneva headers (reused from the common + courtier libraries)
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "courtier/GCourtierEnums.hpp"       // processingStatus
#include "courtier/GProcessingContainerT.hpp" // the work-item base (reused)
#include "courtier2/GSubmissionPolicy.hpp"

namespace Gem::Courtier2 {

/******************************************************************************/
/**
 * Base class for courtier2 consumers. A consumer is handed a *batch* -- a std::span over the
 * population slice to evaluate -- together with a GSubmissionPolicy, and is SOLELY responsible
 * for turning that span into a fully, successfully evaluated population (or for terminating the
 * program when the policy cannot be honoured). There is no "guaranteed full return" assumption
 * anywhere: an evaluation can always fail or never return, so the consumer always reconciles.
 *
 * Concrete consumers implement only dispatch_(): "evaluate these items, blocking until each has
 * returned (PROCESSED) or failed (EXCEPTION_CAUGHT/ERROR_FLAGGED) or -- for networked consumers --
 * timed out (left DO_PROCESS == MISSING)". The shared reconciliation loop in processBatch() then
 * applies the policy (resubmit MISSING, retry transient FAILED, clone, or terminate).
 */
template <typename processable_type>
class GBaseConsumerT {
public:
    using item_ptr = std::shared_ptr<processable_type>;

    GBaseConsumerT() = default;
    virtual ~GBaseConsumerT() = default;

    GBaseConsumerT(const GBaseConsumerT &) = delete;
    GBaseConsumerT(GBaseConsumerT &&) = delete;
    GBaseConsumerT &operator=(const GBaseConsumerT &) = delete;
    GBaseConsumerT &operator=(GBaseConsumerT &&) = delete;

    /***************************************************************************/
    /**
     * Evaluates and reconciles a batch in place. On return, every slot of the span holds a
     * successfully evaluated item (possibly a clone, for tolerant policies). Terminates the
     * program if the policy cannot be honoured.
     */
    void processBatch(std::span<item_ptr> items, const GSubmissionPolicy &policy) {
        using Gem::Courtier::processingStatus;

        const std::size_t n = items.size();
        if(n == 0) {
            return;
        }

        enum class slot { pending, resolved, unresolved };
        std::vector<slot> state(n, slot::pending);
        std::vector<std::size_t> resubmits(n, 0);
        std::vector<std::size_t> failed_retries(n, 0);
        bool any_success = false;

        // Mark every (non-null) slot due for processing. Null slots are treated as resolved.
        for(std::size_t i = 0; i < n; ++i) {
            if(items[i]) {
                items[i]->set_processing_status(processingStatus::DO_PROCESS);
            }
            else {
                state[i] = slot::resolved;
            }
        }

        // Reconciliation loop. Terminates because every MISSING/FAILED slot eventually exhausts
        // its budget and becomes "unresolved" (so it stops being pending).
        while(true) {
            std::vector<item_ptr> to_eval;
            std::vector<std::size_t> idx;
            for(std::size_t i = 0; i < n; ++i) {
                if(state[i] == slot::pending &&
                   items[i]->getProcessingStatus() == processingStatus::DO_PROCESS) {
                    to_eval.push_back(items[i]);
                    idx.push_back(i);
                }
            }
            if(to_eval.empty()) {
                break; // every slot is resolved or unresolved
            }

            // Consumer-specific evaluation of this round.
            this->dispatch_(to_eval);

            for(std::size_t k = 0; k < idx.size(); ++k) {
                const std::size_t i = idx[k];
                const auto st = items[i]->getProcessingStatus();
                if(st == processingStatus::PROCESSED) {
                    state[i] = slot::resolved;
                    any_success = true;
                }
                else if(st == processingStatus::EXCEPTION_CAUGHT ||
                        st == processingStatus::ERROR_FLAGGED) {
                    // FAILED: retry only to ride out *transient* crashes; a deterministic crash
                    // will keep failing, so the budget terminates in "unresolved".
                    if(failed_retries[i] < policy.max_failed_retries) {
                        ++failed_retries[i];
                        items[i]->set_processing_status(processingStatus::DO_PROCESS);
                    }
                    else {
                        state[i] = slot::unresolved;
                    }
                }
                else {
                    // MISSING (still DO_PROCESS -- a networked timeout). Resubmit within budget;
                    // a MISSING item that persists despite resubmissions is treated as a hidden
                    // FAILED (poison individual) and becomes unresolved rather than looping forever.
                    if(resubmits[i] < policy.max_resubmissions) {
                        ++resubmits[i]; // stays DO_PROCESS -> re-dispatched next round
                    }
                    else {
                        state[i] = slot::unresolved;
                    }
                }
            }
        }

        // Floor: if nothing at all could be evaluated, no policy can rescue the batch.
        if(policy.fatal_on_none_returned && not any_success) {
            this->fatal_(
                "processBatch(): no work item could be evaluated at all "
                "(every evaluation failed or went missing)."
            );
        }

        // Reconcile the unresolved slots per the policy.
        for(std::size_t i = 0; i < n; ++i) {
            if(state[i] != slot::unresolved) {
                continue;
            }
            if(policy.unresolved_action == on_unresolved::clone) {
                items[i] = this->clone_a_successful_(items);
            }
            else {
                this->fatal_(
                    "processBatch(): a work item could not be evaluated (failed or missing) "
                    "and the submission policy forbids substitution (full-success-or-fatal)."
                );
            }
        }
    }

protected:
    /***************************************************************************/
    /**
     * Consumer-specific evaluation of one round of items. Must, for each item, either evaluate it
     * (leaving it PROCESSED or, on a caught processing exception, EXCEPTION_CAUGHT/ERROR_FLAGGED)
     * or -- for networked consumers that time out -- leave it DO_PROCESS to signal MISSING. Must
     * not let exceptions escape (a failed evaluation is reported via the item's status, not by
     * throwing).
     */
    virtual void dispatch_(std::vector<item_ptr> &items) = 0;

    /***************************************************************************/
    /** @brief Clean, fatal termination when the policy cannot be honoured. */
    void fatal_(const std::string &msg) const {
        glogger << "In Gem::Courtier2 consumer:" << '\n'
                << "FATAL: " << msg << '\n'
                << GTERMINATION;
    }

private:
    /***************************************************************************/
    /** @brief Returns a clone of the first successfully evaluated item in the batch (for the
     *  clone-on-partial-return policy). Requires a copy-constructible work item; a future
     *  refinement can take a clone functor (e.g. EA's re-pad-from-parents). */
    item_ptr clone_a_successful_(std::span<item_ptr> items) const {
        for(auto &it : items) {
            if(it && it->is_processed()) {
                if constexpr(std::is_copy_constructible_v<processable_type>) {
                    return std::make_shared<processable_type>(*it);
                }
                else {
                    this->fatal_(
                        "clone-on-partial-return needs a copy-constructible work item "
                        "(or a clone functor, to be supplied by the algorithm)."
                    );
                }
            }
        }
        this->fatal_("clone-on-partial-return: no successfully evaluated item to clone from.");
        return {};
    }
};

/******************************************************************************/

} /* namespace Gem::Courtier2 */
