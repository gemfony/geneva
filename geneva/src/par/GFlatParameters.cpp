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

/** @brief Extracts static Gauss config + initial per-individual state from one source param (T = double/float). */
template <typename T>
void captureGaussGroup(
    const GParameterBase *p,
    ChannelLayout<T> &chan,
    std::vector<FlatGaussConfig> &configs,
    std::vector<double> &sigma,
    std::vector<double> &ad_prob,
    std::vector<std::uint32_t> &counter,
    std::size_t &offset
) {
    const std::size_t n = p->countParameters<T>(activityMode::DEFAULTACTIVITYMODE);
    if(n == 0) {
        return;
    }

    const bool constrained = (dynamic_cast<const GConstrainedNumT<T> *>(p) != nullptr) ||
                             (dynamic_cast<const GConstrainedNumCollectionT<T> *>(p) != nullptr);
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

    const auto *with_ad = dynamic_cast<const GParameterBaseWithAdaptorsT<T> *>(p);
    if(with_ad != nullptr) {
        const GAdaptorT<T, adaption_fp_type_t<T>> &base_ad = with_ad->getAdaptor();
        const auto *gauss = dynamic_cast<const GNumGaussAdaptorT<T, adaption_fp_type_t<T>> *>(&base_ad);
        if(gauss != nullptr) {
            cfg.present = true;
            cfg.sigma_sigma = static_cast<double>(gauss->getSigmaAdaptionRate());
            const auto sr = gauss->getSigmaRange();
            cfg.min_sigma = static_cast<double>(std::get<0>(sr));
            cfg.max_sigma = static_cast<double>(std::get<1>(sr));
            cfg.adapt_ad_prob = static_cast<double>(base_ad.getAdaptAdProb());
            const auto ar = base_ad.getAdProbRange();
            cfg.min_ad_prob = static_cast<double>(std::get<0>(ar));
            cfg.max_ad_prob = static_cast<double>(std::get<1>(ar));
            cfg.adapt_adaption_probability = static_cast<double>(base_ad.getAdaptAdaptionProbability());
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
            s0 = static_cast<double>(gauss->getSigma());
            a0 = static_cast<double>(base_ad.getAdaptionProbability());
            c0 = base_ad.getAdaptionCounter();
        }
    }

    // A parameter whose adaptions were switched off (setAdaptionsInactive) must never mutate,
    // regardless of the adaptor's own mode -- mirror the tree's per-parameter adaptionsActive() gate.
    if(not p->adaptionsActive()) {
        cfg.mode = FlatAdaptionMode::Never;
    }

    configs.push_back(cfg);
    sigma.push_back(s0);
    ad_prob.push_back(a0);
    counter.push_back(c0);
    offset += n;
}

/** @brief Extracts static flip config + initial state from one source param (T = int32/bool). */
template <typename T>
void captureFlipGroup(
    const GParameterBase *p,
    ChannelLayout<T> &chan,
    std::vector<FlatFlipConfig> &configs,
    std::vector<double> &ad_prob,
    std::vector<std::uint32_t> &counter,
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
        if(constrained) {
            for(std::size_t k = offset; k < offset + n && k < chan.kind.size(); ++k) {
                chan.kind[k] = SlotKind::Constrained;
            }
        }
    }

    FlatFlipConfig cfg;
    cfg.start = offset;
    cfg.count = n;
    cfg.constrained = constrained;

    double a0 = 0.;
    std::uint32_t c0 = 0;

    // The int/bool defaults are flip adaptors (no sigma); any adaptor exposes the ad_prob config.
    const auto *with_ad = dynamic_cast<const GParameterBaseWithAdaptorsT<T> *>(p);
    if(with_ad != nullptr) {
        const GAdaptorT<T, adaption_fp_type_t<T>> &base_ad = with_ad->getAdaptor();
        cfg.present = true;
        cfg.adapt_ad_prob = static_cast<double>(base_ad.getAdaptAdProb());
        const auto ar = base_ad.getAdProbRange();
        cfg.min_ad_prob = static_cast<double>(std::get<0>(ar));
        cfg.max_ad_prob = static_cast<double>(std::get<1>(ar));
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
        a0 = static_cast<double>(base_ad.getAdaptionProbability());
        c0 = base_ad.getAdaptionCounter();
    }

    // A parameter whose adaptions were switched off (setAdaptionsInactive) must never mutate,
    // regardless of the adaptor's own mode -- mirror the tree's per-parameter adaptionsActive() gate.
    if(not p->adaptionsActive()) {
        cfg.mode = FlatAdaptionMode::Never;
    }

    configs.push_back(cfg);
    ad_prob.push_back(a0);
    counter.push_back(c0);
    offset += n;
}

