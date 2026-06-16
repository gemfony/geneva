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
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

// Geneva headers go here
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GGenomeLayout.hpp"

namespace Gem::Geneva::Genome {

/******************************************************************************/
/**
 * The product of GGenomeBuilder::build(): the per-individual value arrays plus a handle to the
 * shared, immutable adaption layout. A flat individual is set up by handing this to
 * GFlatGenome::setGenome(). The layout is shared (std::shared_ptr<const>) -- a factory builds it
 * once and every produced individual binds to the same instance, so per-individual cost is only the
 * value-array copy.
 */
struct GenomeData {
    std::vector<double> dv;        ///< the double channel start values
    std::vector<float> fv;         ///< the float channel start values
    std::vector<std::int32_t> iv;  ///< the int32 channel start values
    std::vector<std::uint8_t> bv;  ///< the bool channel start values (1/0)
    std::shared_ptr<const GGenomeLayout> layout; ///< the shared structural descriptor
};

/******************************************************************************/
/**
 * A fluent handle to the group of parameters just added to a GGenomeBuilder. It lets the user attach
 * an adaptor and tune the init policy without knowing the storage layout. The handle is a thin view
 * onto the builder's channel + value array; it must not outlive the builder. Returned by the add*
 * methods and meant to be used immediately, e.g.
 *   b.addDouble(0., -10., 10.).gaussAdaptor(0.5, 0.8, 1e-3, 2., 1.);
 */
template <typename T>
class ParamHandle {
public:
    using adfp = adaption_fp_t<T>;

    ParamHandle(
        ChannelLayout<T> *ch,
        std::vector<T> *values,
        std::size_t group_index,
        std::size_t group_count = 1,
        GGenomeLayout *owner = nullptr
    )
      : ch_(ch)
      , values_(values)
      , gi_(group_index)
      , count_(group_count)
      , owner_(owner) {
        /* nothing */
    }

    // NOTE: the adaptor-attaching methods (gaussAdaptor / biGaussAdaptor / intGaussAdaptor / flipAdaptor)
    // have been removed: the genome layout carries only STRUCTURE now. Adaptors are authored on the
    // OA-owned GAdaptionConfig via its fluent API (groupDouble(i).gauss(...) / groupInt32(i).intGauss(...) /
    // groupBool(i).flip(...) / forLabel(...)), built from the finished genome. Only the structural tuning
    // (init / perimeter / adaptionMode / label) remains on the builder handle.

    /** @brief Sets the adaption mode for this group; NEVER marks it inactive (never mutated). Structural --
     *  the adaptor mode itself is set on the OA config. */
    ParamHandle &adaptionMode(Gem::Geneva::adaptionMode mode) {
        this->forEachGroup([&](GroupStructure<T> &g) {
            g.active = (mode != adaptionMode::NEVER);
            for(std::uint32_t k = 0; k < g.len; ++k) {
                ch_->active.at(g.start + k) = g.active ? 1 : 0;
            }
        });
        return *this;
    }

    /** @brief Sets a fixed start value for every parameter of this handle's group(s). */
    ParamHandle &init(T v) {
        this->forEachGroup([&](GroupStructure<T> &g) {
            for(std::uint32_t k = 0; k < g.len; ++k) {
                values_->at(g.start + k) = v;
            }
        });
        return *this;
    }

    /** @brief Sets the random-initialization perimeter for this handle's group(s). */
    ParamHandle &perimeter(T lo, T hi) {
        this->forEachGroup([&](GroupStructure<T> &g) {
            for(std::uint32_t k = 0; k < g.len; ++k) {
                ch_->init_lower.at(g.start + k) = lo;
                ch_->init_upper.at(g.start + k) = hi;
            }
        });
        return *this;
    }

    /**
     * @brief Stamps an interned label onto every group this handle spans. The label string is
     * deduplicated into the shared layout's label table; many handles may share one label (one-to-many).
     * The OA-owned adaption config can then address these groups by name (forLabel) rather than by index.
     */
    ParamHandle &label(const std::string &name) {
        if(owner_ != nullptr) {
            const std::int32_t id = owner_->internLabel(name);
            this->forEachGroup([&](GroupStructure<T> &g) { g.label_id = id; });
        }
        return *this;
    }

private:
    /** @brief Applies fn to every group this handle spans (1 for a single/group add, n for an array). */
    template <typename F>
    void forEachGroup(F fn) {
        for(std::size_t gi = gi_; gi < gi_ + count_; ++gi) {
            fn(ch_->groups.at(gi));
        }
    }

