/**
 * @file GParameterBaseWithAdaptorsT.hpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost header files go here

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#ifndef GPARAMETERBASEWITHADAPTORST_HPP_
#define GPARAMETERBASEWITHADAPTORST_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "GAdaptorT.hpp"
#include "GObject.hpp"
#include "GParameterBase.hpp"

namespace Gem {
namespace Geneva {

/************************************************************************************************/
/**
 * This is a templatized version of the GParameterBase class. Its main
 * addition over that class is the storage of an adaptor, which allows the
 * adaption of parameters. As this functionality has to be type specific,
 * this class has also been implemented as a template. Storing the adaptors in
 * the GParameterBase class would not have been possible, as it cannot be
 * templatized - it serves as a base class for the objects stored in the
 * GParameterSet collections.
 */
template <typename T>
class GParameterBaseWithAdaptorsT:
	public GParameterBase
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterBase)
		   & BOOST_SERIALIZATION_NVP(adaptor_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/*******************************************************************************************/
	/**
	 * The default constructor. adaptor_ is not initialized with an object and will thus
	 * contain a NULL pointer (which is o.k.).
	 */
	GParameterBaseWithAdaptorsT()
		: GParameterBase()
		, adaptor_(boost::shared_ptr<GAdaptorT<T> >())
	{ /* nothing */	}

	/*******************************************************************************************/
	/**
	 * The copy constructor.
	 *
	 * @param cp A copy of another GParameterBaseWithAdaptorsT object
	 */
	GParameterBaseWithAdaptorsT(const GParameterBaseWithAdaptorsT<T>& cp)
		: GParameterBase(cp)
		, adaptor_(cp.adaptor_?(cp.adaptor_)->GObject::clone<GAdaptorT<T> >():boost::shared_ptr<GAdaptorT<T> >())
	{ /* nothing */ }

	/*******************************************************************************************/
	/**
	 * The destructor.
	 */
	virtual ~GParameterBaseWithAdaptorsT(){
		// boost::shared_ptr takes care of the deletion of
		// adaptors in the vector (if we hold the only copy)
		adaptor_.reset();
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GParameterBaseWithAdaptorsT<T> object
	 *
	 * @param  cp A constant reference to another GParameterBaseWithAdaptorsT object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GParameterBaseWithAdaptorsT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GParameterBaseWithAdaptorsT<T>::operator==","cp", CE_SILENT);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another GParameterBaseWithAdaptorsT<T> object
	 *
	 * @param  cp A constant reference to another GParameterBaseWithAdaptorsT object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GParameterBaseWithAdaptorsT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GParameterBaseWithAdaptorsT<T>::operator!=","cp", CE_SILENT);
	}

	/*******************************************************************************************/
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
			const Gem::Common::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Common;

		// Check that we are indeed dealing with a GParamterBase reference
		const GParameterBaseWithAdaptorsT<T>  *p_load = GObject::conversion_cast<GParameterBaseWithAdaptorsT<T> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GParameterBase::checkRelationshipWith(cp, e, limit, "GParameterBaseWithAdaptorsT<T>", y_name, withMessages));

		// ... and then our local data
		deviations.push_back(checkExpectation(withMessages, "GParameterBaseWithAdaptorsT<T>", adaptor_, p_load->adaptor_, "adaptor_", "p_load->adaptor_", e , limit));

		return evaluateDiscrepancies("GParameterBaseWithAdaptorsT<T>", caller, deviations, e);
	}

	/*******************************************************************************************/
	/**
	 * Adds an adaptor to this object. Please note that this class takes ownership of the adaptor
	 * by cloning it.
	 *
	 * @param gat_ptr A boost::shared_ptr to an adaptor
	 */
	void addAdaptor(boost::shared_ptr<GAdaptorT<T> > gat_ptr) {
		// Check that we have indeed been given an adaptor
		if(!gat_ptr){
			std::ostringstream error;
			error << "In GParameterBaseWithAdaptorsT<T>::addAdaptor()" << std::endl
				  << "with typeid(T).name() = " << typeid(T).name() << ":" << std::endl
				  << "Error: Empty adaptor provided." << std::endl;
			throw Gem::Common::gemfony_error_condition(error.str());
		}

		if(adaptor_) { // Is an adaptor already present ?
			if (adaptor_->getAdaptorId() == gat_ptr->getAdaptorId()) {
				adaptor_->GObject::load(gat_ptr);
			}
			// Different type - need to convert
			else {
				adaptor_ = gat_ptr->GObject::clone<GAdaptorT<T> >();
			}
		}
		// None there ? Clone and assign gat_ptr
		else {
			adaptor_ = gat_ptr->GObject::clone<GAdaptorT<T> >();
		}

		// Make our local random number generator known to the adaptor
		adaptor_->assignGRandomPointer(GParameterBase::gr);
	}

	/*******************************************************************************************/
	/**
	 * Retrieves the adaptor. Throws in DBEUG mode , if we have no adaptor. It is assumed
	 * that only the object holding the "master" adaptor pointer should be allowed to modify it.
	 *
	 * @return A boost::shared_ptr to the adaptor
	 */
	boost::shared_ptr<GAdaptorT<T> > getAdaptor() const {
#ifdef DEBUG
		if(!adaptor_) {
			std::ostringstream error;
			error << "In GParameterBaseWithAdaptorsT::getAdaptor() : Error!" << std::endl
				  << "with typeid(T).name() = " << typeid(T).name() << std::endl
				  << "Tried to retrieve adaptor while none is present" << std::endl;
			throw Gem::Common::gemfony_error_condition(error.str());
		}
#endif /* DEBUG */

		return adaptor_;
	}

	/*******************************************************************************************/
	/**
	 * Transforms the adaptor stored in this class to the desired target type. The function
	 * will check in DEBUG mode whether an adaptor was indeed stored in this class. It will
	 * also complain in DEBUG mode if this function was called while no local adaptor was
	 * stored here. Note that this function will only be accessible to the compiler if adaptor_type
	 * is a derivative of GAdaptorT<T>, thanks to the magic of Boost's enable_if and Type Traits
	 * libraries.
	 *
	 * @return The desired adaptor instance, using its "natural" type
	 */
	template <typename adaptor_type>
	boost::shared_ptr<adaptor_type> adaptor_cast(
			typename boost::enable_if<boost::is_base_of<GAdaptorT<T>, adaptor_type> >::type* dummy = 0
	) const {
#ifdef DEBUG
		if(!adaptor_) {
			std::ostringstream error;
			error << "In GParameterBaseWithAdaptorsT::adaptor_cast<adaptor_type>()" << std::endl
				  << "with typeid(T).name() = " << typeid(T).name() << " : Error!" << std::endl
				  << "Tried to access empty adaptor pointer." << std::endl;
			throw Gem::Common::gemfony_error_condition(error.str());
		}

		boost::shared_ptr<adaptor_type> p = boost::dynamic_pointer_cast<adaptor_type>(adaptor_);

		if(p) return p;
		else {
			std::ostringstream error;
			error << "In GParameterBaseWithAdaptorsT::adaptor_cast<adaptor_type>() : Conversion error!" << std::endl;
			throw Gem::Common::gemfony_error_condition(error.str());
		}
#else
		return boost::static_pointer_cast<adaptor_type>(adaptor_);
#endif /* DEBUG */
	}

	/*******************************************************************************************/
	/**
	 * This function resets the local adaptor_ pointer.
	 */
	void resetAdaptor() {
		adaptor_.reset();
	}

	/*******************************************************************************************/
	/** @brief The adaption interface */
	virtual void adaptImpl(void) = 0;

	/*******************************************************************************************/
	/**
	 * Indicates whether an adaptor is present
	 *
	 * @return A boolean indicating whether adaptors are present
	 */
	bool hasAdaptor() const {
		if(adaptor_) return true;
		return false;
	}

	/*******************************************************************************************/
	/**
	 * Assigns a random number generator from another object to this object and any adaptor
	 * contained herein.
	 *
	 * @param gr_cp A reference to another object's GRandomBaseT object derivative
	 */
	virtual void assignGRandomPointer(Gem::Hap::GRandomBaseT<double, boost::int32_t> *gr_cp) {
		if(adaptor_) adaptor_->assignGRandomPointer(gr_cp);
		GParameterBase::assignGRandomPointer(gr_cp);
	}

	/*******************************************************************************************/
	/**
	 * Re-connects the local random number generator to gr and tells the adaptor to do the same.
	 */
	virtual void resetGRandomPointer() {
		if(adaptor_) adaptor_->resetGRandomPointer();
		GParameterBase::resetGRandomPointer();
	}

	/***********************************************************************************/
	/**
	 * Checks whether the local random number generator is used in this class and in a
	 * possible adaptor contained in this object. The result will be true only if both
	 * this object and the adaptor (if available) contain a local random number generator.
	 *
	 * @bool A boolean indicating whether solely the local random number generator is used
	 */
	virtual bool usesLocalRNG() const {
		bool result=true;

		if(adaptor_ && !adaptor_->usesLocalRNG()) result=false;
		if(!GParameterBase::usesLocalRNG()) result=false;

		return result;
	}

