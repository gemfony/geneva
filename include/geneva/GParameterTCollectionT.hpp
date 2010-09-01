/**
 * @file GParameterTCollectionT.hpp
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

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost header files go here

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GPARAMETERTCOLLECTIONT_HPP_
#define GPARAMETERTCOLLECTIONT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva header files go here
#include "common/GHelperFunctionsT.hpp"
#include "GParameterBase.hpp"
#include "GParameterT.hpp"
#include "GStdPtrVectorInterfaceT.hpp"

namespace Gem {
namespace Geneva {

/***********************************************************************************************/
/**
 * This class shares many similarities with the GParameterCollectionT class. Instead
 * of individual values that can be modified with adaptors, however, it assumes that
 * the objects stored in it have their own adapt() function. This class has been designed
 * as a collection of GParameterT objects, hence the name.  As an example, one can create a
 * collection of GConstrainedDouble objects with this class rather than a simple GDoubleCollection.
 * In order to facilitate memory management, the GParameterT objects are stored
 * in boost::shared_ptr objects.
 */
template<typename T>
class GParameterTCollectionT
	:public GParameterBase,
	 public GStdPtrVectorInterfaceT<T>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		// Save the data
		ar & make_nvp("GParameterBase", boost::serialization::base_object<GParameterBase>(*this))
		   & make_nvp("GStdPtrVectorInterfaceT_T", boost::serialization::base_object<GStdPtrVectorInterfaceT<T> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/*******************************************************************************************/
	/**
	 * Allows to find out which type is stored in this class
	 */
	typedef T collection_type;

	/*******************************************************************************************/
	/**
	 * The default constructor
	 */
	GParameterTCollectionT()
	{ /* nothing */ }

	/*******************************************************************************************/
	/**
	 * The copy constructor
	 *
	 * @param cp A copy of another GParameterTCollectionT<T> object
	 */
	GParameterTCollectionT(const GParameterTCollectionT<T>& cp)
		: GParameterBase(cp)
		, GStdPtrVectorInterfaceT<T>(cp)
	{ /* nothing */ }

	/*******************************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GParameterTCollectionT()
	{ /* nothing */ }

	/*******************************************************************************************/
	/**
	 * A standard assignment operator.
	 *
	 * @param cp A copy of another GParameterTCollectionT<T> object
	 * @return A constant reference to this object
	 */
	const GParameterTCollectionT<T>& operator=(const GParameterTCollectionT<T>& cp)
	{
		GParameterTCollectionT<T>::load_(&cp);
		return *this;
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
		const GParameterTCollectionT<T>  *p_load = GObject::conversion_cast<GParameterTCollectionT<T> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent classes' data ...
		deviations.push_back(GParameterBase::checkRelationshipWith(cp, e, limit, "GParameterTCollectionT<T>", y_name, withMessages));
		deviations.push_back(GStdPtrVectorInterfaceT<T>::checkRelationshipWith(*p_load, e, limit, "GParameterTCollectionT<T>", y_name, withMessages));

		// no local data ...

		return evaluateDiscrepancies("GParameterTCollectionT<T>", caller, deviations, e);
	}

	/*******************************************************************************************/
	/**
	 * Allows to adapt the values stored in this class. We assume here that
	 * each item has its own adapt function. Hence we do not need to use or
	 * store own adaptors.
	 */
	virtual void adaptImpl() {
		typename GParameterTCollectionT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->adapt();
		}
	}

	/*******************************************************************************************/
	/**
	 * Swap another object's vector with ours
	 */
	inline void swap(GParameterTCollectionT<T>& cp) { GStdPtrVectorInterfaceT<T>::swap(cp.data); }

	/*******************************************************************************************/
	/**
	 * Swap another vector with ours
	 */
	inline void swap(std::vector<boost::shared_ptr<T> >& cp_data) { GStdPtrVectorInterfaceT<T>::swap(cp_data); }

	/*******************************************************************************************/
	/**
	 * Compares another vector object with ours.
	 *
	 * TODO: Attention: is GStdPtrVectorInterfaceT<T>::operator==(cp_data) defined ??
	 */
	bool operator==(const std::vector<boost::shared_ptr<T> >& cp_data) {
		return GStdPtrVectorInterfaceT<T>::operator==(cp_data);
	}

	/*******************************************************************************************/
	/**
	 * Compares another vector object with ours
	 */
	bool operator!=(const std::vector<boost::shared_ptr<T> >& cp_data) {
		return GStdPtrVectorInterfaceT<T>::operator!=(cp_data);
	}

	/*******************************************************************************************/
	/**
	 * Assign another vector object to ours
	 */
	const std::vector<boost::shared_ptr<T> >& operator=(const std::vector<boost::shared_ptr<T> >& cp_data) {
		return GStdPtrVectorInterfaceT<T>::operator=(cp_data);
	}

	/*******************************************************************************************/
	/**
	 * Initializes floating-point-based parameters with a given value. Allows e.g. to set all
	 * floating point parameters to 0.
	 *
	 * @param val The value to be assigned to the parameters
	 */
	virtual void fpFixedValueInit(const float& val) {
		typename GParameterTCollectionT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->fpFixedValueInit(val);
		}
	}

	/*******************************************************************************************/
	/**
	 * Multiplies floating-point-based parameters with a given value.
	 *
	 * @param val The value to be multiplied with the parameter
	 */
	virtual void fpMultiplyBy(const float& val) {
		typename GParameterTCollectionT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->fpMultiplyBy(val);
		}
	}

	/*******************************************************************************************/
	/**
	 * Multiplies with a random floating point number in a given range.
	 *
	 * @param min The lower boundary for random number generation
	 * @param max The upper boundary for random number generation
	 */
	virtual void fpMultiplyByRandom(const float& min, const float& max)	{
		typename GParameterTCollectionT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->fpMultiplyByRandom(min, max);
		}
	}

	/*******************************************************************************************/
	/**
	 * Multiplies with a random floating point number in the range [0, 1[.
	 */
	virtual void fpMultiplyByRandom() {
		typename GParameterTCollectionT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->fpMultiplyByRandom();
		}
	}

	/*******************************************************************************************/
	/**
	 * Adds the floating point parameters of another GParameterTCollectionT object to this one.
	 *
	 * @oaram p A boost::shared_ptr to another GParameterBase object
	 */
	virtual void fpAdd(boost::shared_ptr<GParameterBase> p_base)	{
		// We first need to convert p_base to our local type
		typename boost::shared_ptr<GParameterTCollectionT<T> > p
			= GParameterBase::parameterbase_cast<GParameterTCollectionT<T> >(p_base);

		// Check that both collections have the same size
		if(this->size() != p->size()) {
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::fpAdd(): Error!" << std::endl
				  << "Collections have different sizes: " << this->size() << " " << p->size() << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// Call fpAdd on all objects stored in this collection
		typename GParameterTCollectionT<T>::iterator it, it_p;
		for(it=this->begin(), it_p=p->begin(); it!=this->end(); ++it, ++it_p) {
			(*it)->fpAdd(*it_p);
		}
	}

	/*******************************************************************************************/
	/**
	 * Subtracts the floating point parameters of another GParameterTCollectionT object from this one.
	 *
	 * @oaram p A boost::shared_ptr to another GParameterBase object
	 */
	virtual void fpSubtract(boost::shared_ptr<GParameterBase> p_base)	{
		// We first need to convert p_base to our local type
		typename boost::shared_ptr<GParameterTCollectionT<T> > p
			= GParameterBase::parameterbase_cast<GParameterTCollectionT<T> >(p_base);

		// Check that both collections have the same size
		if(this->size() != p->size()) {
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::fpSubtract(): Error!" << std::endl
				  << "Collections have different sizes: " << this->size() << " " << p->size() << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// Call fpAdd on all objects stored in this collection
		typename GParameterTCollectionT<T>::iterator it, it_p;
		for(it=this->begin(), it_p=p->begin(); it!=this->end(); ++it, ++it_p) {
			(*it)->fpSubtract(*it_p);
		}
	}

	/*******************************************************************************************/
	/**
	 * Assigns a random number generator from another object to all objects stored in this
	 * collection and to the object itself.
	 *
	 * @param gr_cp A reference to another object's GRandomBaseT object derivative
	 */
	virtual void assignGRandomPointer(Gem::Hap::GRandomBaseT<double, boost::int32_t> *gr_cp) {
		// Do some error checking
		if(!gr_cp) {
			std::ostringstream error;
			error << "In GParameterTCollectionT<T>::assignGRandomPointer(): Error!" << std::endl
				  << "Tried to assign a NULL pointer" << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// Assign the foreign pointer to all objects stored in this collection
		typename GParameterTCollectionT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->assignGRandomPointer(gr_cp);
		}

		// Assign the foreign pointer to this object as well
		GParameterBase::assignGRandomPointer(gr_cp);
	}

	/***********************************************************************************/
	/**
	 * Re-connects the local random number generator to gr and distributes the call
	 * to all objects contained in this collection class.
	 */
	void resetGRandomPointer() {
		// Reset all objects stored in this collection
		typename GParameterTCollectionT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->resetGRandomPointer();
		}

		// Reset our parent class
		GParameterBase::resetGRandomPointer();
	}

	/***********************************************************************************/
	/**
	 * Checks whether solely the local random number generator is used. The function returns
	 * false if at least one component of this class does not use a local random number
	 * generator
	 *
	 * @bool A boolean indicating whether solely the local random number generator is used
	 */
	bool usesLocalRNG() const {
		bool result = true;

		// Check all components of this class
		typename GParameterTCollectionT<T>::const_iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			if(!(*it)->usesLocalRNG()) result = false;
		}

		// Check our parent class
		if(!GParameterBase::usesLocalRNG()) result = false;

		return result;
	}

