/**
 * @file
 */

/* GParameterCollectionT.hpp
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
#include <vector>

// Boost header files go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GPARAMETERCOLLECTIONT_HPP_
#define GPARAMETERCOLLECTIONT_HPP_

// GenEvA header files go here
#include "GParameterBaseWithAdaptorsT.hpp"
#include "GObject.hpp"
#include "GHelperFunctionsT.hpp"

namespace Gem {
namespace GenEvA {

/***********************************************************************************************/
/**
 * A class holding a collection of mutable parameters - usually just an atomic value (double,
 * long, GenEvA::bit, ...). The class is non-virtual, so that it is possible to store simple
 * values in this class without too much fuss.
 */
template<class T>
class GParameterCollectionT: public GParameterBaseWithAdaptorsT<T> ,
		public std::vector<T> {
public:
	/*******************************************************************************************/
	/**
	 * The default constructor
	 */
	GParameterCollectionT() :
		GParameterBaseWithAdaptorsT<T> () { /* nothing */
	}

	/*******************************************************************************************/
	/**
	 * The copy constructor
	 *
	 * @param cp A copy of another GParameterCollectionT<T> object
	 */
	GParameterCollectionT(const GParameterCollectionT<T>& cp) :
		GParameterBaseWithAdaptorsT<T> (cp) {
		typename std::vector<T>::const_iterator cit;
		for (cit = cp.begin(); cit != cp.end(); ++cit)
			this->push_back(*cit);
	}

	/*******************************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GParameterCollectionT() { /* nothing */
	}

	/*******************************************************************************************/
	/**
	 * A standard assignment operator.
	 *
	 * @param cp A copy of another GParameterCollectionT object
	 * @return A constant reference to this object
	 */
	const GParameterCollectionT<T>& operator=(
			const GParameterCollectionT<T>& cp) {
		GParameterCollectionT<T>::load(cp);
		return *this;
	}

	/*******************************************************************************************/
	/**
	 * Creates a deep clone of this object.
	 *
	 * @return A copy of this object, camouflaged as a GObject
	 */
	virtual GObject* clone() {
		return new GParameterCollectionT<T> (*this);
	}

	/*******************************************************************************************/
	/**
	 * Resets the class to its initial state.
	 */
	virtual void reset() {
		// First reset our local data ...
		this->clear();

		// .. and then our parent class'es data
		GParameterBaseWithAdaptorsT<T>::reset();
	}

	/*******************************************************************************************/
	/**
	 * Loads the data of another GObject
	 *
	 * @param cp A copy of another GParameterCollectionT<T> object, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp) {
		// Convert cp into local format
		const typename GParameterCollectionT<T> *gpct = this->checkedConversion<typename GParameterCollectionT<T> >(cp, this);

		// Load our parent class'es data ...
		GParameterBaseWithAdaptorsT<T>::load(cp);

		// ... and then our own data
		typename std::vector<T>::iterator it;
		typename std::vector<T>::const_iterator cit;

		for (cit = gpct->begin(), it = this->begin(); cit != gpct->end(), it
				!= this->end(); ++it, ++cit) {
			(*it) = (*cit);
		}

		std::size_t gpct_sz = gpct->size(), this_sz = this->size();
		if (gpct->size() == this->size())
			return; // Likely the most probable case
		else if (gpct_sz > this_sz) {
			for (cit = gpct->begin() + this_sz; cit != gpct->end(); ++cit) {
				this->push_back(*cit);
			}
		} else if (this_sz > gpct_sz)
			this->resize(gpct_sz); // get rid of the surplus elements
	}

	/*******************************************************************************************/
	/**
	 * Allows to mutate the values stored in this class. If more than one adaptor was registered,
	 * all will be applied to the value. applxFirstAdaptor and applyAllAdaptors expects a reference
	 * to a vector<T>. As we are derived from this class, we can just pass a reference to ourselves
	 * to the functions.
	 */
	virtual void mutate(void) {
		if (GParameterBaseWithAdaptorsT<T>::numberOfAdaptors() == 1) {
			GParameterBaseWithAdaptorsT<T>::applyFirstAdaptor(*this);
		} else {
			GParameterBaseWithAdaptorsT<T>::applyAllAdaptors(*this);
		}
	}
	/*******************************************************************************************/
};

}
} /* namespace Gem::GenEvA */

#endif /* GPARAMETERCOLLECTIONT_HPP_ */
