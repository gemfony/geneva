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
#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>
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
#include "geneva/ind/GCandidateSolution.hpp"
#include "geneva/ind/GGenomeLayout.hpp"
#include "geneva/ind/GGenomeLayoutSerialization.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"

// aliases for ease of use
namespace pt = boost::property_tree;

namespace Gem::Geneva::Genome {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The flat-genome representation layer of the individual hierarchy and the non-template concrete cast /
 * signature target the optimization algorithms operate on.
 *
 * GFlatGenomeBase derives the genome-agnostic GCandidateSolution and adds the parameter STORAGE: four
 * contiguous, type-homogeneous value arrays (double / float / int32 / bool) plus a handle to a shared,
 * immutable GGenomeLayout that describes their bounds, grouping and adaption configuration. The
 * per-individual, per-group adaption state (Gauss sigma, ...) lives in the OA-owned slot scratch, not in
 * the genome. Because all genome state is generic, a concrete individual adds no extra members and only
 * supplies a constructor (building its genome with GGenomeBuilder + setGenome()) and fitnessCalculation();
 * the clone/load/compare/serialize machinery is provided here and reused unchanged via GFlatGenomeT.
 *
 * Value access is CONCRETE here (no virtual indirection): GFlatGenomeBase owns the value arrays, so the
 * algorithms read and write parameter values through the templated surface (streamline<T>() /
 * assignValueVector<T>() / countParameters<T>() / boundaries<T>() and the FP / internal views) directly
 * on this type. A floating-point parameter is stored in the NORMALIZED INTERNAL coordinate (magnitude
 * ~ 1, confined to [-0.5, 0.5) for a bounded parameter); the user-facing EXTERNAL value is its affine
 * image (normalized-genome architecture, §2). This realizes the two-reader split: the OAs read/write the
 * raw internal value (streamlineInternal_/assignValueVectorInternal_), while the objective function, GPU
 * marshaller and user inspection read the scaled external value. The fold lives at WRITE time, not read
 * time. Integer parameters are not normalized (§2.7): they keep the closed-range integer fold on read.
 *
 * Feasibility checking lives here (not on the genome-agnostic assembly) because a constraint reads
 * parameter values: checkFeasibility_() forwards to the shared GProblemPolicy, passing this genome.
 */
class GFlatGenomeBase // NOLINT(cppcoreguidelines-special-member-functions)
  : public GCandidateSolution {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /** @brief Single declaration of this class's plain local data members (the four value channels),
     *  feeding save()/load()/load_()/compare_() from one source. The shared layout_ and the transient
     *  input_omitted_ are handled separately (the layout is interned on the wire and shared, not
     *  value-copied; input_omitted_ is a load-only transient), so they are NOT listed here.
     *  @tparam Self The (const or non-const) deduced type of *this
     *  @param self A reference to *this whose members are tied into the tuple
     *  @return A tuple of named member references */
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
     * @brief Serialises the genome: the four value channels plus the shared structural layout, the latter
     * either by value (self-contained form) or by content id (transport send-once form), as selected by
     * the active wire-serialisation scope. See the body for the two forms.
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive to write the genome into
     * @param version The (unused) serialization version number
     */
    template <typename Archive>
    void save(Archive &ar, [[maybe_unused]] const unsigned int version) const {
        using boost::serialization::make_nvp;
        ar &make_nvp("GCandidateSolution", boost::serialization::base_object<GCandidateSolution>(*this));

        const auto *ctx = Gem::Courtier::GWireSerializationScope::current();

        // RESULTS-ONLY RETURN: when a worker returns a processed item (ctx->returning) and the genome was
        // not modified, omit the (potentially large) input parameters + layout -- only the computed
        // results (already written via the base) travel; the server grafts the input back on. A leading
        // `genome_omitted` marker makes the stream self-describing.
        const bool genome_omitted =
            (ctx != nullptr) && ctx->enabled && ctx->returning && not this->getReturnFullIndividual();
        ar &make_nvp("genome_omitted", genome_omitted);
        if(genome_omitted) {
            return;
        }

        Gem::Common::serialize_members(ar, localMembers_(*this));

        // The layout is shared & immutable in memory; sharing does not survive serialisation. Two wire
        // forms (see GFlatGenome's historical note): SELF-CONTAINED (full layout by value, the only form
        // with no active scope -- checkpoint / file) and SEND-ONCE (referenced by content id, shipped to a
        // peer only the first time the id is seen). A leading `layout_interned` tag is self-describing.
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
        if(not ctx->registry->has(wid)) {
            ctx->registry->put(wid, layoutToWireBlob(*layout_));
        }
        bool layout_present = not ctx->registry->peerHasLayout(ctx->peer, wid);
        ar &make_nvp("layout_present", layout_present);
        if(layout_present) {
            GGenomeLayout layout_copy = *layout_;
            ar &make_nvp("layout_", layout_copy);
            ctx->registry->markPeerHasLayout(ctx->peer, wid);
        }
    }

    /**
     * @brief Restores the genome from an archive: the value channels plus a freshly-owned layout, read
     * either by value or resolved from a content id against the wire registry (with a cache-miss fetch),
     * following the self-describing tag written by save().
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive to read the genome from
     * @param version The (unused) serialization version number
     */
    template <typename Archive>
    void load(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;
        ar &make_nvp("GCandidateSolution", boost::serialization::base_object<GCandidateSolution>(*this));

        bool genome_omitted = false;
        ar &make_nvp("genome_omitted", genome_omitted);
        if(genome_omitted) {
            input_omitted_ = true;
            dv_.clear();
            fv_.clear();
            iv_.clear();
            bv_.clear();
            this->setLayout(std::make_shared<const GGenomeLayout>());
            return;
        }
        input_omitted_ = false;

        Gem::Common::serialize_members(ar, localMembers_(*this));

        bool interned = false;
        ar &make_nvp("layout_interned", interned);
        if(not interned) {
            auto fresh = std::make_shared<GGenomeLayout>();
            ar &make_nvp("layout_", *fresh);
            this->setLayout(fresh);
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
            this->setLayout(fresh);
            if(ctx != nullptr && ctx->registry != nullptr && not ctx->registry->has(wid)) {
                ctx->registry->put(wid, layoutToWireBlob(*fresh));
            }
            return;
        }

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
                << "In GFlatGenomeBase::load(): Error!" << '\n'
                << "An id-only layout reference could not be resolved (cache miss with no usable fetch)."
                << '\n'
            );
        }
        this->setLayout(layoutFromWireBlob(blob));
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GFlatGenomeBase();
    /** @brief Initialization with the number of fitness criteria.
     *  @param n_fitness_criteria The number of fitness criteria this genome will report */
    explicit GFlatGenomeBase(std::size_t n_fitness_criteria);
    /** @brief The copy constructor.
     *  @param cp The genome to copy from (value channels and shared layout handle) */
    GFlatGenomeBase(GFlatGenomeBase const &cp);
    /** @brief The destructor */
    ~GFlatGenomeBase() override = default;

