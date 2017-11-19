/**
 * @file GOptimizableEntity.hpp
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

// Standard header files go here
#include <typeinfo>
#include <limits>

// Boost header files go here
#include <boost/numeric/conversion/bounds.hpp>
#include <boost/serialization/split_member.hpp>

#ifndef GOPTIMIZABLEENTITY_HPP_
#define GOPTIMIZABLEENTITY_HPP_

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonMathHelperFunctions.hpp"
#include "common/GLockVarT.hpp"
#include "geneva/GenevaHelperFunctionsT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/G_Interface_Mutable.hpp"
#include "geneva/G_Interface_Rateable.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/GMultiConstraintT.hpp"

namespace Gem {
namespace Tests {
class GTestIndividual1; // forward declaration, needed for test purposes
} /* namespace Tests */
} /* namespace Gem */

namespace Gem {
namespace Geneva {

// Forward declarations
class GSerialSwarm;

/******************************************************************************/
/**
 * This class acts as an interface class for all objects that can take part
 * in an evolutionary improvement. Such items must possess adaption functionality
 * and must know how to calculate their fitness. They also need the basic GObject
 * interface. In particular, they absolutely need to be serializable. As this library
 * was designed with particularly expensive evaluation calculations in mind, this
 * class also contains a framework for lazy evaluation, so not all evaluations take
 * place at the same time.
 */
class GOptimizableEntity
	: public G_Interface_Mutable
	  , public G_Interface_Rateable
	  , public GObject
{
	 friend class GSerialSwarm; ///< Needed so GSerialSwarm can set the dirty flag
	 friend class Gem::Tests::GTestIndividual1; ///< Needed for testing purposes

	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int version){
		 using boost::serialization::make_nvp;

		 // Some preparation needed if this is a load operation.
		 // This is needed to work around a problem in Boost 1.58
		 if(Archive::is_loading::value) {
			 m_current_fitness_vec.clear();
			 m_worst_known_valids_vec.clear();
		 }

		 ar
		 & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
		 & BOOST_SERIALIZATION_NVP(m_n_fitness_criteria)
		 & BOOST_SERIALIZATION_NVP(m_best_past_primary_fitness)
		 & BOOST_SERIALIZATION_NVP(m_n_stalls)
		 & BOOST_SERIALIZATION_NVP(m_dirty_flag)
		 & BOOST_SERIALIZATION_NVP(m_maximize)
		 & BOOST_SERIALIZATION_NVP(m_assigned_iteration)
		 & BOOST_SERIALIZATION_NVP(m_validity_level)
		 & BOOST_SERIALIZATION_NVP(m_pt_ptr)
		 & BOOST_SERIALIZATION_NVP(m_eval_policy)
		 & BOOST_SERIALIZATION_NVP(m_individual_constraint_ptr)
		 & BOOST_SERIALIZATION_NVP(m_sigmoid_steepness)
		 & BOOST_SERIALIZATION_NVP(m_sigmoid_extremes)
		 & BOOST_SERIALIZATION_NVP(m_marked_as_invalid_by_user)
		 & BOOST_SERIALIZATION_NVP(m_max_unsuccessful_adaptions)
		 & BOOST_SERIALIZATION_NVP(m_max_retries_until_valid)
		 & BOOST_SERIALIZATION_NVP(m_n_adaptions)
		 & BOOST_SERIALIZATION_NVP(m_evaluation_id)
		 & BOOST_SERIALIZATION_NVP(m_current_fitness_vec)
		 & BOOST_SERIALIZATION_NVP(m_worst_known_valids_vec);
	 }

	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief The default constructor */
	 G_API_GENEVA GOptimizableEntity();
	 /** @brief The copy constructor */
	 G_API_GENEVA GOptimizableEntity(const GOptimizableEntity&);
	 /** @brief Initialization with the number of fitness criteria */
	 G_API_GENEVA GOptimizableEntity(const std::size_t&);
	 /** @brief The destructor */
	 virtual G_API_GENEVA ~GOptimizableEntity();

	 /** @brief The standard assignment operator */
	 G_API_GENEVA const GOptimizableEntity& operator=(const GOptimizableEntity&);

