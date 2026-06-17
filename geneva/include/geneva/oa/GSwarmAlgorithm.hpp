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

// Standard headers go here
#include <concepts>

// Boost headers go here

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/oa/GOptimizationAlgorithmT.hpp"
#include "geneva/oa/GSwarmAlgorithm_PersonalityTraits.hpp"

#ifdef GEM_TESTING
#include "geneva/individuals/GTestIndividual1.hpp"
#endif /* GEM_TESTING */

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * The GSwarmAlgorithm class implements a swarm optimization algorithm, based on the infrastructure
 * provided by the GOptimizationAlgorithmBase class. Its population is based on a constant number
 * of neighborhoods, whose amount of members is allowed to vary. This happens so that late
 * arrivals in case of networked execution can still be integrated into later iterations.
 *
 * TODO: Mark checkpoints so the serialization mode can be determined automatically (e.g. using file extension ??)
 */
class GSwarmAlgorithm // NOLINT(cppcoreguidelines-special-member-functions)
  : public GOptimizationAlgorithmT<GSwarmAlgorithm> {
public:
    // Identifiers consumed by the GOptimizationAlgorithmT scaffold.
    static constexpr std::string_view oa_class_name = "GSwarmAlgorithm";
    static constexpr std::string_view oa_algorithm_name = "Swarm Algorithm";
    static constexpr std::string_view oa_personality_type = "PERSONALITY_SWARM";

private:
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /** @brief Single declaration of this class'es UNCONDITIONALLY-handled local data members.
     *
     * Only members whose load_() is a plain, unconditional assignment live here. The
     * conditionally-reconstructed members (n_neighborhoods_, n_neighborhood_members_cnt_,
     * neighborhood_bests_cnt_, global_best_ptr_) are handled in a manual tail in
     * serialize()/load_()/compare_() because their load_() depends on neighborhood count,
     * iteration state and per-element clone()/load(). */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("default_n_neighborhood_members_", default_n_neighborhood_members_),
            Gem::Common::make_member("c_personal_", c_personal_),
            Gem::Common::make_member("c_neighborhood_", c_neighborhood_),
            Gem::Common::make_member("c_global_", c_global_),
            Gem::Common::make_member("c_velocity_", c_velocity_),
            Gem::Common::make_member("update_rule_", update_rule_),
            Gem::Common::make_member("random_fill_up_", random_fill_up_),
            Gem::Common::make_member("repulsion_threshold_", repulsion_threshold_),
            Gem::Common::make_member("dbl_lower_parameter_boundaries_cnt_", dbl_lower_parameter_boundaries_cnt_),
            Gem::Common::make_member("dbl_upper_parameter_boundaries_cnt_", dbl_upper_parameter_boundaries_cnt_),
            Gem::Common::make_member("dbl_vel_max_cnt_", dbl_vel_max_cnt_),
            Gem::Common::make_member("velocity_range_percentage_", velocity_range_percentage_)
        );
    }
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("default_n_neighborhood_members_", default_n_neighborhood_members_),
            Gem::Common::make_member("c_personal_", c_personal_),
            Gem::Common::make_member("c_neighborhood_", c_neighborhood_),
            Gem::Common::make_member("c_global_", c_global_),
            Gem::Common::make_member("c_velocity_", c_velocity_),
            Gem::Common::make_member("update_rule_", update_rule_),
            Gem::Common::make_member("random_fill_up_", random_fill_up_),
            Gem::Common::make_member("repulsion_threshold_", repulsion_threshold_),
            Gem::Common::make_member("dbl_lower_parameter_boundaries_cnt_", dbl_lower_parameter_boundaries_cnt_),
            Gem::Common::make_member("dbl_upper_parameter_boundaries_cnt_", dbl_upper_parameter_boundaries_cnt_),
            Gem::Common::make_member("dbl_vel_max_cnt_", dbl_vel_max_cnt_),
            Gem::Common::make_member("velocity_range_percentage_", velocity_range_percentage_)
        );
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp("GOptimizationAlgorithmBase", boost::serialization::base_object<GOptimizationAlgorithmBase>(*this));
        // Unconditional members derived from the single localMembers() declaration ...
        Gem::Common::serialize_members(ar, this->localMembers());
        // ... and the manual tail for the conditionally-reconstructed members (kept as
        // separate NVPs, with the same names as before).
        ar & BOOST_SERIALIZATION_NVP(n_neighborhoods_) &
            BOOST_SERIALIZATION_NVP(n_neighborhood_members_cnt_) &
            BOOST_SERIALIZATION_NVP(global_best_ptr_) &
            BOOST_SERIALIZATION_NVP(neighborhood_bests_cnt_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GSwarmAlgorithm();
    /**
     * @brief Initialization with neighborhood sizes and amount of individuals in each neighborhood.
     *
     * The first (unnamed) argument is the number of neighborhoods, the second is the (default) number
     * of individuals in each neighborhood.
     */
    GSwarmAlgorithm(const std::size_t &, const std::size_t &);
    /**
     * @brief A standard copy constructor. The (unnamed) argument is the other GSwarmAlgorithm object to be copied.
     */
    GSwarmAlgorithm(const GSwarmAlgorithm &);
    /** @brief The destructor */
    ~GSwarmAlgorithm() override = default;

    /**
     * @brief Sets the number of neighborhoods and the number of members in them.
     *
     * The first (unnamed) argument is the desired number of neighborhoods, the second is the desired
     * number of members per neighborhood.
     */
    void setSwarmSizes(std::size_t, std::size_t);

    /**
     * @brief Allows to set a static multiplier for personal distances.
     *
     * The (unnamed) argument is the new value of the personal-distance multiplier (c_personal_).
     */
    void setCPersonal(double);
    /**
     * @brief Allows to retrieve the static multiplier for personal distances.
     * @return The current value of the personal-distance multiplier (c_personal_)
     */
    double getCPersonal() const;

    /**
     * @brief Allows to set a static multiplier for neighborhood distances.
     *
     * The (unnamed) argument is the new value of the neighborhood-distance multiplier (c_neighborhood_).
     */
    void setCNeighborhood(double);
    /**
     * @brief Allows to retrieve the static multiplier for neighborhood distances.
     * @return The current value of the neighborhood-distance multiplier (c_neighborhood_)
     */
    double getCNeighborhood() const;

    /**
     * @brief Allows to set a static multiplier for global distances.
     *
     * The (unnamed) argument is the new value of the global-distance multiplier (c_global_).
     */
    void setCGlobal(double);
    /**
     * @brief Allows to retrieve the static multiplier for global distances.
     * @return The current value of the global-distance multiplier (c_global_)
     */
    double getCGlobal() const;

    /**
     * @brief Allows to set a static multiplier for velocities.
     *
     * The (unnamed) argument is the new value of the velocity multiplier (c_velocity_).
     */
    void setCVelocity(double);
    /**
     * @brief Allows to retrieve the static multiplier for velocities.
     * @return The current value of the velocity multiplier (c_velocity_)
     */
    double getCVelocity() const;

    /**
     * @brief Allows to set the velocity range percentage.
     *
     * The (unnamed) argument is the new percentage of the parameter value range used when initializing
     * velocities (velocity_range_percentage_).
     */
    void setVelocityRangePercentage(double);
    /**
     * @brief Allows to retrieve the velocity range percentage.
     * @return The current velocity range percentage (velocity_range_percentage_)
     */
    double getVelocityRangePercentage() const;

    /**
     * @brief Retrieves the number of neighborhoods.
     * @return The number of neighborhoods in the population (n_neighborhoods_)
     */
    std::size_t getNNeighborhoods() const;
    /**
     * @brief Retrieves the default number of individuals in each neighborhood.
     * @return The default number of individuals per neighborhood (default_n_neighborhood_members_)
     */
    std::size_t getDefaultNNeighborhoodMembers() const;
    /**
     * @brief Retrieves the current number of individuals in a given neighborhood.
     *
     * The (unnamed) argument is the index of the neighborhood whose current member count is requested.
     * @return The current number of individuals in the requested neighborhood
     */
    std::size_t getCurrentNNeighborhoodMembers(const std::size_t &) const;

    /**
     * @brief Allows to specify the update rule to be used by the swarm.
     *
     * The (unnamed) argument is the new update rule (update_rule_) governing how positions are updated.
     */
    void setUpdateRule(updateRule);
    /**
     * @brief Allows to retrieve the update rule currently used by the swarm.
     * @return The update rule currently in use (update_rule_)
     */
    updateRule getUpdateRule() const;

    /**
     * @brief Allows to specify the number of stalls as of which the algorithm switches to repulsive mode.
     *
     * The (unnamed) argument is the new repulsion threshold (repulsion_threshold_); a value of 0 disables
     * the switch to repulsion.
     */
    void setRepulsionThreshold(std::uint32_t);
    /**
     * @brief Allows to retrieve the number of stalls as of which the algorithm switches to repulsive mode.
     * @return The current repulsion threshold (repulsion_threshold_)
     */
    std::uint32_t getRepulsionThreshold() const;

    /** @brief All individuals automatically added to a neighborhood will have equal value */
    void setNeighborhoodsEqualFillUp();
    /**
     * @brief All individuals automatically added to a neighborhood will have a random value.
     *
     * The (unnamed) argument indicates whether random fill-up should be enabled (defaults to true).
     */
    void setNeighborhoodsRandomFillUp(bool = true);
    /**
     * @brief Allows to check whether neighborhoods are filled up with random individuals.
     * @return true if neighborhoods are filled up with random individuals, false otherwise
     */
    bool neighborhoodsFilledUpRandomly() const;

    /***************************************************************************/
    /**
	  * Retrieves the best individual of a neighborhood and casts it to the desired type. The C++20
	  * `requires std::derived_from` constraint below makes this overload visible to the compiler only when
	  * individual_type is a derivative of GOptimizableEntity.
	  *
	  * @tparam individual_type The target type to which the best individual is cast (must derive from gen::GOptimizableEntity)
	  * @param neighborhood The neighborhood, whose best individual should be returned
	  * @return A converted shared_ptr to the best individual of a given neighborhood
	  */
    template <typename individual_type>
        requires std::derived_from<individual_type, gen::GOptimizableEntity>
    std::shared_ptr<individual_type> getBestNeighborhoodIndividual(std::size_t neighborhood) {
#ifdef DEBUG
        // Check that the neighborhood is in a valid range
        if(neighborhood >= n_neighborhoods_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GSwarmAlgorithm::getBestNeighborhoodIndividual<>() : Error" << '\n'
                << "Requested neighborhood which does not exist: " << neighborhood << " / "
                << n_neighborhoods_ << '\n'
            );
        }
#endif /* DEBUG */

        // Does error checks on the conversion internally
        return Gem::Common::convertSmartPointer<gen::GOptimizableEntity, individual_type>(
            neighborhood_bests_cnt_[neighborhood]
        );
    }

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /**
     * @brief Adds local configuration options to a GParserBuilder object.
     * @param gpb The GParserBuilder object to which configuration options should be added
     */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override;
    /**
     * @brief Loads the data of another population into this one.
     *
     * The (unnamed) argument is a pointer to another GSwarmAlgorithm object, camouflaged as a
     * GOptimizationAlgorithmBase, whose data is copied into this object.
     */
    void load_(const GOptimizationAlgorithmBase *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GSwarmAlgorithm>(
        GSwarmAlgorithm const &,
        GSwarmAlgorithm const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type.
     *
     * The three (unnamed) arguments are, in order: the other object to compare against (camouflaged as a
     * GOptimizationAlgorithmBase), the expectation for this object (e.g. equality), and the limit for
     * allowed deviations of floating point types.
     */
    void compare_(
        const GOptimizationAlgorithmBase & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Resets the settings of this population to what was configured when the optimize()-call was issued */
    void resetToOptimizationStart_() override;

    /** @brief Does some preparatory work before the optimization starts */
    void init() override;
    /** @brief Does any necessary finalization work */
    void finalize() override;

    /**
     * @brief Updates the best individuals found.
     * @return A tuple holding the best raw and transformed fitness values found
     */
    virtual std::tuple<double, double> findBests();
    /** @brief Triggers an update of all individual's positions */
    virtual void updatePositions();

    /** @brief Fixes an incomplete population */
    virtual void adjustNeighborhoods();

    // name_(), clone_(), getAlgorithmName_(), getAlgorithmPersonalityType_() and the GUnitTests
    // stubs are generated by the GOptimizationAlgorithmT scaffold from the oa_* identifiers above.

    /***************************************************************************/

    /**
     * @brief Checks whether each neighborhood has the default size.
     * @return true if every neighborhood currently holds its nominal (default) number of members, false otherwise
     */
    bool neighborhoodsHaveNominalValues() const;

    /**
     * @brief Helper function that returns the id of the first individual of a neighborhood.
     *
     * The (unnamed) argument is the index of the neighborhood whose first individual position is requested.
     * @return The position (index in the population) of the first individual of the given neighborhood
     */
    std::size_t getFirstNIPos(const std::size_t &) const;
    /**
     * @brief Helper function that returns the id of the first individual of a neighborhood, using a vector of neighborhood sizes.
     *
     * The first (unnamed) argument is the index of the neighborhood; the second is a vector holding the
     * sizes of all neighborhoods, used to compute the offset.
     * @return The position (index in the population) of the first individual of the given neighborhood
     */
    std::size_t
    getFirstNIPosVec(const std::size_t &, const std::vector<std::size_t> &) const;
    /**
     * @brief Helper function that returns the id of the last individual of a neighborhood.
     *
     * The (unnamed) argument is the index of the neighborhood whose last individual position is requested.
     * @return The position (index in the population) one past the last individual of the given neighborhood
     */
    std::size_t getLastNIPos(const std::size_t &) const;

    /**
     * @brief Triggers an update of an individual's positions.
     *
     * The (unnamed) arguments are, in order: the neighborhood index, the population slot being moved
     * (borrowed), the neighborhood-best individual, the global-best individual, and a tuple holding the
     * four multipliers (c_personal, c_neighborhood, c_global, c_velocity). The per-particle velocity now
     * lives on the slot's OA scratch.
     */
    void updateIndividualPositions(
        const std::size_t &,
        const std::unique_ptr<gen::GIndividualSlot> &, // the population slot being moved (borrowed)
        std::shared_ptr<gen::GOptimizableEntity>,      // neighborhood best
        std::shared_ptr<gen::GOptimizableEntity>,      // global best
        std::tuple<double, double, double, double>      // c_personal / c_neighborhood / c_global / c_velocity
    );                                                  // (velocity now lives on the slot's OA scratch)

    /**
     * @brief Adjusts the velocity vector so that its values don't exceed the allowed value range.
     *
     * The (unnamed) argument is the velocity vector to be pruned in place (clamped to the maximum allowed
     * per-component velocities).
     */
    void pruneVelocity(std::vector<double> &);

    /**
     * @brief Updates the personal best of an individual.
     *
     * The (unnamed) argument is the population slot whose personal best is unconditionally updated (borrowed).
     */
    void updatePersonalBest(const std::unique_ptr<gen::GIndividualSlot> &);
    /**
     * @brief Updates the personal best of an individual, if a better solution was found.
     *
     * The (unnamed) argument is the population slot whose personal best is updated only when the current
     * solution is better (borrowed).
     */
    void updatePersonalBestIfBetter(const std::unique_ptr<gen::GIndividualSlot> &);

    std::size_t n_neighborhoods_ =
        (DEFAULTNNEIGHBORHOODS ? DEFAULTNNEIGHBORHOODS
                               : 1); ///< The number of neighborhoods in the population
    std::size_t default_n_neighborhood_members_ =
        ((DEFAULTNNEIGHBORHOODMEMBERS <= 1)
             ? 2
             : DEFAULTNNEIGHBORHOODMEMBERS); ///< The desired number of individuals belonging to each neighborhood
    std::vector<std::size_t> n_neighborhood_members_cnt_ = std::vector<std::size_t>(
        n_neighborhoods_,
        0
    ); ///< The current number of individuals belonging to each neighborhood

    std::shared_ptr<gen::GOptimizableEntity> global_best_ptr_; ///< The globally best individual

    std::vector<std::shared_ptr<gen::GOptimizableEntity>> neighborhood_bests_cnt_ =
        std::vector<std::shared_ptr<gen::GOptimizableEntity>>(
            n_neighborhoods_
        ); ///< The collection of best individuals from each neighborhood
    // (Per-particle velocities now live on each GIndividualSlot's OA scratch as a POD double block --
    //  key AUXKEY_SWARM_VELOCITY in GSwarmAlgorithm.cpp -- not in a parallel vector here.)

    double c_personal_ =
        DEFAULTCPERSONAL; ///< A factor for multiplication of personal best distances
    double c_neighborhood_ =
        DEFAULTCNEIGHBORHOOD; ///< A factor for multiplication of neighborhood best distances
    double c_global_ = DEFAULTCGLOBAL; ///< A factor for multiplication of global best distances
    double c_velocity_ = DEFAULTCVELOCITY; ///< A factor for multiplication of velocities

    updateRule update_rule_ = DEFAULTUPDATERULE; ///< Specifies how the parameters are updated
    bool random_fill_up_ =
        true; ///< Specifies whether neighborhoods are filled up with random values

    std::uint32_t repulsion_threshold_ =
        DEFREPULSIONTHRESHOLD; ///< The number of stalls until the swarm algorithm switches to repulsion instead of attraction

    std::vector<double>
        dbl_lower_parameter_boundaries_cnt_; ///< Holds lower boundaries of double parameters
    std::vector<double>
        dbl_upper_parameter_boundaries_cnt_; ///< Holds upper boundaries of double parameters
    std::vector<double>
        dbl_vel_max_cnt_; ///< Holds the maximum allowed values of double-type velocities

    double velocity_range_percentage_ =
        DEFAULTVELOCITYRANGEPERCENTAGE; ///< Indicates the percentage of a value range used for the initialization of the velocity

    std::vector<std::shared_ptr<gen::GOptimizableEntity>>
        last_iteration_individuals_cnt_; ///< A temporary copy of the last iteration's individuals

private:
    /***************************************************************************/
    // Virtual or overridden private functions

    /**
     * @brief The actual business logic to be performed during each iteration.
     * @return A tuple holding the best raw and transformed fitness values achieved in this iteration
     */
    std::tuple<double, double> cycleLogic_() override;
    /** @brief Updates the fitness of all individuals */
    void runFitnessCalculation_() override;

    /**
     * @brief Retrieves the number of processable items for the current iteration.
     * @return The number of individuals that need to be (re-)evaluated in the current iteration
     */
    std::size_t getNProcessableItems_() const override;

    /**
     * @brief Retrieve a GPersonalityTraits object belonging to this algorithm.
     * @return A shared_ptr to a freshly created GSwarmAlgorithm_PersonalityTraits object
     */
    std::shared_ptr<GPersonalityTraits> getPersonalityTraits_() const override;
    /** @brief Gives individuals an opportunity to update their internal structures */
    void actOnStalls_() override;

    /** @brief Resizes the population to the desired level and does some error checks */
    void adjustPopulation_() override;

    /***************************************************************************/

    /**
     * @brief Helper function that checks the content of two nNeighborhoodMembers_ arrays.
     *
     * The two (unnamed) arguments are the two neighborhood-member-count vectors to be compared.
     * @return true if both vectors hold identical neighborhood member counts, false otherwise
     */
    bool nNeighborhoodMembersEqual(
        const std::vector<std::size_t> &,
        const std::vector<std::size_t> &
    ) const;

    /** @brief Small helper function that helps to fill up a neighborhood, if there is just one entry in it */
    void fillUpNeighborhood1();
};

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GSwarmAlgorithm) // NOLINT

