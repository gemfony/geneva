/**
 * @file GOptimizationAlgorithmFactoryT.hpp
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
template <typename prod_type>
class GOptimizationAlgorithmFactoryT
	: public Gem::Common::GFactoryT<prod_type>
{
public:
   /***************************************************************************/
	// Let the audience know what type of algorithm will be produced
	typedef prod_type pType;

   /***************************************************************************/
	/**
	 * The standard constructor
	 */
   explicit GOptimizationAlgorithmFactoryT (
         const std::string& configFile
   )
      : Gem::Common::GFactoryT<prod_type>(configFile)
      , pm_(DEFAULTEXECMODE)
      , nEvaluationThreads_(boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS)))
      , minWaitFactor_(Gem::Courtier::DEFAULTMINBROKERWAITFACTOR)
      , maxWaitFactor_(Gem::Courtier::DEFAULTMAXBROKERWAITFACTOR)
      , doLogging_(false)
      , boundlessWait_(false)
      , waitFactorIncrement_(Gem::Courtier::DEFAULTBROKERWAITFACTORINCREMENT)
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
		: Gem::Common::GFactoryT<prod_type>(configFile)
		, pm_(pm)
		, nEvaluationThreads_(boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS)))
		, minWaitFactor_(Gem::Courtier::DEFAULTMINBROKERWAITFACTOR)
		, maxWaitFactor_(Gem::Courtier::DEFAULTMAXBROKERWAITFACTOR)
      , doLogging_(false)
      , boundlessWait_(false)
		, waitFactorIncrement_(Gem::Courtier::DEFAULTBROKERWAITFACTORINCREMENT)
		, contentCreatorPtr_()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A constructor which also adds a content creation function
	 */
	GOptimizationAlgorithmFactoryT (
	      const std::string& configFile
	      , const execMode& pm
	      , boost::shared_ptr<Gem::Common::GFactoryT<typename prod_type::individual_type> > contentCreatorPtr
	)
	  : Gem::Common::GFactoryT<prod_type>(configFile)
	  , pm_(pm)
	  , nEvaluationThreads_(boost::numeric_cast<boost::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNBOOSTTHREADS)))
	  , minWaitFactor_(Gem::Courtier::DEFAULTMINBROKERWAITFACTOR)
	  , maxWaitFactor_(Gem::Courtier::DEFAULTMAXBROKERWAITFACTOR)
	  , doLogging_(false)
	  , boundlessWait_(false)
	  , waitFactorIncrement_(Gem::Courtier::DEFAULTBROKERWAITFACTORINCREMENT)
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
    * Triggers the creation of objects of the desired type with the preset
    * parallelization mode.
    *
    * @return An object of the desired algorithm type
    */
   virtual boost::shared_ptr<prod_type> get() {
      // Retrieve a work item using the methods implemented in our parent class
      boost::shared_ptr<prod_type> p_alg = Gem::Common::GFactoryT<prod_type>::get();

      // If we have been given a factory function for individuals, fill the object with data
      if(contentCreatorPtr_) { // Has a content creation object been registered ? If so, add individuals to the population
         for(std::size_t ind=0; ind<p_alg->getDefaultPopulationSize(); ind++) {
            boost::shared_ptr<typename prod_type::individual_type> p_ind = (*contentCreatorPtr_)();
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
         p_alg->registerOptimizationMonitor(
               GOAMonitorStore->get(this->getMnemomic())->GObject::template clone<typename prod_type::GOptimizationMonitorT>()
         );
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
   virtual boost::shared_ptr<prod_type> get(execMode pm) {
      // Store the previous value
      execMode previous_pm = pm_;
      // Set the parallelization mode
      pm_ = pm;
      // Retrieve an item of the desired type
      boost::shared_ptr<prod_type> result = this->get();
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
      return Gem::Common::convertSmartPointer<prod_type, target_type>(this->get());
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
      boost::shared_ptr<prod_type> result = this->get();
      // Reset the parallelization mode to its original value
      pm_ = previous_pm;

      // Return a converted pointer
      return Gem::Common::convertSmartPointer<prod_type, target_type>(result);
   }

   /***************************************************************************/
   /**
    * Allows to register a content creator
    */
   void registerContentCreator(
         boost::shared_ptr<Gem::Common::GFactoryT<typename prod_type::individual_type> > cc_ptr
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
	) {
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


		// add local data
		comment = ""; // Reset the comment string
		comment += "The timeout for the retrieval of an;";
		comment += "iteration's first timeout;";
		{
			using namespace boost::posix_time;
			time_duration td(duration_from_string(DEFAULTBROKERFIRSTTIMEOUT));
			gpb.registerFileParameter<boost::posix_time::time_duration>(
				"firstTimeOut" // The name of the variable
				, firstTimeOut_
				, td // The default value
				, Gem::Common::VAR_IS_ESSENTIAL
				, comment
			);
		}

		comment = ""; // Reset the comment string
		comment += "Indicates that the broker connector should wait endlessly;";
		comment += "for further arrivals of individuals in an iteration;";
		gpb.registerFileParameter<bool>(
			"boundlessWait" // The name of the variable
			, boundlessWait_
			, false // The default value
			, Gem::Common::VAR_IS_ESSENTIAL
			, comment
		);

		comment = ""; // Reset the first comment string
		comment += "The lower boundary for the adaption;";
		comment += "of the waitFactor variable;";
		gpb.registerFileParameter<double>(
			"minWaitFactor" // The name of the first variable
			, minWaitFactor_
			, DEFAULTMINBROKERWAITFACTOR // The first default value
			, Gem::Common::VAR_IS_ESSENTIAL
			, comment
		);

		comment = ""; // Reset the second comment string
		comment += "The upper boundary for the adaption;";
		comment += "of the waitFactor variable;";
		gpb.registerFileParameter<double>(
			"maxWaitFactor" // The name of the second variable
			, maxWaitFactor_
			, DEFAULTMAXBROKERWAITFACTOR // The second default value
			, Gem::Common::VAR_IS_ESSENTIAL
			, comment
		);

		comment =  ""; // 	Reset the comment string
		comment += "Specifies the amount by which the wait factor gets;";
		comment += "incremented or decremented during automatic adaption;";
		gpb.registerFileParameter<double>(
			"waitFactorIncrement" // The name of the variable
			, waitFactorIncrement_
			, DEFAULTBROKERWAITFACTORINCREMENT // The default value
			, Gem::Common::VAR_IS_ESSENTIAL
			, comment
		);

		comment = ""; // Reset the comment string
		comment += "Activates (1) or de-activates (0) logging;";
		comment += "iteration's first timeout;";
		gpb.registerFileParameter<bool>(
			"doLogging" // The name of the variable
			, doLogging_
			, false // The default value
			, Gem::Common::VAR_IS_SECONDARY
			, comment
		);
	}

	/***************************************************************************/
	/** @brief Creates individuals of this type */
	virtual boost::shared_ptr<prod_type> getObject_(Gem::Common::GParserBuilder&, const std::size_t&) = 0;
	/** @brief Allows to act on the configuration options received from the configuration file */
	virtual void postProcess_(boost::shared_ptr<prod_type>&) = 0;

	execMode pm_; ///< Holds information about the desired parallelization mode

	boost::uint16_t nEvaluationThreads_; ///< The number of threads used for evaluations in multithreaded execution

	boost::posix_time::time_duration firstTimeOut_; ///< Maximum time frame for first individual
	double minWaitFactor_; ///< The minimum allowed wait factor
	double maxWaitFactor_; ///< The maximum allowed wait factor
	bool doLogging_; ///< Specifies whether arrival times of individuals should be logged
	bool boundlessWait_; ///< Indicates whether the retrieveItem call should wait for an unlimited amount of time
	double waitFactorIncrement_; ///< The amount by which the waitFactor_ may be incremented or decremented

	boost::shared_ptr<Gem::Common::GFactoryT<typename prod_type::individual_type> > contentCreatorPtr_; ///< Holds an object capable of producing objects of the desired type

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
