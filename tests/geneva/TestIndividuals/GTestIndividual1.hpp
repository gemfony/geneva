/**
 * @file GTestIndividual1.hpp
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

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/cast.hpp>

#ifndef GTESTINDIVIDUAL1_HPP_
#define GTESTINDIVIDUAL1_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include "geneva/GParameterSet.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/GConstrainedDouble.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "common/GExceptions.hpp"
#include "common/GGlobalOptionsT.hpp"
#include "common/GCommonEnums.hpp"

namespace Gem
{
namespace Tests
{
/************************************************************************************************/
/**
 * This individual serves as the basis for unit tests of the individual hierarchy. At the time
 * of writing, it was included in order to be able to set the individual's personality without
 * weakening data protection.
 */
class GTestIndividual1 :public Gem::Geneva::GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/********************************************************************************************/
	/**
	 * The default constructor.
	 */
	GTestIndividual1()
		: GParameterSet()
	{
		// Fill with some data
		boost::shared_ptr<Gem::Geneva::GDoubleCollection > gdc_ptr(new Gem::Geneva::GDoubleCollection(100, -10., 10.));
		boost::shared_ptr<Gem::Geneva::GDoubleGaussAdaptor> gdga1(new Gem::Geneva::GDoubleGaussAdaptor(1.,0.6,0.,2.));
		gdc_ptr->addAdaptor(gdga1);
		this->push_back(gdc_ptr);
	}

	/********************************************************************************************/
	/**
	 * The copy constructor
	 *
	 * @param cp A constant reference to another GTestIndividual1 object
	 */
	GTestIndividual1(const GTestIndividual1& cp)
		: GParameterSet(cp)
	{	/* nothing */ }

	/********************************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GTestIndividual1()
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * A standard assignment operator
	 *
	 * @param cp A copy of another GTestIndividual1 object
	 * @return A constant reference to this object
	 */
	const GTestIndividual1& operator=(const GTestIndividual1& cp){
		GTestIndividual1::load_(&cp);
		return *this;
	}

protected:
	/********************************************************************************************/
	/**
	 * Loads the data of another GTestIndividual1, camouflaged as a GObject.
	 *
	 * @param cp A copy of another GTestIndividual1, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp)
	{
		// Check that we are not accidently assigning this object to itself
		GObject::selfAssignmentCheck<GTestIndividual1>(cp);

		// Load our parent's data
		GParameterSet::load_(cp);

		// Load local data here like this:
		// myVar = p_load->myVar;
	}

	/********************************************************************************************/
	/**
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object, camouflaged as a GObject
	 */
	virtual GObject* clone_() const {
		return new GTestIndividual1(*this);
	}

	/********************************************************************************************/
	/**
	 * The actual fitness calculation takes place here.
	 *
	 * @return The value of this object
	 */
	virtual double fitnessCalculation(){
		double result = 0.;

		// Extract the first Gem::Geneva::GDoubleCollection object. In a realistic scenario, you might want
		// to add error checks here upon first invocation.
		boost::shared_ptr<Gem::Geneva::GDoubleCollection> vC = pc_at<Gem::Geneva::GDoubleCollection>(0);

		// Calculate the value of the parabola
		for(std::size_t i=0; i<vC->size(); i++) {
			result += vC->at(i) * vC->at(i);
		}

		return result;
	}
};

} /* namespace Tests */
} /* namespace Gem */

// Needed for serialization purposes
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Tests::GTestIndividual1)

#endif /* GTESTINDIVIDUAL1_HPP_ */
