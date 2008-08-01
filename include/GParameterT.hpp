/**
 * @file
 */

/* GParameterT.hpp
 *
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
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

// Boost header files go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GPARAMETERT_HPP_
#define GPARAMETERT_HPP_

// GenEvA header files go here
#include "GParameterBaseWithAdaptorsT.hpp"
#include "GObject.hpp"
#include "GLogger.hpp"
#include "GenevaExceptions.hpp"

namespace Gem {
namespace GenEvA {

/*******************************************************************************************/
/**
 * A class holding a single, mutable parameter - usually just an atomic value (double, long,
 * GenEvA::bit, ...). The class is non-virtual, so that it is possible to store simple values
 * in this class without too much fuss.
 */
template <class T>
class GParameterT
	:public GParameterBaseWithAdaptorsT<T>
{
public:
	/*******************************************************************************************/
	/**
	 * The default constructor
	 */
	GParameterT()
		:GParameterBaseWithAdaptorsT(),
		 val_((T)NULL)
	{ /* nothing */ }

	/*******************************************************************************************/
	/**
	 * The copy constructor
	 *
	 * @param cp A copy of another GParameterT<T> object
	 */
	GParameterT(const GParameterT<T>& cp)
		:GParameterBaseWithAdaptorsT<T>(cp),
		 val_(cp.val_)
	{ /* nothing */	}

	/*******************************************************************************************/
	/**
	 * Initialization by contained value
	 *
	 * @param val The new value of val_
	 */
	explicit GParameterT(const T& val)
		:GParameterBaseWithAdaptorsT<T>(),
		 val_(val)
	{ /* nothing */	}

	/*******************************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GParameterT()
	{ /* nothing */ }

	/*******************************************************************************************/
	/**
	 * A standard assignment operator.
	 *
	 * @param cp A copy of another GParameterT object
	 * @return A constant reference to this object
	 */
	const GParameterT<T>& operator=(const GParameterT<T>& cp){
		GParameterT<T>::load(cp);
		return *this;
	}

	/*******************************************************************************************/
	/**
	 * An assignment operator that allows us to set val_
	 *
	 * @param val The new value for val_
	 * @return The new value of val_
	 */
	virtual T operator=(T val){
		val_ = val;
		return val_;
	}

	/*******************************************************************************************/
	/**
	 * Retrieval of the value
	 *
	 * @return The value of val_
	 */
	inline T value(){
		return val_;
	}

	/*******************************************************************************************/
	/**
	 * Creates a deep clone of this object.
	 *
	 * @return A copy of this object, camouflaged as a GObject
	 */
	virtual GObject* clone(){
		return new GParameterT<T>(*this);
	}

	/*******************************************************************************************/
	/**
	 * Resets the class to its initial state.
	 */
	virtual void reset(){
		// First reset our local data
		val_ = (T)NULL;

		// Then reset our parent class
		GParameterBaseWithAdaptorsT<T>::reset();
	}

	/*******************************************************************************************/
	/**
	 * Loads the data of another GObject
	 *
	 * @param cp A copy of another GParameterT<T> object, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp){
		// Convert cp into local format
		const GParameterT<T> *gpt = checkedConversion(cp, this);

		// Load our parent class'es data ...
		GParameterBaseWithAdaptorsT<T>::load(cp);

		// ... and then our own data
		val_ = gpt->val_;
	}

	/*******************************************************************************************/
	/**
	 * Allows to mutate the value stored in this class. If more than one adaptor was registered,
	 * all will be applied to the value.
	 */
	virtual void mutate(void){
		if(numberOfAdaptors() == 1){
			GParameterBaseWithAdaptorsT<T>::applyFirstAdaptor(val_);
		}
		else {
			GParameterBaseWithAdaptorsT<T>::applyAllAdaptors(val_);
		}
	}

protected:
	/*******************************************************************************************/

	T val_; ///< The internal representation of our value
};

}} /* namespace Gem::GenEvA */

#endif /* GPARAMETERT_HPP_ */
