/**
 * @file GParameterBaseWithAdaptorsT.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
 */

// Standard header files go here
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>

// Boost header files go here

#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#ifndef GPARAMETERBASEWITHADAPTORST_HPP_
#define GPARAMETERBASEWITHADAPTORST_HPP_

// GenEvA headers go here
#include "GParameterBase.hpp"
#include "GObject.hpp"
#include "GAdaptorT.hpp"
#include "GLogger.hpp"
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
		ar & make_nvp("GParameterBase", boost::serialization::base_object<GParameterBase>(*this));
		ar & make_nvp("adaptor_", adaptor_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/*******************************************************************************************/
	/**
	 * The default constructor. adaptor_ is not initialized with an object and will thus
	 * contain a NULL pointer (which is o.k.).
	 */
	GParameterBaseWithAdaptorsT()
		:GParameterBase()
	{ /* nothing */	}

	/*******************************************************************************************/
	/**
	 * The copy constructor.
	 *
	 * @param cp A copy of another GParameterBaseWithAdaptorsT object
	 */
	GParameterBaseWithAdaptorsT(const GParameterBaseWithAdaptorsT<T>& cp)
		:GParameterBase(cp)
	{
		// Copy the other adaptor's content if it holds an object
		if(cp.adaptor_)  adaptor_ = (cp.adaptor_)->GObject::clone_bptr_cast<GAdaptorT<T> >();
	}

	/*******************************************************************************************/
	/**
	 * The standard destructor.
	 */
	virtual ~GParameterBaseWithAdaptorsT(){
		// boost::shared_ptr takes care of the deletion of
		// adaptors in the vector.
		adaptor_.reset();
	}

	/*******************************************************************************************/
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone() const = 0;

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GParameterBaseWithAdaptorsT<T> object
	 *
	 * @param  cp A constant reference to another GParameterBaseWithAdaptorsT object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GParameterBaseWithAdaptorsT<T>& cp) const {
		return GParameterBaseWithAdaptorsT<T>::isEqualTo(cp, boost::logic::indeterminate);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another GParameterBaseWithAdaptorsT<T> object
	 *
	 * @param  cp A constant reference to another GParameterBaseWithAdaptorsT object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GParameterBaseWithAdaptorsT<T>& cp) const {
		return !GParameterBaseWithAdaptorsT<T>::isEqualTo(cp, boost::logic::indeterminate);
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GParameterBaseWithAdaptorsT<T> object
	 *
	 * @param  cp A constant reference to another GParameterBaseWithAdaptorsT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool isEqualTo(const GObject& cp, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
	    using namespace Gem::Util;

		// Check that we are indeed dealing with a GParameterBaseWithAdaptorsT reference
		// and convert accordingly
		const GParameterBaseWithAdaptorsT<T> *gpbwa_load = GObject::conversion_cast(&cp,  this);

		// Check equality of the parent class
		if(!GParameterBase::isEqualTo(*gpbwa_load, expected)) return false;

		// We have an adaptor, the other instance doesn't (or vice versa)
		if((!adaptor_ && gpbwa_load->adaptor_) || (adaptor_ && !gpbwa_load->adaptor_)) return false;

		// Check our local adaptor
		if((adaptor_ && gpbwa_load->adaptor_) && !adaptor_->isEqualTo(*(gpbwa_load->adaptor_), expected)) return false;

		return true;
	}

	/*******************************************************************************************/
	/**
	 * Checks for similarity with another GParameterBaseWithAdaptorsT<T> object
	 *
	 * @param  cp A constant reference to another GParameterBaseWithAdaptorsT<T> object
	 * @param limit A double value specifying the acceptable level of differences of floating point values
	 * @return A boolean indicating whether both objects are similar to each other
	 */
	virtual bool isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
	    using namespace Gem::Util;

		// Check that we are indeed dealing with a GParameterBaseWithAdaptorsT reference
		// and convert accordingly
		const GParameterBaseWithAdaptorsT<T> *gpbwa_load = GObject::conversion_cast(&cp,  this);

		// Check equality of the parent class
		if(!GParameterBase::isSimilarTo(*gpbwa_load, limit, expected)) return false;

		// We have an adaptor, the other instance doesn't (or vice versa)
		if((!adaptor_ && gpbwa_load->adaptor_) || (adaptor_ && !gpbwa_load->adaptor_)) return false;

		// Then check the local adaptor
		if((adaptor_ && gpbwa_load->adaptor_) && !adaptor_->isSimilarTo(*(gpbwa_load->adaptor_), limit, expected)) return false;

		return true;
	}

	/*******************************************************************************************/
	/**
	 * Determines whether production of random numbers should happen remotely
	 * (RNRFACTORY) or locally (RNRLOCAL)
	 *
	 * @param rnrGenMode A parameter which indicates where random numbers should be produced
	 */
	virtual void setRnrGenerationMode(const Gem::Util::rnrGenerationMode& rnrGenMode) {
		// Set the parent number's mode
		GParameterBase::setRnrGenerationMode(rnrGenMode);

		// Set the modes of our local data, (if we do have local data)
		if(adaptor_) adaptor_->setRnrGenerationMode(rnrGenMode);
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
		const  GParameterBaseWithAdaptorsT<T> *gpbwa_load = this->conversion_cast(cp, this);

		// Load our parent class'es data ...
		GParameterBase::load(cp);

		// and then our local data

		// Only act if the other object actually holds an adaptor
		if(gpbwa_load->adaptor_) {
			// Same type: We can just load the data
			if (adaptor_->name() == gpbwa_load->adaptor_->name()) {
				adaptor_->load((gpbwa_load->adaptor_).get());
			}
			// Different type - need to convert
			else {
				adaptor_ = gpbwa_load->adaptor_->GObject::clone_bptr_cast<GAdaptorT<T> >();
			}
		}
		else {
			// Make sure our adaptor is also empty
			adaptor_.reset();
		}
	}

	/*******************************************************************************************/
	/**
	 * Adds an adaptor to this object. Please note that this class takes ownership of the adaptor.
	 * Thus, at the end of the lifetime, the adaptor will be destroyed.
	 *
	 * @param gat A boost::shared_ptr to an adaptor
	 */
	void addAdaptor(boost::shared_ptr<GAdaptorT<T> > gat) {
		// Check that we have indeed been given an adaptor
		if(!gat){
			std::ostringstream error;
			error << "In GParameterBaseWithAdaptorsT<T>::addAdaptor():" << std::endl
				  << "Error: Empty adaptor provided." << std::endl;

			throw geneva_error_condition(error.str());
		}

		if(adaptor_) { // Is an adaptor already present ?
			if (adaptor_->name() == gat->name()) {
				adaptor_->load(gat.get());
			}
			// Different type - need to convert
			else {
				adaptor_ = gat->GObject::clone_bptr_cast<GAdaptorT<T> >();
			}
		}
		// None there ? Clone and assign gat
		else {
			adaptor_ = gat->GObject::clone_bptr_cast<GAdaptorT<T> >();
		}
	}

	/*******************************************************************************************/
	/**
	 * Retrieves the adaptor.
	 *
	 * @return A boost::shared_ptr to the adaptor
	 */
	boost::shared_ptr<GAdaptorT<T> > getAdaptor() {
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
	 * This function resets the local adaptor_ object
	 */
	void resetAdaptor() {
		adaptor_.reset();
	}

	/*******************************************************************************************/
	/** @brief The mutate interface */
	virtual void mutate(void) = 0;

	/*******************************************************************************************/
	/**
	 * Indicates whether any adaptors are present
	 *
	 * @return A boolean indicating whether adaptors are present
	 */
	bool hasAdaptor() const {
		if(adaptor_) return true;
		return false;
	}

protected:
	/*******************************************************************************************/
	/**
	 * This function applies our adaptor to a value. Note that the argument of
	 * this function will get changed.
	 *
	 * @param value The parameter to be mutated
	 */
	void applyAdaptor(T &value) {
		if (adaptor_) {
			adaptor_->mutate(value);
		} else {
			std::ostringstream error;
			error << "In GParameterBaseWithAdaptorsT<T>::applyAdaptor(T& value):" << std::endl
				  << "Error: No adaptor was found." << std::endl;

			throw geneva_error_condition(error.str());
		}
	}

	/*******************************************************************************************/
	/**
	 * This function applies our adaptor to a collection of values. Note that the argument
	 * of this function will get changed.
	 *
	 * @param collection A vector of values that shall be mutated
	 */
	void applyAdaptor(std::vector<T> &collection) {
		typename std::vector<T>::iterator it;
		for (it = collection.begin(); it != collection.end(); ++it)	applyAdaptor(*it);
	}

private:
	/*******************************************************************************************/
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
