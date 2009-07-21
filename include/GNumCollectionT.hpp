/**
 * @file GNumCollectionT.hpp
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
#include <string>
#include <sstream>
#include <vector>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost header files go here

#include <boost/cstdint.hpp>

#ifndef GNUMCOLLECTIONT_HPP_
#define GNUMCOLLECTIONT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// GenEvA header files go here
#include "GObject.hpp"
#include "GParameterCollectionT.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GenevaExceptions.hpp"

namespace Gem {
namespace GenEvA {

/**********************************************************************/
/**
 * This class represents a collection of numeric values, all modified
 * using the same algorithm. The most likely types to be stored in this
 * class are double and boost::uint32_t . By using the framework provided
 * by GParameterCollectionT, this class becomes rather simple.
 */
template <typename T>
class GNumCollectionT
	: public GParameterCollectionT<T>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GParameterCollectionT",	boost::serialization::base_object<GParameterCollectionT<T> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/******************************************************************/
	/**
	 * The default constructor.
	 */
	GNumCollectionT():
		GParameterCollectionT<T> ()
	{ /* nothing */ }

	/******************************************************************/
	/**
	 * Initialize with a number of random values within given boundaries
	 *
	 * @param nval Number of values to put into the vector
	 * @param min The lower boundary for random entries
	 * @param max The upper boundary for random entries
	 */
	GNumCollectionT(const std::size_t& nval, const T& min, const T& max){
		this->addRandomData(nval, min, max);
	}

	/******************************************************************/
	/**
	 * The standard copy constructor
	 */
	GNumCollectionT(const GNumCollectionT<T>& cp)
		:GParameterCollectionT<T> (cp)
	{ /* nothing */ }

	/******************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GNumCollectionT()
	{ /* nothing */ }

	/******************************************************************/
	/**
	 * The standard assignment operator.
	 *
	 * @param cp A copy of another GDoubleCollection object
	 * @return A constant reference to this object
	 */
	const GNumCollectionT& operator=(const GNumCollectionT<T>& cp){
		GNumCollectionT<T>::load(&cp);
		return *this;
	}

	/******************************************************************/
	/**
	 * Checks for equality with another GNumCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GNumCollectionT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GNumCollectionT<T>& cp) const {
		return GNumCollectionT<T>::isEqualTo(cp, boost::logic::indeterminate);
	}

	/******************************************************************/
	/**
	 * Checks for inequality with another GNumCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GNumCollectionT<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GNumCollectionT<T>& cp) const {
		return !GNumCollectionT<T>::isEqualTo(cp, boost::logic::indeterminate);
	}

	/******************************************************************/
	/**
	 * Checks for equality with another GNumCollectionT<T> object.
	 * As we have no local data, we just check for equality of the parent class.
	 *
	 * @param  cp A constant reference to another GNumCollectionT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool isEqualTo(const GObject& cp, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
	    using namespace Gem::Util;

		// Check that we are indeed dealing with a GNumCollectionT reference
		const GNumCollectionT<T> *gnct_load = GObject::conversion_cast(&cp,  this);

		// Check equality of the parent class
		if(!GParameterCollectionT<T>::isEqualTo(*gnct_load, expected)) return false;

		// No local data

		return true;
	}

	/******************************************************************/
	/**
	 * Checks for similarity with another GNumCollectionT<T> object.
	 * As we have no local data, we just check for equality of the parent class.
	 *
	 * @param  cp A constant reference to another GNumCollectionT<T> object
	 * @param limit A double value specifying the acceptable level of differences of floating point values
	 * @return A boolean indicating whether both objects are similar to each other
	 */
	virtual bool isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
	    using namespace Gem::Util;

		// Check that we are indeed dealing with a GNumCollectionT reference
		const GNumCollectionT<T> *gnct_load = GObject::conversion_cast(&cp,  this);

		// Check similarity of the parent class
		if(!GParameterCollectionT<T>::isSimilarTo(*gnct_load, limit, expected)) return false;

		// No local data

		return true;
	}

	/******************************************************************/
	/**
	 * Creates a deep copy of this object.
	 *
	 * @return A pointer to a deep clone of this object
	 */
	virtual GObject *clone() const {
		return new GNumCollectionT<T>(*this);
	}

	/******************************************************************/
	/**
	 * Loads the data of another GNumCollectionT<T> object,
	 * camouflaged as a GObject. We have no local data, so
	 * all we need to do is to the standard identity check,
	 * preventing that an object is assigned to itself.
	 *
	 * @param cp A copy of another GNumCollectionT<T> object, camouflaged as a GObject
	 */
	virtual void load(const GObject *cp){
		// Check that this object is not accidently assigned to itself.
		// As we do not actually do any calls with this pointer, we
		// can use the faster static_cast<>
		if(static_cast<const GNumCollectionT<T> *>(cp) == this) {
			std::ostringstream error;

			error << "In GNumCollectionT<T>::load(): Error!" << std::endl
				  << "Tried to assign an object to itself." << std::endl;

			throw geneva_error_condition(error.str());
		}

		GParameterCollectionT<T>::load(cp);
	}

	/******************************************************************/
	/**
	 * In derived classes, this function appends nval random values
	 * between min and max to this class. This function is a trap to
	 * early detect improper usage of this class.
	 *
	 * @param nval Number of values to put into the vector
	 * @param min The lower boundary for random entries
	 * @param max The upper boundary for random entries
	 */
	void addRandomData(const std::size_t& nval, const T& min, const T& max){
		std::ostringstream error;

		error << "In GNumCollectionT<T>::addRandomData(): Error!" << std::endl
			  << "This function should never have been called direclty." << std::endl;

		throw geneva_error_condition(error.str());
	}

	/******************************************************************/
};

/**********************************************************************/
// Declaration of some specializations
template<> void GNumCollectionT<double>::addRandomData(const std::size_t&, const double&, const double&);
template<> void GNumCollectionT<boost::int32_t>::addRandomData(const std::size_t&, const boost::int32_t&, const boost::int32_t&);

/**********************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GNUMCOLLECTIONT_HPP_ */
