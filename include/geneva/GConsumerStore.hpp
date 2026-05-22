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
#include "courtier/consumers/GBaseConsumerT.hpp"
#include "geneva/par/GParameterSet.hpp"

namespace Gem::Geneva {

/******************************************************************************/
/**
 * A provider that hands out a single, shared consumer instance (the prototype
 * model): the registered instance IS the working object and is used in place.
 * It mirrors the optimization-algorithm provider's interface so that both global
 * stores share one provisioning abstraction (Gem::Common::GProviderT). The two
 * flavours differ only here: a factory produces many configured objects, a
 * consumer prototype always returns its one instance.
 */
class GConsumerProviderT
  : public Gem::Common::GProviderT<cons::GBaseConsumerT<gpar::GParameterSet>> {
public:
    explicit GConsumerProviderT(
        std::shared_ptr<cons::GBaseConsumerT<gpar::GParameterSet>> consumer
    )
      : consumer_(std::move(consumer)) { /* nothing */ }

    std::shared_ptr<cons::GBaseConsumerT<gpar::GParameterSet>> provide() override {
        return consumer_;
    }
    std::string getMnemonic() const override { return consumer_->getMnemonic(); }
    std::string getName() const override { return consumer_->getConsumerName(); }
    void addCLOptions(
        boost::program_options::options_description &visible,
        boost::program_options::options_description &hidden
    ) override {
        consumer_->addCLOptions(visible, hidden);
    }

private:
    std::shared_ptr<cons::GBaseConsumerT<gpar::GParameterSet>> consumer_;
};

} /* namespace Gem::Geneva */

// A global store for consumer providers (each wraps a single prototype instance).
using GConStore = Gem::Common::GSingletonT<Gem::Common::GGlobalOptionsT<
    std::shared_ptr<Gem::Common::GProviderT<cons::GBaseConsumerT<gpar::GParameterSet>>>>>;
#define GConsumerStore GConStore::Instance(0)
