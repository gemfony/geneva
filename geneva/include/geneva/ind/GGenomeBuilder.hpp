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

    /**
     * @brief Constructs a fluent handle onto a freshly added group (or run of groups) of a builder channel.
     * @param ch The channel the group(s) live in (owned by the builder; not owned by the handle).
     * @param values The builder's start-value array for that channel (not owned by the handle).
     * @param group_index Index of the first group this handle refers to, within the channel's group list.
     * @param group_count Number of consecutive groups this handle spans (1 for a single/group add, n for an array).
     * @param owner The owning layout, used to intern labels; may be nullptr (no label support).
     */
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

    /**
     * @brief Sets the adaption mode for this group; NEVER marks it inactive (never mutated). Structural --
     * the adaptor mode itself is set on the OA config.
     * @param mode The adaption mode; any value other than adaptionMode::NEVER marks the group(s) active.
     * @return *this, for fluent chaining.
     */
    ParamHandle &adaptionMode(Gem::Geneva::adaptionMode mode) {
        this->forEachGroup([&](GroupStructure<T> &g) {
            g.active = (mode != adaptionMode::NEVER);
            for(std::uint32_t k = 0; k < g.len; ++k) {
                ch_->active.at(g.start + k) = g.active ? 1 : 0;
            }
        });
        return *this;
    }

    /**
     * @brief Sets a fixed start value for every parameter of this handle's group(s).
     * @param v The start value to write into every member of every spanned group.
     * @return *this, for fluent chaining.
     */
    ParamHandle &init(T v) {
        this->forEachGroup([&](GroupStructure<T> &g) {
            for(std::uint32_t k = 0; k < g.len; ++k) {
                values_->at(g.start + k) = v;
            }
        });
        return *this;
    }

    /**
     * @brief Sets the random-initialization perimeter for this handle's group(s).
     * @param lo The lower random-init boundary applied to every member of every spanned group.
     * @param hi The upper random-init boundary applied to every member of every spanned group.
     * @return *this, for fluent chaining.
     */
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
     * @param name The label string to intern and stamp onto every spanned group (a no-op if owner is null).
     * @return *this, for fluent chaining.
     */
    ParamHandle &label(const std::string &name) {
        if(owner_ != nullptr) {
            const std::int32_t id = owner_->internLabel(name);
            this->forEachGroup([&](GroupStructure<T> &g) { g.label_id = id; });
        }
        return *this;
    }

