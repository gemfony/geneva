/**
 * @file GMultiPopulationEAT.hpp
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

// Standard headers go here

// Boost headers go here
#include <boost/tuple/tuple.hpp>

#ifndef GMULTIPOPULATIONEAT_HPP_
#define GMULTIPOPULATIONEAT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "common/GThreadPool.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GIndividual.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GBaseParChildT.hpp"
#include "geneva/GMPEAPersonalityTraits.hpp"

namespace Gem {
namespace Geneva {

/**
 * The default sorting mode
 */
const sortingModeMP DEFAULTSMODEMP=MUCOMMANU_SINGLEEVAL_MP;

/******************************************************************************/
/**
 * This is a specialization of the GBaseParChildT<ind_type> class, where GOptimizationAlgorithmT<ind_type>
 * represents an optimization algorithm. It can perform meta-optimization in
 * multi-threaded mode. The class is not meant to be serialized and hence does not
 * contain any serialization code.
 */
template <typename ind_type>
class GMultiPopulationEAT
   :public GBaseParChildT<GOptimizationAlgorithmT<ind_type> >
{
public:
   /***************************************************************************/
   /**
    * The default constructor.
    */
   GMultiPopulationEAT()
      : GBaseParChildT<GOptimizationAlgorithmT<ind_type> >()
      , smodeMP_(DEFAULTSMODEMP)
      , nThreads_(boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS)))
      , storedServerMode_(true)
   {
      // Note: We do not currently register a custom optimization monitor.
      // A basic monitor has already been registered inside of GOptimizationMonitorT

      // Make sure we start with a valid population size if the user does not supply these values
      this->setDefaultPopulationSize(10,1);
   }

   /***************************************************************************/
   /**
    * A standard copy constructor
    *
    * @param cp Another GMultiPopulationEAT object
    */
   GMultiPopulationEAT(const GMultiPopulationEAT& cp)
      : GBaseParChildT<GOptimizationAlgorithmT<ind_type> >(cp)
      , smodeMP_(cp.smodeMP_)
      , nThreads_(cp.nThreads_)
      , storedServerMode_(true)
   {
      // Copying / setting of the optimization algorithm id is done by the parent class. The same
      // applies to the copying of the optimization monitor.
   }

   /***************************************************************************/
   /**
    * The standard destructor. All work is done in the parent class.
    */
   virtual ~GMultiPopulationEAT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The standard assignment operator.
    *
    * @param cp Another GMultiPopulationEAT object
    * @return A constant reference to this object
    */
   const GMultiPopulationEAT<ind_type>& operator=(const GMultiPopulationEAT<ind_type>& cp) {
      GMultiPopulationEAT::load_(&cp);
      return *this;
   }

   /***************************************************************************/
   /**
    * Checks for equality with another GMultiPopulationEAT object
    *
    * @param  cp A constant reference to another GMultiPopulationEAT object
    * @return A boolean indicating whether both objects are equal
    */
   bool operator==(const GMultiPopulationEAT<ind_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GMultiPopulationEAT::operator==","cp", CE_SILENT);
   }

   /***************************************************************************/
   /**
    * Checks for inequality with another GMultiPopulationEAT object
    *
    * @param  cp A constant reference to another GMultiPopulationEAT object
    * @return A boolean indicating whether both objects are inequal
    */
   bool operator!=(const GMultiPopulationEAT<ind_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GMultiPopulationEAT::operator!=","cp", CE_SILENT);
   }

   /***************************************************************************/
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
   virtual boost::optional<std::string> checkRelationshipWith(
         const GObject& cp
         , const Gem::Common::expectation& e
         , const double& limit
         , const std::string& caller
         , const std::string& y_name
         , const bool& withMessages
   ) const  OVERRIDE{
       using namespace Gem::Common;

      // Check that we are indeed dealing with a GParamterBase reference
      const GMultiPopulationEAT<ind_type> *p_load = GObject::gobject_conversion<GMultiPopulationEAT<ind_type> >(&cp);

      // Will hold possible deviations from the expectation, including explanations
      std::vector<boost::optional<std::string> > deviations;

      // Check our parent class'es data ...
      deviations.push_back(GBaseParChildT<GOptimizationAlgorithmT<ind_type> >::checkRelationshipWith(cp, e, limit, "GMultiPopulationEAT<ind_type>", y_name, withMessages));

      // ... and then our local data
      deviations.push_back(checkExpectation(withMessages, "GMultiPopulationEAT<ind_type>", smodeMP_, p_load->smodeMP_, "smodeMP_", "p_load->smodeMP_", e , limit));
      deviations.push_back(checkExpectation(withMessages, "GMultiPopulationEAT<ind_type>", nThreads_, p_load->nThreads_, "nThreads_", "p_load->nThreads_", e , limit));

      return evaluateDiscrepancies("GMultiPopulationEAT<ind_type>", caller, deviations, e);
   }

   /***************************************************************************/
   /**
    * Returns information about the type of optimization algorithm. This function needs
    * to be overloaded by the actual algorithms to return the correct type.
    *
    * @return The type of optimization algorithm
    */
   virtual std::string getOptimizationAlgorithm() const  OVERRIDE {
      return "PERSONALITY_MPEA";
   }

   /***************************************************************************/
   /**
    * Sets the sorting scheme. In MUPLUSNU_SINGLEEVAL, new parents will be selected from the entire
    * population, including the old parents. In MUCOMMANU_SINGLEEVAL new parents will be selected
    * from children only. MUNU1PRETAIN_SINGLEEVAL means that the best parent of the last generation
    * will also become a new parent (unless a better child was found). All other parents are
    * selected from children only.
    *
    * @param smode The desired sorting scheme
    */
   virtual void setSortingScheme(sortingModeMP smode) {
      smodeMP_=smode;
   }

   /***************************************************************************/
   /**
    * Retrieves information about the current sorting scheme (see
    * GMultiPopulationEAT<ind_type>::setSortingScheme() for further information).
    *
    * @return The current sorting scheme
    */
   sortingModeMP getSortingScheme() const {
      return smodeMP_;
   }

   /***************************************************************************/
   /**
    * Returns the name of this optimization algorithm
    *
    * @return The name assigned to this optimization algorithm
    */
   virtual std::string getAlgorithmName() const OVERRIDE {
      return std::string("Multi-Population Evolutionary Algorithm");
   }

   /***************************************************************************/
   /**
    * Adds local configuration options to a GParserBuilder object
    *
    * @param gpb The GParserBuilder object to which configuration options should be added
    * @param showOrigin Makes the function indicate the origin of parameters in comments
    */
   virtual void addConfigurationOptions (
      Gem::Common::GParserBuilder& gpb
      , const bool& showOrigin
   ) OVERRIDE {
      std::string comment;
      std::string comment1;
      std::string comment2;

      // Call our parent class'es function
      GBaseParChildT<GOptimizationAlgorithmT<ind_type> >::addConfigurationOptions(gpb, showOrigin);

      comment = ""; // Reset the comment string
      comment += "The sorting scheme. Options;";
      comment += "0: MUPLUSNU mode with a single evaluation criterion;";
      comment += "1: MUCOMMANU mode with a single evaluation criterion;";
      comment += "2: MUCOMMANU mode with single evaluation criterion,;";
      comment += "   the best parent of the last iteration is retained;";
      comment += "   unless a better individual has been found;";
      if(showOrigin) comment += "[GMultiPopulationEAT<ind_type>]";
      gpb.registerFileParameter<sortingModeMP>(
         "sortingMethod" // The name of the variable
         , DEFAULTSMODEMP // The default value
         , boost::bind(
            &GMultiPopulationEAT<ind_type>::setSortingScheme
            , this
            , _1
           )
         , Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
         , comment
      );

      // Add local data
      comment = ""; // Reset the comment string
      comment += "The number of threads used to simultaneously process individuals;";
      if(showOrigin) comment += "[GMultiPopulationEAT<ind_type>]";
      gpb.registerFileParameter<boost::uint16_t>(
         "nEvaluationThreads" // The name of the variable
         , 0 // The default value
         , boost::bind(
            &GMultiPopulationEAT<ind_type>::setNThreads
            , this
            , _1
           )
         , Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
         , comment
      );
   }

   /***************************************************************************/
   /**
    * Sets the number of threads for this population. If nThreads is set
    * to 0, an attempt will be made to set the number of threads to the
    * number of hardware threading units (e.g. number of cores or hyperthreading
    * units).
    *
    * @param nThreads The number of threads this class uses
    */
   void setNThreads(boost::uint16_t nThreads) {
      if(nThreads == 0) {
         nThreads_ = boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS));
      }
      else {
         nThreads_ = nThreads;
      }
   }

   /***************************************************************************/
   /**
    * Retrieves the number of threads this population uses.
    *
    * @return The maximum number of allowed threads
    */
   boost::uint16_t getNThreads() const  {
      return nThreads_;
   }

   /***************************************************************************/
   /**
    * Emits a name for this class / object
    */
   virtual std::string name() const OVERRIDE {
      return std::string("GMultiPopulationEAT");
   }

