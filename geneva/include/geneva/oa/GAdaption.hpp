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
#include <span>
#include <string>
#include <vector>

// Geneva headers go here
#include "geneva/ind/GAdaptionAuxKeys.hpp"
#include "geneva/ind/GAdaptionKernels.hpp"
#include "geneva/ind/GAuxiliaryStore.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"
#include "hap/GRandomBase.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * The OA-side adaption logic, expressed as stateless free functions over (individual, scratch, config,
 * RNG). It is the data-oriented twin of the individual's former GFlatGenome::customAdaptions() /
 * updateAdaptorsOnStall() / queryAdaptor(), but driven by an OA-OWNED GAdaptionConfig rather than by the
 * (soon structure-only) genome layout. The config supplies each group's adaptor kind + parameters; the
 * genome supplies the mutable internal value spans; the per-group evolving state lives in an OA-owned
 * GAuxiliaryStore (the GIndividualSlot's scratch_) passed in explicitly — it is no longer carried by the
 * individual, which is now pure data. The RNG is the individual's own per-individual stream. Each call
 * touches only this individual's values + its slot's scratch + its RNG and reads the shared config
 * read-only, so the functions compose with the EA's parallel adaptChildren_.
 */

namespace detail {

using Gem::Geneva::Parameters::AuxKey;
using Gem::Geneva::Parameters::BiGaussState;
using Gem::Geneva::Parameters::FlipState;
using Gem::Geneva::Parameters::GaussState;
using Gem::Geneva::Parameters::GAuxiliaryStore;
using Gem::Geneva::Parameters::GFlatGenome;
using Gem::Geneva::Parameters::GroupSpec;

/** @brief Runs the Gauss kernel over one FP channel using the GaussState block (in scratch) under key. */
template <typename T>
std::size_t adaptGaussChannel(
    GAuxiliaryStore &scratch,
    const std::vector<GroupSpec<T>> &groups,
    std::span<T> values,
    AuxKey key,
    Gem::Hap::GRandomBase &gr
) {
    if(not scratch.hasAux(key)) {
        return 0;
    }
    std::span<GaussState<T>> states = scratch.metaRecords<GaussState<T>>(key);
    std::size_t n = 0;
    for(std::size_t gi = 0; gi < groups.size(); ++gi) {
        const GroupSpec<T> &g = groups[gi];
        if(not g.has_gauss || not g.active) {
            continue;
        }
        n += Gem::Geneva::Parameters::adaptGaussGroup<T>(
            g.gauss, states[gi], values.subspan(g.start, g.len), g.range, gr
        );
    }
    return n;
}

/** @brief Runs the bi-gaussian kernel over one FP channel using the BiGaussState block (in scratch) under key. */
template <typename T>
std::size_t adaptBiGaussChannel(
    GAuxiliaryStore &scratch,
    const std::vector<GroupSpec<T>> &groups,
    std::span<T> values,
    AuxKey key,
    Gem::Hap::GRandomBase &gr
) {
    if(not scratch.hasAux(key)) {
        return 0;
    }
    std::span<BiGaussState<T>> states = scratch.metaRecords<BiGaussState<T>>(key);
    std::size_t n = 0;
    for(std::size_t gi = 0; gi < groups.size(); ++gi) {
        const GroupSpec<T> &g = groups[gi];
        if(not g.has_bigauss || not g.active) {
            continue;
        }
        n += Gem::Geneva::Parameters::adaptBiGaussGroup<T>(
            g.bigauss, states[gi], values.subspan(g.start, g.len), g.range, gr
        );
    }
    return n;
}

} // namespace detail

/******************************************************************************/
/**
 * @brief Runs the data-oriented adaption kernels over an individual once, driven by the config (the
 * "customAdaptions" half of adapt(), config-injected). Mirrors GFlatGenome::customAdaptions()'s channel
 * order exactly. Returns the number of values actually adapted.
 */
inline std::size_t runAdaptionKernels(
    detail::GFlatGenome &ind,
    detail::GAuxiliaryStore &scratch,
    const GAdaptionConfigBase &cfg,
    Gem::Hap::GRandomBase &gr
) {
    using namespace Gem::Geneva::Parameters;

    std::size_t n = 0;
    n += detail::adaptGaussChannel<double>(scratch, cfg.doubleGroups(), ind.internalDoubleValues(), AUXKEY_GAUSS_DOUBLE, gr);
    n += detail::adaptGaussChannel<float>(scratch, cfg.floatGroups(), ind.internalFloatValues(), AUXKEY_GAUSS_FLOAT, gr);
    n += detail::adaptBiGaussChannel<double>(scratch, cfg.doubleGroups(), ind.internalDoubleValues(), AUXKEY_BIGAUSS_DOUBLE, gr);
    n += detail::adaptBiGaussChannel<float>(scratch, cfg.floatGroups(), ind.internalFloatValues(), AUXKEY_BIGAUSS_FLOAT, gr);

    // Integer Gauss (state is GaussState<double>, range is int32).
    if(scratch.hasAux(AUXKEY_GAUSS_INT)) {
        std::span<GaussState<double>> states = scratch.metaRecords<GaussState<double>>(AUXKEY_GAUSS_INT);
        std::span<std::int32_t> values = ind.internalInt32Values();
        const auto &groups = cfg.int32Groups();
        for(std::size_t gi = 0; gi < groups.size(); ++gi) {
            const GroupSpec<std::int32_t> &g = groups[gi];
            if(not g.has_gauss || not g.active) {
                continue;
            }
            n += adaptGaussIntGroup(g.gauss, states[gi], values.subspan(g.start, g.len), g.range, gr);
        }
    }

    // Flip over the int32 channel.
    if(scratch.hasAux(AUXKEY_FLIP_INT)) {
        std::span<FlipState> states = scratch.metaRecords<FlipState>(AUXKEY_FLIP_INT);
        std::span<std::int32_t> values = ind.internalInt32Values();
        const auto &groups = cfg.int32Groups();
        for(std::size_t gi = 0; gi < groups.size(); ++gi) {
            const GroupSpec<std::int32_t> &g = groups[gi];
            if(not g.has_flip || not g.active) {
                continue;
            }
            n += adaptFlipIntGroup(g.flip, states[gi], values.subspan(g.start, g.len), gr);
        }
    }

    // Flip over the bool channel.
    if(scratch.hasAux(AUXKEY_FLIP_BOOL)) {
        std::span<FlipState> states = scratch.metaRecords<FlipState>(AUXKEY_FLIP_BOOL);
        std::span<std::uint8_t> values = ind.internalBoolValues();
        const auto &groups = cfg.boolGroups();
        for(std::size_t gi = 0; gi < groups.size(); ++gi) {
            const GroupSpec<bool> &g = groups[gi];
            if(not g.has_flip || not g.active) {
                continue;
            }
            n += adaptFlipBoolGroup(g.flip, states[gi], values.subspan(g.start, g.len), gr);
        }
    }

    return n;
}

/******************************************************************************/
/**
 * @brief The OA-side replacement for GOptimizableEntity::adapt(): runs the data-oriented adaption
 * kernels over an individual inside the same "guarantee a change, then a valid solution" retry loop the
 * individual's adapt() used, but driven by the OA-owned config and the individual's own RNG stream.
 *
 * Mirrors GOptimizableEntity::adapt() exactly: the inner loop retries customAdaptions (here:
 * runAdaptionKernels) until at least one value changed or max_unsuccessful_adaptions is exceeded; the
 * outer loop retries until the individual fulfils its constraints or max_retries_until_valid is
 * exceeded. Marks the individual due for processing iff at least one adaption happened, records the
 * adaption count on the individual, and returns it. Touches only this individual's values + aux state +
 * RNG, so it composes with the EA's parallel adaptChildren_.
 */
inline std::size_t adaptIndividual(
    detail::GFlatGenome &ind,
    detail::GAuxiliaryStore &scratch,
    const GAdaptionConfigBase &cfg
) {
    Gem::Hap::GRandomBase &gr = ind.getRandomEngine();

    const std::size_t max_unsuccessful = ind.getMaxUnsuccessfulAdaptions();
    const std::size_t max_retries = ind.getMaxRetriesUntilValid();

    std::size_t n_adaption_attempts = 0;
    std::size_t n_adaptions = 0;
    std::size_t n_invalid_adaptions = 0;
    double validity = 0;

    while(true) {
        // Make sure at least one modification is performed.
        while(true) {
            if((n_adaptions = runAdaptionKernels(ind, scratch, cfg, gr)) > 0) {
                break;
            }
            if(max_unsuccessful > 0 && ++n_adaption_attempts > max_unsuccessful) {
                break;
            }
        }

        if(ind.fulfillsConstraints(validity) || ++n_invalid_adaptions > max_retries) {
            break;
        }
    }

    if(n_adaptions > 0) {
        ind.mark_as_due_for_processing();
    }
    ind.setNAdaptions(n_adaptions);
    return n_adaptions;
}

/******************************************************************************/
/**
 * @brief Resets an individual's per-group adaption state to the config's seed values (the stall-reset).
 * Mirrors GFlatGenome::updateAdaptorsOnStall(), but driven by the OA-owned config.
 */
inline void resetAdaptionState(detail::GAuxiliaryStore &scratch, const GAdaptionConfigBase &cfg) {
    using namespace Gem::Geneva::Parameters;

    auto resetGauss = [&]<typename T>(const std::vector<GroupSpec<T>> &groups, AuxKey key) {
        if(not scratch.hasAux(key)) {
            return;
        }
        std::span<GaussState<adaption_fp_t<T>>> states = scratch.metaRecords<GaussState<adaption_fp_t<T>>>(key);
        for(std::size_t gi = 0; gi < groups.size(); ++gi) {
            if(not groups[gi].has_gauss) {
                continue;
            }
            states[gi].sigma = groups[gi].start_sigma;
            states[gi].ad_prob = groups[gi].start_ad_prob;
            states[gi].counter = 0;
        }
    };
    auto resetBiGauss = [&]<typename T>(const std::vector<GroupSpec<T>> &groups, AuxKey key) {
        if(not scratch.hasAux(key)) {
            return;
        }
        std::span<BiGaussState<adaption_fp_t<T>>> states = scratch.metaRecords<BiGaussState<adaption_fp_t<T>>>(key);
        for(std::size_t gi = 0; gi < groups.size(); ++gi) {
            if(not groups[gi].has_bigauss) {
                continue;
            }
            states[gi].sigma1 = groups[gi].start_sigma1;
            states[gi].sigma2 = groups[gi].start_sigma2;
            states[gi].delta = groups[gi].start_delta;
            states[gi].ad_prob = groups[gi].start_ad_prob;
            states[gi].counter = 0;
        }
    };
    auto resetFlip = [&]<typename T>(const std::vector<GroupSpec<T>> &groups, AuxKey key) {
        if(not scratch.hasAux(key)) {
            return;
        }
        std::span<FlipState> states = scratch.metaRecords<FlipState>(key);
        for(std::size_t gi = 0; gi < groups.size(); ++gi) {
            if(not groups[gi].has_flip) {
                continue;
            }
            states[gi].ad_prob = groups[gi].start_ad_prob;
        }
    };

    resetGauss(cfg.doubleGroups(), AUXKEY_GAUSS_DOUBLE);
    resetGauss(cfg.floatGroups(), AUXKEY_GAUSS_FLOAT);
    resetGauss(cfg.int32Groups(), AUXKEY_GAUSS_INT); // GaussState<adaption_fp_t<int32>> = GaussState<double>
    resetBiGauss(cfg.doubleGroups(), AUXKEY_BIGAUSS_DOUBLE);
    resetBiGauss(cfg.floatGroups(), AUXKEY_BIGAUSS_FLOAT);
    resetFlip(cfg.int32Groups(), AUXKEY_FLIP_INT);
    resetFlip(cfg.boolGroups(), AUXKEY_FLIP_BOOL);
}

/******************************************************************************/
/**
 * @brief Reads the current per-group sigma of a named Gauss adaptor from an OA-owned adaption-state
 * store (the slot's scratch — one entry per Gauss group), replacing the former GFlatGenome::queryAdaptor()
 * for the pluggable monitors and the in-fitness sigma logging. Recognised names: "GDoubleGaussAdaptor",
 * "GFloatGaussAdaptor", "GInt32GaussAdaptor". Sigmas are returned as double (the float sigma widened).
 * For an individual that is detached from its slot (an archived best, a transport copy, or a standalone
 * individual), pass a scratch freshly seeded via seedAdaptionStates() to obtain the configured seed sigma.
 */
inline std::vector<double> readAdaptionSigmas(
    const detail::GAuxiliaryStore &scratch,
    const GAdaptionConfigBase &cfg,
    const std::string &adaptor_name
) {
    using namespace Gem::Geneva::Parameters;
    std::vector<double> out;

    if(adaptor_name == "GDoubleGaussAdaptor" && scratch.hasAux(AUXKEY_GAUSS_DOUBLE)) {
        std::span<const GaussState<double>> states = scratch.metaRecords<GaussState<double>>(AUXKEY_GAUSS_DOUBLE);
        const auto &groups = cfg.doubleGroups();
        for(std::size_t gi = 0; gi < groups.size(); ++gi) {
            if(groups[gi].has_gauss) {
                out.push_back(states[gi].sigma);
            }
        }
    }
    else if(adaptor_name == "GFloatGaussAdaptor" && scratch.hasAux(AUXKEY_GAUSS_FLOAT)) {
        std::span<const GaussState<float>> states = scratch.metaRecords<GaussState<float>>(AUXKEY_GAUSS_FLOAT);
        const auto &groups = cfg.floatGroups();
        for(std::size_t gi = 0; gi < groups.size(); ++gi) {
            if(groups[gi].has_gauss) {
                out.push_back(static_cast<double>(states[gi].sigma));
            }
        }
    }
    else if(adaptor_name == "GInt32GaussAdaptor" && scratch.hasAux(AUXKEY_GAUSS_INT)) {
        std::span<const GaussState<double>> states = scratch.metaRecords<GaussState<double>>(AUXKEY_GAUSS_INT);
        const auto &groups = cfg.int32Groups();
        for(std::size_t gi = 0; gi < groups.size(); ++gi) {
            if(groups[gi].has_gauss) {
                out.push_back(states[gi].sigma);
            }
        }
    }

    return out;
}

/******************************************************************************/
/**
 * @brief Seeds the per-group adaption state blocks for an individual's genome into a (typically empty,
 * OA-owned) auxiliary store — the convenience one-shot used by self-driven adaption sites, by the
 * off-slot sigma readers, and by tests. The OA's per-iteration setup instead seeds each slot from the
 * single shared config it already holds (config.installInto(slot.scratch())); this helper is for callers
 * that do not keep a config around.
 */
inline void seedAdaptionStates(const detail::GFlatGenome &ind, detail::GAuxiliaryStore &scratch) {
    GAdaptionConfigBase(ind).installInto(scratch);
}

/******************************************************************************/
/**
 * @brief The convenience factory for an OA-owned adaption config, built from a representative genome so it
 * describes exactly the groups that exist. ConfigT selects the per-OA type (GEAAdaptionConfig /
 * GSAAdaptionConfig / the plain base). Today the genome's layout still carries the adaptor settings, so the
 * built config snapshots them directly; after the GGenomeLayout config-strip the genome supplies only the
 * group SKELETON and the caller authors the adaptors onto the returned config via its fluent API
 * (cfg->groupDouble(i).gauss(...) / cfg->forLabel(...).gauss(...)). This is the single seam every call site
 * routes through, so the strip stays invisible to them. It is an oa-side factory because GAdaptionConfig
 * lives in geneva/oa/ while GGenomeBuilder lives in geneva/ind/ (oa depends on ind, not the reverse).
 */
template <typename ConfigT = GAdaptionConfigBase>
std::shared_ptr<ConfigT> makeAdaptionConfig(const detail::GFlatGenome &genome) {
    return std::make_shared<ConfigT>(genome);
}

/******************************************************************************/
/**
 * @brief A small RAII helper that gives a single, slot-less individual its own adaption scratch + config
 * so the data-oriented adaption can be driven outside an optimization algorithm (test individuals'
 * modify hooks, standalone perturbation loops, serialization benchmarks). Construct once and call
 * adapt() repeatedly to preserve sigma self-adaptation across iterations, exactly as the individual's
 * former adapt() did via its own per-individual aux state.
 *
 * Two construction modes: the genome-only form derives the config from the genome (used while the layout
 * still carries the adaptor settings); the explicit-config form takes an OA-owned config the caller
 * authored (the post-strip form — the config is validated against the genome's structure). Both copy the
 * config's base data, which is all the data-oriented adaption needs.
 */
class StandaloneAdapter {
public:
    explicit StandaloneAdapter(const detail::GFlatGenome &ind)
      : cfg_(ind) {
        cfg_.installInto(scratch_);
    }

    StandaloneAdapter(const detail::GFlatGenome &ind, const std::shared_ptr<GAdaptionConfigBase> &cfg)
      : cfg_(*cfg) {
        cfg_.checkConsistency(ind);
        cfg_.installInto(scratch_);
    }

    std::size_t adapt(detail::GFlatGenome &ind) { return adaptIndividual(ind, scratch_, cfg_); }

private:
    detail::GAuxiliaryStore scratch_;
    GAdaptionConfigBase cfg_;
};

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
