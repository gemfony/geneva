/**
 * @file GMultiPopulationEAT.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here

// Boost headers go here
#include <boost/tuple/tuple.hpp>

#ifndef GMULTIPOPULATIONEAT_HPP_
#define GMULTIPOPULATIONEAT_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "common/GThreadPool.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GOptimizableEntity.hpp"
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
 * This is a specialization of the GBaseParChildT<T> class, where T is represented by
 * a GOptimizationAlgorithmT<ind_type>-derivative (i.e. an an optimization algorithm.
 * It can perform meta-optimization in multi-threaded mode. The class is not meant to
 * be serialized and hence does not contain any serialization code.
 */
template <typename oa_type>
class GMultiPopulationEAT
   :public GBaseParChildT<oa_type>
{
   BOOST_MPL_ASSERT((boost::is_base_of<Gem::Geneva::GOptimizableI , oa_type>));
   typedef typename oa_type::individual_type ind_type;

public:
   /***************************************************************************/
   /**
    * The default constructor.
    */
   GMultiPopulationEAT()
      : GBaseParChildT<oa_type>()
      , smodeMP_(DEFAULTSMODEMP)
      , nThreads_(boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS)))
   {
      // Note: We do not currently register a custom optimization monitor.
      // A basic monitor has already been registered inside of GOptimizationMonitorT

      // Make sure we start with a valid population size if the user does not supply these values
      this->setPopulationSizes(10,1);
   }

   /***************************************************************************/
   /**
    * Initialization with the number of threads
    */
   GMultiPopulationEAT(const uint16_t& nThreads)
      : GBaseParChildT<oa_type>()
      , smodeMP_(DEFAULTSMODEMP)
      , nThreads_(nThreads?nThreads:(boost::numeric_cast<uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS))))
   {
      // Note: We do not currently register a custom optimization monitor.
      // A basic monitor has already been registered inside of GOptimizationMonitorT

      // Make sure we start with a valid population size if the user does not supply these values
      this->setPopulationSizes(10,1);
   }

   /***************************************************************************/
   /**
    * A standard copy constructor
    *
    * @param cp Another GMultiPopulationEAT object
    */
   GMultiPopulationEAT(const GMultiPopulationEAT<oa_type>& cp)
      : GBaseParChildT<oa_type>(cp)
      , smodeMP_(cp.smodeMP_)
      , nThreads_(cp.nThreads_)
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
    * The standard assignment operator
    */
   const GCheckCombinerT<ind_type>& operator=(const GCheckCombinerT<ind_type>& cp) {
      this->load_(&cp);
      return *this;
   }

   /***************************************************************************/
   /**
    * Checks for equality with another GMultiPopulationEAT<oa_type> object
    *
    * @param  cp A constant reference to another GMultiPopulationEAT<oa_type> object
    * @return A boolean indicating whether both objects are equal
    */
   bool operator==(const GMultiPopulationEAT<oa_type>& cp) const {
      using namespace Gem::Common;
      try {
         this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
         return true;
      } catch(g_expectation_violation&) {
         return false;
      }
   }

   /***************************************************************************/
   /**
    * Checks for inequality with another GMultiPopulationEAT<oa_type> object
    *
    * @param  cp A constant reference to another GMultiPopulationEAT<oa_type> object
    * @return A boolean indicating whether both objects are inequal
    */
   bool operator!=(const GMultiPopulationEAT<oa_type>& cp) const {
      using namespace Gem::Common;
      try {
         this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
         return true;
      } catch(g_expectation_violation&) {
         return false;
      }
   }

   /***************************************************************************/
   /**
    * Searches for compliance with expectations with respect to another object
    * of the same type
    *
    * @param cp A constant reference to another GObject object
    * @param e The expected outcome of the comparison
    * @param limit The maximum deviation for floating point values (important for similarity checks)
    */
   virtual void compare(
      const GObject& cp
      , const Gem::Common::expectation& e
      , const double& limit
   ) const OVERRIDE {
      using namespace Gem::Common;

      // Check that we are indeed dealing with a GAdaptorT reference
      const GMultiPopulationEAT<oa_type> *p_load = GObject::gobject_conversion<GMultiPopulationEAT<oa_type> >(&cp);

      try {
         BEGIN_COMPARE;

         // Check our parent class'es data ...
         COMPARE_PARENT(GBaseParChildT<oa_type>, cp, e, limit);

         // ... and then our local data
         COMPARE(smodeMP_, p_load->smodeMP_, e, limit);
         COMPARE(nThreads_, p_load->nThreads_, e, limit);

         END_COMPARE;

      } catch(g_expectation_violation& g) { // Create a suitable stack-trace
         throw g("g_expectation_violation caught by GMultiPopulationEAT<oa_type>");
      }
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
    * GMultiPopulationEAT<oa_type>::setSortingScheme() for further information).
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
    */
   virtual void addConfigurationOptions (
      Gem::Common::GParserBuilder& gpb
   ) OVERRIDE {
      std::string comment;
      std::string comment1;
      std::string comment2;

      // Call our parent class'es function
      GBaseParChildT<oa_type>::addConfigurationOptions(gpb);

      comment = ""; // Reset the comment string
      comment += "The sorting scheme. Options;";
      comment += "0: MUPLUSNU mode with a single evaluation criterion;";
      comment += "1: MUCOMMANU mode with a single evaluation criterion;";
      comment += "2: MUCOMMANU mode with single evaluation criterion,;";
      comment += "   the best parent of the last iteration is retained;";
      comment += "   unless a better individual has been found;";
      gpb.registerFileParameter<sortingModeMP>(
         "sortingMethod" // The name of the variable
         , DEFAULTSMODEMP // The default value
         , boost::bind(
            &GMultiPopulationEAT<oa_type>::setSortingScheme
            , this
            , _1
           )
         , Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
         , comment
      );
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
      return std::string("GMultiPopulationEAT<oa_type>");
   }

   /***************************************************************************/
   /**
    * Adds the best individuals of this iteration to a priority queue. The
    * queue will be sorted by the first evaluation criterion of the individuals
    * and may either have a limited or unlimited size, depending on user-
    * settings
    */
   void addIterationBests(
      GParameterSetFixedSizePriorityQueue& bestIndividuals
   ) OVERRIDE {
      const bool CLONE = true;
      const bool DONOTREPLACE = false;

   #ifdef DEBUG
      if(this->empty()) {
         glogger
         << "In GBaseParChildT<GParameterSet>::addIterationBests() :" << std::endl
         << "Tried to retrieve the best individuals even though the population is empty." << std::endl
         << GEXCEPTION;
      }
   #endif /* DEBUG */

      // We simply add the individuals of our first member to the queue
      this->at(0)->addIterationBests(bestIndividuals);
   }

   /***************************************************************************/
   /**
    * If individuals have been stored in this population, they are added to the
    * priority queue. This happens before the optimization cycle starts, so that
    * best individuals from a previous "chained" optimization run aren't lost.
    * Only those individuals are stored in the priority queue that do not have the
    * "dirty flag" set.
    */
   void addCleanStoredBests(
      GParameterSetFixedSizePriorityQueue& bestIndividuals
   ) OVERRIDE {
      const bool CLONE = true;

      typename oa_type::iterator it;
      for(it=this->at(0)->begin(); it!=this->at(0)->end(); ++it) {
         if((*it)->isClean()) {
            bestIndividuals.add(*it, CLONE);
         }
      }
   }

