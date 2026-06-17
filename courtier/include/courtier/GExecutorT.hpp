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
#include "courtier/GBrokerT.hpp"
#include "courtier/GSubmissionPolicy.hpp"

namespace Gem::Courtier {

/******************************************************************************/
/**
 * The executor. The original (pre-rework) courtier had three executors (serial / MT /
 * broker) each with its own resubmission logic; here a single thin executor hands the whole batch
 * -- a std::span over the population slice the algorithm wants evaluated -- to the broker's single
 * consumer, which reconciles it against the submission policy. Because every failure policy
 * preserves the batch size (resubmit / clone / fatal, never shrink), the span is fixed-size and
 * fully valid on return; there is no separate "fixAfterJobSubmission" / executor-side resubmission
 * step anymore.
 *
 * @tparam processable_type The work-item type the executor submits and reconciles
 */
template <typename processable_type>
class GExecutorT {
public:
    using item_ptr = std::unique_ptr<processable_type>;

    /***************************************************************************/
    /**
     * @brief Initialization with the broker the executor submits through.
     *
     * @param broker The broker (shared ownership) whose single consumer reconciles every submitted batch
     */
    explicit GExecutorT(std::shared_ptr<GBrokerT<processable_type>> broker)
        : broker_(std::move(broker))
    { /* nothing */ }

    /***************************************************************************/
    /**
     * @brief Evaluates and reconciles the given batch in place against the policy.
     *
     * On return every slot holds a successfully evaluated item (or the program has terminated, per
     * the policy).
     *
     * @param items A span over the population slice to evaluate; reconciled in place (fixed-size, fully valid on return)
     * @param policy The submission/failure policy the consumer reconciles unresolved slots against (resubmit / clone / fatal)
     * @param clone_template Optional already-evaluated representative the consumer clones from when refilling unresolved slots (ownership transferred); null to clone a successful sibling instead
     */
    void workOn(std::span<item_ptr> items, const GSubmissionPolicy &policy,
                item_ptr clone_template = nullptr) {
        broker_->consumer().processBatch(items, policy, std::move(clone_template));
    }

    /***************************************************************************/
    /**
     * @brief Convenience overload taking a vector reference (forwards to the span overload).
     *
     * @param items The population slice to evaluate; reconciled in place
     * @param policy The submission/failure policy the consumer reconciles unresolved slots against
     * @param clone_template Optional already-evaluated representative the consumer clones from when refilling unresolved slots under clone-on-partial-return (ownership transferred); null to clone a successful sibling instead
     */
    void workOn(std::vector<item_ptr> &items, const GSubmissionPolicy &policy,
                item_ptr clone_template = nullptr) {
        this->workOn(std::span<item_ptr>(items.data(), items.size()), policy, std::move(clone_template));
    }

private:
    std::shared_ptr<GBrokerT<processable_type>> broker_;
};

/******************************************************************************/

} /* namespace Gem::Courtier */
