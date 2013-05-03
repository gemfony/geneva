/**
 * @file GMetaOptimizerIndividual.cpp
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

#include "GMetaOptimizerIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GMetaOptimizerIndividual)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor.
 */
GMetaOptimizerIndividual::GMetaOptimizerIndividual()
   : GParameterSet()
   , nRunsPerOptimization_(GMETAOPT_DEF_NRUNSPEROPT)
   , fitnessTarget_(GMETAOPT_DEF_FITNESSTARGET)
   , iterationThreshold_(GMETAOPT_DEF_ITERATIONTHRESHOLD)
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor
 *
 * @param cp A copy of another GFunctionIndidivual
 */
GMetaOptimizerIndividual::GMetaOptimizerIndividual(const GMetaOptimizerIndividual& cp)
	: GParameterSet(cp)
   , nRunsPerOptimization_(cp.nRunsPerOptimization_)
   , fitnessTarget_(cp.fitnessTarget_)
   , iterationThreshold_(cp.iterationThreshold_)
{ /* nothing */	}

/******************************************************************************/
/**
 * The standard destructor
 */
GMetaOptimizerIndividual::~GMetaOptimizerIndividual()
{ /* nothing */	}

/******************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another GMetaOptimizerIndividual
 */
const GMetaOptimizerIndividual& GMetaOptimizerIndividual::operator=(const GMetaOptimizerIndividual& cp){
	GMetaOptimizerIndividual::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GMetaOptimizerIndividual object
 *
 * @param  cp A constant reference to another GMetaOptimizerIndividual object
 * @return A boolean indicating whether both objects are equal
 */
bool GMetaOptimizerIndividual::operator==(const GMetaOptimizerIndividual& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GMetaOptimizerIndividual::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GMetaOptimizerIndividual object
 *
 * @param  cp A constant reference to another GMetaOptimizerIndividual object
 * @return A boolean indicating whether both objects are in-equal
 */
bool GMetaOptimizerIndividual::operator!=(const GMetaOptimizerIndividual& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GMetaOptimizerIndividual::operator!=","cp", CE_SILENT);
}

/******************************************************************************/
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
boost::optional<std::string> GMetaOptimizerIndividual::checkRelationshipWith(const GObject& cp,
      const Gem::Common::expectation& e,
      const double& limit,
      const std::string& caller,
      const std::string& y_name,
      const bool& withMessages) const
{
   using namespace Gem::Common;
   using namespace Gem::Geneva;

   // Check that we are indeed dealing with a GParamterBase reference
   const GMetaOptimizerIndividual *p_load = gobject_conversion<GMetaOptimizerIndividual>(&cp);

   // Will hold possible deviations from the expectation, including explanations
   std::vector<boost::optional<std::string> > deviations;

   // Check our parent class'es data ...
   deviations.push_back(Gem::Geneva::GParameterSet::checkRelationshipWith(cp, e, limit, "GTestIndividual1", y_name, withMessages));

   // ... and then our local data
   deviations.push_back(checkExpectation(withMessages, "GMetaOptimizerIndividual", nRunsPerOptimization_, p_load->nRunsPerOptimization_, "nRunsPerOptimization_", "p_load->nRunsPerOptimization_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GMetaOptimizerIndividual", fitnessTarget_, p_load->fitnessTarget_, "fitnessTarget_", "p_load->fitnessTarget_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GMetaOptimizerIndividual", iterationThreshold_, p_load->iterationThreshold_, "iterationThreshold_", "p_load->iterationThreshold_", e , limit));

   return evaluateDiscrepancies("GMetaOptimizerIndividual", caller, deviations, e);
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void GMetaOptimizerIndividual::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
	, const bool& showOrigin
) {
	std::string comment;

	// Call our parent class'es function
	GParameterSet::addConfigurationOptions(gpb, showOrigin);

	// Add local data
	comment = ""; // Reset the comment string
	comment += "Specifies the number of optimizations performed";
	if(showOrigin) comment += "[GMetaOptimizerIndividual]";
	gpb.registerFileParameter<std::size_t>(
		"nRunsPerOptimization" // The name of the variable
		, GMETAOPT_DEF_NRUNSPEROPT // The default value
		, boost::bind(
			&GMetaOptimizerIndividual::setNRunsPerOptimization
			, this
			, _1
		  )
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

   comment = ""; // Reset the comment string
   comment += "The fitness below which optimization should stop";
   if(showOrigin) comment += "[GMetaOptimizerIndividual]";
   gpb.registerFileParameter<double>(
      "fitnessTarget_" // The name of the variable
      , GMETAOPT_DEF_FITNESSTARGET // The default value
      , boost::bind(
         &GMetaOptimizerIndividual::setFitnessTarget
         , this
         , _1
        )
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = ""; // Reset the comment string
   comment += "The maximum number of iterations per sub-optimization";
   if(showOrigin) comment += "[GMetaOptimizerIndividual]";
   gpb.registerFileParameter<std::size_t>(
      "iterationThreshold" // The name of the variable
      , GMETAOPT_DEF_ITERATIONTHRESHOLD // The default value
      , boost::bind(
         &GMetaOptimizerIndividual::setIterationThreshold
         , this
         , _1
        )
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );
}

/*******************************************************************************************/
/**
 * Allows to specify how many optimizations should be performed for each (sub-)optimization
 */
void GMetaOptimizerIndividual::setNRunsPerOptimization(std::size_t nrpo) {
#ifdef DEBUG
   if(0==nrpo) {
      glogger
      << "In GMetaOptimizerIndividual::setNRunsPerOptimization(): Error!" << std::endl
      << "Requested number of sub-optimizations is 0" << std::endl
      << GEXCEPTION;
   }
#endif

   nRunsPerOptimization_ = nrpo;
}

/*******************************************************************************************/
/**
 * Allows to retrieve the number of optimizations to be performed for each (sub-)optimization
 */
std::size_t GMetaOptimizerIndividual::getNRunsPerOptimization() const {
   return nRunsPerOptimization_;
}

/******************************************************************************/
/**
 * Allows to set the fitness target for each optimization
 */
void GMetaOptimizerIndividual::setFitnessTarget(double fitnessTarget) {
   fitnessTarget_ = fitnessTarget;
}

/******************************************************************************/
/**
 * Retrieves the fitness target for each optimization
 */
double GMetaOptimizerIndividual::getFitnessTarget() const {
   return fitnessTarget_;
}

/******************************************************************************/
/**
 * Allows to set the iteration threshold
 */
void GMetaOptimizerIndividual::setIterationThreshold(boost::uint32_t iterationThreshold) {
   iterationThreshold_ = iterationThreshold;
}

/******************************************************************************/
/**
 * Allows to retrieve the iteration threshold
 */
boost::uint32_t GMetaOptimizerIndividual::getIterationThreshold() const {
   return iterationThreshold_;
}

/******************************************************************************/
/**
 * Retrieves the current number of parents. Needed for the optimization monitor.
 */
std::size_t GMetaOptimizerIndividual::getNParents() const {
   boost::shared_ptr<GConstrainedInt32Object>  npar_ptr = this->at<GConstrainedInt32Object>(0);
   return npar_ptr->value();
}

/******************************************************************************/
/**
 * Retrieves the current number of children. Needed for the optimization monitor.
 */
std::size_t GMetaOptimizerIndividual::getNChildren() const {
   boost::shared_ptr<GConstrainedInt32Object>  nch_ptr = this->at<GConstrainedInt32Object>(1);
   return nch_ptr->value();
}

/******************************************************************************/
/**
 * Retrieves the adaption probability. Needed for the optimization monitor.
 */
double GMetaOptimizerIndividual::getAdProb() const {
   boost::shared_ptr<GConstrainedDoubleObject> adprob_ptr = this->at<GConstrainedDoubleObject>(2);
   return adprob_ptr->value();
}

/******************************************************************************/
/**
 * Retrieves the lower sigma boundary. Needed for the optimization monitor.
 */
double GMetaOptimizerIndividual::getMinSigma() const {
   boost::shared_ptr<GConstrainedDoubleObject> minsigma_ptr = this->at<GConstrainedDoubleObject>(3);
   return minsigma_ptr->value();
}

/******************************************************************************/
/**
 * Retrieves the sigma range. Needed for the optimization monitor.
 */
double GMetaOptimizerIndividual::getSigmaRange() const {
   boost::shared_ptr<GConstrainedDoubleObject> sigmarange_ptr = this->at<GConstrainedDoubleObject>(4);
   return sigmarange_ptr->value();
}

/******************************************************************************/
/**
 * Retrieves the sigma-sigma parameter. Needed for the optimization monitor.
 */
double GMetaOptimizerIndividual::getSigmaSigma() const {
   boost::shared_ptr<GConstrainedDoubleObject> sigmasigma_ptr = this->at<GConstrainedDoubleObject>(5);
   return sigmasigma_ptr->value();
}

/******************************************************************************/
/**
 * Emit information about this individual
 */
std::string GMetaOptimizerIndividual::print() const {
   std::ostringstream result;

   // Retrieve the parameters
   boost::shared_ptr<GConstrainedInt32Object>  npar_ptr       = this->at<GConstrainedInt32Object>(0);
   boost::shared_ptr<GConstrainedInt32Object>  nch_ptr        = this->at<GConstrainedInt32Object>(1);
   boost::shared_ptr<GConstrainedDoubleObject> adprob_ptr     = this->at<GConstrainedDoubleObject>(2);
   boost::shared_ptr<GConstrainedDoubleObject> minsigma_ptr   = this->at<GConstrainedDoubleObject>(3);
   boost::shared_ptr<GConstrainedDoubleObject> sigmarange_ptr = this->at<GConstrainedDoubleObject>(4);
   boost::shared_ptr<GConstrainedDoubleObject> sigmasigma_ptr = this->at<GConstrainedDoubleObject>(5);

   // Stream the results
   bool tmpDirtyFlag = false;
   result
      << "Fitness = " << this->getCachedFitness(tmpDirtyFlag, 0) << (tmpDirtyFlag?"// dirty flag set":"") << std::endl
      << "#Parents = " << npar_ptr->value() << std::endl
      << "#Children = " << nch_ptr->value() << std::endl
      << "adaption probability = " << adprob_ptr->value() << std::endl
      << "minimum sigma = " << minsigma_ptr->value() << std::endl
      << "maximum sigma = " << minsigma_ptr->value() + sigmarange_ptr->value() << std::endl
      << "sigma-sigma = " << sigmasigma_ptr->value() << std::endl;

   return result.str();
}

/******************************************************************************/
/**
 * Loads the data of another GMetaOptimizerIndividual, camouflaged as a GObject
 *
 * @param cp A copy of another GMetaOptimizerIndividual, camouflaged as a GObject
 */
void GMetaOptimizerIndividual::load_(const GObject* cp){
	// Check that we are indeed dealing with an object of the same type and that we are not
	// accidently trying to compare this object with itself.
	const GMetaOptimizerIndividual *p_load = gobject_conversion<GMetaOptimizerIndividual>(cp);

	// Load our parent class'es data ...
	GParameterSet::load_(cp);

	// ... and then our local data
	nRunsPerOptimization_ = p_load->nRunsPerOptimization_;
	fitnessTarget_ = p_load->fitnessTarget_;
	iterationThreshold_ = p_load->iterationThreshold_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
GObject* GMetaOptimizerIndividual::clone_() const {
	return new GMetaOptimizerIndividual(*this);
}

/******************************************************************************/
/**
 * The actual value calculation takes place here
 *
 * @param The id of the target function (ignored here)
 * @return The value of this object, as calculated with the evaluation function
 */
double GMetaOptimizerIndividual::fitnessCalculation(){
	// Retrieve the parameters
   boost::shared_ptr<GConstrainedInt32Object>  npar_ptr       = this->at<GConstrainedInt32Object>(0);
   boost::shared_ptr<GConstrainedInt32Object>  nch_ptr        = this->at<GConstrainedInt32Object>(1);
   boost::shared_ptr<GConstrainedDoubleObject> adprob_ptr     = this->at<GConstrainedDoubleObject>(2);
   boost::shared_ptr<GConstrainedDoubleObject> minsigma_ptr   = this->at<GConstrainedDoubleObject>(3);
   boost::shared_ptr<GConstrainedDoubleObject> sigmarange_ptr = this->at<GConstrainedDoubleObject>(4);
   boost::shared_ptr<GConstrainedDoubleObject> sigmasigma_ptr = this->at<GConstrainedDoubleObject>(5);

   // Create a factory for GFunctionIndividual objects and perform
   // any necessary initial work.
   GFunctionIndividualFactory gfi("./config/GFunctionIndividual.json");
   // Set the parameters
   double minSigma = minsigma_ptr->value();
   double sigmaRange = sigmarange_ptr->value();
   double maxSigma = minSigma + sigmaRange;

   gfi.setAdProb(adprob_ptr->value());
   gfi.setSigma1Range(boost::tuple<double,double>(minSigma, maxSigma));
   gfi.setSigma1(minSigma + 0.5*sigmaRange); // Use a defined sigma
   gfi.setSigmaSigma1(sigmasigma_ptr->value());

   // Set up a population factory
   GEvolutionaryAlgorithmFactory ea("./config/GSubEvolutionaryAlgorithm.json", EXECMODE_MULTITHREADED);

   // Run the required number of optimizations
   boost::shared_ptr<GBaseEA> ea_ptr;
   boost::uint32_t nChildren = boost::numeric_cast<boost::uint32_t>(nch_ptr->value());
   boost::uint32_t nParents = boost::numeric_cast<boost::uint32_t>(npar_ptr->value());
   boost::uint32_t popSize = nParents + nChildren;
   boost::uint32_t iterationsConsumed = 0;

   std::vector<double> solverCallsPerOptimization;
   std::vector<double> iterationsPerOptimization;

   std::cout << "============================================" << std::endl;
   for(std::size_t opt=0; opt<nRunsPerOptimization_; opt++) {
      ea_ptr = ea.get<GBaseEA>();

      assert(0 == ea_ptr->getIteration());

      // Set the population parameters
      ea_ptr->setDefaultPopulationSize(popSize, nParents);

      // Add the required number of individuals
      for(std::size_t ind=0; ind<popSize; ind++) {
         ea_ptr->push_back(gfi());
      }

      // Set the stop criteria (either maxIterations_ iterations or falling below the quality threshold
      ea_ptr->setQualityThreshold(fitnessTarget_);
      ea_ptr->setMaxIteration(iterationThreshold_);

      // Make sure the optimization is quiet
      ea_ptr->setReportIteration(0);

      // Run the actual optimization
      ea_ptr->optimize();

      // Retrieve the number of iterations
      iterationsConsumed = ea_ptr->getIteration();
      solverCallsPerOptimization.push_back(double((iterationsConsumed+1)*nChildren + nParents));
      iterationsPerOptimization.push_back(double(iterationsConsumed+1));
   }

	// Calculate the average number of iterations
   boost::tuple<double,double> sd = Gem::Common::GStandardDeviation(solverCallsPerOptimization);
   boost::tuple<double,double> itmean = Gem::Common::GStandardDeviation(iterationsPerOptimization);

   // Emit some information
   std::cout << std::endl
         << *this << std::endl
         << boost::get<0>(sd) << " +/- " << boost::get<1>(sd) << " solver calls with " <<  boost::get<0>(itmean) << " average iterations" << std::endl << std::endl;

   // Let the audience know
   return boost::get<0>(sd);
}

#ifdef GEM_TESTING

/******************************************************************************/
/**
 * Applies modifications to this object.
 *
 * @return A boolean indicating whether
 */
bool GMetaOptimizerIndividual::modify_GUnitTests() {
   using boost::unit_test_framework::test_suite;
   using boost::unit_test_framework::test_case;

   bool result = false;

   // Call the parent classes' functions
   if(Gem::Geneva::GParameterSet::modify_GUnitTests()) result = true;

   // Change the parameter settings
   if(!this->empty()) {
      this->adapt();
      result = true;
   }

   // Let the audience know whether we have changed the content
   return result;
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed.
 */
void GMetaOptimizerIndividual::specificTestsNoFailureExpected_GUnitTests() {
   using namespace Gem::Geneva;

   using boost::unit_test_framework::test_suite;
   using boost::unit_test_framework::test_case;

   // Call the parent classes' functions
   Gem::Geneva::GParameterSet::specificTestsNoFailureExpected_GUnitTests();

   //------------------------------------------------------------------------------

   {
      /* nothing. Add test cases here that are expected to succeed. */
   }

   //------------------------------------------------------------------------------
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail.
 */
void GMetaOptimizerIndividual::specificTestsFailuresExpected_GUnitTests() {
   using namespace Gem::Geneva;

   using boost::unit_test_framework::test_suite;
   using boost::unit_test_framework::test_case;

   // Call the parent classes' functions
   Gem::Geneva::GParameterSet::specificTestsFailuresExpected_GUnitTests();

   //------------------------------------------------------------------------------

   {
      /* Nothing. Add test cases here that are expected to fail.
         Enclose with a BOOST_CHECK_THROW, using the expected
         exception type as an additional argument. See the
         documentation for the Boost.Test library for further
         information */
   }

   //------------------------------------------------------------------------------
}

#else /* GEM_TESTING */

/******************************************************************************/
/**
 * Applies modifications to this object. This is function is a trap, as it
 * should not be called if GEM_TESTING isn't set. However, its existence is
 * mandatory, as otherwise this class will have a different API depending on
 * whether GEM_TESTING is set or not.
 */
bool GMetaOptimizerIndividual::modify_GUnitTests() {
   raiseException(
      "In GMetaOptimizerIndividual::modify_GUnitTests(): Error!" << std::endl
      << "Function was called even though GEM_TESTING hasn't been set." << std::endl
   );

   // Make the compiler happy
   return true;
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is function is a trap,
 * as it should not be called if GEM_TESTING isn't set. However, its existence is
 * mandatory, as otherwise this class will have a different API depending on
 * whether GEM_TESTING is set or not.
 */
void GMetaOptimizerIndividual::specificTestsNoFailureExpected_GUnitTests() {
   raiseException(
      "In GMetaOptimizerIndividual::specificTestsNoFailureExpected_GUnitTests(): Error!" << std::endl
      << "Function was called even though GEM_TESTING hasn't been set." << std::endl
   );
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is function is a trap, as
 * it should not be called if GEM_TESTING isn't set. However, its existence is
 * mandatory, as otherwise this class will have a different API depending on
 * whether GEM_TESTING is set or not.
 */
void GMetaOptimizerIndividual::specificTestsFailuresExpected_GUnitTests() {
   raiseException(
      "In GMetaOptimizerIndividual::specificTestsFailuresExpected_GUnitTests(): Error!" << std::endl
      << "Function was called even though GEM_TESTING hasn't been set." << std::endl
   );
}

#endif /* GEM_TESTING */

/******************************************************************************/
/**
 * Allows to output a GMetaOptimizerIndividual or convert it to a string using
 * boost::lexical_cast
 */
std::ostream& operator<<(std::ostream& stream, const GMetaOptimizerIndividual& gsi) {
   stream << gsi.print();
   return stream;
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
GMetaOptimizerIndividualFactory::GMetaOptimizerIndividualFactory(const std::string& configFile)
	: Gem::Common::GFactoryT<GParameterSet>(configFile)
	, initSubNParents_(GMETAOPT_DEF_INITSUBNPARENTS)
	, subNParentsLB_(GMETAOPT_DEF_SUBNPARENTSLB)
	, subNParentsUB_(GMETAOPT_DEF_SUBNPARENTSUB)
	, initSubNChildren_(GMETAOPT_DEF_INITSUBNCHILDREN)
	, subNChildrenLB_(GMETAOPT_DEF_SUBNCHILDRENLB)
	, subNChildrenUB_(GMETAOPT_DEF_SUBNCHILDRENUB)
	, initSubAdProb_(GMETAOPT_DEF_INITSUBADPROB)
	, subAdProbLB_(GMETAOPT_DEF_SUBADPROBLB)
	, subAdProbUB_(GMETAOPT_DEF_SUBADPROBUB)
	, initSubMinSigma_(GMETAOPT_DEF_INITSUBMINSIGMA)
	, subMinSigmaLB_(GMETAOPT_DEF_SUBMINSIGMALB)
	, subMinSigmaUB_(GMETAOPT_DEF_SUBMINSIGMAUB)
	, initSubSigmaRange_(GMETAOPT_DEF_INITSUBSIGMARANGE)
	, subSigmaRangeLB_(GMETAOPT_DEF_SUBSIGMARANGELB)
	, subSigmaRangeUB_(GMETAOPT_DEF_SUBSIGMARANGEUB)
	, initSubSigmaSigma_(GMETAOPT_DEF_INITSUBSIGMASIGMA)
	, subSigmaSigmaLB_(GMETAOPT_DEF_SUBSIGMASIGMALB)
	, subSigmaSigmaUB_(GMETAOPT_DEF_SUBSIGMASIGMAUB)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GMetaOptimizerIndividualFactory::~GMetaOptimizerIndividualFactory()
{ /* nothing */ }

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
boost::shared_ptr<GParameterSet> GMetaOptimizerIndividualFactory::getObject_(
	Gem::Common::GParserBuilder& gpb
	, const std::size_t& id
) {
	// Will hold the result
	boost::shared_ptr<GMetaOptimizerIndividual> target(new GMetaOptimizerIndividual());

	// Make the object's local configuration options known
	target->addConfigurationOptions(gpb, true);

	return target;
}

/******************************************************************************/
/**
 * Allows to describe local configuration options for gradient descents
 */
void GMetaOptimizerIndividualFactory::describeLocalOptions_(Gem::Common::GParserBuilder& gpb) {
	// Describe our own options
	using namespace Gem::Courtier;

	std::string comment;

   comment = "";
   comment += "The initial number of parents in a population;";
   gpb.registerFileParameter<std::size_t>(
      "initSubNParents"
      , initSubNParents_
      , GMETAOPT_DEF_INITSUBNPARENTS
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The lower boundary for variations of the number of parents;";
   gpb.registerFileParameter<std::size_t>(
      "subNParentsLB"
      , subNParentsLB_
      , GMETAOPT_DEF_SUBNPARENTSLB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The upper boundary for variations of the number of parents;";
   gpb.registerFileParameter<std::size_t>(
      "subNParentsUB"
      , subNParentsUB_
      , GMETAOPT_DEF_SUBNPARENTSUB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The initial number of children in a population;";
   gpb.registerFileParameter<std::size_t>(
      "initSubNChildren"
      , initSubNChildren_
      , GMETAOPT_DEF_INITSUBNCHILDREN
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The lower boundary for the variation of the number of children;";
   gpb.registerFileParameter<std::size_t>(
      "subNChildrenLB"
      , subNChildrenLB_
      , GMETAOPT_DEF_SUBNCHILDRENLB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The upper boundary for the variation of the number of children;";
   gpb.registerFileParameter<std::size_t>(
      "subNChildrenUB"
      , subNChildrenUB_
      , GMETAOPT_DEF_SUBNCHILDRENUB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

	comment = "";
	comment += "The initial probability for random adaptions of values in evolutionary algorithms;";
	gpb.registerFileParameter<double>(
		"initSubAdProb"
		, initSubAdProb_
		, GMETAOPT_DEF_INITSUBADPROB
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

   comment = "";
   comment += "The lower boundary for the variation of the adaption probability;";
   gpb.registerFileParameter<double>(
      "subAdProbLB"
      , subAdProbLB_
      , GMETAOPT_DEF_SUBADPROBLB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The upper boundary for the variation of the adaption probability;";
   gpb.registerFileParameter<double>(
      "subAdProbUB"
      , subAdProbUB_
      , GMETAOPT_DEF_SUBADPROBUB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

	comment = "";
	comment += "The initial minimum sigma for gauss-adaption in ES;";
	gpb.registerFileParameter<double>(
		"initSubMinSigma"
		, initSubMinSigma_
		, GMETAOPT_DEF_INITSUBMINSIGMA
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

   comment = "";
   comment += "The lower boundary for the variation of the lower boundary of sigma;";
   gpb.registerFileParameter<double>(
      "subMinSigmaLB"
      , subMinSigmaLB_
      , GMETAOPT_DEF_SUBMINSIGMALB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The upper boundary for the variation of the lower boundary of sigma;";
   gpb.registerFileParameter<double>(
      "subMinSigmaUB"
      , subMinSigmaUB_
      , GMETAOPT_DEF_SUBMINSIGMAUB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The initial maximum range for sigma;";
   gpb.registerFileParameter<double>(
      "initSubSigmaRange"
      , initSubSigmaRange_
      , GMETAOPT_DEF_INITSUBSIGMARANGE
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The lower boundary for the variation of the maximum range of sigma;";
   gpb.registerFileParameter<double>(
      "subSigmaRangeLB"
      , subSigmaRangeLB_
      , GMETAOPT_DEF_SUBSIGMARANGELB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The upper boundary for the variation of the maximum range of sigma;";
   gpb.registerFileParameter<double>(
      "subSigmaRangeUB"
      , subSigmaRangeUB_
      , GMETAOPT_DEF_SUBSIGMARANGEUB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

	comment = "";
	comment += "The initial strength of self-adaption of gauss-mutation in ES;";
	gpb.registerFileParameter<double>(
		"initSubSigmaSigma"
		, initSubSigmaSigma_
		, GMETAOPT_DEF_INITSUBSIGMASIGMA
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

   comment = "";
   comment += "The lower boundary for the variation of the strength of sigma adaption;";
   gpb.registerFileParameter<double>(
      "subSigmaSigmaLB"
      , subSigmaSigmaLB_
      , GMETAOPT_DEF_SUBSIGMASIGMALB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The upper boundary for the variation of the strength of sigma adaption;";
   gpb.registerFileParameter<double>(
      "subSigmaSigmaUB"
      , subSigmaSigmaUB_
      , GMETAOPT_DEF_SUBSIGMASIGMAUB
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
 * we will usually add the parameter objects here. Note that a very similar constructor
 * exists for GMetaOptimizerIndividual, so it may be used independently of the factory.
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void GMetaOptimizerIndividualFactory::postProcess_(boost::shared_ptr<GParameterSet>& p_base) {
   // Convert the base pointer to our local type
   boost::shared_ptr<GMetaOptimizerIndividual> p
      = Gem::Common::convertSmartPointer<GParameterSet, GMetaOptimizerIndividual>(p_base);

   // We simply use a static function defined in GMetaOptimizerIndividual
   GMetaOptimizerIndividual::addContent(
         p
         , initSubNParents_
         , subNParentsLB_
         , subNParentsUB_
         , initSubNChildren_
         , subNChildrenLB_
         , subNChildrenUB_
         , initSubAdProb_
         , subAdProbLB_
         , subAdProbUB_
         , initSubMinSigma_
         , subMinSigmaLB_
         , subMinSigmaUB_
         , initSubSigmaRange_
         , subSigmaRangeLB_
         , subSigmaRangeUB_
         , initSubSigmaSigma_
         , subSigmaSigmaLB_
         , subSigmaSigmaUB_
   );
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
