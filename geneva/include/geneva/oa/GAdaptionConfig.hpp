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
#include <string>
#include <type_traits>
#include <vector>

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GAdaptionAuxKeys.hpp"
#include "geneva/ind/GAdaptionKernels.hpp"
#include "geneva/ind/GAuxiliaryStore.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GGenomeLayout.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
// Names imported from the genome / kernel layer (the Parameters namespace).
using Gem::Geneva::Parameters::adaption_fp_t;
using Gem::Geneva::Parameters::BiGaussConfig;
using Gem::Geneva::Parameters::ChannelTag;
using Gem::Geneva::Parameters::FlipConfig;
using Gem::Geneva::Parameters::GaussConfig;
using Gem::Geneva::Parameters::GAuxiliaryStore;
using Gem::Geneva::Parameters::GFlatGenome;
using Gem::Geneva::Parameters::GGenomeLayout;
using Gem::Geneva::Parameters::GroupRef;
using Gem::Geneva::Parameters::GroupSpec;
using Gem::Geneva::Parameters::GroupStructure;

/******************************************************************************/
/**
 * A fluent handle to one or more adaption groups of a single channel held by a GAdaptionConfig. It
 * applies an adaptor setting (Gauss / bi-Gauss / integer-Gauss / flip / mode) to every group it spans
 * -- one group for a per-index handle, many for a label handle. The setters mirror the GGenomeBuilder's
 * ParamHandle exactly, so an adaption config authored here is bit-for-bit the same configuration the
 * builder would have baked into the (soon structure-only) layout. The handle is a thin view onto the
 * config's group vector; it must not outlive the config.
 */
template <typename T>
class GroupConfigHandle {
public:
    using adfp = adaption_fp_t<T>;

    explicit GroupConfigHandle(std::vector<GroupSpec<T> *> groups)
      : groups_(std::move(groups)) {
        /* nothing */
    }

    /** @brief Attaches a Gauss adaptor to the spanned group(s) (FP channels). */
    GroupConfigHandle &gauss(
        adfp sigma,
        adfp sigma_sigma,
        adfp min_sigma,
        adfp max_sigma,
        adfp ad_prob,
        adfp adapt_ad_prob = adfp(0),
        std::uint32_t adaption_threshold = 1,
        adaptionMode mode = adaptionMode::WITHPROBABILITY,
        adfp min_ad_prob = adfp(0),
        adfp max_ad_prob = adfp(1)
    ) {
        static_assert(std::is_floating_point_v<T>, "gauss() is only available for floating point groups");
        for(GroupSpec<T> *g : groups_) {
            g->has_gauss = true;
            g->start_sigma = sigma;
            g->start_ad_prob = ad_prob;
            g->gauss.sigma_sigma = sigma_sigma;
            g->gauss.min_sigma = min_sigma;
            g->gauss.max_sigma = max_sigma;
            g->gauss.min_ad_prob = min_ad_prob;
            g->gauss.max_ad_prob = max_ad_prob;
            g->gauss.adapt_ad_prob = adapt_ad_prob;
            g->gauss.adaption_threshold = adaption_threshold;
            g->gauss.mode = mode;
        }
        return *this;
    }

    /** @brief Attaches a bi-gaussian adaptor to the spanned group(s) (FP channels). */
    GroupConfigHandle &biGauss(
        adfp sigma1,
        adfp sigma_sigma1,
        adfp min_sigma1,
        adfp max_sigma1,
        adfp sigma2,
        adfp sigma_sigma2,
        adfp min_sigma2,
        adfp max_sigma2,
        adfp delta,
        adfp sigma_delta,
        adfp min_delta,
        adfp max_delta,
        adfp ad_prob,
        bool use_symmetric_sigmas = false,
        adfp adapt_ad_prob = adfp(0),
        std::uint32_t adaption_threshold = 1,
        adaptionMode mode = adaptionMode::WITHPROBABILITY
    ) {
        static_assert(std::is_floating_point_v<T>, "biGauss() is only available for floating point groups");
        for(GroupSpec<T> *g : groups_) {
            g->has_bigauss = true;
            g->start_sigma1 = sigma1;
            g->start_sigma2 = sigma2;
            g->start_delta = delta;
            g->start_ad_prob = ad_prob;
            g->bigauss.sigma_sigma1 = sigma_sigma1;
            g->bigauss.sigma_sigma2 = sigma_sigma2;
            g->bigauss.sigma_delta = sigma_delta;
            g->bigauss.min_sigma1 = min_sigma1;
            g->bigauss.max_sigma1 = max_sigma1;
            g->bigauss.min_sigma2 = min_sigma2;
            g->bigauss.max_sigma2 = max_sigma2;
            g->bigauss.min_delta = min_delta;
            g->bigauss.max_delta = max_delta;
            g->bigauss.min_ad_prob = adfp(0);
            g->bigauss.max_ad_prob = adfp(1);
            g->bigauss.adapt_ad_prob = adapt_ad_prob;
            g->bigauss.adaption_threshold = adaption_threshold;
            g->bigauss.use_symmetric_sigmas = use_symmetric_sigmas;
            g->bigauss.mode = mode;
        }
        return *this;
    }

