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
#include <any>
#include <cstdint>
#include <memory>
#include <random>
#include <span>
#include <string>
#include <tuple>
#include <vector>

// Boost header files go here
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ptree_serialization.hpp>

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"
#include "courtier/GWireSerializationContext.hpp" // layout send-once: the wire (de)serialization context
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GAdaptionKernels.hpp"
#include "geneva/ind/GGenomeLayout.hpp"
#include "geneva/ind/GGenomeLayoutSerialization.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"

// aliases for ease of use
namespace pt = boost::property_tree;

namespace Gem::Geneva::Genome {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The flat-genome implementation of GOptimizableEntity, and the sole genome implementation.
 * GFlatGenome stores the parameters as four contiguous, type-homogeneous value arrays
 * (double / float / int32 / bool) plus a handle to a shared, immutable GGenomeLayout that describes
 * their bounds, grouping and adaption configuration. The per-individual, per-group adaption state
 * (Gauss sigma, ...) lives in the inherited GAuxiliaryStore, not in the genome.
 *
 * Because all genome state is generic, a typical concrete individual adds *no* extra members and only
 * supplies a constructor (which builds its genome with GGenomeBuilder and calls setGenome()) and
 * fitnessCalculation(). The clone/load/compare/serialize machinery is provided here and reused
 * unchanged via the GFlatIndividualT CRTP base.
 *
 * Value access is genome-agnostic: GFlatGenome implements the DM §2 per-type channel virtuals declared
 * on GOptimizableEntity, so the optimization algorithms read and write parameter values without a
 * downcast. A floating-point parameter is stored in the NORMALIZED INTERNAL coordinate (magnitude ≈ 1,
 * confined to [-0.5, 0.5) for a bounded parameter); the user-facing EXTERNAL value is its affine image
 * (normalized-genome architecture, §2). This realizes the two-reader split: the OAs read/write the raw
 * internal value (streamlineInternal_/assignValueVectorInternal_), while the objective function, GPU
 * marshaller and user inspection read the scaled external value (streamline_/assignValueVector_). The
 * fold lives at WRITE time, not read time: an internal (OA) write folds an overshooting bounded value
 * back into [-0.5, 0.5) (and the post-kernel foldConstrainedValuesInPlace() does the same for the
 * adaption path), while an external (user) write range-validates and throws. Integer parameters are not
 * normalized (§2.7): they keep the closed-range integer fold, applied on read.
 */
class GFlatGenome // NOLINT(cppcoreguidelines-special-member-functions)
  : public GOptimizableEntity {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /** @brief Single declaration of this class's plain local data members (the four value channels),
     *  feeding save()/load()/load_()/compare_() from one source. Defined before save() so its deduced
     *  (auto) return type is available there. The shared layout_ and the transient input_omitted_ are
     *  handled separately (the layout is interned on the wire and shared, not value-copied; input_omitted_
     *  is a load-only transient), so they are deliberately NOT listed here. */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            Gem::Common::make_member("dv_", self.dv_),
            Gem::Common::make_member("fv_", self.fv_),
            Gem::Common::make_member("iv_", self.iv_),
            Gem::Common::make_member("bv_", self.bv_)
        );
    }

    /**
     * @brief Serialises the genome: the four value channels plus the shared structural layout, the
     * latter either by value (self-contained form) or by content id (transport send-once form), as
     * selected by the active wire-serialisation scope. See the body for the two forms.
     *
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive to write the genome into
     * @param version The (unused) serialization version number
     */
    template <typename Archive>
    void save(Archive &ar, [[maybe_unused]] const unsigned int version) const {
        using boost::serialization::make_nvp;
        ar &make_nvp("GOptimizableEntity", boost::serialization::base_object<GOptimizableEntity>(*this));

        const auto *ctx = Gem::Courtier::GWireSerializationScope::current();

        // RESULTS-ONLY RETURN: when a worker returns a processed item (ctx->returning) and has not been
        // asked to return the modified individual in full, omit the (potentially large) input parameters
        // and layout entirely -- only the computed results, already written via the base above, travel.
        // The server still holds the originally-submitted item and grafts its parameters back on. A
        // leading `genome_omitted` marker makes the stream self-describing. (The base carries the full
        // multi-criterion result set, so multi-evaluation individuals return correctly.)
        const bool genome_omitted = (ctx != nullptr) && ctx->enabled && ctx->returning &&
                                    not this->getReturnFullIndividual();
        ar &make_nvp("genome_omitted", genome_omitted);
        if(genome_omitted) {
            return;
        }

        Gem::Common::serialize_members(ar, localMembers_(*this));

        // The transient per-group adaption state is NOT serialised here (it is OA-owned slot scratch);
        // full-state checkpointing is a separate concern.
        //
        // The layout is shared & immutable in memory; sharing does not survive serialisation, so a
        // deserialised genome legitimately owns its own layout copy. There are two wire forms:
        //   - SELF-CONTAINED (checkpoint / file, or any transport with interning off): the full layout
        //     travels by value. This is the only form when no wire-serialisation scope is active.
        //   - SEND-ONCE (transport with an enabled scope): the layout is referenced by its content id;
        //     the full layout is shipped to a given peer only the FIRST time that id is seen on its
        //     session, and id-only thereafter (a worker cache miss is resolved by a fetch). This cuts
        //     the per-item payload for a large structured genome, whose layout is identical across the
        //     whole population, from O(parameters) down to a 16-byte id.
        // A leading `layout_interned` tag makes the stream self-describing, so load() follows the tag
        // regardless of its own scope.
        const bool interned =
            (ctx != nullptr) && ctx->enabled && (ctx->registry != nullptr) && (layout_ != nullptr);
        ar &make_nvp("layout_interned", interned);
        if(not interned) {
            GGenomeLayout layout_copy = layout_ ? *layout_ : GGenomeLayout{};
            ar &make_nvp("layout_", layout_copy);
            return;
        }

        const LayoutId lid = layout_->layoutId();
        Gem::Courtier::GWireLayoutId wid{lid.hi, lid.lo};
        ar &make_nvp("layout_id_hi", wid[0]);
        ar &make_nvp("layout_id_lo", wid[1]);
        // Keep a blob in the registry so the server can answer a worker's later cache-miss fetch for
        // this id (amortised: serialised once per distinct layout, not per item).
        if(not ctx->registry->has(wid)) {
            ctx->registry->put(wid, layoutToWireBlob(*layout_));
        }
        bool layout_present = not ctx->registry->peerHasLayout(ctx->peer, wid);
        ar &make_nvp("layout_present", layout_present);
        if(layout_present) {
            GGenomeLayout layout_copy = *layout_;
            ar &make_nvp("layout_", layout_copy);
            // Optimistic: assume the peer receives it. If the send fails the worker simply cache-misses
            // and fetches, so marking before the wire write is safe (and keeps subsequent items id-only).
            ctx->registry->markPeerHasLayout(ctx->peer, wid);
        }
    }

    /**
     * @brief Restores the genome from an archive: the value channels plus a freshly-owned layout, read
     * either by value or resolved from a content id against the wire registry (with a cache-miss fetch),
     * following the self-describing tag written by save().
     *
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive to read the genome from
     * @param version The (unused) serialization version number
     */
    template <typename Archive>
    void load(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;
        ar &make_nvp("GOptimizableEntity", boost::serialization::base_object<GOptimizableEntity>(*this));

        // A results-only return (see save()) omits the input parameters + layout: leave the genome empty
        // and flag it so the server grafts the originally-submitted parameters back on. The base above
        // already carries the computed results.
        bool genome_omitted = false;
        ar &make_nvp("genome_omitted", genome_omitted);
        if(genome_omitted) {
            // Leave NO stale input data behind (the object may have been default-constructed with a
            // genome): an empty, unambiguous genome that the server replaces wholesale when it grafts
            // the originally-submitted parameters back on (see graftInputDataFrom_).
            input_omitted_ = true;
            dv_.clear();
            fv_.clear();
            iv_.clear();
            bv_.clear();
            layout_ = std::make_shared<const GGenomeLayout>();
            return;
        }
        input_omitted_ = false;

        Gem::Common::serialize_members(ar, localMembers_(*this));
        // The per-group adaption state is OA-owned scratch (on the GIndividualSlot): an optimization
        // algorithm seeds each slot's scratch from its config at setup.

        // Follow the self-describing wire form written by save() (see there for the two forms).
        bool interned = false;
        ar &make_nvp("layout_interned", interned);
        if(not interned) {
            auto fresh = std::make_shared<GGenomeLayout>();
            ar &make_nvp("layout_", *fresh);
            layout_ = fresh;
            return;
        }

        Gem::Courtier::GWireLayoutId wid{};
        ar &make_nvp("layout_id_hi", wid[0]);
        ar &make_nvp("layout_id_lo", wid[1]);
        bool layout_present = false;
        ar &make_nvp("layout_present", layout_present);

        const auto *ctx = Gem::Courtier::GWireSerializationScope::current();
        if(layout_present) {
            auto fresh = std::make_shared<GGenomeLayout>();
            ar &make_nvp("layout_", *fresh);
            layout_ = fresh;
            // Cache the blob so subsequent id-only items for this layout resolve locally (no fetch).
            if(ctx != nullptr && ctx->registry != nullptr && not ctx->registry->has(wid)) {
                ctx->registry->put(wid, layoutToWireBlob(*fresh));
            }
            return;
        }

        // id-only reference: resolve from the local registry, fetching from the server on a miss
        // (reconnect / late-joining worker / server-side eviction).
        std::string blob;
        bool have = (ctx != nullptr) && (ctx->registry != nullptr) && ctx->registry->tryGet(wid, blob);
        if(not have && ctx != nullptr && ctx->fetch_blob) {
            blob = ctx->fetch_blob(wid);
            if(not blob.empty()) {
                if(ctx->registry != nullptr) {
                    ctx->registry->put(wid, blob);
                }
                have = true;
            }
        }
        if(not have || blob.empty()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GFlatGenome::load(): Error!" << '\n'
                << "An id-only layout reference could not be resolved (cache miss with no usable fetch)."
                << '\n'
            );
        }
        layout_ = layoutFromWireBlob(blob);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GFlatGenome();
    /**
     * @brief Initialization with the number of fitness criteria.
     *
     * @param n_fitness_criteria The number of fitness criteria this genome will report
     */
    explicit GFlatGenome(std::size_t n_fitness_criteria);
    /**
     * @brief The copy constructor.
     *
     * @param cp The genome to copy from (value channels and shared layout handle)
     */
    GFlatGenome(GFlatGenome const &cp);
    /** @brief The destructor */
    ~GFlatGenome() override = default;

    /** @brief Un-hide the inherited public load(shared_ptr/unique_ptr/ref) overloads, which the
     *  boost split-member load(Archive&, unsigned) below would otherwise hide by name. */
    using GOptimizableEntity::load;

    /**
     * @brief Installs the value arrays + shared layout produced by a GGenomeBuilder.
     *
     * @param g The genome data (the four value channels plus the shared immutable layout)
     */
    void setGenome(GenomeData const &g);

    /**
     * @brief Direct, shared access to the structural layout (problem metadata).
     *
     * @return A shared handle to the immutable layout describing bounds, grouping and adaption config
     */
    std::shared_ptr<const GGenomeLayout> getLayout() const noexcept { return layout_; }

    /***************************************************************************/
    // Mutable access to the raw INTERNAL (normalized) value arrays. The OA-owned adaption kernels add
    // their step directly to these spans (then foldConstrainedValuesInPlace() folds a bounded value back
    // into [-0.5, 0.5)); ordinary external value access goes through streamline()/assignValueVector().

    /** @brief @return A mutable span over the raw internal double channel (unfolded representation) */
    std::span<double> internalDoubleValues() noexcept { return {dv_.data(), dv_.size()}; }
    /** @brief @return A mutable span over the raw internal float channel (unfolded representation) */
    std::span<float> internalFloatValues() noexcept { return {fv_.data(), fv_.size()}; }
    /** @brief @return A mutable span over the raw internal int32 channel (unfolded representation) */
    std::span<std::int32_t> internalInt32Values() noexcept { return {iv_.data(), iv_.size()}; }
    /** @brief @return A mutable span over the raw internal bool channel (bytes, 1/0) */
    std::span<std::uint8_t> internalBoolValues() noexcept { return {bv_.data(), bv_.size()}; }

    /** @brief @return A read-only span over the raw internal double channel (unfolded representation) */
    std::span<const double> internalDoubleValues() const noexcept { return {dv_.data(), dv_.size()}; }
    /** @brief @return A read-only span over the raw internal float channel (unfolded representation) */
    std::span<const float> internalFloatValues() const noexcept { return {fv_.data(), fv_.size()}; }
    /** @brief @return A read-only span over the raw internal int32 channel (unfolded representation) */
    std::span<const std::int32_t> internalInt32Values() const noexcept { return {iv_.data(), iv_.size()}; }
    /** @brief @return A read-only span over the raw internal bool channel (bytes, 1/0) */
    std::span<const std::uint8_t> internalBoolValues() const noexcept { return {bv_.data(), bv_.size()}; }

    /**
     * @brief Transformation of the individual's parameters into a boost::property_tree object.
     *
     * @param ptr The property tree to populate with this individual's parameters
     * @param base_name The base path / key prefix to write under (defaults to "parameterset")
     */
    void toPropertyTree(pt::ptree &ptr, std::string const &base_name = "parameterset") const override;

    /**
     * @brief Transformation of the individual's parameters into a list of comma-separated values.
     *
     * @param with_name_and_type Whether to prefix each value with its name and type (default false)
     * @param with_commas Whether to separate the values with commas (default true)
     * @param use_raw_fitness Whether to emit the raw rather than the transformed fitness (default true)
     * @param show_validity Whether to append the individual's validity flag (default true)
     * @return The parameters (and optionally fitness/validity) as a single CSV string
     */
    std::string toCSV(
        bool with_name_and_type = false
        ,
        bool with_commas = true
        ,
        bool use_raw_fitness = true
        ,
        bool show_validity = true
    ) const override;

    /**
     * @brief Perform a cross-over operation between this object and another.
     *
     * @param cp_base The other genome to cross over with (must be a GFlatGenome)
     * @return A new genome holding the recombined parameter values
     */
    std::shared_ptr<GOptimizableEntity> crossOverWith(GOptimizableEntity const &cp_base) const override;

    /**
     * @brief Retrieves parameters relevant for the evaluation from another GFlatGenome.
     *
     * @param cp_base The genome whose evaluation-relevant parameters are absorbed into this one
     */
    void cannibalize(GOptimizableEntity &cp_base) override;

    /***************************************************************************/
    /** @brief Bulk-flatten fast path: streamlines an FP/int32 channel DIRECTLY into a caller-provided
     *  buffer (no temporary vector, no second copy), returning the number of values written. Produces
     *  exactly the same external (scaled) values as streamline<T>() with the same activityMode --
     *  use it to flatten a whole population into one contiguous device buffer (the GPU marshallers).
     *  NB: a raw memcpy of the channel storage is NOT a valid substitute -- the stored value is the
     *  normalized internal coordinate, mapped to the external user value only here.
     *
     *  @param dst The caller-provided destination buffer for the double channel (must be large enough)
     *  @param am The activity mode selecting which parameters are written (default: all parameters)
     *  @return The number of values written into dst */
    std::size_t streamlineInto(double *dst, activityMode const &am = activityMode::DEFAULTACTIVITYMODE) const {
        return streamlineIntoImpl<double>(dst, layout_->d, dv_, am);
    }
    /** @brief Bulk-flatten fast path for the float channel (see the double overload).
     *  @param dst The caller-provided destination buffer for the float channel (must be large enough)
     *  @param am The activity mode selecting which parameters are written (default: all parameters)
     *  @return The number of values written into dst */
    std::size_t streamlineInto(float *dst, activityMode const &am = activityMode::DEFAULTACTIVITYMODE) const {
        return streamlineIntoImpl<float>(dst, layout_->f, fv_, am);
    }
    /** @brief Bulk-flatten fast path for the int32 channel (see the double overload).
     *  @param dst The caller-provided destination buffer for the int32 channel (must be large enough)
     *  @param am The activity mode selecting which parameters are written (default: all parameters)
     *  @return The number of values written into dst */
    std::size_t streamlineInto(std::int32_t *dst, activityMode const &am = activityMode::DEFAULTACTIVITYMODE) const {
        return streamlineIntoImpl<std::int32_t>(dst, layout_->i, iv_, am);
    }

    /** @brief Adaption-time write-fold (§2.2): folds every bounded value back into range in the FP and
     *  int32 stores (see foldChannelInPlace for the per-channel semantics). Called by the OA adaption
     *  driver right after the kernels run. The bool channel is always valid (0/1), so it is not folded. */
    void foldConstrainedValuesInPlace() {
        foldChannelInPlace<double>(layout_->d, dv_);
        foldChannelInPlace<float>(layout_->f, fv_);
        foldChannelInPlace<std::int32_t>(layout_->i, iv_);
    }

    /***************************************************************************/
    // Deleted functions

    explicit GFlatGenome(float const &) = delete;  ///< Intentionally undefined (deleted to forbid a single-float init)
    explicit GFlatGenome(double const &) = delete; ///< Intentionally undefined (deleted to forbid a single-double init)