    /** @brief Un-hide the inherited public load(shared_ptr/unique_ptr/ref) overloads, which the boost
     *  split-member load(Archive&, unsigned) below would otherwise hide by name. */
    using GCandidateSolution::load;

    /** @brief Installs the value arrays + shared layout produced by a GGenomeBuilder.
     *  @param g The genome data (the four value channels plus the shared immutable layout) */
    void setGenome(GenomeData const &g);

    /** @brief Direct, shared access to the structural layout (problem metadata).
     *  @return A shared handle to the immutable layout describing bounds, grouping and adaption config */
    std::shared_ptr<const GGenomeLayout> getLayout() const noexcept { return layout_; }

    /***************************************************************************/
    // Genome value channels (concrete -- GFlatGenomeBase owns the value arrays). The per-type templates
    // are the ergonomic surface the optimization algorithms use; each forwards to a templated helper.

    /** @brief Streamlines all parameters of type par_type into a vector (cleared first).
     *  @tparam par_type The parameter value type (double, float, std::int32_t or bool)
     *  @param par_vec The vector the parameter values are written into (cleared first)
     *  @param am The activity mode controlling which parameters are included */
    template <typename par_type>
    void streamline(
        std::vector<par_type> &par_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) const {
        if constexpr(std::is_same_v<par_type, double>) { streamlineImpl<double>(par_vec, layout_->d, dv_, am); }
        else if constexpr(std::is_same_v<par_type, float>) { streamlineImpl<float>(par_vec, layout_->f, fv_, am); }
        else if constexpr(std::is_same_v<par_type, std::int32_t>) { streamlineImpl<std::int32_t>(par_vec, layout_->i, iv_, am); }
        else if constexpr(std::is_same_v<par_type, bool>) { streamlineBool(par_vec, am); }
        else { static_assert(sizeof(par_type) == 0, "streamline: unsupported parameter type"); }
    }

