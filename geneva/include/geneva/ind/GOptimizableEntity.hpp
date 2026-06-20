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
#include <functional>
#include <limits>
#include <memory>
#include <random>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <vector>

// Boost header files go here
#include <boost/serialization/split_member.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ptree_serialization.hpp>

// Geneva headers go here
#include "common/GCommonInterfaceT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "courtier/GProcessingContainerT.hpp"
#include "geneva/GMultiConstraintT.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/ind/GAuxiliaryStore.hpp"
#include "geneva/Interface/GMutableI.hpp"
#include "geneva/Interface/GRateableI.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "hap/GRandomT.hpp"

// aliases for ease of use
namespace pt = boost::property_tree;

namespace Gem::Geneva::Genome {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Container for fitness and transformed fitness values, as produced by
 * GOptimizableEntity derivatives.
 */
class individual_processing_result {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /**
     * @brief Serializes this object to/from a Boost archive
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive to read from or write to
     * @param version The serialization version (unused)
     */
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar &BOOST_SERIALIZATION_NVP(raw_fitness_) &
            BOOST_SERIALIZATION_NVP(transformed_fitness_) &
            BOOST_SERIALIZATION_NVP(transformed_fitness_set_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constuctor */
    individual_processing_result() = default;

    /**
     * @brief Initialization with a raw fitness
     * @param raw_fitness The raw fitness value to store
     */
    explicit individual_processing_result(double);

    /**
     * @brief Initialization with a raw and transformed fitness
     * @param raw_fitness The raw fitness value to store
     * @param transformed_fitness The transformed fitness value to store
     */
    individual_processing_result(double, double);

    /**
     * @brief Initialization with a raw fitness and recalculation of the transformed fitness
     * @param raw_fitness The raw fitness value to store
     * @param transform The function used to derive the transformed fitness from the raw value
     */
    individual_processing_result(double, std::function<double(double)>);

    /**
     * @brief Copy construction
     * @param cp The other object to copy from
     */
    individual_processing_result(individual_processing_result const &) = default;

    /**
     * @brief Move construction
     * @param cp The other object to move from
     */
    individual_processing_result(individual_processing_result &&) = default;

    /** @brief Destructor */
    ~individual_processing_result() = default;

    /**
     * @brief Assignment
     * @param cp The other object to copy-assign from
     * @return A reference to this object
     */
    individual_processing_result &operator=(individual_processing_result const &) = default;

    /**
     * @brief Move assignment
     * @param cp The other object to move-assign from
     * @return A reference to this object
     */
    individual_processing_result &operator=(individual_processing_result &&) = default;

    /**
     * @brief Access to the raw fitness
     * @return The stored raw fitness value
     */
    double rawFitness() const;

    /**
     * @brief Access to the transformed fitness
     * @return The stored transformed fitness value
     */
    double transformedFitness() const;

    /**
     * @brief Updates the transformed fitness using an external function
     * @param transform The function applied to the raw fitness to obtain the transformed fitness
     */
    void setTransformedFitnessWith(std::function<double(double)>);

    /**
     * @brief Sets the transformed fitness to a user-defined value
     * @param transformed_fitness The transformed fitness value to store
     */
    void setTransformedFitnessTo(double);

    /** @brief Sets the transformed fitness to the same value as the raw fitness */
    void setTransformedFitnessToRaw();

    /**
     * @brief Checks whether the transformed fitness was set
     * @return true if a transformed fitness value is available, false otherwise
     */
    bool transformedFitnessSet() const;

    /**
     * @brief Resets the object and stores a new raw value in the class
     * @param raw_fitness The new raw fitness value to store
     */
    void reset(double);

    /**
     * @brief Resets the object and stores a new raw and transformed value in the class
     * @param raw_fitness The new raw fitness value to store
     * @param transformed_fitness The new transformed fitness value to store
     */
    void reset(double, double);

    /**
     * @brief Resets the object and stores a new raw value in the class and triggers recalculation of the transformed value
     * @param raw_fitness The new raw fitness value to store
     * @param transform The function used to derive the transformed fitness from the raw value
     */
    void reset(double, std::function<double(double)>);

private:
    /***************************************************************************/
    // Data

    double raw_fitness_ = 0.; ///< The fitness as it comes out of the fitnessCalculation() function
    double transformed_fitness_ =
        0.; ///< The fitness as calculated from raw_fitness_ through
    bool transformed_fitness_set_ =
        false; ///< Indicates whether a suitable transformed_fitness_ value is available
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This is the abstract base class of all optimizable individuals. It carries all
 * the framework-level per-individual state (fitness / processing result, the
 * registered constraint object, best-past fitness, iteration, validity, stall
 * counters, eval policy, personality traits, ...) and the algorithm-facing surface
 * that the optimization algorithms operate on. The genome itself -- how the actual
 * parameters are stored -- is provided by the derived GFlatGenome, which stores plain
 * value vectors (GenomeData). The optimization algorithms hold their population as
 * GOptimizableEntity and reach the genome through this type-safe interface.
 *
 * This class is the CRTP category root (GCommonInterfaceT<GOptimizableEntity>); the
 * genome access points that the algorithms call polymorphically are declared pure
 * virtual here and implemented by the genome-carrying derivatives.
 */
class GOptimizableEntity // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Common::GCommonInterfaceT<GOptimizableEntity>
  , public Interface::GMutableI
  , public Interface::GRateableI
  , public Gem::Courtier::GProcessingContainerT<GOptimizableEntity, individual_processing_result> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /***************************************************************************/
    /**
     * Single declaration of this class'es local data members. This drives serialize(),
     * load_() and compare_() from one place. Plain members use make_member(); the
     * cloneable smart pointers (the aux store's personality traits + individual_constraint_ptr_) use
     * make_cloneable_member(), so g_load_members() deep-clones them while serialize()
     * and compare_() treat them like any other member.
     *
     * Handled manually (NOT in this tuple): the GProcessingContainerT processing base,
     * which is a base-object rather than a local member.
     *
     * @return A tuple of named member references driving serialize(), load_() and compare_()
     */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            Gem::Common::make_member("best_past_primary_fitness_", self.best_past_primary_fitness_),
            Gem::Common::make_member("n_stalls_", self.n_stalls_),
            Gem::Common::make_member("maxmode_", self.maxmode_),
            Gem::Common::make_member("assigned_iteration_", self.assigned_iteration_),
            Gem::Common::make_member("validity_level_", self.validity_level_),
            Gem::Common::make_member("eval_policy_", self.eval_policy_),
            Gem::Common::make_member("sigmoid_steepness_", self.sigmoid_steepness_),
            Gem::Common::make_member("sigmoid_extremes_", self.sigmoid_extremes_),
            Gem::Common::make_member("max_unsuccessful_adaptions_", self.max_unsuccessful_adaptions_),
            Gem::Common::make_member("max_retries_until_valid_", self.max_retries_until_valid_),
            Gem::Common::make_member("n_adaptions_", self.n_adaptions_),
            Gem::Common::make_member("use_random_crash_", self.use_random_crash_),
            Gem::Common::make_member("random_crash_prob_", self.random_crash_prob_),
            Gem::Common::make_cloneable_member("individual_constraint_ptr_", self.individual_constraint_ptr_)
        );
    }

