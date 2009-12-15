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
#include <map>
#include <cmath>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost header files go here
#include <boost/variant.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>


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
#include "GenevaExceptions.hpp"

namespace Gem {
namespace GenEvA {

/**************************************************************************************************/
/**
 * This class acts as an interface class for all objects that can take part
 * in an evolutionary improvement. Such items must possess mutation functionality
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
	// Friend declarations needed so we can keep interaction with personality
	// information private.
	friend class GBasePopulation;

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;
	  ar & make_nvp("GObject", boost::serialization::base_object<GObject>(*this));
	  ar & make_nvp("currentFitness_",currentFitness_);
	  ar & make_nvp("dirtyFlag_",dirtyFlag_);
	  ar & make_nvp("allowLazyEvaluation_",allowLazyEvaluation_);
	  ar & make_nvp("parentPopGeneration_",parentPopGeneration_);
	  ar & make_nvp("parentCounter_",parentCounter_);
	  ar & make_nvp("popPos_",popPos_);
	  ar & make_nvp("attributeTable_",attributeTable_);
	  ar & make_nvp("processingCycles_", processingCycles_);
	  ar & make_nvp("maximize_", maximize_);
	  ar & make_nvp("pers_", pers_);
	  ar & make_nvp("pt_ptr_", pt_ptr_);
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
	/** @brief Checks for equality with another GIndividual object */
	virtual bool isEqualTo(const GObject&, const boost::logic::tribool& expected = boost::logic::indeterminate) const;
	/** @brief Checks for similarity with another GIndividual object */
	virtual bool isSimilarTo(const GObject&, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const;

	/** @brief Creates a deep clone of this object */
	virtual GObject* clone() const = 0;
	/** @brief Loads the data of another GObject */
	virtual void load(const GObject*);

	/** @brief The mutate interface */
	virtual void mutate();
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

	/** @brief Sets the parentPopGeneration_ parameter */
	void setParentPopGeneration(const boost::uint32_t&) ;
	/** @brief Retrieve the parentPopGeneration_ parameter */
	boost::uint32_t getParentPopGeneration() const ;

	/** @brief Sets the position of the individual in the population */
	void setPopulationPosition(std::size_t) ;
	/** @brief Retrieves the position of the individual in the population */
	std::size_t getPopulationPosition(void) const ;

	/** @brief Checks whether this is a parent individual */
	bool isParent() const ;
	/** @brief Retrieves the current value of the parentCounter_ variable */
	boost::uint32_t getParentCounter() const ;

	/** @brief Marks an individual as a parent*/
	bool setIsParent();
	/** @brief Marks an individual as a child */
	bool setIsChild();

	/** @brief Specify whether we want to work in maximization (true) or minimization (false) mode */
	void setMaxMode(const bool& mode);
	/** @brief Allows to retrieve the maximize_ parameter */
	bool getMaxMode() const;

	/** @brief Retrieves the current personality of this individual */
	personality getPersonality() const;

	/*******************************************************************/
	/**
	 * Adds an attribute to the individual.
	 *
	 * @param key The key referring to the attribute
	 * @param value The attribute's value
	 * @return The attribute value
	 */
	template <typename T>
	T setAttribute(const std::string& key, const T& value) {
		// Do some preparatory checks
		if(typeid(T) != typeid(std::string) &&
				typeid(T) != typeid(boost::int32_t) &&
				typeid(T) != typeid(double) &&
				typeid(T) != typeid(bool)) {
			std::ostringstream error;
			error << "In GIndividual::setAttribute<>(): Error: bad type given for attribute: " << typeid(T).name() << std::endl;
			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}

		// Add a suitable variant to the map
		boost::variant<std::string, boost::int32_t, double, bool> attribute(value);
		attributeTable_[key] = attribute;

		return value;
	}

	/*******************************************************************/
	/**
	 * Retrieves an attribute from the individual.
	 *
	 * @param key The key referring to the attribute
	 * @return The retrieved value (or NULL, if not available)
	 */
	template <typename T>
	T getAttribute(const std::string& key) {
		if(this->hasAttribute(key))
			return boost::get<T>(attributeTable_[key]); // will throw if T is not a valid type
		else
			return (T)NULL;
	}

	/*******************************************************************/

	/** @brief This function returns the current personality traits base pointer */
	boost::shared_ptr<GPersonalityTraits> getPersonalityTraits();

	/**************************************************************************************************/
	/**
	 * The function converts the local personality to the desired type and returns it for modification
	 * by the corresponding optimization algorithm. The base algorithms have been declared "friend" of
	 * GIndividual and can thus access this function. External entities have no need to do so.
	 *
	 * @return A boost::shared_ptr converted to the desired target type
	 */
	template <typename personality_type>
	boost::shared_ptr<personality_type> getPersonalityTraits() {
#ifdef DEBUG
		// Convert to the desired target type
		boost::shared_ptr<personality_type> p_load = boost::dynamic_pointer_cast<personality_type>(pt_ptr_);

		// Check that the conversion worked. dynamic_cast emits an empty pointer, if this was not the case.
		if(!p_load){
			std::ostringstream error;
			error << "In GIndividual::getPersonalityTraits<personality_type>() : Conversion error!" << std::endl;
			throw geneva_error_condition(error.str());
		}

		return p_load;
#else
		return boost::static_pointer_cast<personality_type>(pt_ptr_);
#endif
	}

	/**************************************************************************************************/

	/** @brief Removes an attribute from the individual */
	bool delAttribute(const std::string&);
	/** @brief Checks whether a given attribute is present */
	bool hasAttribute(const std::string&) const;
	/** @brief Clears the attribute table */
	void clearAttributes();

	/** @brief Wrapper for customUpdateOnStall that does error checking and sets the dirty flag */
	virtual bool updateOnStall();

protected:
	/** @brief Updates the individual's structure and/or parameters, if the optimization has stalled */
	virtual bool customUpdateOnStall();
	/** @brief The actual fitness calculation takes place here */
	virtual double fitnessCalculation() = 0;
	/** @brief The actual mutation operations */
	virtual void customMutations() = 0;

	/** @brief Sets the dirtyFlag_ */
	void setDirtyFlag() ;

private:
	/** @brief Sets the current personality of this individual */
	void setPersonality(const personality&);
	/** @brief Resets the current personality to NONE */
	void resetPersonality();

	/** @brief Sets the parentCounter_ parameter */
	bool setIsParent(const bool&) ;

	/** @brief Sets the dirtyFlag_ to any desired value */
	bool setDirtyFlag(const bool&) ;

	/** @brief Holds this object's internal fitness */
    double currentFitness_;
    /** @brief Internal representation of the mutation status of this object */
    bool dirtyFlag_;
    /** @brief Steers whether lazy evaluation is allowed */
    bool allowLazyEvaluation_;
    /** @brief Holds the parent population's current generation */
    boost::uint32_t parentPopGeneration_;
    /** @brief Allows populations to mark members as parents or children */
    boost::uint32_t parentCounter_;
    /** @brief Stores the current position in the population */
    std::size_t popPos_;
	/** @brief Holds key/attribute pairs, with several attribute types being allowed */
    std::map<std::string, boost::variant<std::string, boost::int32_t, double, bool> > attributeTable_;
    /** @brief The maximum number of processing cycles. 0 means "loop forever" (use with care!) */
    boost::uint32_t processingCycles_;
    /** @brief Indicates whether we are running in maximization or minimization mode */
    bool maximize_;
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