    /** @brief Assigns values from a vector to the parameters of type par_type (marks the item dirty).
     *  @tparam par_type The parameter value type (double, float, std::int32_t or bool)
     *  @param par_vec The vector of values to scatter onto the matching parameters
     *  @param am The activity mode controlling which parameters are written */
    template <typename par_type>
    void assignValueVector(
        std::vector<par_type> const &par_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) {
#ifdef DEBUG
        if(countParameters<par_type>(am) != par_vec.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GFlatGenomeBase::assignValueVector():" << '\n'
                << "Sizes don't match: " << countParameters<par_type>(am) << " / " << par_vec.size() << '\n'
            );
        }
#endif /* DEBUG */
        if constexpr(std::is_same_v<par_type, double>) { assignImpl<double>(par_vec, layout_->d, dv_, am); }
        else if constexpr(std::is_same_v<par_type, float>) { assignImpl<float>(par_vec, layout_->f, fv_, am); }
        else if constexpr(std::is_same_v<par_type, std::int32_t>) { assignImpl<std::int32_t>(par_vec, layout_->i, iv_, am); }
        else if constexpr(std::is_same_v<par_type, bool>) { assignBool(par_vec, am); }
        else { static_assert(sizeof(par_type) == 0, "assignValueVector: unsupported parameter type"); }
        this->mark_as_due_for_processing();
    }

    /** @brief The number of parameters of type par_type (served from the layout-keyed cache).
     *  @tparam par_type The parameter value type (double, float, std::int32_t or bool)
     *  @param am The activity mode controlling which parameters are counted
     *  @return The number of parameters of the requested type */
    template <typename par_type>
    std::size_t countParameters(activityMode const &am = activityMode::DEFAULTACTIVITYMODE) const {
        if constexpr(std::is_same_v<par_type, double>) { return cachedCount(0, am); }
        else if constexpr(std::is_same_v<par_type, float>) { return cachedCount(1, am); }
        else if constexpr(std::is_same_v<par_type, std::int32_t>) { return cachedCount(2, am); }
        else if constexpr(std::is_same_v<par_type, bool>) { return cachedCount(3, am); }
        else { static_assert(sizeof(par_type) == 0, "countParameters: unsupported parameter type"); return 0; }
    }

    /** @brief Lower/upper boundaries of all parameters of type par_type (cleared first).
     *  @tparam par_type The parameter value type (double, float, std::int32_t or bool)
     *  @param l_bnd_vec The vector the lower boundaries are written into (cleared first)
     *  @param u_bnd_vec The vector the upper boundaries are written into (cleared first)
     *  @param am The activity mode controlling which parameters are included */
    template <typename par_type>
    void boundaries(
        std::vector<par_type> &l_bnd_vec,
        std::vector<par_type> &u_bnd_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) const {
        if constexpr(std::is_same_v<par_type, double>) { boundariesImpl<double>(l_bnd_vec, u_bnd_vec, layout_->d, am); }
        else if constexpr(std::is_same_v<par_type, float>) { boundariesImpl<float>(l_bnd_vec, u_bnd_vec, layout_->f, am); }
        else if constexpr(std::is_same_v<par_type, std::int32_t>) { boundariesImpl<std::int32_t>(l_bnd_vec, u_bnd_vec, layout_->i, am); }
        else if constexpr(std::is_same_v<par_type, bool>) { boundariesBool(l_bnd_vec, u_bnd_vec, am); }
        else { static_assert(sizeof(par_type) == 0, "boundaries: unsupported parameter type"); }
    }

    /***************************************************************************/
    // The precision-agnostic floating point view (double + float widened to double).

    /** @brief @param am The activity mode. @return The combined count of double and float parameters */
    std::size_t countFPParameters(activityMode const &am = activityMode::DEFAULTACTIVITYMODE) const {
        return countParameters<double>(am) + countParameters<float>(am);
    }

    /** @brief Streamlines all FP parameters into a single double vector (double-typed first, then widened float).
     *  @param par_vec The vector the floating point values are written into (cleared first)
     *  @param am The activity mode controlling which parameters are included */
    void streamlineFP(
        std::vector<double> &par_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) const {
        par_vec.clear();
        this->streamline<double>(par_vec, am);
        std::vector<float> float_vec;
        this->streamline<float>(float_vec, am);
        par_vec.reserve(par_vec.size() + float_vec.size());
        for(float f : float_vec) {
            par_vec.push_back(static_cast<double>(f));
        }
    }

    /** @brief Scatters a double vector produced by streamlineFP() back onto the FP parameters.
     *  @param par_vec The combined double vector (double-typed first, then float-typed) to scatter back
     *  @param am The activity mode controlling which parameters are written */
    void assignFPValueVector(
        std::vector<double> const &par_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) {
        const std::size_t n_double = countParameters<double>(am);
        const std::size_t n_float = countParameters<float>(am);
#ifdef DEBUG
        if(n_double + n_float != par_vec.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GFlatGenomeBase::assignFPValueVector():" << '\n'
                << "Sizes don't match: " << (n_double + n_float) << " / " << par_vec.size() << '\n'
            );
        }
