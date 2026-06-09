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

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <tuple>
#include <type_traits>

#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExpectationChecksT.hpp"
#include "geneva/par/GConstrainedNumCollectionT.hpp"
#include "geneva/par/GConstrainedNumT.hpp"
#include "geneva/par/GNumGaussAdaptorT.hpp"
#include "geneva/par/GParameterBaseWithAdaptorsT.hpp"
#include "geneva/par/GParameterSet.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Parameters::GFlatParameters) // NOLINT

namespace Gem::Geneva::Parameters {

/******************************************************************************/
// Anonymous-namespace helpers for capturing, cloning and applying adaptor groups.
namespace {

/**
 * @brief Folds an out-of-range floating-point value back into [lower, upper).
 *
 * This is the flat-node counterpart of GConstrainedFPT::transfer(): the published reflecting
 * ("triangle wave") map -- even regions translate, odd regions reflect -- computed in long double
 * and clamped to the half-open interval. Because the result is stored back after every adaption
 * (mirroring GConstrainedNumT::value(), which resets its mutable internal value to the folded
 * result), the stored value never drifts into unbounded "unhealthy" regions.
 */
template <typename T>
    requires std::is_floating_point_v<T>
T foldIntoRange(T val, T lower, T upper) {
    if(not(upper > lower)) {
        return lower; // degenerate range
    }
    if(not std::isfinite(val)) {
        return lower; // defensive: never store NaN/inf
    }
    if(val >= lower && val < upper) {
        return val;
    }

    const long double lo = static_cast<long double>(lower);
    const long double hi = static_cast<long double>(upper);
    const long double v = static_cast<long double>(val);
    const long double width = hi - lo;

    const std::int64_t region = static_cast<std::int64_t>(std::floor((v - lo) / width));
    long double mapping = 0.0L;
    if(region % 2 == 0) { // region 0, ±2, ... : translate
        mapping = v - static_cast<long double>(region) * width;
    }
    else { // region ±1, ±3, ... : reflect
        mapping = -v + (static_cast<long double>(region - 1) * width + 2.0L * hi);
    }

    T result = static_cast<T>(mapping);
    if(result < lower) {
        result = lower;
    }
    else if(result >= upper) {
        result = std::nextafter(upper, lower); // enforce the half-open [lower, upper)
    }
    return result;
}

/**
 * @brief Integer counterpart of foldIntoRange: the flat-node reimplementation of
 *        GConstrainedIntT::transfer().
 *
 * The integer fold differs from the FP one: the range is INCLUSIVE [lower, upper] (so the period is
 * value_range = upper - lower + 1), region counting uses integer division, and the reflection is
 * revert(x) = upper + lower - x. Clean-room from the same published reflecting map.
 */
template <typename T>
    requires std::is_integral_v<T>
T foldIntoRangeInt(T val, T lower, T upper) {
    if(not(upper >= lower)) {
        return lower; // degenerate range
    }
    if(val >= lower && val <= upper) {
        return val;
    }

    const T value_range = static_cast<T>(upper - lower + 1);
    T mapping = 0;
    if(val < lower) {
        const T n = static_cast<T>((lower - (val + 1)) / value_range);
        mapping = static_cast<T>(val + value_range * (n + 1));
        if(n % 2 == 0) {
            mapping = static_cast<T>(upper + lower - mapping); // revert (descending region)
        }
    }
    else { // val > upper
        const T n = static_cast<T>((val - upper - 1) / value_range);
        mapping = static_cast<T>(val - value_range * (n + 1));
        if(n % 2 == 0) {
            mapping = static_cast<T>(upper + lower - mapping); // revert (descending region)
        }
    }
    return mapping;
}

/** @brief Deep-clones a vector of adaptor groups (independent per-individual adaptor state). */
template <typename T>
std::vector<GFlatAdaptGroup<T>> cloneGroups(const std::vector<GFlatAdaptGroup<T>> &src) {
    std::vector<GFlatAdaptGroup<T>> out;
    out.reserve(src.size());
    for(const GFlatAdaptGroup<T> &g : src) {
        GFlatAdaptGroup<T> c;
        c.start = g.start;
        c.count = g.count;
        c.adaptor = g.adaptor ? g.adaptor->clone_unique() : nullptr;
        out.push_back(std::move(c));
    }
    return out;
}

/**
 * @brief Captures the adaptor of a single source parameter as a group, if the parameter
 *        contributes slots of type T, is plain (unconstrained) and carries an adaptor.
 *
 * The offset is always advanced by the parameter's T-slot count so the value arrays and
 * groups stay aligned with streamline() order, even for parameters whose adaptor is not
 * captured (constrained or nested object-collections).
 */
template <typename T>
void captureGroup(
    const GParameterBase *p,
    std::vector<GFlatAdaptGroup<T>> &groups,
    ChannelLayout<T> &chan,
    std::size_t &offset
) {
    const std::size_t n = p->countParameters<T>(activityMode::DEFAULTACTIVITYMODE);
    if(n == 0) {
        return;
    }

    bool constrained = false;
    if constexpr(not std::is_same_v<T, bool>) {
        constrained = (dynamic_cast<const GConstrainedNumT<T> *>(p) != nullptr) ||
                      (dynamic_cast<const GConstrainedNumCollectionT<T> *>(p) != nullptr);
    }

    if(constrained) {
        for(std::size_t k = offset; k < offset + n && k < chan.kind.size(); ++k) {
            chan.kind[k] = SlotKind::Constrained;
        }
    }

    // Capture the adaptor for every adaptor-bearing parameter -- plain, or constrained (both the
    // FP and the integer fold are replicated; see foldIntoRange / foldIntoRangeInt).
    const auto *with_ad = dynamic_cast<const GParameterBaseWithAdaptorsT<T> *>(p);
    if(with_ad != nullptr) {
        GFlatAdaptGroup<T> g;
        g.start = offset;
        g.count = n;
        g.constrained = constrained;
        g.adaptor = with_ad->getAdaptor().clone_unique();
        groups.push_back(std::move(g));
    }

    offset += n;
}

/** @brief Applies the adaptors of all numeric (double / float / int32) groups to a value array. */
template <typename T>
std::size_t adaptNumericGroups(
    std::vector<GFlatAdaptGroup<T>> &groups,
    std::vector<T> &vals,
    const ChannelLayout<T> &layout,
    Gem::Hap::GRandomBase &gr
) {
    auto fold = [&](T &v, std::size_t slot) {
        if constexpr(std::is_floating_point_v<T>) {
            v = foldIntoRange<T>(v, layout.lower[slot], layout.upper[slot]);
        }
        else if constexpr(std::is_integral_v<T>) {
            v = foldIntoRangeInt<T>(v, layout.lower[slot], layout.upper[slot]);
        }
    };

    std::size_t n_adapted = 0;
    for(GFlatAdaptGroup<T> &g : groups) {
        if(not g.adaptor || g.count == 0) {
            continue;
        }
        // range = upper - lower (the same "comparative range" the tree's range() returns).
        const T range = (layout.upper.size() > g.start) ? (layout.upper[g.start] - layout.lower[g.start])
                                                        : static_cast<T>(1);

        if(g.count == 1) {
            // Scalar fast path (a standalone parameter): adapt the value in place, no temp vector.
            n_adapted += g.adaptor->adapt(vals[g.start], range, gr);
            if(g.constrained) {
                fold(vals[g.start], g.start);
            }
            continue;
        }

        std::vector<T> slice(vals.begin() + static_cast<std::ptrdiff_t>(g.start),
                             vals.begin() + static_cast<std::ptrdiff_t>(g.start + g.count));
        n_adapted += g.adaptor->adapt(slice, range, gr);
        // Constrained slots: fold the post-adaption value back into [lower, upper) and store the
        // folded result -- mirroring GConstrainedNumT::value(), which resets its internal value to
        // the transferred result so it never drifts unbounded.
        if(g.constrained) {
            for(std::size_t k = 0; k < g.count; ++k) {
                fold(slice[k], g.start + k);
            }
        }
        std::copy(slice.begin(), slice.end(), vals.begin() + static_cast<std::ptrdiff_t>(g.start));
    }
    return n_adapted;
}

/******************************************************************************/
// Data-oriented DOUBLE channel: stateless EA/SA adapt kernel + config/state extraction.
// No adaptor objects: the static config lives in the shared layout, the evolving state
// (sigma, ad_prob, counter) in the individual. Thread-safe -- touches only its arguments.

/** @brief Extracts the static Gauss config + initial per-individual state from one source param. */
void captureDoubleGroup(
    const GParameterBase *p,
    ChannelLayout<double> &chan,
    std::vector<FlatGaussConfig> &configs,
    std::vector<double> &sigma,
    std::vector<double> &ad_prob,
    std::vector<std::uint32_t> &counter,
    std::size_t &offset
) {
    const std::size_t n = p->countParameters<double>(activityMode::DEFAULTACTIVITYMODE);
    if(n == 0) {
        return;
    }

    const bool constrained = (dynamic_cast<const GConstrainedNumT<double> *>(p) != nullptr) ||
                             (dynamic_cast<const GConstrainedNumCollectionT<double> *>(p) != nullptr);
    if(constrained) {
        for(std::size_t k = offset; k < offset + n && k < chan.kind.size(); ++k) {
            chan.kind[k] = SlotKind::Constrained;
        }
    }

    FlatGaussConfig cfg;
    cfg.start = offset;
    cfg.count = n;
    cfg.constrained = constrained;

    double s0 = 0.;
    double a0 = 0.;
    std::uint32_t c0 = 0;

    const auto *with_ad = dynamic_cast<const GParameterBaseWithAdaptorsT<double> *>(p);
    if(with_ad != nullptr) {
        const GAdaptorT<double, double> &base_ad = with_ad->getAdaptor();
        const auto *gauss = dynamic_cast<const GNumGaussAdaptorT<double, double> *>(&base_ad);
        if(gauss != nullptr) { // only Gauss adaptors are flattened (the default for doubles)
            cfg.present = true;
            cfg.sigma_sigma = gauss->getSigmaAdaptionRate();
            const auto sr = gauss->getSigmaRange();
            cfg.min_sigma = std::get<0>(sr);
            cfg.max_sigma = std::get<1>(sr);
            cfg.adapt_ad_prob = base_ad.getAdaptAdProb();
            const auto ar = base_ad.getAdProbRange();
            cfg.min_ad_prob = std::get<0>(ar);
            cfg.max_ad_prob = std::get<1>(ar);
            cfg.adapt_adaption_probability = base_ad.getAdaptAdaptionProbability();
            cfg.adaption_threshold = base_ad.getAdaptionThreshold();
            switch(base_ad.getAdaptionMode()) {
            case adaptionMode::WITHPROBABILITY:
                cfg.mode = FlatAdaptionMode::WithProbability;
                break;
            case adaptionMode::ALWAYS:
                cfg.mode = FlatAdaptionMode::Always;
                break;
            default:
                cfg.mode = FlatAdaptionMode::Never;
                break;
            }
            s0 = gauss->getSigma();
            a0 = base_ad.getAdaptionProbability();
            c0 = base_ad.getAdaptionCounter();
        }
    }

    configs.push_back(cfg);
    sigma.push_back(s0); // one state slot per group, kept aligned with configs (even if !present)
    ad_prob.push_back(a0);
    counter.push_back(c0);

    offset += n;
}

/** @brief Stateless EA/SA adapt kernel for one double Gauss group (replicates GAdaptorT::adapt). */
std::size_t adaptDoubleGroup(
    const FlatGaussConfig &cfg,
    double *vals,
    double &sigma,
    double &ad_prob,
    std::uint32_t &counter,
    const ChannelLayout<double> &dlayout,
    Gem::Hap::g_normal_distribution<double> &nd,
    Gem::Hap::g_bernoulli_distribution &bd,
    Gem::Hap::GRandomBase &gr
) {
    // The "comparative range" the tree's range() returns (upper - lower), uniform across the group.
    const double range = (dlayout.upper.size() > cfg.start)
                             ? (dlayout.upper[cfg.start] - dlayout.lower[cfg.start])
                             : 1.0;

    // ad_prob self-adaption (once per group), mirroring GAdaptorT::adapt().
    if(cfg.adapt_ad_prob > 0.) {
        ad_prob *= std::exp(nd(gr, Gem::Hap::g_normal_distribution<double>::param_type(0., cfg.adapt_ad_prob)));
        Gem::Common::enforceRangeConstraint<double>(ad_prob, cfg.min_ad_prob, cfg.max_ad_prob, "GFlatParameters adapt ad_prob");
    }

    // sigma self-adaption (GAdaptorT::adaptAdaption + GNumGaussAdaptorT::customAdaptAdaption).
    auto adaptSigma = [&]() {
        bool do_sigma = false;
        if(cfg.adaption_threshold > 0) {
            if(++counter >= cfg.adaption_threshold) {
                counter = 0;
                do_sigma = true;
            }
        }
        else if(cfg.adapt_adaption_probability > 0.) {
            if(bd(gr, Gem::Hap::g_bernoulli_distribution::param_type(std::abs(cfg.adapt_adaption_probability)))) {
                do_sigma = true;
            }
        }
        if(do_sigma) {
            const double sb = sigma;
            const double mult = std::exp(nd(gr, Gem::Hap::g_normal_distribution<double>::param_type(0., std::abs(cfg.sigma_sigma))));
            sigma *= mult;
            if(sigma == sb) { // ULP guarantee (matches the tree)
                const double dir = (mult < 1.) ? std::numeric_limits<double>::lowest()
                                               : std::numeric_limits<double>::max();
                sigma = std::nextafter(sb, dir);
            }
            Gem::Common::enforceRangeConstraint<double>(sigma, cfg.min_sigma, cfg.max_sigma, "GFlatParameters adapt sigma");
        }
    };

    // value mutation (GFPGaussAdaptorT::customAdaptions).
    auto mutateValue = [&](double &v) {
        const double before = v;
        const double delta = range * nd(gr, Gem::Hap::g_normal_distribution<double>::param_type(0., sigma));
        v = before + delta;
        if(v == before) { // ULP guarantee
            const double dir = (delta < 0.) ? std::numeric_limits<double>::lowest()
                                            : std::numeric_limits<double>::max();
            v = std::nextafter(before, dir);
        }
    };

    std::size_t n_adapted = 0;
    if(cfg.mode == FlatAdaptionMode::WithProbability) {
        for(std::size_t k = 0; k < cfg.count; ++k) {
            if(bd(gr, Gem::Hap::g_bernoulli_distribution::param_type(std::abs(ad_prob)))) {
                adaptSigma();
                mutateValue(vals[k]);
                ++n_adapted;
            }
        }
    }
    else if(cfg.mode == FlatAdaptionMode::Always) {
        for(std::size_t k = 0; k < cfg.count; ++k) {
            adaptSigma();
            mutateValue(vals[k]);
            ++n_adapted;
        }
    }

    // Constrained slots: fold each post-adaption value back into [lower, upper).
    if(cfg.constrained) {
        for(std::size_t k = 0; k < cfg.count; ++k) {
            vals[k] = foldIntoRange<double>(vals[k], dlayout.lower[cfg.start + k], dlayout.upper[cfg.start + k]);
        }
    }
    return n_adapted;
}

} /* anonymous namespace */

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

