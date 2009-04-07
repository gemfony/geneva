/**
 * @file GParameterCollectionT.hpp
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
#include <cmath>

// Boost header files go here

#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GPARAMETERCOLLECTIONT_HPP_
#define GPARAMETERCOLLECTIONT_HPP_

// GenEvA header files go here
#include "GParameterBaseWithAdaptorsT.hpp"
#include "GObject.hpp"
#include "GStdSimpleVectorInterfaceT.hpp"

namespace Gem {
namespace GenEvA {

/***********************************************************************************************/
/**
 * A class holding a collection of mutable parameters - usually just an atomic value (double,
 * long, bool, ...).
 */
template<typename T>
class GParameterCollectionT
	:public GParameterBaseWithAdaptorsT<T>,
	 public GStdSimpleVectorInterfaceT<T>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GParameterBaseWithAdaptorsT_T", boost::serialization::base_object<GParameterBaseWithAdaptorsT<T> >(*this));
		ar & make_nvp("GStdSimpleVectorInterfaceT_T", boost::serialization::base_object<GStdSimpleVectorInterfaceT<T> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/*******************************************************************************************/
	/**
	 * The default constructor
	 */
	GParameterCollectionT() :
		GParameterBaseWithAdaptorsT<T> (),
		GStdSimpleVectorInterfaceT<T>()
	{ /* nothing */ }

	/*******************************************************************************************/
	/**
	 * The copy constructor
	 *
	 * @param cp A copy of another GParameterCollectionT<T> object
	 */
	GParameterCollectionT(const GParameterCollectionT<T>& cp) :
		GParameterBaseWithAdaptorsT<T> (cp),
		GStdSimpleVectorInterfaceT<T>(cp)
	{  /* nothing */ }

	/*******************************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GParameterCollectionT()
	{ /* nothing */ }

	/*******************************************************************************************/
	/**
	 * A standard assignment operator.
	 *
	 * @param cp A copy of another GParameterCollectionT object
	 * @return A constant reference to this object
	 */
	const GParameterCollectionT<T>& operator=(const GParameterCollectionT<T>& cp)
	{
		GParameterCollectionT<T>::load(cp);
		return *this;
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GParameterCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GParameterCollectionT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GParameterCollectionT<T>& cp) const {
		return GParameterCollectionT<T>::isEqualTo(cp);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another GParameterCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GParameterCollectionT<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GParameterCollectionT<T>& cp) const {
		return !GParameterCollectionT<T>::isEqualTo(cp);
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GParameterCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GParameterCollectionT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool isEqualTo(const GParameterCollectionT<T>& cp) const {
		// Check equality of the parent class
		if(!GParameterBaseWithAdaptorsT<T>::isEqualTo(cp)) return false;
		if(!GStdSimpleVectorInterfaceT<T>::isEqualTo(cp)) return false;

		return true;
	}

	/*******************************************************************************************/
	/**
	 * Checks for similarity with another GParameterCollectionT<T> object. We need to
	 * create a specialization for typeof(T) == typof(double), as we need to check for
	 * similarity in this case. For all other types we currently just check for equality.
	 *
	 * @param  cp A constant reference to another GParameterCollectionT<T> object
	 * @param limit A double value specifying the acceptable level of differences of floating point values
	 * @return A boolean indicating whether both objects are similar to each other
	 */
	virtual bool isSimilarTo(const GParameterCollectionT<T>& cp, const double& limit=0) const {
		// Check similarity of the parent class
		if(!GParameterBaseWithAdaptorsT<T>::isSimilarTo(cp, limit)) return false;
		if(!GStdSimpleVectorInterfaceT<T>::isSimilarTo(cp, limit)) return false;

		return true;
	}

	/*******************************************************************************************/
	/**
	 * Creates a deep clone of this object. Purely virtual, so this class cannot be instantiated.
	 */
	virtual GObject* clone() const = 0;

	/*******************************************************************************************/
	/**
	 * Loads the data of another GParameterCollectionT<T> object, camouflaged as a GObject
	 *
	 * @param cp A copy of another GParameterCollectionT<T> object, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp) {
		// Convert cp into local format and check for self-assignment
		const GParameterCollectionT<T> *gpct = this->conversion_cast(cp, this);

		// Load our parent class'es data ...
		GParameterBaseWithAdaptorsT<T>::load(cp);
		GStdSimpleVectorInterfaceT<T>::operator=(*gpct);
	}

	/*******************************************************************************************/
	/**
	 * Allows to mutate the values stored in this class. If more than one adaptor was registered,
	 * all will be applied to the value. applyFirstAdaptor and applyAllAdaptors expects a reference
	 * to a vector<T>. As we are derived from this class, we can just pass a reference to ourselves
	 * to the functions.
	 */
	virtual void mutate() {
		if (GParameterBaseWithAdaptorsT<T>::numberOfAdaptors() == 1) {
			GParameterBaseWithAdaptorsT<T>::applyFirstAdaptor(GStdSimpleVectorInterfaceT<T>::data);
		} else {
			GParameterBaseWithAdaptorsT<T>::applyAllAdaptors(GStdSimpleVectorInterfaceT<T>::data);
		}
	}

protected:
	/*******************************************************************************************/
	/**
	 * Re-implementation of a corresponding function in GStdSimpleVectorInterface.
	 * Make the vector wrapper purely virtual allows the compiler to perform
	 * further optimizations.
	 */
	virtual void dummyFunction() { /* nothing */ }
};

/*********************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

/**************************************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) macro. Needed for Boost.Serialization
 */
namespace boost {
  namespace serialization {
    template<typename T>
    struct is_abstract<Gem::GenEvA::GParameterCollectionT<T> > : boost::true_type {};
    template<typename T>
    struct is_abstract< const Gem::GenEvA::GParameterCollectionT<T> > : boost::true_type {};
  }
}

/**************************************************************************************************/

#endif /* GPARAMETERCOLLECTIONT_HPP_ */