/** @brief ad_prob log-normal self-adaption (once per group), shared by the Gauss + flip kernels. */
inline void selfAdaptAdProb(
    double &ad_prob,
    double adapt_ad_prob,
    double min_ad_prob,
    double max_ad_prob,
    Gem::Hap::g_normal_distribution<double> &nd,
    Gem::Hap::GRandomBase &gr
) {
    if(adapt_ad_prob > 0.) {
        ad_prob *= std::exp(nd(gr, Gem::Hap::g_normal_distribution<double>::param_type(0., adapt_ad_prob)));
        Gem::Common::enforceRangeConstraint<double>(ad_prob, min_ad_prob, max_ad_prob, "GFlatParameters ad_prob");
    }
}

/** @brief Stateless Gauss adapt kernel for one group (T = double/float). Mirrors GAdaptorT::adapt +
 *         GNumGaussAdaptorT self-adaption: ad_prob self-adaption, per-element bernoulli gate, sigma
 *         log-normal self-adaption (+ULP), range*N(0,sigma) value step (+ULP), constrained fold. */
template <typename T>
std::size_t adaptGaussGroup(
    const FlatGaussConfig &cfg,
    T *vals,
    double &sigma,
    double &ad_prob,
    std::uint32_t &counter,
    const ChannelLayout<T> &chan,
    Gem::Hap::g_normal_distribution<double> &nd,
    Gem::Hap::g_bernoulli_distribution &bd,
    Gem::Hap::GRandomBase &gr
) {
    if(cfg.mode == FlatAdaptionMode::Never) {
        return 0; // adaptions switched off: a complete no-op, exactly like the tree's NEVER path
    }

    const double range = (chan.upper.size() > cfg.start)
                             ? (static_cast<double>(chan.upper[cfg.start]) - static_cast<double>(chan.lower[cfg.start]))
                             : 1.0;

    selfAdaptAdProb(ad_prob, cfg.adapt_ad_prob, cfg.min_ad_prob, cfg.max_ad_prob, nd, gr);

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
            if(sigma == sb) {
                const double dir = (mult < 1.) ? std::numeric_limits<double>::lowest() : std::numeric_limits<double>::max();
                sigma = std::nextafter(sb, dir);
            }
            Gem::Common::enforceRangeConstraint<double>(sigma, cfg.min_sigma, cfg.max_sigma, "GFlatParameters sigma");
        }
    };

    auto mutateValue = [&](T &v) {
        const T before = v;
        const T candidate = static_cast<T>(static_cast<double>(before) + range * nd(gr, Gem::Hap::g_normal_distribution<double>::param_type(0., sigma)));
        v = candidate;
        if(v == before) {
            const T dir = (candidate < before) ? std::numeric_limits<T>::lowest() : std::numeric_limits<T>::max();
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

    if(cfg.constrained) {
        for(std::size_t k = 0; k < cfg.count; ++k) {
            vals[k] = foldIntoRange<T>(vals[k], chan.lower[cfg.start + k], chan.upper[cfg.start + k]);
        }
    }
    return n_adapted;
}

/** @brief Stateless flip adapt kernel for one integer group. Mirrors GNumFlipAdaptorT::customAdaptions
 *         (value +-1 with 50/50 direction); only ad_prob self-adapts (no sigma). */
std::size_t adaptFlipIntGroup(
    const FlatFlipConfig &cfg,
    std::int32_t *vals,
    double &ad_prob,
    const ChannelLayout<std::int32_t> &chan,
    Gem::Hap::g_normal_distribution<double> &nd,
    Gem::Hap::g_bernoulli_distribution &bd,
    Gem::Hap::GRandomBase &gr
) {
    if(cfg.mode == FlatAdaptionMode::Never) {
        return 0; // adaptions switched off: a complete no-op, exactly like the tree's NEVER path
    }

    selfAdaptAdProb(ad_prob, cfg.adapt_ad_prob, cfg.min_ad_prob, cfg.max_ad_prob, nd, gr);

    auto flip = [&](std::int32_t &v) {
        v += bd(gr, Gem::Hap::g_bernoulli_distribution::param_type(0.5)) ? 1 : -1;
    };

    std::size_t n_adapted = 0;
    if(cfg.mode == FlatAdaptionMode::WithProbability) {
        for(std::size_t k = 0; k < cfg.count; ++k) {
            if(bd(gr, Gem::Hap::g_bernoulli_distribution::param_type(std::abs(ad_prob)))) {
                flip(vals[k]);
                ++n_adapted;
            }
        }
    }
    else if(cfg.mode == FlatAdaptionMode::Always) {
        for(std::size_t k = 0; k < cfg.count; ++k) {
            flip(vals[k]);
            ++n_adapted;
        }
    }

    if(cfg.constrained) {
        for(std::size_t k = 0; k < cfg.count; ++k) {
            vals[k] = foldIntoRangeInt<std::int32_t>(vals[k], chan.lower[cfg.start + k], chan.upper[cfg.start + k]);
        }
    }
    return n_adapted;
}

/** @brief Stateless flip adapt kernel for one boolean group. Mirrors GBooleanAdaptor::customAdaptions
 *         (negate the bit); only ad_prob self-adapts (no sigma). Bools are stored as bytes. */
std::size_t adaptFlipBoolGroup(
    const FlatFlipConfig &cfg,
    std::uint8_t *vals,
    double &ad_prob,
    Gem::Hap::g_normal_distribution<double> &nd,
    Gem::Hap::g_bernoulli_distribution &bd,
    Gem::Hap::GRandomBase &gr
) {
    if(cfg.mode == FlatAdaptionMode::Never) {
        return 0; // adaptions switched off: a complete no-op, exactly like the tree's NEVER path
    }

    selfAdaptAdProb(ad_prob, cfg.adapt_ad_prob, cfg.min_ad_prob, cfg.max_ad_prob, nd, gr);

    std::size_t n_adapted = 0;
    if(cfg.mode == FlatAdaptionMode::WithProbability) {
        for(std::size_t k = 0; k < cfg.count; ++k) {
            if(bd(gr, Gem::Hap::g_bernoulli_distribution::param_type(std::abs(ad_prob)))) {
                vals[k] = (vals[k] != 0) ? 0 : 1;
                ++n_adapted;
            }
        }
    }
    else if(cfg.mode == FlatAdaptionMode::Always) {
        for(std::size_t k = 0; k < cfg.count; ++k) {
            vals[k] = (vals[k] != 0) ? 0 : 1;
            ++n_adapted;
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
        captureGaussGroup<double>(p, L.d, L.d_adaptor_groups, flat->d_sigma_, flat->d_ad_prob_, flat->d_counter_, d_off);
        captureGaussGroup<float>(p, L.f, L.f_adaptor_groups, flat->f_sigma_, flat->f_ad_prob_, flat->f_counter_, f_off);
        captureFlipGroup<std::int32_t>(p, L.i, L.i_adaptor_groups, flat->i_ad_prob_, flat->i_counter_, i_off);
        captureFlipGroup<bool>(p, L.b, L.b_adaptor_groups, flat->b_ad_prob_, flat->b_counter_, b_off);
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
  // All adaptor state is flat now -> a clone is a plain copy of these arrays (the clone win).
  , d_sigma_(cp.d_sigma_)
  , d_ad_prob_(cp.d_ad_prob_)
  , d_counter_(cp.d_counter_)
  , f_sigma_(cp.f_sigma_)
  , f_ad_prob_(cp.f_ad_prob_)
  , f_counter_(cp.f_counter_)
  , i_ad_prob_(cp.i_ad_prob_)
  , i_counter_(cp.i_counter_)
  , b_ad_prob_(cp.b_ad_prob_)
  , b_counter_(cp.b_counter_) { /* nothing */
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
        f_sigma_ = cp.f_sigma_;
        f_ad_prob_ = cp.f_ad_prob_;
        f_counter_ = cp.f_counter_;
        i_ad_prob_ = cp.i_ad_prob_;
        i_counter_ = cp.i_counter_;
        b_ad_prob_ = cp.b_ad_prob_;
        b_counter_ = cp.b_counter_;
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
// Adaption (EA/SA mutation): stateless data-oriented kernels over the shared config
// (layout_->{d,f,i,b}_adaptor_groups) + this individual's flat per-group state arrays. No
// adaptor objects. Thread-safe: the kernels touch only this individual's data + the local
// distribution objects, so they compose with the EA's parallel adaptChildren_ pool.

std::size_t GFlatParameters::adapt_(Gem::Hap::GRandomBase &gr) {
    std::size_t n_adapted = 0;

    // Distribution objects are constructed once and reused across all groups (each adapt() runs on
    // its own thread, so these locals are private to this call).
    Gem::Hap::g_normal_distribution<double> nd;
    Gem::Hap::g_bernoulli_distribution bd;

    // double / float: Gauss kernel
    const std::vector<FlatGaussConfig> &d_cfgs = layout_->d_adaptor_groups;
    for(std::size_t g = 0; g < d_cfgs.size(); ++g) {
        const FlatGaussConfig &cfg = d_cfgs[g];
        if(cfg.present && cfg.count > 0) {
            n_adapted += adaptGaussGroup<double>(cfg, dv_.data() + cfg.start, d_sigma_[g], d_ad_prob_[g], d_counter_[g], layout_->d, nd, bd, gr);
        }
    }
    const std::vector<FlatGaussConfig> &f_cfgs = layout_->f_adaptor_groups;
    for(std::size_t g = 0; g < f_cfgs.size(); ++g) {
        const FlatGaussConfig &cfg = f_cfgs[g];
        if(cfg.present && cfg.count > 0) {
            n_adapted += adaptGaussGroup<float>(cfg, fv_.data() + cfg.start, f_sigma_[g], f_ad_prob_[g], f_counter_[g], layout_->f, nd, bd, gr);
        }
    }

    // int / bool: flip kernel (only ad_prob self-adapts -- flip adaptors have no step width)
    const std::vector<FlatFlipConfig> &i_cfgs = layout_->i_adaptor_groups;
    for(std::size_t g = 0; g < i_cfgs.size(); ++g) {
        const FlatFlipConfig &cfg = i_cfgs[g];
        if(cfg.present && cfg.count > 0) {
            n_adapted += adaptFlipIntGroup(cfg, iv_.data() + cfg.start, i_ad_prob_[g], layout_->i, nd, bd, gr);
        }
    }
    const std::vector<FlatFlipConfig> &b_cfgs = layout_->b_adaptor_groups;
    for(std::size_t g = 0; g < b_cfgs.size(); ++g) {
        const FlatFlipConfig &cfg = b_cfgs[g];
        if(cfg.present && cfg.count > 0) {
            n_adapted += adaptFlipBoolGroup(cfg, bv_.data() + cfg.start, b_ad_prob_[g], nd, bd, gr);
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
    f_sigma_ = p_load->f_sigma_;
    f_ad_prob_ = p_load->f_ad_prob_;
    f_counter_ = p_load->f_counter_;
    i_ad_prob_ = p_load->i_ad_prob_;
    i_counter_ = p_load->i_counter_;
    b_ad_prob_ = p_load->b_ad_prob_;
    b_counter_ = p_load->b_counter_;
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
