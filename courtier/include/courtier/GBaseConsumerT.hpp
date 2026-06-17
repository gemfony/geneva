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
#include <cstdint>
#include <cstdlib>
#include <functional>
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
#include "courtier/GSubmissionPolicy.hpp"

namespace Gem::Courtier {

/******************************************************************************/
/**
 * Base class for courtier consumers. A consumer is handed a *batch* -- a std::span over the
 * population slice to evaluate -- together with a GSubmissionPolicy, and is SOLELY responsible
 * for turning that span into a fully, successfully evaluated population (or for terminating the
 * program when the policy cannot be honoured). There is no "guaranteed full return" assumption
 * anywhere: an evaluation can always fail or never return, so the consumer always reconciles.
 *
 * Concrete consumers implement only dispatch_(): "evaluate these items, blocking until each has
 * returned (PROCESSED) or failed (EXCEPTION_CAUGHT/ERROR_FLAGGED) or -- for networked consumers --
 * timed out (left DO_PROCESS == MISSING)". The shared reconciliation loop in processBatch() then
 * applies the policy (resubmit MISSING, retry transient FAILED, clone, or terminate).
 *
 * @tparam processable_type The concrete work-item type evaluated by this consumer
 */
template <typename processable_type>
class GBaseConsumerT {
public:
    using item_ptr = std::unique_ptr<processable_type>;

    GBaseConsumerT() = default;
    virtual ~GBaseConsumerT() = default;

    GBaseConsumerT(const GBaseConsumerT &) = delete;
    GBaseConsumerT(GBaseConsumerT &&) = delete;
    GBaseConsumerT &operator=(const GBaseConsumerT &) = delete;
    GBaseConsumerT &operator=(GBaseConsumerT &&) = delete;

    /***************************************************************************/
    /** @brief Enables/sizes the consumer's late-return buffer: results that come back AFTER their
     *  batch was already reconciled are retained (instead of dropped) for the optimization algorithm to
     *  reap. @p cap == 0 disables it. Default: a no-op -- local (serial / thread-pool) consumers never
     *  produce late returns. Networked consumers override this.
     *  @param cap Maximum number of late-return items to buffer (0 disables late-return buffering)
     *  @param ttl_rounds Number of dispatch rounds a buffered late return is kept before being discarded */
    virtual void enableLateReturns(std::size_t /*cap*/, std::uint64_t /*ttl_rounds*/) {}

    /** @brief Hands back (transfers ownership of) the work items the consumer buffered as late returns,
     *  emptying the buffer. Default: none. Networked consumers override this to drain their buffer.
     *  @return The buffered late-return items (ownership transferred); empty if the consumer buffers none */
    virtual std::vector<item_ptr> getLateReturns() { return {}; }

    /***************************************************************************/
    /**
     * Evaluates and reconciles a batch in place. On return, every slot of the span holds a
     * successfully evaluated item (possibly a clone, for tolerant policies). Terminates the
     * program if the policy cannot be honoured.
     *
     * @param items The batch of (uniquely owned) work items to evaluate and reconcile, in place
     * @param policy The submission policy governing resubmit/retry/clone/terminate decisions
     * @param clone_template An optional representative item used as the clone source when refilling
     *        unresolved slots (if null, the first successfully evaluated sibling is used instead)
     */
    void processBatch(std::span<item_ptr> items, const GSubmissionPolicy &policy,
                      item_ptr clone_template = nullptr) {
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
                    // Items are uniquely owned: move each into the working set for this round and
                    // move the (possibly replaced) result straight back below. The batch slot is
                    // transiently null only for the duration of the synchronous dispatch_ call.
                    to_eval.push_back(std::move(items[i]));
                    idx.push_back(i);
                }
            }
            if(to_eval.empty()) {
                break; // every slot is resolved or unresolved
            }

            // Consumer-specific evaluation of this round. A networked consumer evaluates a copy on
            // a remote client and hands back a *different* object (the deserialized result), so
            // dispatch_ may replace entries of to_eval with those results; write them back into the
            // batch. A local consumer mutates each item in place, so the write-back is a no-op.
            this->dispatch_(to_eval);
            for(std::size_t k = 0; k < idx.size(); ++k) {
                items[idx[k]] = std::move(to_eval[k]);
            }

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
                items[i] = this->clone_for_refill_(items, clone_template);
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
     *
     * A consumer that evaluates a copy elsewhere (e.g. a remote client) may overwrite an entry of
     * @p items with the resulting object; the replacement is written back into the batch by the
     * caller. A consumer that mutates each item in place simply leaves the pointers untouched.
     *
     * @param items One round of (uniquely owned) work items to evaluate; entries may be replaced
     *        with the resulting objects for consumers that evaluate a copy elsewhere
     */
    virtual void dispatch_(std::vector<item_ptr> &items) = 0;

