/**
 * @file GFunctionIndividual.cpp
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

#include "geneva-individuals/GFunctionIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GFunctionIndividual)

namespace Gem
{
namespace Geneva
{

/********************************************************************************************/
/**
 * The default constructor
 */
GFunctionIndividual::GFunctionIndividual()
: demoFunction_(PARABOLA)
{ /* nothing */ }

/********************************************************************************************/
/**
 * Destructor
 */
GFunctionIndividualFactory::~GFunctionIndividualFactory()
{ /* nothing */ }

/********************************************************************************************/
/**
 * Initialization with the desired demo function
 *
 * @param dF The id if the demo function
 */
GFunctionIndividual::GFunctionIndividual(const demoFunction& dF)
: demoFunction_(dF)
{ /* nothing */ }

/********************************************************************************************/
/**
 * A standard copy constructor
 *
 * @param cp A copy of another GFunctionIndidivual
 */
GFunctionIndividual::GFunctionIndividual(const GFunctionIndividual& cp)
: GParameterSet(cp)
, demoFunction_(cp.demoFunction_)
{ /* nothing */	}

/********************************************************************************************/
/**
 * The standard destructor
 */
GFunctionIndividual::~GFunctionIndividual()
{ /* nothing */	}

/********************************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another GFunctionIndividual
 */
