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


// Standard header files go here

// Boost header files go here

#ifndef GOPTIMIZABLEENTITY_HPP_
#define GOPTIMIZABLEENTITY_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva header files go here
#include "common/GExceptions.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "common/GMathHelperFunctions.hpp"
#include "geneva/GenevaHelperFunctionsT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/GMutableI.hpp"
#include "geneva/GRateableI.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/GMultiConstraintT.hpp"

namespace Gem {
namespace Tests {
class GTestIndividual1; // forward declaration, needed for test purposes
} /* namespace Tests */
} /* namespace Gem */

namespace Gem {
namespace Geneva {

class GSerialSwarm; // forward declaration

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
	: public GMutableI
	, public GRateableI
	, public GObject
{
	friend class GSerialSwarm; ///< Needed so GSerialSwarm can set the dirty flag
	friend class Gem::Tests::GTestIndividual1; ///< Needed for testing purposes

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar
	  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
	  & BOOST_SERIALIZATION_NVP(currentFitness_)
	  & BOOST_SERIALIZATION_NVP(currentSecondaryFitness_)
	  & BOOST_SERIALIZATION_NVP(nFitnessCriteria_)
	  & BOOST_SERIALIZATION_NVP(bestPastFitness_)
	  & BOOST_SERIALIZATION_NVP(bestPastSecondaryFitness_)
	  & BOOST_SERIALIZATION_NVP(nStalls_)
	  & BOOST_SERIALIZATION_NVP(dirtyFlag_)
	  & BOOST_SERIALIZATION_NVP(serverMode_)
	  & BOOST_SERIALIZATION_NVP(maximize_)
	  & BOOST_SERIALIZATION_NVP(assignedIteration_)
	  & BOOST_SERIALIZATION_NVP(validityLevel_)
	  & BOOST_SERIALIZATION_NVP(pt_ptr_)
	  & BOOST_SERIALIZATION_NVP(evalPolicy_)
	  & BOOST_SERIALIZATION_NVP(individualConstraint_)
	  & BOOST_SERIALIZATION_NVP(steepness_)
	  & BOOST_SERIALIZATION_NVP(barrier_)
	  & BOOST_SERIALIZATION_NVP(worstKnownValid_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GOptimizableEntity();
	/** @brief The copy constructor */
	GOptimizableEntity(const GOptimizableEntity&);
	/** @brief The destructor */
	virtual ~GOptimizableEntity();

	/** @brief Checks for equality with another GOptimizableEntity object */
	bool operator==(const GOptimizableEntity&) const;
	/** @brief Checks for inequality with another GOptimizableEntity object */
	bool operator!=(const GOptimizableEntity&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
	) const OVERRIDE;

	/** @brief The adaption interface */
	virtual void adapt() OVERRIDE;

	/** @brief Calculates the result of the fitness function with id 0 */
	virtual double fitness() OVERRIDE;
	/** @brief Calculate or returns the result of a fitness function with a given id */
	virtual double fitness(const std::size_t&) OVERRIDE;
	/** @brief Adapts and evaluates the individual in one go */
	virtual double adaptAndEvaluate();

   /** @brief Retrieve a value for this class, taking into account invalid solutions; suitable for optimization algorithms only */
   virtual double oa_fitness() OVERRIDE;
   /** @brief Retrieve a value for this class for a given id, taking into account invalid solutions; suitable for optimization algorithms only */
   virtual double oa_fitness(const std::size_t&) OVERRIDE;

	/** @brief Retrieve the current (not necessarily up-to-date) fitness */
	double getCachedFitness(bool&, const std::size_t& = 0) const;
	/** @brief Enforce fitness calculation */
	double doFitnessCalculation();

	/** @brief Registers a new, secondary result value of the custom fitness calculation */
	void registerSecondaryResult(const double&);
	/** @brief Sets the number of fitness criteria to be used with this object */
	void setNumberOfFitnessCriteria(std::size_t);
	/** @brief Determines the overall number of fitness criteria present for this individual */
	std::size_t getNumberOfFitnessCriteria() const;
	/** @brief Determines the number of secondary fitness criteria present for this individual */
	std::size_t getNumberOfSecondaryFitnessCriteria() const;
	/** @brief Determines whether more than one fitness criterion is present for this individual */
	bool hasMultipleFitnessCriteria() const;

	/** @brief (De-)activates the server mode */
	bool setServerMode(const bool&);
	/** @brief Checks whether the server mode is set */
	bool getServerMode() const ;
	/** @brief Checks whether the server mode is set */
	bool serverMode() const;

	/** @brief Check whether the dirty flag is set */
	bool isDirty() const ;
   /** @brief Sets the dirtyFlag_ */
   void setDirtyFlag();

	/** @brief Allows to retrieve the maximize_ parameter */
	bool getMaxMode() const;
	/** @brief Retrieves the worst possible evaluation result, depending on whether we are in maximization or minimization mode */
	double getWorstCase() const;

	/** @brief Retrieves the steepness_ variable */
	double getSteepness() const;
	/** @brief Sets the steepness variable */
	void setSteepness(double);

	/** @brief Retrieves the barrier_ variable */
	double getBarrier() const;
	/** @brief Sets the barrier variable */
	void setBarrier(double);

	/** @brief Allows to set the current iteration of the parent optimization algorithm. */
	void setAssignedIteration(const boost::uint32_t&);
	/** @brief Gives access to the parent optimization algorithm's iteration */
	boost::uint32_t getAssignedIteration() const;

	/** @brief Allows to set the globally best known fitness */
	void setBestKnownFitness(const double&);
	/** @brief Retrieves the value of the globally best known fitness */
	double getBestKnownFitness() const;

	/** @brief Allows to specify the number of optimization cycles without improvement */
	void setNStalls(const boost::uint32_t&);
	/** @brief Allows to retrieve the number of optimization cycles without improvement */
	boost::uint32_t getNStalls() const;

	/** @brief Triggers updates when the optimization process has stalled */
	virtual bool updateOnStall();

	/** @brief Retrieves an identifier for the current personality of this object */
	std::string getPersonality() const;

	/** @brief Allows to randomly initialize parameter members */
	virtual void randomInit() = 0;

	/** @brief Retrieves a parameter of a given type at the specified position */
	virtual boost::any getVarVal(const std::string&, const std::size_t&) = 0;

   /***************************************************************************/
	/**
	 * Retrieves a parameter of a given type at the specified position
	 */
	template <typename val_type>
	val_type getVarVal(const boost::tuple<std::string,std::size_t>& target) {
	   val_type result = val_type(0);

      std::string ttype = boost::get<0>(target);
      std::size_t tpos  = boost::get<1>(target);

	   boost::any val = this->getVarVal(ttype, tpos);

	   if(ttype == "d") {
	      result = boost::numeric_cast<val_type>(boost::any_cast<double>(val));
	   } else if(ttype == "f") {
	      result = boost::numeric_cast<val_type>(boost::any_cast<float>(val));
	   } else if(ttype == "i") {
	      result = boost::numeric_cast<val_type>(boost::any_cast<boost::int32_t>(val));
	   } else if(ttype == "b") {
	      result = boost::numeric_cast<val_type>(boost::any_cast<bool>(val));
	   } else {
	      glogger
	      << "In GOptimizableEntity::getVarVal<>(): Error!" << std::endl
	      << "Received invalid type descriptor " << ttype << std::endl
	      << GEXCEPTION;
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
	 * is a derivative of GPersonalityTraits, thanks to the magic of Boost's
	 * enable_if and Type Traits libraries.
	 *
	 * @return A boost::shared_ptr converted to the desired target type
	 */
	template <typename personality_type>
	boost::shared_ptr<personality_type> getPersonalityTraits(
			typename boost::enable_if<boost::is_base_of<GPersonalityTraits, personality_type> >::type* dummy = 0
	) {
#ifdef DEBUG
      // Check that pt_ptr_ actually points somewhere
      if(!pt_ptr_) {
         glogger
         << "In GOptimizableEntity::getPersonalityTraits<personality_type>() : Empty personality pointer found" << std::endl
         << "This should not happen." << std::endl
         << GEXCEPTION;

         // Make the compiler happy
         return boost::shared_ptr<personality_type>();
      }
#endif /* DEBUG */

	   // Does error checks on the conversion internally
      return Gem::Common::convertSmartPointer<GPersonalityTraits, personality_type>(pt_ptr_);
	}

	/* ----------------------------------------------------------------------------------
	 * Tested in GOptimizableEntity::specificTestsNoFailureExpected_GUnitTests()
	 * Tested in GOptimizableEntity::specificTestsFailureExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/** @brief This function returns the current personality traits base pointer */
	boost::shared_ptr<GPersonalityTraits> getPersonalityTraits();

	/** @brief Sets the current personality of this individual */
	void setPersonality(
	      boost::shared_ptr<GPersonalityTraits>
	);
	/** @brief Resets the current personality to PERSONALITY_NONE */
	void resetPersonality();

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual void addConfigurationOptions(
		Gem::Common::GParserBuilder&
		, const bool&
	) OVERRIDE;

	/** @brief Allows to assign a name to the role of this individual(-derivative) */
	virtual std::string getIndividualCharacteristic() const = 0;

   /** @brief Emits a name for this class / object */
   virtual std::string name() const OVERRIDE;

   /** @brief Check how valid a given solution is */
   double getValidityLevel() const;
   /** @brief Allows to register a constraint with this individual */
   void registerConstraint(boost::shared_ptr<GValidityCheckT<GOptimizableEntity> >);

   /** @brief Allows to set the policy to use in case this individual represents an invalid solution */
   void setEvaluationPolicy(evaluationPolicy evalPolicy);
   /** @brief Allows to retrieve the current policy in case this individual represents an invalid solution */
   evaluationPolicy getEvaluationPolicy() const;

   /** @brief Checks whether this is a valid solution; meant to be called for "clean" individuals only */
   bool isValid() const;

   /** @brief Allows an optimization algorithm to set the worst known valid evaluation up to the current iteration */
   void setWorstKnownValid(double);
   /** @brief Allows to retrieve the worst known valid evaluation up to the current iteration, as set by an external optimization algorithm */
   double getWorstKnownValid() const;

protected:
	/***************************************************************************/
	/** @brief Loads the data of another GOptimizableEntity */
	virtual void load_(const GObject*) OVERRIDE;
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const = 0;

	/** @brief The fitness calculation for the main quality criterion takes place here */
	virtual double fitnessCalculation() = 0;
	/** @brief Sets the fitness to a given set of values and clears the dirty flag */
	void setFitness_(const double&, const std::vector<double>&);

	/** @brief The actual adaption operations */
	virtual void customAdaptions();
	/** @brief Updates the object's structure and/or parameters, if the optimization has stalled */
	virtual bool customUpdateOnStall();
	/** @brief Specify whether we want to work in maximization (true) or minimization (false) mode */
	void setMaxMode_(const bool&);
	/** @brief Sets the dirtyFlag_ to any desired value */
	bool setDirtyFlag(const bool&) ;

	/** @brief Combines secondary evaluation results by adding the individual results */
	double sumCombiner() const;
	/** @brief Combines secondary evaluation results by adding the absolute values of individual results */
	double fabsSumCombiner() const;
	/** @brief Combines secondary evaluation results by calculating the square root of the squared sum */
	double squaredSumCombiner() const;
	/** @brief Combines secondary evaluation results by calculation the square root of the weighed squared sum */
	double weighedSquaredSumCombiner(const std::vector<double>&) const;

private:
   /***************************************************************************/
   /** @brief Checks whether this solution has been rated to be valid; meant to be called by internal functions only */
   bool isValid_(double&) const;

	/***************************************************************************/
	/** @brief Holds this object's internal, primary fitness */
    double currentFitness_;
    /** @brief Holds this object's internal, secondary fitness values */
    std::vector<double> currentSecondaryFitness_;
    /** @brief The total number of fitness criteria */
    std::size_t nFitnessCriteria_;
    /** @brief Holds the globally best known fitness of all individuals */
    double bestPastFitness_;
    /** @brief Holds the globally best known secondary fitness values of all individuals */
    std::vector<double> bestPastSecondaryFitness_;
    /** @brief The number of stalls in the entire set of individuals */
    boost::uint32_t nStalls_;
    /** @brief Internal representation of the adaption status of this object */
    bool dirtyFlag_;
    /** @brief Prevents re-evaluation when set */
    bool serverMode_;
    /** @brief Indicates whether we are running in maximization or minimization mode */
    bool maximize_;
    /** @brief The iteration of the parent algorithm's optimization cycle */
    boost::uint32_t assignedIteration_;
    /** @brief Indicates how valid a given solution is */
    double validityLevel_;
    /** @brief Holds the actual personality information */
    boost::shared_ptr<GPersonalityTraits> pt_ptr_;

    /** @brief Specifies what to do when the individual is marked as invalid */
    evaluationPolicy evalPolicy_;
    /** @brief Determines the "steepness" of a sigmoid function used by optimization algorithms */
    double steepness_;
    /** @brief Determines the extreme values of a sigmoid function used by optimization algorithms */
    double barrier_;
    /** @brief The worst known evaluation up to the current iteration */
    double worstKnownValid_;

    /** @brief A constraint-check to be applied to one or more components of this individual */
    boost::shared_ptr<GValidityCheckT<GOptimizableEntity> > individualConstraint_;

    /***************************************************************************/
public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GOptimizableEntity)

/******************************************************************************/

#endif /* GOPTIMIZABLEENTITY_HPP_ */