#endif /* DEBUG */
        if(n_double > 0) {
            std::vector<double> double_vec(par_vec.begin(), par_vec.begin() + static_cast<std::ptrdiff_t>(n_double));
            this->assignValueVector<double>(double_vec, am);
        }
        if(n_float > 0) {
            std::vector<float> float_vec(n_float);
            for(std::size_t i = 0; i < n_float; ++i) {
                float_vec[i] = static_cast<float>(par_vec[n_double + i]);
            }
            this->assignValueVector<float>(float_vec, am);
        }
    }

    /** @brief Lower/upper boundaries of all FP parameters (matching streamlineFP() ordering).
     *  @param l_bnd_vec The vector the lower boundaries are written into (cleared first)
     *  @param u_bnd_vec The vector the upper boundaries are written into (cleared first)
     *  @param am The activity mode controlling which parameters are included */
    void boundariesFP(
        std::vector<double> &l_bnd_vec,
        std::vector<double> &u_bnd_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) const {
        std::vector<double> l_double;
        std::vector<double> u_double;
        this->boundaries<double>(l_double, u_double, am);
        std::vector<float> l_float;
        std::vector<float> u_float;
        this->boundaries<float>(l_float, u_float, am);
        l_bnd_vec.clear();
        u_bnd_vec.clear();
        l_bnd_vec.reserve(l_double.size() + l_float.size());
        u_bnd_vec.reserve(u_double.size() + u_float.size());
        l_bnd_vec.insert(l_bnd_vec.end(), l_double.begin(), l_double.end());
        u_bnd_vec.insert(u_bnd_vec.end(), u_double.begin(), u_double.end());
        for(float v : l_float) { l_bnd_vec.push_back(static_cast<double>(v)); }
        for(float v : u_float) { u_bnd_vec.push_back(static_cast<double>(v)); }
    }

    /***************************************************************************/
    // The INTERNAL (normalized) floating-point view (§2.3): the OA-facing half of the two-reader split.

    /** @brief Streamlines all FP parameters in their raw INTERNAL representation into a double vector.
     *  @param par_vec The vector the internal values are written into (cleared first)
     *  @param am The activity mode controlling which parameters are included */
    void streamlineFPInternal(
        std::vector<double> &par_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) const {
        par_vec.clear();
        streamlineInternalImpl<double>(par_vec, layout_->d, dv_, am);
        std::vector<float> float_vec;
        streamlineInternalImpl<float>(float_vec, layout_->f, fv_, am);
        par_vec.reserve(par_vec.size() + float_vec.size());
        for(float f : float_vec) { par_vec.push_back(static_cast<double>(f)); }
    }

    /** @brief Scatters a vector of raw INTERNAL values back onto the FP parameters (folds bounded values).
     *  @param par_vec The combined internal-value vector to scatter back
     *  @param am The activity mode controlling which parameters are written */
    void assignFPValueVectorInternal(
        std::vector<double> const &par_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) {
        const std::size_t n_double = countParameters<double>(am);
        const std::size_t n_float = countParameters<float>(am);
#ifdef DEBUG
        if(n_double + n_float != par_vec.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GFlatGenomeBase::assignFPValueVectorInternal():" << '\n'
                << "Sizes don't match: " << (n_double + n_float) << " / " << par_vec.size() << '\n'
            );
        }
