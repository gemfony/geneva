/**
 * @file GBrokerSelfCommunication.cpp
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

#include <vector>
#include <functional>
#include <thread>
#include <mutex>

#include "courtier/GCourtierEnums.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GExecutorT.hpp"
#include "courtier/GAsioSerialTCPConsumerT.hpp"
#include "courtier/GBoostThreadConsumerT.hpp"
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
enum class GBSCModes : Gem::Common::ENUMBASETYPE {
	 SERIAL = 0
	 , INTERNALNETWORKING = 1
	 , NETWORKING = 2
	 , MULTITHREADING = 3
	 , THREADANDINTERNALNETWORKING = 4
	 , THREAEDANDNETWORKING = 5
};

const GBSCModes MAXGBSCMODES = GBSCModes::THREAEDANDNETWORKING;

/********************************************************************************/
/**
 * Puts a GBSCModesitem into a stream
 *
 * @param o The ostream the item should be added to
 * @param gbscmode the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream& operator<<(std::ostream& o, const GBSCModes& gbscmode) {
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
std::istream& operator>>(std::istream& i, GBSCModes& gbscmode) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	gbscmode = boost::numeric_cast<GBSCModes>(tmp);
#else
	gbscmode = static_cast<GBSCModes>(tmp);
#endif /* DEBUG */

	return i;
}

/********************************************************************************/
// Default settings
const std::uint32_t DEFAULTNPRODUCERSAP = 5;
const std::uint32_t DEFAULTNPRODUCTIONCYLCESAP = 10000;
const submissionReturnMode DEFAULTSRMAP = submissionReturnMode::INCOMPLETERETURN;
const std::size_t DEFAULTMAXRESUBMISSIONSAP = 5;
const std::uint32_t DEFAULTNCONTAINEROBJECTSAP = 100;
const std::size_t DEFAULTNCONTAINERENTRIESAP = 100;
const std::uint32_t DEFAULTNWORKERSAP = 4;
const GBSCModes DEFAULTEXECUTIONMODEAP = GBSCModes::MULTITHREADING;
const unsigned short DEFAULTPORTAP=10000;
const std::string DEFAULTIPAP="localhost";
const std::string DEFAULTCONFIGFILEAP="./GBrokerSelfCommunication.cfg";
const std::uint16_t DEFAULTPARALLELIZATIONMODEAP=0;
const Gem::Common::serializationMode DEFAULTSERMODEAP=Gem::Common::serializationMode::SERIALIZATIONMODE_BINARY;
const bool DEFAULTUSEDIRECTBROKERCONNECTIONAP = false;

/********************************************************************************/
/**
 * A function that parses the command line for all required parameters
 */