    /**
     * @brief Serializes this object to/from a Boost archive
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive to read from or write to
     * @param version The serialization version (unused)
     */
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        // This is the CRTP category root. Its CRTP base
        // (Gem::Common::GCommonInterfaceT<GOptimizableEntity>) carries no state and is
        // therefore not serialized as a base_object -- mirroring GObject, whose
        // serialize() is likewise empty. The stateful processing base IS serialized
        // as a base-object rather than a local member.
        ar &make_nvp(
            "GProcessingContainerT_GOptimizableEntity",
            boost::serialization::base_object<Gem::Courtier::GProcessingContainerT<
                GOptimizableEntity,
                individual_processing_result>>(*this)
        );

        // All members (plain and cloneable alike) are derived from the single
        // localMembers() declaration. The individual carries NO optimization-algorithm scratch or
        // identity (the personality object lives on the GIndividualSlot, and post-processing eligibility
        // is decided by the algorithm and vetoed on the work item's processing metadata), so serialize()
        // is unconditionally pure.
        Gem::Common::serialize_members(ar, localMembers_(*this));
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GOptimizableEntity();
    /**
     * @brief Initialization with the number of fitness criteria
     * @param n_fitness_criteria The number of fitness criteria this individual evaluates
     */
    explicit GOptimizableEntity(std::size_t);
    /**
     * @brief The copy constructor
     * @param cp The other GOptimizableEntity whose data is copied
     */
    GOptimizableEntity(GOptimizableEntity const &);
    /** @brief The destructor */
    ~GOptimizableEntity() override = default;

    /**
     * @brief Allows to randomly initialize parameter members
     * @param am The activity mode controlling which parameters are affected
     * @return true if at least one parameter was randomly initialized
     */
    bool randomInit(activityMode const &);

    /**
     * @brief Specify whether we want to work in maximization (maxMode::MAXIMIZE) or minimization (maxMode::MINIMIZE) mode
     * @param mode The optimization mode (maximization or minimization)
     */
    void setMaxMode(maxMode const &);

    /**
     * @brief Requests that this individual be returned to the server in FULL (its input parameters
     * included) rather than in the default lightweight results-only form. A networked client that has
     * MODIFIED the individual -- e.g. a nested / network-tiered optimization that replaces it with a
     * better one it found locally -- sets this so the new parameters travel back. A transient transport
     * hint (not serialized, compared or loaded); the default (false) ships only the computed results,
     * the server grafting the originally-submitted parameters back on.
     * @param full true to force a full return; false (the default) for the lightweight results-only form
     */
    void setReturnFullIndividual(bool full) { return_full_individual_ = full; }
    /**
     * @brief Whether a full return was requested for this individual (see setReturnFullIndividual()).
     * @return true if the full individual should be returned; false for the results-only form
     */
    bool getReturnFullIndividual() const { return return_full_individual_; }

    /**
     * @brief Transformation of the individual's parameters into a boost::property_tree object
     * @param ptr The property tree the parameters are written to
     * @param baseName The base name under which the parameters are stored (default "parameterset")
     */
    virtual void toPropertyTree(pt::ptree &, std::string const & = "parameterset") const = 0;