protected:
	/*******************************************************************************************/
	/**
	 * Loads the data of another GParameterBaseWithAdaptorsT object, which
	 * is camouflaged as a GObject.
	 *
	 * @param cp A copy of another GParameterBaseWithAdaptorsT, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp){
		// Convert cp into local format
		const GParameterBaseWithAdaptorsT<T> *p_load = this->conversion_cast<GParameterBaseWithAdaptorsT<T> >(cp);

		// Load our parent class'es data ...
		GParameterBase::load_(cp);

		// and then our local data

		// Only act if the other object actually holds an adaptor
		if(p_load->adaptor_) {
			// Same type: We can just load the data
		    if (adaptor_ && (adaptor_->getAdaptorId() == p_load->adaptor_->getAdaptorId())) {
				adaptor_->GObject::load(p_load->adaptor_);
			}
			// Different type - need to convert
			else {
				adaptor_ = p_load->adaptor_->GObject::clone<GAdaptorT<T> >();
			}
		}
		else {
			// Make sure our adaptor is also empty
			adaptor_.reset();
		}
	}

	/*******************************************************************************************/
	/** @brief Creates a deep clone of this object. Purely virtual, as we do not want this
	 * class to be instantiated directly */
	virtual GObject* clone_() const = 0;

	/*******************************************************************************************/
	/**
	 * This function applies our adaptor to a value. Note that the argument of
	 * this function will get changed.
	 *
	 * @param value The parameter to be adapted
	 */
	void applyAdaptor(T &value) {
#ifdef DEBUG
		if (adaptor_) {
			adaptor_->adapt(value);
		} else {
			std::ostringstream error;
			error << "In GParameterBaseWithAdaptorsT<T>::applyAdaptor(T& value):" << std::endl
				  << "with typeid(T).name() = " << typeid(T).name() << std::endl
				  << "Error: No adaptor was found." << std::endl;

			throw Gem::Common::gemfony_error_condition(error.str());
		}
#else
		adaptor_->adapt(value);
#endif /* DEBUG */
	}

	/*******************************************************************************************/
	/**
	 * This function applies our adaptor to a collection of values. Note that the argument
	 * of this function will get changed.
	 *
	 * @param collection A vector of values that shall be adapted
	 */
	void applyAdaptor(std::vector<T> &collection) {
#ifdef DEBUG
		if(!adaptor_) {
			std::ostringstream error;
			error << "In GParameterBaseWithAdaptorsT<T>::applyAdaptor(std::vector<T>& collection):" << std::endl
				  << "with typeid(T).name() = " << typeid(T).name() << std::endl
				  << "Error: No adaptor was found." << std::endl;
			throw Gem::Common::gemfony_error_condition(error.str());
		}
#endif /* DEBUG */

		// Apply the adaptor to each data item in turn
		typename std::vector<T>::iterator it;
		for (it = collection.begin(); it != collection.end(); ++it)	{
			adaptor_->adapt(*it);
		}
	}

