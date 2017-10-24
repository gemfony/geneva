/**
 * @file G_OA_SwarmAlgorithm.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here

// Boost headers go here


#ifndef G_OA_SWARMALGORITHM_HPP_
#define G_OA_SWARMALGORITHM_HPP_


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/G_OptimizationAlgorithm_Base.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/G_OptimizationAlgorithm_SwarmAlgorithm_PersonalityTraits.hpp"

#ifdef GEM_TESTING
#include "geneva/GTestIndividual1.hpp"
#endif /* GEM_TESTING */

namespace Gem {
namespace Geneva {


/******************************************************************************/
/**
 * The GSwarmAlgorithm class implements a swarm optimization algorithm, based on the infrastructure
 * provided by the G_OptimizationAlgorithm_Base class. Its population is based on a constant number
 * of neighborhoods, whose amount of members is allowed to vary. This happens so that late
 * arrivals in case of networked execution can still be integrated into later iterations.
 *
 * TODO: Mark checkpoints so the serialization mode can be determined automatically (e.g. using file extension ??)
 */
class GSwarmAlgorithm
	:public G_OptimizationAlgorithm_Base
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("G_OptimizationAlgorithm_Base", boost::serialization::base_object<G_OptimizationAlgorithm_Base>(*this))
		 & BOOST_SERIALIZATION_NVP(m_n_neighborhoods)
		 & BOOST_SERIALIZATION_NVP(m_default_n_neighborhood_members)
		 & BOOST_SERIALIZATION_NVP(m_n_neighborhood_members_vec)
		 & BOOST_SERIALIZATION_NVP(m_global_best_ptr)
		 & BOOST_SERIALIZATION_NVP(m_neighborhood_bests_vec)
		 & BOOST_SERIALIZATION_NVP(m_c_personal)
		 & BOOST_SERIALIZATION_NVP(m_c_neighborhood)
		 & BOOST_SERIALIZATION_NVP(m_c_global)
		 & BOOST_SERIALIZATION_NVP(m_c_velocity)
		 & BOOST_SERIALIZATION_NVP(m_update_rule)
		 & BOOST_SERIALIZATION_NVP(m_random_fill_up)
		 & BOOST_SERIALIZATION_NVP(m_repulsion_threshold)
		 & BOOST_SERIALIZATION_NVP(m_dbl_lower_parameter_boundaries_vec)
		 & BOOST_SERIALIZATION_NVP(m_dbl_upper_parameter_boundaries_vec)
		 & BOOST_SERIALIZATION_NVP(m_dbl_vel_max_vec)
		 & BOOST_SERIALIZATION_NVP(m_velocity_range_percentage);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief The default constructor */
	 G_API_GENEVA GSwarmAlgorithm();
	 /** @brief Initialization with neighborhood sizes and amount of individuals in each neighborhood */
	 G_API_GENEVA GSwarmAlgorithm(const std::size_t&, const std::size_t&);
	 /** @brief A standard copy constructor */
	 G_API_GENEVA GSwarmAlgorithm(const GSwarmAlgorithm&);
	 /** @brief The destructor */
	 virtual G_API_GENEVA ~GSwarmAlgorithm();

	 /** @brief The standard assignment operator */
	 G_API_GENEVA const GSwarmAlgorithm& operator=(const GSwarmAlgorithm&);

	 /** @brief Checks for equality with another GSwarmAlgorithm object */
	 virtual G_API_GENEVA bool operator==(const GSwarmAlgorithm&) const;
	 /** @brief Checks for inequality with another GSwarmAlgorithm object */
	 virtual G_API_GENEVA bool operator!=(const GSwarmAlgorithm&) const;

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 virtual G_API_GENEVA void compare(
		 const GObject& // the other object
		 , const Gem::Common::expectation& // the expectation for this object, e.g. equality
		 , const double& // the limit for allowed deviations of floating point types
	 ) const override;