    /**
     * @brief Transformation of the individual's parameters into a list of comma-separated values
     * @param with_name_and_type Whether to prepend each value with its name and type
     * @param with_commas Whether to separate values with commas
     * @param use_raw_fitness Whether to emit the raw (rather than transformed) fitness
     * @param show_validity Whether to include the validity status
     * @return The CSV representation of this individual
     */
    virtual std::string toCSV(
        bool = false // with_name_and_type
        ,
        bool = true // with_commas
        ,
        bool = true // use_raw_fitness
        ,
        bool = true // show_validity
    ) const = 0;

    /**
     * @brief Checks whether this object is better than a given set of evaluations
     * @param boundaries The set of evaluation values to compare this individual's fitness against
     * @return true if this object is good enough (better than the given evaluations)
     */
    bool isGoodEnough(std::vector<double> const &);

    /**
     * @brief Perform a cross-over operation between this object and another
     * @param cp The other entity to cross over with
     * @return A shared pointer to the resulting offspring entity
     */
    virtual std::shared_ptr<GOptimizableEntity>
    crossOverWith(GOptimizableEntity const &) const = 0;

    /**
     * @brief Retrieves parameters relevant for the evaluation from another GOptimizableEntity
     * @param cp The entity whose evaluation-relevant parameters are absorbed into this object
     */
    virtual void cannibalize(GOptimizableEntity &) = 0;

    /***************************************************************************/
    // Genome value channels (genome-agnostic). The public per-type templates are the ergonomic
    // surface the optimization algorithms use; each dispatches to a non-template virtual that the
    // flat genome (GFlatGenome) implements. The algorithms therefore read and write parameter
    // values without knowing the storage layout -- no downcast.

    /**
     * @brief Streamlines all parameters of type par_type into a vector (cleared first)
     * @tparam par_type The parameter value type (double, float, std::int32_t or bool)
     * @param par_vec The vector the parameter values are written into (cleared first)
     * @param am The activity mode controlling which parameters are included
     */
    template <typename par_type>
    void streamline(
        std::vector<par_type> &par_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) const {
        this->streamline_(par_vec, am);
    }

    /**
     * @brief Assigns values from a vector to the parameters of type par_type
     * @tparam par_type The parameter value type (double, float, std::int32_t or bool)
     * @param par_vec The vector of values to scatter onto the matching parameters
     * @param am The activity mode controlling which parameters are written
     */
    template <typename par_type>
    void assignValueVector(
        std::vector<par_type> const &par_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) {
#ifdef DEBUG
        if(countParameters<par_type>() != par_vec.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GOptimizableEntity::assignValueVector():" << '\n'
                << "Sizes don't match: " << countParameters<par_type>() << " / " << par_vec.size()
                << '\n'
            );
        }
#endif /* DEBUG */
        this->assignValueVector_(par_vec, am);
        // As we have modified our internal data sets, make sure the item is reprocessed
        this->mark_as_due_for_processing();
    }

    /**
     * @brief The number of parameters of type par_type
     * @tparam par_type The parameter value type (double, float, std::int32_t or bool)
     * @param am The activity mode controlling which parameters are counted
     * @return The number of parameters of the requested type
     */
    template <typename par_type>
    std::size_t countParameters(activityMode const &am = activityMode::DEFAULTACTIVITYMODE) const {
        if constexpr(std::is_same_v<par_type, double>) {
            return countParametersDouble_(am);
        }
        else if constexpr(std::is_same_v<par_type, float>) {
            return countParametersFloat_(am);
        }
        else if constexpr(std::is_same_v<par_type, std::int32_t>) {
            return countParametersInt32_(am);
        }
        else if constexpr(std::is_same_v<par_type, bool>) {
            return countParametersBool_(am);
        }
        else {
            static_assert(sizeof(par_type) == 0, "countParameters: unsupported parameter type");
            return 0;
        }
    }

    /**
     * @brief Lower/upper boundaries of all parameters of type par_type (cleared first)
     * @tparam par_type The parameter value type (double, float, std::int32_t or bool)
     * @param l_bnd_vec The vector the lower boundaries are written into (cleared first)
     * @param u_bnd_vec The vector the upper boundaries are written into (cleared first)
     * @param am The activity mode controlling which parameters are included
     */
    template <typename par_type>
    void boundaries(
        std::vector<par_type> &l_bnd_vec,
        std::vector<par_type> &u_bnd_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) const {
        this->boundaries_(l_bnd_vec, u_bnd_vec, am);
    }

    /***************************************************************************/
    // The precision-agnostic floating point view (double + float widened to double). Implemented
    // once here on top of the per-type channels above, so every genome layout gets it for free.
    // Geometric algorithms -- (conjugate) gradient descent, Nelder-Mead, swarm -- use this view.

    /**
     * @brief The combined number of double- and float-typed parameters
     * @param am The activity mode controlling which parameters are counted
     * @return The combined count of double and float parameters
     */
    std::size_t
    countFPParameters(activityMode const &am = activityMode::DEFAULTACTIVITYMODE) const {
        return countParameters<double>(am) + countParameters<float>(am);
    }

    /**
     * @brief Streamlines all floating point parameters into a single double vector (double-typed first, then widened float-typed)
     * @param par_vec The vector the floating point values are written into (cleared first)
     * @param am The activity mode controlling which parameters are included
     */
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

    /**
     * @brief Scatters a double vector produced by streamlineFP() back onto the floating point parameters
     * @param par_vec The combined double vector (double-typed first, then float-typed) to scatter back
     * @param am The activity mode controlling which parameters are written
     */
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
                << "In GOptimizableEntity::assignFPValueVector():" << '\n'
                << "Sizes don't match: " << (n_double + n_float) << " / " << par_vec.size() << '\n'
            );
        }