    ChannelLayout<T> *ch_;     ///< the channel this group lives in (owned by the builder)
    std::vector<T> *values_;   ///< the builder's start-value array for this channel
    std::size_t gi_;           ///< the index of the first group within the channel
    std::size_t count_;        ///< number of consecutive groups this handle spans
    GGenomeLayout *owner_;     ///< the owning layout (for interning labels); may be null
};

/******************************************************************************/
/**
 * The imperative authoring API for a flat genome: the user declares each parameter (or group / array
 * of parameters) once, optionally attaching an adaptor, and calls build() to obtain a GenomeData (value
 * arrays + shared layout). This replaces the tree's "push_back parameter objects + adaptors" idiom
 * without losing fine-grained control (see the migration plan's mapping table).
 *
 * Three group shapes per type:
 *  - addX(init[,min,max])          : one parameter, its own adaption group (size 1)
 *  - addXGroup(n[,min,max])        : n parameters sharing ONE adaption group (like a collection)
 *  - addXArray(n[,min,max])        : n parameters, each its own adaption group (n groups of 1)
 */
class GGenomeBuilder {
public:
    /***************************************************************************/
    // Double channel

    ParamHandle<double> addDouble(double init, double min, double max) {
        return addOne(layout_.d, dv_, init, min, max, ParamKind::Constrained);
    }
    ParamHandle<double> addDouble(double init) {
        return addOne(layout_.d, dv_, init, 0., 1., ParamKind::Plain);
    }
    ParamHandle<double> addDoubleGroup(std::size_t n, double min, double max) {
        return addGrouped(layout_.d, dv_, n, min, max, ParamKind::Constrained);
    }
    ParamHandle<double> addDoubleArray(std::size_t n, double min, double max) {
        return addArray(layout_.d, dv_, n, min, max, ParamKind::Constrained);
    }
    // Unbounded (plain) collections -- like GDoubleCollection / GDoubleObjectCollection: min/max are
    // not constraints (no fold), they only set the random-init perimeter + the Gauss step range.
    ParamHandle<double> addDoublePlainGroup(std::size_t n, double initMin, double initMax) {
        return addGrouped(layout_.d, dv_, n, initMin, initMax, ParamKind::Plain, /*plain_init_from_bounds=*/true);
    }
    ParamHandle<double> addDoublePlainArray(std::size_t n, double initMin, double initMax) {
        return addArray(layout_.d, dv_, n, initMin, initMax, ParamKind::Plain, /*plain_init_from_bounds=*/true);
    }

    /***************************************************************************/
    // Float channel

    ParamHandle<float> addFloat(float init, float min, float max) {
        return addOne(layout_.f, fv_, init, min, max, ParamKind::Constrained);
    }
    ParamHandle<float> addFloat(float init) {
        return addOne(layout_.f, fv_, init, 0.f, 1.f, ParamKind::Plain);
    }
    ParamHandle<float> addFloatGroup(std::size_t n, float min, float max) {
        return addGrouped(layout_.f, fv_, n, min, max, ParamKind::Constrained);
    }
    ParamHandle<float> addFloatArray(std::size_t n, float min, float max) {
        return addArray(layout_.f, fv_, n, min, max, ParamKind::Constrained);
    }

    /***************************************************************************/
    // Int32 channel (Gauss adaptor not yet available -- flip adaptor added later)

    ParamHandle<std::int32_t> addInt32(std::int32_t init, std::int32_t min, std::int32_t max) {
        return addOne(layout_.i, iv_, init, min, max, ParamKind::Constrained);
    }
    ParamHandle<std::int32_t> addInt32(std::int32_t init) {
        return addOne(
            layout_.i,
            iv_,
            init,
            std::numeric_limits<std::int32_t>::lowest(),
            std::numeric_limits<std::int32_t>::max(),
            ParamKind::Plain
        );
    }
    ParamHandle<std::int32_t> addInt32Group(std::size_t n, std::int32_t min, std::int32_t max) {
        return addGrouped(layout_.i, iv_, n, min, max, ParamKind::Constrained);
    }
    ParamHandle<std::int32_t> addInt32Array(std::size_t n, std::int32_t min, std::int32_t max) {
        return addArray(layout_.i, iv_, n, min, max, ParamKind::Constrained);
    }

    /***************************************************************************/
    // Bool channel

    ParamHandle<bool> addBool(bool init) {
        return addOne(layout_.b, bvBool_, init, false, true, ParamKind::Plain);
    }
    ParamHandle<bool> addBoolGroup(std::size_t n) {
        return addGrouped(layout_.b, bvBool_, n, false, true, ParamKind::Plain);
    }
    ParamHandle<bool> addBoolArray(std::size_t n) {
        return addArray(layout_.b, bvBool_, n, false, true, ParamKind::Plain);
    }

