/**
 * @file GParabolaIndividual.hpp
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
#include <iostream>
#include <cmath>
#include <sstream>
#include <vector>

// Boost header files go here
#include <boost/shared_ptr.hpp>

#ifndef GPARABOLAINDIVIDUAL_HPP_
#define GPARABOLAINDIVIDUAL_HPP_

// GenEvA header files go here
#include "GDoubleCollection.hpp"
#include "GParameterSet.hpp"
#include "GDoubleGaussAdaptor.hpp"

namespace Gem
{
namespace GenEvA
{

/************************************************************************************************/
/**
 * This individual searches for the minimum of a simple parabola in n dimensions. It is meant
 * as an example of how to set up custom individuals.
 */
class GParabolaIndividual
	:public GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & make_nvp("ParameterSet",
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
	GParabolaIndividual()
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * A constructor which initializes the individual with a suitable set of random double values.
	 *
	 * @param sz The desired size of the double collection
	 * @param min The minimum value of the random numbers to fill the collection
	 * @param max The maximum value of the random numbers to fill the collection
	 * @param as The number of calls to GDoubleGaussAdaptor::mutate after which mutation should be adapted
	 */
	GParabolaIndividual(std::size_t sz, double min, double max, boost::uint32_t as){
		// Set up a GDoubleCollection with sz values, each initialized
		// with a random number in the range [min,max[
		boost::shared_ptr<GDoubleCollection> gdc(new GDoubleCollection(sz,min,max));

		// Set up and register an adaptor for the collection, so it
		// knows how to be mutated. We want a sigma of 1, a sigma-adaption of 0.001, a
		// minimum sigma of 0.000001 and a maximum sigma of 5.
		boost::shared_ptr<GDoubleGaussAdaptor> gdga(new GDoubleGaussAdaptor(1.,0.001,0.000001,5));
		gdga->setAdaptionThreshold(as);

		gdc->addAdaptor(gdga);

		// Make the parameter collection known to this individual
		this->data.push_back(gdc);
	}

	/********************************************************************************************/
	/**
	 * A standard copy constructor
	 */
	GParabolaIndividual(const GParabolaIndividual& cp)
		:GParameterSet(cp)
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * The standard destructor
	 */
	~GParabolaIndividual()
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * A standard assignment operator
	 */
	const GParabolaIndividual& operator=(const GParabolaIndividual& cp){
		GParabolaIndividual::load(&cp);
		return *this;
	}

	/********************************************************************************************/
	/**
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object, camouflaged as a GObject
	 */
	virtual GObject* clone() const {
		return new GParabolaIndividual(*this);
	}

	/********************************************************************************************/
	/**
	 * Loads the data of another GParabolaIndividual, camouflaged as a GObject
	 *
	 * @param cp A copy of another GParabolaIndividual, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp){
		// We have no local data. Hence we can just pass the pointer to our parent class.
		// Note that we'd have to use the GObject::conversion_cast() function otherwise.
		GParameterSet::load(cp);
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GParabolaIndividual object
	 *
	 * @param  cp A constant reference to another GParabolaIndividual object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GParabolaIndividual& cp) const {
		return GParabolaIndividual::isEqualTo(cp, boost::logic::indeterminate);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another GParabolaIndividual object
	 *
	 * @param  cp A constant reference to another GParabolaIndividual object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GParabolaIndividual& cp) const {
		return !GParabolaIndividual::isEqualTo(cp, boost::logic::indeterminate);
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GParabolaIndividual object.  If T is an object type,
	 * then it must implement operator!= .
	 *
	 * @param  cp A constant reference to another GParabolaIndividual object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool isEqualTo(const GObject& cp, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
		// Check that we are indeed dealing with a GBoundedNumT<T> reference
		const GParabolaIndividual *gpi_load = GObject::conversion_cast(&cp,  this);

		// Check equality of the parent class
		if(!GParameterSet::isEqualTo(*gpi_load, expected)) return false;

		return true;
	}

	/*******************************************************************************************/
	/**
	 * Checks for similarity with another GParabolaIndividual object.  As we do not know the
	 * type of T, we need to create a specialization of this function for typeof(T)==double
	 *
	 * @param  cp A constant reference to another GParabolaIndividual object
	 * @param limit A double value specifying the acceptable level of differences of floating point values
	 * @return A boolean indicating whether both objects are similar to each other
	 */
	virtual bool isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
		// Check that we are indeed dealing with a GBoundedNumT<T> reference
		const GParabolaIndividual *gpi_load = GObject::conversion_cast(&cp,  this);

		// Check equality of the parent class
		if(!GParameterSet::isSimilarTo(*gpi_load, limit, expected)) return false;

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
		double result = 0;
		std::vector<double>::const_iterator cit;

		// Compile in DEBUG mode in order to check this conversion
		boost::shared_ptr<GDoubleCollection> gdc_load = pc_at<GDoubleCollection>(0);

		// Great - now we can do the actual calculations. We do this the fancy way ...
		for(cit=gdc_load->begin(); cit!=gdc_load->end(); ++cit) 	result += std::pow(*cit, 2);

		return result;
	}

	/********************************************************************************************/
};

} /* namespace GenEvA */
} /* namespace Gem */

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GParabolaIndividual)

#endif /* GPARABOLAINDIVIDUAL_HPP_ */
