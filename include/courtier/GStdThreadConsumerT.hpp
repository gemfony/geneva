/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <type_traits>
#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>
#include <functional>

// Boost headers go here

#include <boost/cast.hpp>
#include <boost/lexical_cast.hpp>

// Geneva headers go here
#include "common/GThreadGroup.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GLogger.hpp"
#include "common/GErrorStreamer.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GBaseConsumerT.hpp"
#include "courtier/GProcessingContainerT.hpp"
#include "courtier/GWorkerT.hpp"

namespace Gem {
namespace Courtier {

/** @brief The default number of threads per worker if the number of hardware threads cannot be determined */
const std::uint16_t DEFAULTTHREADSPERWORKER = 4;

/******************************************************************************/
/**
 * A derivative of GBaseConsumerT<>, that processes items in separate threads.
 * Objects of this class can exist alongside a networked consumer, as the broker
 * accepts more than one consumer. You can thus use this class to aid networked
 * optimization, if the server has spare CPU cores that would otherwise run idle.
 * The class makes use of the template arguments' process() function.
 */
template<class processable_type>
class GStdThreadConsumerT
	: public Gem::Courtier::GBaseConsumerT<processable_type>
{
	 // Make sure processable_type adheres to the GProcessingContainerT interface
	 static_assert(
		 std::is_base_of<Gem::Courtier::GProcessingContainerT<processable_type, typename processable_type::result_type>, processable_type>::value
		 , "processable_type does not adhere to the GProcessingContainerT interface"
	 );

public:
	 /***************************************************************************/
	 /**
	  * Initialization with the number of threads. We want to enforce setting of
	  * this quantity upon creation.
	  */
	 explicit GStdThreadConsumerT(
	 	std::size_t nThreads = DEFAULTTHREADSPERWORKER
	 )
	 	: m_nThreads(nThreads>0 ? nThreads : DEFAULTTHREADSPERWORKER)
	 {
		 if(0 == nThreads) {
			 glogger
				 << "In GStdThreadConsumerT::GStdThreadConsumerT(nThreads):" << std::endl
				 << "nThreads == 0 was requested. m_n_threads was set to the default "
				 << DEFAULTTHREADSPERWORKER << std::endl
				 << GWARNING;
		 }
	 }

	 /***************************************************************************/

	 GStdThreadConsumerT(const GStdThreadConsumerT<processable_type> &) = delete; ///< Intentionally left undefined
	 GStdThreadConsumerT(GStdThreadConsumerT<processable_type> &&) = delete; ///< Intentionally left undefined
	 GStdThreadConsumerT<processable_type> &operator=(const GStdThreadConsumerT<processable_type> &) = delete; ///< Intentionally left undefined
	 GStdThreadConsumerT<processable_type> &operator=(GStdThreadConsumerT<processable_type> &&) = delete; ///< Intentionally left undefined

	 /***************************************************************************/
	 /**
	  * Standard destructor. Nothing - our threads receive the stop
	  * signal from the broker and shouldn't exist anymore at this point.
	  */
	 ~GStdThreadConsumerT() override = default;

	 /***************************************************************************/
	 /**
	 * Retrieves the maximum number of allowed threads
	 *
	 * @return The maximum number of allowed threads
	 */
	 std::size_t getNThreadsPerWorker(void) const {
		 return m_nThreads;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether a worker template was registered
	  */
	 bool hasWorkerTemplate() const {
		 if(this->m_workerTemplate) { return true; }
		 return false;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to specify whether this consumer is capable of full return
	  */
	 void setCapableOfFullReturn(bool capableOfFullReturn) {
		 glogger
			 << "In GStdThreadConsumerT<processable_type>::setCapableOfFullReturn():" << std::endl
			 << "m_capableOfFullReturn will be set to " << (capableOfFullReturn?"true":"false") << std::endl
			 << GLOGGING;
		 m_capableOfFullReturn = capableOfFullReturn;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to register a single worker template with this class.
	  */
	 void registerWorkerTemplate(
		 std::shared_ptr<GLocalConsumerWorkerT<processable_type>> workerTemplate
	 ) {
#ifdef DEBUG
		 if(not workerTemplate) { // Does the template point somewhere ?
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GStdThreadConsumerT<processable_type>::registerWorkerTemplate(): Error!" << std::endl
					 << "Found empty worker template pointer" << std::endl
			 );
		 }
#endif /* DEBUG */

		 m_workerTemplate = workerTemplate;
	 }

	 /***************************************************************************/
	 /**
	  * Sets up a consumer and registers it with the broker. This function accepts
	  * a worker as argument.
	  *
	  * @TODO Check if this is needed
	  */
//	 static void setup(
//		 const std::string &configFile,
//		 std::shared_ptr<GLocalConsumerWorkerT<processable_type>> worker_ptr
//	 ) {
//		 std::shared_ptr <GStdThreadConsumerT<processable_type>> consumer_ptr(
//			 new GStdThreadConsumerT<processable_type>()
//		 );
//
//		 consumer_ptr->registerWorkerTemplate(worker_ptr);
//		 consumer_ptr->parseConfigFile(configFile);
//
//		 GBROKER(processable_type)->enrol_consumer(consumer_ptr);
//	 }

protected:
	 /***************************************************************************/
	 /**
	 * Finalization code. Sends all threads an interrupt signal.
	 * process() then waits for them to join.
	 */
	 void shutdown_() override {
		 // Initiate the shutdown procedure
		 GBaseConsumerT<processable_type>::shutdown_();

		 // Wait for local workers to terminate
		 m_gtg.join_all();
		 m_workers.clear();
	 }

	 /***************************************************************************/
	 /**
	  * Adds local configuration options to a GParserBuilder object. We have only
	  * a single local option -- the number of threads
	  *
	  * @param gpb The GParserBuilder object, to which configuration options will be added
	  */
	 void addConfigurationOptions(
		 Gem::Common::GParserBuilder &gpb
	 ) override {
		 // Call our parent class'es function
		 GBaseConsumerT<processable_type>::addConfigurationOptions(gpb);

		 // Add local data
		 gpb.registerFileParameter<std::size_t>(
			 "threadsPerWorker" // The name of the variable
			 , 0 // The default value
			 , [this](std::size_t nt) { this->setNThreads(nt); }
		 )
			 << "Indicates the number of threads used to process each worker." << std::endl
			 << "Setting threadsPerWorker to 0 will result in an attempt to" << std::endl
			 << "automatically determine the number of hardware threads.";
	 }

private:
	 /***************************************************************************/
	 /**
	  * Adds local command line options to a boost::program_options::options_description object.
	  *
	  * @param visible Command line options that should always be visible
	  * @param hidden Command line options that should only be visible upon request
	  */
	 void addCLOptions_(
		 boost::program_options::options_description &visible
		 , boost::program_options::options_description &hidden
	 ) override {
		 namespace po = boost::program_options;

		 hidden.add_options()
			 ("nWorkerThreads", po::value<std::size_t>(&m_nThreads)->default_value(m_nThreads),
				 "\t[stc] The number of threads used to process the worker");

		 hidden.add_options()
			 ("stcCapableOfFullReturn", po::value<bool>(&m_capableOfFullReturn)->default_value(m_capableOfFullReturn),
				 "\t[stc] A debugging option making the multi-threaded consumer use timeouts in the executor");
	 }

	 /***************************************************************************/
	 /**
	  * Takes a boost::program_options::variables_map object and checks for supplied options.
	  */
	 void actOnCLOptions_(const boost::program_options::variables_map &vm) override
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Sets the number of threads. Note that this function
	  * will only have an effect before the threads have been started.
	  * If nThreads is set to 0, a warning will be printed and the
	  * number of threads will be set to the default value.
	  *
	  * @param nThreads The maximum number of allowed threads
	  */
	 void setNThreads(std::size_t nThreads) {
		 if (nThreads == 0) {
			 glogger
				 << "In GStdThreadConsumerT::setNThreads(nThreads):" << std::endl
				 << "nThreads == 0 was requested. nThreads was reset to the default "
				 << DEFAULTTHREADSPERWORKER << std::endl
				 << GWARNING;

			 m_nThreads = DEFAULTTHREADSPERWORKER;
		 }
		 else {
			 m_nThreads = nThreads;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * A unique identifier for a given consumer
	  *
	  * @return A unique identifier for a given consumer
	  */
	 std::string getConsumerName_() const override {
		 return std::string("GStdThreadConsumerT");
	 }

	 /***************************************************************************/
	 /**
	  * Returns a short identifier for this consumer
	  */
	 std::string getMnemonic_() const override {
		 return std::string("stc");
	 }

	 /***************************************************************************/
	 /**
	 * Starts the worker threads. This function will not block.
	 * Termination of the threads is triggered by a call to GBaseConsumerT<processable_type>::shutdown().
	 */
	 void async_startProcessing_() override {
		 // Add a default worker if no worker was registered
		 if(not m_workerTemplate) {
			 std::shared_ptr<GLocalConsumerWorkerT<processable_type>> default_worker(new GLocalConsumerWorkerT<processable_type>());
			 this->registerWorkerTemplate(default_worker);
		 }

		 // Start m_nWorkerThreads threads for each registered worker template
		 glogger
			 << "Starting " << m_nThreads << " processing threads in GStdThreadConsumerT<processable_type>" << std::endl
			 << GLOGGING;
		 for (std::size_t worker_id = 0; worker_id < m_nThreads; worker_id++) {
			 // The actual worker
			 std::shared_ptr<GLocalConsumerWorkerT<processable_type>> p_worker
				 = std::dynamic_pointer_cast<GLocalConsumerWorkerT<processable_type>>(m_workerTemplate->clone());

			 // The "broker ferry" holding the connection to the broker
			 std::shared_ptr<GBrokerFerryT<processable_type>> broker_ferry_ptr(
				 new GBrokerFerryT<processable_type>(
					 //----------------------
					 worker_id
					 //----------------------
					 , [this](
						 const std::chrono::milliseconds& timeout
					 ) -> std::shared_ptr<processable_type> {
						 std::shared_ptr<processable_type> p;
						 GBROKER(processable_type)->get(p, timeout);
						 return p;
					 }
					 //----------------------
					 , [this](
						 std::shared_ptr<processable_type> p
						 , const std::chrono::milliseconds& timeout
					 ) -> void { GBROKER(processable_type)->put(p, timeout); }
					 //----------------------
					 , [this]() -> bool { return this->stopped(); }
					 //----------------------
				 )
			 );

			 // Register the broker ferry with the worker
			 p_worker->registerBrokerFerry(broker_ferry_ptr);

			 // Start the actual thread
			 m_gtg.create_thread(
				 [p_worker]() -> void { p_worker->run(); }
			 );

			 // Store the worker for later reference
			 m_workers.push_back(p_worker);
		 }
	 }

	 /***************************************************************************/
	 /**
  	 * Returns the (possibly estimated) number of concurrent processing units.
    * A return value of 0 means "unknown". Note that this function does not
    * make any assumptions whether processing units are dedicated solely to a
    * given task.
    */
	 std::size_t getNProcessingUnitsEstimate_(bool& exact) const override {
		 // Mark the answer as exact
		 exact=true;
		 // Return the result
		 return boost::numeric_cast<std::size_t>(this->getNThreadsPerWorker());
	 }

	 /***************************************************************************/
	 /**
	  * Returns an indication whether full return can be expected from this
	  * consumer. Since evaluation is performed in threads, we assume that this
	  * is possible and return true. If you believe that this is not the case,
	  * make sure to set m_capableOfFullReturn to false using the setCapableOfFullReturn()
	  * function. Note that, while processing-errors will likely be caught,
	  * "full return" does not mean "fully processed return", as errors (be it in
	  * user- or Geneva-code) are always possible.
	  */
	 bool capableOfFullReturn_() const override {
		 return m_capableOfFullReturn;
	 }

	 /***************************************************************************/

	 bool m_capableOfFullReturn = true; ///< Indicates whether this consumer is capable of full return

	 std::size_t m_nThreads = DEFAULTTHREADSPERWORKER; ///< The maximum number of allowed threads in the pool
	 Gem::Common::GThreadGroup m_gtg; ///< Holds the processing threads

	 std::vector<std::shared_ptr<GLocalConsumerWorkerT<processable_type>>> m_workers; ///< Holds the current worker objects
	 std::shared_ptr<GLocalConsumerWorkerT<processable_type>> m_workerTemplate; ///< All workers will be created as a clone of this worker
};

/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