#endif /* DEBUG */
        if(n_double > 0) {
            std::vector<double> double_vec(par_vec.begin(), par_vec.begin() + static_cast<std::ptrdiff_t>(n_double));
            assignInternalImpl<double>(double_vec, layout_->d, dv_, am);
        }
        if(n_float > 0) {
            std::vector<float> float_vec(n_float);
            for(std::size_t i = 0; i < n_float; ++i) {
                float_vec[i] = static_cast<float>(par_vec[n_double + i]);
            }
            assignInternalImpl<float>(float_vec, layout_->f, fv_, am);
        }
        this->mark_as_due_for_processing();
    }

    /***************************************************************************/
    /** @brief Retrieves a parameter of a given type at the specified active index.
     *  @tparam val_type The value type to retrieve (double, float, std::int32_t or bool)
     *  @param target A (type-index, name, position) tuple; the third element is the active index
     *  @return The parameter value of the requested type at the requested index */
    template <typename val_type>
    val_type getVarVal(std::tuple<std::size_t, std::string, std::size_t> const &target) {
        static_assert(
            std::is_same_v<val_type, double> || std::is_same_v<val_type, float> ||
                std::is_same_v<val_type, std::int32_t> || std::is_same_v<val_type, bool>,
            "GFlatGenomeBase::getVarVal<>(): unsupported value type (use double, float, std::int32_t or bool)"
        );
        const std::size_t idx = std::get<2>(target);
        if constexpr(std::is_same_v<val_type, double>) { return getVarVal_d_(idx); }
        else if constexpr(std::is_same_v<val_type, float>) { return getVarVal_f_(idx); }
        else if constexpr(std::is_same_v<val_type, std::int32_t>) { return getVarVal_i_(idx); }
        else { return getVarVal_b_(idx); }
    }

    /***************************************************************************/
    // Mutable access to the raw INTERNAL (normalized) value arrays (the OA-owned adaption kernels write here).

    /** @brief @return A mutable span over the raw internal double channel */
    std::span<double> internalDoubleValues() noexcept { return {dv_.data(), dv_.size()}; }
    /** @brief @return A mutable span over the raw internal float channel */
    std::span<float> internalFloatValues() noexcept { return {fv_.data(), fv_.size()}; }
    /** @brief @return A mutable span over the raw internal int32 channel */
    std::span<std::int32_t> internalInt32Values() noexcept { return {iv_.data(), iv_.size()}; }
    /** @brief @return A mutable span over the raw internal bool channel (bytes, 1/0) */
    std::span<std::uint8_t> internalBoolValues() noexcept { return {bv_.data(), bv_.size()}; }

    /** @brief @return A read-only span over the raw internal double channel */
    std::span<const double> internalDoubleValues() const noexcept { return {dv_.data(), dv_.size()}; }
    /** @brief @return A read-only span over the raw internal float channel */
    std::span<const float> internalFloatValues() const noexcept { return {fv_.data(), fv_.size()}; }
    /** @brief @return A read-only span over the raw internal int32 channel */
    std::span<const std::int32_t> internalInt32Values() const noexcept { return {iv_.data(), iv_.size()}; }
    /** @brief @return A read-only span over the raw internal bool channel (bytes, 1/0) */
    std::span<const std::uint8_t> internalBoolValues() const noexcept { return {bv_.data(), bv_.size()}; }

    /***************************************************************************/
    // Bulk-flatten fast path (the GPU marshallers) -- see GFlatGenome's historical notes.

    /** @brief Bulk-flatten the double channel directly into a caller buffer (external/scaled values).
     *  @param dst The destination buffer (must be large enough)
     *  @param am The activity mode selecting which parameters are written
     *  @return The number of values written into dst */
    std::size_t streamlineInto(double *dst, activityMode const &am = activityMode::DEFAULTACTIVITYMODE) const {
        return streamlineIntoImpl<double>(dst, layout_->d, dv_, am);
    }
    /** @brief Bulk-flatten the float channel (see the double overload).
     *  @param dst The destination buffer. @param am The activity mode. @return The number written */
    std::size_t streamlineInto(float *dst, activityMode const &am = activityMode::DEFAULTACTIVITYMODE) const {
        return streamlineIntoImpl<float>(dst, layout_->f, fv_, am);
    }
    /** @brief Bulk-flatten the int32 channel (see the double overload).
     *  @param dst The destination buffer. @param am The activity mode. @return The number written */
    std::size_t streamlineInto(std::int32_t *dst, activityMode const &am = activityMode::DEFAULTACTIVITYMODE) const {
        return streamlineIntoImpl<std::int32_t>(dst, layout_->i, iv_, am);
    }

    /** @brief Adaption-time write-fold (§2.2): folds every bounded value back into range (FP + int32). */
    void foldConstrainedValuesInPlace() {
        foldChannelInPlace<double>(layout_->d, dv_);
        foldChannelInPlace<float>(layout_->f, fv_);
        foldChannelInPlace<std::int32_t>(layout_->i, iv_);
    }

    /***************************************************************************/
    // Algorithm-facing genome operations (concrete on the flat genome).

    /** @brief Transformation of the individual's parameters into a boost::property_tree object.
     *  @param ptr The property tree to populate. @param base_name The base path / key prefix */
    void toPropertyTree(pt::ptree &ptr, std::string const &base_name = "parameterset") const;

    /** @brief Transformation of the individual's parameters into a list of comma-separated values.
     *  @param with_name_and_type Whether to prefix each value with its name and type
     *  @param with_commas Whether to separate the values with commas
     *  @param use_raw_fitness Whether to emit the raw rather than the transformed fitness
     *  @param show_validity Whether to append the individual's validity flag
     *  @return The parameters (and optionally fitness/validity) as a single CSV string */
    std::string toCSV(
        bool with_name_and_type = false,
        bool with_commas = true,
        bool use_raw_fitness = true,
        bool show_validity = true
    ) const;

    /** @brief Perform a cross-over operation between this genome and another.
     *  @param cp_base The other genome to cross over with (must be a GFlatGenomeBase)
     *  @return A new genome holding the recombined parameter values */
    std::shared_ptr<GCandidateSolution> crossOverWith(GFlatGenomeBase const &cp_base) const;

    /** @brief Retrieves parameters relevant for the evaluation from another GFlatGenomeBase.
     *  @param cp_base The genome whose evaluation-relevant parameters are absorbed into this one */
    void cannibalize(GFlatGenomeBase &cp_base);

    /** @brief Whether a full return was requested for this individual (transient transport hint).
     *  @return true if the full individual should be returned; false for the results-only form */
    bool getReturnFullIndividual() const { return return_full_individual_; }
    /** @brief Requests that this individual be returned to the server in FULL (input parameters included).
     *  @param full true to force a full return; false for the lightweight results-only form */
    void setReturnFullIndividual(bool full) { return_full_individual_ = full; }

    /***************************************************************************/
    // Deleted functions

    explicit GFlatGenomeBase(float const &) = delete;  ///< Intentionally undefined
    explicit GFlatGenomeBase(double const &) = delete; ///< Intentionally undefined

