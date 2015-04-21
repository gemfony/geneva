/**
 * @file GOptimizationAlgorithmFactoryT.hpp
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

// Standard header files go here
#include <string>

// Boost header files go here
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GOPTIMIZATIONALGORITHMFACTORYT_HPP_
#define GOPTIMIZATIONALGORITHMFACTORYT_HPP_

// Geneva headers go here
#include "common/GHelperFunctionsT.hpp"
#include "common/GFactoryT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "geneva/GBaseEA.hpp"
#include "geneva/GSerialEA.hpp"
#include "geneva/GMultiThreadedEA.hpp"
#include "geneva/GBrokerEA.hpp"
#include "geneva/GOAMonitorStore.hpp"
#include "geneva/GPluggableOptimizationMonitorsT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
// Default settings
const boost::uint16_t FACT_DEF_NEVALUATIONTHREADS=0;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is a specialization of the GFactoryT<> class for optimization algorithms.
 */
template <typename optalg_type>
class GOptimizationAlgorithmFactoryT
	: public Gem::Common::GFactoryT<optalg_type>
{
public:
   /***************************************************************************/
	// Let the audience know what type of algorithm will be produced
	typedef optalg_type pType;

   /***************************************************************************/
	/**
	 * The standard constructor
	 */
   explicit GOptimizationAlgorithmFactoryT (
         const std::string& configFile
   )
      : Gem::Common::GFactoryT<optalg_type>(configFile)
      , pm_(DEFAULTEXECMODE)
      , nEvaluationThreads_(boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS)))
      , waitFactor_(Gem::Courtier::DEFAULTBROKERWAITFACTOR2)
      , doLogging_(false)
      , contentCreatorPtr_()
      , maxIterationCL_(-1)
      , maxStallIterationCL_(-1)
      , maxSecondsCL_(-1)
   { /* nothing */ }


	/***************************************************************************/
	/**
	 * Initialization with configuration file and parallelization mode
	 */
   GOptimizationAlgorithmFactoryT (
	      const std::string& configFile
	      , const execMode& pm
	)
		: Gem::Common::GFactoryT<optalg_type>(configFile)
		, pm_(pm)
		, nEvaluationThreads_(boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS)))
		, waitFactor_(Gem::Courtier::DEFAULTBROKERWAITFACTOR2)
		, doLogging_(false)
		, contentCreatorPtr_()
	   , maxIterationCL_(-1)
	   , maxStallIterationCL_(-1)
	   , maxSecondsCL_(-1)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A constructor which also adds a content creation function
	 */
   GOptimizationAlgorithmFactoryT (
	      const std::string& configFile
	      , const execMode& pm
	      , boost::shared_ptr<Gem::Common::GFactoryT<typename optalg_type::individual_type> > contentCreatorPtr
	)
	  : Gem::Common::GFactoryT<optalg_type>(configFile)
	  , pm_(pm)
	  , nEvaluationThreads_(boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS)))
	  , waitFactor_(Gem::Courtier::DEFAULTBROKERWAITFACTOR2)
	  , doLogging_(false)
	  , contentCreatorPtr_(contentCreatorPtr)
	  , maxIterationCL_(-1)
	  , maxStallIterationCL_(-1)
	  , maxSecondsCL_(-1)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GOptimizationAlgorithmFactoryT()
	{ /* nothing */ }

   /***************************************************************************/
   /**
    * Adds local command line options to a boost::program_options::options_description object.
    * These are options common to all implemented algorithms. The command line parameter,
    * however, needs to be specific to a given algorithm, so we can select which algorithm
    * should receive which option. This happens with the help of the small mnemonic assigned
    * to each algorithm (e.g. "ea" for evolutionary algorithms). In order not to "clutter"
    * the output, some options are hidden and will only be shown upon explicit request by
    * the user
    *
    * @param visible Command line options that should always be visible
    * @param hidden Command line options that should only be visible upon request
    */
   virtual void addCLOptions(
      boost::program_options::options_description& visible
      , boost::program_options::options_description& hidden
   ) BASE {
      namespace po = boost::program_options;

      hidden.add_options() (
         (this->getMnemonic() + std::string("MaxIterations")).c_str()
         , po::value<boost::int32_t>(&maxIterationCL_)->default_value(-1)
         , (std::string("\t[GOptimizationAlgorithmFactoryT / ") + this->getMnemonic() + "] The maximum allowed number of iterations or 0 to disable limit").c_str()
      ) (
         (this->getMnemonic() + std::string("MaxStallIterations")).c_str()
         , po::value<boost::int32_t>(&maxStallIterationCL_)->default_value(-1)
         , (std::string("\t[GOptimizationAlgorithmFactoryT / ") + this->getMnemonic() + "] The maximum allowed number of stalled iterations or 0 to disable limit").c_str()
      ) (
         (this->getMnemonic() + std::string("MaxSeconds")).c_str()
         , po::value<boost::int32_t>(&maxSecondsCL_)->default_value(-1)
         , (std::string("\t[GOptimizationAlgorithmFactoryT / ") + this->getMnemonic() + "] The maximum allowed duration in seconds or 0 to disable limit").c_str()
      )
      ;
   }

   /***************************************************************************/
   /**
    * Triggers the creation of objects of the desired type with the preset
    * parallelization mode.
    *
    * @return An object of the desired algorithm type
    */
   virtual boost::shared_ptr<optalg_type> get() override {
      // Retrieve a work item using the methods implemented in our parent class
      boost::shared_ptr<optalg_type> p_alg = Gem::Common::GFactoryT<optalg_type>::get();

      // If we have been given a factory function for individuals, fill the object with data
      if(contentCreatorPtr_) { // Has a content creation object been registered ? If so, add individuals to the population
         for(std::size_t ind=0; ind<p_alg->getDefaultPopulationSize(); ind++) {
            boost::shared_ptr<typename optalg_type::individual_type> p_ind = (*contentCreatorPtr_)();
            if(!p_ind) { // No valid item received, the factory has run empty
               break;
            } else {
               p_alg->push_back(p_ind);
            }
         }
      }

      // Has a custom optimization monitor been registered with the global store ?
      // If so, add a clone to the algorithm
      if(GOAMonitorStore->exists(this->getMnemonic())) {
         boost::shared_ptr<typename optalg_type::GOptimizationMonitorT> p_mon =
               GOAMonitorStore->get(this->getMnemonic())->GObject::template clone<typename optalg_type::GOptimizationMonitorT>();

         if(pluggableInfoFunction_) {
            p_mon->registerPluggableOM(pluggableInfoFunction_);
         }

         p_alg->registerOptimizationMonitor(p_mon);
      }

      // Return the filled object to the audience
      return p_alg;
   }

   /***************************************************************************/
   /**
    * Triggers the creation of objects of the desired type with a user-defined
    * parallelization mode. The function will internally store the previous
    * parallelization mode and reset it to the desired type when done.
    *
    * @param pm A user-defined parallelization mode
    * @return An object of the desired algorithm type
    */
   virtual boost::shared_ptr<optalg_type> get(execMode pm) BASE {
      // Store the previous value
      execMode previous_pm = pm_;
      // Set the parallelization mode
      pm_ = pm;
      // Retrieve an item of the desired type
      boost::shared_ptr<optalg_type> result = this->get();
      // Reset the parallelization mode to its original value
      pm_ = previous_pm;

      // Let the audience know
      return result;
   }

   /***************************************************************************/
   /**
    * Triggers the creation of objects of the desired type and converts them
    * to a given target type. Will throw if conversion is unsuccessful.
    *
    * @return A converted copy of the desired production type
    */
   template <typename target_type>
   boost::shared_ptr<target_type> get() {
      return Gem::Common::convertSmartPointer<optalg_type, target_type>(this->get());
   }

   /***************************************************************************/
   /**
    * Triggers the creation of objects of the desired type with a user-defined
    * parallelization mode and converts them to a given target type. Will throw
    * if conversion is unsuccessful. The function will internally store the previous
    * parallelization mode and reset it to the desired type when done.
    *
    * @return A converted copy of the desired production type
    */
   template <typename target_type>
   boost::shared_ptr<target_type> get(execMode pm) {
      execMode previous_pm = pm_;
      // Set the parallelization mode
      pm_ = pm;
      // Retrieve a work item of the production type
      boost::shared_ptr<optalg_type> result = this->get();
      // Reset the parallelization mode to its original value
      pm_ = previous_pm;

      // Return a converted pointer
      return Gem::Common::convertSmartPointer<optalg_type, target_type>(result);
   }

   /***************************************************************************/
   /**
    * Allows to set the wait factor to be applied to timeouts. Note that a wait
    * factor of 0 will be silently amended and become 1.
    */
   void setWaitFactor(std::size_t waitFactor) {
      if(0==waitFactor) waitFactor_=1;
      else waitFactor_ = waitFactor;
   }

   /***************************************************************************/
   /**
    * Allows to retrieve the wait factor variable
    */
   std::size_t getWaitFactor() const {
      return waitFactor_;
   }

   /***************************************************************************/
   /**
    * Allows to register a content creator
    */
   void registerContentCreator(
      boost::shared_ptr<Gem::Common::GFactoryT<typename optalg_type::individual_type> > cc_ptr
   ) {
      if(!cc_ptr) {
         glogger
         << "In GOptiomizationAlgorithmFactoryT<T>::registerContentCreator(): Error!" << std::endl
         << "Tried to register an empty pointer" << std::endl
         << GEXCEPTION;
      }

      contentCreatorPtr_ = cc_ptr;
   }

   /***************************************************************************/
   /**
    * Allows to register a pluggable optimization monitor
    */
   void registerPluggableOM(boost::function<void(const infoMode&, GOptimizationAlgorithmT<typename optalg_type::individual_type> * const)> pluggableInfoFunction) {
      if(pluggableInfoFunction) {
         pluggableInfoFunction_ = pluggableInfoFunction;
      } else {
         glogger
         << "In GoptimizationAlgorithmFactoryT<>::registerPluggableOM(): Tried to register empty call-back" << std::endl
         << GEXCEPTION;
      }
   }

   /***************************************************************************/
   /**
    * Allows to reset the local pluggable optimization monitor
    */
   void resetPluggableOM() {
      pluggableInfoFunction_= boost::function<void(const infoMode&, GOptimizationAlgorithmT<typename optalg_type::individual_type> * const)>();
   }

   /***************************************************************************/
   /**
    * Gives access to the mnemonics / nickname describing an algorithm
    */
   virtual std::string getMnemonic() const = 0;

   /***************************************************************************/
   /**
    * Gives access to a clear-text description of an algorithm
    */
   virtual std::string getAlgorithmName() const = 0;

   /***************************************************************************/
   /**
    * Allows to manually set the maximum number of iterations as is usually specified on the command line
    */
   void setMaxIterationCL(boost::uint32_t maxIterationCL) {
      maxIterationCL_ = boost::numeric_cast<boost::int32_t>(maxIterationCL);
   }

   /***************************************************************************/
   /**
    * Allows to check whether the maximum number of iterations was set on the command line or using the manual function
    */
   bool maxIterationsCLSet() const {
      if(maxIterationCL_ >=0) return true;
      else return false;
   }

   /***************************************************************************/
   /**
    * Allows to retrieve the maximum number of iterations as set on the command line
    */
   boost::uint32_t getMaxIterationCL() const {
      if(maxIterationCL_ >= 0) {
         return boost::numeric_cast<boost::uint32_t>(maxIterationCL_);
      }
      else {
         glogger
         << "In GOptimizationAlgorithmT<>::getMaxIterationCL(): Error!" << std::endl
         << "maxIterationCL_ wasn't set" << std::endl
         << GEXCEPTION;

         // Make the compiler happy
         return boost::uint32_t(0);
      }
   }

   /***************************************************************************/
   /**
    * Allows to manually set the maximum number of stall iterations as is usually specified on the command line
    */
   void setMaxStallIterationCL(boost::uint32_t maxStallIterationCL) {
      maxStallIterationCL_ = boost::numeric_cast<boost::int32_t>(maxStallIterationCL);
   }

   /***************************************************************************/
   /**
    * Allows to check whether the maximum number of stall iterations was set on the command line or using the manual function
    */
   bool maxStallIterationsCLSet() const {
      if(maxStallIterationCL_ >=0) return true;
      else return false;
   }

   /***************************************************************************/
   /**
    * Allows to retrieve the maximum number of stall iterations as set on the command line
    */
   boost::uint32_t getMaxStallIterationCL() const {
      if(maxStallIterationCL_ >= 0) {
         return boost::numeric_cast<boost::uint32_t>(maxStallIterationCL_);
      }
      else {
         glogger
         << "In GOptimizationAlgorithmT<>::getMaxStallIterationCL(): Error!" << std::endl
         << "maxStallIterationCL_ wasn't set" << std::endl
         << GEXCEPTION;

         // Make the compiler happy
         return boost::uint32_t(0);
      }
   }

   /***************************************************************************/
   /**
    * Allows to manually set the maximum number of seconds for a run as is usually specified on the command line
    */
   void setMaxSecondsCL(boost::uint32_t maxSecondsCL) {
      maxSecondsCL_ = boost::numeric_cast<boost::int32_t>(maxSecondsCL);
   }

   /***************************************************************************/
   /**
    * Allows to check whether the maximum number of seconds was set on the command line or using the manual function
    */
   bool maxSecondsCLSet() const {
      if(maxSecondsCL_ >=0) return true;
      else return false;
   }

   /***************************************************************************/
   /**
    * Allows to retrieve the maximum number of seconds as set on the command line
    */
   boost::posix_time::time_duration getMaxTimeCL() const {
      if(maxSecondsCL_ >= 0) {
         boost::posix_time::time_duration maxDuration = boost::posix_time::seconds(boost::numeric_cast<long>(maxSecondsCL_));
         return maxDuration;
      }
      else {
         glogger
         << "In GOptimizationAlgorithmT<>::getMaxTimeCL(): Error!" << std::endl
         << "maxSecondsCL_ wasn't set" << std::endl
         << GEXCEPTION;

         // Make the compiler happy
         return boost::posix_time::time_duration(boost::posix_time::seconds(0));
      }
   }