    // Capture one adaptor group per source parameter object (EA/SA mutation). A standalone
    // parameter yields a count-1 group with its own adaptor; a collection yields a count-N group
    // sharing one adaptor -- so individual parameters keep individual adaptor settings. Constrained
    // parameters and nested object-collections are skipped (transported but not yet mutated); the
    // offsets still advance so everything stays aligned with streamline() order.
    std::size_t d_off = 0;
    std::size_t f_off = 0;
    std::size_t i_off = 0;
    std::size_t b_off = 0;
    for(const auto &p_ptr : src) {
        const GParameterBase *p = p_ptr.get();
        captureDoubleGroup(p, L.d, L.d_adaptor_groups, flat->d_sigma_, flat->d_ad_prob_, flat->d_counter_, d_off);
        captureGroup<float>(p, flat->f_groups_, L.f, f_off);
        captureGroup<std::int32_t>(p, flat->i_groups_, L.i, i_off);
        captureGroup<bool>(p, flat->b_groups_, L.b, b_off);
    }

    return flat;
}

/******************************************************************************/
// Copy semantics: share the immutable layout, deep-copy the values, and DEEP-CLONE the
// per-group adaptors (their mutation state is per-individual and must not be shared).

GFlatParameters::GFlatParameters(const GFlatParameters &cp)
  : GParameterBase(cp)
  , layout_(cp.layout_)
  , dv_(cp.dv_)
  , fv_(cp.fv_)
  , iv_(cp.iv_)
  , bv_(cp.bv_)
  , d_sigma_(cp.d_sigma_)   // double channel state is flat -> a plain copy (the clone win)
  , d_ad_prob_(cp.d_ad_prob_)
  , d_counter_(cp.d_counter_)
  , f_groups_(cloneGroups(cp.f_groups_))
  , i_groups_(cloneGroups(cp.i_groups_))
  , b_groups_(cloneGroups(cp.b_groups_)) { /* nothing */
}