#endif /* DEBUG */

        if(n_double > 0) {
            std::vector<double> double_vec(
                par_vec.begin(),
                par_vec.begin() + static_cast<std::ptrdiff_t>(n_double)
            );
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

    /**
     * @brief Lower/upper boundaries of all floating point parameters (matching streamlineFP() ordering)
     * @param l_bnd_vec The vector the lower boundaries are written into (cleared first)
     * @param u_bnd_vec The vector the upper boundaries are written into (cleared first)
     * @param am The activity mode controlling which parameters are included
     */
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
        for(float v : l_float) {
            l_bnd_vec.push_back(static_cast<double>(v));
        }
        for(float v : u_float) {
            u_bnd_vec.push_back(static_cast<double>(v));
        }
    }

    /***************************************************************************/
    // The INTERNAL (normalized) floating-point view (normalized-genome architecture, §2.3). The
    // optimization algorithms read and write the raw, normalized internal coordinate (magnitude ≈ 1,
    // confined to the canonical interval [-0.5, 0.5) for a bounded parameter), NOT the user-facing
    // external value. This is the "two-reader split": OAs use these *Internal accessors; the objective
    // function / GPU marshaller / user inspection use the external streamlineFP / assignFPValueVector /
    // boundariesFP above. The internal view costs no per-read affine map (it is the stored value), and
    // an internal write FOLDS an overshooting bounded value back into range (never throws), whereas an
    // external write scales + range-validates (throws out of range). Ordering matches streamlineFP:
    // double-typed first, then float-typed widened to double.

    /**
     * @brief Streamlines all floating point parameters in their raw INTERNAL (normalized) representation
     *  into a single double vector (double-typed first, then widened float-typed). For OA move generation.
     * @param par_vec The vector the internal values are written into (cleared first)
     * @param am The activity mode controlling which parameters are included
     */
    void streamlineFPInternal(
        std::vector<double> &par_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) const {
        par_vec.clear();
        this->streamlineInternal_(par_vec, am);

        std::vector<float> float_vec;
        this->streamlineInternal_(float_vec, am);
        par_vec.reserve(par_vec.size() + float_vec.size());
        for(float f : float_vec) {
            par_vec.push_back(static_cast<double>(f));
        }
    }

    /**
     * @brief Scatters a vector of raw INTERNAL (normalized) values back onto the floating point parameters
     *  (an OA write: a bounded value is folded into [-0.5, 0.5), never range-validated). Ordering matches
     *  streamlineFPInternal() (double-typed first, then float-typed).
     * @param par_vec The combined internal-value vector to scatter back
     * @param am The activity mode controlling which parameters are written
     */
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
                << "In GOptimizableEntity::assignFPValueVectorInternal():" << '\n'
                << "Sizes don't match: " << (n_double + n_float) << " / " << par_vec.size() << '\n'
            );
        }
