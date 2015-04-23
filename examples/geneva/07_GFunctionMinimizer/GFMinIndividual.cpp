/**
 * @file GFMinIndividual.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
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
 * http://www.gemfony.eu .
 */

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
	boost::uint16_t tmp = static_cast<boost::uint16_t>(tF);
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
	boost::uint16_t tmp;
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
 * The default constructor
 */
GFMinIndividual::GFMinIndividual()
	: targetFunction_(GFM_PARABOLA)
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

/******************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another GFMinIndividual
 */
const GFMinIndividual& GFMinIndividual::operator=(const GFMinIndividual& cp){
	GFMinIndividual::load_(&cp);
	return *this;
}

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
	// Check that we are indeed dealing with an object of the same type and that we are not
	// accidently trying to compare this object with itself.
	const GFMinIndividual *p_load = gobject_conversion<GFMinIndividual>(cp);

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
	case GFM_PARABOLA:
		return parabola(parVec);
		break;

	//-----------------------------------------------------------
	// A "noisy" parabola, i.e. a parabola with a very large
	// number of overlaid local optima
	case GFM_NOISYPARABOLA:
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
GFMinIndividualFactory::GFMinIndividualFactory(const std::string& configFile)
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

	// Randomly initialize
	gcdc_ptr->randomInit(ACTIVEONLY);

	std::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(sigma_, sigmaSigma_, minSigma_, maxSigma_));
	gdga_ptr->setAdaptionProbability(adProb_);
	gcdc_ptr->addAdaptor(gdga_ptr);

	// Make the parameter collection known to this individual
	p->push_back(gcdc_ptr);
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
