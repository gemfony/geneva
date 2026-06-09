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

#include "geneva/par/GFlatParameterSet.hpp"

#include <memory>

#include "common/GExceptions.hpp"

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * Compiles the parameter objects added so far into a single flat node, replacing them.
 */
void GFlatParameterSet::compileToFlat() {
    std::unique_ptr<GFlatParameters> flat = GFlatParameters::compileFrom(*this);
    this->clear();
    this->push_back(std::shared_ptr<GFlatParameters>(std::move(flat)));
}

/******************************************************************************/
/**
 * Access to the single flat node.
 */
GFlatParameters &GFlatParameterSet::flatNode() {
    if(this->empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFlatParameterSet::flatNode(): Error!" << '\n'
            << "No flat node present -- did you forget to call compileToFlat()?" << '\n'
        );
    }
    return dynamic_cast<GFlatParameters &>(*this->data_cnt_.at(0));
}

const GFlatParameters &GFlatParameterSet::flatNode() const {
    if(this->empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GFlatParameterSet::flatNode() const: Error!" << '\n'
            << "No flat node present -- did you forget to call compileToFlat()?" << '\n'
        );
    }
    return dynamic_cast<const GFlatParameters &>(*this->data_cnt_.at(0));
}

/******************************************************************************/

const std::vector<double> &GFlatParameterSet::flatDoubleValues() const {
    return this->flatNode().doubleValues();
}

const std::vector<float> &GFlatParameterSet::flatFloatValues() const {
    return this->flatNode().floatValues();
}

const std::vector<std::int32_t> &GFlatParameterSet::flatInt32Values() const {
    return this->flatNode().int32Values();
}

const std::vector<std::uint8_t> &GFlatParameterSet::flatBoolValues() const {
    return this->flatNode().boolValues();
}

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
