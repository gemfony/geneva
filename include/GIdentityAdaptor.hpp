/**
 * @file GIdentityAdaptorT.hpp
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


// Standard headers go here
#include <cmath>
#include <string>
#include <sstream>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/limits.hpp>
#include <boost/cast.hpp>

#ifndef GIDENTITYADAPTORT_HPP_
#define GIDENTITYADAPTORT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA headers go here

#include "GAdaptorT.hpp"
#include "GBoundedDouble.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GObject.hpp"
#include "GRandom.hpp"
#include "GenevaExceptions.hpp"
#include "GEnums.hpp"

namespace Gem {
namespace GenEvA {

/************************************************************************************************/
/**
 * GIdentityAdaptorT simply returns the unchanged value. It is used as the default adaptor, when
 * no adaptor has been registered or if certain values should remain unchanged.
 */
template<typename T>
class GIdentityAdaptorT:
	public GAdaptorT<T>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GAdaptorT", boost::serialization::base_object<GAdaptorT<T> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/********************************************************************************************/
	/**
	 * The standard constructor.
	 */
	GIdentityAdaptorT()
		:GAdaptorT<T> (0.)
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * This constructor takes an argument, that specifies the (initial) probability
	 * for the mutation of the argument. Note that it is no mistake that we do not use
	 * this argument -- GIdentityAdaptorT is not intended to do any changes at all,
	 * so we can just as well set the likelihood to 0. .
	 *
	 * @param prob The probability for a flip
	 */
	explicit GIdentityAdaptorT(const double& prob)
		:GAdaptorT<T>(0.)
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * A standard copy constructor.
	 *
	 * @param cp Another GIdentityAdaptorT object
	 */
	GIdentityAdaptorT(const GIdentityAdaptorT<T>& cp)
		:GAdaptorT<T>(cp)
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * The standard destructor. Empty, as we have no local, dynamically
	 * allocated data.
	 */
	~GIdentityAdaptorT()
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * A standard assignment operator for GIdentityAdaptorT objects,
	 *
	 * @param cp A copy of another GIdentityAdaptorT object
	 */
	const GIdentityAdaptorT<T>& operator=(const GIdentityAdaptorT<T>& cp)
	{
		GIdentityAdaptorT<T>::load(&cp);
		return *this;
	}

	/********************************************************************************************/
	/**
	 * This function loads the data of another GIdentityAdaptorT, camouflaged as a GObject.
	 *
	 * @param A copy of another GIdentityAdaptorT, camouflaged as a GObject
	 */
	void load(const GObject *cp)
	{
		// Convert GObject pointer to local format
		// (also checks for self-assignments in DEBUG mode)
		const GIdentityAdaptorT<T> *gifa = this->conversion_cast(cp, this);

		// Load the data of our parent class ...
		GAdaptorT<T>::load(cp);

		// no local data
	}


	/********************************************************************************************/
	/**
	 * This function creates a deep copy of this object
	 *
	 * @return A deep copy of this object
	 */
	GObject *clone() const
	{
		return new GIdentityAdaptorT<T>(*this);
	}

	/********************************************************************************************/
	/**
	 * Checks for equality with another GIdentityAdaptorT<T> object
	 *
	 * @param  cp A constant reference to another GIdentityAdaptorT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GIdentityAdaptorT<T>& cp) const {
		return GIdentityAdaptorT<T>::isEqualTo(cp, boost::logic::indeterminate);
	}

	/********************************************************************************************/
	/**
	 * Checks for inequality with another GIdentityAdaptorT<T> object
	 *
	 * @param  cp A constant reference to another GIdentityAdaptorT<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GIdentityAdaptorT<T>& cp) const {
		return !GIdentityAdaptorT<T>::isEqualTo(cp, boost::logic::indeterminate);
	}

	/********************************************************************************************/
	/**
	 * Checks for equality with another GIdentityAdaptorT<T> object Equality means
	 * that all individual sub-values are equal and that the parent class is equal.
	 *
	 * @param  cp A constant reference to another GIdentityAdaptorT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool isEqualTo(const GObject& cp, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
	    using namespace Gem::Util;

		// Check that we are indeed dealing with a GIdentityAdaptorT reference
		const GIdentityAdaptorT<T> *gifat_load = GObject::conversion_cast(&cp,  this);

		// Check our parent class
		if(!GAdaptorT<T>::isEqualTo(*gifat_load, expected)) return false;

		// no local data

		return true;
	}

	/********************************************************************************************/
	/**
	 * Checks for similarity with another GIdentityAdaptorT<T> object. Similarity means
	 * that all double values are similar to each other within a given limit and that all other
	 * values are equal. Also, parent classes must be similar to each other.
	 *
	 * @param  cp A constant reference to another GIdentityAdaptorT<T> object
	 * @param limit A double value specifying the acceptable level of differences of floating point values
	 * @return A boolean indicating whether both objects are similar to each other
	 */
	virtual bool isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
	    using namespace Gem::Util;

		// Check that we are indeed dealing with a GIdentityAdaptorT reference
		const GIdentityAdaptorT<T> *gifat_load = GObject::conversion_cast(&cp,  this);

		// First check our parent class
		if(!GAdaptorT<T>::isSimilarTo(*gifat_load, limit, expected))  return false;

		// no local data

		return true;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the id of the adaptor.
	 *
	 * @return The id of the adaptor
	 */
	virtual Gem::GenEvA::adaptorId getAdaptorId() const {
		return GIDENTITYADAPTOR;
	}

protected:
	/********************************************************************************************/
	/**
	 * We do not want any changes performed, so this function is just empty.
	 *
	 * @param value The value to be mutated
	 */
	virtual void customMutations(T& value) {
		return; // nothing
	}
};

/************************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GIDENTITYADAPTORT_HPP_ */
