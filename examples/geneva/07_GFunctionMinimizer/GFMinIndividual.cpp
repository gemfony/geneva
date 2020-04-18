/**
 * @file GFMinIndividual.cpp
 */

/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "GFMinIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GFMinIndividual)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * Puts a Gem::Geneva::targetFunction item into a stream
 *
 * @param o The ostream the item should be added to
 * @param tF the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::Geneva::targetFunction& tF) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(tF);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::targetFunction item from a stream
 *
 * @param i The stream the item should be read from
 * @param tF The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::Geneva::targetFunction& tF) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	tF = boost::numeric_cast<Gem::Geneva::targetFunction>(tmp);
#else
	tF = static_cast<Gem::Geneva::targetFunction>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * The default constructor. Data members may be initialized in the class body.
 */
GFMinIndividual::GFMinIndividual()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor
 *
 * @param cp A copy of another GFunctionIndidivual
 */
GFMinIndividual::GFMinIndividual(const GFMinIndividual& cp)
	: GParameterSet(cp)
	, targetFunction_(cp.targetFunction_)
{ /* nothing */	}

/******************************************************************************/
/**
 * The standard destructor
 */
GFMinIndividual::~GFMinIndividual()
{ /* nothing */	}

/*******************************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GFMinIndividual::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
) {
	// Call our parent class'es function
	GParameterSet::addConfigurationOptions(gpb);

	// Add local data
	gpb.registerFileParameter<targetFunction>(
		"targetFunction" // The name of the variable
		, GO_DEF_TARGETFUNCTION // The default value
		, [this](targetFunction tF) { this->setTargetFunction(tF); }
	)
	<< "Specifies which target function should be used:" << std::endl
	<< "0: Parabola" << std::endl
	<< "1: Berlich";
}

/*******************************************************************************************/
/**
 * Allows to set the demo function
 *
 * @param tF The id if the demo function
 */
void GFMinIndividual::setTargetFunction(targetFunction tF) {
	targetFunction_ = tF;
}

/*******************************************************************************************/
/**
 * Allows to retrieve the demo function
 *
 * @return The id of the currently selected demo function
 */
targetFunction GFMinIndividual::getTargetFunction() const {
	return targetFunction_;
}

/*******************************************************************************************/
/**
 * Retrieves the average value of sigma used in Gauss adaptors. Note: This function is highly
 * dependent on the parameter object loaded into this class. It is not meant for general
 * consumption, but has been added here to allow an optimization monitor demo to extract
 * further information.
 *
 * @return The average value of sigma used in Gauss adaptors
 */
double GFMinIndividual::getAverageSigma() const {
	// Extract the parameter object
	std::shared_ptr<GConstrainedDoubleCollection> ind
		= this->at<GConstrainedDoubleCollection>(0);

	// Extract the adaptor
	std::shared_ptr<GDoubleGaussAdaptor> adaptor
		= ind->getAdaptor<GDoubleGaussAdaptor>();

	// Extract and return the sigma value. Only a single parameter object
	// has been registered, so we do not need to calculate any averages.
	return adaptor->getSigma();
}

/******************************************************************************/
/**
 * Loads the data of another GFMinIndividual, camouflaged as a GObject
 *
 * @param cp A copy of another GFMinIndividual, camouflaged as a GObject
 */
