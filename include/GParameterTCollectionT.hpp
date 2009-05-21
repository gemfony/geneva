/**
 * @file GParameterTCollectionT.hpp
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

// Boost header files go here

#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GPARAMETERTCOLLECTIONT_HPP_
#define GPARAMETERTCOLLECTIONT_HPP_

// GenEvA header files go here
#include "GParameterBaseWithAdaptorsT.hpp"
#include "GParameterT.hpp"
#include "GHelperFunctionsT.hpp"
#include "GStdPtrVectorInterfaceT.hpp"

namespace Gem {
namespace GenEvA {

/***********************************************************************************************/
/**
 * This class shares many similarities with the GParameterCollectionT class. Instead
 * of individual values that can be modified with adaptors, however, it assumes that
 * the objects stored in it have their own mutate() function. This class has been designed
 * as a collection of GParameterT objects, hence the name.  As an example, one can create a
 * collection of GBoundedDouble objects with this class rather than a simple GDoubleCollection.
 * In order to facilitate memory management, the GParameterT objects are stored
 * in boost::shared_ptr objects. When supplied with a local adaptor, it is used for all
 * dependent GParameterT objects.
 */
template<typename T>
class GParameterTCollectionT
	:public GParameterBaseWithAdaptorsT<typename T::p_type >,
	 public GStdPtrVectorInterfaceT<T>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GParameterBaseWithAdaptorsT_ptype", boost::serialization::base_object<GParameterBaseWithAdaptorsT<typename T::p_type > >(*this));
		ar & make_nvp("GStdPtrVectorInterfaceT_T", boost::serialization::base_object<GStdPtrVectorInterfaceT<T> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/*******************************************************************************************/
	/**
	 * The default constructor
	 */
	GParameterTCollectionT()
		:GParameterBaseWithAdaptorsT<typename T::p_type >()
	{ /* nothing */ }

	/*******************************************************************************************/
	/**
	 * The copy constructor
	 *
	 * @param cp A copy of another GParameterTCollectionT<T> object
	 */
	GParameterTCollectionT(const GParameterTCollectionT<T>& cp)
		:GParameterBaseWithAdaptorsT<typename T::p_type >(cp),
		GStdPtrVectorInterfaceT<T>(cp)
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
		GParameterTCollectionT<T>::load(&cp);
		return *this;
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GParameterTCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GParameterTCollectionT object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GParameterTCollectionT<T>& cp) const {
		return GParameterTCollectionT<T>::isEqualTo(cp, boost::logic::indeterminate);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another GParameterTCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GParameterTCollectionT object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GParameterTCollectionT<T>& cp) const {
		return !GParameterTCollectionT<T>::isEqualTo(cp, boost::logic::indeterminate);
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GParameterTCollectionT<T> object. This function
	 * assumes that T has an isEqualTo function itself.
	 *
	 * @param  cp A constant reference to another GParameterTCollectionT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool isEqualTo(const GObject& cp, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
	    using namespace Gem::Util;

		// Check that we are indeed dealing with a GParamterCollectionT reference
		const GParameterTCollectionT<T> *gptct_load = GObject::conversion_cast(&cp,  this);

		// Check equality of the parent classes
		if(!GParameterBaseWithAdaptorsT<typename T::p_type >::isEqualTo(*gptct_load, expected)) return false;
		if(!GStdPtrVectorInterfaceT<T>::checkIsEqualTo(*gptct_load, expected)) return false;

		return true;
	}

	/*******************************************************************************************/
	/**
	 * Checks for similarity with another GParameterTCollectionT<T> object.  This function
	 * assumes that T has an isSimilarTo function itself.
	 *
	 * @param  cp A constant reference to another GParameterTCollectionT<T> object
	 * @param limit A double value specifying the acceptable level of differences of floating point values
	 * @return A boolean indicating whether both objects are similar to each other
	 */
	virtual bool isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
	    using namespace Gem::Util;

		// Check that we are indeed dealing with a GParamterCollectionT reference
		const GParameterTCollectionT<T> *gptct_load = GObject::conversion_cast(&cp,  this);

		// Check similarity of the parent classes
		if(!GParameterBaseWithAdaptorsT<typename T::p_type >::isSimilarTo(*gptct_load, limit, expected))  return false;
		if(!GStdPtrVectorInterfaceT<T>::checkIsSimilarTo(*gptct_load, limit, expected)) return false;

		return true;
	}

	/*******************************************************************************************/
	/**
	 * Loads the data of another GParameterTCollectionT<T> object, camouflaged as a GObject
	 *
	 * @param cp A copy of another GParameterTCollectionT<T> object, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp) {
		// Convert cp into local format
		const GParameterTCollectionT<T> *gptct = this->conversion_cast(cp, this);

		// Load our parent class'es data ...
		GParameterBaseWithAdaptorsT<typename T::p_type >::load(cp);
		GStdPtrVectorInterfaceT<T>::operator=(*gptct);
	}

	/*******************************************************************************************/
	/**
	 * Creates a deep clone of this object.
	 */
	virtual GObject* clone() const{
		return new GParameterTCollectionT<T>(*this);
	}

	/*******************************************************************************************/
	/**
	 * Allows to mutate the values stored in this class. We assume here that
	 * each item has its own mutate function. Hence we do not need to use or
	 * store own adaptors.
	 */
	virtual void mutate() {
		typename GParameterTCollectionT<T>::iterator it;
		if(this->hasAdaptor()) {
			for(it=this->begin(); it!=this->end(); ++it) {
				(*it)->addAdaptorNoClone(this->getAdaptor());
				(*it)->mutate();
			}
		}
		else {
			for(it=this->begin(); it!=this->end(); ++it) (*it)->mutate();
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
	 * Compares another vector object with ours
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
	 * Assign anither vector object to ours
	 */
	const std::vector<boost::shared_ptr<T> >& operator=(const std::vector<boost::shared_ptr<T> >& cp_data) {
		return GStdPtrVectorInterfaceT<T>::operator=(cp_data);
	}

	/*******************************************************************************************/

protected:
	/**
	 * Re-implementation of a corresponding function in GStdPtrVectorInterface.
	 * Make the vector wrapper purely virtual allows the compiler to perform
	 * further optimizations.
	 */
	virtual void dummyFunction() { /* nothing */ }
};


/***********************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

/**************************************************************************************************/

#endif /* GPARAMETERTCOLLECTIONT_HPP_ */
