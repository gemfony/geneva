/**
 * @file GFunctionIndividual.hpp
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

// Geneva header files go here
#include <geneva/GDoubleCollection.hpp>
#include <geneva/GDoubleGaussAdaptor.hpp>
#include <geneva/GParameterSet.hpp>
#include <common/GParserBuilder.hpp>

namespace Gem
{
namespace Geneva
{

/************************************************************************************************/
/**
 * This enum denotes the possible demo function types
 */
enum demoFunction {
	  PARABOLA=0
	, BERLICH=1
	, ROSENBROCK=2
	, ACKLEY=3
	, RASTRIGIN=4
	, SCHWEFEL=5
	, SALOMON=6
};

const demoFunction MAXDEMOFUNCTION=SALOMON;

/************************************************************************************************/
/**
 * This individual searches for a minimum of a number of predefined functions, each capable
 * of processing their input in multiple dimensions.
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
		GFunctionIndividual<dF>::load_(&cp);
		return *this;
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GFunctionIndividual object
	 *
	 * @param  cp A constant reference to another GFunctionIndividual object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GFunctionIndividual<dF>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GFunctionIndividual<dF>::operator==","cp", CE_SILENT);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another GFunctionIndividual object
	 *
	 * @param  cp A constant reference to another GFunctionIndividual object
	 * @return A boolean indicating whether both objects are in-equal
	 */
	bool operator!=(const GFunctionIndividual<dF>& cp) const {
		using namespace Gem::Common;
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
			const Gem::Common::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Common;

		// Check that we are not accidently assigning this object to itself
		GObject::selfAssignmentCheck<GFunctionIndividual<dF> >(&cp);

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

	/*******************************************************************************************/
	/**
	 * A factory function that returns a function individual of the desired type.
	 *
	 * @param df The id of the desired function individual
	 * @return A function individual of the desired type
	 */
	static boost::shared_ptr<GParameterSet> getFunctionIndividual(const demoFunction& df) {
		boost::shared_ptr<GParameterSet> functionIndividual_ptr;

		// Set up a single function individual, depending on the expected function type
		switch(df) {
		case PARABOLA:
			functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual<PARABOLA> >(new GFunctionIndividual<PARABOLA>());
			break;
		case BERLICH:
			functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual<BERLICH> >(new GFunctionIndividual<BERLICH>());
			break;
		case ROSENBROCK:
			functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual<ROSENBROCK> >(new GFunctionIndividual<ROSENBROCK>());
			break;
		case ACKLEY:
			functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual<ACKLEY> >(new GFunctionIndividual<ACKLEY>());
			break;
		case RASTRIGIN:
			functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual<RASTRIGIN> >(new GFunctionIndividual<RASTRIGIN>());
			break;
		case SCHWEFEL:
			functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual<SCHWEFEL> >(new GFunctionIndividual<SCHWEFEL>());
			break;
		case SALOMON:
			functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual<SALOMON> >(new GFunctionIndividual<SALOMON>());
			break;
		}

		return functionIndividual_ptr;
	}

	/*******************************************************************************************/
	/**
	 * A factory function that returns a function individual, setting its values according to
	 * the specifications of a configuration file.
	 *
	 * @return A function individual of the desired type
	 */
	static boost::shared_ptr<GParameterSet> getFunctionIndividual(const std::string& cf = "GFunctionIndividual.cfg") {
		using namespace Gem::Common;

		double adProb = 0.05;
		boost::uint32_t adaptionThreshold = 1;
		double sigma = 0.5;
		double sigmaSigma = 0.8;
		double minSigma = 0.001;
		double maxSigma = 2.;
		std::size_t parDim = 2;
		double minVar = -30.;
		double maxVar =  30.;
		boost::uint32_t processingCycles;
		boost::uint16_t evalFunction = 3;

		// Register a number of parameters
		GParserBuilder gpb(cf.c_str());
		gpb.registerParameter("adProb", adProb, 0.05);
		gpb.registerParameter("adaptionThreshold", adaptionThreshold, (boost::uint32_t)1);
		gpb.registerParameter("sigma", sigma, 0.5);
		gpb.registerParameter("sigmaSigma", sigmaSigma, 0.8);
		gpb.registerParameter("minSigma", minSigma, 0.001);
		gpb.registerParameter("maxSigma", maxSigma, 2.);
		gpb.registerParameter("parDim", parDim, (std::size_t)2);
		gpb.registerParameter("minVar", minVar, -30.);
		gpb.registerParameter("maxVar", maxVar,  30.);
		gpb.registerParameter("processingCycles", processingCycles, (boost::uint32_t)1);
		gpb.registerParameter("evalFunction", evalFunction, (boost::uint16_t)3);

		// Read the parameters from the configuration file
		if(!gpb.parse()) {
			std::ostringstream error;
			error << "In GFunctionIndividual::getFunctionIndividual(const std::string&): Error!" << std::endl
				  << "Could not parse configuration file " << cf << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// Assign the demo function
		if(evalFunction > (boost::uint16_t)MAXDEMOFUNCTION) {
			std::cout << "Error: Invalid evaluation function: " << evalFunction << std::endl
					  << "Assigning parabola instead." << std::endl;
			evalFunction = 0;
		}
		demoFunction df=(demoFunction)evalFunction;

		boost::shared_ptr<GParameterSet> functionIndividual_ptr;

		// Set up a single function individual, depending on the expected function type
		switch(df) {
		case PARABOLA:
			functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual<PARABOLA> >(new GFunctionIndividual<PARABOLA>());
			break;
		case BERLICH:
			functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual<BERLICH> >(new GFunctionIndividual<BERLICH>());
			break;
		case ROSENBROCK:
			functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual<ROSENBROCK> >(new GFunctionIndividual<ROSENBROCK>());
			break;
		case ACKLEY:
			functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual<ACKLEY> >(new GFunctionIndividual<ACKLEY>());
			break;
		case RASTRIGIN:
			functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual<RASTRIGIN> >(new GFunctionIndividual<RASTRIGIN>());
			break;
		case SCHWEFEL:
			functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual<SCHWEFEL> >(new GFunctionIndividual<SCHWEFEL>());
			break;
		case SALOMON:
			functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual<SALOMON> >(new GFunctionIndividual<SALOMON>());
			break;
		}

		// Set up a GDoubleCollection with dimension values, each initialized
		// with a random number in the range [min,max[
		boost::shared_ptr<GDoubleCollection> gdc_ptr(new GDoubleCollection(parDim, minVar, maxVar));
		// Let the GDoubleCollection know about its desired initialization range
		gdc_ptr->setInitBoundaries(minVar, maxVar);

		// Set up and register an adaptor for the collection, so it
		// knows how to be adapted.
		boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(sigma, sigmaSigma, minSigma, maxSigma));
		gdga_ptr->setAdaptionThreshold(adaptionThreshold);
		gdga_ptr->setAdaptionProbability(adProb);
		gdc_ptr->addAdaptor(gdga_ptr);

		// Make the parameter collection known to this individual
		functionIndividual_ptr->push_back(gdc_ptr);
		functionIndividual_ptr->setProcessingCycles(processingCycles);

		return functionIndividual_ptr;
	}

	/*******************************************************************************************/
	/**
	 * This function converts the function id to a string representation. This is a convenience
	 * function that is mostly used in GArgumentParser.cpp of various Geneva examples.
	 *
	 * @param df The id of the desired function individual
	 * @return A string representing the name of the current function
	 */
	static std::string getStringRepresentation(const demoFunction& df) {
		std::string result;

		// Set up a single function individual, depending on the expected function type
		switch(df) {
		case PARABOLA:
			result="Parabola";
			break;
		case BERLICH:
			result="Berlich noisy parabola";
			break;
		case ROSENBROCK:
			result="Rosenbrock";
			break;
		case ACKLEY:
			result="Ackley";
			break;
		case RASTRIGIN:
			result="Rastrigin";
			break;
		case SCHWEFEL:
			result="Schwefel";
			break;
		case SALOMON:
			result="Salomon";
			break;
		}

		return result;
	}

	/*******************************************************************************************/
	/**
	 * Retrieves a string in ROOT format (see http://root.cern.ch) of the 2D version of a
	 * given function.
	 *
	 * @param df The id of the desired function individual
	 * @return A string suitable for plotting a 2D version of this function with the ROOT analysis framework
	 */
	static std::string get2DROOTFunction(const demoFunction& df) {
		std::string result;

		// Set up a single function individual, depending on the expected function type
		switch(df) {
		case PARABOLA:
			result="x^2 + y^2";
			break;
		case BERLICH:
			result="(cos(x^2 + y^2) + 2.) * (x^2 + y^2)";
			break;
		case ROSENBROCK:
			result="100.*(x^2 - y)^2 + (1 - x)^2";
			break;
		case ACKLEY:
			result="exp(-0.2)*sqrt(x^2 + y^2) + 3.*(cos(2.*x) + sin(2.*y))";
			break;
		case RASTRIGIN:
			result="20.+(x^2 - 10.*cos(2*pi*x)) + (y^2 - 10.*cos(2*pi*y))";
			break;
		case SCHWEFEL:
			result="-0.5*(x*sin(sqrt(abs(x))) + y*sin(sqrt(abs(y))))";
			break;
		case SALOMON:
			result="-cos(2.*pi*sqrt(x^2 + y^2)) + 0.1*sqrt(x^2 + y^2) + 1.";
			break;
		}

		return result;
	}

	/*******************************************************************************************/
	/**
	 * Retrieves the minimum x-value(s) of a given (2D) demo function
	 *
	 * @param df The id of the desired function individual
	 * @return The x-coordinate(s) of the global optimium in 2D
	 */
	static std::vector<double> getXMin(const demoFunction& df) {
		std::vector<double> result;

		// Set up a single function individual, depending on the expected function type
		switch(df) {
		case PARABOLA:
			result.push_back(0.);
			break;
		case BERLICH:
			result.push_back(0.);
			break;
		case ROSENBROCK:
			result.push_back(1.);
			break;
		case ACKLEY:
			// two global optima
			result.push_back(-1.5096201);
			result.push_back( 1.5096201);
			break;
		case RASTRIGIN:
			result.push_back(0.);
			break;
		case SCHWEFEL:
			result.push_back(420.968746);
			break;
		case SALOMON:
			result.push_back(0.);
			break;
		}

		return result;
	}

	/*******************************************************************************************/
	/**
	 * Retrieves the minimum y-value(s) of a given (2D) demo function
	 *
	 * @param df The id of the desired function individual
	 * @return The y-coordinate(s) of the global optimium in 2D
	 */
	static std::vector<double> getYMin(const demoFunction& df) {
		std::vector<double> result;

		// Set up a single function individual, depending on the expected function type
		switch(df) {
		case PARABOLA:
			result.push_back(0.);
			break;
		case BERLICH:
			result.push_back(0.);
			break;
		case ROSENBROCK:
			result.push_back(1.);
			break;
		case ACKLEY:
			result.push_back(-0.7548651);
			break;
		case RASTRIGIN:
			result.push_back(0.);
			break;
		case SCHWEFEL:
			result.push_back(420.968746);
			break;
		case SALOMON:
			result.push_back(0.);
			break;
		}

		return result;
	}

protected:
	/********************************************************************************************/
	/**
	 * Loads the data of another GFunctionIndividual, camouflaged as a GObject
	 *
	 * @param cp A copy of another GFunctionIndividual, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp){
		// Check that we are not accidently assigning this object to itself
		GObject::selfAssignmentCheck<GFunctionIndividual<dF> >(cp);

		// Load our parent class'es data ...
		GParameterSet::load_(cp);

		// ... no local data
	}

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
		throw(Gem::Common::gemfony_error_condition(error.str()));
		return 0.; // Make the compiler happy
	}

	/********************************************************************************************/
private:
	const demoFunction demoFunction_; ///< Specifies which demo function is being used
};

//////////////////////////////////////////////////////////////////////////////////////////////////
// A number of specializations of fitnessCalculation() for the three function types

template<> double GFunctionIndividual<PARABOLA>::fitnessCalculation();
template<> double GFunctionIndividual<BERLICH>::fitnessCalculation();
template<> double GFunctionIndividual<ROSENBROCK>::fitnessCalculation();
template<> double GFunctionIndividual<ACKLEY>::fitnessCalculation();
template<> double GFunctionIndividual<RASTRIGIN>::fitnessCalculation();
template<> double GFunctionIndividual<SCHWEFEL>::fitnessCalculation();
template<> double GFunctionIndividual<SALOMON>::fitnessCalculation();

/************************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GFUNCTIONINDIVIDUAL_HPP_ */
