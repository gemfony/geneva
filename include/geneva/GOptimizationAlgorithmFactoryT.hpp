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

// Standard header files go here
#include <string>

// Boost header files go here
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GOPTIMIZATIONALGORITHMFACTORYT_HPP_
#define GOPTIMIZATIONALGORITHMFACTORYT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

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
    * By default we do nothing so that derived classes do not need to re-implement this
    * function.
    */
   virtual void addCLOptions(boost::program_options::options_description& desc) BASE
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Triggers the creation of objects of the desired type with the preset
    * parallelization mode.
    *
    * @return An object of the desired algorithm type
    */
   virtual boost::shared_ptr<optalg_type> get() OVERRIDE {
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
      if(GOAMonitorStore->exists(this->getMnemomic())) {
         boost::shared_ptr<typename optalg_type::GOptimizationMonitorT> p_mon =
               GOAMonitorStore->get(this->getMnemomic())->GObject::template clone<typename optalg_type::GOptimizationMonitorT>();

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
   virtual std::string getMnemomic() const = 0;

protected:
	/***************************************************************************/
	/**
	 * Allows to describe configuration options
	 *
	 * @param gpb A reference to the parser-builder
	 */
	virtual void describeLocalOptions_(
		Gem::Common::GParserBuilder& gpb
	) OVERRIDE {
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
	/** @brief Creates individuals of this type */
	virtual boost::shared_ptr<optalg_type> getObject_(Gem::Common::GParserBuilder&, const std::size_t&) = 0;
	/** @brief Allows to act on the configuration options received from the configuration file */
	virtual void postProcess_(boost::shared_ptr<optalg_type>&) = 0;

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
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GOPTIMIZATIONALGORITHMFACTORYT_HPP_ */
