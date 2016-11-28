/**
 * @file GWebSocketConsumerT.hpp
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

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <array>
#include <regex>

// Boost headers go here
#include <boost/asio.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/fusion/include/tuple.hpp>
#include <boost/fusion/include/boost_tuple.hpp>
#include <boost/fusion/adapted/boost_tuple.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_lit.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_serialize.hpp>

// Thirdparty websocket library headers go here
#include "thirdparty/WebSocket-Server-Eidheim/client_ws.hpp"
#include "thirdparty/WebSocket-Server-Eidheim/server_ws.hpp"

#ifndef GWEBSOCKETCONSUMERT_HPP_
#define GWEBSOCKETCONSUMERT_HPP_

// Geneva headers go here
#include "common/GThreadGroup.hpp"
#include "common/GThreadPool.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GHelperFunctions.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "courtier/GAsioHelperFunctions.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GBaseConsumerT.hpp"
#include "courtier/GBaseClientT.hpp"

namespace Gem {
namespace Courtier {

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
using WsClient = SimpleWeb::SocketClient<SimpleWeb::WS>;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The client code used for parallel / distributed execution via Websockets.
 * Note that this class isn't copyable, as the base class is derived from
 * boost::noncopyable.
 */
template<typename processable_type>
class GWebSocketClientT
	: public Gem::Courtier::GBaseClientT<processable_type>
{
public:
	 /***************************************************************************/
	 /**
	  * Initialization by server name/ip, port and endpoint on the server
	  *
	  * @param server Identifies the server
	  * @param port Identifies the port on the server
	  * @param endpoint The "path" of the websocket server
	  */
	 GWebSocketClientT(
		 const std::string &server
		 , const std::string &port
		 , const std::string &endpoint
	 ) :
		 GBaseClientT<processable_type>()
		 , m_client(server + ":" + port + "/" + endpoint)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initialization by server name/ip, port and a model for the item to
	  * be processed, as well as static data which doesn't change for clients.
	  *
	  * @param server Identifies the server
	  * @param port Identifies the port on the server
	  * @param endpoint The "path" of the websocket server
	  * @param additionalDataTemplate Any additional data to be made available to the client
	  */
	 GWebSocketClientT(
		 const std::string &server
		 , const std::string &port
		 , const std::string &endpoint
		 , std::shared_ptr<processable_type> additionalDataTemplate
	 ) :
		 GBaseClientT<processable_type>(additionalDataTemplate)
		 , m_client(server + ":" + port + "/" + endpoint)
	 	 , m_server(server)
	 	 , m_port(port)
	 	 , m_endpoint(endpoint)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The standard destructor.
	  */
	 virtual ~GWebSocketClientT() {
		 // Stop the websocket client
		 m_client.stop();
	 }

	 /***************************************************************************/
	 /**
	  * This is the main loop of the client. It will take care to initiate the first
	  * connection to the server and will then wait for the event loop to terminate
	  * (which will usually be initiated by a call to onClose() through the websocket
	  * library).
	  *
	  * TODO: Restart connection a number of times in case of errors or if
	  * the client code has crashed
	  */
	 virtual void run() override {
		 // Start the actual websocket client. We do this in a
		 // thread so we may wait asynchronously for its termination
		 std::thread t([&]() -> void {
			 m_client.start()
		 });

		 // Wait for the client to terminate
		 t.join();
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Code to be executed when a new connection is established. As a first
	  * action, the client will ask the server for wor.
	  */
	 void onOpen() {
		 glogger
		 << "Client: Closed opened connection to \"" << (m_server + ":" + m_port + "/" + m_endpoint) << "\"" << std::endl
		 << "Asking server for work" << std::endl
		 << GLOGGING;

		 this->sendCommand("ready");
	 }

	 /***************************************************************************/
	 /**
	  * Code to be executed when the connection is closed
	  */
	 void onClose(
		 int status
		 , const std::string& reason
	 ) {
		 glogger
		 << "Client: Closed connection with status code \"" << status << "\"" << std::endl
		 << GLOGGING;
	 }

	 /***************************************************************************/
	 /**
	  * Code to be executed when a message was received. Apart from dealing with
	  * the message and responding to it it must also check the termination
	  * criteria of the client.
	  */
	 void onMessage(std::shared_ptr<WsClient::Message> message_ptr) {
		 // First retrieve the message
		 auto message_str=message->string();

		 // A regular expression for the "compute" command received from the server
		 std::regex compute_regex(R"(^compute\((XML|TEXT|BINARY)\)\((.+)\)$)");
		 // A regular expression for an idle period (in milliseconds)
		 std::regex idle_regex(R"(^idle\(([0-9]+)\)$)");

		 // Act on the message
		 std::smatch sub_matches;
		 if(std::regex_match(message_str, sub_matches, idle_regex)) {
			 // Retrieve the idle time
			 for(auto m: sub_matches) {
				 std::cout << m << std::endl;
			 }

			 // If we have been asked to be quiet, start a deadline timer?
			 // Or wait asynchronously and then ask for work later ?
		 } else if(std::regex_match(message_str, sub_matches, compute_regex)) {
			 // Retrieve the serialization mode and the serialization string
			 for(auto m: sub_matches) {
				 std::cout << m << std::endl;
			 }
		 } else {
			 // Received an unknown command -- complain
			 std::cout
				 << "-------------------------------------------" << std::endl
				 << "Got unknown command " << message_str << std::endl;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Code to be executed in case of errors -- let the audience know. Code-paths
	  * resulting in a call to "onerror" will lead to a termination of the event
	  * loop. Error code meanings may be found at
	  * http://www.boost.org/doc/libs/1_62_0/doc/html/boost_asio/reference.html
	  *
	  * TODO: Use a more differentiated response
	  */
	 void onError(const boost::system::error_code& ec) {
	    glogger
		 << "Client: Error \"" << ec << "\" with error message \"" << ec.message() << "\"" << std::endl
		 << GWARNING;
	 }

private:
	 /***************************************************************************/
	 /**
	  * Sends a specific command to the server
	  */
	 void sendCommand(const std::string& command) {
		 auto send_stream=std::make_shared<WsClient::SendStream>();
		 *send_stream << command;
		 m_client.send(send_stream);
	 }

	 /***************************************************************************/
    // Data

	 WsClient m_client; ///< The core websocket implementation

	 std::string m_server; ///< The name or ip of the server
	 std::string m_port; ///< The port used by the server
	 std::string m_endpoint; ///< The endpoint used by the server
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A derivative of GBaseConsumerT<>, that processes items in separate threads.
 * Objects of this class can exist alongside a networked consumer, as the broker
 * accepts more than one consumer. You can thus use this class to aid networked
 * optimization, if the server has spare CPU cores that would otherwise run idle.
 * The class makes use of the template arguments' process() function.
 */
template <class processable_type>
class GWebSocketConsumerT
	:public Gem::Courtier::GBaseConsumerT<processable_type>
{
private:
	 // Make sure processable_type adheres to the GSubmissionContainerT interface
	 BOOST_MPL_ASSERT((boost::is_base_of<Gem::Courtier::GSubmissionContainerT<processable_type>, processable_type>));

public:
	 class GWorker; ///< Forward declaration
	 class GDefaultWorker; ///< Forward declaration

	 /***************************************************************************/
	 /**
	  * The default constructor.
	  */
	 GWebSocketConsumerT()
		 : Gem::Courtier::GBaseConsumerT<processable_type>()
			, threadsPerWorker_(boost::numeric_cast<std::size_t>(Gem::Common::getNHardwareThreads(DEFAULTTHREADSPERWORKER)))
			, broker_ptr_(GBROKER(processable_type))
			, workerTemplates_(1, boost::shared_ptr<GWorker>(new GDefaultWorker()))
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	 * Standard destructor. Nothing - our threads receive the stop
	 * signal from the broker and shouldn't exist at this point anymore.
	 */
	 virtual ~GWebSocketConsumerT()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	 * Sets the number of threads per worker. Note that this function
	 * will only have an effect before the threads have been started.
	 * If threadsPerWorker is set to 0, an attempt will be made to automatically
	 * determine a suitable number of threads.
	 *
	 * @param tpw The maximum number of allowed threads
	 */
	 void setNThreadsPerWorker(const std::size_t& tpw) {
		 if(tpw == 0) {
			 threadsPerWorker_ = boost::numeric_cast<std::size_t>(Gem::Common::getNHardwareThreads(DEFAULTTHREADSPERWORKER));
		 }
		 else {
			 threadsPerWorker_ = tpw;
		 }
	 }

	 /***************************************************************************/
	 /**
	 * Retrieves the maximum number of allowed threads
	 *
	 * @return The maximum number of allowed threads
	 */
	 std::size_t getNThreadsPerWorker(void) const  {
		 return threadsPerWorker_;
	 }

	 /***************************************************************************/
	 /**
	 * Finalization code. Sends all threads an interrupt signal.
	 * process() then waits for them to join.
	 */
	 void shutdown() {
		 // Initiate the shutdown procedure
		 GBaseConsumerT<processable_type>::shutdown();

		 // Wait for local workers to terminate
		 gtg_.join_all();
		 workers_.clear();
	 }

	 /***************************************************************************/
	 /**
	 * A unique identifier for a given consumer
	 *
	 * @return A unique identifier for a given consumer
	 */
	 virtual std::string getConsumerName() const {
		 return std::string("GWebSocketConsumerT");
	 }

	 /***************************************************************************/
	 /**
	  * Returns a short identifier for this consumer
	  */
	 virtual std::string getMnemonic() const {
		 return std::string("btc");
	 }

	 /***************************************************************************/
	 /**
	  * Returns an indication whether full return can be expected from this
	  * consumer. Since evaluation is performed in threads, we assume that this
	  * is possible and return true.
	  */
	 virtual bool capableOfFullReturn() const {
		 return true;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the number of workers registered with this class
	  */
	 std::size_t getNWorkers() const {
		 return workerTemplates_.size();
	 }

	 /***************************************************************************/
	 /**
	 * Starts the worker threads. This function will not block.
	 * Termination of the threads is triggered by a call to GBaseConsumerT<processable_type>::shutdown().
	 */
	 virtual void async_startProcessing() {
#ifdef DEBUG
		 if(workerTemplates_.empty()) { // Is the template vector empty ?
         glogger
         << "In GWebSocketConsumerT<processable_type>::async_startProcessing(): Error!" << std::endl
         << "The workerTemplates_ vector is empty when it should not be empty" << std::endl
         << GEXCEPTION;
      }
#endif /* DEBUG */

		 // Start threadsPerWorker_ threads for each registered worker template
		 for(std::size_t w=0; w<workerTemplates_.size(); w++) {
			 for(std::size_t i=0; i<threadsPerWorker_; i++) {
				 boost::shared_ptr<GWorker> p_worker = (workerTemplates_.at(w))->clone(i,this);
				 gtg_.create_thread(boost::bind(&GWebSocketConsumerT<processable_type>::GWorker::run, p_worker));
				 workers_.push_back(p_worker);
			 }
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Allows to register a set of worker templates with this class. Note that all
	  * existing worker templates will be deleted. The class will not take ownership
	  * of the worker templates.
	  */
	 void registerWorkerTemplates(const std::vector<boost::shared_ptr<GWorker> >& workerTemplates) {
#ifdef DEBUG
		 if(workerTemplates_.empty()) { // Is the template vector empty ?
         glogger
         << "In GWebSocketConsumerT<processable_type>::registerWorkerTemplates(): Error!" << std::endl
         << "workerTemplates vector is empty when it should not be empty" << std::endl
         << GEXCEPTION;
      }

      for(std::size_t i=0; i<workerTemplates.size(); i++) {
         if(!workerTemplates.at(i)) { // Does the template point somewhere ?
            glogger
            << "In GWebSocketConsumerT<processable_type>::registerWorkerTemplates(): Error!" << std::endl
            << "Found empty worker template pointer in position " << i << std::endl
            << GEXCEPTION;
         }
      }
#endif /* DEBUG */

		 workerTemplates_.clear();
		 workerTemplates_ = workerTemplates;

		 assert(workerTemplates.size() == workerTemplates_.size());
	 }

	 /***************************************************************************/
	 /**
	  * Allows to register a single worker template with this class. Note that all
	  * existing worker templates will be deleted. The class will not take ownership
	  * of the worker template.
	  */
	 void registerWorkerTemplate(boost::shared_ptr<GWorker> workerTemplate) {
#ifdef DEBUG
		 if(!workerTemplate) { // Does the template point somewhere ?
         glogger
         << "In GWebSocketConsumerT<processable_type>::registerWorkerTemplate(): Error!" << std::endl
         << "Found empty worker template pointer" << std::endl
         << GEXCEPTION;
      }
#endif /* DEBUG */

		 workerTemplates_.clear();
		 workerTemplates_.push_back(workerTemplate);
	 }

	 /***************************************************************************/
	 /**
	  * Sets up a consumer and registers it with the broker. This function accepts
	  * a set of workers as argument.
	  */
	 static void setup(
		 const std::string& configFile
		 , std::vector<boost::shared_ptr<Gem::Courtier::GWebSocketConsumerT<processable_type>::GWorker> > workers
	 ) {
		 boost::shared_ptr<GWebSocketConsumerT<processable_type> > consumer_ptr(new GWebSocketConsumerT<processable_type>());
		 consumer_ptr->registerWorkerTemplates(workers);
		 consumer_ptr->parseConfigFile(configFile);
		 GBROKER(processable_type)->enrol(consumer_ptr);
	 }

	 /***************************************************************************/
	 /**
	  * Sets up a consumer and registers it with the broker. This function accepts
	  * a worker as argument.
	  */
	 static void setup(
		 const std::string& configFile
		 , boost::shared_ptr<Gem::Courtier::GWebSocketConsumerT<processable_type>::GWorker> worker_ptr
	 ) {
		 boost::shared_ptr<GWebSocketConsumerT<processable_type> > consumer_ptr(new GWebSocketConsumerT<processable_type>());
		 consumer_ptr->registerWorkerTemplate(worker_ptr);
		 consumer_ptr->parseConfigFile(configFile);
		 GBROKER(processable_type)->enrol(consumer_ptr);
	 }

	 /***************************************************************************/
	 /**
	  * Sets up a consumer and registers it with the broker. This function uses
	  * the default worker.
	  */
	 static void setup(
		 const std::string& configFile
	 ) {
		 boost::shared_ptr<GWebSocketConsumerT<processable_type> > consumer_ptr(new GWebSocketConsumerT<processable_type>());
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
	  * @param showOrigin Indicates, whether the origin of a configuration option should be shown in the configuration file
	  */
	 virtual void addConfigurationOptions(
		 Gem::Common::GParserBuilder& gpb
		 , const bool& showOrigin
	 ){
		 std::string comment;

		 // Call our parent class'es function
		 GBaseConsumerT<processable_type>::addConfigurationOptions(gpb, showOrigin);

		 // Add local data
		 comment = ""; // Reset the comment string
		 if(showOrigin) {
			 comment += "[Origin] GWebSocketConsumerT<processable_type>;";
			 comment += (std::string("with typeid(processable_type).name() = ") + typeid(processable_type).name() + ";");
		 }
		 comment += "Indicates the number of threads used to process each worker.;";
		 comment += "Setting threadsPerWorker to 0 will result in an attempt to;";
		 comment += "automatically determine the number of hardware threads.";
		 if(showOrigin) comment += "[GWebSocketConsumerT<>]";
		 gpb.registerFileParameter<std::size_t>(
			 "threadsPerWorker" // The name of the variable
			 , 0 // The default value
			 , boost::bind(
				 &GWebSocketConsumerT<processable_type>::setNThreadsPerWorker
				 , this
				 , _1
			 )
			 , Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
			 , comment
		 );
	 }

	 /***************************************************************************/
	 /**
	  * Adds local command line options to a boost::program_options::options_description object.
	  *
	  * @param visible Command line options that should always be visible
	  * @param hidden Command line options that should only be visible upon request
	  */
	 virtual void addCLOptions(
		 boost::program_options::options_description& visible
		 , boost::program_options::options_description& hidden
	 ) {
		 namespace po = boost::program_options;

		 hidden.add_options()
			 ("threadsPerWorker", po::value<std::size_t>(&threadsPerWorker_)->default_value(threadsPerWorker_), "\t[btc] The number of threads used to process each worker")
			 ;
	 }

	 /***************************************************************************/
	 /**
	  * Takes a boost::program_options::variables_map object and checks for supplied options.
	  */
	 virtual void actOnCLOptions(const boost::program_options::variables_map& vm)
	 { /* nothing */ }

private:
	 /***************************************************************************/

	 GWebSocketConsumerT(const GWebSocketConsumerT<processable_type>&); ///< Intentionally left undefined
	 const GWebSocketConsumerT<processable_type>& operator=(const GWebSocketConsumerT<processable_type>&); ///< Intentionally left undefined

	 std::size_t threadsPerWorker_; ///< The maximum number of allowed threads in the pool
	 Gem::Common::GThreadGroup gtg_; ///< Holds the processing threads
	 boost::shared_ptr<GBrokerT<processable_type> > broker_ptr_; ///< A shortcut to the broker so we do not have to go through the singleton
	 std::vector<boost::shared_ptr<GWorker> > workers_; ///< Holds the current worker objects
	 std::vector<boost::shared_ptr<GWorker> > workerTemplates_; ///< All workers will be created as a clone of these workers

public:
	 /***************************************************************************/
	 /**
	  * A nested class that performs the actual work inside of a thread. It is
	  * meant as a means to  Classes derived from GWebSocketConsumerT
	  * may use their own derivative from this class and store complex
	  * information associated with the execution inside of the worker threads.
	  * Note that a GWorker(-derivative) must be copy-constructible and implement
	  * the clone() function.
	  */
	 class GWorker {
	 public:
		  /************************************************************************/
		  /**
			* The default constructor
			*/
		  GWorker()
			  : thread_id_(0)
				 , outer_(NULL)
				 , parsed_(false)
				 , runLoopHasCommenced_(false)
		  { /* nothing */ }

	 protected:
		  /************************************************************************/
		  /**
			* The copy constructor. We do not copy the thread id, as it is set by
			* async_startprocessing().
			*/
		  GWorker(
			  const GWorker& cp
			  , const std::size_t& thread_id
			  , const GWebSocketConsumerT<processable_type> *c_ptr
		  )
			  : thread_id_(thread_id)
				 , outer_(c_ptr)
				 , parsed_(cp.parsed_)
				 , runLoopHasCommenced_(false)
		  { /* nothing */ }

	 public:
		  /************************************************************************/
		  /**
			* The destructor
			*/
		  virtual ~GWorker()
		  { /* nothing */ }

		  /************************************************************************/
		  /**
			* The main entry point for the execution
			*/
		  void run() {
			  try{
				  runLoopHasCommenced_=false;

				  boost::shared_ptr<processable_type> p;
				  Gem::Common::PORTIDTYPE id;
				  boost::posix_time::time_duration timeout(boost::posix_time::milliseconds(10));

				  while(true){
					  // Have we been asked to stop ?
					  if(outer_->stopped()) break;

					  // If we didn't get a valid item, start again with the while loop
					  if(!outer_->broker_ptr_->get(id, p, timeout)) {
						  continue;
					  }

#ifdef DEBUG
					  // Check that we indeed got a valid item
               if(!p) { // We didn't get a valid item after all
                  glogger
                  << "In GWebSocketConsumerT<processable_type>::GWorker::run(): Error!" << std::endl
                  << "Got empty item when it shouldn't be empty!" << std::endl
                  << GEXCEPTION;
               }
#endif /* DEBUG */

					  // Perform setup work once for the loop, as soon as we have
					  // a processable item. Such setup work might require information
					  // from that item, so we pass it to the function.
					  if(!runLoopHasCommenced_) {
						  processInit(p);
						  runLoopHasCommenced_ = true;
					  }

					  // Initiate the actual processing
					  this->process(p);

					  // Return the item to the broker. The item will be discarded
					  // if the requested target queue cannot be found.
					  try {
						  while(!outer_->broker_ptr_->put(id, p, timeout)){ // This can lead to a loss of items
							  // Terminate if we have been asked to stop
							  if(outer_->stopped()) break;
						  }
					  } catch (Gem::Courtier::buffer_not_present&) {
						  continue;
					  }
				  }
			  } catch(boost::thread_interrupted&){ // Normal termination
				  // Perform any final work
				  processFinalize();
				  return;
			  } catch(std::exception& e) {
				  std::ostringstream error;
				  error << "In GWebSocketConsumerT<processable_type>::GWorker::run():" << std::endl
						  << "Caught std::exception with message" << std::endl
						  << e.what() << std::endl;
				  std::cerr << error.str();
				  std::terminate();
			  }
			  catch(boost::exception& e){
				  std::ostringstream error;
				  error << "In GWebSocketConsumerT<processable_type>::GWorker::run():" << std::endl
						  << "Caught boost::exception with message" << std::endl;
				  std::cerr << error.str();
				  std::terminate();
			  }
			  catch(...) {
				  std::ostringstream error;
				  error << "In GWebSocketConsumerT<processable_type>::GWorker::run():" << std::endl
						  << "Caught unknown exception." << std::endl;
				  std::cerr << error.str();
				  std::terminate();
			  }
		  }

		  /************************************************************************/
		  /**
			* Retrieve this class'es id
			*/
		  std::size_t getThreadId() const {
			  return thread_id_;
		  }

		  /************************************************************************/
		  /**
			* Parses a given configuration file. Note that parsing is done but once.
			*
			* @param configFile The name of a configuration file
			*/
		  void parseConfigFile(const std::string& configFile) {
			  if(parsed_) return;

			  // Create a parser builder object -- local options will be added to it
			  Gem::Common::GParserBuilder gpb;

			  // Add configuration options of this and of derived classes
			  addConfigurationOptions(gpb, true);

			  // Do the actual parsing. Note that this
			  // will try to write out a default configuration file,
			  // if no existing config file can be found
			  gpb.parseConfigFile(configFile);

			  parsed_ = true;
		  }

	 protected:
		  /************************************************************************/
		  /**
			* Initialization code for processing. Can be specified in derived classes.
			*
			* @param p A pointer to a processable item meant to allow item-based setup
			*/
		  virtual void processInit(boost::shared_ptr<processable_type> p)
		  { /* nothing */ }

		  /************************************************************************/
		  /**
			* Finalization code for processing. Can be specified in derived classes.
			*/
		  virtual void processFinalize()
		  { /* nothing */ }

		  /************************************************************************/
		  /**
			* Adds local configuration options to a GParserBuilder object. We have no local
			* data, hence this function is empty. It could have been declared purely virtual,
			* however, we do not want to force derived classes to implement this function,
			* as it might not always be needed.
			*
			* @param gpb The GParserBuilder object, to which configuration options will be added
			* @param showOrigin Indicates, whether the origin of a configuration option should be shown in the configuration file
			*/
		  virtual void addConfigurationOptions(
			  Gem::Common::GParserBuilder& gpb
			  , const bool& showOrigin
		  ){ /* nothing -- no local data */ }

		  /************************************************************************/

		  std::size_t thread_id_; ///< The id of the thread running this class'es operator()
		  const GWebSocketConsumerT<processable_type> * outer_;

	 private:
		  /************************************************************************/

		  bool parsed_; ///< Indicates whether parsing has been completed
		  bool runLoopHasCommenced_; ///< Allows to check whether the while loop inside of the run function has started

	 public:
		  /************************************************************************/
		  // Some purely virtual functions

		  /** @brief Creation of deep clones of this object('s derivatives) */
		  virtual boost::shared_ptr<GWorker> clone(
			  const std::size_t&
			  , const GWebSocketConsumerT<processable_type> *
		  ) const = 0;
		  /** @brief Actual per-item work is done here -- Implement this in derived classes */
		  virtual void process(boost::shared_ptr<processable_type> p) = 0;
	 };

	 /***************************************************************************/
	 /**
	  * The default derivative of GWorker that is used when no other worker has
	  * been registered with our outer class.
	  */
	 class GDefaultWorker
		 : public GWebSocketConsumerT<processable_type>::GWorker
	 {
	 public:
		  /************************************************************************/
		  /**
			* The default constructor
			*/
		  GDefaultWorker() : GWorker()
		  { /* nothing */ }

	 protected:
		  /************************************************************************/
		  /**
			* The copy constructor.
			*/
		  GDefaultWorker(
			  const GDefaultWorker& cp
			  , const std::size_t& thread_id
			  , const GWebSocketConsumerT<processable_type> *outer
		  ) : GWorker(cp, thread_id, outer)
		  { /* nothing */ }

	 public:
		  /************************************************************************/
		  /**
			* The destructor
			*/
		  virtual ~GDefaultWorker()
		  { /* nothing */ }

		  /************************************************************************/
		  /**
			* Create a deep clone of this object, camouflaged as a GWorker
			*/
		  virtual boost::shared_ptr<GWorker> clone(
			  const std::size_t& thread_id
			  , const GWebSocketConsumerT<processable_type> *outer
		  ) const {
#ifdef DEBUG
			  if(!outer) {
            glogger
            << "In GWebSocketConsumerT<processable_type>::GWorker::clone(): Error!" << std::endl
            << "Got empty pointer!" << std::endl
            << GEXCEPTION;
         }
#endif /* DEBUG */

			  return boost::shared_ptr<GDefaultWorker>(new GDefaultWorker(*this, thread_id, outer));
		  }

		  /************************************************************************/
		  /**
			* Actual per-item work is done here. Overload this function if you want
			* to do something different here.
			*/
		  virtual void process(boost::shared_ptr<processable_type> p) {
			  // Do the actual work
#ifdef DEBUG
			  if(p) p->process();
         else {
            glogger
            << "In GWebSocketConsumerT<processable_type>::GWorker::process(): Error!" << std::endl
            << "Received empty pointer for processable object" << std::endl
            << GEXCEPTION;
         }
#else
			  p->process();
#endif /* DEBUG */
		  }

		  /************************************************************************/
	 };
};

/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GWEBSOCKETCONSUMERT_HPP_ */
