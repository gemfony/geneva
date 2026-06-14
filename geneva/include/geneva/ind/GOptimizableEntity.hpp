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

namespace Gem::Geneva::Parameters {

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

    /** @brief Initialization with a raw fitness */
    explicit individual_processing_result(double);

    /** @brief Initialization with a raw and transformed fitness */
    individual_processing_result(double, double);

    /** @brief Initialization with a raw fitness and recalculation of the transformed fitness */
    individual_processing_result(double, std::function<double(double)>);

    /** @brief Copy construction */
    individual_processing_result(individual_processing_result const &) = default;

    /** @brief Move construction */
    individual_processing_result(individual_processing_result &&) = default;

    /** @brief Destructor */
    ~individual_processing_result() = default;

    /** @brief Assignment */
    individual_processing_result &operator=(individual_processing_result const &) = default;

    /** @brief Move assignment */
    individual_processing_result &operator=(individual_processing_result &&) = default;

    /** @brief Access to the raw fitness */
    double rawFitness() const;

    /** @brief Access to the transformed fitness */
    double transformedFitness() const;

    /** @brief Updates the transformed fitness using an external function */
    void setTransformedFitnessWith(std::function<double(double)>);

    /** @brief Sets the transformed fitness to a user-defined value */
    void setTransformedFitnessTo(double);

    /** @brief Sets the transformed fitness to the same value as the raw fitness */
    void setTransformedFitnessToRaw();

    /** @brief Checks whether the transformed fitness was set */
    bool transformedFitnessSet() const;

    /** @brief Resets the object and stores a new raw value in the class */
    void reset(double);

    /** @brief Resets the object and stores a new raw and transformed value in the class */
    void reset(double, double);