	 /** @brief Checks for equality with another GOptimizableEntity object */
	 virtual G_API_GENEVA bool operator==(const GOptimizableEntity&) const;
	 /** @brief Checks for inequality with another GOptimizableEntity object */
	 virtual G_API_GENEVA bool operator!=(const GOptimizableEntity&) const;

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 virtual G_API_GENEVA void compare(
		 const GObject& // the other object
		 , const Gem::Common::expectation& // the expectation for this object, e.g. equality
		 , const double& // the limit for allowed deviations of floating point types
	 ) const override;

	 /** @brief The adaption interface */
	 virtual G_API_GENEVA std::size_t adapt() override;

	 /** @brief Returns the raw result of the fitness function with id 0 */
	 virtual G_API_GENEVA double fitness() const override;
	 /** @brief Returns the raw result of a fitness function with a given id */
	 virtual G_API_GENEVA double fitness(const std::size_t&) const override;

	 /** @brief Calculate or returns the result of a fitness function with a given id */
	 virtual G_API_GENEVA double fitness(const std::size_t&, bool, bool) override;
	 /** @brief Calculate or returns the result of a fitness function with a given id */
	 virtual G_API_GENEVA double fitness(const std::size_t&, bool, bool) const override;

	 /** @brief Returns the transformed result of the fitness function with id 0 */
	 virtual G_API_GENEVA double transformedFitness() const override;
	 /** @brief Returns the transformed result of a fitness function with a given id */
	 virtual G_API_GENEVA double transformedFitness(const std::size_t&) const override;

	 /** @brief Returns a fitness targetted at optimization algorithms, taking into account maximization and minimization */
	 virtual G_API_GENEVA double minOnly_fitness() const override;
	 /** @brief Returns a fitness targetted at optimization algorithms, taking into account maximization and minimization */
	 virtual G_API_GENEVA double minOnly_fitness(const std::size_t&) const override;

	 /** @brief Returns all raw fitness results in a std::vector */
	 virtual G_API_GENEVA std::vector<double> fitnessVec() const override;
	 /** @brief Returns all raw or transformed results in a std::vector */
	 virtual G_API_GENEVA std::vector<double> fitnessVec(bool) const override;
	 /** @brief Returns all transformed fitness results in a std::vector */
	 virtual G_API_GENEVA std::vector<double> transformedFitnessVec() const override;

	 /** @brief A wrapper for the non-const fitness function, so we can bind to it */
	 G_API_GENEVA double nonConstFitness(const std::size_t&, bool, bool);
	 /** @brief A wrapper for the const fitness function, so we can bind to it */
	 G_API_GENEVA double constFitness(const std::size_t&, bool, bool) const;

	 /** @brief Retrieve the current (not necessarily up-to-date) fitness */
	 G_API_GENEVA double getCachedFitness(const std::size_t& = 0, const bool& = USETRANSFORMEDFITNESS) const;

	 /** @brief Enforce fitness (re-)calculation */
	 G_API_GENEVA void enforceFitnessUpdate(std::function<std::vector<double>()> =  std::function<std::vector<double>()>());

	 /** @brief Registers a new, secondary result value of the custom fitness calculation */
	 G_API_GENEVA void registerSecondaryResult(const std::size_t&, const double&);
	 /** @brief Determines the overall number of fitness criteria present for this individual */
	 G_API_GENEVA std::size_t getNumberOfFitnessCriteria() const;
	 /** @brief Allows to reset the number of fitness criteria */
	 G_API_GENEVA void setNumberOfFitnessCriteria(std::size_t);
	 /** @brief Determines whether more than one fitness criterion is present for this individual */
	 G_API_GENEVA bool hasMultipleFitnessCriteria() const;

	 /** @brief Checks the worst fitness and updates it when needed */
	 G_API_GENEVA void challengeWorstValidFitness(std::tuple<double, double>&, const std::size_t&);
	 /** @brief Retrieve the fitness tuple at a given evaluation position */
	 G_API_GENEVA std::tuple<double,double> getFitnessTuple(const std::uint32_t& = 0) const;

