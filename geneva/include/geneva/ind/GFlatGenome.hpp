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
 * The flat-genome implementation of GOptimizableEntity. Where GTreeGenome stores a tree of
 * GParameterBase objects, GFlatGenome stores the parameters as four contiguous, type-homogeneous
 * value arrays (double / float / int32 / bool) plus a handle to a shared, immutable GGenomeLayout
 * that describes their bounds, grouping and adaption configuration. The per-individual, per-group
 * adaption state (Gauss sigma, ...) lives in the inherited GAuxiliaryStore, not in the genome.
 *
 * Because all genome state is generic, a typical concrete individual adds *no* extra members and only
 * supplies a constructor (which builds its genome with GGenomeBuilder and calls setGenome()) and
 * fitnessCalculation(). The clone/load/compare/serialize machinery is provided here and reused
 * unchanged via the GFlatIndividualT CRTP base.
 *
 * Value access is genome-agnostic: GFlatGenome implements the DM §2 per-type channel virtuals
 * declared on GOptimizableEntity (streamline_/assignValueVector_/countParameters*_/boundaries_), so
 * the optimization algorithms read and write parameter values identically to the tree, without a
 * downcast. Constrained values are folded into their external range at ADAPTION time (the adaption
 * kernels add to the value, then foldConstrainedValuesInPlace() folds it back into [lo, hi) before it
 * is stored), so the stored representation stays in range and equals the external value. Read access
 * (streamline) still folds defensively -- a no-op fast path for an in-range value -- so a value that
 * arrives out of range by some other path (e.g. a legacy deserialised genome) is still corrected.
 */
