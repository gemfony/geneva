/**
 * @file GAdaptorT.hpp
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

// Standard headers go here
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>

#ifndef GADAPTORT_HPP_
#define GADAPTORT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here

#include "thirdparty/boost/GTriboolSerialization.hpp"
#include "GObject.hpp"
#include "GRandom.hpp"
#include "GEnums.hpp"
#include "GPODExpectationChecksT.hpp"

namespace Gem {
namespace GenEvA {

/******************************************************************************************/
/**
 * In GenEvA, two mechanisms exist that let the user specify the
 * type of mutation he wants to have executed on collections of
 * items (basic types or any other types).  The most basic
 * possibility is for the user to overload the GIndividual::customMutations()
 * function and manually specify the types of mutations (s)he
 * wants. This allows great flexibility, but is not very practicable
 * for standard mutations.
 *
 * Classes derived from GParameterBaseWithAdaptorsT<T> can additionally store
 * "adaptors". These are templatized function objects that can act
 * on the items of a collection of user-defined types. Predefined
 * adaptors exist for standard types (with the most prominent
 * examples being bits and double values).
 *
 * The GAdaptorT class mostly acts as an interface for these
 * adaptors, but also implements some functionality of its own. E.g., it is possible
 * to specify a function that shall be called every adaptionThreshold_ calls of the
 * mutate() function. It is also possible to set a mutation probability, only a certain
 * percentage of mutations is actually performed at run-time.
 *
 * In order to use this class, the user must derive a class from
 * GAdaptorT<T> and specify the type of mutation he wishes to
 * have applied to items, by overloading of
 * GAdaptorT<T>::customMutations(T&) .  T will often be
 * represented by a basic value (double, long, bool, ...). Where
 * this is not the case, the adaptor will only be able to access
 * public functions of T, unless T declares the adaptor as a friend.
 *
 * As a derivative of GObject, this class follows similar rules as
 * the other GenEvA classes.
 */