	 /** @brief Check whether this individual is "clean", i.e neither "dirty" nor has a delayed evaluation */
	 G_API_GENEVA bool isClean() const;
	 /** @brief Check whether the dirty flag is set */
	 G_API_GENEVA bool isDirty() const ;
	 /** @brief Sets the dirtyFlag_ */
	 G_API_GENEVA void setDirtyFlag();
	 /** @brief Checks whether evaluation was delayed */
	 G_API_GENEVA bool evaluationDelayed() const;

	 /** @brief Allows to retrieve the maximize_ parameter */
	 G_API_GENEVA bool getMaxMode() const;

	 /** @brief Retrieves the worst possible evaluation result, depending on whether we are in maximization or minimization mode */
	 virtual G_API_GENEVA double getWorstCase() const BASE;
	 /** @brief Retrieves the best possible evaluation result, depending on whether we are in maximization or minimization mode */
	 virtual G_API_GENEVA double getBestCase() const BASE;

	 /** @brief Retrieves the steepness_ variable (used for the sigmoid transformation) */
	 G_API_GENEVA double getSteepness() const;
	 /** @brief Sets the steepness variable (used for the sigmoid transformation) */
	 G_API_GENEVA void setSteepness(double);

	 /** @brief Retrieves the barrier_ variable (used for the sigmoid transformation) */
	 G_API_GENEVA double getBarrier() const;
	 /** @brief Sets the barrier variable (used for the sigmoid transformation) */
	 G_API_GENEVA void setBarrier(double);

	 /** @brief Sets the maximum number of calls to customAdaptions() that may pass without actual modifications */
	 G_API_GENEVA void setMaxUnsuccessfulAdaptions(std::size_t);
	 /** @brief Retrieves the maximum number of calls to customAdaptions that may pass without actual modifications */
	 G_API_GENEVA std::size_t getMaxUnsuccessfulAdaptions() const;

	 /** @brief Set maximum number of retries until a valid individual was found  */
	 G_API_GENEVA void setMaxRetriesUntilValid(std::size_t maxRetriesUntilValid);
	 /** Retrieves the maximum number of retries until a valid individual was found. */
	 G_API_GENEVA std::size_t getMaxRetriesUntilValid() const;

	 /** @brief Retrieves the number of adaptions performed during the last call to adapt() */
	 G_API_GENEVA std::size_t getNAdaptions() const;

	 /** @brief Allows to set the current iteration of the parent optimization algorithm. */
	 G_API_GENEVA void setAssignedIteration(const std::uint32_t&);
	 /** @brief Gives access to the parent optimization algorithm's iteration */
	 G_API_GENEVA std::uint32_t getAssignedIteration() const;

	 /** @brief Allows to specify the number of optimization cycles without improvement of the primary fitness criterion */
	 G_API_GENEVA void setNStalls(const std::uint32_t&);
	 /** @brief Allows to retrieve the number of optimization cycles without improvement of the primary fitness criterion */
	 G_API_GENEVA std::uint32_t getNStalls() const;

	 /** @brief Retrieves an identifier for the current personality of this object */
	 G_API_GENEVA std::string getPersonality() const;

	 /** @brief Allows to randomly initialize parameter members */
	 virtual G_API_GENEVA bool randomInit(const activityMode&) BASE = 0;

	 /** @brief Retrieves a parameter of a given type at the specified position */
	 virtual G_API_GENEVA boost::any getVarVal(
		 const std::string&
		 , const std::tuple<std::size_t, std::string, std::size_t>& target
	 ) = 0;

	 /***************************************************************************/
	 /**
	  * Retrieves a parameter of a given type at the specified position.
	  */
	 template <typename val_type>
	 val_type getVarVal(
		 const std::tuple<std::size_t, std::string, std::size_t>& target
	 ) {
		 val_type result = val_type(0);

		 if(typeid(val_type) == typeid(double)) {
			 return boost::numeric_cast<val_type>(boost::any_cast<double>(this->getVarVal("d", target)));
		 } else if(typeid(val_type) == typeid(float)) {
			 return boost::numeric_cast<val_type>(boost::any_cast<float>(this->getVarVal("f", target)));
		 } if(typeid(val_type) == typeid(std::int32_t)) {
			 return boost::numeric_cast<val_type>(boost::any_cast<std::int32_t>(this->getVarVal("i", target)));
		 } if(typeid(val_type) == typeid(bool)) {
			 return boost::numeric_cast<val_type>(boost::any_cast<bool>(this->getVarVal("b", target)));
		 } else {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GOptimizableEntity::getVarVal<>(): Error!" << std::endl
					 << "Received invalid type descriptor " << std::endl
			 );
		 }