    /** @brief Attaches an integer Gauss adaptor to the spanned group(s) (int32 channel). */
    GroupConfigHandle &intGauss(
        double sigma,
        double sigma_sigma,
        double min_sigma,
        double max_sigma,
        double ad_prob,
        double adapt_ad_prob = 0.,
        std::uint32_t adaption_threshold = 1,
        adaptionMode mode = adaptionMode::WITHPROBABILITY,
        double min_ad_prob = 0.,
        double max_ad_prob = 1.
    ) {
        static_assert(std::is_same_v<T, std::int32_t>, "intGauss() is only available for int32 groups");
        for(GroupSpec<T> *g : groups_) {
            g->has_gauss = true;
            g->start_sigma = sigma;
            g->start_ad_prob = ad_prob;
            g->gauss.sigma_sigma = sigma_sigma;
            g->gauss.min_sigma = min_sigma;
            g->gauss.max_sigma = max_sigma;
            g->gauss.min_ad_prob = min_ad_prob;
            g->gauss.max_ad_prob = max_ad_prob;
            g->gauss.adapt_ad_prob = adapt_ad_prob;
            g->gauss.adaption_threshold = adaption_threshold;
            g->gauss.mode = mode;
        }
        return *this;
    }

    /** @brief Attaches a flip adaptor to the spanned group(s) (int32 / bool channels). */
    GroupConfigHandle &flip(
        double ad_prob,
        double adapt_ad_prob = 0.,
        double min_ad_prob = 0.,
        double max_ad_prob = 1.,
        adaptionMode mode = adaptionMode::WITHPROBABILITY
    ) {
        static_assert(std::is_integral_v<T>, "flip() is only available for integer / boolean groups");
        for(GroupSpec<T> *g : groups_) {
            g->has_flip = true;
            g->start_ad_prob = ad_prob;
            g->flip.min_ad_prob = min_ad_prob;
            g->flip.max_ad_prob = max_ad_prob;
            g->flip.adapt_ad_prob = adapt_ad_prob;
            g->flip.mode = mode;
        }
        return *this;
    }

    /** @brief Sets the adaption mode for the spanned group(s) (NEVER ⇒ the group is not adapted). */
    GroupConfigHandle &adaptionMode(Gem::Geneva::adaptionMode mode) {
        for(GroupSpec<T> *g : groups_) {
            g->gauss.mode = mode;
            g->bigauss.mode = mode;
            g->flip.mode = mode;
            g->active = (mode != adaptionMode::NEVER);
        }
        return *this;
    }

    /** @brief The number of groups this handle spans. */
    std::size_t size() const { return groups_.size(); }

private:
    std::vector<GroupSpec<T> *> groups_;
};

/******************************************************************************/
/**
 * A fluent handle aggregating, by channel, every group that carries one interned label. It forwards an
 * adaptor setting to the groups of the channels for which that setting is valid (gauss / biGauss → the
 * FP groups; intGauss → the int32 groups; flip → the int32 + bool groups; adaptionMode → all). A label
 * usually tags a single channel (e.g. "position" → the FP cx/cy groups), but the aggregation is general.
 */
class GLabelConfigHandle {
public:
    GLabelConfigHandle(
        GroupConfigHandle<double> d,
        GroupConfigHandle<float> f,
        GroupConfigHandle<std::int32_t> i,
        GroupConfigHandle<bool> b
    )
      : d_(std::move(d))
      , f_(std::move(f))
      , i_(std::move(i))
      , b_(std::move(b)) {
        /* nothing */
    }