template <typename T>
class GAdaptorT:
	public GObject
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
		   & BOOST_SERIALIZATION_NVP(gr)
		   & BOOST_SERIALIZATION_NVP(adaptionCounter_)
		   & BOOST_SERIALIZATION_NVP(adaptionThreshold_)
		   & BOOST_SERIALIZATION_NVP(mutProb_)
		   & BOOST_SERIALIZATION_NVP(mutationMode_)
		   & BOOST_SERIALIZATION_NVP(currentIndex_)
		   & BOOST_SERIALIZATION_NVP(maxVars_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/***********************************************************************************/
	/**
	 * Allows external callers to find out about the type stored in this object
	 */
	typedef T mutant_type;

	/***********************************************************************************/
	/**
	 * The default constructor.
	 */
	GAdaptorT()
		: GObject()
		, gr(Gem::Util::DEFAULTRNRGENMODE)
		, adaptionCounter_(0)
		, adaptionThreshold_(0)
		, mutProb_(DEFAULTMUTPROB)
		, mutationMode_(boost::logic::indeterminate)
		, currentIndex_(0)
		, maxVars_(1)
	{ /* nothing */ }

	/***********************************************************************************/
	/**
	 * This constructor allows to set the probability with which a mutation is indeed
	 * performed.
	 */
	GAdaptorT(const double& prob)
		: GObject()
		, gr(Gem::Util::DEFAULTRNRGENMODE)
		, adaptionCounter_(0)
		, adaptionThreshold_(0)
		, mutProb_(prob)
		, mutationMode_(boost::logic::indeterminate)
		, currentIndex_(0)
		, maxVars_(1)
	{ /* nothing */ }

	/***********************************************************************************/
	/**
	 * A standard copy constructor.
	 *
	 * @param cp A copy of another GAdaptorT<T>
	 */
	GAdaptorT(const GAdaptorT<T>& cp)
		: GObject(cp)
		, gr(cp.gr)
		, adaptionCounter_(cp.adaptionCounter_)
		, adaptionThreshold_(cp.adaptionThreshold_)
		, mutProb_(cp.mutProb_)
		, mutationMode_(cp.mutationMode_)
		, currentIndex_(cp.currentIndex_)
		, maxVars_(cp.maxVars_)
	{
#ifdef DEBUG
		if(maxVars_ < 1) {
			std::ostringstream error;
			error << "In GAdaptorT<T>::GAdaptorT(cp):: Error!" << std::endl
			      << "The maximum number of variables must be at least 1" << std::endl;
			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}
#endif /* DEBUG */
	}

	/***********************************************************************************/
	/**
	 * The standard destructor. We have no local, dynamically allocated data, so the body of
	 * this function is empty.
	 */
	virtual ~GAdaptorT() { /* nothing */ }

	/***********************************************************************************/
	/**
	 * A standard assignment operator for GAdaptorT<T> objects,
	 *
	 * @param cp A copy of another GAdaptorT<T> object
	 */
	const GAdaptorT<T>& operator=(const GAdaptorT<T>& cp) {
		GAdaptorT<T>::load(&cp);
		return *this;
	}

	/***********************************************************************************/
	/**
	 * Loads the contents of another GAdaptorT<T>. The function
	 * is similar to a copy constructor (but with a pointer as
	 * argument). As this function might be called in an environment
	 * where we do not know the exact type of the class, the
	 * GAdaptorT<T> is camouflaged as a GObject . This implies the
	 * need for dynamic conversion.
	 *
	 * @param gb A pointer to another GAdaptorT<T>, camouflaged as a GObject
	 */
	virtual void load(const GObject *cp) {
		// Convert cp into local format
		const GAdaptorT<T> *p_load = this->conversion_cast(cp, this);

		// Load the parent class'es data
		GObject::load(cp);

		// Then our own data
		gr.load(&(p_load->gr));
		adaptionCounter_ = p_load->adaptionCounter_;
		adaptionThreshold_ = p_load->adaptionThreshold_;
		mutProb_ = p_load->mutProb_;
		mutationMode_ = p_load->mutationMode_;
		currentIndex_ = p_load->currentIndex_;
		maxVars_ = p_load->maxVars_;

#ifdef DEBUG
		if(maxVars_ < 1) {
			std::ostringstream error;
			error << "In GAdaptorT<T>::load(cp):: Error!" << std::endl
			      << "The maximum number of variables must be at least 1" << std::endl;
			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}
#endif /* DEBUG */
	}

	/***********************************************************************************/
	/**
	 * Checks for equality with another GAdaptorT<T> object
	 *
	 * @param  cp A constant reference to another GAdaptorT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GAdaptorT<T>& cp) const {
		using namespace Gem::Util;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GAdaptorT<T>::operator==","cp", CE_SILENT);
	}

	/***********************************************************************************/
	/**
	 * Checks for inequality with another GAdaptorT<T> object
	 *
	 * @param  cp A constant reference to another GAdaptorT<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GAdaptorT<T>& cp) const {
		using namespace Gem::Util;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GAdaptorT<T>::operator!=","cp", CE_SILENT);
	}

	/***********************************************************************************/
	/**
	 * Checks whether a given expectation for the relationship between this object and another object
	 * is fulfilled.
	 *
	 * @param cp A constant reference to another object, camouflaged as a GObject
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 * @param caller An identifier for the calling entity
	 * @param y_name An identifier for the object that should be compared to this one
	 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
	 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
	 */
	boost::optional<std::string> checkRelationshipWith(const GObject& cp,
			const Gem::Util::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Util;
	    using namespace Gem::Util::POD;

		// Check that we are indeed dealing with a GParamterBase reference
		const GAdaptorT<T>  *p_load = GObject::conversion_cast(&cp,  this);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GObject::checkRelationshipWith(cp, e, limit, "GAdaptorT<T>", y_name, withMessages));

		// ... and then our local data
		deviations.push_back(checkExpectation(withMessages, "GAdaptorT<T>", adaptionCounter_, p_load->adaptionCounter_, "adaptionCounter_", "p_load->adaptionCounter_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GAdaptorT<T>", adaptionThreshold_, p_load->adaptionThreshold_, "adaptionThreshold_", "p_load->adaptionThreshold_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GAdaptorT<T>", mutProb_, p_load->mutProb_, "mutProb_", "p_load->mutProb_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GAdaptorT<T>", mutationMode_, p_load->mutationMode_, "mutationMode_", "p_load->mutationMode_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GAdaptorT<T>", currentIndex_, p_load->currentIndex_, "currentIndex_", "p_load->currentIndex_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GAdaptorT<T>", maxVars_, p_load->maxVars_, "maxVars_", "p_load->maxVars_", e , limit));

		return evaluateDiscrepancies("GAdaptorT<T>", caller, deviations, e);
	}

	/***********************************************************************************/
	/**
	 * Retrieves the id of the adaptor. Purely virtual, must be implemented by the
	 * actual adaptors.
	 *
	 * @return The id of the adaptor
	 */
	virtual Gem::GenEvA::adaptorId getAdaptorId() const = 0;

	/***********************************************************************************/
	/**
	 * Determines whether production of random numbers should happen remotely
	 * (RNRFACTORY) or locally (RNRLOCAL) in the local random number generator.
	 *
	 * @param rnrGenMode A parameter which indicates where random numbers should be produced
	 */
	virtual void setRnrGenerationMode(const Gem::Util::rnrGenerationMode& rnrGenMode) {
		// Set the local random number generation mode
		gr.setRnrGenerationMode(rnrGenMode);
	}

	/***********************************************************************************/
	/**
	 * Retrieves the random number generators current generation mode.
	 *
	 * @return The current random number generation mode of the local generator
	 */
	Gem::Util::rnrGenerationMode getRnrGenerationMode() const {
		return gr.getRnrGenerationMode();
	}

	/***********************************************************************************/
	/**
	 * Sets the mutation probability to a given value. This function will throw
	 * if the probability is not in the allowed range.
	 *
	 * @param val The new value of the probability for integer flips
	 */
	void setMutationProbability(const double& probability) {
		// Check the supplied probability value
		if(probability < 0. || probability > 1.) {
			std::ostringstream error;
			error << "In GAdaptorT::setMutationProbability(const double&) : Error!" << std::endl
				  << "Bad probability value given: " << probability << std::endl;

			// throw an exception. Add some information so that if the exception
			// is caught through a base object, no information is lost.
			throw geneva_error_condition(error.str());
		}

		mutProb_ = probability;
	}

	/***********************************************************************************/
	/**
	 * Retrieves the current value of the mutation probability
	 *
	 * @return The current value of the mutation probability
	 */
	double getMutationProbability() const {
		return mutProb_;
	}

	/***********************************************************************************/
	/**
	 * Retrieves the current value of the adaptionCounter_ variable.
	 *
	 * @return The value of the adaptionCounter_ variable
	 */
	virtual boost::uint32_t getAdaptionCounter() const  {
		return adaptionCounter_;
	}

	/***********************************************************************************/
	/**
	 * Sets the value of adaptionThreshold_. If set to 0, no adaption of the optimization
	 * parameters will take place
	 *
	 * @param adaptionCounter The value that should be assigned to the adaptionCounter_ variable
	 */
	void setAdaptionThreshold(const boost::uint32_t& adaptionThreshold)  {
		adaptionThreshold_ = adaptionThreshold;
	}

	/***********************************************************************************/
	/**
	 * Retrieves the value of the adaptionThreshold_ variable.
	 *
	 * @return The value of the adaptionThreshold_ variable
	 */
	boost::uint32_t getAdaptionThreshold() const  {
		return adaptionThreshold_;
	}

	/***********************************************************************************/
	/**
	 * Allows to specify whether mutations should happen always, never, or with a given
	 * probability. This uses the boost::logic::tribool class. The function is declared
	 * virtual so adaptors requiring mutations to happen always or never can prevent
	 * resetting of the mutationMode_ variable.
	 *
	 * @param mutationMode The desired mode (always/never/with a given probability)
	 */
	virtual void setMutationMode(boost::logic::tribool mutationMode) {
		mutationMode_ = mutationMode;
	}

	/***********************************************************************************/
	/**
	 * Returns the current value of the mutationMode_ variable
	 *
	 * @return The current value of the mutationMode_ variable
	 */
	boost::logic::tribool getMutationMode() const {
		return mutationMode_;
	}

	/***********************************************************************************/
	/**
	 * Common interface for all adaptors to the mutation
	 * functionality. The user specifies this functionality in the
	 * customMutations() function.
	 *
	 * @param val The value that needs to be mutated
	 */
	void mutate(T& val)  {
		if(boost::logic::indeterminate(mutationMode_)) { // The most likely case
			// We only allow mutations in a certain percentage of cases
			if(this->gr.evenRandom() <= mutProb_) {
				if(adaptionThreshold_ && adaptionCounter_++ >= adaptionThreshold_){
					adaptionCounter_ = 0;
					this->adaptMutation();
				}

				this->customMutations(val);
			}
		}
		else if(mutationMode_ == true) { // always mutate
			this->customMutations(val);
		}
		// No need to test for mutationMode_ == false as no action is needed in this case

		// Wrap index if we have reached the maximum, otherwise increment
		if(maxVars_>1 && ++currentIndex_ >= maxVars_) currentIndex_ = 0;
	}

	/***********************************************************************************/
	/**
	 * Sets the maximum number of variables this adaptor can expect to mutate in a row.
	 * The knowledge about that quantity can become important when dealing with collections
	 * of variables, such as a GDoubleCollection or a GBoundedDoubleCollection. The function
	 * also resets the current index counter.
	 *
	 * @param maxVars The maximum number of variables this adaptor can expect to mutate in a row
	 */
	void setMaxVars(const std::size_t maxVars) {
#ifdef DEBUG
		if(maxVar_ < 1) {
			std::ostringstream error;
			error << "In GAdaptorT<T>::setMaxVars() : Error!" << std::endl
				  << "The maximum number of variables must be at least 1" << std::endl;
			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}
#endif /* DEBUG */

		maxVars_ = maxVars;
		currentIndex_ = 0;
	}

	/***********************************************************************************/
	/**
	 * Retrieves the value for the maximum number of mutations this adaptor expects
	 * to perform in a row.
	 *
	 * @return The maximum number of mutations this adaptor expects
	 */
	std::size_t getMaxVars() const {
		return maxVars_;
	}

	/***********************************************************************************/
	/**
	 * Retrieves the current index counter
	 *
	 * @return The current index counter variable
	 */
	std::size_t getCurrentIndex() const {
		return currentIndex_;
	}


