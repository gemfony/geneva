/**
 * @file GFunctionIndividual.hpp
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

#ifndef GFUNCTIONINDIVIDUAL_HPP_
#define GFUNCTIONINDIVIDUAL_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA header files go here
#include "GDoubleCollection.hpp"
#include "GParameterSet.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GFunctionIndividualDefines.hpp"

namespace Gem
{
namespace GenEvA
{

/************************************************************************************************/
/**
 * This individual searches for a minimum of a number of predefined functions, each capable
 * of processing their input in multiple dimensions. Currently implemented are:
 * - A simple parabola
 * - A "noisy" parabola, featuring an immense number of local optima
 * - The generalized Rosenbrock function
 *
 * Note that the free variables of this example are not equipped with boundaries.
 * See the GBoundedParabola example for ways of specifying boundaries for variables.
 * This class is purely meant for demonstration purposes and in order to check the
 * performance of the Geneva library.
 */
template <demoFunction dF=PARABOLA>
class GFunctionIndividual: public GParameterSet
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
	GFunctionIndividual()
		: demoFunction_(dF)
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * A standard copy constructor
	 *
	 * @param cp A copy of another GFunctionIndidivual
	 */
	GFunctionIndividual(const GFunctionIndividual<dF>& cp)
		: GParameterSet(cp)
		, demoFunction_(dF)
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * The standard destructor
	 */
	~GFunctionIndividual()
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * A standard assignment operator
	 *
 	 * @param cp A copy of another GFunctionIndividual
	 */
	const GFunctionIndividual<dF>& operator=(const GFunctionIndividual<dF>& cp){
		GFunctionIndividual::load(&cp);
		return *this;
	}

	/********************************************************************************************/
	/**
	 * Loads the data of another GFunctionIndividual, camouflaged as a GObject
	 *
	 * @param cp A copy of another GFunctionIndividual, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp){
		const GFunctionIndividual<dF> *p_load = conversion_cast(cp, this);

		// Load our parent class'es data ...
		GParameterSet::load(cp);

		// ... no local data
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GFunctionIndividual object
	 *
	 * @param  cp A constant reference to another GFunctionIndividual object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GFunctionIndividual<dF>& cp) const {
		using namespace Gem::Util;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GFunctionIndividual<dF>::operator==","cp", CE_SILENT);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another GFunctionIndividual object
	 *
	 * @param  cp A constant reference to another GFunctionIndividual object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GFunctionIndividual<dF>& cp) const {
		using namespace Gem::Util;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GFunctionIndividual<dF>::operator!=","cp", CE_SILENT);
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

		// Check that we are indeed dealing with a GParamterBase reference
		const GFunctionIndividual<dF> *p_load = GObject::conversion_cast(&cp,  this);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GParameterSet::checkRelationshipWith(cp, e, limit, "GFunctionIndividual<dF>", y_name, withMessages));

		// ... no local data

		return evaluateDiscrepancies("GFunctionIndividual<dF>", caller, deviations, e);
	}

	/*******************************************************************************************/
	/**
	 * Allows to retrieve the demo function
	 *
	 * @return The current value of the demoFunction_ variable
	 */
	demoFunction getDemoFunction() const {
		return demoFunction_;
	}

protected:
	/********************************************************************************************/
	/**
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object, camouflaged as a GObject
	 */
	virtual GObject* clone_() const {
		return new GFunctionIndividual<dF>(*this);
	}

	/********************************************************************************************/
	/**
	 * This is a trap to capture invalid demo Function assignments.
	 *
	 * @return The value of this object
	 */
	virtual double fitnessCalculation(){
		std::ostringstream error;
		error << "In GFunctionIndividual<dF>::fitnessCalculation(): Error" << std::endl
  			  << "Class seems to have been instantiated with an invalid demo function" << std::endl;
		throw(Gem::GenEvA::geneva_error_condition(error.str()));
		return 0.; // Make the compiler happy
	}

	/********************************************************************************************/
private:
	const demoFunction demoFunction_; ///< Specifies which demo function is being used
};

/************************************************************************************************/
// A number of specializations of fitnessCalculation() for the three function types

/********************************************************************************************/
/**
 * A simple, multi-dimensional parabola
 *
 * @return The result of the calculation
 */
template<> double GFunctionIndividual<PARABOLA>::fitnessCalculation() {
	// Extract the GDoubleCollection object
	boost::shared_ptr<GDoubleCollection> x = pc_at<GDoubleCollection>(0);
	const GDoubleCollection& x_ref = *x; // Avoid frequent dereferencing

	std::size_t parameterSize = x_ref.size();
	double result = 0.;
	for(std::size_t i=0; i<parameterSize; i++) result += GSQUARED(x_ref[i]);
	return result;
}

/********************************************************************************************/
/**
 * A "noisy" parabola, i.e. a parabola with a very large number of local optima
 *
 * @return The result of the calculation
 */
template<> double GFunctionIndividual<NOISYPARABOLA>::fitnessCalculation() {
	// Extract the GDoubleCollection object
	boost::shared_ptr<GDoubleCollection> x = pc_at<GDoubleCollection>(0);
	const GDoubleCollection& x_ref = *x; // Avoid frequent dereferencing

	std::size_t parameterSize = x_ref.size();
	double xsquared = 0.;
	for(std::size_t i=0; i<parameterSize; i++){
		xsquared += GSQUARED(x_ref[i]);
	}
	return (cos(xsquared) + 2.) * xsquared;
}

/********************************************************************************************/
/**
 * The generalized Rosenbrock function (see e.g. http://en.wikipedia.org/wiki/Rosenbrock_function)
 *
 * @param x The input parameters for the function
 * @return The result of the calculation
 */
template<> double GFunctionIndividual<ROSENBROCK>::fitnessCalculation() {
	// Extract the GDoubleCollection object
	boost::shared_ptr<GDoubleCollection> x = pc_at<GDoubleCollection>(0);
	const GDoubleCollection& x_ref = *x; // Avoid frequent dereferencing

	std::size_t parameterSize = x_ref.size();
	double result = 0.;

#ifdef DEBUG
	// Check the size of the parameter vector -- must be at least 2
	if(parameterSize < 2) {
		std::ostringstream error;
		error << "In GFunctionIndividual<dF>::rosenbrock(): Error!" << std::endl
			  << "Need to use at least two input dimensions, but got " << parameterSize << std::endl;
		throw(Gem::GenEvA::geneva_error_condition(error.str()));
	}
#endif /* DEBUG */

	for(std::size_t i=0; i<(parameterSize-1); i++) {
		result += GSQUARED(1.-x_ref[i]) + 100.*GSQUARED(x_ref[i+1]-GSQUARED(x_ref[i]));
	}

	return result;
}

/************************************************************************************************/
} /* namespace GenEvA */
} /* namespace Gem */

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(Gem::GenEvA::GFunctionIndividual<Gem::GenEvA::PARABOLA>, "GFunctionIndividual_PARABOLA")
BOOST_CLASS_EXPORT_GUID(Gem::GenEvA::GFunctionIndividual<Gem::GenEvA::NOISYPARABOLA>, "GFunctionIndividual_NOISYPARABOLA")
BOOST_CLASS_EXPORT_GUID(Gem::GenEvA::GFunctionIndividual<Gem::GenEvA::ROSENBROCK>, "GFunctionIndividual_ROSENBROCK")

#endif /* GFUNCTIONINDIVIDUAL_HPP_ */