GFlatParameters &GFlatParameters::operator=(const GFlatParameters &cp) {
    if(this != &cp) {
        GParameterBase::operator=(cp);
        layout_ = cp.layout_;
        dv_ = cp.dv_;
        fv_ = cp.fv_;
        iv_ = cp.iv_;
        bv_ = cp.bv_;
        d_sigma_ = cp.d_sigma_;
        d_ad_prob_ = cp.d_ad_prob_;
        d_counter_ = cp.d_counter_;
        f_groups_ = cloneGroups(cp.f_groups_);
        i_groups_ = cloneGroups(cp.i_groups_);
        b_groups_ = cloneGroups(cp.b_groups_);
    }
    return *this;
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
    for(std::size_t k = 0; k < dv_.size(); ++k) {
        double v = vec.at(pos);
        ++pos;
        if(k < layout_->d.kind.size() && layout_->d.kind[k] == SlotKind::Constrained) {
            v = foldIntoRange<double>(v, layout_->d.lower[k], layout_->d.upper[k]);
        }
        dv_[k] = v;
    }
}

void GFlatParameters::assignFloatValueVector(
    const std::vector<float> &vec,
    std::size_t &pos,
    const activityMode &
) {
    for(std::size_t k = 0; k < fv_.size(); ++k) {
        float v = vec.at(pos);
        ++pos;
        if(k < layout_->f.kind.size() && layout_->f.kind[k] == SlotKind::Constrained) {
            v = foldIntoRange<float>(v, layout_->f.lower[k], layout_->f.upper[k]);
        }
        fv_[k] = v;
    }
}