class GFlatGenome // NOLINT(cppcoreguidelines-special-member-functions)
  : public GOptimizableEntity {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /**
     * @brief Serialises the genome (the four value channels plus a by-value copy of the shared layout).
     *
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive to write the genome into
     * @param unsigned The (unused) serialization version number
     */
    template <typename Archive>
    void save(Archive &ar, const unsigned int) const {
        using boost::serialization::make_nvp;
        ar &make_nvp("GOptimizableEntity", boost::serialization::base_object<GOptimizableEntity>(*this));
        ar &BOOST_SERIALIZATION_NVP(dv_) &BOOST_SERIALIZATION_NVP(fv_) &
            BOOST_SERIALIZATION_NVP(iv_) &BOOST_SERIALIZATION_NVP(bv_);
        // The layout is shared & immutable in memory; for transport we serialise its *content* by
        // value (a deserialised genome legitimately owns its own layout copy -- sharing is only an
        // in-process optimisation). The transient per-group adaption state is NOT serialised here
        // (it is re-seeded on load); full-state checkpointing is a separate, later concern.
        //
        // TRANSPORT-SIZE OPTIMISATION OPPORTUNITY (deferred): the layout is IDENTICAL for
        // every individual in a population and never changes during a run, yet it is re-serialised with
        // EVERY work item -- for a large genome (~2 bounds + kind/active per parameter) this roughly
        // doubles the per-item wire payload and reconstructs the whole structure on every round-trip.
        // Interning it (send the layout ONCE per batch / cache it by id on the client+server and have
        // items reference it) would cut transport ~2x for large genomes. NOT a correctness issue, and
        // higher-value than the originally-noted zero-construction allocation (flat construction is
        // already cheap: a shared layout ptr + four contiguous value vectors). Cross-item caching is
        // needed because each item is serialised in its own archive, so Boost's intra-archive object
        // tracking does not span items.
        GGenomeLayout layout_copy = layout_ ? *layout_ : GGenomeLayout{};
        ar &make_nvp("layout_", layout_copy);
    }

    /**
     * @brief Restores the genome from an archive (value channels plus a freshly-owned layout copy).
     *
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive to read the genome from
     * @param unsigned The (unused) serialization version number
     */
    template <typename Archive>
    void load(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar &make_nvp("GOptimizableEntity", boost::serialization::base_object<GOptimizableEntity>(*this));
        ar &BOOST_SERIALIZATION_NVP(dv_) &BOOST_SERIALIZATION_NVP(fv_) &
            BOOST_SERIALIZATION_NVP(iv_) &BOOST_SERIALIZATION_NVP(bv_);
        auto fresh = std::make_shared<GGenomeLayout>();
        ar &make_nvp("layout_", *fresh);
        layout_ = fresh;
        // The per-group adaption state is OA-owned scratch (on the GIndividualSlot), no longer seeded
        // here: an optimization algorithm seeds each slot's scratch from its config at setup.
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GFlatGenome();
    /**
     * @brief Initialization with the number of fitness criteria.
     *
     * @param std::size_t The number of fitness criteria this genome will report
     */
    explicit GFlatGenome(std::size_t);
    /**
     * @brief The copy constructor.
     *
     * @param GFlatGenome The genome to copy from (value channels and shared layout handle)
     */
    GFlatGenome(GFlatGenome const &);
    /** @brief The destructor */
    ~GFlatGenome() override = default;

    /** @brief Un-hide the inherited public load(shared_ptr/unique_ptr/ref) overloads, which the
     *  boost split-member load(Archive&, unsigned) below would otherwise hide by name. */
    using GOptimizableEntity::load;

    /**
     * @brief Installs the value arrays + shared layout produced by a GGenomeBuilder.
     *
     * @param GenomeData The genome data (the four value channels plus the shared immutable layout)
     */
    void setGenome(GenomeData const &);

    /**
     * @brief Direct, shared access to the structural layout (problem metadata).
     *
     * @return A shared handle to the immutable layout describing bounds, grouping and adaption config
     */
    std::shared_ptr<const GGenomeLayout> getLayout() const noexcept { return layout_; }

    /***************************************************************************/
    // Mutable access to the raw INTERNAL value arrays. Adaption drifts the unbounded internal
    // representation in place (constrained values are folded into range only on read, via streamline) --
    // so the OA-owned adaption kernels must operate on these spans, NOT on the folded streamline view.
    // These are the seam the free-function adaption uses; ordinary value access still goes
    // through streamline()/assignValueVector().

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
     * @param pt::ptree The property tree to populate with this individual's parameters
     * @param std::string The base path / key prefix to write under (defaults to "parameterset")
     */
    void toPropertyTree(pt::ptree &, std::string const & = "parameterset") const override;

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
        bool = false // with_name_and_type
        ,
        bool = true // with_commas
        ,
        bool = true // use_raw_fitness
        ,
        bool = true // show_validity
    ) const override;

    /**
     * @brief Perform a cross-over operation between this object and another.
     *
     * @param GOptimizableEntity The other genome to cross over with (must be a GFlatGenome)
     * @return A new genome holding the recombined parameter values
     */
    std::shared_ptr<GOptimizableEntity> crossOverWith(GOptimizableEntity const &) const override;

    /**
     * @brief Retrieves parameters relevant for the evaluation from another GFlatGenome.
     *
     * @param GOptimizableEntity The genome whose evaluation-relevant parameters are absorbed into this one
     */
    void cannibalize(GOptimizableEntity &) override;

    /***************************************************************************/
    /** @brief Bulk-flatten fast path: streamlines an FP/int32 channel DIRECTLY into a caller-provided
     *  buffer (no temporary vector, no second copy), returning the number of values written. Produces
     *  exactly the same external (range-folded) values as streamline<T>() with the same activityMode --
     *  use it to flatten a whole population into one contiguous device buffer (the GPU marshallers).
     *  NB: a raw memcpy of the channel storage is NOT a valid substitute -- constrained values are kept
     *  in their unbounded internal form and only folded to range here.
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

    /** @brief Adaption-time fold: rewrites every CONSTRAINED stored value to its external (range-folded)
     *  representation, so the stored internal == external. Called by the OA adaption driver right after
     *  the adaption kernels run, so a constrained value never drifts unboundedly outside [lo, hi) between
     *  generations (unbounded "plain" parameters are left untouched). The bool channel is always valid
     *  (0/1), so it is not folded. */
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
     * @param GOptimizableEntity A base pointer to the GFlatGenome whose data is copied into this object
     */
    void load_(const GOptimizableEntity *) override;

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
     * @param GOptimizableEntity The other object to compare this one against
     * @param expectation The expected relation (equality / inequality) between the two objects
     * @param double The maximum allowed deviation for floating-point comparisons (the limit)
     */
    void compare_(
        GOptimizableEntity const &,
        Gem::Common::expectation const &,
        double const &
    ) const override;

    /**
     * @brief Random initialization of the genome's parameter values.
     *
     * @param activityMode The activity mode selecting which parameters are randomly initialised
     * @return true if at least one parameter value was changed, false otherwise
     */
    bool randomInit_(activityMode const &) override;

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
     *  @param std::size_t The size of the value vector to pick a cross-over position in
     *  @param std::size_t The minimum allowed cross-over position
     *  @return A valid cross-over position within the vector */
    std::size_t getCrossOverPos(std::size_t, std::size_t);

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

    /** @brief THE single source of truth for the per-element constrained-value fold: maps a stored value
     *  to its external (range-folded) representation -- identity for an unbounded parameter,
     *  foldConstrainedFP / foldConstrainedInt for a constrained one. BOTH the read-time fold
     *  (streamlineImpl / streamlineIntoImpl) AND the adaption-time fold (foldChannelInPlace) go through
     *  this one function, so the two paths can never diverge -- any change to the fold belongs here (or,
     *  for the maths, in foldConstrainedFP / foldConstrainedInt).
     *
     *  @tparam T The channel's value type (double / float / int32)
     *  @param ch The channel layout describing the parameter's kind and bounds
     *  @param stored The internally stored (possibly unbounded) value
     *  @param k The index of the parameter within the channel
     *  @return The external, range-folded representation of the stored value */
    template <typename T>
    static T externalValue(ChannelLayout<T> const &ch, T stored, std::size_t k) {
        if(ch.kind[k] != ParamKind::Constrained) {
            return stored;
        }
        if constexpr(std::is_floating_point_v<T>) {
            return foldConstrainedFP<T>(stored, ch.lower[k], ch.upper[k]);
        }
        else {
            return foldConstrainedInt<T>(stored, ch.lower[k], ch.upper[k]);
        }
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

    /** @brief Writes the active channel's external (range-folded) values DIRECTLY into a caller buffer,
     *  returning the count written. Same per-element fold as streamlineImpl() (both via externalValue()),
     *  but no temporary vector and no second copy -- the bulk-flatten fast path behind streamlineInto().
     *  Since values are folded at adaption time the fold here is the in-range fast path (~a copy), but it
     *  is retained defensively, so a raw memcpy of `store` is not a guaranteed-correct substitute.
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

    /** @brief Adaption-time fold: normalises every value of a channel to its external (range-folded)
     *  representation IN THE STORE, via the SAME externalValue() the read path uses (identity for
     *  unbounded parameters, so only constrained values move back into [lo, hi)). After this the stored
     *  internal == external. Sharing externalValue() is what keeps this in lock-step with read-time folds.
     *
     *  @tparam T The channel's value type (double / float / int32)
     *  @param ch The channel layout (kind / bounds)
     *  @param store The internal value storage, rewritten in place to its external representation */
    template <typename T>
    static void foldChannelInPlace(ChannelLayout<T> const &ch, std::vector<T> &store) {
        for(std::size_t k = 0; k < store.size(); ++k) {
            store[k] = externalValue<T>(ch, store[k], k);
        }
    }

    /** @brief Writes a vector of external values back into the active slots of a channel's storage.
     *  @tparam T The channel's value type (double / float / int32)
     *  @param in The incoming values, consumed in order for each active parameter
     *  @param ch The channel layout (active flags)
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
                store[k] = in.at(pos++);
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
            if(ch.kind[k] == ParamKind::Constrained) {
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
                bv_[k] = in.at(pos++) ? std::uint8_t(1) : std::uint8_t(0);
            }
        }
    }

    /***************************************************************************/
    // Random-init channel helpers. (The per-channel ADAPTION kernels are no longer driven from the
    // individual: the adaption state and logic are OA-owned -- see geneva/oa/GAdaption.hpp, fed by the
    // GIndividualSlot's scratch.)

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
    // Data: the four contiguous value channels + the shared structural layout.

    std::vector<double> dv_;        ///< the double channel values (internal representation)
    std::vector<float> fv_;         ///< the float channel values (internal representation)
    std::vector<std::int32_t> iv_;  ///< the int32 channel values (internal representation)
    std::vector<std::uint8_t> bv_;  ///< the bool channel values (1/0)

    /** @brief The shared, immutable structural descriptor (bounds / grouping / adaption config) */
    std::shared_ptr<const GGenomeLayout> layout_ = std::make_shared<const GGenomeLayout>();
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