	 /** @brief Resets the settings of this population to what was configured when the optimize()-call was issued */
	 virtual G_API_GENEVA void resetToOptimizationStart() override;

	 /** @brief Sets the number of neighborhoods and the number of members in them */
	 G_API_GENEVA void setSwarmSizes(std::size_t, std::size_t);

	 /** @brief Returns information about the type of optimization algorithm */
	 virtual G_API_GENEVA std::string getAlgorithmPersonalityType() const override;

	 /** @brief Allows to set a static multiplier for personal distances */
	 G_API_GENEVA void setCPersonal(double);
	 /** @brief Allows to retrieve the static multiplier for personal distances */
	 G_API_GENEVA double getCPersonal() const;

	 /** @brief Allows to set a static multiplier for neighborhood distances */
	 G_API_GENEVA void setCNeighborhood(double);
	 /** @brief Allows to retrieve the static multiplier for neighborhood distances */
	 G_API_GENEVA double getCNeighborhood() const;

	 /** @brief Allows to set a static multiplier for global distances */
	 G_API_GENEVA void setCGlobal(double);
	 /** @brief Allows to retrieve the static multiplier for global distances */
	 G_API_GENEVA double getCGlobal() const;

	 /** @brief Allows to set a static multiplier for velocities */
	 G_API_GENEVA void setCVelocity(double);
	 /** @brief Allows to retrieve the static multiplier for velocities */
	 G_API_GENEVA double getCVelocity() const;

	 /** @brief Allows to set the velocity range percentage */
	 G_API_GENEVA void setVelocityRangePercentage(double);
	 /** @brief Allows to retrieve the velocity range percentage */
	 G_API_GENEVA double getVelocityRangePercentage() const;

	 /** @brief Retrieves the number of neighborhoods */
	 G_API_GENEVA std::size_t getNNeighborhoods() const;
	 /** @brief Retrieves the default number of individuals in each neighborhood */
	 G_API_GENEVA std::size_t getDefaultNNeighborhoodMembers() const;
	 /** @brief Retrieves the current number of individuals in a given neighborhood */
	 G_API_GENEVA std::size_t getCurrentNNeighborhoodMembers(const std::size_t&) const;

	 /** @brief Allows to specify the update rule to be used by the swarm */
	 G_API_GENEVA void setUpdateRule(updateRule);
	 /** @brief Allows to retrieve the update rule currently used by the swarm */
	 G_API_GENEVA updateRule getUpdateRule() const;

	 /** @brief Allows to specify the number of stalls as of which the algorithm switches to repulsive mode */
	 G_API_GENEVA void setRepulsionThreshold(std::uint32_t);
	 /** @brief Allows to retrieve the number of stalls as of which the algorithm switches to repulsive mode */
	 G_API_GENEVA std::uint32_t getRepulsionThreshold() const;

	 /** @brief All individuals automatically added to a neighborhood will have equal value */
	 G_API_GENEVA void setNeighborhoodsEqualFillUp();
	 /** @brief All individuals automatically added to a neighborhood will have a random value */
	 G_API_GENEVA void setNeighborhoodsRandomFillUp(bool = true);
	 /** @brief Allows to check whether neighborhoods are filled up with random individuals */
	 G_API_GENEVA bool neighborhoodsFilledUpRandomly() const;

	 /** @brief Retrieves the number of processable items for the current iteration */
	 virtual G_API_GENEVA std::size_t getNProcessableItems() const override;

	 /** @brief Adds local configuration options to a GParserBuilder object */
	 virtual G_API_GENEVA void addConfigurationOptions (
		 Gem::Common::GParserBuilder& gpb
	 ) override;

