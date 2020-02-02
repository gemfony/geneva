/**
 * @file GConsumerPerformance.cpp
 */

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

/**
 * TODO:
 * - Give clients the option not to return data
 * - Make the submitter optionally check for complete returns
 * - Make the submitter optionally return statistics of completed returns
 * - Catch "In GAsioAsyncServerSessionT::process(): Caught boost::system::system_error exception with messages: read: End of file"
 */

// Standard includes
#include <vector>
#include <functional>
#include <thread>
#include <mutex>

// Geneva includes
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GExecutorT.hpp"
#include "courtier/GAsioConsumerT.hpp"
#include "courtier/GStdThreadConsumerT.hpp"
#include "courtier/GSerialConsumerT.hpp"
#include "common/GExceptions.hpp"
#include "common/GThreadGroup.hpp"
#include "common/GParserBuilder.hpp"
#include "common/GCommonEnums.hpp"

#include "../../Misc/GRandomNumberContainer.hpp"
#include "../../Misc/GSimpleContainer.hpp"

std::size_t producer_counter;
std::mutex producer_counter_mutex;

using namespace Gem::Courtier;
using namespace Gem::Common;

// #define WORKLOAD GRandomNumberContainer
#define WORKLOAD Tests::GSimpleContainer

/********************************************************************************/

/**
 * This enum defines the available execution modes of the GBrokerSelfCommunication
 * example
 */
enum class GCPModes : Gem::Common::ENUMBASETYPE {
	 SERIAL = 0
	 , MULTITHREADING = 1
	 , INTERNALSERIALNETWORKING = 2
	 , EXTERNALSERIALNETWORKING = 3
	 , THREADANDINTERNALSERIALNETWORKING = 4
	 , THREAEDANDSERIALNETWORKING = 5
	 , INTERNALASYNCNETWORKING = 6
	 , EXTERNALASYNCNETWORKING = 7
	 , THREADANDINTERNALASYNCNETWORKING = 8
	 , THREAEDANDASYNCNETWORKING = 9
};

const GCPModes MAXGCPMODES = GCPModes::THREAEDANDSERIALNETWORKING;

/********************************************************************************/
/**
 * Puts a GBSCModesitem into a stream
 *
 * @param o The ostream the item should be added to
 * @param gbscmode the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const GCPModes& gbscmode) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(gbscmode);
	o << tmp;
	return o;
}

/********************************************************************************/
/**
 * Reads a Gem::Courtier::Tests::GBSCModes item from a stream
 *
 * @param i The stream the item should be read from
 * @param gbscmode The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream& operator>>(std::istream& i, GCPModes& gbscmode) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	gbscmode = boost::numeric_cast<GCPModes>(tmp);
#else
	gbscmode = static_cast<GCPModes>(tmp);
#endif /* DEBUG */

	return i;
}

/********************************************************************************/
// Default settings
const std::uint32_t DEFAULTNPRODUCERSAP = 5;
const std::uint32_t DEFAULTNPRODUCTIONCYLCESAP = 250;
const submissionReturnMode DEFAULTSRMAP = submissionReturnMode::INCOMPLETERETURN;
const std::size_t DEFAULTMAXRESUBMISSIONSAP = 5;
const std::uint32_t DEFAULTNCONTAINEROBJECTSAP = 100;
const std::size_t DEFAULTNCONTAINERENTRIESAP = 100;
const std::uint32_t DEFAULTNWORKERSAP = 4;
const GCPModes DEFAULTEXECUTIONMODEAP = GCPModes::MULTITHREADING;
const unsigned short DEFAULTPORTAP=10000;
const std::string DEFAULTIPAP="localhost";
const std::uint16_t DEFAULTPARALLELIZATIONMODEAP=0;
const Gem::Common::serializationMode DEFAULTSERMODEAP=Gem::Common::serializationMode::BINARY;
const bool DEFAULTUSEDIRECTBROKERCONNECTIONAP = false;

/********************************************************************************/
/**
 * A function that parses the command line for all required parameters
 */
