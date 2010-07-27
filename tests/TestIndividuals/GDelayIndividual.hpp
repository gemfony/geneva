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

// Geneva header files go here
#include "optimization/GDoubleCollection.hpp"
#include "optimization/GParameterSet.hpp"
#include "optimization/GDoubleGaussAdaptor.hpp"

namespace Gem
{
namespace Geneva
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

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
		   & BOOST_SERIALIZATION_NVP(sleepTime_);
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
		: sleepTime_(sleepTime)
    {/* nothing */}

	/********************************************************************************************/
	/**
	 * A standard copy constructor
	 *
	 * @param cp A copy of another GDelayIndividual
	 */
	GDelayIndividual(const GDelayIndividual& cp)
		: GParameterSet(cp)
		, sleepTime_(cp.sleepTime_)
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
		GDelayIndividual::load_(&cp);
		return *this;
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GDelayIndividual object
	 *
	 * @param  cp A constant reference to another GDelayIndividual object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GDelayIndividual& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GDelayIndividual::operator==","cp", CE_SILENT);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another GDelayIndividual object
	 *
	 * @param  cp A constant reference to another GDelayIndividual object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GDelayIndividual& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GDelayIndividual::operator!=","cp", CE_SILENT);
	}

	/********************************************************************************************/
	/**
	 * Checks whether a given expectation for the relationship between this object and another object
	 * is fulfilled.
	 *
	 * @param cp A constant reference to another object, camouflaged as a GObject
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 * @param caller An identifier for the calling entity
	 * @param y_name An identifier for the object that should be compared to this one
	 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
	 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
	 */
	boost::optional<std::string> checkRelationshipWith(const GObject& cp,
			const Gem::Common::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Common;

		// Check that we are indeed dealing with a GParamterBase reference
		const GDelayIndividual *p_load = GObject::conversion_cast<GDelayIndividual>(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GParameterSet::checkRelationshipWith(cp, e, limit, "GDelayIndividual", y_name, withMessages));

		// ... and then our local data
		deviations.push_back(checkExpectation(withMessages, "GDelayIndividual", sleepTime_, p_load->sleepTime_, "sleepTime_", "p_load->sleepTime_", e , limit));

		return evaluateDiscrepancies("GDelayIndividual", caller, deviations, e);
	}


protected:
	/********************************************************************************************/
	/**
	 * Loads the data of another GDelayIndividual, camouflaged as a GObject
	 *
	 * @param cp A copy of another GDelayIndividual, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp){
		const GDelayIndividual *p_load = conversion_cast<GDelayIndividual>(cp);

		// Load our parent class'es data ...
		GParameterSet::load_(cp);

		// ... and then our own.
		sleepTime_ = p_load->sleepTime_;
	}

	/********************************************************************************************/
	/**
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object, camouflaged as a GObject
	 */
	virtual GObject* clone_() const {
		return new GDelayIndividual(*this);
	}

	/**********************************************************************************/
	/**
	 * The actual adaption operations. We want to avoid spending time on adaptions, as
	 * all we want to do is measure the overhead of the parallelization. We thus simply
	 * provide an empty replacement for the default behavior.
	 */
	virtual void customAdaptions(){ /* nothing */ }


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


} /* namespace Geneva */
} /* namespace Gem */

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GDelayIndividual)

#endif /* GDELAYINDIVIDUAL_HPP_ */
