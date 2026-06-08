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

#include "geneva/par/GFlatParameters.hpp"

#include <random>

#include "common/GExpectationChecksT.hpp"
#include "geneva/par/GParameterSet.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Parameters::GFlatParameters) // NOLINT

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * Builds a flat parameter node from an existing (tree-based) individual by
 * streamlining its values and boundaries per scalar type.
 */
std::unique_ptr<GFlatParameters> GFlatParameters::compileFrom(const GParameterSet &src) {
    auto flat = std::make_unique<GFlatParameters>();
    GParameterLayout &L = *flat->layout_;

    src.streamline<double>(flat->dv_);
    src.boundaries<double>(L.d.lower, L.d.upper);
    L.d.kind.assign(flat->dv_.size(), SlotKind::Plain);

    src.streamline<float>(flat->fv_);
    src.boundaries<float>(L.f.lower, L.f.upper);
    L.f.kind.assign(flat->fv_.size(), SlotKind::Plain);

    src.streamline<std::int32_t>(flat->iv_);
    src.boundaries<std::int32_t>(L.i.lower, L.i.upper);
    L.i.kind.assign(flat->iv_.size(), SlotKind::Plain);

    std::vector<bool> b_tmp;
    src.streamline<bool>(b_tmp);
    flat->bv_.assign(b_tmp.begin(), b_tmp.end());
    src.boundaries<bool>(L.b.lower, L.b.upper);
    L.b.kind.assign(flat->bv_.size(), SlotKind::Plain);

    return flat;
}

/******************************************************************************/
// Streamline channels -- append flat values.

void GFlatParameters::doubleStreamline(std::vector<double> &out, const activityMode &) const {
    out.insert(out.end(), dv_.begin(), dv_.end());
}

void GFlatParameters::floatStreamline(std::vector<float> &out, const activityMode &) const {
    out.insert(out.end(), fv_.begin(), fv_.end());
}

void GFlatParameters::int32Streamline(std::vector<std::int32_t> &out, const activityMode &) const {
    out.insert(out.end(), iv_.begin(), iv_.end());
}

void GFlatParameters::booleanStreamline(std::vector<bool> &out, const activityMode &) const {
    out.reserve(out.size() + bv_.size());
    for(std::uint8_t v : bv_) {
        out.push_back(v != 0);
    }
}

/******************************************************************************/
// Boundary channels -- append per-slot boundaries from the shared layout.

void GFlatParameters::doubleBoundaries(
    std::vector<double> &lo,
    std::vector<double> &up,
    const activityMode &
) const {
    lo.insert(lo.end(), layout_->d.lower.begin(), layout_->d.lower.end());
    up.insert(up.end(), layout_->d.upper.begin(), layout_->d.upper.end());
}

void GFlatParameters::floatBoundaries(
    std::vector<float> &lo,
    std::vector<float> &up,
    const activityMode &
) const {
    lo.insert(lo.end(), layout_->f.lower.begin(), layout_->f.lower.end());
    up.insert(up.end(), layout_->f.upper.begin(), layout_->f.upper.end());
}

void GFlatParameters::int32Boundaries(
    std::vector<std::int32_t> &lo,
    std::vector<std::int32_t> &up,
    const activityMode &
) const {
    lo.insert(lo.end(), layout_->i.lower.begin(), layout_->i.lower.end());
    up.insert(up.end(), layout_->i.upper.begin(), layout_->i.upper.end());
}

void GFlatParameters::booleanBoundaries(
    std::vector<bool> &lo,
    std::vector<bool> &up,
    const activityMode &
) const {
    lo.insert(lo.end(), layout_->b.lower.begin(), layout_->b.lower.end());
    up.insert(up.end(), layout_->b.upper.begin(), layout_->b.upper.end());
}

/******************************************************************************/
// Count channels.

std::size_t GFlatParameters::countDoubleParameters(const activityMode &) const {
    return dv_.size();
}

std::size_t GFlatParameters::countFloatParameters(const activityMode &) const {
    return fv_.size();
}

std::size_t GFlatParameters::countInt32Parameters(const activityMode &) const {
    return iv_.size();
}

std::size_t GFlatParameters::countBoolParameters(const activityMode &) const {
    return bv_.size();
}

/******************************************************************************/
// Assign-back channels -- absorb values from a flat vector, advancing pos.

void GFlatParameters::assignDoubleValueVector(
    const std::vector<double> &vec,
    std::size_t &pos,
    const activityMode &
) {
    for(double &v : dv_) {
        v = vec.at(pos);
        ++pos;
    }
}

void GFlatParameters::assignFloatValueVector(
    const std::vector<float> &vec,
    std::size_t &pos,
    const activityMode &
) {
    for(float &v : fv_) {
        v = vec.at(pos);
        ++pos;
    }
}

void GFlatParameters::assignInt32ValueVector(
    const std::vector<std::int32_t> &vec,
    std::size_t &pos,
    const activityMode &
) {
    for(std::int32_t &v : iv_) {
        v = vec.at(pos);
        ++pos;
    }
}

void GFlatParameters::assignBooleanValueVector(
    const std::vector<bool> &vec,
    std::size_t &pos,
    const activityMode &
) {
    for(std::uint8_t &v : bv_) {
        v = vec.at(pos) ? 1 : 0;
        ++pos;
    }
}

