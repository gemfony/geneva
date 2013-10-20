/**
 * @file GMultiThreadedPS.cpp
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

#include "geneva/GMultiThreadedPS.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GMultiThreadedPS)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GMultiThreadedPS::GMultiThreadedPS()
   : GBasePS()
   , nThreads_(boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS)))
   , storedServerMode_(false)
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor
 */
GMultiThreadedPS::GMultiThreadedPS(const GMultiThreadedPS& cp)
   : GBasePS(cp)
   , nThreads_(cp.nThreads_)
   , storedServerMode_(cp.storedServerMode_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor. We clear remaining work items in the
 * thread pool and wait for active tasks to finish.
 */
GMultiThreadedPS::~GMultiThreadedPS()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator for GMultiThreadedPS objects.
 *
 * @param cp Reference to another GMultiThreadedPS object
 * @return A constant reference to this object
 */
const GMultiThreadedPS& GMultiThreadedPS::operator=(const GMultiThreadedPS& cp) {
   GMultiThreadedPS::load_(&cp);
   return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GMultiThreadedPS object
 *
 * @param  cp A constant reference to another GMultiThreadedPS object
 * @return A boolean indicating whether both objects are equal
 */
bool GMultiThreadedPS::operator==(const GMultiThreadedPS& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GMultiThreadedPS::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GMultiThreadedPS object
 *
 * @param  cp A constant reference to another GMultiThreadedPS object
 * @return A boolean indicating whether both objects are inequal
 */
bool GMultiThreadedPS::operator!=(const GMultiThreadedPS& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GMultiThreadedPS::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GMultiThreadedPS::checkRelationshipWith(
      const GObject& cp,
      const Gem::Common::expectation& e,
      const double& limit,
      const std::string& caller,
      const std::string& y_name,
      const bool& withMessages
) const {
    using namespace Gem::Common;

   // Check that we are indeed dealing with a GParamterBase reference
   const GMultiThreadedPS *p_load = GObject::gobject_conversion<GMultiThreadedPS>(&cp);

   // Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

   // Check our parent class'es data ...
   deviations.push_back(GBasePS::checkRelationshipWith(cp, e, limit, "GMultiThreadedPS", y_name, withMessages));

   // ... and then our local data
   deviations.push_back(checkExpectation(withMessages, "GMultiThreadedPS", nThreads_, p_load->nThreads_, "nThreads_", "p_load->nThreads_", e , limit));

   return evaluateDiscrepancies("GMultiThreadedPS", caller, deviations, e);
}

/***********************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GMultiThreadedPS::name() const {
   return std::string("GMultiThreadedPS");
}

/******************************************************************************/
/**
 * Sets the number of threads for this population. If nThreads is set
 * to 0, an attempt will be made to set the number of threads to the
 * number of hardware threading units (e.g. number of cores or hyperthreading
 * units).
 *
 * @param nThreads The number of threads this class uses
 */
void GMultiThreadedPS::setNThreads(boost::uint16_t nThreads) {
   if(nThreads == 0) {
      nThreads_ = boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS));
   }
   else {
      nThreads_ = nThreads;
   }
}

/******************************************************************************/
/**
 * Retrieves the number of threads this population uses.
 *
 * @return The maximum number of allowed threads
 */
boost::uint16_t GMultiThreadedPS::getNThreads() const  {
   return nThreads_;
}

/******************************************************************************/
/**
 * Loads the data from another GMultiThreadedPS object.
 *
 * @param vp Pointer to another GMultiThreadedPS object, camouflaged as a GObject
 */
void GMultiThreadedPS::load_(const GObject *cp) {
   // Convert GObject pointer to local format
   const GMultiThreadedPS *p_load = this->gobject_conversion<GMultiThreadedPS>(cp);

   // First load our parent class'es data ...
   GBasePS::load_(cp);

   // ... and then our own
   nThreads_ = p_load->nThreads_;

   // Note that we do not copy storedServerMode_ as it is used for internal caching only
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, camouflaged as a GObject
 */
GObject *GMultiThreadedPS::clone_() const  {
   return new GMultiThreadedPS(*this);
}

/******************************************************************************/
/**
 * Necessary initialization work before the start of the optimization
 */
void GMultiThreadedPS::init() {
   // GBasePS sees exactly the environment it would when called from its own class
   GBasePS::init();

   // Initialize our thread pool
   tp_.reset(new Gem::Common::GThreadPool(nThreads_));

   // We want to confine re-evaluation to defined places. However, we also want to restore
   // the original flags. We thus record the previous setting when setting the flag to true.
   // The function will throw if not all individuals have the same server mode flag.

   // Set the server mode and store the original flag
   bool first = true;
   std::vector<boost::shared_ptr<GParameterSet> >::iterator it;
   for(it=data.begin(); it!=data.end(); ++it){
      if(first){
         storedServerMode_ = (*it)->getServerMode();
         first = false;
      }

      if(storedServerMode_ != (*it)->setServerMode(true)) {
         glogger
         << "In GMultiThreadedPS::init():" << std::endl
         << "Not all server mode flags have the same value!" << std::endl
         << GEXCEPTION;
      }
   }
}

/******************************************************************************/
/**
 * Necessary clean-up work after the optimization has finished
 */
void GMultiThreadedPS::finalize() {
   // Restore the original values
   std::vector<boost::shared_ptr<GParameterSet> >::iterator it;
   for(it=data.begin(); it!=data.end(); ++it) {
      (*it)->setServerMode(storedServerMode_);
   }

   // Terminate our thread pool
   tp_.reset();

   // GBasePS sees exactly the environment it would when called from its own class
   GBasePS::finalize();
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void GMultiThreadedPS::addConfigurationOptions (
   Gem::Common::GParserBuilder& gpb
   , const bool& showOrigin
) {
   std::string comment;

   // Call our parent class'es function
   GBasePS::addConfigurationOptions(gpb, showOrigin);

   // add local data
   comment = ""; // Reset the comment string
   comment += "The number of evaluation threads;";
   comment += "0 means: determine automatically;";
   if(showOrigin) comment += "[GMultiThreadedPS]";
   gpb.registerFileParameter<boost::uint16_t>(
      "nEvaluationThreads" // The name of the variable
      , 0 // The default value
      , boost::bind(
         &GMultiThreadedPS::setNThreads
         , this
         , _1
      )
      , Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
      , comment
   );
}

/******************************************************************************/
/**
 * Allows to assign a name to the role of this individual(-derivative). This is mostly important for the
 * GBrokerEA class which should prevent objects of its type from being stored as an individual in its population.
 * All other objects do not need to re-implement this function (unless they rely on the name for some reason).
 */
std::string GMultiThreadedPS::getIndividualCharacteristic() const {
   return std::string("GENEVA_MTOPTALG");
}

/******************************************************************************/
/**
 * Triggers fitness calculation of a number of individuals. This function performs the same task as done
 * in GBasePS, albeit multi-threaded.
 *
 * @param finalPos The position in the vector up to which the fitness calculation should be performed
 * @return The best fitness found amongst all parents
 */
void GMultiThreadedPS::runFitnessCalculation() {
   // Trigger value calculation for all individuals (including parents)
   GMultiThreadedPS::iterator it;
   for(it=this->begin(); it!=this->end(); ++it) {
#ifdef DEBUG
      // Make sure the evaluated individuals have the dirty flag set
      if(!(*it)->isDirty()) {
         glogger
         << "In GMultiThreadedPS::doFitnessCalculation(const std::size_t&):" << std::endl
         << "Found individual in position " << std::distance(this->begin(), it) << " whose dirty flag isn't set" << std::endl
         << GEXCEPTION;
      }
#endif /* DEBUG */

      // Make sure we are allowed to perform value calculation
      (*it)->setServerMode(false);

      // Submit the actual task
      tp_->schedule(boost::function<double()>(boost::bind(&GParameterSet::fitness, *it, 0)));
   }

   // wait for the pool to run out of tasks
   tp_->wait();
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GMultiThreadedPS::modify_GUnitTests() {
#ifdef GEM_TESTING
   bool result = false;

   // Call the parent class'es function
   if(GBasePS::modify_GUnitTests()) result = true;

   return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GMultiThreadedPS::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GMultiThreadedPS::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
   // Call the parent class'es function
   GBasePS::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GMultiThreadedPS::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GMultiThreadedPS::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
   // Call the parent class'es function
   GBasePS::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GMultiThreadedPS::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

