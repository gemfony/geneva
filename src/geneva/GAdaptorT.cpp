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

#include "geneva/GAdaptorT.hpp"

namespace Gem::Geneva {

/******************************************************************************/
/**
 * Common interface for all adaptors to the adaption functionality. Specialization
 * for the T==bool case . Note that
 *
 * @param val_vec A vector of values that need to be adapted
 * @param range A typical value range for type T
 * @return The number of adaptions that were carried out
 */
template <>
std::size_t GAdaptorT<bool, double>::adapt(
    std::vector<bool> &val_vec,
    const bool &range,
    Gem::Hap::GRandomBase &gr
) {
    using namespace Gem::Common;
    using namespace Gem::Hap;

    std::size_t n_adapted = 0;

    // Update the adaption probability, if requested by the user
    if(adaptAdProb_ > double(0.)) {
        adProb_ *= std::exp(normal_distribution_(
            gr,
            typename std::normal_distribution<double>::param_type(0., adaptAdProb_)
        ));
        Gem::Common::enforceRangeConstraint<double>(
            adProb_,
            minAdProb_,
            maxAdProb_,
            "GAdaptorT<bool,double>::adapt()"
        );
    }

    bool dummy_val;

    if(adaptionMode::WITHPROBABILITY ==
       adaptionMode_) { // The most likely case is indeterminate (means: "depends")
        for(auto &&val : val_vec) {
            // A likelihood of adProb_ for adaption
            if(weighted_bool_(gr, std::bernoulli_distribution::param_type(std::abs(adProb_)))) {
                dummy_val = val;
                adaptAdaption(range, gr);
                customAdaptions(
                    dummy_val,
                    range,
                    gr
                ); // does not know about the bool-proxy of std::vector<bool>
                val = dummy_val;
                n_adapted += 1;
            }
        }
    }
    else if(adaptionMode::ALWAYS == adaptionMode_) { // always adapt
        for(auto &&val : val_vec) {
            dummy_val = val;
            adaptAdaption(range, gr);
            customAdaptions(dummy_val, range, gr);
            val = dummy_val;
            n_adapted += 1;
        }
    }

    // No need to test for "adaptionMode_ == adaptionMode::NEVER" as no action is needed in this case

    return n_adapted;
}

/******************************************************************************/

} /* namespace Gem::Geneva */
