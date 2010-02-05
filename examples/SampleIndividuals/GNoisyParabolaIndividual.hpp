/**
 * @file GNoisyParabolaIndividual.hpp
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
#include <iostream>
#include <cmath>
#include <sstream>
#include <vector>

// Boost header files go here
#include <boost/shared_ptr.hpp>

#ifndef GNOISYPARABOLAINDIVIDUAL_HPP_
#define GNOISYPARABOLAINDIVIDUAL_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA header files go here
#include "GRandom.hpp"
#include "GDoubleCollection.hpp"
#include "GParameterSet.hpp"
#include "GDoubleGaussAdaptor.hpp"

namespace Gem
{
namespace GenEvA
{

/************************************************************************************************/
/**
 * This individual searches for the minimum of a "noisy parabola" in n dimensions. It is meant
 * as an example of how to set up custom individuals. The individual is very similar to the
 * GParabolaIndividual example, however, the function to be minimized has a very large number
 * of local optima, making optimization much more difficult.
 */
class GNoisyParabolaIndividual
	:public GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet);

		// add all local variables here, if you want them to be serialized. E.g.:
		// ar & make_nvp("myLocalVar_",myLocalVar_);
		// or
		// ar & BOOST_SERIALIZATION_NVP(myLocalVar);
		// This also works with objects, if they have a corresponding serialize() function.
		// The first function can be necessary when dealing with templates
	}
	///////////////////////////////////////////////////////////////////////

public:
	/********************************************************************************************/
	/**
	 * The default constructor.
	 */
	GNoisyParabolaIndividual()
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * A constructor which initializes the individual with a suitable set of random double values.
	 *
	 * @param sz The desired size of the double collection
	 * @param min The minimum value of the random numbers to fill the collection
	 * @param max The maximum value of the random numbers to fill the collection
	 * @param adaptionThreshold The number of calls to GDoubleGaussAdaptor::mutate after which mutation should be adapted
	 */
	GNoisyParabolaIndividual(std::size_t sz, double min, double max, boost::uint32_t adaptionThreshold){
		// Set up a GDoubleCollection with sz values, each initialized
		// with a random number in the range [min,max[
		boost::shared_ptr<GDoubleCollection> gdc(new GDoubleCollection(sz,min,max));

		// Set up and register an adaptor for the collection, so it
		// knows how to be mutated. We want to start with a large sigma ("sweep"), a
		// sigma-adaption of 0.001, a minimum sigma of 0.000001 and a maximum sigma of 5.
		boost::shared_ptr<GDoubleGaussAdaptor> gdga(new GDoubleGaussAdaptor(5.,0.001,0.000001,5));
		gdga->setAdaptionThreshold(adaptionThreshold);

		gdc->addAdaptor(gdga);

		// Make the parameter collection known to this individual
		this->data.push_back(gdc);
	}

	/********************************************************************************************/
	/**
	 * A standard copy constructor
	 */
	GNoisyParabolaIndividual(const GNoisyParabolaIndividual& cp)
		:GParameterSet(cp)
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * The standard destructor
	 */
	~GNoisyParabolaIndividual()
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * A standard assignment operator
	 */
	const GNoisyParabolaIndividual& operator=(const GNoisyParabolaIndividual& cp){
		GNoisyParabolaIndividual::load_(&cp);
		return *this;
	}

protected:
	/********************************************************************************************/
	/**
	 * Loads the data of another GNoisyParabolaIndividual, camouflaged as a GObject
	 *
	 * @param cp A copy of another GNoisyParabolaIndividual, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp){
		// Check that we are not accidently assigning this object to itself
		GObject::selfAssignmentCheck<GNoisyParabolaIndividual>(cp);

		// We have no local data. Hence we can just pass the pointer to our parent class.
		// Note that we'd have to use the GObject::conversion_cast() function otherwise.
		GParameterSet::load_(cp);
	}


	/********************************************************************************************/
	/**
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object, camouflaged as a GObject
	 */
	virtual GObject* clone_() const {
		return new GNoisyParabolaIndividual(*this);
	}

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
		for(cit=gdc_load->begin(); cit!=gdc_load->end(); ++cit){
			double xsquared = *cit * *cit;
			result += (cos(xsquared) + 2)*xsquared;
		}

		return result;
	}

	/********************************************************************************************/
};

} /* namespace GenEvA */
} /* namespace Gem */

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GNoisyParabolaIndividual)

#endif /* GNOISYPARABOLAINDIVIDUAL_HPP_ */