bool parseCommandLine(
	int argc, char **argv
	, GBSCModes &executionMode
	, bool &serverMode
	, std::string &ip
	, unsigned short &port
	, Gem::Common::serializationMode &serMode
	, submissionReturnMode &srm
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

	gpb.registerCLParameter<GBSCModes>(
		"executionMode,e"
		, executionMode
		, DEFAULTEXECUTIONMODEAP
		, "\"Whether to run this program with a serial consumer (0), with internal networking (1), networking (2), multi-threaded (3), multithreaded and internal networking (4) or multithreaded and networked mode (5)\""
	);

	gpb.registerCLParameter<bool>(
		"serverMode,s"
		, serverMode
		, false // Use client mode, if no server option is specified
		, "Whether to run networked execution in server or client mode. The option only has an effect if \"--parallelizationMode=2\". You can either say \"--server=true\" or just \"--server\"."
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

	gpb.registerCLParameter<submissionReturnMode>(
		"srm,f"
		, srm
		, DEFAULTSRMAP
		, "Whether items from older iterations may return and an incomplete return is acceptable (0), items "
			"should be resubmitted (1) or whether a complete return of a given submission\'s items is required"
	);

	gpb.registerCLParameter<bool>(
		"useDirectBrokerConnection"
		, useDirectBrokerConnection
		, DEFAULTUSEDIRECTBROKERCONNECTIONAP // Use client mode, if no server option is specified
		, "Indicates whether producers should connect directly to the broker or through the broker connector object"
		, true // Permit implicit values
		, true // Use server mode, if only -u or --useDirectBrokerConnection was specified
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
		, "The number of contaoner objects / work item prduced in one go"
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
	, submissionReturnMode srm
	, std::size_t maxResubmissions
) {
	std::size_t id;

	{ // Assign a counter to this producer
		std::unique_lock<std::mutex> lk(producer_counter_mutex);
		id = producer_counter++;
	}

	// Holds the broker connector (i.e. the entity that connects us to the broker)
	Gem::Courtier::GBrokerExecutorT<WORKLOAD> brokerConnector(srm);
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

		bool complete = brokerConnector.workOn(
			data
			, std::tuple<std::size_t,std::size_t>(0, data.size())
			, oldWorkItems
			, true // Remove unprocessed items
		);

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
	using GBufferPortT_ptr = std::shared_ptr<Gem::Courtier::GBufferPortT<std::shared_ptr<WORKLOAD>>>;

	{ // Assign a counter to this producer
		std::unique_lock<std::mutex> lk(producer_counter_mutex);
		id = producer_counter++;
	}

	// Create a buffer port and register it with the broker
	GBufferPortT_ptr CurrentBufferPort_(new Gem::Courtier::GBufferPortT<std::shared_ptr<WORKLOAD>>());
	GBROKER(WORKLOAD)->enrol(CurrentBufferPort_);

	// Start the loop
	std::uint32_t cycleCounter = 0;
	while(cycleCounter++ < nProductionCycles) {
		// Submit the required number of items directly to the broker
		for(std::size_t i=0; i<nContainerObjects; i++) {
			CurrentBufferPort_->push_raw(std::shared_ptr<WORKLOAD>(new WORKLOAD(nContainerEntries)));
		}

		// Wait for all items to return
		std::shared_ptr<WORKLOAD> p;
		for(std::size_t i=0; i<nContainerObjects; i++) {
			CurrentBufferPort_->pop_processed(p);
			if(!p) {
				raiseException(
					"In brokerProducer: " << "got invalid item" << std::endl
				);
			}
		}

		std::cout << "Cycle " << cycleCounter << " completed in producer " << id << std::endl << std::flush;
	}

	// Get rid of the buffer port object
	CurrentBufferPort_.reset();

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
	submissionReturnMode srm;
	std::size_t maxResubmissions;
	std::uint32_t nContainerObjects;
	std::size_t nContainerEntries;
	std::uint32_t nWorkers;
	GBSCModes executionMode;
	bool useDirectBrokerConnection;
	std::vector<std::shared_ptr<GAsioSerialTCPClientT<WORKLOAD>> > clients;

	// Initialize the global producer counter
	producer_counter = 0;

	// Some thread groups needed for producers and workers
	Gem::Common::GThreadGroup connectorProducer_gtg;
	Gem::Common::GThreadGroup worker_gtg;

	//--------------------------------------------------------------------------------
	// Find out about our configuration options
	if(!parseCommandLine(
		argc, argv
		, executionMode
		, serverMode
		, ip
		, port
		, serMode
		, srm
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
	// If we are in (networked) client mode, start the client code
	if((executionMode==GBSCModes::NETWORKING || executionMode==GBSCModes::THREAEDANDNETWORKING) && !serverMode) {
		std::shared_ptr<GAsioSerialTCPClientT<WORKLOAD>> p(new GAsioSerialTCPClientT<WORKLOAD>(ip, boost::lexical_cast<std::string>(port)));

		p->setMaxStalls(0); // An infinite number of stalled data retrievals
		p->setMaxConnectionAttempts(100); // Up to 100 failed connection attempts

		// Start the actual processing loop
		p->run();

		return 0;
	}

	//--------------------------------------------------------------------------------
	// Create the required number of connectorProducer threads
	if(useDirectBrokerConnection) {
		connectorProducer_gtg.create_threads(
			std::bind(
				brokerProducer
				, nProductionCycles
				, nContainerObjects
				, nContainerEntries
			)
			, nProducers
		);
	} else {
		connectorProducer_gtg.create_threads(
			std::bind(
				connectorProducer
				, nProductionCycles
				, nContainerObjects
				, nContainerEntries
				, srm
				, maxResubmissions
			)
			, nProducers
		);
	}

	//--------------------------------------------------------------------------------
	// Add the desired consumers to the broker
	switch(executionMode) {
		case GBSCModes::SERIAL:
		{
			std::cout << "Using a serial consumer" << std::endl;

			// Create a serial consumer and enrol it with the broker
			std::shared_ptr<GSerialConsumerT<WORKLOAD>> gatc(new GSerialConsumerT<WORKLOAD>());
			GBROKER(WORKLOAD)->enrol(gatc);
		}
			break;

		case GBSCModes::INTERNALNETWORKING:
		{
			std::cout << "Using internal networking" << std::endl;

			// Create a network consumer and enrol it with the broker
			std::shared_ptr<GAsioSerialTCPConsumerT<WORKLOAD>> gatc(new GAsioSerialTCPConsumerT<WORKLOAD>((unsigned short)10000));
			GBROKER(WORKLOAD)->enrol(gatc);

			// Start the workers
			clients.clear();
			for(std::size_t worker=0; worker<nWorkers; worker++) {
				std::shared_ptr<GAsioSerialTCPClientT<WORKLOAD>> p(new GAsioSerialTCPClientT<WORKLOAD>("localhost", "10000"));
				clients.push_back(p);

				worker_gtg.create_thread( [p](){ p->run(); } );
			}
		}
			break;

		case GBSCModes::NETWORKING:
		{
			std::cout << "Using networked mode" << std::endl;

			// Create a network consumer and enrol it with the broker
			std::shared_ptr<GAsioSerialTCPConsumerT<WORKLOAD>> gatc(new GAsioSerialTCPConsumerT<WORKLOAD>(port));
			GBROKER(WORKLOAD)->enrol(gatc);
		}
			break;

		case GBSCModes::MULTITHREADING:
		{
			std::cout << "Using the multithreaded mode" << std::endl;

			// Create a consumer and make it known to the global broker
			std::shared_ptr< GBoostThreadConsumerT<WORKLOAD>> gbtc(new GBoostThreadConsumerT<WORKLOAD>());
			gbtc->setNThreadsPerWorker(10);
			GBROKER(WORKLOAD)->enrol(gbtc);
		}
			break;

		case GBSCModes::THREADANDINTERNALNETWORKING:
		{
			std::cout << "Using multithreading and internal networking" << std::endl;

			std::shared_ptr<GAsioSerialTCPConsumerT<WORKLOAD>> gatc(new GAsioSerialTCPConsumerT<WORKLOAD>((unsigned short)10000));
			std::shared_ptr< GBoostThreadConsumerT<WORKLOAD>> gbtc(new GBoostThreadConsumerT<WORKLOAD>());

			GBROKER(WORKLOAD)->enrol(gatc);
			GBROKER(WORKLOAD)->enrol(gbtc);

			// Start the workers
			clients.clear();
			for(std::size_t worker=0; worker<nWorkers; worker++) {
				std::shared_ptr<GAsioSerialTCPClientT<WORKLOAD>> p(new GAsioSerialTCPClientT<WORKLOAD>("localhost", "10000"));
				clients.push_back(p);

				worker_gtg.create_thread( [p](){ p->run(); } );
			}
		}
			break;

		case GBSCModes::THREAEDANDNETWORKING:
		{
			std::cout << "Using multithreading and the networked mode" << std::endl;

			std::shared_ptr<GAsioSerialTCPConsumerT<WORKLOAD>> gatc(new GAsioSerialTCPConsumerT<WORKLOAD>(port));
			std::shared_ptr< GBoostThreadConsumerT<WORKLOAD>> gbtc(new GBoostThreadConsumerT<WORKLOAD>());

			GBROKER(WORKLOAD)->enrol(gatc);
			GBROKER(WORKLOAD)->enrol(gbtc);
		}
			break;

		default:
		{
			raiseException(
				"Error: Invalid execution mode requested: " << executionMode << std::endl
			);
		}
			break;
	};

	//--------------------------------------------------------------------------------
	// Wait for all threads to finish
	connectorProducer_gtg.join_all();

	if(
		executionMode == GBSCModes::INTERNALNETWORKING ||
		executionMode == GBSCModes::THREADANDINTERNALNETWORKING
	) {
		worker_gtg.join_all();
	}

	std::cout << "All threads have joined" << std::endl;

	// Terminate the broker
	GBROKER(WORKLOAD)->finalize();
}
