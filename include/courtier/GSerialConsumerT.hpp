/**
 * @file GSerialConsumerT.hpp
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

#ifndef GSERIALCONSUMERT_HPP_
#define GSERIALCONSUMERT_HPP_

// Geneva headers go here
#include "common/GLogger.hpp"
#include "common/GExceptions.hpp"
#include "common/GErrorStreamer.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GBaseConsumerT.hpp"
#include "courtier/GWorkerT.hpp"

namespace Gem {
namespace Courtier {

/******************************************************************************/
/**
 * This class adds a serial consumer to the collection of consumers.
 * This allows to use a single implementation of the available
 * optimization algorithms with all available execution modes instead
 * of different implementations of the algorithms for each mode.
 */
template<class processable_type>
class GSerialConsumerT
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
	  * The default constructor
	  */
	 GSerialConsumerT() = default;

	 /***************************************************************************/
	 /**
	  * Standard destructor
	  */
	 virtual ~GSerialConsumerT() = default;

	 /***************************************************************************/
	 /**
	 * A unique identifier for a given consumer
	 *
	 * @return A unique identifier for a given consumer
	 */
	 virtual std::string getConsumerName() const override {
		 return std::string("GSerialConsumerT");
	 }

	 /***************************************************************************/
	 /**
	  * Returns a short identifier for this consumer
	  */
	 virtual std::string getMnemonic() const override {
		 return std::string("sc");
	 }

	 /***************************************************************************/
	 /**
	 * Finalization code. Sends all threads an interrupt signal.
	 */
	 virtual void shutdown() override {
		 // This will set the GBaseConsumerT<processable_type>::stop_ flag
		 GBaseConsumerT<processable_type>::shutdown();
		 // Wait for our local threads to join
		 m_processingThread.join();
	 }

	 /***************************************************************************/
	 /**
	  * Returns an indication whether full return can be expected from this
	  * consumer. Since evaluation is performed in a single thread, we assume that this
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
	  * Allows to specifcy whether this consumer is capable of full return
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
  	 * A return value of 0 means "unknown".
  	 */
	 virtual std::size_t getNProcessingUnitsEstimate(bool& exact) const override {
		 // Mark the answer as exact
		 exact=true;
		 // Return the result
		 return boost::numeric_cast<std::size_t>(1);
	 }

	 /***************************************************************************/
	 /**
	  * Starts a single worker thread. Termination of the thread is
	  * triggered by a call to GBaseConsumerT<processable_type>::shutdown().
	  */
	 virtual void async_startProcessing() override {
		 // Add a default worker if no worker was registered
		 if(!m_workerTemplate) {
			 std::shared_ptr<GLocalConsumerWorkerT<processable_type>> default_worker(new GLocalConsumerWorkerT<processable_type>());
			 this->registerWorkerTemplate(default_worker);
		 }

		 glogger
			 << "Starting single thread in GSerialConsumerT<processable_type>" << std::endl
			 << GLOGGING;

	    // The actual worker
		 std::shared_ptr<GLocalConsumerWorkerT<processable_type>> p_worker
			 = std::dynamic_pointer_cast<GLocalConsumerWorkerT<processable_type>>(m_workerTemplate->clone());

		 // The "broker ferry" holding the connection to the broker
		 std::shared_ptr<GBrokerFerryT<processable_type>> broker_ferry_ptr(
			 new GBrokerFerryT<processable_type>(
				 //----------------------
				 std::size_t(0) // we only have one worker
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

		 m_processingThread = std::move(std::thread(
			 [p_worker]() -> void { p_worker->run(); }
		 ));

		 // Store the worker for later reference
		 m_worker = p_worker;
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
					 << "In GSerialConsumerT<processable_type>::registerWorkerTemplate(): Error!" << std::endl
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
		 std::shared_ptr <GSerialConsumerT<processable_type>> consumer_ptr(
			 new GSerialConsumerT<processable_type>()
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
		 // ... no local configuration
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
			 ("scCapableOfFullReturn", po::value<bool>(&m_capableOfFullReturn)->default_value(m_capableOfFullReturn),
				 "\t[sc] A debugging option making the multi-threaded consumer use timeouts in the executor");
	 }

	 /***************************************************************************/
	 /**
	  * Takes a boost::program_options::variables_map object and checks for supplied options.
	  */
	 virtual void actOnCLOptions(const boost::program_options::variables_map &vm) override
	 { /* nothing */ }

private:
	 /***************************************************************************/

	 GSerialConsumerT(const GSerialConsumerT<processable_type> &) = delete; ///< Intentionally left undefined
	 const GSerialConsumerT<processable_type> &operator=(const GSerialConsumerT<processable_type> &) = delete; ///< Intentionally left undefined


	 /***************************************************************************/

	 std::thread m_processingThread; ///< A single thread holding the worker

	 bool m_capableOfFullReturn = true; ///< Indicates whether this consumer is capable of full return

	 std::shared_ptr<GLocalConsumerWorkerT<processable_type>> m_worker; ///< Holds the worker assigned to this consumer
	 std::shared_ptr<GLocalConsumerWorkerT<processable_type>> m_workerTemplate; ///< Holds an external worker assigned to this consumer

	 std::shared_ptr<GBrokerT<processable_type>> m_broker_ptr = GBROKER(processable_type); ///< A shortcut to the broker so we do not have to go through the singleton
};

/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GSERIALCONSUMERT_HPP_ */