bool parseCommandLine(
	int argc, char **argv
	, GCPModes &executionMode
	, bool &serverMode
	, std::string &ip
	, unsigned short &port
	, Gem::Common::serializationMode &serMode
	, bool &useDirectBrokerConnection
	, std::uint32_t &nProducers
	, std::uint32_t &nProductionCycles
	, std::uint32_t &nContainerObjects
	, std::size_t &nContainerEntries
	, std::size_t &maxResubmissions
	, std::uint32_t &nWorkers
) {
	// Create the parser builder
	Gem::Common::GParserBuilder gpb;

	gpb.registerCLParameter<GCPModes>(
		"executionMode,e"
		, executionMode
		, DEFAULTEXECUTIONMODEAP
		, "Whether to run this program with a serial consumer (0), multi-threaded (1), internal serial networking (2), serial networking (3), "
			"multithreaded and internal serial networking (4), multithreaded and serial networked mode (5),"
			"internal async networking (6), async networking (7), multithreaded and internal async networking (8) or multithreaded and async networking (9)"
	);

	gpb.registerCLParameter<bool>(
		"serverMode,s"
		, serverMode
		, false // Use client mode, if no server option is specified
		, "Whether to run networked execution in server or client mode. The option only has an effect for modes requiring a server. You can either say \"--serverMode=true\", just \"--serverMode\" or simple \"-s\"."
		, GCL_IMPLICIT_ALLOWED // Permit implicit values, so that we can say --server instead of --server=true
		, true // Use server mode, of only -s or --server was specified
	);

	gpb.registerCLParameter<std::string>(
		std::string("ip")
		, ip
		, DEFAULTIPAP
		, "The ip of the server"
	);

	gpb.registerCLParameter<unsigned short>(
		"port"
		, port
		, DEFAULTPORTAP
		, "The port on the server"
	);

	gpb.registerCLParameter<Gem::Common::serializationMode>(
		"serializationMode"
		, serMode
		, DEFAULTSERMODEAP
		, "Specifies whether serialization shall be done in TEXTMODE (0), XMLMODE (1) or BINARYMODE (2)"
	);

	gpb.registerCLParameter<bool>(
		"useDirectBrokerConnection"
		, useDirectBrokerConnection
		, DEFAULTUSEDIRECTBROKERCONNECTIONAP // Use connector object, if the option wasn't specified
		, "Indicates whether producers should connect directly to the broker or through the broker connector object"
		, true // Permit implicit values
		, true // Use direct connection, if only --useDirectBrokerConnection was specified
	);

	gpb.registerCLParameter<std::uint32_t>(
		"nProducers"
		, nProducers
		, DEFAULTNPRODUCERSAP
		, "The number of producers of work items"
	);

	gpb.registerCLParameter<std::uint32_t>(
		"nProductionCycles"
		, nProductionCycles
		, DEFAULTNPRODUCTIONCYLCESAP
		, "The number of production iterations perfomed by the program"
	);

	gpb.registerCLParameter<std::uint32_t>(
		"nContainerObjects"
		, nContainerObjects
		, DEFAULTNCONTAINEROBJECTSAP
		, "The number of container objects / work item prduced in one go"
	);

	gpb.registerCLParameter<std::size_t>(
		"nContainerEntries"
		, nContainerEntries
		, DEFAULTNCONTAINERENTRIESAP
		, "The number of entries stored in a container object"
	);

	gpb.registerCLParameter<std::size_t>(
		"maxResubmissions"
		, maxResubmissions
		, DEFAULTMAXRESUBMISSIONSAP
		, "The maximum number of times a work item may be resubmitted"
	);

	gpb.registerCLParameter<std::uint32_t>(
		"nWorkers"
		, nWorkers
		, DEFAULTNWORKERSAP
		, "The number of worker threads"
	);

	// Parse the command line and leave if the help flag was given. The parser
	// will emit an appropriate help message by itself
	if(Gem::Common::GCL_HELP_REQUESTED == gpb.parseCommandLine(argc, argv, true /*verbose*/)) {
		return false; // Do not continue
	}

	return true;
}

/********************************************************************************/
/**
 * Produces work items and submits them through the broker connector in different,
 * user-selectable modes, then retrieves them back.
 */
