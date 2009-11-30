/**
 * @file CLASSNAME.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich  and Karlsruhe Institute of Technology
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

#ifndef UPPERCASECLASSNAME_HPP_
#define UPPERCASECLASSNAME_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA header files go here
#include "GDoubleCollection.hpp"
#include "GParameterSet.hpp"
#include "GDoubleGaussAdaptor.hpp"
/* Add any other required GenEvA includes */

namespace Gem
{
namespace GenEvA
{

/************************************************************************************************/
/**
 * Add a description of this particular individual here
 */
class CLASSNAME
	:public GParameterSet
{
	//-----------------------------------------------------------------------------------
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & make_nvp("ParameterSet",
				boost::serialization::base_object<GParameterSet>(*this));
		// add all local variables here, if you want them to be serialized. E.g.:
		// ar & make_nvp("myLocalDoubleVar_",myLocalDoubleVar_);
		// ar & make_nvp("myLocalIntVar_",myLocalIntVar_);
		// This also works with objects, if they have a corresponding serialize() function.
		// Serialization can also be split in "load()" and "save()" functions, if you
		// want any specific behavior. See e.g. GDataExchange.hpp for an example.
	}
	//-----------------------------------------------------------------------------------

public:
	/********************************************************************************************/
	/**
	 * The default constructor.
	 */
	CLASSNAME()
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * A standard copy constructor
	 */
	CLASSNAME(const CLASSNAME& cp)
		:GParameterSet(cp)
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * The destructor
	 */
	~CLASSNAME()
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * A standard assignment operator
	 */
	const CLASSNAME& operator=(const CLASSNAME& cp){
		CLASSNAME::load(&cp);
		return *this;
	}

	/********************************************************************************************/
	/**
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object, camouflaged as a GObject
	 */
	virtual GObject* clone() const {
		return new CLASSNAME(*this);
	}

	/********************************************************************************************/
	/**
	 * Loads the data of another CLASSNAME, camouflaged as a GObject. Note that you can
	 * omit the cast, if there is no local data.
	 *
	 * @param cp A copy of another CLASSNAME, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp){
		// This conversion will throw in DEBUG mode, if the object was assigned to itself by mistake
		const CLASSNAME *cn_load = conversion_cast(cp, this);

		// Load the data of the parent class
		GParameterSet::load(cp);

		// Load local data here
		// myLocalDoubleVar_ = cn_load->myLocalDoubleVar_;
		// myLocalIntVar_ = cn_load->myLocalIntVar_;
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another CLASSNAME object
	 *
	 * @param  cp A constant reference to another CLASSNAME object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const CLASSNAME& cp) const {
		return CLASSNAME::isEqualTo(cp);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another CLASSNAME object
	 *
	 * @param  cp A constant reference to another CLASSNAME object
	 * @return A boolean indicating whether both objects differ
	 */
	bool operator!=(const CLASSNAME& cp) const {
		return !CLASSNAME::isEqualTo(cp);
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another CLASSNAME object.  This is useful for testing.
	 *
	 * @param  cp A constant reference to another CLASSNAME object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool isEqualTo(const GObject& cp) const {
		// Convert the GObject reference, also checks for self-assignment
		const CLASSNAME *cn_load = GObject::conversion_cast(&cp,  this);

		// Check equality of the parent class
		if(!GParameterSet::isEqualTo(*cn_load)) return false;

		// Check for equality of local data
		//  if(myLocalDoubleVar_ != cn_load->myLocalDoubleVar_) return false;
		//  if(myLocalIntVar_ != cn_load->myLocalIntVar_) return false;

		return true;
	}

	/*******************************************************************************************/
	/**
	 * Checks for similarity with another CLASSNAME object. This is useful for testing.
	 * Checks for similarity do not differ from checks for equality, with the exception of
	 * floating point values.
	 *
	 * @param  cp A constant reference to another CLASSNAME object
	 * @param limit A double value specifying the acceptable level of differences of two double values
	 * @return A boolean indicating whether both objects are similar to each other
	 */
	virtual bool isSimilarTo(const GObject& cp, const double& limit=0) const {
		// Convert the GObject reference, also checks for self-assignment
		const CLASSNAME *cn_load = GObject::conversion_cast(&cp,  this);

		// Check equality of the parent class
		if(!GParameterSet::isSimilarTo(*cn, limit)) return false;

		// Check for similarity of local data
		//  if(myLocalDoubleVar_ != cn_load->myLocalDoubleVar_) return false;
		//  if(myLocalIntVar_ != cn_load->myLocalIntVar_) return false;

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

private:
	// double myLocalDoubleVar_; ///< An example of a local variable
	// double myLocalIntVar_; ///< Another example of a local variable
};

}  /* namespace GenEvA */
}  /* namespace Gem */

#endif /* UPPERCASECLASSNAME_HPP_ */