    /***************************************************************************/
    /** @brief Produces the value arrays + an interned, shared adaption layout. */
    GenomeData build() const {
        GenomeData g;
        g.dv = dv_;
        g.fv = fv_;
        g.iv = iv_;
        g.bv.reserve(bvBool_.size());
        for(bool v : bvBool_) {
            g.bv.push_back(v ? std::uint8_t(1) : std::uint8_t(0));
        }
        g.layout = std::make_shared<const GGenomeLayout>(layout_);
        return g;
    }

    /** @brief Produces the shared layout only (for factories that bind value arrays separately). */
    std::shared_ptr<const GGenomeLayout> buildLayout() const {
        return std::make_shared<const GGenomeLayout>(layout_);
    }

private:
    /***************************************************************************/
    // One parameter = one group of size 1.
    template <typename T>
    ParamHandle<T> addOne(
        ChannelLayout<T> &ch,
        std::vector<T> &values,
        T init,
        T min,
        T max,
        ParamKind kind,
        bool plain_init_from_bounds = false
    ) {
        return addGroupImpl(ch, values, 1, init, min, max, kind, plain_init_from_bounds);
    }

    // n parameters sharing one group.
    template <typename T>
    ParamHandle<T> addGrouped(
        ChannelLayout<T> &ch,
        std::vector<T> &values,
        std::size_t n,
        T min,
        T max,
        ParamKind kind,
        bool plain_init_from_bounds = false
    ) {
        return addGroupImpl(ch, values, n, min, min, max, kind, plain_init_from_bounds);
    }

    // n parameters, each its own group of size 1. Returns a handle to the first group.
    template <typename T>
    ParamHandle<T> addArray(
        ChannelLayout<T> &ch,
        std::vector<T> &values,
        std::size_t n,
        T min,
        T max,
        ParamKind kind,
        bool plain_init_from_bounds = false
    ) {
        const std::size_t first_group = ch.groups.size();
        for(std::size_t k = 0; k < n; ++k) {
            addGroupImpl(ch, values, 1, min, min, max, kind, plain_init_from_bounds);
        }
        // The handle spans all n freshly-created groups, so an adaptor / init / perimeter applied to it
        // configures every one of them (not just the first).
        return ParamHandle<T>(&ch, &values, first_group, n, &layout_);
    }

    /***************************************************************************/
    /**
     * Appends one group of `len` values to a channel, extending all per-value vectors and registering
     * the group. `init` seeds the start value of every member; the init perimeter defaults to the
     * bounds (Constrained) or [0,1] (Plain), and can be overridden via the returned handle. For an
     * unbounded (Plain) *collection* the caller passes plain_init_from_bounds = true so the random-init
     * perimeter and the comparative Gauss step range follow [min, max] (like GDoubleCollection) rather
     * than the conservative [0, 1] default used for a bare unbounded scalar.
     */
    template <typename T>
    ParamHandle<T> addGroupImpl(
        ChannelLayout<T> &ch,
        std::vector<T> &values,
        std::size_t len,
        T init,
        T min,
        T max,
        ParamKind kind,
        bool plain_init_from_bounds
    ) {
        const auto start = static_cast<std::uint32_t>(ch.size());

        const bool from_bounds = (kind == ParamKind::Constrained) || plain_init_from_bounds;
        const T init_lo = from_bounds ? min : T(0);
        const T init_hi = from_bounds ? max : T(1);

        for(std::size_t k = 0; k < len; ++k) {
            ch.lower.push_back(min);
            ch.upper.push_back(max);
            ch.init_lower.push_back(init_lo);
            ch.init_upper.push_back(init_hi);
            ch.kind.push_back(kind);
            ch.active.push_back(1);
            values.push_back(init);
        }

        GroupStructure<T> g;
        g.start = start;
        g.len = static_cast<std::uint32_t>(len);
        g.active = true;
        // The comparative range mirrors the tree: (upper-lower) for constrained, the init range
        // otherwise. Used to scale the Gauss step independently of a parameter's value range. Relevant
        // for the FP channels (Gauss / bi-Gauss) and the int32 channel (integer Gauss adaptor); left at
        // 1 for bool (flip only, no range).
        if constexpr(std::is_floating_point_v<T> || (std::is_integral_v<T> && !std::is_same_v<T, bool>)) {
            g.range = (kind == ParamKind::Constrained) ? static_cast<T>(max - min)
                                                       : static_cast<T>(init_hi - init_lo);
        }
        else {
            g.range = T(1);
        }

        const std::size_t gi = ch.groups.size();
        ch.groups.push_back(g);
        return ParamHandle<T>(&ch, &values, gi, 1, &layout_);
    }

    /***************************************************************************/
    // The layout under construction and the matching start-value arrays.
    GGenomeLayout layout_;
    std::vector<double> dv_;
    std::vector<float> fv_;
    std::vector<std::int32_t> iv_;
    std::vector<bool> bvBool_;
};

/******************************************************************************/

} /* namespace Gem::Geneva::Genome */
