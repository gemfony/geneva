/**
 * @file GDelayIndividual.hpp
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
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/thread.hpp>

#ifndef GDELAYINDIVIDUAL_HPP_
#define GDELAYINDIVIDUAL_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

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
 * This individual waits for a predefined amount time before returning the result of the evaluation
 * (which is always the same). Its purpose is to measure the overhead of the parallelization, compared
 * to the serial execution.
 */
class GDelayIndividual: public GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
		using boost::serialization::make_nvp;

		ar & make_nvp("ParameterSet", boost::serialization::base_object<GParameterSet>(*this));
		ar & make_nvp("sleepTime_", sleepTime_);
    }
	///////////////////////////////////////////////////////////////////////

public:
	/********************************************************************************************/
    /**
     * Initialization with the amount of time the fitness evaluation should sleep before continuing
     *
     * @param sleepTime The amount of time the fitness calculation should pause before continuing
     */
    GDelayIndividual(const boost::posix_time::time_duration sleepTime)
		:sleepTime_(sleepTime)
    {/* nothing */}

	/********************************************************************************************/
	/**
	 * A standard copy constructor
	 *
	 * @param cp A copy of another GDelayIndividual
	 */
	GDelayIndividual(const GDelayIndividual& cp)
		:GParameterSet(cp),
		 sleepTime_(cp.sleepTime_)
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * The standard destructor
	 */
	~GDelayIndividual()
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * A standard assignment operator
	 *
	 * @param cp A copy of another GDelayIndividual
	 * @return A constant reference to the function argument
	 */
	const GDelayIndividual& operator=(const GDelayIndividual& cp){
		GDelayIndividual::load(&cp);
		return *this;
	}

	/********************************************************************************************/
	/**
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object, camouflaged as a GObject
	 */
	virtual GObject* clone() const {
		return new GDelayIndividual(*this);
	}

	/********************************************************************************************/
	/**
	 * Loads the data of another GDelayIndividual, camouflaged as a GObject
	 *
	 * @param cp A copy of another GDelayIndividual, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp){
		const GDelayIndividual *gdi_load = conversion_cast(cp, this);

		// Load our parent class'es data ...
		GParameterSet::load(cp);

		// ... and then our own.
		sleepTime_ = gdi_load->sleepTime_;
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GDelayIndividual object
	 *
	 * @param  cp A constant reference to another GDelayIndividual object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GDelayIndividual& cp) const {
		return GDelayIndividual::isEqualTo(cp, boost::logic::indeterminate);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another GDelayIndividual object
	 *
	 * @param  cp A constant reference to another GDelayIndividual object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GDelayIndividual& cp) const {
		return !GDelayIndividual::isEqualTo(cp, boost::logic::indeterminate);
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GDelayIndividual object.
	 *
	 * @param  cp A constant reference to another GDelayIndividual object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool isEqualTo(const GObject& cp, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
		using namespace Gem::Util;

		// Check that we are indeed dealing with a GBoundedNumT<T> reference
		const GDelayIndividual *gdi_load = GObject::conversion_cast(&cp,  this);

		// Check equality of the parent class
		if(!GParameterSet::isEqualTo(*gdi_load, expected)) return false;

		// Check local data
		if(checkForInequality("GDelayIndividual", sleepTime_, gdi_load->sleepTime_,"sleepTime_", "gdi_load->sleepTime_", expected)) return false;

		return true;
	}

	/*******************************************************************************************/
	/**
	 * Checks for similarity with another GDelayIndividual object.
	 *
	 * @param  cp A constant reference to another GDelayIndividual object
	 * @param limit A double value specifying the acceptable level of differences of floating point values
	 * @return A boolean indicating whether both objects are similar to each other
	 */
	virtual bool isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
		using namespace Gem::Util;

		// Check that we are indeed dealing with a GBoundedNumT<T> reference
		const GDelayIndividual *gdi_load = GObject::conversion_cast(&cp,  this);

		// Check equality of the parent class
		if(!GParameterSet::isSimilarTo(*gdi_load, limit, expected)) return false;

		// Check our local data
		if(checkForDissimilarity("GDelayIndividual", sleepTime_, gdi_load->sleepTime_, limit, "sleepTime_", "gdi_load->sleepTime_", expected)) return false;

		return true;
	}

protected:
	/**********************************************************************************/
	/**
	 * The actual mutation operations. We want to avoid spending time on mutations, as
	 * all we want to do is measure the overhead of the parallelization. We thus simply
	 * provide an empty replacement for the default behavior.
	 */
	virtual void customMutations(){ /* nothing */ }


	/********************************************************************************************/
	/**
	 * The actual fitness calculation takes place here.
	 *
	 * @return The value of this object
	 */
	virtual double fitnessCalculation(){
		// Sleep for the desired amount of time
		boost::this_thread::sleep(sleepTime_);

		// We always return the same value
		return 1.;
	}

private:

	/********************************************************************************************/
	/**
	 * The default constructor. Intentionally private -- needed only for (de-)serialization.
	 */
	GDelayIndividual(){ /* nothing */ }

	/********************************************************************************************/

	boost::posix_time::time_duration sleepTime_; ///< The amount of time the evaluation function should sleep before continuing
};


} /* namespace GenEvA */
} /* namespace Gem */

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GDelayIndividual)

#endif /* GDELAYINDIVIDUAL_HPP_ */