    /** @brief Applies a Gauss adaptor to the labelled FP groups (double in this width, float narrowed). */
    GLabelConfigHandle &gauss(
        double sigma,
        double sigma_sigma,
        double min_sigma,
        double max_sigma,
        double ad_prob,
        double adapt_ad_prob = 0.,
        std::uint32_t adaption_threshold = 1,
        adaptionMode mode = adaptionMode::WITHPROBABILITY,
        double min_ad_prob = 0.,
        double max_ad_prob = 1.
    ) {
        if(d_.size() > 0) {
            d_.gauss(sigma, sigma_sigma, min_sigma, max_sigma, ad_prob, adapt_ad_prob, adaption_threshold, mode, min_ad_prob, max_ad_prob);
        }
        if(f_.size() > 0) {
            f_.gauss(
                static_cast<float>(sigma), static_cast<float>(sigma_sigma), static_cast<float>(min_sigma),
                static_cast<float>(max_sigma), static_cast<float>(ad_prob), static_cast<float>(adapt_ad_prob),
                adaption_threshold, mode, static_cast<float>(min_ad_prob), static_cast<float>(max_ad_prob)
            );
        }
        return *this;
    }

    /** @brief Applies an integer Gauss adaptor to the labelled int32 groups. */
    GLabelConfigHandle &intGauss(
        double sigma,
        double sigma_sigma,
        double min_sigma,
        double max_sigma,
        double ad_prob,
        double adapt_ad_prob = 0.,
        std::uint32_t adaption_threshold = 1,
        adaptionMode mode = adaptionMode::WITHPROBABILITY,
        double min_ad_prob = 0.,
        double max_ad_prob = 1.
    ) {
        if(i_.size() > 0) {
            i_.intGauss(sigma, sigma_sigma, min_sigma, max_sigma, ad_prob, adapt_ad_prob, adaption_threshold, mode, min_ad_prob, max_ad_prob);
        }
        return *this;
    }

    /** @brief Applies a flip adaptor to the labelled int32 + bool groups. */
    GLabelConfigHandle &flip(
        double ad_prob,
        double adapt_ad_prob = 0.,
        double min_ad_prob = 0.,
        double max_ad_prob = 1.,
        adaptionMode mode = adaptionMode::WITHPROBABILITY
    ) {
        if(i_.size() > 0) {
            i_.flip(ad_prob, adapt_ad_prob, min_ad_prob, max_ad_prob, mode);
        }
        if(b_.size() > 0) {
            b_.flip(ad_prob, adapt_ad_prob, min_ad_prob, max_ad_prob, mode);
        }
        return *this;
    }

    /** @brief Sets the adaption mode for every labelled group, across all channels. */
    GLabelConfigHandle &adaptionMode(Gem::Geneva::adaptionMode mode) {
        if(d_.size() > 0) { d_.adaptionMode(mode); }
        if(f_.size() > 0) { f_.adaptionMode(mode); }
        if(i_.size() > 0) { i_.adaptionMode(mode); }
        if(b_.size() > 0) { b_.adaptionMode(mode); }
        return *this;
    }

private:
    GroupConfigHandle<double> d_;
    GroupConfigHandle<float> f_;
    GroupConfigHandle<std::int32_t> i_;
    GroupConfigHandle<bool> b_;
};

/******************************************************************************/
/**
 * The base of the OA-owned adaption configuration. It carries the per-group adaptor settings (Gauss /
 * bi-Gauss / integer-Gauss / flip + their seeds) addressed by channel + index or by interned label, and
 * is built FROM a genome so it can only describe groups that actually exist. Concretely it holds one
 * GroupSpec vector per channel -- the *config half* of the (currently still layout-resident) group
 * specs -- plus the interned label table. Phase 8 step 3 will strip the adaptor fields out of the
 * GGenomeLayout, at which point this object becomes their sole home; for now it is a separate,
 * OA-owned copy that nothing is yet wired to.
 *
 * The base owns the OA-agnostic machinery: existence-validated authoring, a structural signature +
 * checkConsistency(genome) cross-check (so a genome and a config can be verified as belonging
 * together), label resolution, and the install-state-into-aux hook. Derived classes (GEAAdaptionConfig
 * / GSAAdaptionConfig) are the per-OA types; they may diverge later.
 */