private:
    /**
     * @brief Applies fn to every group this handle spans (1 for a single/group add, n for an array).
     * @tparam F A callable taking a GroupStructure<T>& reference.
     * @param fn The function invoked once per spanned group, with that group's GroupStructure<T>.
     */
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
 * arrays + shared layout), with fine-grained per-parameter control.
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

    /**
     * @brief Adds one constrained double parameter as its own adaption group (size 1).
     * @param init The start value of the parameter.
     * @param min The lower constraint boundary (also the lower random-init bound).
     * @param max The upper constraint boundary (also the upper random-init bound).
     * @return A handle to the new group, for fluent tuning.
     */
    ParamHandle<double> addDouble(double init, double min, double max) {
        return addOne(layout_.d, dv_, init, min, max, ParamKind::Constrained);
    }
    /**
     * @brief Adds one unbounded (Plain) double parameter as its own adaption group (size 1).
     * @param init The start value of the parameter (random-init perimeter defaults to [0, 1)).
     * @return A handle to the new group, for fluent tuning.
     */
    ParamHandle<double> addDouble(double init) {
        return addOne(layout_.d, dv_, init, 0., 1., ParamKind::Plain);
    }
    /**
     * @brief Adds n constrained double parameters sharing ONE adaption group (a collection).
     * @param n The number of parameters in the group.
     * @param min The lower constraint boundary, shared by every member (also the lower init bound).
     * @param max The upper constraint boundary, shared by every member (also the upper init bound).
     * @return A handle to the new group, for fluent tuning.
     */
    ParamHandle<double> addDoubleGroup(std::size_t n, double min, double max) {
        return addGrouped(layout_.d, dv_, n, min, max, ParamKind::Constrained);
    }
    /**
     * @brief Adds n constrained double parameters, each its own adaption group of size 1.
     * @param n The number of parameters (and the number of size-1 groups created).
     * @param min The lower constraint boundary applied to every parameter (also the lower init bound).
     * @param max The upper constraint boundary applied to every parameter (also the upper init bound).
     * @return A handle spanning all n new groups, for fluent tuning.
     */
    ParamHandle<double> addDoubleArray(std::size_t n, double min, double max) {
        return addArray(layout_.d, dv_, n, min, max, ParamKind::Constrained);
    }
    // Unbounded (plain) collections -- like GDoubleCollection / GDoubleObjectCollection: min/max are
    // not constraints (no fold), they only set the random-init perimeter + the Gauss step range.
    /**
     * @brief Adds n unbounded (Plain) double parameters sharing ONE adaption group; min/max set only the
     * random-init perimeter and Gauss step range, NOT a constraint (no fold).
     * @param n The number of parameters in the group.
     * @param initMin The lower random-init / step-range bound (not a constraint).
     * @param initMax The upper random-init / step-range bound (not a constraint).
     * @return A handle to the new group, for fluent tuning.
     */
    ParamHandle<double> addDoublePlainGroup(std::size_t n, double initMin, double initMax) {
        return addGrouped(layout_.d, dv_, n, initMin, initMax, ParamKind::Plain, /*plain_init_from_bounds=*/true);
    }
    /**
     * @brief Adds n unbounded (Plain) double parameters, each its own adaption group of size 1; initMin/
     * initMax set only the random-init perimeter and Gauss step range, NOT a constraint (no fold).
     * @param n The number of parameters (and the number of size-1 groups created).
     * @param initMin The lower random-init / step-range bound (not a constraint).
     * @param initMax The upper random-init / step-range bound (not a constraint).
     * @return A handle spanning all n new groups, for fluent tuning.
     */
    ParamHandle<double> addDoublePlainArray(std::size_t n, double initMin, double initMax) {
        return addArray(layout_.d, dv_, n, initMin, initMax, ParamKind::Plain, /*plain_init_from_bounds=*/true);
    }

    /***************************************************************************/
    // Float channel

    /**
     * @brief Adds one constrained float parameter as its own adaption group (size 1).
     * @param init The start value of the parameter.
     * @param min The lower constraint boundary (also the lower random-init bound).
     * @param max The upper constraint boundary (also the upper random-init bound).
     * @return A handle to the new group, for fluent tuning.
     */
    ParamHandle<float> addFloat(float init, float min, float max) {
        return addOne(layout_.f, fv_, init, min, max, ParamKind::Constrained);
    }
    /**
     * @brief Adds one unbounded (Plain) float parameter as its own adaption group (size 1).
     * @param init The start value of the parameter (random-init perimeter defaults to [0, 1)).
     * @return A handle to the new group, for fluent tuning.
     */
    ParamHandle<float> addFloat(float init) {
        return addOne(layout_.f, fv_, init, 0.f, 1.f, ParamKind::Plain);
    }
    /**
     * @brief Adds n constrained float parameters sharing ONE adaption group (a collection).
     * @param n The number of parameters in the group.
     * @param min The lower constraint boundary, shared by every member (also the lower init bound).
     * @param max The upper constraint boundary, shared by every member (also the upper init bound).
     * @return A handle to the new group, for fluent tuning.
     */
    ParamHandle<float> addFloatGroup(std::size_t n, float min, float max) {
        return addGrouped(layout_.f, fv_, n, min, max, ParamKind::Constrained);
    }
    /**
     * @brief Adds n constrained float parameters, each its own adaption group of size 1.
     * @param n The number of parameters (and the number of size-1 groups created).
     * @param min The lower constraint boundary applied to every parameter (also the lower init bound).
     * @param max The upper constraint boundary applied to every parameter (also the upper init bound).
     * @return A handle spanning all n new groups, for fluent tuning.
     */
    ParamHandle<float> addFloatArray(std::size_t n, float min, float max) {
        return addArray(layout_.f, fv_, n, min, max, ParamKind::Constrained);
    }

    /***************************************************************************/
    // Int32 channel (Gauss adaptor not yet available -- flip adaptor added later)

    /**
     * @brief Adds one constrained int32 parameter as its own adaption group (size 1).
     * @param init The start value of the parameter.
     * @param min The lower constraint boundary (closed range; also the lower random-init bound).
     * @param max The upper constraint boundary (closed range; also the upper random-init bound).
     * @return A handle to the new group, for fluent tuning.
     */
    ParamHandle<std::int32_t> addInt32(std::int32_t init, std::int32_t min, std::int32_t max) {
        return addOne(layout_.i, iv_, init, min, max, ParamKind::Constrained);
    }
    /**
     * @brief Adds one unbounded (Plain) int32 parameter as its own adaption group (size 1); the bounds
     * default to the full int32 range.
     * @param init The start value of the parameter.
     * @return A handle to the new group, for fluent tuning.
     */
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
    /**
     * @brief Adds n constrained int32 parameters sharing ONE adaption group (a collection).
     * @param n The number of parameters in the group.
     * @param min The lower constraint boundary, shared by every member (closed range).
     * @param max The upper constraint boundary, shared by every member (closed range).
     * @return A handle to the new group, for fluent tuning.
     */
    ParamHandle<std::int32_t> addInt32Group(std::size_t n, std::int32_t min, std::int32_t max) {
        return addGrouped(layout_.i, iv_, n, min, max, ParamKind::Constrained);
    }
    /**
     * @brief Adds n constrained int32 parameters, each its own adaption group of size 1.
     * @param n The number of parameters (and the number of size-1 groups created).
     * @param min The lower constraint boundary applied to every parameter (closed range).
     * @param max The upper constraint boundary applied to every parameter (closed range).
     * @return A handle spanning all n new groups, for fluent tuning.
     */
    ParamHandle<std::int32_t> addInt32Array(std::size_t n, std::int32_t min, std::int32_t max) {
        return addArray(layout_.i, iv_, n, min, max, ParamKind::Constrained);
    }

    /***************************************************************************/
    // Bool channel

    /**
     * @brief Adds one bool parameter as its own adaption group (size 1).
     * @param init The start value of the parameter.
     * @return A handle to the new group, for fluent tuning.
     */
    ParamHandle<bool> addBool(bool init) {
        return addOne(layout_.b, bvBool_, init, false, true, ParamKind::Plain);
    }
    /**
     * @brief Adds n bool parameters sharing ONE adaption group (a collection).
     * @param n The number of parameters in the group.
     * @return A handle to the new group, for fluent tuning.
     */
    ParamHandle<bool> addBoolGroup(std::size_t n) {
        return addGrouped(layout_.b, bvBool_, n, false, true, ParamKind::Plain);
    }
    /**
     * @brief Adds n bool parameters, each its own adaption group of size 1.
     * @param n The number of parameters (and the number of size-1 groups created).
     * @return A handle spanning all n new groups, for fluent tuning.
     */
    ParamHandle<bool> addBoolArray(std::size_t n) {
        return addArray(layout_.b, bvBool_, n, false, true, ParamKind::Plain);
    }

    /***************************************************************************/
    /**
     * @brief Produces the value arrays + an interned, shared adaption layout.
     * @return A GenomeData carrying copies of the four channels' start-value arrays plus a
     * std::shared_ptr<const GGenomeLayout> to the freshly interned structural descriptor.
     */
    GenomeData build() const {
        GenomeData g;
        g.dv = dv_;
        g.fv = fv_;
        g.iv = iv_;
        g.bv.reserve(bvBool_.size());
        for(bool v : bvBool_) {
            g.bv.push_back(v ? static_cast<std::uint8_t>(1) : static_cast<std::uint8_t>(0));
        }
        g.layout = std::make_shared<const GGenomeLayout>(layout_);
        return g;
    }

    /**
     * @brief Produces the shared layout only (for factories that bind value arrays separately).
     * @return A std::shared_ptr<const GGenomeLayout> to a freshly interned copy of the built structure.
     */
    std::shared_ptr<const GGenomeLayout> buildLayout() const {
        return std::make_shared<const GGenomeLayout>(layout_);
    }

