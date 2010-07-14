/**
 * @file GTestIndividual1.hpp
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

// GenEvA header files go here
#include "GParameterSet.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GBoundedDoubleCollection.hpp"
#include "GBoundedDouble.hpp"
#include "GExceptions.hpp"
#include "GGlobalOptionsT.hpp"
#include "GCommonEnums.hpp"

namespace Gem
{
namespace GenEvA
{
/************************************************************************************************/
/**
 * This individual serves as the basis for unit tests of the individual hierarchy. At the time
 * of writing, it was included in order to be able to set the individual's personality without
 * weakening data protection.
 */
class GTestIndividual1 :public GParameterSet
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

	/********************************************************************************************/
	/**
	 * Allows to set the individual's personality. Note that this is not a normal feature of
	 * individuals but has been added in this particular individual in order to allow unit tests.
	 */
	void setPersonalityType(const Gem::GenEvA::personality& pers) {
		this->setPersonality(pers);
	}

	/********************************************************************************************/
	/**
	 * Allows to retrieve the individual's personality. Note that this is not a normal feature of
	 * individuals but has been added in this particular individual in order to allow unit tests.
	 */
	personality getPersonalityType() const {
		return this->getPersonality();
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

		// Extract the first GDoubleCollection object. In a realistic scenario, you might want
		// to add error checks here upon first invocation.
		boost::shared_ptr<GDoubleCollection> vC = pc_at<GDoubleCollection>(0);

		// Calculate the value of the parabola
		for(std::size_t i=0; i<vC->size(); i++) {
			result += vC->at(i) * vC->at(i);
		}

		return result;
	}
};

} /* namespace GenEvA */
} /* namespace Gem */

// Needed for serialization purposes
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GTestIndividual1)

#endif /* GTESTINDIVIDUAL1_HPP_ */
