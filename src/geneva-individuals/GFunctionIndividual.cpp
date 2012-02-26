/**
 * @file GFunctionIndividual.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
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
 * Puts a Gem::Geneva::solverFunction item into a stream
 *
 * @param o The ostream the item should be added to
 * @param ur the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::Geneva::solverFunction& ur) {
	boost::uint16_t tmp = static_cast<boost::uint16_t>(ur);
	o << tmp;
	return o;
}

/********************************************************************************************/
/**
 * Reads a Gem::Geneva::solverFunction item from a stream
 *
 * @param i The stream the item should be read from
 * @param ur The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::Geneva::solverFunction& ur) {
	boost::uint16_t tmp;
	i >> tmp;

#ifdef DEBUG
	ur = boost::numeric_cast<Gem::Geneva::solverFunction>(tmp);
#else
	ur = static_cast<Gem::Geneva::solverFunction>(tmp);
#endif /* DEBUG */

	return i;
}

/********************************************************************************************/
/**
 * The default constructor
 */
GFunctionIndividual::GFunctionIndividual()
	: demoFunction_(PARABOLA)
{ /* nothing */ }

/********************************************************************************************/
/**
 * Initialization with the desired demo function
 *
 * @param dF The id of the demo function
 */
GFunctionIndividual::GFunctionIndividual(const solverFunction& dF)
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
	const GFunctionIndividual *p_load = gobject_conversion<GFunctionIndividual>(&cp);

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
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void GFunctionIndividual::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
	, const bool& showOrigin
) {
	std::string comment;

	// Call our parent class'es function
	GParameterSet::addConfigurationOptions(gpb, showOrigin);

	// Add local data
	comment = ""; // Reset the comment string
	comment += "Specifies which demo function should be used:;";
	comment += "0: Parabola;";
	comment += "1: Berlich;";
	comment += "2: Rosenbrock;";
	comment += "3: Ackley;";
	comment += "4: Rastrigin;";
	comment += "5: Schwefel;";
	comment += "6: Salomon;";
	if(showOrigin) comment += "[GFunctionIndividual]";
	gpb.registerFileParameter<solverFunction>(
		"demoFunction" // The name of the variable
		, GO_DEF_EVALFUNCTION // The default value
		, boost::bind(
			&GFunctionIndividual::setDemoFunction
			, this
			, _1
		  )
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
}

/*******************************************************************************************/
/**
 * Allows to set the demo function
 *
 * @param dF The id if the demo function
 */
void GFunctionIndividual::setDemoFunction(solverFunction dF) {
	demoFunction_ = dF;
}

/*******************************************************************************************/
/**
 * Allows to retrieve the demo function
 *
 * @return The id of the currently selected demo function
 */
solverFunction GFunctionIndividual::getDemoFunction() const {
	return demoFunction_;
}

/********************************************************************************************/
/**
 * Allows to cross check the parameter size
 *
 * @return The number of doubles stored in this object
 */
std::size_t GFunctionIndividual::getParameterSize() const {
	// Retrieve the parameters
	std::vector<double> parVec;
	this->streamline(parVec);
	return parVec.size();
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
	const GFunctionIndividual *p_load = gobject_conversion<GFunctionIndividual>(cp);

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
 * @param The id of the target function (ignored here)
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
	case NOISYPARABOLA:
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
		result = 10*double(parameterSize);

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
 * A constructor with the ability to switch the parallelization mode. It initializes a
 * target item as needed.
 *
 * @param configFile The name of the configuration file
 */
GFunctionIndividualFactory::GFunctionIndividualFactory(const std::string& configFile)
	: Gem::Common::GFactoryT<GFunctionIndividual>(configFile)
	, adProb_(GFI_DEF_ADPROB)
	, adaptionThreshold_(GFI_DEF_ADAPTIONTHRESHOLD)
	, useBiGaussian_(GFI_DEF_USEBIGAUSSIAN)
	, sigma1_(GFI_DEF_SIGMA1)
	, sigmaSigma1_(GFI_DEF_SIGMASIGMA1)
	, minSigma1_(GFI_DEF_MINSIGMA1)
	, maxSigma1_(GFI_DEF_MAXSIGMA1)
	, sigma2_(GFI_DEF_SIGMA2)
	, sigmaSigma2_(GFI_DEF_SIGMASIGMA2)
	, minSigma2_(GFI_DEF_MINSIGMA2)
	, maxSigma2_(GFI_DEF_MAXSIGMA2)
	, delta_(GFI_DEF_DELTA)
	, sigmaDelta_(GFI_DEF_SIGMADELTA)
	, minDelta_(GFI_DEF_MINDELTA)
	, maxDelta_(GFI_DEF_MAXDELTA)
	, parDim_(GFI_DEF_PARDIM)
	, parDimLocal_(0)
	, minVar_(GFI_DEF_MINVAR)
	, maxVar_(GFI_DEF_MAXVAR)
	, useConstrainedDoubleCollection_(GFI_DEF_USECONSTRAINEDDOUBLECOLLECTION)
	, processingCycles_(GO_DEF_PROCESSINGCYCLES)
{ /* nothing */ }

/********************************************************************************************/
/**
 * The destructor
 */
GFunctionIndividualFactory::~GFunctionIndividualFactory()
{ /* nothing */ }

/********************************************************************************************/
/**
 * (Re-)Set the dimension of the function
 */
void GFunctionIndividualFactory::setParDim(std::size_t parDim) {
	if(parDim == 0) {
		raiseException(
			"In GFunctionIndividualFactory::setParDim(): Error!" << std::endl
			<< "Dimension of the function is set to 0" << std::endl
		);
	}

	parDimLocal_ = parDim;
}

/********************************************************************************************/
/**
 * Extract the minimum and maximum boundaries of the variables
 */
boost::tuple<double,double> GFunctionIndividualFactory::getVarBoundaries() const {
	return boost::tuple<double,double>(minVar_, maxVar_);
}

/********************************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
boost::shared_ptr<GFunctionIndividual> GFunctionIndividualFactory::getObject_(
	Gem::Common::GParserBuilder& gpb
	, const std::size_t& id
) {
	// Will hold the result
	boost::shared_ptr<GFunctionIndividual> target(new GFunctionIndividual());

	// Make the object's local configuration options known
	target->addConfigurationOptions(gpb, true);

	return target;
}

/********************************************************************************************/
/**
 * Allows to describe local configuration options for gradient descents
 */
void GFunctionIndividualFactory::describeLocalOptions_(Gem::Common::GParserBuilder& gpb) {
	// Describe our own options
	using namespace Gem::Courtier;

	std::string comment;

	comment = "";
	comment += "The probability for random adaption of values in evolutionary algorithms;";
	gpb.registerFileParameter<double>(
		"adProb"
		, adProb_
		, GFI_DEF_ADPROB
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "The number of calls to an adaptor after which adaption takes place;";
	gpb.registerFileParameter<boost::uint32_t>(
		"adaptionThreshold"
		, adaptionThreshold_
		, GFI_DEF_ADAPTIONTHRESHOLD
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "Whether to use a double gaussion for the adaption of parmeters in ES;";
	gpb.registerFileParameter<bool>(
		"useBiGaussian"
		, useBiGaussian_
		, GFI_DEF_USEBIGAUSSIAN
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "The sigma for gauss-adaption in ES;(or the sigma of the left peak of a double gaussian);";
	gpb.registerFileParameter<double>(
		"sigma1"
		, sigma1_
		, GFI_DEF_SIGMA1
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "Influences the self-adaption of gauss-mutation in ES;";
	gpb.registerFileParameter<double>(
		"sigmaSigma1"
		, sigmaSigma1_
		, GFI_DEF_SIGMASIGMA1
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "The minimum value of sigma1;";
	gpb.registerFileParameter<double>(
		"minSigma1"
		, minSigma1_
		, GFI_DEF_MINSIGMA1
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "The maximum value of sigma1;";
	gpb.registerFileParameter<double>(
		"maxSigma1"
		, maxSigma1_
		, GFI_DEF_MAXSIGMA1
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "The sigma of the right peak of a double gaussian (if any);";
	gpb.registerFileParameter<double>(
		"sigma2"
		, sigma2_
		, GFI_DEF_SIGMA2
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "Influences the self-adaption of gauss-mutation in ES;";
	gpb.registerFileParameter<double>(
		"sigmaSigma2"
		, sigmaSigma2_
		, GFI_DEF_SIGMASIGMA2
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "The minimum value of sigma2;";
	gpb.registerFileParameter<double>(
		"minSigma2"
		, minSigma2_
		, GFI_DEF_MINSIGMA2
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "The maximum value of sigma2;";
	gpb.registerFileParameter<double>(
		"maxSigma2"
		, maxSigma2_
		, GFI_DEF_MAXSIGMA2
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "The start distance between both peaks used for bi-gaussian mutations in ES;";
	gpb.registerFileParameter<double>(
		"delta"
		, delta_
		, GFI_DEF_DELTA
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "The width of the gaussian used for mutations of the delta parameter;";
	gpb.registerFileParameter<double>(
		"sigmaDelta"
		, sigmaDelta_
		, GFI_DEF_SIGMADELTA
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "The minimum allowed value of delta;";
	gpb.registerFileParameter<double>(
		"minDelta"
		, minDelta_
		, GFI_DEF_MINDELTA
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "The maximum allowed value of delta;";
	gpb.registerFileParameter<double>(
		"maxDelta"
		, maxDelta_
		, GFI_DEF_MAXDELTA
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "The number of dimensions used for the demo function;";
	gpb.registerFileParameter<std::size_t>(
		"parDim"
		, parDim_
		, GFI_DEF_PARDIM
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "The lower boundary of the initialization range for parameters;";
	gpb.registerFileParameter<double>(
		"minVar"
		, minVar_
		, GFI_DEF_MINVAR
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "The upper boundary of the initialization range for parameters;";
	gpb.registerFileParameter<double>(
		"maxVar"
		, maxVar_
		, GFI_DEF_MAXVAR
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "Indicates whether a GConstrainedDoubleCollection should be used;";
	gpb.registerFileParameter<bool>(
		"useConstrainedDoubleCollection"
		, useConstrainedDoubleCollection_
		, GFI_DEF_USECONSTRAINEDDOUBLECOLLECTION
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "The maximum number of processing cycles without improvement;";
	comment += "that may be used by a remote individual before it returns its result;";
	gpb.registerFileParameter<boost::uint32_t>(
		"processingCycles"
		, processingCycles_
		, GO_DEF_PROCESSINGCYCLES
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);


	// Allow our parent class to describe its options
	Gem::Common::GFactoryT<GFunctionIndividual>::describeLocalOptions_(gpb);
}

/********************************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object. In practice,
 * we add the parameter objects here
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void GFunctionIndividualFactory::postProcess_(boost::shared_ptr<GFunctionIndividual>& p) {
	boost::shared_ptr<GParameterCollectionT<double> > c_ptr;
	if(useConstrainedDoubleCollection_) {
		// Set up a collection with dimension values
		boost::shared_ptr<GConstrainedDoubleCollection> gcdc_ptr(new GConstrainedDoubleCollection(parDimLocal_?parDimLocal_:parDim_, minVar_, maxVar_));
		// Randomly initialize
		gcdc_ptr->randomInit();
		// Attach to the "parent pointer"
		c_ptr = gcdc_ptr;
	} else {
		// Set up a collection with parDimLocal_ or parDim_ values, each initialized with a random number in the range [min,max[
		// Random initialization happens in the constructor.
		boost::shared_ptr<GDoubleCollection> gdc_ptr(new GDoubleCollection(parDimLocal_?parDimLocal_:parDim_, minVar_, maxVar_));
		// Let the GDoubleCollection know about its desired initialization range
		gdc_ptr->setInitBoundaries(minVar_, maxVar_);
		// Attach to the "parent pointer"
		c_ptr = gdc_ptr;
	}

	// Set up and register an adaptor for the collection, so it
	// knows how to be adapted.
	if(useBiGaussian_) {
		boost::shared_ptr<GDoubleBiGaussAdaptor> gdbga_ptr(new GDoubleBiGaussAdaptor());
		gdbga_ptr->setAllSigma1(sigma1_, sigmaSigma1_, minSigma1_, maxSigma1_);
		gdbga_ptr->setAllSigma1(sigma2_, sigmaSigma2_, minSigma2_, maxSigma2_);
		gdbga_ptr->setAllSigma1(delta_, sigmaDelta_, minDelta_, maxDelta_);
		gdbga_ptr->setAdaptionThreshold(adaptionThreshold_);
		gdbga_ptr->setAdaptionProbability(adProb_);
		c_ptr->addAdaptor(gdbga_ptr);
	} else {
		boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(sigma1_, sigmaSigma1_, minSigma1_, maxSigma1_));
		gdga_ptr->setAdaptionThreshold(adaptionThreshold_);
		gdga_ptr->setAdaptionProbability(adProb_);
		c_ptr->addAdaptor(gdga_ptr);
	}

	// Make the parameter collection known to this individual
	p->push_back(c_ptr);
	p->setProcessingCycles(processingCycles_);
}

/********************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