protected:
	/***********************************************************************************/
	/** @brief Creates a deep copy of this object */
	virtual GObject *clone_(void) const =0;

	/***********************************************************************************/
    /**
     * A random number generator. Note that the actual calculation is possibly
     * done in a random number server. GRandom also has a local generator
     * in case the factory is unreachable, or local storage of random
     * number containers requires too much memory.
     */
	Gem::Util::GRandom gr;

	/***********************************************************************************/
	/**
	 *  This function is re-implemented by derived classes, if they wish to
	 *  implement special behavior upon a new mutation run. E.g., an internal
	 *  variable could be set to a new value. The function will be called every
	 *  adaptionThreshold_ calls of the mutate function, unless the threshold is
	 *  set to 0 . It is not purely virtual, as we do not force derived classes
	 *  to re-implement this function. Note though that, if the function is
	 *  re-implemented, this class'es function should be called as the last action,
	 *  as later versions of this function might implement local logic. Adaption
	 *  of mutation parameters can be switched off by setting the adaptionThreshold_
	 *  variable to 0.
	 */
	virtual void adaptMutation() { /* nothing */ }

	/***********************************************************************************/
	/** @brief Mutation of values as specified by the user */
	virtual void customMutations(T& val)=0;

private:
	/***********************************************************************************/
	boost::uint32_t adaptionCounter_; ///< A local counter
	boost::uint32_t adaptionThreshold_; ///< Specifies after how many mutations the mutation itself should be adapted
	double mutProb_; ///< internal representation of the mutation probability
	boost::logic::tribool mutationMode_; ///< false == never mutate; indeterminate == mutate with mutProb_ probability; true == always mutate
	std::size_t currentIndex_; ///< The index of variable to be changed, when dealing with collections
	std::size_t maxVars_; ///< The maximum number of variables this adaptor deals with
};

/******************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

/********************************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename T>
		struct is_abstract<Gem::GenEvA::GAdaptorT<T> > : public boost::true_type {};
		template<typename T>
		struct is_abstract< const Gem::GenEvA::GAdaptorT<T> > : public boost::true_type {};
	}
}

/********************************************************************************************/

#endif /* GADAPTORT_HPP_ */