	 /***************************************************************************/
	 /**
	  * Retrieves the best individual of a neighborhood and casts it to the desired type. Note that this
	  * function will only be accessible to the compiler if parameterset_type is a derivative of GParameterSet,
	  * thanks to the magic of std::enable_if and type_traits
	  *
	  * @param neighborhood The neighborhood, whose best individual should be returned
	  * @return A converted shared_ptr to the best individual of a given neighborhood
	  */
	 template <typename parameterset_type>
	 std::shared_ptr<parameterset_type> getBestNeighborhoodIndividual(
		 std::size_t neighborhood
		 , typename std::enable_if<std::is_base_of<GParameterSet, parameterset_type>::value>::type *dummy = nullptr
	 ){
#ifdef DEBUG
		 // Check that the neighborhood is in a valid range
		 if(neighborhood >= m_n_neighborhoods) {
			 glogger
				 << "In GSwarmAlgorithm::getBestNeighborhoodIndividual<>() : Error" << std::endl
				 << "Requested neighborhood which does not exist: " << neighborhood << " / " << m_n_neighborhoods << std::endl
				 << GEXCEPTION;

			 // Make the compiler happy
			 return std::shared_ptr<parameterset_type>();
		 }
#endif /* DEBUG */

		 // Does error checks on the conversion internally
		 return Gem::Common::convertSmartPointer<GParameterSet, parameterset_type>(m_neighborhood_bests_vec[neighborhood]);
	 }

	 /** @brief Emits a name for this class / object */
	 virtual G_API_GENEVA std::string name() const override;

protected:
	 /** @brief Loads the data of another population */
	 virtual G_API_GENEVA void load_(const GObject *) override;
	 /** @brief Creates a deep clone of this object */
	 virtual G_API_GENEVA GObject *clone_() const override;

	 /** @brief Does some preparatory work before the optimization starts */
	 virtual G_API_GENEVA void init() override;
	 /** @brief Does any necessary finalization work */
	 virtual G_API_GENEVA void finalize() override;

	 /** @brief Retrieve a GPersonalityTraits object belonging to this algorithm */
	 virtual G_API_GENEVA std::shared_ptr<GPersonalityTraits> getPersonalityTraits() const override;

	 /** @brief The actual business logic to be performed during each iteration; Returns the best achieved fitness */
	 virtual G_API_GENEVA std::tuple<double, double> cycleLogic() override;
	 /** @brief Fixes an incomplete population */
	 virtual G_API_GENEVA void adjustNeighborhoods() BASE;

	 /** @brief Checks whether each neighborhood has the default size */
	 G_API_GENEVA bool neighborhoodsHaveNominalValues() const;

	 /** @brief Updates the best individuals found */
	 virtual G_API_GENEVA std::tuple<double, double> findBests();
	 /** @brief Resizes the population to the desired level and does some error checks */
	 virtual G_API_GENEVA void adjustPopulation() override;

	 /** @brief Helper function that returns the id of the first individual of a neighborhood */
	 G_API_GENEVA std::size_t getFirstNIPos(const std::size_t&) const;
	 /** @brief Helper function that returns the id of the first individual of a neighborhood, using a vector of neighborhood sizes */
	 G_API_GENEVA std::size_t getFirstNIPosVec(const std::size_t&, const std::vector<std::size_t>&) const;
	 /** @brief Helper function that returns the id of the last individual of a neighborhood */
	 G_API_GENEVA std::size_t getLastNIPos(const std::size_t&) const;

	 /** @brief Triggers an update of an individual's positions */
	 G_API_GENEVA void updateIndividualPositions(
		 const std::size_t&
		 , std::shared_ptr<GParameterSet>
		 , std::shared_ptr<GParameterSet>
		 , std::shared_ptr<GParameterSet>
		 , std::shared_ptr<GParameterSet>
		 , std::tuple<double, double, double, double>
	 );

	 /** @brief Triggers an update of all individual's positions */
	 virtual G_API_GENEVA void updatePositions();

	 /** @brief Updates the fitness of all individuals */
	 virtual G_API_GENEVA void runFitnessCalculation() override;
	 /** @brief Adjusts the velocity vector so that its values don't exceed the allowed value range */
	 G_API_GENEVA void pruneVelocity(std::vector<double>&);

