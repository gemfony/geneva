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

// Geneva headers
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "courtier2/GBaseConsumerT.hpp"

namespace Gem::Courtier2 {

/******************************************************************************/
/**
 * The courtier2 broker. Unlike the original courtier broker (which round-robined work items
 * across many simultaneously-registered consumers via raw/processed buffer ports), the courtier2
 * broker holds exactly ONE consumer. The fan-out to multiple workers / clients is the registered
 * consumer's own responsibility (a thread pool locally, many clients networked). This collapses
 * the broker's queueing layer: the executor hands a whole batch straight to the consumer, which
 * reconciles it against the policy.
 */
template <typename processable_type>
class GBrokerT {
public:
    using consumer_type = GBaseConsumerT<processable_type>;

    GBrokerT() = default;
    ~GBrokerT() = default;

    GBrokerT(const GBrokerT &) = delete;
    GBrokerT(GBrokerT &&) = delete;
    GBrokerT &operator=(const GBrokerT &) = delete;
    GBrokerT &operator=(GBrokerT &&) = delete;

    /***************************************************************************/
    /** @brief Registers the (single) consumer. Replacing an existing one is allowed. */
    void registerConsumer(std::shared_ptr<consumer_type> c) {
        consumer_ = std::move(c);
    }

    /***************************************************************************/
    /** @brief Access to the registered consumer. Throws if none was registered. */
    consumer_type &consumer() {
        if(not consumer_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In Gem::Courtier2::GBrokerT::consumer():" << '\n'
                << "No consumer has been registered with the broker." << '\n'
            );
        }
        return *consumer_;
    }

    /***************************************************************************/
    /** @brief Whether a consumer has been registered. */
    [[nodiscard]] bool hasConsumer() const noexcept {
        return static_cast<bool>(consumer_);
    }

private:
    std::shared_ptr<consumer_type> consumer_;
};

/******************************************************************************/

} /* namespace Gem::Courtier2 */
