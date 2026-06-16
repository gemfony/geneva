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
#include <cstddef>
#include <memory>
#include <span>
#include <vector>

// Geneva headers go here
#include "courtier/GBrokerT.hpp"
#include "courtier/GExecutorT.hpp"
#include "courtier/GExecutorStatusT.hpp" // executor_status_t
#include "courtier/GSubmissionPolicy.hpp"
#include "courtier/consumers/GSerialConsumerT.hpp"
#include "courtier/consumers/GStdThreadConsumerT.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

namespace gen = Gem::Geneva::Genome;

/******************************************************************************/
/**
 * Identifies which courtier LOCAL consumer an algorithm should submit through when courtier routing is
 * active. Go2 maps the chosen parallelisation mnemonic onto one of these and plumbs it into the
 * algorithm via setLocalConsumer(). "none" (the default) means "build a local consumer at init".
 */
enum class local_consumer_kind {
    none,         ///< Not yet configured (init defaults it to multithreaded).
    serial,       ///< Inline, single-threaded courtier consumer (mnemonic "sc").
    multithreaded ///< Thread-pool courtier consumer (mnemonic "stc").
};

/******************************************************************************/
/**
 * The courtier SUBMISSION mechanism for an optimization algorithm, extracted out of
 * GOptimizationAlgorithmBase (D-3, single-responsibility): it owns the broker / executor / local-consumer
 * lifecycle and the late-return buffer wiring, so the algorithm base is left to do optimization, not
 * courtier plumbing.
 *
 * The algorithm configures it (setLocalConsumer / setBroker), then submits each generation's work items
 * through workOn(); the policy lazily builds the broker + executor + local consumer on first use and
 * reuses them across generations. This object is TRANSIENT run state (broker/executor): it is neither
 * serialised nor copied with the algorithm -- a cloned algorithm gets a fresh, unconfigured policy and
 * re-establishes routing at setup, exactly as the inlined members did before the extraction.
 */
class GOptimizerExecutionPolicy {
public:
    using item_ptr = std::unique_ptr<gen::GOptimizableEntity>;
    using broker_ptr = std::shared_ptr<Gem::Courtier::GBrokerT<gen::GOptimizableEntity>>;

    /***************************************************************************/
    /** @brief Selects a LOCAL consumer (serial / multithreaded) to submit through. n_threads == 0 means
     *  hardware concurrency. Ignored once an external broker has been injected via setBroker(). */
    void setLocalConsumer(local_consumer_kind kind, unsigned int n_threads = 0) {
        local_kind_ = kind;
        local_threads_ = n_threads;
    }

    /** @brief Injects a ready broker (its consumer registered, clone function set, server started for the
     *  networked case), e.g. from Go2. Takes precedence over setLocalConsumer(). */
    void setBroker(broker_ptr broker) {
        broker_ = std::move(broker);
        external_broker_ = true;
    }

    /** @brief Init-time default: when nothing was configured (no external broker, no local kind), default
     *  to a local multithreaded consumer so a bare algorithm->optimize() works standalone. */
    void applyInitDefault() {
        if(local_kind_ == local_consumer_kind::none && not external_broker_) {
            local_kind_ = local_consumer_kind::multithreaded; // 0 threads == hardware concurrency
        }
    }

    /***************************************************************************/
    /**
     * @brief Submits a span of work items through the courtier span+policy executor, returning the
     * executor status. Lazily builds the broker + executor + local consumer (and enables the late-return
     * buffer, sized to @p late_return_cap) on first use. The span aliases the caller's population
     * sub-range, so results + cloned refills are written in place; @p policy is the algorithm's choice
     * (clone-on-partial-return for tolerant population OAs, full-success-or-fatal for need-all OAs).
     */
    Gem::Courtier::executor_status_t workOn(
        std::span<item_ptr> sp,
        const Gem::Courtier::GSubmissionPolicy &policy,
        std::size_t late_return_cap
    ) {
        ensureExecutor_(late_return_cap);

        if(sp.empty()) {
            return Gem::Courtier::executor_status_t{true, false};
        }
        executor_->workOn(sp, policy);

        bool has_errors = false;
        for(const auto &it : sp) {
            if(it && it->has_errors()) {
                has_errors = true;
                break;
            }
        }
        return Gem::Courtier::executor_status_t{true, has_errors};
    }

    /** @brief Reaps any LATE returns the consumer buffered (results that came back after their batch was
     *  reconciled). Empty for a local consumer; a networked consumer hands back its bounded buffer. */
    std::vector<item_ptr> getOldWorkItems() {
        if(broker_ && broker_->hasConsumer()) {
            return broker_->consumer().getLateReturns();
        }
        return {};
    }

private:
    /***************************************************************************/
    /** @brief Lazily builds the broker (with the selected local consumer) + executor on first use, and
     *  enables the consumer's late-return buffer. A no-op once established. */
    void ensureExecutor_(std::size_t late_return_cap) {
        if(executor_) {
            return;
        }
        // A networked broker injected via setBroker() arrives ready (consumer registered, clone function
        // set, server started). Only the LOCAL path builds its own consumer here.
        if(not broker_) {
            broker_ = std::make_shared<Gem::Courtier::GBrokerT<gen::GOptimizableEntity>>();
            std::shared_ptr<Gem::Courtier::GBaseConsumerT<gen::GOptimizableEntity>> consumer;
            if(local_kind_ == local_consumer_kind::serial) {
                consumer = std::make_shared<Gem::Courtier::GSerialConsumerT<gen::GOptimizableEntity>>();
            }
            else {
                consumer = std::make_shared<Gem::Courtier::GStdThreadConsumerT<gen::GOptimizableEntity>>(
                    local_threads_
                );
            }
            // Polymorphic clone (GOptimizableEntity holds a concrete individual; copy-construction slices).
            consumer->setCloneFunction([](const std::unique_ptr<gen::GOptimizableEntity> &p) {
                return p->clone_unique();
            });
            broker_->registerConsumer(consumer);
        }
        executor_ = std::make_shared<Gem::Courtier::GExecutorT<gen::GOptimizableEntity>>(broker_);

        // Enable the consumer's late-return buffer (#13b): a result that comes back AFTER its batch was
        // reconciled is retained for the OA to reap, instead of dropped. A no-op on local consumers.
        if(broker_ && broker_->hasConsumer()) {
            broker_->consumer().enableLateReturns(late_return_cap, /*ttl_rounds*/ 3);
        }
    }

    local_consumer_kind local_kind_ = local_consumer_kind::none; ///< Which local consumer (none == build at init)
    unsigned int local_threads_ = 0;  ///< Thread-pool size for the multithreaded kind (0 == hardware concurrency)
    bool external_broker_ = false;    ///< True when a ready broker was injected via setBroker()
    broker_ptr broker_;               ///< The courtier broker (transient run state)
    std::shared_ptr<Gem::Courtier::GExecutorT<gen::GOptimizableEntity>> executor_; ///< The span+policy executor
};

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