protected:
   /***************************************************************************/
   /**
    * Loads the data of another GMultiPopulationEAT object, camouflaged as a GObject.
    *
    * @param cp A pointer to another GMultiPopulationEAT object, camouflaged as a GObject
    */
   virtual void load_(const GObject * cp) OVERRIDE {
      const GMultiPopulationEAT<oa_type> *p_load = GObject::gobject_conversion<GMultiPopulationEAT<oa_type> >(cp);

      // First load the parent class'es data ...
      GBaseParChildT<oa_type>::load_(cp);

      // ... and then our own data
      smodeMP_  = p_load->smodeMP_;
      nThreads_ = p_load->nThreads_;
   }

   /***************************************************************************/
   /**
    * Creates a deep clone of this object
    *
    * @return A deep clone of this object
    */
   virtual GObject *clone_() const OVERRIDE {
      return new GMultiPopulationEAT<oa_type>(*this);
   }

   /***************************************************************************/
   /**
    * Retrieves the  best individual found. Note that this protected function will return the item itself.
    * Direct usage of this function should be avoided even by derived classes. We suggest to use the
    * function GOptimizableI::getBestIndividual<ind_type>() instead, which internally uses
    * this function and returns copies of the best individual, converted to the desired target type.
    *
    * @return The best individual found
    */
   virtual boost::shared_ptr<GParameterSet> customGetBestIndividual() OVERRIDE {
#ifdef DEBUG
         if(this->empty()) {
            glogger
            << "In GMultiPopulationEAT<oa_type>::customGetBestIndividual() :" << std::endl
            << "Tried to access item at position 0 even though population is empty." << std::endl
            << GEXCEPTION;
         }
#endif /* DEBUG */

      return this->at(0)->GOptimizableI::template getBestIndividual<GParameterSet>();
   }

   /***************************************************************************/
   /**
    * Retrieves a list of the best individuals found.
    *
    * @return A list of the best individuals found
    */
   virtual std::vector<boost::shared_ptr<GParameterSet> > customGetBestIndividuals() OVERRIDE {
      // Some error checking
      if(this->empty()) {
         glogger
         << "In GMultiPopulationEAT<oa_type>::customGetBestIndividuals() :" << std::endl
         << "Population is empty." << std::endl
         << GEXCEPTION;
      }


      return this->at(0)->GOptimizableI::template getBestIndividuals<GParameterSet>();
   }

   /***************************************************************************/
   /**
    * Some error checks related to population sizes
    */
   virtual void populationSanityChecks() const OVERRIDE {
      std::size_t nP      = GBaseParChildT<oa_type>::getNParents();
      std::size_t popSize = GBaseParChildT<oa_type>::getPopulationSize();

      // First check that we have been given a suitable value for the number of parents.
      // Note that a number of checks (e.g. population size != 0) has already been done
      // in the parent class.
      if(0 == nP) {
         glogger
         << "In GMultiPopulationEAT<oa_type>::populationSanityChecks(): Error!" << std::endl
         << "Number of parents is set to 0"
         << GEXCEPTION;
      }

      // In MUCOMMANU_SINGLEEVAL mode we want to have at least as many children as parents,
      // whereas MUPLUSNU_SINGLEEVAL only requires the population size to be larger than the
      // number of parents. MUNU1PRETAIN has the same requirements as MUCOMMANU_SINGLEEVAL,
      // as it is theoretically possible that all children are better than the former
      // parents, so that the first parent individual will be replaced.
      if(
         ((smodeMP_==MUCOMMANU_SINGLEEVAL_MP || smodeMP_==MUNU1PRETAIN_SINGLEEVAL_MP) && (popSize < 2*nP)) ||
         (smodeMP_==MUPLUSNU_SINGLEEVAL_MP && popSize<=nP)
      ){
         std::ostringstream error;
         error
         << "In GMultiPopulationEAT<oa_type>::populationSanityChecks() :" << std::endl
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
    * Adapt all children in parallel. Evaluation is done in a separate function (runFitnessCalculation).
    */
   virtual void adaptChildren() OVERRIDE {
      boost::tuple<std::size_t,std::size_t> range = this->getAdaptionRange();
      typename std::vector<boost::shared_ptr<oa_type> >::iterator it;

      for(it=(this->data).begin()+boost::get<0>(range); it!=(this->data).begin()+boost::get<1>(range); ++it) {
#ifdef DEBUG
         if(!(*it)) {
            glogger
            << "In GMultiPopulationEAT<>::adaptChildren(): Error!" << std::endl
            << "Work item is empty" << std::endl
            << GEXCEPTION;
         }
#endif /* DEBUG */

         tp_ptr_->async_schedule(boost::function<void()>(boost::bind(&oa_type::adapt, *it)));
      }

      // Wait for all threads in the pool to complete their work
      tp_ptr_->wait();
   }

   /***************************************************************************/
   /**
    * Evaluate all children (and possibly parents, depending on the iteration and sorting mode) in parallel
    */
   virtual void runFitnessCalculation() OVERRIDE {
      boost::tuple<std::size_t,std::size_t> range = this->getEvaluationRange();
      typename std::vector<boost::shared_ptr<oa_type> >::iterator it;

#ifdef DEBUG
      // There should be no situation in which a "clean" individual is submitted
      // through this function
      for(std::size_t i=boost::get<0>(range); i<boost::get<1>(range); i++) {
         if(!this->at(i)->isDirty()) {
            glogger
            << "In GMultiPopulationEAT<oa_type>::runFitnessCalculation(): Error!" << std::endl
            << "Tried to evaluate \"clean\" children." << std::endl
            << GEXCEPTION;
         }
      }
#endif

      // Make evaluation possible and initiate the worker threads
      for(it=(this->data).begin() + boost::get<0>(range); it!=(this->data).begin() + boost::get<1>(range); ++it) {
         tp_ptr_->async_schedule(
            boost::function<double()>(
               boost::bind(
                  &oa_type::nonConstFitness
                  , *it
                  , 0
                  , Gem::Geneva::ALLOWREEVALUATION
                  , Gem::Geneva::USETRANSFORMEDFITNESS
               )
            )
         );
      }

      // Wait for all threads in the pool to complete their work
      tp_ptr_->wait();
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
         << "In GMultiPopulationEAT<oa_type>::select():" << std::endl
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
         if(this->inFirstIteration()) {
            this->sortMuPlusNuMode();
         } else {
            this->sortMunu1pretainMode();
         }
         break;

      //----------------------------------------------------------------------------

      case MUCOMMANU_SINGLEEVAL_MP:
         if(this->inFirstIteration()) {
            this->sortMuPlusNuMode();
         } else {
            this->sortMuCommaNuMode();
         }
         break;

      //----------------------------------------------------------------------------
      default:
      {
         glogger
         << "In GMultiPopulationEAT<oa_type>::selectBest(): Error" << std::endl
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
      return boost::tuple<std::size_t, std::size_t>(
            this->inFirstIteration()?0:GMultiPopulationEAT<oa_type>::getNParents()
            ,  this->data.size()
      );
   }

   /***************************************************************************/
   /**
    * The function checks that the population size meets the requirements and does some
    * tagging. It is called from within GOptimizationAlgorithmT<T>::optimize(), before the
    * actual optimization cycle starts.
    */
   virtual void init() OVERRIDE {
      // To be performed before any other action
      GBaseParChildT<oa_type>::init();

      // Initialize our thread pool
      tp_ptr_.reset(new Gem::Common::GThreadPool(boost::numeric_cast<std::size_t>(nThreads_)));
   }


   /***************************************************************************/
   /**
    * Does any necessary finalization work
    */
   virtual void finalize() OVERRIDE {
      // Check whether there were any errors during thread execution
      if(tp_ptr_->hasErrors()) {
         std::vector<std::string> errors;
         errors = tp_ptr_->getErrors();

         glogger
         << "========================================================================" << std::endl
         << "In GMultiPopulationEAT::finalize():" << std::endl
         << "There were errors during thread execution:" << std::endl
         << std::endl;

         for(std::vector<std::string>::iterator it=errors.begin(); it!=errors.end(); ++it) {
            glogger << *it << std::endl;
         }

         glogger << "" << std::endl // This is a hack. Currently glogger does not accept a std::endl directly next to it TODO
         << "========================================================================" << std::endl
         << GEXCEPTION;
      }

      // Terminate our thread pool
      tp_ptr_.reset();

      // Last action
      GBaseParChildT<oa_type>::finalize();
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

   boost::shared_ptr<Gem::Common::GThreadPool> tp_ptr_; ///< Temporarily holds a thread pool

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
      if(GBaseParChildT<oa_type>::modify_GUnitTests()) result = true;

      return result;

   #else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
      condnotset("GMultiPopulationEAT<oa_type>::modify_GUnitTests", "GEM_TESTING");
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
      GBaseParChildT<oa_type>::specificTestsNoFailureExpected_GUnitTests();

      //------------------------------------------------------------------------------
      //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
      condnotset("GMultiPopulationEAT<oa_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
   }

   /***************************************************************************/
   /**
    * Performs self tests that are expected to fail. This is needed for testing purposes
    */
   virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
      // Call the parent class'es function
      GBaseParChildT<oa_type>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */
      condnotset("GMultiPopulationEAT<oa_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
   }
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GMULTIPOPULATIONEAT_HPP_ */