void GFMinIndividual::load_(const GObject* cp){
	// Check that we are dealing with a GFMinIndividual reference independent of this object and convert the pointer
	const GFMinIndividual *p_load = Gem::Common::g_convert_and_compare<GObject, GFMinIndividual>(cp, this);

	// Load our parent class'es data ...
	GParameterSet::load_(cp);

	// ... and then our local data
	targetFunction_ = p_load->targetFunction_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
GObject* GFMinIndividual::clone_() const {
	return new GFMinIndividual(*this);
}

/******************************************************************************/
/**
 * The actual value calculation takes place here
 *
 * @param The id of the target function (ignored here)
 * @return The value of this object, as calculated with the evaluation function
 */
double GFMinIndividual::fitnessCalculation() {
	// Retrieve the parameters
	std::vector<double> parVec;
	this->streamline(parVec);

	// Perform the actual calculation
	switch(targetFunction_) {
		//-----------------------------------------------------------
		// A simple, multi-dimensional parabola
		case targetFunction::GFM_PARABOLA:
			return parabola(parVec);
			break;

			//-----------------------------------------------------------
			// A "noisy" parabola, i.e. a parabola with a very large
			// number of overlaid local optima
		case targetFunction::GFM_NOISYPARABOLA:
			return noisyParabola(parVec);
			break;
			//-----------------------------------------------------------
	};

	// Make the compiler happy
	return 0.;
}

/******************************************************************************/
/**
 * A simple n-dimensional parabola
 */
double GFMinIndividual::parabola(const std::vector<double>& parVec) const {
	double result = 0.;

	std::vector<double>::const_iterator cit;
	for(cit=parVec.begin(); cit!=parVec.end(); ++cit) {
		result += GSQUARED(*cit);
	}

	return result;
}

/******************************************************************************/
/**
 * A "noisy" parabola
 */
double GFMinIndividual::noisyParabola(const std::vector<double>& parVec) const {
	double xsquared = 0.;

	std::vector<double>::const_iterator cit;
	for(cit=parVec.begin(); cit!=parVec.end(); ++cit) {
		xsquared += GSQUARED(*cit);
	}

	return (cos(xsquared) + 2.) * xsquared;
}

/******************************************************************************/
/**
 * Provide an easy way to print the individual's content
 */
std::ostream& operator<<(std::ostream& s, const Gem::Geneva::GFMinIndividual& f) {
	std::vector<double> parVec;
	f.streamline(parVec);

	std::vector<double>::iterator it;
	for(it=parVec.begin(); it!=parVec.end(); ++it) {
		std::cout << (it-parVec.begin()) << ": " << *it << std::endl;
	}

	return s;
}

/******************************************************************************/
/**
 * Provide an easy way to print the individual's content through a smart-pointer
 */
std::ostream& operator<<(std::ostream& s, std::shared_ptr<Gem::Geneva::GFMinIndividual> f_ptr) {
	return operator<<(s,*f_ptr);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode. It initializes a
 * target item as needed.
 *
 * @param configFile The name of the configuration file
 */
GFMinIndividualFactory::GFMinIndividualFactory(std::filesystem::path const& configFile)
	: Gem::Common::GFactoryT<GParameterSet>(configFile)
	, adProb_(GFI_DEF_ADPROB)
	, sigma_(GFI_DEF_SIGMA)
	, sigmaSigma_(GFI_DEF_SIGMASIGMA)
	, minSigma_(GFI_DEF_MINSIGMA)
	, maxSigma_(GFI_DEF_MAXSIGMA)
	, parDim_(GFI_DEF_PARDIM)
	, minVar_(GFI_DEF_MINVAR)
	, maxVar_(GFI_DEF_MAXVAR)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GFMinIndividualFactory::~GFMinIndividualFactory()
{ /* nothing */ }

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr<GParameterSet> GFMinIndividualFactory::getObject_(
	Gem::Common::GParserBuilder& gpb
	, const std::size_t& id
) {
	// Will hold the result
	std::shared_ptr<GFMinIndividual> target(new GFMinIndividual());

	// Make the object's local configuration options known
	target->addConfigurationOptions(gpb);

	return target;
}

/******************************************************************************/
/**
 * Allows to describe local configuration options for gradient descents
 */
void GFMinIndividualFactory::describeLocalOptions_(Gem::Common::GParserBuilder& gpb) {
	// Describe our own options
	using namespace Gem::Courtier;

	std::string comment;

	comment = "";
	comment += "The probability for random adaptions of values in evolutionary algorithms;";
	gpb.registerFileParameter<double>(
		"adProb"
		, adProb_
		, GFI_DEF_ADPROB
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "The sigma for gauss-adaption in ES;";
	gpb.registerFileParameter<double>(
		"sigma"
		, sigma_
		, GFI_DEF_SIGMA
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "Influences the self-adaption of gauss-mutation in ES;";
	gpb.registerFileParameter<double>(
		"sigmaSigma"
		, sigmaSigma_
		, GFI_DEF_SIGMASIGMA
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "The minimum amount value of sigma;";
	gpb.registerFileParameter<double>(
		"minSigma"
		, minSigma_
		, GFI_DEF_MINSIGMA
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = "";
	comment += "The maximum amount value of sigma;";
	gpb.registerFileParameter<double>(
		"maxSigma"
		, maxSigma_
		, GFI_DEF_MAXSIGMA
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

	// Allow our parent class to describe its options
	Gem::Common::GFactoryT<GParameterSet>::describeLocalOptions_(gpb);
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object. In practice,
 * we add the parameter objects here
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void GFMinIndividualFactory::postProcess_(std::shared_ptr<GParameterSet>& p) {
	// Set up a collection with parDim_ values
	std::shared_ptr<GConstrainedDoubleCollection> gcdc_ptr(new GConstrainedDoubleCollection(parDim_, minVar_, maxVar_));

	std::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(sigma_, sigmaSigma_, minSigma_, maxSigma_));
	gdga_ptr->setAdaptionProbability(adProb_);
	gcdc_ptr->addAdaptor(gdga_ptr);

	// Make the parameter collection known to this individual
	p->push_back(gcdc_ptr);

	// Randomly initialize
	p->randomInit(activityMode::ACTIVEONLY);
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