protected:
	/*******************************************************************************************/
	/**
	 * Loads the data of another GParameterTCollectionT<T> object, camouflaged as a GObject
	 *
	 * @param cp A copy of another GParameterTCollectionT<T> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) {
		// Convert cp into local format
		const GParameterTCollectionT<T> *p_load = GObject::conversion_cast<GParameterTCollectionT<T> >(cp);

		// Load our parent class'es data ...
		GParameterBase::load_(cp);
		GStdPtrVectorInterfaceT<T>::operator=(*p_load);
	}

	/*******************************************************************************************/
	/**
	 * Creates a deep clone of this object. Declared purely virtual, as this class is not
	 * intended to be used directly.
	 */
	virtual GObject* clone_() const = 0;

	/*******************************************************************************************/
	/**
	 * Re-implementation of a corresponding function in GStdPtrVectorInterface.
	 * Making the vector wrapper purely virtual allows the compiler to perform
	 * further optimizations.
	 */
	virtual void dummyFunction() { /* nothing */ }

	/*******************************************************************************************/
	/**
	 * Triggers random initialization of all parameter objects
	 */
	virtual void randomInit_() {
		typename GParameterTCollectionT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			// Note that we do not call the randomInit_() function. First of all, we
			// do not have access to it. Secondly it might be that re-initialization of
			// a specific object is not desired.
			(*it)->GParameterBase::randomInit();
		}
	}

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
		if(GStdPtrVectorInterfaceT<T>::modify_GUnitTests()) result = true;

		return result;
	}

	/*******************************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
		// Call the parent classes' functions
		GParameterBase::specificTestsNoFailureExpected_GUnitTests();
		GStdPtrVectorInterfaceT<T>::specificTestsNoFailureExpected_GUnitTests();
	}

	/*******************************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
		// Call the parent classes' functions
		GParameterBase::specificTestsFailuresExpected_GUnitTests();
		GStdPtrVectorInterfaceT<T>::specificTestsFailuresExpected_GUnitTests();
	}
#endif /* GENEVATESTING */
};


/***********************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/**************************************************************************************************/

#endif /* GPARAMETERTCOLLECTIONT_HPP_ */