    /** @brief Resets the object and stores a new raw value in the class and triggers recalculation of the transformed value */
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
 * parameters are stored -- is left to the derived classes: GTreeGenome stores a
 * tree of GParameterBase objects, while a flat sibling may store plain value
 * vectors. The optimization algorithms hold their population as
 * GOptimizableEntity, so genome layouts are interchangeable type-safely.
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
     */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("best_past_primary_fitness_", best_past_primary_fitness_),
            Gem::Common::make_member("n_stalls_", n_stalls_),
            Gem::Common::make_member("maxmode_", maxmode_),
            Gem::Common::make_member("assigned_iteration_", assigned_iteration_),
            Gem::Common::make_member("validity_level_", validity_level_),
            Gem::Common::make_member("eval_policy_", eval_policy_),
            Gem::Common::make_member("sigmoid_steepness_", sigmoid_steepness_),
            Gem::Common::make_member("sigmoid_extremes_", sigmoid_extremes_),
            Gem::Common::make_member("max_unsuccessful_adaptions_", max_unsuccessful_adaptions_),
            Gem::Common::make_member("max_retries_until_valid_", max_retries_until_valid_),
            Gem::Common::make_member("n_adaptions_", n_adaptions_),
            Gem::Common::make_member("use_random_crash_", use_random_crash_),
            Gem::Common::make_member("random_crash_prob_", random_crash_prob_),
            Gem::Common::make_cloneable_member("pt_ptr_", aux_.personalityRef()),
            Gem::Common::make_cloneable_member("individual_constraint_ptr_", individual_constraint_ptr_)
        );
    }
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("best_past_primary_fitness_", best_past_primary_fitness_),
            Gem::Common::make_member("n_stalls_", n_stalls_),
            Gem::Common::make_member("maxmode_", maxmode_),
            Gem::Common::make_member("assigned_iteration_", assigned_iteration_),
            Gem::Common::make_member("validity_level_", validity_level_),
            Gem::Common::make_member("eval_policy_", eval_policy_),
            Gem::Common::make_member("sigmoid_steepness_", sigmoid_steepness_),
            Gem::Common::make_member("sigmoid_extremes_", sigmoid_extremes_),
            Gem::Common::make_member("max_unsuccessful_adaptions_", max_unsuccessful_adaptions_),
            Gem::Common::make_member("max_retries_until_valid_", max_retries_until_valid_),
            Gem::Common::make_member("n_adaptions_", n_adaptions_),
            Gem::Common::make_member("use_random_crash_", use_random_crash_),
            Gem::Common::make_member("random_crash_prob_", random_crash_prob_),
            Gem::Common::make_cloneable_member("pt_ptr_", aux_.personalityRef()),
            Gem::Common::make_cloneable_member("individual_constraint_ptr_", individual_constraint_ptr_)
        );
    }

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
        // localMembers() declaration.
        Gem::Common::serialize_members(ar, this->localMembers());
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GOptimizableEntity();
    /** @brief Initialization with the number of fitness criteria */
    explicit GOptimizableEntity(std::size_t);
    /** @brief The copy constructor */
    GOptimizableEntity(GOptimizableEntity const &);
    /** @brief The destructor */
    ~GOptimizableEntity() override = default;

    /** @brief Allows to randomly initialize parameter members */
    bool randomInit(activityMode const &);

    /** @brief Specify whether we want to work in maximization (maxMode::MAXIMIZE) or minimization (maxMode::MINIMIZE) mode */
    void setMaxMode(maxMode const &);

    /** @brief Transformation of the individual's parameter objects into a boost::property_tree object */
    virtual void toPropertyTree(pt::ptree &, std::string const & = "parameterset") const = 0;

    /** @brief Transformation of the individual's parameter objects into a list of comma-separated values */
    virtual std::string toCSV(
        bool = false // with_name_and_type
        ,
        bool = true // with_commas
        ,
        bool = true // use_raw_fitness
        ,
        bool = true // show_validity
    ) const = 0;

    /** @brief Checks whether this object is better than a given set of evaluations */
    bool isGoodEnough(std::vector<double> const &);

    /** @brief Perform a cross-over operation between this object and another */
    virtual std::shared_ptr<GOptimizableEntity>
    crossOverWith(GOptimizableEntity const &) const = 0;

    /** @brief Triggers updates of adaptors contained in this object */
    virtual void updateAdaptorsOnStall(std::uint32_t) = 0;

    /** @brief Retrieves information from adaptors with a given property */
    virtual void queryAdaptor(
        std::string const &adaptor_name,
        std::string const &property,
        std::vector<std::any> &data
    ) const = 0;

    /** @brief Retrieves parameters relevant for the evaluation from another GOptimizableEntity */
    virtual void cannibalize(GOptimizableEntity &) = 0;

    /***************************************************************************/
    // Genome value channels (genome-agnostic). The public per-type templates are the ergonomic
    // surface the optimization algorithms use; each dispatches to a non-template virtual that the
    // concrete genome (GTreeGenome / a future GFlatGenome) implements. The algorithms therefore
    // read and write parameter values without knowing the storage layout -- no downcast.

    /** @brief Streamlines all parameters of type par_type into a vector (cleared first) */
    template <typename par_type>
    void streamline(
        std::vector<par_type> &par_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) const {
        this->streamline_(par_vec, am);
    }

    /** @brief Assigns values from a vector to the parameters of type par_type */
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

    /** @brief The number of parameters of type par_type */
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

    /** @brief Lower/upper boundaries of all parameters of type par_type (cleared first) */
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

    /** @brief The combined number of double- and float-typed parameters */
    std::size_t
    countFPParameters(activityMode const &am = activityMode::DEFAULTACTIVITYMODE) const {
        return countParameters<double>(am) + countParameters<float>(am);
    }

    /** @brief Streamlines all floating point parameters into a single double vector (double-typed first, then widened float-typed) */
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

    /** @brief Scatters a double vector produced by streamlineFP() back onto the floating point parameters */
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

    /** @brief Lower/upper boundaries of all floating point parameters (matching streamlineFP() ordering) */
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

    /** @brief The adaption interface */
    std::size_t adapt() override;

    /** @brief Register another result value of the fitness calculation */
    void setResult(std::size_t, double);
    /** @brief Determines whether more than one fitness criterion is present for this individual */
    bool hasMultipleFitnessCriteria() const;

    /** @brief Retrieve the fitness tuple at a given evaluation position */
    std::tuple<double, double> getFitnessTuple(std::uint32_t = 0) const;

    /** @brief Allows to retrieve the maxmode_ parameter */
    maxMode getMaxMode() const;

    /** @brief Retrieves the worst possible evaluation result, depending on whether we are in maximization or minimization mode */
    virtual double getWorstCase() const;

    /** @brief Retrieves the best possible evaluation result, depending on whether we are in maximization or minimization mode */
    virtual double getBestCase() const;

    /** @brief Retrieves the steepness_ variable (used for the sigmoid transformation) */
    double getSteepness() const;
    /** @brief Sets the steepness variable (used for the sigmoid transformation) */
    void setSteepness(double);

    /** @brief Retrieves the barrier_ variable (used for the sigmoid transformation) */
    double getBarrier() const;
    /** @brief Sets the barrier variable (used for the sigmoid transformation) */
    void setBarrier(double);

    /** @brief Sets the maximum number of adaption attempts that may pass without actual modifications */
    void setMaxUnsuccessfulAdaptions(std::size_t);
    /** @brief Retrieves the maximum number of adaption attempts that may pass without actual modifications */
    std::size_t getMaxUnsuccessfulAdaptions() const;

    /** @brief Set maximum number of retries until a valid individual was found  */
    void setMaxRetriesUntilValid(std::size_t max_retries_until_valid);
    /** Retrieves the maximum number of retries until a valid individual was found. */
    std::size_t getMaxRetriesUntilValid() const;

    /** @brief Retrieves the number of adaptions performed during the last call to adapt() */
    std::size_t getNAdaptions() const;
    /** @brief Records the number of adaptions performed (used by the OA-owned adaption free functions) */
    void setNAdaptions(std::size_t n) { n_adaptions_ = n; }

    /**
     * @brief Public, non-folding access to this individual's per-individual RNG stream. The OA-owned
     * adaption free functions (Phase 8) draw from it; each individual owns its own stream, so parallel
     * adaption of distinct individuals is lock-free.
     */
    Gem::Hap::GRandomBase &getRandomEngine() { return gr_; }

    /**
     * @brief Public constraint check used by the OA-owned adaption retry loop. Forwards to the protected
     * individualFulfillsConstraints(); returns true if the individual satisfies its constraints and writes
     * the validity level to the out-parameter.
     */
    bool fulfillsConstraints(double &validity_level) const {
        return this->individualFulfillsConstraints(validity_level);
    }

    /** @brief Allows to set the current iteration of the parent optimization algorithm. */
    void setAssignedIteration(std::uint32_t const &);
    /** @brief Gives access to the parent optimization algorithm's iteration */
    std::uint32_t getAssignedIteration() const;

    /** @brief Allows to specify the number of optimization cycles without improvement of the primary fitness criterion */
    void setNStalls(std::uint32_t const &);
    /** @brief Allows to retrieve the number of optimization cycles without improvement of the primary fitness criterion */
    std::uint32_t getNStalls() const;

    /** @brief Retrieves an identifier for the current personality of this object */
    std::string getPersonality() const;

    /** @brief Allows to activate random crashes for debugging purposes */
    void setRandomCrash(bool, double);
    /** @brief Allows to check whether random crashes are activated, and with which probability the occur */
    std::tuple<bool, double> getRandomCrash() const;

    /***************************************************************************/
    /**
     * Retrieves a parameter of a given type at the specified position.
     */
    template <typename val_type>
    val_type getVarVal(std::tuple<std::size_t, std::string, std::size_t> const &target) {
        val_type result = val_type(0);

        if(typeid(val_type) == typeid(double)) {
            return Gem::Common::narrow<val_type>(
                std::any_cast<double>(this->getVarValImpl("d", target))
            );
        }
        if(typeid(val_type) == typeid(float)) {
            return Gem::Common::narrow<val_type>(
                std::any_cast<float>(this->getVarValImpl("f", target))
            );
        }
        if(typeid(val_type) == typeid(std::int32_t)) {
            return Gem::Common::narrow<val_type>(
                std::any_cast<std::int32_t>(this->getVarValImpl("i", target))
            );
        }
        if(typeid(val_type) == typeid(bool)) {
            return Gem::Common::narrow<val_type>(
                std::any_cast<bool>(this->getVarValImpl("b", target))
            );
        }
                    throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GOptimizableEntity::getVarVal<>(): Error!" << '\n'
                << "Received invalid type descriptor " << '\n'
            );


        return result;
    }

    /***************************************************************************/
    /**
     * The function converts the local personality base pointer to the desired type
     * and returns it for modification by the corresponding optimization algorithm.
     * The base algorithms have been declared "friend" of GOptimizableEntity and
     * can thus access this function. External entities have no need to do so. Note
     * that this function will only be accessible to the compiler if personality_type
     * is a derivative of GPersonalityTraits, thanks to the magic of std::enable_if
     * and type_traits.
     *
     * @return A std::shared_ptr converted to the desired target type
     */
    template <typename personality_type>
        requires std::derived_from<personality_type, GPersonalityTraits>
    std::shared_ptr<personality_type> getPersonalityTraits() {
#ifdef DEBUG
        // Check that the personality pointer actually points somewhere
        if(not aux_.personalityRef()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GOptimizableEntity::getPersonalityTraits<personality_type>() : Empty personality "
                   "pointer found"
                << '\n'
                << "This should not happen." << '\n'
            );

            // Make the compiler happy
            return std::shared_ptr<personality_type>();
        }