private:
    /***************************************************************************/
    /**
     * @brief One parameter = one group of size 1; delegates to addGroupImpl with len == 1.
     * @tparam T The channel's value type (double / float / int32 / bool).
     * @param ch The channel to append to.
     * @param values The channel's start-value array to append to.
     * @param init The start value of the parameter.
     * @param min The lower bound (constraint or, for Plain, init perimeter).
     * @param max The upper bound (constraint or, for Plain, init perimeter).
     * @param kind Whether the parameter is Plain (unbounded) or Constrained.
     * @param plain_init_from_bounds For a Plain collection, true ⇒ derive the init perimeter / step range from [min, max].
     * @return A handle to the single new group.
     */
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

    /**
     * @brief n parameters sharing one group; the start value of every member is set to min.
     * @tparam T The channel's value type.
     * @param ch The channel to append to.
     * @param values The channel's start-value array to append to.
     * @param n The number of parameters in the group.
     * @param min The lower bound; also seeds the start value of every member.
     * @param max The upper bound.
     * @param kind Whether the parameters are Plain (unbounded) or Constrained.
     * @param plain_init_from_bounds For a Plain collection, true ⇒ derive the init perimeter / step range from [min, max].
     * @return A handle to the single new group.
     */
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

    /**
     * @brief n parameters, each its own group of size 1; returns a handle spanning all n new groups.
     * @tparam T The channel's value type.
     * @param ch The channel to append to.
     * @param values The channel's start-value array to append to.
     * @param n The number of size-1 groups to create.
     * @param min The lower bound; also seeds the start value of every parameter.
     * @param max The upper bound.
     * @param kind Whether the parameters are Plain (unbounded) or Constrained.
     * @param plain_init_from_bounds For a Plain collection, true ⇒ derive the init perimeter / step range from [min, max].
     * @return A handle spanning all n freshly-created groups.
     */
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
     * @brief Appends one group of `len` values to a channel, extending all per-value vectors and registering
     * the group. `init` seeds the start value of every member; the init perimeter defaults to the
     * bounds (Constrained) or [0,1] (Plain), and can be overridden via the returned handle. For an
     * unbounded (Plain) collection the caller passes plain_init_from_bounds = true so the random-init
     * perimeter and the comparative Gauss step range follow [min, max] (like GDoubleCollection) rather
     * than the conservative [0, 1] default used for a bare unbounded scalar.
     * @tparam T The channel's value type.
     * @param ch The channel to append the group to.
     * @param values The channel's start-value array to append to.
     * @param len The number of values in the new group.
     * @param init The start value seeded into every member of the group.
     * @param min The lower bound stored per value (constraint or, for Plain, unused as a constraint).
     * @param max The upper bound stored per value.
     * @param kind Whether the group is Plain (unbounded) or Constrained.
     * @param plain_init_from_bounds For a Plain group, true ⇒ derive the init perimeter and comparative range from [min, max].
     * @return A handle to the single new group.
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
        // The parameter's natural scale (upper-lower constrained, init span plain) is NOT stored: it is
        // computed on demand from the bounds via ngScale wherever needed (the normalized model's single
        // `scale` concept, used by the FP transform and the integer Gauss adaptor).

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
