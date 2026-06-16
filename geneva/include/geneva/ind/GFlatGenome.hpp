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

namespace Gem::Geneva::Parameters {

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
 * downcast. Constrained values are stored in their unbounded *internal* representation and folded
 * into their external range on read (mirroring GConstrainedFPT / GConstrainedIntT exactly).
 */
class GFlatGenome // NOLINT(cppcoreguidelines-special-member-functions)
  : public GOptimizableEntity {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

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
        GGenomeLayout layout_copy = layout_ ? *layout_ : GGenomeLayout{};
        ar &make_nvp("layout_", layout_copy);
    }

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
    /** @brief Initialization with the number of fitness criteria */
    explicit GFlatGenome(std::size_t);
    /** @brief The copy constructor */
    GFlatGenome(GFlatGenome const &);
    /** @brief The destructor */
    ~GFlatGenome() override = default;

    /** @brief Un-hide the inherited public load(shared_ptr/unique_ptr/ref) overloads, which the
     *  boost split-member load(Archive&, unsigned) below would otherwise hide by name. */
    using GOptimizableEntity::load;

    /** @brief Installs the value arrays + shared layout produced by a GGenomeBuilder */
    void setGenome(Genome const &);

    /** @brief Direct, shared access to the structural layout (problem metadata) */
    std::shared_ptr<const GGenomeLayout> getLayout() const { return layout_; }

    /***************************************************************************/
    // Mutable access to the raw INTERNAL value arrays. Adaption drifts the unbounded internal
    // representation in place (constrained values are folded into range only on read, via streamline) --
    // so the OA-owned adaption kernels must operate on these spans, NOT on the folded streamline view.
    // These are the seam the Phase-8 free-function adaption uses; ordinary value access still goes
    // through streamline()/assignValueVector().

    std::span<double> internalDoubleValues() { return {dv_.data(), dv_.size()}; }
    std::span<float> internalFloatValues() { return {fv_.data(), fv_.size()}; }
    std::span<std::int32_t> internalInt32Values() { return {iv_.data(), iv_.size()}; }
    std::span<std::uint8_t> internalBoolValues() { return {bv_.data(), bv_.size()}; }

    std::span<const double> internalDoubleValues() const { return {dv_.data(), dv_.size()}; }
    std::span<const float> internalFloatValues() const { return {fv_.data(), fv_.size()}; }
    std::span<const std::int32_t> internalInt32Values() const { return {iv_.data(), iv_.size()}; }
    std::span<const std::uint8_t> internalBoolValues() const { return {bv_.data(), bv_.size()}; }

    /** @brief Transformation of the individual's parameters into a boost::property_tree object */
    void toPropertyTree(pt::ptree &, std::string const & = "parameterset") const override;

    /** @brief Transformation of the individual's parameters into a list of comma-separated values */
    std::string toCSV(
        bool = false // with_name_and_type
        ,
        bool = true // with_commas
        ,
        bool = true // use_raw_fitness
        ,
        bool = true // show_validity
    ) const override;

    /** @brief Perform a cross-over operation between this object and another */
    std::shared_ptr<GOptimizableEntity> crossOverWith(GOptimizableEntity const &) const override;

    /** @brief Retrieves parameters relevant for the evaluation from another GFlatGenome */
    void cannibalize(GOptimizableEntity &) override;

    /***************************************************************************/
    /** @brief Bulk-flatten fast path: streamlines an FP/int32 channel DIRECTLY into a caller-provided
     *  buffer (no temporary vector, no second copy), returning the number of values written. Produces
     *  exactly the same external (range-folded) values as streamline<T>() with the same activityMode --
     *  use it to flatten a whole population into one contiguous device buffer (the GPU marshallers).
     *  NB: a raw memcpy of the channel storage is NOT a valid substitute -- constrained values are kept
     *  in their unbounded internal form and only folded to range here. */
    std::size_t streamlineInto(double *dst, activityMode const &am = activityMode::DEFAULTACTIVITYMODE) const {
        return streamlineIntoImpl<double>(dst, layout_->d, dv_, am);
    }
    std::size_t streamlineInto(float *dst, activityMode const &am = activityMode::DEFAULTACTIVITYMODE) const {
        return streamlineIntoImpl<float>(dst, layout_->f, fv_, am);
    }
    std::size_t streamlineInto(std::int32_t *dst, activityMode const &am = activityMode::DEFAULTACTIVITYMODE) const {
        return streamlineIntoImpl<std::int32_t>(dst, layout_->i, iv_, am);
    }

    /***************************************************************************/
    // Deleted functions

