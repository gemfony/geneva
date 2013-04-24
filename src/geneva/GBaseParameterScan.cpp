/**
 * @file GBaseParameterScan.cpp
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

#include "geneva/GBaseParameterScan.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBaseParameterScan::GParameterScanOptimizationMonitor)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GBaseParameterScan::GBaseParameterScan()
   : GOptimizationAlgorithmT<GParameterSet>()
   , cycleLogicHalt_(false)
   , scanRandomly_(true)
{
   // Register the default optimization monitor
   this->registerOptimizationMonitor(
         boost::shared_ptr<GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT>(
               new GParameterScanOptimizationMonitor()
         )
   );
}

/******************************************************************************/
/**
 * A standard copy constructor.
 *
 * @param cp A copy of another GradientDescent object
 */
GBaseParameterScan::GBaseParameterScan(const GBaseParameterScan& cp)
   : GOptimizationAlgorithmT<GParameterSet>(cp)
   , cycleLogicHalt_(cp.cycleLogicHalt_)
   , scanRandomly_(cp.scanRandomly_)
{
   // Copying / setting of the optimization algorithm id is done by the parent class. The same
   // applies to the copying of the optimization monitor.

   // Load the parameter objects
   std::vector<boost::shared_ptr<bScanPar> >::const_iterator b_it;
   for(b_it=cp.bVec_.begin(); b_it!=cp.bVec_.end(); ++b_it) {
      bVec_.push_back((*b_it)->clone());
   }

   std::vector<boost::shared_ptr<int32ScanPar> >::const_iterator i_it;
   for(i_it=cp.int32Vec_.begin(); i_it!=cp.int32Vec_.end(); ++i_it) {
      int32Vec_.push_back((*i_it)->clone());
   }

   std::vector<boost::shared_ptr<dScanPar> >::const_iterator d_it;
   for(d_it=cp.dVec_.begin(); d_it!=cp.dVec_.end(); ++d_it) {
      dVec_.push_back((*d_it)->clone());
   }

   std::vector<boost::shared_ptr<fScanPar> >::const_iterator f_it;
   for(f_it=cp.fVec_.begin(); f_it!=cp.fVec_.end(); ++f_it) {
      fVec_.push_back((*f_it)->clone());
   }
}

/******************************************************************************/
/**
 * The destructor
 */
GBaseParameterScan::~GBaseParameterScan()
{ /* nothing */ }

/******************************************************************************/
/**
 * Returns information about the type of optimization algorithm. This function needs
 * to be overloaded by the actual algorithms to return the correct type.
 *
 * @return The type of optimization algorithm
 */
personality_oa GBaseParameterScan::getOptimizationAlgorithm() const {
   return PERSONALITY_PS;
}

/******************************************************************************/
/**
 * Retrieve the number of processable items in the current iteration.
 *
 * @return The number of processable items in the current iteration
 */
std::size_t GBaseParameterScan::getNProcessableItems() const {
   return this->size(); // Evaluation always needs to be done for the entire population
}

/******************************************************************************/
/**
 * Returns the name of this optimization algorithm
 *
 * @return The name assigned to this optimization algorithm
 */
std::string GBaseParameterScan::getAlgorithmName() const {
   return std::string("Parameter Scan");
}

/******************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another GRadientDescent object
 */