/******************************************************************************/
// Random (re-)initialization within each slot's boundaries.

bool GFlatParameters::randomInit_(const activityMode &, Gem::Hap::GRandomBase &gr) {
    for(std::size_t k = 0; k < dv_.size(); ++k) {
        const double lo = layout_->d.lower.at(k);
        const double hi = layout_->d.upper.at(k);
        dv_[k] = (hi > lo) ? std::uniform_real_distribution<double>(lo, hi)(gr) : lo;
    }
    for(std::size_t k = 0; k < fv_.size(); ++k) {
        const float lo = layout_->f.lower.at(k);
        const float hi = layout_->f.upper.at(k);
        fv_[k] = (hi > lo) ? std::uniform_real_distribution<float>(lo, hi)(gr) : lo;
    }
    for(std::size_t k = 0; k < iv_.size(); ++k) {
        const std::int32_t lo = layout_->i.lower.at(k);
        const std::int32_t hi = layout_->i.upper.at(k);
        iv_[k] = (hi > lo) ? std::uniform_int_distribution<std::int32_t>(lo, hi)(gr) : lo;
    }
    for(std::uint8_t &v : bv_) {
        v = static_cast<std::uint8_t>(std::uniform_int_distribution<int>(0, 1)(gr));
    }
    return true;
}

/******************************************************************************/
// Adaption is not yet implemented for the flat node (Phase 0). It will be added
// as a flat, AdaptorKind-dispatched free operation in a later phase.

std::size_t GFlatParameters::adapt_(Gem::Hap::GRandomBase &) {
    return 0;
}

bool GFlatParameters::updateAdaptorsOnStall_(std::size_t) {
    return false;
}

void GFlatParameters::queryAdaptor_(
    const std::string &,
    const std::string &,
    std::vector<std::any> &
) const { /* no adaptors yet */
}

/******************************************************************************/
// Property-tree output (used for result files). Minimal positional dump.

void GFlatParameters::toPropertyTree(pt::ptree &ptr, const std::string &base_name) const {
    ptr.put(base_name + ".type", this->name_());
    ptr.put(base_name + ".nDouble", dv_.size());
    ptr.put(base_name + ".nFloat", fv_.size());
    ptr.put(base_name + ".nInt32", iv_.size());
    ptr.put(base_name + ".nBool", bv_.size());
    for(std::size_t k = 0; k < dv_.size(); ++k) {
        ptr.put(base_name + ".d.value" + Gem::Common::to_string(k), dv_[k]);
    }
    for(std::size_t k = 0; k < fv_.size(); ++k) {
        ptr.put(base_name + ".f.value" + Gem::Common::to_string(k), fv_[k]);
    }
    for(std::size_t k = 0; k < iv_.size(); ++k) {
        ptr.put(base_name + ".i.value" + Gem::Common::to_string(k), iv_[k]);
    }
    for(std::size_t k = 0; k < bv_.size(); ++k) {
        ptr.put(base_name + ".b.value" + Gem::Common::to_string(k), bv_[k] != 0);
    }
}

/******************************************************************************/
// Loading, comparison, cloning, naming.

void GFlatParameters::load_(const GParameterBase *cp) {
    const GFlatParameters *p_load =
        Gem::Common::g_convert_and_compare<GParameterBase, GFlatParameters>(cp, this);

    // Load our parent class'es data ...
    GParameterBase::load_(cp);

    // ... and then our local data. The layout is immutable and shared, so we
    // share the pointer rather than deep-copying it.
    layout_ = p_load->layout_;
    dv_ = p_load->dv_;
    fv_ = p_load->fv_;
    iv_ = p_load->iv_;
    bv_ = p_load->bv_;
}

void GFlatParameters::compare_(
    const GParameterBase &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double &limit
) const {
    using namespace Gem::Common;

    const GFlatParameters *p_load =
        Gem::Common::g_convert_and_compare<GParameterBase, GFlatParameters>(cp, this);

    GToken token("GFlatParameters", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GParameterBase>(*this, *p_load, token);

    // ... and then the flat value arrays.
    compare_t(IDENTITY(dv_, p_load->dv_), token);
    compare_t(IDENTITY(fv_, p_load->fv_), token);
    compare_t(IDENTITY(iv_, p_load->iv_), token);
    compare_t(IDENTITY(bv_, p_load->bv_), token);

    token.evaluate();
}

std::string GFlatParameters::name_() const {
    return std::string("GFlatParameters");
}

GParameterBase *GFlatParameters::clone_() const {
    return new GFlatParameters(*this);
}

/******************************************************************************/
// Test hooks.

bool GFlatParameters::modify_GUnitTests_() {
    bool result = false;
    if(not dv_.empty()) {
        dv_[0] += 1.0;
        result = true;
    }
    else if(not fv_.empty()) {
        fv_[0] += 1.0F;
        result = true;
    }
    else if(not iv_.empty()) {
        iv_[0] += 1;
        result = true;
    }
    else if(not bv_.empty()) {
        bv_[0] = static_cast<std::uint8_t>(bv_[0] != 0 ? 0 : 1);
        result = true;
    }
    return result;
}

void GFlatParameters::specificTestsNoFailureExpected_GUnitTests_() { /* nothing */ }

void GFlatParameters::specificTestsFailuresExpected_GUnitTests_() { /* nothing */ }

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