		 return result;
	 }

	 /***************************************************************************/
	 /**
	  * The function converts the local personality base pointer to the desired type
	  * and returns it for modification by the corresponding optimization algorithm.
	  * The base algorithms have been declared "friend" of GParameterSet and
	  * can thus access this function. External entities have no need to do so. Note
	  * that this function will only be accessible to the compiler if personality_type
	  * is a derivative of GPersonalityTraits, thanks to the magic of std::enable_if
	  * and type_traits.
	  *
	  * @return A std::shared_ptr converted to the desired target type
	  */
	 template <typename personality_type>
	 std::shared_ptr<personality_type> getPersonalityTraits(
		 typename std::enable_if<std::is_base_of<GPersonalityTraits, personality_type>::value>::type *dummy = nullptr
	 ) {
#ifdef DEBUG
		 // Check that m_pt_ptr actually points somewhere
		 if(!m_pt_ptr) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GOptimizableEntity::getPersonalityTraits<personality_type>() : Empty personality pointer found" << std::endl
					 << "This should not happen." << std::endl
			 );

			 // Make the compiler happy
			 return std::shared_ptr<personality_type>();
		 }
#endif /* DEBUG */

		 // Does error checks on the conversion internally
		 return Gem::Common::convertSmartPointer<GPersonalityTraits, personality_type>(m_pt_ptr);
	 }

	 /* ----------------------------------------------------------------------------------
	  * Tested in GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests()
	  * Tested in GOptimizableEntity::specificTestsFailureExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

	 /***************************************************************************/
	 /** @brief This function returns the current personality traits base pointer */
	 G_API_GENEVA std::shared_ptr<GPersonalityTraits> getPersonalityTraits();

	 /** @brief Sets the current personality of this individual */
	 G_API_GENEVA void setPersonality(
		 std::shared_ptr<GPersonalityTraits>
	 );
	 /** @brief Resets the current personality to PERSONALITY_NONE */
	 G_API_GENEVA void resetPersonality();
	 /** @brief Retrieves the mnemonic used for the optimization of this object */
	 G_API_GENEVA std::string getMnemonic() const;

	 /** @brief Adds local configuration options to a GParserBuilder object */
	 virtual G_API_GENEVA void addConfigurationOptions(
		 Gem::Common::GParserBuilder&
	 ) override;

	 /** @brief Allows to assign a name to the role of this individual(-derivative) */
	 virtual G_API_GENEVA std::string getIndividualCharacteristic() const = 0;

	 /** @brief Emits a name for this class / object */
	 virtual G_API_GENEVA std::string name() const override;

	 /** @brief Check how valid a given solution is */
	 G_API_GENEVA double getValidityLevel() const;
	 /** @brief Checks whether all constraints were fulfilled */
	 G_API_GENEVA bool constraintsFulfilled() const;
	 /** @brief Allows to register a constraint with this individual */
	 G_API_GENEVA void registerConstraint(std::shared_ptr<GPreEvaluationValidityCheckT<GOptimizableEntity>>);

	 /** @brief Allows to set the policy to use in case this individual represents an invalid solution */
	 G_API_GENEVA void setEvaluationPolicy(evaluationPolicy evalPolicy);
	 /** @brief Allows to retrieve the current policy in case this individual represents an invalid solution */
	 G_API_GENEVA evaluationPolicy getEvaluationPolicy() const;

	 /** @brief Checks whether this is a valid solution; meant to be called for "clean" individuals only */
	 G_API_GENEVA bool isValid() const;
	 /** @brief Checks whether this solution is invalid */
	 G_API_GENEVA bool isInValid() const;

	 /** @brief Allows an optimization algorithm to set the worst known valid evaluation up to the current iteration */
	 G_API_GENEVA void setWorstKnownValid(const std::vector<std::tuple<double, double>>&);
	 /** @brief Allows to retrieve the worst known valid evaluation up to the current iteration, as set by an external optimization algorithm */
	 G_API_GENEVA std::tuple<double, double> getWorstKnownValid(const std::uint32_t&) const;
	 /** @brief Allows to retrieve all worst known valid evaluations up to the current iteration, as set by an external optimization algorithm */
	 G_API_GENEVA std::vector<std::tuple<double, double>> getWorstKnownValids() const;
	 /** @brief Fills the worstKnownValid-vector with best values */
	 G_API_GENEVA void populateWorstKnownValid();

	 /** @brief Triggers an update of the internal evaluation, if necessary */
	 G_API_GENEVA void postEvaluationUpdate();

	 /** @brief Allows to set the globally best known primary fitness */
	 G_API_GENEVA void setBestKnownPrimaryFitness(const std::tuple<double, double>&);
	 /** @brief Retrieves the value of the globally best known primary fitness */
	 G_API_GENEVA std::tuple<double, double> getBestKnownPrimaryFitness() const;

	 /** @brief Retrieve the id assigned to the current evaluation */
	 G_API_GENEVA std::string getCurrentEvaluationID() const;

	 /** @brief Checks whether a new solution is worse then an older solution, depending on the maxMode */
	 virtual G_API_GENEVA bool isWorse(double, const double&) const BASE;
	 /** @brief Checks whether a new solution is better then an older solution, depending on the maxMode */
	 virtual G_API_GENEVA bool isBetter(double, const double&) const BASE;

	 /** @brief Checks whether this object is better than the argument, depending on the maxMode */
	 G_API_GENEVA bool isBetterThan(std::shared_ptr<GOptimizableEntity>) const;
	 /** @brief Checks whether this object is worse than the argument, depending on the maxMode */
	 G_API_GENEVA bool isWorseThan(std::shared_ptr<GOptimizableEntity>) const;

	 /***************************************************************************/
	 /**
	  * Checks if a given position of a std::tuple is better then another,
	  * depending on our maximization mode
	  */
	 template <std::size_t pos>
	 bool isWorse(
		 std::tuple<double, double> newValue
		 , std::tuple<double, double> oldValue
	 ) const {
		 if(this->getMaxMode()) {
			 return (std::get<pos>(newValue) < std::get<pos>(oldValue));
		 } else { // minimization
			 return (std::get<pos>(newValue) > std::get<pos>(oldValue));
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Checks if a given position of a std::tuple is better then another,
	  * depending on our maximization mode
	  */
	 template <std::size_t pos>
	 bool isBetter(
		 std::tuple<double, double> newValue
		 , std::tuple<double, double> oldValue
	 ) const {
		 if(this->getMaxMode()) {
			 return (std::get<pos>(newValue) > std::get<pos>(oldValue));
		 } else { // minimization
			 return (std::get<pos>(newValue) < std::get<pos>(oldValue));
		 }
	 }

protected:
	 /***************************************************************************/
	 /** @brief Loads the data of another GOptimizableEntity */
	 virtual G_API_GENEVA void load_(const GObject*) override;

	 /** @brief The fitness calculation for the main quality criterion takes place here */
	 virtual G_API_GENEVA double fitnessCalculation() BASE = 0;
	 /** @brief Sets the fitness to a given set of values and clears the dirty flag */
	 G_API_GENEVA void setFitness_(const std::vector<double>&);

	 /** @brief The actual adaption operations */
	 virtual G_API_GENEVA std::size_t customAdaptions() BASE;
	 /** @brief Specify whether we want to work in maximization (true) or minimization (false) mode */
	 G_API_GENEVA void setMaxMode_(const bool&);
	 /** @brief Sets the dirtyFlag_ to any desired value */
	 G_API_GENEVA boost::logic::tribool setDirtyFlag(const boost::logic::tribool&) ;

	 /** @brief Combines secondary evaluation results by adding the individual results */
	 G_API_GENEVA double sumCombiner() const;
	 /** @brief Combines secondary evaluation results by adding the absolute values of individual results */
	 G_API_GENEVA double fabsSumCombiner() const;
	 /** @brief Combines secondary evaluation results by calculating the square root of the squared sum */
	 G_API_GENEVA double squaredSumCombiner() const;
	 /** @brief Combines secondary evaluation results by calculation the square root of the weighed squared sum */
	 G_API_GENEVA double weighedSquaredSumCombiner(const std::vector<double>&) const;

	 /** @brief Allows users to mark this solution as invalid in derived classes (usually from within the evaluation function) */
	 G_API_GENEVA void markAsInvalid();
	 /** @brief Allows to check whether this solution was marked as invalid */
	 G_API_GENEVA bool markedAsInvalidByUser() const;

	 /** @brief Checks whether this solution has been rated to be valid; meant to be called by internal functions only */
	 G_API_GENEVA bool parameterSetFulfillsConstraints(double&) const;

private:
	 /***************************************************************************/
	 /** @brief Creates a deep clone of this object */
	 virtual G_API_GENEVA GObject* clone_() const override = 0;

	 /***************************************************************************/
	 /** @brief Checks whether all results are at the worst possible value */
	 bool allRawResultsAtWorst() const;

	 /***************************************************************************/
	 /** @brief The total number of fitness criteria */
	 std::size_t m_n_fitness_criteria = 1;
	 /** @brief Holds this object's internal, raw and transformed fitness */
	 std::vector<std::tuple<double, double>> m_current_fitness_vec;

	 /** @brief The worst known evaluation up to the current iteration */
	 std::vector<std::tuple<double, double>> m_worst_known_valids_vec;
	 /** @brief Indicates whether the user has marked this solution as invalid inside of the evaluation function */
	 Gem::Common::GLockVarT<bool> m_marked_as_invalid_by_user;

	 /** @brief Holds the globally best known primary fitness of all individuals */
	 std::tuple<double, double> m_best_past_primary_fitness;
	 /** @brief The number of stalls of the primary fitness criterion in the entire set of individuals */
	 std::uint32_t m_n_stalls = 0;
	 /** @brief Internal representation of the adaption status of this object */
	 boost::logic::tribool m_dirty_flag = true; // boost::logic::indeterminate refers to "delayed evaluation"
	 /** @brief Indicates whether we are running in maximization or minimization mode */
	 bool m_maximize = false;
	 /** @brief The iteration of the parent algorithm's optimization cycle */
	 std::uint32_t m_assigned_iteration = 0;
	 /** @brief Indicates how valid a given solution is */
	 double m_validity_level = 0.;
	 /** @brief Holds the actual personality information */
	 std::shared_ptr<GPersonalityTraits> m_pt_ptr;

	 /** @brief Specifies what to do when the individual is marked as invalid */
	 evaluationPolicy m_eval_policy = Gem::Geneva::evaluationPolicy::USESIMPLEEVALUATION;
	 /** @brief Determines the "steepness" of a sigmoid function used by optimization algorithms */
	 double m_sigmoid_steepness = Gem::Geneva::FITNESSSIGMOIDSTEEPNESS;
	 /** @brief Determines the extreme values of a sigmoid function used by optimization algorithms */
	 double m_sigmoid_extremes = Gem::Geneva::WORSTALLOWEDVALIDFITNESS;

	 /** @brief A constraint-check to be applied to one or more components of this individual */
	 std::shared_ptr<GPreEvaluationValidityCheckT<GOptimizableEntity>> m_individual_constraint_ptr;

	 std::size_t m_max_unsuccessful_adaptions = Gem::Geneva::DEFMAXUNSUCCESSFULADAPTIONS; ///< The maximum number of calls to customAdaptions() in a row without actual modifications
	 std::size_t m_max_retries_until_valid = Gem::Geneva::DEFMAXRETRIESUNTILVALID; ///< The maximum number an adaption of an individual should be performed until a valid parameter set was found
	 std::size_t m_n_adaptions = 0; ///< Stores the actual number of adaptions after a call to "adapt()"

	 /** @brief A unique id that is assigned to an evaluation */
	 std::string m_evaluation_id = "empty";

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

/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GOptimizableEntity)

/******************************************************************************/

#endif /* GOPTIMIZABLEENTITY_HPP_ */
