/**
 * @file GParabolaIndividual.hpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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
#include <cstdlib>
#include <sstream>
#include <vector>
#include <list>
#include <algorithm> // for std::sort
#include <utility> // For std::pair

// Includes check for correct Boost version(s)
#include <common/GGlobalDefines.hpp>

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/cast.hpp>

#ifndef GPARABOLAINDIVIDUAL_HPP_
#define GPARABOLAINDIVIDUAL_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include <hap/GRandomT.hpp>
#include <common/GCommonEnums.hpp>
#include <common/GExceptions.hpp>
#include <geneva/GConstrainedDoubleObject.hpp>
#include <geneva/GConstrainedDoubleObjectCollection.hpp>
#include <geneva/GDoubleGaussAdaptor.hpp>
#include <geneva/GObjectExpectationChecksT.hpp>
#include <geneva/GParameterObjectCollection.hpp>
#include <geneva/GParameterSet.hpp>

namespace Gem
{
namespace Geneva
{
/************************************************************************************************/
/**
 * This individual searches for the minimum of a parabola of a given dimension,
 * It is part of an introductory example, used in the Geneva manual, but can also
 * be used as a starting point for your own projects-
 */
class GParabolaIndividual :public GParameterSet
{
public:
	/********************************************************************************************/
	/**
	 * The default constructor. Intentionally private and empty, as it is only needed for
	 * serialization purposes.
	 */
	GParabolaIndividual() :GParameterSet()
	{	/* nothing */ }

	/********************************************************************************************/
	/**
	 * A simple constructor that initializes this object with a collection of bounded
	 * double variables.
	 *
	 * @param dim The amount of variables
	 * @param min The lower boundary of the variables
	 * @param max The upper boundary of the variables
	 */
	GParabolaIndividual(
			const std::size_t& dim
			, const double& min
			, const double& max
	)
		: GParameterSet()
	{
		// Set up a GConstrainedDoubleObjectCollection
		// boost::shared_ptr<GConstrainedDoubleObjectCollection> gbdc_ptr(new GConstrainedDoubleObjectCollection());
		// boost::shared_ptr<GParameterObjectCollection> gpoc_ptr(new GParameterObjectCollection());

		// Add bounded double objects
		for(std::size_t i=0; i<dim; i++) {
			// GConstrainedDoubleObject will start with random values in the range [min:max]
			boost::shared_ptr<GConstrainedDoubleObject> gbd_ptr(new GConstrainedDoubleObject(gr.uniform_real(min, max), min, max) );

			// Create a suitable adaptor (sigma=0.1, sigma-adaption=0.5, min sigma=0, max sigma=0,5)
			boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.1, 0.5, 0., 0.5));
			gdga_ptr->setAdaptionThreshold(1); // Adaption parameters are modified after each adaption
			gdga_ptr->setAdaptionProbability(0.05); // The likelihood for a parameter to be adapted

			// Register the adaptor with GConstrainedDoubleObject objects
			gbd_ptr->addAdaptor(gdga_ptr);

			// Add a GConstrainedDoubleObject object to the collection
			// gbdc_ptr->push_back(gbd_ptr);
			// gpoc_ptr->push_back(gbd_ptr);
			this->push_back(gbd_ptr);
		}

		// Add the collection to this object
		// this->push_back(gbdc_ptr);
		// this->push_back(gpoc_ptr);
	  }

	/********************************************************************************************/
	/**
	 * A standard copy constructor
	 *
	 * @param cp A copy of another GParabolaIndividual
	 */
	GParabolaIndividual(const GParabolaIndividual& cp)
		: GParameterSet(cp)
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GParabolaIndividual()
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * A standard assignment operator
	 *
	 * @param cp A copy of another GParabolaIndividual object
	 * @return A constant reference to this object
	 */
	const GParabolaIndividual& operator=(const GParabolaIndividual& cp){
		GParabolaIndividual::load_(&cp);
		return *this;
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GParabolaIndividual object.
	 *
	 * NOTE: THIS FUNCTION IS OPTIONAL AND IS MAINLY USED IN CONJUNCTION WITH UNIT TESTS.
	 * You do not need it if you do not intend to perform unit tests.
	 *
	 * @param  cp A constant reference to another GParabolaIndividual object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GParabolaIndividual& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GParabolaIndividual::operator==","cp", CE_SILENT);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another GParabolaIndividual object.
	 *
	 * NOTE: THIS FUNCTION IS OPTIONAL AND IS MAINLY USED IN CONJUNCTION WITH UNIT TESTS.
	 * You do not need it if you do not intend to perform unit tests.
	 *
	 * @param  cp A constant reference to another GParabolaIndividual object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GParabolaIndividual& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GParabolaIndividual::operator!=","cp", CE_SILENT);
	}

protected:
	/********************************************************************************************/
	/**
	 * Loads the data of another GParabolaIndividual, camouflaged as a GObject.
	 *
	 * @param cp A copy of another GParabolaIndividual, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp)
	{
		const GParabolaIndividual *p_load = GObject::conversion_cast<GParabolaIndividual>(cp);

		// Load our parent's data
		GParameterSet::load_(cp);

		// Load local data here like this:
		dim_ = p_load->dim_;
	}

	/********************************************************************************************/
	/**
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object, camouflaged as a GObject
	 */
	virtual GObject* clone_() const {
		return new GParabolaIndividual(*this);
	}

	/********************************************************************************************/
	/**
	 * The actual fitness calculation takes place here.
	 *
	 * @return The value of this object
	 */
	virtual double fitnessCalculation(){
		double result = 0.;

		// Extract the GConstrainedDoubleObjectCollection object. In a realistic scenario, you might want
		// to add error checks here upon first invocation.
		/*
		boost::shared_ptr<GConstrainedDoubleObjectCollection> vC = at<GConstrainedDoubleObjectCollection>(0);

		// Calculate the value of the parabola
		for(std::size_t i=0; i<vC->size(); i++)
			result += vC->at(i)->value() * vC->at(i)->value();
		 */

		// boost::shared_ptr<GParameterObjectCollection> vC = at<GParameterObjectCollection>(0);
		// GParameterObjectCollection::conversion_iterator<GConstrainedDoubleObject> it(vC->end());
		GParabolaIndividual::conversion_iterator<GConstrainedDoubleObject> it(this->end());
		// for(it=vC->begin(); it!=vC->end(); ++it) {
		for(it=this->begin(); it!=this->end(); ++it) {
			result += (*it)->value() * (*it)->value();
		}

		return result;
	}

private:
	/********************************************************************************************/
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
		   & BOOST_SERIALIZATION_NVP(dim_);
	}

	/********************************************************************************************/
	std::size_t dim_; ///< The dimension of the parabola
};

/*************************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/*************************************************************************************************/

#endif /* GPARABOLAINDIVIDUAL_HPP_ */