class GAdaptionConfigBase {
public:
    /** @brief Builds a config describing exactly the groups of the passed genome. */
    explicit GAdaptionConfigBase(const GFlatGenome &genome) { initFrom(*genome.getLayout()); }
    /** @brief Builds a config describing exactly the groups of the passed layout. */
    explicit GAdaptionConfigBase(const GGenomeLayout &layout) { initFrom(layout); }

    GAdaptionConfigBase(const GAdaptionConfigBase &) = default;
    GAdaptionConfigBase(GAdaptionConfigBase &&) = default;
    GAdaptionConfigBase &operator=(const GAdaptionConfigBase &) = default;
    GAdaptionConfigBase &operator=(GAdaptionConfigBase &&) = default;
    virtual ~GAdaptionConfigBase() = default;

    /***************************************************************************/
    // Authoring -- per channel + index (throws on an out-of-range index).

    GroupConfigHandle<double> groupDouble(std::size_t index) { return oneHandle(d_, index, "double"); }
    GroupConfigHandle<float> groupFloat(std::size_t index) { return oneHandle(f_, index, "float"); }
    GroupConfigHandle<std::int32_t> groupInt32(std::size_t index) { return oneHandle(i_, index, "int32"); }
    GroupConfigHandle<bool> groupBool(std::size_t index) { return oneHandle(b_, index, "bool"); }

    /** @brief Authoring by interned label -- spans every group that carries it (throws if none does). */
    GLabelConfigHandle forLabel(const std::string &name) {
        const std::int32_t id = labelId(name);
        if(id < 0) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GAdaptionConfigBase::forLabel(): Error!" << '\n'
                << "Label \"" << name << "\" is not present in the genome." << '\n'
            );
        }
        GLabelConfigHandle h(
            labelHandle(d_, id), labelHandle(f_, id), labelHandle(i_, id), labelHandle(b_, id)
        );
        if(d_labelEmpty(id) && f_labelEmpty(id) && i_labelEmpty(id) && b_labelEmpty(id)) {
            // Defensive: a registered label string with no group carrying its id (should not happen).
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GAdaptionConfigBase::forLabel(): Error!" << '\n'
                << "Label \"" << name << "\" resolves to no group." << '\n'
            );
        }
        return h;
    }

    /***************************************************************************/
    // Validation.

    /** @brief Throws unless the passed genome has exactly the structure this config was authored against. */
    void checkConsistency(const GFlatGenome &genome) const { checkConsistency(*genome.getLayout()); }

    /** @brief Throws unless the passed layout has exactly the structure this config was authored against. */
    void checkConsistency(const GGenomeLayout &layout) const {
        checkChannel(d_, layout.d.groups, "double");
        checkChannel(f_, layout.f.groups, "float");
        checkChannel(i_, layout.i.groups, "int32");
        checkChannel(b_, layout.b.groups, "bool");
    }

    /***************************************************************************/
    // Label resolution (read access).

    /** @brief Resolves a label string to its id, or -1 if it is not present. */
    std::int32_t labelId(const std::string &name) const {
        for(std::size_t k = 0; k < labels_.size(); ++k) {
            if(labels_[k] == name) {
                return static_cast<std::int32_t>(k);
            }
        }
        return -1;
    }

    /** @brief Resolves a label string to every group it tags, across all channels (one-to-many). */
    std::vector<GroupRef> groupsForLabel(const std::string &name) const {
        std::vector<GroupRef> out;
        const std::int32_t id = labelId(name);
        if(id < 0) {
            return out;
        }
        collect(d_, ChannelTag::Double, id, out);
        collect(f_, ChannelTag::Float, id, out);
        collect(i_, ChannelTag::Int32, id, out);
        collect(b_, ChannelTag::Bool, id, out);
        return out;
    }

    /***************************************************************************/
    // State install.

    /**
     * @brief Seeds the per-group adaption state blocks into an OA-owned auxiliary store (the slot's
     * scratch) from this config (one block per adaptor kind + channel that is actually used). The
     * per-group adaption state is OA scratch and lives on the GIndividualSlot, NOT on the individual,
     * so the seeding targets a GAuxiliaryStore directly. The seeds also serve as the reset targets used
     * by the stall-reset free function.
     */
    void installInto(GAuxiliaryStore &scratch) const {
        using namespace Gem::Geneva::Parameters;
        seedGauss(scratch, d_, AUXKEY_GAUSS_DOUBLE);
        seedGauss(scratch, f_, AUXKEY_GAUSS_FLOAT);
        seedGauss(scratch, i_, AUXKEY_GAUSS_INT); // GaussState<adaption_fp_t<int32>> = GaussState<double>
        seedBiGauss(scratch, d_, AUXKEY_BIGAUSS_DOUBLE);
        seedBiGauss(scratch, f_, AUXKEY_BIGAUSS_FLOAT);
        seedFlip(scratch, i_, AUXKEY_FLIP_INT);
        seedFlip(scratch, b_, AUXKEY_FLIP_BOOL);
    }

    /***************************************************************************/
    // Read access for the adaption free functions.

    const std::vector<GroupSpec<double>> &doubleGroups() const { return d_; }
    const std::vector<GroupSpec<float>> &floatGroups() const { return f_; }
    const std::vector<GroupSpec<std::int32_t>> &int32Groups() const { return i_; }
    const std::vector<GroupSpec<bool>> &boolGroups() const { return b_; }
    const std::vector<std::string> &labels() const { return labels_; }