void connectorProducer(
	std::uint32_t nProductionCycles
	, std::uint32_t nContainerObjects
	, std::size_t nContainerEntries
	, std::size_t maxResubmissions
) {
	std::size_t id;

	{ // Assign a counter to this producer
		std::unique_lock<std::mutex> lk(producer_counter_mutex);
		id = producer_counter++;
	}

	// Holds the broker connector (i.e. the entity that connects us to the broker)
	Gem::Courtier::GBrokerExecutorT<WORKLOAD> brokerConnector;
	brokerConnector.init(); // This will particularly set up the buffer port
	brokerConnector.setMaxResubmissions(maxResubmissions);

	// Will hold the data items
	std::vector<std::shared_ptr<WORKLOAD>> data, oldWorkItems;

	// Start the loop
	std::uint32_t cycleCounter = 0;
	std::uint32_t nSentItems = 0;
	std::uint32_t nReceivedItemsNew = 0;
	std::uint32_t nReceivedItemsOld = 0;
	while(cycleCounter++ < nProductionCycles) {
		// Clear the data vector
		data.clear();
		oldWorkItems.clear();

		// Fill a std::vector<> with WORKLOAD objects
		for(std::size_t i=0; i<nContainerObjects; i++) {
			data.push_back(std::shared_ptr<WORKLOAD>(new WORKLOAD(nContainerEntries)));
		}
		nSentItems += boost::numeric_cast<std::uint32_t>(data.size());

		std::vector<bool> workItemPos(data.size(), Gem::Courtier::GBC_UNPROCESSED);
		for(auto item_ptr: data) {
			item_ptr->set_processing_status(Gem::Courtier::processingStatus::DO_PROCESS);
		}
		auto status = brokerConnector.workOn(
			data
			, false // Do not resubmit unprocessed items
		);

		// Take care of unprocessed items, if these exist
		if(!status.is_complete) {
			std::size_t n_erased = Gem::Common::erase_if(
				data
				, [](std::shared_ptr<WORKLOAD> p) -> bool {
					return (p->getProcessingStatus() == Gem::Courtier::processingStatus::DO_PROCESS);
				}
			);

#ifdef DEBUG
			glogger
				<< "In connectorProducer(): " << std::endl
				<< "Removed " << n_erased << " unprocessed work items in iteration " << std::endl
				<< GLOGGING;
#endif
		}

		// Remove items for which an error has occurred during processing
		if(status.has_errors) {
			std::size_t n_erased = Gem::Common::erase_if(
				data
				, [](std::shared_ptr<WORKLOAD> p) -> bool {
					return p->has_errors();
				}
			);

#ifdef DEBUG
			glogger
				<< "In connectorProducer(): " << std::endl
				<< "Removed " << n_erased << " erroneous work items in iteration " << std::endl
				<< GLOGGING;
#endif
		}

		// Receive a list of old work items
		oldWorkItems = brokerConnector.getOldWorkItems();

		nReceivedItemsNew += boost::numeric_cast<std::uint32_t>(data.size());
		nReceivedItemsOld += boost::numeric_cast<std::uint32_t>(oldWorkItems.size());

		std::cout << "Cycle " << cycleCounter << " completed in producer " << id << std::endl << std::flush;
	}

	brokerConnector.finalize(); // This will reset the buffer port

	std::cout
	<< "connectorProducer " << id << " has finished." << std::endl
	<< "Sent = " << nSentItems << std::endl
	<< "Received current = " << nReceivedItemsNew << std::endl
	<< "Received older = " << nReceivedItemsOld << std::endl
	<< "Total received = " << nReceivedItemsNew + nReceivedItemsOld << std::endl
	<< "Missing = " << nSentItems-(nReceivedItemsNew+nReceivedItemsOld) << std::endl << std::flush;
}

/********************************************************************************/
/**
 * Produces work items and submits them directly to the broker, then retrieves
 * them back. By bypassing the broker connector, we can detect differences
 * between both modes.
 */