#endif /* DEBUG */

        if(n_double > 0) {
            std::vector<double> double_vec(
                par_vec.begin(),
                par_vec.begin() + static_cast<std::ptrdiff_t>(n_double)
            );
            this->assignValueVectorInternal_(double_vec, am);
        }
        if(n_float > 0) {
            std::vector<float> float_vec(n_float);
            for(std::size_t i = 0; i < n_float; ++i) {
                float_vec[i] = static_cast<float>(par_vec[n_double + i]);
            }
            this->assignValueVectorInternal_(float_vec, am);
        }
        // As with the external assign, modifying the parameters marks the item for reprocessing.
        this->mark_as_due_for_processing();
    }

    /**
     * @brief Lower/upper boundaries of all floating point parameters in INTERNAL coordinates (matching
     *  streamlineFPInternal() ordering): [-0.5, 0.5) for a bounded parameter, the full ±range for an
     *  unbounded one (which has no internal wall).
     * @param l_bnd_vec The vector the lower boundaries are written into (cleared first)
     * @param u_bnd_vec The vector the upper boundaries are written into (cleared first)
     * @param am The activity mode controlling which parameters are included
     */
    void boundariesFPInternal(
        std::vector<double> &l_bnd_vec,
        std::vector<double> &u_bnd_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) const {
        std::vector<double> l_double;
        std::vector<double> u_double;
        this->boundariesInternal_(l_double, u_double, am);

        std::vector<float> l_float;
        std::vector<float> u_float;
        this->boundariesInternal_(l_float, u_float, am);

        l_bnd_vec.clear();
        u_bnd_vec.clear();
        l_bnd_vec.reserve(l_double.size() + l_float.size());
        u_bnd_vec.reserve(u_double.size() + u_float.size());
        l_bnd_vec.insert(l_bnd_vec.end(), l_double.begin(), l_double.end());
        u_bnd_vec.insert(u_bnd_vec.end(), u_double.begin(), u_double.end());
        for(float v : l_float) {
            l_bnd_vec.push_back(static_cast<double>(v));
        }
        for(float v : u_float) {
            u_bnd_vec.push_back(static_cast<double>(v));
        }
    }

    /**
     * @brief Register another result value of the fitness calculation
     * @param id The index of the fitness criterion to store the result for
     * @param value The result value to register
     */
    void setResult(std::size_t, double);
    /**
     * @brief Determines whether more than one fitness criterion is present for this individual
     * @return true if more than one fitness criterion is present
     */
    bool hasMultipleFitnessCriteria() const;

    /**
     * @brief Retrieve the fitness tuple at a given evaluation position
     * @param id The evaluation position (fitness criterion index); defaults to 0
     * @return A (raw, transformed) fitness tuple at the requested position
     */
    std::tuple<double, double> getFitnessTuple(std::uint32_t = 0) const;

    /**
     * @brief Allows to retrieve the maxmode_ parameter
     * @return The optimization mode (maximization or minimization)
     */
    maxMode getMaxMode() const;

    /**
     * @brief Retrieves the worst possible evaluation result, depending on whether we are in maximization or minimization mode
     * @return The worst-case evaluation value for the current mode
     */
    virtual double getWorstCase() const;

    /**
     * @brief Retrieves the best possible evaluation result, depending on whether we are in maximization or minimization mode
     * @return The best-case evaluation value for the current mode
     */
    virtual double getBestCase() const;

    /**
     * @brief Retrieves the steepness_ variable (used for the sigmoid transformation)
     * @return The sigmoid steepness value
     */
    double getSteepness() const;
    /**
     * @brief Sets the steepness variable (used for the sigmoid transformation)
     * @param steepness The new sigmoid steepness value
     */
    void setSteepness(double);

    /**
     * @brief Retrieves the barrier_ variable (used for the sigmoid transformation)
     * @return The sigmoid barrier (extreme) value
     */
    double getBarrier() const;
    /**
     * @brief Sets the barrier variable (used for the sigmoid transformation)
     * @param barrier The new sigmoid barrier (extreme) value
     */
    void setBarrier(double);

    /**
     * @brief Sets the maximum number of adaption attempts that may pass without actual modifications
     * @param max_unsuccessful_adaptions The maximum number of unsuccessful adaption attempts allowed
     */
    void setMaxUnsuccessfulAdaptions(std::size_t);
    /**
     * @brief Retrieves the maximum number of adaption attempts that may pass without actual modifications
     * @return The maximum number of unsuccessful adaption attempts allowed
     */
    std::size_t getMaxUnsuccessfulAdaptions() const;

    /**
     * @brief Set maximum number of retries until a valid individual was found
     * @param max_retries_until_valid The maximum number of adaption retries until a valid individual is found
     */
    void setMaxRetriesUntilValid(std::size_t max_retries_until_valid);
    /**
     * @brief Retrieves the maximum number of retries until a valid individual was found
     * @return The maximum number of adaption retries until a valid individual is found
     */
    std::size_t getMaxRetriesUntilValid() const;

    /**
     * @brief Retrieves the number of adaptions performed during the last call to adapt()
     * @return The number of adaptions performed during the last adaption
     */
    std::size_t getNAdaptions() const;
    /**
     * @brief Records the number of adaptions performed (used by the OA-owned adaption free functions)
     * @param n The number of adaptions performed to record
     */
    void setNAdaptions(std::size_t n) { n_adaptions_ = n; }

    /**
     * @brief Public, non-folding access to this individual's per-individual RNG stream. The OA-owned
     * adaption free functions draw from it; each individual owns its own stream, so parallel
     * adaption of distinct individuals is lock-free.
     *
     * @return A reference to this individual's per-individual random engine
     */
    Gem::Hap::GRandomBase &getRandomEngine() { return gr_; }

    /**
     * @brief Public constraint check used by the OA-owned adaption retry loop. Forwards to the protected
     * individualFulfillsConstraints(); returns true if the individual satisfies its constraints and writes
     * the validity level to the out-parameter.
     *
     * @param validity_level Out-parameter receiving the computed validity level
     * @return true if the individual satisfies its constraints, false otherwise
     */
    bool fulfillsConstraints(double &validity_level) const {
        return this->individualFulfillsConstraints(validity_level);
    }

    /**
     * @brief Allows to set the current iteration of the parent optimization algorithm.
     * @param iteration The current iteration of the parent optimization algorithm
     */
    void setAssignedIteration(std::uint32_t const &);
    /**
     * @brief Gives access to the parent optimization algorithm's iteration
     * @return The current iteration of the parent optimization algorithm
     */
    std::uint32_t getAssignedIteration() const;

    /**
     * @brief Allows to specify the number of optimization cycles without improvement of the primary fitness criterion
     * @param nStalls The number of stalled optimization cycles to record
     */
    void setNStalls(std::uint32_t const &);
    /**
     * @brief Allows to retrieve the number of optimization cycles without improvement of the primary fitness criterion
     * @return The number of stalled optimization cycles
     */
    std::uint32_t getNStalls() const;

    /**
     * @brief Allows to activate random crashes for debugging purposes
     * @param useRandomCrash Whether random crashes are enabled
     * @param prob The probability with which a random crash occurs
     */
    void setRandomCrash(bool, double);
    /**
     * @brief Allows to check whether random crashes are activated, and with which probability the occur
     * @return A (enabled, probability) tuple describing the random-crash configuration
     */
    std::tuple<bool, double> getRandomCrash() const;

    /***************************************************************************/
    /**
     * @brief Retrieves a parameter of a given type at the specified position.
     * @tparam val_type The value type to retrieve (double, float, std::int32_t or bool)
     * @param target A (type-index, name, position) tuple; the third element is the index of the
     *        active parameter to retrieve
     * @return The parameter value of the requested type at the requested index
     */
    template <typename val_type>
    val_type getVarVal(std::tuple<std::size_t, std::string, std::size_t> const &target) {
        static_assert(
            std::is_same_v<val_type, double> || std::is_same_v<val_type, float> ||
                std::is_same_v<val_type, std::int32_t> || std::is_same_v<val_type, bool>,
            "GOptimizableEntity::getVarVal<>(): unsupported value type (use double, float, std::int32_t or bool)"
        );

        const std::size_t idx = std::get<2>(target);

        // Compile-time dispatch to the matching typed virtual -- no std::any boxing, no runtime typeid
        // branch, on what is a per-iteration monitor hot path.
        if constexpr (std::is_same_v<val_type, double>) {
            return this->getVarVal_d_(idx);
        } else if constexpr (std::is_same_v<val_type, float>) {
            return this->getVarVal_f_(idx);
        } else if constexpr (std::is_same_v<val_type, std::int32_t>) {
            return this->getVarVal_i_(idx);
        } else { // bool, by the static_assert above
            return this->getVarVal_b_(idx);
        }
    }

    /***************************************************************************/
    // OA-owned scratch — the personality OBJECT (GPersonalityTraits) AND the per-group adaption POD
    // state (sigma / ad_prob / counter, …) — lives on the GIndividualSlot, NOT on the individual. The
    // individual is pure data: genome + bounds + fitness + constraints + the courtier processing
    // container. The adaption logic is OA-owned (geneva/oa/GAdaption.hpp), driven by the slot's scratch.

    /**
     * @brief Check how valid a given solution is
     * @return The validity level of the current solution
     */
    double getValidityLevel() const;
    /**
     * @brief Checks whether all constraints were fulfilled
     * @return true if all registered constraints are fulfilled
     */
    bool constraintsFulfilled() const;
    /**
     * @brief Allows to register a constraint with this individual
     * @param constraint_ptr The constraint-check object to register with this individual
     */
    void
        registerConstraint(std::shared_ptr<GPreEvaluationValidityCheckT<GOptimizableEntity>>);

    /**
     * @brief Allows to set the policy to use in case this individual represents an invalid solution
     * @param eval_policy The evaluation policy to apply for invalid solutions
     */
    void setEvaluationPolicy(evaluationPolicy eval_policy);
    /**
     * @brief Allows to retrieve the current policy in case this individual represents an invalid solution
     * @return The current evaluation policy
     */
    evaluationPolicy getEvaluationPolicy() const;

    /**
     * @brief Checks whether this is a valid solution; meant to be called for "clean" individuals only
     * @return true if this is a valid solution
     */
    bool isValid() const;
    /**
     * @brief Checks whether this solution is invalid
     * @return true if this solution is invalid
     */
    bool isInValid() const;

    /**
     * @brief Allows to set the globally best known primary fitness
     * @param bnf The (raw, transformed) globally best known primary fitness tuple
     */
    void setBestKnownPrimaryFitness(std::tuple<double, double> const &);
    /**
     * @brief Retrieves the value of the globally best known primary fitness
     * @return The (raw, transformed) globally best known primary fitness tuple
     */
    std::tuple<double, double> getBestKnownPrimaryFitness() const;

    /***************************************************************************/
    // Deleted functions

    explicit GOptimizableEntity(float const &) = delete;  ///< Intentionally undefined
    explicit GOptimizableEntity(double const &) = delete; ///< Intentionally undefined

