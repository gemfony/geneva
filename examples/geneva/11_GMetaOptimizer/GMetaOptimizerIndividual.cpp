/**
 * @file GMetaOptimizerIndividual.cpp
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

#include "GMetaOptimizerIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GMetaOptimizerIndividual)

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Puts a Gem::Geneva::metaOptimizationTarget item into a stream
 *
 * @param o The ostream the item should be added to
 * @param mot the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const Gem::Geneva::metaOptimizationTarget& mot) {
   boost::uint16_t tmp = static_cast<boost::uint16_t>(mot);
   o << tmp;
   return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::metaOptimizationTarget item from a stream
 *
 * @param i The stream the item should be read from
 * @param mot The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, Gem::Geneva::metaOptimizationTarget& mot) {
   boost::uint16_t tmp;
   i >> tmp;

#ifdef DEBUG
   mot = boost::numeric_cast<Gem::Geneva::metaOptimizationTarget>(tmp);
#else
   mot = static_cast<Gem::Geneva::metaOptimizationTarget>(tmp);
#endif /* DEBUG */

   return i;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor.
 */
GMetaOptimizerIndividual::GMetaOptimizerIndividual()
   : GParameterSet()
   , nRunsPerOptimization_(GMETAOPT_DEF_NRUNSPEROPT)
   , fitnessTarget_(GMETAOPT_DEF_FITNESSTARGET)
   , iterationThreshold_(GMETAOPT_DEF_ITERATIONTHRESHOLD)
   , moTarget_(GMETAOPT_DEF_MOTARGET)
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
   , moTarget_(cp.moTarget_)
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
   const Gem::Common::expectation& e
   , const double& limit
   , const std::string& caller
   , const std::string& y_name
   , const bool& withMessages
) const {
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
   deviations.push_back(checkExpectation(withMessages, "GMetaOptimizerIndividual", moTarget_, p_load->moTarget_, "moTarget_", "p_load->moTarget_", e , limit));

   return evaluateDiscrepancies("GMetaOptimizerIndividual", caller, deviations, e);
}

/******************************************************************************/
/**
 * Allows to set the desired target of the meta-optimization
 */
void GMetaOptimizerIndividual::setMetaOptimizationTarget(metaOptimizationTarget moTarget) {
   moTarget_ = moTarget;

   if(MC_MINSOLVER_BESTFITNESS == moTarget_) { // multi-criterion optimization. We need to set the number of fitness criteria
      this->setNumberOfFitnessCriteria(2);
   }
}

/******************************************************************************/
/**
 * Allows to retrieve the current target of the meta-optimization
 */
