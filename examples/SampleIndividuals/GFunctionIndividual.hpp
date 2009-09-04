/**
 * @file GFunctionIndividual.hpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
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
class GFunctionIndividual: public GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
		using boost::serialization::make_nvp;

		ar & make_nvp("ParameterSet", boost::serialization::base_object<GParameterSet>(*this));
		ar & make_nvp("demoFunction_", demoFunction_);

		// add all local variables here, if you want them to be serialized. E.g.:
		// ar & make_nvp("myLocalVar_",myLocalVar_);
		// This also works with objects, if they have a corresponding function.
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
		using boost::serialization::make_nvp;

		ar & make_nvp("ParameterSet", boost::serialization::base_object<GParameterSet>(*this));

		demoFunction tmpDemoFunction;

		ar & make_nvp("demoFunction_", tmpDemoFunction);
		this->setDemoFunction(tmpDemoFunction);

		// add other local variables here, if you want them to be de-serialized. E.g.:
		// ar & make_nvp("myLocalVar_",myLocalVar_);
		// This also works with objects, if they have a corresponding function.
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

	///////////////////////////////////////////////////////////////////////

public:
	/********************************************************************************************/
	/**
	 * The default constructor.
	 */
	GFunctionIndividual():
		demoFunction_(PARABOLA),
		eval_(&GFunctionIndividual::parabola)
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * A standard copy constructor
	 */
	GFunctionIndividual(const GFunctionIndividual& cp)
		:GParameterSet(cp),
		 demoFunction_(cp.demoFunction_)
	{
		// Register a suitable transfer function, depending on the value of transferMode_
		switch(demoFunction_){
		case PARABOLA:
			eval_ = &GFunctionIndividual::parabola;
			break;
		case NOISYPARABOLA:
			eval_ = &GFunctionIndividual::noisyParabola;
			break;
		case ROSENBROCK:
			eval_ = &GFunctionIndividual::rosenbrock;
			break;
		}
	}

	/********************************************************************************************/
	/**
	 * The standard destructor
	 */
	~GFunctionIndividual()
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * A standard assignment operator
	 */
	const GFunctionIndividual& operator=(const GFunctionIndividual& cp){
		GFunctionIndividual::load(&cp);
		return *this;
	}

	/********************************************************************************************/
	/**
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object, camouflaged as a GObject
	 */
	virtual GObject* clone() const {
		return new GFunctionIndividual(*this);
	}

	/********************************************************************************************/
	/**
	 * Loads the data of another GFunctionIndividual, camouflaged as a GObject
	 *
	 * @param cp A copy of another GFunctionIndividual, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp){
		const GFunctionIndividual *gfi_load = conversion_cast(cp, this);

		// We have no local data. Hence we can just pass the pointer to our parent class.
		// Note that we'd have to use the GObject::conversion_cast() function otherwise.
		GParameterSet::load(cp);

		this->setDemoFunction(gfi_load->demoFunction_);
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GFunctionIndividual object
	 *
	 * @param  cp A constant reference to another GFunctionIndividual object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GFunctionIndividual& cp) const {
		return GFunctionIndividual::isEqualTo(cp, boost::logic::indeterminate);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another GFunctionIndividual object
	 *
	 * @param  cp A constant reference to another GFunctionIndividual object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GFunctionIndividual& cp) const {
		return !GFunctionIndividual::isEqualTo(cp, boost::logic::indeterminate);
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GFunctionIndividual object.  If T is an object type,
	 * then it must implement operator!= .
	 *
	 * @param  cp A constant reference to another GFunctionIndividual object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool isEqualTo(const GObject& cp, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
		using namespace Gem::Util;

		// Check that we are indeed dealing with a GBoundedNumT<T> reference
		const GFunctionIndividual *gpi_load = GObject::conversion_cast(&cp,  this);

		// Check equality of the parent class
		if(!GParameterSet::isEqualTo(*gpi_load, expected)) return false;

		// Check local data
		if(checkForInequality("GFunctionIndividual", demoFunction_, gpi_load->demoFunction_,"demoFunction_", "gpi_load->demoFunction_", expected)) return false;

		return true;
	}

	/*******************************************************************************************/
	/**
	 * Checks for similarity with another GFunctionIndividual object.  As we do not know the
	 * type of T, we need to create a specialization of this function for typeof(T)==double
	 *
	 * @param  cp A constant reference to another GFunctionIndividual object
	 * @param limit A double value specifying the acceptable level of differences of floating point values
	 * @return A boolean indicating whether both objects are similar to each other
	 */
	virtual bool isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
		using namespace Gem::Util;

		// Check that we are indeed dealing with a GBoundedNumT<T> reference
		const GFunctionIndividual *gpi_load = GObject::conversion_cast(&cp,  this);

		// Check equality of the parent class
		if(!GParameterSet::isSimilarTo(*gpi_load, limit, expected)) return false;

		// Check our local data
		if(checkForDissimilarity("GFunctionIndividual", demoFunction_, gpi_load->demoFunction_, limit, "demoFunction_", "gpi_load->demoFunction_", expected)) return false;

		return true;
	}

	/*******************************************************************************************/
	/**
	 * Specifies that a given function should be used for the evaluation step
	 *
	 * @param df The id of the demo function
	 */
	void setDemoFunction(const demoFunction& df) {
		if(df == demoFunction_) return; // nothing to do

		// Register a suitable transfer function, depending on the value of transferMode_
		switch(demoFunction_){
		case PARABOLA:
			eval_ = &GFunctionIndividual::parabola;
			break;
		case NOISYPARABOLA:
			eval_ = &GFunctionIndividual::noisyParabola;
			break;
		case ROSENBROCK:
			eval_ = &GFunctionIndividual::rosenbrock;
			break;
		}

		demoFunction_ = df;
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
	 * The actual fitness calculation takes place here.
	 *
	 * @return The value of this object
	 */
	virtual double fitnessCalculation(){
		// Check whether an evaluation function is available
		if(!eval_) {
			std::ostringstream error;
			error << "In GFunctionIndividual::fitnessCalculation(): Error!" << std::endl
				  << "No evaluation function is available." << std::endl;
			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}

		// Extract the GDoubleCollection object
		boost::shared_ptr<GDoubleCollection> x = pc_at<GDoubleCollection>(0);

		// Do the actual calculation
		return eval_(x);
	}

	/********************************************************************************************/
private:
	/**
	 * This function object gives access to the actual evaluation function
	 */
	boost::function<double(boost::shared_ptr<GDoubleCollection>)> eval_;

	demoFunction demoFunction_; ///< Specifies which demo function should be used

	/********************************************************************************************/
	/**
	 * A simple, multi-dimensional parabola
	 *
	 * @param x The input parameters for the function
	 * @return The result of the calculation
	 */
	static double parabola(boost::shared_ptr<GDoubleCollection> x) {
		double result = 0.;
		GDoubleCollection::const_iterator cit;

		for(cit=x->begin(); cit!=x->end(); ++cit) {
			result += std::pow(*cit,2);
		}

		return result;
	}

	/********************************************************************************************/
	/**
	 * A "noisy" parabola, i.e. a parabola with a very large number of local optima
	 *
	 * @param x The input parameters for the function
	 * @return The result of the calculation
	 */
	static double noisyParabola(boost::shared_ptr<GDoubleCollection> x) {
		double result = 0.;
		GDoubleCollection::const_iterator cit;

		// Great - now we can do the actual calculations. We do this the fancy way ...
		for(cit=x->begin(); cit!=x->end(); ++cit){
			double xsquared = std::pow(*cit, 2);
			result += (cos(xsquared) + 2)*xsquared;
		}

		return result;
	}

	/********************************************************************************************/
	/**
	 * The generalized Rosenbrock function (see e.g. http://en.wikipedia.org/wiki/Rosenbrock_function)
	 *
	 * @param x The input parameters for the function
	 * @return The result of the calculation
	 */
	static double rosenbrock(boost::shared_ptr<GDoubleCollection> x) {
		std::size_t parameterSize = x->size();
		double result = 0.;

		// Check the size of the parameter vector -- must be at least 2
		if(parameterSize < 2) {
			std::ostringstream error;
			error << "In GFunctionIndividual::rosenbrock(): Error!" << std::endl
				  << "Need to use at least two input dimensions, but got " << parameterSize << std::endl;
			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}

		for(std::size_t i=0; i<(parameterSize-1); i++) {
			double firstTerm = pow(1.-x->at(i),2.);
			double secondTerm = 100.*pow(x->at(i+1)-pow(x->at(i), 2.),2.);
			result += firstTerm + secondTerm;
		}

		return result;
	}

	/********************************************************************************************/
};


} /* namespace GenEvA */
} /* namespace Gem */

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GFunctionIndividual)

#endif /* GFUNCTIONINDIVIDUAL_HPP_ */