	 /** Updates the personal best of an individual */
	 G_API_GENEVA void updatePersonalBest(std::shared_ptr<GParameterSet>);
	 /** Updates the personal best of an individual, if a better solution was found */
	 G_API_GENEVA void updatePersonalBestIfBetter(std::shared_ptr<GParameterSet>);

	 /** @brief Returns the name of this optimization algorithm */
	 virtual G_API_GENEVA std::string getAlgorithmName() const override;

	 std::size_t m_n_neighborhoods = (DEFAULTNNEIGHBORHOODS ? DEFAULTNNEIGHBORHOODS : 1); ///< The number of neighborhoods in the population
	 std::size_t m_default_n_neighborhood_members = ((DEFAULTNNEIGHBORHOODMEMBERS <= 1) ? 2 : DEFAULTNNEIGHBORHOODMEMBERS); ///< The desired number of individuals belonging to each neighborhood
	 std::vector<std::size_t> m_n_neighborhood_members_vec = std::vector<std::size_t>(m_n_neighborhoods, 0); ///< The current number of individuals belonging to each neighborhood

	 std::shared_ptr<GParameterSet> m_global_best_ptr; ///< The globally best individual

	 std::vector<std::shared_ptr<GParameterSet>> m_neighborhood_bests_vec = std::vector<std::shared_ptr<GParameterSet>>(m_n_neighborhoods); ///< The collection of best individuals from each neighborhood
	 std::vector<std::shared_ptr<GParameterSet>> m_velocities_vec = std::vector<std::shared_ptr<GParameterSet>>(); ///< Holds velocities, as calculated in the previous iteration

	 double m_c_personal = DEFAULTCPERSONAL; ///< A factor for multiplication of personal best distances
	 double m_c_neighborhood = DEFAULTCNEIGHBORHOOD; ///< A factor for multiplication of neighborhood best distances
	 double m_c_global = DEFAULTCGLOBAL; ///< A factor for multiplication of global best distances
	 double m_c_velocity = DEFAULTCVELOCITY; ///< A factor for multiplication of velocities

	 updateRule m_update_rule = DEFAULTUPDATERULE; ///< Specifies how the parameters are updated
	 bool m_random_fill_up = true; ///< Specifies whether neighborhoods are filled up with random values

	 std::uint32_t m_repulsion_threshold = DEFREPULSIONTHRESHOLD; ///< The number of stalls until the swarm algorithm switches to repulsion instead of attraction

	 std::vector<double> m_dbl_lower_parameter_boundaries_vec = std::vector<double>(); ///< Holds lower boundaries of double parameters
	 std::vector<double> m_dbl_upper_parameter_boundaries_vec = std::vector<double>(); ///< Holds upper boundaries of double parameters
	 std::vector<double> m_dbl_vel_max_vec = std::vector<double>(); ///< Holds the maximum allowed values of double-type velocities

	 double m_velocity_range_percentage = DEFAULTVELOCITYRANGEPERCENTAGE; ///< Indicates the percentage of a value range used for the initialization of the velocity

	 std::vector<std::shared_ptr<GParameterSet>> m_last_iteration_individuals_vec; ///< A temporary copy of the last iteration's individuals

private:
	 /***************************************************************************/
	 /** @brief Helper function that checks the content of two nNeighborhoodMembers_ arrays */
	 bool nNeighborhoodMembersEqual(const std::vector<std::size_t>&, const std::vector<std::size_t>&) const;

	 /** @brief Small helper function that helps to fill up a neighborhood, if there is just one entry in it */
	 void fillUpNeighborhood1();

public:
	 /***************************************************************************/
	 /** @brief Applies modifications to this object. This is needed for testing purposes */
	 virtual G_API_GENEVA bool modify_GUnitTests() override;
	 /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	 virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() override;
	 /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	 virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() override;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GSwarmAlgorithm)

#endif /* G_OA_SWARMALGORITHM_HPP_ */
