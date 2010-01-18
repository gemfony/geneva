/**
 * @file GSwarmAdaptor.hpp
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

#ifndef GSWARMADAPTOR_HPP_
#define GSWARMADAPTOR_HPP_

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
 * This adaptor implements the mutations performed by swarm algorithms. Just like swarm algorithms
 * it is specific to double values.
 */
class GSwarmAdaptor:
public GAdaptorT<double>
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
	 * The standard constructor. We want to always perform mutations when this adaptor is called.
	 */
	GSwarmAdaptor()
	:GAdaptorT<T> ()
	 {
		GAdaptorT<double>::setMutationMode(true);
	 }

	/********************************************************************************************/
	/**
	 * A standard copy constructor.
	 *
	 * @param cp Another GSwarmAdaptor object
	 */
	GSwarmAdaptor(const GSwarmAdaptor<T>& cp)
	:GAdaptorT<T>(cp)
	 {
		GAdaptorT<double>::setMutationMode(true);
	 }

	/********************************************************************************************/
	/**
	 * The standard destructor. Empty, as we have no local, dynamically
	 * allocated data.
	 */
	~GSwarmAdaptor()
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * A standard assignment operator for GSwarmAdaptor objects,
	 *
	 * @param cp A copy of another GSwarmAdaptor object
	 */
	const GSwarmAdaptor<T>& operator=(const GSwarmAdaptor<T>& cp)
	{
		GSwarmAdaptor<T>::load(&cp);
		return *this;
	}

	/********************************************************************************************/
	/**
	 * This function loads the data of another GSwarmAdaptor, camouflaged as a GObject.
	 *
	 * @param A copy of another GSwarmAdaptor, camouflaged as a GObject
	 */
	void load(const GObject *cp)
	{
		// Convert GObject pointer to local format
		// (also checks for self-assignments in DEBUG mode)
		const GSwarmAdaptor<T> *gifa = this->conversion_cast(cp, this);
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
		return new GSwarmAdaptor<T>(*this);
			}

	/********************************************************************************************/
	/**
	 * Checks for equality with another GSwarmAdaptor<T> object
	 *
	 * @param  cp A constant reference to another GSwarmAdaptor<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GSwarmAdaptor<T>& cp) const {
		return GSwarmAdaptor<T>::isEqualTo(cp, boost::logic::indeterminate);
	}

	/********************************************************************************************/
	/**
	 * Checks for inequality with another GSwarmAdaptor<T> object
	 *
	 * @param  cp A constant reference to another GSwarmAdaptor<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GSwarmAdaptor<T>& cp) const {
		return !GSwarmAdaptor<T>::isEqualTo(cp, boost::logic::indeterminate);
	}

	/********************************************************************************************/
	/**
	 * Checks for equality with another GSwarmAdaptor<T> object Equality means
	 * that all individual sub-values are equal and that the parent class is equal.
	 *
	 * @param  cp A constant reference to another GSwarmAdaptor<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool isEqualTo(const GObject& cp, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
		using namespace Gem::Util;
		// Check that we are indeed dealing with a GSwarmAdaptor reference
		const GSwarmAdaptor<T> *gifat_load = GObject::conversion_cast(&cp,  this);
		// Check our parent class
		if(!GAdaptorT<T>::isEqualTo(*gifat_load, expected)) return false;
		// no local data
		return true;
	}

	/********************************************************************************************/
	/**
	 * Checks for similarity with another GSwarmAdaptor<T> object. Similarity means
	 * that all double values are similar to each other within a given limit and that all other
	 * values are equal. Also, parent classes must be similar to each other.
	 *
	 * @param  cp A constant reference to another GSwarmAdaptor<T> object
	 * @param limit A double value specifying the acceptable level of differences of floating point values
	 * @return A boolean indicating whether both objects are similar to each other
	 */
	virtual bool isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
		using namespace Gem::Util;
		// Check that we are indeed dealing with a GSwarmAdaptor reference
		const GSwarmAdaptor<T> *gifat_load = GObject::conversion_cast(&cp,  this);
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

	/***********************************************************************************/
	/**
	 * Prevents the mutation mode to be reset. This function is a trap.
	 *
	 * @param mutationMode The desired mode (always/never/with a given probability)
	 */
	virtual void setMutationMode(boost::logic::tribool mutationMode) {
		std::ostringstream error;
		error << "In GSwarmAdaptor::setMutationMode(): Error!" << std::endl
			  << "This function should not have been called for this adaptor" << std::endl;
		throw(Gem::GenEvA::geneva_error_condition(error.str()));
	}

protected:
	/********************************************************************************************/
	/**
	 * This is a trap in case this function is called for a type the class was not designed for.
	 *
	 * @param value The value to be mutated
	 */
	virtual void customMutations(T& value) {
#ifdef DEBUG
		if(typeid(T) != typeid(double) &&
				typeid(T) != typeid(bool) &&
				typeid(T) != typeid(char) &&
				typeid(T) != typeid(boost::int32_t)) {
			std::ostringstream error;
			error << "In GSwarmAdaptor::customMutations() : Error!" << std::endl
					<< "Class was instantiated with a type it was not designed for." << std::endl
					<< "Typeid is " << typeid(value).name() << std::endl;
			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}
#endif

		return; // nothing
	}
};

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GSWARMADAPTOR_HPP_ */
