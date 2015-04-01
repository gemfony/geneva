/**
 * @file GBasePS.cpp
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

#include "geneva/GBasePS.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBasePS::GPSOptimizationMonitor)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A simple output operator for parSet object, mostly meant for debugging
 */
std::ostream& operator<<(std::ostream& os, const parSet& pS) {
   os
   << "###########################################################" << std::endl
   << "# New parSet object:" << std::endl;

   // Boolean data
   if(!pS.bParVec.empty()) {
      os << "# Boolean data" << std::endl;
      std::vector<singleBPar>::const_iterator cit;
      for(cit=pS.bParVec.begin(); cit!=pS.bParVec.end(); ++cit) {
         os << (boost::get<1>(*cit)?"true":"false") << ":" << boost::get<0>(*cit);
         if(cit+1 != pS.bParVec.end()) {
            os << ", ";
         }
      }
      os << std::endl;
   }

   // boost::int32_t data
   if(!pS.iParVec.empty()) {
      os << "# boost::int32_t data" << std::endl;
      std::vector<singleInt32Par>::const_iterator cit;
      for(cit=pS.iParVec.begin(); cit!=pS.iParVec.end(); ++cit) {
         os << boost::get<1>(*cit) << ":" << boost::get<0>(*cit);
         if(cit+1 != pS.iParVec.end()) {
            os << ", ";
         }
      }
      os << std::endl;
   }

   // float data
   if(!pS.fParVec.empty()) {
      os << "# float data" << std::endl;
      std::vector<singleFPar>::const_iterator cit;
      for(cit=pS.fParVec.begin(); cit!=pS.fParVec.end(); ++cit) {
         os << boost::get<1>(*cit) << ":" << boost::get<0>(*cit);
         if(cit+1 != pS.fParVec.end()) {
            os << ", ";
         }
      }
      os << std::endl;
   }

   // double data
   if(!pS.dParVec.empty()) {
      os << "# double data" << std::endl;
      std::vector<singleDPar>::const_iterator cit;
      for(cit=pS.dParVec.begin(); cit!=pS.dParVec.end(); ++cit) {
         os << boost::get<1>(*cit) << ":" << boost::get<0>(*cit);
         if(cit+1 != pS.dParVec.end()) {
            os << ", ";
         }
      }
      os << std::endl;
   }

   return os;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/** A short identifier suitable for storage in a std::map */
G_API_GENEVA const std::string GBasePS::nickname = "ps";

/******************************************************************************/
/**
 * The default constructor
 */
GBasePS::GBasePS()
   : GOptimizationAlgorithmT<GParameterSet>()
   , cycleLogicHalt_(false)
   , scanRandomly_(true)
   , nMonitorInds_(DEFAULTNMONITORINDS)
   , simpleScanItems_(0)
   , scansPerformed_(0)
{
   // Register the default optimization monitor
   this->registerOptimizationMonitor(
         boost::shared_ptr<GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT>(
               new GPSOptimizationMonitor()
         )
   );
}

/******************************************************************************/
/**
 * A standard copy constructor.
 *
 * @param cp A copy of another GradientDescent object
 */
GBasePS::GBasePS(const GBasePS& cp)
   : GOptimizationAlgorithmT<GParameterSet>(cp)
   , cycleLogicHalt_(cp.cycleLogicHalt_)
   , scanRandomly_(cp.scanRandomly_)
   , nMonitorInds_(cp.nMonitorInds_)
   , simpleScanItems_(cp.simpleScanItems_)
   , scansPerformed_(cp.scansPerformed_)
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
GBasePS::~GBasePS()
{ /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GBasePS& GBasePS::operator=(const GBasePS& cp) {
   this->load_(&cp);
   return *this;
}

/******************************************************************************/
/**
 * Returns information about the type of optimization algorithm. This function needs
 * to be overloaded by the actual algorithms to return the correct type.
 *
 * @return The type of optimization algorithm
 */
std::string GBasePS::getOptimizationAlgorithm() const {
   return "PERSONALITY_PS";
}

/******************************************************************************/
/**
 * Retrieve the number of processable items in the current iteration.
 *
 * @return The number of processable items in the current iteration
 */
std::size_t GBasePS::getNProcessableItems() const {
   return this->size(); // Evaluation always needs to be done for the entire population
}

/******************************************************************************/
/**
 * Returns the name of this optimization algorithm
 *
 * @return The name assigned to this optimization algorithm
 */
std::string GBasePS::getAlgorithmName() const {
   return std::string("Parameter Scan");
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
boost::optional<std::string> GBasePS::checkRelationshipWith(
   const GObject& cp
   , const Gem::Common::expectation& e
   , const double& limit
   , const std::string& caller
   , const std::string& y_name
   , const bool& withMessages
) const {
    using namespace Gem::Common;

   // Check that we are indeed dealing with a GParamterBase reference
   const GBasePS *p_load = GObject::gobject_conversion<GBasePS>(&cp);

   // Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

   // Check our parent class'es data ...
    deviations.push_back(GOptimizationAlgorithmT<GParameterSet>::checkRelationshipWith(cp, e, limit, "GOptimizationAlgorithmT<GParameterSet>", y_name, withMessages));

   // ... and then our local data
   deviations.push_back(checkExpectation(withMessages, "GBasePS", cycleLogicHalt_, p_load->cycleLogicHalt_, "cycleLogicHalt_", "p_load->cycleLogicHalt_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GBasePS", scanRandomly_, p_load->scanRandomly_, "scanRandomly_", "p_load->scanRandomly_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GBasePS", nMonitorInds_, p_load->nMonitorInds_, "nMonitorInds_", "p_load->nMonitorInds_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GBasePS", simpleScanItems_, p_load->simpleScanItems_, "simpleScanItems_", "p_load->simpleScanItems_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GBasePS", scansPerformed_, p_load->scansPerformed_, "scansPerformed_", "p_load->scansPerformed_", e , limit));

   // We do not check our temporary parameter objects yet (bVec_ etc.)

   return evaluateDiscrepancies("GBasePS", caller, deviations, e);
}


/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GBasePS::compare(
   const GObject& cp
   , const Gem::Common::expectation& e
   , const double& limit
) const {
   using namespace Gem::Common;

   // Check that we are indeed dealing with a GBasePS reference
   const GBasePS *p_load = GObject::gobject_conversion<GBasePS>(&cp);

   try {
      // Check our parent class'es data ...
      GOptimizationAlgorithmT<GParameterSet>::compare(cp, e, limit);

      // ... and then our local data
      COMPARE(cycleLogicHalt_, p_load->cycleLogicHalt_, e, limit);
      COMPARE(scanRandomly_, p_load->scanRandomly_, e, limit);
      COMPARE(nMonitorInds_, p_load->nMonitorInds_, e, limit);
      COMPARE(simpleScanItems_, p_load->simpleScanItems_, e, limit);
      COMPARE(scansPerformed_, p_load->scansPerformed_, e, limit);

   } catch(g_expectation_violation& g) { // Create a suitable stack-trace
      g.add("g_expectation_violation caught by GBasePS");
      throw g;
   }
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GBasePS::name() const {
   return std::string("GBasePS");
}

/******************************************************************************/
/**
 * Allows to set the number of "best" individuals to be monitored
 * over the course of the algorithm run
 */
void GBasePS::setNMonitorInds(std::size_t nMonitorInds) {
   nMonitorInds_ = nMonitorInds;
}

/******************************************************************************/
/**
 * Allows to retrieve  the number of "best" individuals to be monitored
 * over the course of the algorithm run
 */
std::size_t GBasePS::getNMonitorInds() const {
   return nMonitorInds_;
}

/******************************************************************************/
/**
 * Loads the data of another population
 *
 * @param cp A pointer to another GBasePS object, camouflaged as a GObject
 */
void GBasePS::load_(const GObject *cp) {
   const GBasePS *p_load = this->gobject_conversion<GBasePS>(cp);

   // First load the parent class'es data.
   // This will also take care of copying all individuals.
   GOptimizationAlgorithmT<GParameterSet>::load_(cp);

   // ... and then our own data
   cycleLogicHalt_  = p_load->cycleLogicHalt_;
   scanRandomly_    = p_load->scanRandomly_;
   nMonitorInds_    = p_load->nMonitorInds_;
   simpleScanItems_ = p_load->simpleScanItems_;
   scansPerformed_  = p_load->scansPerformed_;

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
boost::tuple<double, double> GBasePS::cycleLogic() {
   boost::tuple<double, double> bestFitness = boost::make_tuple(this->getWorstCase(), this->getWorstCase());

   // Apply all necessary modifications to individuals
   if(0 == simpleScanItems_) { // We have been asked to deal with specific parameters
      updateSelectedParameters();
   } else { // We have been asked to randomly initialize the individuals a given number of times
      randomShuffle();
   }

   // Trigger value calculation for all individuals
   // This function is purely virtual and needs to be
   // re-implemented in derived classes
   runFitnessCalculation();

   // Perform post-evaluation updates (mostly of individuals)
   postEvaluationWork();

   // Retrieve information about the best fitness found and disallow re-evaluation
   GBasePS::iterator it;
   boost::tuple<double, double> newEval = boost::make_tuple(0., 0.);
   for(it=this->begin(); it!=this->end(); ++it) {
#ifdef DEBUG
      if(!(*it)->isClean()) {
         glogger
         << "In GBasePS::cycleLogic(): Error!" << std::endl
         << "Individual in position " << (it-this->begin()) << " is not clean" << std::endl
         << GEXCEPTION;
      }
#endif

      newEval = (*it)->getFitnessTuple();
      if(this->isBetter(boost::get<G_TRANSFORMED_FITNESS>(newEval), boost::get<G_TRANSFORMED_FITNESS>(bestFitness))) {
         bestFitness = newEval;
      }
   }

   // Let the audience know
   return bestFitness;
}

/******************************************************************************/
/**
 * Adds new values to the population's individuals. Note that this function
 * may resize the population and set the default population size, if there
 * is no sufficient number of data sets to be evaluated left.
 */
void GBasePS::updateSelectedParameters() {
   std::size_t indPos = 0;

   while(true) {
      //------------------------------------------------------------------------
      // Retrieve a work item
      std::size_t mode = 0;
      boost::shared_ptr<parSet> pS = getParameterSet(mode);

      switch(mode) {
         //---------------------------------------------------------------------
         case 0: // Parameters are referenced by index
         {
            std::vector<bool> bData;
            std::vector<boost::int32_t> iData;
            std::vector<float> fData;
            std::vector<double> dData;

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
         }
         break;

         //---------------------------------------------------------------------
         // Mode 1 and 2 are treated alike
         case 1: // Parameters are referenced as var[n]
         case 2: // Parameters are references as var --> equivalent to var[0]
         {
            std::map<std::string, std::vector<bool> > bData;
            std::map<std::string, std::vector<boost::int32_t> > iData;
            std::map<std::string, std::vector<float> > fData;
            std::map<std::string, std::vector<double> > dData;

            // Retrieve the parameter maos
            this->at(indPos)->streamline<bool>(bData);
            this->at(indPos)->streamline<boost::int32_t>(iData);
            this->at(indPos)->streamline<float>(fData);
            this->at(indPos)->streamline<double>(dData);

            // Add the data items from the parSet object to the maos

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
            this->at(indPos)->assignValueVectors<bool>(bData);
            this->at(indPos)->assignValueVectors<boost::int32_t>(iData);
            this->at(indPos)->assignValueVectors<float>(fData);
            this->at(indPos)->assignValueVectors<double>(dData);
         }

         break;

         //---------------------------------------------------------------------
         default:
         {
            glogger
            << "In GBasePS::updateSelectedParameters(): Error!" << std::endl
            << "Encountered invalid mode " << mode << std::endl
            << GEXCEPTION;
         }
         break;
      }

      //------------------------------------------------------------------------
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
         this->resize(indPos+1);

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
 * Randomly initialize the individuals a given number of times
 */
void GBasePS::randomShuffle() {
   std::size_t indPos = 0;

   while(true) {
      // Update the individual and mark it as "dirty"
      this->at(indPos)->randomInit(ACTIVEONLY);
      // Mark the individual as "dirty", so it gets re-evaluated the
      // next time the fitness() function is called
      this->at(indPos)->setDirtyFlag();

      // We were successful
      cycleLogicHalt_ = false;

      //------------------------------------------------------------------------
      // We do not want to exceed the boundaries of the population -- stop
      // if we have reached the end of the population
      if(++indPos >= this->getDefaultPopulationSize()) break;

      //------------------------------------------------------------------------
      // Make sure we terminate when the desired overall number of random scans has
      // been performed
      if(++scansPerformed_ >= simpleScanItems_) {
         // Let the audience know that the optimization may be stopped
         this->cycleLogicHalt_ = true;

         // Reset all parameter objects for the next run (if desired)
         this->resetParameterObjects();

         // Resize the population, so we only have modified individuals
         this->resize(indPos+1);

         // Terminate the loop
         break;
      }
   }
}

/******************************************************************************/
/**
 * Resets all parameter objects
 */
void GBasePS::resetParameterObjects() {
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

   simpleScanItems_ = std::size_t(0);
}

/******************************************************************************/
/**
 * Retrieves a parameter set by filling the current parameter combinations
 * into a parSet object.
 *
 * @param mode Indicates whether parameters are identified by name or by id
 */
boost::shared_ptr<parSet> GBasePS::getParameterSet(std::size_t& mode) {
   // Create a new parSet object
   boost::shared_ptr<parSet> result(new parSet());

   bool modeSet = false;

   // Extract the relevant data and store it in a parSet object
   // 1) For boolean objects
   std::vector<boost::shared_ptr<bScanPar> >::iterator b_it;
   for(b_it=bVec_.begin(); b_it!=bVec_.end(); ++b_it) {
      NAMEANDIDTYPE var = (*b_it)->getVarAddress();

      if(modeSet) {
         if(boost::get<0>(var) != mode) {
            glogger
            << "In GBasePS::getParameterSet(): Error!" << std::endl
            << "Expected mode " << mode << " but got " << boost::get<0>(var) << std::endl
            << GEXCEPTION;
         }
      } else {
         mode = boost::get<0>(var);
         modeSet=true;
      }

      singleBPar item((*b_it)->getCurrentItem(), boost::get<0>(var), boost::get<1>(var), boost::get<2>(var));
      (result->bParVec).push_back(item);
   }
   // 2) For boost::int32_t objects
   std::vector<boost::shared_ptr<int32ScanPar> >::iterator i_it;
   for(i_it=int32Vec_.begin(); i_it!=int32Vec_.end(); ++i_it) {
      NAMEANDIDTYPE var = (*i_it)->getVarAddress();

      if(modeSet) {
         if(boost::get<0>(var) != mode) {
            glogger
            << "In GBasePS::getParameterSet(): Error!" << std::endl
            << "Expected mode " << mode << " but got " << boost::get<0>(var) << std::endl
            << GEXCEPTION;
         }
      } else {
         mode = boost::get<0>(var);
         modeSet=true;
      }

      singleInt32Par item((*i_it)->getCurrentItem(), boost::get<0>(var), boost::get<1>(var), boost::get<2>(var));
      (result->iParVec).push_back(item);
   }
   // 3) For float objects
   std::vector<boost::shared_ptr<fScanPar> >::iterator f_it;
   for(f_it=fVec_.begin(); f_it!=fVec_.end(); ++f_it) {
      NAMEANDIDTYPE var = (*f_it)->getVarAddress();

      if(modeSet) {
         if(boost::get<0>(var) != mode) {
            glogger
            << "In GBasePS::getParameterSet(): Error!" << std::endl
            << "Expected mode " << mode << " but got " << boost::get<0>(var) << std::endl
            << GEXCEPTION;
         }
      } else {
         mode = boost::get<0>(var);
         modeSet=true;
      }

      singleFPar item((*f_it)->getCurrentItem(), boost::get<0>(var), boost::get<1>(var), boost::get<2>(var));
      (result->fParVec).push_back(item);
   }
   // 4) For double objects
   std::vector<boost::shared_ptr<dScanPar> >::iterator d_it;
   for(d_it=dVec_.begin(); d_it!=dVec_.end(); ++d_it) {
      NAMEANDIDTYPE var = (*d_it)->getVarAddress();

      if(modeSet) {
         if(boost::get<0>(var) != mode) {
            glogger
            << "In GBasePS::getParameterSet(): Error!" << std::endl
            << "Expected mode " << mode << " but got " << boost::get<0>(var) << std::endl
            << GEXCEPTION;
         }
      } else {
         mode = boost::get<0>(var);
         modeSet=true;
      }

      singleDPar item((*d_it)->getCurrentItem(), boost::get<0>(var), boost::get<1>(var), boost::get<2>(var));
      (result->dParVec).push_back(item);
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
bool GBasePS::switchToNextParameterSet() {
   std::vector<boost::shared_ptr<scanParInterface> >::iterator it = allParVec_.begin();

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
 * Fills all parameter objects into the allParVec_ vector
 */
void GBasePS::fillAllParVec() {
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
void GBasePS::clearAllParVec() {
   allParVec_.clear();
}

/******************************************************************************/
/**
 * A custom halt criterion for the optimization, allowing to stop the loop
 * when no items are left to be scanned
 */
bool GBasePS::customHalt() const {
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
 */
void GBasePS::addConfigurationOptions (
   Gem::Common::GParserBuilder& gpb
) {
   // Call our parent class'es function
   GOptimizationAlgorithmT<GParameterSet>::addConfigurationOptions(gpb);

   gpb.registerFileParameter<std::size_t>(
      "size" // The name of the first variable
      , DEFAULTPOPULATIONSIZE
      , boost::bind(
         &GBasePS::setDefaultPopulationSize
         , this
         , _1
        )
   )
   << "The total size of the population";

   gpb.registerFileParameter<bool>(
      "scanRandomly" // The name of the variable
      , scanRandomly_
      , true // The default value
   )
   << "Indicates whether scans of individual variables should be done randomly" << std::endl
   << "(1) or on a grid (0)";

   // Override the default value of maxStallIteration, as the parent
   // default does not make sense for us (we do not need stall iterations)
   gpb.resetFileParameterDefaults(
         "maxStallIteration"
         , DEFAULTMAXPARSCANSTALLIT
   );
}

/******************************************************************************/
/**
 * Analyzes the parameters to be scanned. Note that this function will clear any
 * existing parameter definitions, as parStr represents a new set of parameters
 * to be scanned.
 */
void GBasePS::setParameterSpecs(std::string parStr) {
   // Check that the parameter string isn't empty
   if(parStr.empty()) {
      glogger
      << "In GBasePS::addParameterSpecs(): Error!" << std::endl
      << "Parameter string " << parStr << " is empty" << std::endl
      << GEXCEPTION;
   }

   //---------------------------------------------------------------------------
   // Clear the parameter vectors
   dVec_.clear();
   fVec_.clear();
   int32Vec_.clear();
   bVec_.clear();

   // Parse the parameter string
   GParameterPropertyParser ppp(parStr);

   //---------------------------------------------------------------------------
   // Assign the parameter definitions to our internal parameter vectors.
   // We distinguish between a simple scan, where only a number of work items
   // will be initialized randomly repeatedly, and scans of individual variables.
   simpleScanItems_ = ppp.getNSimpleScanItems();
   if(0 == simpleScanItems_) { // Only act if no "simple scan" was requested
      // Retrieve double parameters
      boost::tuple<
         std::vector<parPropSpec<double> >::const_iterator
         , std::vector<parPropSpec<double> >::const_iterator
      > t_d = ppp.getIterators<double>();

      std::vector<parPropSpec<double> >::const_iterator d_cit = boost::get<0>(t_d);
      std::vector<parPropSpec<double> >::const_iterator d_end = boost::get<1>(t_d);
      for(; d_cit!=d_end; ++d_cit) { // Note: d_cit is already set to the begin of the double parameter arrays
         dVec_.push_back(boost::shared_ptr<dScanPar>(new dScanPar(*d_cit, scanRandomly_)));
      }

      // Retrieve float parameters
      boost::tuple<
         std::vector<parPropSpec<float> >::const_iterator
         , std::vector<parPropSpec<float> >::const_iterator
      > t_f = ppp.getIterators<float>();

      std::vector<parPropSpec<float> >::const_iterator f_cit = boost::get<0>(t_f);
      std::vector<parPropSpec<float> >::const_iterator f_end = boost::get<1>(t_f);
      for(; f_cit!=f_end; ++f_cit) { // Note: f_cit is already set to the begin of the double parameter arrays
         fVec_.push_back(boost::shared_ptr<fScanPar>(new fScanPar(*f_cit, scanRandomly_)));
      }

      // Retrieve integer parameters
      boost::tuple<
         std::vector<parPropSpec<boost::int32_t> >::const_iterator
         , std::vector<parPropSpec<boost::int32_t> >::const_iterator
      > t_i = ppp.getIterators<boost::int32_t>();

      std::vector<parPropSpec<boost::int32_t> >::const_iterator i_cit = boost::get<0>(t_i);
      std::vector<parPropSpec<boost::int32_t> >::const_iterator i_end = boost::get<1>(t_i);
      for(; i_cit!=i_end; ++i_cit) { // Note: i_cit is already set to the begin of the double parameter arrays
         int32Vec_.push_back(boost::shared_ptr<int32ScanPar>(new int32ScanPar(*i_cit, scanRandomly_)));
      }

      // Retrieve boolean parameters
      boost::tuple<
         std::vector<parPropSpec<bool> >::const_iterator
         , std::vector<parPropSpec<bool> >::const_iterator
      > t_b = ppp.getIterators<bool>();

      std::vector<parPropSpec<bool> >::const_iterator b_cit = boost::get<0>(t_b);
      std::vector<parPropSpec<bool> >::const_iterator b_end = boost::get<1>(t_b);
      for(; b_cit!=b_end; ++b_cit) { // Note: b_cit is already set to the begin of the double parameter arrays
         bVec_.push_back(boost::shared_ptr<bScanPar>(new bScanPar(*b_cit, scanRandomly_)));
      }
   }

   //---------------------------------------------------------------------------
}

/******************************************************************************/
/**
 * Specified the number of simple scans an puts the class in "simple scan" mode
 */
void GBasePS::setNSimpleScans(std::size_t simpleScanItems) {
   simpleScanItems_ = simpleScanItems;
}

/******************************************************************************/
/**
 * Retrieves the number of simple scans (or 0, if disabled)
 */
std::size_t GBasePS::getNSimpleScans() const {
   return simpleScanItems_;
}

/******************************************************************************/
/**
 * Retrieves the number of simple scans performed so far
 */
std::size_t GBasePS::getNScansPerformed() const {
   return scansPerformed_;
}

/******************************************************************************/
/**
 * Allows to specify whether the parameter space should be scanned randomly
 * or on a grid
 */
void GBasePS::setScanRandomly(bool scanRandomly) {
   scanRandomly_ = scanRandomly;
}

/******************************************************************************/
/**
 * Allows to check whether the parameter space should be scanned randomly
 * or on a grid
 */
bool GBasePS::getScanRandomly() const {
   return scanRandomly_;
}

/******************************************************************************/
/**
 * Does some preparatory work before the optimization starts
 */
void GBasePS::init() {
   // To be performed before any other action
   GOptimizationAlgorithmT<GParameterSet>::init();

   // Reset the custom halt criterion
   cycleLogicHalt_ = false;

   // No scans have been peformed so far
   scansPerformed_ = 0;

   // Make sure we start with a fresh central vector of parameter objects
   this->clearAllParVec();

   // Copy all parameter objects to the central vector for easier handling
   this->fillAllParVec();
}

/******************************************************************************/
/**
 * Does any necessary finalization work
 */
void GBasePS::finalize() {
   // Last action
   GOptimizationAlgorithmT<GParameterSet>::finalize();
}

/******************************************************************************/
/**
 * Retrieve a GPersonalityTraits object belonging to this algorithm
 */
boost::shared_ptr<GPersonalityTraits> GBasePS::getPersonalityTraits() const {
   return boost::shared_ptr<GPSPersonalityTraits>(new GPSPersonalityTraits());
}

/******************************************************************************/
/**
 * Resizes the population to the desired level and does some error checks.
 */
void GBasePS::adjustPopulation() {
   // Check how many individuals we already have
   std::size_t nStart = this->size();

   // Do some error checking ...

   // An empty population is an error
   if(nStart == 0) {
      glogger
      << "In GBasePS::adjustPopulation(): Error!" << std::endl
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
      << "In GBasePS::adjustPopulation(): Error!" << std::endl
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
void GBasePS::loadCheckpoint(const boost::filesystem::path& cpFile) {
   this->fromFile(cpFile, getCheckpointSerializationMode());
}

/******************************************************************************/
/**
 * Saves the state of the object to disc. We simply serialize the entire object.
 */
void GBasePS::saveCheckpoint() const {
   double newValue = this->at(0)->transformedFitness();

   // Determine a suitable name for the output file
   std::string outputFile = getCheckpointDirectory() + boost::lexical_cast<std::string>(this->getIteration()) + "_"
      + boost::lexical_cast<std::string>(newValue) + "_" + getCheckpointBaseName();

   this->toFile(boost::filesystem::path(outputFile), getCheckpointSerializationMode());
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GBasePS::modify_GUnitTests() {
#ifdef GEM_TESTING
   bool result = false;

   // Call the parent class'es function
   if(GOptimizationAlgorithmT<GParameterSet>::modify_GUnitTests()) result = true;

   return result;
#else /* GEM_TESTING */
   condnotset("GBasePS::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBasePS::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
   // Call the parent class'es function
   GOptimizationAlgorithmT<GParameterSet>::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBasePS::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBasePS::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
   // Call the parent class'es function
   GOptimizationAlgorithmT<GParameterSet>::specificTestsFailuresExpected_GUnitTests();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBasePS::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GBasePS::GPSOptimizationMonitor::GPSOptimizationMonitor()
   : csvResultFile_(DEFAULTCSVRESULTFILEOM)
   , withNameAndType_(false)
   , withCommas_ (true)
   , useRawFitness_(true)
   , showValidity_(true)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GPSOptimizationMonitor object
 */
GBasePS::GPSOptimizationMonitor::GPSOptimizationMonitor(const GBasePS::GPSOptimizationMonitor& cp)
   : GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT(cp)
   , csvResultFile_(cp.csvResultFile_)
   , withNameAndType_(cp.withNameAndType_)
   , withCommas_ (cp.withCommas_)
   , useRawFitness_(cp.useRawFitness_)
   , showValidity_(cp.showValidity_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GBasePS::GPSOptimizationMonitor::~GPSOptimizationMonitor()
{ /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GBasePS::GPSOptimizationMonitor& GBasePS::GPSOptimizationMonitor::operator=(
   const GBasePS::GPSOptimizationMonitor& cp
) {
   this->load_(&cp);
   return *this;
}


/******************************************************************************/
/**
 * Checks for equality with another GParameter Base object
 *
 * @param  cp A constant reference to another GPSOptimizationMonitor object
 * @return A boolean indicating whether both objects are equal
 */
bool GBasePS::GPSOptimizationMonitor::operator==(const GBasePS::GPSOptimizationMonitor& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBasePS::GPSOptimizationMonitor::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GPSOptimizationMonitor object
 *
 * @param  cp A constant reference to another GPSOptimizationMonitor object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBasePS::GPSOptimizationMonitor::operator!=(const GBasePS::GPSOptimizationMonitor& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBasePS::GPSOptimizationMonitor::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GBasePS::GPSOptimizationMonitor::checkRelationshipWith(
   const GObject& cp
   , const Gem::Common::expectation& e
   , const double& limit
   , const std::string& caller
   , const std::string& y_name
   , const bool& withMessages
) const {
   using namespace Gem::Common;

   // Check that we are indeed dealing with a GParamterBase reference
   const GBasePS::GPSOptimizationMonitor *p_load = GObject::gobject_conversion<GBasePS::GPSOptimizationMonitor >(&cp);

   // Will hold possible deviations from the expectation, including explanations
   std::vector<boost::optional<std::string> > deviations;

   // Check our parent class'es data ...
   deviations.push_back(GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::checkRelationshipWith(cp, e, limit, "GBasePS::GPSOptimizationMonitor", y_name, withMessages));

   // ... and then our local data
   deviations.push_back(checkExpectation(withMessages, "GBasePS::GPSOptimizationMonitor", csvResultFile_, p_load->csvResultFile_, "csvResultFile_", "p_load->csvResultFile_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GBasePS::GPSOptimizationMonitor", withNameAndType_, p_load->withNameAndType_, "withNameAndType_", "p_load->withNameAndType_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GBasePS::GPSOptimizationMonitor", withCommas_, p_load->withCommas_, "withCommas_", "p_load->withCommas_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GBasePS::GPSOptimizationMonitor", useRawFitness_, p_load->useRawFitness_, "useRawFitness_", "p_load->useRawFitness_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GBasePS::GPSOptimizationMonitor", showValidity_, p_load->showValidity_, "showValidity_", "p_load->showValidity_", e , limit));

   return evaluateDiscrepancies("GBasePS::GPSOptimizationMonitor", caller, deviations, e);
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GBasePS::GPSOptimizationMonitor::compare(
   const GObject& cp
   , const Gem::Common::expectation& e
   , const double& limit
) const {
   using namespace Gem::Common;

   // Check that we are indeed dealing with a GBaseEA reference
   const GBasePS::GPSOptimizationMonitor *p_load = GObject::gobject_conversion<GBasePS::GPSOptimizationMonitor>(&cp);

   try {
      // Check our parent class'es data ...
      GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::compare(cp, e, limit);

      // ... and then our local data
      COMPARE(csvResultFile_, p_load->csvResultFile_, e, limit);
      COMPARE(withNameAndType_, p_load->withNameAndType_, e, limit);
      COMPARE(withCommas_, p_load->withCommas_, e, limit);
      COMPARE(useRawFitness_, p_load->useRawFitness_, e, limit);
      COMPARE(showValidity_, p_load->showValidity_, e, limit);

   } catch(g_expectation_violation& g) { // Create a suitable stack-trace
      g.add("g_expectation_violation caught by GBasePS::GPSOptimizationMonitor");
      throw g;
   }
}


/******************************************************************************/
/**
 * Allows to specify a different name for the result file
 *
 * @param resultFile The desired name of the result file
 */
void GBasePS::GPSOptimizationMonitor::setCSVResultFileName(
   const std::string& csvResultFile
) {
   csvResultFile_ = csvResultFile;
}

/******************************************************************************/
/**
 * Allows to retrieve the current value of the result file name
 *
 * @return The current name of the result file
 */
std::string GBasePS::GPSOptimizationMonitor::getCSVResultFileName() const {
  return csvResultFile_;
}

/***************************************************************************/
/**
 * Allows to specify whether explanations should be printed for parameter-
 * and fitness values.
 */
void GBasePS::GPSOptimizationMonitor::setPrintWithNameAndType(bool withNameAndType) {
   withNameAndType_ = withNameAndType;
}

/***************************************************************************/
/**
 * Allows to check whether explanations should be printed for parameter-
 * and fitness values
 */
bool GBasePS::GPSOptimizationMonitor::getPrintWithNameAndType() const {
   return withNameAndType_;
}

/***************************************************************************/
/**
 * Allows to specify whether commas should be printed in-between values
 */
void GBasePS::GPSOptimizationMonitor::setPrintWithCommas(bool withCommas) {
   withCommas_ = withCommas;
}

/***************************************************************************/
/**
 * Allows to check whether commas should be printed in-between values
 */
bool GBasePS::GPSOptimizationMonitor::getPrintWithCommas() const {
   return withCommas_;
}

/***************************************************************************/
/**
 * Allows to specify whether the true (instead of the transformed) fitness should be shown
 */
void GBasePS::GPSOptimizationMonitor::setUseTrueFitness(bool useRawFitness) {
   useRawFitness_ = useRawFitness;
}

/***************************************************************************/
/**
 * Allows to retrieve whether the true (instead of the transformed) fitness should be shown
 */
bool GBasePS::GPSOptimizationMonitor::getUseTrueFitness() const {
   return useRawFitness_;
}

/***************************************************************************/
/**
 * Allows to specify whether the validity of a solution should be shown
 */
void GBasePS::GPSOptimizationMonitor::setShowValidity(bool showValidity) {
   showValidity_ = showValidity;
}

/***************************************************************************/
/**
 * Allows to check whether the validity of a solution will be shown
 */
bool GBasePS::GPSOptimizationMonitor::getShowValidity() const {
   return showValidity_;
}

/******************************************************************************/
/**
 * A function that is called once before the optimization starts
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 */
void GBasePS::GPSOptimizationMonitor::firstInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
#ifdef DEBUG
   if(goa->getOptimizationAlgorithm() != "PERSONALITY_PS") {
      glogger
      << "In GBasePS::GPSOptimizationMonitor::cycleInformation():" << std::endl
      << "Provided optimization algorithm has wrong type: " << goa->getOptimizationAlgorithm() << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

   // If a file with this name already exists, remove it
   if(boost::filesystem::exists(csvResultFile_.c_str())) {
      boost::filesystem::remove(csvResultFile_.c_str());
   }
}

/******************************************************************************/
/**
 * A function that is called during each optimization cycle. It is possible to
 * extract quite comprehensive information in each iteration. For examples, see
 * the standard overloads provided for the various optimization algorithms.
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 */
void GBasePS::GPSOptimizationMonitor::cycleInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
   // Perform the conversion to the target algorithm
   GBasePS * const ps = static_cast<GBasePS * const>(goa);

   // Open the result file in append mode
   bf::ofstream result(csvResultFile_, std::ofstream::app);

   GBasePS::iterator it;
   std::size_t pos=0;
   for(it=ps->begin(); it!=ps->end(); ++it) {
      if(ps->inFirstIteration() && 0==pos) { // First call to this function
         result << (*it)->toCSV(true, withCommas_, useRawFitness_, showValidity_); // always output variable names and types header
      } else {
         result << (*it)->toCSV(withNameAndType_, withCommas_, useRawFitness_, showValidity_);
      }

      pos++;
   }

   result.close();
}

/******************************************************************************/
/**
 * A function that is called once at the end of the optimization cycle
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 */
void GBasePS::GPSOptimizationMonitor::lastInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
   // nothing
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * @param cp A pointer to another GPSOptimizationMonitor object, camouflaged as a GObject
 */
void GBasePS::GPSOptimizationMonitor::load_(const GObject* cp) {
   const GBasePS::GPSOptimizationMonitor *p_load = gobject_conversion<GBasePS::GPSOptimizationMonitor>(cp);

   // Load the parent classes' data ...
   GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::load_(cp);

   // ... and then our local data
   csvResultFile_ = p_load->csvResultFile_;
   withNameAndType_ = p_load->withNameAndType_;
   withCommas_ = p_load->withCommas_;
   useRawFitness_ = p_load->useRawFitness_;
   showValidity_ = p_load->showValidity_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object
 */
GObject* GBasePS::GPSOptimizationMonitor::clone_() const {
   return new GBasePS::GPSOptimizationMonitor(*this);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GBasePS::GPSOptimizationMonitor::modify_GUnitTests() {
#ifdef GEM_TESTING
   bool result = false;

   // Call the parent class'es function
   if(GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::modify_GUnitTests()) result = true;

   return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBasePS::GPSOptimizationMonitor::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBasePS::GPSOptimizationMonitor::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
   // Call the parent class'es function
   GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBasePS::GPSOptimizationMonitor::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBasePS::GPSOptimizationMonitor::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
   // Call the parent class'es function
   GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::specificTestsFailuresExpected_GUnitTests();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBasePS::GPSOptimizationMonitor::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */


