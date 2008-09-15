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
#include "GRandom.hpp"
#include "GDoubleCollection.hpp"
#include "GParameterSet.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"

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
	void serialize(Archive & ar, const unsigned int version) {
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
	virtual GObject* clone(){
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
		boost::shared_ptr<GDoubleCollection> gdc_load = parameterbase_cast<GDoubleCollection>(0);

		// Great - now we can do the actual calculations. We do this the fancy way ...
		for(cit=gdc_load->data.begin(); cit!=gdc_load->data.end(); ++cit)
			result += std::pow(*cit, 2);

		return result;
	}

	/********************************************************************************************/
};

} /* namespace GenEvA */
} /* namespace Gem */

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GParabolaIndividual)

#endif /* GPARABOLAINDIVIDUAL_HPP_ */
