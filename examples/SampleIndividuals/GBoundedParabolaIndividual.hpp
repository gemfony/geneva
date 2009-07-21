/**
 * @file BoundedGParabolaIndividual.hpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
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
#include <iostream>
#include <cmath>
#include <sstream>
#include <vector>

// Boost header files go here
#include <boost/shared_ptr.hpp>

#ifndef GBOUNDEDPARABOLAINDIVIDUAL_HPP_
#define GBOUNDEDPARABOLAINDIVIDUAL_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA header files go here
#include "GParameterSet.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GBoundedDoubleCollection.hpp"
#include "GBoundedDouble.hpp"

namespace Gem
{
namespace GenEvA
{

/************************************************************************************************/
/**
 * This individual searches for the minimum of a simple parabola in n dimensions. It is meant
 * as an example of how to set up custom individuals. In contrast to the GParabolaIndividual,
 * this class stores collections of GBoundedDouble objects.
 */
class GBoundedParabolaIndividual
	:public GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & make_nvp("GParameterSet",
				boost::serialization::base_object<GParameterSet>(*this));
		// add all local variables here, if you want them to be serialized. E.g.:
		// ar & make_nvp("myLocalVar_",myLocalVar_);
		// This also works with objects, if they have a corresponding serialize() function.
	}
	///////////////////////////////////////////////////////////////////////

public:
	/********************************************************************************************/
	/**
	 * The default constructor.
	 */
	GBoundedParabolaIndividual()
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * A standard copy constructor
	 */
	GBoundedParabolaIndividual(const GBoundedParabolaIndividual& cp)
		:GParameterSet(cp)
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GBoundedParabolaIndividual()
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * A standard assignment operator
	 */
	const GBoundedParabolaIndividual& operator=(const GBoundedParabolaIndividual& cp){
		GBoundedParabolaIndividual::load(&cp);
		return *this;
	}

	/********************************************************************************************/
	/**
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object, camouflaged as a GObject
	 */
	virtual GObject* clone() const {
		return new GBoundedParabolaIndividual(*this);
	}

	/********************************************************************************************/
	/**
	 * Loads the data of another GBoundedParabolaIndividual, camouflaged as a GObject
	 *
	 * @param cp A copy of another GBoundedParabolaIndividual, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp) {
		// We have no local data. Hence we can just pass the pointer to our parent class.
		// Note that we'd have to use the GObject::conversion_cast() function otherwise.
		GParameterSet::load(cp);
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GBoundedParabolaIndividual object
	 *
	 * @param  cp A constant reference to another GBoundedParabolaIndividual object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GBoundedParabolaIndividual& cp) const {
		return GBoundedParabolaIndividual::isEqualTo(cp, boost::logic::indeterminate);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another GBoundedParabolaIndividual object
	 *
	 * @param  cp A constant reference to another GBoundedParabolaIndividual object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GBoundedParabolaIndividual& cp) const {
		return !GBoundedParabolaIndividual::isEqualTo(cp, boost::logic::indeterminate);
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GBoundedParabolaIndividual object.  If T is an object type,
	 * then it must implement operator!= .
	 *
	 * @param  cp A constant reference to another GBoundedParabolaIndividual object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool isEqualTo(const GObject& cp, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
		// Check that we are indeed dealing with a GBoundedNumT<T> reference
		const GBoundedParabolaIndividual *gbpi_load = GObject::conversion_cast(&cp,  this);

		// Check equality of the parent class
		if(!GParameterSet::isEqualTo(*gbpi_load, expected)) return false;

		return true;
	}

	/*******************************************************************************************/
	/**
	 * Checks for similarity with another GBoundedParabolaIndividual object.  As we do not know the
	 * type of T, we need to create a specialization of this function for typeof(T)==double
	 *
	 * @param  cp A constant reference to another GBoundedParabolaIndividual object
	 * @param limit A double value specifying the acceptable level of differences of floating point values
	 * @return A boolean indicating whether both objects are similar to each other
	 */
	virtual bool isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
		// Check that we are indeed dealing with a GBoundedNumT<T> reference
		const GBoundedParabolaIndividual *gbpi_load = GObject::conversion_cast(&cp,  this);

		// Check equality of the parent class
		if(!GParameterSet::isSimilarTo(*gbpi_load, limit, expected)) return false;

		return true;
	}

protected:
	/********************************************************************************************/
	/**
	 * The actual fitness calculation takes place here
	 *
	 * @return The value of this object
	 */
	virtual double fitnessCalculation(){
		// Compile in DEBUG mode in order to check this conversion
		// The desired collection is in the first position, hence we pass a 0 to the function.
		boost::shared_ptr<GBoundedDoubleCollection> gbdc_load = pc_at<GBoundedDoubleCollection>(0);

		// Great - now we can do the actual calculations. We do this the fancy way ...
		// Note that (*it) will be of type boost::shared_ptr<GBoundedDouble> .
		double result = 0;
		GBoundedDoubleCollection::iterator it;
		for(it=gbdc_load->begin(); it!=gbdc_load->end(); ++it)
			result += std::pow((*it)->value(), 2);

		return result;
	}

	/********************************************************************************************/
};

} /* namespace GenEvA */
} /* namespace Gem */

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GBoundedParabolaIndividual)

#endif /* GBOUNDEDPARABOLAINDIVIDUAL_HPP_ */
