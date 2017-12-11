/**
 * @file GStdThreadConsumerT.hpp
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
#include <type_traits>
#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>
#include <functional>

// Boost headers go here

#include <boost/cast.hpp>
#include <boost/lexical_cast.hpp>

#ifndef GSTDTHREADCONSUMERT_HPP_
#define GSTDTHREADCONSUMERT_HPP_

// Geneva headers go here
#include "common/GStdThreadGroup.hpp"
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
const std::uint16_t DEFAULTTHREADSPERWORKER = 1;

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
	  * The default constructor.
	  */
	 GStdThreadConsumerT()
		 : Gem::Courtier::GBaseConsumerT<processable_type>()
		 , m_nWorkerThreads(boost::numeric_cast<std::size_t>(Gem::Common::getNHardwareThreads(DEFAULTTHREADSPERWORKER)))
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	 * Standard destructor. Nothing - our threads receive the stop
	 * signal from the broker and shouldn't exist at this point anymore.
	 */
	 virtual ~GStdThreadConsumerT() { /* nothing */ }

	 /***************************************************************************/
	 /**
	 * Sets the number of threads per worker. Note that this function
	 * will only have an effect before the threads have been started.
	 * If threadsPerWorker is set to 0, an attempt will be made to automatically
	 * determine a suitable number of threads.
	 *
	 * @param tpw The maximum number of allowed threads
	 */
	 void setNThreadsPerWorker(const std::size_t &tpw) {
		 if (tpw == 0) {
			 m_nWorkerThreads = boost::numeric_cast<std::size_t>(
				 Gem::Common::getNHardwareThreads(DEFAULTTHREADSPERWORKER));
		 }
		 else {
			 m_nWorkerThreads = tpw;
		 }
	 }

	 /***************************************************************************/
	 /**
	 * Retrieves the maximum number of allowed threads
	 *
	 * @return The maximum number of allowed threads
	 */
	 std::size_t getNThreadsPerWorker(void) const {
		 return m_nWorkerThreads;
	 }

	 /***************************************************************************/
	 /**
	 * Finalization code. Sends all threads an interrupt signal.
	 * process() then waits for them to join.
	 */
	 virtual void shutdown() override {
		 // Initiate the shutdown procedure
		 GBaseConsumerT<processable_type>::shutdown();

		 // Wait for local workers to terminate
		 m_gtg.join_all();
		 m_workers.clear();
	 }

	 /***************************************************************************/
	 /**
	 * A unique identifier for a given consumer
	 *
	 * @return A unique identifier for a given consumer
	 */
	 virtual std::string getConsumerName() const override {
		 return std::string("GStdThreadConsumerT");
	 }

	 /***************************************************************************/
	 /**
	  * Returns a short identifier for this consumer
	  */
	 virtual std::string getMnemonic() const override {
		 return std::string("stc");
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
	  * Returns an indication whether full return can be expected from this
	  * consumer. Since evaluation is performed in threads, we assume that this
	  * is possible and return true. If you believe that this is not the case,
	  * make sure to set m_capableOfFullReturn to false using the setCapableOfFullReturn()
	  * function. Note that, while processing-errors will likely be caught,
	  * "full return" does not mean "fully processed return", as errors (be it in
	  * user- or Geneva-code) are always possible.
	  */
	 virtual bool capableOfFullReturn() const override {
		 return m_capableOfFullReturn;
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
  	 * Returns the (possibly estimated) number of concurrent processing units.
    * A return value of 0 means "unknown". Note that this function does not
    * make any assumptions whether processing units are dedicated solely to a
    * given task.
    */
	 virtual std::size_t getNProcessingUnitsEstimate(bool& exact) const override {
		 // Mark the answer as exact
		 exact=true;
		 // Return the result
		 return boost::numeric_cast<std::size_t>(this->getNThreadsPerWorker());
	 }

	 /***************************************************************************/
	 /**
	 * Starts the worker threads. This function will not block.
	 * Termination of the threads is triggered by a call to GBaseConsumerT<processable_type>::shutdown().
	 */
	 virtual void async_startProcessing() override {
		 // Add a default worker if no worker was registered
		 if(!m_workerTemplate) {
			 std::shared_ptr<GLocalConsumerWorkerT<processable_type>> default_worker(new GLocalConsumerWorkerT<processable_type>());
			 this->registerWorkerTemplate(default_worker);
		 }

		 // Start m_nWorkerThreads threads for each registered worker template
		 glogger
			 << "Starting " << m_nWorkerThreads << " processing threads in GStdThreadConsumerT<processable_type>" << std::endl
			 << GLOGGING;
		 for (std::size_t worker_id = 0; worker_id < m_nWorkerThreads; worker_id++) {
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
						m_broker_ptr->get(p, timeout);
						return p;
					}
					//----------------------
					, [this](
						std::shared_ptr<processable_type> p
						, const std::chrono::milliseconds& timeout
					) -> void { m_broker_ptr->put(p, timeout); }
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
	  * Allows to register a single worker template with this class.
	  */
	 void registerWorkerTemplate(
		 std::shared_ptr<GLocalConsumerWorkerT<processable_type>> workerTemplate
	 ) {
#ifdef DEBUG
		 if(!workerTemplate) { // Does the template point somewhere ?
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
	  */
	 static void setup(
		 const std::string &configFile,
		 std::shared_ptr<GLocalConsumerWorkerT<processable_type>> worker_ptr
	 ) {
		 std::shared_ptr <GStdThreadConsumerT<processable_type>> consumer_ptr(
			 new GStdThreadConsumerT<processable_type>()
		 );

		 consumer_ptr->registerWorkerTemplate(worker_ptr);
		 consumer_ptr->parseConfigFile(configFile);

		 GBROKER(processable_type)->enrol(consumer_ptr);
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Adds local configuration options to a GParserBuilder object. We have only
	  * a single local option -- the number of threads
	  *
	  * @param gpb The GParserBuilder object, to which configuration options will be added
	  */
	 virtual void addConfigurationOptions(
		 Gem::Common::GParserBuilder &gpb
	 ) override {
		 // Call our parent class'es function
		 GBaseConsumerT<processable_type>::addConfigurationOptions(gpb);

		 // Add local data
		 gpb.registerFileParameter<std::size_t>(
			 "threadsPerWorker" // The name of the variable
			 , 0 // The default value
			 , [this](std::size_t nt) { this->setNThreadsPerWorker(nt); }
		 )
			 << "Indicates the number of threads used to process each worker." << std::endl
			 << "Setting threadsPerWorker to 0 will result in an attempt to" << std::endl
			 << "automatically determine the number of hardware threads.";
	 }

	 /***************************************************************************/
	 /**
	  * Adds local command line options to a boost::program_options::options_description object.
	  *
	  * @param visible Command line options that should always be visible
	  * @param hidden Command line options that should only be visible upon request
	  */
	 virtual void addCLOptions(
		 boost::program_options::options_description &visible, boost::program_options::options_description &hidden
	 ) override {
		 namespace po = boost::program_options;

		 hidden.add_options()
			 ("nWorkerThreads", po::value<std::size_t>(&m_nWorkerThreads)->default_value(m_nWorkerThreads),
				 "\t[stc] The number of threads used to process the worker");

		 hidden.add_options()
			 ("stcCapableOfFullReturn", po::value<bool>(&m_capableOfFullReturn)->default_value(m_capableOfFullReturn),
			 	"\t[stc] A debugging option making the multi-threaded consumer use timeouts in the executor");
	 }

	 /***************************************************************************/
	 /**
	  * Takes a boost::program_options::variables_map object and checks for supplied options.
	  */
	 virtual void actOnCLOptions(const boost::program_options::variables_map &vm) override
	 { /* nothing */ }

private:
	 /***************************************************************************/

	 GStdThreadConsumerT(const GStdThreadConsumerT<processable_type> &) = delete; ///< Intentionally left undefined
	 GStdThreadConsumerT<processable_type> &operator=(const GStdThreadConsumerT<processable_type> &) = delete; ///< Intentionally left undefined

	 bool m_capableOfFullReturn = true; ///< Indicates whether this consumer is capable of full return

	 std::size_t m_nWorkerThreads; ///< The maximum number of allowed threads in the pool
	 Gem::Common::GStdThreadGroup m_gtg; ///< Holds the processing threads

	 std::vector<std::shared_ptr<GLocalConsumerWorkerT<processable_type>>> m_workers; ///< Holds the current worker objects
	 std::shared_ptr<GLocalConsumerWorkerT<processable_type>> m_workerTemplate; ///< All workers will be created as a clone of this worker

	 std::shared_ptr<GBrokerT<processable_type>> m_broker_ptr = GBROKER(processable_type); ///< A shortcut to the broker so we do not have to go through the singleton
};

/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GSTDTHREADCONSUMERT_HPP_ */