protected:
    /**
     * @brief Loads the data of another GFlatGenome, camouflaged as a base pointer.
     *
     * @param cp A base pointer to the GFlatGenome whose data is copied into this object
     */
    void load_(const GOptimizableEntity *cp) override;

    /**
     * @brief Allow access to this class's compare_ function.
     *
     * @param GFlatGenome The first genome to compare
     * @param GFlatGenome The second genome to compare
     * @param GToken The token accumulating the comparison results / deviations
     */
    friend void Gem::Common::compare_base_t<GFlatGenome>(
        GFlatGenome const &,
        GFlatGenome const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type.
     *
     * @param cp The other object to compare this one against
     * @param e The expected relation (equality / inequality) between the two objects
     * @param limit The maximum allowed deviation for floating-point comparisons (the limit)
     */
    void compare_(
        GOptimizableEntity const &cp,
        Gem::Common::expectation const &e,
        [[maybe_unused]] double const &limit
    ) const override;

    /**
     * @brief Random initialization of the genome's parameter values.
     *
     * @param activityMode The activity mode selecting which parameters are randomly initialised
     * @return true if at least one parameter value was changed, false otherwise
     */
    bool randomInit_(activityMode const &am) override;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /***************************************************************************/
    // Overridden or virtual private functions

    /** @brief Emits a name for this class / object
     *  @return The name of this class / object */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object (supplied by the concrete individual)
     *  @return A heap-allocated deep copy of this genome */
    GFlatGenome *clone_() const override = 0;

    /** @brief Whether this genome was deserialised from a results-only return (input data omitted).
     *  @return true iff the input parameters were omitted on the wire and must be grafted. */
    bool inputDataOmitted_() const override { return input_omitted_; }
    /** @brief Grafts the input parameters (value channels + shared layout) of @p original onto this
     *  results-only genome, which already carries the computed results. After the graft this genome is
     *  complete and equivalent to a full return.
     *  @param original The originally-submitted item (a GFlatGenome) that supplies the input data. */
    void graftInputDataFrom_(const GOptimizableEntity &original) override {
        const auto &src = dynamic_cast<const GFlatGenome &>(original);
        dv_ = src.dv_;
        fv_ = src.fv_;
        iv_ = src.iv_;
        bv_ = src.bv_;
        layout_ = src.layout_;
        input_omitted_ = false;
    }

    /** @brief Retrieve the active double parameter at the given positional index.
     *  @param idx The position of the parameter among the active double parameters
     *  @return The (range-folded) value of that double parameter */
    double getVarVal_d_(std::size_t idx) override;
    /** @brief Retrieve the active float parameter at the given positional index.
     *  @param idx The position of the parameter among the active float parameters
     *  @return The (range-folded) value of that float parameter */
    float getVarVal_f_(std::size_t idx) override;
    /** @brief Retrieve the active int32 parameter at the given positional index.
     *  @param idx The position of the parameter among the active int32 parameters
     *  @return The (range-folded) value of that int32 parameter */
    std::int32_t getVarVal_i_(std::size_t idx) override;
    /** @brief Retrieve the active bool parameter at the given positional index.
     *  @param idx The position of the parameter among the active bool parameters
     *  @return The value of that bool parameter */
    bool getVarVal_b_(std::size_t idx) override;

    /** @brief Retrieval of a suitable position for cross over inside of a vector.
     *  @param lower The size of the value vector to pick a cross-over position in
     *  @param upper The minimum allowed cross-over position
     *  @return A valid cross-over position within the vector */
    std::size_t getCrossOverPos(std::size_t lower, std::size_t upper);

    /***************************************************************************/
    // Activity / fold helpers (genome-agnostic value mapping).

    /** @brief Whether a value with the given active flag matches the requested activity mode.
     *  @param active The per-parameter active flag (non-zero == active)
     *  @param am The requested activity mode (active-only / inactive-only / all)
     *  @return true if the parameter should be included under the given activity mode */
    static bool amMatch(std::uint8_t active, activityMode const &am) {
        switch(am) {
        case activityMode::ACTIVEONLY:
            return active != 0;
        case activityMode::INACTIVEONLY:
            return active == 0;
        case activityMode::ALLPARAMETERS: // == DEFAULTACTIVITYMODE
        default:
            return true;
        }
    }

    /** @brief THE single source of truth for the per-element internal->external map (normalized-genome
     *  architecture §2.1). For a FLOATING-POINT channel the stored value is the NORMALIZED internal
     *  coordinate (magnitude ≈ 1, confined to [-0.5, 0.5) for a bounded parameter); the external value is
     *  its affine image `anchor + u*scale` (composed in long double), with the half-open [lo, hi) contract
     *  re-enforced after the narrowing cast for a bounded parameter (an unbounded one has no wall). The
     *  fold is NOT applied here -- it was moved to write time (§2.2), so the read path is scale-only; a
     *  DEBUG build asserts the internal value really is in range, catching any write path that forgot to
     *  fold/validate. INTEGER channels are NOT normalized (§2.7): they keep the closed-range integer fold.
     *
     *  @tparam T The channel's value type (double / float / int32)
     *  @param ch The channel layout describing the parameter's kind and bounds
     *  @param stored The internally stored value (normalized internal for FP, unbounded int for int32)
     *  @param k The index of the parameter within the channel
     *  @return The external (user-coordinate) representation of the stored value */
    template <typename T>
    static T externalValue(ChannelLayout<T> const &ch, T stored, std::size_t k) {
        if constexpr(std::is_floating_point_v<T>) {
            const T scale = ngScale<T>(ch, k);
            const T anchor = ngAnchor<T>(ch, k);
            if(not ch.fold[k]) {
                // Unbounded (plain): the internal value may roam ℝ; affine-map it, no fold, no clamp.
                return ngInternalToExternal<T>(stored, scale, anchor);
            }
#ifdef DEBUG
            // Fold-on-write guarantees the internal value is in [-0.5, 0.5); a violation means a write
            // path stored a raw value without folding/validating. (Frozen scale <= 0 stores the centre 0.)
            if(scale > T(0) && not(stored >= T(-0.5) && stored < T(0.5))) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GFlatGenome::externalValue(): Error!" << '\n'
                    << "Internal coordinate " << stored << " of bounded parameter " << k
                    << " is outside the canonical interval [-0.5, 0.5) -- a write path failed to fold." << '\n'
                );
            }
#endif /* DEBUG */
            // The read-side clamp is in EXTERNAL coordinates and is not redundant with the write-side
            // internal clamp: a valid internal value (< 0.5) can still round, through the affine map and
            // the narrowing cast, exactly onto the exclusive external upper -- this enforces [lo, hi) there.
            return ngClampHalfOpen<T>(
                ngInternalToExternal<T>(stored, scale, anchor), ch.lower[k], ch.upper[k]
            );
        }
        else {
            if(not ch.fold[k]) {
                return stored;
            }
            return foldConstrainedInt<T>(stored, ch.lower[k], ch.upper[k]);
        }
    }

    /** @brief The per-element EXTERNAL->INTERNAL map for a floating-point external write (§2.2 write
     *  taxonomy). A bounded parameter is range-validated against the half-open [lo, hi) (throwing on an
     *  out-of-range value, INCLUDING exactly the open upper bound -- a user error, not silently clamped),
     *  then scaled to the normalized internal coordinate; an unbounded parameter is scaled with no
     *  validation. A frozen parameter (scale <= 0, lo == hi) maps to the interval centre 0.
     *
     *  @p allow_upper_bound relaxes the upper check to the CLOSED [lo, hi] for a DECLARED start value (the
     *  builder's init / a factory default may legitimately sit exactly on the bound, e.g. a tunable whose
     *  default equals its max); such a boundary value snaps to the largest representable internal value
     *  just inside the wall. A runtime user assignment leaves it false, so exactly the upper bound throws.
     *
     *  @tparam T The floating-point channel's value type (double / float)
     *  @param ch The channel layout describing the parameter's kind and bounds
     *  @param x The external (user-coordinate) value being written
     *  @param k The index of the parameter within the channel
     *  @param allow_upper_bound Whether a value exactly on the (inclusive) upper bound is accepted + snapped
     *  @return The normalized internal coordinate to store */
    template <typename T>
    static T externalToInternalChecked(
        ChannelLayout<T> const &ch, T x, std::size_t k, bool allow_upper_bound = false
    ) {
        static_assert(std::is_floating_point_v<T>, "FP-only external write");
        const T scale = ngScale<T>(ch, k);
        const T anchor = ngAnchor<T>(ch, k);
        if(ch.fold[k]) {
            if(scale <= T(0)) {
                return T(0); // frozen: the single valid value maps to the centre
            }
            const bool below = x < ch.lower[k];
            const bool above = allow_upper_bound ? (x > ch.upper[k]) : not(x < ch.upper[k]);
            if(below || above) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GFlatGenome::externalToInternalChecked(): Error!" << '\n'
                    << "External value " << x << " is outside the "
                    << (allow_upper_bound ? "range [" : "half-open range [")
                    << ch.lower[k] << ", " << ch.upper[k] << (allow_upper_bound ? "]" : ")")
                    << " of bounded parameter " << k << '\n'
                );
            }
            // A validated in-range x maps into [-0.5, 0.5); enforce the half-open internal contract in
            // case the narrowing cast (or an inclusive-upper start value) lands exactly on the open upper.
            return ngClampHalfOpen<T>(ngExternalToInternal<T>(x, scale, anchor), T(-0.5), T(0.5));
        }
        return ngExternalToInternal<T>(x, scale, anchor); // unbounded: no validation
    }

    /***************************************************************************/
    // Per-type channel helpers -- the templated bodies the typed virtual overrides forward to.

    /** @brief Collects a channel's active, range-folded values into an output vector.
     *  @tparam T The channel's value type (double / float / int32)
     *  @param out The output vector, cleared then filled with the selected external values
     *  @param ch The channel layout (kind / bounds / active flags)
     *  @param store The internal value storage for this channel
     *  @param am The activity mode selecting which parameters are collected */
    template <typename T>
    void streamlineImpl(
        std::vector<T> &out,
        ChannelLayout<T> const &ch,
        std::vector<T> const &store,
        activityMode const &am
    ) const {
        out.clear();
        for(std::size_t k = 0; k < store.size(); ++k) {
            if(amMatch(ch.active[k], am)) {
                out.push_back(externalValue<T>(ch, store[k], k));
            }
        }
    }

    /** @brief Writes the active channel's external (scaled) values DIRECTLY into a caller buffer,
     *  returning the count written. Same per-element internal->external map as streamlineImpl() (both via
     *  externalValue()), but no temporary vector and no second copy -- the bulk-flatten fast path behind
     *  streamlineInto(). The stored value is the normalized internal coordinate, so a raw memcpy of
     *  `store` is not a valid substitute.
     *
     *  @tparam T The channel's value type (double / float / int32)
     *  @param dst The caller-provided destination buffer (must hold at least the active count)
     *  @param ch The channel layout (kind / bounds / active flags)
     *  @param store The internal value storage for this channel
     *  @param am The activity mode selecting which parameters are written
     *  @return The number of values written into dst */
    template <typename T>
    std::size_t streamlineIntoImpl(
        T *dst,
        ChannelLayout<T> const &ch,
        std::vector<T> const &store,
        activityMode const &am
    ) const {
        std::size_t n = 0;
        for(std::size_t k = 0; k < store.size(); ++k) {
            if(amMatch(ch.active[k], am)) {
                dst[n++] = externalValue<T>(ch, store[k], k);
            }
        }
        return n;
    }

    /** @brief Adaption-time write-fold (§2.2): contains every bounded value IN THE STORE after the
     *  adaption kernels added their (unbounded) step. A floating-point bounded value is folded into the
     *  canonical internal interval [-0.5, 0.5) (ngFoldInternal); an integer bounded value is folded into
     *  its closed [lo, hi] range (foldConstrainedInt); unbounded values are left to roam. The fold is
     *  external-value-preserving and idempotent, so this keeps the stored internal magnitude bounded
     *  without changing the value an OA's step produced.
     *
     *  @tparam T The channel's value type (double / float / int32)
     *  @param ch The channel layout (kind / bounds)
     *  @param store The internal value storage, folded in place */
    template <typename T>
    static void foldChannelInPlace(ChannelLayout<T> const &ch, std::vector<T> &store) {
        for(std::size_t k = 0; k < store.size(); ++k) {
            if(not ch.fold[k]) {
                continue; // unbounded: roams freely
            }
            if constexpr(std::is_floating_point_v<T>) {
                store[k] = ngFoldInternal<T>(store[k]);
            }
            else {
                store[k] = foldConstrainedInt<T>(store[k], ch.lower[k], ch.upper[k]);
            }
        }
    }

    /** @brief Writes a vector of EXTERNAL (user-coordinate) values back into the active slots of a
     *  channel's storage (§2.2 external write). A floating-point value is range-validated and scaled to
     *  the normalized internal coordinate (externalToInternalChecked -- throws on an out-of-range bounded
     *  value); an integer value is stored raw (it is not normalized -- folded into its closed range on
     *  read).
     *  @tparam T The channel's value type (double / float / int32)
     *  @param in The incoming external values, consumed in order for each active parameter
     *  @param ch The channel layout (kind / bounds / active flags)
     *  @param store The internal value storage to update at the active positions
     *  @param am The activity mode selecting which parameters are overwritten */
    template <typename T>
    void assignImpl(
        std::vector<T> const &in,
        ChannelLayout<T> const &ch,
        std::vector<T> &store,
        activityMode const &am
    ) {
        std::size_t pos = 0;
        for(std::size_t k = 0; k < store.size(); ++k) {
            if(amMatch(ch.active[k], am)) {
                const T x = in.at(pos++);
                if constexpr(std::is_floating_point_v<T>) {
                    store[k] = externalToInternalChecked<T>(ch, x, k);
                }
                else {
                    store[k] = x; // int: stored raw, folded into its closed range on read
                }
            }
        }
    }

    /** @brief Collects a channel's active values in their raw INTERNAL representation (no scale, no fold)
     *  -- the OA-facing read of the two-reader split (§2.3). For an FP channel this is the normalized
     *  internal coordinate; only the FP channels use it.
     *  @tparam T The channel's value type (double / float)
     *  @param out The output vector, cleared then filled with the selected internal values
     *  @param ch The channel layout (active flags)
     *  @param store The internal value storage for this channel
     *  @param am The activity mode selecting which parameters are collected */
    template <typename T>
    void streamlineInternalImpl(
        std::vector<T> &out,
        ChannelLayout<T> const &ch,
        std::vector<T> const &store,
        activityMode const &am
    ) const {
        out.clear();
        for(std::size_t k = 0; k < store.size(); ++k) {
            if(amMatch(ch.active[k], am)) {
                out.push_back(store[k]);
            }
        }
    }

    /** @brief Writes a vector of raw INTERNAL values back into the active slots of an FP channel (§2.2
     *  internal write): a bounded value is FOLDED into the canonical interval [-0.5, 0.5) (the fold is
     *  external-value-preserving, so an OA's overshooting step is contained, not rejected); an unbounded
     *  value is stored as-is (it may roam). Never validates / throws -- that is the external write's job.
     *  @tparam T The FP channel's value type (double / float)
     *  @param in The incoming internal values, consumed in order for each active parameter
     *  @param ch The channel layout (kind / active flags)
     *  @param store The internal value storage to update at the active positions
     *  @param am The activity mode selecting which parameters are overwritten */
    template <typename T>
    void assignInternalImpl(
        std::vector<T> const &in,
        ChannelLayout<T> const &ch,
        std::vector<T> &store,
        activityMode const &am
    ) {
        static_assert(std::is_floating_point_v<T>, "the internal write is FP-only");
        std::size_t pos = 0;
        for(std::size_t k = 0; k < store.size(); ++k) {
            if(amMatch(ch.active[k], am)) {
                const T u = in.at(pos++);
                store[k] = (ch.fold[k]) ? ngFoldInternal<T>(u) : u;
            }
        }
    }

    /** @brief Counts how many of a channel's parameters match the requested activity mode.
     *  @tparam T The channel's value type (double / float / int32 / bool)
     *  @param ch The channel layout (active flags)
     *  @param am The activity mode selecting which parameters are counted
     *  @return The number of matching parameters */
    template <typename T>
    std::size_t countImpl(ChannelLayout<T> const &ch, activityMode const &am) const {
        std::size_t n = 0;
        for(std::uint8_t a : ch.active) {
            if(amMatch(a, am)) {
                ++n;
            }
        }
        return n;
    }

    /** @brief Collects the lower/upper bounds of a channel's active parameters.
     *  @tparam T The channel's value type (double / float / int32)
     *  @param l Output vector filled with each active parameter's lower bound (type lowest if unbounded)
     *  @param u Output vector filled with each active parameter's upper bound (type max if unbounded)
     *  @param ch The channel layout (kind / bounds / active flags)
     *  @param am The activity mode selecting which parameters are reported */
    template <typename T>
    void boundariesImpl(
        std::vector<T> &l,
        std::vector<T> &u,
        ChannelLayout<T> const &ch,
        activityMode const &am
    ) const {
        l.clear();
        u.clear();
        for(std::size_t k = 0; k < ch.size(); ++k) {
            if(not amMatch(ch.active[k], am)) {
                continue;
            }
            if(ch.fold[k]) {
                l.push_back(ch.lower[k]);
                u.push_back(ch.upper[k]);
            }
            else {
                l.push_back(std::numeric_limits<T>::lowest());
                u.push_back((std::numeric_limits<T>::max)());
            }
        }
    }

    /***************************************************************************/
    // The bool channel is stored as bytes; these adapt the byte storage to the bool-typed surface.

    /** @brief Collects the active bool parameters from the byte storage into a bool vector.
     *  @param out The output vector, cleared then filled with the selected bool values
     *  @param am The activity mode selecting which bool parameters are collected */
    void streamlineBool(std::vector<bool> &out, activityMode const &am) const {
        out.clear();
        for(std::size_t k = 0; k < bv_.size(); ++k) {
            if(amMatch(layout_->b.active[k], am)) {
                out.push_back(bv_[k] != 0);
            }
        }
    }
    /** @brief Writes a bool vector back into the active slots of the byte storage.
     *  @param in The incoming bool values, consumed in order for each active parameter
     *  @param am The activity mode selecting which bool parameters are overwritten */
    void assignBool(std::vector<bool> const &in, activityMode const &am) {
        std::size_t pos = 0;
        for(std::size_t k = 0; k < bv_.size(); ++k) {
            if(amMatch(layout_->b.active[k], am)) {
                bv_[k] = in.at(pos++) ? static_cast<std::uint8_t>(1) : static_cast<std::uint8_t>(0);
            }
        }
    }

    /***************************************************************************/
    // Random-init channel helpers. (The per-channel ADAPTION kernels are OA-owned -- see
    // geneva/oa/GAdaption.hpp, fed by the GIndividualSlot's scratch.)

    /** @brief Randomly initialises a floating-point channel's active parameters.
     *  @tparam T The channel's value type (double / float)
     *  @param store The internal value storage for this channel, overwritten at active positions
     *  @param ch The channel layout (kind / bounds / active flags)
     *  @param am The activity mode selecting which parameters are initialised
     *  @return true if at least one value was changed, false otherwise */
    template <typename T>
    bool randomInitFP(std::vector<T> &store, ChannelLayout<T> const &ch, activityMode const &am);
    /** @brief Randomly initialises the int32 channel's active parameters.
     *  @param am The activity mode selecting which parameters are initialised
     *  @return true if at least one value was changed, false otherwise */
    bool randomInitInt(activityMode const &am);
    /** @brief Randomly initialises the bool channel's active parameters.
     *  @param am The activity mode selecting which parameters are initialised
     *  @return true if at least one value was changed, false otherwise */
    bool randomInitBool(activityMode const &am);

    /***************************************************************************/
    // The DM §2 per-type virtual overrides (forward to the templated helpers above).

    /** @brief Streamlines the double channel into v. @param v Output value vector. @param am Activity mode. */
    void streamline_(std::vector<double> &v, activityMode const &am) const override { streamlineImpl<double>(v, layout_->d, dv_, am); }
    /** @brief Streamlines the float channel into v. @param v Output value vector. @param am Activity mode. */
    void streamline_(std::vector<float> &v, activityMode const &am) const override { streamlineImpl<float>(v, layout_->f, fv_, am); }
    /** @brief Streamlines the int32 channel into v. @param v Output value vector. @param am Activity mode. */
    void streamline_(std::vector<std::int32_t> &v, activityMode const &am) const override { streamlineImpl<std::int32_t>(v, layout_->i, iv_, am); }
    /** @brief Streamlines the bool channel into v. @param v Output value vector. @param am Activity mode. */
    void streamline_(std::vector<bool> &v, activityMode const &am) const override { streamlineBool(v, am); }

    /** @brief Assigns v into the double channel's active slots. @param v Incoming values. @param am Activity mode. */
    void assignValueVector_(std::vector<double> const &v, activityMode const &am) override { assignImpl<double>(v, layout_->d, dv_, am); }
    /** @brief Assigns v into the float channel's active slots. @param v Incoming values. @param am Activity mode. */
    void assignValueVector_(std::vector<float> const &v, activityMode const &am) override { assignImpl<float>(v, layout_->f, fv_, am); }
    /** @brief Assigns v into the int32 channel's active slots. @param v Incoming values. @param am Activity mode. */
    void assignValueVector_(std::vector<std::int32_t> const &v, activityMode const &am) override { assignImpl<std::int32_t>(v, layout_->i, iv_, am); }
    /** @brief Assigns v into the bool channel's active slots. @param v Incoming values. @param am Activity mode. */
    void assignValueVector_(std::vector<bool> const &v, activityMode const &am) override { assignBool(v, am); }

    /** @brief @param am Activity mode. @return The number of matching double parameters. */
    std::size_t countParametersDouble_(activityMode const &am) const override { return countImpl<double>(layout_->d, am); }
    /** @brief @param am Activity mode. @return The number of matching float parameters. */
    std::size_t countParametersFloat_(activityMode const &am) const override { return countImpl<float>(layout_->f, am); }
    /** @brief @param am Activity mode. @return The number of matching int32 parameters. */
    std::size_t countParametersInt32_(activityMode const &am) const override { return countImpl<std::int32_t>(layout_->i, am); }
    /** @brief @param am Activity mode. @return The number of matching bool parameters. */
    std::size_t countParametersBool_(activityMode const &am) const override { return countImpl<bool>(layout_->b, am); }

    /** @brief Collects double-channel bounds. @param l Lower bounds out. @param u Upper bounds out. @param am Activity mode. */
    void boundaries_(std::vector<double> &l, std::vector<double> &u, activityMode const &am) const override { boundariesImpl<double>(l, u, layout_->d, am); }
    /** @brief Collects float-channel bounds. @param l Lower bounds out. @param u Upper bounds out. @param am Activity mode. */
    void boundaries_(std::vector<float> &l, std::vector<float> &u, activityMode const &am) const override { boundariesImpl<float>(l, u, layout_->f, am); }
    /** @brief Collects int32-channel bounds. @param l Lower bounds out. @param u Upper bounds out. @param am Activity mode. */
    void boundaries_(std::vector<std::int32_t> &l, std::vector<std::int32_t> &u, activityMode const &am) const override { boundariesImpl<std::int32_t>(l, u, layout_->i, am); }
    /** @brief Collects bool-channel bounds. @param l Lower bounds out. @param u Upper bounds out. @param am Activity mode. */
    void boundaries_(std::vector<bool> &l, std::vector<bool> &u, activityMode const &am) const override;

    /***************************************************************************/
    // The INTERNAL (normalized) FP-channel virtuals (normalized-genome architecture §2.3) -- the OA-facing
    // half of the two-reader split. Only double/float have an internal/external distinction.

    /** @brief Reads the raw internal double channel into v. @param v Output vector. @param am Activity mode. */
    void streamlineInternal_(std::vector<double> &v, activityMode const &am) const override { streamlineInternalImpl<double>(v, layout_->d, dv_, am); }
    /** @brief Reads the raw internal float channel into v. @param v Output vector. @param am Activity mode. */
    void streamlineInternal_(std::vector<float> &v, activityMode const &am) const override { streamlineInternalImpl<float>(v, layout_->f, fv_, am); }

    /** @brief Internal-writes (folds) v into the double channel's active slots. @param v Internal values. @param am Activity mode. */
    void assignValueVectorInternal_(std::vector<double> const &v, activityMode const &am) override { assignInternalImpl<double>(v, layout_->d, dv_, am); }
    /** @brief Internal-writes (folds) v into the float channel's active slots. @param v Internal values. @param am Activity mode. */
    void assignValueVectorInternal_(std::vector<float> const &v, activityMode const &am) override { assignInternalImpl<float>(v, layout_->f, fv_, am); }

    /***************************************************************************/
    // Data: the four contiguous value channels + the shared structural layout.

    std::vector<double> dv_;        ///< the double channel values (internal representation)
    std::vector<float> fv_;         ///< the float channel values (internal representation)
    std::vector<std::int32_t> iv_;  ///< the int32 channel values (internal representation)
    std::vector<std::uint8_t> bv_;  ///< the bool channel values (1/0)

    /** @brief The shared, immutable structural descriptor (bounds / grouping / adaption config) */
    std::shared_ptr<const GGenomeLayout> layout_ = std::make_shared<const GGenomeLayout>();

    /** @brief Transient (NOT serialized): set by load() when a results-only return arrived without the
     *  input parameters, so the server knows to graft them from the originally-submitted item. */
    bool input_omitted_ = false;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Genome */

/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::Genome::GFlatGenome) // NOLINT
/******************************************************************************/