protected:
    /** @brief Loads the data of another GFlatGenomeBase, camouflaged as a base pointer.
     *  @param cp A base pointer to the GFlatGenomeBase whose data is copied into this object */
    void load_(const GCandidateSolution *cp) override;

    /** @brief Allow access to this class's compare_ function. */
    friend void Gem::Common::compare_base_t<GFlatGenomeBase>(
        GFlatGenomeBase const &,
        GFlatGenomeBase const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object.
     *  @param cp The other object to compare this one against
     *  @param e The expected relation (equality / inequality)
     *  @param limit The maximum allowed deviation for floating-point comparisons */
    void compare_(
        GCandidateSolution const &cp,
        Gem::Common::expectation const &e,
        double const &limit
    ) const override;

    /** @brief Random initialization of the genome's parameter values.
     *  @param am The activity mode selecting which parameters are randomly initialised
     *  @return true if at least one parameter value was changed */
    bool randomInit_(activityMode const &am) override;

    /** @brief Feasibility check via the shared policy (the constraint reads this genome's values).
     *  @param validity_level Out-parameter receiving the computed validity level
     *  @return true if feasible (or no constraint is registered) */
    bool checkFeasibility_(double &validity_level) const override;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /***************************************************************************/
    /** @brief Emits a name for this class / object. @return The name of this class / object */
    std::string name_() const override { return std::string("GFlatGenomeBase"); }
    /** @brief Creates a deep clone of this object (supplied by the concrete individual). @return A copy */
    GFlatGenomeBase *clone_() const override = 0;

    /** @brief Whether this genome was deserialised from a results-only return (input data omitted).
     *  @return true iff the input parameters were omitted on the wire and must be grafted. */
    bool inputDataOmitted_() const override { return input_omitted_; }
    /** @brief Grafts the input parameters of @p original onto this results-only genome.
     *  @param original The originally-submitted item (a GFlatGenomeBase) supplying the input data. */
    void graftInputDataFrom_(const GCandidateSolution &original) override {
        const auto &src = dynamic_cast<const GFlatGenomeBase &>(original);
        dv_ = src.dv_;
        fv_ = src.fv_;
        iv_ = src.iv_;
        bv_ = src.bv_;
        this->setLayout(src.layout_);
        input_omitted_ = false;
    }

    /** @brief Installs a new shared layout and invalidates the parameter-count cache.
     *  @param layout The new shared, immutable layout (the count cache is re-keyed to it) */
    void setLayout(std::shared_ptr<const GGenomeLayout> layout) {
        layout_ = std::move(layout);
        // Re-key the count cache. Holding a copy of the shared_ptr keeps the layout alive while cached, so
        // a later pointer comparison can never alias a freed-then-reallocated layout (ABA-safe).
        counts_layout_sp_.reset();
    }

    /** @brief Maps an activity mode to a cache column (0 == ALL, 1 == ACTIVEONLY, 2 == INACTIVEONLY).
     *  @param am The activity mode. @return The cache column index */
    static std::size_t amIndex(activityMode const &am) {
        switch(am) {
        case activityMode::ACTIVEONLY: return 1;
        case activityMode::INACTIVEONLY: return 2;
        case activityMode::ALLPARAMETERS: // == DEFAULTACTIVITYMODE
        default: return 0;
        }
    }

    /** @brief Returns a cached parameter count, recomputing the whole cache if the layout has changed.
     *  The cache is keyed on the SHARED layout (a held shared_ptr), so a structural change (a new layout
     *  object) re-keys it and the layout cannot be freed-and-reused underneath us while cached.
     *  @param type_idx The channel index (0 double, 1 float, 2 int32, 3 bool)
     *  @param am The activity mode. @return The number of matching parameters */
    std::size_t cachedCount(std::size_t type_idx, activityMode const &am) const {
        if(counts_layout_sp_.get() != layout_.get()) {
            // Recompute all 4 channels x 3 activity modes for the current layout.
            for(std::size_t a = 0; a < 3; ++a) {
                const activityMode m = (a == 1) ? activityMode::ACTIVEONLY
                                     : (a == 2) ? activityMode::INACTIVEONLY
                                                : activityMode::ALLPARAMETERS;
                counts_cache_[0][a] = countImpl<double>(layout_->d, m);
                counts_cache_[1][a] = countImpl<float>(layout_->f, m);
                counts_cache_[2][a] = countImpl<std::int32_t>(layout_->i, m);
                counts_cache_[3][a] = countImpl<bool>(layout_->b, m);
            }
            counts_layout_sp_ = layout_; // re-key (also pins the layout while cached)
        }
        return counts_cache_[type_idx][amIndex(am)];
    }

    /** @brief Retrieve the active double parameter at the given positional index.
     *  @param idx The position among the active double parameters. @return Its (folded) value */
    double getVarVal_d_(std::size_t idx);
    /** @brief Retrieve the active float parameter at the given positional index.
     *  @param idx The position among the active float parameters. @return Its (folded) value */
    float getVarVal_f_(std::size_t idx);
    /** @brief Retrieve the active int32 parameter at the given positional index.
     *  @param idx The position among the active int32 parameters. @return Its (folded) value */
    std::int32_t getVarVal_i_(std::size_t idx);
    /** @brief Retrieve the active bool parameter at the given positional index.
     *  @param idx The position among the active bool parameters. @return Its value */
    bool getVarVal_b_(std::size_t idx);

    /** @brief Retrieval of a suitable position for cross over inside of a vector.
     *  @param lower The minimum allowed cross-over position (must be > 0)
     *  @param upper The exclusive upper bound of the cross-over range
     *  @return A valid cross-over position within the vector */
    std::size_t getCrossOverPos(std::size_t lower, std::size_t upper);

    /***************************************************************************/
    // Activity / fold helpers (genome-agnostic value mapping).

    /** @brief Whether a value with the given active flag matches the requested activity mode.
     *  @param active The per-parameter active flag (non-zero == active)
     *  @param am The requested activity mode
     *  @return true if the parameter should be included under the given activity mode */
    static bool amMatch(std::uint8_t active, activityMode const &am) {
        switch(am) {
        case activityMode::ACTIVEONLY: return active != 0;
        case activityMode::INACTIVEONLY: return active == 0;
        case activityMode::ALLPARAMETERS: // == DEFAULTACTIVITYMODE
        default: return true;
        }
    }

    /** @brief THE single source of truth for the per-element internal->external map (§2.1).
     *  @tparam T The channel's value type (double / float / int32)
     *  @param ch The channel layout. @param stored The internally stored value. @param k The index
     *  @return The external (user-coordinate) representation of the stored value */
    template <typename T>
    static T externalValue(ChannelLayout<T> const &ch, T stored, std::size_t k) {
        if constexpr(std::is_floating_point_v<T>) {
            const T scale = ngScale<T>(ch, k);
            const T anchor = ngAnchor<T>(ch, k);
            if(not ch.fold[k]) {
                return ngInternalToExternal<T>(stored, scale, anchor);
            }
#ifdef DEBUG
            if(scale > T(0) && not(stored >= T(-0.5) && stored < T(0.5))) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GFlatGenomeBase::externalValue(): Error!" << '\n'
                    << "Internal coordinate " << stored << " of bounded parameter " << k
                    << " is outside the canonical interval [-0.5, 0.5) -- a write path failed to fold." << '\n'
                );
            }
#endif /* DEBUG */
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

    /** @brief The per-element EXTERNAL->INTERNAL map for a floating-point external write (§2.2).
     *  @tparam T The floating-point channel's value type (double / float)
     *  @param ch The channel layout. @param x The external value. @param k The index
     *  @param allow_upper_bound Whether a value exactly on the (inclusive) upper bound is accepted
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
                return T(0);
            }
            const bool below = x < ch.lower[k];
            const bool above = allow_upper_bound ? (x > ch.upper[k]) : not(x < ch.upper[k]);
            if(below || above) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GFlatGenomeBase::externalToInternalChecked(): Error!" << '\n'
                    << "External value " << x << " is outside the "
                    << (allow_upper_bound ? "range [" : "half-open range [")
                    << ch.lower[k] << ", " << ch.upper[k] << (allow_upper_bound ? "]" : ")")
                    << " of bounded parameter " << k << '\n'
                );
            }
            return ngClampHalfOpen<T>(ngExternalToInternal<T>(x, scale, anchor), T(-0.5), T(0.5));
        }
        return ngExternalToInternal<T>(x, scale, anchor);
    }

    /***************************************************************************/
    // Per-type channel helpers -- the templated bodies the public surface forwards to.

    /** @brief Collects a channel's active, range-folded values into an output vector.
     *  @tparam T The channel's value type. @param out The output vector (cleared then filled)
     *  @param ch The channel layout. @param store The internal value storage. @param am The activity mode */
    template <typename T>
    void streamlineImpl(
        std::vector<T> &out, ChannelLayout<T> const &ch, std::vector<T> const &store, activityMode const &am
    ) const {
        out.clear();
        for(std::size_t k = 0; k < store.size(); ++k) {
            if(amMatch(ch.active[k], am)) {
                out.push_back(externalValue<T>(ch, store[k], k));
            }
        }
    }

    /** @brief Writes the active channel's external values directly into a caller buffer (count returned).
     *  @tparam T The channel's value type. @param dst The destination buffer
     *  @param ch The channel layout. @param store The internal value storage. @param am The activity mode
     *  @return The number of values written into dst */
    template <typename T>
    std::size_t streamlineIntoImpl(
        T *dst, ChannelLayout<T> const &ch, std::vector<T> const &store, activityMode const &am
    ) const {
        std::size_t n = 0;
        for(std::size_t k = 0; k < store.size(); ++k) {
            if(amMatch(ch.active[k], am)) {
                dst[n++] = externalValue<T>(ch, store[k], k);
            }
        }
        return n;
    }

    /** @brief Adaption-time write-fold (§2.2): contains every bounded value in the store.
     *  @tparam T The channel's value type. @param ch The channel layout. @param store The storage (folded) */
    template <typename T>
    static void foldChannelInPlace(ChannelLayout<T> const &ch, std::vector<T> &store) {
        for(std::size_t k = 0; k < store.size(); ++k) {
            if(not ch.fold[k]) {
                continue;
            }
            if constexpr(std::is_floating_point_v<T>) {
                store[k] = ngFoldInternal<T>(store[k]);
            }
            else {
                store[k] = foldConstrainedInt<T>(store[k], ch.lower[k], ch.upper[k]);
            }
        }
    }

    /** @brief Writes a vector of EXTERNAL values back into the active slots of a channel (§2.2).
     *  @tparam T The channel's value type. @param in The incoming external values
     *  @param ch The channel layout. @param store The storage to update. @param am The activity mode */
    template <typename T>
    void assignImpl(
        std::vector<T> const &in, ChannelLayout<T> const &ch, std::vector<T> &store, activityMode const &am
    ) {
        std::size_t pos = 0;
        for(std::size_t k = 0; k < store.size(); ++k) {
            if(amMatch(ch.active[k], am)) {
                const T x = in.at(pos++);
                if constexpr(std::is_floating_point_v<T>) {
                    store[k] = externalToInternalChecked<T>(ch, x, k);
                }
                else {
                    store[k] = x;
                }
            }
        }
    }

    /** @brief Collects a channel's active values in their raw INTERNAL representation (§2.3).
     *  @tparam T The channel's value type. @param out The output vector (cleared then filled)
     *  @param ch The channel layout. @param store The internal value storage. @param am The activity mode */
    template <typename T>
    void streamlineInternalImpl(
        std::vector<T> &out, ChannelLayout<T> const &ch, std::vector<T> const &store, activityMode const &am
    ) const {
        out.clear();
        for(std::size_t k = 0; k < store.size(); ++k) {
            if(amMatch(ch.active[k], am)) {
                out.push_back(store[k]);
            }
        }
    }

    /** @brief Writes a vector of raw INTERNAL values back into the active slots of an FP channel (§2.2).
     *  @tparam T The FP channel's value type. @param in The incoming internal values
     *  @param ch The channel layout. @param store The storage to update. @param am The activity mode */
    template <typename T>
    void assignInternalImpl(
        std::vector<T> const &in, ChannelLayout<T> const &ch, std::vector<T> &store, activityMode const &am
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
     *  @tparam T The channel's value type. @param ch The channel layout. @param am The activity mode
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
     *  @tparam T The channel's value type. @param l Lower bounds out. @param u Upper bounds out
     *  @param ch The channel layout. @param am The activity mode */
    template <typename T>
    void boundariesImpl(
        std::vector<T> &l, std::vector<T> &u, ChannelLayout<T> const &ch, activityMode const &am
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
     *  @param out The output vector (cleared then filled). @param am The activity mode */
    void streamlineBool(std::vector<bool> &out, activityMode const &am) const {
        out.clear();
        for(std::size_t k = 0; k < bv_.size(); ++k) {
            if(amMatch(layout_->b.active[k], am)) {
                out.push_back(bv_[k] != 0);
            }
        }
    }
    /** @brief Writes a bool vector back into the active slots of the byte storage.
     *  @param in The incoming bool values. @param am The activity mode */
    void assignBool(std::vector<bool> const &in, activityMode const &am) {
        std::size_t pos = 0;
        for(std::size_t k = 0; k < bv_.size(); ++k) {
            if(amMatch(layout_->b.active[k], am)) {
                bv_[k] = in.at(pos++) ? static_cast<std::uint8_t>(1) : static_cast<std::uint8_t>(0);
            }
        }
    }
    /** @brief Collects the bounds of the active bool parameters (always [false, true]).
     *  @param l Lower bounds out. @param u Upper bounds out. @param am The activity mode */
    void boundariesBool(std::vector<bool> &l, std::vector<bool> &u, activityMode const &am) const {
        l.clear();
        u.clear();
        for(std::size_t k = 0; k < layout_->b.size(); ++k) {
            if(amMatch(layout_->b.active[k], am)) {
                l.push_back(false);
                u.push_back(true);
            }
        }
    }

    /***************************************************************************/
    // Random-init channel helpers (the per-channel ADAPTION kernels are OA-owned).

    /** @brief Randomly initialises a floating-point channel's active parameters.
     *  @tparam T The channel's value type. @param store The internal storage (overwritten at active positions)
     *  @param ch The channel layout. @param am The activity mode. @return true if a value changed */
    template <typename T>
    bool randomInitFP(std::vector<T> &store, ChannelLayout<T> const &ch, activityMode const &am);
    /** @brief Randomly initialises the int32 channel's active parameters.
     *  @param am The activity mode. @return true if a value changed */
    bool randomInitInt(activityMode const &am);
    /** @brief Randomly initialises the bool channel's active parameters.
     *  @param am The activity mode. @return true if a value changed */
    bool randomInitBool(activityMode const &am);

    /***************************************************************************/
    // Data: the four contiguous value channels + the shared structural layout.

    std::vector<double> dv_;        ///< the double channel values (internal representation)
    std::vector<float> fv_;         ///< the float channel values (internal representation)
    std::vector<std::int32_t> iv_;  ///< the int32 channel values (internal representation)
    std::vector<std::uint8_t> bv_;  ///< the bool channel values (1/0)

    /** @brief The shared, immutable structural descriptor (bounds / grouping / adaption config) */
    std::shared_ptr<const GGenomeLayout> layout_ = std::make_shared<const GGenomeLayout>();

    /** @brief Transient (NOT serialized): set by load() when a results-only return arrived. */
    bool input_omitted_ = false;

    /** @brief Transient transport hint (NOT serialized / compared / loaded): force a full return. */
    bool return_full_individual_ = false;

    /***************************************************************************/
    // The layout-keyed parameter-count cache (watertight: re-keyed whenever the shared layout changes;
    // holding the shared_ptr pins the layout while cached, so pointer comparison can never alias). All
    // mutable, because the count accessors are logically const.

    mutable std::shared_ptr<const GGenomeLayout> counts_layout_sp_; ///< the layout the cache is keyed to
    mutable std::array<std::array<std::size_t, 3>, 4>
        counts_cache_{}; ///< [channel: d/f/i/b][activity mode: ALL/ACTIVEONLY/INACTIVEONLY]
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Genome */

/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::Genome::GFlatGenomeBase) // NOLINT
/******************************************************************************/