void brokerProducer(
	std::uint32_t nProductionCycles
	, std::uint32_t nContainerObjects
	, std::size_t nContainerEntries
) {
	std::size_t id;
	using GBufferPortT_ptr = std::shared_ptr<Gem::Courtier::GBufferPortT<WORKLOAD>>;

	{ // Assign a counter to this producer
		std::unique_lock<std::mutex> lk(producer_counter_mutex);
		id = producer_counter++;
	}

	// Create a buffer port and register it with the broker
	GBufferPortT_ptr CurrentBufferPort(new Gem::Courtier::GBufferPortT<WORKLOAD>());
	GBROKER(WORKLOAD)->enrol(CurrentBufferPort);

	// Start the loop
	std::uint32_t cycleCounter = 0;
	while(cycleCounter++ < nProductionCycles) {
		// Submit the required number of items directly to the broker
		for(std::size_t i=0; i<nContainerObjects; i++) {
			CurrentBufferPort->push_raw(std::shared_ptr<WORKLOAD>(new WORKLOAD(nContainerEntries)));
		}

		// Wait for all items to return
		std::shared_ptr<WORKLOAD> p;
		for(std::size_t i=0; i<nContainerObjects; i++) {
			CurrentBufferPort->pop_processed(p);
			if(!p) {
				throw gemfony_exception(
					g_error_streamer(DO_LOG,  time_and_place)
						<< "In brokerProducer: " << "got invalid item" << std::endl
				);
			}
		}

		std::cout << "Cycle " << cycleCounter << " completed in producer " << id << std::endl << std::flush;
	}

	// Get rid of the buffer port object
	CurrentBufferPort.reset();

	std::cout << "brokerProducer " << id << " has finished producing" << std::endl << std::endl;
}

/********************************************************************************/
/**
 * This test tries to male bottlenecks in the broker architecture visible.
 */