protected:
   /***************************************************************************/
   /**
    * Loads the data of another GMultiPopulationEAT object, camouflaged as a GObject.
    *
    * @param cp A pointer to another GMultiPopulationEAT object, camouflaged as a GObject
    */
   virtual void load_(const GObject * cp) OVERRIDE {
      const GMultiPopulationEAT<ind_type> *p_load = GObject::gobject_conversion<GMultiPopulationEAT<ind_type> >(cp);

      // First load the parent class'es data ...
      GBaseParChildT<GOptimizationAlgorithmT<ind_type> >::load_(cp);

      // ... and then our own data
      smodeMP_ = p_load->smodeMP_;
      nThreads_ = p_load->nThreads_;
   }

   /***************************************************************************/
   /**
    * Creates a deep clone of this object
    *
    * @return A deep clone of this object
    */
   virtual GObject *clone_() const OVERRIDE {
      return new GMultiPopulationEAT<ind_type>(*this);
   }

   /***************************************************************************/
   /**
    * Some error checks related to population sizes
    */
   virtual void populationSanityChecks() const OVERRIDE {
      std::size_t nP = GBaseParChildT<GOptimizationAlgorithmT<ind_type> >::getNParents();

      // First check that we have been given a suitable value for the number of parents.
      // Note that a number of checks (e.g. population size != 0) has already been done
      // in the parent class.
      if(0 == nP) {
         glogger
         << "In GMultiPopulationEAT<ind_type>::populationSanityChecks(): Error!" << std::endl
         << "Number of parents is set to 0"
         << GEXCEPTION;
      }

      // In MUCOMMANU_SINGLEEVAL mode we want to have at least as many children as parents,
      // whereas MUPLUSNU_SINGLEEVAL only requires the population size to be larger than the
      // number of parents. MUNU1PRETAIN has the same requirements as MUCOMMANU_SINGLEEVAL and SA_SINGLEEVAL,
      // as it is theoretically possible that all children are better than the former
      // parents, so that the first parent individual will be replaced.
      std::size_t popSize = GBaseParChildT<GOptimizationAlgorithmT<ind_type> >::getPopulationSize();
      if(
            ((smodeMP_==MUCOMMANU_SINGLEEVAL_MP || smodeMP_==MUNU1PRETAIN_SINGLEEVAL_MP) && (popSize < 2*nP)) ||
            (smodeMP_==MUPLUSNU_SINGLEEVAL_MP && popSize<=nP)
      ){
         std::ostringstream error;
         error
         << "In GMultiPopulationEAT<ind_type>::populationSanityChecks() :" << std::endl
         << "Requested size of population is too small :" << popSize << " " << nP << std::endl
         << "Sorting scheme is ";

         switch(smodeMP_) {
         case MUPLUSNU_SINGLEEVAL:
            error << "MUPLUSNU_SINGLEEVAL" << std::endl;
            break;
         case MUCOMMANU_SINGLEEVAL:
            error << "MUCOMMANU_SINGLEEVAL" << std::endl;
            break;
         case MUNU1PRETAIN_SINGLEEVAL:
            error << "MUNU1PRETAIN" << std::endl;
            break;
         };

         glogger
         << error.str()
         << GEXCEPTION;
      }

   }

   /***************************************************************************/
   /**
    * Adapt all children in parallel. Evaluation is done in a seperate function (evaluateChildren).
    */
   virtual void adaptChildren() OVERRIDE {
      boost::tuple<std::size_t,std::size_t> range = this->getAdaptionRange();
      typename std::vector<boost::shared_ptr<GOptimizationAlgorithmT<ind_type> > >::iterator it;

      for(it=(this->data).begin()+boost::get<0>(range); it!=(this->data).begin()+boost::get<1>(range); ++it) {
         tp_->schedule(boost::function<void()>(boost::bind(&GOptimizationAlgorithmT<ind_type>::adapt, *it)));
      }

      // Wait for all threads in the pool to complete their work
      tp_->wait();
   }

   /***************************************************************************/
   /**
    * Evaluate all children (and possibly parents, depending on the iteration and sorting mode) in parallel
    */
   virtual void evaluateChildren() OVERRIDE {
      boost::tuple<std::size_t,std::size_t> range = this->getEvaluationRange();
      typename std::vector<boost::shared_ptr<GOptimizationAlgorithmT<ind_type> > >::iterator it;

#ifdef DEBUG
      // There should be no situation in which a "clean" individual is submitted
      // through this function
      for(std::size_t i=boost::get<0>(range); i<boost::get<1>(range); i++) {
         if(!this->at(i)->isDirty()) {
            glogger
            << "In GMultiPopulationEAT<ind_type>::evaluateChildren(): Error!" << std::endl
            << "Tried to evaluate \"clean\" children." << std::endl
            << GEXCEPTION;
         }
      }
#endif

      // Make evaluation possible and initiate the worker threads
      for(it=(this->data).begin() + boost::get<0>(range); it!=(this->data).begin() + boost::get<1>(range); ++it) {
         (*it)->setServerMode(false);
         tp_->schedule(boost::function<double()>(boost::bind(&GOptimizationAlgorithmT<ind_type>::doFitnessCalculation, *it)));
      }

      // Wait for all threads in the pool to complete their work
      tp_->wait();

      // Make re-evaluation impossible
      for(it=(this->data).begin() + boost::get<0>(range); it!=(this->data).begin() + boost::get<1>(range); ++it) {
         (*it)->setServerMode(true);
      }
   }

   /***************************************************************************/
   /**
    * Choose new parents, based on the selection scheme set by the user.
    */
   virtual void selectBest() OVERRIDE {
   #ifdef DEBUG
      // We require at this stage that at least the default number of
      // children is present. If individuals can get lost in your setting,
      // you must add mechanisms to "repair" the population before this
      // function is called
      if(((this->data).size()-this->getNParents()) < this->getDefaultNChildren()){
         glogger
         << "In GMultiPopulationEAT<ind_type>::select():" << std::endl
         << "Too few children. Got " << (this->data).size()-this->getNParents() << "," << std::endl
         << "but was expecting at least " << this->getDefaultNChildren() << std::endl
         << GEXCEPTION;
      }
   #endif /* DEBUG */

      switch(smodeMP_) {
      //----------------------------------------------------------------------------
      case MUPLUSNU_SINGLEEVAL_MP:
         this->sortMuPlusNuMode();
         break;

      //----------------------------------------------------------------------------
      case MUNU1PRETAIN_SINGLEEVAL_MP:
         this->sortMunu1pretainMode();
         break;

      //----------------------------------------------------------------------------
      case MUCOMMANU_SINGLEEVAL_MP:
         this->sortMuCommaNuMode();
         break;
      //----------------------------------------------------------------------------

      //----------------------------------------------------------------------------
      default:
      {
         glogger
         << "In GMultiPopulationEAT<ind_type>::selectBest(): Error" << std::endl
         << "Incorrect sorting scheme requested: " << smodeMP_ << std::endl
         << GEXCEPTION;
      }
         break;
      }

      // Let parents know they are parents
      this->markParents();
   }

   /***************************************************************************/
   /**
    * Retrieves the evaluation range in a given iteration and sorting scheme. Depending on the
    * iteration and sorting scheme, the start point will be different. The end-point is not meant
    * to be inclusive.
    *
    * @return The range inside which evaluation should take place
    */
   virtual boost::tuple<std::size_t,std::size_t> getEvaluationRange() const OVERRIDE {
      std::size_t nParents = GMultiPopulationEAT<ind_type>::getNParents();
      std::size_t first=nParents, last=(this->data).size();

      if(this->inFirstIteration()) {
         switch(this->getSortingScheme()) {
         //--------------------------------------------------------------
         case MUPLUSNU_SINGLEEVAL:
         case MUNU1PRETAIN_SINGLEEVAL: // same procedure. We do not know which parent is best
         {
            first = 0;
         }
            break;

         case MUCOMMANU_SINGLEEVAL: // No evaluation of parents is necessary
            break; // nothing -- we start with the first child, i.e. first == nParents;

         default:
         {
            glogger
            << "In GMultiPopulationEAT<ind_type>::getEvaluationRange(): Error" << std::endl
            << "Incorrect sorting scheme requested: " << getSortingScheme() << std::endl
            << GEXCEPTION;
         }
            break;
         }
      }

      return boost::tuple<std::size_t, std::size_t>(first, last);
   }

   /***************************************************************************/
   /**
    * The function checks that the population size meets the requirements and does some
    * tagging. It is called from within GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::optimize(), before the
    * actual optimization cycle starts.
    */
   virtual void init() OVERRIDE {
      // To be performed before any other action
      GBaseParChildT<GOptimizationAlgorithmT<ind_type> >::init();
   }


   /***************************************************************************/
   /**
    * Does any necessary finalization work
    */
   virtual void finalize() OVERRIDE {
      // Last action
      GBaseParChildT<GOptimizationAlgorithmT<ind_type> >::finalize();
   }


   /***************************************************************************/
   /**
    * Retrieve a GPersonalityTraits object belonging to this algorithm
    */
   virtual boost::shared_ptr<GPersonalityTraits> getPersonalityTraits() const  OVERRIDE {
      return boost::shared_ptr<GMPEAPersonalityTraits>(new GMPEAPersonalityTraits());
   }