    explicit GFlatGenome(float const &) = delete;  ///< Intentionally undefined
    explicit GFlatGenome(double const &) = delete; ///< Intentionally undefined

protected:
    /** @brief Loads the data of another GFlatGenome, camouflaged as a base pointer */
    void load_(const GOptimizableEntity *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GFlatGenome>(
        GFlatGenome const &,
        GFlatGenome const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        GOptimizableEntity const &,
        Gem::Common::expectation const &,
        double const &
    ) const override;

    /** @brief Random initialization */
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

    /** @brief Emits a name for this class / object */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object (supplied by the concrete individual) */
    GFlatGenome *clone_() const override = 0;

    /** @brief Retrieve the active parameter at the given index, per type (positional) */
    double getVarVal_d_(std::size_t idx) override;
    float getVarVal_f_(std::size_t idx) override;
    std::int32_t getVarVal_i_(std::size_t idx) override;
    bool getVarVal_b_(std::size_t idx) override;

    /** @brief Retrieval of a suitable position for cross over inside of a vector */
    std::size_t getCrossOverPos(std::size_t, std::size_t);

    /***************************************************************************/
    // Activity / fold helpers (genome-agnostic value mapping).

    /** @brief Whether a value with the given active flag matches the requested activity mode */
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

    /** @brief The external (range-folded) value of a stored FP/int value */
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
     *  returning the count written. Identical per-element fold to streamlineImpl(), but no temporary
     *  vector and no second copy -- the bulk-flatten fast path behind the public streamlineInto(). NB a
     *  raw memcpy of `store` would be WRONG: constrained values are stored unbounded and only folded to
     *  their external range here, so the device must receive the folded values, not the raw storage. */
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

    void streamlineBool(std::vector<bool> &out, activityMode const &am) const {
        out.clear();
        for(std::size_t k = 0; k < bv_.size(); ++k) {
            if(amMatch(layout_->b.active[k], am)) {
                out.push_back(bv_[k] != 0);
            }
        }
    }
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

    template <typename T>
    bool randomInitFP(std::vector<T> &store, ChannelLayout<T> const &ch, activityMode const &am);
    bool randomInitInt(activityMode const &am);
    bool randomInitBool(activityMode const &am);

    /***************************************************************************/
    // The DM §2 per-type virtual overrides (forward to the templated helpers above).

    void streamline_(std::vector<double> &v, activityMode const &am) const override { streamlineImpl<double>(v, layout_->d, dv_, am); }
    void streamline_(std::vector<float> &v, activityMode const &am) const override { streamlineImpl<float>(v, layout_->f, fv_, am); }
    void streamline_(std::vector<std::int32_t> &v, activityMode const &am) const override { streamlineImpl<std::int32_t>(v, layout_->i, iv_, am); }
    void streamline_(std::vector<bool> &v, activityMode const &am) const override { streamlineBool(v, am); }

    void assignValueVector_(std::vector<double> const &v, activityMode const &am) override { assignImpl<double>(v, layout_->d, dv_, am); }
    void assignValueVector_(std::vector<float> const &v, activityMode const &am) override { assignImpl<float>(v, layout_->f, fv_, am); }
    void assignValueVector_(std::vector<std::int32_t> const &v, activityMode const &am) override { assignImpl<std::int32_t>(v, layout_->i, iv_, am); }
    void assignValueVector_(std::vector<bool> const &v, activityMode const &am) override { assignBool(v, am); }

    std::size_t countParametersDouble_(activityMode const &am) const override { return countImpl<double>(layout_->d, am); }
    std::size_t countParametersFloat_(activityMode const &am) const override { return countImpl<float>(layout_->f, am); }
    std::size_t countParametersInt32_(activityMode const &am) const override { return countImpl<std::int32_t>(layout_->i, am); }
    std::size_t countParametersBool_(activityMode const &am) const override { return countImpl<bool>(layout_->b, am); }

    void boundaries_(std::vector<double> &l, std::vector<double> &u, activityMode const &am) const override { boundariesImpl<double>(l, u, layout_->d, am); }
    void boundaries_(std::vector<float> &l, std::vector<float> &u, activityMode const &am) const override { boundariesImpl<float>(l, u, layout_->f, am); }
    void boundaries_(std::vector<std::int32_t> &l, std::vector<std::int32_t> &u, activityMode const &am) const override { boundariesImpl<std::int32_t>(l, u, layout_->i, am); }
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

} /* namespace Gem::Geneva::Parameters */

/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::Parameters::GFlatGenome) // NOLINT
/******************************************************************************/