void GFlatParameters::assignInt32ValueVector(
    const std::vector<std::int32_t> &vec,
    std::size_t &pos,
    const activityMode &
) {
    for(std::size_t k = 0; k < iv_.size(); ++k) {
        std::int32_t v = vec.at(pos);
        ++pos;
        if(k < layout_->i.kind.size() && layout_->i.kind[k] == SlotKind::Constrained) {
            v = foldIntoRangeInt<std::int32_t>(v, layout_->i.lower[k], layout_->i.upper[k]);
        }
        iv_[k] = v;
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
// Adaption (EA/SA mutation): apply each group's adaptor to its slice of the value
// arrays, reusing the existing, validated adaptor math. Constrained parameters are
// not captured as groups yet, so they are left unchanged here (a later phase adds
// the internal-value + transfer handling needed to mutate them correctly).

std::size_t GFlatParameters::adapt_(Gem::Hap::GRandomBase &gr) {
    std::size_t n_adapted = 0;
    // Double channel: data-oriented stateless kernel over the shared config + this individual's
    // flat per-group state arrays (no adaptor objects).
    const std::vector<FlatGaussConfig> &d_cfgs = layout_->d_adaptor_groups;
    // Distribution objects are constructed once and reused across all groups (cheap + matches the
    // tree's member-distribution reuse); each adapt() runs on its own thread, so these locals are
    // private to this call -- the kernel stays stateless w.r.t. anything shared.
    Gem::Hap::g_normal_distribution<double> nd;
    Gem::Hap::g_bernoulli_distribution bd;
    for(std::size_t g = 0; g < d_cfgs.size(); ++g) {
        const FlatGaussConfig &cfg = d_cfgs[g];
        if(not cfg.present || cfg.count == 0) {
            continue;
        }
        n_adapted += adaptDoubleGroup(cfg, dv_.data() + cfg.start, d_sigma_[g], d_ad_prob_[g], d_counter_[g], layout_->d, nd, bd, gr);
    }

    n_adapted += adaptNumericGroups(f_groups_, fv_, layout_->f, gr);
    n_adapted += adaptNumericGroups(i_groups_, iv_, layout_->i, gr);

    // Booleans are stored as bytes; bridge to the adaptor's std::vector<bool> interface.
    for(GFlatAdaptGroup<bool> &g : b_groups_) {
        if(not g.adaptor || g.count == 0) {
            continue;
        }
        if(g.count == 1) {
            // Scalar fast path (a standalone boolean parameter).
            bool b = (bv_[g.start] != 0);
            n_adapted += g.adaptor->adapt(b, true, gr);
            bv_[g.start] = b ? 1 : 0;
            continue;
        }
        std::vector<bool> slice;
        slice.reserve(g.count);
        for(std::size_t k = 0; k < g.count; ++k) {
            slice.push_back(bv_[g.start + k] != 0);
        }
        n_adapted += g.adaptor->adapt(slice, true, gr);
        for(std::size_t k = 0; k < g.count; ++k) {
            bv_[g.start + k] = slice[k] ? 1 : 0;
        }
    }

    return n_adapted;
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
    // share the pointer rather than deep-copying it; the adaptors, however, carry
    // per-individual mutation state and are deep-cloned.
    layout_ = p_load->layout_;
    dv_ = p_load->dv_;
    fv_ = p_load->fv_;
    iv_ = p_load->iv_;
    bv_ = p_load->bv_;
    d_sigma_ = p_load->d_sigma_;
    d_ad_prob_ = p_load->d_ad_prob_;
    d_counter_ = p_load->d_counter_;
    f_groups_ = cloneGroups(p_load->f_groups_);
    i_groups_ = cloneGroups(p_load->i_groups_);
    b_groups_ = cloneGroups(p_load->b_groups_);
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