private:
   /***************************************************************************/
   // Local data

   sortingModeMP smodeMP_; ///< The chosen sorting scheme

   boost::uint16_t nThreads_; ///< The number of threads
   bool storedServerMode_; ///< Temporary storage for individual server mode flags during optimization runs

   boost::shared_ptr<Gem::Common::GThreadPool> tp_; ///< Temporarily holds a thread pool

public:
   /***************************************************************************/
   /**
    * Applies modifications to this object. This is needed for testing purposes
    *
    * @return A boolean which indicates whether modifications were made
    */
   virtual bool modify_GUnitTests() OVERRIDE {
   #ifdef GEM_TESTING

      bool result = false;

      // Call the parent class'es function
      if(GBaseParChildT<GOptimizationAlgorithmT<ind_type> >::modify_GUnitTests()) result = true;

      return result;

   #else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
      condnotset("GMultiPopulationEAT<ind_type>::modify_GUnitTests", "GEM_TESTING");
      return false;
   #endif /* GEM_TESTING */
   }

   /***************************************************************************/
   /**
    * Performs self tests that are expected to succeed. This is needed for testing purposes
    */
   virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
      // Call the parent class'es function
      GBaseParChildT<GOptimizationAlgorithmT<ind_type> >::specificTestsNoFailureExpected_GUnitTests();

      //------------------------------------------------------------------------------
      //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
      condnotset("GMultiPopulationEAT<ind_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
   }

   /***************************************************************************/
   /**
    * Performs self tests that are expected to fail. This is needed for testing purposes
    */
   virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
      // Call the parent class'es function
      GBaseParChildT<GOptimizationAlgorithmT<ind_type> >::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */
      condnotset("GMultiPopulationEAT<ind_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
   }
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GMULTIPOPULATIONEAT_HPP_ */