const GBaseParameterScan& GBaseParameterScan::operator=(const GBaseParameterScan& cp) {
   GBaseParameterScan::load_(&cp);
   return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GBaseParameterScan object
 *
 * @param cp A copy of another GRadientDescent object
 */
bool GBaseParameterScan::operator==(const GBaseParameterScan& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBaseParameterScan::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GBaseParameterScan object
 *
 * @param cp A copy of another GRadientDescent object
 */
bool GBaseParameterScan::operator!=(const GBaseParameterScan& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBaseParameterScan::operator!=","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks whether this object fulfills a given expectation in relation to another object
 *
 * @param cp A constant reference to another object, camouflaged as a GObject
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 * @param caller An identifier for the calling entity
 * @param y_name An identifier for the object that should be compared to this one
 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
 */
boost::optional<std::string> GBaseParameterScan::checkRelationshipWith(
      const GObject& cp
      , const Gem::Common::expectation& e
      , const double& limit
      , const std::string& caller
      , const std::string& y_name
      , const bool& withMessages
) const {
    using namespace Gem::Common;

   // Check that we are indeed dealing with a GParamterBase reference
   const GBaseParameterScan *p_load = GObject::gobject_conversion<GBaseParameterScan>(&cp);

   // Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

   // Check our parent class'es data ...
    deviations.push_back(GOptimizationAlgorithmT<GParameterSet>::checkRelationshipWith(cp, e, limit, "GOptimizationAlgorithmT<GParameterSet>", y_name, withMessages));

   // ... and then our local data
   deviations.push_back(checkExpectation(withMessages, "GBaseParameterScan", scanRandomly_, p_load->scanRandomly_, "scanRandomly_", "p_load->scanRandomly_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GBaseParameterScan", cycleLogicHalt_, p_load->cycleLogicHalt_, "cycleLogicHalt_", "p_load->cycleLogicHalt_", e , limit));

   // We do not check our temporary parameter objects yet (bVec_ etc.)

   return evaluateDiscrepancies("GBaseParameterScan", caller, deviations, e);
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GBaseParameterScan::name() const {
   return std::string("GBaseParameterScan");
}

/******************************************************************************/
/**
 * Loads the data of another population
 *
 * @param cp A pointer to another GBaseParameterScan object, camouflaged as a GObject
 */
void GBaseParameterScan::load_(const GObject *cp) {
   const GBaseParameterScan *p_load = this->gobject_conversion<GBaseParameterScan>(cp);

   // First load the parent class'es data.
   // This will also take care of copying all individuals.
   GOptimizationAlgorithmT<GParameterSet>::load_(cp);

   // ... and then our own data
   cycleLogicHalt_ = p_load->cycleLogicHalt_;
   scanRandomly_ = p_load->scanRandomly_;

   // Load the parameter objects
   bVec_.clear();
   std::vector<boost::shared_ptr<bScanPar> >::const_iterator b_it;
   for(b_it=(p_load->bVec_).begin(); b_it!=(p_load->bVec_).end(); ++b_it) {
      bVec_.push_back((*b_it)->clone());
   }

   int32Vec_.clear();
   std::vector<boost::shared_ptr<int32ScanPar> >::const_iterator i_it;
   for(i_it=(p_load->int32Vec_).begin(); i_it!=(p_load->int32Vec_).end(); ++i_it) {
      int32Vec_.push_back((*i_it)->clone());
   }

   dVec_.clear();
   std::vector<boost::shared_ptr<dScanPar> >::const_iterator d_it;
   for(d_it=(p_load->dVec_).begin(); d_it!=(p_load->dVec_).end(); ++d_it) {
      dVec_.push_back((*d_it)->clone());
   }

   fVec_.clear();
   std::vector<boost::shared_ptr<fScanPar> >::const_iterator f_it;
   for(f_it=(p_load->fVec_).begin(); f_it!=(p_load->fVec_).end(); ++f_it) {
      fVec_.push_back((*f_it)->clone());
   }
}

/******************************************************************************/
/**
 * The actual business logic to be performed during each iteration. Returns the best achieved fitness
 *
 * @return The value of the best individual found
 */
double GBaseParameterScan::cycleLogic() {
   // Apply all necessary modifications to individuals
   updateIndividuals();

   // Trigger value calculation for all individuals
   // This function is purely virtual and needs to be
   // re-implemented in derived classes
   double result=doFitnessCalculation(this->size());

   // This function will sort the population according to
   // its primary fitness value
   sortPopulation();

   // Let the audience know
   return result;
}

/******************************************************************************/
/**
 * Adds new values to the population's individuals. Note that this function
 * may resize the population and set the default population size, if there
 * is no sufficient number of data sets to be evaluated left.
 */
void GBaseParameterScan::updateIndividuals() {
   std::size_t indPos = 0;
   std::vector<bool> bData;
   std::vector<boost::int32_t> iData;
   std::vector<float> fData;
   std::vector<double> dData;

   while(true) {
      //------------------------------------------------------------------------
      // Retrieve a work item
      boost::shared_ptr<parSet> pS = getParameterSet();

      //------------------------------------------------------------------------
      // Fill the parameter set data into the current individual

      // Retrieve the parameter vectors
      this->at(indPos)->streamline<bool>(bData);
      this->at(indPos)->streamline<boost::int32_t>(iData);
      this->at(indPos)->streamline<float>(fData);
      this->at(indPos)->streamline<double>(dData);

      // Add the data items from the parSet object to the vectors

      // 1) For boolean data
      std::vector<singleBPar>::iterator b_it;
      for(b_it=pS->bParVec.begin(); b_it!=pS->bParVec.end(); ++b_it) {
         this->addDataPoint<bool>(*b_it, bData);
      }

      // 2) For boost::int32_t data
      std::vector<singleInt32Par>::iterator i_it;
      for(i_it=pS->iParVec.begin(); i_it!=pS->iParVec.end(); ++i_it) {
         this->addDataPoint<boost::int32_t>(*i_it, iData);
      }

      // 3) For float values
      std::vector<singleFPar>::iterator f_it;
      for(f_it=pS->fParVec.begin(); f_it!=pS->fParVec.end(); ++f_it) {
         this->addDataPoint<float>(*f_it, fData);
      }

      // 4) For double values
      std::vector<singleDPar>::iterator d_it;
      for(d_it=pS->dParVec.begin(); d_it!=pS->dParVec.end(); ++d_it) {
         this->addDataPoint<double>(*d_it, dData);
      }

      // Copy the data back into the individual
      this->at(indPos)->assignValueVector<bool>(bData);
      this->at(indPos)->assignValueVector<boost::int32_t>(iData);
      this->at(indPos)->assignValueVector<float>(fData);
      this->at(indPos)->assignValueVector<double>(dData);

      // Mark the individual as "dirty", so it gets re-evaluated the
      // next time the fitness() function is called
      this->at(indPos)->setDirtyFlag();

      // We were successful
      cycleLogicHalt_ = false;

      //------------------------------------------------------------------------
      // Make sure we continue with the next parameter set in the next iteration
      if(!this->switchToNextParameterSet()) {
         // Let the audience know that the optimization may be stopped
         this->cycleLogicHalt_ = true;

         // Reset all parameter objects for the next run (if desired)
         this->resetParameterObjects();

         // Resize the population, so we only have modified individuals
         this->resize(indPos);

         // Terminate the loop
         break;
      }

      //------------------------------------------------------------------------
      // We do not want to exceed the boundaries of the population
      if(++indPos >= this->getDefaultPopulationSize()) break;
   }
}

/******************************************************************************/
/**
 * Resets all parameter objects
 */
void GBaseParameterScan::resetParameterObjects() {
   std::vector<boost::shared_ptr<bScanPar> >::iterator b_it;
   for(b_it=bVec_.begin(); b_it!=bVec_.end(); ++b_it) {
      (*b_it)->resetPosition();
   }

   std::vector<boost::shared_ptr<int32ScanPar> >::iterator i_it;
   for(i_it=int32Vec_.begin(); i_it!=int32Vec_.end(); ++i_it) {
      (*i_it)->resetPosition();
   }

   std::vector<boost::shared_ptr<fScanPar> >::iterator f_it;
   for(f_it=fVec_.begin(); f_it!=fVec_.end(); ++f_it) {
      (*f_it)->resetPosition();
   }

   std::vector<boost::shared_ptr<dScanPar> >::iterator d_it;
   for(d_it=dVec_.begin(); d_it!=dVec_.end(); ++d_it) {
      (*d_it)->resetPosition();
   }
}

/******************************************************************************/
/**
 * Retrieves a parameter set by filling the current parameter combinations
 * into a parSet object.
 */
boost::shared_ptr<parSet> GBaseParameterScan::getParameterSet() {
   // Create a new parSet object
   boost::shared_ptr<parSet> result(new parSet());

   // Extract the relevant data and store it in a parSet object
   // 1) For boolean objects
   std::vector<boost::shared_ptr<bScanPar> >::iterator b_it;
   for(b_it=bVec_.begin(); b_it!=bVec_.end(); ++b_it) {
      (result->bParVec).push_back((*b_it)->getCurrentItem());
   }
   // 2) For boost::int32_t objects
   std::vector<boost::shared_ptr<int32ScanPar> >::iterator i_it;
   for(i_it=int32Vec_.begin(); i_it!=int32Vec_.end(); ++i_it) {
      (result->iParVec).push_back((*i_it)->getCurrentItem());
   }
   // 3) For float objects
   std::vector<boost::shared_ptr<fScanPar> >::iterator f_it;
   for(f_it=fVec_.begin(); f_it!=fVec_.end(); ++f_it) {
      (result->fParVec).push_back((*f_it)->getCurrentItem());
   }
   // 4) For double objects
   std::vector<boost::shared_ptr<dScanPar> >::iterator d_it;
   for(d_it=dVec_.begin(); d_it!=dVec_.end(); ++d_it) {
      (result->dParVec).push_back((*d_it)->getCurrentItem());
   }

   return result;
}

/******************************************************************************/
/**
 * Switches to the next parameter set
 *
 * @return A boolean indicating whether there indeed is a following
 * parameter set (true) or whether we have reached the end of the
 * collection (false)
 */
bool GBaseParameterScan::switchToNextParameterSet() {
   std::vector<boost::shared_ptr<scanParI> >::iterator it = allParVec_.begin();

   // Switch to the next parameter set
   while(true) {
      if((*it)->goToNextItem()) { // Will trigger if a warp has occurred
         if(it+1 == allParVec_.end()) return false; // All possible combinations were found
         else ++it; // Try the next parameter object
      } else {
         return true; // We have successfully switched to the next parameter set
      }
   }

   // Make the compiler happy
   return false;
}

/******************************************************************************/
/**
 * This function will sort the population according to its primary fitness value.
 */
void GBaseParameterScan::sortPopulation() {
   if(true == this->getMaxMode()) { // Maximization
      std::sort((this->data).begin(), (this->data).end(), boost::bind(&GParameterSet::fitness, _1, 0) > boost::bind(&GParameterSet::fitness, _2, 0));
   } else { // Minimization
      std::sort((this->data).begin(), (this->data).end(), boost::bind(&GParameterSet::fitness, _1, 0) < boost::bind(&GParameterSet::fitness, _2, 0));
   }
}

/******************************************************************************/
/** @brief Fills all parameter objects into the allParVec_ vector */
void GBaseParameterScan::fillAllParVec() {
   // 1) For boolean objects
   std::vector<boost::shared_ptr<bScanPar> >::iterator b_it;
   for(b_it=bVec_.begin(); b_it!=bVec_.end(); ++b_it) {
      allParVec_.push_back(*b_it);
   }
   // 2) For boost::int32_t objects
   std::vector<boost::shared_ptr<int32ScanPar> >::iterator i_it;
   for(i_it=int32Vec_.begin(); i_it!=int32Vec_.end(); ++i_it) {
      allParVec_.push_back(*i_it);
   }
   // 3) For float objects
   std::vector<boost::shared_ptr<fScanPar> >::iterator f_it;
   for(f_it=fVec_.begin(); f_it!=fVec_.end(); ++f_it) {
      allParVec_.push_back(*f_it);
   }
   // 4) For double objects
   std::vector<boost::shared_ptr<dScanPar> >::iterator d_it;
   for(d_it=dVec_.begin(); d_it!=dVec_.end(); ++d_it) {
      allParVec_.push_back(*d_it);
   }
}

/******************************************************************************/
/**
 * Clears the allParVec_ vector
 */
void GBaseParameterScan::clearAllParVec() {
   allParVec_.clear();
}

/******************************************************************************/
/**
 * Retrieves the best individual of the population and returns it in Gem::Geneva::GIndividual format.
 * Note that this protected function will return the item itself. Direct usage of this function should
 * be avoided even by derived classes. We suggest to use the function
 * GOptimizableI::getBestIndividual<individual_type>() instead, which internally uses
 * this function and returns copies of the best individual, converted to the desired target type.
 *
 * @return A shared_ptr to the best individual of the population
 */
boost::shared_ptr<GIndividual> GBaseParameterScan::getBestIndividual(){
#ifdef DEBUG
   if(this->empty()) {
      glogger
      << "In GBaseParameterScan::getBestIndividual() :" << std::endl
      << "Tried to access individual at position 0 even though population is empty." << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

   return this->at(0);
}

/******************************************************************************/
/**
 * Retrieves a list of the best individuals found. For now we only return a single
 * (the best individual).
 *
 * @return A list of the best individuals found
 */
std::vector<boost::shared_ptr<GIndividual> > GBaseParameterScan::getBestIndividuals() {
   std::vector<boost::shared_ptr<GIndividual> > result;
   result.push_back(this->getBestIndividual());
   return result;
}

/******************************************************************************/
/**
 * A custom halt criterion for the optimization, allowing to stop the loop
 * when no items are left to be scanned
 */
bool GBaseParameterScan::customHalt() const {
   if(this->cycleLogicHalt_) {
      std::cerr
      << "Terminating the loop as no items are left to be" << std::endl
      << "processed in parameter scan." << std::endl;
      return true;
   } else {
      return false;
   }
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void GBaseParameterScan::addConfigurationOptions (
   Gem::Common::GParserBuilder& gpb
   , const bool& showOrigin
) {
   std::string comment;

   // Call our parent class'es function
   GOptimizationAlgorithmT<GParameterSet>::addConfigurationOptions(gpb, showOrigin);

   comment = ""; // Reset the comment string
   comment += "Indicates whether scans should be done randomly;";
   comment += "(1) or on a grid (0);";
   if(showOrigin) comment += "[GBaseParameterScan]";
   gpb.registerFileParameter<bool>(
      "scanRandomly_" // The name of the variable
      , scanRandomly_
      , true // The default value
      , Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
      , comment
   );

   std::vector<std::string> defaultStrParVec; // The default values
   defaultStrParVec.push_back("d 0 0.3 0.7 10"); // double value in position 0, scan from 0.3-0.7 (inclusive) in 10 steps
   defaultStrParVec.push_back("f 1 0.3 0.7 10"); // float value in position 1, scan from 0.3-0.7 (inclusive) in 10 steps
   defaultStrParVec.push_back("i 2 5 8 10");     // 32 bit integer in position 2, scan from 5-8 (inclusive). 10 steps are specified. Higher number of steps than possible will be ignored, except for random mode.
   defaultStrParVec.push_back("b 3 0 1 10");     // Boolean in position 3; false and true will be tested. Note that the "boundaries" still need to be specified. 10 steps are specified. A number of steps other than 2 will be ignored, except for random mode.

   comment = ""; // Reset the comment string
   comment += "Specification of the parameters to be used;";
   comment += "in the parameter scan.;";
   gpb.registerFileParameter<std::string>(
      "parameterOptions"
      , defaultStrParVec
      , boost::bind(
            &GBaseParameterScan::parseParameterValues // The call-back function
            , this
            , _1
        )
      , Gem::Common::VAR_IS_ESSENTIAL // Could also be VAR_IS_SECONDARY
      , comment
   );
}

/******************************************************************************/
/**
 * Fills vectors with parameter values
 */
void GBaseParameterScan::parseParameterValues(std::vector<std::string> parStrVec) {
   std::vector<std::string>::iterator it;
   for(it=parStrVec.begin(); it!=parStrVec.end(); ++it) {
      // Remove white-space characters at the edges of the string
      boost::trim(*it);

      // Check that the parameter string isn't empty
      if(it->empty()) {
         glogger
         << "In GBaseParameterScan::parseParameterValues(): Error!" << std::endl
         << "Parameter string in position " << it-parStrVec.begin() << " seems to be empty" << std::endl
         << GEXCEPTION;
      }

      // Retrieve the first character
      char first = it->at(0);

      // Act on the character -- This will result in a collection of parameter objects,
      // which can be used to extract allowed parameter values
      if(first == 'd') {
         dVec_.push_back(boost::shared_ptr<dScanPar>(new dScanPar(*it, scanRandomly_)));
      } else if(first == 'f') {
         fVec_.push_back(boost::shared_ptr<fScanPar>(new fScanPar(*it, scanRandomly_)));
      } else if(first == 'i') {
         int32Vec_.push_back(boost::shared_ptr<int32ScanPar>(new int32ScanPar(*it, scanRandomly_)));
      } else if(first == 'b') {
         bVec_.push_back(boost::shared_ptr<bScanPar>(new bScanPar(*it, scanRandomly_)));
      } else { // Raise an exception
         glogger
         << "In GBaseParameterScan::parseParameterValues(): Error!" << std::endl
         << "Parameter string in position " << it-parStrVec.begin() << " has invalid type: \"" << first << "\"" << std::endl
         << GEXCEPTION;
      }
   }
}

/******************************************************************************/
/**
 * Does some preparatory work before the optimization starts
 */
void GBaseParameterScan::init() {
   // To be performed before any other action
   GOptimizationAlgorithmT<GParameterSet>::init();

   // Reset the custom halt criterion
   cycleLogicHalt_ = false;

   // Make sure we start with a fresh central vector of parameter objects
   this->clearAllParVec();

   // Copy all parameter objects to the central vector for easier handling
   this->fillAllParVec();
}

/******************************************************************************/
/**
 * Does any necessary finalization work
 */
void GBaseParameterScan::finalize() {
   // Last action
   GOptimizationAlgorithmT<GParameterSet>::finalize();
}

/******************************************************************************/
/**
 * Resizes the population to the desired level and does some error checks.
 */
void GBaseParameterScan::adjustPopulation() {
   // Check how many individuals we already have
   std::size_t nStart = this->size();

   // Do some error checking ...

   // An empty population is an error
   if(nStart == 0) {
      glogger
      << "In GBaseParameterScan::adjustPopulation(): Error!" << std::endl
      << "You didn't add any individuals to the collection. We need at least one." << std::endl
      << GEXCEPTION;
   }

   // We want exactly one individual in the beginning. All other registered
   // individuals will be discarded.
   if(nStart > 1) {
      this->resize(1);
      nStart = 1;
   }

   // Check that we have a valid default population size
   if(0 == this->getDefaultPopulationSize()) {
      glogger
      << "In GBaseParameterScan::adjustPopulation(): Error!" << std::endl
      << "Default-size of the population is 0" << std::endl
      << GEXCEPTION;
   }

   // Create the desired number of (identical) individuals in the population.
   for(std::size_t ind=1; ind<this->getDefaultPopulationSize(); ind++) {
      this->push_back(this->at(0)->GObject::clone<GParameterSet>());
   }
}

/******************************************************************************/
/**
 * Loads a checkpoint from disk
 *
 * @param cpFile The name of the file the checkpoint should be loaded from
 */
void GBaseParameterScan::loadCheckpoint(const std::string& cpFile) {
   this->fromFile(cpFile, getCheckpointSerializationMode());
}

/******************************************************************************/
/**
 * Saves the state of the object to disc. We simply serialize the entire object.
 */
void GBaseParameterScan::saveCheckpoint() const {
   bool isDirty;
   double newValue = this->at(0)->GIndividual::getCachedFitness(isDirty);

#ifdef DEBUG
   if(isDirty) {
      glogger
      << "In GBaseParameterScan::saveCheckpoint():" << std::endl
      << "Best individual has dirty flag set!" << std::endl
      << GEXCEPTION;
   }
#endif

   // Determine a suitable name for the output file
   std::string outputFile = getCheckpointDirectory() + boost::lexical_cast<std::string>(this->getIteration()) + "_"
      + boost::lexical_cast<std::string>(newValue) + "_" + getCheckpointBaseName();

   this->toFile(outputFile, getCheckpointSerializationMode());
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GBaseParameterScan::modify_GUnitTests() {
#ifdef GEM_TESTING
   bool result = false;

   // Call the parent class'es function
   if(GOptimizationAlgorithmT<GParameterSet>::modify_GUnitTests()) result = true;

   return result;
#else /* GEM_TESTING */
   condnotset("GBaseParameterScan::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBaseParameterScan::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
   // Call the parent class'es function
   GOptimizationAlgorithmT<GParameterSet>::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseParameterScan::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBaseParameterScan::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
   // Call the parent class'es function
   GOptimizationAlgorithmT<GParameterSet>::specificTestsFailuresExpected_GUnitTests();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseParameterScan::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GBaseParameterScan::GParameterScanOptimizationMonitor::GParameterScanOptimizationMonitor()
   : resultFile_(DEFAULTRESULTFILEOM)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GParameterScanOptimizationMonitor object
 */
GBaseParameterScan::GParameterScanOptimizationMonitor::GParameterScanOptimizationMonitor(const GBaseParameterScan::GParameterScanOptimizationMonitor& cp)
   : GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT(cp)
   , resultFile_(cp.resultFile_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GBaseParameterScan::GParameterScanOptimizationMonitor::~GParameterScanOptimizationMonitor()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GParameterScanOptimizationMonitor object
 * @return A constant reference to this object
 */
const GBaseParameterScan::GParameterScanOptimizationMonitor& GBaseParameterScan::GParameterScanOptimizationMonitor::operator=(const GBaseParameterScan::GParameterScanOptimizationMonitor& cp){
   GBaseParameterScan::GParameterScanOptimizationMonitor::load_(&cp);
   return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GParameter Base object
 *
 * @param  cp A constant reference to another GParameterScanOptimizationMonitor object
 * @return A boolean indicating whether both objects are equal
 */
bool GBaseParameterScan::GParameterScanOptimizationMonitor::operator==(const GBaseParameterScan::GParameterScanOptimizationMonitor& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBaseParameterScan::GParameterScanOptimizationMonitor::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GParameterScanOptimizationMonitor object
 *
 * @param  cp A constant reference to another GParameterScanOptimizationMonitor object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBaseParameterScan::GParameterScanOptimizationMonitor::operator!=(const GBaseParameterScan::GParameterScanOptimizationMonitor& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBaseParameterScan::GParameterScanOptimizationMonitor::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GBaseParameterScan::GParameterScanOptimizationMonitor::checkRelationshipWith(
      const GObject& cp
      , const Gem::Common::expectation& e
      , const double& limit
      , const std::string& caller
      , const std::string& y_name
      , const bool& withMessages
) const {
   using namespace Gem::Common;

   // Check that we are indeed dealing with a GParamterBase reference
   const GBaseParameterScan::GParameterScanOptimizationMonitor *p_load = GObject::gobject_conversion<GBaseParameterScan::GParameterScanOptimizationMonitor >(&cp);

   // Will hold possible deviations from the expectation, including explanations
   std::vector<boost::optional<std::string> > deviations;

   // Check our parent class'es data ...
   deviations.push_back(GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::checkRelationshipWith(cp, e, limit, "GBaseParameterScan::GParameterScanOptimizationMonitor", y_name, withMessages));

   // ... and then our local data
   deviations.push_back(checkExpectation(withMessages, "GBaseParameterScan::GParameterScanOptimizationMonitor", resultFile_, p_load->resultFile_, "resultFile_", "p_load->resultFile_", e , limit));

   return evaluateDiscrepancies("GBaseParameterScan::GParameterScanOptimizationMonitor", caller, deviations, e);
}

/******************************************************************************/
/**
 * Allows to specify a different name for the result file
 *
 * @param resultFile The desired name of the result file
 */
void GBaseParameterScan::GParameterScanOptimizationMonitor::setResultFileName(
      const std::string& resultFile
) {
  resultFile_ = resultFile;
}

/******************************************************************************/
/**
 * Allows to retrieve the current value of the result file name
 *
 * @return The current name of the result file
 */
std::string GBaseParameterScan::GParameterScanOptimizationMonitor::getResultFileName() const {
  return resultFile_;
}

/******************************************************************************/
/**
 * A function that is called once before the optimization starts
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 */
void GBaseParameterScan::GParameterScanOptimizationMonitor::firstInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
#ifdef DEBUG
   if(goa->getOptimizationAlgorithm() != PERSONALITY_PS) {
      glogger
      << "In GBaseParameterScan::GParameterScanOptimizationMonitor::cycleInformation():" << std::endl
      << "Provided optimization algorithm has wrong type: " << goa->getOptimizationAlgorithm() << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

   // TODO
}

/******************************************************************************/
/**
 * A function that is called during each optimization cycle. It is possible to
 * extract quite comprehensive information in each iteration. For examples, see
 * the standard overloads provided for the various optimization algorithms.
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 */
void GBaseParameterScan::GParameterScanOptimizationMonitor::cycleInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
   // Perform the conversion to the target algorithm
   GBaseParameterScan * const gd = static_cast<GBaseParameterScan * const>(goa);

   // TODO
}

/******************************************************************************/
/**
 * A function that is called once at the end of the optimization cycle
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 */
void GBaseParameterScan::GParameterScanOptimizationMonitor::lastInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
   // TODO
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * @param cp A pointer to another GParameterScanOptimizationMonitor object, camouflaged as a GObject
 */
void GBaseParameterScan::GParameterScanOptimizationMonitor::load_(const GObject* cp) {
   const GBaseParameterScan::GParameterScanOptimizationMonitor *p_load = gobject_conversion<GBaseParameterScan::GParameterScanOptimizationMonitor>(cp);

   // Load the parent classes' data ...
   GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::load_(cp);

   // ... and then our local data
   resultFile_ = p_load->resultFile_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object
 */
GObject* GBaseParameterScan::GParameterScanOptimizationMonitor::clone_() const {
   return new GBaseParameterScan::GParameterScanOptimizationMonitor(*this);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GBaseParameterScan::GParameterScanOptimizationMonitor::modify_GUnitTests() {
#ifdef GEM_TESTING
   bool result = false;

   // Call the parent class'es function
   if(GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::modify_GUnitTests()) result = true;

   return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseParameterScan::GParameterScanOptimizationMonitor::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBaseParameterScan::GParameterScanOptimizationMonitor::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
   // Call the parent class'es function
   GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseParameterScan::GParameterScanOptimizationMonitor::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBaseParameterScan::GParameterScanOptimizationMonitor::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
   // Call the parent class'es function
   GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::specificTestsFailuresExpected_GUnitTests();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseParameterScan::GParameterScanOptimizationMonitor::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */


