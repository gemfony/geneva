/**
 * @file GIndividual.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

// Standard header files go here
#include <sstream>
#include <string>
#include <cmath>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost header files go here
#include <boost/variant.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>

#ifndef GINDIVIDUAL_HPP_
#define GINDIVIDUAL_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva header files go here
#include "GMutableI.hpp"
#include "GRateableI.hpp"
#include "GObject.hpp"
#include "GEAPersonalityTraits.hpp"
#include "GGDPersonalityTraits.hpp"
#include "GSwarmPersonalityTraits.hpp"
#include "GExceptions.hpp"

namespace Gem {
namespace GenEvA {

/**************************************************************************************************/
/**
 * This class acts as an interface class for all objects that can take part
 * in an evolutionary improvement. Such items must possess adaption functionality
 * and must know how to calculate their fitness. They also need the basic GObject
 * interface. In particular, they absolutely need to be serializable. As this library
 * was designed with particularly expensive evaluation calculations in mind, this
 * class also contains a framework for lazy evaluation, so not all evaluations take
 * place at the same time.
 */
class GIndividual
	:public GMutableI,
	 public GRateableI,
	 public GObject
{
	///////////////////////////////////////////////////////////////////////
	// Needed so only the corresponding optimization algorithms can set the
	// personality of an individual
	friend class GOptimizationAlgorithm;
	friend class GEvolutionaryAlgorithm;
	friend class GSwarm;

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
	     & BOOST_SERIALIZATION_NVP(currentFitness_)
	     & BOOST_SERIALIZATION_NVP(bestPastFitness_)
	     & BOOST_SERIALIZATION_NVP(nStalls_)
	     & BOOST_SERIALIZATION_NVP(dirtyFlag_)
	     & BOOST_SERIALIZATION_NVP(allowLazyEvaluation_)
	     & BOOST_SERIALIZATION_NVP(processingCycles_)
	     & BOOST_SERIALIZATION_NVP(maximize_)
	     & BOOST_SERIALIZATION_NVP(parentAlgIteration_)
	     & BOOST_SERIALIZATION_NVP(pers_)
	     & BOOST_SERIALIZATION_NVP(pt_ptr_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GIndividual();
	/** @brief The copy constructor */
	GIndividual(const GIndividual&);
	/** @brief The destructor */
	virtual ~GIndividual();

	/** @brief Checks for equality with another GIndividual object */
	bool operator==(const GIndividual&) const;
	/** @brief Checks for inequality with another GIndividual object */
	bool operator!=(const GIndividual&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Util::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief The adaption interface */
	virtual void adapt();
	/** @brief Calculate the fitness of this object */
	virtual double fitness();
	/** @brief Do the required processing for this object */
	bool process();
	/** @brief Do the required processing for this object and catch all exceptions */
	bool checkedProcess();
	/** @brief Allows to instruct this individual to perform multiple process operations in one go. */
	void setProcessingCycles(const boost::uint32_t&);
	/** @brief Retrieves the number of allowed processing cycles */
	boost::uint32_t getProcessingCycles() const;

	/** @brief Retrieve a value for this class and check for exceptions. Useful for threads */
	virtual double checkedFitness();
	/** @brief Retrieve the current (not necessarily up-to-date) fitness */
	double getCurrentFitness(bool&) const;
	/** @brief Enforce fitness calculation */
	double doFitnessCalculation();

	/** @brief Indicate whether lazy evaluation is allowed */
	bool setAllowLazyEvaluation(const bool&) ;
	/** @brief Retrieve the allowLazyEvaluation_ parameter */
	bool getAllowLazyEvaluation() const ;

	/** @brief Check whether the dirty flag is set */
	bool isDirty() const ;

	/** @brief Specify whether we want to work in maximization (true) or minimization (false) mode */
	void setMaxMode(const bool& mode);
	/** @brief Allows to retrieve the maximize_ parameter */
	bool getMaxMode() const;

	/** @brief Allows to set the current iteration of the parent optimization algorithm. */
	void setParentAlgIteration(const boost::uint32_t&);
	/** @brief Gives access to the parent optimization algorithm's iteration */
	boost::uint32_t getParentAlgIteration() const;

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

	/**********************************************************************/
	/** @brief Retrieves the current personality of this object */
	personality getPersonality() const;

	/**********************************************************************/
	/**
	 * The function converts the local personality to the desired type and
	 * returns it for modification by the corresponding optimization algorithm.
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
			std::ostringstream error;
			error << "In GIndividual::getPersonalityTraits<personality_type>() : Empty personality pointer found" << std::endl;
			throw geneva_error_condition(error.str());
		}

		boost::shared_ptr<personality_type> p = boost::dynamic_pointer_cast<personality_type>(pt_ptr_);

		if(p) return p;
		else {
			std::ostringstream error;
			error << "In GIndividual::getPersonalityTraits<personality_type>() : Conversion error" << std::endl;
			throw geneva_error_condition(error.str());
		}
#else
		return boost::static_pointer_cast<personality_type>(pt_ptr_);
#endif /* DEBUG */

	}

	/**************************************************************************************************/

	/** @brief This function returns the current personality traits base pointer */
	boost::shared_ptr<GPersonalityTraits> getPersonalityTraits();
	/** @brief This function returns the current evolutionary algorithm personality traits pointer */
	boost::shared_ptr<GEAPersonalityTraits> getEAPersonalityTraits();
	/** @brief This function returns the current gradient descent personality traits pointer */
	boost::shared_ptr<GGDPersonalityTraits> getGDPersonalityTraits();
	/** @brief This function returns the current swarm algorithm personality traits pointer */
	boost::shared_ptr<GSwarmPersonalityTraits> getSwarmPersonalityTraits();


	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();

protected:
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const = 0;
	/** @brief The actual fitness calculation takes place here */
	virtual double fitnessCalculation() = 0;
	/** @brief The actual adaption operations */
	virtual void customAdaptions() = 0;
	/** @brief Updates the object's structure and/or parameters, if the optimization has stalled */
	virtual bool customUpdateOnStall();

	/** @brief Sets the dirtyFlag_ */
	void setDirtyFlag() ;

	/** @brief Sets the current personality of this individual */
	void setPersonality(const personality&);
	/** @brief Resets the current personality to NONE */
	void resetPersonality();

private:
	/** @brief Sets the dirtyFlag_ to any desired value */
	bool setDirtyFlag(const bool&) ;

	/** @brief Holds this object's internal fitness */
    double currentFitness_;
    /** @brief Holds the globally best known fitness of all individuals */
    double bestPastFitness_;
    /** @brief The number of stalls in the entire set of individuals */
    boost::uint32_t nStalls_;
    /** @brief Internal representation of the adaption status of this object */
    bool dirtyFlag_;
    /** @brief Steers whether lazy evaluation is allowed */
    bool allowLazyEvaluation_;
    /** @brief The maximum number of processing cycles. 0 means "loop forever" (use with care!) */
    boost::uint32_t processingCycles_;
    /** @brief Indicates whether we are running in maximization or minimization mode */
    bool maximize_;
    /** @brief The iteration of the parent algorithm's optimization cycle */
    boost::uint32_t parentAlgIteration_;
    /** @brief Indicates the optimization algorithm the individual takes part in */
    personality pers_;
    /** @brief Holds the actual personality information */
    boost::shared_ptr<GPersonalityTraits> pt_ptr_;
};

} /* namespace GenEvA */
} /* namespace Gem */


/**************************************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::GenEvA::GIndividual)

/**************************************************************************************************/

#endif /* GINDIVIDUAL_HPP_ */