private:
	/*******************************************************************************************/
	/**
	 * @brief Holds the adaptor used for adaption of the values stored in derived classes.
	 */
	boost::shared_ptr<GAdaptorT<T> > adaptor_;

#ifdef GENEVATESTING
public:
	/*******************************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() {
		bool result = false;

		// Call the parent classes' functions
		if(GParameterBase::modify_GUnitTests()) result = true;

		return result;
	}

	/*******************************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
		// Call the parent classes' functions
		GParameterBase::specificTestsNoFailureExpected_GUnitTests();
	}

	/*******************************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
		// Call the parent classes' functions
		GParameterBase::specificTestsFailuresExpected_GUnitTests();
	}
#endif /* GENEVATESTING */
};

// Declaration of specializations for std::vector<bool>
template<> void GParameterBaseWithAdaptorsT<bool>::applyAdaptor(std::vector<bool>&);

} /* namespace Geneva */
} /* namespace Gem */

/********************************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename T>
		struct is_abstract<Gem::Geneva::GParameterBaseWithAdaptorsT<T> > : public boost::true_type {};
		template<typename T>
		struct is_abstract< const Gem::Geneva::GParameterBaseWithAdaptorsT<T> > : public boost::true_type {};
	}
}

/********************************************************************************************/

#endif /* GPARAMETERBASEWITHADAPTORST_HPP_ */
