/**
 * @file GParameterTCollectionT.hpp
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


// GenEvA header files go here
#include "common/GHelperFunctionsT.hpp"
#include "GParameterBase.hpp"
#include "GParameterT.hpp"
#include "GStdPtrVectorInterfaceT.hpp"

namespace Gem {
namespace GenEvA {

/***********************************************************************************************/
/**
 * This class shares many similarities with the GParameterCollectionT class. Instead
 * of individual values that can be modified with adaptors, however, it assumes that
 * the objects stored in it have their own adapt() function. This class has been designed
 * as a collection of GParameterT objects, hence the name.  As an example, one can create a
 * collection of GBoundedDouble objects with this class rather than a simple GDoubleCollection.
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

#ifdef GENEVATESTING
	/*******************************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() {
		bool result;

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

	/*******************************************************************************************/
	/**
	 * Initializes double-based parameters with a given value. Allows e.g. to set all
	 * floating point parameters to 0.
	 *
	 * @param val The value to be assigned to the parameters
	 */
	virtual void fixedValueInit_(const double& val)	{
		typename GParameterTCollectionT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->GParameterBase::fixedValueInit(val);
		}
	}

	/*******************************************************************************************/
	/**
	 * Multiplies double-based parameters with a given value.
	 *
	 * @param val The value to be multiplied with the parameter
	 */
	virtual void multiplyBy_(const double& val) {
		typename GParameterTCollectionT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->GParameterBase::multiplyBy(val);
		}
	}

	/*******************************************************************************************/
};


/***********************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

/**************************************************************************************************/

#endif /* GPARAMETERTCOLLECTIONT_HPP_ */