const GFunctionIndividual& GFunctionIndividual::operator=(const GFunctionIndividual& cp){
	GFunctionIndividual::load_(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Checks for equality with another GFunctionIndividual object
 *
 * @param  cp A constant reference to another GFunctionIndividual object
 * @return A boolean indicating whether both objects are equal
 */
bool GFunctionIndividual::operator==(const GFunctionIndividual& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GFunctionIndividual::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GFunctionIndividual object
 *
 * @param  cp A constant reference to another GFunctionIndividual object
 * @return A boolean indicating whether both objects are in-equal
 */
bool GFunctionIndividual::operator!=(const GFunctionIndividual& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GFunctionIndividual::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GFunctionIndividual::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
	using namespace Gem::Common;

	// Check that we are indeed dealing with an object of the same type and that we are not
	// accidently trying to compare this object with itself.
	const GFunctionIndividual *p_load = conversion_cast<GFunctionIndividual>(&cp);

	// Will hold possible deviations from the expectation, including explanations
	std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GParameterSet::checkRelationshipWith(cp, e, limit, "GFunctionIndividual", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GFunctionIndividual", demoFunction_, p_load->demoFunction_, "demoFunction_", "p_load->demoFunction_", e , limit));

	return evaluateDiscrepancies("GFunctionIndividual", caller, deviations, e);
}

/*******************************************************************************************/
/**
 * Allows to set the demo function
 *
 * @param dF The id if the demo function
 */
void GFunctionIndividual::setDemoFunction(const demoFunction& dF) {
	demoFunction_ = dF;
}

/*******************************************************************************************/
/**
 * Allows to retrieve the demo function
 *
 * @return The id of the currently selected demo function
 */
demoFunction GFunctionIndividual::getDemoFunction() const {
	return demoFunction_;
}


/********************************************************************************************/
/**
 * Loads the data of another GFunctionIndividual, camouflaged as a GObject
 *
 * @param cp A copy of another GFunctionIndividual, camouflaged as a GObject
 */
void GFunctionIndividual::load_(const GObject* cp){
	// Check that we are indeed dealing with an object of the same type and that we are not
	// accidently trying to compare this object with itself.
	const GFunctionIndividual *p_load = conversion_cast<GFunctionIndividual>(cp);

	// Load our parent class'es data ...
	GParameterSet::load_(cp);

	// ... and then our local data
	demoFunction_ = p_load->demoFunction_;
}

/********************************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
GObject* GFunctionIndividual::clone_() const {
	return new GFunctionIndividual(*this);
}

/********************************************************************************************/
/**
 * The actual value calculation takes place here
 *
 * @return The value of this object, as calculated with the evaluation function
 */
double GFunctionIndividual::fitnessCalculation(){
	double result = 0;

	// Retrieve the parameters
	std::vector<double> parVec;
	this->streamline(parVec);
	std::size_t parameterSize = parVec.size();

	// Perform the actual calculation
	switch(demoFunction_) {
	//-----------------------------------------------------------
	// A simple, multi-dimensional parabola
	case PARABOLA:
	{
		for(std::size_t i=0; i<parameterSize; i++) {
			result += GSQUARED(parVec[i]);
		}
	}
	break;

	//-----------------------------------------------------------
	// A "noisy" parabola, i.e. a parabola with a very large number of overlaid local optima
	case BERLICH:
	{
		double xsquared = 0.;
		for(std::size_t i=0; i<parameterSize; i++){
			xsquared += GSQUARED(parVec[i]);
		}
		result = (cos(xsquared) + 2.) * xsquared;
	}
	break;

	//-----------------------------------------------------------
	// The generalized Rosenbrock function (see e.g. http://en.wikipedia.org/wiki/Rosenbrock_function)
	// or http://www.it.lut.fi/ip/evo/functions/node5.html .
	case ROSENBROCK:
	{
#ifdef DEBUG
		// Check the size of the parameter vector -- must be at least 2
		if(parameterSize < 2) {
			raiseException(
					"In GFunctionIndividual::fitnessCalculation() / ROSENBROCK: Error!" << std::endl
					<< "Need to use at least two input dimensions, but got " << parameterSize
			);
		}
#endif /* DEBUG */

		for(std::size_t i=0; i<(parameterSize-1); i++) {
			result += 100.*GSQUARED(GSQUARED(parVec[i]) - parVec[i+1]) + GSQUARED(1.-parVec[i]);
		}
	}
	break;

	//-----------------------------------------------------------
	// The Ackeley function (see e.g. http://www.it.lut.fi/ip/evo/functions/node14.html)
	case ACKLEY:
	{
#ifdef DEBUG
		// Check the size of the parameter vector -- must be at least 2
		if(parameterSize < 2) {
			raiseException(
					"In GFunctionIndividual::fitnessCalculation() / ACKLEY: Error!" << std::endl
					<< "Need to use at least two input dimensions, but got " << parameterSize
			);
		}
#endif /* DEBUG */

		for(std::size_t i=0; i<(parameterSize-1); i++) {
			result += (exp(-0.2)*sqrt(GSQUARED(parVec[i]) + GSQUARED(parVec[i+1])) + 3.*(cos(2.*parVec[i]) + sin(2.*parVec[i+1])));
		}
	}
	break;

	//-----------------------------------------------------------
	// The Rastrigin function (see e.g. http://www.it.lut.fi/ip/evo/functions/node6.html)
	case RASTRIGIN:
	{
		double result = 10*double(parameterSize);

		for(std::size_t i=0; i<parameterSize; i++) {
			result += (GSQUARED(parVec[i]) - 10.*cos(2*M_PI*parVec[i]));
		}
	}
	break;

	//-----------------------------------------------------------
	// The Schwefel function (see e.g. http://www.it.lut.fi/ip/evo/functions/node10.html)
	case SCHWEFEL:
	{
		for(std::size_t i=0; i<parameterSize; i++) {
			result += -parVec[i]*sin(sqrt(fabs(parVec[i])));
		}

		result /= parameterSize;
	}
	break;

	//-----------------------------------------------------------
	// The Salomon function (see e.g. http://www.it.lut.fi/ip/evo/functions/node12.html)
	case SALOMON:
	{
		double sum_root = 0.;
		for(std::size_t i=0; i<parameterSize; i++) {
			sum_root += GSQUARED(parVec[i]);
		}
		sum_root=sqrt(sum_root);

		result = -cos(2*M_PI*sum_root) + 0.1*sum_root + 1;
	}
	break;

	//-----------------------------------------------------------
	};

	// Let the audience know
	return result;
}

/********************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////
/********************************************************************************************/
/**
 * The standard constructor for this class
 *
 * @param cF The name of the configuration file
 */
GFunctionIndividualFactory::GFunctionIndividualFactory(const std::string& cF)
	: GIndividualFactoryT<GFunctionIndividual>(cF)
	, adProb(0.05)
	, adaptionThreshold(1)
	, sigma(0.5)
	, sigmaSigma(0.8)
	, minSigma(0.001)
	, maxSigma(2.)
	, parDim(2)
	, minVar(-30.)
	, maxVar(30.)
	, processingCycles(1)
	, evalFunction(3)
{ /* nothing */ }

/********************************************************************************************/
/**
 * Allows to describe configuration options in derived classes
 */
void GFunctionIndividualFactory::describeConfigurationOptions_() {
	gpb.registerParameter("adProb", adProb, adProb);
	gpb.registerParameter("adaptionThreshold", adaptionThreshold, adaptionThreshold);
	gpb.registerParameter("sigma", sigma, sigma);
	gpb.registerParameter("sigmaSigma", sigmaSigma, sigmaSigma);
	gpb.registerParameter("minSigma", minSigma, minSigma);
	gpb.registerParameter("maxSigma", maxSigma, maxSigma);
	gpb.registerParameter("parDim", parDim, parDim);
	gpb.registerParameter("minVar", minVar, minVar);
	gpb.registerParameter("maxVar", maxVar,  maxVar);
	gpb.registerParameter("processingCycles", processingCycles, processingCycles);
	gpb.registerParameter("evalFunction", evalFunction, evalFunction);
}

/********************************************************************************************/
/**
 * Creates individuals of the desired type. The argument "id" gives the function a means
 * of detecting how often it has been called before. The id will be incremented for each call.
 * This can e.g. be used to act differently for the first call to this function.
 *
 * @param id The id of the individual to be created
 * @return An individual of the desired type
 */
boost::shared_ptr<GFunctionIndividual> GFunctionIndividualFactory::getIndividual_(const std::size_t& id) {
	// Assign the demo function
	if(evalFunction > (boost::uint16_t)MAXDEMOFUNCTION) {
		std::cout << "Error: Invalid evaluation function: " << evalFunction << std::endl
				  << "Assigning parabola instead." << std::endl;
		evalFunction = 0;
	}
	demoFunction df=(demoFunction)evalFunction;

	boost::shared_ptr<GFunctionIndividual> functionIndividual_ptr;

	// Set up a single function individual, depending on the expected function type
	switch(df) {
	case PARABOLA:
		functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual>(new GFunctionIndividual(PARABOLA));
		break;
	case BERLICH:
		functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual>(new GFunctionIndividual(BERLICH));
		break;
	case ROSENBROCK:
		functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual>(new GFunctionIndividual(ROSENBROCK));
		break;
	case ACKLEY:
		functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual>(new GFunctionIndividual(ACKLEY));
		break;
	case RASTRIGIN:
		functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual>(new GFunctionIndividual(RASTRIGIN));
		break;
	case SCHWEFEL:
		functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual>(new GFunctionIndividual(SCHWEFEL));
		break;
	case SALOMON:
		functionIndividual_ptr = boost::shared_ptr<GFunctionIndividual>(new GFunctionIndividual(SALOMON));
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

/********************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