int main(int argc, char **argv) {
	std::string configFile;
	bool serverMode;
	std::string ip;
	unsigned short port;
	Gem::Common::serializationMode serMode;
	std::uint32_t nProducers;
	std::uint32_t nProductionCycles;
	std::size_t maxResubmissions;
	std::uint32_t nContainerObjects;
	std::size_t nContainerEntries;
	std::uint32_t nWorkers;
	GCPModes executionMode;
	bool useDirectBrokerConnection;
	std::vector<std::shared_ptr<GBaseClientT<WORKLOAD>>> clients;

	// Initialize the global producer counter
	producer_counter = 0;

	// Some thread groups needed for producers and workers
	Gem::Common::GStdThreadGroup producer_gtg;
	Gem::Common::GStdThreadGroup worker_gtg;

	//--------------------------------------------------------------------------------
	// Find out about our configuration options
	if(!parseCommandLine(
		argc, argv
		, executionMode
		, serverMode
		, ip
		, port
		, serMode
		, useDirectBrokerConnection
		, nProducers
		, nProductionCycles
		, nContainerObjects
		, nContainerEntries
		, maxResubmissions
		, nWorkers
		)
	){ exit(0); }

	//--------------------------------------------------------------------------------
	// Initialize the broker
	GBROKER(WORKLOAD)->init();

	//--------------------------------------------------------------------------------
	// If we are in serial networked client mode, start corresponding the client code
	if((executionMode==GCPModes::EXTERNALSERIALNETWORKING || executionMode==GCPModes::THREAEDANDSERIALNETWORKING) && !serverMode) {
		std::shared_ptr<GAsioConsumerClientT<WORKLOAD>> p(new GAsioConsumerClientT<WORKLOAD>(ip, Gem::Common::to_string(port)));

		// Start the actual processing loop
		p->run();

		return 0;
	}

	//--------------------------------------------------------------------------------
	// If we are in async networked client mode, start corresponding the client code
	if((executionMode==GCPModes::EXTERNALASYNCNETWORKING || executionMode==GCPModes::THREAEDANDASYNCNETWORKING) && !serverMode) {
		std::shared_ptr<GAsioConsumerClientT<WORKLOAD>> p(new GAsioConsumerClientT<WORKLOAD>(ip, Gem::Common::to_string(port)));

		// Start the actual processing loop
		p->run();

		return 0;
	}

	//--------------------------------------------------------------------------------
	// Create the required number of connectorProducer threads
	if(useDirectBrokerConnection) {
		producer_gtg.create_threads(
			std::bind(
				brokerProducer
				, nProductionCycles
				, nContainerObjects
				, nContainerEntries
			)
			, nProducers
		);
	} else {
		producer_gtg.create_threads(
			std::bind(
				connectorProducer
				, nProductionCycles
				, nContainerObjects
				, nContainerEntries
				, maxResubmissions
			)
			, nProducers
		);
	}

	//--------------------------------------------------------------------------------
	// Add the desired consumers to the broker
	switch(executionMode) {
		case GCPModes::SERIAL:
		{
			std::cout << "Using a serial consumer" << std::endl;

			// Create a serial consumer and enrol it with the broker
			std::shared_ptr<GSerialConsumerT<WORKLOAD>> gatc(new GSerialConsumerT<WORKLOAD>());
			GBROKER(WORKLOAD)->enrol(gatc);
		}
			break;

		case GCPModes::MULTITHREADING:
		{
			std::cout << "Using the multithreaded mode" << std::endl;

			// Create a consumer and make it known to the global broker
			std::shared_ptr< GStdThreadConsumerT<WORKLOAD>> gbtc(new GStdThreadConsumerT<WORKLOAD>());
			gbtc->setNThreadsPerWorker(10);
			GBROKER(WORKLOAD)->enrol(gbtc);
		}
			break;

		case GCPModes::INTERNALSERIALNETWORKING:
		{
			std::cout << "Using internal serial networking" << std::endl;

			// Create a network consumer and enrol it with the broker
			std::shared_ptr<GAsioSerialTCPConsumerT<WORKLOAD>> gatc(new GAsioSerialTCPConsumerT<WORKLOAD>(port));
			GBROKER(WORKLOAD)->enrol(gatc);

			// Start the workers
			clients.clear();
			for(std::size_t worker=0; worker<nWorkers; worker++) {
				std::shared_ptr<GAsioSerialTCPClientT<WORKLOAD>> p(new GAsioSerialTCPClientT<WORKLOAD>("localhost", Gem::Common::to_string(port)));
				clients.push_back(p);

				worker_gtg.create_thread( [p](){ p->run(); } );
			}
		}
			break;

		case GCPModes::EXTERNALSERIALNETWORKING:
		{
			std::cout << "Using external serial networked mode" << std::endl;

			// Create a network consumer and enrol it with the broker
			std::shared_ptr<GAsioSerialTCPConsumerT<WORKLOAD>> gatc(new GAsioSerialTCPConsumerT<WORKLOAD>(port));
			GBROKER(WORKLOAD)->enrol(gatc);
		}
			break;

		case GCPModes::THREADANDINTERNALSERIALNETWORKING:
		{
			std::cout << "Using multithreading and internal serial networking" << std::endl;

			std::shared_ptr<GAsioSerialTCPConsumerT<WORKLOAD>> gatc(new GAsioSerialTCPConsumerT<WORKLOAD>(port));
			std::shared_ptr<GStdThreadConsumerT<WORKLOAD>> gbtc(new GStdThreadConsumerT<WORKLOAD>());

			std::vector<std::shared_ptr<GBaseConsumerT<WORKLOAD>>> consumers {gatc, gbtc};
			GBROKER(WORKLOAD)->enrol(consumers);

			// Start the workers
			clients.clear();
			for(std::size_t worker=0; worker<nWorkers; worker++) {
				std::shared_ptr<GAsioSerialTCPClientT<WORKLOAD>> p(new GAsioSerialTCPClientT<WORKLOAD>("localhost", Gem::Common::to_string(port)));
				clients.push_back(p);

				worker_gtg.create_thread( [p](){ p->run(); } );
			}
		}
			break;

		case GCPModes::THREAEDANDSERIALNETWORKING:
		{
			std::cout << "Using multithreading and external serial networked mode" << std::endl;

			std::shared_ptr<GAsioSerialTCPConsumerT<WORKLOAD>> gatc(new GAsioSerialTCPConsumerT<WORKLOAD>(port));
			std::shared_ptr< GStdThreadConsumerT<WORKLOAD>> gbtc(new GStdThreadConsumerT<WORKLOAD>());

			std::vector<std::shared_ptr<GBaseConsumerT<WORKLOAD>>> consumers {gatc, gbtc};
			GBROKER(WORKLOAD)->enrol(consumers);
		}
			break;

		case GCPModes::INTERNALASYNCNETWORKING:
		{
			std::cout << "Using internal async networking" << std::endl;

			// Create a network consumer and enrol it with the broker
			std::shared_ptr<GAsioAsyncTCPConsumerT<WORKLOAD>> gatc(new GAsioAsyncTCPConsumerT<WORKLOAD>(port));
			GBROKER(WORKLOAD)->enrol(gatc);

			// Start the workers
			clients.clear();
			for(std::size_t worker=0; worker<nWorkers; worker++) {
				std::shared_ptr<GAsioAsyncTCPClientT<WORKLOAD>> p(new GAsioAsyncTCPClientT<WORKLOAD>("localhost", Gem::Common::to_string(port)));
				clients.push_back(p);

				worker_gtg.create_thread( [p](){ p->run(); } );
			}
		}
			break;

		case GCPModes::EXTERNALASYNCNETWORKING:
		{
			std::cout << "Using external async networked mode" << std::endl;

			// Create a network consumer and enrol it with the broker
			std::shared_ptr<GAsioAsyncTCPConsumerT<WORKLOAD>> gatc(new GAsioAsyncTCPConsumerT<WORKLOAD>(port));
			GBROKER(WORKLOAD)->enrol(gatc);
		}
			break;

		case GCPModes::THREADANDINTERNALASYNCNETWORKING:
		{
			std::cout << "Using multithreading and internal async networking" << std::endl;

			std::shared_ptr<GAsioAsyncTCPConsumerT<WORKLOAD>> gatc(new GAsioAsyncTCPConsumerT<WORKLOAD>(port));
			std::shared_ptr<GStdThreadConsumerT<WORKLOAD>> gbtc(new GStdThreadConsumerT<WORKLOAD>());

			std::vector<std::shared_ptr<GBaseConsumerT<WORKLOAD>>> consumers {gatc, gbtc};
			GBROKER(WORKLOAD)->enrol(consumers);

			// Start the workers
			clients.clear();
			for(std::size_t worker=0; worker<nWorkers; worker++) {
				std::shared_ptr<GAsioAsyncTCPClientT<WORKLOAD>> p(new GAsioAsyncTCPClientT<WORKLOAD>("localhost", Gem::Common::to_string(port)));
				clients.push_back(p);

				worker_gtg.create_thread( [p](){ p->run(); } );
			}
		}
			break;

		case GCPModes::THREAEDANDASYNCNETWORKING:
		{
			std::cout << "Using multithreading and external async networked mode" << std::endl;

			std::shared_ptr<GAsioAsyncTCPConsumerT<WORKLOAD>> gatc(new GAsioAsyncTCPConsumerT<WORKLOAD>(port));
			std::shared_ptr<GStdThreadConsumerT<WORKLOAD>> gbtc(new GStdThreadConsumerT<WORKLOAD>());

			std::vector<std::shared_ptr<GBaseConsumerT<WORKLOAD>>> consumers {gatc, gbtc};
			GBROKER(WORKLOAD)->enrol(consumers);
		}
			break;
	};

	//--------------------------------------------------------------------------------
	// Wait for all producer threads to finish
	producer_gtg.join_all();

	if(
		executionMode == GCPModes::INTERNALSERIALNETWORKING
		|| executionMode == GCPModes::THREADANDINTERNALSERIALNETWORKING
		|| executionMode == GCPModes::INTERNALASYNCNETWORKING
		|| executionMode == GCPModes::THREADANDINTERNALASYNCNETWORKING
	) {
		for(auto p: clients) {
			p->flagCloseRequested();
		}

		worker_gtg.join_all();
	}

	std::cout << "All threads have joined" << std::endl;

	// Terminate the broker
	GBROKER(WORKLOAD)->finalize();
}