#endif /* DEBUG */

        // Does error checks on the conversion internally
        return Gem::Common::convertSmartPointer<GPersonalityTraits, personality_type>(aux_.personalityRef());
    }

    /* ----------------------------------------------------------------------------------
     * Tested in GTreeGenome::specificTestsNoFailureExpected_GUnitTests()
     * Tested in GTreeGenome::specificTestsFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /** @brief This function returns the current personality traits base pointer */
    std::shared_ptr<GPersonalityTraits> getPersonalityTraits();

    /** @brief Sets the current personality of this individual */
    void setPersonality(std::shared_ptr<GPersonalityTraits>);
    /** @brief Resets the current personality to PERSONALITY_NONE */
    void resetPersonality();
    /** @brief Clears all algorithm-scoped auxiliary scratch (personality + POD metadata blocks). Meant to be called at optimization-algorithm boundaries. */
    void clearOAScratch();

    /***************************************************************************/
    // Generic per-parameter / per-group OA metadata (DM §3c): an opaque, keyed store of POD blocks.
    // The optimization algorithm supplies the POD type; the individual treats the bytes as opaque, so
    // it needs no knowledge of what (e.g. EA Gauss state) is stored. See GAuxiliaryStore.

    /** @brief Installs (or replaces) a zero-initialised POD metadata block of record_count records under key */
    template <typename POD>
    void installAuxBlock(AuxKey key, std::size_t record_count, AuxScope scope = AuxScope::PerGroup) {
        aux_.installAuxBlock<POD>(key, record_count, scope);
    }
    /** @brief A typed view over the records of the POD metadata block under key */
    template <typename POD>
    std::span<POD> metaRecords(AuxKey key) {
        return aux_.metaRecords<POD>(key);
    }
    template <typename POD>
    std::span<const POD> metaRecords(AuxKey key) const {
        return aux_.metaRecords<POD>(key);
    }
    /** @brief Typed access to a per-individual (single-record) POD metadata block */
    template <typename POD>
    POD &metaScalar(AuxKey key) {
        return aux_.metaScalar<POD>(key);
    }
    /** @brief Whether a POD metadata block is installed under key */
    bool hasAux(AuxKey key) const {
        return aux_.hasAux(key);
    }

    /** @brief Retrieves the mnemonic used for the optimization of this object */
    std::string getMnemonic() const;

    /** @brief Check how valid a given solution is */
    double getValidityLevel() const;
    /** @brief Checks whether all constraints were fulfilled */
    bool constraintsFulfilled() const;
    /** @brief Allows to register a constraint with this individual */
    void
        registerConstraint(std::shared_ptr<GPreEvaluationValidityCheckT<GOptimizableEntity>>);

    /** @brief Allows to set the policy to use in case this individual represents an invalid solution */
    void setEvaluationPolicy(evaluationPolicy eval_policy);
    /** @brief Allows to retrieve the current policy in case this individual represents an invalid solution */
    evaluationPolicy getEvaluationPolicy() const;

    /** @brief Checks whether this is a valid solution; meant to be called for "clean" individuals only */
    bool isValid() const;
    /** @brief Checks whether this solution is invalid */
    bool isInValid() const;

    /** @brief Allows to set the globally best known primary fitness */
    void setBestKnownPrimaryFitness(std::tuple<double, double> const &);
    /** @brief Retrieves the value of the globally best known primary fitness */
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
    /** @brief Do the required processing for this object */
    void process_(
        const std::vector<individual_processing_result> &res_vec =
            std::vector<individual_processing_result>()
    ) final;

    /** @brief Adds local configuration options to a GParserBuilder object */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &) override;
    /** @brief Loads the data of another GOptimizableEntity */
    void load_(const GOptimizableEntity *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GOptimizableEntity>(
        GOptimizableEntity const &,
        GOptimizableEntity const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        GOptimizableEntity const & // the other object
        ,
        Gem::Common::expectation const & // the expectation for this object, e.g. equality
        ,
        double const & // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Random initialization */
    virtual bool randomInit_(activityMode const &) = 0;

    /* @brief The actual adaption operations. */
    virtual std::size_t customAdaptions() = 0;

    /** @brief The fitness calculation for the main quality criterion takes place here */
    double fitnessCalculation() override = 0;
    /** @brief Sets the fitness to a given set of values and clears the dirty flag */
    void setFitness_(std::vector<double> const &);

    /** @brief Combines secondary evaluation results by adding the individual results */
    double sumCombiner() const;
    /** @brief Combines secondary evaluation results by adding the absolute values of individual results */
    double fabsSumCombiner() const;
    /** @brief Combines secondary evaluation results by calculating the square root of the squared sum */
    double squaredSumCombiner() const;
    /** @brief Combines secondary evaluation results by calculation the square root of the weighed squared sum */
    double weighedSquaredSumCombiner(std::vector<double> const &) const;

    /** @brief Checks whether this solution has been rated to be valid; meant to be called by internal functions only */
    bool individualFulfillsConstraints(double &) const;

private:
    /***************************************************************************/
    // Overridden or virtual private functions

    /** @brief Emits a name for this class / object */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    GOptimizableEntity *clone_() const override = 0;

    /** @brief Retrieves the stored raw fitness with a given id */
    double raw_fitness_(std::size_t) const final;
    /** @brief Retrieves the stored transformed fitness with a given id */
    double transformed_fitness_(std::size_t) const final;

    /** @brief Returns all raw fitness results in a std::vector */
    std::vector<double> raw_fitness_vec_() const final;
    /** @brief Returns all transformed fitness results in a std::vector */
    std::vector<double> transformed_fitness_vec_() const final;

    /***************************************************************************/

    /** @brief Retrieves a parameter of a given type at the specified position (genome-specific dispatch) */
    virtual std::any
    getVarValImpl(const std::string &, const std::tuple<std::size_t, std::string, std::size_t> &target) = 0;

    /***************************************************************************/
    // Per-type genome value channels -- the non-template dispatch targets of the public
    // streamline<T>/assignValueVector<T>/countParameters<T>/boundaries<T> templates. Implemented by
    // the concrete genome (the tree iterates its parameter objects; a flat genome copies a channel
    // array). These are the entire seam that makes value access genome-agnostic.
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

    /** @brief  Allows to set all fitnesses to the same value (both raw and transformed values) */
    void setAllFitnessTo(double);

    /** @brief  Allows to set all fitnesses to the same value (raw and transformed values seperately) */
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
    /** @brief The per-individual auxiliary store -- holds the personality traits (and, later, the per-group POD adaptor scratch) */
    GAuxiliaryStore aux_;

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
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::Parameters::GOptimizableEntity)        // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Parameters::individual_processing_result) // NOLINT
/******************************************************************************/
