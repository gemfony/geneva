/**
 * @file GParameterBaseWithAdaptorsT.hpp
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
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

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


// GenEvA headers go here
#include "GParameterBase.hpp"
#include "GObject.hpp"
#include "GAdaptorT.hpp"
#include "GenevaExceptions.hpp"

namespace Gem {
namespace GenEvA {

/************************************************************************************************/
/**
 * This is a templatized version of the GParameterBase class. Its main
 * addition over that class is the storage of an adaptor, which allows the
 * mutation of parameters. As this functionality has to be type specific,
 * this class is also implemented as a template. Storing the adaptors in
 * the GParameterBase class would not have been possible, as it cannot be
 * templatized - it serves as a base class for the objects stored in the
 * GParameterSet collections. This class may either hold its own, globally
 * unique adaptor, or can contain a shared_ptr to a "foreign" adaptor. The
 * latter is useful in conjunction with the GParameterTCollectionT class,
 * if all contained GParameterT objects should use the same adaptor. Note
 * that, in all relevant functions of this class, we only copy foreign adaptors
 * if they are unique. If this is not the case, we assume that "someone else"
 * will give us an adaptor before the first call to mutate().
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
		   & BOOST_SERIALIZATION_NVP(hasLocalAdaptor_);

		if(hasLocalAdaptor_) {
			ar & BOOST_SERIALIZATION_NVP(adaptor_);
		}
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
		, hasLocalAdaptor_(false)
	{ /* nothing */	}

	/*******************************************************************************************/
	/**
	 * The copy constructor.
	 *
	 * @param cp A copy of another GParameterBaseWithAdaptorsT object
	 */
	GParameterBaseWithAdaptorsT(const GParameterBaseWithAdaptorsT<T>& cp)
		: GParameterBase(cp)
		, hasLocalAdaptor_(cp.hasLocalAdaptor_)
	{
		// Copy the other adaptor's content if it holds an object
		if(hasLocalAdaptor_ && cp.adaptor_)  adaptor_ = (cp.adaptor_)->GObject::clone<GAdaptorT<T> >();
	}

	/*******************************************************************************************/
	/**
	 * The standard destructor.
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
		using namespace Gem::Util;
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
		using namespace Gem::Util;
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
			const Gem::Util::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Util;
	    using namespace Gem::Util::POD;

		// Check that we are indeed dealing with a GParamterBase reference
		const GParameterBaseWithAdaptorsT<T>  *p_load = GObject::conversion_cast<GParameterBaseWithAdaptorsT<T> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GParameterBase::checkRelationshipWith(cp, e, limit, "GParameterBaseWithAdaptorsT<T>", y_name, withMessages));

		// ... and then our local data
		deviations.push_back(checkExpectation(withMessages, "GParameterBaseWithAdaptorsT<T>", hasLocalAdaptor_, p_load->hasLocalAdaptor_, "hasLocalAdaptor_", "p_load->hasLocalAdaptor_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GParameterBaseWithAdaptorsT<T>", adaptor_, p_load->adaptor_, "adaptor_", "p_load->adaptor_", e , limit));

		return POD::evaluateDiscrepancies("GParameterBaseWithAdaptorsT<T>", caller, deviations, e);
	}

	/*******************************************************************************************/
	/**
	 * Loads the data of another GParameterBaseWithAdaptorsT object, which
	 * is camouflaged as a GObject.
	 *
	 * @param cp A copy of another GParameterBaseWithAdaptorsT, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp){
		// Convert cp into local format
		const  GParameterBaseWithAdaptorsT<T> *p_load = this->conversion_cast<GParameterBaseWithAdaptorsT<T> >(cp);

		// Load our parent class'es data ...
		GParameterBase::load(cp);

		// and then our local data
		hasLocalAdaptor_ = p_load->hasLocalAdaptor_;

		// Only act if the other object actually holds a unique adaptor
		if(hasLocalAdaptor_ && p_load->adaptor_) {
			// Same type: We can just load the data
		    if (adaptor_ && (adaptor_->getAdaptorId() == p_load->adaptor_->getAdaptorId())) {
				adaptor_->load((p_load->adaptor_).get());
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
			error << "In GParameterBaseWithAdaptorsT<T>::addAdaptor():" << std::endl
				  << "Error: Empty adaptor provided." << std::endl;

			throw geneva_error_condition(error.str());
		}

		if(adaptor_) { // Is an adaptor already present ?
			if (adaptor_->getAdaptorId() == gat_ptr->getAdaptorId()) {
				adaptor_->load(gat_ptr.get());
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

		hasLocalAdaptor_ = true;
	}

	/*******************************************************************************************/
	/**
	 * Adds a pointer to a "foreign" adaptor to this object. Thus external modification of the
	 * adaptor can also influence this object. This is useful in conjunction with the
	 * GParameterTCollectionT class, if all contained GParameterT objects should use the same
	 * adaptor.
	 *
	 * BAD: This function should be private, with a friend declaration for GParameterTCollectionT's
	 * mutate() function, as it is not intended for public use.
	 *
	 * @param gat_ptr A boost::shared_ptr to an adaptor
	 */
	void addAdaptorNoClone(boost::shared_ptr<GAdaptorT<T> > gat_ptr) {
		// Check that we have indeed been given an adaptor
		if(!gat_ptr){
			std::ostringstream error;
			error << "In GParameterBaseWithAdaptorsT<T>::addAdaptor():" << std::endl
				  << "Error: Empty adaptor provided." << std::endl;

			throw geneva_error_condition(error.str());
		}

		adaptor_ = gat_ptr;
		hasLocalAdaptor_ = false;
	}


	/*******************************************************************************************/
	/**
	 * Retrieves the adaptor. Throws in DBEUG mode , if we have no local adaptor. It is assumed
	 * that only the object holding the "master" adaptor pointer should be allowed to modify it.
	 *
	 * @return A boost::shared_ptr to the adaptor
	 */
	boost::shared_ptr<GAdaptorT<T> > getAdaptor() const {
#ifdef DEBUG
		if(!hasLocalAdaptor_) {
			std::ostringstream error;
			error << "In GParameterBaseWithAdaptorsT::getAdaptor() : Error!" << std::endl
				  << "Tried to retrieve adaptor that is not unique" << std::endl;
			throw geneva_error_condition(error.str());
		}
#endif /* DEBUG */

		return adaptor_;
	}

	/*******************************************************************************************/
	/**
	 * When compiled in debug mode, performs all necessary checks of the conversion of the
	 * adaptor to the target type. Otherwise uses a faster static cast.
	 *
	 * @return The desired adaptor instance, using its "natural" type
	 */
	template <typename adaptor_type>
	boost::shared_ptr<adaptor_type> adaptor_cast() const {

#ifdef DEBUG
		if(!hasLocalAdaptor_) {
			std::ostringstream error;
			error << "In GParameterBaseWithAdaptorsT::adaptor_cast<adaptor_type>() : Error!" << std::endl
				  << "Tried to retrieve adaptor that is not unique" << std::endl;
			throw geneva_error_condition(error.str());
		}

		// Convert to the desired target type
		boost::shared_ptr<adaptor_type> p_adaptor = boost::dynamic_pointer_cast<adaptor_type>(adaptor_);

		// Check that the conversion worked. dynamic_cast emits an empty pointer,
		// if this was not the case.
		if(!p_adaptor){
			std::ostringstream error;
			error << "In GParameterBaseWithAdaptorsT::adaptor_cast<adaptor_type>() : Conversion error!" << std::endl;

			// throw an exception. Add some information so that if the exception
			// is caught through a base object, no information is lost.
			throw geneva_error_condition(error.str());
		}

		return p_adaptor;
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
	/** @brief The mutate interface */
	virtual void mutateImpl(void) = 0;

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
	 * Indicates whether a local adaptor is present
	 *
	 * @return A boolean indicating whether adaptors are present
	 */
	bool hasLocalAdaptor() const {
		return hasLocalAdaptor_;
	}

protected:
	/*******************************************************************************************/
	/** @brief Creates a deep clone of this object. Purely virtual, as we do not want this
	 * class to be instantiated directly */
	virtual GObject* clone_() const = 0;

	/*******************************************************************************************/
	/**
	 * This function applies our adaptor to a value. Note that the argument of
	 * this function will get changed.
	 *
	 * @param value The parameter to be mutated
	 */
	void applyAdaptor(T &value) {
		// Let the adaptor know about the number of variables to expect
		if(hasLocalAdaptor_) adaptor_->setMaxVars(1);

#ifdef DEBUG
		if (adaptor_) {
			adaptor_->mutate(value);
		} else {
			std::ostringstream error;
			error << "In GParameterBaseWithAdaptorsT<T>::applyAdaptor(T& value):" << std::endl
				  << "Error: No adaptor was found." << std::endl;

			throw geneva_error_condition(error.str());
		}
#else
		adaptor_->mutate(value);
#endif /* DEBUG */
	}

	/*******************************************************************************************/
	/**
	 * This function applies our adaptor to a collection of values. Note that the argument
	 * of this function will get changed.
	 *
	 * @param collection A vector of values that shall be mutated
	 */
	void applyAdaptor(std::vector<T> &collection) {
		// Let the adaptor know about the number of variables to expect
		adaptor_->setMaxVars(collection.size());

		// Apply the adaptor to each data item in turn
		typename std::vector<T>::iterator it;
		for (it = collection.begin(); it != collection.end(); ++it)	applyAdaptor(*it);
	}

private:
	/*******************************************************************************************/
	/**
	 * Specifies whether we use a unique (i.e. cloned) adaptor, or one that can be in use
	 * by another object
	 */
	bool hasLocalAdaptor_;

	/**
	 * Holds the adaptor used for mutation of the values stored in derived classes.
	 */
	boost::shared_ptr<GAdaptorT<T> > adaptor_;
};

// Declaration of specializations for std::vector<bool>
template<> void GParameterBaseWithAdaptorsT<bool>::applyAdaptor(std::vector<bool>&);

} /* namespace GenEvA */
} /* namespace Gem */

/********************************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename T>
		struct is_abstract<Gem::GenEvA::GParameterBaseWithAdaptorsT<T> > : public boost::true_type {};
		template<typename T>
		struct is_abstract< const Gem::GenEvA::GParameterBaseWithAdaptorsT<T> > : public boost::true_type {};
	}
}

/********************************************************************************************/

#endif /* GPARAMETERBASEWITHADAPTORST_HPP_ */