protected:
	/***************************************************************************/
	/**
	 * Allows to describe configuration options
	 *
	 * @param gpb A reference to the parser-builder
	 */
	virtual void describeLocalOptions_(
		Gem::Common::GParserBuilder& gpb
	) override {
		using namespace Gem::Courtier;

		std::string comment;

		comment = "";
		comment += "Determines the number of threads simultaneously running;";
		comment += "evaluations in multi-threaded mode. 0 means \"automatic\";";
		gpb.registerFileParameter<boost::uint16_t>(
			"nEvaluationThreads"
			, nEvaluationThreads_
			, FACT_DEF_NEVALUATIONTHREADS
			, Gem::Common::VAR_IS_ESSENTIAL
			, comment
		);

		comment = ""; // Reset the comment string
		comment += "Activates (1) or de-activates (0) logging;";
		gpb.registerFileParameter<bool>(
			"doLogging" // The name of the variable
			, doLogging_
			, false // The default value
			, Gem::Common::VAR_IS_SECONDARY
			, comment
		);

      // add local data
      comment = ""; // Reset the comment string
      comment += "A static factor to be applied to timeouts";
      gpb.registerFileParameter<std::size_t>(
         "waitFactor" // The name of the variable
         , waitFactor_
         , EXPECTFULLRETURN // The default value
         , Gem::Common::VAR_IS_ESSENTIAL
         , comment
      );
	}

   /***************************************************************************/
   /**
    * Allows to act on the configuration options received from the configuration file or from the command line
    */
   virtual void postProcess_(boost::shared_ptr<optalg_type>& p) BASE {
      // Set local options

      // The maximum allowed number of iterations
      if(this->maxIterationsCLSet()) {
         p->optalg_type::setMaxIteration(this->getMaxIterationCL());
      }

      // The maximum number of stalls until operation stops
      if(this->maxStallIterationsCLSet()) {
         p->optalg_type::setMaxStallIteration(this->getMaxStallIterationCL());
      }

      // The maximum amount of time until operation stops
      if(this->maxSecondsCLSet()) {
         p->optalg_type::setMaxTime(this->getMaxTimeCL());
      }
   }

   /***************************************************************************/
	/** @brief Creates individuals of this type */
	virtual boost::shared_ptr<optalg_type> getObject_(Gem::Common::GParserBuilder&, const std::size_t&) = 0;

	execMode pm_; ///< Holds information about the desired parallelization mode
	boost::uint16_t nEvaluationThreads_; ///< The number of threads used for evaluations in multithreaded execution

   std::size_t waitFactor_; ///< A static factor to be applied to timeouts
	bool doLogging_; ///< Specifies whether arrival times of individuals should be logged

	boost::shared_ptr<Gem::Common::GFactoryT<typename optalg_type::individual_type> > contentCreatorPtr_; ///< Holds an object capable of producing objects of the desired type
   boost::function<void(const infoMode&, GOptimizationAlgorithmT<typename optalg_type::individual_type> * const)> pluggableInfoFunction_; ///< A user-defined call-back for information retrieval

private:
	/***************************************************************************/
	/** @brief The default constructor. Intentionally private and undefined */
	GOptimizationAlgorithmFactoryT();

   boost::int32_t maxIterationCL_; ///< The maximum number of iterations. NOTE: SIGNED TO ALLOW CHECK WHETHER PARAMETER WAS SET
   boost::int32_t maxStallIterationCL_; ///< The maximum number of generations without improvement, after which optimization is stopped. NOTE: SIGNED TO ALLOW CHECK WHETHER PARAMETER WAS SET
   boost::int32_t maxSecondsCL_; ///< The maximum number of seconds for the optimization to run. NOTE: SIGNED TO ALLOW CHECK WHETHER PARAMETER WAS SET
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GOPTIMIZATIONALGORITHMFACTORYT_HPP_ */