protected:
    /***************************************************************************/
    // Snapshots the group STRUCTURE from a (structure-only) genome layout into this config's group specs;
    // the adaptor fields stay default-off until the caller authors them via the fluent API.
    void initFrom(const GGenomeLayout &layout) {
        copyStructure(layout.d.groups, d_);
        copyStructure(layout.f.groups, f_);
        copyStructure(layout.i.groups, i_);
        copyStructure(layout.b.groups, b_);
        labels_ = layout.labels;
    }

private:
    /***************************************************************************/
    template <typename T>
    static void
    copyStructure(const std::vector<GroupStructure<T>> &src, std::vector<GroupSpec<T>> &dst) {
        dst.clear();
        dst.reserve(src.size());
        for(const GroupStructure<T> &s : src) {
            GroupSpec<T> g;
            g.start = s.start;
            g.len = s.len;
            g.label_id = s.label_id;
            g.active = s.active;
            g.range = s.range;
            dst.push_back(g);
        }
    }

    /***************************************************************************/
    template <typename T>
    static GroupConfigHandle<T>
    oneHandle(std::vector<GroupSpec<T>> &groups, std::size_t index, const char *channel) {
        if(index >= groups.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GAdaptionConfigBase::group" << channel << "(): Error!" << '\n'
                << "Group index " << index << " is out of range (" << groups.size() << " groups)." << '\n'
            );
        }
        return GroupConfigHandle<T>(std::vector<GroupSpec<T> *>{&groups[index]});
    }

    template <typename T>
    static GroupConfigHandle<T> labelHandle(std::vector<GroupSpec<T>> &groups, std::int32_t id) {
        std::vector<GroupSpec<T> *> targets;
        for(GroupSpec<T> &g : groups) {
            if(g.label_id == id) {
                targets.push_back(&g);
            }
        }
        return GroupConfigHandle<T>(std::move(targets));
    }

    template <typename T>
    static void
    collect(const std::vector<GroupSpec<T>> &groups, ChannelTag tag, std::int32_t id, std::vector<GroupRef> &out) {
        for(std::size_t gi = 0; gi < groups.size(); ++gi) {
            if(groups[gi].label_id == id) {
                out.push_back(GroupRef{tag, gi});
            }
        }
    }

    template <typename T>
    static void
    checkChannel(const std::vector<GroupSpec<T>> &cfg, const std::vector<GroupStructure<T>> &layout, const char *channel) {
        if(cfg.size() != layout.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GAdaptionConfigBase::checkConsistency(): Error!" << '\n'
                << "Group count mismatch on the " << channel << " channel: config has " << cfg.size()
                << ", genome has " << layout.size() << '\n'
            );
        }
        for(std::size_t gi = 0; gi < cfg.size(); ++gi) {
            if(cfg[gi].start != layout[gi].start || cfg[gi].len != layout[gi].len ||
               cfg[gi].label_id != layout[gi].label_id) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GAdaptionConfigBase::checkConsistency(): Error!" << '\n'
                    << "Structural mismatch on the " << channel << " channel, group " << gi << '\n'
                );
            }
        }
    }

    bool d_labelEmpty(std::int32_t id) const { return noneWithLabel(d_, id); }
    bool f_labelEmpty(std::int32_t id) const { return noneWithLabel(f_, id); }
    bool i_labelEmpty(std::int32_t id) const { return noneWithLabel(i_, id); }
    bool b_labelEmpty(std::int32_t id) const { return noneWithLabel(b_, id); }

    template <typename T>
    static bool noneWithLabel(const std::vector<GroupSpec<T>> &groups, std::int32_t id) {
        for(const GroupSpec<T> &g : groups) {
            if(g.label_id == id) {
                return false;
            }
        }
        return true;
    }

    /***************************************************************************/
    // State seeding helpers (mirror GFlatGenome::installAdaptionStates).

    template <typename T>
    static void seedGauss(GAuxiliaryStore &scratch, const std::vector<GroupSpec<T>> &groups, Gem::Geneva::Parameters::AuxKey key) {
        using Gem::Geneva::Parameters::GaussState;
        using Gem::Geneva::Parameters::AuxScope;
        bool any = false;
        for(const GroupSpec<T> &g : groups) {
            if(g.has_gauss) { any = true; break; }
        }
        if(not any) {
            return;
        }
        scratch.installAuxBlock<GaussState<adaption_fp_t<T>>>(key, groups.size(), AuxScope::PerIndividual);
        std::span<GaussState<adaption_fp_t<T>>> states = scratch.metaRecords<GaussState<adaption_fp_t<T>>>(key);
        for(std::size_t gi = 0; gi < groups.size(); ++gi) {
            states[gi].sigma = groups[gi].start_sigma;
            states[gi].ad_prob = groups[gi].start_ad_prob;
            states[gi].counter = 0;
        }
    }

    template <typename T>
    static void seedBiGauss(GAuxiliaryStore &scratch, const std::vector<GroupSpec<T>> &groups, Gem::Geneva::Parameters::AuxKey key) {
        using Gem::Geneva::Parameters::BiGaussState;
        using Gem::Geneva::Parameters::AuxScope;
        bool any = false;
        for(const GroupSpec<T> &g : groups) {
            if(g.has_bigauss) { any = true; break; }
        }
        if(not any) {
            return;
        }
        scratch.installAuxBlock<BiGaussState<adaption_fp_t<T>>>(key, groups.size(), AuxScope::PerIndividual);
        std::span<BiGaussState<adaption_fp_t<T>>> states = scratch.metaRecords<BiGaussState<adaption_fp_t<T>>>(key);
        for(std::size_t gi = 0; gi < groups.size(); ++gi) {
            states[gi].sigma1 = groups[gi].start_sigma1;
            states[gi].sigma2 = groups[gi].start_sigma2;
            states[gi].delta = groups[gi].start_delta;
            states[gi].ad_prob = groups[gi].start_ad_prob;
            states[gi].counter = 0;
        }
    }

    template <typename T>
    static void seedFlip(GAuxiliaryStore &scratch, const std::vector<GroupSpec<T>> &groups, Gem::Geneva::Parameters::AuxKey key) {
        using Gem::Geneva::Parameters::FlipState;
        using Gem::Geneva::Parameters::AuxScope;
        bool any = false;
        for(const GroupSpec<T> &g : groups) {
            if(g.has_flip) { any = true; break; }
        }
        if(not any) {
            return;
        }
        scratch.installAuxBlock<FlipState>(key, groups.size(), AuxScope::PerIndividual);
        std::span<FlipState> states = scratch.metaRecords<FlipState>(key);
        for(std::size_t gi = 0; gi < groups.size(); ++gi) {
            states[gi].ad_prob = groups[gi].start_ad_prob;
        }
    }

    /***************************************************************************/
    // The config half of the genome's groups, per channel, + the interned label table.
    std::vector<GroupSpec<double>> d_;
    std::vector<GroupSpec<float>> f_;
    std::vector<GroupSpec<std::int32_t>> i_;
    std::vector<GroupSpec<bool>> b_;
    std::vector<std::string> labels_;
};

/******************************************************************************/
/** @brief The evolutionary-algorithm adaption configuration. */
class GEAAdaptionConfig : public GAdaptionConfigBase {
public:
    using GAdaptionConfigBase::GAdaptionConfigBase;
};

/******************************************************************************/
/** @brief The simulated-annealing adaption configuration. */
class GSAAdaptionConfig : public GAdaptionConfigBase {
public:
    using GAdaptionConfigBase::GAdaptionConfigBase;
};

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
