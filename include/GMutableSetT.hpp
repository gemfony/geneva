/**
 * @file GMutableSetT.hpp
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
#include <typeinfo>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost header files go here

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GMUTABLESETT_HPP_
#define GMUTABLESETT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


#include "GIndividual.hpp"
#include "GParameterBase.hpp"
#include "GObject.hpp"
#include "GHelperFunctionsT.hpp"
#include "GStdPtrVectorInterfaceT.hpp"

namespace Gem {
namespace GenEvA {

/******************************************************************************************/
/**
 * This class forms the basis for many user-defined individuals. It acts as a collection
 * of different parameter sets. User individuals can thus contain a mix of parameters of
 * different types, such as double, GenEvA::bit, long, ... . Derived classes must implement
 * a useful operator=(). It is also assumed that template arguments have the GObject and the
 * GMutableI interfaces, in particular the load(), clone() and mutate() functions.
 */
template <typename T>
class GMutableSetT:
	public GIndividual,
	public GStdPtrVectorInterfaceT<T>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;
      ar & make_nvp("GIndividual",boost::serialization::base_object<GIndividual>(*this));
      ar & make_nvp("GStdPtrVectorInterfaceT_T", boost::serialization::base_object<GStdPtrVectorInterfaceT<T> >(*this));
    }
    ///////////////////////////////////////////////////////////////////////

public:
	/**********************************************************************************/
	/**
	 * The default constructor. No local data, hence nothing to do.
	 */
	GMutableSetT()
		:GIndividual(),
		 GStdPtrVectorInterfaceT<T>()
	{ /* nothing */	}

	/**********************************************************************************/
	/**
	 * The copy constructor. Note that we cannot rely on the operator=() of the vector
	 * here, as we do not know the actual type of T objects.
	 *
	 * @param cp A copy of another GMutableSetT<T> object
	 */
	GMutableSetT(const GMutableSetT<T>& cp):
		GIndividual(cp),
		GStdPtrVectorInterfaceT<T>(cp)
	{ /* nothing */ }

	/**********************************************************************************/
	/**
	 * The destructor. As we use smart pointers, we do not need to take care of the
	 * data in the vector ourselves.
	 */
	virtual ~GMutableSetT()
	{ /* nothing */ }

	/**********************************************************************************/
	/** @brief Creates a deep clone of this object. Purely virtual, so this class cannot be instantiated */
	virtual GObject* clone() const = 0;

	/**********************************************************************************/
	/**
	 * Loads the data of another GParameterBase object, camouflaged as a GObject
	 *
	 * @param cp A copy of another GMutableSetT object, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp){
		// Convert cp into local format
	    const GMutableSetT<T> *gms_load = this->conversion_cast(cp, this);

	    // No local data - load the parent class'es data
	    GIndividual::load(cp);
		GStdPtrVectorInterfaceT<T>::operator=(*gms_load);
	}

	/**********************************************************************************/
	/**
	 * Checks for equality with another GNumCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GMutableSetT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GMutableSetT<T>& cp) const {
		return GMutableSetT<T>::isEqualTo(cp, boost::logic::indeterminate);
	}

	/**********************************************************************************/
	/**
	 * Checks for inequality with another GMutableSetT<T> object
	 *
	 * @param  cp A constant reference to another GMutableSetT<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GMutableSetT<T>& cp) const {
		return !GMutableSetT<T>::isEqualTo(cp, boost::logic::indeterminate);
	}

	/**********************************************************************************/
	/**
	 * Checks for equality with another GMutableSetT<T> object. This function assumes
	 * that each T has its own isEqualTo function, i.e. follows the Geneva rules.
	 *
	 * @param  cp A constant reference to another GMutableSetT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool isEqualTo(const GObject& cp, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
	    using namespace Gem::Util;

		// Check that we are indeed dealing with a GIndividual reference
		const GMutableSetT<T> *gmst_load = GObject::conversion_cast(&cp,  this);

		// Check equality of the parent class
		if(!GIndividual::isEqualTo(*gmst_load, expected)) return false;
		if(!GStdPtrVectorInterfaceT<T>::checkIsEqualTo(*gmst_load, expected)) return false;

		// No local data

		return true;
	}

	/**********************************************************************************/
	/**
	 * Checks for similarity with another GMutableSetT<T> object.
	 * As we have no local data, we just check for equality of the parent class.
	 *
	 * @param  cp A constant reference to another GMutableSetT<T> object
	 * @param limit A double value specifying the acceptable level of differences of floating point values
	 * @return A boolean indicating whether both objects are similar to each other
	 */
	virtual bool isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
	    using namespace Gem::Util;

		// Check that we are indeed dealing with a GIndividual reference
		const GMutableSetT<T> *gmst_load = GObject::conversion_cast(&cp,  this);

		// Check similarity of the parent class
		if(!GIndividual::isSimilarTo(*gmst_load, limit, expected)) return false;
		if(!GStdPtrVectorInterfaceT<T>::checkIsSimilarTo(*gmst_load, limit, expected)) return false;

		// No local data

		return true;
	}

	/*******************************************************************************************/
	/**
	 * Swap another object's vector with ours
	 */
	inline void swap(GMutableSetT<T>& cp) { GStdPtrVectorInterfaceT<T>::swap(cp.data); }

	/*******************************************************************************************/
	/**
	 * Swap another vector with ours
	 */
	inline void swap(std::vector<boost::shared_ptr<T> >& cp_data) { GStdPtrVectorInterfaceT<T>::swap(cp_data); }

protected:
	/**********************************************************************************/
	/** @brief The actual fitness calculation takes place here */
	virtual double fitnessCalculation() = 0;

	/**********************************************************************************/
	/**
	 * The actual mutation operations. Easy, as we know that all GParameterBase objects
	 * in this object must implement the mutate() function.
	 */
	virtual void customMutations(){
		typename GMutableSetT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) (*it)->mutate();
	}

	/**********************************************************************************/
	/**
	 * Re-implementation of a corresponding function in GStdPtrVectorInterface.
	 * Make the vector wrapper purely virtual allows the compiler to perform
	 * further optimizations.
	 */
	virtual void dummyFunction() { /* nothing */ }

	/**********************************************************************************/
};

} /* namespace GenEvA */
} /* namespace Gem */

/**************************************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) macro. Needed for Boost.Serialization
 */
namespace boost {
  namespace serialization {
    template<typename T>
    struct is_abstract<Gem::GenEvA::GMutableSetT<T> > : public boost::true_type {};
    template<typename T>
    struct is_abstract< const Gem::GenEvA::GMutableSetT<T> > : public boost::true_type {};
  }
}

/**************************************************************************************************/

#endif /* GMUTABLESETT_HPP_ */