    /***************************************************************************/
    /** @brief Clean, fatal exit when the policy cannot be honoured. This is an expected terminal
     *  condition (e.g. a lost evaluation under a need-all policy), not an internal fault, so it
     *  exits cleanly via LOGEXIT rather than std::terminate()-ing with a core dump.
     *  @param msg The human-readable reason for the fatal exit (logged before exiting) */
    void fatal_(const std::string &msg) const {
        glogger << "In Gem::Courtier consumer:" << '\n'
                << "FATAL: " << msg << '\n'
                << LOGEXIT(EXIT_FAILURE);
    }

    /***************************************************************************/
    /** @brief Deep-clones one (uniquely owned) work item into a fresh owning item. Uses the polymorphic
     *  clone functor when set (required for polymorphic item types such as GFlatGenome, to avoid
     *  slicing), otherwise copy-construction (correct for leaf/concrete item types). Used both by the
     *  refill path and by the networked consumers when handing a session a copy to ship.
     *  @param src The (borrowed) uniquely owned work item to clone from
     *  @return A fresh owning copy of @p src; an empty item_ptr if cloning is impossible (after fatal exit) */
    item_ptr clone_item_(const item_ptr &src) const {
        if(clone_fn_) {
            return clone_fn_(src);
        }
        if constexpr(std::is_copy_constructible_v<processable_type>) {
            return std::make_unique<processable_type>(*src);
        }
        else {
            this->fatal_(
                "cloning a work item needs a clone function (setCloneFunction) or a "
                "copy-constructible work item."
            );
        }
        return {};
    }

public:
    /***************************************************************************/
    /** @brief Sets a polymorphic clone function for clone-on-partial-return. REQUIRED when the work
     *  item is a polymorphic base (e.g. GFlatGenome holding a concrete individual): plain
     *  copy-construction of processable_type would SLICE it. The functor should deep-clone via the
     *  type's own clone mechanism, e.g. `[](const item_ptr& p){ return p->template clone<T>(); }`.
     *  When unset, refill falls back to copy-construction (correct for leaf/concrete item types).
     *  @param fn The deep-clone functor taking a borrowed source item and returning a fresh owning copy */
    void setCloneFunction(std::function<item_ptr(const item_ptr &)> fn) { clone_fn_ = std::move(fn); }

private:
    /***************************************************************************/
    /** @brief Produces a replacement item to refill an unresolved slot under clone-on-partial-return.
     *  Source = a caller-supplied @p clone_template (e.g. a representative individual) if present,
     *  else the first successfully evaluated sibling in the batch. The clone itself uses the
     *  polymorphic clone functor when set (required for polymorphic item types to avoid slicing),
     *  otherwise copy-construction (leaf types).
     *  @param items The batch, scanned for a successfully evaluated sibling to clone from when no template is given
     *  @param clone_template An explicit clone source; if empty, the first processed item of @p items is used
     *  @return A fresh owning replacement item; an empty item_ptr if no source is available (after fatal exit) */
    item_ptr clone_for_refill_(std::span<item_ptr> items, const item_ptr &clone_template) const {
        // Items are uniquely owned, so the source is only borrowed (a pointer to the chosen owner),
        // never copied; we clone from it to produce the fresh owning item.
        const item_ptr *src = &clone_template;
        if(not *src) {
            for(auto &it : items) {
                if(it && it->is_processed()) {
                    src = &it;
                    break;
                }
            }
        }
        if(not *src) {
            this->fatal_(
                "clone-on-partial-return: no clone template was supplied and no successfully "
                "evaluated item is available to clone from."
            );
            return {};
        }
        return this->clone_item_(*src);
    }

    /***************************************************************************/
    std::function<item_ptr(const item_ptr &)> clone_fn_; ///< Polymorphic clone (see setCloneFunction)
};

/******************************************************************************/

} /* namespace Gem::Courtier */