protected:
    /***************************************************************************/
    /**
     * A random number generator. Note that the actual calculation is
     * done in a random number proxy / factory
     */
    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr_;

    /** @brief Uniformly distributed integer random numbers */
    std::uniform_int_distribution<std::size_t> uniform_int_;

    /***************************************************************************/
    /**
     * @brief Do the required processing for this object
     * @param res_vec An optional vector of pre-computed processing results; if empty, the
     *        fitness is calculated here
     */
    void process_(
        const std::vector<individual_processing_result> &res_vec =
            std::vector<individual_processing_result>()
    ) final;

    /**
     * @brief Adds local configuration options to a GParserBuilder object
     * @param gpb The parser builder the configuration options are registered with
     */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &) override;
    /**
     * @brief Loads the data of another GOptimizableEntity
     * @param cp Pointer to the other GOptimizableEntity whose data is loaded
     */
    void load_(const GOptimizableEntity *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GOptimizableEntity>(
        GOptimizableEntity const &,
        GOptimizableEntity const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type
     * @param cp The other object to compare against
     * @param e The expectation for this object, e.g. equality
     * @param limit The limit for allowed deviations of floating point types
     */
    void compare_(
        GOptimizableEntity const & // the other object
        ,
        Gem::Common::expectation const & // the expectation for this object, e.g. equality
        ,
        double const & // the limit for allowed deviations of floating point types
    ) const override;

    /**
     * @brief Random initialization
     * @param am The activity mode controlling which parameters are affected
     * @return true if at least one parameter was randomly initialized
     */
    virtual bool randomInit_(activityMode const &) = 0;

    /**
     * @brief The fitness calculation for the main quality criterion takes place here
     * @return The computed primary fitness value
     */
    double fitnessCalculation() override = 0;
    /**
     * @brief Sets the fitness to a given set of values and clears the dirty flag
     * @param fitness_vec The set of fitness values to assign
     */
    void setFitness_(std::vector<double> const &);

    /**
     * @brief Combines secondary evaluation results by adding the individual results
     * @return The sum of the secondary evaluation results
     */
    double sumCombiner() const;
    /**
     * @brief Combines secondary evaluation results by adding the absolute values of individual results
     * @return The sum of the absolute values of the secondary evaluation results
     */
    double fabsSumCombiner() const;
    /**
     * @brief Combines secondary evaluation results by calculating the square root of the squared sum
     * @return The Euclidean (square-root-of-squared-sum) combination of the secondary results
     */
    double squaredSumCombiner() const;
    /**
     * @brief Combines secondary evaluation results by calculation the square root of the weighed squared sum
     * @param weights The per-result weights applied before squaring and summing
     * @return The square root of the weighed squared sum of the secondary results
     */
    double weighedSquaredSumCombiner(std::vector<double> const &) const;

    /**
     * @brief Checks whether this solution has been rated to be valid; meant to be called by internal functions only
     * @param validity_level Out-parameter receiving the computed validity level
     * @return true if this solution fulfils its constraints
     */
    bool individualFulfillsConstraints(double &) const;

private:
    /***************************************************************************/
    // Overridden or virtual private functions

    /**
     * @brief Emits a name for this class / object
     * @return The class / object name
     */
    std::string name_() const override;
    /**
     * @brief Creates a deep clone of this object
     * @return A pointer to a newly allocated deep copy of this object
     */
    GOptimizableEntity *clone_() const override = 0;

    /**
     * @brief Retrieves the stored raw fitness with a given id
     * @param id The index of the fitness criterion
     * @return The stored raw fitness for the requested criterion
     */
    double raw_fitness_(std::size_t) const final;
    /**
     * @brief Retrieves the stored transformed fitness with a given id
     * @param id The index of the fitness criterion
     * @return The stored transformed fitness for the requested criterion
     */
    double transformed_fitness_(std::size_t) const final;

    /**
     * @brief Returns all raw fitness results in a std::vector
     * @return A vector of all stored raw fitness results
     */
    std::vector<double> raw_fitness_vec_() const final;
    /**
     * @brief Returns all transformed fitness results in a std::vector
     * @return A vector of all stored transformed fitness results
     */
    std::vector<double> transformed_fitness_vec_() const final;

    /***************************************************************************/

    /** @brief Retrieve the value of the active parameter at the given index, per type (genome-specific
     *  dispatch). The non-template targets of the public getVarVal<T>() template -- typed virtuals rather
     *  than a std::any-returning impl, so there is no boxing/typeid on the (hot) monitor path.
     *  @param idx The index of the active parameter to retrieve
     *  @return The active parameter value at @p idx (double for _d_, float for _f_, std::int32_t for _i_, bool for _b_) */
    virtual double getVarVal_d_(std::size_t idx) = 0;
    virtual float getVarVal_f_(std::size_t idx) = 0;
    virtual std::int32_t getVarVal_i_(std::size_t idx) = 0;
    virtual bool getVarVal_b_(std::size_t idx) = 0;

    /***************************************************************************/
    // Per-type genome value channels -- the non-template dispatch targets of the public
    // streamline<T>/assignValueVector<T>/countParameters<T>/boundaries<T> templates. Implemented by
    // the concrete flat genome (which copies a channel array). These are the entire seam that makes
    // value access genome-agnostic. Each overload takes the per-type value/boundary vector(s) and the
    // activityMode controlling which parameters participate.
    virtual void streamline_(std::vector<double> &, activityMode const &) const = 0;
    virtual void streamline_(std::vector<float> &, activityMode const &) const = 0;
    virtual void streamline_(std::vector<std::int32_t> &, activityMode const &) const = 0;
    virtual void streamline_(std::vector<bool> &, activityMode const &) const = 0;

    virtual void assignValueVector_(std::vector<double> const &, activityMode const &) = 0;
    virtual void assignValueVector_(std::vector<float> const &, activityMode const &) = 0;
    virtual void assignValueVector_(std::vector<std::int32_t> const &, activityMode const &) = 0;
    virtual void assignValueVector_(std::vector<bool> const &, activityMode const &) = 0;

    virtual std::size_t countParametersDouble_(activityMode const &) const = 0;
    virtual std::size_t countParametersFloat_(activityMode const &) const = 0;
    virtual std::size_t countParametersInt32_(activityMode const &) const = 0;
    virtual std::size_t countParametersBool_(activityMode const &) const = 0;

    virtual void boundaries_(std::vector<double> &, std::vector<double> &, activityMode const &) const = 0;
    virtual void boundaries_(std::vector<float> &, std::vector<float> &, activityMode const &) const = 0;
    virtual void boundaries_(std::vector<std::int32_t> &, std::vector<std::int32_t> &, activityMode const &) const = 0;
    virtual void boundaries_(std::vector<bool> &, std::vector<bool> &, activityMode const &) const = 0;

    // The INTERNAL (normalized) floating-point channel virtuals backing streamlineFPInternal /
    // assignFPValueVectorInternal / boundariesFPInternal (normalized-genome architecture §2.3). Only the
    // FP channels carry an internal/external distinction (int / bool are not normalized), so only double
    // and float overloads exist. The internal read returns the raw stored value; the internal write folds
    // a bounded value into [-0.5, 0.5) (no range validation); the internal boundaries are [-0.5, 0.5) for
    // a bounded parameter and the full ±range for an unbounded one.
    virtual void streamlineInternal_(std::vector<double> &, activityMode const &) const = 0;
    virtual void streamlineInternal_(std::vector<float> &, activityMode const &) const = 0;
    virtual void assignValueVectorInternal_(std::vector<double> const &, activityMode const &) = 0;
    virtual void assignValueVectorInternal_(std::vector<float> const &, activityMode const &) = 0;
    virtual void boundariesInternal_(std::vector<double> &, std::vector<double> &, activityMode const &) const = 0;
    virtual void boundariesInternal_(std::vector<float> &, std::vector<float> &, activityMode const &) const = 0;

    /**
     * @brief  Allows to set all fitnesses to the same value (both raw and transformed values)
     * @param val The value assigned to every raw and transformed fitness
     */
    void setAllFitnessTo(double);

    /**
     * @brief  Allows to set all fitnesses to the same value (raw and transformed values seperately)
     * @param raw_val The value assigned to every raw fitness
     * @param transformed_val The value assigned to every transformed fitness
     */
    void setAllFitnessTo(double, double);

    /***************************************************************************/
    // Data

    /** @brief Holds the globally best known primary fitness of all individuals */
    std::tuple<double, double> best_past_primary_fitness_{std::make_tuple(0., 0.)};
    /** @brief The number of stalls of the primary fitness criterion in the entire set of individuals */
    std::uint32_t n_stalls_ = 0;
    /** @brief Indicates whether we are using maximization or minimization mode */
    maxMode maxmode_ = maxMode::MINIMIZE;
    /** @brief The iteration of the parent algorithm's optimization cycle */
    std::uint32_t assigned_iteration_ = 0;
    /** @brief Indicates how valid a given solution is */
    double validity_level_ = 0.;

    /** @brief Specifies what to do when the individual is marked as invalid */
    evaluationPolicy eval_policy_ = Gem::Geneva::evaluationPolicy::USESIMPLEEVALUATION;
    /** @brief Determines the "steepness" of a sigmoid function used by optimization algorithms */
    double sigmoid_steepness_ = Gem::Geneva::FITNESSSIGMOIDSTEEPNESS;
    /** @brief Determines the extreme values of a sigmoid function used by optimization algorithms */
    double sigmoid_extremes_ = Gem::Geneva::WORSTALLOWEDVALIDFITNESS;

    /** @brief A constraint-check to be applied to one or more components of this individual */
    std::shared_ptr<GPreEvaluationValidityCheckT<GOptimizableEntity>> individual_constraint_ptr_;

    std::size_t max_unsuccessful_adaptions_ = Gem::Geneva::
        DEFMAXUNSUCCESSFULADAPTIONS; ///< The maximum number of calls to customAdaptions() in a row without actual modifications
    std::size_t max_retries_until_valid_ = Gem::Geneva::
        DEFMAXRETRIESUNTILVALID; ///< The maximum number an adaption of an individual should be performed until a valid parameter set was found
    std::size_t n_adaptions_ =
        0; ///< Stores the actual number of adaptions after a call to "adapt()"

    bool use_random_crash_ =
        false; ///< Indicates whether the individual should crash at random intervals for debugging purposes
    double random_crash_prob_ = 0.; ///< The probability for a random crash

    /** @brief Transient transport hint (NOT serialized / compared / loaded): when a networked client
     *  has MODIFIED this individual and wants the modified version returned in full, it sets this so the
     *  return carries the input parameters rather than the default lightweight results-only form. */
    bool return_full_individual_ = false;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Genome */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::Genome::GOptimizableEntity)        // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Genome::individual_processing_result) // NOLINT
/******************************************************************************/
