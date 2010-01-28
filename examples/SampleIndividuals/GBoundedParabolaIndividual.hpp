/**
 * @file BoundedGParabolaIndividual.hpp
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
	GBoundedParabolaIndividual()
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * A standard copy constructor
	 */
	GBoundedParabolaIndividual(const GBoundedParabolaIndividual& cp)
		: GParameterSet(cp)
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
		using namespace Gem::Util;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBoundedParabolaIndividual::operator==","cp", CE_SILENT);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another GBoundedParabolaIndividual object
	 *
	 * @param  cp A constant reference to another GBoundedParabolaIndividual object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GBoundedParabolaIndividual& cp) const {
		using namespace Gem::Util;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBoundedParabolaIndividual::operator!=","cp", CE_SILENT);
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
			const Gem::Util::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Util;
	    using namespace Gem::Util::POD;

		// Check that we are indeed dealing with a GBoundedParabolaIndividual reference
		const GBoundedParabolaIndividual *p_load = GObject::conversion_cast(&cp,  this);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GParameterSet::checkRelationshipWith(cp, e, limit, "GBoundedParabolaIndividual", y_name, withMessages));

		// no lolocal data ...

		return evaluateDiscrepancies("GBoundedParabolaIndividual", caller, deviations, e);
	}


protected:
	/********************************************************************************************/
	/**
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object, camouflaged as a GObject
	 */
	virtual GObject* clone_() const {
		return new GBoundedParabolaIndividual(*this);
	}

	/********************************************************************************************/
	/**
	 * The actual fitness calculation takes place here
	 *
	 * @return The value of this object
	 */
	virtual double fitnessCalculation(){
		// If you know the exact structure of the individual, addressing each parameter collection
		// individually might be a good choice. See further below for a different possibility.

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


		// The following allows to extract all GBoundedDoubleCollection objects from this
		// individual. This can be a good choice if more than one collection is stored in
		// the individual, and if there are collections of different types.
		/*
		double result = 0.;

		std::vector<boost::shared_ptr<GBoundedDoubleCollection> > v;
		this->attachViewTo(v);

		std::vector<boost::shared_ptr<GBoundedDoubleCollection> >::iterator v_it;
		for(v_it=v.begin(); v_it!=v.end(); ++v_it) {
			GBoundedDoubleCollection::iterator it;
			for(it=(*v_it)->begin(); it!=(*v_it)->end(); ++it) {
				result += std::pow((*it)->value(), 2);
			}
		}
		return result;
		*/
	}

	/********************************************************************************************/
};

} /* namespace GenEvA */
} /* namespace Gem */

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GBoundedParabolaIndividual)

#endif /* GBOUNDEDPARABOLAINDIVIDUAL_HPP_ */