metaOptimizationTarget GMetaOptimizerIndividual::getMetaOptimizationTarget() const {
   return moTarget_;
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

   comment = ""; // Reset the comment string
   comment += "The target for the meta-optimization: best fitness (0),;";
   comment += "minimum number of solver calls (1), multi-criterion with best fitness;";
   comment += "and smallest number of solver calls as target (2);";
   if(showOrigin) comment += "[GMetaOptimizerIndividual]";
   gpb.registerFileParameter<metaOptimizationTarget>(
      "metaOptimizationTarget" // The name of the variable
      , GMETAOPT_DEF_MOTARGET // The default value
      , boost::bind(
         &GMetaOptimizerIndividual::setMetaOptimizationTarget
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
   boost::shared_ptr<GConstrainedInt32Object>  npar_ptr = this->at<GConstrainedInt32Object>(MOT_NPARENTS);
   return npar_ptr->value();
}

/******************************************************************************/
/**
 * Retrieves the current number of children. Needed for the optimization monitor.
 */
std::size_t GMetaOptimizerIndividual::getNChildren() const {
   boost::shared_ptr<GConstrainedInt32Object>  nch_ptr = this->at<GConstrainedInt32Object>(MOT_NCHILDREN);
   return nch_ptr->value();
}

/******************************************************************************/
/**
 * Retrieves the adaption probability. Needed for the optimization monitor.
 */
double GMetaOptimizerIndividual::getAdProb() const {
   boost::shared_ptr<GConstrainedDoubleObject> minAdProb_ptr             = this->at<GConstrainedDoubleObject>(MOT_MINADPROB);
   boost::shared_ptr<GConstrainedDoubleObject> adProbRange_ptr           = this->at<GConstrainedDoubleObject>(MOT_ADPROBRANGE);
   boost::shared_ptr<GConstrainedDoubleObject> adProbStartPercentage_ptr = this->at<GConstrainedDoubleObject>(MOT_ADPROBSTARTPERCENTAGE);

   return minAdProb_ptr->value() + adProbStartPercentage_ptr->value() * adProbRange_ptr->value();
}

/******************************************************************************/
/**
 * Retrieves the lower sigma boundary. Needed for the optimization monitor.
 */
double GMetaOptimizerIndividual::getMinSigma() const {
   boost::shared_ptr<GConstrainedDoubleObject> minsigma_ptr              = this->at<GConstrainedDoubleObject>(MOT_MINSIGMA);
   return minsigma_ptr->value();
}

/******************************************************************************/
/**
 * Retrieves the sigma range. Needed for the optimization monitor.
 */
double GMetaOptimizerIndividual::getSigmaRange() const {
   boost::shared_ptr<GConstrainedDoubleObject> sigmarange_ptr            = this->at<GConstrainedDoubleObject>(MOT_SIGMARANGE);
   return sigmarange_ptr->value();
}

/******************************************************************************/
/**
 * Retrieves the sigma-sigma parameter. Needed for the optimization monitor.
 */
double GMetaOptimizerIndividual::getSigmaSigma() const {
   boost::shared_ptr<GConstrainedDoubleObject> sigmasigma_ptr            = this->at<GConstrainedDoubleObject>(MOT_SIGMASIGMA);
   return sigmasigma_ptr->value();
}

/******************************************************************************/
/**
 * Retrieves a clear-text description of the optimization target
 */
std::string GMetaOptimizerIndividual::getClearTextMOT(const metaOptimizationTarget& mot) const {
   switch(mot) {
      case BESTFITNESS:
      return std::string("\"best fitness\"");
      break;

      case MINSOLVERCALLS:
      return std::string("\"minimum number of solver calls\"");
      break;

      case MC_MINSOLVER_BESTFITNESS:
      return std::string("\"multi-criterion target with best fitness, minimum number of solver calls\"");
      break;
   }

   // Make the compiler happy
   return std::string();
}

/******************************************************************************/
/**
 * Emit information about this individual
 */
std::string GMetaOptimizerIndividual::print(bool withFitness) const {
   std::ostringstream result;

   // Retrieve the parameters
   boost::shared_ptr<GConstrainedInt32Object>  npar_ptr                  = this->at<GConstrainedInt32Object> (MOT_NPARENTS);
   boost::shared_ptr<GConstrainedInt32Object>  nch_ptr                   = this->at<GConstrainedInt32Object> (MOT_NCHILDREN);
   boost::shared_ptr<GConstrainedDoubleObject> amalgamation_ptr          = this->at<GConstrainedDoubleObject>(MOT_AMALGAMATION);
   boost::shared_ptr<GConstrainedDoubleObject> minAdProb_ptr             = this->at<GConstrainedDoubleObject>(MOT_MINADPROB);
   boost::shared_ptr<GConstrainedDoubleObject> adProbRange_ptr           = this->at<GConstrainedDoubleObject>(MOT_ADPROBRANGE);
   boost::shared_ptr<GConstrainedDoubleObject> adProbStartPercentage_ptr = this->at<GConstrainedDoubleObject>(MOT_ADPROBSTARTPERCENTAGE);
   boost::shared_ptr<GConstrainedDoubleObject> adaptAdprob_ptr           = this->at<GConstrainedDoubleObject>(MOT_ADAPTADPROB);
   boost::shared_ptr<GConstrainedDoubleObject> minsigma_ptr              = this->at<GConstrainedDoubleObject>(MOT_MINSIGMA);
   boost::shared_ptr<GConstrainedDoubleObject> sigmarange_ptr            = this->at<GConstrainedDoubleObject>(MOT_SIGMARANGE);
   boost::shared_ptr<GConstrainedDoubleObject> sigmaRangePercentage_ptr  = this->at<GConstrainedDoubleObject>(MOT_SIGMARANGEPERCENTAGE);
   boost::shared_ptr<GConstrainedDoubleObject> sigmasigma_ptr            = this->at<GConstrainedDoubleObject>(MOT_SIGMASIGMA);
   boost::shared_ptr<GConstrainedDoubleObject> crossOverProb_ptr         = this->at<GConstrainedDoubleObject>(MOT_CROSSOVERPROB);


   // Stream the results

   bool dirtyFlag = this->isDirty();
   double transformedPrimaryFitness = dirtyFlag?this->getWorstCase():this->transformedFitness();

   result
   << "============================================================================================" << std::endl;

   if(withFitness) {
      result << "Fitness = " << transformedPrimaryFitness << (dirtyFlag?" // dirty flag set":"") << std::endl;
   }

   result
   << "Optimization target: " << getClearTextMOT(moTarget_) << std::endl
   << std::endl
   << "population::size = " << npar_ptr->value() + nch_ptr->value() << std::endl
   << "population::nParents = " << npar_ptr->value() << std::endl
   << "population::amalgamationLikelihood = " << amalgamation_ptr->value() << std::endl
   << "individual::adProb = " << minAdProb_ptr->value() + adProbRange_ptr->value()*adProbRange_ptr->value() << std::endl
   << "individual::minAdProb = " << minAdProb_ptr->value() << std::endl
   << "individual::maxAdProb = " << minAdProb_ptr->value() + adProbRange_ptr->value() << std::endl
   << "individual::adaptAdProb = " << adaptAdprob_ptr->value() << std::endl
   << "individual::sigma1 = " << minsigma_ptr->value() + sigmarange_ptr->value()*sigmaRangePercentage_ptr->value() << std::endl
   << "individual::minSigma1 = " << minsigma_ptr->value() << std::endl
   << "individual::maxSigma1 = " << minsigma_ptr->value() + sigmarange_ptr->value() << std::endl
   << "individual::sigmaSigma1 = " << sigmasigma_ptr->value() << std::endl
   << "individual::perItemCrossOverProbability = " << crossOverProb_ptr->value() << std::endl
   << "============================================================================================" << std::endl
   << std::endl;

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
	moTarget_ = p_load->moTarget_;
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
double GMetaOptimizerIndividual::fitnessCalculation() {
   bool first = true;
   bool maxMode = false;

	// Retrieve the parameters
   boost::shared_ptr<GConstrainedInt32Object>  npar_ptr                  = this->at<GConstrainedInt32Object> (MOT_NPARENTS);
   boost::shared_ptr<GConstrainedInt32Object>  nch_ptr                   = this->at<GConstrainedInt32Object> (MOT_NCHILDREN);
   boost::shared_ptr<GConstrainedDoubleObject> amalgamation_ptr          = this->at<GConstrainedDoubleObject>(MOT_AMALGAMATION);
   boost::shared_ptr<GConstrainedDoubleObject> minAdProb_ptr             = this->at<GConstrainedDoubleObject>(MOT_MINADPROB);
   boost::shared_ptr<GConstrainedDoubleObject> adProbRange_ptr           = this->at<GConstrainedDoubleObject>(MOT_ADPROBRANGE);
   boost::shared_ptr<GConstrainedDoubleObject> adProbStartPercentage_ptr = this->at<GConstrainedDoubleObject>(MOT_ADPROBSTARTPERCENTAGE);
   boost::shared_ptr<GConstrainedDoubleObject> adaptAdprob_ptr           = this->at<GConstrainedDoubleObject>(MOT_ADAPTADPROB);
   boost::shared_ptr<GConstrainedDoubleObject> minsigma_ptr              = this->at<GConstrainedDoubleObject>(MOT_MINSIGMA);
   boost::shared_ptr<GConstrainedDoubleObject> sigmarange_ptr            = this->at<GConstrainedDoubleObject>(MOT_SIGMARANGE);
   boost::shared_ptr<GConstrainedDoubleObject> sigmaRangePercentage_ptr  = this->at<GConstrainedDoubleObject>(MOT_SIGMARANGEPERCENTAGE);
   boost::shared_ptr<GConstrainedDoubleObject> sigmasigma_ptr            = this->at<GConstrainedDoubleObject>(MOT_SIGMASIGMA);
   boost::shared_ptr<GConstrainedDoubleObject> crossOverProb_ptr         = this->at<GConstrainedDoubleObject>(MOT_CROSSOVERPROB);

   // Create a factory for GFunctionIndividual objects and perform
   // any necessary initial work.
   GFunctionIndividualFactory gfi("./config/GFunctionIndividual.json");
   // Set the parameters
   double minSigma = minsigma_ptr->value();
   double sigmaRange = sigmarange_ptr->value();
   double maxSigma = minSigma + sigmaRange;
   double sigmaRangePercentage = sigmaRangePercentage_ptr->value();
   double startSigma = minSigma + sigmaRangePercentage*sigmaRange;

   gfi.setSigma1Range(boost::tuple<double,double>(minSigma, maxSigma));
   gfi.setSigma1(startSigma);
   gfi.setSigmaSigma1(sigmasigma_ptr->value());

   double minAdProb = minAdProb_ptr->value();
   double adProbRange = adProbRange_ptr->value();
   double maxAdProb = minAdProb + adProbRange;
   double adProbStartPercentage = adProbStartPercentage_ptr->value();
   double startAdProb = minAdProb + adProbStartPercentage*adProbRange;

   double adaptAdProb = adaptAdprob_ptr->value();

   gfi.setAdProbRange(minAdProb, maxAdProb);
   gfi.setAdProb(startAdProb);
   gfi.setAdaptAdProb(adaptAdProb);

   // Set up a population factory
   GEvolutionaryAlgorithmFactory ea("./config/GSubEvolutionaryAlgorithm.json", EXECMODE_SERIAL);

   // Run the required number of optimizations
   boost::shared_ptr<GBaseEA> ea_ptr;

   boost::uint32_t nChildren = boost::numeric_cast<boost::uint32_t>(nch_ptr->value());
   boost::uint32_t nParents = boost::numeric_cast<boost::uint32_t>(npar_ptr->value());
   boost::uint32_t popSize = nParents + nChildren;
   boost::uint32_t iterationsConsumed = 0;
   double amalgamationLikelihood = amalgamation_ptr->value();

   std::vector<double> solverCallsPerOptimization;
   std::vector<double> iterationsPerOptimization;
   std::vector<double> bestEvaluations;

   for(std::size_t opt=0; opt<nRunsPerOptimization_; opt++) {
      std::cout << "Starting measurement " << opt+1 << " / " << nRunsPerOptimization_ << std::endl;
      ea_ptr = ea.get<GBaseEA>();

      assert(0 == ea_ptr->getIteration());

      // Set the population parameters
      ea_ptr->setPopulationSizes(popSize, nParents);

      // Add the required number of individuals
      for(std::size_t ind=0; ind<popSize; ind++) {
         // Retrieve an individual
         boost::shared_ptr<GParameterSet> gfi_ptr = gfi();

         // Find out whether this is a maximization or minimization once per call to fitnessCalculation
         if(first) {
            maxMode = gfi_ptr->getMaxMode();
            first = false;
         }

         // Set the "per item cross-over probability"
         gfi_ptr->setPerItemCrossOverProbability(crossOverProb_ptr->value());

         ea_ptr->push_back(gfi_ptr);
      }

      // Set the likelihood for work items to be produced through cross-over rather than mutation alone
      ea_ptr->setAmalgamationLikelihood(amalgamationLikelihood);

      if(MINSOLVERCALLS == moTarget_) {
         // Set the stop criteria (either maxIterations_ iterations or falling below the quality threshold
         ea_ptr->setQualityThreshold(fitnessTarget_);
         ea_ptr->setMaxIteration(iterationThreshold_);

         // Make sure the optimization does not emit the termination reason
         ea_ptr->setEmitTerminationReason(false);

         // Make sure the optimization does not stop due to stalls (which is the default in the EA-config
         ea_ptr->setMaxStallIteration(0);
      } else  { // Optimization of best fitness found or multi-criterion optimization: BESTFITNESS / MC_MINSOLVER_BESTFITNESS
         // Set the stop criterion maxIterations only
         ea_ptr->setMaxIteration(iterationThreshold_);

         // Make sure the optimization does not emit the termination reason
         ea_ptr->setEmitTerminationReason(false);

         // Set a relatively high stall threshold
         ea_ptr->setMaxStallIteration(50);
      }

      // Make sure the optimization is quiet
      ea_ptr->setReportIteration(0);

      // Run the actual optimization
       ea_ptr->optimize();

       // Retrieve the best individual
       boost::shared_ptr<GParameterSet> bestIndividual = ea_ptr->getBestIndividual<GParameterSet>();

      // Retrieve the number of iterations
      iterationsConsumed = ea_ptr->getIteration();

      // Do book-keeping
      solverCallsPerOptimization.push_back(double((iterationsConsumed+1)*nChildren + nParents));
      iterationsPerOptimization.push_back(double(iterationsConsumed+1));
      bestEvaluations.push_back(bestIndividual->fitness());
   }

	// Calculate the average number of iterations and solver calls
   boost::tuple<double,double> sd = Gem::Common::GStandardDeviation(solverCallsPerOptimization);
   boost::tuple<double,double> itmean = Gem::Common::GStandardDeviation(iterationsPerOptimization);
   boost::tuple<double,double> bestMean = Gem::Common::GStandardDeviation(bestEvaluations); // TODO: Deal with vectors containing max-double

   double evaluation = 0.;
   if(MINSOLVERCALLS == moTarget_) {
      evaluation = boost::get<0>(sd);
   } else if(BESTFITNESS == moTarget_) {
      evaluation = boost::get<0>(bestMean);
   } else if(MC_MINSOLVER_BESTFITNESS == moTarget_) {
      evaluation = boost::get<0>(bestMean); // The primary result
      this->registerSecondaryResult(1, boost::get<0>(sd)); // The secondary result
   }

   // Emit some information
   std::cout
   << std::endl
   << boost::get<0>(sd) << " +/- " << boost::get<1>(sd) << " solver calls with " << std::endl
   << boost::get<0>(itmean) << " +/- " << boost::get<1>(itmean) << " average iterations " << std::endl
   << "and a mean evaluation of " << boost::get<0>(bestMean) << " +/- " << boost::get<1>(bestMean) << std::endl
   << "out of " << nRunsPerOptimization_ << " consecutive runs" << std::endl
   << this->print(false) << std::endl // print without fitness -- not defined at this stage
   << std::endl;

   // Let the audience know
   return evaluation;
}

/******************************************************************************/
/**
 * Applies modifications to this object.
 *
 * @return A boolean indicating whether
 */
bool GMetaOptimizerIndividual::modify_GUnitTests() {
#ifdef GEM_TESTING
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

#else /* GEM_TESTING */
   condnotset("GMetaOptimizerIndividual::modify_GUnitTests()", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed.
 */
void GMetaOptimizerIndividual::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
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
#else /* GEM_TESTING */
   condnotset("GMetaOptimizerIndividual::specificTestsNoFailureExpected_GUnitTests()", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail.
 */
void GMetaOptimizerIndividual::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
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

#else /* GEM_TESTING */
   condnotset("GMetaOptimizerIndividual::specificTestsNoFailureExpected_GUnitTests()", "GEM_TESTING");
#endif /* GEM_TESTING */
}

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
	, initNParents_(GMETAOPT_DEF_INITNPARENTS)
	, nParents_LB_(GMETAOPT_DEF_NPARENTS_LB)
	, nParents_UB_(GMETAOPT_DEF_NPARENTS_UB)
	, initNChildren_(GMETAOPT_DEF_INITNCHILDREN)
	, nChildren_LB_(GMETAOPT_DEF_NCHILDREN_LB)
	, nChildren_UB_(GMETAOPT_DEF_NCHILDREN_UB)
	, initAmalgamationLklh_(GMETAOPT_DEF_INITAMALGLKLHOOD)
	, amalgamationLklh_LB_(GMETAOPT_DEF_AMALGLKLHOOD_LB)
	, amalgamationLklh_UB_(GMETAOPT_DEF_AMALGLKLHOOD_UB)
   , initMinAdProb_(GMETAOPT_DEF_INITMINADPROB)
   , minAdProb_LB_(GMETAOPT_DEF_MINADPROB_LB)
   , minAdProb_UB_(GMETAOPT_DEF_MINADPROB_UB)
   , initAdProbRange_(GMETAOPT_DEF_INITADPROBRANGE)
   , adProbRange_LB_(GMETAOPT_DEF_ADPROBRANGE_LB)
   , adProbRange_UB_(GMETAOPT_DEF_ADPROBRANGE_UB)
   , initAdProbStartPercentage_(GMETAOPT_DEF_INITADPROBSTARTPERCENTAGE)
	, initAdaptAdProb_(GMETAOPT_DEF_INITADAPTADPROB)
	, adaptAdProb_LB_(GMETAOPT_DEF_ADAPTADPROB_LB)
	, adaptAdProb_UB_(GMETAOPT_DEF_ADAPTADPROB_UB)
	, initMinSigma_(GMETAOPT_DEF_INITMINSIGMA)
	, minSigma_LB_(GMETAOPT_DEF_MINSIGMA_LB)
	, minSigma_UB_(GMETAOPT_DEF_MINSIGMA_UB)
	, initSigmaRange_(GMETAOPT_DEF_INITSIGMARANGE)
	, sigmaRange_LB_(GMETAOPT_DEF_SIGMARANGE_LB)
	, sigmaRange_UB_(GMETAOPT_DEF_SIGMARANGE_UB)
	, initSigmaRangePercentage_(GMETAOPT_DEF_INITSIGMARANGEPERCENTAGE)
	, initSigmaSigma_(GMETAOPT_DEF_INITSIGMASIGMA)
	, sigmaSigma_LB_(GMETAOPT_DEF_SIGMASIGMA_LB)
	, sigmaSigma_UB_(GMETAOPT_DEF_SIGMASIGMA_UB)
	, initCrossOverProb_(GMETAOPT_DEF_INITCROSSOVERPROB)
	, crossOverProb_LB_(GMETAOPT_DEF_CROSSOVERPROB_LB)
	, crossOverProb_UB_(GMETAOPT_DEF_CROSSOVERPROB_UB)
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
      "initNParents"
      , initNParents_
      , GMETAOPT_DEF_INITNPARENTS
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The lower boundary for variations of the number of parents;";
   gpb.registerFileParameter<std::size_t>(
      "nParents_LB"
      , nParents_LB_
      , GMETAOPT_DEF_NPARENTS_LB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The upper boundary for variations of the number of parents;";
   gpb.registerFileParameter<std::size_t>(
      "nParents_UB"
      , nParents_UB_
      , GMETAOPT_DEF_NPARENTS_UB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The initial number of children in a population;";
   gpb.registerFileParameter<std::size_t>(
      "initNChildren"
      , initNChildren_
      , GMETAOPT_DEF_INITNCHILDREN
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The lower boundary for the variation of the number of children;";
   gpb.registerFileParameter<std::size_t>(
      "nChildren_LB"
      , nChildren_LB_
      , GMETAOPT_DEF_NCHILDREN_LB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The upper boundary for the variation of the number of children;";
   gpb.registerFileParameter<std::size_t>(
      "nChildren_UB"
      , nChildren_UB_
      , GMETAOPT_DEF_NCHILDREN_UB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The initial likelihood for an individual being created from cross-over rather than just duplication;";
   gpb.registerFileParameter<double>(
      "initAmalgamationLklh"
      , initAmalgamationLklh_
      , GMETAOPT_DEF_INITAMALGLKLHOOD
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The lower boundary for the variation of the amalgamation likelihood ;";
   gpb.registerFileParameter<double>(
      "amalgamationLklh_LB"
      , amalgamationLklh_LB_
      , GMETAOPT_DEF_AMALGLKLHOOD_LB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The upper boundary for the variation of the amalgamation likelihood ;";
   gpb.registerFileParameter<double>(
      "amalgamationLklh_UB"
      , amalgamationLklh_UB_
      , GMETAOPT_DEF_AMALGLKLHOOD_UB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

	comment = "";
	comment += "The initial lower boundary for the variation of adProb;";
	gpb.registerFileParameter<double>(
		"initMinAdProb"
		, initMinAdProb_
		, GMETAOPT_DEF_INITMINADPROB
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

   comment = "";
   comment += "The lower boundary for minAdProb;";
   gpb.registerFileParameter<double>(
      "minAdProb_LB"
      , minAdProb_LB_
      , GMETAOPT_DEF_MINADPROB_LB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The upper boundary for minAdProb;";
   gpb.registerFileParameter<double>(
      "minAdProb_UB"
      , minAdProb_UB_
      , GMETAOPT_DEF_MINADPROB_UB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The initial range for the variation of adProb;";
   gpb.registerFileParameter<double>(
      "initAdProbRange"
      , initAdProbRange_
      , GMETAOPT_DEF_INITADPROBRANGE
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The lower boundary for adProbRange;";
   gpb.registerFileParameter<double>(
      "adProbRange_LB"
      , adProbRange_LB_
      , GMETAOPT_DEF_ADPROBRANGE_LB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The upper boundary for adProbRange;";
   gpb.registerFileParameter<double>(
      "adProbRange_UB"
      , adProbRange_UB_
      , GMETAOPT_DEF_ADPROBRANGE_UB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The start value for adProb relative to the allowed value range;";
   gpb.registerFileParameter<double>(
      "initAdProbStartPercentage"
      , initAdProbStartPercentage_
      , GMETAOPT_DEF_INITADPROBSTARTPERCENTAGE
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The initial value of the strength of adProb_ adaption;";
   gpb.registerFileParameter<double>(
      "initAdaptAdProb"
      , initAdaptAdProb_
      , GMETAOPT_DEF_INITADAPTADPROB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The lower boundary for the variation of the strength of adProb_ adaption;";
   gpb.registerFileParameter<double>(
      "adaptAdProb_LB"
      , adaptAdProb_LB_
      , GMETAOPT_DEF_ADAPTADPROB_LB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The upper boundary for the variation of the strength of adProb_ adaption;";
   gpb.registerFileParameter<double>(
      "adaptAdProb_UB"
      , adaptAdProb_UB_
      , GMETAOPT_DEF_ADAPTADPROB_UB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

	comment = "";
	comment += "The initial minimum sigma for gauss-adaption in ES;";
	gpb.registerFileParameter<double>(
		"initMinSigma"
		, initMinSigma_
		, GMETAOPT_DEF_INITMINSIGMA
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

   comment = "";
   comment += "The lower boundary for the variation of the lower boundary of sigma;";
   gpb.registerFileParameter<double>(
      "minSigma_LB"
      , minSigma_LB_
      , GMETAOPT_DEF_MINSIGMA_LB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The upper boundary for the variation of the lower boundary of sigma;";
   gpb.registerFileParameter<double>(
      "minSigma_UB"
      , minSigma_UB_
      , GMETAOPT_DEF_MINSIGMA_UB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The initial maximum range for sigma;";
   gpb.registerFileParameter<double>(
      "initSigmaRange"
      , initSigmaRange_
      , GMETAOPT_DEF_INITSIGMARANGE
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The lower boundary for the variation of the maximum range of sigma;";
   gpb.registerFileParameter<double>(
      "sigmaRange_LB"
      , sigmaRange_LB_
      , GMETAOPT_DEF_SIGMARANGE_LB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The upper boundary for the variation of the maximum range of sigma;";
   gpb.registerFileParameter<double>(
      "sigmaRange_UB"
      , sigmaRange_UB_
      , GMETAOPT_DEF_SIGMARANGE_UB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The initial percentage of the sigma range as a start value;";
   gpb.registerFileParameter<double>(
      "initSigmaRangePercentage"
      , initSigmaRangePercentage_
      , GMETAOPT_DEF_INITSIGMARANGEPERCENTAGE
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

	comment = "";
	comment += "The initial strength of self-adaption of gauss-mutation in ES;";
	gpb.registerFileParameter<double>(
		"initSigmaSigma"
		, initSigmaSigma_
		, GMETAOPT_DEF_INITSIGMASIGMA
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

   comment = "";
   comment += "The lower boundary for the variation of the strength of sigma adaption;";
   gpb.registerFileParameter<double>(
      "sigmaSigma_LB"
      , sigmaSigma_LB_
      , GMETAOPT_DEF_SIGMASIGMA_LB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The upper boundary for the variation of the strength of sigma adaption;";
   gpb.registerFileParameter<double>(
      "sigmaSigma_UB"
      , sigmaSigma_UB_
      , GMETAOPT_DEF_SIGMASIGMA_UB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The likelihood for two data items to be exchanged in a cross-over operation;";
   gpb.registerFileParameter<double>(
      "initCrossOverProb"
      , initCrossOverProb_
      , GMETAOPT_DEF_INITCROSSOVERPROB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The lower boundary for the variation of the cross-over probability ;";
   gpb.registerFileParameter<double>(
      "crossOverProb_LB"
      , crossOverProb_LB_
      , GMETAOPT_DEF_CROSSOVERPROB_LB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The upper boundary for the variation of the cross-over probability ;";
   gpb.registerFileParameter<double>(
      "crossOverProb_UB"
      , crossOverProb_UB_
      , GMETAOPT_DEF_CROSSOVERPROB_UB
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
      , initNParents_
      , nParents_LB_
      , nParents_UB_
      , initNChildren_
      , nChildren_LB_
      , nChildren_UB_
      , initAmalgamationLklh_
      , amalgamationLklh_LB_
      , amalgamationLklh_UB_
      , initMinAdProb_
      , minAdProb_LB_
      , minAdProb_UB_
      , initAdProbRange_
      , adProbRange_LB_
      , adProbRange_UB_
      , initAdProbStartPercentage_
      , initAdaptAdProb_
      , adaptAdProb_LB_
      , adaptAdProb_UB_
      , initMinSigma_
      , minSigma_LB_
      , minSigma_UB_
      , initSigmaRange_
      , sigmaRange_LB_
      , sigmaRange_UB_
      , initSigmaRangePercentage_
      , initSigmaSigma_
      , sigmaSigma_LB_
      , sigmaSigma_UB_
      , initCrossOverProb_
      , crossOverProb_LB_
      , crossOverProb_UB_
   );
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
