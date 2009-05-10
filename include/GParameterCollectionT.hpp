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
	void serialize(Archive & ar, const unsigned int) {
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
		return GParameterCollectionT<T>::isEqualTo(cp, boost::logic::indeterminate);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another GParameterCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GParameterCollectionT<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GParameterCollectionT<T>& cp) const {
		return !Gem::GenEvA::GParameterCollectionT<T>::isEqualTo(cp, boost::logic::indeterminate);
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GParameterCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GParameterCollectionT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool isEqualTo(const GObject& cp, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
	    using namespace Gem::Util;

		// Check that we are indeed dealing with a GParamterCollectionT reference
		const GParameterCollectionT<T> *gpct_load = GObject::conversion_cast(&cp,  this);

		// Check equality of the parent class
		if(!Gem::GenEvA::GParameterBaseWithAdaptorsT<T>::isEqualTo(*gpct_load, expected)) return false;
		if(!Gem::GenEvA::GStdSimpleVectorInterfaceT<T>::checkIsEqualTo(*gpct_load, expected)) return false;

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
	bool isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
	    using namespace Gem::Util;

		// Check that we are indeed dealing with a GParamterCollectionT reference
		const GParameterCollectionT<T> *gpct_load = GObject::conversion_cast(&cp,  this);

		// Check similarity of the parent class
		if(!Gem::GenEvA::GParameterBaseWithAdaptorsT<T>::isSimilarTo(*gpct_load, limit, expected)) return false;
		if(!Gem::GenEvA::GStdSimpleVectorInterfaceT<T>::checkIsSimilarTo(*gpct_load, limit, expected)) return false;

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
		GParameterBaseWithAdaptorsT<T>::setRnrGenerationMode(rnrGenMode);
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
	 * Allows to mutate the values stored in this class. applyAdaptor expects a reference
	 * to a std::vector<T>. As we are derived from a wrapper of this class, we can just pass
	 * a reference to its data vector to the function.
	 */
	virtual void mutate() {
		GParameterBaseWithAdaptorsT<T>::applyAdaptor(GStdSimpleVectorInterfaceT<T>::data);
	}

	/*******************************************************************************************/
	/**
	 * Swap another object's vector with ours
	 */
	inline void swap(GParameterCollectionT<T>& cp) { GStdSimpleVectorInterfaceT<T>::swap(cp.data); }

	/*******************************************************************************************/
	/**
	 * Swap another vector with ours
	 */
	virtual inline void swap(std::vector<T>& cp_data) { GStdSimpleVectorInterfaceT<T>::swap(cp_data); }


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
    struct is_abstract<Gem::GenEvA::GParameterCollectionT<T> > : public boost::true_type {};
    template<typename T>
    struct is_abstract< const Gem::GenEvA::GParameterCollectionT<T> > : public boost::true_type {};
  }
}

/**************************************************************************************************/

#endif /* GPARAMETERCOLLECTIONT_HPP_ */
